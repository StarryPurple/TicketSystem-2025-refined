#ifndef INSOMNIA_LRU_K_REPLACER_H
#define INSOMNIA_LRU_K_REPLACER_H

#include "unordered_map.h"
#include "priority_queue.h"

namespace insomnia {

// Access id must be lower than capacity.
// We recommend you to use k that is power of 2.
class LruKReplacer {
  using time_t = int;
private:
  class HistoryT {
  public:
    HistoryT(int k) : k_(k), history_(new time_t[k]), pinned_(false), vis_count_(0) {}
    ~HistoryT() { delete[] history_; }
    void reactivate() {
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
    time_t dist() const {
      return is_hotspot() ? history_[vis_count_ % k_] : history_[0];
    }
  private:
    const int k_;
    time_t *history_;
    bool pinned_;
    time_t vis_count_;
  };
public:
  using access_id_t = int;
  explicit LruKReplacer(access_id_t capacity, int k = 2) : capacity_(capacity), k_(k), time_(0) {}
  void access(access_id_t access_id);
  bool can_evict() const;
  access_id_t evict(); // returns capacity of the replacer(invalid) if no eviction can be performed.
  bool remove();
  void pin();
  void unpin();
private:
  const access_id_t capacity_;
  const int k_;
  time_t time_;
  HistoryT *slots_;
  unordered_map<access_id_t, int> l0_map_, l1_map_;
  // priority_queue<>
};

}

#endif