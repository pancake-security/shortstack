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
    if (!proxy_->is_set_prev()) {
        // TODO: What if it is set, but the chain predecessor changed due to failure resolution
        proxy_->reset_prev(prot_);
    }
    proxy_->chain_request(seq, arguments);
}

void l2_handler::setup_chain(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t role, const std::string& next_block_id) {
    proxy_->setup(path, chain, (chain_role) role, next_block_id);
}
    
void l2_handler::resend_pending(const int32_t block_id, const int64_t successor_seq) {
    proxy_->resend_pending(successor_seq);
}

int64_t l2_handler::fetch_seq(const int32_t block_id) {
    if(block_id == -1995) {
    std::cerr << "Failure injection" << std::endl;
    exit(-1);
  }
    return proxy_->fetch_seq();
}

void l2_handler::update_connections(const int32_t type, const int32_t column, const std::string& hostname, const int32_t port, const int32_t num_workers) {
    proxy_->update_connections(type, column, hostname, port, num_workers);
}

void l2_handler::selective_resend_pending(const int32_t column, const int32_t num_columns) {
    proxy_->selective_resend_pending(column, num_columns);
}

void l2_handler::external_ack(const sequence_id& seq) {
    if(seq.client_id == -1995 && seq.client_seq_no == -1995) {
        std::cerr << "Failure injection" << std::endl;
        exit(-1);
    }
    
    proxy_->external_ack(seq);
}
void l2_handler::external_ack_batch(const std::vector<sequence_id> & seqs) {
    proxy_->external_ack_batch(seqs);
}
