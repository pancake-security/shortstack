#include "initializer.h"

#include <iostream>

void initializer::init(const distribution &real_dist, int object_size, std::shared_ptr<host_info> hosts) {
    object_size_ = object_size;
    label_count_ = 0;

    dist_info_ = std::make_shared<distribution_info>();
    dist_info_->real_distribution_ = real_dist;
    dist_info_->num_keys_ = dist_info_->real_distribution_.get_items().size();
    dist_info_->dummy_key_ = rand_str(16);

    cpp_redis::network::set_default_nb_workers(10);

    std::vector<host> kv_hosts;
    hosts->get_hosts_by_type(HOST_TYPE_KV, kv_hosts);
    storage_interface_ =
        std::make_shared<redis>(kv_hosts[0].hostname, kv_hosts[0].port);
    for (int j = 1; j < kv_hosts.size(); j++) {
        storage_interface_->add_server(kv_hosts[j].hostname, kv_hosts[j].port);
    }

    create_replicas();    
}

std::shared_ptr<distribution_info> initializer::get_distinfo() {
    return dist_info_;
}

void initializer::create_replicas() {
    int keys_created = 0;
    
    auto keys = dist_info_->real_distribution_.get_items();
    auto probabilities = dist_info_->real_distribution_.get_probabilities();

    alpha_ = 1.0 / keys.size();
    delta_ = 0.5;

    std::vector<double> fake_probabilities;
    int index = 0;
    for (auto key: keys) {
        double pi = probabilities[index];
        int r_i = ceil(pi/alpha_);
        double pi_f = r_i * (alpha_-pi/r_i)/(1/delta_ - 1);
        fake_probabilities.push_back(pi_f);
        dist_info_->key_to_number_of_replicas_[key] = r_i;
        // std::cout << "wrote to ktnr" << " " << r_i << std::endl;
        insert_replicas(key, r_i);
        keys_created += r_i;
        index++;
    }
    int dummy_r_i = 0 > 2*keys.size()-keys_created ? 0 : 2*keys.size()-keys_created;
    if(dummy_r_i != 0) {
        fake_probabilities.push_back((alpha_ / (1 / delta_ - 1)) * dummy_r_i);
        dist_info_->key_to_number_of_replicas_[dist_info_->dummy_key_] = dummy_r_i;
        insert_replicas(dist_info_->dummy_key_, dummy_r_i);
    }

    auto full_keys = keys;
    full_keys.push_back(dist_info_->dummy_key_);
    dist_info_->fake_distribution_ = distribution(full_keys, fake_probabilities);
}


void initializer::insert_replicas(const std::string &key, int num_replicas){
    std::string value_cipher = encryption_engine_.encrypt(rand_str(object_size_));
    std::vector<std::string> labels;
    for (int i = 0; i < num_replicas; i++){
        std::string replica = key+std::to_string(i);
        std::string replica_cipher = std::to_string(label_count_);
        dist_info_->replica_to_label_[replica] = label_count_;
        labels.push_back(std::to_string(label_count_));
        if (labels.size() >= 50){
            storage_interface_->put_batch(labels, std::vector<std::string>(labels.size(), value_cipher));
            labels.clear();
        }
        label_count_++;
    }
    if (!labels.empty()){
        storage_interface_->put_batch(labels, std::vector<std::string>(labels.size(), value_cipher));
    }
}


