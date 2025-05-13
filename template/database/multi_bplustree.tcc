#ifndef INSOMNIA_MULTI_BPLUSTREE_TCC
#define INSOMNIA_MULTI_BPLUSTREE_TCC

#include "multi_bplustree.h"

namespace insomnia {

template <class KeyT, class ValueT, class KeyCompare, class ValueCompare>
MultiBpt<KeyT, ValueT, KeyCompare, ValueCompare>::MultiBpt(
  const std::filesystem::path &path, int buffer_capacity, int replacer_k_arg)
    : buf_pool_(path.string() + ".multdb", buffer_capacity, replacer_k_arg) {
  if(!buf_pool_.read_meta(&root_ptr_))
    root_ptr_ = NULL_PAGE_ID;
}

template <class KeyT, class ValueT, class KeyCompare, class ValueCompare>
MultiBpt<KeyT, ValueT, KeyCompare, ValueCompare>::~MultiBpt() {
  buf_pool_.write_meta(&root_ptr_);
}

template <class KeyT, class ValueT, class KeyCompare, class ValueCompare>
vector<ValueT> MultiBpt<KeyT, ValueT, KeyCompare, ValueCompare>::search(const KeyT &key) {
  if(root_ptr_ == NULL_PAGE_ID)
    return vector<ValueT>();
  vector<Visitor> visitors;
  visitors.push_back(buf_pool_.visitor(root_ptr_));
  while(!visitors.back().template as<Base>()->is_leaf()) {
    auto node = visitors.back().template as<Internal>();
    auto pos = node->locate_key(key, key_compare_);
    auto ptr = node->child(pos);
    visitors.push_back(buf_pool_.visitor(ptr));
  }
  auto visitor = std::move(visitors.back());
  visitors.pop_back();
  auto node = visitor.template as<Leaf>();
  auto pos = node->locate_key(key, key_compare_);
  vector<ValueT> result;
  while(true) {
    if(pos == node->size()) {
      auto rht_ptr = node->rht_ptr();
      if(rht_ptr == NULL_PAGE_ID)
        return result;
      visitor = buf_pool_.visitor(rht_ptr);
      node = visitor.template as<Leaf>();
      pos = 0;
    }
    if(!key_equal(node->key(pos), key))
      return result;
    result.push_back(node->value(pos));
    ++pos;
  }
}

template <class KeyT, class ValueT, class KeyCompare, class ValueCompare>
vector<ValueT> MultiBpt<KeyT, ValueT, KeyCompare, ValueCompare>::slow_search(const KeyT &key) {
  if(root_ptr_ == NULL_PAGE_ID)
    return vector<ValueT>();
  vector<Visitor> visitors;
  visitors.push_back(buf_pool_.visitor(root_ptr_));
  while(!visitors.back().template as<Base>()->is_leaf()) {
    auto node = visitors.back().template as<Internal>();
    auto pos = 0;
    auto ptr = node->child(pos);
    visitors.push_back(buf_pool_.visitor(ptr));
  }
  auto visitor = std::move(visitors.back());
  visitors.pop_back();
  auto node = visitor.template as<Leaf>();
  auto pos = 0;
  vector<ValueT> result;
  while(true) {
    if(pos == node->size()) {
      auto rht_ptr = node->rht_ptr();
      if(rht_ptr == NULL_PAGE_ID)
        return result;
      visitor = buf_pool_.visitor(rht_ptr);
      node = visitor.template as<Leaf>();
      pos = 0;
    }
    if(key_equal(node->key(pos), key))
      result.push_back(node->value(pos));
    ++pos;
  }
}


template <class KeyT, class ValueT, class KeyCompare, class ValueCompare>
bool MultiBpt<KeyT, ValueT, KeyCompare, ValueCompare>::insert(const KeyT &key, const ValueT &value) {
  if(root_ptr_ == NULL_PAGE_ID) {
    root_ptr_ = buf_pool_.alloc();
    auto visitor = buf_pool_.visitor(root_ptr_);
    auto root_node = visitor.template as_mut<Leaf>();
    root_node->init();
    root_node->insert(0, key, value);
    return true;
  }
  vector<Visitor> visitors;
  visitors.push_back(buf_pool_.visitor(root_ptr_));
  while(!visitors.back().template as<Base>()->is_leaf()) {
    auto node = visitors.back().template as<Internal>();
    auto pos = node->locate_pair(key, value, kv_compare_);
    auto ptr = node->child(pos);
    visitors.push_back(buf_pool_.visitor(ptr));
  }
  auto leaf_visitor = std::move(visitors.back());
  visitors.pop_back();
  auto leaf_immut = leaf_visitor.template as<Leaf>();
  auto leaf_pos = leaf_immut->locate_pair(key, value, kv_compare_);
  if(leaf_pos != leaf_immut->size() &&
    key_equal(leaf_immut->key(leaf_pos), key) &&
    value_equal(leaf_immut->value(leaf_pos), value))
    return false;
  auto leaf = leaf_visitor.template as_mut<Leaf>();
  leaf->insert(leaf_pos, key, value);
  if(!leaf->too_large())
    return true;

  {
    auto rht_ptr = buf_pool_.alloc();
    auto rht_visitor = buf_pool_.visitor(rht_ptr);
    auto rht_leaf = rht_visitor.template as_mut<Leaf>();
    rht_leaf->init();
    leaf->split(rht_leaf, rht_ptr);
    if(visitors.empty()) {
      auto root_ptr = buf_pool_.alloc();
      auto root_visitor = buf_pool_.visitor(root_ptr);
      auto root = root_visitor.template as_mut<Internal>();
      root->init();
      root->insert(0, leaf->key(0), leaf->value(0), root_ptr_);
      root->insert(1, rht_leaf->key(0), rht_leaf->value(0), rht_ptr);
      root_ptr_ = root_ptr;
      return true;
    }
    auto &parent_visitor = visitors.back();
    auto parent_node = parent_visitor.template as_mut<Internal>();
    auto pos = parent_node->locate_pair(leaf->key(0), leaf->value(0), kv_compare_);
    parent_node->insert(pos + 1, rht_leaf->key(0), rht_leaf->value(0), rht_ptr);
  }
  leaf_visitor.drop();
  while(visitors.size() > 1) {
    Visitor visitor = std::move(visitors.back());
    visitors.pop_back();
    if(!visitor.template as<Base>()->too_large())
      return true;
    auto node = visitor.template as_mut<Internal>();
    auto rht_ptr = buf_pool_.alloc();
    auto rht_visitor = buf_pool_.visitor(rht_ptr);
    auto rht_node = rht_visitor.template as_mut<Internal>();
    rht_node->init();
    node->split(rht_node);
    auto &parent_visitor = visitors.back();
    auto parent_node = parent_visitor.template as_mut<Internal>();
    auto pos = parent_node->locate_pair(node->key(0), node->value(0), kv_compare_);
    parent_node->insert(pos + 1, rht_node->key(0), rht_node->value(0), rht_ptr);
  }
  auto root_visitor = std::move(visitors.back());
  visitors.pop_back();
  if(!root_visitor.template as<Base>()->too_large())
    return true;
  auto root_node = root_visitor.template as_mut<Internal>();
  auto rht_ptr = buf_pool_.alloc();
  auto rht_visitor = buf_pool_.visitor(rht_ptr);
  auto rht_node = rht_visitor.template as_mut<Internal>();
  rht_node->init();
  root_node->split(rht_node);
  auto new_root_ptr = buf_pool_.alloc();
  auto new_root_visitor = buf_pool_.visitor(new_root_ptr);
  auto new_root_node = new_root_visitor.template as_mut<Internal>();
  new_root_node->init();
  new_root_node->insert(0, root_node->key(0), root_node->value(0), root_ptr_);
  new_root_node->insert(1, rht_node->key(0), rht_node->value(0), rht_ptr);
  root_ptr_ = new_root_ptr;
  return true;
}

template <class KeyT, class ValueT, class KeyCompare, class ValueCompare>
bool MultiBpt<KeyT, ValueT, KeyCompare, ValueCompare>::remove(const KeyT &key, const ValueT &value) {
  if(root_ptr_ == NULL_PAGE_ID)
    return false;
  vector<Visitor> visitors;
  visitors.push_back(buf_pool_.visitor(root_ptr_));
  while(!visitors.back().template as<Base>()->is_leaf()) {
    auto node = visitors.back().template as<Internal>();
    auto pos = node->locate_pair(key, value, kv_compare_);
    auto ptr = node->child(pos);
    visitors.push_back(buf_pool_.visitor(ptr));
  }
  auto leaf_visitor = std::move(visitors.back());
  visitors.pop_back();
  auto leaf_immut = leaf_visitor.template as<Leaf>();
  auto leaf_pos = leaf_immut->locate_pair(key, value, kv_compare_);
  if(leaf_pos == leaf_immut->size() ||
    !key_equal(leaf_immut->key(leaf_pos), key) ||
    !value_equal(leaf_immut->value(leaf_pos), value))
    return false;
  auto leaf = leaf_visitor.template as_mut<Leaf>();
  leaf->remove(leaf_pos);
  if(!leaf->too_small())
    return true;
  {
    if(visitors.empty()) {
      if(leaf->size() == 0) {
        leaf_visitor.drop();
        buf_pool_.dealloc(root_ptr_);
        root_ptr_ = NULL_PAGE_ID;
      }
      return true;
    }
    auto &parent_visitor = visitors.back();
    auto parent_node = parent_visitor.template as_mut<Internal>();
    auto pos = parent_node->locate_pair(leaf->key(0), leaf->value(0), kv_compare_);
    if(pos > 0) {
      auto lft_writer = buf_pool_.visitor(parent_node->child(pos - 1));
      auto lft_node = lft_writer.template as_mut<Leaf>();
      if(lft_node->size() + leaf->size() <= lft_node->merge_bound()) {
        lft_node->coalesce(leaf);
        leaf_visitor.drop();
        buf_pool_.dealloc(parent_node->child(pos));
        parent_node->remove(pos);
      } else {
        lft_node->redistribute_right(leaf);
        parent_node->update(pos, leaf->key(0), leaf->value(0));
      }
    } else {
      auto rht_writer = buf_pool_.visitor(parent_node->child(pos + 1));
      auto rht_node = rht_writer.template as_mut<Leaf>();
      if(leaf->size() + rht_node->size() <= rht_node->merge_bound()) {
        leaf->coalesce(rht_node);
        rht_writer.drop();
        buf_pool_.dealloc(parent_node->child(pos + 1));
        parent_node->remove(pos + 1);
      } else {
        rht_node->redistribute_left(leaf);
        parent_node->update(pos + 1, rht_node->key(0), rht_node->value(0));
      }
    }
  }
  leaf_visitor.drop();
  while(visitors.size() > 1) {
    auto visitor = std::move(visitors.back());
    visitors.pop_back();
    if(!visitor.template as<Base>()->too_small())
      return true;
    auto node = visitor.template as_mut<Internal>();

    auto &parent_visitor = visitors.back();
    auto parent_node = parent_visitor.template as_mut<Internal>();
    auto pos = parent_node->locate_pair(node->key(0), node->value(0), kv_compare_);
    if(pos > 0) {
      auto lft_writer = buf_pool_.visitor(parent_node->child(pos - 1));
      auto lft_node = lft_writer.template as_mut<Internal>();
      if(lft_node->size() + node->size() <= lft_node->merge_bound()) {
        lft_node->coalesce(node);
        leaf_visitor.drop();
        buf_pool_.dealloc(parent_node->value(pos));
        parent_node->remove(pos);
      } else {
        lft_node->redistribute_right(node);
        parent_node->update(pos, node->key(0), node->value(0));
      }
    } else {
      auto rht_writer = buf_pool_.visitor(parent_node->child(pos + 1));
      auto rht_node = rht_writer.template as_mut<Internal>();
      if(node->size() + rht_node->size() <= rht_node->merge_bound()) {
        node->coalesce(rht_node);
        rht_writer.drop();
        buf_pool_.dealloc(parent_node->value(pos + 1));
        parent_node->remove(pos + 1);
      } else {
        rht_node->redistribute_left(node);
        parent_node->update(pos + 1, rht_node->key(0), rht_node->value(0));
      }
    }
  }
  auto root_visitor = std::move(visitors.back());
  visitors.pop_back();
  if(root_visitor.template as<Base>()->size() > 1)
    return true;
  auto root_node = root_visitor.template as<Internal>();
  auto new_root_ptr = root_node->value(0);
  // root_node->remove(0);
  root_visitor.drop();
  buf_pool_.dealloc(root_ptr_);
  root_ptr_ = new_root_ptr;
  return true;
}




}

#endif