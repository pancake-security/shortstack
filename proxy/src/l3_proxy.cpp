// Shortstack L3 proxy implementation

#include <spdlog/spdlog.h>
#include "l3_proxy.h"

void l3_proxy::init_proxy(
    std::shared_ptr<host_info> hosts, std::string instance_name,
    int kvclient_threads, int storage_batch_size,
    std::shared_ptr<thrift_response_client_map> client_map) {

  instance_name_ = instance_name;
  storage_batch_size_ = storage_batch_size;

  id_to_client_ = client_map;

  if (!hosts->get_hostname(instance_name, server_host_name_)) {
    throw std::runtime_error("Unkown instance name: " + instance_name);
  }

  if (!hosts->get_port(instance_name, server_port_)) {
    throw std::runtime_error("Unkown instance name: " + instance_name);
  }

  int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
  for (int i = 0; i < num_cores; i++) {
    auto q = std::make_shared<queue<l3_operation>>();
    operation_queues_.push_back(q);
  }

  cpp_redis::network::set_default_nb_workers(std::min(10, kvclient_threads));

  std::vector<host> kv_hosts;
  hosts->get_hosts_by_type(HOST_TYPE_KV, kv_hosts);
  for (int i = 0; i < num_cores; i++) {
    auto storage_iface =
        std::make_shared<redis>(kv_hosts[0].hostname, kv_hosts[0].port);
    for (int j = 1; j < kv_hosts.size(); j++) {
      storage_iface->add_server(kv_hosts[j].hostname, kv_hosts[j].port);
    }
    storage_ifaces_.push_back(storage_iface);
  }

  finished_.store(false);
  for (int i = 0; i < num_cores; i++) {
    threads_.push_back(std::thread(&l3_proxy::consumer_thread, this, i,
                                   new encryption_engine(encryption_engine_)));
  }
  threads_.push_back(std::thread(&l3_proxy::responder_thread, this));
}

void l3_proxy::async_operation(const sequence_id &seq_id,
                               const std::string &label,
                               const std::string &value,
                               bool is_read) {
  l3_operation operat;
  operat.seq_id = seq_id;
  operat.label = label;
  operat.value = value;
  operat.is_read = is_read;

  auto id = (std::hash<std::string>{}(std::string(operat.label)) %
             operation_queues_.size());
  operation_queues_[id]->push(operat);
}

void l3_proxy::consumer_thread(int id, encryption_engine *enc_engine) {
  // TODO: Handle exceptions

  std::cerr << "Consumer " << id << " running" << std::endl;

  auto storage_iface = storage_ifaces_[id];

  while (true) {
    std::vector<l3_operation> storage_batch;
    while (storage_batch.size() < storage_batch_size_ && !finished_.load()) {
      auto op = operation_queues_[id]->pop(); // Blocking call
      spdlog::debug("recvd op client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);
      storage_batch.push_back(op);
    }

    if (finished_.load()) {
      break;
    }

    execute_batch(storage_batch, storage_iface, enc_engine);
  }
}

void l3_proxy::execute_batch(
    const std::vector<l3_operation> &operations,
    std::shared_ptr<storage_interface> storage_interface,
    encryption_engine *enc_engine) {

  std::vector<std::string> storage_keys;
  for (int i = 0; i < operations.size(); i++) {
    storage_keys.push_back(operations[i].label);
  }
  // std::cout << "get_batch start\n";
  std::vector<std::string> responses;
  try{
    responses = storage_interface->get_batch(storage_keys);
  } catch(std::exception& ex) {
    std::cout << ex.what();
  }
  
  // std::cout << "get_batch end\n";
  std::vector<std::string> storage_values;
  for (int i = 0; i < operations.size(); i++) {
    auto cipher = responses[i];
    auto plaintext = enc_engine->decrypt(cipher);

    if (operations[i].value != "") {
      plaintext = operations[i].value;
    }
    
    // Enqueue responses for real queries
    if (operations[i].seq_id.client_id != fake_client_id_) {
      client_response resp;
      resp.seq_id = operations[i].seq_id;
      resp.result = (operations[i].is_read) ? plaintext : "";
      resp.op_code = (operations[i].is_read) ? GET : PUT;

      respond_queue_.push(resp);
    }

    storage_values.push_back(enc_engine->encrypt(plaintext));
  }
  // std::cout << "put_batch start\n";
  storage_interface->put_batch(storage_keys, storage_values);
  // std::cout << "put_batch end\n";
}

void l3_proxy::responder_thread(){
    while (true){
        auto resp = respond_queue_.pop();
    
        std::vector<std::string>results;
        results.push_back(resp.result);
        id_to_client_->async_respond_client(resp.seq_id, resp.op_code, results);
    }
    std::cout << "Quitting response thread" << std::endl;
}

void l3_proxy::close() {
  finished_.store(true);
  // push dummy ops into queues to unblock
  l3_operation dummy;
  dummy.seq_id.client_id = -1;
  dummy.label = "$end$";
  dummy.value = "";
  dummy.is_read = true;
  for(int i = 0; i < operation_queues_.size(); i++) {
    operation_queues_[i]->push(dummy);
  }
  for (int i = 0; i < threads_.size(); i++) {
    threads_[i].join();
  }

  // TODO: Join responder thread
}