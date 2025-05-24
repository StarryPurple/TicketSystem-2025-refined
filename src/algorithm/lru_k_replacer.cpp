#include "lru_k_replacer.h"

namespace insomnia {

void LruKReplacer::access(access_id_t access_id) {
  ++time_;
  if(l0_set_.contains(access_id)) {
    slots_[access_id].access(time_);
    return;
  }
  if(l1_set_.contains(access_id)) {
    slots_[access_id].access(time_);
    if(slots_[access_id].is_hotspot()) {
      l1_set_.erase(access_id);
      l0_set_.insert(access_id);
    }
    return;
  }
  slots_[access_id].reset();
  slots_[access_id].access(time_);
  l1_set_.insert(access_id);
  ++size_;
}

LruKReplacer::access_id_t LruKReplacer::evict() {
  if(!can_evict())
    throw algorithm_exception("Lru k replacer can't evict anything.");
  if(!l1_set_.empty()) {
    access_id_t evict_id = capacity_;
    time_t k_time = time_ + 1;
    for(const auto &access_id : l1_set_) {
      if(slots_[access_id].is_pinned())
        continue;
      int time = slots_[access_id].k_time();
      if(time < k_time) {
        evict_id = access_id;
        k_time = time;
      }
    }
    if(evict_id != capacity_) {
      l1_set_.erase(evict_id);
      --size_;
      return evict_id;
    }
  }
  if(!l0_set_.empty()) {
    access_id_t evict_id = capacity_;
    time_t k_time = time_ + 1;
    for(const auto &access_id : l0_set_) {
      if(slots_[access_id].is_pinned())
        continue;
      int time = slots_[access_id].k_time();
      if(time < k_time) {
        evict_id = access_id;
        k_time = time;
      }
    }
    if(evict_id != capacity_) {
      l0_set_.erase(evict_id);
      --size_;
      return evict_id;
    }
  }
  throw algorithm_exception("Invalid code. Please check the code logic.");
  return capacity_; // Shouldn't reach here.
}

bool LruKReplacer::remove(access_id_t access_id) {
  if(l0_set_.contains(access_id)) {
    if(slots_[access_id].is_pinned())
      return false;
    l0_set_.erase(access_id);
    --size_;
    return true;
  }
  if(l1_set_.contains(access_id)) {
    if(slots_[access_id].is_pinned())
      return false;
    l1_set_.erase(access_id);
    --size_;
    return true;
  }
  return false;
}


bool LruKReplacer::pin(access_id_t access_id) {
  if(l0_set_.contains(access_id)) {
    if(!slots_[access_id].is_pinned()) {
      --size_;
      slots_[access_id].pin();
    }
    return true;
  }
  if(l1_set_.contains(access_id)) {
    if(!slots_[access_id].is_pinned()) {
      --size_;
      slots_[access_id].pin();
    }
    return true;
  }
  return false;
}

bool LruKReplacer::unpin(access_id_t access_id) {
  if(l0_set_.contains(access_id)) {
    if(slots_[access_id].is_pinned()) {
      ++size_;
      slots_[access_id].unpin();
    }
    return true;
  }
  if(l1_set_.contains(access_id)) {
    if(slots_[access_id].is_pinned()) {
      ++size_;
      slots_[access_id].unpin();
    }
    return true;
  }
  return false;
}

void LruKReplacer::clear() {
  size_ = 0;
  time_ = 0;
  for(auto &slot : slots_)
    slot.reset();
  l0_set_.clear(); l1_set_.clear();
}


}