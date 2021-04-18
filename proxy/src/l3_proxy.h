//
// Shortstack L3 proxy
//

#ifndef L3_PROXY_H
#define L3_PROXY_H

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
#include "encryption_engine.h"
#include "host_info.h"
#include "l3_interface.h"
#include "operation.h"
#include "queue.h"
// #include "redis.h"
#include "redis_interface.h"
#include "thrift_response_client_map.h"
#include "update_cache.h"
#include "util.h"
#include "dummy_kv.h"
#include "reverse_connector.h"

#include "atomicops.h"
#include "readerwriterqueue.h"

struct client_response {
  sequence_id seq_id;
  int op_code;
  std::string result;
};

struct crypto_operation {
  l3_operation l3_op;
  std::string kv_response;
};

typedef std::vector<crypto_operation> crypto_op_batch;

const int OP_GET = 0;
const int OP_PUT = 1;

class l2ack_interface : public reverse_connector {
  public:
    l2ack_interface(std::shared_ptr<host_info> hosts);
    int route(const sequence_id &seq) override;
};

class l3_proxy {
public:
  void init_proxy(std::shared_ptr<host_info> hosts, std::string instance_name,
                  int kvclient_threads, int storage_batch_size,
                  std::shared_ptr<thrift_response_client_map> client_map,
                  bool encryption_enabled, bool resp_delivery,
                  bool kv_interaction, int local_idx,
                  int64_t timeout_us, bool ack_delivery, bool stats);

  void async_operation(const sequence_id &seq_id, const std::string &label,
                       const std::string &value, bool is_read, bool dedup);

  void close();

  void update_connections(int type, int column, std::string hostname, int port, int num_workers);

private:
  void consumer_thread();
  void crypto_thread(encryption_engine *enc_engine);
  void responder_thread();

  // void execute_batch(const std::vector<l3_operation> &operations,
  //                    std::shared_ptr<storage_interface> storage_interface,
  //                    encryption_engine *enc_engine);

  std::string instance_name_;
  // std::string server_host_name_;
  // int server_port_;

  // Base encryption engine
  // WARNING: Not thread-safe
  encryption_engine encryption_engine_;

  
  std::atomic<bool> finished_;
  std::vector<std::thread> threads_;

  // Per-consumer thread state
  // std::shared_ptr<moodycamel::BlockingReaderWriterQueue<l3_operation>> operation_queue_;
  std::shared_ptr<redis_interface> storage_iface_;

  std::shared_ptr<l2ack_interface> ack_iface_;

  // Per-crypto thread state
  std::shared_ptr<moodycamel::BlockingReaderWriterQueue<crypto_op_batch>> crypto_queue_;
  std::shared_ptr<redis_interface> storage_iface2_;

  std::shared_ptr<thrift_response_client_map> id_to_client_;
  std::shared_ptr<queue<client_response>> respond_queue_;

  int storage_batch_size_;
  const int64_t fake_client_id_ = -1995;

  bool encryption_enabled_ = true;
  bool resp_delivery_ = true;
  bool kv_interaction_ = true;
  bool ack_delivery_ = true;

  int idx_;
  std::shared_ptr<host_info> hosts_{nullptr};

  // Per-L2 sequence numbers
  std::vector<int64_t> last_seen_seq_;
  int64_t timeout_us_;

  bool stats_;
};

#endif // L3_PROXY_H
