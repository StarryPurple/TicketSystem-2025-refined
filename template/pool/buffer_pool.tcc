#ifndef INSOMNIA_BUFFER_POOL_TCC
#define INSOMNIA_BUFFER_POOL_TCC

#include "buffer_pool.h"

namespace insomnia {

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
typename BufferPool<T, Meta, max_size>::Visitor BufferPool<T, Meta, max_size>::visitor(page_id_t page_id) {
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


}

#endif