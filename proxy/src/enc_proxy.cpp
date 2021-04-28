// Shortstack L1 proxy implementation

#include <spdlog/spdlog.h>

#include "enc_proxy.h"

void enc_proxy::init_proxy(std::shared_ptr<host_info> hosts,
                          std::string instance_name,
                          int local_idx, std::shared_ptr<thrift_response_client_map> client_map,
                          int storage_batch_size) {
  instance_name_ = instance_name;
  host this_host;

  if (!hosts->get_host(instance_name, this_host)) {
    throw std::runtime_error("Unkown instance name: " + instance_name);
  }

  int base_idx;
  hosts->get_base_idx(instance_name_, base_idx);
  idx_ = base_idx + local_idx;

  id_to_client_ = client_map;


  storage_batch_size_ = storage_batch_size;

  encrypt_queue_ = std::make_shared<moodycamel::BlockingReaderWriterQueue<crypto_op_batch>>();

  decrypt_queue_ = std::make_shared<moodycamel::BlockingReaderWriterQueue<crypto_op_batch>>();

  respond_queue_ = std::make_shared<queue<client_response>>();

  auto resp_queue = respond_queue_;
  auto crypto_queue = decrypt_queue_;
  auto enc_engine = std::make_shared<encryption_engine>();
  redis_interface::get_callback get_cb = [resp_queue, crypto_queue, enc_engine](const std::vector<l3_operation> &ops, const std::vector<std::string> & vals) {
      crypto_op_batch batch;
      for(int i = 0; i < ops.size(); i++) 
      {
        const l3_operation &op = ops[i];
        spdlog::debug("recvd KV GET response client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);

        // Enqueue task for crypto thread
        crypto_operation crypto_op;
        crypto_op.l3_op = op;
        crypto_op.kv_response = vals[i];
        batch.push_back(crypto_op);
      }

      crypto_queue->enqueue(batch);
  };


  redis_interface::put_callback put_cb = [resp_queue](const std::vector<l3_operation> &ops) {
      for(int i = 0; i < ops.size(); i++) 
      {
        const l3_operation &op = ops[i];
        spdlog::debug("recvd KV PUT response client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);

          client_response resp;
          resp.seq_id = op.seq_id;
          resp.result = "";
          resp.op_code = OP_PUT;


          // TODO: Even this can be batched
          resp_queue->push(resp);
      }
  };

  std::vector<host> kv_hosts;
  hosts->get_hosts_by_type(HOST_TYPE_KV, kv_hosts);
  if(kv_interaction_) {

    int redis_idx = hosts->get_host_idx(HOST_TYPE_L1, this_host.hostname);  
    
      storage_iface_ =
          std::make_shared<redis_interface>(kv_hosts[redis_idx].hostname, kv_hosts[redis_idx].port, storage_batch_size_, get_cb, put_cb);
      // for (int j = 1; j < kv_hosts.size(); j++) {
      //   storage_iface_->add_server(kv_hosts[j].hostname, kv_hosts[j].port);
      // }

      storage_iface2_ =
          std::make_shared<redis_interface>(kv_hosts[redis_idx].hostname, kv_hosts[redis_idx].port, storage_batch_size_, get_cb, put_cb);
      // for (int j = 1; j < kv_hosts.size(); j++) {
      //   storage_iface2_->add_server(kv_hosts[j].hostname, kv_hosts[j].port);
      // }


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

    threads_.push_back(std::thread(&enc_proxy::encrypt_thread, this,
                                   new encryption_engine(encryption_engine_)));

    threads_.push_back(std::thread(&enc_proxy::decrypt_thread, this,
                                   new encryption_engine(encryption_engine_)));

  threads_.push_back(std::thread(&enc_proxy::responder_thread, this));
  


  spdlog::info("Initialized pancake proxy");
}

void enc_proxy::init(const std::vector<std::string> &keys,
                    const std::vector<std::string> &values, void **args) {
  throw std::logic_error("Not implemented");
}


std::string enc_proxy::get(const std::string &key) {
  throw std::logic_error("Not implemented");
};

void enc_proxy::async_get(const sequence_id &seq_id, const std::string &key) {
  async_get(seq_id, rand_uint32(0, RAND_MAX), key);
};

void enc_proxy::put(const std::string &key, const std::string &value) {
  throw std::logic_error("Not implemented");
};

void enc_proxy::async_put(const sequence_id &seq_id, const std::string &key,
                         const std::string &value) {
  async_put(seq_id, rand_uint32(0, RAND_MAX), key, value);
};

std::vector<std::string>
enc_proxy::get_batch(const std::vector<std::string> &keys) {
  throw std::logic_error("Not implemented");
};

void enc_proxy::async_get_batch(const sequence_id &seq_id,
                               const std::vector<std::string> &keys) {
  async_get_batch(seq_id, rand_uint32(0, RAND_MAX), keys);
};

void enc_proxy::put_batch(const std::vector<std::string> &keys,
                         const std::vector<std::string> &values) {
  throw std::logic_error("Not implemented");
};

void enc_proxy::async_put_batch(const sequence_id &seq_id,
                               const std::vector<std::string> &keys,
                               const std::vector<std::string> &values) {
  async_put_batch(seq_id, rand_uint32(0, RAND_MAX), keys, values);
};

std::string enc_proxy::get(int queue_id, const std::string &key) {
  throw std::logic_error("Not implemented");
};

void enc_proxy::async_get(const sequence_id &seq_id, int queue_id,
                         const std::string &key) {
  l1_operation operat;
  operat.seq_id = seq_id;
  operat.key = key;
  operat.value = "";
  // operation_queues_[queue_id % operation_queues_.size()]->push(operat);
  process_op(operat);
};

void enc_proxy::put(int queue_id, const std::string &key,
                   const std::string &value) {
  throw std::logic_error("Not implemented");
};

void enc_proxy::async_put(const sequence_id &seq_id, int queue_id,
                         const std::string &key, const std::string &value) {
  l1_operation operat;
  operat.seq_id = seq_id;
  operat.key = key;
  operat.value = value;
  // operation_queues_[queue_id % operation_queues_.size()]->push(operat);
  process_op(operat);
};

std::vector<std::string>
enc_proxy::get_batch(int queue_id, const std::vector<std::string> &keys) {
  throw std::logic_error("Not implemented");
};

void enc_proxy::async_get_batch(const sequence_id &seq_id, int queue_id,
                               const std::vector<std::string> &keys) {
  for (auto k : keys) {
    async_get(seq_id, queue_id, k);
  }
};

void enc_proxy::put_batch(int queue_id, const std::vector<std::string> &keys,
                         const std::vector<std::string> &values) {
  throw std::logic_error("Not implemented");
};

void enc_proxy::async_put_batch(const sequence_id &seq_id, int queue_id,
                               const std::vector<std::string> &keys,
                               const std::vector<std::string> &values) {
  for (int i = 0; i < (int)keys.size(); i++) {
    async_put(seq_id, queue_id, keys[i], values[i]);
  }
};

// void enc_proxy::consumer_thread(int id) {
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

void enc_proxy::process_op(const l1_operation &op) {
  spdlog::debug("recvd op client_id:{}, seq_no:{}", op.seq_id.client_id, op.seq_id.client_seq_no);
  // Encrypt and send to KV
  if(op.value != "") {
    l3_operation l3_op;
    l3_op.seq_id = op.seq_id;
    l3_op.label = op.key;
    l3_op.value = op.value;
    l3_op.is_read = false;
    internal_queue_.push(l3_op);
    
    if(internal_queue_.size() >= storage_batch_size_) 
    {
      crypto_op_batch batch;
      while(!internal_queue_.empty()) {
        crypto_operation crypto_op;
        crypto_op.l3_op = internal_queue_.front();
        batch.push_back(crypto_op);
        internal_queue_.pop();
      }
      encrypt_queue_->enqueue(batch);
    }
  } else {
    l3_operation l3_op;
    l3_op.seq_id = op.seq_id;
    l3_op.label = op.key;
    l3_op.value = "";
    l3_op.is_read = true;
    storage_iface_->async_get(op.key, l3_op);
  }

}


void enc_proxy::flush() { 

    if(storage_iface_ != nullptr) {
      storage_iface_->flush();
    }

 }

void enc_proxy::close() {
  // finished_.store(true);
  // // TODO: push dummy ops into queues to unblock
  // for (int i = 0; i < threads_.size(); i++)
  //   threads_[i].join();
}

void enc_proxy::encrypt_thread(encryption_engine *enc_engine) {
  spdlog::info("Worker {}: Encryption thread started", idx_);

  auto storage_iface = storage_iface2_;

  while(true) {
    crypto_op_batch batch;
    encrypt_queue_->wait_dequeue(batch); // Blocking call

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
        auto plaintext = l3_op.value;

        spdlog::debug("encrypting value. len={}", plaintext.size());
        auto writeback_val = (encryption_enabled_)?(enc_engine->encrypt(plaintext)):(plaintext);

        // Send PUT to KV
        // l3_op.plaintext = plaintext;
        storage_iface->async_put(l3_op.label, writeback_val, l3_op);
    }

    storage_iface->flush();
  }
}

void enc_proxy::decrypt_thread(encryption_engine *enc_engine) {
  spdlog::info("Worker {}: Decryption thread started", idx_);

  while(true) {
    crypto_op_batch batch;
    decrypt_queue_->wait_dequeue(batch); // Blocking call

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

        spdlog::debug("decrypting value. len={}", cipher.size());
        auto plaintext = (encryption_enabled_)?(enc_engine->decrypt(cipher)):(cipher);

        // TODO: Even this can be batched
        client_response resp;
        resp.seq_id = l3_op.seq_id;
        resp.result = plaintext;
        resp.op_code = OP_GET;
        respond_queue_->push(resp);
    }
  }
}

void enc_proxy::responder_thread(){
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

void enc_proxy::chain_req(const sequence_id& seq, const std::vector<std::string> & arguments) {
    throw std::logic_error("Not supported");
}

void enc_proxy::setup_chain_stub(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t role, const std::string& next_block_id) {
  throw std::logic_error("Not supported");
}

void enc_proxy::resend_pending_stub(const int32_t block_id) {
  throw std::logic_error("Not supported");
}
