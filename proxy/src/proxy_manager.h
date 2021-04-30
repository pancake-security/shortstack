#ifndef PROXY_MANAGER_H
#define PROXY_MANAGER_H

#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TTransportUtils.h>
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <iostream>

// #include "client.h"
#include "queue.h"
#include "pancake_thrift.h"
#include "pancake_thrift_response.h"
#include "command_response_reader.h"
#include "host_info.h"
#include "l3proxy.h"
#include "l2proxy.h"
#include "util.h"
#include "chain_module.h"


using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;


// WARNING: Not thread-safe

class proxy_manager {

    struct block_srv_client {
        std::shared_ptr<block_request_serviceClient> client;
        std::shared_ptr<TSocket> socket;
        std::shared_ptr<TTransport> transport;
        std::shared_ptr<TProtocol> protocol;

    }; 

public:

    void init(std::shared_ptr<host_info> hosts);

    void setup_reverse_connections();

    void fail_node(std::string instance_name, int delay_sec);

    // Fail all nodes running on a given host
    void fail_host(std::string hostname);


private:

    int get_idx(const host &h, const std::vector<host> &list);
    void setup_chain(host *h, std::string path, chain_role role, host *next);
    void resend_pending(host *h, host* next);
    void update_connections(host *h, int type, int column, host *target);
    void selective_resend_pending(host *h, int column, int num_columns);
    void crash_host(host *h);

    std::shared_ptr<block_request_serviceClient> get_block_client(std::string hostname, int port);

    std::shared_ptr<host_info> hosts_{nullptr};

    std::map<std::pair<std::string, int>, block_srv_client> block_client_cache_;
    
};


#endif //PROXY_MANAGER_H
