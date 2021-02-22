#include "MurmurHash2.h"
#include <stdint.h>

#include "consistent_hash.h"

int32_t JumpConsistentHash(uint64_t key, int32_t num_buckets) {
  int64_t b = 1, j = 0;
  while (j < num_buckets) {
    b = j;
    key = key * 2862933555777941757ULL + 1; // Did you say magic number?
    j = (b + 1) * (double(1LL << 31) / double((key >> 33) + 1));
  }
  return b;
}

int consistent_hash(const std::string &str, int num_buckets) {
  return JumpConsistentHash(MurmurHash64A(str.data(), str.length(), 1995),
                            num_buckets);
}