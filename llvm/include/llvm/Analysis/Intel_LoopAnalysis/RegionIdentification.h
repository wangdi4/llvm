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
#include "llvm/Pass.h"
#include <set>

namespace llvm {

class Function;
class Loop;
class LoopInfo;
class DominatorTree;
class ScalarEvolution;

namespace loopopt {

class RegionIdentification : public FunctionPass {

  /// RegionBBlocks - Region is defined as a pair of entry BasicBlock and a set
  /// of BasicBlocks (including the entry BasicBlock).
  typedef SmallVector< std::pair< BasicBlock* , std::set<BasicBlock*> >, 16> 
    RegionBBlocksTy;

  RegionBBlocksTy RegionBBlocks;
  
  /// Func - The function we are analyzing.
  Function *Func;

  /// LI - The loop information for the function we are currently analyzing.
  LoopInfo *LI;

  /// DT - The dominator tree.
  DominatorTree *DT; 

  /// SE - Scalar Evolution analysis for the function.
  ScalarEvolution *SE;

public:
  static char ID; // Pass identification
  RegionIdentification();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module* = nullptr) const override;
  void verifyAnalysis() const override;

  const RegionBBlocksTy& getRegionBBlocks() { return RegionBBlocks; }

   /// \brief Returns true if HIR is able to handle this loop.
  bool isCandidateLoop(Loop& Lp);

  /// \brief Identifies regions in the incoming LLVM IR.
  void formRegions();
};

} // End namespace loopopt

} // End namespace llvm

#endif

