#include "l2_handler_factory.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

l2_handler_factory::l2_handler_factory(std::shared_ptr<l2_proxy> proxy) {
    proxy_ = proxy;
}

l2proxyIf* l2_handler_factory::getHandler(const ::apache::thrift::TConnectionInfo &conn_info) {
    std::shared_ptr<TSocket> sock = std::dynamic_pointer_cast<TSocket>(conn_info.transport);
    sock->setSendTimeout(1200000);
    auto transport = std::make_shared<TFramedTransport>(conn_info.transport);
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    return new l2_handler(proxy_, protocol);
}

void l2_handler_factory::releaseHandler(block_request_serviceIf* handler) {
    auto br_handler = reinterpret_cast<l2_handler *>(handler);
    delete handler;
}
