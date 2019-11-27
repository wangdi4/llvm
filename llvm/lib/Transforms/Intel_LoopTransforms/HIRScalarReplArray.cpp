//===--- HIRScalarReplArray.cpp -Loop Scalar Replacement Impl -*- C++ -*---===//
// Implement HIR Loop Scalar Replacement of Array Access Transformation
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===---------------------------------------------------------------------===//
// HIR Loop Scalar Replacement Simple Example1: loads only
//
// [ORIGINAL]                       [AFTER ScalarRepl Array]
//                                  t0 = A[0];
// for (int i=0;i<=100;++i) {       for (int i = 0; i <= 100; ++i) {
//   B[i] = A[i] + A[i+1];            t1   = A[i+1];
//}                                   B[i] = t0 + t1;
//                                    t0   = t1;
//                                  }
//
// --------------------------------------------------------------------------
//
// HIR Loop Scalar Replacement Simple Example2: stores only
//
// [ORIGINAL]                       [AFTER ScalarRepl Array]
// for (int i=0;i<=100;++i) {       for (int i = 0; i <= 100; ++i) {
//  A[i]  = B[i]+1;                   t0   = B[i] + 1;
//  A[i+1]= C[i]-1;                   A[i] = t0;
//}                                   t1   = C[i] - 1;
//                                  }
//                                  A[101] = t1;
//
//
//===---------------------------------------------------------------------===//
//
// This file implements HIR Loop Scalar Replacement of Array Transformation.
//
// [Command-line options]
//-hir-scalarrepl-array: run the HIRScalarReplArray pass
//-disable-hir-scalarrepl-array: disable the HIRScalarReplArray pass
//(default is false)
//
// TODO:
// 1. Improve hasValidIV() to allow IVBlobs that are known to be positive or
// negative at compile time.
//
// It will help to release more groups that are potentially suitable/benefical
// from ScalarRepl transformation.
//
// 2. when there is both load and store on the same index, when store comes
// first, there is no need to generate the matching load in prehdr.
// E.g.
//
// |  . = A[i+3];
// |  A[i] = .  ;
// |
// |  A[i+1] = ..;
// |  .      = A[i+1];
// |
//
// Note:
// the t1 = A[UB+1] is not needed in prehdr in this case.
//
// 3. In the given example (scalarrepl.compute.maxIdxLoad1.ll), the load on
// %scalarepl = (@A)[0][0] is NOT needed.
//
// IR Dump Before HIR Scalar Repl:
//
// BEGIN REGION{};
// +DO i1 = 0, 100, 1 < DO_LOOP > ;
// | (@A)[0][i1 + 1] = i1;
// | (@A)[0][i1] = (@A)[0][i1 + 1];
// +END LOOP;
// END REGION
//
// IR Dump After HIR Scalar Repl:
//
// BEGIN REGION { modified }
//  %scalarepl = (@A)[0][0];
//  + DO i1 = 0, 100, 1   <DO_LOOP>
//  |   %scalarepl1 = i1;
//  |   %scalarepl = %scalarepl1;
//  |   (@A)[0][i1] = %scalarepl;
//  |   %scalarepl = %scalarepl1;
//  + END LOOP
//  (@A)[0][101] = %scalarepl;
// END REGION
//
// In general, a load (in preheader) is not required if store dominates load(s)
// in the curren group.
//
//

// -------------------------------------------------------------------
#include "llvm/Transforms/Intel_LoopTransforms/HIRScalarReplArray.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/DDGraph.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

#include "HIRScalarReplArrayImpl.h"

#define DEBUG_TYPE "hir-scalarrepl-array"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::scalarreplarray;

const std::string ScalarReplTempName = "scalarepl";
const std::string ScalarReplCopyName = "copy";
const std::string ScalarReplLoadName = "load";
const std::string ScalarReplStoreName = "store";

// Disable the HIR Scalar Replacement of Array Transformation:
// (default is false)
static cl::opt<bool> DisableHIRScalarReplArray(
    "disable-hir-scalarrepl-array", cl::init(false), cl::Hidden,
    cl::desc("Disable HIR Scalar Replacement of Array (HSRA) Transformation"));

// Threshold for max allowed dependence distance in a reference group.
static cl::opt<unsigned> HIRScalarReplArrayDepDistThreshold(
    "hir-scalarrepl-array-depdist-threshold", cl::init(8), cl::Hidden,
    cl::desc("Dependence distance threshold for locality groups"));

// Threshold for number of registers which can be used.
static cl::opt<unsigned> HIRScalarReplArrayNumRegThreshold(
    "hir-scalarrepl-array-num-reg-threshold", cl::init(100), cl::Hidden,
    cl::desc("Threshold for number of registers which can be used per loop."));

STATISTIC(HIRScalarReplArrayPerformed,
          "Number of HIR Scalar Replacement of Array (HSRA) Performed");

#ifndef NDEBUG
LLVM_DUMP_METHOD void RefTuple::print(bool NewLine) const {
  formatted_raw_ostream FOS(dbgs());

  FOS << "(";
  if (MemRef) {
    MemRef->print(FOS);
    // MemRef is read or write
    if (MemRef->isLval()) {
      FOS << " (W)";
    } else {
      FOS << " (R)";
    }
    FOS << ", ";
  } else {
    FOS << "null, ";
  }

  FOS << TmpId << ", ";

  if (TmpRef) {
    TmpRef->print(FOS);
    FOS << " ";
  } else {
    FOS << "null ";
  }

  FOS << ") ";

  if (NewLine) {
    FOS << "\n";
  }
}
#endif

typedef DDRefGrouping::RefGroupVecTy<const RegDDRef *> RefGroupVecTy;

static unsigned getMaxDepDist(const RefGroupTy &Group, unsigned LoopLevel) {
  const RegDDRef *FirstRef = Group[0];
  const RegDDRef *LastRef = Group[Group.size() - 1];

  int64_t MaxDist = 0;
  bool Ret = DDRefUtils::getConstIterationDistance(LastRef, FirstRef, LoopLevel,
                                                   &MaxDist);
  assert(Ret && "Expect DepDist exist\n");
  (void)Ret;
  uint64_t AbsMaxDist = std::abs(MaxDist);
  assert((AbsMaxDist <= HIRScalarReplArrayDepDistThreshold) &&
         "Expect MaxDepDist within bound\n");

  return AbsMaxDist;
}

// Returns the Max-index load with MIN TOPO#: may find if #Loads >0
// E.g. .., A[i+3](.), A[i+4](R) .. A[i+4](R) ...
//                     ^max_index load with MinTOPO#
// Returns -1 if not max index load is not applicable.
// Note:
// The logic is contrived because the current MemRefs are sorted such that all
// write(s) appear before all read(s) after the collection from
// HIRLocaltyAnalysis, instead of the default (expected) sort by Topological
// numbers over the MemRefs.
//
// This makes the logic is bit difficult to understand.
//
// Once HIRLoopLocality relaxes this behavior, the code can be simplified
// with easy-to-understand logic.
//
static unsigned getMaxLoadIndex(const RefGroupTy &Group, unsigned LoopLevel,
                                unsigned MaxDepDist) {

  // *MAY* find the MaxLoad if there is 1+ load(s)
  unsigned Size = Group.size();
  unsigned MinTopNum = unsigned(-1); // may shrink
  const RegDDRef *FirstRef = Group[0];
  bool DepDistExist = false, MaxIndexIsRVal = false;
  int64_t DepDist = 0;
  unsigned MaxLoadIdx = 0;

  // Search from highest index (== MaxDepDist) only (in all available MemRefs):
  for (signed I = Size - 1; I >= 0; --I) {
    const RegDDRef *MemRef = Group[I];
    DepDistExist = DDRefUtils::getConstIterationDistance(MemRef, FirstRef,
                                                         LoopLevel, &DepDist);
    assert(DepDistExist && "Expect DepDist exist\n");
    (void)DepDistExist;
    DepDist = std::abs(DepDist);

    // Only check those MemRefs with MaxDepDist
    if (DepDist != MaxDepDist) {
      break;
    }

    // Find the minimal TOPO#: + record its matching Index
    unsigned CurTopNum = MemRef->getHLDDNode()->getTopSortNum();
    if (CurTopNum < MinTopNum) {
      MinTopNum = CurTopNum;
      // Set flag based on whether MemRef is Rval or not
      MaxIndexIsRVal = MemRef->isRval();
      MaxLoadIdx = I;
    }
  }

  return MaxIndexIsRVal ? MaxLoadIdx : -1;
}

/// Returns true if the first iteration value of the MemRef (substituting IV by
/// lower bound) can be deduced to be safe using the dimension info.
static bool isMinIndexWithinBounds(const RegDDRef *MemRef, const HLLoop *Lp) {
  unsigned LoopLevel = Lp->getNestingLevel();

  for (unsigned I = 1, Num = MemRef->getNumDimensions(); I <= Num; ++I) {
    auto *CE = MemRef->getDimensionIndex(I);

    if (!CE->hasIV(LoopLevel)) {
      continue;
    }

    uint64_t NumElements = MemRef->getNumDimensionElements(I);

    // Give up if we do not have dimension info.
    if (NumElements == 0) {
      return false;
    }

    std::unique_ptr<CanonExpr> CEClone(CE->clone());

    if (!CanonExprUtils::replaceIVByCanonExpr(
            CEClone.get(), LoopLevel, Lp->getLowerCanonExpr(), Lp->isNSW())) {
      return false;
    }

    int64_t ConstVal;

    // Can be extended by checking for min/max val.
    if (!CEClone->isIntConstant(&ConstVal)) {
      return false;
    }

    if ((ConstVal >= 0) && ((uint64_t)ConstVal < NumElements)) {
      return true;
    }

    break;
  }

  return false;
}

/// Returns true if the min index load can be safely hoisted to the preheader.
static bool canHoistMinLoadIndex(const RefGroupTy &Group, const HLLoop *Lp,
                                 bool HasForwardGotos) {
  unsigned LoopLevel = Lp->getNestingLevel();

  int64_t DepDist = 0;
  auto *FirstRef = Group[0];

  // Any min index memref which is unconditionally executed within the loop does
  // the job.
  for (auto *MemRef : Group) {
    bool Res = DDRefUtils::getConstIterationDistance(MemRef, FirstRef,
                                                     LoopLevel, &DepDist);
    (void)Res;
    assert(Res && "Const distance expected between refs!");

    if (DepDist != 0) {
      break;
    }

    // This is a cheaper post-domination check.
    // TODO: replace by HLNodeUtils::postDominates()
    if (!HasForwardGotos && isa<HLLoop>(MemRef->getHLDDNode()->getParent())) {
      return true;
    }
  }

  return isMinIndexWithinBounds(FirstRef, Lp);
}

bool MemRefGroup::createRefTuple(const RefGroupTy &Group) {

  bool IsUnknown = Lp->isUnknown();
  bool HasForwardGotos = (Lp->getNumExits() > 1) ||
                         HSRA.HLS.getSelfLoopStatistics(Lp).hasForwardGotos();

  // Perform basic checks on the group.
  for (auto *MemRef : Group) {
    if (MemRef->isVolatile() || MemRef->isFake()) {
      return false;
    }

    if (MemRef->isLval()) {
      // Do not handle stores in unknown loop. This can be extended by creating
      // copy instruction for IV. Do not handle conditional stores. Conditional
      // loads are okay.
      if (IsUnknown || HasForwardGotos ||
          !isa<HLLoop>(MemRef->getHLDDNode()->getParent())) {
        return false;
      }
      ++NumStores;
    } else {
      ++NumLoads;
    }
  }

  if (!canHoistMinLoadIndex(Group, Lp, HasForwardGotos)) {
    return false;
  }

  MaxDepDist = getMaxDepDist(Group, LoopLevel);

  if (NumLoads != 0) {
    MaxLoadIndex = getMaxLoadIndex(Group, LoopLevel, MaxDepDist);
  }

  // TODO: set min store here as well for simplicity.

  // MaxLoad should be unconditionally executed within the loop.
  if (hasMaxLoadIndex() &&
      !HLNodeUtils::dominates(Group[MaxLoadIndex]->getHLDDNode(),
                              Lp->getLastChild())) {
    return false;
  }

  // Create a partially filled RefTuple and save it into RefTupleVec.
  // E.g. (A[i], -1, nullptr)
  for (auto *MemRef : Group) {
    RefTupleVec.emplace_back(const_cast<RegDDRef *>(MemRef));
  }

  return true;
}

MemRefGroup::MemRefGroup(HIRScalarReplArray &HSRA, HLLoop *Lp,
                         const RefGroupTy &Group)
    : HSRA(HSRA), Lp(Lp), NumLoads(0), NumStores(0),
      LoopLevel(Lp->getNestingLevel()), IsValid(true), HasRWGap(false),
      MaxLoadIndex(-1), MinStoreIndex(-1), MaxStoreDist(0) {

  if (!createRefTuple(Group)) {
    IsValid = false;
    return;
  }

  Symbase = Group[0]->getSymbase();
  BaseCE = Group[0]->getBaseCE();
}

const RefTuple *MemRefGroup::getByDist(unsigned Dist) const {
  assert((Dist <= MaxDepDist) && "Dist is out of bound\n");

  for (auto &RT : RefTupleVec) {
    if ((unsigned)RT.getTmpId() == Dist) {
      return &RT;
    }
  }

  llvm_unreachable(
      "MemRefGroup::getByDist(.) - not expect control to reach here!\n");
  return nullptr; // turn off a compiler warning
}

bool MemRefGroup::belongs(RegDDRef *Ref) const {
  // Check: Symbase and BaseCE must match
  if (!((Symbase == Ref->getSymbase()) &&
        CanonExprUtils::areEqual(BaseCE, Ref->getBaseCE()))) {
    return false;
  }

  // Check: MemRef* MUST physically be in the MemRefGroup
  for (auto &RT : RefTupleVec) {
    if (Ref == RT.getMemRef()) {
      return true;
    }
  }

  return false;
}

void MemRefGroup::markMinStore(void) {
  if (NumStores == 0) {
    return;
  }

  // Identify the 1st store and its DepDist (MinDD): must Find!
  unsigned MinDD = 0;
  bool FindMinDD = false;
  unsigned Size = RefTupleVec.size();

  for (unsigned I = 0; I <= Size - 1; ++I) {
    RefTuple *RT = &RefTupleVec[I];

    // Only try 1st store
    RegDDRef *MemRef = RT->getMemRef();
    if (MemRef->isLval()) {
      MinDD = RT->getTmpId();
      FindMinDD = true;
      break;
    }
  }

  // must find if there is 1+ store.
  assert(FindMinDD && "Fail to find MinDD\n");
  (void)FindMinDD;

  unsigned MaxTopNum = 0; // will grow

  for (unsigned I = 0; I <= Size - 1; ++I) { // may search the full vector
    RefTuple *RT = &RefTupleVec[I];

    // Only check Store(s)
    RegDDRef *MemRef = RT->getMemRef();
    if (MemRef->isRval()) {
      continue;
    }

    // Only check any RT whose TmpId == MinDD
    uint64_t TmpId = RT->getTmpId();
    if (TmpId < MinDD) {
      continue;
    } else if (TmpId > MinDD) {
      break;
    }

    // Find the largest TOPO#
    unsigned CurTopNum = MemRef->getHLDDNode()->getTopSortNum();
    if (CurTopNum > MaxTopNum) {
      MinStoreIndex = I;
      MaxTopNum = CurTopNum;
    }
  }

  // must find the MinIndxStoreRT
  assert(hasMinStoreIndex() && "fail to find MinStoreIndex\n");
}

void MemRefGroup::identifyGaps(SmallVectorImpl<bool> &RWGap) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  assert(MaxDepDist != unsigned(-1));
  RWGap.resize(MaxDepDist + 1);

  // Accumulate MemRefs into the RWGap mask
  for (auto &RT : RefTupleVec) {
    unsigned Dist = RT.getTmpId();
    assert((Dist <= MaxDepDist) && "MemRef distance out of range\n");
    RWGap[Dist] = true;
  }

  // Check: is there any gap?
  for (signed I = MaxDepDist; I >= 0; --I) {
    if (!RWGap[I]) {
      HasRWGap = true;
    }
  }

  LLVM_DEBUG(FOS << "\nHasRWGap: " << HasRWGap << "\n";);
}

bool MemRefGroup::isCompleteStoreOnly(void) {
  if (NumLoads) {
    return false;
  }
  return !HasRWGap;
}

bool MemRefGroup::isLegal(void) const {
  // TODO: first check unique group symbases returned by
  // populateTemporalLocalityGroups() to save compile time by avoiding DDG.
  DDGraph DDG = HSRA.HDDA.getGraph(Lp);

  // Check: outgoing edge(s)
  if (!areDDEdgesInSameMRG<false>(DDG)) {
    return false;
  }

  // Check: incoming edge(s)
  if (!areDDEdgesInSameMRG<true>(DDG)) {
    return false;
  }

  return true;
}

template <bool IsIncoming>
bool MemRefGroup::areDDEdgesInSameMRG(DDGraph &DDG) const {
  DDRef *OtherRef = nullptr;

  for (auto &RT : RefTupleVec) {
    RegDDRef *Ref = RT.getMemRef();

    for (const DDEdge *Edge :
         (IsIncoming ? DDG.incoming(Ref) : DDG.outgoing(Ref))) {
      LLVM_DEBUG(Edge->print(dbgs()););

      if (IsIncoming) {
        OtherRef = Edge->getSrc();
      } else {
        OtherRef = Edge->getSink();
      }

      // Check: OtherRef must be in the same MRG
      if (!belongs(dyn_cast<RegDDRef>(OtherRef))) {
        return false;
      }
    }
  }

  return true;
}

bool MemRefGroup::hasReuse(void) const {
  uint64_t TripCount = 0;

  // If the loop's trip count is available, use it
  if (Lp->isConstTripLoop(&TripCount)) {
    return (MaxDepDist < TripCount);
  }

  // Otherwise, check the loop's estimated trip count
  uint64_t MaxTripCountEst = Lp->getMaxTripCountEstimate();
  if (MaxTripCountEst) {
    return (MaxDepDist < MaxTripCountEst);
  }

  return true;
}

void MemRefGroup::markMaxStoreDist(void) {
  // Sanity: need 2 stores at least
  if (NumStores < 2) {
    return;
  }

  // Identify lowest-idx store ref: scan from left to right
  unsigned Size = RefTupleVec.size();
  RegDDRef *LowestStore = nullptr;
  for (unsigned I = 0; I < Size; ++I) {
    RegDDRef *MemRef = RefTupleVec[I].getMemRef();

    // Break once we find the lowest store
    if (MemRef->isLval()) {
      LowestStore = MemRef;
      break;
    }
  }
  assert(LowestStore && "Expect LowestStore available\n");

  // Identify highest-idx store: scan from right to left
  RegDDRef *HighestStore = nullptr;
  for (signed I = RefTupleVec.size() - 1; I >= 0; --I) {
    RegDDRef *MemRef = RefTupleVec[I].getMemRef();

    // Break once we find the highest store
    if (MemRef->isLval()) {
      HighestStore = MemRef;
      break;
    }
  }
  assert(HighestStore && "Expect HighestStore available\n");

  // Save MaxStoreDist: the max distance between 2 boundary stores
  int64_t DepDist = 0;
  bool Res = DDRefUtils::getConstIterationDistance(HighestStore, LowestStore,
                                                   LoopLevel, &DepDist);
  assert(Res && "MaxStoreDist must exist\n");
  (void)Res;
  MaxStoreDist = std::abs(DepDist);
}

bool MemRefGroup::doPostCheckOnRef(const HLLoop *Lp, bool IsLoad) const {
  const CanonExpr *BoundCE =
      IsLoad ? Lp->getLowerCanonExpr() : Lp->getUpperCanonExpr();
  RegDDRef *FirstRef = RefTupleVec[0].getMemRef();

  return DDRefUtils::canReplaceIVByCanonExpr(FirstRef, LoopLevel, BoundCE,
                                             true);
}

bool MemRefGroup::doPostChecks(const HLLoop *Lp) const {

  if (requiresLoadInPrehdr() && !doPostCheckOnRef(Lp, true)) {
    return false;
  }

  if (requiresStoreInPostexit() && !doPostCheckOnRef(Lp, false)) {
    return false;
  }

  return true;
}

void MemRefGroup::handleTemps(void) {
  RegDDRef *FirstRef = RefTupleVec[0].getMemRef();
  Type *DestType = FirstRef->getDestType();
  HLNodeUtils &HNU = HSRA.HNU;

  // Create all TmpRef(s), and push them into TmpV
  for (unsigned Idx = 0; Idx < getNumTemps(); ++Idx) {
    RegDDRef *TmpRef = HNU.createTemp(DestType, ScalarReplTempName);
    TmpV.push_back(TmpRef);
  }

  // Update each MemRef in RefTuple, with proper TmpId and TmpRef
  unsigned Size = getSize();
  for (unsigned Idx = 0; Idx < Size; ++Idx) {
    RefTuple *RT = &RefTupleVec[Idx];
    RegDDRef *MemRef = RT->getMemRef();

    // set TmpId (the Dependence Distance)
    int64_t TmpId = 0;
    DDRefUtils::getConstIterationDistance(MemRef, FirstRef, LoopLevel, &TmpId);
    TmpId = std::abs(TmpId);
    assert((TmpId <= (int64_t)MaxDepDist) && "TmpId is out of bound\n");

    // setup TmpId and TmpRef
    RT->setTmpId(TmpId);
    RT->setTmpRef(TmpV[TmpId]);
  }
}

void MemRefGroup::generateTempRotation(HLLoop *Lp) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  LLVM_DEBUG(FOS << "BEFORE generateTempRotation(.): \n"; Lp->dump();
             FOS << "\n");
  HLNodeUtils &HNU = HSRA.HNU;

  // For unknown loops we will be inserting inside the bottom test's then case.
  HLNode *InsertAfterNode = !Lp->isUnknown() ? Lp->getLastChild() : nullptr;

  for (unsigned Idx = 0, IdxE = TmpV.size() - 1; Idx < IdxE; ++Idx) {
    // Generate a CopyInst: Tn = Tn1
    RegDDRef *LvalRef = TmpV[Idx];
    RegDDRef *RvalRef = TmpV[Idx + 1];
    HLInst *CopyInst = HNU.createCopyInst(RvalRef->clone(), ScalarReplCopyName,
                                          LvalRef->clone());

    if (!InsertAfterNode) {
      HLNodeUtils::insertAsFirstThenChild(cast<HLIf>(Lp->getLastChild()),
                                          CopyInst);
    } else {
      HLNodeUtils::insertAfter(InsertAfterNode, CopyInst);
    }

    InsertAfterNode = CopyInst;
  }

  LLVM_DEBUG(FOS << "AFTER generateTempRotation(.): \n"; Lp->dump();
             FOS << "\n");
}

void MemRefGroup::generateLoadToTmps(HLLoop *Lp, SmallVectorImpl<bool> &RWGap) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  LLVM_DEBUG(FOS << "BEFORE generateLoadToTmps(.): \n"; Lp->dump();
             FOS << "\n");
  LLVM_DEBUG(printRefTupleVec(true););
  LLVM_DEBUG(printTmpVec(true););

  // Iterate over each possible index:
  // - NO gap: generate a load use clone() from its matching MemRef;
  // -    gap: create a new MemRef using MemRef[0]->clone() and shift(), then
  //           generate a load from it;
  RegDDRef *MemRef = nullptr;
  RegDDRef *TmpRef = nullptr;
  CanonExpr *LBCE = Lp->getLowerCanonExpr();
  RegDDRef *BaseRef = RefTupleVec[0].getMemRef();

  // Scan in [0 .. MaxDD): NOT to generate a load for MemRef[MaxDD](R)
  for (unsigned Idx = 0; Idx < MaxDepDist; ++Idx) {
    bool HasMemRef = RWGap[Idx];
    LLVM_DEBUG(FOS << "Idx: " << Idx << " HasMemRef: " << HasMemRef << "\n";);

    if (HasMemRef) {
      const RefTuple *RT = getByDist(Idx);
      LLVM_DEBUG(RT->print(true););
      MemRef = RT->getMemRef();
      TmpRef = RT->getTmpRef();
    } else {
      // create the missing MemRef, and obtain its matching TmpRef
      MemRef = BaseRef->clone();
      MemRef->shift(LoopLevel, Idx);
      TmpRef = TmpV[Idx];
      LLVM_DEBUG(MemRef->dump(); FOS << ", " << Idx << ", "; TmpRef->dump();
                 FOS << "\n";);
    }

    // generate the load in loop's prehdr:
    generateLoadInPrehdr(Lp, MemRef, Idx, TmpRef, !HasMemRef, LBCE);
  }

  LLVM_DEBUG(FOS << "AFTER generateLoadToTmps(.): \n"; Lp->dump(); FOS << "\n");
}

void MemRefGroup::generateLoadInPrehdr(HLLoop *Lp, RegDDRef *MemRef,
                                       unsigned Index, RegDDRef *TmpRef,
                                       bool IndepMemRef, CanonExpr *LBCE) {

  // Create a load from MemRef into Tmp
  RegDDRef *MemRef2 = IndepMemRef ? MemRef : MemRef->clone();
  RegDDRef *TmpRefClone = TmpRef->clone();
  DDRefUtils::replaceIVByCanonExpr(MemRef2, LoopLevel, LBCE, Lp->isNSW());

  // Insert the load into the Lp's preheader
  HLNodeUtils &HNU = HSRA.HNU;
  HLInst *LoadInst = HNU.createLoad(MemRef2, ScalarReplTempName, TmpRefClone);
  HLNodeUtils::insertAsLastPreheaderNode(Lp, LoadInst);

  // Mark TempRefClone as Lp's LiveIn
  Lp->addLiveInTemp(TmpRefClone->getSymbase());

  // Make MemRef2 consistent
  const SmallVector<const RegDDRef *, 1> AuxRefs = {Lp->getLowerDDRef()};
  MemRef2->makeConsistent(AuxRefs, Lp->getNestingLevel() - 1);
}

void MemRefGroup::generateStoreFromTmps(HLLoop *Lp) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  LLVM_DEBUG(FOS << "BEFORE generateStoreFromTmps(.): \n"; Lp->dump();
             FOS << "\n");
  LLVM_DEBUG(printRefTupleVec(true););
  LLVM_DEBUG(printTmpVec(true););

  // Compute the store boundaries: [MinStorePos .. MaxStorePos]
  // E.g. for the following MemRefGroup:
  // ( r   ,  w   , rw  ,  w    )
  //   0      1     2      3
  //          ^            ^
  //          MinStore     MaxStore, MaxStoreDist is 2 (we knew this)
  //
  const RefTuple *StoreRT = getMinStoreRefTuple();
  assert(StoreRT && "Expect storeRT available\n");
  unsigned MinStoreOffset = StoreRT->getTmpId();
  CanonExpr *UBCE = Lp->getUpperCanonExpr();
  RegDDRef *MemRef = nullptr, *TmpRef = nullptr;
  signed AdjustIdx = isCompleteStoreOnly() ? 0 : (-1);
  HLInst *StoreInst = nullptr;

  // For each unique index in [MinStoreOffset+1 ..
  // MinStoreOffset+MaxStoreDist]:
  // - generate a store in Postexit regardless of gap situation;
  // - temp associated with the store:
  //   . use normally mapped temp for a CompleteStoreOnly group;
  //   . use a skewed (-1 mapped) temp otherwise;
  // - stores are generated in natural order;
  //
  RegDDRef *BaseRef = StoreRT->getMemRef();

  for (unsigned Idx = 1; Idx <= MaxStoreDist; ++Idx) {
    MemRef = BaseRef->clone();
    MemRef->shift(LoopLevel, Idx);
    TmpRef = TmpV[Idx + MinStoreOffset + AdjustIdx]->clone();

    LLVM_DEBUG(MemRef->dump(); FOS << ", " << Idx << ", "; TmpRef->dump();
               FOS << "\n";);

    StoreInst = generateStoreInPostexit(Lp, MemRef, TmpRef, UBCE, StoreInst);
  }

  LLVM_DEBUG(FOS << "AFTER generateStoreFromTmps(.): \n"; Lp->dump();
             FOS << "\n");
}

HLInst *MemRefGroup::generateStoreInPostexit(HLLoop *Lp, RegDDRef *MemRef,
                                             RegDDRef *TmpRef, CanonExpr *UBCE,
                                             HLInst *InsertAfter) {

  // Simplify: Replace IV with UBCE
  HLNodeUtils &HNU = HSRA.HNU;
  DDRefUtils::replaceIVByCanonExpr(MemRef, LoopLevel, UBCE, Lp->isNSW());

  // Create a StoreInst
  HLInst *StoreInst = HNU.createStore(TmpRef, ScalarReplStoreName, MemRef);

  if (InsertAfter) {
    HLNodeUtils::insertAfter(InsertAfter, StoreInst);
  } else {
    HLNodeUtils::insertAsFirstPostexitNode(Lp, StoreInst);
  }

  Lp->addLiveOutTemp(TmpRef->getSymbase());

  // Make MemRef consistent: remove any stale blob(s)
  const SmallVector<const RegDDRef *, 1> AuxRefs = {Lp->getUpperDDRef()};
  MemRef->makeConsistent(AuxRefs, Lp->getNestingLevel() - 1);

  return StoreInst;
}

bool MemRefGroup::analyze(HLLoop *Lp) {
  assert(isValid() && "Invalid MemRefGroup encountered during analysis!");

  // do Profit Test:
  if (!isProfitable()) {
    IsValid = false;
    return false;
  }

  // do Legal Test:
  if (!isLegal()) {
    IsValid = false;
    return false;
  }

  // identify Store Bounds and mark MaxStoreDist (doPostChecks() needs it)
  markMaxStoreDist();

  // do PostChecks:
  if (!doPostChecks(Lp)) {
    IsValid = false;
    return false;
  }

  return true;
}

bool MemRefGroup::verify(void) {
  // Check: RefTupleVec is not empty
  if (RefTupleVec.empty()) {
    return false;
  }

  // Check: for each RT in RTV, all fields are populated
  for (auto &RT : RefTupleVec) {

    if (!RT.getMemRef()) {
      return false;
    }

    // int64_t TmpId = RT.getTmpId();
    if (RT.getTmpId() == -1) {
      return false;
    }

    if (!RT.getTmpRef()) {
      return false;
    }
  }

  return true;
}

#ifndef NDEBUG
void MemRefGroup::print(bool NewLine) {
  formatted_raw_ostream FOS(dbgs());
  unsigned NumLoad = 0, NumStore = 0;

  // Print the total # of items in this MRG:
  FOS << "<" << RefTupleVec.size() << "> { ";

  // Print RefTuple
  for (auto &RT : RefTupleVec) {
    RT.print();

    // For each ref, print 'W' for a write, and 'R' for a read
    RegDDRef *Ref = RT.getMemRef();
    if (Ref->isLval()) {
      ++NumStore;
    } else {
      ++NumLoad;
    }

    FOS << ", ";
  }

  FOS << " } ";

  // Print # of Read(s) and Write(s):
  FOS << NumStore << "W : " << NumLoad << "R ";

  // MaxDepDist:
  FOS << ", MaxDepDist: " << MaxDepDist;

  // Symbase:
  FOS << ", Symbase: " << Symbase << ", ";

  // Print MaxLoadIndex: if available
  FOS << "MaxLoadIndex: ";
  if (hasMaxLoadIndex()) {
    getMaxLoadRefTuple()->print();
  } else {
    FOS << " null ";
  }
  FOS << ", ";

  // Print MinStoreIndex: if available
  FOS << "MinStoreIndex: ";
  if (hasMinStoreIndex()) {
    getMinStoreRefTuple()->print();
  } else {
    FOS << " null ";
  }
  FOS << ", ";

  // Print TmpV
  FOS << ", TmpV:<";
  unsigned TmpSize = TmpV.size();
  FOS << TmpSize << "> [ ";
  unsigned Count = 0;
  for (auto &TmpRef : TmpV) {
    if (TmpRef == nullptr) {
      FOS << " null";
    } else {
      TmpRef->print(FOS);
    }
    if (Count != TmpSize - 1) {
      FOS << ", ";
    }
    ++Count;
  }
  FOS << "] ";

  if (NewLine) {
    FOS << "\n";
  }
}

void MemRefGroup::printRefTupleVec(bool NewLine) {
  for (auto &RT : getRefTupleVec()) {
    RT.print(true);
  }

  if (NewLine) {
    formatted_raw_ostream FOS(dbgs());
    FOS << "\n";
  }
}

void MemRefGroup::printTmpVec(bool NewLine) {
  formatted_raw_ostream FOS(dbgs());

  for (auto &TmpRef : getTmpV()) {
    TmpRef->dump();
    FOS << ", ";
  }

  if (NewLine) {
    FOS << "\n";
  }
}
#endif

HIRScalarReplArray::HIRScalarReplArray(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                                       HIRLoopLocality &HLA,
                                       HIRLoopStatistics &HLS)
    : HIRF(HIRF), HDDA(HDDA), HLA(HLA), HLS(HLS), HNU(HIRF.getHLNodeUtils()),
      DDRU(HIRF.getDDRefUtils()), CEU(HIRF.getCanonExprUtils()) {

  // TODO: set platform specific thresholds.
  // Check whether the target is an X86 (32b) or X64 (64b) platform
  // llvm::Triple TargetTriple(HIRF.getModule().getTargetTriple());
  // Is32Bit = (TargetTriple.getArch() == llvm::Triple::x86);
}

bool HIRScalarReplArray::run() {
  if (DisableHIRScalarReplArray) {
    LLVM_DEBUG(
        dbgs() << "HIR Scalar Replacement of Array Transformation Disabled "
                  "or Skipped\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIRScalarReplArray on Function : "
                    << HIRF.getFunction().getName() << "\n";);

  // Gather ALL Innermost Loops as Candidates, use 64 increment
  SmallVector<HLLoop *, 64> CandidateLoops;
  HNU.gatherInnermostLoops(CandidateLoops);

  if (CandidateLoops.empty()) {
    LLVM_DEBUG(
        dbgs() << HIRF.getFunction().getName()
               << "() has no inner-most loop for HIR scalar replacement\n ");
    return false;
  }

  for (auto &Lp : CandidateLoops) {
    clearWorkingSetMemory();

    // Analyze the loop and check if it is suitable for ScalarRepl
    if (!doAnalysis(Lp)) {
      continue;
    }

    doTransform(Lp);
  }

  CandidateLoops.clear();
  return false;
}

bool HIRScalarReplArray::doAnalysis(HLLoop *Lp) {
  if (!doPreliminaryChecks(Lp)) {
    LLVM_DEBUG(dbgs() << "ScalarRepl: Loop Preliminary Checks failed\n";);
    return false;
  }

  if (!doCollection(Lp)) {
    LLVM_DEBUG(dbgs() << "ScalarRepl: collection failed\n");
    return false;
  }

  bool Result = false;

  // Analyze each MemRefGroup in MRGVec
  for (auto &MRG : MRGVec) {
    if (MRG.analyze(Lp)) {
      Result = true;
    }
  }

  return Result;
}

bool HIRScalarReplArray::doPreliminaryChecks(const HLLoop *Lp) {
  // Skip if the loop is vectorized:
  // (If a loop's stride is not 1, assume it is a vector loop.)
  int64_t StrideConst = 0;
  Lp->getStrideDDRef()->isIntConstant(&StrideConst);
  // We can handle unknown loops whose stride is set to 0.
  if (StrideConst > 1) {
    return false;
  }

  const LoopStatistics &LS = HLS.getSelfLoopStatistics(Lp);
  // LLVM_DEBUG(LS.dump(););
  if (LS.hasCallsWithUnknownAliasing()) {
    return false;
  }

  return true;
}

static bool hasValidIV(const RegDDRef *Ref, unsigned LoopLevel,
                       bool &HasNegIVCoeff) {
  bool HasIV = false;

  for (auto I = Ref->canon_begin(), E = Ref->canon_end(); I != E; ++I) {
    const CanonExpr *CE = (*I);

    int64_t Coeff;
    unsigned BlobIndex;
    CE->getIVCoeff(LoopLevel, &BlobIndex, &Coeff);

    if (Coeff == 0) {
      continue;
    }

    HasIV = true;

    // Check any valid IVBlob
    // TODO: allow known positive/negative blobs.
    if (BlobIndex != InvalidBlobIndex) {
      return false;
    }

    // TODO: what happens if it has both positive and negative coeffs in
    // different indices?
    if (Coeff < 0) {
      HasNegIVCoeff = true;
    }
  }

  return HasIV;
}

static bool isValid(RefGroupTy &Group, unsigned LoopLevel) {

  if (Group.size() == 1) {
    return false;
  }

  auto *FirstRef = Group[0];

  if (FirstRef->isNonLinear()) {
    return false;
  }

  bool HasNegIVCoeff = false;
  if (!hasValidIV(FirstRef, LoopLevel, HasNegIVCoeff)) {
    return false;
  }

  // Bail out if group has references like A[i1] and (i32*)A[i1] as we cannot
  // generate temp rotation code for different types. NOTE: we should move
  // FloatToInt pass after LoopOpt to minimize such cases.
  auto *BitCastTy = FirstRef->getBitCastDestType();

  for (auto *Ref : Group) {
    if (BitCastTy != Ref->getBitCastDestType()) {
      return false;
    }
  }

  // Reverse the group if it has any negative IVCoeff
  if (HasNegIVCoeff) {
    std::reverse(Group.begin(), Group.end());
  }

  return true;
}

bool HIRScalarReplArray::doCollection(HLLoop *Lp) {
  // Collect and group RegDDRefs:Don't sort the groups
  RefGroupVecTy Groups;
  HIRLoopLocality::populateTemporalLocalityGroups(
      Lp, HIRScalarReplArrayDepDistThreshold, Groups);
  LLVM_DEBUG(DDRefGrouping::dump(Groups));

  // Examine each individual group, validate it, and save only the good ones.
  bool Result = false;
  unsigned LoopLevel = Lp->getNestingLevel();

  for (RefGroupTy &Group : Groups) {

    if (!isValid(Group, LoopLevel)) {
      continue;
    }

    MemRefGroup MRG(*this, Lp, Group);

    // Constructor performs sanity checks and populates the structure.
    if (!MRG.isValid()) {
      continue;
    }

    MRGVec.push_back(std::move(MRG));

    Result = true;
  }

  LLVM_DEBUG(print());
  return Result;
}

bool HIRScalarReplArray::checkAndUpdateQuota(MemRefGroup &MRG,
                                             unsigned &NumGPRsUsed) const {
  bool Result =
      (NumGPRsUsed + MRG.getNumTemps() <= HIRScalarReplArrayNumRegThreshold);
  if (Result) {
    NumGPRsUsed += MRG.getNumTemps();
  }
  return Result;
}

void HIRScalarReplArray::doTransform(HLLoop *Lp) {
  unsigned NumGPRsPromoted = 0;
  bool Transformed = false;

  // Transform each suitable Group as long as there is still quota available
  for (auto &MRG : MRGVec) {
    if (MRG.isValid() && checkAndUpdateQuota(MRG, NumGPRsPromoted)) {
      doTransform(Lp, MRG);
      Transformed = true;
    }
  }

  LoopOptReportBuilder &LORBuilder =
      Lp->getHLNodeUtils().getHIRFramework().getLORBuilder();

  LORBuilder(*Lp).addRemark(OptReportVerbosity::Low,
                            "Number of Array Refs Scalar Replaced In Loop: %d",
                            NumGPRsPromoted);

  // Mark the loop has been changed, request CodeGen support
  // Note: ScalarReplArray won't change current HIRLoopStatistics
  if (Transformed) {
    assert(Lp->getParentRegion() && " Loop does not have a parent region\n");
    Lp->getParentRegion()->setGenCode();
    HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Lp);
    HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(
        Lp);
  }
}

void HIRScalarReplArray::doTransform(HLLoop *Lp, MemRefGroup &MRG) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  LLVM_DEBUG(FOS << "BEFORE doTransform(.):\n"; Lp->dump(); MRG.print(););

  // Preparations:
  MRG.handleTemps();
  assert(MRG.verify() && "MRG verification failed\n");

  MRG.markMinStore();

  SmallVector<bool, 16> RWGap;
  MRG.identifyGaps(RWGap);

  LLVM_DEBUG(FOS << "AFTER Preparation:\n"; MRG.print(););

  // 3-step scalar-repl transformation:
  doPreLoopProc(Lp, MRG, RWGap);
  doPostLoopProc(Lp, MRG);
  doInLoopProc(Lp, MRG);

  ++HIRScalarReplArrayPerformed;

  LLVM_DEBUG(FOS << "AFTER doTransform(.):\n"; Lp->dump(););
}

// Pre-loop processing:
// Generate Loads (load from MemRef into its matching Tmp) when needed;
// (Gaps are given in RWGap vector)
//
// [TO GEN]
// i. generate a load for any unique available MemRef[ir](R) in MRG
//  (where r in [0..MaxDD))
// ii.generate a load for any unique missing MemRef[ir](R) in MRG
//  (where r in [0..MaxDD))
//
// [NOT TO GEN]
// i. NOT to generate a load if MemRef[MaxDD](R) exists in MRG;
//
void HIRScalarReplArray::doPreLoopProc(HLLoop *Lp, MemRefGroup &MRG,
                                       SmallVectorImpl<bool> &RWGap) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  LLVM_DEBUG(FOS << "BEFORE doPreLoopProc(.): \n"; Lp->dump(); FOS << "\n");

  // Sanity: No need for any Complete StoreOnly MRG
  if (MRG.isCompleteStoreOnly()) {
    return;
  }

  MRG.generateLoadToTmps(Lp, RWGap);

  LLVM_DEBUG(FOS << "AFTER doPreLoopProc(.): \n"; Lp->dump(); FOS << "\n");
}

void HIRScalarReplArray::doPostLoopProc(HLLoop *Lp, MemRefGroup &MRG) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  LLVM_DEBUG(FOS << "BEFORE doPostLoopProc(.): \n"; Lp->dump(); FOS << "\n");

  if (!MRG.requiresStoreInPostexit()) {
    return;
  }

  MRG.generateStoreFromTmps(Lp);

  LLVM_DEBUG(FOS << "AFTER doPostLoopProc(.): \n"; Lp->dump(); FOS << "\n");
}

void HIRScalarReplArray::doInLoopProc(HLLoop *Lp, MemRefGroup &MRG) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  LLVM_DEBUG(FOS << "BEFORE doInLoopProc(.): \n"; Lp->dump(); FOS << "\n";);

  // Generate a load if MaxLoadIndex is available
  if (MRG.hasMaxLoadIndex()) {
    const RefTuple *MaxLoadRefTuple = MRG.getMaxLoadRefTuple();
    RegDDRef *MemRef = MaxLoadRefTuple->getMemRef();
    RegDDRef *MemRefClone = MemRef->clone();
    RegDDRef *TmpRefClone = MaxLoadRefTuple->getTmpRef()->clone();

    HLInst *LoadInst =
        HNU.createLoad(MemRefClone, ScalarReplLoadName, TmpRefClone);
    HLDDNode *DDNode = MemRef->getHLDDNode();
    HLNodeUtils::insertBefore(DDNode, LoadInst);
  }

  // Generate a store if MinStoreIndex is available
  if (MRG.hasMinStoreIndex()) {
    const RefTuple *MinStoreRefTuple = MRG.getMinStoreRefTuple();
    RegDDRef *MemRef = MinStoreRefTuple->getMemRef();
    RegDDRef *MemRefClone = MemRef->clone();
    RegDDRef *TmpRefClone = MinStoreRefTuple->getTmpRef()->clone();

    HLInst *StoreInst =
        HNU.createStore(TmpRefClone, ScalarReplStoreName, MemRefClone);
    HLDDNode *DDNode = MemRef->getHLDDNode();
    HLNodeUtils::insertAfter(DDNode, StoreInst);
  }

  // Replace each MemRef with its matching Temp
  for (auto &RT : MRG.getRefTupleVec()) {
    // LLVM_DEBUG(RT.print(true););
    replaceMemRefWithTmp(RT.getMemRef(), RT.getTmpRef());
  }

  LLVM_DEBUG(FOS << "AFTER handle MemRefs in loop: \n"; Lp->dump();
             FOS << "\n");

  // Generate temp-rotation code
  // Note: Not need for any Complete Store-Only MRG
  if (!MRG.isCompleteStoreOnly()) {
    MRG.generateTempRotation(Lp);
  }

  LLVM_DEBUG(FOS << "AFTER doInLoopProc(.): \n"; Lp->dump(); FOS << "\n";);
}

void HIRScalarReplArray::clearWorkingSetMemory(void) { MRGVec.clear(); }

void HIRScalarReplArray::replaceMemRefWithTmp(RegDDRef *MemRef,
                                              RegDDRef *TmpRef) {
  HLDDNode *ParentNode = MemRef->getHLDDNode();
  RegDDRef *TmpRefClone = TmpRef->clone();

  // Debug: Examine the DDNode's Parent BEFORE replacement
  // LLVM_DEBUG(ParentNode->getParent()->dump(););

  HLInst *HInst = dyn_cast<HLInst>(ParentNode);
  // Handle HLInst* special cases: LoadInst and StoreInst
  if (HInst) {
    const Instruction *LLVMInst = HInst->getLLVMInstruction();
    HLInst *CopyInst = nullptr;
    RegDDRef *OtherRef = nullptr;

    // StoreInst: replace with a LoadInst or CopyInst depending on the rval.
    if (isa<StoreInst>(LLVMInst) && MemRef->isLval()) {
      OtherRef = HInst->removeOperandDDRef(1);
      if (OtherRef->isMemRef()) {
        auto LInst = HNU.createLoad(OtherRef, ScalarReplCopyName, TmpRefClone);
        HLNodeUtils::replace(HInst, LInst);
      } else {
        CopyInst =
            HNU.createCopyInst(OtherRef, ScalarReplCopyName, TmpRefClone);
        HLNodeUtils::replace(HInst, CopyInst);
      }
    }
    // LoadInst: replace with a CopyInst
    else if (isa<LoadInst>(LLVMInst)) {
      OtherRef = HInst->removeOperandDDRef(0);
      CopyInst = HNU.createCopyInst(TmpRefClone, ScalarReplCopyName, OtherRef);
      HLNodeUtils::replace(HInst, CopyInst);
    }
    // Neither a Load nor a Store: do regular replacement
    else {
      HInst->replaceOperandDDRef(MemRef, TmpRefClone);
    }

  }
  // All other cases: do regular replacement
  else {
    ParentNode->replaceOperandDDRef(MemRef, TmpRefClone);
  }

  // Debug: Examine the DDNode's Parent AFTER replacement
  // LLVM_DEBUG(ParentNode->getParent()->dump(););
}

#ifndef NDEBUG
LLVM_DUMP_METHOD void HIRScalarReplArray::print(void) {
  formatted_raw_ostream FOS(dbgs());
  FOS << "HIRScalarReplArray::print(), MRGVec entries: " << MRGVec.size()
      << "\n";

  // Sanity check: any available MRG in collection?
  if (MRGVec.size() == 0) {
    FOS << " __EMPTY__ \n";
    return;
  }

  for (auto &MRG : MRGVec) {
    FOS << "  ";
    MRG.print(true);
  }
}

void HIRScalarReplArray::printRefGroupTy(RefGroupTy &Group, bool PrintNewLine) {
  formatted_raw_ostream FOS(dbgs());

  if (PrintNewLine) {
    FOS << "\n";
  }

  for (auto &Ref : Group) {
    Ref->dump();
    FOS << ", ";
  }

  if (PrintNewLine) {
    FOS << "\n";
  }
}
#endif

PreservedAnalyses
HIRScalarReplArrayPass::run(llvm::Function &F,
                            llvm::FunctionAnalysisManager &AM) {
  HIRScalarReplArray(AM.getResult<HIRFrameworkAnalysis>(F),
                     AM.getResult<HIRDDAnalysisPass>(F),
                     AM.getResult<HIRLoopLocalityAnalysis>(F),
                     AM.getResult<HIRLoopStatisticsAnalysis>(F))
      .run();

  return PreservedAnalyses::all();
}

class HIRScalarReplArrayLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRScalarReplArrayLegacyPass() : HIRTransformPass(ID) {
    initializeHIRScalarReplArrayLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.addRequiredTransitive<HIRLoopLocalityWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRScalarReplArray(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
               getAnalysis<HIRLoopLocalityWrapperPass>().getHLL(),
               getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS())
        .run();
  }
};

char HIRScalarReplArrayLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRScalarReplArrayLegacyPass, "hir-scalarrepl-array",
                      "HIR Scalar Replacement of Array ", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopLocalityWrapperPass)
INITIALIZE_PASS_END(HIRScalarReplArrayLegacyPass, "hir-scalarrepl-array",
                    "HIR Scalar Replacement of Array ", false, false)

FunctionPass *llvm::createHIRScalarReplArrayPass() {
  return new HIRScalarReplArrayLegacyPass();
}
