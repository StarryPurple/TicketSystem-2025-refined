#ifndef INSOMNIA_MULTI_BPLUSTREE_H
#define INSOMNIA_MULTI_BPLUSTREE_H

#include "buffer_pool.h"
#include "multi_bplustree_nodes.h"

namespace insomnia {

template <class KeyT, class ValueT, class KeyCompare = std::less<KeyT>, class ValueCompare = std::less<ValueT>>
class MultiBpt {

  using Base = MultiBptNodeBase;
  using Internal = MultiBptInternal<KeyT, ValueT>;
  using Leaf = MultiBptLeaf<KeyT, ValueT>;
  using BufferType = BufferPool<Base, page_id_t, std::max(sizeof(Internal), sizeof(Leaf))>;
  using Visitor = typename BufferType::Visitor;

public:

  MultiBpt(const std::filesystem::path &path, int buffer_capacity, int replacer_k_arg);

  ~MultiBpt();

  vector<ValueT> search(const KeyT &key);

  bool insert(const KeyT &key, const ValueT &value);

  bool remove(const KeyT &key, const ValueT &value);

private:

  bool key_equal(const KeyT &k1, const KeyT &k2) const {
    return !key_compare_(k1, k2) && !key_compare_(k2, k1);
  }

  bool value_equal(const ValueT &v1, const ValueT &v2) const {
    return !value_compare_(v1, v2) && !value_compare_(v2, v1);
  }

  struct KVCompare {
    KeyCompare key_compare;
    ValueCompare value_compare;
    bool operator()(const KeyT &k1, const ValueT &v1, const KeyT &k2, const ValueT &v2) const {
      if(key_compare(k1, k2))
        return true;
      if(key_compare(k2, k1))
        return false;
      return value_compare(v1, v2);
    }
  };

  BufferType buf_pool_;
  page_id_t root_ptr_;
  KeyCompare key_compare_;
  ValueCompare value_compare_;
  KVCompare kv_compare_;
};


}

#include "multi_bplustree.tcc"

#endif