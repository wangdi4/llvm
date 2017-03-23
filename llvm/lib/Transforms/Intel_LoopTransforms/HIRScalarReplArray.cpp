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
// 1. Handle special case using 1 Temp without ration for MaxDD == 1 (and,
// possibly using 2 Temps for MaxDD == 2).
//
// 2. Remove a majority of MemRef->clone() calls within the 3-step scalar-repl
// transformations (doPreLoopProc(), doPostLooProc(), and doInLoopProc()).
//
// Steps:
// - change the order to:
//   .doInLoopProc(): do all MemRef replacement with temps;
//   .doPreLoopProc(): use MemRef directly without cloning;
//   .doPostLoopProc():use MemRef directly without cloning;
//
// - in doInLoopProc():
//   .do replacement, mark new insert location (once replacement happens,
//   LoadInst/StoreInst will be replaced with CopyInst)
//
// - in doPreLoopProc()/doPostLoopProc():
//   .use MemRef directly without cloning
//
// Note:
// . It is not possible to completely remove all calls to MemRef->clone.
//   However,it is possible to remove most of them.
// . The goal of this todo item is to reduce the MemRef->clone() calls, thus
// reduce the expensive malloc() calls at runtime.
//
// 3. Improve checkIV() to allow IVBlobs that are known to be positive or
// negative at compile time.
//
// It will help to release more groups that are potentially suitable/benefical
// from ScalarRepl transformation.
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
#include "llvm/Transforms/Intel_LoopTransforms/HIRScalarReplArray.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGatherer.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

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

MemRefGroup::MemRefGroup(RefGroupTy &Group, HIRScalarReplArray *HSRA,
                         DDGraph &DDG)
    : HasRWGap(false), HSRA(HSRA), DDG(DDG), MaxDepDist(-1), NumLoads(0),
      NumStores(0), IsLegal(false), IsProfitable(false), IsPostChecksOk(false),
      IsSuitable(false), MaxIdxLoadRT(nullptr), MinIdxStoreRT(nullptr) {
  RegDDRef *FirstRef = const_cast<RegDDRef *>(Group[0]);
  Symbase = FirstRef->getSymbase();
  BaseCE = FirstRef->getBaseCE();

  // Save all MemRef*s from Group into RefTupleVec
  for (auto &Ref : Group) {
    insert(const_cast<RegDDRef *>(Ref));
  }

  Lp = FirstRef->getHLDDNode()->getParentLoop();
  assert(Lp && "Lp can't be a nullptr\n");
  LoopLevel = Lp->getNestingLevel();
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
      MinIdxStoreRT = RT;
      MaxTopNum = CurTopNum;
    }
  }

  // must find the MinIndxStoreRT
  assert(MinIdxStoreRT && "fail to find MinIdxStoreRT\n");
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

  // Identify RWGap only
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

bool MemRefGroup::isLegal(DDGraph &DDG) const {

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

  // may find the MaxLoad if there is 1+ load
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
      MaxIdxLoadRT = RT; // the MaxIdxLoadRT Pointer we save
      MinTopNum = CurTopNum;
    }
  }
}

bool MemRefGroup::hasReuse(void) const {
  // If the loop's trip count is available, use it
  uint64_t TripCount = 0;
  if (Lp->isConstTripLoop(&TripCount)) {
    return (getMaxDepDist() < TripCount);
  }

  return true;
}

bool MemRefGroup::hasStoreDepDistGreaterEqualOne() const {
  // Sanity: need 2 stores at least
  if (NumStores < 2) {
    return false;
  }

  // Identify lowest-idx ref
  unsigned Size = RefTupleVec.size();
  RegDDRef *LowestRef = nullptr;
  for (unsigned I = 0; I < Size; ++I) {
    RegDDRef *MemRef = RefTupleVec[I].getMemRef();

    if (MemRef->isLval()) {
      LowestRef = MemRef;
      break;
    }
  }
  assert(LowestRef && "Expect LowestRef available\n");

  // Identify highest-idx ref
  RegDDRef *HighestRef = nullptr;
  for (signed I = Size - 1; I >= 0; --I) {
    RegDDRef *MemRef = RefTupleVec[I].getMemRef();

    if (MemRef->isLval()) {
      HighestRef = MemRef;
      break;
    }
  }
  assert(HighestRef && "Expect HighestRef available\n");

  // Compute and check the dependence distance between the 2 boundary refs
  int64_t DepDist = 0;
  if (DDRefUtils::getConstIterationDistance(HighestRef, LowestRef, LoopLevel,
                                            &DepDist)) {
    if (std::llabs(DepDist) >= 1) {
      return true;
    }
  }

  return false;
}

bool MemRefGroup::doPostCheckOnRef(const HLLoop *Lp, bool IsLoad) {
  const CanonExpr *BoundCE =
      IsLoad ? Lp->getLowerCanonExpr() : Lp->getUpperCanonExpr();
  RegDDRef *FirstRef = RefTupleVec[0].getMemRef();

  return DDRefUtils::canReplaceIVByCanonExpr(FirstRef, LoopLevel, BoundCE,
                                             true);
}

// Check: expect MaxDepDist be below a pre-defined threshold
// Notes:
// - Since MRG is sorted, MaxDepDist is the distance between 1st and last ref.
// - If the group has negative IVConst Coeff, the DepDist could be negative
//   since the group had been reversed earlier.
//   A call to std::abs() is needed in this case.
//
void MemRefGroup::checkAndSetMaxDepDist(void) {
  RegDDRef *FirstRef = RefTupleVec[0].getMemRef();
  RegDDRef *LastRef = RefTupleVec[getSize() - 1].getMemRef();
  int64_t MaxDepDist = 0;
  bool Ret = DDRefUtils::getConstIterationDistance(LastRef, FirstRef, LoopLevel,
                                                   &MaxDepDist);
  assert(Ret && "Expect DepDist exist\n");
  (void)Ret;
  uint64_t AbsMaxDepDist = std::abs(MaxDepDist);
  assert((AbsMaxDepDist <= HSRA->ScalarReplArrayMaxDepDist) &&
         "Expect MaxDepDist within bound\n");

  // Save MaxDepDist:
  setMaxDepDist(AbsMaxDepDist);
}

bool MemRefGroup::doPostChecks(const HLLoop *Lp) {
  // If: the group has MaxDepDist > 0
  // Then:any outstanding (non-max-dd) load needs to be merge-able with Lp's
  // LB
  if ((MaxDepDist > 0) && !doPostCheckOnRef(Lp, true)) {
    return false;
  }

  // If: the group has multiple stores with MaxStoreDepDist >=1
  // Then:any outstanding (non-min-dd) store needs to be merge-able with Lp's
  // UB
  if (hasStoreDepDistGreaterEqualOne() && !doPostCheckOnRef(Lp, false)) {
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
    TmpId = std::llabs(TmpId);
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

  DEBUG(FOS << "BEFORE generateLoadToTmps(.): \n";; Lp->dump(); FOS << "\n");
  DEBUG(printRefTupleVec(true););
  DEBUG(printTmpVec(true););

  // Iterate over each possible index:
  // - NO gap: generate a load use existing MemRef;
  // -    gap: create its matching MemRef, then generate a load with it;
  RegDDRef *MemRef = nullptr;
  RegDDRef *TmpRef = nullptr;
  CanonExpr *LBCE = Lp->getLowerCanonExpr();

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
      MemRef = RefTupleVec[0].getMemRef()->clone();
      MemRef->shift(LoopLevel, Idx);
      TmpRef = TmpV[Idx];
      DEBUG(MemRef->dump(); FOS << ", " << Idx << ", "; TmpRef->dump();
            FOS << "\n";);
    }

    // generate the load
    generateLoadWithMemRef(Lp, MemRef, Idx, TmpRef, !HasMemRef, LBCE);
  }

  DEBUG(FOS << "AFTER generateLoadToTmps(.): \n"; Lp->dump(); FOS << "\n");
}

void MemRefGroup::generateLoadWithMemRef(HLLoop *Lp, RegDDRef *MemRef,
                                         unsigned Index, RegDDRef *TmpRef,
                                         bool IndepMemRef, CanonExpr *LBCE) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif

  DEBUG(FOS << "BEFORE generateLoadWithMemRef(.): \n"; Lp->dump(); FOS << "\n");

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

  DEBUG(FOS << "AFTER generateLoadWithMemRef(.): \n"; Lp->dump(); FOS << "\n");
}

bool MemRefGroup::analyze(HLLoop *Lp, DDGraph &DDG) {
  // Count: #Loads and #Stores
  for (auto &RT : RefTupleVec) {
    if (RT.getMemRef()->isLval()) {
      ++NumStores;
    } else {
      ++NumLoads;
    }
  }

  // set MaxDepDist: markMaxLoad() needs it
  checkAndSetMaxDepDist();

  // this sets MaxIdxLoadRT, profit test needs it
  markMaxLoad();

  // do Profit Test:
  if (!(IsProfitable = isProfitable())) {
    return false;
  }

  // do Legal Test:
  if (!(IsLegal = isLegal(DDG))) {
    return false;
  }

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
    // RegDDRef *MemRef = RT.getMemRef();
    if (!RT.getMemRef()) {
      return false;
    }

    // int64_t TmpId = RT.getTmpId();
    if (RT.getTmpId() == -1) {
      return false;
    }

    // RegDDRef *TmpRef = RT.getTmpRef();
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

  // Print MaxIdxLoadRT:
  FOS << "MaxIdxLoadRT: ";
  if (MaxIdxLoadRT) {
    MaxIdxLoadRT->print();
  } else {
    FOS << " null ";
  }
  FOS << ", ";

  // Print MinIdxStoreRT:
  FOS << "MinIdxStoreRT: ";
  if (MinIdxStoreRT) {
    MinIdxStoreRT->print();
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
                 << "() has no inner-most loop for scalar replacement\n ");
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

  DDGraph DDG = HDDA->getGraph(Lp, false);
  if (!doCollection(Lp, DDG)) {
    DEBUG(dbgs() << "ScalarRepl: collection failed\n");
    return false;
  }

  bool Result = false;

  for (auto &MRG : MRGVec) {
    // Analyze each MemRefGroup
    if (MRG.analyze(Lp, DDG)) {
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
  if (LS.hasCalls() || LS.hasGotos()) {
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

bool HIRScalarReplArray::doCollection(HLLoop *Lp, DDGraph &DDG) {
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
    MemRefGroup MRG(Group, this, DDG);
    insert(MRG);

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

  // markMaxLoad() is moved to analysis
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

  // Check: not needed if the MRG has <=1 write
  if (MRG.getNumStores() <= 1) {
    return;
  }

  RefTuple *StoreRT = MRG.getMinIdxStoreRT();
  assert(StoreRT && "Expect storeRT available\n");
  unsigned SkipDepDist = StoreRT->getTmpId();
  CanonExpr *UBCE = Lp->getUpperCanonExpr();

  // Create store(s) from temp at begin of the Loop's Postexit
  for (auto &RT : MRG.getRefTupleVec()) {
    // skip any Load
    if (RT.getMemRef()->isRval()) {
      continue;
    }

    // skip any Ref whose DD is the same as SkipDepDist
    unsigned DepDist = RT.getTmpId();
    if (DepDist == SkipDepDist) {
      continue;
    }

    // mask SkipDepDist with Current DepDist
    //(avoiding future duplicates on the same DepDist)
    SkipDepDist = DepDist;

    RegDDRef *MemRef = RT.getMemRef();
    RegDDRef *TmpRef = RT.getTmpRef();
    RegDDRef *MemRefClone = MemRef->clone();

    // Replace IV with UBCE
    DDRefUtils::replaceIVByCanonExpr(MemRefClone, LoopLevel, UBCE);

    // Create the StoreInst
    RegDDRef *TmpRefClone = TmpRef->clone();
    HLInst *StoreInst =
        HNU->createStore(TmpRefClone, ScalarReplStoreName, MemRefClone);
    HNU->insertAsFirstPostexitNode(Lp, StoreInst);
    Lp->addLiveOutTemp(TmpRefClone->getSymbase());

    // Make MemRefClone consistent: remove any stale blob(s)
    const SmallVector<const RegDDRef *, 1> AuxRefs = {Lp->getUpperDDRef()};
    MemRefClone->makeConsistent(&AuxRefs, Lp->getNestingLevel() - 1);
  }

  DEBUG(FOS << "AFTER doPostLoopProc(.): \n"; Lp->dump(); FOS << "\n");
}

void HIRScalarReplArray::doInLoopProc(HLLoop *Lp, MemRefGroup &MRG) {
#ifndef NDEBUG
  formatted_raw_ostream FOS(dbgs());
#endif
  DEBUG(FOS << "BEFORE doInLoopProc(.): \n"; Lp->dump(); FOS << "\n";);

  // Generate a load if MaxIdxLoadRT is available
  RefTuple *MaxIdxLoadRT = MRG.getMaxIdxLoadRT();
  if (MaxIdxLoadRT) {
    RegDDRef *MemRef = MaxIdxLoadRT->getMemRef();
    RegDDRef *MemRefClone = MemRef->clone();
    RegDDRef *TmpRefClone = MaxIdxLoadRT->getTmpRef()->clone();

    HLInst *LoadInst =
        HNU->createLoad(MemRefClone, ScalarReplLoadName, TmpRefClone);
    HLDDNode *DDNode = MemRef->getHLDDNode();
    HNU->insertBefore(DDNode, LoadInst);
  }

  // Generate a store if MinIdxStoreRT is available
  RefTuple *MinIdxStoreRT = MRG.getMinIdxStoreRT();
  if (MinIdxStoreRT) {
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
