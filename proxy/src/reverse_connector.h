// Reverse connector for delivering acks

#ifndef REVERSE_CONNECTOR_H
#define REVERSE_CONNECTOR_H

#include <string>
#include <queue>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "block_request_service.h"
#include "proxy_types.h"
#include "host_info.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;


class reverse_connector {

public:

  typedef std::vector<std::shared_ptr<TSocket>> sock_list;
  typedef std::vector<std::shared_ptr<TTransport>> transport_list;
  typedef std::vector<std::shared_ptr<TProtocol>> prot_list;
  typedef std::vector<std::shared_ptr<block_request_serviceClient>> client_list;

  reverse_connector(std::shared_ptr<host_info> hinfo, int type, int batch_size);

  void send_ack(const sequence_id &seq);

  void update_connections(int column, std::string hostname, int port, int num_workers);

  virtual int route(const sequence_id &seq) = 0;

private:

void recompute_client_array();

  std::shared_ptr<host_info> hosts_;

  std::vector<sock_list> sockets_;
  std::vector<transport_list> transports_;
  std::vector<prot_list> protocols_;
  std::vector<client_list> clients_;

  int type_;  
  client_list client_arr_;

   std::vector<std::queue<sequence_id>> ack_queues_;

   int batch_size_{1};
};

#endif // REVERSE_CONNECTOR_H