//
// Shortstack L1 proxy
//

#ifndef L1_PROXY_H
#define L1_PROXY_H

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
#include "operation.h"
#include "proxy.h"
#include "queue.h"
#include "util.h"

struct l1_operation {
  sequence_id seq_id;
  std::string key;
  std::string value;
};

class l1_proxy : public proxy {
public:
  void init_proxy(std::shared_ptr<host_info> hosts, std::string instance_name,
                  std::shared_ptr<distribution_info> dist_info);
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

  void flush();

  std::string output_location_ = "log";
  std::string trace_location_ = "";
  std::string instance_name_ = "";

  int security_batch_size_ = 3;
  bool is_static_ = true;

private:
  void create_security_batch(std::queue<l1_operation> &queue,
                             std::vector<l2_operation> &batch,
                             std::vector<bool> &is_trues);

  bool is_true_distribution();
  void consumer_thread(int id);

  std::atomic<bool> finished_;
  std::string server_host_name_;
  int server_port_;

  // Distribution state
  int num_keys_;
  std::string dummy_key_;
  double delta_;
  std::unordered_map<std::string, int> key_to_number_of_replicas_;
  distribution fake_distribution_;
  distribution real_distribution_;

  // Per-consumer thread state
  std::vector<std::thread> threads_;
  std::vector<std::shared_ptr<l2proxy_interface>> l2_ifaces_;
  std::vector<std::shared_ptr<queue<l1_operation>>> operation_queues_;

  const int64_t fake_client_id_ = -1995;
};

#endif // L1_PROXY_H