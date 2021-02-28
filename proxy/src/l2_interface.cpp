#include "l2_interface.h"

#include "consistent_hash.h"

l2proxy_interface::l2proxy_interface(std::vector<std::string> hosts,
                                     std::vector<int> ports) {
  hosts_ = hosts;
  ports_ = ports;
}

void l2proxy_interface::connect() {
    for (int i = 0; i < hosts_.size(); i++) {
    auto socket = std::make_shared<TSocket>(hosts_[i], ports_[i]);
    // TODO: Do we need these?
    // socket->setRecvTimeout(10000);
    // socket->setSendTimeout(1200000);
    auto transport = std::shared_ptr<TTransport>(new TFramedTransport(socket));
    auto protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport));
    auto client = std::make_shared<l2proxyClient>(protocol);
    transport->open();

    sockets_.push_back(socket);
    transports_.push_back(transport);
    protocols_.push_back(protocol);
    clients_.push_back(client);
  }
}

void l2proxy_interface::send_op(const l2_operation &op) {
    auto id = consistent_hash(op.key, clients_.size());
    clients_[id]->l2request(op.seq_id, op.key, op.replica, op.value);
}