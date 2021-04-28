//
// Created by Lloyd Brown on 10/24/19.
//

// for windows mkdir
#ifdef _WIN32
#include <direct.h>
#endif

#include <unordered_map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <thread>
#include <set>
#include <spdlog/spdlog.h>
#include "timer.h"
// #include "distribution.h"
#include "host_info.h"
#include "redis.h"
#include "encryption_engine.h"

typedef std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>> trace_vector;

void load_trace(const std::string &trace_location, trace_vector &trace, int client_batch_size) {
    std::vector<std::string> get_keys;
    std::vector<std::string> put_keys;
    std::vector<std::string> put_values;

    std::unordered_map<std::string, int> key_to_frequency;
    int frequency_sum = 0;
    std::string op, key, val;
    std::ifstream in_workload_file;
    in_workload_file.open(trace_location, std::ios::in);
    if(!in_workload_file){
        std::perror("Unable to find workload file");
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
            if (get_keys.size() == client_batch_size){
                trace.push_back(std::make_pair(get_keys, std::vector<std::string>()));
                get_keys.clear();
            }
        }
        else {
            put_keys.push_back(key);
            put_values.push_back(val);
            if (put_keys.size() == client_batch_size){
                trace.push_back(std::make_pair(put_keys, put_values));
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
        trace.push_back(std::make_pair(get_keys, std::vector<std::string>()));
        get_keys.clear();
    }
    if (put_keys.size() > 0){
        trace.push_back(std::make_pair(put_keys, put_values));
        put_keys.clear();
        put_values.clear();
    }
    in_workload_file.close();
};

void run_benchmark(int run_time, bool stats, std::vector<int> &latencies, int client_batch_size,
                 trace_vector &trace, std::atomic<int> &xput, redis& client) {

    int ops = 0;
    
    uint64_t start, end;
    auto ticks_per_ns = static_cast<double>(rdtscuhz()) / 1000;
    auto s = std::chrono::high_resolution_clock::now();
    auto e = std::chrono::high_resolution_clock::now();
    int elapsed = 0;
    std::vector<std::string> results;
    int i = 0;
    while (elapsed < run_time*1000000) {
        if (stats) {
            rdtscll(start);
        }
        auto keys_values_pair = trace[i];
        if (keys_values_pair.second.empty()){
            client.get_batch(keys_values_pair.first);
        }
        else {
            client.put_batch(keys_values_pair.first, keys_values_pair.second);
        }
        if (stats) {
            ops += keys_values_pair.first.size();
            rdtscll(end);
            double cycles = static_cast<double>(end - start);
            latencies.push_back((cycles / ticks_per_ns) / client_batch_size);
            rdtscll(start);
        }
        e = std::chrono::high_resolution_clock::now();
        elapsed = static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(e - s).count());
        i = (i+1)%trace.size();
    }

    e = std::chrono::high_resolution_clock::now(); 
    elapsed = static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(e - s).count());
    if (stats)
        xput += (int)(static_cast<double>(ops) * 1000000 / elapsed);
}

void warmup(std::vector<int> &latencies, int client_batch_size,
             trace_vector &trace, std::atomic<int> &xput, redis& client) {
    run_benchmark(15, false, latencies, client_batch_size, trace, xput, client);
}

void cooldown(std::vector<int> &latencies, int client_batch_size,
                trace_vector &trace, std::atomic<int> &xput, redis& client) {
    run_benchmark(15, false, latencies, client_batch_size,  trace, xput, client);
}

// (client, i, client_batch_size, std::ref(trace), output_path, hinfo, std::ref(xput), std::ref(client_lats[i])))
void client(int idx, int client_batch_size, trace_vector &trace, std::string output_path, std::shared_ptr<host_info> hosts, std::atomic<int> &xput, std::vector<int> &latencies) {
    
    std::vector<host> kv_hosts;
    hosts->get_hosts_by_type(HOST_TYPE_KV, kv_hosts); 
      
    redis client(kv_hosts[0].hostname, kv_hosts[0].port, client_batch_size);
    for (int j = 1; j < kv_hosts.size(); j++) {
        client.add_server(kv_hosts[j].hostname, kv_hosts[j].port);
      }

    std::cout << "Client initialized" << std::endl;
    std::atomic<int> indiv_xput;
    std::atomic_init(&indiv_xput, 0);
    // std::vector<int> latencies;
    std::cout << "Beginning warmup" << std::endl;
    warmup(latencies, client_batch_size, trace, indiv_xput, client);
    std::cout << "Beginning benchmark" << std::endl;
    run_benchmark(20, true, latencies, client_batch_size, trace, indiv_xput, client);
    std::string location = output_path + "/redisclient" + std::to_string(idx);
    std::ofstream out(location);
    std::string line("");
    for (auto lat : latencies) {
        line.append(std::to_string(lat) + "\n");
        out << line;
        line.clear();
    }
    line.append("Xput: " + std::to_string(indiv_xput) + "\n");
    out << line;
    xput += indiv_xput;
    std::cout << "Beginning cooldown" << std::endl;
    cooldown(latencies, client_batch_size, trace, indiv_xput, client);

    // client.finish();
}

void usage() {
    std::cout << "Redis benchmark\n";
    std::cout << "\t -h: Hosts filen";
    std::cout << "\t -t: Trace Location\n";
    std::cout << "\t -n: Number of threads to spawn\n";
    std::cout << "\t -s: Batch size\n";
    std::cout << "\t -o: Output Prefix\n";
    std::cout << "\t -i: Init the KV store\n";
};

int _mkdir(const char *path) {
    #ifdef _WIN32
        return ::_mkdir(path);
    #else
        #if _POSIX_C_SOURCE
            return ::mkdir(path, 0755);
        #else
            return ::mkdir(path, 0755); // not sure if this works on mac
        #endif
    #endif
}

void init_idx(trace_vector &trace, int obj_size, std::string host, int port) {
    encryption_engine enc_engine;


    redis client(host, port, 50);

    spdlog::info("Redis client connected");

    std::set<std::string> keys;
    for(auto &kv_pair : trace) {
        for(auto &key : kv_pair.first) {
            keys.insert(key);
        }
    }

    spdlog::info("Num keys: {}", keys.size());

    std::string dummy(obj_size, '0');
    auto dummy_ct = enc_engine.encrypt(dummy);

    std::vector<std::string> put_keys;
    std::vector<std::string> put_vals;

    for(auto &key : keys) {
        put_keys.push_back(key);
        put_vals.push_back(dummy_ct);

        if(put_keys.size() >= 50) {
            client.put_batch(put_keys, put_vals);
            put_keys.clear();
            put_vals.clear();
        }
    }

    if(put_keys.size() > 0) {
        client.put_batch(put_keys, put_vals);
        put_keys.clear();
        put_vals.clear();
    }
}

void init(trace_vector &trace, std::shared_ptr<host_info> hosts, int obj_size) {
    cpp_redis::network::set_default_nb_workers(10);

    std::vector<host> kv_hosts;
    hosts->get_hosts_by_type(HOST_TYPE_KV, kv_hosts);

    for(auto &h : kv_hosts) {
        init_idx(trace, obj_size, h.hostname, h.port);
    }
    

    spdlog::info("Init complete");

}

int main(int argc, char *argv[]) {
    std::string trace_location = "";
    int client_batch_size = 1;
    // int object_size = 1000;
    int num_clients = 1;
    int queue_depth = 1;

    std::string output_prefix = "foo";

    int o;
    std::string hosts_file;
    bool debug_mode = false;
    bool init_mode = false;
    int obj_size = 1000;
    while ((o = getopt(argc, argv, "h:t:n:o:q:gs:iz:")) != -1) {
        switch (o) {
            case 'h':
                hosts_file = std::string(optarg);
                break;
            case 't':
                trace_location = std::string(optarg);
                break;
            case 'n':
                num_clients = std::atoi(optarg);
                break;
            case 'o':
                output_prefix = std::string(optarg);
                break;
            case 'q':
                queue_depth = std::atoi(optarg);
                break;
            case 'g':
                debug_mode = true;
                break;
            case 's':
                client_batch_size = std::atoi(optarg);
                break;
            case 'i':
                init_mode = true;
                break;
            case 'z':
                obj_size = std::atoi(optarg);
                break;
            default:
                usage();
                exit(-1);
        }
    }

    trace_vector trace;
    load_trace(trace_location, trace, client_batch_size);
    std::cout << "trace loaded" << std::endl;

    if(debug_mode) {
        spdlog::set_level(spdlog::level::debug);
    }

    auto hinfo = std::make_shared<host_info>();
    if(!hinfo->load(hosts_file)) {
        std::cerr << "Unable to load hosts file" << std::endl;
        exit(-1);
    }

    if(init_mode) {
        init(trace, hinfo, obj_size);
        return 0;
    }

    std::string output_path = "data/" + output_prefix;


    // _mkdir((output_directory).c_str());
    std::string rm_cmdline = "rm " + output_path + "*";
    system(rm_cmdline.c_str());
    std::atomic<int> xput;
    std::atomic_init(&xput, 0);

    

    std::vector<std::vector<int>> client_lats;
    for(int i = 0; i < num_clients; i++) 
    {
        std::vector<int> lats;
        client_lats.push_back(lats);
    }

    cpp_redis::network::set_default_nb_workers(num_clients);

    std::vector<std::thread> threads;
    for (int i = 0; i < num_clients; i++) {
        threads.push_back(std::thread(client, i, client_batch_size, std::ref(trace),
                          output_path, hinfo, std::ref(xput), std::ref(client_lats[i])));
    }
    for (int i = 0; i < num_clients; i++)
        threads[i].join();
    std::cout << "Xput was: " << xput << std::endl;

    double lat_sum = 0;
    double lat_count = 0;
    for(int i = 0; i < num_clients; i++) 
    {
        for(auto l : client_lats[i]) {
            lat_sum += l;
            lat_count += 1;
        }
    }

    std::cout << "Average latency: " << (lat_sum/lat_count) << std::endl;
}
