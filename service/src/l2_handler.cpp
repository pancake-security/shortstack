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

void l2_handler::setup_chain(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t role, const std::string& next_block_id) {
    proxy_->setup(path, chain, (chain_role) role, next_block_id);
}
    
void l2_handler::resend_pending(const int32_t block_id) {
    proxy_->resend_pending();
}

void l2_handler::update_connections(const int32_t type, const int32_t column, const std::string& hostname, const int32_t port, const int32_t num_workers) {
    proxy_->update_connections(type, column, hostname, port, num_workers);
}