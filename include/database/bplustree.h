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