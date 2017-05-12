//===--- HIRLMM.cpp -Implements Loop Memory Motion Pass -*- C++ -*---===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-----------------------------------------------------------------===//
// HIR LIMM Case1: Load Hosting Only
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
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLoopStatistics.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRLMM.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

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

MemRefGroup::MemRefGroup(RegDDRef *FirstRef, HIRLoopStatistics *HLS)
    : IsProfitable(false), IsLegal(false), IsAnalyzed(false), HasLoad(false),
      HasLoadOnDomPath(false), HasStore(false), HasStoreOnDomPath(false),
      HLS(HLS) {
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

  // Scan each Ref in group, and do statistical collection, for:
  // Load, LoadOnDomPath, Store, StoreOnDomPath
  for (auto &Ref : RefV) {
    bool IsLoad = Ref->isRval();

    // Load
    if (IsLoad) {
      HasLoad = true;

      // Load on DomPath
      if (!HasLoadOnDomPath &&
          HLNodeUtils::dominates(Ref->getHLDDNode(), LoopTail, HLS)) {
        HasLoadOnDomPath = true;
      }
    }
    // Store
    else {
      HasStore = true;

      // Store on DomPath
      if (!HasStoreOnDomPath &&
          HLNodeUtils::dominates(Ref->getHLDDNode(), LoopTail, HLS)) {
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
    MRVV.emplace_back(Ref, HLS);
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

  // DEBUG(MRC.print(););
}

char HIRLMM::ID = 0;

INITIALIZE_PASS_BEGIN(HIRLMM, "hir-lmm", "HIR Loop Memory Motion", false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysis)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatistics)
INITIALIZE_PASS_END(HIRLMM, "hir-lmm", "HIR Loop Memory Motion", false, false)

FunctionPass *llvm::createHIRLMMPass() { return new HIRLMM(); }

HIRLMM::HIRLMM(void) : HIRTransformPass(ID) {
  initializeHIRLMMPass(*PassRegistry::getPassRegistry());
}

void HIRLMM::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequiredTransitive<HIRFramework>();
  AU.addRequiredTransitive<HIRDDAnalysis>();
  AU.addRequiredTransitive<HIRLoopStatistics>();
  AU.setPreservesAll();
}

// The following preliminary conditions are currently checked:
// - Multiple exits
// - Empty loop body
// - LoopStatistics
//   . no call
//
bool HIRLMM::doLoopPreliminaryChecks(const HLLoop *Lp) {
  if (!Lp->isDo()) {
    return false;
  }

  const LoopStatistics &LS = HLS->getSelfLoopStatistics(Lp);
  // DEBUG(LS.dump(););
  if (LS.hasCallsWithUnsafeSideEffects()) {
    return false;
  }

  return true;
}

bool HIRLMM::handleCmdlineArgs(Function &F) {
  if (DisableHIRLMM || skipFunction(F)) {
    DEBUG(dbgs() << "HIRLMM (Loop Memory Motion) Disabled or Skipped\n");
    return false;
  }

  return true;
}

bool HIRLMM::runOnFunction(Function &F) {
  if (!handleCmdlineArgs(F)) {
    return false;
  }

  DEBUG(dbgs() << "HIR LIMM on Function : " << F.getName() << "\n");

  // Gather all inner-most Loop Candidates
  SmallVector<HLLoop *, 64> CandidateLoops;

  auto HIRF = &getAnalysis<HIRFramework>();
  HIRF->getHLNodeUtils().gatherInnermostLoops(CandidateLoops);

  if (CandidateLoops.empty()) {
    DEBUG(dbgs() << F.getName() << "() has no inner-most loop\n ");
    return false;
  }
  // DEBUG(dbgs() << " # Innermost Loops: " << CandidateLoops.size() << "\n");

  HDDA = &getAnalysis<HIRDDAnalysis>();
  HLS = &getAnalysis<HIRLoopStatistics>();
  MRC.HLS = HLS;
  bool Result = false;

  for (auto &Lp : CandidateLoops) {
    Result = doLoopMemoryMotion(Lp, *HDDA, *HLS) || Result;
  }

  CandidateLoops.clear();
  return Result;
}

bool HIRLMM::doLoopMemoryMotion(HLLoop *Lp, HIRDDAnalysis &DDA,
                                HIRLoopStatistics &LS) {
  // Analyze the loop and reject if unsuitable
  if (!doAnalysis(Lp, DDA, LS)) {
    return false;
  }

  // Do Loop Memory Motion (LMM) promotion
  doTransform(Lp);
  return true;
}

// Conduct ALL HIR-LMM Tests to decide whether the loop is good for LMM
bool HIRLMM::doAnalysis(HLLoop *Lp, HIRDDAnalysis &DDA, HIRLoopStatistics &LS) {
  // DEBUG(dbgs() << "Current Loop: \n"; Lp->dump(););
  clearWorkingSetMemory();
  LoopLevel = Lp->getNestingLevel();

  HDDA = &DDA;
  HLS = &LS;
  HNU = &(Lp->getHLNodeUtils());

  if (!doLoopPreliminaryChecks(Lp)) {
    DEBUG(dbgs() << "HIRLMM: failed Loop Preliminary Checks\n";);
    return false;
  }

  if (!doCollection(Lp)) {
    DEBUG(dbgs() << "HIRLMM: failed DoCollection\n");
    return false;
  }

  if (!isProfitable(Lp)) {
    DEBUG(dbgs() << "HIRLMM: failed profit test\n");
    return false;
  }

  if (!isLegal(Lp)) {
    DEBUG(dbgs() << "HIRLMM: failed legal test\n");
    return false;
  }

  return true;
}

// Do a loop collection
// (After collection, data is in MRC)
bool HIRLMM::doCollection(HLLoop *Lp) {
  // Collect all loop-inv MemRefs within the loop's body
  CollectMemRefs Collector(MRC, LoopLevel);
  HNU->visitRange(Collector, Lp->getFirstChild(), Lp->getLastChild());

  // Examine the collection result
  // DEBUG(MRC.print(););

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
  DDGraph DDG = HDDA->getGraph(Lp, false);

  // Do legal test on any profitable MRG
  for (unsigned Idx = 0, IdxE = MRC.getSize(); Idx != IdxE; ++Idx) {
    MemRefGroup &MRG = MRC.get(Idx);

    if (MRG.getProfitable() && isLegal(Lp, MRG, DDG)) {
      MRG.setLegal(true);
      Result = true;
    }
  }

  // Check the MRC after legality analysis
  // DEBUG(MRC.print(););

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
    DEBUG(Edge->print(dbgs()););

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

  // Iterate over each group in MRC, do LMM promotion if suitable.
  for (unsigned Idx = 0, IdxE = MRC.getSize(); Idx < IdxE; ++Idx) {
    MemRefGroup &MRG = MRC.get(Idx);

    // Skip MRG if it is either illegal or non-profitable
    if (!MRG.getProfitable() || !MRG.getLegal()) {
      continue;
    }

    // DEBUG(MRG.print(true); dbgs() << "Before LIMM on a MRG\n";
    // Lp->dump(););
    doLIMMRef(Lp, MRG);
    ++NumLIMM;
    // DEBUG(dbgs() << "After LIMM on a MRG\n"; Lp->dump(););
  }
  HIRLIMMRefPromoted += NumLIMM;

  // Mark the loop and its parent loop/region have been changed
  Lp->getParentRegion()->setGenCode();
  HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Lp);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(Lp);
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
    if (HLNodeUtils::dominates(CurRef->getHLDDNode(), LoopTail, HLS)) {
      return false;
    }
  }

  llvm_unreachable("Not expect control to reach here\n");
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
void HIRLMM::doLIMMRef(HLLoop *Lp, MemRefGroup &MRG) {
  bool NeedLoadInPrehdr = false, NeedStoreInPostexit = false;
  RegDDRef *TmpDDRef = nullptr;
  RegDDRef *FirstRef = MRG.get(0);
  HLInst *LoadInPrehdr = nullptr;
  bool IsLoadOnly = MRG.isLoadOnly();

  // Debug: Examine the Loop BEFORE transformation
  // DEBUG(Lp->dump(););

  // *** Prepare LMM for the MRG ***

  // Need a Store in postexit: if there is at least 1 store in MRG
  NeedStoreInPostexit = !IsLoadOnly;

  // Need a Load in prehdr: check algorithm for details
  NeedLoadInPrehdr = isLoadNeededInPrehder(Lp, MRG);

  // ### Promote LIMM for the MRG ###

  // Create a Load in prehdr if needed
  if (NeedLoadInPrehdr) {
    LoadInPrehdr = findOrCreateLoadInPreheader(Lp, FirstRef);
    TmpDDRef = LoadInPrehdr->getLvalDDRef();
  }

  // Create a TempDDRef if needed
  if (!TmpDDRef) {
    TmpDDRef = HNU->createTemp(FirstRef->getDestType(), LIMMTempName);
  }

  // Create a Store in postexit if needed
  if (NeedStoreInPostexit) {
    findOrCreateStoreInPostexit(Lp, FirstRef, TmpDDRef);
  }

  // LMM process each Ref in MRG
  for (unsigned Idx = 0, IdxE = MRG.getSize(); Idx < IdxE; ++Idx) {
    handleInLoopMemRef(Lp, MRG.get(Idx), TmpDDRef, IsLoadOnly);
  }

  // Debug: Examine the Loop AFTER transformation
  // DEBUG(Lp->dump(););
}

// Call setDefinedAtLevel(LoopLevel -1) to setLinear on a given RegDDRef*
void HIRLMM::setLinear(RegDDRef *TmpRef) {
  assert(TmpRef->isSingleCanonExpr() && "Expect SingleCE to be true\n");

  CanonExpr *CE = TmpRef->getSingleCanonExpr();
  CE->setDefinedAtLevel(LoopLevel - 1);
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
  // DEBUG(Lp->dump(););

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

    // StoreInst: replace with a CopyInst
    if (isa<StoreInst>(LLVMInst) && Ref->isLval()) {
      OtherRef = DDNode->removeOperandDDRef(1);
      CopyInst = HNU->createCopyInst(OtherRef, LIMMCopyName, TmpRefClone);
      HNU->replace(DDNode, CopyInst);
    }
    // LoadInst: replace with a CopyInst
    else if (isa<LoadInst>(LLVMInst) && Ref->isRval()) {
      OtherRef = DDNode->removeOperandDDRef(0);
      CopyInst = HNU->createCopyInst(TmpRefClone, LIMMCopyName, OtherRef);
      HNU->replace(DDNode, CopyInst);
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
  // DEBUG(Lp->dump(););
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
HLInst *HIRLMM::findOrCreateLoadInPreheader(HLLoop *Lp, RegDDRef *Ref) const {
  HLInst *LoadInPrehdr = nullptr;
  RegDDRef *TmpRef = nullptr;

  // Debug: Examine the Loop
  // DEBUG(Lp->dump(););

  // Check if a LoadInst (E.g. t0 = A[0]) exists in Preheader
  LoadInPrehdr = getLoadInLoopPreheader(Lp, Ref);

  // If the LoadInst doesn't exit in prehdr, create it
  if (!LoadInPrehdr) {
    auto RvalRef = Ref->clone();
    LoadInPrehdr = HNU->createLoad(RvalRef, LIMMTempName);

    // Insert the new Load as a last node into Loop's Prehdr
    HNU->insertAsLastPreheaderNode(Lp, LoadInPrehdr);

    // Mark as Loop's LiveIn Temp
    TmpRef = LoadInPrehdr->getLvalDDRef();
    Lp->addLiveInTemp(TmpRef->getSymbase());

    // Call updateDefLevel() for the newly-created load
    RvalRef->updateDefLevel(Lp->getNestingLevel() - 1);
  }
  assert(LoadInPrehdr && "LoadInPrehdr can't be null\n");

  // Debug: Examine the Loop, notice the temp in prehdr
  // DEBUG(Lp->dump(););

  return LoadInPrehdr;
}

// Create a StoreInst In Loop's postexit
// (If the Store already exists, obtain the StoreInst.)
//
// Note: Mark the newly created TempDDRef as
// - live-out of loop
// - linear, defined at level = looplevel -1
// - Call updateDefLevel() for the Lval of the new store
//
void HIRLMM::findOrCreateStoreInPostexit(HLLoop *Lp, RegDDRef *Ref,
                                         RegDDRef *TmpRef) const {
  // HLInst *StoreInPostexit = nullptr;
  // Debug: Examine the Loop
  // DEBUG(Lp->dump(););

  // Check if a suitable store exists in postexit
  HLInst *StoreInPostexit = getStoreInLoopPostexit(Lp, Ref);

  // If no matching store is available, create one
  if (!StoreInPostexit) {
    RegDDRef *TmpRefClone = TmpRef->clone();
    Lp->addLiveOutTemp(TmpRefClone->getSymbase());

    auto LvalRef = Ref->clone();
    StoreInPostexit = HNU->createStore(TmpRefClone, LIMMTempName, LvalRef);

    // Insert the new store as the 1st HLInst in Lp's Postexit
    HNU->insertAsFirstPostexitNode(Lp, StoreInPostexit);

    // Call updateDefLevel() for the newly-created store
    LvalRef->updateDefLevel(Lp->getNestingLevel() - 1);
  }

  // Debug: Examine the Loop, notice the tmp in postexit
  // DEBUG(Lp->dump(););

  assert(StoreInPostexit && "StoreInPostexit can't be null\n");
}

// Search a Loop's preheader for a LoadInst matching a given MemRef
HLInst *HIRLMM::getLoadInLoopPreheader(HLLoop *Lp, RegDDRef *MemRef) const {

  for (auto It = Lp->pre_begin(), ItE = Lp->pre_end(); It != ItE; ++It) {
    HLInst *HInst = cast<HLInst>(It);

    // Examine the HLInst*
    // DEBUG(HInst->dump(););

    // Only interested in LoadInst
    const Instruction *LLVMInst = HInst->getLLVMInstruction();
    if (!isa<LoadInst>(LLVMInst)) {
      continue;
    }

    // Check:
    // -Lval() is a TempDDRef
    // -RVal() matches the input MemRef
    if (HInst->getLvalDDRef()->isTerminalRef() &&
        DDRefUtils::areEqual(MemRef, HInst->getRvalDDRef())) {
      return HInst;
    }
  }

  return nullptr;
}

// Search a Loop's postexit for a StoreInst matching a given MemRef
HLInst *HIRLMM::getStoreInLoopPostexit(HLLoop *Lp, RegDDRef *MemRef) const {

  for (auto It = Lp->post_begin(), ItE = Lp->post_end(); It != ItE; ++It) {
    HLInst *HInst = cast<HLInst>(It);

    // Examine the HLInst*
    // DEBUG(HInst->dump(););

    // Only interested in StoreInst
    const Instruction *LLVMInst = HInst->getLLVMInstruction();
    if (!isa<StoreInst>(LLVMInst)) {
      continue;
    }

    // Check:
    // -RVal() is a TempDDRef
    // -Lval() is a MemRef matching the input Ref
    if ((HInst->getRvalDDRef()->isTerminalRef()) &&
        DDRefUtils::areEqual(MemRef, HInst->getLvalDDRef())) {
      return HInst;
    }
  }

  return nullptr;
}
