#ifndef INSOMNIA_BPLUSTREE_NODES_TCC
#define INSOMNIA_BPLUSTREE_NODES_TCC

#include "bplustree_nodes.h"

namespace insomnia {

class BptNodeBase {

};

template <class Key>
class BptInternalNode : public BptNodeBase {

};

template <class Key, class Value>
class BptLeafNode : public BptNodeBase {

};

}

#endif