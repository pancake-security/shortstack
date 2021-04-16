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

public:

    void init(std::shared_ptr<host_info> hosts);

    void setup_reverse_connections();

    void fail_node(std::string instance_name);


private:

    int get_idx(const host &h, const std::vector<host> &list);
    void setup_chain(host *h, std::string path, chain_role role, host *next);
    void resend_pending(host *h);
    void update_connections(host *h, int type, int column, host *target);
    void selective_resend_pending(host *h, int column, int num_columns);

    std::shared_ptr<host_info> hosts_{nullptr};
    
};


#endif //PROXY_MANAGER_H
