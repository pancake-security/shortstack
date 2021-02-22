#ifndef DISTRIBUTION_INFO_H
#define DISTRIBUTION_INFO_H

#include <unordered_map>

#include "distribution.h"

class distribution_info {

public:

  void load(std::string filename);
  void dump(std::string filename);

  int num_keys_; // Number of real keys
  std::string dummy_key_;
  std::unordered_map<std::string, int>
      key_to_number_of_replicas_; // Includes dummy key
  distribution fake_distribution_;
  distribution real_distribution_;
  std::unordered_map<std::string, int> replica_to_label_;
};

#endif // DISTRIBUTION_INFO_H