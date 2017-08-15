//===--------- HIRLoopFormation.h - Creates HIR loops ---------*-- C++ --*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This analysis is used to create HIR loops inside HIR Regions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_LOOPFORMATION_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_LOOPFORMATION_H

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Pass.h"

namespace llvm {

class Function;
class Loop;
class LoopInfo;
class ScalarEvolution;

namespace loopopt {

class HLRegion;
class HLLoop;
class HLIf;
class HIRRegionIdentification;
class HIRCreation;
class HIRCleanup;

/// \brief This analysis forms HIR loops within HIR regions created by the
/// HIRCreation pass.
class HIRLoopFormation : public FunctionPass {
public:
  typedef std::pair<const Loop *, HLLoop *> LoopPairTy;

private:
  /// Func - The function we are analyzing.
  Function *Func;

  /// LI - The loop information for the function we are currently analyzing.
  LoopInfo *LI;

  /// SE - Scalar Evolution analysis for the function.
  ScalarEvolution *SE;

  /// RI - Pointer to HIRRegionIdentification pass.
  HIRRegionIdentification *RI;

  /// HIR - Pointer to HIRCreation pass.
  HIRCreation *HIR;

  /// HIRC - Pointer to HIRCleanup pass.
  HIRCleanup *HIRC;

  /// Loops - Sorted vector of Loops to HLLoops.
  SmallVector<LoopPairTy, 32> Loops;

  /// Contains loops which require inversion of ztt predicate.
  SmallPtrSet<HLLoop *, 16> InvertedZttLoops;

  /// \brief Inserts (Lp, HLoop) pair in the map.
  void insertHLLoop(const Loop *Lp, HLLoop *HLoop);

  /// \brief Implements find()/insert() functionality.
  HLLoop *findOrInsertHLLoopImpl(const Loop *Lp, HLLoop *HLoop, bool Insert);

  /// \brief Returns true if Inst represents a non-negative NSW SCEVAddRecExpr.
  bool isNonNegativeNSWIV(const Instruction *Inst) const;

  /// \brief Returns true if normalized loop IV has NSW semantics.
  bool hasNSWSemantics(const Loop *Lp, const PHINode *IVPhi) const;

  /// \brief Sets the IV type for HLoop.
  void setIVType(HLLoop *HLoop) const;

  /// \brief Moves children of IfParent to loop's preheader/postexit if they are
  /// valid, else returns false.
  static bool populatedPreheaderPostexitNodes(HLLoop *HLoop, HLIf *IfParent,
                                              bool PredicateInversion);

  /// \brief Sets the parent if node of the loop as its ztt.
  void setZtt(HLLoop *HLoop);

  /// \brief Forms loops in HIR.
  void formLoops();

public:
  static char ID; // Pass identification
  HIRLoopFormation();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void verifyAnalysis() const override;

  /// \brief Returns HLLoop corresponding to Lp.
  HLLoop *findHLLoop(const Loop *Lp);

  /// Returns true if this loop requires ztt predicate inversion.
  bool requiresZttInversion(HLLoop *Loop) const {
    return InvertedZttLoops.count(Loop);
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
