#ifndef INSOMNIA_INDEX_POOL_H
#define INSOMNIA_INDEX_POOL_H
#include <fstream>
#include <filesystem>
#include "vector.h"

namespace insomnia {

using index_t = int;
inline constexpr index_t NULL_INDEX = 0;

// Guarantee that never alloc NULL_INDEX.
class IndexPool {
public:
  explicit IndexPool(const std::filesystem::path &path);
  ~IndexPool();
  index_t alloc();
  void dealloc(index_t index); // no solid validness check here.
  index_t max_index() const { return max_index_; }
  std::filesystem::path path() const { return path_; }
private:
  void load();
  void save();
  vector<index_t> unallocated_;
  index_t max_index_;
  std::fstream fstream_;
  std::filesystem::path path_;
};

}



#endif