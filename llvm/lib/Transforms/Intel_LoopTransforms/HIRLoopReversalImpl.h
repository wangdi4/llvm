//===--- HIRLoopReversal.h - Declaration of HIRLoopReversal Pass -------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===--------------------------------------------------------------------===//
//
// Note:
// This file contains HIRLoopReversal pass's class declaration.
// This allows a complete HIRLoopReversal class declaration available to the
// HIRLoopTransformUtils.cpp file,so that it can instantiate a complete
// HIRLoopReversal object to use inside the independent HIR Loop Transform
// Utility.
//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_REVERSALIMPL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_REVERSALIMPL_H

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

namespace llvm {

namespace loopopt {

struct DirectionVector;
class HIRFramework;
class HIRDDAnalysis;
class HIRLoopStatistics;
class HIRSafeReductionAnalysis;

namespace reversal {

///\brief MarkedCanonExpr is a CanonExpr* with some additional properties
struct MarkedCanonExpr {
  // Data:
  CanonExpr *CE;
  uint64_t Stride;
  RegDDRef *DDRef;

  // For IVExpr: C * Blob * IV where Blob's sign is known at compile time,
  // we compute CalculatedWeight = C * MaxMin_of_Blob.
  //
  // For IVExpr: C * IV where there is no IVBlob,
  // we compute CalculatedWeight = C.
  int64_t CalculatedWeight;

  explicit MarkedCanonExpr(CanonExpr *InitCE, uint64_t InitStride,
                           RegDDRef *TheDDRef, int64_t InitWeight)
      : CE(InitCE), Stride(InitStride), DDRef(TheDDRef),
        CalculatedWeight(InitWeight) {}

  /// \brief Do not allow default constructor
  MarkedCanonExpr() = delete;

  // Getters+setters
  CanonExpr *getCE(void) const { return CE; }
  bool isWrite(void) const { return DDRef->isLval(); }
  bool isMemRef(void) const { return DDRef->isMemRef(); }
  uint64_t getStride(void) const { return Stride; }
  RegDDRef *getDDRef(void) const { return DDRef; }

  int64_t getCalculatedWeight(void) const { return CalculatedWeight; }

#ifndef NDEBUG
  LLVM_DUMP_METHOD void dump(void) {
    formatted_raw_ostream FOS(dbgs());
    // CE:
    CE->dump();
    FOS << "  ";

    // Write flag:
    isWrite() ? (FOS << " W ") : (FOS << " R ");

    // IsMemRef flag:
    isMemRef() ? (FOS << " MemRef ") : (FOS << " NOT MemRef ");

    // Stride:
    FOS << " Stride: " << Stride << "  ";

    // DDRef:
    FOS << "DDRef: ";
    DDRef->dump();
    FOS << "  ";

    // CalculatedWeight:
    if (CalculatedWeight) {
      FOS << " CalculatedWeight: " << CalculatedWeight << "  ";
    }

    // End: newline return
    FOS << "\n"; // new line return
  }
#endif
};

// HIRLoopReversal Pass Declaration
class HIRLoopReversal {
  HIRFramework &HIRF;
  HIRDDAnalysis &HDDA;
  HIRLoopStatistics &HLS;
  HIRSafeReductionAnalysis &HSRA;

  SmallVector<MarkedCanonExpr, 8> MCEAV; // Vector of MarkedCanonExpr
  struct MarkedCECollector;              // MarkedCE Collector
  struct AnalyzeDDInfo;                  // AnalyzeDDInfo Forward Declaration
  unsigned LoopLevel = 0;                // Current Loop's Level

public:
  HIRLoopReversal(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                  HIRLoopStatistics &HLS, HIRSafeReductionAnalysis &HSRA)
      : HIRF(HIRF), HDDA(HDDA), HLS(HLS), HSRA(HSRA) {}

  /// Entry to the HIRLoopReversal pass
  bool run();

  /// Free per-iteration working-set memory
  void clearWorkingSetMemory(void);

  /// Do Preliminary Checks on the given loop
  /// (and bail out quickly if the loop doesn't pass the filter tests.)
  bool doLoopPreliminaryChecks(const HLLoop *Lp, bool CheckProfitability,
                               bool SkipLoopBoundChecks);

  /// Do a Collection on the given HLLoop
  /// (and bail out if the loop doesn't have any suitable case to reverse)
  bool doCollection(HLLoop *Lp, bool CheckProfitability);

  /// Profitability check for HIR Loop Reversal
  //(is the given loop Profitable?)
  bool isProfitable(const HLLoop *Lp);

  /// Legality check for HIR Loop Reversal
  //(is the given loop Legal to reverse?)
  bool isLegal(const HLLoop *Lp);

  /// conduct HIR-Loop-Reversal related Tests to decide whether the given
  /// loop is suitable for reversal.
  //
  // Add control flags to allow the same function to be called from inside a
  // pass and from external utility APIs.
  bool isReversible(HLLoop *Lp, bool DoProfitTest, bool DoLegalTest,
                    bool SkipLoopBoundChecks);

  /// \brief HIR Loop Reversal Transformation
  bool doHIRReversalTransform(HLLoop *Lp);

private:
  /// \brief Legality check for a given DVectorTy with a loop level
  static bool isLegal(const DirectionVector &DV, unsigned Level);
};
} // namespace reversal
} // namespace loopopt
} // namespace llvm

#endif
