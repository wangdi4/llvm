//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2017 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPDOMINATORTREE_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPDOMINATORTREE_H

#include "../Intel_VPlan.h"
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
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_VPDOMINATORTREE_H
