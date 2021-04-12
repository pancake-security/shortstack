#include <algorithm>
#include <iterator>
#include <spdlog/spdlog.h>

#include "proxy_manager.h"
#include "block_id_parser.h"

void proxy_manager::init(std::shared_ptr<host_info> hosts) {
    hosts_ = hosts;
}

void proxy_manager::fail_node(std::string instance_name) {
    host failed_host;
    if(!hosts_->get_host(instance_name, failed_host)) {
        throw std::runtime_error("Invalid instance name");
    }

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
            resend_pending(&prev);
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
    for(int i = 0; i < h->num_workers; i++) {
        auto socket = std::make_shared<TSocket>(h->hostname, h->port + i);
        auto transport = std::shared_ptr<TTransport>(new TFramedTransport(socket));
        auto protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport));
        auto client = std::make_shared<block_request_serviceClient>(protocol);
        transport->open();

        std::vector<std::string> dummy_chain;
        std::string next_block_id = (next == nullptr)?("nil"):(block_id_parser::make(next->hostname, next->port + i, next->port + i, i));
        client->setup_chain(i, path, dummy_chain, role, next_block_id);

        transport->close();
    }
}

void proxy_manager::resend_pending(host *h) {
    for(int i = 0; i < h->num_workers; i++) {
        auto socket = std::make_shared<TSocket>(h->hostname, h->port + i);
        auto transport = std::shared_ptr<TTransport>(new TFramedTransport(socket));
        auto protocol = std::shared_ptr<TProtocol>(new TBinaryProtocol(transport));
        auto client = std::make_shared<block_request_serviceClient>(protocol);
        transport->open();

        std::vector<std::string> dummy_chain;
        client->resend_pending(i);

        transport->close();
    }
}
 


