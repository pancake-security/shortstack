#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "host_info.h"

std::vector<std::string> getNextLineAndSplitIntoTokens(std::istream& str)
{
    std::vector<std::string>   result;
    std::string                line;
    std::getline(str,line);

    std::stringstream          lineStream(line);
    std::string                cell;

    while(std::getline(lineStream,cell, ' '))
    {
        result.push_back(cell);
    }
    // // This checks for a trailing comma with no data after it.
    // if (!lineStream && cell.empty())
    // {
    //     // If there was a trailing comma then add an empty element.
    //     result.push_back("");
    // }
    return result;
}

bool host_info::load(std::string filename) {
    std::ifstream f(filename);
    if(!f.is_open()) {
        std::cerr << "Unable to open file " + filename << std::endl;
        return false;
    }

    while(true) {
        auto row = getNextLineAndSplitIntoTokens(f);
        if(row.size() == 0) {
            break;
        }
        
        if(row.size() < 7) {
            std::cerr << "Invalid CSV row" << std::endl;
            return false;
        }

        host h;
        h.instance_name = row[0];
        if(row[1] == "L1") {
            h.type = HOST_TYPE_L1;
        } else if(row[1] == "L2") {
            h.type = HOST_TYPE_L2;
        } else if(row[1] == "L3") {
            h.type = HOST_TYPE_L3;
        } else if(row[1] == "KV") {
            h.type = HOST_TYPE_KV;
        } else {
            std::cerr << "Unknown host type: " << row[1] << std::endl;
            return false;
        }

        h.hostname = row[2];
        try {
            h.port = std::stoi(row[3]);
        } catch(...) {
            std::cerr << "Invalid port number: " << row[3] << std::endl;
            return false;
        }

        try {
            h.row = std::stoi(row[4]);
        } catch(...) {
            std::cerr << "Invalid row number: " << row[4] << std::endl;
            return false;
        }

        try {
            h.column = std::stoi(row[5]);
        } catch(...) {
            std::cerr << "Invalid column number: " << row[5] << std::endl;
            return false;
        }

        try {
            h.num_workers = std::stoi(row[6]);
        } catch(...) {
            std::cerr << "Invalid number of workers: " << row[6] << std::endl;
            return false;
        }
        
        hosts_.push_back(h);
    }

    return true;
}

bool host_info::get_hostname(const std::string &instance_name, std::string &hostname) {
    for(auto &h : hosts_) {
        if(h.instance_name == instance_name) {
            hostname = h.hostname;
            return true;
        }
    }
    return false;
}

bool host_info::get_port(const std::string &instance_name, int &port) {
    for(auto &h : hosts_) {
        if(h.instance_name == instance_name) {
            port = h.port;
            return true;
        }
    }
    return false;
}

bool host_info::get_host(const std::string &instance_name, host& out) {
    for(auto &h : hosts_) {
        if(h.instance_name == instance_name) {
            out = h;
            return true;
        }
    }
    return false;
}

void host_info::get_hosts_by_type(int type, std::vector<host> &hosts) {
    for(auto &h : hosts_) {
      if(h.type == type) {
          hosts.push_back(h);
      }  
    }
}

bool host_info::get_base_idx(const std::string &instance_name, int &idx) {
    host h;
    if(!get_host(instance_name, h)) {
        return false;
    } 

    std::vector<host> peer_hosts;
    get_hosts_by_type(h.type, peer_hosts);

    int ret = 0;
    for(auto &peer : peer_hosts) 
    {   
        if(peer.row == h.row && peer.column < h.column) {
            ret += peer.num_workers;
        }
    }


    idx = ret;
    return true;

}