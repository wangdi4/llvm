//===- HIRRuntimeDD.h - Implements Multiversioning for Runtime DD *-- C++ --*-//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a pass for the runtime data dependency multiversioning.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRRUNTIMEDD_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRRUNTIMEDD_H

#include "llvm/Support/Debug.h"

#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/IR/Intel_LoopIR/RegDDRef.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRDDAnalysis.h"

#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGrouping.h"

namespace llvm {
namespace loopopt {

const unsigned ExpectedNumberOfTests = 8;
const unsigned SmallTripCountTest = 4;

enum RuntimeDDResult {
  OK,
  NO_OPPORTUNITIES,
  NON_PERFECT_LOOPNEST,
  NON_LINEAR_BASE,
  NON_LINEAR_SUBS,
  NON_CONSTANT_IV_STRIDE,
  SMALL_TRIPCOUNT,
  ALREADY_MV,
  TOO_MANY_TESTS,
  UPPER_SUB_TYPE_MISMATCH,
  BLOB_IV_COEFF,
  SAME_BASE,
  NON_DO_LOOP,
};

// The struct represents a segment of memory. It is used to construct checks
// for memory intersection
struct Segment {
  RegDDRef *Lower;
  RegDDRef *Upper;
  const CanonExpr *BaseCE;

  Segment(RegDDRef *Lower, RegDDRef *Upper) : Lower(Lower), Upper(Upper) {
    BaseCE = Lower->getBaseCE();
  }

#ifndef NDEBUG
  LLVM_DUMP_METHOD void dump() {
    dbgs() << "[";
    Lower->dump();
    dbgs() << ", ";
    Upper->dump();
    dbgs() << "]\n";
  }
#endif

  Type *getType() const { return Lower->getDestType(); }
};

// The class represents a floating constant length memory segment that depends
// on the IV. This class is used to generate specific memory segments where
// IV is replaced with a Lower and Upper bound.
class IVSegment {
  RegDDRef *Lower;
  RegDDRef *Upper;
  const CanonExpr *BaseCE;

  bool IsWrite;

  static void updateRefIVWithBounds(RegDDRef *Ref, unsigned Level,
                                    const RegDDRef *MaxRef,
                                    const RegDDRef *MinRef,
                                    const HLLoop *InnerLoop);

public:
  IVSegment(const DDRefGrouping::RefGroupTy &Group);
  IVSegment(const IVSegment &) = delete;
  IVSegment(IVSegment &&Segment);

  ~IVSegment();

  RuntimeDDResult isSegmentSupported(const HLLoop *Loop,
                                     const HLLoop *InnermostLoop) const;

  void updateIVWithBounds(unsigned Level, const RegDDRef *LowerBound,
                          const RegDDRef *UpperBound, const HLLoop *InnerLoop);

  Segment genSegment() const;

  bool isWrite() const { return IsWrite; }
  RegDDRef *getLower() { return Lower; }
  RegDDRef *getUpper() { return Upper; }
  const RegDDRef *getLower() const { return Lower; }
  const RegDDRef *getUpper() const { return Upper; }
  const CanonExpr *getBaseCE() const { return BaseCE; }

#ifndef NDEBUG
  LLVM_DUMP_METHOD void dump() {
    dbgs() << "[";
    Lower->dump();
    dbgs() << ", ";
    Upper->dump();
    dbgs() << "]\n";
  }
#endif
};

struct LoopCandidate {
  HLLoop *Loop;
  llvm::SmallVector<Segment, ExpectedNumberOfTests> SegmentList;
  bool GenTripCountTest;

#ifndef NDEBUG
  LLVM_DUMP_METHOD void dump() {
    dbgs() << "Loop " << Loop->getNumber() << ":\n";
    for (auto &Segment : SegmentList) {
      Segment.dump();
    }
  }
#endif
};

class HIRRuntimeDD : public HIRTransformPass {
public:
  static char ID;

  HIRRuntimeDD() : HIRTransformPass(ID) {
    initializeHIRRuntimeDDPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequiredTransitive<HIRFramework>();
    AU.addRequiredTransitive<HIRDDAnalysis>();
    AU.setPreservesAll();
  }

private:
#ifndef NDEBUG
  static const char *getResultString(RuntimeDDResult Result);
#endif

  struct LoopAnalyzer;

  // \brief The method processes each IV segment and updates bounds according to
  // a specified loopnest.
  // It also fills the applicability vector for the further use.
  static RuntimeDDResult
  processLoopnest(const HLLoop *OuterLoop, const HLLoop *InnerLoop,
                  SmallVectorImpl<IVSegment> &IVSegments,
                  SmallVectorImpl<RuntimeDDResult> &SegmentConditions,
                  bool &ShouldGenerateTripCount);

  // \brief The predicate used in ref grouping. Returns true if two references
  // belong to the same group.
  static bool isGroupMemRefMatchForRTDD(const RegDDRef *Ref1,
                                        const RegDDRef *Ref2);

  // \brief Returns required DD tests for an arbitrary loop L.
  static RuntimeDDResult computeTests(HLLoop *Loop, LoopCandidate &Candidate);

  HLIf *createIfStmtForIntersection(HLContainerTy &Nodes, Segment &S1,
                                    Segment &S2) const;

  // \brief Modifies HIR implementing specified tests.
  void generateDDTest(LoopCandidate &Candidate) const;
};

}
}

#endif /* LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRRUNTIMEDD_H */
