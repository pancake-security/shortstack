//
// Created by Lloyd Brown on 10/3/19.
//

#ifndef PANCAKE_THRIFT_HANDLER_H
#define PANCAKE_THRIFT_HANDLER_H
#include "pancake_thrift.h"
#include "proxy.h"
#include "pancake_proxy.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/server/TServer.h>
#include <thrift/server/TNonblockingServer.h>
// #include <thrift_response_client.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

class thrift_handler : virtual public pancake_thriftIf {
public:
    //thrift_handler();

    //thrift_handler(std::shared_ptr<proxy> proxy, const std::string &proxy_type);

    thrift_handler(std::shared_ptr<proxy> proxy, const std::string &proxy_type,
                   std::atomic<int64_t> &client_id_gen,
                   std::shared_ptr<::apache::thrift::protocol::TProtocol> prot,
                   std::shared_ptr<thrift_response_client_map> &id_to_client);

    int64_t get_client_id();

    void register_client_id(const int32_t block_id, const int64_t client_id);

    void get(std::string& _return, const std::string& key);

    void async_get(const sequence_id& seq_id, const std::string& key);

    void put(const std::string& key, const std::string& value);

    void async_put(const sequence_id& seq_id, const std::string& key, const std::string& value);

    void get_batch(std::vector<std::string> & _return, const std::vector<std::string> & keys);

    void async_get_batch(const sequence_id& seq_id, const std::vector<std::string> & keys);

    void put_batch(const std::vector<std::string> & keys, const std::vector<std::string> & values);

    void async_put_batch(const sequence_id& seq_id, const std::vector<std::string> & keys, const std::vector<std::string> & values);

    void chain_request(const sequence_id& seq, const int32_t block_id, const std::vector<std::string> & arguments);

    void setup_chain(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t role, const std::string& next_block_id);
    
    void resend_pending(const int32_t block_id);

    void update_connections(const int32_t type, const int32_t column, const std::string& hostname, const int32_t port, const int32_t num_workers);

    void external_ack(const sequence_id& seq);

    void external_ack_batch(const std::vector<sequence_id> & seqs);

private:
    int operation_count_ = 0;
    std::shared_ptr<proxy> proxy_;
    std::string proxy_type_ = "pancake";

    std::shared_ptr<::apache::thrift::protocol::TProtocol> prot_;

    /* Block response client */
    std::shared_ptr<thrift_response_client> client_;

    /* Registered client identifier */
    int64_t registered_client_id_;

    /* Client identifier generator */
    std::atomic<int64_t> &client_id_gen_;

    std::shared_ptr<thrift_response_client_map> &id_to_client_;
};
#endif //PANCAKE_THRIFT_HANDLER_H
