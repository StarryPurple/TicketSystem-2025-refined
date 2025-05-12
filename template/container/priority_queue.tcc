#ifndef INSOMNIA_PRIORITY_QUEUE_TCC
#define INSOMNIA_PRIORITY_QUEUE_TCC

#include "priority_queue.h"

namespace insomnia {

template <class T, class Compare>
void priority_queue<T, Compare>::merge(priority_queue &other) {
  if(this == &other) return; // I don't know what should be done here.
  if(other.empty()) return;
  if(empty()) {
    swap(other);
    return;
  }
  bool is_change = _comp(_max_n->val, other._max_n->val);
  merge_root(_max_n, other._max_n);
  if(is_change)
    _max_n = other._max_n;
  _size += other._size;
  other._max_n = nullptr;
  other._size = 0;
}

template <class T, class Compare>
void priority_queue<T, Compare>::iterate_copy(Node *&node, Node *parent, Node *other) {
  if(!other) return;
  node = new Node(other->val);
  node->parent = parent;
  Node *dest = node, *src = other;
  while(src->lft != other) {
    dest->lft = new Node(src->lft->val);
    dest->lft->rht = dest;
    dest->parent = parent;
    iterate_copy(dest->child, dest, src->child);
    dest = dest->lft;
    src = src->lft;
  }
  dest->lft = node;
  node->rht = dest;
}

template <class T, class Compare>
void priority_queue<T, Compare>::iterate_erase(Node *node) {
  if(!node) return;
  for(Node *cur = node->lft, *nxt = cur->lft; cur != node; cur = nxt, nxt = cur->lft) {
    iterate_erase(cur->child);
    delete cur;
  }
  iterate_erase(node->child);
  delete node;
}

template <class T, class Compare>
void priority_queue<T, Compare>::insert_node(Node *node, Node *target) {
  Node *lft = target->lft, *parent = lft->parent;
  lft->rht = node; target->lft = node;
  node->lft = lft; node->rht = target;
  node->parent = parent;
  if(parent)
    ++parent->deg;
}

template <class T, class Compare>
void priority_queue<T, Compare>::insert_node(Node *node) {
  if(!_max_n) {
    _max_n = node;
    node->lft = node->rht = node;
    node->parent = nullptr;
  } else {
    Node *new_min = _comp(_max_n->val, node->val) ? node : _max_n;
    insert_node(node, _max_n);
    _max_n = new_min;
  }
}

template <class T, class Compare>
void priority_queue<T, Compare>::merge_root(Node *rootA, Node *rootB) {
  Node *lA = rootA->lft, *lB = rootB->lft;
  lA->rht = rootB; rootB->lft = lA;
  lB->rht = rootA; rootA->lft = lB;
}

template <class T, class Compare>
void priority_queue<T, Compare>::delete_min() {
  if(_size == 1) {
    delete _max_n;
    _max_n = nullptr;
    return;
  }
  Node *child = _max_n->child, *new_root;
  if(child) {
    Node *cur = child;
    do {
      Node *nxt = cur->rht;
      insert_node(cur, _max_n);
      cur = nxt;
    } while(cur != child);
    new_root = _max_n->lft;
  } else {
    // different root exists.
    new_root = _comp(_max_n->lft->val, _max_n->rht->val) ? _max_n->rht : _max_n->lft;
  }
  Node *del = _max_n;
  _max_n = new_root;
  del->lft->rht = del->rht;
  del->rht->lft = del->lft;
  delete del;
}

template <class T, class Compare>
void priority_queue<T, Compare>::link_root(Node *root, Node *target_root) {
  root->lft->rht = root->rht;
  root->rht->lft = root->lft;
  if(target_root->child)
    insert_node(root, target_root->child);
  else {
    target_root->child = root;
    root->parent = target_root;
    root->lft = root->rht = root;
    ++target_root->deg;
  }
}

template <class T, class Compare>
void priority_queue<T, Compare>::consolidate()  {
  if(_max_n == nullptr) return;
  size_t desired_size = static_cast<size_t>(std::log(_size - 1) / kLogPhi) + 1;
  if(ranks_size_ < desired_size) {
    delete[] ranks_;
    ranks_ = new Node*[2 * desired_size];
    ranks_size_ = 2 * desired_size;
  }
  for(int i = 0; i < ranks_size_; ++i)
    ranks_[i] = nullptr;
  Node *end = _max_n, *cur = end->rht;
  while(cur != end) {
    Node *nxt = cur->rht;
    while(ranks_[cur->deg]) {
      Node *other = ranks_[cur->deg];
      ranks_[cur->deg] = nullptr;
      if(_comp(cur->val, other->val))
        std::swap(cur, other);
      link_root(other, cur);
    }
    ranks_[cur->deg] = cur;
    if(!_comp(cur->val, _max_n->val))
      _max_n = cur;
    cur = nxt;
  }
  /** The same **/
  while(ranks_[cur->deg]) {
    Node *other = ranks_[cur->deg];
    ranks_[cur->deg] = nullptr;
    if(_comp(cur->val, other->val))
      std::swap(cur, other);
    link_root(other, cur);
  }
  ranks_[cur->deg] = cur;
  if(!_comp(cur->val, _max_n->val))
    _max_n = cur;
  /** The same **/
}

}

#endif