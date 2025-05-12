#ifndef INSOMNIA_FSTREAM_H
#define INSOMNIA_FSTREAM_H

#include "index_pool.h"

namespace insomnia {

inline constexpr size_t SECTOR_SIZE = 4096; // 512 in some regions/nations... maybe imagination?

template <class T>
concept SectorAligned = sizeof(T) % SECTOR_SIZE == 0;

template <class T, class Meta>
concept FstreamConcept =
  SectorAligned<T> &&
  (std::is_same_v<Meta, void> || SectorAligned<Meta>);

// Hard coded under the fact that NULL_INDEX = 0.
template <class T, class Meta = void>
requires FstreamConcept<T, Meta>
class fstream {
  static constexpr size_t SIZE_T = sizeof(T);
  static constexpr size_t SIZE_META = std::is_same_v<Meta, void> ? 0 : sizeof(Meta);
public:
  explicit fstream(const std::filesystem::path &path);
  ~fstream();
  void write(index_t pos, const T *data);
  bool read(index_t pos, T *data); // returns false if read failed.
  void write_meta(const Meta *data) requires (!std::is_same_v<Meta, void>);
  bool read_meta(Meta *data) requires (!std::is_same_v<Meta, void>); // returns false if read failed.
  std::filesystem::path path() const { return path_; }
  std::filesystem::path index_path() const { return index_pool_.path(); }
private:
  void reserve(size_t required_size);
  IndexPool index_pool_;
  std::fstream fstream_;
  std::filesystem::path path_;
  size_t file_size_;
};

}

#include "fstream.tcc"

#endif