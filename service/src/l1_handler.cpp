
#include "l1_handler.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

// l1_handler::l1_handler() {
//     client_id_gen_(client_id_gen);
// }

// l1_handler::l1_handler(std::shared_ptr<proxy> proxy, const std::string &proxy_type) {
//     proxy_ = proxy;
//     proxy_type_ = proxy_type;
//     std::atomic<int64_t> client_id_gen_;
// }

l1_handler::l1_handler(std::shared_ptr<l1_proxy> proxy, const std::string &proxy_type,
                               std::atomic<int64_t> &client_id_gen,
                               std::shared_ptr<::apache::thrift::protocol::TProtocol> prot,
                               std::shared_ptr<thrift_response_client_map> &id_to_client)
    : prot_(std::move(prot)),
      client_(std::make_shared<thrift_response_client>(prot_)),
      registered_client_id_(-1),
      client_id_gen_(client_id_gen),
      id_to_client_(id_to_client)
{
    proxy_ = proxy;
    proxy_type_ = proxy_type;
}

int64_t l1_handler::l1_handler::get_client_id() {
    return client_id_gen_.fetch_add(1L);    
}

void l1_handler::register_client_id(const int32_t block_id, const int64_t client_id) {
    registered_client_id_ = client_id;
    id_to_client_->add_client(client_id, client_);
}

void l1_handler::async_get(const sequence_id& seq_id, const std::string& key) {
    proxy_->async_get(seq_id, operation_count_++, key);
}

void l1_handler::async_put(const sequence_id& seq_id, const std::string& key, const std::string& value) {
    proxy_->async_put(seq_id, operation_count_++, key, value);
}

void l1_handler::async_get_batch(const sequence_id& seq_id, const std::vector<std::string> & keys) {
    proxy_->async_get_batch(seq_id, operation_count_++, keys);
}

void l1_handler::async_put_batch(const sequence_id& seq_id, const std::vector<std::string> & keys, const std::vector<std::string> & values) {
    proxy_->async_put_batch(seq_id, operation_count_++, keys, values);
}


void l1_handler::get(std::string& _return, const std::string& key) {
    _return = proxy_->get(operation_count_++, key);
}

void l1_handler::put(const std::string& key, const std::string& value) {
    proxy_->put(operation_count_++, key, value);
}

void l1_handler::get_batch(std::vector<std::string> & _return, const std::vector<std::string> &keys) {
    _return = proxy_->get_batch(operation_count_++, keys);
}

void l1_handler::put_batch(const std::vector<std::string> & keys, const std::vector<std::string> & values) {
    proxy_->put_batch(operation_count_++, keys, values);
}

void l1_handler::chain_request(const sequence_id& seq, const int32_t block_id, const std::vector<std::string> & arguments) {
    if (!proxy_->is_set_prev()) {
        proxy_->reset_prev(prot_);
    }
    proxy_->chain_request(seq, arguments);
}

void l1_handler::setup_chain(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t role, const std::string& next_block_id) {
    proxy_->setup(path, chain, (chain_role)role, next_block_id);
}
    
void l1_handler::resend_pending(const int32_t block_id) {
    proxy_->resend_pending();
}