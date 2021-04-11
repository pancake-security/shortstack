// Shortstack L2 proxy implementation

#include <spdlog/spdlog.h>
#include "l2_proxy.h"

void l2_proxy::init_proxy(std::shared_ptr<host_info> hosts,
                          std::string instance_name,
                          std::shared_ptr<distribution_info> dist_info,
                          int num_cores, bool uc_enabled) {
  instance_name_ = instance_name;
  update_cache_enabled_ = uc_enabled;

  if (!hosts->get_hostname(instance_name, server_host_name_)) {
    throw std::runtime_error("Unkown instance name: " + instance_name);
  }

  if (!hosts->get_port(instance_name, server_port_)) {
    throw std::runtime_error("Unkown instance name: " + instance_name);
  }

  replica_to_label_ = dist_info->replica_to_label_;
  key_to_number_of_replicas_ = dist_info->key_to_number_of_replicas_;
  dummy_key_ = dist_info->dummy_key_;

  // initialize empty update cache
  update_cache_ = update_cache();

  std::vector<host> l3_hosts;
  hosts->get_hosts_by_type(HOST_TYPE_L3, l3_hosts);

  std::vector<std::string> l3_hostnames;
  std::vector<int> l3_ports;
  for (auto h : l3_hosts) {
    l3_hostnames.push_back(h.hostname);
    l3_ports.push_back(h.port);
  }

  // int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
  for (int i = 0; i < num_cores; i++) {
    auto q = std::make_shared<queue<l2_operation>>();
    operation_queues_.push_back(q);
    l3_ifaces_.push_back(
        std::make_shared<l3proxy_interface>(l3_hostnames, l3_ports));
  }

  finished_.store(false);
  for (int i = 0; i < num_cores; i++) {
    threads_.push_back(std::thread(&l2_proxy::consumer_thread, this, i));
  }
}

void l2_proxy::async_operation(const sequence_id &seq_id,
                               const std::string &key, int replica,
                               const std::string &value) {
  l2_operation operat;
  operat.seq_id = seq_id;
  operat.key = key;
  operat.replica = replica;
  operat.value = value;

  auto it = replica_to_label_.find(key + std::to_string(replica));
  if (it == replica_to_label_.end()) {
    throw std::runtime_error("key not found: " + key);
  }

  operation_queues_[it->second % operation_queues_.size()]->push(operat);
}

void l2_proxy::consumer_thread(int id) {
  // TODO: Handle exceptions
  std::shared_ptr<l3proxy_interface> l3_interface = l3_ifaces_[id];

  // Connect to L2 servers
  l3_interface->connect();

  std::cerr << "Consumer " << id << ": "
            << "L3 interface connected" << std::endl;

  while (true) {
    auto op = operation_queues_[id]->pop(); // Blocking call
    spdlog::debug("recvd op client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);
    if (finished_.load()) {
      break;
    }

    if (key_to_number_of_replicas_.find(op.key) ==
        key_to_number_of_replicas_.end()) {
      std::cerr << "Key not found in key_to_number_of_replicas_: " << op.key
                << std::endl;
      continue;
    }

    if (update_cache_enabled_ && op.value != "") {
      update_cache_.populate_replica_updates(
          op.key, op.value, key_to_number_of_replicas_[op.key]);
    }
    // TODO: Currently check_for_update also clears the status bit. This can lead to inconsistency. The bit should be cleared only after PUT to KV is complete.
    std::string plaintext_update;
    if(update_cache_enabled_) {
      plaintext_update = update_cache_.check_for_update(op.key, op.replica);
    } else {
      plaintext_update = op.value;
    }
    
    spdlog::debug("plaintext_update={}..., client_id:{}, seq_no:{}", plaintext_update.substr(0,5), op.seq_id.client_id, op.seq_id.client_seq_no);

    auto it = replica_to_label_.find(op.key + std::to_string(op.replica));
    if (it == replica_to_label_.end()) {
      std::cerr << "Replica not found in label map: " << op.key << ","
                << op.replica << std::endl;
      continue;
    }
    auto label = std::to_string(it->second);

    // Send to L3
    l3_operation l3_op;
    l3_op.seq_id = op.seq_id;
    l3_op.label = label;
    l3_op.value = plaintext_update;
    l3_op.is_read = op.value == "";
    l3_ifaces_[id]->send_op(l3_op);
  }
}

void l2_proxy::close() {
  finished_.store(true);
  // push dummy ops into queues to unblock
  l2_operation dummy;
  dummy.seq_id.client_id = -1;
  dummy.key = "$end$";
  dummy.replica = -1;
  dummy.value = "";
  for (int i = 0; i < operation_queues_.size(); i++) {
    operation_queues_[i]->push(dummy);
  }
  for (int i = 0; i < threads_.size(); i++) {
    threads_[i].join();
  }
}

void l2_proxy::run_command(const sequence_id &seq, const arg_list &args) {
  // TODO: Implement
}

void l2_proxy::replication_complete(const sequence_id &seq, const arg_list &args) {
  // TODO: Implement
}