#include "index_pool.h"

namespace insomnia {

IndexPool::IndexPool(const std::filesystem::path &path) : max_index_(NULL_INDEX), path_(path) {
  bool file_exists = std::filesystem::exists(path);
  auto open_mode = std::ios::binary | std::ios::in | std::ios::out;
  if(!file_exists)
    open_mode |= std::ios::trunc;
  fstream_.open(path, open_mode);
  if(!fstream_.is_open())
    throw invalid_pool(std::string("IndexPool failed to construct. Path: " + path.string()).c_str());
  if(file_exists && std::filesystem::file_size(path) > sizeof(index_t) + sizeof(size_t))
    load();
}

IndexPool::~IndexPool() {
  if(fstream_.is_open()) {
    save();
    fstream_.close();
  }
}

index_t IndexPool::alloc() {
  if(!unallocated_.empty()) {
    index_t index = unallocated_.back();
    unallocated_.pop_back();
    return index;
  }
  return ++max_index_;
}

void IndexPool::dealloc(index_t index) {
  /*
  if(index <= NULL_INDEX || index > max_index_)
    throw pool_exception("Deallocing definitely invalid index.");
  */
  unallocated_.push_back(index);
}

void IndexPool::load() {
  fstream_.seekg(0);
  fstream_.read(reinterpret_cast<char*>(&max_index_), sizeof(index_t));
  size_t unallocated_size;
  fstream_.read(reinterpret_cast<char*>(&unallocated_size), sizeof(size_t));
  unallocated_.resize(unallocated_size);
  fstream_.read(reinterpret_cast<char*>(unallocated_.data()), unallocated_size * sizeof(index_t));
}

void IndexPool::save() {
  fstream_.seekp(0);
  fstream_.write(reinterpret_cast<const char*>(&max_index_), sizeof(index_t));
  size_t unallocated_size = unallocated_.size();
  fstream_.write(reinterpret_cast<const char*>(&unallocated_size), sizeof(size_t));
  fstream_.write(reinterpret_cast<const char*>(unallocated_.data()), unallocated_size * sizeof(index_t));
  fstream_.flush();
}

}