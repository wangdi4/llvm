#if INTEL_COLLAB // -*- C++ -*-
//===--------- CFGRestructuring.h - Restructures CFG for VPO*- C++ -*------===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass invokes the CFGRestructuring member function of VPOUtils class on
// a function. The CFG for the function is restructured, such that each
// directive for Cilk, OpenMP, Offload, Vectorization is put into a standalone
// basic block. This is a pre-required process for constructing WRegion for a
// function.
//
//            Before                 |            After
//          -------------------------+---------------------------
// B1:                               |       B1:
//   %1 = begin_region()             |         %1 = begin_region()
//                                   |         br B2
//                                   |       B2:
//   ...                             |         ...
//                                   |         br B3
//                                   |       B3:
//   end_region(%1)                  |         end_region(%1)
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_UTILS_CFGRESTRUCTURING_H
#define LLVM_TRANSFORMS_VPO_UTILS_CFGRESTRUCTURING_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// Re-constructs CFG with single-entry-single-exit BBLOCKS. Entry BBLOCK
/// contains @llvm.directive.region.entry(...) Instruction and Exit BBLOCK
/// contains @llvm.directive.region.exit(...) Instruction.
class VPOCFGRestructuringPass : public PassInfoMixin<VPOCFGRestructuringPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  bool runImpl(Function &F, DominatorTree *DT, LoopInfo *LI);
};
} // end namespace llvm

#endif // LLVM_TRANSFORMS_VPO_UTILS_CFGRESTRUCTURING_H

#endif // INTEL_COLLAB
