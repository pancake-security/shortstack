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
#include "host_info.h"

// TODO: Hack
#define L3_NUM_CORES 16

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

struct l3_operation {
  sequence_id seq_id;
  std::string label;
  std::string value;
  std::string plaintext;
  bool is_read;
  bool dedup;
};

class l3proxy_interface {

public:

  typedef std::vector<std::shared_ptr<TSocket>> sock_list;
  typedef std::vector<std::shared_ptr<TTransport>> transport_list;
  typedef std::vector<std::shared_ptr<TProtocol>> prot_list;
  typedef std::vector<std::shared_ptr<l3proxyClient>> client_list;

  l3proxy_interface(std::shared_ptr<host_info> hinfo);

  void connect();

  void send_op(const l3_operation &op);

  void remove_connection(int column);

private:

  std::shared_ptr<host_info> hosts_;

  std::vector<sock_list> sockets_;
  std::vector<transport_list> transports_;
  std::vector<prot_list> protocols_;
  std::vector<client_list> clients_;
};

#endif // L3_INTERFACE_H