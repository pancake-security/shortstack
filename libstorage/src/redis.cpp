//
// Created by Lloyd Brown on 9/2/19.
//

#include "redis.h"
#include "MurmurHash2.h"
#include <spdlog/spdlog.h>
// #include <iostream>

    redis::redis(const std::string &host_name, int port, int batch_size){
        storage_batch_size_ = batch_size;
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

    void redis::add_server(const std::string &host_name, int port){
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

    std::string redis::get(const std::string &key){
        auto idx = (MurmurHash64A(key.data(), key.length(), 1995) % clients.size());
        auto fut = clients[idx]->get(key);
        clients[idx]->commit();
        auto reply = fut.get();
        if (reply.is_error()){
            throw std::runtime_error(reply.error());
        }
        return reply.as_string();
    }

    void redis::put(const std::string &key, const std::string &value){
        auto idx = (MurmurHash64A(key.data(), key.length(), 1995) % clients.size());
        auto fut = clients[idx]->set(key, value);
        clients[idx]->commit();
        auto reply = fut.get();
        if (reply.is_error()){
            throw std::runtime_error(reply.error());
        }
    }

    std::vector<std::string> redis::get_batch(const std::vector<std::string> &keys){
        std::queue<std::future<cpp_redis::reply>> futures;
        std::unordered_map<int, std::vector<std::string>> key_vectors;

        // Gather all relevant storage interface's by id and create vector for key batch
        for (const auto &key: keys) {
            auto id = (MurmurHash64A(key.data(), key.length(), 1995) % clients.size());
            key_vectors[id].emplace_back(key);
        }

        for (auto it = key_vectors.begin(); it != key_vectors.end(); it++) {
            auto future = clients[it->first]->mget(it->second);
            futures.push(std::move(future));

        }
        // Issue requests to each storage server
        for (auto it = key_vectors.begin(); it != key_vectors.end(); it++)
            clients[it->first]->commit();

        std::vector< std::string> return_vector;

        // std::cout << "get_batch futures.size() == " << futures.size() << "\n";
        while (!futures.empty()) {
            auto reply = futures.front().get();
            futures.pop();
            if (reply.is_error()){
                throw std::runtime_error(reply.error());
            }
            if(reply.is_null()) {
                throw std::runtime_error("GET returned NULL value");
            }
            auto reply_array = reply.as_array();
            // std::cout << "reply_array.size() == " << reply_array.size() << "\n";
            for (auto nested_reply: reply_array){
                if (nested_reply.is_error()){
                    throw std::runtime_error(nested_reply.error());
                }
                if(nested_reply.is_null()) {
                    throw std::runtime_error("GET returned NULL value");
                }
                return_vector.push_back(nested_reply.as_string());
                // std::cout << "pushed to return_vector" << futures.size() << "\n";
            }
        }
        return return_vector;
    }

    void redis::put_batch(const std::vector<std::string> &keys, const std::vector<std::string> &values){
        std::queue<std::future<cpp_redis::reply>> futures;
        std::unordered_map<int, std::vector<std::pair<std::string, std::string>>> key_value_vector_pairs;

        // Gather all relevant storage interface's by id and create vector for key batch
        int i = 0;
        for (const auto &key: keys) {
            auto id = (MurmurHash64A(key.data(), key.length(), 1995) % clients.size());;
            key_value_vector_pairs[id].push_back(std::make_pair(key, values[i]));
            i++;
        }

        for (auto it = key_value_vector_pairs.begin(); it != key_value_vector_pairs.end(); it++) {
            auto future = clients[it->first]->mset(it->second);
            futures.push(std::move(future));
        }

        // Issue requests to each storage server
        for (auto it = key_value_vector_pairs.begin(); it != key_value_vector_pairs.end(); it++)
            clients[it->first]->commit();

        std::shared_ptr<std::vector< std::string>> return_vector;

        while (!futures.empty()){
            auto reply = futures.front().get();
            futures.pop();
            if (reply.is_error()){
                throw std::runtime_error(reply.error());
            }
        }
    }

    void redis::async_get(const std::string &key, std::function<void (const std::string&)> callback) {
        auto idx = (MurmurHash64A(key.data(), key.length(), 1995) % clients.size());

        get_queues_[idx].push(std::make_tuple(key, callback));

        if(get_queues_[idx].size() >= storage_batch_size_) {
            flush_get_queue(idx);       
        }

        
    }

    void redis::async_put(const std::string &key, const std::string &value, std::function<void ()> callback) {
        auto idx = (MurmurHash64A(key.data(), key.length(), 1995) % clients.size());

        put_queues_[idx].push(std::make_tuple(key, value, callback));
        
        if(put_queues_[idx].size() >= storage_batch_size_) {
            flush_put_queue(idx);       
        }
    }

    void redis::flush() {
        for(int i = 0; i < put_queues_.size(); i++) {
            flush_put_queue(i);
        }
        for(int i = 0; i < get_queues_.size(); i++) {
            flush_get_queue(i);
        }
    }

    void redis::flush_get_queue(int idx) {
        if(get_queues_[idx].empty()) {
            return;
        }

        spdlog::debug("Flushing get queue, size: {}", get_queues_[idx].size());

        std::vector<std::string> keys;
        std::vector<get_callback> callbacks;
        while(!get_queues_[idx].empty()) {
            auto &tup = get_queues_[idx].front();
            keys.push_back(std::get<0>(tup));
            callbacks.push_back(std::get<1>(tup));

            get_queues_[idx].pop();
        }

        clients[idx]->mget(keys, [callbacks](cpp_redis::reply& reply) {
            if (reply.is_error()){
                throw std::runtime_error(reply.error());
            }
            if(reply.is_null()) {
                throw std::runtime_error("GET returned NULL value");
            }
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
                callbacks[i](nested_reply.as_string());
            }
        });
        clients[idx]->commit();
    }

    void redis::flush_put_queue(int idx) {
        if(put_queues_[idx].empty()) {
            return;
        }

        spdlog::debug("Flushing put queue, size: {}", put_queues_[idx].size());

        std::vector<std::pair<std::string, std::string>> key_vals;
        std::vector<put_callback> callbacks;
        while(!put_queues_[idx].empty()) {
            auto &tup = put_queues_[idx].front();
            key_vals.push_back(std::make_pair(std::get<0>(tup), std::get<1>(tup)));
            callbacks.push_back(std::get<2>(tup));
            
            put_queues_[idx].pop();
        }

        // spdlog::debug("Calling redis MSET. ker_vals: {}", key_vals.size());
        clients[idx]->mset(key_vals, [callbacks](cpp_redis::reply& reply) {
            // spdlog::debug("Recvd redis PUT reply");
            if (reply.is_error()){
                throw std::runtime_error(reply.error());
            }
            // spdlog::debug("Recvd successful PUT reply");
            // std::cout << "reply_array.size() == " << reply_array.size() << "\n";
            for (int i = 0; i < callbacks.size(); i++){
                callbacks[i]();
            }
        });
        clients[idx]->commit();
    }

