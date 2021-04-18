#ifndef REDIS_INTERFACE_H
#define REDIS_INTERFACE_H

#include <queue>
#include <unordered_map>
#include <tuple>
#include <cpp_redis/cpp_redis>

#include "l3_interface.h"

class redis_interface {
public:
    typedef std::function<void (const std::vector<l3_operation> &, const std::vector<std::string>&)> get_callback;
    typedef std::function<void (const std::vector<l3_operation> &)> put_callback;
    typedef std::tuple<std::string, l3_operation> get_request;
    typedef std::tuple<std::string, std::string, l3_operation> put_request;


    redis_interface(const std::string &host_name, int port, int batch_size, get_callback get_cb, put_callback put_cb);
    void add_server(const std::string &host_name, int port);



    void async_get(const std::string &key, const l3_operation &op);
    void async_put(const std::string &key, const std::string &value, const l3_operation &op);

    void flush();

private:
    void flush_get_queue(int idx);
    void flush_put_queue(int idx);

    std::vector<std::shared_ptr<cpp_redis::client>> clients;

    std::vector<std::queue<get_request>> get_queues_;

    std::vector<std::queue<put_request>> put_queues_;
 
    get_callback get_cb_;
    put_callback put_cb_;


    int storage_batch_size_;
};

#endif //REDIS_INTERFACE_H
