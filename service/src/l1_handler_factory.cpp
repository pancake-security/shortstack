
#include "l1_handler_factory.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

l1_handler_factory::l1_handler_factory(std::shared_ptr<l1_proxy> proxy, const std::string &proxy_type,
                                               std::shared_ptr<thrift_response_client_map> id_to_client) {
    proxy_ = proxy;
    proxy_type_ = proxy_type;
    id_to_client_ = id_to_client;
}

pancake_thriftIf* l1_handler_factory::getHandler(const ::apache::thrift::TConnectionInfo &conn_info) {
    std::shared_ptr<TSocket> sock = std::dynamic_pointer_cast<TSocket>(conn_info.transport);
    sock->setSendTimeout(1200000);
    auto transport = std::make_shared<TFramedTransport>(conn_info.transport);
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    return new l1_handler(proxy_, proxy_type_, client_id_gen_, protocol, id_to_client_);
}

void l1_handler_factory::releaseHandler(block_request_serviceIf *handler) {
    auto br_handler = reinterpret_cast<l1_handler *>(handler);
    delete handler;
}
