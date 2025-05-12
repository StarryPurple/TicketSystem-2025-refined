#ifndef INSOMNIA_MAP_TCC
#define INSOMNIA_MAP_TCC

#include "map.h"

namespace insomnia {

template <class Key, class Tp, class Compare>
void map<Key, Tp, Compare>::recurse_clear(Node *node) {
  // node to delete
  // if(node == nullptr) return; // for the deconstruction of rvalue-moved map.
  if(node->left != nullptr) recurse_clear(node->left);
  if(node->right != nullptr) recurse_clear(node->right);
  delete node;
}

template <class Key, class Tp, class Compare>
void map<Key, Tp, Compare>::recurse_copy(Node *des, Node *src, const map &other) {
  // src already copied to des
  if(src->left != nullptr) {
    des->left = new Node(src->left->value);
    des->left->parent = des;
    des->left->color = src->left->color;
    if(other._left_most == src->left) _left_most = des->left;
    recurse_copy(des->left, src->left, other);
  }
  if(src->right != nullptr) {
    des->right = new Node(src->right->value);
    des->right->parent = des;
    des->right->color = src->right->color;
    if(other._right_most == src->right) _right_most = des->right;
    recurse_copy(des->right, src->right, other);
  }
}

template <class Key, class Tp, class Compare>
void map<Key, Tp, Compare>::left_rotate(Node *node) {
  Node *child = node->right;
  child->parent = node->parent;
  if(child->parent == nullptr) _root = child;
  else if(child->parent->left == node) child->parent->left = child;
  else child->parent->right = child;
  node->right = child->left;
  if(node->right != nullptr) node->right->parent = node;
  child->left = node;
  node->parent = child;
}

template <class Key, class Tp, class Compare>
void map<Key, Tp, Compare>::right_rotate(Node *node) {
  Node *child = node->left;
  child->parent = node->parent;
  if(child->parent == nullptr) _root = child;
  else if(child->parent->left == node) child->parent->left = child;
  else child->parent->right = child;
  node->left = child->right;
  if(node->left != nullptr) node->left->parent = node;
  child->right = node;
  node->parent = child;
}

template <class Key, class Tp, class Compare>
typename map<Key, Tp, Compare>::Node*
  map<Key, Tp, Compare>::get_next(Node *node) const {
  if(node == nullptr) return _left_most;
  if(node == _right_most) return nullptr;
  // if node == root here, for node != right_most_, we must have node->right != nullptr.
  // so is in the loop.
  if(node->right == nullptr) {
    while(node->parent->right == node) node = node->parent;
    node = node->parent;
    return node;
  }
  node = node->right;
  while(node->left != nullptr) node = node->left;
  return node;
}

template <class Key, class Tp, class Compare>
typename map<Key, Tp, Compare>::Node*
  map<Key, Tp, Compare>::get_prev(Node *node) const {
  if(node == nullptr) return _right_most;
  if(node == _left_most) return nullptr;
  // if node == root here, for node != left_most_, we must have node->left != nullptr.
  // so is in the loop.
  if(node->left == nullptr) {
    while(node->parent->left == node) node = node->parent;
    node = node->parent;
    return node;
  }
  node = node->left;
  while(node->right != nullptr) node = node->right;
  return node;
}

template <class Key, class Tp, class Compare>
void map<Key, Tp, Compare>::insertion_maintain(Node *node) {
  // This node should be red.
  // maintain upwards.

  // Case 1: the new node is the root / tree is previously empty.
  if(node == _root) {
    node->color = Node::Color::Black;
    return;
  }

  // node has parent.
  Node *parent = node->parent;
  // Case 2: parent is black.
  if(parent->color == Node::Color::Black) return;

  // parent is red.
  // Case 3: parent is red root.
  if(parent == _root) {
    parent->color = Node::Color::Black;
    return;
  }

  // node has grandparent (black).
  Node *grandparent = parent->parent;
  Node *uncle = (grandparent->left == parent) ? grandparent->right : grandparent->left;
  // Case 4: uncle node exists, and it's red.
  if(uncle != nullptr && uncle->color == Node::Color::Red) {
    parent->color = uncle->color = Node::Color::Black;
    grandparent->color = Node::Color::Red;
    insertion_maintain(grandparent);
    return;
  }

  // uncle node doesn't exist or is black.
  // Case 5: the direction of the two parent-child relationship isn't identical.
  if((grandparent->left == parent) ^ (parent->left == node)) {
    if(parent->left == node) right_rotate(parent);
    else left_rotate(parent);
    std::swap(node, parent);
    // Go on to case 6.
  }

  // Case 6: the direction of the two parent-child relationship is identical.
  parent->color = Node::Color::Black;
  grandparent->color = Node::Color::Red;
  if(grandparent->left == parent) right_rotate(grandparent);
  else left_rotate(grandparent);
}

template <class Key, class Tp, class Compare>
void map<Key, Tp, Compare>::erasure_maintain(Node *node) {
    // The black length of parent of this node has just been shortened by 1.
    // the ancestors of this node might be affected.
    // maintain upwards.

    // Case 1: node is root.
    if(node == _root) return; // no actual node is affected.

    // node here has parent.
    Node *parent = node->parent;
    Node *sibling = (parent->left == node) ? parent->right : parent->left;
    // Check case: sibling not exist.
    if(sibling == nullptr) {
      // at least parent node is not affected.
      // other ancestors are still in need of maintenance.
      erasure_maintain(parent);
      return;
    }

    // Case 2: sibling node is red.
    if(sibling->color == Node::Color::Red) {
      // parent should be black.
      // children of sibling nodes should be black.
      parent->color = Node::Color::Red;
      sibling->color = Node::Color::Black;
      if(parent->left == sibling) {
        right_rotate(parent);
        sibling = parent->left;
      } else {
        left_rotate(parent);
        sibling = parent->right;
      }
      // now sibling node is black (or not exist).
      // continue to further maintenance.
    }

    // Check case: sibling not exist.
    if(sibling == nullptr) {
      // at least parent node is not affected.
      // other ancestors are still in need of maintenance.
      erasure_maintain(parent);
      return;
    }

    // Case 3: sibling has no red children
    if((sibling->left == nullptr || sibling->left->color == Node::Color::Black)
      && (sibling->right == nullptr || sibling->right->color == Node::Color::Black)) {
      sibling->color = Node::Color::Black;
      if(parent->color == Node::Color::Red) {
        parent->color = Node::Color::Black;
        return;
      } else {
        erasure_maintain(parent);
        return;
      }
    }

    // Case 4: sibling has at least one red children
    // Modify: make sibling's opposite-side (compared to node's side in parent) child is red.
    if(node == parent->left) {
      if(sibling->right == nullptr || sibling->right->color == Node::Color::Black) {
        // sibling->left->color == Red
        sibling->left->color = Node::Color::Black;
        sibling->color = Node::Color::Red;
        right_rotate(sibling);
        sibling = parent->right;
      }
      // now sibling->right->color = Red
      sibling->color = parent->color;
      parent->color = Node::Color::Black;
      sibling->right->color = Node::Color::Black;
      left_rotate(parent);
      return;
    } else {
      // node == parent_right
      if(sibling->left == nullptr || sibling->left->color == Node::Color::Black) {
        // sibling->right->color == Red
        sibling->right->color = Node::Color::Black;
        sibling->color = Node::Color::Red;
        left_rotate(sibling);
        sibling = parent->left;
      }
      // now sibling->left->color = Red
      sibling->color = parent->color;
      parent->color = Node::Color::Black;
      sibling->left->color = Node::Color::Black;
      right_rotate(parent);
      return;
    }
  }

template <class Key, class Tp, class Compare>
typename map<Key, Tp, Compare>::iterator
map<Key, Tp, Compare>::find(const Key &key) {
  if(empty()) return end();
  Node *node = _root;
  while(node != nullptr) {
    if(_comparer(key, node->value.first))
      node = node->left;
    else if(_comparer(node->value.first, key))
      node = node->right;
    else return iterator(this, node);
  }
  return end();
}

template <class Key, class Tp, class Compare>
typename map<Key, Tp, Compare>::const_iterator
map<Key, Tp, Compare>::find(const Key &key) const {
  if(empty()) return cend();
  Node *node = _root;
  while(node != nullptr) {
    if(_comparer(key, node->value.first))
      node = node->left;
    else if(_comparer(node->value.first, key))
      node = node->right;
    else return const_iterator(this, node);
  }
  return cend();
}

template <class Key, class Tp, class Compare>
Tp& map<Key, Tp, Compare>::operator[](const Key &key) {
  static_assert(std::is_default_constructible<Tp>::value,
    "The type of value (Tp) should be default constructible "
    "if you want to use non-const operator[]");
  if(empty()) {
    _size = 1;
    _root = _left_most = _right_most = new Node(value_type(key, Tp()));
    return _root->value.second;
  }
  Node *node = _root, *parent = nullptr;
  bool is_left = true;
  // at lease one comparison would be done.
  while(node != nullptr) {
    if(_comparer(key, node->value.first)) {
      parent = node;
      node = node->left;
      is_left = true;
    } else if(_comparer(node->value.first, key)) {
      parent = node;
      node = node->right;
      is_left = false;
    } else return node->value.second;
  }
  ++_size;
  Node *res = new Node(value_type(key, Tp()));
  res->parent = parent;
  if(is_left) {
    parent->left = res;
    if(parent == _left_most) _left_most = res;
  } else {
    parent->right = res;
    if(parent == _right_most) _right_most = res;
  }
  insertion_maintain(res);
  return res->value.second;
}

template <class Key, class Tp, class Compare>
std::pair<typename map<Key, Tp, Compare>::iterator, bool>
map<Key, Tp, Compare>::insert(const value_type &value){
  if(empty()) {
    _size = 1;
    _root = _left_most = _right_most = new Node(value);
    return std::pair<iterator, bool>(iterator(this, _root), true);
  }
  Node *node = _root, *parent = nullptr;
  bool is_left = true;
  // at lease one comparison would be done.
  while(node != nullptr) {
    if(_comparer(value.first, node->value.first)) {
      parent = node;
      node = node->left;
      is_left = true;
    } else if(_comparer(node->value.first, value.first)) {
      parent = node;
      node = node->right;
      is_left = false;
    } else return std::pair<iterator, bool>(iterator(this, node), false);
  }
  ++_size;
  Node *res = new Node(value);
  res->parent = parent;
  if(is_left) {
    parent->left = res;
    if(parent == _left_most) _left_most = res;
  } else {
    parent->right = res;
    if(parent == _right_most) _right_most = res;
  }
  insertion_maintain(res);
  return std::pair<iterator, bool>(iterator(this, res), true);
}

template <class Key, class Tp, class Compare>
void map<Key, Tp, Compare>::erase(iterator pos) {
    // I assume this pos is a valid iterator that are in use (and, including end() and rend() for now).
    if(pos.container != this || empty() || pos == end()) throw invalid_iterator();
    if(_size == 1) {
      if(pos.node != _root) throw invalid_iterator();
      delete _root;
      _root = nullptr;
      _left_most = _right_most = nullptr;
      _size = 0;
      return;
    }
    --_size;
    Node *node = pos.node;
    if(node == _right_most) _right_most = get_prev(node);
    if(node == _left_most) _left_most = get_next(node);
    if(node->left != nullptr && node->right != nullptr) {
      Node *prev = get_prev(node);
      // This approach works, but it breaks the legality of iterator to prev.
      /*
      delete node->value;
      node->value = new value_type(*prev->value);
      if(prev == left_most_) left_most_ = node;
      node = prev;
      */
      if(node->left == prev) {
        // prev->right == nullptr
        right_rotate(node);
        // now node->left == nullptr
      } else {
        Node *node_parent = node->parent, *node_left = node->left, *node_right = node->right;
        Node *prev_parent = prev->parent, *prev_left = prev->left, *prev_right = prev->right;

        node->parent = prev_parent; node->left = prev_left; node->right = prev_right;
        prev->parent = node_parent; prev->left = node_left; prev->right = node_right;
        auto color = node->color; node->color = prev->color; prev->color = color;

        if(node_parent != nullptr) {
          if(node_parent->left == node) node_parent->left = prev;
          else node_parent->right = prev;
        }
        if(node_left != nullptr) node_left->parent = prev;
        if(node_right != nullptr) node_right->parent = prev;
        if(prev_parent != nullptr) {
          if(prev_parent->left == prev) prev_parent->left = node;
          else prev_parent->right = node;
        }
        if(prev_left != nullptr) prev_left->parent = node;
        if(prev_right != nullptr) prev_right->parent = node;
        if(_root == node) _root = prev; // Don't forget this.
      }
      // now (node->left == nullptr || node->right == nullptr) is true.
    }
    // no two-child node here.
    bool to_maintain = (node->color == Node::Color::Black);
    if(node->left == nullptr && node->right == nullptr) {
      // discard it.
      Node *parent = node->parent; // definitely not nullptr, for size_ == 1 case has been handled.
      if(to_maintain) {
        // temporarily use a new nil node whose parent is "parent".
        Node *helper_node = new Node(parent->value);
        helper_node->color = Node::Color::Black; // somehow needless
        helper_node->parent = parent;
        if(parent->left == node) {
          parent->left = helper_node;
          erasure_maintain(helper_node);
          parent->left = nullptr;
        } else {
          parent->right = helper_node;
          erasure_maintain(helper_node);
          parent->right = nullptr;
        }
        helper_node->parent = nullptr;
        delete helper_node;
      } else {
        if(parent->left == node) parent->left = nullptr;
        else parent->right = nullptr;
      }
      delete node;
      return;
    }
    Node *parent = node->parent; // may be nullptr if node == root_
    Node *child = (node->left == nullptr) ? node->right : node->left; // only one side is not nullptr.
    if(parent == nullptr) _root = child;
    else if(parent->left == node) parent->left = child;
    else parent->right = child;
    child->parent = parent;
    child->color = node->color; // important?
    erasure_maintain(child);
    delete node;
  }

}


#endif