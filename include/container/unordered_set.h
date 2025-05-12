#ifndef INSOMNIA_UNORDERED_SET_H
#define INSOMNIA_UNORDERED_SET_H

namespace insomnia {

template <
  class T,
  class Hash = std::hash<T>,
  class Equal = std::equal_to<T>>
class unordered_set {
public:
  using value_type = const T;
  class iterator;
  friend iterator;

private:
  struct NodeBase {
    NodeBase *t_nxt{nullptr}, *t_prv{nullptr}, *b_nxt{nullptr}, *b_prv{nullptr};
    // virtual NodeBase* clone() { return new NodeBase; }
    virtual ~NodeBase() = default;
  };
  struct Node : NodeBase {
    value_type value;
    Node(value_type _value) : NodeBase(), value(std::move(_value)) {}
    // virtual NodeBase* clone() override { return new Node(value_pair); }
    virtual ~Node() override = default;
  };

public:

  unordered_set();
  // unordered_set(const unordered_set &other);
  unordered_set(unordered_set &&other);
  // unordered_set& operator=(const unordered_set &other);
  unordered_set& operator=(unordered_set &&other);
  ~unordered_set();

  class iterator {
    friend unordered_set;
  public:
    iterator() : _container(nullptr), _node(nullptr) {}
    iterator(const unordered_set *container, NodeBase *node)
      : _container(container), _node(node) {}
    iterator& operator++() {
      if(!_container || !_node) throw invalid_iterator();
      if(_node == _container->_header) throw index_out_of_bound();
      _node = _node->t_nxt;
      return *this;
    }
    iterator& operator--() {
      if(!_container || !_node) throw invalid_iterator();
      if(_node == _container->_header->t_nxt) throw index_out_of_bound();
      _node = _node->t_prv;
      return *this;
    }
    iterator operator++(int) {
      auto tmp = *this;
      ++*this;
      return tmp;
    }
    iterator operator--(int) {
      auto tmp = *this;
      --*this;
      return tmp;
    }
    value_type& operator*() {
      if(!_container || !_node) throw invalid_iterator();
      Node *node = dynamic_cast<Node*>(_node);
      if(!node) throw invalid_iterator();
      return node->value;
    }
    value_type* operator->() {
      return &**this;
    }
    bool operator==(const iterator &other) const {
      return _container == other._container && _node == other._node;
    }
    bool operator!=(const iterator &other) const {
      return _container != other._container || _node != other._node;
    }
  private:
    const unordered_set *_container;
    NodeBase *_node;
  };

  iterator begin() { return iterator{this, _header->t_nxt}; }
  iterator end() { return iterator{this, _header}; }

  iterator insert(const value_type &value);
  iterator insert(value_type &&value);
  template <class ...Args>
  iterator emplace(Args &&...args);

  void erase(iterator it);
  void erase(const T &t) { erase(find(t)); }

  iterator find(const T &t);
  bool contains(const T &t) { return find(t) != end(); }

  size_t size() const { return _size; }
  bool empty() const { return _size == 0; }
  void clear();
  void reserve(size_t bucket_size) { rehash(bucket_size); }

private:
  void rehash(size_t bucket_size);
  iterator insert_node(NodeBase *node, size_t idx);
  bool overload() const { return _size >= _bucket_size * 2; }

  NodeBase **_node_list, *_header;
  size_t _size, _bucket_size;
  Hash hash_;
  Equal t_equal_;
};
}

#include "unordered_set.tcc"

#endif