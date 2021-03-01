//
// Shortstack L2 proxy
//

#ifndef L2_PROXY_H
#define L2_PROXY_H

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
#include "l2_interface.h"
#include "l3_interface.h"
#include "operation.h"
#include "queue.h"
#include "update_cache.h"
#include "util.h"

class l2_proxy {
public:
  void init_proxy(std::shared_ptr<host_info> hosts, std::string instance_name,
                  std::shared_ptr<distribution_info> dist_info, int num_cores);

  void async_operation(const sequence_id &seq_id, const std::string &key,
                       int replica, const std::string &value);

  void close();

private:
  void consumer_thread(int id);

  std::string instance_name_;
  std::string server_host_name_;
  int server_port_;

  // Label map
  std::unordered_map<std::string, int> replica_to_label_;
  std::unordered_map<std::string, int> key_to_number_of_replicas_;

  // Update Cache
  update_cache update_cache_;

  // Per-consumer thread state
  std::atomic<bool> finished_;
  std::vector<std::thread> threads_;
  std::vector<std::shared_ptr<l3proxy_interface>> l3_ifaces_;
  std::vector<std::shared_ptr<queue<l2_operation>>> operation_queues_;

};

#endif // L2_PROXY_H
