//===- IntelVPlanDominatorTree.h---------------------------------*- C++ -*-===//
//
//   Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
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

namespace llvm {
namespace vpo {

/// Template specialization of the standard LLVM dominator tree utility for
/// VPBlockBases.
using VPDominatorTree = DomTreeBase<VPBlockBase>;

using VPDomTreeNode = DomTreeNodeBase<VPBlockBase>;

/// Template specialization of the standard LLVM post-dominator tree utility for
/// VPBlockBases.
using VPPostDominatorTree = PostDomTreeBase<VPBlockBase>;

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
