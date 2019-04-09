//===-------- Intel_RegionSplitter.h - Class definition -*- C++ -*---------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This file defines the RegionSplitter class.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_REGION_SPLITTER_H
#define LLVM_TRANSFORMS_INTEL_REGION_SPLITTER_H

#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"

namespace llvm {

// A region of blocks that are to be extracted from the function, and replaced
// with a call to a new function. The new function will take ownership of the
// blocks when the function is split.
using SplinterRegionT = llvm::SmallSetVector<BasicBlock *, 16>;

// Helper class that implements the functionality for splitting a
// SplinterRegion out of the Function.
class RegionSplitter {
public:
  RegionSplitter(DominatorTree &DT, BlockFrequencyInfo &BFI,
                 BranchProbabilityInfo &BPI)
      : DT(DT), BFI(BFI), BPI(BPI) {}

  // Split the function, and return a pointer to the newly created Function.
  Function *splitRegion(const SplinterRegionT &Region);

  // Split the function using a loop body
  Function *splitRegion(Loop &LoopRegion);

  // Return 'true' if there is only a single entry basic block that enters the
  // region, and all exits from the region go to the same basic block outside of
  // the region (or all paths out of the region return from the function).
  bool isSingleEntrySingleExit(SplinterRegionT &Region);

private:
  // Update the IR to prepare the PHINodes of function to receive values
  // from the split out code.
  bool prepareRegionForSplit(const SplinterRegionT &Region);

  // Do the actual split.
  Function *doSplit(const SplinterRegionT &Region);

  // Do the split with the input loop
  Function *doSplit(Loop &L);

  // Set the properties in the new function
  void setProperties(Function &NewF);

  DominatorTree &DT;
  BlockFrequencyInfo &BFI;
  BranchProbabilityInfo &BPI;

};

} // end llvm namespace

#endif // LLVM_TRANSFORMS_INTEL_REGION_SPLITTER_H
