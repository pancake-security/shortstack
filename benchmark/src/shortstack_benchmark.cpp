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
#include <cstdlib>
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
                trace_vector &trace, std::atomic<int> &xput, shortstack_client& client, int queue_depth,
                std::vector<std::string> &diags, std::atomic<uint64_t> &total_op_count) {
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
        std::string out, diag;
        auto seq = client.poll_responses(out, diag);
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
            diags.push_back(diag);
        }
        total_op_count++;
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
            trace_vector &trace, std::atomic<int> &xput, shortstack_client& client, int qd, std::atomic<uint64_t> &total_op_count) {
                std::vector<std::string> dummy;
    run_benchmark(5, false, latencies, client_batch_size, trace, xput, client, qd, dummy, total_op_count);
}

void cooldown(std::vector<int> &latencies, int client_batch_size,
               trace_vector &trace, std::atomic<int> &xput, shortstack_client& client, int qd, std::atomic<uint64_t> &total_op_count) {
                   std::vector<std::string> dummy;
    run_benchmark(5, false, latencies, client_batch_size, trace, xput, client, qd, dummy, total_op_count);
}

void client(bool warmcool, int idx, int client_batch_size, trace_vector &trace, std::string output_path, std::shared_ptr<host_info> hinfo, std::atomic<int> &xput, int queue_depth, std::vector<int> &latencies, std::vector<std::string> &diags, std::atomic<uint64_t> &total_op_count) {
    shortstack_client client;
    client.init(idx, hinfo);

    std::cout << "Client " << idx << " initialized" << std::endl;
    std::atomic<int> indiv_xput;
    std::atomic_init(&indiv_xput, 0);
    // std::vector<int> latencies;
    if(warmcool) {
    	std::cout << "Beginning warmup" << std::endl;
    	warmup(latencies, client_batch_size, trace, indiv_xput, client, queue_depth, total_op_count);
    }
    std::cout << "Beginning benchmark" << std::endl;
    run_benchmark(10, true, latencies, client_batch_size, trace, indiv_xput, client, queue_depth, diags, total_op_count);
    std::string location = output_path + "-client" + std::to_string(idx)+ ".lat";
    std::ofstream out(location);
    std::string line("");
    for (int i = 0; i < latencies.size(); i++) {
        line.append(std::to_string(latencies[i]) + "," + diags[i] + "\n");
        out << line;
        line.clear();
    }
    line.append("Xput: " + std::to_string(indiv_xput) + "\n");
    out << line;
    xput += indiv_xput;
    
    if(warmcool) {
    	std::cout << "Beginning cooldown" << std::endl;
    	cooldown(latencies, client_batch_size, trace, indiv_xput, client, queue_depth, total_op_count);
    }
    

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

void sleep_nanos(long delay);
// Yield for certain number of nanoseconds
// delay should not exceed 1e9 micros
void sleep_nanos(long delay)
{
 struct timespec tv;
 /* Construct the timespec from the number of whole seconds... */
 tv.tv_sec = 0;
 /* ... and the remainder in nanoseconds. */
 tv.tv_nsec = delay;

 while (1)
 {
  /* Sleep for the time specified in tv. If interrupted by a
    signal, place the remaining time left to sleep back into tv. */
  int rval = nanosleep (&tv, &tv);
  if (rval == 0)
   /* Completed the entire sleep time; all done. */
   return;
  else if (errno == EINTR)
   /* Interrupted by a signal. Try again. */
   continue;
  else 
  {
	/* Some other error; bail out. */
	fprintf(stderr, "nanesleep failed\n");
	exit(1);
  }

 }
}

void monitor_xput(std::atomic<uint64_t>& total_op_count, std::atomic<bool>& done, int sleep_us, std::vector<int>& inst_xput) {
    uint64_t prev_count = 0;
    while(!done.load()) {
        auto cur = total_op_count.load();
        inst_xput.push_back(cur - prev_count);
        prev_count = cur;

        sleep_nanos(sleep_us * 1000);
    }
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
    std::string output_prefix = "foo";

    int o;
    std::string hosts_file;
    bool debug_mode = false;
    int monitor_us = 1000;
    bool warmcool = true;
    while ((o = getopt(argc, argv, "h:t:n:o:q:gm:w")) != -1) {
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
            case 'm':
                monitor_us = std::atoi(optarg);
                break;
	    case 'w':
                warmcool = false;
                break;
            default:
                usage();
                exit(-1);
        }
    }

    std::string output_path = "data/" + output_prefix;

    if(debug_mode) {
        spdlog::set_level(spdlog::level::debug);
    }

    auto hinfo = std::make_shared<host_info>();
    if(!hinfo->load(hosts_file)) {
        std::cerr << "Unable to load hosts file" << std::endl;
        exit(-1);
    }

    // _mkdir((output_directory).c_str());
    std::string rm_cmdline = "rm " + output_path + "*";
    system(rm_cmdline.c_str());
    std::atomic<int> xput;
    std::atomic_init(&xput, 0);

    trace_vector trace;
    load_trace(trace_location, trace);
    std::cout << "trace loaded" << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<int64_t> distrib(0,10000);
    int64_t base_client_id = distrib(gen);

    std::vector<std::vector<int>> client_lats;
    std::vector<std::vector<std::string>> client_diags;
    for(int i = 0; i < num_clients; i++) 
    {
        std::vector<int> lats;
        client_lats.push_back(lats);
        std::vector<std::string> d;
        client_diags.push_back(d);
    }

    std::atomic<uint64_t> total_op_count_;
    total_op_count_.store(0);

    std::atomic<bool> done;
    done.store(false);


    std::vector<std::thread> threads;
    for (int i = 0; i < num_clients; i++) {
        threads.push_back(std::thread(client, warmcool, base_client_id + i, client_batch_size, std::ref(trace),
                          output_path, hinfo, std::ref(xput), queue_depth, std::ref(client_lats[i]),
                          std::ref(client_diags[i]), std::ref(total_op_count_)));
    }

    std::vector<int> inst_xput; 
    std::thread monitor(monitor_xput, std::ref(total_op_count_), std::ref(done), monitor_us, std::ref(inst_xput));

    for (int i = 0; i < num_clients; i++)
        threads[i].join();

    done.store(true);

    monitor.join();
	
    std::string stats_location = output_path + ".stats";
    std::ofstream stats_out(stats_location);
    
    std::cout << "Xput was: " << xput << std::endl;
    stats_out << "Xput: " << xput << "\n";

    std::string location = output_path + ".xput";
    std::ofstream out(location);
    std::string line("");
    for (int i = 0; i < inst_xput.size(); i++) {
        double xp = ((double)inst_xput[i] * 1e6)/monitor_us;
        line.append(std::to_string(xp) + "\n");
        out << line;
        line.clear();
    }

    out.flush();
    out.close();

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
    stats_out << "Average latency: " << (lat_sum/lat_count) << "\n";

    stats_out.flush();
    stats_out.close();
}
