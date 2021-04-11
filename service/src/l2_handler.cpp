#include "l2_handler.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

l2_handler::l2_handler(std::shared_ptr<l2_proxy> proxy,
                               std::shared_ptr<::apache::thrift::protocol::TProtocol> prot)
    : prot_(std::move(prot))
{
    proxy_ = proxy;
}

void l2_handler::l2request(const sequence_id &seq_id, const std::string &key,
                 const int32_t replica, const std::string &value) {
    proxy_->async_operation(seq_id, key, replica, value);
}

void l2_handler::chain_request(const sequence_id& seq, const int32_t block_id, const std::vector<std::string> & arguments) {
    proxy_->chain_request(seq, arguments);
}
