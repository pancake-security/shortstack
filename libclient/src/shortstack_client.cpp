
#include <random>
#include <iostream>

#include "shortstack_client.h"


#include <spdlog/spdlog.h>

void shortstack_client::init(int64_t client_id, std::shared_ptr<host_info> hosts) {
    client_id_ = client_id;

    done_.store(false);

    std::vector<host> l1_hosts;
  hosts->get_hosts_by_type(HOST_TYPE_L1, l1_hosts);

  std::vector<std::string> l1_hostnames;
  std::vector<int> l1_ports;
  std::vector<int> l1_workers;
  // Connect to L1 heads
  for (auto h : l1_hosts) {
    if(h.row != 0) {
        // TODO: Need to update these connection upon failure
        continue;
    }
    l1_hostnames.push_back(h.hostname);
    l1_ports.push_back(h.port);
    l1_workers.push_back(h.num_workers);
  }

  for(int i = 0; i < l1_hostnames.size(); i++) 
  {
    for(int j = 0; j < l1_workers[i]; j++)
     {
        auto socket = std::make_shared<TSocket>(l1_hostnames[i], l1_ports[i] + j);
        socket->setRecvTimeout(10000);
        socket->setSendTimeout(1200000);
        auto transport = std::shared_ptr<TTransport>(new TFramedTransport(socket));
        auto protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport));
        auto client = std::make_shared<pancake_thriftClient>(protocol);
        transport->open();

        l1_sockets_.push_back(socket);
        l1_transports_.push_back(transport);
        l1_protocols_.push_back(protocol);
        l1_clients_.push_back(client);
     }
  }

  std::vector<host> l3_hosts;
  hosts->get_hosts_by_type(HOST_TYPE_L3, l3_hosts);

  std::vector<std::string> l3_hostnames;
  std::vector<int> l3_ports;
  std::vector<int> l3_workers;
  for (auto h : l3_hosts) {
    l3_hostnames.push_back(h.hostname);
    l3_ports.push_back(h.port);
    l3_workers.push_back(h.num_workers);
  }

  for (int i = 0; i < l3_hostnames.size(); i++) {
    for(int j = 0; j < l3_workers[i]; j++)
     {
        auto socket = std::make_shared<TSocket>(l3_hostnames[i], l3_ports[i] + j);
        socket->setRecvTimeout(10000);
        socket->setSendTimeout(1200000);
        auto transport = std::shared_ptr<TTransport>(new TFramedTransport(socket));
        auto protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport));
        auto client = std::make_shared<l3proxyClient>(protocol);
        
        try{
            transport->open();
        } catch(TTransportException &e) {
            spdlog::error("Connection to {}:{} failed with error: {}", l3_hostnames[i], l3_ports[i] + j, e.what());
            continue;
        }
        

        l3_sockets_.push_back(socket);
        l3_transports_.push_back(transport);
        l3_protocols_.push_back(protocol);
        l3_clients_.push_back(client);
     }

    // Register with all L3 servers
    for(int i = 0; i < l3_clients_.size(); i++) {
        l3_clients_[i]->register_client_id(client_id_);
    }
  }

    response_queue_ = std::make_shared<queue<l3_response>>();
    for(int i = 0; i < l3_protocols_.size(); i++) 
    {
        readers_.push_back(command_response_reader(l3_protocols_[i]));
        
    }

    for(int i = 0; i < readers_.size(); i++) 
    {
        response_threads_.push_back(std::thread(&shortstack_client::response_thread, this, i));
    }

}

int64_t shortstack_client::get_client_id() {
    return client_id_;
}

int64_t shortstack_client::get(const std::string &key) {
    // Pick random L1 proxy
    int idx = rand_uint32(0, RAND_MAX) % l1_clients_.size();
    sequence_id seq;
    seq.client_id = client_id_;
    seq.client_seq_no = sequence_num_;
    sequence_num_ += 1;
    l1_clients_[idx]->async_get(seq, key);
    return seq.client_seq_no;
}

int64_t shortstack_client::put(const std::string &key, const std::string &value) {
    // Pick random L1 proxy
    int idx = rand_uint32(0, RAND_MAX) % l1_clients_.size();
    sequence_id seq;
    seq.client_id = client_id_;
    seq.client_seq_no = sequence_num_;
    sequence_num_ += 1;
    l1_clients_[idx]->async_put(seq, key, value);
    return seq.client_seq_no;
}

int64_t shortstack_client::poll_responses(std::string &out) {
    auto resp = response_queue_->pop();
    out = resp.value;
    return resp.sequence_num;
}

void shortstack_client::finish() {
    done_.store(true);
    sleep(5);
    for(int i = 0; i < response_threads_.size(); i++) 
    {
        response_threads_[i].join();
    }
}

void shortstack_client::response_thread(int idx) {
    std::vector<std::string> _return;
    while (!done_.load()) {
        l3_response resp;
        try {
            resp.sequence_num = readers_[idx].recv_response(_return);
        } catch(apache::thrift::transport::TTransportException e){
            std::cerr << e.what() << std::endl;
            continue;
        }

        resp.value = _return[0]; 
        response_queue_->push(resp);

        _return.clear();
    }
}





