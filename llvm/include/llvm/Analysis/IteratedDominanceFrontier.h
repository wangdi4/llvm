//===- IteratedDominanceFrontier.h - Calculate IDF --------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_IDF_H
#define LLVM_ANALYSIS_IDF_H

#include "llvm/IR/CFGDiff.h"
#include "llvm/Support/GenericIteratedDominanceFrontier.h"
#if INTEL_CUSTOMIZATION
#include "../../../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlan.h"
#include "../../../lib/Transforms/Vectorize/Intel_VPlan/IntelVPlanDominatorTree.h"
#endif // INTEL_CUSTOMIZATION

namespace llvm {

class BasicBlock;

namespace IDFCalculatorDetail {

/// Specialization for BasicBlock and VPBlockBase for the optional   // INTEL
/// use of GraphDiff.                                                // INTEL
template <typename BlockTy, bool IsPostDom>                          // INTEL
struct BBChildrenGetterTy {                                          // INTEL
  using NodeRef = BlockTy *;                                         // INTEL
  using ChildrenTy = SmallVector<BlockTy *, 8>;                      // INTEL

  BBChildrenGetterTy() = default;                                    // INTEL
  BBChildrenGetterTy(const GraphDiff<BlockTy *, IsPostDom> *GD)      // INTEL
      : GD(GD) {                                                     // INTEL
    assert(GD);
  }

  ChildrenTy get(const NodeRef &N);

  const GraphDiff<BlockTy *, IsPostDom> *GD = nullptr;               // INTEL
};
#if INTEL_CUSTOMIZATION

template <bool IsPostDom>
struct ChildrenGetterTy<BasicBlock, IsPostDom>
    : BBChildrenGetterTy<BasicBlock, IsPostDom> {
  using BBChildrenGetterTy<BasicBlock, IsPostDom>::BBChildrenGetterTy;
};

template <bool IsPostDom>
struct ChildrenGetterTy<vpo::VPBlockBase, IsPostDom>
    : BBChildrenGetterTy<vpo::VPBlockBase, IsPostDom> {
  using BBChildrenGetterTy<vpo::VPBlockBase, IsPostDom>::BBChildrenGetterTy;
};
#endif // INTEL_CUSTOMIZATION

} // end of namespace IDFCalculatorDetail

template <typename BlockTy, bool IsPostDom>                          // INTEL
class IDFCalculator final                                            // INTEL
    : public IDFCalculatorBase<BlockTy, IsPostDom> {                 // INTEL
public:
  using IDFCalculatorBase =
      typename llvm::IDFCalculatorBase<BlockTy, IsPostDom>;          // INTEL
  using ChildrenGetterTy = typename IDFCalculatorBase::ChildrenGetterTy;

  IDFCalculator(DominatorTreeBase<BlockTy, IsPostDom> &DT)           // INTEL
      : IDFCalculatorBase(DT) {}

  IDFCalculator(DominatorTreeBase<BlockTy, IsPostDom> &DT,           // INTEL
                const GraphDiff<BlockTy *, IsPostDom> *GD)           // INTEL
      : IDFCalculatorBase(DT, ChildrenGetterTy(GD)) {
    assert(GD);
  }
};

using ForwardIDFCalculator = IDFCalculator<BasicBlock, false>;       // INTEL
using ReverseIDFCalculator = IDFCalculator<BasicBlock, true>;        // INTEL
#if INTEL_CUSTOMIZATION
using VPlanForwardIDFCalculator = IDFCalculator<vpo::VPBlockBase, false>;
#endif // INTEL_CUSTOMIZATION

//===----------------------------------------------------------------------===//
// Implementation.
//===----------------------------------------------------------------------===//

namespace IDFCalculatorDetail {

template <typename BlockTy, bool IsPostDom>                          // INTEL
typename BBChildrenGetterTy<BlockTy, IsPostDom>::ChildrenTy          // INTEL
BBChildrenGetterTy<BlockTy, IsPostDom>::get(                         // INTEL
    const BBChildrenGetterTy<BlockTy, IsPostDom>::NodeRef &N) {      // INTEL

  using OrderedNodeTy =
      typename IDFCalculatorBase<BlockTy, IsPostDom>::OrderedNodeTy; // INTEL

  if (!GD) {
    auto Children = children<OrderedNodeTy>(N);
    return {Children.begin(), Children.end()};
  }

  using SnapShotBBPairTy =                                           // INTEL
      std::pair<const GraphDiff<BlockTy *, IsPostDom> *,             // INTEL
                OrderedNodeTy>;                                      // INTEL

  ChildrenTy Ret;
  for (const auto &SnapShotBBPair : children<SnapShotBBPairTy>({GD, N}))
    Ret.emplace_back(SnapShotBBPair.second);
  return Ret;
}

} // end of namespace IDFCalculatorDetail

} // end of namespace llvm

#endif
