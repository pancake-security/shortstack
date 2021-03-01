//
// Shortstack L2 proxy
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
#include "redis.h"
#include "storage_interface.h"
#include "thrift_response_client_map.h"
#include "update_cache.h"
#include "util.h"

struct client_response {
  sequence_id seq_id;
  int op_code;
  std::string result;
};

class l3_proxy {
public:
  void init_proxy(std::shared_ptr<host_info> hosts, std::string instance_name,
                  int kvclient_threads, int storage_batch_size,
                  std::shared_ptr<thrift_response_client_map> client_map,
                  int num_cores);

  void async_operation(const sequence_id &seq_id, const std::string &label,
                       const std::string &value, bool is_read);

  void close();

private:
  void consumer_thread(int id, encryption_engine *enc_engine);
  void responder_thread();

  void execute_batch(const std::vector<l3_operation> &operations,
                     std::shared_ptr<storage_interface> storage_interface,
                     encryption_engine *enc_engine);

  std::string instance_name_;
  std::string server_host_name_;
  int server_port_;

  // Base encryption engine
  // WARNING: Not thread-safe
  encryption_engine encryption_engine_;

  // Per-consumer thread state
  std::atomic<bool> finished_;
  std::vector<std::thread> threads_;
  std::vector<std::shared_ptr<queue<l3_operation>>> operation_queues_;
  std::vector<std::shared_ptr<storage_interface>> storage_ifaces_;

  std::shared_ptr<thrift_response_client_map> id_to_client_;
  queue<client_response> respond_queue_;

  int storage_batch_size_;
  const int64_t fake_client_id_ = -1995;

  int GET = 0;
  int PUT = 1;
};

#endif // L3_PROXY_H
