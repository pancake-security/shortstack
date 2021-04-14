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
#include <spdlog/spdlog.h>
#include "timer.h"
#include "shortstack_client.h"
#include "thrift_utils.h"
#include "host_info.h"

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

void run_benchmark(int run_time, bool stats, std::vector<int> &latencies, int client_batch_size,
                trace_vector &trace, std::atomic<int> &xput, shortstack_client& client, int queue_depth) {
    int ops = 0;
    uint64_t start, end;
    auto ticks_per_ns = static_cast<double>(rdtscuhz()) / 1000;
    auto s = std::chrono::high_resolution_clock::now();
    auto e = std::chrono::high_resolution_clock::now();
    int elapsed = 0;
    std::vector<std::string> results;
    std::unordered_map<int64_t, uint64_t> start_ts;
    int idx = 0;
    int64_t smallest_seq = INT64_MAX;

    // Submit initial set of requests
    for(int i = 0; i < queue_depth; i++) {
        auto kv_pair = trace[idx];
        idx = (idx+1)%trace.size();
        auto key = kv_pair.first;
        auto val = kv_pair.second;
        if(stats) {
            rdtscll(start);
        }

        int64_t seq;
        if(val.empty()) {
            seq = client.get(key);
            
        } else {
            seq = client.put(key, val);
        }
        spdlog::debug("sent request client_id:{}, seq_no:{}", client.get_client_id(), seq);

        smallest_seq = std::min(smallest_seq, seq);

        if (stats) {
            start_ts[seq] = start;
        }
    }

    while (elapsed < run_time*1000000) {
        std::string out;
        auto seq = client.poll_responses(out);
        if(seq < smallest_seq) 
        {
            // Stale request
            spdlog::debug("Recvd response with stale seq no: {}, {}, {}", seq, start_ts[seq], end);
            e = std::chrono::high_resolution_clock::now();
            elapsed = static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(e - s).count());
            continue;
        }
        if (stats) {
            rdtscll(end);
            
            double cycles = static_cast<double>(end - start_ts[seq]);
            latencies.push_back((cycles / ticks_per_ns)/1000);
        }
        ops += 1;

        spdlog::debug("recvd response client_id:{}, seq_no:{}", client.get_client_id(), seq);

        // Send new request
        auto kv_pair = trace[idx];
        idx = (idx+1)%trace.size();
        auto key = kv_pair.first;
        auto val = kv_pair.second;
        if(stats) {
            rdtscll(start);
        }

        if(val.empty()) {
            seq = client.get(key);
            
        } else {
            seq = client.put(key, val);
        }
        spdlog::debug("sent request client_id:{}, seq_no:{}", client.get_client_id(), seq);

        if (stats) {
            start_ts[seq] = start;
        }

        e = std::chrono::high_resolution_clock::now();
        elapsed = static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(e - s).count());
    }

    e = std::chrono::high_resolution_clock::now(); 
    elapsed = static_cast<int>(std::chrono::duration_cast<std::chrono::microseconds>(e - s).count());
    if (stats)
        xput += (int)(static_cast<double>(ops) * 1000000 / elapsed);
}

void warmup(std::vector<int> &latencies, int client_batch_size,
            trace_vector &trace, std::atomic<int> &xput, shortstack_client& client, int qd) {
    run_benchmark(5, false, latencies, client_batch_size, trace, xput, client, qd);
}

void cooldown(std::vector<int> &latencies, int client_batch_size,
               trace_vector &trace, std::atomic<int> &xput, shortstack_client& client, int qd) {
    run_benchmark(5, false, latencies, client_batch_size, trace, xput, client, qd);
}

void client(int idx, int client_batch_size, trace_vector &trace, std::string output_directory, std::shared_ptr<host_info> hinfo, std::atomic<int> &xput, int queue_depth) {
    shortstack_client client;
    client.init(idx, hinfo);

    std::cout << "Client " << idx << " initialized" << std::endl;
    std::atomic<int> indiv_xput;
    std::atomic_init(&indiv_xput, 0);
    std::vector<int> latencies;
    std::cout << "Beginning warmup" << std::endl;
    warmup(latencies, client_batch_size, trace, indiv_xput, client, queue_depth);
    std::cout << "Beginning benchmark" << std::endl;
    run_benchmark(10, true, latencies, client_batch_size, trace, indiv_xput, client, queue_depth);
    std::string location = output_directory + "/client" + std::to_string(idx)+ ".lat";
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
    cooldown(latencies, client_batch_size, trace, indiv_xput, client, queue_depth);

    client.finish();
}

void usage() {
    std::cout << "Shortstack client\n";
    std::cout << "\t -h: Hosts csv file\n";
    std::cout << "\t -t: Trace Location\n";
    std::cout << "\t -n: Number of threads to spawn\n";
    std::cout << "\t -q: Queue depth\n";
    std::cout << "\t -o: Output Directory\n";
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

int main(int argc, char *argv[]) {
    std::string trace_location = "";
    int client_batch_size = 1;
    // int object_size = 1000;
    int num_clients = 1;
    int queue_depth = 1;

    std::time_t end_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto date_string = std::string(std::ctime(&end_time));
    date_string = date_string.substr(0, date_string.rfind(":"));
    date_string.erase(remove(date_string.begin(), date_string.end(), ' '), date_string.end());
    std::string output_directory = "data/"+date_string;

    int o;
    std::string hosts_file;
    bool debug_mode = false;
    while ((o = getopt(argc, argv, "h:t:n:o:q:g")) != -1) {
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
                output_directory = std::string(optarg);
                break;
            case 'q':
                queue_depth = std::atoi(optarg);
                break;
            case 'g':
                debug_mode = true;
                break;
            default:
                usage();
                exit(-1);
        }
    }

    if(debug_mode) {
        spdlog::set_level(spdlog::level::debug);
    }

    auto hinfo = std::make_shared<host_info>();
    if(!hinfo->load(hosts_file)) {
        std::cerr << "Unable to load hosts file" << std::endl;
        exit(-1);
    }

    _mkdir((output_directory).c_str());
    std::atomic<int> xput;
    std::atomic_init(&xput, 0);

    trace_vector trace;
    load_trace(trace_location, trace);
    std::cout << "trace loaded" << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<int64_t> distrib(0,10000);
    int64_t base_client_id = distrib(gen);

    std::vector<std::thread> threads;
    for (int i = 0; i < num_clients; i++) {
        threads.push_back(std::thread(client, base_client_id + i, client_batch_size, std::ref(trace),
                          output_directory, hinfo, std::ref(xput), queue_depth));
    }
    for (int i = 0; i < num_clients; i++)
        threads[i].join();
    std::cout << "Xput was: " << xput << std::endl;
}
