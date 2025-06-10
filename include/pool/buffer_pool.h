#ifndef INSOMNIA_BUFFER_POOL_H
#define INSOMNIA_BUFFER_POOL_H

#include "fstream.h"
#include "lru_k_replacer.h"

namespace insomnia {

using frame_id_t = LruKReplacer::access_id_t;

// According to the document of LruKReplacer,
// replacer_k_arg is recommended to be power of 2.
template <class T, class Meta = MonoType, size_t max_size = sizeof(T)> requires (max_size >= sizeof(T))
class BufferPool {
public:
  class Visitor;

  BufferPool(const std::filesystem::path &path, frame_id_t frame_cnt, int replacer_k_arg);
  ~BufferPool() { flush_all(); }

private:
  using fstream_t = fstream<SectorWrapper<T, max_size>, SectorWrapper<Meta>>;
  class Frame {
    friend Visitor;
    friend BufferPool;
  public:
    explicit Frame(frame_id_t _frame_id)
      : frame_id(_frame_id), pin_count(0), is_dirty(false), is_valid(false) {}
  private:
    char* data() { return data_wrapper.data(); }

    const frame_id_t frame_id;
    index_t page_id;
    size_t pin_count; // only to avoid halfway drop.
    bool is_dirty;
    bool is_valid;
    SectorWrapper<T, max_size> data_wrapper;
  };

public:
  class Visitor {
    // validness checked by frame_ == nullptr.
    friend BufferPool;
  public:
    Visitor() : frame_(nullptr), fs_(nullptr), replacer_(nullptr) {}
    // Actually no need in single thread... whatever.
    Visitor(const Visitor &) = delete;
    Visitor& operator=(const Visitor &) noexcept = delete;
    Visitor(Visitor &&other);
    Visitor& operator=(Visitor &&other) noexcept;
    ~Visitor() { drop(); }
    void flush();
    void drop();

    bool is_valid() const { return frame_ != nullptr; }
    page_id_t page_id() const { return frame_->page_id; }
    char* data() { return frame_->data(); }
    const char* data() const { return frame_->data(); }

    template <class Derived> requires (std::derived_from<Derived, T> && (max_size >= sizeof(Derived)))
    const Derived* as() const;
    template <class Derived> requires (std::derived_from<Derived, T> && (max_size >= sizeof(Derived)))
    Derived* as_mut();

  private:
    Visitor(Frame *frame, fstream_t *fs, LruKReplacer *replacer);

    Frame *frame_;
    fstream_t *fs_;
    LruKReplacer *replacer_;
  };

  void write_meta(const Meta *meta) requires (!EmptyMeta<Meta>);
  bool read_meta(Meta *meta) requires (!EmptyMeta<Meta>);

  page_id_t alloc() { return fs_.alloc(); }
  void dealloc(page_id_t page_id);
  page_id_t max_page_id() { return fs_.max_page_id(); }

  Visitor visitor(page_id_t page_id);

  // void flush_page(page_id_t page_id);
  void flush_all();

  void clear();

private:
  void flush_frame(Frame &frame);

  const std::filesystem::path path_;
  const int frame_count_;
  vector<Frame> frames_;
  unordered_map<page_id_t, frame_id_t> usage_map_;
  vector<frame_id_t> free_frames_;
  fstream_t fs_;
  LruKReplacer replacer_;
  [[no_unique_address]] SectorWrapper<Meta> meta_wrapper_;
};

// no disk space recycle implemented.
template <class T>
class CompressedBufferPool {

  static constexpr size_t CAPACITY = SectorAlignedSize(sizeof(T)) / sizeof(T);

  struct Storage {
    T data[CAPACITY];
  };

  using BufferType = BufferPool<Storage, index_t>;

public:

  CompressedBufferPool(const std::filesystem::path &path, frame_id_t frame_cnt, int replacer_k_arg)
  : buffer_pool_(path.string() + "_comp", frame_cnt, replacer_k_arg) {
    if(!buffer_pool_.read_meta(&max_index_))
      max_index_ = NULL_INDEX;
  }
  ~CompressedBufferPool() { buffer_pool_.write_meta(&max_index_); }

  class Visitor {
    friend CompressedBufferPool;
  public:
    Visitor() = default;
    // Actually no need in single thread... whatever.
    Visitor(const Visitor &) = delete;
    Visitor& operator=(const Visitor &) noexcept = delete;
    Visitor(Visitor &&other) = default;
    Visitor& operator=(Visitor &&other) noexcept = default;
    ~Visitor() { drop(); }
    void flush() { visitor_.flush(); }
    void drop() { visitor_.drop(); pos_ = 0; }

    bool is_valid() const { return visitor_.is_valid(); }
    page_id_t index_id() const { return (visitor_.page_id() - 1) * CAPACITY + pos_ + 1; }

    const T* as() const { return &visitor_.template as<Storage>()->data[pos_]; }
    T* as_mut() { return &visitor_.template as_mut<Storage>()->data[pos_]; }

  private:
    Visitor(typename BufferType::Visitor &&visitor, int pos)
    : visitor_(std::move(visitor)), pos_(pos) {}

    typename BufferType::Visitor visitor_;
    int pos_;
  };

  index_t alloc() {
    if(max_index_ % CAPACITY == 0)
      buffer_pool_.alloc();
    return ++max_index_;
  }
  Visitor visitor(index_t index) {
    if(index == NULL_INDEX) throw pool_exception("Using invalid index");
    return Visitor(buffer_pool_.visitor((index - 1) / CAPACITY + 1), (index - 1) % CAPACITY);
  }

  // void flush_page(page_id_t page_id);
  // void flush_all();

  void clear() { buffer_pool_.clear(); max_index_ = NULL_INDEX; }

private:

  index_t max_index_;
  BufferType buffer_pool_;
};

}

#include "buffer_pool.tcc"

#endif