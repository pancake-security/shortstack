// Shortstack intializer

#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <memory>

#include "distribution.h"
#include "distribution_info.h"
#include "encryption_engine.h"
#include "util.h"
#include "storage_interface.h"
#include "redis.h"
#include "host_info.h"

class initializer {
public:

    void init(const distribution &real_dist, int object_size, std::shared_ptr<host_info> hosts);
    std::shared_ptr<distribution_info> get_distinfo();
    

private:
    void create_replicas();
    void insert_replicas(const std::string &key, int num_replicas, const std::string &value_cipher);

    std::shared_ptr<distribution_info> dist_info_;
    encryption_engine encryption_engine_;
    int object_size_;
    int label_count_;
    double alpha_;
    double delta_;

    std::vector<std::string> labels_;

    std::shared_ptr<storage_interface> storage_interface_;
};


#endif // INITIALIZER_H