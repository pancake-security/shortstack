#include <unordered_map>
#include <iostream>
#include <memory>
#include <cstdint>
#include <thread>
#include "assert.h"

#include "host_info.h"
#include "shortstack_client.h"

void usage() {
    std::cout << "<binary> -h <hosts_file>" << std::endl;
    std::cout << "Make sure proxys are initialized with traces/simple" << std::endl;
}

int main(int argc, char *argv[]) {
    int o;
    std::string hosts_file;
    while ((o = getopt(argc, argv, "h:")) != -1) {
        switch (o) {
            case 'h':
                hosts_file = std::string(optarg);
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

    std::random_device rd;
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<int64_t> distrib(0,10000);
    int64_t base_client_id = distrib(gen);

    auto client = std::make_shared<shortstack_client>();
    client->init(base_client_id, hinfo);
    std::cout << "Initialized client" << std::endl;

    std::string res;
    int64_t req_id = client->get("1");
    std::cout << "get 1 sent\n";
    assert(client->poll_responses(res) == req_id);
    std::cout << "get response recvd\n";

    
    req_id = client->put("1", "hello");
    std::cout << "put 1 sent\n";
    assert(client->poll_responses(res) == req_id);
    std::cout << "put 1 response recvd\n";
    assert(res == "");

    req_id = client->get("1");
    std::cout << "get 1 sent\n";
    assert(client->poll_responses(res) == req_id);
    std::cout << "get 1 response recvd\n";
    if(res != "hello")
    {
        std::cout << "inconsistency " << res << "\n";
    }
    assert(res == "hello");

    
    for(int i = 1; i <= 5; i++) {
        client->put(std::to_string(i), std::to_string(i));
    }

    // Wait for put responses
    for(int i = 1; i <= 5; i++) 
    {
        std::string out;
        client->poll_responses(out);
        assert(out == "");
    }

    std::unordered_map<int64_t, std::string> seq_to_key;

    for(int i = 1; i <= 5; i++) {
        auto seq = client->get(std::to_string(i));
        seq_to_key[seq] = std::to_string(i);
    }

    for(int i = 1; i <= 5; i++) 
    {
        std::string out;
        auto seq = client->poll_responses(out);
        if(seq_to_key[seq] != out) {
            std::cout << "Inconsistency! " << seq_to_key[seq] << ", " << out << "\n"; 
        }
        assert(seq_to_key[seq] == out);
    }

    client->finish();

    std::cout << "Finished\n";

}
