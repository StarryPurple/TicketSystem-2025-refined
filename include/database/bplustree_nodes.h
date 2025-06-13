#ifndef INSOMNIA_MULTI_BPLUSTREE_NODES_H
#define INSOMNIA_MULTI_BPLUSTREE_NODES_H

#include "fstream.h"

namespace insomnia {

inline constexpr size_t NODE_CAPACITY_LIM = 4;

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
class MultiBptInternalNode : public BptNodeBase {
  struct Storage {
    KeyT key;
    ValueT value;
    page_id_t child;
  };

  static constexpr size_t CAPACITY = std::max(NODE_CAPACITY_LIM,
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

  void split(MultiBptInternalNode *rht);

  void coalesce(MultiBptInternalNode *rht);

  // lft->size < this->size
  void redistribute_left(MultiBptInternalNode *lft);

  // rht->size < this->size
  void redistribute_right(MultiBptInternalNode *rht);

  const KeyT& key(int pos) const { return storage_[pos].key; }
  ValueT& value(int pos) { return storage_[pos].value; }
  const ValueT& value(int pos) const { return storage_[pos].value; }
  page_id_t child(int pos) const { return storage_[pos].child; }

private:
  Storage storage_[CAPACITY];
};

template <class KeyT, class ValueT>
class BptLeafNode : public BptNodeBase {
  struct Storage {
    KeyT key;
    ValueT value;
  };
  static constexpr size_t CAPACITY = std::max(NODE_CAPACITY_LIM,
    (SectorAlignedSize(sizeof(Storage) + sizeof(page_id_t)) - sizeof(page_id_t)) / sizeof(Storage));

public:

  void init(int max_size = CAPACITY - 1);

  template <class KVCompare>
  int locate_pair(const KeyT &key, const ValueT &value, KVCompare kv_compare) const;

  // returns size() if key too large.
  template <class KeyCompare>
  int locate_key(const KeyT &key, KeyCompare key_compare) const;

  void insert(int pos, const KeyT &key, const ValueT &value);

  void remove(int pos);

  void split(BptLeafNode *rht, page_id_t rht_ptr);

  void coalesce(BptLeafNode *rht);

  // lft->size < this->size
  void redistribute_left(BptLeafNode *lft);

  // rht->size < this->size
  void redistribute_right(BptLeafNode *rht);

  const KeyT& key(int pos) const { return storage_[pos].key; }
  ValueT& value(int pos) { return storage_[pos].value; }
  const ValueT& value(int pos) const { return storage_[pos].value; }
  page_id_t rht_ptr() const { return rht_ptr_; }

private:
  Storage storage_[CAPACITY];
  page_id_t rht_ptr_ {};
};

template <class KeyT>
class BptInternalNode : public BptNodeBase {

  struct Storage {
    KeyT key;
    page_id_t child;
  };

  static constexpr size_t CAPACITY = std::max(NODE_CAPACITY_LIM,
    SectorAlignedSize(sizeof(Storage)) / sizeof(Storage));

public:

  void init(int max_size = CAPACITY - 1);

  // returns size() if key too large.
  template <class KeyCompare>
  int locate_key(const KeyT &key, KeyCompare key_compare) const;

  void insert(int pos, const KeyT &key, page_id_t child);

  void remove(int pos);

  void update(int pos, const KeyT &key);

  void split(BptInternalNode *rht);

  void coalesce(BptInternalNode *rht);

  // lft->size < this->size
  void redistribute_left(BptInternalNode *lft);

  // rht->size < this->size
  void redistribute_right(BptInternalNode *rht);

  const KeyT& key(int pos) const { return storage_[pos].key; }
  page_id_t child(int pos) const { return storage_[pos].child; }

private:
  Storage storage_[CAPACITY];
};

}

#include "bplustree_nodes.tcc"

#endif