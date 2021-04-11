#include "l2_interface.h"

#include "consistent_hash.h"
#include "util.h"

l2proxy_interface::l2proxy_interface(std::vector<std::string> hosts,
                                     std::vector<int> ports,
                                     std::string dummy_key) {
  hosts_ = hosts;
  ports_ = ports;
  dummy_key_ = dummy_key;
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
    int id;
    if(op.key == dummy_key_) {
      // Randomly load balance requests to dummy key
      id = rand_uint32(0, clients_.size() - 1);
    } else {
      id = consistent_hash(op.key, clients_.size());
    }
    
    clients_[id]->l2request(op.seq_id, op.key, op.replica, op.value);
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