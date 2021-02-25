#ifndef L2_HANDLER_H
#define L2_HANDLER_H
#include "l2_proxy.h"
#include "l2proxy.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/server/TServer.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/transport/TServerSocket.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

class l2_handler : virtual public l2proxyIf {
public:
  l2_handler(std::shared_ptr<l2_proxy> proxy,
             std::shared_ptr<::apache::thrift::protocol::TProtocol> prot);

  void l2request(const sequence_id &seq_id, const std::string &key,
                 const int32_t replica, const std::string &value);

private:
  std::shared_ptr<l2_proxy> proxy_;

  std::shared_ptr<::apache::thrift::protocol::TProtocol> prot_;
};
#endif // L2_HANDLER_H
