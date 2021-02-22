// Shortstack L2 proxy

#ifndef L2_INTERFACE_H
#define L2_INTERFACE_H

#include <string>

#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TTransportUtils.h>

#include "proxy_types.h"
#include "l2proxy.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

struct l2_operation {
  sequence_id seq_id;
  std::string key;
  int replica;
  std::string value;
};

class l2proxy_interface {

public:
  l2proxy_interface(std::vector<std::string> hosts, std::vector<int> ports);

  void connect();

  void send_op(const l2_operation &op);

private:
  std::vector<std::string> hosts_;
  std::vector<int> ports_;

  std::vector<std::shared_ptr<TSocket>> sockets_;
  std::vector<std::shared_ptr<TTransport>> transports_;
  std::vector<std::shared_ptr<TProtocol>> protocols_;
  std::vector<std::shared_ptr<l2proxyClient>> clients_;
  
};

#endif // L2_INTERFACE_H