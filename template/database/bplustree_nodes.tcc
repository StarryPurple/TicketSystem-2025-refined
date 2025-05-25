#ifndef INSOMNIA_MULTI_BPLUSTREE_NODES_TCC
#define INSOMNIA_MULTI_BPLUSTREE_NODES_TCC

#include "multi_bplustree.h"

namespace insomnia {

template <class KeyT, class ValueT>
void MultiBptInternal<KeyT, ValueT>::init(int max_size) {
  node_type_ = NodeT::Internal;
  max_size_ = max_size;
  size_ = 0;
}

template <class KeyT, class ValueT>
template <class KVCompare>
int MultiBptInternal<KeyT, ValueT>::locate_pair(
  const KeyT &key, const ValueT &value, KVCompare kv_compare) const {
  int lft = 1, rht = size_ - 1;
  if(kv_compare(key, value, storage_[lft].key, storage_[lft].value))
    return lft - 1;
  while(lft < rht) {
    int mid = (lft + rht) / 2 + 1;
    if(kv_compare(key, value, storage_[mid].key, storage_[mid].value))
      rht = mid - 1;
    else
      lft = mid;
  }
  return rht;
}

template <class KeyT, class ValueT>
template <class KeyCompare>
int MultiBptInternal<KeyT, ValueT>::locate_key(const KeyT &key, KeyCompare key_compare) const {
  int lft = 1, rht = size_ - 1;
  if(!key_compare(storage_[lft].key, key))
    return lft - 1;
  while(lft < rht) {
    int mid = (lft + rht) / 2 + 1;
    if(!key_compare(storage_[mid].key, key))
      rht = mid - 1;
    else
      lft = mid;
  }
  return rht;
}

template <class KeyT, class ValueT>
void MultiBptInternal<KeyT, ValueT>::insert(int pos, const KeyT &key, const ValueT &value, page_id_t child) {
  memmove(storage_ + pos + 1, storage_ + pos, (size_ - pos) * sizeof(Storage));
  storage_[pos] = {key, value, child};
  size_ += 1;
}

template <class KeyT, class ValueT>
void MultiBptInternal<KeyT, ValueT>::remove(int pos) {
  memmove(storage_ + pos, storage_ + pos + 1, (size_ - pos - 1) * sizeof(Storage));
  size_ -= 1;
}

template <class KeyT, class ValueT>
void MultiBptInternal<KeyT, ValueT>::update(int pos, const KeyT &key, const ValueT &value) {
  storage_[pos].key = key;
  storage_[pos].value = value;
}

template <class KeyT, class ValueT>
void MultiBptInternal<KeyT, ValueT>::split(MultiBptInternal *rht) {
  int lft_size = size_ / 2, rht_size = size_ - lft_size;
  memcpy(rht->storage_, storage_ + lft_size, rht_size * sizeof(Storage));
  size_ = lft_size;
  rht->size_ = rht_size;
}


template <class KeyT, class ValueT>
void MultiBptInternal<KeyT, ValueT>::coalesce(MultiBptInternal *rht) {
  int lft_old_size = size_, rht_old_size = rht->size_;
  int tot_size = lft_old_size + rht_old_size;
  memcpy(storage_ + lft_old_size, rht->storage_, rht_old_size * sizeof(Storage));
  size_ = tot_size;
  rht->size_ = 0;
}

template <class KeyT, class ValueT>
void MultiBptInternal<KeyT, ValueT>::redistribute_left(MultiBptInternal *lft) {
  int lft_old_size = lft->size_, rht_old_size = size_;
  int tot_size = lft_old_size + rht_old_size;
  int lft_new_size = tot_size / 2, rht_new_size = tot_size - lft_new_size;
  int diff = lft_new_size - lft_old_size;
  memcpy(lft->storage_ + lft_old_size, storage_, diff * sizeof(Storage));
  memmove(storage_, storage_ + diff, rht_new_size * sizeof(Storage));
  lft->size_ = lft_new_size;
  size_ = rht_new_size;
}

template <class KeyT, class ValueT>
void MultiBptInternal<KeyT, ValueT>::redistribute_right(MultiBptInternal *rht) {
  int lft_old_size = size_, rht_old_size = rht->size_;
  int tot_size = lft_old_size + rht_old_size;
  int lft_new_size = tot_size / 2, rht_new_size = tot_size - lft_new_size;
  int diff = rht_new_size - rht_old_size;
  memmove(rht->storage_ + diff, rht->storage_, rht_old_size * sizeof(Storage));
  memcpy(rht->storage_, storage_ + lft_new_size, diff * sizeof(Storage));
  size_ = lft_new_size;
  rht->size_ = rht_new_size;
}

/**********************************************************************************************************************/

template <class KeyT, class ValueT>
void BptLeaf<KeyT, ValueT>::init(int max_size) {
  node_type_ = NodeT::Leaf;
  max_size_ = max_size;
  size_ = 0;
  rht_ptr_ = NULL_PAGE_ID;
}

template <class KeyT, class ValueT>
template <class KVCompare>
int BptLeaf<KeyT, ValueT>::locate_pair(
  const KeyT &key, const ValueT &value, KVCompare kv_compare) const {
  int lft = 0, rht = size_ - 1;
  if(kv_compare(storage_[rht].key, storage_[rht].value, key, value))
    return rht + 1;
  while(lft < rht) {
    int mid = (lft + rht) / 2;
    if(kv_compare(storage_[mid].key, storage_[mid].value, key, value))
      lft = mid + 1;
    else
      rht = mid;
  }
  return rht;
}

template <class KeyT, class ValueT>
template <class KeyCompare>
int BptLeaf<KeyT, ValueT>::locate_key(const KeyT &key, KeyCompare key_compare) const {
  int lft = 0, rht = size_ - 1;
  if(key_compare(storage_[rht].key, key))
    return rht + 1;
  while(lft < rht) {
    int mid = (lft + rht) / 2;
    if(key_compare(storage_[mid].key, key))
      lft = mid + 1;
    else
      rht = mid;
  }
  return rht;
}

template <class KeyT, class ValueT>
void BptLeaf<KeyT, ValueT>::insert(int pos, const KeyT &key, const ValueT &value) {
  memmove(storage_ + pos + 1, storage_ + pos, (size_ - pos) * sizeof(Storage));
  storage_[pos] = {key, value};
  size_ += 1;
}

template <class KeyT, class ValueT>
void BptLeaf<KeyT, ValueT>::remove(int pos) {
  memmove(storage_ + pos, storage_ + pos + 1, (size_ - pos - 1) * sizeof(Storage));
  size_ -= 1;
}

template <class KeyT, class ValueT>
void BptLeaf<KeyT, ValueT>::split(BptLeaf *rht, page_id_t rht_ptr) {
  int lft_size = size_ / 2, rht_size = size_ - lft_size;
  memcpy(rht->storage_, storage_ + lft_size, rht_size * sizeof(Storage));
  size_ = lft_size;
  rht->size_ = rht_size;
  rht->rht_ptr_ = rht_ptr_;
  rht_ptr_ = rht_ptr;
}

template <class KeyT, class ValueT>
void BptLeaf<KeyT, ValueT>::coalesce(BptLeaf *rht) {
  int lft_old_size = size_, rht_old_size = rht->size_;
  int tot_size = lft_old_size + rht_old_size;
  memcpy(storage_ + lft_old_size, rht->storage_, rht_old_size * sizeof(Storage));
  size_ = tot_size;
  rht->size_ = 0;
  rht_ptr_ = rht->rht_ptr_;
  rht->rht_ptr_ = NULL_PAGE_ID;
}

template <class KeyT, class ValueT>
void BptLeaf<KeyT, ValueT>::redistribute_left(BptLeaf *lft) {
  int lft_old_size = lft->size_, rht_old_size = size_;
  int tot_size = lft_old_size + rht_old_size;
  int lft_new_size = tot_size / 2, rht_new_size = tot_size - lft_new_size;
  int diff = lft_new_size - lft_old_size;
  memcpy(lft->storage_ + lft_old_size, storage_, diff * sizeof(Storage));
  memmove(storage_, storage_ + diff, rht_new_size * sizeof(Storage));
  lft->size_ = lft_new_size;
  size_ = rht_new_size;
}

template <class KeyT, class ValueT>
void BptLeaf<KeyT, ValueT>::redistribute_right(BptLeaf *rht) {
  int lft_old_size = size_, rht_old_size = rht->size_;
  int tot_size = lft_old_size + rht_old_size;
  int lft_new_size = tot_size / 2, rht_new_size = tot_size - lft_new_size;
  int diff = rht_new_size - rht_old_size;
  memmove(rht->storage_ + diff, rht->storage_, rht_old_size * sizeof(Storage));
  memcpy(rht->storage_, storage_ + lft_new_size, diff * sizeof(Storage));
  size_ = lft_new_size;
  rht->size_ = rht_new_size;
}

}

#endif