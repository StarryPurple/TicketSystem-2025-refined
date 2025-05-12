#ifndef INSOMNIA_UNORDERED_SET_TCC
#define INSOMNIA_UNORDERED_SET_TCC

#include <unordered_set>

#include "unordered_set.h"

namespace insomnia {

template <class T, class Hash, class Equal>
unordered_set<T, Hash, Equal>::unordered_set() {
  _bucket_size = 16;
  _size = 0;
  _header = new NodeBase;
  _node_list = new NodeBase*[_bucket_size];
  for(size_t i = 0; i < _bucket_size; ++i)
    _node_list[i] = new NodeBase;
  _header->t_prv = _header->t_nxt = _header;
}

template <class T, class Hash, class Equal>
unordered_set<T, Hash, Equal>::unordered_set(unordered_set &&other) {
  _node_list = other._node_list;
  _header = other._header;
  _size = other._size;
  _bucket_size = other._bucket_size;
  other._node_list = nullptr;
  other._header = nullptr;
  other._size = 0;
  other._bucket_size = 0;
}

template <class T, class Hash, class Equal>
unordered_set<T, Hash, Equal>&
  unordered_set<T, Hash, Equal>::operator=(unordered_set &&other) {
  if(this == &other) return *this;
  clear();
  for(size_t i = 0; i < _bucket_size; ++i)
    delete _node_list[i];
  delete[] _node_list;
  delete _header;
  _node_list = other._node_list;
  _header = other._header;
  _size = other._size;
  _bucket_size = other._bucket_size;
  other._node_list = nullptr;
  other._header = nullptr;
  other._size = 0;
  other._bucket_size = 0;
  return *this;
}

template <class T, class Hash, class Equal>
unordered_set<T, Hash, Equal>::~unordered_set() {
  clear();
  for(size_t i = 0; i < _bucket_size; ++i)
    delete _node_list[i];
  delete[] _node_list;
  delete _header;
}

template <class T, class Hash, class Equal>
typename unordered_set<T, Hash, Equal>::iterator
unordered_set<T, Hash, Equal>::insert(const value_type &value) {
  if(auto it = find(value); it != end())
    return it;
  size_t idx = hash_(value) % _bucket_size;
  NodeBase *node = new Node(value);
  return insert_node(node, idx);
}

template <class T, class Hash, class Equal>
typename unordered_set<T, Hash, Equal>::iterator
unordered_set<T, Hash, Equal>::insert(value_type &&value) {
  if(auto it = find(value); it != end())
    return it;
  size_t idx = hash_(value) % _bucket_size;
  NodeBase *node = new Node(std::move(value));
  return insert_node(node, idx);
}

template <class T, class Hash, class Equal>
template <class... Args>
typename unordered_set<T, Hash, Equal>::iterator
unordered_set<T, Hash, Equal>::emplace(Args &&... args) {
  return insert(value_type(std::forward<Args>(args)...));
}

template <class T, class Hash, class Equal>
typename unordered_set<T, Hash, Equal>::iterator
unordered_set<T, Hash, Equal>::insert_node(NodeBase *node, size_t idx) {
  node->b_nxt = _node_list[idx]->b_nxt;
  node->b_prv = _node_list[idx];
  if(node->b_nxt) node->b_nxt->b_prv = node;
  _node_list[idx]->b_nxt = node;

  node->t_nxt = _header;
  node->t_prv = _header->t_prv;
  node->t_nxt->t_prv = node;
  node->t_prv->t_nxt = node;

  ++_size;
  if(overload()) rehash(_size);
  return iterator(this, node);
}

template <class T, class Hash, class Equal>
void unordered_set<T, Hash, Equal>::clear() {
  for(size_t i = 0; i < _bucket_size; ++i) {
    for(NodeBase *curr = _node_list[i]->b_nxt, *del; curr != nullptr;) {
      del = curr;
      curr = curr->b_nxt;
      delete del;
    }
    _node_list[i]->b_nxt = nullptr;
  }
  _header->t_nxt = _header->t_prv = nullptr;
  _size = 0;
}

template <class T, class Hash, class Equal>
void unordered_set<T, Hash, Equal>::erase(iterator it) {
  if(it._container != this || it._node == nullptr) throw invalid_iterator();
  if(it._node == _header) return;
  NodeBase *node = it._node;
  if(node->b_nxt) node->b_nxt->b_prv = node->b_prv;
  node->b_prv->b_nxt = node->b_nxt;
  node->t_nxt->t_prv = node->t_prv;
  node->t_prv->t_nxt = node->t_nxt;
  --_size;
  delete node;
}

template <class T, class Hash, class Equal>
typename unordered_set<T, Hash, Equal>::iterator
unordered_set<T, Hash, Equal>::find(const T &t) {
  size_t idx = hash_(t) % _bucket_size;
  for(NodeBase *curr = _node_list[idx]->b_nxt; curr != nullptr; curr = curr->b_nxt)
    if(Node *node = static_cast<Node*>(curr); t_equal_(node->value, t))
      return iterator(this, curr);
  return end();
}

template <class T, class Hash, class Equal>
void unordered_set<T, Hash, Equal>::rehash(size_t bucket_size) {
  if(bucket_size <= _bucket_size) return;
  NodeBase **new_node_list = new NodeBase*[bucket_size];
  for(size_t i = 0; i < bucket_size; ++i)
    new_node_list[i] = new NodeBase;
  for(NodeBase *curr = _header->t_nxt; curr != _header;) {
    NodeBase *nxt = curr->t_nxt;

    size_t idx = hash_(static_cast<Node*>(curr)->value) % bucket_size;
    curr->b_nxt = new_node_list[idx]->b_nxt;
    curr->b_prv = new_node_list[idx];
    if(curr->b_nxt) curr->b_nxt->b_prv = curr;
    new_node_list[idx]->b_nxt = curr;

    curr = nxt;
  }
  for(size_t i = 0; i < _bucket_size; ++i)
    delete _node_list[i];
  delete[] _node_list;
  _node_list = new_node_list;
  _bucket_size = bucket_size;
}




}

#endif