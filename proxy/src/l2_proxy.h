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
#include "chain_module.h"
#include "reverse_connector.h"

class l1ack_interface : public reverse_connector {
  public:
    l1ack_interface(std::shared_ptr<host_info> hosts, int batch_size);
    int route(const sequence_id &seq) override;
};

class l2_proxy: public chain_module {
public:
  void init_proxy(std::shared_ptr<host_info> hosts, std::string instance_name,
                  std::shared_ptr<distribution_info> dist_info,
                  std::shared_ptr<update_cache> update_cache,
                  bool uc_enabled, int local_idx, bool stats,
                  int ack_batch_size);

  void async_operation(const sequence_id &seq_id, const std::string &key,
                       int replica, const std::string &value);

  void close();

  void run_command(const sequence_id &seq, const arg_list &args) override;
  void replication_complete(const sequence_id &seq, const arg_list &args) override;
  void setup_callback() override;
  void ack_callback(const sequence_id &seq) override;

  void update_connections(int type, int column, std::string hostname, int port, int num_workers);

  void selective_resend_pending(const int32_t column, const int32_t num_columns);

  void external_ack(const sequence_id& seq);

  void external_ack_batch(const std::vector<sequence_id> & seqs);

private:
  // void consumer_thread(int id);

  //  Filter requests that map to a given L3 column
  bool filter_request(const sequence_id &seq, const arg_list &args, int column, int num_columns);

  void forward_request(const sequence_id &seq, const arg_list &args, bool dedup);

  std::string instance_name_;
  // std::string server_host_name_;
  // int server_port_;

  // Label map
  std::unordered_map<std::string, int> replica_to_label_;
  std::unordered_map<std::string, int> key_to_number_of_replicas_;

  // Update Cache
  std::shared_ptr<update_cache> update_cache_;

  // Per-consumer thread state
  // std::atomic<bool> finished_;
  // std::vector<std::thread> threads_;
  std::shared_ptr<l3proxy_interface> l3_iface_{nullptr};
  // std::vector<std::shared_ptr<queue<l2_operation>>> operation_queues_;

  std::string dummy_key_;
  bool update_cache_enabled_ = true;

  int idx_;
  std::shared_ptr<host_info> hosts_{nullptr};

  // Per-L1 sequence numbers
  std::vector<int64_t> last_seen_seq_;

  std::shared_ptr<l1ack_interface> ack_iface_{nullptr};
  bool stats_;

  int ack_batch_size_{1};

};

#endif // L2_PROXY_H
