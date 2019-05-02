#if INTEL_COLLAB // -*- C++ -*-
//===--------- CFGRestructuring.h - Restructures CFG for VPO*- C++ -*------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This file declares the VPOCFGRestructuringPass pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_UTILS_CFGRESTRUCTURING_H
#define LLVM_TRANSFORMS_VPO_UTILS_CFGRESTRUCTURING_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// Re-constructs CFG with single-entry-single-exit BBLOCKS. Entry BBLOCK
/// contains `@llvm.directive.region.entry(...)` Instruction and Exit BBLOCK
/// contains `@llvm.directive.region.exit(...)` Instruction.
///
/// This pass invokes the VPOUtils::CFGRestructuring() on a given function.
/// The CFG for the function is restructured, such that each directive for Cilk,
/// OpenMP, Offload, Vectorization is put into a standalone basic block. This is
/// a pre-required process for constructing WRegion for a function.
/// \code
///            Before                 |            After
///          -------------------------+---------------------------
/// B1:                               |       B1:
///   %1 = begin_region()             |         %1 = begin_region()
///                                   |         br B2
///                                   |       B2:
///   ...                             |         ...
///                                   |         br B3
///                                   |       B3:
///   end_region(%1)                  |         end_region(%1)
/// \endcode
class VPOCFGRestructuringPass : public PassInfoMixin<VPOCFGRestructuringPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  bool runImpl(Function &F, DominatorTree *DT, LoopInfo *LI);
};
} // end namespace llvm

#endif // LLVM_TRANSFORMS_VPO_UTILS_CFGRESTRUCTURING_H

#endif // INTEL_COLLAB
