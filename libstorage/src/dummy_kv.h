
#ifndef dummy_kv_H
#define dummy_kv_H

#include "storage_interface.h"
#include <queue>
#include <unordered_map>

class dummy_kv : public storage_interface
{
public:
    dummy_kv(int val_size);
    void add_server(const std::string &host_name, int port) override;
    std::string get(const std::string &key) override;
    void put(const std::string &key, const std::string &value) override;
    std::vector< std::string> get_batch(const std::vector<std::string> &keys) override;
    void put_batch(const std::vector< std::string> &keys, const std::vector<std::string> &values) override;

    void async_get(const std::string &key, std::function<void (const std::string&)> callback) override;
    void async_put(const std::string &key, const std::string &value, std::function<void ()> callback) override;

private:
    int val_size_;
    std::string dummy_val_;
};

#endif //dummy_kv_H
