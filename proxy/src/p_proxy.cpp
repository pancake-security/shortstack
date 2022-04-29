// Shortstack L1 proxy implementation

#include <spdlog/spdlog.h>

#include "p_proxy.h"

void p_proxy::init_proxy(std::shared_ptr<host_info> hosts,
                          std::string instance_name,
                          std::shared_ptr<distribution_info> dist_info,
                          int local_idx, std::shared_ptr<thrift_response_client_map> client_map,
                          int storage_batch_size, std::shared_ptr<update_cache> update_cache) {
  instance_name_ = instance_name;
  host this_host;

  if (!hosts->get_host(instance_name, this_host)) {
    throw std::runtime_error("Unkown instance name: " + instance_name);
  }

  int base_idx;
  hosts->get_base_idx(instance_name_, base_idx);
  idx_ = base_idx + local_idx;

  num_keys_ = dist_info->num_keys_;
  dummy_key_ = dist_info->dummy_key_;
  delta_ = 0.5;
  key_to_number_of_replicas_ = dist_info->key_to_number_of_replicas_;
  fake_distribution_ = dist_info->fake_distribution_;
  real_distribution_ = dist_info->real_distribution_;
  replica_to_label_ = dist_info->replica_to_label_;

  update_cache_ = update_cache;

  id_to_client_ = client_map;


  storage_batch_size_ = storage_batch_size;

  crypto_queue_ = std::make_shared<moodycamel::BlockingReaderWriterQueue<crypto_op_batch>>();;

  respond_queue_ = std::make_shared<queue<client_response>>();

  cpp_redis::network::set_default_nb_workers(10);

  auto crypto_queue = crypto_queue_;
  bool stats_enbl = false;
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
  stats_enbl = false;

  redis_interface::put_callback put_cb = [fake_id, resp_queue, stats_enbl](const std::vector<l3_operation> &ops) {
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
      }
  };

  redis_interface::put_callback put_cb_dummy = [](const std::vector<l3_operation> &ops) {
    spdlog::error("Uninitialized put callback");
  };

  std::vector<host> kv_hosts;
  hosts->get_hosts_by_type(HOST_TYPE_KV, kv_hosts);
  if(kv_interaction_) {  
    
      storage_iface_ =
          std::make_shared<redis_interface>(kv_hosts[0].hostname, kv_hosts[0].port, storage_batch_size_, get_cb, put_cb_dummy);
      for (int j = 1; j < kv_hosts.size(); j++) {
        storage_iface_->add_server(kv_hosts[j].hostname, kv_hosts[j].port);
      }
  
      storage_iface2_ =
          std::make_shared<redis_interface>(kv_hosts[0].hostname, kv_hosts[0].port, 10000000, get_cb_dummy, put_cb);
      for (int j = 1; j < kv_hosts.size(); j++) {
        storage_iface2_->add_server(kv_hosts[j].hostname, kv_hosts[j].port);
      }

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

    threads_.push_back(std::thread(&p_proxy::crypto_thread, this,
                                   new encryption_engine(encryption_engine_)));

  threads_.push_back(std::thread(&p_proxy::responder_thread, this));
  


  spdlog::info("Initialized pancake proxy");
}

void p_proxy::init(const std::vector<std::string> &keys,
                    const std::vector<std::string> &values, void **args) {
  throw std::logic_error("Not implemented");
}

bool p_proxy::is_true_distribution() { return prob(delta_); };

void p_proxy::create_security_batch(std::queue<l1_operation> &q,
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

std::string p_proxy::get(const std::string &key) {
  throw std::logic_error("Not implemented");
};

void p_proxy::async_get(const sequence_id &seq_id, const std::string &key) {
  async_get(seq_id, rand_uint32(0, RAND_MAX), key);
};

void p_proxy::put(const std::string &key, const std::string &value) {
  throw std::logic_error("Not implemented");
};

void p_proxy::async_put(const sequence_id &seq_id, const std::string &key,
                         const std::string &value) {
  async_put(seq_id, rand_uint32(0, RAND_MAX), key, value);
};

std::vector<std::string>
p_proxy::get_batch(const std::vector<std::string> &keys) {
  throw std::logic_error("Not implemented");
};

void p_proxy::async_get_batch(const sequence_id &seq_id,
                               const std::vector<std::string> &keys) {
  async_get_batch(seq_id, rand_uint32(0, RAND_MAX), keys);
};

void p_proxy::put_batch(const std::vector<std::string> &keys,
                         const std::vector<std::string> &values) {
  throw std::logic_error("Not implemented");
};

void p_proxy::async_put_batch(const sequence_id &seq_id,
                               const std::vector<std::string> &keys,
                               const std::vector<std::string> &values) {
  async_put_batch(seq_id, rand_uint32(0, RAND_MAX), keys, values);
};

std::string p_proxy::get(int queue_id, const std::string &key) {
  throw std::logic_error("Not implemented");
};

void p_proxy::async_get(const sequence_id &seq_id, int queue_id,
                         const std::string &key) {
  l1_operation operat;
  operat.seq_id = seq_id;
  operat.key = key;
  operat.value = "";
  // operation_queues_[queue_id % operation_queues_.size()]->push(operat);
  process_op(operat);
};

void p_proxy::put(int queue_id, const std::string &key,
                   const std::string &value) {
  throw std::logic_error("Not implemented");
};

void p_proxy::async_put(const sequence_id &seq_id, int queue_id,
                         const std::string &key, const std::string &value) {
  l1_operation operat;
  operat.seq_id = seq_id;
  operat.key = key;
  operat.value = value;
  // operation_queues_[queue_id % operation_queues_.size()]->push(operat);
  process_op(operat);
};

std::vector<std::string>
p_proxy::get_batch(int queue_id, const std::vector<std::string> &keys) {
  throw std::logic_error("Not implemented");
};

void p_proxy::async_get_batch(const sequence_id &seq_id, int queue_id,
                               const std::vector<std::string> &keys) {
  for (auto k : keys) {
    async_get(seq_id, queue_id, k);
  }
};

void p_proxy::put_batch(int queue_id, const std::vector<std::string> &keys,
                         const std::vector<std::string> &values) {
  throw std::logic_error("Not implemented");
};

void p_proxy::async_put_batch(const sequence_id &seq_id, int queue_id,
                               const std::vector<std::string> &keys,
                               const std::vector<std::string> &values) {
  for (int i = 0; i < (int)keys.size(); i++) {
    async_put(seq_id, queue_id, keys[i], values[i]);
  }
};

// void p_proxy::consumer_thread(int id) {
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

void p_proxy::process_op(const l1_operation &op) {
  spdlog::debug("recvd op client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);
    // Generate batch
    internal_queue_.push(op);
    std::vector<l2_operation> batch;
    std::vector<bool> is_trues;
    create_security_batch(internal_queue_, batch, is_trues);

    // Forward requests in batch to L2
    for (auto &op : batch) {
      process_l2(op);
    }
}

void p_proxy::process_l2(const l2_operation &l2op) {
  l2_operation op = l2op;

  spdlog::debug("recvd op client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);

  if (key_to_number_of_replicas_.find(op.key) ==
      key_to_number_of_replicas_.end()) {
        spdlog::error("Key not found in key_to_number_of_replicas_: {}", op.key);
        throw std::runtime_error("Key not found");
        return;
  }

  if (update_cache_enabled_ && op.value != "") {
    update_cache_->populate_replica_updates(
        op.key, op.value, key_to_number_of_replicas_[op.key]);
  }
  // TODO: Currently check_for_update also clears the status bit. This can lead to inconsistency. The bit should be cleared only after PUT to KV is complete.
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

  // Send to L3
  l3_operation l3_op;
  l3_op.seq_id = op.seq_id;
  l3_op.label = label;
  l3_op.value = plaintext_update;
  l3_op.is_read = op.value == "";

  storage_iface_->async_get(l3_op.label, l3_op);
}

void p_proxy::flush() { throw std::logic_error("not implemented"); }

void p_proxy::close() {
  // finished_.store(true);
  // // TODO: push dummy ops into queues to unblock
  // for (int i = 0; i < threads_.size(); i++)
  //   threads_[i].join();
}

void p_proxy::crypto_thread(encryption_engine *enc_engine) {
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

        // Send PUT to KV
        l3_op.plaintext = plaintext;
        storage_iface->async_put(l3_op.label, writeback_val, l3_op);
    }

    storage_iface->flush();
  }
}

void p_proxy::responder_thread(){
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
        spdlog::debug("sending response to client");
        id_to_client_->async_respond_client(resp.seq_id, resp.op_code, results);
    }
    std::cout << "Quitting response thread" << std::endl;
}

void p_proxy::chain_req(const sequence_id& seq, const std::vector<std::string> & arguments) {
    throw std::logic_error("Not supported");
}

void p_proxy::setup_chain_stub(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t role, const std::string& next_block_id) {
  throw std::logic_error("Not supported");
}

void p_proxy::resend_pending_stub(const int32_t block_id) {
  throw std::logic_error("Not supported");
}
