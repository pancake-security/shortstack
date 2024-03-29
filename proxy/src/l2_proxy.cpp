// Shortstack L2 proxy implementation

#include <spdlog/spdlog.h>
#include <chrono>
#include "l2_proxy.h"

#include "consistent_hash.h"

void l2_proxy::init_proxy(std::shared_ptr<host_info> hosts,
                          std::string instance_name,
                          std::shared_ptr<distribution_info> dist_info,
                          std::shared_ptr<update_cache> update_cache,
                          bool uc_enabled, int local_idx, bool stats,
                          int ack_batch_size) {
  hosts_ = hosts;
  instance_name_ = instance_name;
  update_cache_enabled_ = uc_enabled;
  stats_ = stats;
  ack_batch_size_ = ack_batch_size;
  chain_ack_batch_size_ = ack_batch_size;

  replica_to_label_ = dist_info->replica_to_label_;
  key_to_number_of_replicas_ = dist_info->key_to_number_of_replicas_;
  dummy_key_ = dist_info->dummy_key_;

  host this_host;
  if (!hosts->get_host(instance_name, this_host)) {
    throw std::runtime_error("Unkown instance name: " + instance_name);
  }

  int base_idx;
  hosts->get_base_idx(instance_name_, base_idx);
  idx_ = base_idx + local_idx;

  update_cache_ = update_cache;

  int num_l1_cols = hosts->get_num_columns(HOST_TYPE_L1, true);
  for(int i = 0; i < num_l1_cols; i++) {
    last_seen_seq_.push_back(-1);
  }

  // Setup chain_module
  std::vector<host> replicas;
  hosts->get_replicas(HOST_TYPE_L2, this_host.column, replicas);
  chain_role role;
  if(replicas.size() == 1) {
    role = chain_role::singleton;
  } else if (replicas.front().instance_name == this_host.instance_name) {
    role = chain_role::head;
  } else if (replicas.back().instance_name == this_host.instance_name) {
    role = chain_role::tail;
  } else {
    role = chain_role::mid;
  }
  std::vector<std::string> chain;
  std::string next_block_id = "nil";
  int next_row = this_host.row + 1;
  for(auto &h : replicas) {
    auto bid = block_id_parser::make(h.hostname, h.port + local_idx, h.port + local_idx, local_idx);
    chain.push_back(bid);
    if(h.row == next_row) {
      next_block_id = bid;
    }
  }
  setup("/", chain, role, next_block_id);
  spdlog::info("Worker {}: Chain module setup", idx_);

  spdlog::info("Initialized L2 proxy");
}

void l2_proxy::async_operation(const sequence_id &seq_id,
                               const std::string &key, int replica,
                               const std::string &value) {
  l2_operation op;
  op.seq_id = seq_id;
  op.key = key;
  op.replica = replica;
  op.value = value;

  spdlog::debug("recvd op client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);

  if(op.seq_id.l1_seq_no <= last_seen_seq_[op.seq_id.l1_idx]) {
    spdlog::info("Received duplicate L1 request, l1_idx: {}, l1_seq_no: {}", op.seq_id.l1_idx, op.seq_id.l1_seq_no);
    return;
  }

  if(!is_head()) {
    spdlog::error("Received direct request at non-head node");
    throw std::runtime_error("Received direct request at non-head node");
    return;
  }

  if(stats_) {
    int64_t us_from_epoch = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    op.seq_id.__set_ts(us_from_epoch);
  }

  if (key_to_number_of_replicas_.find(op.key) ==
      key_to_number_of_replicas_.end()) {
        spdlog::error("Key not found in key_to_number_of_replicas_: {}", op.key);
        throw std::runtime_error("Key not found");
        return;
  }

  // Replicate request
  std::vector<std::string> args;
  op.serialize(args);
  chain_request(op.seq_id, args);

  // Execution to be continued in replication_complete() callback at tail
}

void l2_proxy::close() {
  // finished_.store(true);
  // // push dummy ops into queues to unblock
  // l2_operation dummy;
  // dummy.seq_id.client_id = -1;
  // dummy.key = "$end$";
  // dummy.replica = -1;
  // dummy.value = "";
  // for (int i = 0; i < operation_queues_.size(); i++) {
  //   operation_queues_[i]->push(dummy);
  // }
  // for (int i = 0; i < threads_.size(); i++) {
  //   threads_[i].join();
  // }
}

void l2_proxy::run_command(const sequence_id &seq, const arg_list &args) {
  
  spdlog::debug("run_command, server_seq_no: {}, len(args): {}", seq.server_seq_no, args.size());

  // Update per-L1 sequence number
  if(seq.l1_seq_no <= last_seen_seq_[seq.l1_idx]) {
    spdlog::error("chain command with out-of-order sequency number, l1_seq_no: {}, l1_idx: {}, last_seen: {}", seq.l1_seq_no, seq.l1_idx, last_seen_seq_[seq.l1_idx]);
    throw std::runtime_error("chain command with out-of-order sequency number");
    return;
  }

  last_seen_seq_[seq.l1_idx] = seq.l1_seq_no;

  l2_operation op;
  op.deserialize(args, 0);
  op.seq_id = seq;

  if (update_cache_enabled_ && op.value != "") {
    update_cache_->populate_replica_updates(
        op.key, op.value, key_to_number_of_replicas_[op.key]);
  }
  
}

void l2_proxy::replication_complete(const sequence_id &seq, const arg_list &args) {
  
  spdlog::debug("replication_complete, server_seq_no: {}, len(args): {}", seq.server_seq_no, args.size());
  
  forward_request(seq, args, true);
  
}

void l2_proxy::setup_callback() {

  if(is_tail() && l3_iface_ == nullptr) {
    std::vector<host> l3_hosts;
    hosts_->get_hosts_by_type(HOST_TYPE_L3, l3_hosts);

    std::vector<std::string> l3_hostnames;
    std::vector<int> l3_ports;
    for (auto h : l3_hosts) {
      l3_hostnames.push_back(h.hostname);
      l3_ports.push_back(h.port);
    }

    // int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    
    l3_iface_ =  std::make_shared<l3proxy_interface>(hosts_);

    // Connect to L3 servers
    l3_iface_->connect();

    spdlog::info("Worker {}: L3 interface connected", idx_);
  }

  if(is_head() && ack_iface_ == nullptr) {
    ack_iface_ = std::make_shared<l1ack_interface>(hosts_, ack_batch_size_);
    spdlog::info("Worker {}: L1 ack interface initialized", idx_);
  }
  
}

void l2_proxy::update_connections(int type, int column, std::string hostname, int port, int num_workers) {
  if(type == HOST_TYPE_L3) {
    if(!is_tail() || l3_iface_ == nullptr) {
      spdlog::error("update_connections called on non-tail L2 node");
      throw std::runtime_error("Invalid update_connections call");
      return;
    }

    if(hostname != "nil") {
      spdlog::error("Invalid update_connections call: hostname not nil");
      throw std::runtime_error("Invalid update_connections call");
      return;
    }

    l3_iface_->remove_connection(column);
    spdlog::info("Removed L3 connection for column: {}", column);

  } else if(type == HOST_TYPE_L1) {
      if(!is_head() || ack_iface_ == nullptr) {
        spdlog::error("update_connections called on non-head L2 node");
        throw std::runtime_error("Invalid update_connections call");
        return;
      }

      ack_iface_->update_connections(column, hostname, port, num_workers);
      spdlog::info("Updated L1 connection for column: {}", column);
  } else {
    spdlog::error("Invalid update_connections call");
    throw std::runtime_error("Invalid update_connections call");
    return;
  }
  
}

// Selectively resend pending requests that hash to a given column
void l2_proxy::selective_resend_pending(const int32_t column, const int32_t num_columns) {
  if(!is_tail()) {
    spdlog::error("selective_resend_pending called on non-tail node");
    throw std::runtime_error("selective_resend_pending called on non-tail node");
    return;
  }

  auto ops = pending_.lock_table();
  try {
    for (const auto &op: ops) {
      if(filter_request(op.second.seq, op.second.args, column, num_columns)) {
        forward_request(op.second.seq, op.second.args, false);
      }
    }
  } catch (...) {
    ops.unlock();
    std::rethrow_exception(std::current_exception());
  }
  ops.unlock();
  spdlog::info("Selectively resent pending requests");
}

void l2_proxy::forward_request(const sequence_id &seq, const arg_list &args, bool dedup) {
  l2_operation op;
  op.deserialize(args, 0);
  op.seq_id = seq;
  

  std::string plaintext_update;
  if(update_cache_enabled_) {
    plaintext_update = update_cache_->check_for_update_immutable(op.key, op.replica);
  } else {
    plaintext_update = op.value;
  }
  
  spdlog::debug("plaintext_update={}..., client_id:{}, seq_no:{}", plaintext_update.substr(0,5), op.seq_id.client_id, op.seq_id.client_seq_no);

  auto it = replica_to_label_.find(op.key + std::to_string(op.replica));
  if (it == replica_to_label_.end()) {
    spdlog::error("Replica not found in label map: {}, {}", op.key, op.replica);
    throw std::runtime_error("Replica not found in label map");
    return;
  }
  auto label = std::to_string(it->second);

  if(l3_iface_ == nullptr) {
    spdlog::error("replication_complete on non-tail node");
    return;
  }

  if(stats_) {
        int64_t us_from_epoch = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        auto elapsed = us_from_epoch - op.seq_id.ts;
        op.seq_id.__set_diag(op.seq_id.diag + std::to_string(elapsed) + ",");
      }

  // Send to L3
  l3_operation l3_op;
  l3_op.seq_id = op.seq_id;
  l3_op.seq_id.l2_idx = idx_;
  l3_op.seq_id.l2_seq_no = seq.server_seq_no;
  l3_op.label = label;
  l3_op.value = plaintext_update;
  l3_op.is_read = op.value == "";
  l3_op.dedup = dedup;
  l3_iface_->send_op(l3_op);
}

bool l2_proxy::filter_request(const sequence_id &seq, const arg_list &args, int column, int num_columns) {
    l2_operation op;
    op.deserialize(args, 0);
    op.seq_id = seq;

    auto it = replica_to_label_.find(op.key + std::to_string(op.replica));
    if (it == replica_to_label_.end()) {
      spdlog::error("Replica not found in label map: {}, {}", op.key, op.replica);
      throw std::runtime_error("Replica not found in label map");
    }
    auto label = std::to_string(it->second);

    auto id = consistent_hash(label, num_columns);

    // TODO: Hack
    return (id >= column && id < column + L3_NUM_CORES);
}

void l2_proxy::external_ack(const sequence_id& seq) {
  // TODO: Temp debugging
  spdlog::debug("Recvd external ack, l2_seq_no: {}", seq.l2_seq_no);
  sequence_id seq_id = seq;
  seq_id.server_seq_no = seq_id.l2_seq_no;
  ack(seq_id);
}

void l2_proxy::external_ack_batch(const std::vector<sequence_id> & seqs) {
  for(auto & seq : seqs) {
    external_ack(seq);
  }
}

l1ack_interface::l1ack_interface(std::shared_ptr<host_info> hosts, int batch_size)
: reverse_connector(hosts, HOST_TYPE_L1, batch_size) {

}

int l1ack_interface::route(const sequence_id &seq) {
  return seq.l1_idx;
}

void l2_proxy::ack_callback(const sequence_id &seq) {
  if(is_head()) {
    if(ack_iface_ == nullptr) {
      spdlog::error("Invalide state: ack_iface is null at L2 head");
      throw std::runtime_error("Invalid state");
    }

    ack_iface_->send_ack(seq);
  }
}