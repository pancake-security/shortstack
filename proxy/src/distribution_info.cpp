
#include "dist_info_types.h"
#include <thrift/transport/TSimpleFileTransport.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include "distribution_info.h"

using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

void distribution_info::load(std::string filename) {
    auto trans = std::make_shared<TSimpleFileTransport>(filename, true, false);
    auto proto = std::make_shared<TBinaryProtocol>(trans);
    trans->open();
    dist_info dinfo;
    dinfo.read(proto.get());
    trans->close();

    num_keys_ = dinfo.num_keys;
    dummy_key_ = dinfo.dummy_key;
    
    for(auto &it : dinfo.key_to_number_of_replicas) {
        key_to_number_of_replicas_[it.first] = it.second;
    }
    for(auto &it : dinfo.replica_to_label) {
        replica_to_label_[it.first] = it.second;
    }

    {
        std::vector<std::string> items;
        std::vector<double> probs;
        for(auto &it : dinfo.fake_distribution) {
            items.push_back(it.first);
            probs.push_back(it.second);
        }
        distribution fake_dist(items, probs);
        fake_distribution_ = fake_dist;
    }

    {
        std::vector<std::string> items;
        std::vector<double> probs;
        for(auto &it : dinfo.real_distribution) {
            items.push_back(it.first);
            probs.push_back(it.second);
        }
        distribution real_dist(items, probs);
        real_distribution_ = real_dist;
    }
}

void distribution_info::dump(std::string filename) {
    auto trans = std::make_shared<TSimpleFileTransport>(filename, false, true);
    auto proto = std::make_shared<TBinaryProtocol>(trans);
    trans->open();
    
    dist_info dinfo;
    dinfo.num_keys = num_keys_;
    dinfo.dummy_key = dummy_key_;
    for(auto &it : key_to_number_of_replicas_) {
        dinfo.key_to_number_of_replicas[it.first] = it.second;
    }
    for(auto &it : replica_to_label_) {
        dinfo.replica_to_label[it.first] = it.second;
    }

    auto items = fake_distribution_.get_items();
    auto probs = fake_distribution_.get_probabilities();
    for(int i = 0; i < items.size(); i++) {
        dinfo.fake_distribution[items[i]] = probs[i];
    }

    items = real_distribution_.get_items();
    probs = real_distribution_.get_probabilities();
    for(int i = 0; i < items.size(); i++) {
        dinfo.real_distribution[items[i]] = probs[i];
    }

    dinfo.write(proto.get());

    trans->flush();
    trans->close();
}