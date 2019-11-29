//===- HIRRuntimeDDImpl.h - Implements MV for Runtime DD ---------*-- C++ --*-//
//
// Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
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
#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRRUNTIMEDDIMPL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRRUNTIMEDDIMPL_H

#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"

namespace llvm {
namespace loopopt {
namespace runtimedd {

typedef DDRefGrouping::RefGroupTy<RegDDRef *> RefGroupTy;
typedef DDRefGrouping::RefGroupVecTy<RegDDRef *> RefGroupVecTy;

const unsigned ExpectedNumberOfTests = 16;
const unsigned SmallTripCountTest = 10;

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
  NON_NORMALIZED_BLOB_IV_COEFF,
  DELINEARIZATION_FAILED,
  NON_DO_LOOP,
  UNROLL_PRAGMA_LOOP,
  IVDEP_PRAGMA_LOOP,
  NON_PROFITABLE,
  NON_PROFITABLE_SUBS,
  STRUCT_ACCESS,
  DIFF_ADDR_SPACE,
  UNSIZED,
  SIMD_LOOP,
  UNKNOWN_MIN_MAX,
};

enum RTDDMethod {
  Compare,
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
  std::unique_ptr<RegDDRef> Lower;
  std::unique_ptr<RegDDRef> Upper;
  const CanonExpr *BaseCE;

  bool IsWrite;
public:
  IVSegment(const RefGroupTy &Group, bool IsWrite);
  IVSegment(const IVSegment &) = delete;
  IVSegment(IVSegment &&Segment);

  RuntimeDDResult isSegmentSupported(const HLLoop *Loop,
                                     const HLLoop *InnermostLoop) const;

  void replaceIVWithBounds(const HLLoop *Loop, const HLLoop *InnerLoop);

  void makeConsistent(ArrayRef<const RegDDRef *> AuxRefs, unsigned Level);

  Segment genSegment() const;

  bool isWrite() const {
    assert(!isEmpty());
    return IsWrite;
  }

  bool isEmpty() const { return BaseCE == nullptr; }

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

struct LoopContext {
  HLLoop *Loop;
  RefGroupVecTy Groups;
  SmallVector<Segment, ExpectedNumberOfTests> SegmentList;
  SmallVector<PredicateTuple, 8> PreConditions;
  RTDDMethod Method = RTDDMethod::Compare;

#ifndef NDEBUG
  LLVM_DUMP_METHOD void dump() {
    dbgs() << "Loop " << Loop->getNumber() << ":\n";
    for (auto &Segment : SegmentList) {
      Segment.dump();
    }
  }
#endif
};

class HIRRuntimeDD {
  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;
  HIRLoopStatistics &HLS;

  typedef DDRefGatherer<RegDDRef, MemRefs | FakeRefs> MemRefGatherer;

public:
  static char ID;

  HIRRuntimeDD(HIRFramework &HIRF, HIRDDAnalysis &DDA, HIRLoopStatistics &HLS)
      : HIRF(HIRF), DDA(DDA), HLS(HLS) {}

  bool run();

private:
#ifndef NDEBUG
  static const char *getResultString(RuntimeDDResult Result);
#endif

  struct MemoryAliasAnalyzer;

  // Returns true if \p Loop is considered as profitable for multiversioning.
  bool isProfitable(const HLLoop *Loop);

  RuntimeDDResult processDDGToGroupPairs(
      const HLLoop *Loop, MemRefGatherer::VectorTy &Refs,
      DenseMap<RegDDRef *, unsigned> &RefGroupIndex,
      SmallSetVector<std::pair<unsigned, unsigned>, ExpectedNumberOfTests>
          &Tests) const;

  // The method processes each IV segment and updates bounds according to
  // a specified loopnest.
  // It also fills the applicability vector for the further use.
  void processLoopnest(const HLLoop *OuterLoop, const HLLoop *InnerLoop,
                       SmallVectorImpl<IVSegment> &IVSegments);

  // Returns required DD tests for an arbitrary loop L.
  RuntimeDDResult computeTests(HLLoop *Loop, LoopContext &Context);

  // Creates UGE compare of \p Ref1 and \p Ref2, handles type mismatch.
  static HLInst *createUGECompare(HLNodeUtils &HNU, HLContainerTy &Nodes,
                                  RegDDRef *Ref1, RegDDRef *Ref2);

  static HLInst *createIntersectionCondition(HLNodeUtils &HNU,
                                             HLContainerTy &Nodes, Segment &S1,
                                             Segment &S2);

  // Generate runtime DD tests using inline compare method.
  static HLIf *createCompareCondition(LoopContext &Context, HLIf *MasterIf,
                                      HLContainerTy &Nodes);

  // Create IF statement which will be selecting a loop version.
  static HLIf *createMasterCondition(LoopContext &Context,
                                     HLContainerTy &Nodes);

  // \brief Modifies HIR implementing specified tests.
  static void generateHLNodes(LoopContext &Context);

  // \brief Marks all DDRefs independent across groups.
  static void markDDRefsIndep(LoopContext &Context);
};
} // namespace runtimedd
} // namespace loopopt
} // namespace llvm

#endif /* LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRRUNTIMEDDIMPL_H */
