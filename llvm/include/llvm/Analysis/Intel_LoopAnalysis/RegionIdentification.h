//===------- RegionIdentification.h - Identifies HIR regions ---*- C++ --*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Pass.h"

namespace llvm {

class Function;
class Loop;
class LoopInfo;
class DominatorTree;
class ScalarEvolution;

namespace loopopt {

/// \brief This analysis is the first step in creating HIR. We start by
/// identiyfing regions as a set of basic blocks in the incoming IR. This
/// information is then used by HIRCreation pass to create and populate
/// HIR regions.
class RegionIdentification : public FunctionPass {
public:
  typedef SmallPtrSet<const BasicBlock *, 32> RegionBBlocksTy;

  /// IRRegion is defined as a pair of entry BasicBlock and a set
  /// of BasicBlocks (including the entry BasicBlock).
  struct IRRegion {
    BasicBlock *EntryBB;
    RegionIdentification::RegionBBlocksTy BasicBlocks;

    IRRegion(BasicBlock *Entry, RegionIdentification::RegionBBlocksTy BBlocks);
    IRRegion(const IRRegion &Reg);
    ~IRRegion();
  };

  typedef SmallVector<IRRegion *, 16> IRRegionsTy;

  /// Iterators to iterate over regions
  typedef IRRegionsTy::const_iterator const_iterator;
  typedef IRRegionsTy::const_reverse_iterator const_reverse_iterator;

private:
  /// IRRegions - Vector of IRRegion.
  IRRegionsTy IRRegions;

  /// Func - The function we are analyzing.
  Function *Func;

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
  const_iterator begin() const { return IRRegions.begin(); }
  const_iterator end() const { return IRRegions.end(); }

  const_reverse_iterator rbegin() const { return IRRegions.rbegin(); }
  const_reverse_iterator rend() const { return IRRegions.rend(); }
};

} // End namespace loopopt

} // End namespace llvm

#endif
