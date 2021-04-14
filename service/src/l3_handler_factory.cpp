#include "l3_handler_factory.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

l3_handler_factory::l3_handler_factory(std::shared_ptr<l3_proxy> proxy,
                                               std::shared_ptr<thrift_response_client_map> id_to_client) {
    proxy_ = proxy;
    id_to_client_ = id_to_client;
}

l3proxyIf* l3_handler_factory::getHandler(const ::apache::thrift::TConnectionInfo &conn_info) {
    std::shared_ptr<TSocket> sock = std::dynamic_pointer_cast<TSocket>(conn_info.transport);
    sock->setSendTimeout(1200000);
    auto transport = std::make_shared<TFramedTransport>(conn_info.transport);
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    return new l3_handler(proxy_, protocol, id_to_client_, client_id_gen_);
}

void l3_handler_factory::releaseHandler(l3proxyIf *handler) {
    auto br_handler = reinterpret_cast<l3_handler *>(handler);
    delete handler;
}