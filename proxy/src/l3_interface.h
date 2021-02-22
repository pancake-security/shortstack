// L3 proxy interface

#ifndef L3_INTERFACE_H
#define L3_INTERFACE_H

#include <string>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "l3proxy.h"
#include "proxy_types.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

struct l3_operation {
  sequence_id seq_id;
  std::string label;
  std::string value;
  bool is_read;
};

class l3proxy_interface {

public:
  l3proxy_interface(std::vector<std::string> hosts, std::vector<int> ports);

  void connect();

  void send_op(const l3_operation &op);

private:
  std::vector<std::string> hosts_;
  std::vector<int> ports_;

  std::vector<std::shared_ptr<TSocket>> sockets_;
  std::vector<std::shared_ptr<TTransport>> transports_;
  std::vector<std::shared_ptr<TProtocol>> protocols_;
  std::vector<std::shared_ptr<l3proxyClient>> clients_;
};

#endif // L3_INTERFACE_H