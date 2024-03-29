// Shortstack L3 proxy implementation

#include <spdlog/spdlog.h>
#include <chrono>
#include "l3_proxy.h"

void l3_proxy::init_proxy(
    std::shared_ptr<host_info> hosts, std::string instance_name,
    int kvclient_threads, int storage_batch_size,
    std::shared_ptr<thrift_response_client_map> client_map,
    bool encryption_enabled, bool resp_delivery,
    bool kv_interaction, int local_idx, int64_t timeout_us, bool ack_delivery,
    bool stats, int ack_batch_size) {

  hosts_ = hosts;
  instance_name_ = instance_name;
  storage_batch_size_ = storage_batch_size;
  encryption_enabled_ = encryption_enabled;
  resp_delivery_ = resp_delivery;
  kv_interaction_ = kv_interaction;
  timeout_us_ = timeout_us;
  ack_delivery_ = ack_delivery;
  stats_ = stats;
  ack_batch_size_ = ack_batch_size;

  req_cnt_ = 0;

  id_to_client_ = client_map;

  host this_host;
  if (!hosts->get_host(instance_name, this_host)) {
    throw std::runtime_error("Unkown instance name: " + instance_name);
  }

  int base_idx;
  hosts->get_base_idx(instance_name_, base_idx);
  idx_ = base_idx + local_idx;

  int num_l2_cols = hosts->get_num_columns(HOST_TYPE_L2, true);
  for(int i = 0; i < num_l2_cols; i++) {
    last_seen_seq_.push_back(-1);
  }

  // int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

  // operation_queue_ = std::make_shared<moodycamel::BlockingReaderWriterQueue<l3_operation>>();

  crypto_queue_ = std::make_shared<moodycamel::BlockingReaderWriterQueue<crypto_op_batch>>();;

  respond_queue_ = std::make_shared<queue<client_response>>();

  ack_iface_ = std::make_shared<l2ack_interface>(hosts_, ack_batch_size_);

  spdlog::info("Worker {}: Ack interface initialized", idx_);

  cpp_redis::network::set_default_nb_workers(kvclient_threads);

  auto crypto_queue = crypto_queue_;
  auto stats_enbl = stats_;
  redis_interface::get_callback get_cb = [crypto_queue, stats_enbl](const std::vector<l3_operation> &ops, const std::vector<std::string> & vals) {
      crypto_op_batch batch;
      for(int i = 0; i < ops.size(); i++) 
      {
        const l3_operation &op = ops[i];
        spdlog::debug("recvd KV GET response client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);

        // Enqueue task for crypto thread
        crypto_operation crypto_op;
        crypto_op.l3_op = op;
        crypto_op.kv_response = vals[i];
        if(stats_enbl) {
          int64_t us_from_epoch = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
          auto elapsed = us_from_epoch - crypto_op.l3_op.seq_id.ts;
          crypto_op.l3_op.seq_id.__set_diag(crypto_op.l3_op.seq_id.diag + std::to_string(elapsed) + ",");
          crypto_op.l3_op.seq_id.ts = us_from_epoch;
        }
        batch.push_back(crypto_op);
      }

      crypto_queue->enqueue(batch);
  };

  redis_interface::get_callback get_cb_dummy = [](const std::vector<l3_operation> &ops, const std::vector<std::string> & vals) {
    spdlog::error("Uninitialized get callback");
  };

  auto fake_id = fake_client_id_;
  auto resp_queue = respond_queue_;
  auto ack_iface = ack_iface_;
  auto ack_del = ack_delivery_;

  redis_interface::put_callback put_cb = [fake_id, resp_queue, ack_iface, ack_del, stats_enbl](const std::vector<l3_operation> &ops) {
      for(int i = 0; i < ops.size(); i++) {
        const l3_operation &l3_op = ops[i];
        spdlog::debug("recvd KV PUT response client_id:{}, seq_no:{}", l3_op.seq_id.client_id, l3_op.seq_id.client_seq_no);
        // Enqueue responses for real queries
        if (l3_op.seq_id.client_id != fake_id) {
          client_response resp;
          resp.seq_id = l3_op.seq_id;
          resp.result = (l3_op.is_read) ? l3_op.plaintext : "";
          resp.op_code = (l3_op.is_read) ? OP_GET : OP_PUT;

          if(stats_enbl) {
            int64_t us_from_epoch = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            auto elapsed = us_from_epoch - resp.seq_id.ts;
            resp.seq_id.__set_diag(resp.seq_id.diag + std::to_string(elapsed) + ",");
            resp.seq_id.ts = us_from_epoch;
          }

          // TODO: Even this can be batched
          resp_queue->push(resp);
        }

        if(ack_del) {
          ack_iface->send_ack(l3_op.seq_id);
        }
      }
  };

  redis_interface::put_callback put_cb_dummy = [](const std::vector<l3_operation> &ops) {
    spdlog::error("Uninitialized put callback");
  };

  std::vector<host> kv_hosts;
  hosts->get_hosts_by_type(HOST_TYPE_KV, kv_hosts);
  if(kv_interaction_) {  
    
      // storage_iface_ =
      //     std::make_shared<redis_interface>(kv_hosts[0].hostname, kv_hosts[0].port, storage_batch_size_, get_cb, put_cb_dummy);
      // for (int j = 1; j < kv_hosts.size(); j++) {
      //   storage_iface_->add_server(kv_hosts[j].hostname, kv_hosts[j].port);
      // }
  
      // storage_iface2_ =
      //     std::make_shared<redis_interface>(kv_hosts[0].hostname, kv_hosts[0].port, 10000000, get_cb_dummy, put_cb);
      // for (int j = 1; j < kv_hosts.size(); j++) {
      //   storage_iface2_->add_server(kv_hosts[j].hostname, kv_hosts[j].port);
      // }

      int redis_idx = hosts->get_host_idx(HOST_TYPE_L3, this_host.hostname);  
    
      storage_iface_ =
          std::make_shared<redis_interface>(kv_hosts[redis_idx].hostname, kv_hosts[redis_idx].port, storage_batch_size_, get_cb, put_cb);
      // for (int j = 1; j < kv_hosts.size(); j++) {
      //   storage_iface_->add_server(kv_hosts[j].hostname, kv_hosts[j].port);
      // }

      storage_iface2_ =
          std::make_shared<redis_interface>(kv_hosts[redis_idx].hostname, kv_hosts[redis_idx].port, storage_batch_size_, get_cb, put_cb);

      spdlog::info("Worker {}: Storage interfaces connected", idx_);
   
  } else {
      // storage_iface_ =
      //     std::make_shared<dummy_kv>(1000); // TODO: val size is hardcoded
    
      // storage_iface2_ =
      //     std::make_shared<dummy_kv>(1000); // TODO: val size is hardcoded
      throw std::logic_error("dummy KV Not implemented");
  }

  finished_.store(false);

  // threads_.push_back(std::thread(&l3_proxy::consumer_thread, this));

    threads_.push_back(std::thread(&l3_proxy::crypto_thread, this,
                                   new encryption_engine(encryption_engine_)));

  threads_.push_back(std::thread(&l3_proxy::responder_thread, this));
}

void l3_proxy::async_operation(const sequence_id &seq_id,
                               const std::string &label,
                               const std::string &value,
                               bool is_read,
                               bool dedup) {
  l3_operation op;
  op.seq_id = seq_id;
  op.label = label;
  op.value = value;
  op.is_read = is_read;
  op.dedup = dedup;

  if(op.dedup && op.seq_id.l2_seq_no <= last_seen_seq_[op.seq_id.l2_idx]) {
    spdlog::info("Received duplicate L2 request, l2_idx: {}, l2_seq_no: {}", op.seq_id.l2_idx, op.seq_id.l2_seq_no);
    return;
  }

  last_seen_seq_[op.seq_id.l2_idx] = std::max(last_seen_seq_[op.seq_id.l2_idx], op.seq_id.l2_seq_no);

  spdlog::debug("recvd op client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);

  req_cnt_ += 1;

  if(stats_) {
    int64_t us_from_epoch = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    op.seq_id.__set_ts(us_from_epoch);
  }

    auto storage_iface = storage_iface_;
    // auto crypto_queue = crypto_queue_;
    

    // Send GET request to KV
    // Execution will continue in crypto_thread
    storage_iface->async_get(op.label, op);
  
}

// void l3_proxy::consumer_thread() {
//   while(true) {
//     bool success;
//     l3_operation op;
//     success = operation_queue_->wait_dequeue_timed(op, timeout_us_);
//     if(!success) {
//       // Timeout
//       spdlog::info("Batch timeout");
//       storage_iface_->flush();
//       continue;
//     }

//     spdlog::debug("recvd op client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);

//     if (finished_.load()) {
//         break;
//       }

//     auto storage_iface = storage_iface_;
//     auto crypto_queue = crypto_queue_;
    

//     // Send GET request to KV
//     // Execution will continue in crypto_thread
//     storage_iface->async_get(op.label, [op, crypto_queue](const std::string &resp_val) {
//         spdlog::debug("recvd KV GET response client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);
//         // Enqueue task for crypto thread
//         crypto_operation crypto_op;
//         crypto_op.l3_op = op;
//         crypto_op.kv_response = resp_val;
//         crypto_queue->enqueue(crypto_op);
//     });
//   }
  
// }



void l3_proxy::crypto_thread(encryption_engine *enc_engine) {
  spdlog::info("Worker {}: Crypto thread started", idx_);

  auto storage_iface = storage_iface2_;
  auto fake_id = fake_client_id_;
  auto resp_queue = respond_queue_;

  while(true) {
    crypto_op_batch batch;
    crypto_queue_->wait_dequeue(batch); // Blocking call

    // if(!success) {
    //   // Timeout
    //   spdlog::info("Batch timeout");
    //   storage_iface->flush();
    //   continue;
    // }

     if (finished_.load()) {
      break;
    }

    for(auto &crypto_op : batch) {
        spdlog::debug("recvd crypto op client_id:{}, seq_no:{}", crypto_op.l3_op.seq_id.client_id, crypto_op.l3_op.seq_id.client_seq_no);

       

        auto l3_op = crypto_op.l3_op;
        auto cipher = crypto_op.kv_response;

        if(stats_) {
          int64_t us_from_epoch = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
          auto elapsed = us_from_epoch - l3_op.seq_id.ts;
          l3_op.seq_id.__set_diag(l3_op.seq_id.diag + std::to_string(elapsed) + ",");
          l3_op.seq_id.ts = us_from_epoch;
        }  

        spdlog::debug("decrypting value. len={}", cipher.size());
        auto plaintext = (encryption_enabled_)?(enc_engine->decrypt(cipher)):(cipher);

        if (l3_op.value != "") {
          plaintext = l3_op.value;
        }

        spdlog::debug("encrypting value. len={}", plaintext.size());
        auto writeback_val = (encryption_enabled_)?(enc_engine->encrypt(plaintext)):(plaintext);

        if(stats_) {
          int64_t us_from_epoch = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
          auto elapsed = us_from_epoch - l3_op.seq_id.ts;
          l3_op.seq_id.__set_diag(l3_op.seq_id.diag + std::to_string(elapsed) + ",");
          l3_op.seq_id.ts = us_from_epoch;
        }

        auto ack_iface = ack_iface_;
        auto ack_delivery = ack_delivery_;

        // Send PUT to KV
        l3_op.plaintext = plaintext;
        storage_iface->async_put(l3_op.label, writeback_val, l3_op);
    }

    storage_iface->flush();
  }
}

void l3_proxy::responder_thread(){
    while (true){
        auto resp = respond_queue_->pop();
    
        std::vector<std::string>results;
        results.push_back((resp_delivery_)?(resp.result):(""));
        // // TODO: Disabling response delivery for perf debugging
        // results.push_back("");
        if(stats_) {
          int64_t us_from_epoch = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
          auto elapsed = us_from_epoch - resp.seq_id.ts;
          resp.seq_id.__set_diag(resp.seq_id.diag + std::to_string(elapsed) + ",");
          resp.seq_id.ts - us_from_epoch;
        }
        id_to_client_->async_respond_client(resp.seq_id, resp.op_code, results);
    }
    std::cout << "Quitting response thread" << std::endl;
}

void l3_proxy::close() {
  finished_.store(true);
  // push dummy ops into queues to unblock

  for (int i = 0; i < threads_.size(); i++) {
    threads_[i].join();
  }

  // TODO: Join responder thread
}

l2ack_interface::l2ack_interface(std::shared_ptr<host_info> hosts, int batch_size)
: reverse_connector(hosts, HOST_TYPE_L2, batch_size) {

}

int l2ack_interface::route(const sequence_id &seq) {
  return seq.l2_idx;
}

void l3_proxy::update_connections(int type, int column, std::string hostname, int port, int num_workers) {
  if(type != HOST_TYPE_L2) {
    spdlog::error("Invalid update_connections call");
    throw std::runtime_error("Invalid update_connections call");
    return;
  }

  ack_iface_->update_connections(column, hostname, port, num_workers);

  spdlog::info("Updated L2 ack connection for column: {}", column);
}

void l3_proxy::log_stats() {
  std::cout << "Req cnt: " << req_cnt_ << std::endl;
}