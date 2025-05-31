#ifndef INSOMNIA_BUFFER_POOL_H
#define INSOMNIA_BUFFER_POOL_H

#include "fstream.h"
#include "lru_k_replacer.h"

namespace insomnia {

// According to the document of LruKReplacer,
// replacer_k_arg is recommended to be power of 2.
template <class T, class Meta = MonoType, size_t max_size = sizeof(T)> requires (max_size >= sizeof(T))
class BufferPool {
public:
  using frame_id_t = LruKReplacer::access_id_t;
private:
  using fstream_t = fstream<SectorWrapper<T, max_size>, SectorWrapper<Meta>>;
  struct Frame {

    explicit Frame(frame_id_t _frame_id)
      : frame_id(_frame_id), pin_count(0), is_dirty(false), is_valid(false) {}

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
  public:

    Visitor() : frame_(nullptr), fs_(nullptr), replacer_(nullptr) {}

    Visitor(Frame *frame, fstream_t *fs, LruKReplacer *replacer)
      : frame_(frame), fs_(fs), replacer_(replacer) {
      replacer->access(frame->frame_id);
      if(frame->pin_count == 0)
        replacer->pin(frame->frame_id);
      ++frame->pin_count;
    }

    // Actually no need in single thread... whatever.
    Visitor(const Visitor &) = delete;
    Visitor& operator=(const Visitor &) noexcept = delete;

    Visitor(Visitor &&other)
      : frame_(other.frame_), fs_(other.fs_), replacer_(other.replacer_) {
      other.frame_ = nullptr;
      other.fs_ = nullptr;
      other.replacer_ = nullptr;
    }

    Visitor& operator=(Visitor &&other) noexcept {
      if(this == &other)
        return *this;
      drop();
      frame_ = other.frame_;       other.frame_ = nullptr;
      fs_ = other.fs_;             other.fs_ = nullptr;
      replacer_ = other.replacer_; other.replacer_ = nullptr;
      return *this;
    }

    ~Visitor() { drop(); }

    void flush() {
      if(frame_ == nullptr)
        throw invalid_page("Buffer pool error: Flushing invalid visitor.");
      if(frame_->is_dirty) {
        fs_->write(frame_->page_id, &frame_->data_wrapper);
        frame_->is_dirty = false;
      }
    }

    void drop() {
      if(frame_ == nullptr)
        return;
      --frame_->pin_count;
      if(frame_->pin_count == 0)
        replacer_->unpin(frame_->frame_id);
      frame_ = nullptr;
      fs_ = nullptr;
      replacer_ = nullptr;
    }

    bool is_valid() const { return frame_ != nullptr; }

    page_id_t page_id() const { return frame_->page_id; }

    char* data() { return frame_->data(); }
    const char* data() const { return frame_->data(); }

    template <class Derived> requires (std::derived_from<Derived, T> && (max_size >= sizeof(Derived)))
    const Derived* as() {
      if(frame_ == nullptr)
        throw invalid_page("Buffer pool error: Using invalid visitor");
      return reinterpret_cast<const Derived*>(frame_->data());
    }

    template <class Derived> requires (std::derived_from<Derived, T> && (max_size >= sizeof(Derived)))
    Derived* as_mut() {
      if(frame_ == nullptr)
        throw invalid_page("Buffer pool error: Using invalid visitor");
      frame_->is_dirty = true;
      return reinterpret_cast<Derived*>(frame_->data());
    }

  private:
    Frame *frame_;
    fstream_t *fs_;
    LruKReplacer *replacer_;
  };

  void write_meta(const Meta *meta) requires (!EmptyMeta<Meta>) {
    memcpy(meta_wrapper_.data(), meta, sizeof(Meta));
    fs_.write_meta(&meta_wrapper_);
  }

  bool read_meta(Meta *meta) requires (!EmptyMeta<Meta>) {
    if(!fs_.read_meta(&meta_wrapper_))
      return false;
    memcpy(meta, meta_wrapper_.data(), sizeof(Meta));
    return true;
  }

  class DefaultVisitor {
  public:
    DefaultVisitor(page_id_t page_id, fstream_t *fs) : page_id_(page_id), is_dirty_(false), fs_(fs) {
      fs_->read(page_id, &data_);
    }
    ~DefaultVisitor() { drop(); }
    DefaultVisitor(const DefaultVisitor&) = delete;
    DefaultVisitor(DefaultVisitor&&) = default;
    DefaultVisitor& operator=(const DefaultVisitor&) = delete;
    DefaultVisitor& operator=(DefaultVisitor&&) = default;

    void flush() {
      if(is_dirty_) {
        fs_->write(page_id_, &data_);
        is_dirty_ = false;
      }
    }
    void drop() { flush(); }

    char* data() { return data_.data(); }
    const char* data() const { return data_.data(); }
    template <class Derived> requires (std::derived_from<Derived, T> && (max_size >= sizeof(Derived)))
    const Derived* as() {
      return reinterpret_cast<const Derived*>(data_.data());
    }

    template <class Derived> requires (std::derived_from<Derived, T> && (max_size >= sizeof(Derived)))
    Derived* as_mut() {
      is_dirty_ = true;
      return reinterpret_cast<Derived*>(data_.data());
    }

  private:
    page_id_t page_id_;
    SectorWrapper<T, max_size> data_;
    bool is_dirty_;
    fstream_t *fs_;
  };

  BufferPool(const std::filesystem::path &path, frame_id_t frame_cnt, int replacer_k_arg);

  ~BufferPool() { flush_all(); }

  page_id_t alloc() { return fs_.alloc(); }

  void dealloc(page_id_t page_id);

  Visitor visitor(page_id_t page_id);

  void flush_page(page_id_t page_id) {
    if(auto it = usage_map_.find(page_id); it != usage_map_.end()) {
      frame_id_t frame_id = it->second;
      flush_frame(frames_[frame_id]);
    }
  }

  void flush_all() {
    // since no concurrency involved, even a pinned frame can be flushed.
    for(frame_id_t i = 0; i < frame_count_; ++i)
      if(frames_[i].is_valid)
        flush_frame(frames_[i]);
  }

  void clear();

private:

  void flush_frame(Frame &frame) {
    if(!frame.is_valid)
      throw invalid_page("Buffer pool error: Flushing invalid frame.");
    if(frame.is_dirty) {
      fs_.write(frame.page_id, &frame.data_wrapper);
      frame.is_dirty = false;
    }
  }

  const std::filesystem::path path_;
  const int frame_count_;
  vector<Frame> frames_;

  unordered_map<page_id_t, frame_id_t> usage_map_;
  vector<frame_id_t> free_frames_;

  fstream_t fs_;
  LruKReplacer replacer_;

  [[no_unique_address]] SectorWrapper<Meta> meta_wrapper_;
};

}

#include "buffer_pool.tcc"

#endif