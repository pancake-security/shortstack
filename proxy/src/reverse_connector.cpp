
#include <spdlog/spdlog.h>

#include "reverse_connector.h"

#include "MurmurHash2.h"
#include "consistent_hash.h"
#include "util.h"

reverse_connector::reverse_connector(std::shared_ptr<host_info> hinfo,
                                     int type,
                                     int batch_size) {
  hosts_ = hinfo;
  type_ = type;
  batch_size_= batch_size;

  int num_cols = hosts_->get_num_columns(type, false);

    for (int i = 0; i < num_cols; i++) {

      std::vector<host> replicas;
      hosts_->get_replicas(type, i, replicas);
      host h = replicas.back();

      sock_list sockets;
      transport_list transports;
      prot_list protocols;
      client_list clients;

      for(int j = 0; j < h.num_workers; j++) 
      {

        sockets.push_back(nullptr);
        transports.push_back(nullptr);
        protocols.push_back(nullptr);
        clients.push_back(nullptr);
      }

      sockets_.push_back(sockets);
      transports_.push_back(transports);
      protocols_.push_back(protocols);
      clients_.push_back(clients);
    
  }

  recompute_client_array();
}

void reverse_connector::recompute_client_array() {
    client_arr_.clear();

    ack_queues_.clear(); // We may end up dropping acks here upon failure

    for(int i = 0; i < clients_.size(); i++) 
    {
        for(int j = 0; j < clients_[i].size(); j++) 
        {
            client_arr_.push_back(clients_[i][j]);
            std::queue<sequence_id> q;
            ack_queues_.push_back(q);
        }
    }
}

void reverse_connector::send_ack(const sequence_id &seq) {
    int id = route(seq);
    if(!(id >= 0 && id < client_arr_.size())) {
        throw std::runtime_error("id out of bound");
    }
    if(client_arr_[id] == nullptr) {
        throw std::runtime_error("connection to id uninitialized");
    }

    ack_queues_[id].push(seq);

    if(ack_queues_[id].size() >= batch_size_) {
      spdlog::debug("Flushing ack queue, idx: {}", id);

      std::vector<sequence_id> batch;
      while(!ack_queues_[id].empty()) 
      {
          batch.push_back(ack_queues_[id].front());
          ack_queues_[id].pop();
      }

      client_arr_[id]->external_ack_batch(batch);
    }
}


void reverse_connector::update_connections(int column, std::string hostname, int port, int num_workers) {
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
    auto client = std::make_shared<block_request_serviceClient>(protocol);
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

  recompute_client_array();
}
