
#ifndef L1_HANDLER_FACTORY_H
#define L1_HANDLER_FACTORY_H

#include "l1_handler.h"
//#include <thrift/protocol/TBinaryProtocol.h>
//#include <thrift/server/TSimpleServer.h>
//#include <thrift/transport/TSocket.h>
//#include <thrift/transport/TServerSocket.h>
//#include <thrift/transport/TNonblockinsgServerSocket.h>
//#include <thrift/transport/TBufferTransports.h>
//#include <thrift/server/TServer.h>
//#include <thrift/server/TNonblockingServer.h>
//#include "../../proxy/src/proxy.h"
#include "pancake_thrift.h"

class l1_handler_factory : public pancake_thriftIfFactory {
public:
    explicit l1_handler_factory(std::shared_ptr<l1_proxy> proxy, const std::string &proxy_type, 
                                    std::shared_ptr<thrift_response_client_map> id_to_client);
    pancake_thriftIf *getHandler(const ::apache::thrift::TConnectionInfo &connInfo) override;
    void releaseHandler(block_request_serviceIf *anIf) override;

private:
    std::shared_ptr<l1_proxy> proxy_;
    std::string proxy_type_ = "l1";
    std::shared_ptr<thrift_response_client_map> id_to_client_;
    std::atomic<int64_t> client_id_gen_;
};

#endif //L1_HANDLER_FACTORY_H
