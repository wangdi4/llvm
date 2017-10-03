//===-- VectorGraphPredicator.h ---------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VectorGraphPredicator.h -- Defines the Vector Graph Predication analysis.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_VGPREDICATOR_H
#define LLVM_TRANSFORMS_VPO_VGPREDICATOR_H

#include "llvm/Pass.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/Intel_VPO/Vecopt/VectorGraph.h"
#include "llvm/Analysis/ScalarEvolution.h"

namespace llvm {

class VectorGraphInfo;
class VGLoop;
class VGBlock;
class VGSESERegion;

/// \brief Template specialization of the standard LLVM dominator tree utility
/// for VGBlock CFGs.
class VGDominatorTree : public DomTreeBase<VGBlock> {
public:
  VGDominatorTree() : DomTreeBase<VGBlock>() {}
};

/// \brief Template specialization of the standard LLVM post-dominator tree
/// utility for VGBlock CFGs.
class VGPostDominatorTree : public PostDomTreeBase<VGBlock> {
public:
  VGPostDominatorTree() : PostDomTreeBase<VGBlock>() {}
};

class VectorGraphPredicator : public FunctionPass {

public:

  /// Pass Identification
  static char ID;

  /// Pass constructor
  VectorGraphPredicator();
  /// Utility constructor
  VectorGraphPredicator(ScalarEvolution *SE);

  bool runOnFunction(Function &F);
  void getAnalysisUsage(AnalysisUsage &AU) const override;

  void runOnAvr(VGLoop* ALoop);

private:

  VectorGraphInfo *VGI;
  ScalarEvolution *SE;

  void predicateLoop(VGLoop* ALoop);
  void handleVGSESERegion(const VGSESERegion *Region, VGLoop *ALoop,
                          const VGDominatorTree &DomTree);
  void predicate(VGBlock *Entry, VGLoop *Loop, const VGDominatorTree &DomTree);
  // void removeCFG(AVRBlock* Entry);
};

} // End namespace llvm

#endif // LLVM_TRANSFORMS_VPO_VGPREDICATOR_H
