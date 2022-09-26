//===- HIRRuntimeDDImpl.h - Implements MV for Runtime DD ---------*-- C++ --*-//
//
// Copyright (C) 2016-2020 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"

#include "llvm/Analysis/TargetTransformInfo.h"

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
  NON_PROFITABLE_ALIAS,
  STRUCT_ACCESS,
  DIFF_ADDR_SPACE,
  UNSIZED,
  SIMD_LOOP,
  UNKNOWN_MIN_MAX,
  UNKNOWN_ADDR_RANGE
};

enum RTDDMethod {
  Compare,
  LibraryCall,
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

  void replaceIVWithBounds(const HLLoop *Loop, const HLLoop *InnerLoop,
                           RegDDRef *UnknownLoopUBRef);

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
  HLLoop *Loop = nullptr;
  HLLoop *InnermostLoop = nullptr;
  RefGroupVecTy Groups;
  SmallVector<Segment, ExpectedNumberOfTests> SegmentList;
  SmallVector<PredicateTuple, 8> PreConditions;
  SmallVector<unsigned, 8> DelinearizedGroupIndices;
  DenseMap<unsigned, unsigned> SplitedGroupsOriginalIndices;
  RTDDMethod Method = RTDDMethod::Compare;

  // For convertible unknown loops, we need to create upper bound-related
  // instructions just after the analysis, so we can use the upper-bound temp
  // in IVSegments (which is built before the transformation). Those
  // instructions need to be saved somewhere (e.g., here in LoopContext) for
  // later use in the transformation.
  HLInst *UnknownLoopUBLoad = nullptr;
  HLInst *UnknownLoopUBMax = nullptr;

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
  TargetLibraryInfo &TLI;
  TargetTransformInfo &TTI;
  HIRSafeReductionAnalysis &SRA;

  bool EnableLibraryCallMethod = true;

public:
  static char ID;

  HIRRuntimeDD(HIRFramework &HIRF, HIRDDAnalysis &DDA, HIRLoopStatistics &HLS,
               TargetLibraryInfo &TLI, TargetTransformInfo &TTI,
               HIRSafeReductionAnalysis &SRA)
      : HIRF(HIRF), DDA(DDA), HLS(HLS), TLI(TLI), TTI(TTI), SRA(SRA) {}

  bool run();

  typedef DDRefGatherer<RegDDRef, MemRefs | FakeRefs> MemRefGatherer;

private:
#ifndef NDEBUG
  static const char *getResultString(RuntimeDDResult Result);
#endif

  struct MemoryAliasAnalyzer;

  // Returns true if \p Loop is considered as profitable for multiversioning.
  bool isProfitable(const HLLoop *Loop);

  // Returns false if multiversioning the loop will not enable vectorization.
  bool canHelpVectorization(const HLLoop *InnermostLoop) const;

  RuntimeDDResult processDDGToGroupPairs(
      const HLLoop *Loop, MemRefGatherer::VectorTy &Refs,
      DenseMap<const RegDDRef *, unsigned> &RefGroupIndex,
      SmallSetVector<std::pair<unsigned, unsigned>, ExpectedNumberOfTests>
          &Tests) const;

  // The method processes each IV segment and updates bounds according to
  // a specified loopnest.
  // It also fills the applicability vector for the further use.
  void processLoopnest(const HLLoop *OuterLoop, const HLLoop *InnerLoop,
                       SmallVectorImpl<IVSegment> &IVSegments,
                       RegDDRef *UnknownLoopUBRef);

  // Check if a given loop is an UNKNOWN loop convertible to a DO loop.
  bool isConvertibleUnknownLoop(const HLLoop *Loop);

  // Create load and max/ext instructions for the unknown-loop upper-bound.
  void createUnknownLoopUBInsts(LoopContext &Context);

  // Returns required DD tests for an arbitrary loop L.
  RuntimeDDResult computeTests(HLLoop *Loop, LoopContext &Context);

  // Creates UGE compare of \p Ref1 and \p Ref2, handles type mismatch.
  static HLInst *createUGECompare(HLNodeUtils &HNU, HLContainerTy &Nodes,
                                  RegDDRef *Lower, RegDDRef *Upper);

  static HLInst *createIntersectionCondition(HLNodeUtils &HNU,
                                             HLContainerTy &Nodes, Segment &S1,
                                             Segment &S2);

  // Generate runtime DD tests using inline compare method.
  static HLIf *createCompareCondition(LoopContext &Context, HLIf *MasterIf,
                                      HLContainerTy &Nodes);

  // Generate runtime DD tests using library call method.
  static HLIf *
  createLibraryCallCondition(LoopContext &Context, HLIf *MasterIf,
                             HLContainerTy &Nodes,
                             SmallVectorImpl<unsigned> &NewSymbases);

  // Create IF statement which will be selecting a loop version.
  static HLIf *
  createMasterCondition(LoopContext &Context, HLContainerTy &Nodes,
                        SmallVectorImpl<unsigned> &NewLiveinSymbases);

  // \brief Modifies HIR implementing specified tests.
  static void generateHLNodes(LoopContext &Context,
                              const TargetLibraryInfo &TLI);

  // \brief Marks all DDRefs independent across groups.
  static void markDDRefsIndep(LoopContext &Context);
};
} // namespace runtimedd
} // namespace loopopt
} // namespace llvm

#endif /* LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRRUNTIMEDDIMPL_H */
