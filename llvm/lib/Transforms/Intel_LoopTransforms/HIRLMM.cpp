//===--- HIRLMM.cpp -Implements Loop Memory Motion Pass -*- C++ -*---===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-----------------------------------------------------------------===//
// HIR LIMM Case1: Load Hoisting Only
//
// [ORIGINAL]                       [AFTER LIMM]
//
//                                  t0 = A[0];
// for(i=0; i<=4; ++i){             for(i=0; i<=4; ++i){
//   ..                               ..
//    = A[0];                        = . t0;
//   ..                               ..
// }                                }
//
// *** ----------------             --------------------***
//
// HIR LIMM Case2: Store Sinking Only
//
// [ORIGINAL]                       [AFTER LIMM]
//
// for(i=0; i<=4; ++i){             for(i=0; i<=4; ++i){
//   ..                               ..
//   A[0] = .                        t0 = .
//   ..                               ..
// }                                }
//                                  A[0] = t0;
//
// *** ----------------             --------------------***
//
// HIR LIMM Case3: Load Hoisting and Store Sinking
//  (with a leading LoopInv Load)
//
// [ORIGINAL]                       [AFTER LIMM]
//
//                                  t0 = A[0];
// for(i=0; i<=4; ++i){             for(i=0; i<=4; ++i){
//   ..                               ..
//        = A[0]                        = t0
//   ..                              ..
//   A[0] = .                        t0 = .
//   ..                               ..
// }                                }
//                                  A[0] = t0;
//
// *** ----------------             --------------------***
//
// HIR LIMM Case3: Load Hoisting and Store Sinking
//  (with a leading LoopInv Store)
//
// [ORIGINAL]                       [AFTER LIMM]
//
// for(i=0; i<=4; ++i){             for(i=0; i<=4; ++i){
//   ..                               ..
//   A[0] = .                        t0 = .
//   ..                               ..
//    .   = A[0]                        = t0
//   ..                               ..
// }                                }
//                                  A[0] = t0;
//
//===----------------------------------------------------------------------===//
//
// This file implements HIR Loop-Memory-Motion Transformation (LMM) Pass.
//
// Available options:
// -hir-lmm:          Perform HIR Loop Memory Motion
// -disable-hir-lmm:  Disable/Bypass HIR Loop Memory Motion
//
#include "llvm/Transforms/Intel_LoopTransforms/HIRLMM.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"

#include "HIRLMMImpl.h"

#define DEBUG_TYPE "hir-lmm"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::lmm;

const std::string LIMMTempName = "limm";
const std::string LIMMCopyName = "copy";

// Disable the HIR Loop-Invariant Memory Motion (default is false)
static cl::opt<bool>
    DisableHIRLMM("disable-hir-lmm", cl::init(false), cl::Hidden,
                  cl::desc("Disable HIR Loop Memory Motion (LMM)"));

STATISTIC(
    HIRLIMMRefPromoted,
    "Number of HIR loop-invariant memory load(s)/store(s) References Promoted");

MemRefGroup::MemRefGroup(RegDDRef *FirstRef)
    : IsProfitable(false), IsLegal(false), IsAnalyzed(false), HasLoad(false),
      HasLoadOnDomPath(false), HasStore(false), HasStoreOnDomPath(false) {
  RefV.push_back(FirstRef);

  Lp = FirstRef->getHLDDNode()->getParentLoop();
  assert(Lp && "Not expecting null Lp\n");
}

bool MemRefGroup::belongs(RegDDRef *Ref) const {
  return DDRefUtils::areEqual(Ref, RefV[0]);
}

void MemRefGroup::analyze(void) {
  // Only analyze once
  assert(!IsAnalyzed && "Analyze ONCE only\n");

  IsAnalyzed = true;
  const HLNode *LoopTail = Lp->getLastChild();
  const HLNode *LoopHead = Lp->getFirstChild();

  // Scan each Ref in group, and do statistical collection, for:
  // Load, LoadOnDomPath, Store, StoreOnDomPath
  for (auto &Ref : RefV) {
    bool IsLoad = Ref->isRval();

    // Load
    if (IsLoad) {
      HasLoad = true;

      // Load on DomPath
      if (!HasLoadOnDomPath &&
          HLNodeUtils::postDominates(Ref->getHLDDNode(), LoopHead)) {
        HasLoadOnDomPath = true;
      }
    }
    // Store
    else {
      HasStore = true;

      // Store on DomPath
      if (!HasStoreOnDomPath &&
          HLNodeUtils::dominates(Ref->getHLDDNode(), LoopTail)) {
        HasStoreOnDomPath = true;
      }
    }
  }
}

#ifndef NDEBUG
LLVM_DUMP_METHOD void MemRefGroup::print(bool NewLine) {
  formatted_raw_ostream FOS(dbgs());

  // Print 1st item (since they all look the same)
  RefV[0]->print(FOS);
  FOS << " { ";

  // For each ref, print 'W' for a write, and 'R' for a read
  unsigned NumLoad = 0, NumStore = 0;
  for (auto &Ref : RefV) {
    if (Ref->isLval()) {
      FOS << " W ";
      ++NumStore;
    } else {
      FOS << " R ";
      ++NumLoad;
    }
  }

  FOS << " } ";

  // Print # of Read(s) and Write(s):
  FOS << NumStore << "W : " << NumLoad << "R ";

  // IsProfitable info
  FOS << (IsProfitable ? " profitable " : " not profitable ");

  // IsLegal info
  FOS << (IsLegal ? " legal " : " illegal ");

  // IsAnalyzed?
  FOS << (IsAnalyzed ? " analyzed " : " not analyzed ");
  if (IsAnalyzed) {
    std::string msg;
    if (isStoreOnly()) {
      msg = " :store-only: ";
    } else if (isLoadOnly()) {
      msg = " :load-only: ";
    } else {
      msg = " :load-store mix: ";
    }
    FOS << msg;
  }

  // 3. Newline?
  if (NewLine) {
    FOS << "\n";
  }
}
#endif

// *** Implementation of long MemRefCollection Methods ***

bool MemRefCollection::find(RegDDRef *Ref, unsigned &Index) const {
  // Search available record(s) in the collection
  for (unsigned Idx = 0, IdxE = MRVV.size(); Idx < IdxE; ++Idx) {
    auto &MRG = MRVV[Idx];

    // Check if Ref belongs to MRG
    if (MRG.belongs(Ref)) {
      Index = Idx;
      return true;
    }
  }

  // Default: failed to find a match
  return false;
}

void MemRefCollection::insert(RegDDRef *Ref) {
  unsigned Idx = 0;

  // Check if the Ref is available in current Collection
  // -yes: insert using the idx
  // -no:  create a new MRG with the ref, and push the MRG into MRVV
  if (find(Ref, Idx)) {
    MRVV[Idx].insert(Ref);
  } else {
    MRVV.emplace_back(Ref);
  }
}

#ifndef NDEBUG
LLVM_DUMP_METHOD void MemRefCollection::print(void) {
  formatted_raw_ostream FOS(dbgs());
  FOS << "MemRefCollection, entries: " << MRVV.size() << "\n";

  // Sanity check: any available MRG in collection?
  if (MRVV.size() == 0) {
    FOS << " __EMPTY__ \n";
    return;
  }

  for (auto &MRG : MRVV) {
    FOS << "  ";
    MRG.print(true);
  }
}
#endif

// Collect every loop-inv MemRef within the given range
class HIRLMM::CollectMemRefs final : public HLNodeVisitorBase {
private:
  struct MemRefCollection &MRC;
  unsigned LoopLevel;

public:
  CollectMemRefs(MemRefCollection &InitMRC, unsigned InitLevel)
      : MRC(InitMRC), LoopLevel(InitLevel) {
    assert(CanonExprUtils::isValidLoopLevel(InitLevel) &&
           "LoopLevel is out of bound\n");
  }

  void visit(HLDDNode *Node) {
    // Scan each HLDDNode from Right to Left, so that any potential load
    // appears earlier than store on the same HLDDNode*.
    for (auto It = Node->op_ddref_rbegin(), ItE = Node->op_ddref_rend();
         It != ItE; ++It) {
      collectMemRef(*It);
    }
  }

  // No processing needed for Goto, Label and HLNode types
  void visit(HLGoto *Goto){};
  void visit(HLLabel *Label){};
  void visit(HLNode *Node) {
    llvm_unreachable(" visit(HLNode *) - Node not supported\n");
  }
  void postVisit(HLNode *Node) {}

  bool isEmpty(void) { return MRC.isEmpty(); }

  void collectMemRef(RegDDRef *Ref);
};

// Collect all relevant MemRef(s)
void HIRLMM::CollectMemRefs::collectMemRef(RegDDRef *Ref) {
  // Check: must be MemRef
  if (!Ref->isMemRef()) {
    return;
  }

  // Check: must be LoopInvariant w.r.t a given loop level
  if (!Ref->isStructurallyInvariantAtLevel(LoopLevel)) {
    return;
  }

  // Collect: insert the RegDDRef* into MRC
  MRC.insert(Ref);

  // LLVM_DEBUG(MRC.print(););
}

// The following preliminary conditions are currently checked:
// - Empty loop body
// - LoopStatistics
//   . no call
//
bool HIRLMM::doLoopPreliminaryChecks(const HLLoop *Lp) {

  if (Lp->hasDistributePoint()) {
    return false;
  }
  const LoopStatistics &LS = HLS.getSelfLoopStatistics(Lp);
  // LLVM_DEBUG(LS.dump(););
  if (LS.hasCallsWithUnknownAliasing()) {
    return false;
  }

  return true;
}

bool HIRLMM::run() {
  if (DisableHIRLMM) {
    LLVM_DEBUG(dbgs() << "HIRLMM (Loop Memory Motion) Disabled or Skipped\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIR LIMM on Function : " << HIRF.getFunction().getName()
                    << "\n");

  // Gather all inner-most Loop Candidates
  SmallVector<HLLoop *, 64> CandidateLoops;

  HNU.gatherInnermostLoops(CandidateLoops);

  if (CandidateLoops.empty()) {
    LLVM_DEBUG(dbgs() << HIRF.getFunction().getName()
                      << "() has no inner-most loop\n ");
    return false;
  }
  // LLVM_DEBUG(dbgs() << " # Innermost Loops: " << CandidateLoops.size() <<
  // "\n");

  bool Result = false;

  for (auto &Lp : CandidateLoops) {
    Result = doLoopMemoryMotion(Lp) || Result;
  }

  CandidateLoops.clear();
  return Result;
}

bool HIRLMM::doLoopMemoryMotion(HLLoop *Lp) {
  // Analyze the loop and reject if unsuitable
  if (!doAnalysis(Lp)) {
    return false;
  }

  // Do Loop Memory Motion (LMM) promotion
  doTransform(Lp);
  return true;
}

// Conduct ALL HIR-LMM Tests to decide whether the loop is good for LMM
bool HIRLMM::doAnalysis(HLLoop *Lp) {
  // LLVM_DEBUG(dbgs() << "Current Loop: \n"; Lp->dump(););
  clearWorkingSetMemory();
  LoopLevel = Lp->getNestingLevel();

  if (!doLoopPreliminaryChecks(Lp)) {
    LLVM_DEBUG(dbgs() << "HIRLMM: failed Loop Preliminary Checks\n";);
    return false;
  }

  if (!doCollection(Lp)) {
    LLVM_DEBUG(dbgs() << "HIRLMM: failed DoCollection\n");
    return false;
  }

  if (!isProfitable(Lp)) {
    LLVM_DEBUG(dbgs() << "HIRLMM: failed profit test\n");
    return false;
  }

  if (!isLegal(Lp)) {
    LLVM_DEBUG(dbgs() << "HIRLMM: failed legal test\n");
    return false;
  }

  return true;
}

// Do a loop collection
// (After collection, data is in MRC)
bool HIRLMM::doCollection(HLLoop *Lp) {
  // Collect all loop-inv MemRefs within the loop's body
  CollectMemRefs Collector(MRC, LoopLevel);
  HLNodeUtils::visitRange(Collector, Lp->getFirstChild(), Lp->getLastChild());

  // Examine the collection result
  // LLVM_DEBUG(MRC.print(););

  // Analyze each Group inside MRC
  MRC.analyze();

  // Check if there is at least 1 MRG available after collection
  return !Collector.isEmpty();
}

// A loop is profitable for LMM if there is at least 1 profitable Group
bool HIRLMM::isProfitable(const HLLoop *Lp) {
  bool Result = false;

  for (unsigned Idx = 0, IdxE = MRC.getSize(); Idx != IdxE; ++Idx) {
    MemRefGroup &MRG = MRC.get(Idx);

    if (MRG.hasAnyLoadOrStoreOnDominatePath()) {
      MRG.setProfitable(true);
      Result = true;
    }
  }

  return Result;
}

// A Loop is legal if there is it has at least 1 legal group
bool HIRLMM::isLegal(const HLLoop *Lp) {
  bool Result = false;
  DDGraph DDG = HDDA.getGraph(Lp);

  // Do legal test on any profitable MRG
  for (unsigned Idx = 0, IdxE = MRC.getSize(); Idx != IdxE; ++Idx) {
    MemRefGroup &MRG = MRC.get(Idx);

    if (MRG.getProfitable() && isLegal(Lp, MRG, DDG)) {
      MRG.setLegal(true);
      Result = true;
    }
  }

  // Check the MRC after legality analysis
  // LLVM_DEBUG(MRC.print(););

  return Result;
}

// A Group is legal IF&F every Ref is legal within the MRG
bool HIRLMM::isLegal(const HLLoop *Lp, MemRefGroup &MRG, DDGraph &DDG) {
  for (unsigned II = 0, IE = MRG.getSize(); II != IE; ++II) {
    const RegDDRef *Ref = MRG.get(II);
    if (!areDDEdgesLegal(Lp, Ref, DDG)) {
      return false;
    }
  }

  return true;
}

bool HIRLMM::areDDEdgesLegal(const HLLoop *Lp, const RegDDRef *Ref,
                             DDGraph &DDG) {
  bool IsLoad = Ref->isRval();
  DDRef *OtherRef = nullptr;
  bool Result = true;

  // Iterate over each relevant DDEdge
  // Load: incoming-edge iterators
  // Store: outgoing-edge iterators
  for (const DDEdge *Edge : (IsLoad ? DDG.incoming(Ref) : DDG.outgoing(Ref))) {
    LLVM_DEBUG(Edge->print(dbgs()););

    // Setup OtherRef
    if (IsLoad) {
      OtherRef = Edge->getSrc();
    } else {
      OtherRef = Edge->getSink();
    }

    // Test: both Ref and OtherRef are equal
    if (!DDRefUtils::areEqual(Ref, OtherRef)) {
      // Test: DV has any < or > before the loop level
      const DirectionVector &DV = Edge->getDV();
      if (!DV.isIndepFromLevel(LoopLevel)) {
        Result = false;
      }
    }
  }

  return Result;
}

// LMM transform always succeeds because the given loop is guaranteed to have
// at least 1 suitable (both profitable and legal) group.
void HIRLMM::doTransform(HLLoop *Lp) {
  unsigned NumLIMM = 0;
  SmallSet<unsigned, 32> TempRefSet;

  // Iterate over each group in MRC, do LMM promotion if suitable.
  for (unsigned Idx = 0, IdxE = MRC.getSize(); Idx < IdxE; ++Idx) {
    MemRefGroup &MRG = MRC.get(Idx);

    // Skip MRG if it is either illegal or non-profitable
    if (!MRG.getProfitable() || !MRG.getLegal()) {
      continue;
    }

    // LLVM_DEBUG(MRG.print(true); dbgs() << "Before LIMM on a MRG\n";
    // Lp->dump(););
    doLIMMRef(Lp, MRG, TempRefSet);
    ++NumLIMM;
    // LLVM_DEBUG(dbgs() << "After LIMM on a MRG\n"; Lp->dump(););
  }

  HIRLIMMRefPromoted += NumLIMM;

  // Mark the loop and its parent loop/region have been changed
  Lp->getParentRegion()->setGenCode();
  HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Lp);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(Lp);

  HLNodeUtils::removeEmptyNodes(Lp);
}

bool HIRLMM::canHoistSingleLoad(HLLoop *Lp, RegDDRef *FirstRef,
                                MemRefGroup &MRG,
                                SmallSet<unsigned, 32> &TempRefSet) const {
  if (MRG.getSize() != 1 || !FirstRef->isRval()) {
    return false;
  }

  HLDDNode *LoadDDNode = FirstRef->getHLDDNode();

  HLInst *LoadHInst = dyn_cast<HLInst>(LoadDDNode);

  if (!LoadHInst) {
    return false;
  }

  const Instruction *LLVMLoadInst = LoadHInst->getLLVMInstruction();

  if (!isa<LoadInst>(LLVMLoadInst)) {
    return false;
  }

  RegDDRef *LRef = LoadHInst->getLvalDDRef();

  if (TempRefSet.count(LRef->getSymbase())) {
    return false;
  }

  DDGraph DDG = HDDA.getGraph(Lp);

  for (DDEdge *E : DDG.incoming(LRef)) {
    if (E->isAnti() || E->isOutput()) {
      return false;
    }
  }

  for (DDEdge *E : DDG.outgoing(LRef)) {
    if (E->isOutput()) {
      return false;
    }
  }

  return true;
}

bool HIRLMM::canSinkSingleStore(HLLoop *Lp, RegDDRef *FirstRef,
                                MemRefGroup &MRG,
                                SmallSet<unsigned, 32> &TempRefSet) const {
  if (Lp->getNumExits() > 1) {
    return false;
  }

  if (MRG.getSize() != 1 || !FirstRef->isLval()) {
    return false;
  }

  HLDDNode *StoreDDNode = FirstRef->getHLDDNode();
  HLInst *StoreHInst = dyn_cast<HLInst>(StoreDDNode);

  if (!StoreHInst) {
    return false;
  }

  const Instruction *LLVMStoreInst = StoreHInst->getLLVMInstruction();

  if (!isa<StoreInst>(LLVMStoreInst)) {
    return false;
  }

  RegDDRef *RRef = StoreHInst->getRvalDDRef();

  if (!RRef->isSelfBlob()) {
    return false;
  }

  if (StoreHInst == Lp->getLastChild()) {
    return true;
  }

  if (TempRefSet.count(RRef->getSymbase())) {
    return false;
  }

  DDGraph DDG = HDDA.getGraph(Lp);

  for (DDEdge *E : DDG.outgoing(RRef)) {
    if (E->isAnti()) {
      return false;
    }
  }

  return true;
}

// Check whether we need a Load in the Loops' preheader:
// - If there is no load (store-only MRG), no need for tmp in prehder
// - Scan the MRG from beginning:
//  . 1st hit a load: need
//  . 1st hit a store on dom path: no need
//
bool HIRLMM::isLoadNeededInPrehder(HLLoop *Lp, MemRefGroup &MRG) {
  if (MRG.isStoreOnly()) {
    return false;
  }

  const HLNode *LoopTail = Lp->getLastChild();

  for (unsigned Idx = 0, IdxE = MRG.getSize(); Idx < IdxE; ++Idx) {
    RegDDRef *CurRef = MRG.get(Idx);

    // If hit a Load 1st, need tmp
    if (CurRef->isRval()) {
      return true;
    }

    // If hit a Store (on dominate path) 1st, no need of tmp
    if (HLNodeUtils::dominates(CurRef->getHLDDNode(), LoopTail)) {
      return false;
    }
  }

  llvm_unreachable("Not expect control to reach here\n");
}

// Check all the LHS MemRef within the parent loop equals to the LoadRef and
// skip checking the current loop
class LvalMemRefChecker final : public HLNodeVisitorBase {
private:
  HLNode *SkipNode;
  RegDDRef *LoadRef;
  bool IsLegal;

public:
  LvalMemRefChecker(HLNode *SkipNode, RegDDRef *LoadRef)
      : SkipNode(SkipNode), LoadRef(LoadRef), IsLegal(true) {}

  void visit(HLDDNode *Node) {
    for (auto It = Node->ddref_begin(), ItE = Node->ddref_end(); It != ItE;
         ++It) {
      preventsHoisting(*It);
    }
  }

  void visit(HLNode *Node) {}

  void postVisit(HLNode *Node) {}

  bool skipRecursion(HLNode *Node) { return Node == SkipNode; }

  bool isLegal() const { return IsLegal; }

  void preventsHoisting(RegDDRef *Ref);
};

void LvalMemRefChecker::preventsHoisting(RegDDRef *Ref) {
  if (!Ref->isMemRef()) {
    return;
  }

  if (!Ref->isLval()) {
    return;
  }

  if (Ref->getSymbase() != LoadRef->getSymbase()) {
    return;
  }

  int64_t Distance;

  if (!DDRefUtils::getConstByteDistance(Ref, LoadRef, &Distance)) {
    IsLegal = false;
    return;
  }

  uint64_t Dist = std::abs(Distance);

  if (Dist < LoadRef->getDestTypeSizeInBytes()) {
    IsLegal = false;
    return;
  }
}

HLLoop *HIRLMM::getOuterLoopCandidateForSingleLoad(HLLoop *Lp, RegDDRef *Ref,
                                                   MemRefGroup &MRG) {
  if (MRG.getSize() != 1 || !Ref->isRval()) {
    return Lp;
  }

  HLLoop *ParentLp = nullptr;

  for (unsigned ParentLevel = Lp->getNestingLevel() - 1; ParentLevel > 0;
       Lp = ParentLp, --ParentLevel) {
    if (Lp->hasZtt()) {
      break;
    }

    ParentLp = dyn_cast<HLLoop>(Lp->getParent());

    if (!ParentLp) {
      break;
    }

    if (!Ref->isStructurallyInvariantAtLevel(ParentLevel)) {
      break;
    }

    const LoopStatistics &LS = HLS.getSelfLoopStatistics(ParentLp);

    if (LS.hasCallsWithUnknownAliasing()) {
      break;
    }

    LvalMemRefChecker Checker(Lp, Ref);
    HLNodeUtils::visitRange(Checker, ParentLp->getFirstChild(),
                            ParentLp->getLastChild());

    if (!Checker.isLegal()) {
      break;
    }

    Lp = ParentLp;
  }

  return Lp;
}

bool HIRLMM::hoistedSingleLoad(HLLoop *Lp, RegDDRef *LoadRef, MemRefGroup &MRG,
                               SmallSet<unsigned, 32> &TempRefSet,
                               LoopOptReportBuilder &LORBuilder) {
  if (!canHoistSingleLoad(Lp, LoadRef, MRG, TempRefSet)) {
    return false;
  }

  HLDDNode *LoadDDNode = LoadRef->getHLDDNode();

  RegDDRef *TempRef = LoadDDNode->getLvalDDRef();

  Lp->addLiveInTemp(TempRef->getSymbase());

  DDGraph DDG = HDDA.getGraph(Lp);

  for (DDEdge *E : DDG.outgoing(TempRef)) {
    if (E->isFlow()) {
      DDRef *DDRefSink = E->getSink();
      setLinear(DDRefSink);
    }
  }

  HLNodeUtils::moveAsLastPreheaderNode(Lp, LoadDDNode);

  LoadRef->updateDefLevel(LoopLevel - 1);

  LORBuilder(*Lp).addRemark(OptReportVerbosity::Low,
                            "Load hoisted out of the loop");

  return true;
}

bool HIRLMM::sinkedSingleStore(HLLoop *Lp, RegDDRef *StoreRef, MemRefGroup &MRG,
                               SmallSet<unsigned, 32> &TempRefSet,
                               LoopOptReportBuilder &LORBuilder) {
  if (!canSinkSingleStore(Lp, StoreRef, MRG, TempRefSet)) {
    return false;
  }

  HLDDNode *StoreDDNode = StoreRef->getHLDDNode();

  HLNodeUtils::moveAsFirstPostexitNode(Lp, StoreDDNode);

  RegDDRef *TempRef = StoreDDNode->getRvalDDRef();

  Lp->addLiveOutTemp(TempRef->getSymbase());

  StoreRef->updateDefLevel(LoopLevel - 1);
  TempRef->updateDefLevel(LoopLevel - 1);

  LORBuilder(*Lp).addRemark(OptReportVerbosity::Low,
                            "Store sinked out of the loop");
  return true;
}

// Do LIMM promotion on a MRG
//
// [Algorithm]
//
// For each candidate group (MRG)
//
// [Preparation]
// Decide if a store is needed in Loop's postexit:
//  -yes: if there is at least 1 store in MRG
//
// Decide if a load is needed in Loop's preheader
//  -details in isLoadNeededInPrehder()
//
// [Do LIMM Promotion]
//  - Create a load in preheader if needed
//  - Create a store in postexit if needed
//  - Replace each load/store in group with the Tmp
//
void HIRLMM::doLIMMRef(HLLoop *Lp, MemRefGroup &MRG,
                       SmallSet<unsigned, 32> &TempRefSet) {
  bool NeedLoadInPrehdr = false, NeedStoreInPostexit = false;
  RegDDRef *TmpDDRef = nullptr;
  RegDDRef *FirstRef = MRG.get(0);

  HLInst *LoadInPrehdr = nullptr;

  bool IsLoadOnly = MRG.isLoadOnly();
  // Debug: Examine the Loop BEFORE transformation
  // LLVM_DEBUG(Lp->dump(););

  // *** Prepare LMM for the MRG ***

  // Need a Store in postexit: if there is at least 1 store in MRG
  NeedStoreInPostexit = !IsLoadOnly;

  // Need a Load in prehdr: check algorithm for details
  NeedLoadInPrehdr = isLoadNeededInPrehder(Lp, MRG);

  LoopOptReportBuilder &LORBuilder =
      Lp->getHLNodeUtils().getHIRFramework().getLORBuilder();

  HLLoop *OuterLp = getOuterLoopCandidateForSingleLoad(Lp, FirstRef, MRG);

  // Hoist a Load or a Store without replacing a temp
  if (hoistedSingleLoad(Lp, FirstRef, MRG, TempRefSet, LORBuilder) ||
      sinkedSingleStore(Lp, FirstRef, MRG, TempRefSet, LORBuilder)) {
    return;
  }

  // ### Promote LIMM for the MRG ###

  // Create a Load in prehdr if needed
  if (NeedLoadInPrehdr) {
    LORBuilder(*Lp).addRemark(OptReportVerbosity::Low,
                              "Load hoisted out of the loop");

    LoadInPrehdr = createLoadInPreheader(Lp, FirstRef, OuterLp);

    TmpDDRef = LoadInPrehdr->getLvalDDRef();
  }

  // Create a TempDDRef if needed
  if (!TmpDDRef) {
    TmpDDRef = HNU.createTemp(FirstRef->getDestType(), LIMMTempName);
  }

  TempRefSet.insert(TmpDDRef->getSymbase());

  // Create a Store in postexit if needed
  if (NeedStoreInPostexit) {
    LORBuilder(*Lp).addRemark(OptReportVerbosity::Low,
                              "Store sinked out of the loop");
    RegDDRef *FirstStore = FirstRef;

    // In the multi-exit loop, we need to find the exact store ref to compare
    // the top sort number with gotos.
    if (Lp->getNumExits() > 1) {
      for (unsigned Idx = 0, IdxE = MRG.getSize(); Idx < IdxE; ++Idx) {
        if (MRG.get(Idx)->isLval()) {
          FirstStore = MRG.get(Idx);
          break;
        }
      }
    }

    createStoreInPostexit(Lp, FirstStore, TmpDDRef, NeedLoadInPrehdr);
  }

  // LMM process each Ref in MRG
  for (unsigned Idx = 0, IdxE = MRG.getSize(); Idx < IdxE; ++Idx) {
    handleInLoopMemRef(Lp, MRG.get(Idx), TmpDDRef, IsLoadOnly);
  }

  // Debug: Examine the Loop AFTER transformation
  // LLVM_DEBUG(Lp->dump(););
}

// Call setDefinedAtLevel(LoopLevel -1) to setLinear on a given RegDDRef*
void HIRLMM::setLinear(DDRef *TmpRef) {
  TmpRef->getSingleCanonExpr()->setDefinedAtLevel(LoopLevel - 1);

  if (auto BlobRef = dyn_cast<BlobDDRef>(TmpRef)) {
    BlobRef->getParentDDRef()->updateDefLevel(LoopLevel);
  } else {
    assert(cast<RegDDRef>(TmpRef)->isTerminalRef() &&
           "Expecting a terminal ref");
  }
}

// Handle each loop-inv MemRef inside a group
//
// Special case: create a CopyInst and do proper replacement
// - LoadInst
// - StoreInst
//
// All Others: do regular replacement
//
void HIRLMM::handleInLoopMemRef(HLLoop *Lp, RegDDRef *Ref, RegDDRef *TmpRef,
                                bool IsLoadOnly) {
  // Debug: Examine the Loop Before processing
  // LLVM_DEBUG(Lp->dump(););
  HLDDNode *DDNode = Ref->getHLDDNode();
  RegDDRef *TmpRefClone = TmpRef->clone();

  if (IsLoadOnly) {
    setLinear(TmpRefClone);
  }

  HLInst *HInst = dyn_cast<HLInst>(DDNode);
  // Handle HLInst* special cases: LoadInst and StoreInst
  if (HInst) {
    const Instruction *LLVMInst = HInst->getLLVMInstruction();
    HLInst *CopyInst = nullptr;
    RegDDRef *OtherRef = nullptr;

    // StoreInst: replace with a LoadInst or CopyInst depending on the rval.
    if (isa<StoreInst>(LLVMInst) && Ref->isLval()) {
      OtherRef = DDNode->removeOperandDDRef(1);
      if (OtherRef->isMemRef()) {
        auto LInst = HNU.createLoad(OtherRef, LIMMCopyName, TmpRefClone);
        HLNodeUtils::replace(DDNode, LInst);
      } else {
        CopyInst = HNU.createCopyInst(OtherRef, LIMMCopyName, TmpRefClone);
        HLNodeUtils::replace(DDNode, CopyInst);
      }
    }
    // LoadInst: replace with a CopyInst
    else if (isa<LoadInst>(LLVMInst)) {
      OtherRef = DDNode->removeOperandDDRef(0);
      CopyInst = HNU.createCopyInst(TmpRefClone, LIMMCopyName, OtherRef);
      HLNodeUtils::replace(DDNode, CopyInst);
    }
    // Neither a Load nor a Store in HLInst*: do regular replacement
    else {
      DDNode->replaceOperandDDRef(Ref, TmpRefClone);
    }

  }
  // All other cases: do regular replacement
  else {
    DDNode->replaceOperandDDRef(Ref, TmpRefClone);
  }

  // Debug: Examine the Loop again, notice the LIMM promotion(s) for
  // load, store, and binaryOp types
  // LLVM_DEBUG(Lp->dump(););
}

static void insertInPreheader(HLLoop *Lp, HLInst *LoadInPrehdr,
                              RegDDRef *RvalRef) {
  // Insert the new Load as a last node into Loop's Prehdr
  HLNodeUtils::insertAsLastPreheaderNode(Lp, LoadInPrehdr);

  // Call updateDefLevel() for the newly-created load
  RvalRef->updateDefLevel(Lp->getNestingLevel() - 1);
  return;
}

// Check if a LoadInst (E.g. t0 = A[0]) exists in Preheader
// . YES: obtain the HLInst*
// . NO:  create a new LoadInst in preheader
//
// Return: The LoadToTemp HLInst*
//
// Additional Work:
// - Mark the new Temp as live-in to loop
// - Mark the new Temp as linear, defined at level = looplevel -1
// - Call updateDefLevel() for the Rval of the new load
//
HLInst *HIRLMM::createLoadInPreheader(HLLoop *InnermostLp, RegDDRef *Ref,
                                      HLLoop *OuterLp) const{
  // Debug: Examine the Loop
  // LLVM_DEBUG(InnermostLp->dump(););

  auto RvalRef = Ref->clone();
  HLInst *LoadInPrehdr = HNU.createLoad(RvalRef, LIMMTempName);

  RegDDRef *TmpRef = LoadInPrehdr->getLvalDDRef();
  unsigned TmpSB = TmpRef->getSymbase();

  insertInPreheader(OuterLp, LoadInPrehdr, RvalRef);

  unsigned OuterLpLevel = OuterLp->getNestingLevel();
  unsigned Level = InnermostLp->getNestingLevel();

  while (Level >= OuterLpLevel) {
    InnermostLp->addLiveInTemp(TmpSB);
    InnermostLp = InnermostLp->getParentLoop();
    Level--;
  }

  // Debug: Examine the Loop, notice the temp in prehdr
  // LLVM_DEBUG(InnermostLp->dump(););

  return LoadInPrehdr;
}

// Creates the following HLIf-
// if (i1 != 0)
static HLIf *createNewIVComparison(HLLoop *Lp) {
  auto &HNU = Lp->getHLNodeUtils();
  auto &DRU = HNU.getDDRefUtils();
  auto &CEU = DRU.getCanonExprUtils();
  auto IVType = Lp->getIVType();
  unsigned LoopLevel = Lp->getNestingLevel();

  CanonExpr *CEIV = CEU.createCanonExpr(IVType);
  CEIV->addIV(LoopLevel, InvalidBlobIndex, 1);

  RegDDRef *LHS = DRU.createScalarRegDDRef(GenericRvalSymbase, CEIV);
  RegDDRef *RHS = DRU.createNullDDRef(LHS->getDestType());

  HLIf *IVCheckIf = HNU.createHLIf(PredicateTy::ICMP_NE, LHS, RHS);

  return IVCheckIf;
}

// Looks for a HLIf of the form 'if (i1 != 0)' before goto.
// Creates one, if not present.
static HLIf *getIVComparisonIf(HLLoop *Lp, HLGoto *Goto) {
  if (auto *PrevIf = dyn_cast_or_null<HLIf>(Goto->getPrevNode())) {
    auto PredI = PrevIf->pred_begin();
    const RegDDRef *LHSRef = PrevIf->getPredicateOperandDDRef(PredI, true);
    const RegDDRef *RHSRef = PrevIf->getPredicateOperandDDRef(PredI, false);
    PredicateTy Pred = *PredI;
    unsigned Level;
    unsigned LoopLevel = Lp->getNestingLevel();

    if (PrevIf->getNumPredicates() == 1 && Pred == CmpInst::ICMP_NE &&
        LHSRef->isStandAloneIV(true, &Level) && (Level == LoopLevel) &&
        RHSRef->isZero()) {
      return PrevIf;
    }
  }

  HLIf *IVCheckIf = createNewIVComparison(Lp);
  HLNodeUtils::insertBefore(Goto, IVCheckIf);
  return IVCheckIf;
}

// Create a StoreInst In Loop's postexit
// (If the Store already exists, obtain the StoreInst.)
//
// Note: Mark the newly created TempDDRef as
// - live-out of loop
// - linear, defined at level = looplevel -1
// - Call updateDefLevel() for the Lval of the new store
//
void HIRLMM::createStoreInPostexit(HLLoop *Lp, RegDDRef *Ref, RegDDRef *TmpRef,
                                   bool NeedLoadInPrehdr) const {
  // Debug: Examine the Loop
  // LLVM_DEBUG(Lp->dump(););

  // If no matching store is available, create one
  RegDDRef *TmpRefClone = TmpRef->clone();
  Lp->addLiveOutTemp(TmpRefClone->getSymbase());

  auto LvalRef = Ref->clone();
  auto *StoreInPostexit = HNU.createStore(TmpRefClone, LIMMTempName, LvalRef);

  if (Lp->getNumExits() > 1) {
    // Collect all the Gotos if it is a multi-exit loop
    SmallVector<HLGoto *, 16> Gotos;
    Lp->populateEarlyExits(Gotos);
    bool TmpIsInitialized = NeedLoadInPrehdr;

    for (auto &Goto : Gotos) {
      auto *StoreInst = StoreInPostexit->clone();

      if (Ref->getHLDDNode()->getTopSortNum() < Goto->getTopSortNum()) {
        HLNodeUtils::insertBefore(Goto, StoreInst);
      } else {
        // Create an inst like %t = 0 and insert it as first prehearder of the
        // loop
        if (!TmpIsInitialized) {
          RegDDRef *LHS = TmpRefClone->clone();
          RegDDRef *RHS =
              HIRF.getDDRefUtils().createNullDDRef(LHS->getDestType());
          auto *InitialTemp = HNU.createCopyInst(RHS, "copy", LHS);
          Lp->addLiveInTemp(LHS->getSymbase());
          HLNodeUtils::insertAsFirstPreheaderNode(Lp, InitialTemp);
          TmpIsInitialized = true;
        }

        HLIf *IVCheckIf = getIVComparisonIf(Lp, Goto);
        HLNodeUtils::insertAsFirstThenChild(IVCheckIf, StoreInst);
      }
    }
  }

  // Insert the new store as the 1st HLInst in Lp's Postexit
  HLNodeUtils::insertAsFirstPostexitNode(Lp, StoreInPostexit);

  // Call updateDefLevel() for the newly-created store
  LvalRef->updateDefLevel(Lp->getNestingLevel() - 1);

  // Debug: Examine the Loop, notice the tmp in postexit
  // LLVM_DEBUG(Lp->dump(););
}

PreservedAnalyses HIRLMMPass::run(llvm::Function &F,
                                  llvm::FunctionAnalysisManager &AM) {
  HIRLMM(AM.getResult<HIRFrameworkAnalysis>(F),
         AM.getResult<HIRDDAnalysisPass>(F),
         AM.getResult<HIRLoopStatisticsAnalysis>(F))
      .run();
  return PreservedAnalyses::all();
}

class HIRLMMLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRLMMLegacyPass() : HIRTransformPass(ID) {
    initializeHIRLMMLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRLMM(getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
                  getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
                  getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS())
        .run();
  }
};

char HIRLMMLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLMMLegacyPass, "hir-lmm", "HIR Loop Memory Motion",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRLMMLegacyPass, "hir-lmm", "HIR Loop Memory Motion",
                    false, false)

FunctionPass *llvm::createHIRLMMPass() { return new HIRLMMLegacyPass(); }
