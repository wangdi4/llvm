//===--------- HIRLoopFormation.h - Creates HIR loops ---------*-- C++ --*-===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRRegionIdentification.h"

namespace llvm {

class Function;
class Type;
class Loop;
class LoopInfo;
class ScalarEvolution;
class SCEV;
class SCEVAddRecExpr;
class APInt;
class PHINode;

namespace loopopt {

class HLNode;
class HLRegion;
class HLLabel;
class HLLoop;
class HLIf;
class HIRCreation;
class HIRCleanup;
class HLNodeUtils;

/// This analysis forms HIR loops within HIR regions created by the
/// HIRCreation pass.
class HIRLoopFormation {
public:
  typedef std::pair<const Loop *, HLLoop *> LoopPairTy;
  typedef std::pair<HLLabel *, HLIf *> LoopLabelAndBottomTestPairTy;

private:
  /// The function we are analyzing.
  Function *Func;

  DominatorTree &DT;
  LoopInfo &LI;
  HIRRegionIdentification &RI;
  ScopedScalarEvolution &ScopedSE;
  HIRCreation &HIRCr;
  HIRCleanup &HIRC;
  HLNodeUtils &HNU;

  // Region we are processing.
  HLRegion *CurRegion;

  /// Loops - Sorted vector of Loops to HLLoops.
  SmallVector<LoopPairTy, 32> Loops;

  /// Contains loops which require inversion of ztt predicate.
  SmallPtrSet<HLLoop *, 16> InvertedZttLoops;

  /// Ztt candidates deferred until parsing.
  SmallVector<std::pair<HLLoop *, HLIf *>, 16> DeferredZtts;

  /// Maps HLLoops to their label and bottom test.
  /// This is used as a backup to convert countable loops to unknown if parsing
  /// fails.
  DenseMap<HLLoop *, LoopLabelAndBottomTestPairTy> LoopLabelAndBottomTestMap;

  /// Inserts (Lp, HLoop) pair in the map.
  void insertHLLoop(const Loop *Lp, HLLoop *HLoop);

  /// Implements find()/insert() functionality.
  HLLoop *findOrInsertHLLoopImpl(const Loop *Lp, HLLoop *HLoop, bool Insert);

  /// Returns the signed max of \p AddRec by analyzing its stride.
  APInt getAddRecRefinedSignedMax(const SCEVAddRecExpr *AddRec) const;

  /// Returns true if Inst represents a non-negative NSW SCEVAddRecExpr.
  bool isNonNegativeNSWIV(const Loop *Loop, const PHINode *IVPhi) const;

  /// Returns true if normalized loop IV has NSW semantics.
  bool hasNSWSemantics(const Loop *Lp, Type *IVType, const SCEV *BECount) const;

  /// Returns IV definition PHINode of the loop.
  const PHINode *findIVDefInHeader(const Loop &Lp,
                                   const Instruction *Inst) const;

  /// Sets the IV type for HLoop.
  void setIVType(HLLoop *HLoop, const SCEV *BECount) const;

  /// Moves children of IfParent to loop's preheader/postexit if they are
  /// valid, else returns false.
  static bool populatedPreheaderPostexitNodes(HLLoop *HLoop, HLIf *IfParent,
                                              bool PredicateInversion);

  /// Sets the parent if node of the loop as its ztt.
  void setZtt(HLLoop *HLoop);

  /// Moves the loop exit goto after the loop if it is not removed as redundant
  /// by HIRCleanup pass.
  void processLoopExitGoto(HLIf *BottomTest, HLLabel *LoopLabel,
                           HLLoop *HLoop) const;

  /// Forms loops in HIR.
  void formLoops();

public:
  HIRLoopFormation(DominatorTree &DT, LoopInfo &LI, HIRRegionIdentification &RI,
                   HIRCreation &HIRCr, HIRCleanup &HIRC, HLNodeUtils &HNU)
      : DT(DT), LI(LI), RI(RI), ScopedSE(RI.getScopedSE()), HIRCr(HIRCr),
        HIRC(HIRC), HNU(HNU) {}

  void run();

  /// Returns HLLoop corresponding to Lp.
  HLLoop *findHLLoop(const Loop *Lp);

  /// Returns true if this loop requires ztt predicate inversion.
  bool requiresZttInversion(HLLoop *Loop) const {
    return InvertedZttLoops.count(Loop);
  }

  /// Ztt candidates deferred to parser due to children present on both sides of
  /// the If.
  const SmallVectorImpl<std::pair<HLLoop *, HLIf *>> &getDeferredZtts() const {
    return DeferredZtts;
  }

  /// Extracts \p IfParent which has been recognized as the legal Ztt of \p
  /// HLoop from the IR and sets it as the Ztt.
  static bool setRecognizedZtt(HLLoop *HLoop, HLIf *IfParent,
                               bool PredicateInversion);

  /// Reattaches loop label and bottom test back to this loop.
  void reattachLoopLabelAndBottomTest(HLLoop *Loop);

  /// Erase all the stored loop labels and bottom tests.
  void eraseStoredLoopLabelsAndBottomTests();
};

} // End namespace loopopt

} // End namespace llvm

#endif
