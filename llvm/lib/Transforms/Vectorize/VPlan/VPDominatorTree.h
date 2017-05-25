#ifndef VPLAN_VPDOMINATORTREE_H
#define VPLAN_VPDOMINATORTREE_H

#include "../VPlan.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/IR/Dominators.h"

namespace llvm {

class VPBlockBase;
/// \brief Template specialization of the standard LLVM dominator tree utility
/// for VPBlocks.
class VPDominatorTree : public DominatorTreeBase<VPBlockBase> {
public:
  VPDominatorTree(bool isPostDom) : DominatorTreeBase<VPBlockBase>(isPostDom) {}

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
