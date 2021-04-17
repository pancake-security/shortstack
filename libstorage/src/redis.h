//
// Created by Lloyd Brown on 8/29/19.
//

#ifndef REDIS_H
#define REDIS_H

#include "storage_interface.h"
#include <queue>
#include <unordered_map>
#include <tuple>
#include <cpp_redis/cpp_redis>

class redis : public storage_interface
{
public:
    typedef std::function<void (const std::string&)> get_callback;
    typedef std::function<void ()> put_callback;
    typedef std::tuple<std::string, get_callback> get_request;
    typedef std::tuple<std::string, std::string, put_callback> put_request;


    redis(const std::string &host_name, int port, int batch_size=1);
    void add_server(const std::string &host_name, int port) override;
    std::string get(const std::string &key) override;
    void put(const std::string &key, const std::string &value) override;
    std::vector< std::string> get_batch(const std::vector<std::string> &keys) override;
    void put_batch(const std::vector< std::string> &keys, const std::vector<std::string> &values) override;

    void async_get(const std::string &key, std::function<void (const std::string&)> callback) override;
    void async_put(const std::string &key, const std::string &value, std::function<void ()> callback) override;

    void flush() override;

private:
    void flush_get_queue(int idx);
    void flush_put_queue(int idx);

    std::vector<std::shared_ptr<cpp_redis::client>> clients;

    std::vector<std::queue<get_request>> get_queues_;

    std::vector<std::queue<put_request>> put_queues_;


    int storage_batch_size_;
};

#endif //REDIS_H
