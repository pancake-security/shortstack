#ifndef L3_SERVER_H
#define L3_SERVER_H

#include <thrift/concurrency/PlatformThreadFactory.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/server/TServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/transport/TServerSocket.h>

#include "l3_handler_factory.h"

class l3_server {
public:
  typedef std::shared_ptr<apache::thrift::server::TServer> server_ptr;
  static server_ptr
  create(std::shared_ptr<l3_proxy> proxy,
         std::shared_ptr<thrift_response_client_map> id_to_client_, int port,
         size_t num_worker_threads, size_t num_io_threads);
};

#endif // L3_SERVER_H
