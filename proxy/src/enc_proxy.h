//
// Encryption proxy
//

#ifndef ENC_PROXY_H
#define ENC_PROXY_H

#include <algorithm>
#include <atomic>
#include <fstream>
#include <future>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "distribution.h"
#include "distribution_info.h"
#include "host_info.h"
#include "operation.h"
#include "l2_interface.h"
#include "proxy.h"
#include "queue.h"
#include "util.h"
#include "update_cache.h"
#include "l3_proxy.h"
#include "l1_proxy.h"
#include "redis_interface.h"
#include "atomicops.h"
#include "readerwriterqueue.h"

class enc_proxy : public proxy {
public:
  void init_proxy(std::shared_ptr<host_info> hosts,
                          std::string instance_name,
                          int local_idx, std::shared_ptr<thrift_response_client_map> client_map,
                          int storage_batch_size);
  void init(const std::vector<std::string> &keys,
            const std::vector<std::string> &values, void **args) override;
  void close() override;
  std::string get(const std::string &key) override;
  void put(const std::string &key, const std::string &value) override;
  std::vector<std::string>
  get_batch(const std::vector<std::string> &keys) override;
  void put_batch(const std::vector<std::string> &keys,
                 const std::vector<std::string> &values) override;

  std::string get(int queue_id, const std::string &key) override;
  void put(int queue_id, const std::string &key,
           const std::string &value) override;
  std::vector<std::string>
  get_batch(int queue_id, const std::vector<std::string> &keys) override;
  void put_batch(int queue_id, const std::vector<std::string> &keys,
                 const std::vector<std::string> &values) override;

  void async_get(const sequence_id &seq_id, const std::string &key);
  void async_put(const sequence_id &seq_id, const std::string &key,
                 const std::string &value);
  void async_get_batch(const sequence_id &seq_id,
                       const std::vector<std::string> &keys);
  void async_put_batch(const sequence_id &seq_id,
                       const std::vector<std::string> &keys,
                       const std::vector<std::string> &values);

  void async_get(const sequence_id &seq_id, int queue_id,
                 const std::string &key);
  void async_put(const sequence_id &seq_id, int queue_id,
                 const std::string &key, const std::string &value);
  void async_get_batch(const sequence_id &seq_id, int queue_id,
                       const std::vector<std::string> &keys);
  void async_put_batch(const sequence_id &seq_id, int queue_id,
                       const std::vector<std::string> &keys,
                       const std::vector<std::string> &values);

  std::future<std::string> get_future(int queue_id, const std::string &key);
  std::future<std::string> put_future(int queue_id, const std::string &key,
                                      const std::string &value);

  void chain_req(const sequence_id& seq, const std::vector<std::string> & arguments) override;
    void setup_chain_stub(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t role, const std::string& next_block_id);
    void resend_pending_stub(const int32_t block_id);

  void flush();

  std::string output_location_ = "log";
  std::string trace_location_ = "";
  std::string instance_name_ = "";

  int security_batch_size_ = 3;
  bool is_static_ = true;

private:
  // void create_security_batch(std::queue<l1_operation> &queue,
  //                            std::vector<l2_operation> &batch,
  //                            std::vector<bool> &is_trues);

  void process_op(const l1_operation &op);

  void encrypt_thread(encryption_engine *enc_engine);
  void decrypt_thread(encryption_engine *enc_engine);
  void responder_thread();

  // std::atomic<bool> finished_;
  // std::string server_host_name_;
  // int server_port_;


  int idx_;

  // Base encryption engine
  // WARNING: Not thread-safe
  encryption_engine encryption_engine_;

  encryption_engine encryption_engine2_;

  
  std::atomic<bool> finished_;
  std::vector<std::thread> threads_;

  // Per-consumer thread state
  // std::vector<std::shared_ptr<queue<l3_operation>>> operation_queues_;
  std::shared_ptr<redis_interface> storage_iface_;
  std::shared_ptr<redis_interface> storage_iface2_;

  std::shared_ptr<moodycamel::BlockingReaderWriterQueue<crypto_op_batch>> encrypt_queue_;
  std::shared_ptr<moodycamel::BlockingReaderWriterQueue<crypto_op_batch>> decrypt_queue_;

  std::queue<l3_operation> internal_queue_;

  // Per-crypto thread state
  // std::shared_ptr<moodycamel::BlockingReaderWriterQueue<crypto_op_batch>> crypto_queue_;
  // std::shared_ptr<redis_interface> storage_iface2_;

  std::shared_ptr<thrift_response_client_map> id_to_client_;
  std::shared_ptr<queue<client_response>> respond_queue_;

  int storage_batch_size_;

  bool encryption_enabled_ = true;
  bool resp_delivery_ = true;
  bool kv_interaction_ = true;
  bool stats_ = false;

};

#endif // ENC_PROXY_H