#ifndef INSOMNIA_MAP_H
#define INSOMNIA_MAP_H

namespace insomnia {


template<class Key, class Tp, class Compare = std::less<Key>>
class map {
public:
  using value_type = std::pair<const Key, Tp>;
private:
  struct Node {
    enum class Color { Red, Black };
    Node *parent, *left, *right;
    value_type value;
    Color color;
    Node(const value_type &_value)
      : parent(nullptr), left(nullptr), right(nullptr), value(_value), color(Color::Red) {}
    Node(const Node &other) = delete;
    Node(Node &&other) = delete;
  };

  Node *_root, *_left_most, *_right_most;
  size_t _size;
  Compare _comparer;

  void recurse_clear(Node *node);
  void recurse_copy(Node *des, Node *src, const map &other);
  void left_rotate(Node *node);
  void right_rotate(Node *node);
  Node* get_next(Node *node) const;
  Node* get_prev(Node *node) const;
  void insertion_maintain(Node *node);
  void erasure_maintain(Node *node);

public:
  class const_iterator;
  class iterator {
    friend map;
    friend const_iterator;
  private:
    const map<Key, Tp, Compare> *container;
    Node *node;

  public:
    iterator(): container(nullptr), node(nullptr) {} // default iterator as end()
    iterator(const map<Key, Tp, Compare> *the_map, Node *the_node): container(the_map), node(the_node) {}
    iterator(const iterator &other): container(other.container), node(other.node) {}
    iterator operator++(int) {
      iterator res = *this;
      ++(*this);
      return res;
    }
    iterator& operator++() {
      if(node == nullptr) // end()
        throw invalid_iterator();
      Node *next = container->get_next(node);
      node = next;
      return *this;
    }
    iterator operator--(int) {
      iterator res = *this;
      --(*this);
      return res;
    }
    iterator& operator--() {
      if(node == container->_left_most) // begin()
        throw invalid_iterator();
      Node *prev = container->get_prev(node);
      node = prev;
      return *this;
    }
    bool operator==(const iterator &other) const {
      return container == other.container && node == other.node;
    }
    bool operator==(const const_iterator &other) const {
      return container == other.container && node == other.node;
    }
    bool operator!=(const iterator &other) const {
      return !(*this == other);
    }
    bool operator!=(const const_iterator &other) const {
      return !(*this == other);
    }
    value_type& operator*() const {
      return node->value;
    }
    value_type* operator->() const {
      return &*(*this);
    }
  };
  class const_iterator {
    friend map; // ?
    friend iterator;
  private:
    const map<Key, Tp, Compare> *container;
    Node *node;

  public:
    const_iterator(): container(nullptr), node(nullptr) {} // default iterator as cend()
    const_iterator(const map<Key, Tp, Compare> *the_map, Node *the_node): container(the_map), node(the_node) {}
    const_iterator(const const_iterator &other): container(other.container), node(other.node) {}
    const_iterator(const iterator &other): container(other.container), node(other.node) {}
    const_iterator operator++(int) {
      const_iterator res = *this;
      ++(*this);
      return res;
    }
    const_iterator& operator++() {
      if(node == nullptr) // cend()
        throw invalid_iterator();
      Node *next = container->get_next(node);
      node = next;
      return *this;
    }
    const_iterator operator--(int) {
      const_iterator res = *this;
      --(*this);
      return res;
    }
    const_iterator& operator--() {
      if(node == container->_left_most) // cbegin()
        throw invalid_iterator();
      Node *prev = container->get_prev(node);
      node = prev;
      return *this;
    }
    bool operator==(const iterator &other) const {
      return container == other.container && node == other.node;
    }
    bool operator==(const const_iterator &other) const {
      return container == other.container && node == other.node;
    }
    bool operator!=(const iterator &other) const {
      return !(*this == other);
    }
    bool operator!=(const const_iterator &other) const {
      return !(*this == other);
    }
    const value_type& operator*() const {
      return node->value;
    }
    const value_type* operator->() const {
      return &*(*this);
    }
  };

  map(): _root(nullptr), _size(0) {
    _root = nullptr;
    _left_most = _right_most = nullptr;
  }
  map(const map &other): map() {
    if(other.empty()) return;
    _size = other._size;
    _root = new Node(other._root->value);
    _root->color =  other._root->color;
    if(other._left_most == other._root) _left_most = _root;
    if(other._right_most == other._root) _right_most = _root;
    recurse_copy(_root, other._root, other);
  }
  map(map &&other) {
    _size = other._size;
    _root = other._root;
    _left_most = other._left_most;
    _right_most = other._right_most;
  }
  ~map() {
    clear();
  }
  map& operator=(const map &other) {
    if(this == &other) return *this;
    clear();
    if(other.empty()) return *this;
    _size = other._size;
    _root = new Node(other._root->value);
    _root->color = other._root->color;
    if(other._left_most == other._root) _left_most = _root;
    if(other._right_most == other._root) _right_most = _root;
    recurse_copy(_root, other._root, other);
    return *this;
  }
  map& operator=(map &&other) {
    if(this == &other) return *this;
    clear();
    _size = other._size;
    _root = other._root;
    _left_most = other._left_most;
    _right_most = other._right_most;
    return *this;
  }
  iterator begin() {
    return iterator(this, _left_most);
  }
  iterator end() {
    return iterator(this, nullptr);
  }
  value_type& front() {
    if(empty())
      throw container_is_empty();
    return _left_most->value;
  }
  value_type& back() {
    if(empty())
      throw container_is_empty();
    return _right_most->value;
  }
  const_iterator cbegin() const {
    return const_iterator(this, _left_most);
  }
  const_iterator cend() const {
    return const_iterator(this, nullptr);
  }
  void clear() {
    if(empty()) return;
    recurse_clear(_root);
    _size = 0;
    _root = nullptr;
    _left_most = _right_most = nullptr;
  }
  size_t size() const {
    return _size;
  }
  bool empty() const {
    return _size == 0;
  }
  // returns end iterator if search fails.
  iterator find(const Key &key);
  const_iterator find(const Key &key) const;
  bool contains(const Key &key) const {
    return find(key) != cend();
  }
  Tp& at(const Key &key) {
    iterator it = find(key);
    if(it == end()) throw index_out_of_bound();
    return it->second;
  }
  const Tp& at(const Key &key) const {
    const_iterator it = find(key);
    if(it == cend()) throw index_out_of_bound();
    return it->second;
  }
  // insert an empty Tp value into the map.
  Tp& operator[](const Key &key);
  // throws index_out_of_bound if key doesn't exist.
  const Tp& operator[](const Key &key) const {
    const_iterator it = find(key);
    if(it == cend()) throw index_out_of_bound();
    return it->second;
  }
  std::pair<iterator, bool> insert(const value_type &value);
  // A fake implementation.
  template <class ...Args>
  std::pair<iterator, bool> emplace(Args &&...args) {
    return insert(value_type(std::forward<Args>(args)...));
  }
  // may throw invalid_iterator if invalidation is detected.
  // (visiting deleted pointer may occur, resulting in core dump(?).)
  void erase(iterator pos);
};

}

#include "map.tcc"

#endif