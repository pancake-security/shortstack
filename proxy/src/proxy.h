//
// Created by Lloyd Brown on 8/29/19.
//

#ifndef PROXY_H
#define PROXY_H

#include <string>
#include <vector>
#include "proxy_types.h"

class proxy {
public:

    virtual void init(const std::vector<std::string> &keys, const std::vector<std::string> &values, void ** args) = 0;
    virtual void close() = 0;
    virtual std::string get(const std::string &key) = 0;
    virtual void put(const std::string &key, const std::string &value) = 0;
    virtual std::vector<std::string> get_batch(const std::vector<std::string> &keys) = 0;
    virtual void put_batch(const std::vector<std::string> &keys, const std::vector<std::string> &values) = 0;
    virtual std::string get(int queue_id, const std::string &key) = 0;
    virtual void put(int queue_id, const std::string &key, const std::string &value) = 0;
    virtual std::vector<std::string> get_batch(int queue_id, const std::vector<std::string> &keys) = 0;
    virtual void put_batch(int queue_id, const std::vector<std::string> &keys, const std::vector<std::string> &values) = 0;

    virtual void async_get(const sequence_id &seq_id, const std::string &key) = 0;
    virtual void async_put(const sequence_id &seq_id, const std::string &key, const std::string &value) = 0;
    virtual void async_get_batch(const sequence_id &seq_id, const std::vector<std::string> &keys) = 0;
    virtual void async_put_batch(const sequence_id &seq_id, const std::vector<std::string> &keys, const std::vector<std::string> &values) = 0;
    virtual void async_get(const sequence_id &seq_id, int queue_id, const std::string &key) = 0;
    virtual void async_put(const sequence_id &seq_id, int queue_id, const std::string &key, const std::string &value) = 0;
    virtual void async_get_batch(const sequence_id &seq_id, int queue_id, const std::vector<std::string> &keys) = 0;
    virtual void async_put_batch(const sequence_id &seq_id, int queue_id, const std::vector<std::string> &keys, const std::vector<std::string> &values) = 0;

    virtual void chain_req(const sequence_id& seq, const std::vector<std::string> & arguments) = 0;
    virtual void setup_chain_stub(const int32_t block_id, const std::string& path, const std::vector<std::string> & chain, const int32_t role, const std::string& next_block_id) = 0;
    virtual void resend_pending_stub(const int32_t block_id) = 0;


};
#endif //PANCAKE_PROXY_H
