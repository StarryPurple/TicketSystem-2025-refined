#ifndef INSOMNIA_FSTREAM_H
#define INSOMNIA_FSTREAM_H

#include "index_pool.h"

namespace insomnia {

using page_id_t = index_t;

inline constexpr page_id_t NULL_PAGE_ID = NULL_INDEX;

inline constexpr size_t SECTOR_SIZE = 4096; // 512 in some regions/nations... maybe imagination?

template <class T>
concept SectorAligned =
    !std::is_void_v<T> ||
    (sizeof(T) % SECTOR_SIZE == 0);

template <class T, size_t max_size = sizeof(T)>
requires (max_size >= sizeof(T))
class SectorWrapper {
public:
  static constexpr size_t size() { return sizeof(data_); }
  T* data() { return reinterpret_cast<T*>(data_); }
private:
  alignas(SECTOR_SIZE) char data_[(max_size + SECTOR_SIZE - 1) / SECTOR_SIZE * SECTOR_SIZE] {};
};

template <size_t max_size>
class SectorWrapper<void, max_size> {
  static constexpr size_t size() { return 0; }
  void* data() = delete;
};

template <class T>
struct is_void_sector_wrapper : std::false_type {};

template <size_t max_size>
struct is_void_sector_wrapper<SectorWrapper<void, max_size>> : std::true_type {};

template <class T>
inline constexpr bool is_void_sector_wrapper_v = is_void_sector_wrapper<T>::value;

template <class Meta>
concept EmptyMeta =
    std::is_void_v<Meta> ||
    is_void_sector_wrapper_v<Meta>;

template <class T, class Meta>
concept FstreamConcept =
  SectorAligned<T> &&
  (EmptyMeta<Meta> || SectorAligned<Meta>);

// Hard coded under the fact that NULL_INDEX = 0.
template <class T, class Meta = void>
requires FstreamConcept<T, Meta>
class fstream {
  static constexpr size_t SIZE_T = sizeof(T);
  static constexpr size_t SIZE_META = EmptyMeta<Meta> ? 0 : sizeof(Meta);
public:
  explicit fstream(const std::filesystem::path &path);
  ~fstream();
  void write(page_id_t pos, const T *data);
  bool read(page_id_t pos, T *data); // returns false if read failed.
  void write_meta(const Meta *data) requires (!EmptyMeta<Meta>);
  bool read_meta(Meta *data) requires (!EmptyMeta<Meta>); // returns false if read failed.
  page_id_t alloc() { return index_allocator_.alloc(); }
  void dealloc(page_id_t page_id) { index_allocator_.dealloc(page_id); }
private:
  void reserve(size_t required_size);
  IndexPool index_allocator_;
  std::fstream fstream_;
  const std::filesystem::path path_;
  size_t file_size_;
};

}

#include "fstream.tcc"

#endif