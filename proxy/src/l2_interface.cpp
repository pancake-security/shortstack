#include "l2_interface.h"

#include "MurmurHash2.h"
#include "consistent_hash.h"
#include "util.h"

l2proxy_interface::l2proxy_interface(std::shared_ptr<host_info> hinfo,
                                     std::string dummy_key) {
  hosts_ = hinfo;
  dummy_key_ = dummy_key;
}

void l2proxy_interface::connect() {

  int num_cols = hosts_->get_num_columns(HOST_TYPE_L2, false);

    for (int i = 0; i < num_cols; i++) {

      std::vector<host> replicas;
      hosts_->get_replicas(HOST_TYPE_L2, i, replicas);
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
        auto client = std::make_shared<l2proxyClient>(protocol);
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

void l2proxy_interface::send_op(const l2_operation &op) {
    // pick L2 server based on hash of key
    int id;
    if(op.key == dummy_key_) {
      // Randomly load balance requests to dummy key
      id = rand_uint32(0, clients_.size() - 1);
    } else {
      // TODO: We don't really need consistent hashing here
      id = consistent_hash(op.key, clients_.size());
    }

    // pick worker within L2 server based on hash of label
    int wid;
    std::string label = op.key + std::to_string(op.replica);
    wid = MurmurHash64A(label.data(), label.length(), 1995) % clients_[id].size();

    clients_[id][wid]->l2request(op.seq_id, op.key, op.replica, op.value);
}

void l2proxy_interface::update_connections(int column, std::string hostname, int port, int num_workers) {
  sock_list sockets;
  transport_list transports;
  prot_list protocols;
  client_list clients;

  for(int j = 0; j < num_workers; j++) 
  {
    auto socket = std::make_shared<TSocket>(hostname, port + j);
    // TODO: Do we need these?
    // socket->setRecvTimeout(10000);
    // socket->setSendTimeout(1200000);
    auto transport = std::shared_ptr<TTransport>(new TFramedTransport(socket));
    auto protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport));
    auto client = std::make_shared<l2proxyClient>(protocol);
    transport->open();

    sockets.push_back(socket);
    transports.push_back(transport);
    protocols.push_back(protocol);
    clients.push_back(client);
  }

  sockets_[column] = sockets;
  transports_[column] = transports;
  protocols_[column] = protocols;
  clients_[column] = clients;
}

// <client_id, client_seq_no, key, replica, value>
void l2_operation::serialize(std::vector<std::string> &args) {
  args.push_back(std::to_string(seq_id.client_id));
  args.push_back(std::to_string(seq_id.client_seq_no));
  args.push_back(key);
  args.push_back(std::to_string(replica));
  args.push_back(value);
}


int l2_operation::deserialize(const std::vector<std::string> & args, int idx) {
  if((int)args.size() - idx < 5) {
    throw std::logic_error("l2_operation deserialize failed");
  }

  int start = idx;
  seq_id.client_id = std::stoi(args[idx++]);
  seq_id.client_seq_no = std::stoi(args[idx++]);
  key = args[idx++];
  replica = std::stoi(args[idx++]);
  value = args[idx++];

  return idx - start;
}