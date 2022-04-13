#include <unordered_map>
#include <iostream>
#include <memory>
#include <cstdint>
#include <sstream>
#include <fstream>
#include <thread>
#include "assert.h"

#include "host_info.h"
#include "shortstack_client.h"

void usage() {
    std::cout << "<binary> -h <hosts_file> -t <trace_file>" << std::endl;
    std::cout << "Make sure proxys are initialized before running" << std::endl;
}

typedef std::vector<std::pair<std::string, std::string>> trace_vector;

void load_trace(const std::string &trace_location, trace_vector &trace) {

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
        trace.push_back(std::make_pair(key, val));
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

    in_workload_file.close();
};

int main(int argc, char *argv[]) {
    int o;
    std::string hosts_file;
    std::string trace_location;
    while ((o = getopt(argc, argv, "h:t:")) != -1) {
        switch (o) {
            case 'h':
                hosts_file = std::string(optarg);
                break;
            case 't':
                trace_location = std::string(optarg);
                break;
            default:
                usage();
                exit(-1);
        }
    }

    auto hinfo = std::make_shared<host_info>();
    if(!hinfo->load(hosts_file)) {
        std::cerr << "Unable to load hosts file" << std::endl;
        exit(-1);
    }
  
    trace_vector trace;
    load_trace(trace_location, trace);
    std::cout << "trace loaded" << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<int64_t> distrib(0,10000);
    int64_t base_client_id = distrib(gen);

    auto client = std::make_shared<shortstack_client>();
    client->init(base_client_id, hinfo);
    std::cout << "Initialized client" << std::endl;
  
    for(int i = 0; i < trace.size(); i++) {
      auto kv_pair = trace[i];
      auto key = kv_pair.first;
      auto val = kv_pair.second;
      int64_t seq;
      
      if(val.empty()) {
            seq = client->get(key);
            std::cout << "GET " << key << " request sent\n";
            
      } else {
            seq = client->put(key, val);
            std::cout << "PUT " << key << " " << val << " request sent\n";
      }
      
      std::string res, diag;
      
      assert(client->poll_responses(res, diag) == seq);
      
      if(val.empty()) {
          std::cout << "GET Response: " << res << "\n";
      } else {
          assert(res == "");
          std::cout << "PUT Response: OK\n"; 
      }
      
    }

    client->finish();

    std::cout << "Finished\n";

}
