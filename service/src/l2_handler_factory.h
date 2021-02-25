#ifndef L2_HANDLER_FACTORY_H
#define L2_HANDLER_FACTORY_H

#include "l2_handler.h"
#include "l2proxy.h"
#include "l2_proxy.h" // YUCK!

class l2_handler_factory : public l2proxyIfFactory {
public:
    explicit l2_handler_factory(std::shared_ptr<l2_proxy> proxy);
    l2proxyIf *getHandler(const ::apache::thrift::TConnectionInfo &connInfo) override;
    void releaseHandler(l2proxyIf *anIf) override;

private:
    std::shared_ptr<l2_proxy> proxy_;
};

#endif //L2_HANDLER_FACTORY_H
