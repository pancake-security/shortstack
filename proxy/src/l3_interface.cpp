#include "l3_interface.h"

#include "MurmurHash2.h"
#include "consistent_hash.h"

l3proxy_interface::l3proxy_interface(std::shared_ptr<host_info> hinfo) {
  hosts_ = hinfo;
}

void l3proxy_interface::connect() {
  int num_cols = hosts_->get_num_columns(HOST_TYPE_L3, false);

    for (int i = 0; i < num_cols; i++) {

      std::vector<host> replicas;
      hosts_->get_replicas(HOST_TYPE_L3, i, replicas);
      host h = replicas[0];

      sock_list sockets;
      transport_list transports;
      prot_list protocols;
      client_list clients;

      for(int j = 0; j < h.num_workers; j++) 
      {
        auto socket = std::make_shared<TSocket>(h.hostname, h.port + j);
        // TODO: Do we need these?
        // socket->setRecvTimeout(10000);
        // socket->setSendTimeout(1200000);
        auto transport = std::shared_ptr<TTransport>(new TFramedTransport(socket));
        auto protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport));
        auto client = std::make_shared<l3proxyClient>(protocol);
        transport->open();

        sockets.push_back(socket);
        transports.push_back(transport);
        protocols.push_back(protocol);
        clients.push_back(client);
      }

      sockets_.push_back(sockets);
      transports_.push_back(transports);
      protocols_.push_back(protocols);
      clients_.push_back(clients);
    
  }
}

void l3proxy_interface::send_op(const l3_operation &op) {
  // Pick L3 server based on consistent hash of label
  auto id = consistent_hash(op.label, clients_.size());

  // Pick worker within L3 based on hash of label
  int wid;
  wid = MurmurHash64A(op.label.data(), op.label.length(), 1995) % clients_[id].size();
  clients_[id][wid]->l3request(op.seq_id, op.label, op.value, op.is_read, op.dedup);
}

// Remove connection at given column
void l3proxy_interface::remove_connection(int column) {
  if(!(column >= 0 && column < clients_.size())) {
    throw std::runtime_error("remove_connection column out of range");
  }

  clients_.erase(clients_.begin() + column);
  protocols_.erase(protocols_.begin() + column);
  transports_.erase(transports_.begin() + column);
  sockets_.erase(sockets_.begin() + column);

  spdlog::info("Removed L3 connection, column: {}, clients_.size = {}", column, clients_.size());

}