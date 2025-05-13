#ifndef INSOMNIA_LRU_K_REPLACER_H
#define INSOMNIA_LRU_K_REPLACER_H

#include "vector.h"
#include "unordered_set.h"
#include "unordered_map.h"
#include "priority_queue.h"

namespace insomnia {

// Access id must be lower than capacity.
// We recommend you to use k that is power of 2.
class LruKReplacer {
public:
  using time_t = int;
  using access_id_t = int;
private:
  class Slot {
  public:
    Slot(int k) : k_(k), history_(k), pinned_(false), vis_count_(0) {}
    // some unnecessary memory alloc/dealloc might happen?
    ~Slot() = default;
    void reset() {
      pinned_ = false;
      vis_count_ = 0;
    }
    void access(time_t time) {
      history_[vis_count_ % k_] = time;
      ++vis_count_;
    }
    void pin() { pinned_ = true; }
    void unpin() { pinned_ = false; }
    bool is_pinned() const { return pinned_; }
    bool is_hotspot() const { return vis_count_ >= k_; }
    time_t k_time() const {
      return is_hotspot() ? history_[vis_count_ % k_] : history_[0];
    }
  private:
    const int k_;
    vector<time_t> history_;
    bool pinned_;
    time_t vis_count_;
  };
  class SlotCompare {
  public:
    explicit SlotCompare(vector<Slot> *slots) : slots_(slots) {}
    bool operator()(int a, int b) const {
      return (*slots_)[a].k_time() < (*slots_)[b].k_time();
    }
  private:
    vector<Slot> *slots_;
  };
public:
  explicit LruKReplacer(access_id_t capacity, int k = 2)
    : capacity_(capacity), k_(k), size_(0), time_(0), slots_(capacity, Slot(k)) {}
  void access(access_id_t access_id); // Attention: initially taken as pinned.
  bool can_evict() const { return size_ > 0; }
  access_id_t free_cnt() const { return size_; }
  access_id_t evict(); // returns capacity of the replacer(invalid) if no eviction can be performed.
  bool remove(access_id_t access_id);
  bool pin(access_id_t access_id);    // returns false if access id not in replacer.
  bool unpin(access_id_t access_id);  // returns false if access id not in replacer.
private:
  const access_id_t capacity_;
  const int k_;
  access_id_t size_;
  time_t time_;
  vector<Slot> slots_;
  unordered_set<access_id_t> l0_set_, l1_set_;
  /*
  unordered_set<access_id_t> l0_set_, l1_set_;         // which level is this access id in.
  unordered_map<access_id_t, time_t> l0_map_, l1_map_; // the k-dist of the access id.
  // the reverse table of the map above. (time_ is unique)
  unordered_map<time_t, unordered_map<access_id_t, time_t>::iterator> l0_rev_map_, l1_rev_map_;
  */
};

}

#endif