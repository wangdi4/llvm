//===--- HIRLoopReversal.h - Declaration of HIRLoopReversal Pass --------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Note:
// This file contains HIRLoopReversal pass's class declaration.
// This allows a complete HIRLoopReversal class declaration available to the
// HIRLoopTransformUtils.cpp file,so that it can instantiate a complete
// HIRLoopReversal object to use inside the independent HIR Loop Transform
// Utility.
//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_REVERSAL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_REVERSAL_H

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRSafeReductionAnalysis.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

namespace llvm {
namespace loopopt {
namespace reversal {

///\brief MarkedCanonExpr is a CanonExpr* with some additional properties
struct MarkedCanonExpr {
  // Data:
  CanonExpr *CE;
  bool IsWrite;
  bool IsMemRef;
  uint64_t Stride;
  RegDDRef *DDRef;

  // explicit constructor
  explicit MarkedCanonExpr(CanonExpr *InitCE, bool InitIsWrite,
                           bool InitIsMemRef, uint64_t initStride,
                           RegDDRef *TheDDRef)
      : CE(InitCE), IsWrite(InitIsWrite), IsMemRef(InitIsMemRef),
        Stride(initStride), DDRef(TheDDRef) {}

  /// \brief Do not allow default constructor
  MarkedCanonExpr() = delete;

  // Getters+setters
  CanonExpr *getCE(void) const { return CE; }
  bool isWrite(void) const { return IsWrite; }
  bool isMemRef(void) const { return IsMemRef; }
  uint64_t getStride(void) const { return Stride; }
  RegDDRef *getDDRef(void) const { return DDRef; }
  void setDDRef(RegDDRef *TheDDRef) { DDRef = TheDDRef; }

#ifndef NDEBUG
  LLVM_DUMP_METHOD void dump(void) {
    formatted_raw_ostream FOS(dbgs());
    // CE:
    CE->dump();
    FOS << "  ";

    // Write flag:
    IsWrite ? (FOS << " W ") : (FOS << " R ");

    // IsMemRef flag:
    IsMemRef ? (FOS << " MemRef ") : (FOS << " NOT MemRef ");

    // Stride:
    FOS << " Stride: " << Stride << "  ";

    // DDRef:
    FOS << "DDRef: ";
    DDRef->dump();
    FOS << "  ";

    // End: newline return
    FOS << "\n"; // new line return
  }
#endif
};

// HIRLoopReversal Pass Declaration
class HIRLoopReversal : public HIRTransformPass {
private:
  HIRDDAnalysis *DDA; // Data-Dependence Analysis Result
  HIRSafeReductionAnalysis *SRA;
  HIRLoopStatistics *HLS;
  SmallVector<MarkedCanonExpr, 8> CEAV; // Vector of MarkedCanonExpr
  struct CollectDDInfo;                 // CollectDDInfo Forward Declaration
  unsigned LoopLevel = 0;               // Current Loop's Level

public:
  static char ID;

  /// \brief HIRLoopReversal's default constructor
  HIRLoopReversal(void);

  /// \brief Entry to the HIRLoopReversal pass
  bool runOnFunction(Function &F) override;

  /// \brief Free pass-specific memory
  void releaseMemory(void) override;

  /// \brief Free per-iteration working-set memory
  void clearWorkingSetMemory(void);

  /// \brief Do Preliminary Checks on the given loop
  /// (and bail out quickly if the loop doesn't pass the filter tests.)
  bool doLoopPreliminaryChecks(const HLLoop *Lp);

  /// \brief Do Collection on the given Loop
  /// (Collect all inner-most loop from the given Function)
  bool doLoopCollection(HLLoop *Lp);

  /// \brief Applicability check:
  // (is there any applicable case suitable for HIR Loop Reversal?)
  bool isApplicable(HLLoop *Lp);

  /// \brief Profitability check for HIR Loop Reversal
  //(is the given loop Profitable?)
  bool isProfitable(const HLLoop *Lp);

  /// \brief Legality check for HIR Loop Reversal
  bool isLegal(const HLLoop *Lp);

  /// \brief conduct ALL HIR-Loop-Reversal related Tests to decide whether the
  /// given loop is suitable for reversal
  bool isReversible(HLLoop *Lp);

  /// \brief HIR Loop Reversal Transformation
  bool doHIRReversalTransform(HLLoop *Lp);

  /// \brief Add all needed passes and mark changes
  void getAnalysisUsage(AnalysisUsage &AU) const;

  /// \brief handle command-line arguments
  bool handleCmdlineArgs(Function &F);

  // *** Interface to external utility  ***

  /// \brief run the pass on a HLLoop *
  bool runOnLoop(
      HLLoop *Lp,     // INPUT + OUTPUT: a given loop
      bool DoReverse, // INPUT: true to reverse the loop if the loop is suitable
      HIRDDAnalysis &DDA,            // INPUT: Existing HIRDDAnalysis
      HIRSafeReductionAnalysis &SRA, // INPUT: Existing HIRSafeReductionAnalysis
      HIRLoopStatistics &LS,         // INPUT: Existing HIRLoopStatistics
      bool &LoopReversed // OUTPUT: true if the loop is successfully reversed
      );

private:
  /// \brief Legality check for a given DVectorTy with a loop level
  bool isLegal(const DirectionVector &DV, unsigned Level);
};
}
}
}

#endif
