//===--- HIRScalarReplArray.cpp -Loop Scalar Replacement Impl -*- C++ -*---===//
// Implement HIR Loop Scalar Replacement of Array Access Transformation
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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
//   B[i] = A[i] + A[i1];             t1   = A[i+1];
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
// 1. Improve checkIV() to allow IVBlobs that are known to be positive or
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
//
// -------------------------------------------------------------------
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/DDGraph.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLoopStatistics.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGatherer.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#include "HIRScalarReplArray.h"

#define DEBUG_TYPE "hir-scalarrepl-array"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::scalarreplarray;

const std::string ScalarReplTempName = "scalarepl";
const std::string ScalarReplCopyName = "copy";
const std::string ScalarReplLoadName = "load";
const std::string ScalarReplStoreName = "store";
const unsigned ScalarReplDepDistThresholdDefX64 = 5;
const unsigned ScalarReplDepDistThresholdDefX86 = 2;
const unsigned ScalarReplMaxNumReg = 8;

// Disable the HIR Scalar Replacement of Array Transformation:
// (default is false)
static cl::opt<bool> DisableHIRScalarReplArray(
    "disable-hir-scalarrepl-array", cl::init(false), cl::Hidden,
    cl::desc("Disable HIR Scalar Replacement of Array (HSRA) Transformation"));

// Threshold for Max allowed Dependence Distance on x86
static cl::opt<unsigned> HIRScalarReplArrayDepDistThresholdX86(
    "hir-scalarrepl-array-depdist-threshold-x86",
    cl::init(ScalarReplDepDistThresholdDefX86), cl::Hidden,
    cl::desc(
        "Dependence Distance Threshold for HIR Scalar Replacement of Array "
        "Transformation on X86 (32b) platform"));

// Threshold for Max allowed Dependence Distance on x64
static cl::opt<unsigned> HIRScalarReplArrayDepDistThresholdX64(
    "hir-scalarrepl-array-depdist-threshold-x64",
    cl::init(ScalarReplDepDistThresholdDefX64), cl::Hidden,
    cl::desc(
        "Dependence Distance Threshold for HIR Scalar Replacement of Array "
        "Transformation on X64 (64b) platform"));

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

typedef DDRefGrouping::RefGroupVecTy<const RegDDRef> RefGroupVecTy;

MemRefGroup::MemRefGroup(RefGroupTy &Group, HIRScalarReplArray *HSRA)
    : HasRWGap(false), HSRA(HSRA), MaxDepDist(-1), NumLoads(0), NumStores(0),
      IsLegal(false), IsProfitable(false), IsPostChecksOk(false),
      IsSuitable(false), MaxIdxLoadRT(-1), MinIdxStoreRT(-1), MaxStoreDist(0) {
  const RegDDRef *FirstRef = Group[0];
  Symbase = FirstRef->getSymbase();
  BaseCE = FirstRef->getBaseCE();

  for (auto &Ref : Group) {
    // Count: # of load(s)/store(s)
    if (Ref->isLval()) {
      ++NumStores;
    } else {
      ++NumLoads;
    }

    // Create a partially filled RefTuple and save it into RefTupleVec.
    // E.g. (A[i], -1, nullptr)
    insert(const_cast<RegDDRef *>(Ref));
  }

  Lp = FirstRef->getHLDDNode()->getParentLoop();
  assert(Lp && "Lp can't be a nullptr\n");
  LoopLevel = Lp->getNestingLevel();

  // Check the group's Max DepDist exist and is within bound
  const RegDDRef *LastRef = Group[Group.size() - 1];
  int64_t MaxDepDist = 0;
  bool Ret = DDRefUtils::getConstIterationDistance(LastRef, FirstRef, LoopLevel,
                                                   &MaxDepDist);
  assert(Ret && "Expect DepDist exist\n");
  (void)Ret;
  uint64_t AbsMaxDepDist = std::abs(MaxDepDist);
  assert((AbsMaxDepDist <= HSRA->ScalarReplArrayMaxDepDist) &&
         "Expect MaxDepDist within bound\n");

  // set MaxDepDist:
  this->MaxDepDist = AbsMaxDepDist;

  // set MaxIdxLoadRT, profit test needs it
  markMaxLoad();
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
      MinIdxStoreRT = I; // save index into MinIndexStoreRT
      MaxTopNum = CurTopNum;
    }
  }

  // must find the MinIndxStoreRT
  assert(hasMinIdxStoreRT() && "fail to find MinIdxStoreRT\n");
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

  DEBUG(FOS << "\nHasRWGap: " << HasRWGap << "\n";);
}

bool MemRefGroup::isCompleteStoreOnly(void) {
  if (NumLoads) {
    return false;
  }
  return !HasRWGap;
}

bool MemRefGroup::isLegal(void) const {
  DDGraph DDG = HSRA->HDDA->getGraph(Lp, false);

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
      DEBUG(Edge->print(dbgs()););

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

void MemRefGroup::markMaxLoad(void) {
  if (NumLoads == 0) {
    return;
  }

  // may find the MaxLoad if there is 1+ load(s)
  unsigned Size = RefTupleVec.size();
  unsigned MinTopNum = -1; // will shrink
  RegDDRef *FirstRef = RefTupleVec[0].getMemRef();
  int64_t MaxDepDist = getMaxDepDist();
  bool DepDistExist = false;
  int64_t DepDist = 0;

  for (signed I = Size - 1; I >= 0; --I) { // search high index only
    RefTuple *RT = &RefTupleVec[I];
    RegDDRef *MemRef = RT->getMemRef();
    DepDistExist = DDRefUtils::getConstIterationDistance(MemRef, FirstRef,
                                                         LoopLevel, &DepDist);
    assert(DepDistExist && "Expect DepDist exist\n");
    (void)DepDistExist;
    DepDist = std::abs(DepDist);

    // Only check those RefTuples with MaxDepDist
    if (DepDist != MaxDepDist) {
      break;
    }

    // Only check Load(s);
    if (MemRef->isLval()) {
      continue;
    }

    // Find the smallest TOPO#
    unsigned CurTopNum = MemRef->getHLDDNode()->getTopSortNum();
    if (CurTopNum < MinTopNum) {
      MaxIdxLoadRT = I; // save the MaxIdxLoadRT index
      MinTopNum = CurTopNum;
    }
  }
}

bool MemRefGroup::hasReuse(void) const {
  uint64_t TripCount = 0;
  unsigned MaxDepDist = getMaxDepDist();

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
  HLNodeUtils *HNU = HSRA->HNU;

  // Create all TmpRef(s), and push them into TmpV
  for (unsigned Idx = 0; Idx < getNumTemps(); ++Idx) {
    RegDDRef *TmpRef = HNU->createTemp(DestType, ScalarReplTempName);
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

  DEBUG(FOS << "BEFORE generateTempRotation(.): \n"; Lp->dump(); FOS << "\n");
  HLNodeUtils *HNU = HSRA->HNU;

  for (unsigned Idx = 0, IdxE = TmpV.size() - 1; Idx < IdxE; ++Idx) {
    // Generate a CopyInst: Tn = Tn1
    RegDDRef *LvalRef = TmpV[Idx];
    RegDDRef *RvalRef = TmpV[Idx + 1];
    HLInst *CopyInst = HNU->createCopyInst(RvalRef->clone(), ScalarReplCopyName,
                                           LvalRef->clone());
    HNU->insertAsLastChild(Lp, CopyInst);
  }

  DEBUG(FOS << "AFTER generateTempRotation(.): \n"; Lp->dump(); FOS << "\n");
}

void MemRefGroup::generateLoadToTmps(HLLoop *Lp, SmallVectorImpl<bool> &RWGap) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  DEBUG(FOS << "BEFORE generateLoadToTmps(.): \n"; Lp->dump(); FOS << "\n");
  DEBUG(printRefTupleVec(true););
  DEBUG(printTmpVec(true););

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
    DEBUG(FOS << "Idx: " << Idx << " HasMemRef: " << HasMemRef << "\n";);

    if (HasMemRef) {
      const RefTuple *RT = getByDist(Idx);
      DEBUG(RT->print(true););
      MemRef = RT->getMemRef();
      TmpRef = RT->getTmpRef();
    } else {
      // create the missing MemRef, and obtain its matching TmpRef
      MemRef = BaseRef->clone();
      MemRef->shift(LoopLevel, Idx);
      TmpRef = TmpV[Idx];
      DEBUG(MemRef->dump(); FOS << ", " << Idx << ", "; TmpRef->dump();
            FOS << "\n";);
    }

    // generate the load in loop's prehdr:
    generateLoadInPrehdr(Lp, MemRef, Idx, TmpRef, !HasMemRef, LBCE);
  }

  DEBUG(FOS << "AFTER generateLoadToTmps(.): \n"; Lp->dump(); FOS << "\n");
}

void MemRefGroup::generateLoadInPrehdr(HLLoop *Lp, RegDDRef *MemRef,
                                       unsigned Index, RegDDRef *TmpRef,
                                       bool IndepMemRef, CanonExpr *LBCE) {

  // Create a load from MemRef into Tmp
  RegDDRef *MemRef2 = IndepMemRef ? MemRef : MemRef->clone();
  RegDDRef *TmpRefClone = TmpRef->clone();
  DDRefUtils::replaceIVByCanonExpr(MemRef2, LoopLevel, LBCE);

  // Insert the load into the Lp's preheader
  HLNodeUtils *HNU = HSRA->HNU;
  HLInst *LoadInst = HNU->createLoad(MemRef2, ScalarReplTempName, TmpRefClone);
  HNU->insertAsLastPreheaderNode(Lp, LoadInst);

  // Mark TempRefClone as Lp's LiveIn
  Lp->addLiveInTemp(TmpRefClone->getSymbase());

  // Make MemRef2 consistent
  const SmallVector<const RegDDRef *, 1> AuxRefs = {Lp->getLowerDDRef()};
  MemRef2->makeConsistent(&AuxRefs, Lp->getNestingLevel() - 1);
}

void MemRefGroup::generateStoreFromTmps(HLLoop *Lp) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  DEBUG(FOS << "BEFORE generateStoreFromTmps(.): \n"; Lp->dump(); FOS << "\n");
  DEBUG(printRefTupleVec(true););
  DEBUG(printTmpVec(true););

  // Compute the store boundaries: [MinStorePos .. MaxStorePos]
  // E.g. for the following MemRefGroup:
  // ( r   ,  w   , rw  ,  w    )
  //   0      1     2      3
  //          ^            ^
  //          MinStore     MaxStore, MaxStoreDist is 2 (we knew this)
  //
  const RefTuple *StoreRT = getMinIdxStoreRT();
  assert(StoreRT && "Expect storeRT available\n");
  unsigned MinStoreOffset = StoreRT->getTmpId();
  CanonExpr *UBCE = Lp->getUpperCanonExpr();
  RegDDRef *MemRef = nullptr, *TmpRef = nullptr;
  signed AdjustIdx = isCompleteStoreOnly() ? 0 : (-1);
  HLInst *StoreInst = nullptr;

  // For each unique index in [MinStoreOffset+1 .. MinStoreOffset+MaxStoreDist]:
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

    DEBUG(MemRef->dump(); FOS << ", " << Idx << ", "; TmpRef->dump();
          FOS << "\n";);

    StoreInst = generateStoreInPostexit(Lp, MemRef, TmpRef, UBCE, StoreInst);
  }

  DEBUG(FOS << "AFTER generateStoreFromTmps(.): \n"; Lp->dump(); FOS << "\n");
}

HLInst *MemRefGroup::generateStoreInPostexit(HLLoop *Lp, RegDDRef *MemRef,
                                             RegDDRef *TmpRef, CanonExpr *UBCE,
                                             HLInst *InsertAfter) {

  // Simplify: Replace IV with UBCE
  HLNodeUtils *HNU = HSRA->HNU;
  DDRefUtils::replaceIVByCanonExpr(MemRef, LoopLevel, UBCE);

  // Create a StoreInst
  HLInst *StoreInst = HNU->createStore(TmpRef, ScalarReplStoreName, MemRef);

  if (InsertAfter) {
    HNU->insertAfter(InsertAfter, StoreInst);
  } else {
    HNU->insertAsFirstPostexitNode(Lp, StoreInst);
  }

  Lp->addLiveOutTemp(TmpRef->getSymbase());

  // Make MemRef consistent: remove any stale blob(s)
  const SmallVector<const RegDDRef *, 1> AuxRefs = {Lp->getUpperDDRef()};
  MemRef->makeConsistent(&AuxRefs, Lp->getNestingLevel() - 1);

  return StoreInst;
}

bool MemRefGroup::analyze(HLLoop *Lp) {

  // do Profit Test:
  if (!(IsProfitable = isProfitable())) {
    return false;
  }

  // do Legal Test:
  if (!(IsLegal = isLegal())) {
    return false;
  }

  // identify Store Bounds and mark MaxStoreDist (doPostChecks() needs it)
  markMaxStoreDist();

  // do PostChecks:
  if (!(IsPostChecksOk = doPostChecks(Lp))) {
    return false;
  }

  // set Suitable if all tests pass
  setSuitable(true);

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

  // Profitable:
  FOS << (IsProfitable ? ", profitable " : ", not profitable ");

  // Legal:
  FOS << (IsLegal ? ", legal " : ", illegal ");

  // IsPostChecksOk:
  FOS << (IsPostChecksOk ? ", postchecks: pass " : ", postchecks: failed ");

  // IsSuitable
  FOS << (IsSuitable ? ", suitable " : ", not-suitable ");

  // MaxDepDist:
  FOS << ", MaxDepDist: " << MaxDepDist;

  // Symbase:
  FOS << ", Symbase: " << Symbase << ", ";

  // Print MaxIdxLoadRT: if available
  FOS << "MaxIdxLoadRT: ";
  if (hasMaxIdxLoadRT()) {
    getMaxIdxLoadRT()->print();
  } else {
    FOS << " null ";
  }
  FOS << ", ";

  // Print MinIdxStoreRT: if available
  FOS << "MinIdxStoreRT: ";
  if (hasMinIdxStoreRT()) {
    getMinIdxStoreRT()->print();
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

char HIRScalarReplArray::ID = 0;

INITIALIZE_PASS_BEGIN(HIRScalarReplArray, "hir-scalarrepl-array",
                      "HIR Scalar Replacement of Array ", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatistics)
INITIALIZE_PASS_DEPENDENCY(HIRLocalityAnalysis)
INITIALIZE_PASS_END(HIRScalarReplArray, "hir-scalarrepl-array",
                    "HIR Scalar Replacement of Array ", false, false)

FunctionPass *llvm::createHIRScalarReplArrayPass() {
  return new HIRScalarReplArray();
}

HIRScalarReplArray::HIRScalarReplArray(void) : HIRTransformPass(ID) {
  initializeHIRScalarReplArrayPass(*PassRegistry::getPassRegistry());
}

void HIRScalarReplArray::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequiredTransitive<HIRFramework>();
  AU.addRequiredTransitive<HIRDDAnalysis>();
  AU.addRequiredTransitive<HIRLoopStatistics>();
  AU.addRequiredTransitive<HIRLocalityAnalysis>();
  AU.setPreservesAll();
}

bool HIRScalarReplArray::doInitialization(Module &M) {
  // Check whether the target is an X86 (32b) or X64 (64b) platform
  llvm::Triple TargetTriple(M.getTargetTriple());
  Is32Bit = (TargetTriple.getArch() == llvm::Triple::x86);

  // Adjust default parameter(s) according to current platform
  if (Is32Bit) {
    ScalarReplArrayMaxDepDist = HIRScalarReplArrayDepDistThresholdX86;
  } else {
    ScalarReplArrayMaxDepDist = HIRScalarReplArrayDepDistThresholdX64;
  }

  return true;
}

bool HIRScalarReplArray::handleCmdlineArgs(Function &F) {
  // Skip the Pass if DisableHIRScalarReplArray flag is on
  // or
  // support opt-bisect via skipFunction() call
  if (DisableHIRScalarReplArray || skipFunction(F)) {
    DEBUG(dbgs() << "HIR Scalar Replacement of Array Transformation Disabled "
                    "or Skipped\n");
    return false;
  }

  // Check: ScalarReplArrayMaxDepDist is within bound
  if (ScalarReplArrayMaxDepDist > ScalarReplMaxNumReg) {
    DEBUG(dbgs() << "ScalarReplArrayMaxDepDist is out of bound\n ");
    return false;
  }

  return true;
}

bool HIRScalarReplArray::runOnFunction(Function &F) {
  bool CmdLineOptions = handleCmdlineArgs(F);
  if (!CmdLineOptions) {
    return false;
  }

  DEBUG(dbgs() << "HIRScalarReplArray on Function : " << F.getName() << "\n";);

  // Gather ALL Innermost Loops as Candidates, use 64 increment
  SmallVector<HLLoop *, 64> CandidateLoops;
  auto HIRF = &getAnalysis<HIRFramework>();
  HIRF->getHLNodeUtils().gatherInnermostLoops(CandidateLoops);

  if (CandidateLoops.empty()) {
    DEBUG(dbgs() << F.getName()
                 << "() has no inner-most loop for HIR scalar replacement\n ");
    return false;
  }

  HDDA = &getAnalysis<HIRDDAnalysis>();
  HLA = &getAnalysis<HIRLocalityAnalysis>();
  HLS = &getAnalysis<HIRLoopStatistics>();

  for (auto &Lp : CandidateLoops) {
    setupEnvForLoop(Lp);

    // Analyze the loop and check if it is suitable for ScalarRepl
    if (!doAnalysis(Lp)) {
      continue;
    }

    doTransform(Lp);
  }

  CandidateLoops.clear();
  return false;
}

void HIRScalarReplArray::setupEnvForLoop(const HLLoop *Lp) {
  clearWorkingSetMemory();
  LoopLevel = Lp->getNestingLevel();
}

bool HIRScalarReplArray::doAnalysis(HLLoop *Lp) {
  HNU = &(Lp->getHLNodeUtils());
  DDRU = &(Lp->getDDRefUtils());
  CEU = &(Lp->getCanonExprUtils());

  if (!doPreliminaryChecks(Lp)) {
    DEBUG(dbgs() << "ScalarRepl: Loop Preliminary Checks failed\n";);
    return false;
  }

  if (!doCollection(Lp)) {
    DEBUG(dbgs() << "ScalarRepl: collection failed\n");
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
  if (!Lp->isDo()) {
    return false;
  }

  // Skip if the loop is vectorized:
  // (If a loop's stride is not 1, assume it is a vector loop.)
  int64_t StrideConst = 0;
  Lp->getStrideDDRef()->isIntConstant(&StrideConst);
  if (StrideConst != 1) {
    return false;
  }

  // Skip if the loop has Goto or Call.
  // Note:
  // - allow IF, as long as no relevant MemRef is inside the HLIf.
  // - allow Label: label is harmless if there is no GOTO(s).
  const LoopStatistics &LS = HLS->getSelfLoopStatistics(Lp);
  // DEBUG(LS.dump(););
  if (LS.hasCallsWithUnsafeSideEffects() || LS.hasGotos()) {
    return false;
  }

  return true;
}

bool HIRScalarReplArray::isValid(RefGroupTy &Group, bool &HasNegIVCoeff) {
  // Check the group: expect 2+ items
  if (Group.size() == 1) {
    return false;
  }

  // Check only 1 occurrence
  const RegDDRef *MemRef = Group[0];
  if (!MemRef->hasIV(LoopLevel)) {
    return false;
  }

  if (MemRef->isNonLinear()) {
    return false;
  }

  if (checkIV(MemRef, HasNegIVCoeff)) {
    return false;
  }

  // Check for each occurrence(s):
  for (auto &MemRef : Group) {
    if (MemRef->isVolatile()) {
      return false;
    }

    if (!isa<HLLoop>(MemRef->getHLDDNode()->getParent())) {
      return false;
    }
  }

  return true;
}

bool HIRScalarReplArray::checkIV(const RegDDRef *Ref,
                                 bool &HasNegIVCoeff) const {

  for (auto I = Ref->canon_begin(), E = Ref->canon_end(); I != E; ++I) {
    CanonExpr *CE = (*I);

    int64_t IvCoeff;
    unsigned IvBlobIndex;
    CE->getIVCoeff(LoopLevel, &IvBlobIndex, &IvCoeff);

    // Check any valid IVBlob
    if (IvBlobIndex != InvalidBlobIndex) {
      return true;
    }

    // Check: negative IvCoeff
    if (IvCoeff < 0) {
      HasNegIVCoeff = true;
    }
  }

  return false;
}

bool HIRScalarReplArray::doCollection(HLLoop *Lp) {
  // Collect and group RegDDRefs:
  RefGroupVecTy Groups;
  HLA->populateTemporalLocalityGroups(Lp, ScalarReplArrayMaxDepDist, Groups);
  DEBUG(DDRefGrouping::dump(Groups));

  // Examine each individual group, validate it, and save only the good ones.
  bool Result = false;
  for (RefGroupTy &Group : Groups) {
    bool HasNegIVCoeff = false;

    // Validate and skip any non-suitable group
    if (!isValid(Group, HasNegIVCoeff)) {
      continue;
    }

    // Reverse the group if its has any negative IVCoeff
    if (HasNegIVCoeff) {
      std::reverse(Group.begin(), Group.end());
      DEBUG(printRefGroupTy(Group););
    }

    // Build a MemRefGroup and insert it into MemRefVec
    MRGVec.emplace_back(Group, this);

    Result = true;
  }

  DEBUG(print());
  return Result;
}

bool HIRScalarReplArray::checkAndUpdateQuota(MemRefGroup &MRG,
                                             unsigned &NumGPRsUsed) const {
  bool Result = (NumGPRsUsed + MRG.getNumTemps() <= ScalarReplArrayMaxDepDist);
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
    if (MRG.isSuitable() && checkAndUpdateQuota(MRG, NumGPRsPromoted)) {
      doTransform(Lp, MRG);
      Transformed = true;
    }
  }

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
  DEBUG(FOS << "BEFORE doTransform(.):\n"; Lp->dump(); MRG.print(););

  // Preparations:
  MRG.handleTemps();
  assert(MRG.verify() && "MRG verification failed\n");

  MRG.markMinStore();

  SmallVector<bool, 16> RWGap;
  MRG.identifyGaps(RWGap);

  DEBUG(FOS << "AFTER Preparation:\n"; MRG.print(););

  // 3-step scalar-repl transformation:
  doPreLoopProc(Lp, MRG, RWGap);
  doPostLoopProc(Lp, MRG);
  doInLoopProc(Lp, MRG);

  ++HIRScalarReplArrayPerformed;

  DEBUG(FOS << "AFTER doTransform(.):\n"; Lp->dump(););
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

  DEBUG(FOS << "BEFORE doPreLoopProc(.): \n"; Lp->dump(); FOS << "\n");

  // Sanity: No need for any Complete StoreOnly MRG
  if (MRG.isCompleteStoreOnly()) {
    return;
  }

  MRG.generateLoadToTmps(Lp, RWGap);

  DEBUG(FOS << "AFTER doPreLoopProc(.): \n"; Lp->dump(); FOS << "\n");
}

void HIRScalarReplArray::doPostLoopProc(HLLoop *Lp, MemRefGroup &MRG) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  DEBUG(FOS << "BEFORE doPostLoopProc(.): \n"; Lp->dump(); FOS << "\n");

  if (!MRG.requiresStoreInPostexit()) {
    return;
  }

  MRG.generateStoreFromTmps(Lp);

  DEBUG(FOS << "AFTER doPostLoopProc(.): \n"; Lp->dump(); FOS << "\n");
}

void HIRScalarReplArray::doInLoopProc(HLLoop *Lp, MemRefGroup &MRG) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  DEBUG(FOS << "BEFORE doInLoopProc(.): \n"; Lp->dump(); FOS << "\n";);

  // Generate a load if MaxIdxLoadRT is available
  if (MRG.hasMaxIdxLoadRT()) {
    const RefTuple *MaxIdxLoadRT = MRG.getMaxIdxLoadRT();
    RegDDRef *MemRef = MaxIdxLoadRT->getMemRef();
    RegDDRef *MemRefClone = MemRef->clone();
    RegDDRef *TmpRefClone = MaxIdxLoadRT->getTmpRef()->clone();

    HLInst *LoadInst =
        HNU->createLoad(MemRefClone, ScalarReplLoadName, TmpRefClone);
    HLDDNode *DDNode = MemRef->getHLDDNode();
    HNU->insertBefore(DDNode, LoadInst);
  }

  // Generate a store if MinIdxStoreRT is available
  if (MRG.hasMinIdxStoreRT()) {
    const RefTuple *MinIdxStoreRT = MRG.getMinIdxStoreRT();
    RegDDRef *MemRef = MinIdxStoreRT->getMemRef();
    RegDDRef *MemRefClone = MemRef->clone();
    RegDDRef *TmpRefClone = MinIdxStoreRT->getTmpRef()->clone();

    HLInst *StoreInst =
        HNU->createStore(TmpRefClone, ScalarReplStoreName, MemRefClone);
    HLDDNode *DDNode = MemRef->getHLDDNode();
    HNU->insertAfter(DDNode, StoreInst);
  }

  // Replace each MemRef with its matching Temp
  for (auto &RT : MRG.getRefTupleVec()) {
    // DEBUG(RT.print(true););
    replaceMemRefWithTmp(RT.getMemRef(), RT.getTmpRef());
  }

  DEBUG(FOS << "AFTER handle MemRefs in loop: \n"; Lp->dump(); FOS << "\n");

  // Generate temp-rotation code
  // Note: Not need for any Complete Store-Only MRG
  if (!MRG.isCompleteStoreOnly()) {
    MRG.generateTempRotation(Lp);
  }

  DEBUG(FOS << "AFTER doInLoopProc(.): \n"; Lp->dump(); FOS << "\n";);
}

void HIRScalarReplArray::clearWorkingSetMemory(void) { MRGVec.clear(); }

void HIRScalarReplArray::releaseMemory(void) { clearWorkingSetMemory(); }

void HIRScalarReplArray::replaceMemRefWithTmp(RegDDRef *MemRef,
                                              RegDDRef *TmpRef) {
  HLDDNode *ParentNode = MemRef->getHLDDNode();
  RegDDRef *TmpRefClone = TmpRef->clone();

  // Debug: Examine the DDNode's Parent BEFORE replacement
  // DEBUG(ParentNode->getParent()->dump(););

  HLInst *HInst = dyn_cast<HLInst>(ParentNode);
  // Handle HLInst* special cases: LoadInst and StoreInst
  if (HInst) {
    const Instruction *LLVMInst = HInst->getLLVMInstruction();
    HLInst *CopyInst = nullptr;
    RegDDRef *OtherRef = nullptr;

    // StoreInst: replace with a CopyInst
    if (isa<StoreInst>(LLVMInst) && MemRef->isLval()) {
      OtherRef = HInst->removeOperandDDRef(1);
      CopyInst = HNU->createCopyInst(OtherRef, ScalarReplCopyName, TmpRefClone);
      HNU->replace(HInst, CopyInst);
    }
    // LoadInst: replace with a CopyInst
    else if (isa<LoadInst>(LLVMInst) && MemRef->isRval()) {
      OtherRef = HInst->removeOperandDDRef(0);
      CopyInst = HNU->createCopyInst(TmpRefClone, ScalarReplCopyName, OtherRef);
      HNU->replace(HInst, CopyInst);
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
  // DEBUG(ParentNode->getParent()->dump(););
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
