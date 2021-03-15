// Shortstack L3 proxy implementation

#include <spdlog/spdlog.h>
#include "l3_proxy.h"

void l3_proxy::init_proxy(
    std::shared_ptr<host_info> hosts, std::string instance_name,
    int kvclient_threads, int storage_batch_size,
    std::shared_ptr<thrift_response_client_map> client_map,
    int num_cores, bool encryption_enabled) {

  instance_name_ = instance_name;
  storage_batch_size_ = storage_batch_size;
  encryption_enabled_ = encryption_enabled;

  id_to_client_ = client_map;

  if (!hosts->get_hostname(instance_name, server_host_name_)) {
    throw std::runtime_error("Unkown instance name: " + instance_name);
  }

  if (!hosts->get_port(instance_name, server_port_)) {
    throw std::runtime_error("Unkown instance name: " + instance_name);
  }

  // int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
  for (int i = 0; i < num_cores; i++) {
    auto q = std::make_shared<queue<l3_operation>>();
    operation_queues_.push_back(q);
  }
  for (int i = 0; i < num_cores; i++) {
    auto q = std::make_shared<queue<crypto_operation>>();
    crypto_queues_.push_back(q);
  }

  respond_queue_ = std::make_shared<queue<client_response>>();

  cpp_redis::network::set_default_nb_workers(kvclient_threads);

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

  for (int i = 0; i < num_cores; i++) {
    auto storage_iface =
        std::make_shared<redis>(kv_hosts[0].hostname, kv_hosts[0].port);
    for (int j = 1; j < kv_hosts.size(); j++) {
      storage_iface->add_server(kv_hosts[j].hostname, kv_hosts[j].port);
    }
    storage_ifaces2_.push_back(storage_iface);
  }

  finished_.store(false);
  for (int i = 0; i < num_cores; i++) {
    threads_.push_back(std::thread(&l3_proxy::consumer_thread, this, i));
  }
  for (int i = 0; i < num_cores; i++) {
    threads_.push_back(std::thread(&l3_proxy::crypto_thread, this, i,
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

void l3_proxy::consumer_thread(int id) {
  // TODO: Handle exceptions

  std::cerr << "Consumer " << id << " running" << std::endl;

  auto storage_iface = storage_ifaces_[id];
  auto crypto_queue = crypto_queues_[id];

  while (true) {
    auto op = operation_queues_[id]->pop(); // Blocking call
    spdlog::debug("recvd op client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);

    if (finished_.load()) {
      break;
    }

    // Send GET request to KV
    // Execution will continue in crypto_thread
    storage_iface->async_get(op.label, [op, crypto_queue](const std::string &resp_val) {
        spdlog::debug("recvd KV GET response client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);
        // Enqueue task for crypto thread
        crypto_operation crypto_op;
        crypto_op.l3_op = op;
        crypto_op.kv_response = resp_val;
        crypto_queue->push(crypto_op);
    });
  }
}


void l3_proxy::crypto_thread(int id, encryption_engine *enc_engine) {
  spdlog::info("Crypto worker {} started", id);

  auto storage_iface = storage_ifaces2_[id];
  auto fake_id = fake_client_id_;
  auto resp_queue = respond_queue_;

  while(true) {
    auto crypto_op = crypto_queues_[id]->pop(); // Blocking call
    spdlog::debug("recvd crypto op client_id:{}, seq_no:{}", crypto_op.l3_op.seq_id.client_id, crypto_op.l3_op.seq_id.client_seq_no);

    if (finished_.load()) {
      break;
    }

    auto l3_op = crypto_op.l3_op;
    auto cipher = crypto_op.kv_response;
    spdlog::debug("decrypting value. len={}", cipher.size());
    auto plaintext = (encryption_enabled_)?(enc_engine->decrypt(cipher)):(cipher);

    if (l3_op.value != "") {
      plaintext = l3_op.value;
    }

    spdlog::debug("encrypting value. len={}", plaintext.size());
    auto writeback_val = (encryption_enabled_)?(enc_engine->encrypt(plaintext)):(plaintext);

    // Send PUT to KV
    storage_iface->async_put(l3_op.label, writeback_val, [l3_op, fake_id, resp_queue, plaintext]() {
      spdlog::debug("recvd KV PUT response client_id:{}, seq_no:{}", l3_op.seq_id.client_id, l3_op.seq_id.client_seq_no);
      // Enqueue responses for real queries
      if (l3_op.seq_id.client_id != fake_id) {
        client_response resp;
        resp.seq_id = l3_op.seq_id;
        resp.result = (l3_op.is_read) ? plaintext : "";
        resp.op_code = (l3_op.is_read) ? OP_GET : OP_PUT;

        resp_queue->push(resp);
      }
    });
  }
}

void l3_proxy::responder_thread(){
    while (true){
        auto resp = respond_queue_->pop();
    
        std::vector<std::string>results;
        results.push_back(resp.result);
        // // TODO: Disabling response delivery for perf debugging
        // results.push_back("");
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