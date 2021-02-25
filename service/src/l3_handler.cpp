#include "l3_handler.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

l3_handler::l3_handler(
    std::shared_ptr<l3_proxy> proxy,
    std::shared_ptr<::apache::thrift::protocol::TProtocol> prot,
    std::shared_ptr<thrift_response_client_map> &id_to_client)
    : prot_(std::move(prot)),
      client_(std::make_shared<thrift_response_client>(prot_)),
      registered_client_id_(-1), id_to_client_(id_to_client) {
  proxy_ = proxy;
}

void l3_handler::register_client_id(const int64_t client_id) {
  registered_client_id_ = client_id;
  id_to_client_->add_client(client_id, client_);
}

void l3_handler::l3request(const sequence_id &seq_id, const std::string &label,
                           const std::string &value, const bool is_read) {
  proxy_->async_operation(seq_id, label, value, is_read);
}
