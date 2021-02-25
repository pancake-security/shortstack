#ifndef L3_HANDLER_FACTORY_H
#define L3_HANDLER_FACTORY_H

#include "l3_handler.h"
#include "l3proxy.h"
#include "l3_proxy.h" //Yuck!

class l3_handler_factory : public l3proxyIfFactory {
public:
    explicit l3_handler_factory(std::shared_ptr<l3_proxy> proxy,
                                    std::shared_ptr<thrift_response_client_map> id_to_client);
    l3proxyIf *getHandler(const ::apache::thrift::TConnectionInfo &connInfo) override;
    void releaseHandler(l3proxyIf *anIf) override;

private:
    std::shared_ptr<l3_proxy> proxy_;
    std::shared_ptr<thrift_response_client_map> id_to_client_;
    // std::atomic<int64_t> client_id_gen_;
};

#endif //L3_HANDLER_FACTORY_H
