#ifndef INSOMNIA_BUFFER_POOL_TCC
#define INSOMNIA_BUFFER_POOL_TCC

#include "buffer_pool.h"

namespace insomnia {

/********* BufferPool **********/

template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
BufferPool<T, Meta, max_size>::BufferPool(const std::filesystem::path &path, frame_id_t frame_cnt, int replacer_k_arg)
    : path_(path), frame_count_(frame_cnt), replacer_(frame_count_, replacer_k_arg),
      fs_(path.string() + ".dat") {
  frames_.reserve(frame_cnt);
  free_frames_.reserve(frame_cnt);
  for(frame_id_t i = 0; i < frame_cnt; ++i) {
    frames_.push_back(Frame(i));
    free_frames_.push_back(i);
  }
}

template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
BufferPool<T, Meta, max_size>::Visitor::Visitor(Frame *frame, fstream_t *fs, LruKReplacer *replacer)
: frame_(frame), fs_(fs), replacer_(replacer) {
  replacer->access(frame->frame_id);
  if(frame->pin_count == 0)
    replacer->pin(frame->frame_id);
  ++frame->pin_count;
}

template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
BufferPool<T, Meta, max_size>::Visitor::Visitor(Visitor &&other)
: frame_(other.frame_), fs_(other.fs_), replacer_(other.replacer_) {
  other.frame_ = nullptr;
  other.fs_ = nullptr;
  other.replacer_ = nullptr;
}

template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
typename BufferPool<T, Meta, max_size>::Visitor&
  BufferPool<T, Meta, max_size>::Visitor::operator=(Visitor &&other) noexcept {

  if(this == &other)
    return *this;
  drop();
  frame_ = other.frame_;       other.frame_ = nullptr;
  fs_ = other.fs_;             other.fs_ = nullptr;
  replacer_ = other.replacer_; other.replacer_ = nullptr;
  return *this;
}

template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
void BufferPool<T, Meta, max_size>::Visitor::flush() {
  if(frame_ == nullptr)
    throw invalid_page("Buffer pool error: Flushing invalid visitor.");
  if(frame_->is_dirty) {
    fs_->write(frame_->page_id, &frame_->data_wrapper);
    frame_->is_dirty = false;
  }
}

template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
void BufferPool<T, Meta, max_size>::Visitor::drop() {
  if(frame_ == nullptr)
    return;
  --frame_->pin_count;
  if(frame_->pin_count == 0)
    replacer_->unpin(frame_->frame_id);
  frame_ = nullptr;
  fs_ = nullptr;
  replacer_ = nullptr;
}

template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
template <class Derived> requires (std::derived_from<Derived, T> && (max_size >= sizeof(Derived)))
const Derived* BufferPool<T, Meta, max_size>::Visitor::as() const {
  if(frame_ == nullptr)
    throw invalid_page("Buffer pool error: Using invalid visitor");
  return reinterpret_cast<const Derived*>(frame_->data());
}

template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
template <class Derived> requires (std::derived_from<Derived, T> && (max_size >= sizeof(Derived)))
Derived* BufferPool<T, Meta, max_size>::Visitor::as_mut() {
  if(frame_ == nullptr)
    throw invalid_page("Buffer pool error: Using invalid visitor");
  frame_->is_dirty = true;
  return reinterpret_cast<Derived*>(frame_->data());
}

template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
void BufferPool<T, Meta, max_size>::write_meta(const Meta *meta) requires (!EmptyMeta<Meta>) {
  memcpy(meta_wrapper_.data(), meta, sizeof(Meta));
  fs_.write_meta(&meta_wrapper_);
}

template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
bool BufferPool<T, Meta, max_size>::read_meta(Meta *meta) requires (!EmptyMeta<Meta>) {
  if(!fs_.read_meta(&meta_wrapper_))
    return false;
  memcpy(meta, meta_wrapper_.data(), sizeof(Meta));
  return true;
}

template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
void BufferPool<T, Meta, max_size>::dealloc(page_id_t page_id) {
  if(auto it = usage_map_.find(page_id); it != usage_map_.end()) {
    frame_id_t frame_id = it->second;
    if(frames_[frame_id].pin_count > 0)
      throw pool_exception("Buffer pool error : Freeing pages in use.");
    frames_[frame_id].is_valid = false;
    free_frames_.push_back(frame_id);
    usage_map_.erase(it);
  }
  fs_.dealloc(page_id);
}

template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
typename BufferPool<T, Meta, max_size>::Visitor
BufferPool<T, Meta, max_size>::visitor(page_id_t page_id) {
  // return DefaultVisitor(page_id, &fs_);
  frame_id_t frame_id;
  if(auto it = usage_map_.find(page_id); it != usage_map_.end()) {
    frame_id = it->second;
    return Visitor(&frames_[frame_id], &fs_, &replacer_);
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
  return Visitor(&frames_[frame_id], &fs_, &replacer_);
}

/*
template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
void BufferPool<T, Meta, max_size>::flush_page(page_id_t page_id) {
  if(auto it = usage_map_.find(page_id); it != usage_map_.end()) {
    frame_id_t frame_id = it->second;
    flush_frame(frames_[frame_id]);
  }
}
*/

template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
void BufferPool<T, Meta, max_size>::flush_all() {
  // since no concurrency involved, even a pinned frame can be flushed.
  for(frame_id_t i = 0; i < frame_count_; ++i)
    if(frames_[i].is_valid)
      flush_frame(frames_[i]);
}

template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
void BufferPool<T, Meta, max_size>::clear() {
  fs_.clear();
  usage_map_.clear();
  frames_.clear();
  free_frames_.clear();
  for(frame_id_t i = 0; i < frame_count_; ++i) {
    frames_.push_back(Frame(i));
    free_frames_.push_back(i);
  }
  replacer_.clear();
}

template <class T, class Meta, size_t max_size> requires (max_size >= sizeof(T))
void BufferPool<T, Meta, max_size>::flush_frame(Frame &frame) {
  if(!frame.is_valid)
    throw invalid_page("Buffer pool error: Flushing invalid frame.");
  if(frame.is_dirty) {
    fs_.write(frame.page_id, &frame.data_wrapper);
    frame.is_dirty = false;
  }
}


/**** CompressedBufferPool ****/


}

#endif