#include <algorithm>
#include <iterator>
#include <chrono>
#include <future>
#include <spdlog/spdlog.h>

#include "proxy_manager.h"
#include "block_id_parser.h"

template<typename Duration = std::chrono::microseconds,
         typename F,
         typename ... Args>
typename Duration::rep profile(F&& fun,  Args&&... args) {
  const auto beg = std::chrono::high_resolution_clock::now();
  std::forward<F>(fun)(std::forward<Args>(args)...);
  const auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<Duration>(end - beg).count();
}

void proxy_manager::init(std::shared_ptr<host_info> hosts) {
    hosts_ = hosts;

    // Pre-create connections to all cores
    std::vector<host> all_hosts;
    hosts_->get_hosts_by_type(-1, all_hosts);
    for(auto &h : all_hosts) {
        if(h.type == HOST_TYPE_L1 || h.type == HOST_TYPE_L2 || h.type == HOST_TYPE_L3) {
            for(int i = 0; i < h.num_workers; i++) {
                get_block_client(h.hostname, h.port + i);
            }
        }
    }

    spdlog::info("Pre-created connections");
}

void proxy_manager::setup_reverse_connections() {
    int num_cols_l3 = hosts_->get_num_columns(HOST_TYPE_L3, false);
    int num_cols_l2 = hosts_->get_num_columns(HOST_TYPE_L2, false);
    int num_cols_l1 = hosts_->get_num_columns(HOST_TYPE_L1, false);

    std::vector<host> l3_hosts;
    std::vector<host> l2_hosts;
    std::vector<host> l2_heads;
    std::vector<host> l1_tails;
    for(int i = 0; i < num_cols_l3; i++) {
        std::vector<host> replicas;
        hosts_->get_replicas(HOST_TYPE_L3, i, replicas);
        l3_hosts.push_back(replicas.front());
    }
    for(int i = 0; i < num_cols_l2; i++) {
        std::vector<host> replicas;
        hosts_->get_replicas(HOST_TYPE_L2, i, replicas);
        l2_hosts.push_back(replicas.back());
        l2_heads.push_back(replicas.front());
    }
    for(int i = 0; i < num_cols_l1; i++) {
        std::vector<host> replicas;
        hosts_->get_replicas(HOST_TYPE_L1, i, replicas);
        l1_tails.push_back(replicas.back());
    }

    for(int i = 0; i < l3_hosts.size(); i++) 
    {
        for(int j = 0; j < l2_hosts.size(); j++) {
            update_connections(&l3_hosts[i], HOST_TYPE_L2, j, &l2_hosts[j]);
        }
        spdlog::info("Setup reverse connections for L3: {}", l3_hosts[i].instance_name);
    }

    for(int i = 0; i < l2_heads.size(); i++) 
    {
        for(int j = 0; j < l1_tails.size(); j++) {
            update_connections(&l2_heads[i], HOST_TYPE_L1, j, &l1_tails[j]);
        }
        spdlog::info("Setup reverse connections for L2: {}", l2_heads[i].instance_name);
    }

    
}

void proxy_manager::fail_host(std::string hostname) {
    std::vector<host> all_hosts;
    hosts_->get_hosts_by_type(-1, all_hosts);

    for(auto &h : all_hosts) {
        if(h.hostname == hostname) {
            fail_node(h.instance_name, 5);
        }
    }
}

void proxy_manager::crash_host(host *h) {
    
    auto client = get_block_client(h->hostname, h->port);
    
    // Send kill code
    sequence_id seq;
    seq.client_id = -1995;
    seq.client_seq_no = -1995;
    client->external_ack(seq);

}

void proxy_manager::fail_node(std::string instance_name, int delay_sec) {
    host failed_host;
    if(!hosts_->get_host(instance_name, failed_host)) {
        throw std::runtime_error("Invalid instance name");
    }

    sleep(delay_sec);

    // Fail the node
    crash_host(&failed_host);

    if(failed_host.type == HOST_TYPE_L1) {
        std::vector<host> replicas;
        hosts_->get_replicas(HOST_TYPE_L1, failed_host.column, replicas);
              
        std::vector<host> fixed_replicas;
        std::copy_if(replicas.begin(), replicas.end(), std::back_inserter(fixed_replicas), [&failed_host](const host &elem){ return elem.instance_name != failed_host.instance_name;});

        int idx = get_idx(failed_host, replicas);
        assert(idx != -1);
        if(idx > 0) {
            host prev = replicas[idx - 1];
            int prev_idx = get_idx(prev, fixed_replicas);

            chain_role role;
            if(fixed_replicas.size() == 1) {
                role = chain_role::singleton;
            } else {
                role = (prev_idx == 0) ? chain_role::head : (prev_idx == fixed_replicas.size() - 1) ? chain_role::tail : chain_role::mid;
            }

            host *next_replica = (prev_idx == fixed_replicas.size() - 1) ? nullptr : &fixed_replicas[prev_idx + 1];
            spdlog::info("setup chain on {}", prev.instance_name);
            setup_chain(&prev, "/", role, next_replica);
            spdlog::info("resend pending on {}", prev.instance_name);
            resend_pending(&prev, next_replica);
        }

    } else if(failed_host.type == HOST_TYPE_L2) {
        std::vector<host> replicas;
        hosts_->get_replicas(HOST_TYPE_L2, failed_host.column, replicas);
              
        std::vector<host> fixed_replicas;
        std::copy_if(replicas.begin(), replicas.end(), std::back_inserter(fixed_replicas), [&failed_host](const host &elem){ return elem.instance_name != failed_host.instance_name;});

        int idx = get_idx(failed_host, replicas);
        assert(idx != -1);
        if(idx > 0) {
            host prev = replicas[idx - 1];
            int prev_idx = get_idx(prev, fixed_replicas);

            chain_role role;
            if(fixed_replicas.size() == 1) {
                role = chain_role::singleton;
            } else {
                role = (prev_idx == 0) ? chain_role::head : (prev_idx == fixed_replicas.size() - 1) ? chain_role::tail : chain_role::mid;
            }

            host *next_replica = (prev_idx == fixed_replicas.size() - 1) ? nullptr : &fixed_replicas[prev_idx + 1];

            auto start_ts = std::chrono::high_resolution_clock::now();
            setup_chain(&prev, "/", role, next_replica);
            auto end_ts = std::chrono::high_resolution_clock::now();
            long long elapsed_tim = std::chrono::duration_cast<std::chrono::microseconds>(end_ts - start_ts).count();
            spdlog::info("setup chain on {}, dur: {}", prev.instance_name, elapsed_tim);

            start_ts = std::chrono::high_resolution_clock::now();
            resend_pending(&prev, next_replica);
            end_ts = std::chrono::high_resolution_clock::now();
            elapsed_tim = std::chrono::duration_cast<std::chrono::microseconds>(end_ts - start_ts).count();
            spdlog::info("resend pending on {}, dur: {}", prev.instance_name, elapsed_tim);

        } else {
            // Head failure
            assert(fixed_replicas.size() >= 1);
            host new_head = fixed_replicas[0];
            chain_role role = (fixed_replicas.size() == 1)?(chain_role::singleton):(chain_role::head);
            host *next_replica = (fixed_replicas.size() == 1) ? nullptr : &fixed_replicas[1];
            spdlog::info("setup chain on {}", new_head.instance_name);
            setup_chain(&new_head, "/", role, next_replica);
            
            // Update connections & resend pending at L1 tails
            int num_cols = hosts_->get_num_columns(HOST_TYPE_L1, false);
            for(int i = 0; i < num_cols; i++) 
            {
                std::vector<host> replicas;
                hosts_->get_replicas(HOST_TYPE_L1, i, replicas);
                host l1_tail = replicas.back();
                update_connections(&l1_tail, HOST_TYPE_L2, new_head.column, &new_head);
            }
            for(int i = 0; i < num_cols; i++) 
            {
                std::vector<host> replicas;
                hosts_->get_replicas(HOST_TYPE_L1, i, replicas);
                host l1_tail = replicas.back();
                // TODO: needs to be updated
                resend_pending(&l1_tail, nullptr);
            }

        }
    } else if(failed_host.type == HOST_TYPE_L3) {
        int num_l3_cols = hosts_->get_num_columns(HOST_TYPE_L3, false);

        // Update connections at all L2 tails
        int num_l2_cols = hosts_->get_num_columns(HOST_TYPE_L2, false);
        for(int i = 0; i < num_l2_cols; i++) 
        {
            std::vector<host> replicas;
            hosts_->get_replicas(HOST_TYPE_L2, i, replicas);
            host l2_tail = replicas.back();
            update_connections(&l2_tail, HOST_TYPE_L3, failed_host.column, nullptr);
            spdlog::info("Update L3 connections at {}", l2_tail.instance_name);
        }

        // Selectively resend pending requests from L2 tails
        for(int i = 0; i < num_l2_cols; i++) 
        {
            std::vector<host> replicas;
            hosts_->get_replicas(HOST_TYPE_L2, i, replicas);
            host l2_tail = replicas.back();
            selective_resend_pending(&l2_tail, failed_host.column, num_l3_cols);
            spdlog::info("Selectively resend pending requests at {}", l2_tail.instance_name);
        }

    } else {
        throw std::logic_error("Not implemented");
    }
}

int proxy_manager::get_idx(const host &h, const std::vector<host> &replicas) {
    auto it = std::find_if(replicas.begin(), replicas.end(), [&h](const host &elem){return elem.instance_name == h.instance_name;});
    if(it == replicas.end()) {
        return -1;
    }
    return std::distance(replicas.begin(), it);
}

void proxy_manager::setup_chain(host *h, std::string path, chain_role role, host *next) {
    std::vector<std::thread> threads;
    for(int i = 0; i < h->num_workers; i++) {
        threads.push_back(std::thread([=]() {
            auto client = this->get_block_client(h->hostname, h->port + i);
        
            // auto socket = std::make_shared<TSocket>(h->hostname, h->port + i);
            // auto transport = std::shared_ptr<TTransport>(new TFramedTransport(socket));
            // auto protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport));
            // auto client = std::make_shared<block_request_serviceClient>(protocol);
            // transport->open();

            std::vector<std::string> dummy_chain;
            std::string next_block_id = (next == nullptr)?("nil"):(block_id_parser::make(next->hostname, next->port + i, next->port + i, i));
            client->setup_chain(i, path, dummy_chain, role, next_block_id);

            // transport->close();
        }));
    }

    for(auto &t : threads) {
        t.join();
    }
}

void proxy_manager::resend_pending(host *h, host *next) {

    std::vector<int64_t> cur_seqs;
    for(int i = 0; i < h->num_workers; i++) {
        cur_seqs.push_back(-1);
    } 
    
    if(next != nullptr) {
        // Fetch current sequence number of successor
        std::vector<std::future<int64_t>> seqs;
        for(int i = 0; i < h->num_workers; i++) {
            seqs.push_back(std::async(std::launch::async, [=](){
                auto client = this->get_block_client(next->hostname, next->port + i);
                //  auto socket = std::make_shared<TSocket>(next->hostname, next->port + i);
                // auto transport = std::shared_ptr<TTransport>(new TFramedTransport(socket));
                // auto protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport));
                // auto client = std::make_shared<block_request_serviceClient>(protocol);
                // transport->open();

                int64_t res = client->fetch_seq(i);
                
                // transport->close();

                return res; 
            }));
        } 

        for(int i = 0; i < h->num_workers; i++) {
            cur_seqs[i] = seqs[i].get();
        }

        spdlog::info("Fetched successor sequence numbers");
    }

    
    std::vector<std::thread> threads;
    for(int i = 0; i < h->num_workers; i++) {
        threads.push_back(std::thread([=]() {
            auto client = this->get_block_client(h->hostname, h->port + i);
            // auto socket = std::make_shared<TSocket>(h->hostname, h->port + i);
            // auto transport = std::shared_ptr<TTransport>(new TFramedTransport(socket));
            // auto protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport));
            // auto client = std::make_shared<block_request_serviceClient>(protocol);
            // transport->open();

            // std::vector<std::string> dummy_chain;
            client->resend_pending(i, cur_seqs[i]);

            // transport->close();
        }));
    }

    for(auto &t : threads) {
        t.join();
    }
}

void proxy_manager::update_connections(host *h, int type, int column, host *target) {
    for(int i = 0; i < h->num_workers; i++) {
        auto socket = std::make_shared<TSocket>(h->hostname, h->port + i);
        auto transport = std::shared_ptr<TTransport>(new TFramedTransport(socket));
        auto protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport));
        auto client = std::make_shared<block_request_serviceClient>(protocol);
        transport->open();

        client->update_connections(type, column, (target == nullptr)?("nil"):(target->hostname), (target == nullptr)?(0):(target->port), (target == nullptr)?(0):(target->num_workers));

        transport->close();
    }
}

void proxy_manager::selective_resend_pending(host *h, int column, int num_columns) {
    if(h->type != HOST_TYPE_L2) {
        throw std::logic_error("selective_resend_pending call in invalid node type");
    }

    for(int i = 0; i < h->num_workers; i++) {
        auto socket = std::make_shared<TSocket>(h->hostname, h->port + i);
        auto transport = std::shared_ptr<TTransport>(new TFramedTransport(socket));
        auto protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport));
        auto client = std::make_shared<l2proxyClient>(protocol);
        transport->open();

        client->selective_resend_pending(column, num_columns);

        transport->close();
    }
}

std::shared_ptr<block_request_serviceClient> proxy_manager::get_block_client(std::string hostname, int port) {
    if(block_client_cache_.find(std::make_pair(hostname, port)) == block_client_cache_.end()) {
        auto socket = std::make_shared<TSocket>(hostname, port);
        auto transport = std::shared_ptr<TTransport>(new TFramedTransport(socket));
        auto protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport));
        auto client = std::make_shared<block_request_serviceClient>(protocol);
        transport->open();

        block_srv_client c;
        c.socket = socket;
        c.transport = transport;
        c.protocol = protocol;
        c.client = client;
        block_client_cache_[std::make_pair(hostname, port)] = c;

        spdlog::info("Created connection to {}:{}", hostname, port);
    }

    return block_client_cache_[std::make_pair(hostname, port)].client;
}
 


