#ifndef PANCAKE_CLIENTTT_H
#define PANCAKE_CLIENTTT_H

#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TTransportUtils.h>
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <iostream>

// #include "client.h"
#include "queue.h"
#include "pancake_thrift.h"
#include "pancake_thrift_response.h"
#include "command_response_reader.h"
#include "host_info.h"
#include "l3proxy.h"
#include "util.h"


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

struct l3_response {
    int64_t sequence_num;
    std::string value;
    std::string diag;
};

// WARNING: Not thread-safe

class pancake_client {

public:

    // client_id -> unique client identifier
    void init(int64_t client_id, std::shared_ptr<host_info> hosts);
    int64_t get_client_id();
    int64_t get(const std::string &key);
    int64_t put(const std::string &key, const std::string &value);
    int64_t poll_responses(std::string &out, std::string &diag);
    // std::vector<std::string> get_batch(const std::vector<std::string> &keys) override;
    // void put_batch(const std::vector<std::string> &keys, const std::vector<std::string> &values) override;
    void finish();


private:

    void response_thread(int idx);

    void flush_thread();

    // L1 proxy connections
    std::vector<std::shared_ptr<TSocket>> l1_sockets_;
  std::vector<std::shared_ptr<TTransport>> l1_transports_;
  std::vector<std::shared_ptr<TProtocol>> l1_protocols_;
  std::vector<std::shared_ptr<pancake_thriftClient>> l1_clients_;

  // Extra L1 proxy connections
    std::vector<std::shared_ptr<TSocket>> flush_sockets_;
  std::vector<std::shared_ptr<TTransport>> flush_transports_;
  std::vector<std::shared_ptr<TProtocol>> flush_protocols_;
  std::vector<std::shared_ptr<pancake_thriftClient>> flush_clients_;

    // L3 proxy connections
    std::vector<std::shared_ptr<TSocket>> l3_sockets_;
  std::vector<std::shared_ptr<TTransport>> l3_transports_;
  std::vector<std::shared_ptr<TProtocol>> l3_protocols_;
  std::vector<std::shared_ptr<pancake_thriftClient>> l3_clients_;

    int64_t total_ = 0;
    int64_t sequence_num_ = 0;
    int64_t client_id_;

    std::vector<command_response_reader> readers_;

    std::vector<std::thread> response_threads_;
    std::thread flush_thread_;
    std::shared_ptr<queue<l3_response>> response_queue_;
    std::atomic<bool> done_;

    int num_servers_ = 0;
    int num_workers_ = 0;
    
};


#endif //PANCAKE_CLIENTTT_H
