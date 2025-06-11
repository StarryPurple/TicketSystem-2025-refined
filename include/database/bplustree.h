#ifndef INSOMNIA_BPLUSTREE_H
#define INSOMNIA_BPLUSTREE_H

#include "optional.h"
#include "buffer_pool.h"
#include "bplustree_nodes.h"

namespace insomnia {

template <class KeyT, class ValueT, class KeyCompare = std::less<KeyT>>
class Bplustree {

  using Base = BptNodeBase;
  using Internal = BptInternalNode<KeyT>;
  using Leaf = BptLeafNode<KeyT, ValueT>;
  using BufferType = BufferPool<Base, page_id_t, std::max(sizeof(Internal), sizeof(Leaf))>;
  using Visitor = typename BufferType::Visitor;

public:

  Bplustree(const std::filesystem::path &path, int buffer_capacity, int replacer_k_arg);

  ~Bplustree();

  optional<ValueT> search(const KeyT &key);

  // optional<ValueT> slow_search(const KeyT &key);

  bool insert(const KeyT &key, const ValueT &value);

  bool remove(const KeyT &key);

  void clear() {
    buf_pool_.clear();
    root_ptr_ = NULL_PAGE_ID;
  }

  [[nodiscard]]
  bool empty() const { return root_ptr_ == NULL_PAGE_ID; }

  class iterator {
    friend Bplustree;

  public:
    iterator() = default;

    // invalid ones can have a non-null buf_pool_ pointer.
    bool is_valid() const { return visitor_.is_valid(); }
    void reset() {
      buf_pool_ = nullptr;
      visitor_.drop();
      pos_ = 0;
    }

    pair<const KeyT&, ValueT&> operator*() {
      if(!visitor_.is_valid()) throw invalid_iterator("invalid bpt iterator");
      const auto ptr = visitor_.template as_mut<Leaf>();
      return insomnia::make_pair(std::ref(ptr->key(pos_)), std::ref(ptr->value(pos_))); // ADL problems
    }
    pair<const KeyT&, const ValueT&> operator*() const {
      if(!visitor_.is_valid()) throw invalid_iterator("invalid bpt iterator");
      auto ptr = visitor_.template as<Leaf>();
      return insomnia::make_pair(std::ref(ptr->key(pos_)), std::ref(ptr->value(pos_)));
    }
    pair<const KeyT&, const ValueT&> view() const {
      if(!visitor_.is_valid()) throw invalid_iterator("invalid bpt iterator");
      auto ptr = visitor_.template as<Leaf>();
      return insomnia::make_pair(std::ref(ptr->key(pos_)), std::ref(ptr->value(pos_)));
    }

    iterator& operator++();

    bool operator==(const iterator &other) const {
      if(buf_pool_ != other.buf_pool_) return false;
      if(buf_pool_ == nullptr) return true;
      if(visitor_.is_valid() != other.visitor_.is_valid()) return false;
      if(!visitor_.is_valid()) return true;
      return (visitor_.page_id() == other.visitor_.page_id()) && (pos_ == other.pos_);
    }
    bool operator!=(const iterator &other) const {
      return !(*this == other);
    }

  private:
    iterator(BufferType *buf_pool, Visitor visitor, int pos)
      : buf_pool_(buf_pool), visitor_(std::move(visitor)), pos_(pos) {}

    BufferType *buf_pool_;
    Visitor visitor_;
    int pos_;
  };

  iterator begin();
  iterator end() { return iterator(&buf_pool_, Visitor(), 0); }

  // return end() if failed.
  iterator find(const KeyT &key);
  // the first that holds a key not lower than given key.
  iterator find_upper(const KeyT &key);

private:

  bool key_equal(const KeyT &k1, const KeyT &k2) const {
    return !key_compare_(k1, k2) && !key_compare_(k2, k1);
  }

  bool value_equal(const ValueT &v1, const ValueT &v2) const {
    return !value_compare_(v1, v2) && !value_compare_(v2, v1);
  }

  BufferType buf_pool_;
  page_id_t root_ptr_;
  KeyCompare key_compare_;
};

}

#include "bplustree.tcc"

#endif