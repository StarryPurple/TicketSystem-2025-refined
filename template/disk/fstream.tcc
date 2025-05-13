#ifndef INSOMNIA_FSTREAM_TCC
#define INSOMNIA_FSTREAM_TCC

#include "fstream.h"

namespace insomnia {

template <class T, class Meta> requires FstreamConcept<T, Meta>
fstream<T, Meta>::fstream(const std::filesystem::path &path)
    : path_(path), index_allocator_(path.string() + ".idx") {
  bool file_exists = std::filesystem::exists(path);
  auto open_mode = std::ios::binary | std::ios::in | std::ios::out;
  if(!file_exists)
    open_mode |= std::ios::trunc;
  fstream_.open(path, open_mode);
  if(!fstream_.is_open())
    throw invalid_pool(std::string("FStream failed to construct. Path: " + path.string()).c_str());
  file_size_ = std::filesystem::file_size(path);
}

template <class T, class Meta> requires FstreamConcept<T, Meta>
fstream<T, Meta>::~fstream() {
  if(fstream_.is_open())
    fstream_.close();
}

template <class T, class Meta> requires FstreamConcept<T, Meta>
bool fstream<T, Meta>::read(page_id_t pos, T *data) {
  if(pos <= NULL_PAGE_ID)
    throw pool_exception(std::string("Accessing invalid page. Correlated file: " + path_.string()).c_str());
  size_t offset = SIZE_META + (pos - NULL_PAGE_ID - 1) * SIZE_T;
  size_t required_size = offset + SIZE_T;
  if(required_size > file_size_)
    return false;
  fstream_.seekg(offset);
  fstream_.read(reinterpret_cast<char*>(data), SIZE_T);
  return true;
}

template <class T, class Meta> requires FstreamConcept<T, Meta>
void fstream<T, Meta>::write(page_id_t pos, const T *data) {
  if(pos <= NULL_PAGE_ID)
    throw pool_exception(std::string("Accessing invalid page. Correlated file: " + path_.string()).c_str());
  size_t offset = SIZE_META + (pos - NULL_PAGE_ID - 1) * SIZE_T;
  size_t required_size = offset + SIZE_T;
  if(required_size > file_size_)
    reserve(required_size + (pos - NULL_PAGE_ID - 1) * SIZE_T);
  fstream_.seekp(offset);
  fstream_.write(reinterpret_cast<const char*>(data), SIZE_T);
}

template <class T, class Meta> requires FstreamConcept<T, Meta>
bool fstream<T, Meta>::read_meta(Meta *data) requires (!EmptyMeta<Meta>) {
  size_t required_size = SIZE_META;
  if(required_size > file_size_)
    return false;
  fstream_.seekg(0);
  fstream_.read(reinterpret_cast<char*>(data), sizeof(Meta));
  return true;
}

template <class T, class Meta> requires FstreamConcept<T, Meta>
void fstream<T, Meta>::write_meta(const Meta *data) requires (!EmptyMeta<Meta>) {
  size_t required_size = SIZE_META;
  if(required_size > file_size_)
    reserve(required_size);
  fstream_.seekp(0);
  fstream_.write(reinterpret_cast<const char*>(data), SIZE_META);
}

template <class T, class Meta> requires FstreamConcept<T, Meta>
void fstream<T, Meta>::reserve(size_t required_size) {
  if(required_size <= file_size_)
    return;
  fstream_.close();
  std::filesystem::resize_file(path_, required_size);
  /*
  {
    std::ofstream temp(path_, std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
    temp.seekp(required_size - 1);
    temp.put('0');
  }
  */
  fstream_.open(path_, std::ios::binary | std::ios::in | std::ios::out);
  file_size_ = required_size;
}

}

#endif