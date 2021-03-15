//
// Created by Lloyd Brown on 8/29/19.
//

#ifndef STORAGE_INTERFACE_H
#define STORAGE_INTERFACE_H
#include <string>
#include <vector>
#include <functional>

class storage_interface {
public:
    virtual std::string get(const std::string &key) = 0;
    virtual void put(const std::string &key, const std::string &value) = 0;
    virtual std::vector<std::string> get_batch(const std::vector<std::string> &keys) = 0;
    virtual void put_batch(const std::vector<std::string> &keys, const std::vector<std::string> &values) = 0;
    virtual void add_server(const std::string &host_name, int port) = 0;

    // Async operations
    virtual void async_get(const std::string &key, std::function<void (const std::string&)> callback) = 0;
    virtual void async_put(const std::string &key, const std::string &value, std::function<void ()> callback) = 0;
};
#endif //STORAGE_INTERFACE_H
