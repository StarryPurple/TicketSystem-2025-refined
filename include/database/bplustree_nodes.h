#ifndef INSOMNIA_MULTI_BPLUSTREE_NODES_H
#define INSOMNIA_MULTI_BPLUSTREE_NODES_H

#include "fstream.h"

namespace insomnia {

class BptNodeBase {
public:

  bool is_leaf() const { return node_type_ == NodeT::Leaf; }

  int max_size() const { return max_size_; }

  int size() const { return size_; }

  int min_size() const { return max_size_ * 0.40; }

  int merge_bound() const { return max_size_ * 0.90; }

  bool too_large() const { return size() > max_size(); }

  bool too_small() const { return size() < min_size(); }

protected:

  enum class NodeT { Invalid, Internal, Leaf };

  NodeT node_type_;
  int max_size_;
  int size_;
};

template <class KeyT, class ValueT>
class MultiBptInternal : public BptNodeBase {
  struct Storage {
    KeyT key;
    ValueT value;
    page_id_t child;
  };

  static constexpr int CAPACITY = std::max(8ul,
    SectorAlignedSize(sizeof(Storage)) / sizeof(Storage));

public:

  void init(int max_size = CAPACITY - 1);

  template <class KVCompare>
  int locate_pair(const KeyT &key, const ValueT &value, KVCompare kv_compare) const;

  template <class KeyCompare>
  int locate_key(const KeyT &key, KeyCompare key_compare) const;

  void insert(int pos, const KeyT &key, const ValueT &value, page_id_t child);

  void remove(int pos);

  void update(int pos, const KeyT &key, const ValueT &value);

  void split(MultiBptInternal *rht);

  void coalesce(MultiBptInternal *rht);

  // lft->size < this->size
  void redistribute_left(MultiBptInternal *lft);

  // rht->size < this->size
  void redistribute_right(MultiBptInternal *rht);

  const KeyT& key(int pos) const { return storage_[pos].key; }
  const ValueT& value(int pos) const { return storage_[pos].value; }
  page_id_t child(int pos) const { return storage_[pos].child; }

private:
  Storage storage_[CAPACITY];
};

template <class KeyT, class ValueT>
class BptLeaf : public BptNodeBase {
  struct Storage {
    KeyT key;
    ValueT value;
  };
  static constexpr int CAPACITY = std::max(8ul,
    (SectorAlignedSize(sizeof(Storage) + sizeof(page_id_t)) - sizeof(page_id_t)) / sizeof(Storage));

public:

  void init(int max_size = CAPACITY - 1);

  template <class KVCompare>
  int locate_pair(const KeyT &key, const ValueT &value, KVCompare kv_compare) const;

  template <class KeyCompare>
  int locate_key(const KeyT &key, KeyCompare key_compare) const;

  void insert(int pos, const KeyT &key, const ValueT &value);

  void remove(int pos);

  void split(BptLeaf *rht, page_id_t rht_ptr);

  void coalesce(BptLeaf *rht);

  // lft->size < this->size
  void redistribute_left(BptLeaf *lft);

  // rht->size < this->size
  void redistribute_right(BptLeaf *rht);

  const KeyT& key(int pos) const { return storage_[pos].key; }
  const ValueT& value(int pos) const { return storage_[pos].value; }
  page_id_t rht_ptr() const { return rht_ptr_; }

private:
  Storage storage_[CAPACITY];
  page_id_t rht_ptr_ {};
};

}

#include "bplustree_nodes.tcc"

#endif