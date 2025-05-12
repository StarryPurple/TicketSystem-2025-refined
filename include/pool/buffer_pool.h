#ifndef INSOMNIA_BUFFER_POOL_H
#define INSOMNIA_BUFFER_POOL_H

#include "fstream.h"
#include "lru_k_replacer.h"

namespace insomnia {

// According to the document of LruKReplacer,
// replacer_k_arg is recommended to be power of 2.
template <class T, class Meta = void, size_t max_size = sizeof(T)> requires (max_size >= sizeof(T))
class BufferPool {
public:
  using frame_id_t = LruKReplacer::access_id_t;
private:
  using fstream_t = fstream<SectorWrapper<T, max_size>, SectorWrapper<Meta>>;
  struct Frame {

    explicit Frame(frame_id_t _frame_id) : frame_id(_frame_id), pin_count(0), is_dirty(false), is_valid(false) {}

    T* data() { return data_wrapper.data(); }

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

    Visitor() : frame_(nullptr), fs_(nullptr) {}

    Visitor(Frame *frame, fstream_t *fs)
      : frame_(frame), fs_(fs) {
      ++frame_->pin_count;
    }

    // Actually no need in single thread... whatever.
    Visitor(const Visitor &) = delete;
    Visitor& operator=(const Visitor &) noexcept = delete;

    Visitor(Visitor &&other)
      : frame_(other.frame_), fs_(other.fs_) {
      other.frame_ = nullptr;
      other.fs_ = nullptr;
    }

    Visitor& operator=(Visitor &&other) noexcept {
      if(this == &other)
        return *this;
      drop();
      frame_ = other.frame_;
      fs_ = other.fs_;
      other.frame_ = nullptr;
      other.fs_ = nullptr;
      return *this;
    }

    ~Visitor() { drop(); }

    void flush() {
      if(frame_ == nullptr)
        throw pool_exception("Buffer pool error: Flushing invalid visitor.");
      if(frame_->is_dirty) {
        fs_->write(frame_->page_id, &frame_->data_wrapper);
        frame_->is_dirty = false;
      }
    }

    void drop() {
      if(frame_ == nullptr)
        return;
      --frame_->pin_count;
      frame_ = nullptr;
      fs_ = nullptr;
    }

    template <class Derived> requires (std::derived_from<Derived, T> && (max_size >= sizeof(Derived)))
    const Derived* as() {
      if(frame_ == nullptr)
        throw pool_exception("Buffer pool error: Using invalid visitor");
      return dynamic_cast<const Derived*>(frame_->data());
    }

    template <class Derived> requires (std::derived_from<Derived, T> && (max_size >= sizeof(Derived)))
    Derived* as_mut() {
      if(frame_ == nullptr)
        throw pool_exception("Buffer pool error: Using invalid visitor");
      frame_->is_dirty = true;
      return dynamic_cast<Derived*>(frame_->data());
    }

  private:
    Frame *frame_;
    fstream_t *fs_;
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

  BufferPool(const std::filesystem::path &path, int frame_cnt, int replacer_k_arg)
    : path_(path), frame_count_(frame_cnt), replacer_(frame_count_, replacer_k_arg),
      fs_(path.string() + ".dat") {
    frames_.reserve(frame_cnt);
    free_frames_.reserve(frame_cnt);
    for(frame_id_t i = 0; i < frame_cnt; ++i) {
      frames_.push_back(Frame(i));
      free_frames_.push_back(i);
    }
  }

  ~BufferPool() { flush_all(); }

  page_id_t alloc() { return fs_.alloc(); }

  void dealloc(page_id_t page_id) {
    if(auto it = usage_map_.find(page_id); it != usage_map_.end()) {
      frame_id_t frame_id = it->second;
      if(frames_[frame_id].pin_count > 0)
        throw pool_exception("Buffer pool error : Freeing pages in use.");
      frames_[frame_id].is_valid = false;
      free_frames_.push_back(frame_id);
    }
    fs_.dealloc(page_id);
  }

  Visitor visitor(page_id_t page_id) {
    frame_id_t frame_id;
    if(auto it = usage_map_.find(page_id); it != usage_map_.end()) {
      frame_id = it->second;
      return Visitor(frames_ + frame_id, &fs_);
    }
    if(!free_frames_.empty()) {
      frame_id = free_frames_.back();
      free_frames_.pop_back();
      frames_[frame_id].is_valid = true;
    } else {
      if(!replacer_.can_evict()) {
        // You can design a coarse visitor which interacts with disk memory
        // directly through fstream.
        throw pool_overflow("Buffer pool full");
      }
      frame_id = replacer_.evict();
      flush_frame(frames_[frame_id]);
      usage_map_.erase(frames_[frame_id].page_id);
    }
    frames_[frame_id].page_id = page_id;
    usage_map_.emplace(page_id, frame_id);
    fs_.read(page_id, &frames_[frame_id].data_wrapper);
    return Visitor(frames_ + frame_id, &fs_);
  }

  void flush_page(page_id_t page_id) {
    if(auto it = usage_map_.find(page_id); it != usage_map_.end()) {
      frame_id_t frame_id = it->second;
      flush_frame(frames_[frame_id]);
    }
  }

  void flush_all() {
    // since no concurrency involved, even a pinned frame can be flushed.
    for(frame_id_t i = 0; i < frame_count_; ++i)
      flush_frame(frames_[i]);
  }

private:

  void flush_frame(Frame &frame) {
    if(!frame.is_valid)
      throw pool_exception("Buffer pool error: Flushing invalid frame.");
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