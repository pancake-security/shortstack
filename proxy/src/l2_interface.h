// Shortstack L2 proxy

#ifndef L2_INTERFACE_H
#define L2_INTERFACE_H

#include <string>

#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TTransportUtils.h>

#include "proxy_types.h"
#include "host_info.h"
#include "l2proxy.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

struct l2_operation {
  sequence_id seq_id;
  std::string key;
  int replica;
  std::string value;

  void serialize(std::vector<std::string> & args);
  int deserialize(const std::vector<std::string> & args, int idx);
};

class l2proxy_interface {

public:

  typedef std::vector<std::shared_ptr<TSocket>> sock_list;
  typedef std::vector<std::shared_ptr<TTransport>> transport_list;
  typedef std::vector<std::shared_ptr<TProtocol>> prot_list;
  typedef std::vector<std::shared_ptr<l2proxyClient>> client_list;


  l2proxy_interface(std::vector<host> hosts, std::string dummy_key);

  void connect();

  void send_op(const l2_operation &op);

private:
  std::vector<host> hosts_;

  std::vector<sock_list> sockets_;
  std::vector<transport_list> transports_;
  std::vector<prot_list> protocols_;
  std::vector<client_list> clients_;

  std::string dummy_key_;
  
};

#endif // L2_INTERFACE_H