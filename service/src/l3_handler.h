#ifndef L3_HANDLER_H
#define L3_HANDLER_H
#include "l3_proxy.h"
#include "l3proxy.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/server/TServer.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/transport/TServerSocket.h>
// #include <thrift_response_client.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

class l3_handler : virtual public l3proxyIf {
public:
  // l3_handler();

  // l3_handler(std::shared_ptr<proxy> proxy, const std::string &proxy_type);

  l3_handler(std::shared_ptr<l3_proxy> proxy,
             std::shared_ptr<::apache::thrift::protocol::TProtocol> prot,
             std::shared_ptr<thrift_response_client_map> &id_to_client,
             std::atomic<int64_t> &client_id_gen);

  void register_client_id(const int64_t client_id);

  int64_t get_client_id();

  void l3request(const sequence_id& seq_id, const std::string& label, const std::string& value, const bool is_read, const bool dedup);

  void chain_request(const sequence_id& seq, const int32_t block_id, const std::vector<std::string> & arguments);

  void setup_chain(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t role, const std::string& next_block_id);
    
  void resend_pending(const int32_t block_id, const int64_t successor_seq);
    
    int64_t fetch_seq(const int32_t block_id);

  void update_connections(const int32_t type, const int32_t column, const std::string& hostname, const int32_t port, const int32_t num_workers);

  void external_ack(const sequence_id& seq);

  void external_ack_batch(const std::vector<sequence_id> & seqs);

private:
  std::shared_ptr<l3_proxy> proxy_;

  std::shared_ptr<::apache::thrift::protocol::TProtocol> prot_;

  /* Block response client */
  std::shared_ptr<thrift_response_client> client_;

  /* Registered client identifier */
  int64_t registered_client_id_;

  std::shared_ptr<thrift_response_client_map> &id_to_client_;

  std::atomic<int64_t> &client_id_gen_;

};
#endif // L3_HANDLER_H
