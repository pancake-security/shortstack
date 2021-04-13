//
// Created by Lloyd Brown on 10/5/19.
//


#include <unordered_map>
#include <iostream>
#include <spdlog/spdlog.h>

#include "distribution.h"
#include "pancake_proxy.h"
#include "l1_proxy.h"
#include "l2_proxy.h"
#include "l3_proxy.h"
#include "host_info.h"
#include "distribution_info.h"
//#include "thrift_response_client_map.h"
#include "thrift_server.h"
#include "l1_server.h"
#include "l2_server.h"
#include "l3_server.h"
#include "initializer.h"
#include "thrift_utils.h"
#include "proxy_manager.h"
#include "update_cache.h"

#define HOST "127.0.0.1"
#define PROXY_PORT 9090

typedef std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>> trace_vector;

distribution load_frequencies_from_trace(const std::string &trace_location, trace_vector &trace_, int client_batch_size_) {
    std::vector<std::string> get_keys;
    std::vector<std::string> put_keys;
    std::vector<std::string> put_values;

    std::unordered_map<std::string, int> key_to_frequency;
    int frequency_sum = 0;
    std::string op, key, val;
    std::ifstream in_workload_file;
    in_workload_file.open(trace_location, std::ios::in);
    if(!in_workload_file.is_open()){
        std::perror("Unable to find workload file");
    }
    if(in_workload_file.fail()){
        std::perror("Opening workload file failed");
    }
    std::string line;
    while (std::getline(in_workload_file, line)) {
        op = line.substr(0, line.find(" "));
        key = line.substr(line.find(" ")+1);
        val = "";

        if (key.find(" ") != -1) {
            val = key.substr(key.find(" ")+1);
            key = key.substr(0, key.find(" "));
        }
        if(val == ""){
            get_keys.push_back(key);
            if (get_keys.size() == client_batch_size_){
                trace_.push_back(std::make_pair(get_keys, std::vector<std::string>()));
                get_keys.clear();
            }
        }
        else {
            put_keys.push_back(key);
            put_values.push_back(val);
            if (put_keys.size() == client_batch_size_){
                trace_.push_back(std::make_pair(put_keys, put_values));
                put_keys.clear();
                put_values.clear();
            }
        }
        assert (key != "PUT");
        assert (key != "GET");
        if (key_to_frequency.count(key) == 0){
            key_to_frequency[key] = 1;
            frequency_sum += 1;
        }
        else {
            key_to_frequency[key] += 1;
            frequency_sum += 1;
        }
    }
    if (get_keys.size() > 0){
        trace_.push_back(std::make_pair(get_keys, std::vector<std::string>()));
        get_keys.clear();
    }
    if (put_keys.size() > 0){
        trace_.push_back(std::make_pair(put_keys, put_values));
        put_keys.clear();
        put_values.clear();
    }
    std::vector<std::string> keys;
    std::vector<double> frequencies;
    for (auto pair: key_to_frequency){
        keys.push_back(pair.first);
        frequencies.push_back(pair.second/(double)frequency_sum);
    }
    in_workload_file.close();
    distribution dist(keys, frequencies);
    return dist;
};

void flush_thread(std::shared_ptr<proxy> proxy){
    while (true){
        sleep(1);
        dynamic_cast<pancake_proxy&>(*proxy).flush();
    }
    std::cout << "Quitting flush thread" << std::endl;
}

void pancake_usage() {
    std::cout << "Pancake proxy: frequency flattening kvs\n";
    // Network Parameters
    std::cout << "\t -h: Storage server host name\n";
    std::cout << "\t -p: Storage server port\n";
    std::cout << "\t -s: Storage server type (redis, rocksdb, memcached)\n";
    std::cout << "\t -n: Storage server count\n";
    std::cout << "\t -z: Proxy server type\n";
    // Workload parameters
    std::cout << "\t -l: Workload file\n";
    std::cout << "\t -v: Value size\n";
    std::cout << "\t -b: Security batch size\n";
    std::cout << "\t -c: Storage batch size\n";
    std::cout << "\t -t: Number of worker threads for cpp_redis\n";
    // Other parameters
    std::cout << "\t -o: Output location for sizing thread\n";
    std::cout << "\t -d: Core to run on\n";
};


int pancake_main(int argc, char *argv[]) {
    int client_batch_size = 50;
    std::atomic<int> xput;
    std::atomic_init(&xput, 0);
    int object_size_ = 1000;

    std::shared_ptr<proxy> proxy_ = std::make_shared<pancake_proxy>();
    int o;
    std::string proxy_type_ = "pancake";
    while ((o = getopt(argc, argv, "h:p:s:n:v:b:c:t:o:d:z:q:l:")) != -1) {
        switch (o) {
            case 'h':
                dynamic_cast<pancake_proxy&>(*proxy_).server_host_name_ = std::string(optarg);
                break;
            case 'p':
                dynamic_cast<pancake_proxy&>(*proxy_).server_port_ = std::atoi(optarg);
                break;
            case 's':
                dynamic_cast<pancake_proxy&>(*proxy_).server_type_ = std::string(optarg);
                break;
            case 'n':
                dynamic_cast<pancake_proxy&>(*proxy_).server_count_ = std::atoi(optarg);
                break;
            case 'v':
                dynamic_cast<pancake_proxy&>(*proxy_).object_size_ = std::atoi(optarg);
                break;
            case 'b':
                dynamic_cast<pancake_proxy&>(*proxy_).security_batch_size_ = std::atoi(optarg);
                break;
            case 'c':
                dynamic_cast<pancake_proxy&>(*proxy_).storage_batch_size_ = std::atoi(optarg);
                break;
            case 't':
                dynamic_cast<pancake_proxy&>(*proxy_).p_threads_ = std::atoi(optarg);
                break;
            case 'o':
                dynamic_cast<pancake_proxy&>(*proxy_).output_location_ = std::string(optarg);
                break;
            case 'd':
                dynamic_cast<pancake_proxy&>(*proxy_).core_ = std::atoi(optarg) - 1;
                break;
            case 'z':
                proxy_type_ = std::string(optarg);
                break;
            case 'q':
                client_batch_size = std::atoi(optarg);
                break;
            case 'l':
                dynamic_cast<pancake_proxy&>(*proxy_).trace_location_ = std::string(optarg);
                break;
            default:
                pancake_usage();
                exit(-1);
        }
    }

    void *arguments[4];
    std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>> trace_;
    assert(dynamic_cast<pancake_proxy&>(*proxy_).trace_location_ != "");
    auto dist = load_frequencies_from_trace(dynamic_cast<pancake_proxy&>(*proxy_).trace_location_, trace_, client_batch_size);

    arguments[0] = &dist;
    auto items = dist.get_items();
    double alpha = 1.0 / items.size();
    double delta = 1.0 / (2 * items.size()) * 1 / alpha;
    auto id_to_client = std::make_shared<thrift_response_client_map>();
    arguments[1] = &alpha;
    arguments[2] = &delta;
    arguments[3] = &id_to_client;
    std::string dummy(object_size_, '0');
    std::cout <<"Initializing pancake" << std::endl;
    dynamic_cast<pancake_proxy&>(*proxy_).init(items, std::vector<std::string>(items.size(), dummy), arguments);
    std::cout << "Initialized pancake" << std::endl;
    auto proxy_server = thrift_server::create(proxy_, "pancake", id_to_client, PROXY_PORT, 1);
    std::thread proxy_serve_thread([&proxy_server] { proxy_server->serve(); });
    wait_for_server_start(HOST, PROXY_PORT);
    std::cout << "Proxy server is reachable" << std::endl;
    sleep(10000);
    //flush_thread(proxy_);
    //proxy_->close();
    //proxy_server->stop();
    return 0;
}

void l1_usage() {
    std::cout << "Shortstack L1 proxy\n";
    // Network Parameters
    std::cout << "\t -h: Hosts file\n";
    std::cout << "\t -d: Distribution info file\n";
    std::cout << "\t -i: Instance name\n";
}

int l1_main(int argc, char *argv[]) {
    int o;
    std::string hosts_file;
    std::string dist_file;
    std::string instance_name;
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    while ((o = getopt(argc, argv, "h:d:i:gc:")) != -1) {
        switch (o) {
            case 'h':
                hosts_file = std::string(optarg);
                break;
            case 'd':
                dist_file = std::string(optarg);
                break;
            case 'i':
                instance_name = std::string(optarg);
                break;
            case 'g':
                spdlog::set_level(spdlog::level::debug);
                break;
            case 'c':
                num_cores = std::atoi(optarg);
                break;
            default:
                l1_usage();
                exit(-1);
        }
    }

    auto hinfo = std::make_shared<host_info>();
    if(!hinfo->load(hosts_file)) {
        std::cerr << "Unable to load hosst file" << std::endl;
        exit(-1);
    }

    std::string proxy_host;
    int proxy_port;
    int num_workers;
    host this_host;
    if(!hinfo->get_host(instance_name, this_host)) {
        std::cerr << "Invalid instance name" << std::endl;
        exit(-1);
    }

    proxy_host = this_host.hostname;
    proxy_port = this_host.port;
    num_workers = this_host.num_workers;

    auto dinfo = std::make_shared<distribution_info>();
    // TODO: exception handling
    dinfo->load(dist_file);

    std::vector<std::thread> proxy_serve_threads(num_workers);
    std::vector<std::shared_ptr<TServer>> proxy_servers(num_workers);
    std::vector<std::shared_ptr<l1_proxy>> proxys(num_workers);

    auto id_to_client = std::make_shared<thrift_response_client_map>();

    for(int i = 0; i < num_workers; i++) 
    {
        proxys[i] = std::make_shared<l1_proxy>();
        proxys[i]->init_proxy(hinfo, instance_name, dinfo, i);
        proxy_servers[i] = l1_server::create(proxys[i], "l1", id_to_client, proxy_port + i, 1);
        proxy_serve_threads[i] = std::thread([&proxy_servers, i] { proxy_servers[i]->serve(); });
    }
    
    for(int i = 0; i < num_workers; i++) {
        wait_for_server_start(proxy_host, proxy_port + i);
    }
    
    std::cout << "Proxy server is reachable" << std::endl;
    sleep(10000);

    return 0;

}

void l2_usage() {
    std::cout << "Shortstack L2 proxy\n";
    // Network Parameters
    std::cout << "\t -h: Hosts file\n";
    std::cout << "\t -d: Distribution info file\n";
    std::cout << "\t -i: Instance name\n";
}

int l2_main(int argc, char *argv[]) {
    int o;
    std::string hosts_file;
    std::string dist_file;
    std::string instance_name;
    bool uc_enabled = true;
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

    while ((o = getopt(argc, argv, "h:d:i:gc:u")) != -1) {
        switch (o) {
            case 'h':
                hosts_file = std::string(optarg);
                break;
            case 'd':
                dist_file = std::string(optarg);
                break;
            case 'i':
                instance_name = std::string(optarg);
                break;
            case 'g':
                spdlog::set_level(spdlog::level::debug);
                break;
            case 'c':
                num_cores = std::atoi(optarg);
                break;
            case 'u':
                uc_enabled = false;
                break;
            default:
                l2_usage();
                exit(-1);
        }
    }

    auto hinfo = std::make_shared<host_info>();
    if(!hinfo->load(hosts_file)) {
        std::cerr << "Unable to load hosst file" << std::endl;
        exit(-1);
    }

    std::string proxy_host;
    int proxy_port;
    int num_workers;
    host this_host;
    if(!hinfo->get_host(instance_name, this_host)) {
        std::cerr << "Invalid instance name" << std::endl;
        exit(-1);
    }

    proxy_host = this_host.hostname;
    proxy_port = this_host.port;
    num_workers = this_host.num_workers;

    auto dinfo = std::make_shared<distribution_info>();
    // TODO: exception handling
    dinfo->load(dist_file);

    std::vector<std::thread> proxy_serve_threads(num_workers);
    std::vector<std::shared_ptr<TServer>> proxy_servers(num_workers);
    std::vector<std::shared_ptr<l2_proxy>> proxys(num_workers);

    auto cache = std::make_shared<update_cache>();

    for(int i = 0; i < num_workers; i++) 
    {
        proxys[i] = std::make_shared<l2_proxy>();
        proxys[i]->init_proxy(hinfo, instance_name, dinfo, cache, uc_enabled, i);
        proxy_servers[i] = l2_server::create(proxys[i], proxy_port + i, 1, 1);
        proxy_serve_threads[i] = std::thread([&proxy_servers, i] { proxy_servers[i]->serve(); });
    }
    
    for(int i = 0; i < num_workers; i++) {
        wait_for_server_start(proxy_host, proxy_port + i);
    }

    std::cout << "Proxy server is reachable" << std::endl;
    sleep(10000);

    return 0;

}

void l3_usage() {
    std::cout << "Shortstack L3 proxy\n";
    // Network Parameters
    std::cout << "\t -h: Hosts file\n";
    std::cout << "\t -d: Distribution info file\n";
    std::cout << "\t -i: Instance name\n";
}

int l3_main(int argc, char *argv[]) {
    int o;
    std::string hosts_file;
    std::string dist_file;
    std::string instance_name;
    int storage_batch_size;
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    bool encryption = true;
    bool resp_delivery = true;
    bool kv_interaction = true;
    while ((o = getopt(argc, argv, "h:i:s:gc:prk")) != -1) {
        switch (o) {
            case 'h':
                hosts_file = std::string(optarg);
                break;
            case 'i':
                instance_name = std::string(optarg);
                break;
            case 's':
                storage_batch_size = std::atoi(optarg);
                break;
            case 'g':
                spdlog::set_level(spdlog::level::debug);
                break;
            case 'c':
                num_cores = std::atoi(optarg);
                break;
            case 'p':
                encryption = false;
                break;
            case 'r':
                resp_delivery = false;
                break;
            case 'k':
                kv_interaction = false;
                break;
            default:
                l3_usage();
                exit(-1);
        }
    }

    auto hinfo = std::make_shared<host_info>();
    if(!hinfo->load(hosts_file)) {
        std::cerr << "Unable to load hosst file" << std::endl;
        exit(-1);
    }

    std::string proxy_host;
    int proxy_port;
    int num_workers;
    host this_host;
    if(!hinfo->get_host(instance_name, this_host)) {
        std::cerr << "Invalid instance name" << std::endl;
        exit(-1);
    }

    proxy_host = this_host.hostname;
    proxy_port = this_host.port;
    num_workers = this_host.num_workers;

    // auto dinfo = std::make_shared<distribution_info>();
    // // TODO: exception handling
    // dinfo->load(dist_file);

    std::vector<std::thread> proxy_serve_threads(num_workers);
    std::vector<std::shared_ptr<TServer>> proxy_servers(num_workers);
    std::vector<std::shared_ptr<l3_proxy>> proxys(num_workers);
    std::vector<std::shared_ptr<thrift_response_client_map>> id_to_clients(num_workers);

    

    for(int i = 0; i < num_workers; i++) 
    {
        id_to_clients[i] = std::make_shared<thrift_response_client_map>();
        proxys[i] = std::make_shared<l3_proxy>();
        proxys[i]->init_proxy(hinfo, instance_name, 1, storage_batch_size, id_to_clients[i], encryption, resp_delivery, kv_interaction, i);
        proxy_servers[i] = l3_server::create(proxys[i], id_to_clients[i], proxy_port + i, 1, 1);
        proxy_serve_threads[i] = std::thread([&proxy_servers, i] { proxy_servers[i]->serve(); });
    }
    
    for(int i = 0; i < num_workers; i++) {
        wait_for_server_start(proxy_host, proxy_port + i);
    }

    std::cout << "Proxy server is reachable" << std::endl;
    sleep(10000);

    return 0;

}

void init_usage() {
    std::cout << "Shortstack init\n";

    std::cout << "\t -h: Hosts file\n";
    std::cout << "\t -o: Object size\n";
    std::cout << "\t -t: Trace file\n";
    std::cout << "\t -d: Distinfo file target path\n";
}

int init_main(int argc, char *argv[]) {
    int client_batch_size = 50;
    int object_size = 1000;
    std::string hosts_file;
    std::string trace_file;
    std::string dinfo_file;
    int o;
    while ((o = getopt(argc, argv, "h:o:t:d:")) != -1) {
        switch (o) {
            case 'h':
                hosts_file = std::string(optarg);
                break;
            case 'o':
                object_size = std::atoi(optarg);
                break;
            case 't':
                trace_file = std::string(optarg);
                break;
            case 'd':   
                dinfo_file = std::string(optarg);
                break;
            default:
                init_usage();
                exit(-1);
        }
    }

    auto hinfo = std::make_shared<host_info>();
    if(!hinfo->load(hosts_file)) {
        std::cerr << "Unable to load hosts file" << std::endl;
        exit(-1);
    }

    std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>> trace_;
    auto dist = load_frequencies_from_trace(trace_file, trace_, client_batch_size);

    std::cout << "Real distribution support size: " << dist.get_items().size() << "\n";

    initializer initer;

    initer.init(dist, object_size, hinfo);
    std::cout << "Initialized KV store\n";

    auto dinfo = initer.get_distinfo();
    dinfo->dump(dinfo_file);
    std::cout << "Dumped distribution info file\n";
    
    return 0;
}

void dump_usage() {
    std::cout << "Dump distribution info file\n";

    std::cout << "\t -d: Distinfo file target path\n";
}


int dump_main(int argc, char *argv[]) {
    std::string dinfo_file;
    int o;
    while ((o = getopt(argc, argv, "d:")) != -1) {
        switch (o) {
            case 'd':   
                dinfo_file = std::string(optarg);
                break;
            default:
                dump_usage();
                exit(-1);
        }
    }

    auto dinfo = std::make_shared<distribution_info>();
    dinfo->load(dinfo_file);

    std::cout << "dummy_key: " << dinfo->dummy_key_ << std::endl;
    std::cout << "num_keys: " << dinfo->num_keys_ << std::endl;

    std::cout << "real_distribution:" << std::endl;
    auto keys = dinfo->real_distribution_.get_items();
    auto probs = dinfo->real_distribution_.get_probabilities();
    for(int i = 0; i < keys.size(); i++) {
        std::cout << keys[i] << " " << probs[i] << std::endl;
    }

    std::cout << "fake_distribution:" << std::endl;
    keys = dinfo->fake_distribution_.get_items();
    probs = dinfo->fake_distribution_.get_probabilities();
    for(int i = 0; i < keys.size(); i++) {
        std::cout << keys[i] << " " << probs[i] << std::endl;
    }

    std::cout << "key_to_number_of_replicas:" << std::endl;
    for(auto &it : dinfo->key_to_number_of_replicas_) {
        std::cout << it.first << " " << it.second << std::endl;
    }

    std::cout << "replica_to_label:" << std::endl;
    for(auto &it : dinfo->replica_to_label_) {
        std::cout << it.first << " " << it.second << std::endl;
    }

    return 0;
}

void manager_usage() {
    std::cout << "Shortstack proxy manager\n";

    std::cout << "\t -h: Hosts file\n";
    std::cout << "\t -f: Instance name to fail\n";
}

int manager_main(int argc, char *argv[]) {
    std::string hosts_file;
    std::string fail_node;
    int o;
    while ((o = getopt(argc, argv, "h:f:")) != -1) {
        switch (o) {
            case 'h':
                hosts_file = std::string(optarg);
                break;
            case 'f':
                fail_node = std::string(optarg);
                break;
            default:
                manager_usage();
                exit(-1);
        }
    }

    auto hinfo = std::make_shared<host_info>();
    if(!hinfo->load(hosts_file)) {
        std::cerr << "Unable to load hosts file" << std::endl;
        exit(-1);
    }

    auto manager = std::make_shared<proxy_manager>();
    manager->init(hinfo);

    manager->fail_node(fail_node);
}

void usage() {
    std::cout << "Usage: ./proxy_server <type> .....\n";
}

int main(int argc, char *argv[]) {

    if(argc < 2) {
        usage();
        exit(-1);
    }

    if(strcmp(argv[1], "pancake") == 0) {
        return pancake_main(argc - 1, argv + 1);
    } else if(strcmp(argv[1], "l1") == 0) {
        return l1_main(argc - 1, argv + 1);
    } else if(strcmp(argv[1], "l2") == 0) {
        return l2_main(argc - 1, argv + 1);
    } else if(strcmp(argv[1], "l3") == 0) {
        return l3_main(argc - 1, argv + 1);
    } else if(strcmp(argv[1], "init") == 0) {
        return init_main(argc - 1, argv + 1);
    } else if(strcmp(argv[1], "dump") == 0) {
        return dump_main(argc - 1, argv + 1);
    } else if(strcmp(argv[1], "manager") == 0) {
        return manager_main(argc - 1, argv + 1);
    } else {    
        std::cerr << "Unkown proxy type" << std::endl;
        exit(-1);
    }

}
