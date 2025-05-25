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

  bool remove(const KeyT &key, const ValueT &value);

  void clear();

};

}

#include "bplustree.tcc"

#endif