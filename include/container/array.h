#ifndef INSOMNIA_ARRAY_H
#define INSOMNIA_ARRAY_H

#include <cstring>

#include "exception.h"
#include "algorithm.h"

namespace insomnia {

template <class T, size_t N> requires std::is_default_constructible_v<T> && (N > 0)
class array {
public:
  class iterator;
  class const_iterator;
  friend iterator;
  friend const_iterator;

  array() = default;
  array(std::initializer_list<T> &&list) {
    for(size_t i = 0; i < list.size() && i < N; ++i)
      _data[i] = std::move(list[i]);
  }
  array(const T *data, size_t n) {
    if(data == nullptr) throw invalid_argument("Data is null.");
    if(n > N) throw invalid_argument("n too large.");
    for(size_t i = 0; i < n && i < N; ++i)
      _data[i] = *(data + i);
  }
  ~array() = default;

  class iterator {
    friend array;
  public:
    iterator() = default;
  private:
    iterator(const array *container, size_t idx) : container_(container), idx_(idx) {}
    iterator& operator++() { ++idx_; return *this; }
    iterator& operator--() { --idx_; return *this; }
    iterator operator++(int) { auto tmp = *this; ++idx_; return tmp; }
    iterator operator--(int) { auto tmp = *this; --idx_; return tmp; }
    T& operator*() {
      if(idx_ >= N) throw invalid_iterator();
      return container_->_data[idx_];
    }
    T* operator->() {
      if(idx_ >= N) throw invalid_iterator();
      return &container_->_data[idx_];
    }
    bool operator==(const iterator &other) const { return container_ == other.container_ && idx_ == other.idx_; }
    bool operator!=(const iterator &other) const { return container_ != other.container_ || idx_ != other.idx_; }
    bool operator==(const const_iterator &other) const { return container_ == other._container && idx_ == other._idx; }
    bool operator!=(const const_iterator &other) const { return container_ != other._container || idx_ != other._idx; }
    array *container_{nullptr};
    size_t idx_{0};
  };

  class const_iterator {
    friend array;
  public:
    const_iterator() = default;
    const_iterator(const iterator &iter) : container_(iter._container), idx_(iter._idx) {}
  private:
    const_iterator(const array *container, size_t idx) : container_(container), idx_(idx) {}
    const_iterator& operator++() { ++idx_; return *this; }
    const_iterator& operator--() { --idx_; return *this; }
    const_iterator operator++(int) { auto tmp = *this; ++idx_; return tmp; }
    const_iterator operator--(int) { auto tmp = *this; --idx_; return tmp; }
    const T& operator*() const {
      if(idx_ >= N) throw invalid_iterator();
      return container_->_data[idx_];
    }
    const T* operator->() const {
      if(idx_ >= N) throw invalid_iterator();
      return &container_->_data[idx_];
    }
    bool operator==(const iterator &other) const { return container_ == other._container && idx_ == other._idx; }
    bool operator!=(const iterator &other) const { return container_ != other._container || idx_ != other._idx; }
    bool operator==(const const_iterator &other) const { return container_ == other.container_ && idx_ == other.idx_; }
    bool operator!=(const const_iterator &other) const { return container_ != other.container_ || idx_ != other.idx_; }
    const array *container_{nullptr};
    size_t idx_{0};
  };

  iterator begin() { return iterator(this, 0); }
  iterator end() { return iterator(this, N); }
  const_iterator cbegin() const { return const_iterator(this, 0); }
  const_iterator cend() const { return const_iterator(this, N); }

  T& at(size_t idx) {
    if(idx >= N) throw index_out_of_bound();
    return _data[idx];
  }
  const T& at(size_t idx) const {
    if(idx >= N) throw index_out_of_bound();
    return _data[idx];
  }
  T& operator[](size_t idx) { return _data[idx]; }
  const T& operator[](size_t idx) const { return _data[idx]; }

  T* data() { return _data; }
private:
  T _data[N];
};

// the sentinel '\0' still exists, but not always where end() points to.
// use c_str() for a const char* string.
template <size_t N>
class array<char, N> {
public:
  class iterator;
  class const_iterator;
  friend iterator;
  friend const_iterator;

  array() { _data[0] = '\0'; }
  array(const char *str) {
    if(str == nullptr) throw invalid_argument("Data is null.");
    if(std::strlen(str) > N) throw invalid_argument("String too long.");
    auto len = std::min(N, std::strlen(str));
    memcpy(_data, str, len);
    _data[len] = '\0';
  }
  array(const std::string &str) {
    if(str.length() > N) throw invalid_argument("String too long.");
    memcpy(_data, str.c_str(), str.length());
    _data[str.length()] = '\0';
  }
  array(const char *str, size_t n) {
    if(str == nullptr) throw invalid_argument("Data is null.");
    if(n > N) throw invalid_argument("n too large.");
    memcpy(_data, str, n);
    _data[n] = '\0';
  }
  ~array() = default;

  class iterator {
    friend array;
  public:
    iterator() = default;
  private:
    iterator(const array *container, size_t idx) : _container(container), _idx(idx) {}
    iterator& operator++() { ++_idx; return *this; }
    iterator& operator--() { --_idx; return *this; }
    iterator operator++(int) { auto tmp = *this; ++_idx; return tmp; }
    iterator operator--(int) { auto tmp = *this; --_idx; return tmp; }
    char& operator*() {
      if(_idx >= N) throw invalid_iterator();
      return _container->_data[_idx];
    }
    bool operator==(const iterator &other) const { return _container == other._container && _idx == other._idx; }
    bool operator!=(const iterator &other) const { return _container != other._container || _idx != other._idx; }
    bool operator==(const const_iterator &other) const { return _container == other._container && _idx == other._idx; }
    bool operator!=(const const_iterator &other) const { return _container != other._container || _idx != other._idx; }
    array *_container{nullptr};
    size_t _idx{0};
  };

  class const_iterator {
    friend array;
  public:
    const_iterator() = default;
    const_iterator(const iterator &iter) : _container(iter._container), _idx(iter._idx) {}
  private:
    const_iterator(const array *container, size_t idx) : _container(container), _idx(idx) {}
    const_iterator& operator++() { ++_idx; return *this; }
    const_iterator& operator--() { --_idx; return *this; }
    const_iterator operator++(int) { auto tmp = *this; ++_idx; return tmp; }
    const_iterator operator--(int) { auto tmp = *this; --_idx; return tmp; }
    const char& operator*() const {
      if(_idx >= N) throw invalid_iterator();
      return _container->operator[](_idx);
    }
    bool operator==(const iterator &other) const { return _container == other._container && _idx == other._idx; }
    bool operator!=(const iterator &other) const { return _container != other._container || _idx != other._idx; }
    bool operator==(const const_iterator &other) const { return _container == other._container && _idx == other._idx; }
    bool operator!=(const const_iterator &other) const { return _container != other._container || _idx != other._idx; }
    const array *_container{nullptr};
    size_t _idx{0};
  };

  iterator begin() { return iterator(this, 0); }
  iterator end() { return iterator(this, N); }
  const_iterator cbegin() const { return const_iterator(this, 0); }
  const_iterator cend() const { return const_iterator(this, N); }

  char& at(size_t idx) {
    if(idx >= N) throw index_out_of_bound();
    return _data[idx];
  }
  const char& at(size_t idx) const {
    if(idx >= N) throw index_out_of_bound();
    return _data[idx];
  }
  char& operator[](size_t idx) { return _data[idx]; }
  const char& operator[](size_t idx) const { return _data[idx]; }

  bool operator==(const array &other) const {
    return strcmp(_data, other._data) == 0;
  }
  bool operator!=(const array &other) const {
    return strcmp(_data, other._data) != 0;
  }
  bool operator<(const array &other) const {
    return strcmp(_data, other._data) < 0;
  }
  bool operator>(const array &other) const {
    return strcmp(_data, other._data) > 0;
  }
  bool operator<=(const array &other) const {
    return strcmp(_data, other._data) <= 0;
  }
  bool operator>=(const array &other) const {
    return strcmp(_data, other._data) >= 0;
  }

  char* data() { return _data; }
  const char* c_str() const { return _data; }
  size_t length() const { return std::strlen(_data); }
  std::string str() const { return std::string(_data); }

  hash_result_t hash() const noexcept {
    return insomnia::hash<array<char, N>>()(*this);
  }

private:
  char _data[N + 1];
};

template <size_t N>
struct hash<array<char, N>> {
  constexpr hash_result_t operator()(const array<char, N> &arr) const noexcept {
    return hash<const char*>()(arr.c_str());
  }
  constexpr hash_result_t operator()(const array<char, N> &arr, size_t n) const noexcept {
    return hash<const char*>()(arr.c_str(), n);
  }
};

}

#endif