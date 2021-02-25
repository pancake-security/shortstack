#ifndef L2_SERVER_H
#define L2_SERVER_H

#include <thrift/concurrency/PlatformThreadFactory.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/server/TServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/transport/TServerSocket.h>

#include "l2_proxy.h"

class l2_server {
public:
  typedef std::shared_ptr<apache::thrift::server::TServer> server_ptr;
  static server_ptr create(std::shared_ptr<l2_proxy> proxy, int port,
                           size_t num_worker_threads, size_t num_io_threads);
};

#endif // L2_SERVER_H
