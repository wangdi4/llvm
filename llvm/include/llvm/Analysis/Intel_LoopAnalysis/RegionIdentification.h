//===------- RegionIdentification.h - Identifies HIR regions ---*- C++ --*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This analysis is used to identify sections(regions) of LLVM IR on which loop
// transformations can be applied.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_REGIONIDENTIFICATION_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_REGIONIDENTIFICATION_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/DenseMap.h"

#include "llvm/Pass.h"

namespace llvm {

class Value;
class Function;
class Loop;
class LoopInfo;
class DominatorTree;
class ScalarEvolution;

namespace loopopt {

class IRRegion;

/// \brief This analysis is the first step in creating HIR. We start by
/// identiyfing regions as a set of basic blocks in the incoming IR. This
/// information is then used by HIRCreation pass to create and populate
/// HIR regions.
class RegionIdentification : public FunctionPass {
public:
  typedef SmallVector<IRRegion *, 16> IRRegionsTy;

  /// Iterators to iterate over regions
  typedef IRRegionsTy::iterator iterator;
  typedef IRRegionsTy::const_iterator const_iterator;
  typedef IRRegionsTy::reverse_iterator reverse_iterator;
  typedef IRRegionsTy::const_reverse_iterator const_reverse_iterator;

private:
  /// IRRegions - Vector of IRRegion.
  IRRegionsTy IRRegions;

  /// LI - The loop information for the function we are currently analyzing.
  LoopInfo *LI;

  /// DT - The dominator tree.
  DominatorTree *DT;

  /// SE - Scalar Evolution analysis for the function.
  ScalarEvolution *SE;

  /// \brief Returns true if Lp appears to be generable without looking at the
  /// sub loops.
  bool isSelfGenerable(const Loop &Lp, unsigned LoopnestDepth) const;

  /// \brief Creates a Region out of Lp's basic blocks.
  void createRegion(const Loop &Lp);

  /// \brief Returns true if we can form a region around this loop. Returns the
  /// max loopnest depth in LoopnestDepth.
  bool formRegionForLoop(const Loop &Lp, unsigned *LoopnestDepth);

  /// \brief Identifies regions in the incoming LLVM IR.
  void formRegions();

public:
  static char ID; // Pass identification
  RegionIdentification();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void verifyAnalysis() const override;

  /// IRRegion iterator methods
  iterator begin() { return IRRegions.begin(); }
  const_iterator begin() const { return IRRegions.begin(); }
  iterator end() { return IRRegions.end(); }
  const_iterator end() const { return IRRegions.end(); }

  reverse_iterator rbegin() { return IRRegions.rbegin(); }
  const_reverse_iterator rbegin() const { return IRRegions.rbegin(); }
  reverse_iterator rend() { return IRRegions.rend(); }
  const_reverse_iterator rend() const { return IRRegions.rend(); }

  unsigned getNumRegions() const { return IRRegions.size(); }
};

} // End namespace loopopt

} // End namespace llvm

#endif
