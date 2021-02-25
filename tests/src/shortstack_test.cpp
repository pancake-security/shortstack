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

    auto client = std::make_shared<shortstack_client>();
    client->init(0, hinfo);
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
    assert(res == "hello");


    client->finish();

    std::cout << "Finished\n";

}
