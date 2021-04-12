// Shortstack L1 proxy implementation

#include <spdlog/spdlog.h>

#include "l1_proxy.h"

void l1_proxy::init_proxy(std::shared_ptr<host_info> hosts,
                          std::string instance_name,
                          std::shared_ptr<distribution_info> dist_info,
                          int local_idx) {
  hosts_ = hosts;
  instance_name_ = instance_name;

  num_keys_ = dist_info->num_keys_;
  dummy_key_ = dist_info->dummy_key_;
  delta_ = 0.5;
  key_to_number_of_replicas_ = dist_info->key_to_number_of_replicas_;
  fake_distribution_ = dist_info->fake_distribution_;
  real_distribution_ = dist_info->real_distribution_;

  host this_host;
  if (!hosts->get_host(instance_name, this_host)) {
    throw std::runtime_error("Unkown instance name: " + instance_name);
  }

  int base_idx;
  hosts->get_base_idx(instance_name_, base_idx);
  idx_ = base_idx + local_idx;

  // Setup chain_module
  std::vector<host> replicas;
  hosts->get_replicas(HOST_TYPE_L1, this_host.column, replicas);
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

  spdlog::info("Initialized L1 proxy");
}

void l1_proxy::init(const std::vector<std::string> &keys,
                    const std::vector<std::string> &values, void **args) {
  throw std::logic_error("Not implemented");
}

bool l1_proxy::is_true_distribution() { return prob(delta_); };

void l1_proxy::create_security_batch(std::queue<l1_operation> &q,
                                     std::vector<l2_operation> &batch,
                                     std::vector<bool> &is_trues) {

  bool atleast_one_true = false;
  for (int i = 0; i < security_batch_size_; i++) {
    l2_operation operat;
    if (is_true_distribution()) {
      // std::cerr << "True toss" << std::endl;
      atleast_one_true = true;
      if (q.empty()) {
        operat.seq_id.client_id = fake_client_id_;
        operat.key = real_distribution_.sample();
        operat.value = "";
        is_trues.push_back(false);
      } else {
        const l1_operation &op = q.front();
        operat.seq_id = op.seq_id;
        operat.key = op.key;
        operat.value = op.value;
        is_trues.push_back(true);
        q.pop();
      }
    } else {
      // std::cerr << "False toss" << std::endl;
      operat.seq_id.client_id = fake_client_id_;
      operat.key = fake_distribution_.sample();
      operat.value = "";
      is_trues.push_back(false);
    }

    auto it = key_to_number_of_replicas_.find(operat.key);
    if (it == key_to_number_of_replicas_.end()) {
      throw std::runtime_error("key not found: " + operat.key);
    }

    operat.replica = rand_uint32(0, it->second - 1);

    batch.push_back(operat);
  }

  if(!atleast_one_true) {
    spdlog::debug("Batch with all false requests");
  }
}

std::string l1_proxy::get(const std::string &key) {
  throw std::logic_error("Not implemented");
};

void l1_proxy::async_get(const sequence_id &seq_id, const std::string &key) {
  async_get(seq_id, rand_uint32(0, RAND_MAX), key);
};

void l1_proxy::put(const std::string &key, const std::string &value) {
  throw std::logic_error("Not implemented");
};

void l1_proxy::async_put(const sequence_id &seq_id, const std::string &key,
                         const std::string &value) {
  async_put(seq_id, rand_uint32(0, RAND_MAX), key, value);
};

std::vector<std::string>
l1_proxy::get_batch(const std::vector<std::string> &keys) {
  throw std::logic_error("Not implemented");
};

void l1_proxy::async_get_batch(const sequence_id &seq_id,
                               const std::vector<std::string> &keys) {
  async_get_batch(seq_id, rand_uint32(0, RAND_MAX), keys);
};

void l1_proxy::put_batch(const std::vector<std::string> &keys,
                         const std::vector<std::string> &values) {
  throw std::logic_error("Not implemented");
};

void l1_proxy::async_put_batch(const sequence_id &seq_id,
                               const std::vector<std::string> &keys,
                               const std::vector<std::string> &values) {
  async_put_batch(seq_id, rand_uint32(0, RAND_MAX), keys, values);
};

std::string l1_proxy::get(int queue_id, const std::string &key) {
  throw std::logic_error("Not implemented");
};

void l1_proxy::async_get(const sequence_id &seq_id, int queue_id,
                         const std::string &key) {
  l1_operation operat;
  operat.seq_id = seq_id;
  operat.key = key;
  operat.value = "";
  // operation_queues_[queue_id % operation_queues_.size()]->push(operat);
  process_op(operat);
};

void l1_proxy::put(int queue_id, const std::string &key,
                   const std::string &value) {
  throw std::logic_error("Not implemented");
};

void l1_proxy::async_put(const sequence_id &seq_id, int queue_id,
                         const std::string &key, const std::string &value) {
  l1_operation operat;
  operat.seq_id = seq_id;
  operat.key = key;
  operat.value = value;
  // operation_queues_[queue_id % operation_queues_.size()]->push(operat);
  process_op(operat);
};

std::vector<std::string>
l1_proxy::get_batch(int queue_id, const std::vector<std::string> &keys) {
  throw std::logic_error("Not implemented");
};

void l1_proxy::async_get_batch(const sequence_id &seq_id, int queue_id,
                               const std::vector<std::string> &keys) {
  for (auto k : keys) {
    async_get(seq_id, queue_id, k);
  }
};

void l1_proxy::run_command(const sequence_id &seq, const arg_list &args) {
  // TODO: Maintain per-request ACK count
  spdlog::debug("run_command, server_seq_no: {}, len(args): {}", seq.server_seq_no, args.size());
}

void l1_proxy::replication_complete(const sequence_id &seq, const arg_list &args) {

  spdlog::debug("replication_complete, server_seq_no: {}, len(args): {}", seq.server_seq_no, args.size());

  std::vector<l2_operation> batch;
  int idx = 0;
  while(idx < args.size()) {
    l2_operation op;
    int count = op.deserialize(args, idx);
    batch.push_back(op);
    idx += count;
  }

  if(batch.size() != security_batch_size_) {
    throw std::logic_error("Incorrectly sized security batch");
  }
  
  if(l2_iface_ == nullptr) {
    spdlog::error("replication_complete on non-tail node");
    return;
  }
  // Forward requests in batch to L2
    for (auto &op : batch) {
      // TODO: Update l1_seq_no in op
      l2_iface_->send_op(op);
    }
}

void l1_proxy::setup_callback() {
  if(is_tail() && l2_iface_ == nullptr) {
    std::vector<host> l2_hosts;
    hosts_->get_hosts_by_type(HOST_TYPE_L2, l2_hosts);

    std::vector<std::string> l2_hostnames;
    std::vector<int> l2_ports;
    for (auto h : l2_hosts) {
      l2_hostnames.push_back(h.hostname);
      l2_ports.push_back(h.port);
    }

    l2_iface_ = std::make_shared<l2proxy_interface>(l2_hostnames, l2_ports, dummy_key_);

    // Connect to L2 servers
    l2_iface_->connect();

    spdlog::info("Worker {}: L2 interface connected", idx_);
  }
}


void l1_proxy::put_batch(int queue_id, const std::vector<std::string> &keys,
                         const std::vector<std::string> &values) {
  throw std::logic_error("Not implemented");
};

void l1_proxy::async_put_batch(const sequence_id &seq_id, int queue_id,
                               const std::vector<std::string> &keys,
                               const std::vector<std::string> &values) {
  for (int i = 0; i < (int)keys.size(); i++) {
    async_put(seq_id, queue_id, keys[i], values[i]);
  }
};

// void l1_proxy::consumer_thread(int id) {
//   // TODO: Handle exceptions
//   std::shared_ptr<l2proxy_interface> l2_interface = l2_ifaces_[id];

//   // Connect to L2 servers
//   l2_interface->connect();

//   spdlog::info("Consumer {}: L2 interface connected", id);

//   std::queue<l1_operation> internal_queue;
//   while (true) {
//     auto op = operation_queues_[id]->pop(); // Blocking call
//     spdlog::debug("recvd op client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);
//     if (finished_.load()) {
//       break;
//     }

//     // Generate batch
//     internal_queue.push(op);
//     std::vector<l2_operation> batch;
//     std::vector<bool> is_trues;
//     create_security_batch(internal_queue, batch, is_trues);

//     // Forward requests in batch to L2
//     for (auto &op : batch) {
//       l2_interface->send_op(op);
//     }
//   }
// }

void l1_proxy::process_op(const l1_operation &op) {
  spdlog::debug("recvd op client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);
  if(!is_head()) {
    spdlog::error("Received direct request at non-head node");
    return;
  }
    // Generate batch
    internal_queue_.push(op);
    std::vector<l2_operation> batch;
    std::vector<bool> is_trues;
    create_security_batch(internal_queue_, batch, is_trues);

  // Replicate request
  std::vector<std::string> args;
  for(auto &op : batch) {
    op.serialize(args);
  }
  sequence_id seq;
  seq.client_id = -1;
  seq.client_seq_no = -1;
  seq.server_seq_no = -1;
  chain_request(seq, args);

  // To be continued in replication_complete callback at tail

}

void l1_proxy::flush() { throw std::logic_error("not implemented"); }

void l1_proxy::close() {
  // finished_.store(true);
  // // TODO: push dummy ops into queues to unblock
  // for (int i = 0; i < threads_.size(); i++)
  //   threads_[i].join();
}

void l1_proxy::chain_req(const sequence_id& seq, const std::vector<std::string> & arguments) {
  chain_request(seq, arguments);
}