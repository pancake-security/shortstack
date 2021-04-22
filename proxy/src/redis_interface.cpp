
#include "redis_interface.h"
#include "consistent_hash.h"
#include <spdlog/spdlog.h>
// #include <iostream>

    redis_interface::redis_interface(const std::string &host_name, int port, int batch_size, get_callback get_cb, put_callback put_cb){
        storage_batch_size_ = batch_size;
        get_cb_ = get_cb;
        put_cb_ = put_cb;
        this->clients.push_back(std::move(std::make_shared<cpp_redis::client>()));
        this->clients.back()->connect(host_name, port,
                [](const std::string &host, std::size_t port, cpp_redis::client::connect_state status) {
                    if (status == cpp_redis::client::connect_state::dropped || status == cpp_redis::client::connect_state::failed || status == cpp_redis::client::connect_state::lookup_failed){
                        std::cerr << "Redis client disconnected from " << host << ":" << port << std::endl;
                        exit(-1);
                    }
                });
        std::queue<get_request> get_q;
        std::queue<put_request> put_q;
        get_queues_.push_back(get_q);
        put_queues_.push_back(put_q);
    }

    void redis_interface::add_server(const std::string &host_name, int port){
        this->clients.push_back(std::move(std::make_shared<cpp_redis::client>()));
        this->clients.back()->connect(host_name, port,
                                  [](const std::string &host, std::size_t port, cpp_redis::client::connect_state status) {
                                      if (status == cpp_redis::client::connect_state::dropped || status == cpp_redis::client::connect_state::failed || status == cpp_redis::client::connect_state::lookup_failed){
                                          std::cerr << "Redis client disconnected from " << host << ":" << port << std::endl;
                                          exit(-1);
                                      }
                                  });

        std::queue<get_request> get_q;
        std::queue<put_request> put_q;
        get_queues_.push_back(get_q);
        put_queues_.push_back(put_q);
    }

    void redis_interface::async_get(const std::string &key, const l3_operation &op) {
        // auto idx = (MurmurHash64A(key.data(), key.length(), 1995) % clients.size());
        auto idx = consistent_hash(key, clients.size());

        get_queues_[idx].push(std::make_tuple(key, op));

        if(get_queues_[idx].size() >= storage_batch_size_) {
            flush_get_queue(idx);       
        }

        
    }

    void redis_interface::async_put(const std::string &key, const std::string &value, const l3_operation &op) {
        // auto idx = (MurmurHash64A(key.data(), key.length(), 1995) % clients.size());
        auto idx = consistent_hash(key, clients.size());

        put_queues_[idx].push(std::make_tuple(key, value, op));
        
        if(put_queues_[idx].size() >= storage_batch_size_) {
            flush_put_queue(idx);       
        }
    }

    void redis_interface::flush() {
        for(int i = 0; i < put_queues_.size(); i++) {
            flush_put_queue(i);
        }
        for(int i = 0; i < get_queues_.size(); i++) {
            flush_get_queue(i);
        }
    }

    void redis_interface::flush_get_queue(int idx) {
        if(get_queues_[idx].empty()) {
            return;
        }

        spdlog::debug("Flushing get queue, size: {}", get_queues_[idx].size());

        std::vector<std::string> keys;
        std::vector<l3_operation> ops;
        while(!get_queues_[idx].empty()) {
            auto &tup = get_queues_[idx].front();
            keys.push_back(std::get<0>(tup));
            ops.push_back(std::get<1>(tup));

            get_queues_[idx].pop();
        }

        auto callback = get_cb_;
        clients[idx]->mget(keys, [ops, callback](cpp_redis::reply& reply) {
            if (reply.is_error()){
                throw std::runtime_error(reply.error());
            }
            if(reply.is_null()) {
                throw std::runtime_error("GET returned NULL value");
            }
            std::vector<std::string> vals;
            auto reply_array = reply.as_array();
            // std::cout << "reply_array.size() == " << reply_array.size() << "\n";
            for (int i = 0; i < reply_array.size(); i++){
                auto &nested_reply = reply_array[i];
                if (nested_reply.is_error()){
                    throw std::runtime_error(nested_reply.error());
                }
                if(nested_reply.is_null()) {
                    throw std::runtime_error("GET returned NULL value");
                }
                vals.push_back(nested_reply.as_string());
            }

            callback(ops, vals);
        });
        clients[idx]->commit();
    }

    void redis_interface::flush_put_queue(int idx) {
        if(put_queues_[idx].empty()) {
            return;
        }

        spdlog::debug("Flushing put queue, size: {}", put_queues_[idx].size());

        std::vector<std::pair<std::string, std::string>> key_vals;
        std::vector<l3_operation> ops;
        while(!put_queues_[idx].empty()) {
            auto &tup = put_queues_[idx].front();
            key_vals.push_back(std::make_pair(std::get<0>(tup), std::get<1>(tup)));
            ops.push_back(std::get<2>(tup));
            
            put_queues_[idx].pop();
        }

        auto callback = put_cb_;
        // spdlog::debug("Calling redis MSET. ker_vals: {}", key_vals.size());
        clients[idx]->mset(key_vals, [ops, callback](cpp_redis::reply& reply) {
            // spdlog::debug("Recvd redis PUT reply");
            if (reply.is_error()){
                throw std::runtime_error(reply.error());
            }
            // spdlog::debug("Recvd successful PUT reply");
            // std::cout << "reply_array.size() == " << reply_array.size() << "\n";
            callback(ops);
        });
        clients[idx]->commit();
    }

