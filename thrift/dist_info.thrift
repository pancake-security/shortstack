// int num_keys_; // Number of real keys
//   std::string dummy_key_;
//   std::unordered_map<std::string, int>
//       key_to_number_of_replicas_; // Includes dummy key
//   distribution fake_distribution_;
//   distribution real_distribution_;
//   std::unordered_map<std::string, int> replica_to_label_;

struct dist_info {
  1: i32 num_keys,
  2: string dummy_key,
  3: map<string, i32> key_to_number_of_replicas,
  4: map<string, i32> replica_to_label,
  5: map<string, double> fake_distribution,
  6: map<string, double> real_distribution
}