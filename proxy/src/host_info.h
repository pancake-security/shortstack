#ifndef HOST_INFO_H
#define HOST_INFO_H

#include <string>
#include <vector>

const int HOST_TYPE_L1 = 0;
const int HOST_TYPE_L2 = 1;
const int HOST_TYPE_L3 = 2;
const int HOST_TYPE_KV = 3;

struct host {
  std::string instance_name;
  int type;
  std::string hostname;
  int port;
};

// CSV Schema: instance_name, instance_type, hostname, port

class host_info {

public:
  bool load(std::string filename);
  bool get_hostname(const std::string &instance_name, std::string &hostname);
  bool get_port(const std::string &instance_name, int &port);

  void get_hosts_by_type(int type, std::vector<host> &hosts);

private:
  std::vector<host> hosts_;
};

#endif // HOST_INFO_H