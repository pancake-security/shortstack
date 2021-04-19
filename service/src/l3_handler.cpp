#include "l3_handler.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

l3_handler::l3_handler(
    std::shared_ptr<l3_proxy> proxy,
    std::shared_ptr<::apache::thrift::protocol::TProtocol> prot,
    std::shared_ptr<thrift_response_client_map> &id_to_client,
    std::atomic<int64_t> &client_id_gen)
    : prot_(std::move(prot)),
      client_(std::make_shared<thrift_response_client>(prot_)),
      registered_client_id_(-1), id_to_client_(id_to_client),
      client_id_gen_(client_id_gen) {
  proxy_ = proxy;
}

void l3_handler::register_client_id(const int64_t client_id) {
  registered_client_id_ = client_id;
  id_to_client_->add_client(client_id, client_);
}



void l3_handler::l3request(const sequence_id &seq_id, const std::string &label,
                           const std::string &value, const bool is_read,
                           const bool dedup) {
  proxy_->async_operation(seq_id, label, value, is_read, dedup);
}

void l3_handler::chain_request(const sequence_id& seq, const int32_t block_id, const std::vector<std::string> & arguments) {
  throw std::logic_error("Not implemented"); 
}

void l3_handler::setup_chain(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t role, const std::string& next_block_id) {
  throw std::logic_error("Not implemented"); 
}

void l3_handler::resend_pending(const int32_t block_id) {
  throw std::logic_error("Not implemented"); 
}

void l3_handler::update_connections(const int32_t type, const int32_t column, const std::string& hostname, const int32_t port, const int32_t num_workers) {
  proxy_->update_connections(type, column, hostname, port, num_workers);
}

void l3_handler::external_ack(const sequence_id& seq) {
  throw std::logic_error("Not implemented"); 
}

void l3_handler::external_ack_batch(const std::vector<sequence_id> & seqs) {
  throw std::logic_error("Not implemented");
}

int64_t l3_handler::get_client_id() {
  return client_id_gen_.fetch_add(1L);
}
