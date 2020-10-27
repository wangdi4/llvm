//===- IntelVPlanDominatorTree.h---------------------------------*- C++ -*-===//
//
//   Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements dominator tree analysis for a single level of a VPlan's
/// H-CFG.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANDOMINATORTREE_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANDOMINATORTREE_H

#include "IntelVPlan.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/CfgTraits.h"

namespace llvm {
namespace vpo {

class VPCfgTraitsBase : public CfgTraitsBase {
public:
  using ParentType = VPlan;
  using BlockRef = VPBasicBlock *;
  using ValueRef = VPValue *;

  static CfgBlockRef wrapRef(BlockRef block) {
    return makeOpaque<CfgBlockRefTag>(block);
  }
  static CfgValueRef wrapRef(ValueRef block) {
    return makeOpaque<CfgValueRefTag>(block);
  }
  static BlockRef unwrapRef(CfgBlockRef block) {
    return static_cast<BlockRef>(getOpaque(block));
  }
  static ValueRef unwrapRef(CfgValueRef block) {
    return static_cast<ValueRef>(getOpaque(block));
  }
};

class VPCfgTraits : public CfgTraits<VPCfgTraitsBase, VPCfgTraits> {
  explicit VPCfgTraits(VPlan * /*parent*/) {}

  static VPlan *getBlockParent(VPBasicBlock *Block) {
    return Block->getParent();
  }

  static auto predecessors(VPBasicBlock *Block) {
    return Block->getPredecessors();
  }
  static auto successors(VPBasicBlock *Block) { return Block->getSuccessors(); }
};
} // namespace vpo
template <> struct CfgTraitsFor<vpo::VPBasicBlock> {
  using CfgTraits = vpo::VPCfgTraits;
};
namespace vpo {
/// Template specialization of the standard LLVM dominator tree utility for
/// VPBasicBlocks.
class VPDominatorTree : public DomTreeBase<VPBasicBlock> {};

using VPDomTreeNode = DomTreeNodeBase<VPBasicBlock>;

/// Template specialization of the standard LLVM post-dominator tree utility for
/// VPBasicBlocks.
class VPPostDominatorTree : public PostDomTreeBase<VPBasicBlock> {
  using Base = PostDomTreeBase<VPBasicBlock>;

public:
  /// Ensure base class overloads are visible.
  using Base::dominates;

  /// Return true if \p I1 dominates \p I2. This checks if \p I2 comes before
  /// \p I1 if they belongs to the same basic block.
  bool dominates(const VPInstruction *I1, const VPInstruction *I2) const {
    assert(I1 && I2 && "Expecting valid I1 and I2.");

    const VPBasicBlock *BB1 = I1->getParent();
    const VPBasicBlock *BB2 = I2->getParent();

    if (BB1 != BB2)
      return Base::dominates(BB1, BB2);

    // An instruction post dominates itself.
    if (I1 == I2)
      return true;

    // PHI nodes in a block are unordered.
    if (isa<VPPHINode>(I1) && isa<VPPHINode>(I2))
      return false;

    // Loop through the basic block until we find I1 or I2.
    VPBasicBlock::const_iterator I = BB1->begin();
    for (; &*I != I1 && &*I != I2; ++I)
      /*empty*/;

    return &*I == I2;
  }
};

} // namespace vpo

/// Template specializations of GraphTraits for VPDomTreeNode.
template <>
struct GraphTraits<vpo::VPDomTreeNode *>
    : public DomTreeGraphTraitsBase<vpo::VPDomTreeNode,
                                    vpo::VPDomTreeNode::iterator> {};

template <>
struct GraphTraits<const vpo::VPDomTreeNode *>
    : public DomTreeGraphTraitsBase<const vpo::VPDomTreeNode,
                                    vpo::VPDomTreeNode::const_iterator> {};
} // namespace llvm
#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANDOMINATORTREE_H
