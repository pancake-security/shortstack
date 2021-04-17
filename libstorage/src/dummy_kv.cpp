#include "dummy_kv.h"

dummy_kv::dummy_kv(int val_size)
{
    val_size_ = val_size;
    dummy_val_ = "";
    for(int i = 0; i < val_size; i++) 
    {
        dummy_val_ += "0";
    }
}

void dummy_kv::add_server(const std::string &host_name, int port) {
    
}

std::string dummy_kv::get(const std::string &key) {
    return dummy_val_;
}

void dummy_kv::put(const std::string &key, const std::string &value) {
    
}

std::vector< std::string> dummy_kv::get_batch(const std::vector<std::string> &keys) {
    std::vector<std::string> res;
    for(auto key : keys) {
        res.push_back(dummy_val_);
    }
    return res;
}

void dummy_kv::put_batch(const std::vector< std::string> &keys, const std::vector<std::string> &values) {
    
}

void dummy_kv::async_get(const std::string &key, std::function<void (const std::string&)> callback) {
    callback(dummy_val_);
}

void dummy_kv::async_put(const std::string &key, const std::string &value, std::function<void ()> callback) {
    callback();
}

void dummy_kv::flush() {
    // Do nothing
}
