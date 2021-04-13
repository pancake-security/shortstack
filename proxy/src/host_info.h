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
  int row;
  int column;
  int num_workers;
};

// CSV Schema: instance_name, instance_type, hostname, port, row, column, num_workers

class host_info {

public:
  bool load(std::string filename);
  bool get_hostname(const std::string &instance_name, std::string &hostname);
  bool get_port(const std::string &instance_name, int &port);

  bool get_host(const std::string &instance_name, host &out);
  bool get_base_idx(const std::string &instance_name, int &idx);
  void get_hosts_by_type(int type, std::vector<host> &hosts);
  void get_replicas(int type, int column, std::vector<host> &replicas);

  int get_num_columns(int type, bool count_workers);

private:
  std::vector<host> hosts_;
};

#endif // HOST_INFO_H