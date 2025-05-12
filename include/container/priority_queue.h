#ifndef INSOMNIA_PRIORITY_QUEUE_H
#define INSOMNIA_PRIORITY_QUEUE_H

#include <functional>
#include <cmath>

namespace insomnia {

template <class T, class Compare = std::less<T>>
class priority_queue {
public:
  priority_queue(): _max_n(nullptr), _size(0), _comp(), ranks_(nullptr), ranks_size_(0) {}
  explicit priority_queue(Compare comp)
    : _max_n(nullptr), _size(0), _comp(std::move(comp)), ranks_(nullptr), ranks_size_(0) {}
  priority_queue(const priority_queue &other)
    : _size(other._size), _comp() {
    iterate_copy(_max_n, nullptr, other._max_n);
    ranks_ = nullptr;
    ranks_size_ = other.ranks_size_;
    if(ranks_size_ > 0)
      ranks_ = new Node*[ranks_size_];
  }
  priority_queue(priority_queue &&other) {
    _max_n = other._max_n; other._max_n = nullptr;
    _size = other._size; other._size = 0;
    ranks_ = other.ranks_; other.ranks_ = nullptr;
    ranks_size_ = other.ranks_size_; other.ranks_size_ = 0;
  }
  ~priority_queue() {
    clear();
    delete[] ranks_;
  }
  priority_queue& operator=(const priority_queue &other) {
    if(this == &other) return *this;
    clear();
    _size = other._size;
    iterate_copy(_max_n, nullptr, other._max_n);
    return *this;
  }
  priority_queue& operator=(priority_queue &&other) {
    if(this == &other) return *this;
    _max_n = other._max_n; other._max_n = nullptr;
    _size = other._size; other._size = 0;
    return *this;
  }
  const T& top() const {
    if(empty())
      throw container_is_empty();
    return _max_n->val;
  }
  void push(const T &val) {
    Node *node = new Node(val);
    try {
      insert_node(node);
      ++_size;
    } catch(...) {
      delete node;
      throw;
    }
  }
  void push(T &&val) {
    Node *node = new Node(std::move(val));
    try {
      insert_node(node);
      ++_size;
    } catch(...) {
      delete node;
      throw;
    }
  }
  void pop() {
    if(empty())
      throw container_is_empty();
    delete_min();
    consolidate();
    --_size;
  }
  size_t size() const {
    return _size;
  }
  bool empty() const {
    return !_max_n;
  }
  void merge(priority_queue &other);
  void clear() {
    iterate_erase(_max_n);
    _max_n = nullptr;
    _size = 0;
  }
  void swap(priority_queue &other) {
    if(this == &other) return;
    std::swap(_max_n, other._max_n);
    std::swap(_size, other._size);
  }
  // You can use it when you need to change the status used in Compare functor.
  void decrease_top(const T &val) {
    if(empty())
      throw container_is_empty();
    delete_min();
    if(_max_n != nullptr) {
      Node *new_max_n = _max_n->rht;
      for(Node *cur = _max_n->rht; cur != _max_n; cur = cur->rht)
        if(_comp(new_max_n->val, cur->val))
          new_max_n = cur;
      if(_comp(_max_n->val, new_max_n->val))
        _max_n = new_max_n;
    }
    push(val); // _size increased 1 here
    --_size;
  }
  // You can use it when you need to change the status used in Compare functor.
  void decrease_top(T &&val) {
    if(empty())
      throw container_is_empty();
    delete_min();
    if(_max_n != nullptr) {
      Node *new_max_n = _max_n->rht;
      for(Node *cur = _max_n->rht; cur != _max_n; cur = cur->rht)
        if(_comp(new_max_n->val, cur->val))
          new_max_n = cur;
      if(_comp(_max_n->val, new_max_n->val))
        _max_n = new_max_n;
    }
    push(std::move(val)); // _size increased 1 here
    --_size;
  }

private:
  static constexpr double kPhi = (1 + std::sqrt(5)) / 2;
  static constexpr double kLogPhi = std::log(kPhi);
  struct Node {
    T val;
    Node *lft, *rht, *parent, *child;
    size_t deg;
    // bool marked;
    Node(const T &_val): val(_val) {
      lft = rht = parent = child = nullptr;
      deg = 0;
      // marked = false;
    }
    Node(T &&_val): val(std::move(_val)) {
      lft = rht = parent = child = nullptr;
      deg = 0;
      // marked = false;
    }
  };
  Node *_max_n, **ranks_;
  size_t _size, ranks_size_;
  Compare _comp;
  void iterate_copy(Node *&node, Node *parent, Node *other);
  void iterate_erase(Node *node);
  // requirement: target != nullptr
  // insert node into the left of target.
  void insert_node(Node *node, Node *target);
  void insert_node(Node *node);
  // Two roots should be in different lists/heaps
  void merge_root(Node *rootA, Node *rootB);
  // _min_n may not be the smallest root. Further maintenance required.
  void delete_min();
  // insert root as the child of the target_root
  void link_root(Node *root, Node *target_root);
  // the maintenance.
  void consolidate();
};

}


#include "priority_queue.tcc"

#endif