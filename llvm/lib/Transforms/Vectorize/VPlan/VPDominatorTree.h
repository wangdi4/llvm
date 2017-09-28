#ifndef VPLAN_VPDOMINATORTREE_H
#define VPLAN_VPDOMINATORTREE_H

#include "../VPlan.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/IR/Dominators.h"

namespace llvm {

// ***** ATTENTION: This file is not used for now because there was a cyclic
// dependency between this header and VPlan.h. After the last pulldown, this
// caused some forward declaration errors on VPBlockBase depending on the order
// in which VPlan.h or this header was included from other files. Thus, this
// code was moved to VPlan.h.

//class VPBlockBase;
/// \brief Template specialization of the standard LLVM dominator tree utility
/// for VPBlocks.
class VPDominatorTree : public DominatorTreeBase<VPBlockBase, false> {
public:
  VPDominatorTree(bool isPostDom) : DominatorTreeBase<VPBlockBase, false>() {}

  virtual ~VPDominatorTree() {}
};

typedef DomTreeNodeBase<VPBlockBase> VPDomTreeNode;

template <>
struct GraphTraits<VPDomTreeNode *>
    : public DomTreeGraphTraitsBase<VPDomTreeNode, VPDomTreeNode::iterator> {};

template <>
struct GraphTraits<const VPDomTreeNode *>
    : public DomTreeGraphTraitsBase<const VPDomTreeNode,
                                    VPDomTreeNode::const_iterator> {};
} // namespace llvm
#endif
