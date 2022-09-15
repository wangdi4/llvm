//===--- HIRLMM.cpp -Implements Loop Memory Motion Pass -*- C++ -*---===//
//
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
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
#include "llvm/Transforms/Intel_LoopTransforms/HIRLMMPass.h"

#if INTEL_FEATURE_SW_DTRANS
#include "Intel_DTrans/Analysis/DTransFieldModRef.h"
#include "Intel_DTrans/DTransCommon.h"
#endif // INTEL_FEATURE_SW_DTRANS

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Passes.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#include "HIRLMM.h"

#define DEBUG_TYPE "hir-lmm"

using namespace llvm;
using namespace llvm::loopopt;
using namespace llvm::loopopt::lmm;

// Disable the HIR Loop-Invariant Memory Motion (default is false)
static cl::opt<bool>
    DisableHIRLMM("disable-hir-lmm", cl::init(false), cl::Hidden,
                  cl::desc("Disable HIR Loop Memory Motion (LMM)"));

static cl::opt<bool> ForceLoopNestHoisting(
    "hir-lmm-loopnest-hoisting", cl::init(false), cl::Hidden,
    cl::desc("Performs memory motion at the loopnest level"));

STATISTIC(
    HIRLIMMRefPromoted,
    "Number of HIR loop-invariant memory load(s)/store(s) References Promoted");

MemRefGroup::MemRefGroup(RegDDRef *FirstRef)
    : IsProfitable(false), IsLegal(false), IsAnalyzed(false), HasLoad(false),
      HasLoadOnDomPath(false), HasStore(false), HasStoreOnDomPath(false),
      IsInsideLifetimeIntrinsics(false) {
  RefVec.push_back(FirstRef);
}

bool MemRefGroup::belongs(RegDDRef *Ref) const {
  return DDRefUtils::areEqual(Ref, RefVec[0]);
}

static bool foundRegionDominatingLoadOrStore(DominatorTree &DT,
                                             const RegDDRef *LoadRef,
                                             const HLRegion *Reg) {

  bool IsPrecise;
  auto *RefGepInst =
      dyn_cast<GetElementPtrInst>(LoadRef->getLocationPtr(IsPrecise));

  if (!IsPrecise || !RefGepInst) {
    return false;
  }

  auto *RegEntryBB = Reg->getEntryBBlock();

  auto *DomNode = DT.getNode(RegEntryBB);
  assert(DomNode && "DomNode of region entry block is null!");

  // Traverse dominating nodes of region entry block to find an identical GEP.
  bool FoundDominatingLoadOrStore = false;
  while ((DomNode = DomNode->getIDom())) {
    auto *DomBlock = DomNode->getBlock();

    for (auto &Inst : *DomBlock) {
      auto *DomGepInst = dyn_cast<GetElementPtrInst>(&Inst);

      if (!DomGepInst) {
        continue;
      }

      if (DomGepInst->getNumOperands() != RefGepInst->getNumOperands()) {
        continue;
      }

      if (DomGepInst->getPointerOperand() != RefGepInst->getPointerOperand()) {
        continue;
      }

      if (!std::equal(DomGepInst->idx_begin(), DomGepInst->idx_end(),
                      RefGepInst->idx_begin(), RefGepInst->idx_end())) {
        continue;
      }

      // Try to find a load or store user of the GEP which dominates region
      // entry.
      for (auto *GepUser :
           make_range(DomGepInst->user_begin(), DomGepInst->user_end())) {

        if (!isa<LoadInst>(GepUser) && !isa<StoreInst>(GepUser)) {
          continue;
        }

        auto *UserInst = cast<Instruction>(GepUser);
        if (!DT.dominates(UserInst, RegEntryBB)) {
          continue;
        }

        FoundDominatingLoadOrStore = true;
        break;
      }

      break;
    }

    if (FoundDominatingLoadOrStore) {
      break;
    }
  }

  return FoundDominatingLoadOrStore;
}

/* =========================================================== */

void MemRefGroup::analyze(const HLLoop *Lp, DominatorTree *DT,
                          bool LoopNestHoistingOnly) {
  // Only analyze once
  assert(!IsAnalyzed && "Analyze ONCE only\n");

  const HLNode *LoopTail = Lp->getLastChild();
  const HLNode *LoopHead = Lp->getFirstChild();

  bool IsInnermost = Lp->isInnermost();
  bool AllRefsInSameLoop = true;

  const HLLoop *PrevLoop = nullptr;
  // Scan each Ref in group, and do statistical collection, for:
  // Load, LoadOnDomPath, Store, StoreOnDomPath
  for (auto &Ref : RefVec) {
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

    if (!IsInnermost && AllRefsInSameLoop) {
      auto *CurLoop = Ref->getLexicalParentLoop();
      if (PrevLoop && PrevLoop != CurLoop) {
        AllRefsInSameLoop = false;
      } else {
        PrevLoop = CurLoop;
      }
    }
  }

  IsAnalyzed = true;

  bool IsLoadOnly = isLoadOnly();

  if (!LoopNestHoistingOnly) {
    IsProfitable = hasAnyLoadOrStoreOnDominatePath();

  } else if (IsLoadOnly && AllRefsInSameLoop) {
    // We only allow loads with the same parent loop in loopnest hoisting mode.
    // This is to keep the transformation simple but the limination can be
    // removed.

    assert(Lp->getNestingLevel() == 1 &&
           "Loop level 1 expected in loopnest hoisting mode!");

    IsProfitable = hasAnyLoadOnDominatePath();

    // Technically, this check only proves safety of hoisting. Profitability is
    // harder to prove unless we check aliasing with any intervening stores.
    if (!IsProfitable && DT &&
        foundRegionDominatingLoadOrStore(*DT, RefVec[0],
                                         Lp->getParentRegion())) {
      IsProfitable = true;
    }
  }
}

#ifndef NDEBUG
LLVM_DUMP_METHOD void MemRefGroup::print(bool NewLine) {
  formatted_raw_ostream FOS(dbgs());

  // Print 1st item (since they all look the same)
  RefVec[0]->print(FOS);
  FOS << " { ";

  // For each ref, print 'W' for a write, and 'R' for a read
  unsigned NumLoad = 0, NumStore = 0;
  for (auto &Ref : RefVec) {
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
  for (unsigned Idx = 0, IdxE = MemRefGroups.size(); Idx < IdxE; ++Idx) {
    auto &Group = MemRefGroups[Idx];

    // Check if Ref belongs to Group
    if (Group.belongs(Ref)) {
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
  // -no:  create a new Group with the ref, and push the Group into MemRefGroups
  if (find(Ref, Idx)) {
    MemRefGroups[Idx].insert(Ref);
  } else {
    MemRefGroups.emplace_back(Ref);
  }
}

#ifndef NDEBUG
LLVM_DUMP_METHOD void MemRefCollection::print(void) {
  formatted_raw_ostream FOS(dbgs());
  FOS << "MemRefCollection, entries: " << MemRefGroups.size() << "\n";

  // Sanity check: any available Group in collection?
  if (MemRefGroups.size() == 0) {
    FOS << " __EMPTY__ \n";
    return;
  }

  for (auto &Group : MemRefGroups) {
    FOS << "  ";
    Group.print(true);
  }
}
#endif

// Collects all info relevant to memory motion like loop invariant memrefs and
// calls with unknown aliasing.
class MemAccessCollector final : public HLNodeVisitorBase {
private:
  MemRefCollection &MRC;
  SmallVectorImpl<HLInst *> &UnknownAliasingCallInsts;
  unsigned LoopLevel;
  bool SkipMemRefs;
  bool IsDone;

public:
  MemAccessCollector(MemRefCollection &InitMRC,
                     SmallVectorImpl<HLInst *> &UnknownAliasingCallInsts,
                     unsigned InitLevel, RegDDRef *CandidateMemRef,
                     bool IgnoreIVs)
      : MRC(InitMRC), UnknownAliasingCallInsts(UnknownAliasingCallInsts),
        LoopLevel(InitLevel), SkipMemRefs(CandidateMemRef), IsDone(false) {
    assert(CanonExpr::isValidLoopLevel(InitLevel) &&
           "LoopLevel is out of bound\n");

    if (CandidateMemRef && !collectMemRef(CandidateMemRef, IgnoreIVs)) {
      IsDone = true;
    }
  }

  void visit(HLDDNode *Node) {
    // Scan each HLDDNode from Right to Left, so that any potential load
    // appears earlier than store on the same HLDDNode*.
    if (!SkipMemRefs) {
      for (auto *Ref :
           make_range(Node->op_ddref_rbegin(), Node->op_ddref_rend())) {
        collectMemRef(Ref);
      }
    }

    if (auto *Inst = dyn_cast<HLInst>(Node)) {
      if (Inst->isUnknownAliasingCallInst()) {
        UnknownAliasingCallInsts.push_back(Inst);
      }
    }
  }

  // No processing needed for Goto, Label and HLNode types
  void visit(HLGoto *Goto){};
  void visit(HLLabel *Label){};
  void visit(HLNode *Node) {
    llvm_unreachable(" visit(HLNode *) - Node not supported\n");
  }
  void postVisit(HLNode *Node) {}

  bool collectMemRef(RegDDRef *Ref, bool IgnoreIVs = false);

  bool isDone() const { return IsDone; }
};

// Collects \p Ref as candidate if it passes structural checks.
bool MemAccessCollector::collectMemRef(RegDDRef *Ref, bool IgnoreIVs) {
  if (!Ref->isMemRef()) {
    return false;
  }

  if (!IgnoreIVs) {
    if (!Ref->isStructurallyInvariantAtLevel(LoopLevel)) {
      return false;
    }
  } else {
    // Allow base pointer to be structurally variant for single dimension refs.
    // Additional legality checks will be done using DD. Give up if some other
    // blob is non-linear.

    unsigned BaseIndex = Ref->getBasePtrBlobIndex();
    bool IsSingleDim = Ref->isSingleDimension();

    for (auto *BRef : make_range(Ref->blob_begin(), Ref->blob_end())) {
      if (!BRef->isLinearAtLevel(LoopLevel) &&
          (!IsSingleDim || (BRef->getBlobIndex() != BaseIndex))) {
        return false;
      }
    }
  }

  MRC.insert(Ref);
  return true;
}

bool HIRLMM::doLoopPreliminaryChecks(const HLLoop *Lp,
                                     bool AllowUnknownAliasingCalls) {

  if (Lp->hasDistributePoint()) {
    return false;
  }
  const LoopStatistics &LS = HLS.getTotalLoopStatistics(Lp);
  if (!AllowUnknownAliasingCalls && LS.hasCallsWithUnknownAliasing()) {
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

  SmallVector<HLLoop *, 64> CandidateLoops;

  if (LoopNestHoistingOnly) {
    HNU.gatherOutermostLoops(CandidateLoops);
  } else {
    HNU.gatherInnermostLoops(CandidateLoops);
  }

  if (CandidateLoops.empty()) {
    LLVM_DEBUG(dbgs() << HIRF.getFunction().getName()
                      << "() has no inner-most loop\n ");
    return false;
  }

  bool Result = false;

  for (auto &Lp : CandidateLoops) {
    Result = doLoopMemoryMotion(Lp) || Result;
  }

  return Result;
}

bool HIRLMM::doLoopMemoryMotion(HLLoop *Lp) {
  if (!doAnalysis(Lp)) {
    return false;
  }

  doTransform(Lp);

  return true;
}

// Conduct ALL HIR-LMM Tests to decide whether the loop is good for LMM
bool HIRLMM::doAnalysis(HLLoop *Lp) {
  clearWorkingSetMemory();
  LoopLevel = Lp->getNestingLevel();

  if (!doLoopPreliminaryChecks(Lp, LoopNestHoistingOnly)) {
    LLVM_DEBUG(dbgs() << "HIRLMM: failed Loop Preliminary Checks\n";);
    return false;
  }

  if (!doCollection(Lp)) {
    LLVM_DEBUG(dbgs() << "HIRLMM: failed DoCollection\n");
    return false;
  }

  if (!processLegalityAndProfitability(Lp)) {
    LLVM_DEBUG(dbgs() << "HIRLMM: failed legal test\n");
    return false;
  }

  return true;
}

bool HIRLMM::isLoopInvariant(const RegDDRef *MemRef, const HLLoop *Lp,
                             bool IgnoreIVs) {
  clearWorkingSetMemory();
  LoopLevel = Lp->getNestingLevel();

#if INTEL_FEATURE_SW_DTRANS
  if (!doLoopPreliminaryChecks(Lp, FieldModRef != nullptr)) {
#else  // INTEL_FEATURE_SW_DTRANS
  if (!doLoopPreliminaryChecks(Lp, false)) {
#endif // INTEL_FEATURE_SW_DTRANS
    LLVM_DEBUG(dbgs() << "HIRLMM: failed Loop Preliminary Checks\n";);
    return false;
  }

  // We need const_cast because Ref is stored as non-const in MemRefGroup.
  auto *NonConstLp = const_cast<HLLoop *>(Lp);
  auto *NonConstRef = const_cast<RegDDRef *>(MemRef);

  if (!doCollection(NonConstLp, NonConstRef, IgnoreIVs)) {
    LLVM_DEBUG(dbgs() << "HIRLMM: failed DoCollection\n");
    return false;
  }

  assert(MRC.size() == 1 && "Single candidate group expected in query mode!");

  return isLegal(Lp, *MRC.begin(), true);
}

bool HIRLMM::doCollection(HLLoop *Lp, RegDDRef *CandidateMemRef,
                          bool IgnoreIVs) {
  MemAccessCollector Collector(MRC, UnknownAliasingCallInsts, LoopLevel,
                               CandidateMemRef, IgnoreIVs);
  HLNodeUtils::visitRange(Collector, Lp->child_begin(), Lp->child_end());

  // Analyze each Group inside MRC
  MRC.analyze(Lp, DT, LoopNestHoistingOnly);

  // Check if there is at least 1 Group available after collection
  return !MRC.empty();
}

// A Loop is legal if there is it has at least 1 legal group
bool HIRLMM::processLegalityAndProfitability(const HLLoop *Lp) {
  bool Result = false;
  // Do legal test on any profitable Group
  for (MemRefGroup &Group : MRC) {

    bool IsInsideLifetimeIntrinsics = false;
    if (Group.isProfitable() &&
        isLegal(Lp, Group, false, &IsInsideLifetimeIntrinsics)) {
      Group.setLegal(true);

      if (IsInsideLifetimeIntrinsics) {
        Group.setInsideLifetimeIntrinsics();
      }

      Result = true;
    }
  }

  // Check the MRC after legality analysis
  // LLVM_DEBUG(MRC.print(););

  return Result;
}

static bool areDDEdgesLegal(const RegDDRef *MemRef, const DDGraph &DDG,
                            unsigned LoopLevel,
                            ArrayRef<HLInst *> UnknownAliasingCallInsts,
                            const MemRefGroup &Group,
                            bool *IsInsideLifetimeIntrinsics = nullptr) {
  bool IsLoad = MemRef->isRval();
  const RegDDRef *OtherMemRef = nullptr;

  assert(!DDG.empty() && "Empty DDG not expected!");

  // Iterate over each relevant DDEdge
  // Load: incoming-edge iterators
  // Store: outgoing-edge iterators
  for (const DDEdge *Edge :
       (IsLoad ? DDG.incoming(MemRef) : DDG.outgoing(MemRef))) {
    LLVM_DEBUG(Edge->print(dbgs()););

    // Setup OtherMemRef
    if (IsLoad) {
      OtherMemRef = cast<RegDDRef>(Edge->getSrc());
    } else {
      OtherMemRef = cast<RegDDRef>(Edge->getSink());
    }

    auto *OtherInst = dyn_cast<HLInst>(OtherMemRef->getHLDDNode());
    if (OtherInst) {
      bool IgnoreEdge = false;
      // Ignore edges to unknown aliasing calls. They will be handled using
      // mod-ref later.
      for (auto *UnknownCall : UnknownAliasingCallInsts) {
        if (UnknownCall == OtherInst) {
          IgnoreEdge = true;
          break;
        }
      }

      Intrinsic::ID Id;
      if (!IgnoreEdge && OtherInst->isIntrinCall(Id) &&
          (Id == Intrinsic::lifetime_start || Id == Intrinsic::lifetime_end)) {

        bool EqualBase = CanonExprUtils::areEqual(MemRef->getBaseCE(),
                                                  OtherMemRef->getBaseCE());
        if (!EqualBase) {
          // We can ignore lifetime start/end intrinsics if they reference some
          // other base pointer. There shouldn't be aliasing issue with these
          // intrinsics.
          IgnoreEdge = true;
        } else if (IsInsideLifetimeIntrinsics) {
          IgnoreEdge = true;
          *IsInsideLifetimeIntrinsics = true;
        }
      }

      if (IgnoreEdge) {
        continue;
      }
    }

    if (!is_contained(Group, OtherMemRef)) {
      // Test: DV has any < or > before the loop level
      const DirectionVector &DV = Edge->getDV();
      if (!DV.isIndepFromLevel(LoopLevel)) {
        return false;
      }
    }
  }

  return true;
}

// A Group is legal IF&F every Ref is legal within the Group
bool HIRLMM::isLegal(const HLLoop *Lp, const MemRefGroup &Group, bool QueryMode,
                     bool *IsInsideLifetimeIntrinsics) {

  if (DDG.empty()) {
    DDG = HDDA.getGraph(Lp);
  }

  auto *FirstRef = Group[0];
  const RegDDRef *BasePtrLoadRef = nullptr;

  if (!FirstRef->isLinearAtLevel(LoopLevel)) {
    assert(QueryMode && "memref is not structually invariant w.r.t loop!");
    (void)QueryMode;

    if (!(BasePtrLoadRef = DDUtils::getSingleBasePtrLoadRef(DDG, FirstRef))) {
      return false;
    }

    // Only allow a structure field load with precise location.
    // This is because only in this case are the results returned by FieldModRef
    // applicable to both Ref and BasePtrLoadRef.
    // TODO: Find a cleaner way by querying mod-ref twice.
    if (!BasePtrLoadRef->hasTrailingStructOffsets(1)) {
      return false;
    }

    bool LocIsPrecise;
    BasePtrLoadRef->getLocationPtr(LocIsPrecise);

    if (!LocIsPrecise) {
      return false;
    }

    if (!areDDEdgesLegal(BasePtrLoadRef, DDG, LoopLevel,
                         UnknownAliasingCallInsts, Group)) {
      return false;
    }
  }

  for (const RegDDRef *Ref : Group) {
    if (!areDDEdgesLegal(Ref, DDG, LoopLevel, UnknownAliasingCallInsts, Group,
                         QueryMode ? nullptr : IsInsideLifetimeIntrinsics)) {
      return false;
    }
  }

#if INTEL_FEATURE_SW_DTRANS
  if (!UnknownAliasingCallInsts.empty()) {
    // Bail out if analysis is not available.
    if (!FieldModRef) {
      return false;
    }

    // Use BasePtrLoadRef for mod-ref queries as it returns result for both
    // itself and its dereference(FirstRef).
    auto MemLoc = BasePtrLoadRef ? BasePtrLoadRef->getMemoryLocation()
                                 : FirstRef->getMemoryLocation();

    for (auto *HInst : UnknownAliasingCallInsts) {
      auto *Call = cast<CallInst>(HInst->getLLVMInstruction());

      if (isModSet(FieldModRef->getModRefInfo(Call, MemLoc))) {
        return false;
      }
    }
  }
#endif // INTEL_FEATURE_SW_DTRANS

  return true;
}

// LMM transform always succeeds because the given loop is guaranteed to have
// at least 1 suitable (both profitable and legal) group.
void HIRLMM::doTransform(HLLoop *Lp) {
  unsigned NumLIMM = 0;
  SmallSet<unsigned, 32> TempRefSet;

  // Iterate over each group in MRC, do LMM promotion if suitable.
  for (MemRefGroup &Group : MRC) {

    // Skip Group if it is either illegal or non-profitable
    if (!Group.isLegal()) {
      continue;
    }

    assert(Group.isProfitable() && "Legal group should be profitable!");

    // LLVM_DEBUG(Group.print(true); dbgs() << "Before LIMM on a Group\n";
    //           Lp->dump(););
    doLIMMRef(Lp, Group, TempRefSet);
    ++NumLIMM;
    // LLVM_DEBUG(dbgs() << "After LIMM on a Group\n"; Lp->dump(););
  }

  HIRLIMMRefPromoted += NumLIMM;

  // Mark the loop and its parent loop/region have been changed
  Lp->getParentRegion()->setGenCode();
  HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(Lp);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(Lp);

  HLNodeUtils::removeEmptyNodes(Lp);
}

HLInst *HIRLMM::canHoistLoadsUsingExistingTemp(
    HLLoop *Lp, MemRefGroup &Group, SmallSet<unsigned, 32> &TempRefSet) const {

  // Handle load only group as long as only one instruction is a load
  // instruction with a hoistable temp ref.
  if (!Group.isLoadOnly()) {
    return nullptr;
  }

  HLInst *SingleLoadInst = nullptr;

  for (auto *Ref : Group) {

    HLInst *LoadHInst = dyn_cast<HLInst>(Ref->getHLDDNode());

    if (!LoadHInst) {
      continue;
    }

    if (!isa<LoadInst>(LoadHInst->getLLVMInstruction())) {
      continue;
    }

    if (SingleLoadInst) {
      return nullptr;
    }

    SingleLoadInst = LoadHInst;

    RegDDRef *LRef = SingleLoadInst->getLvalDDRef();

    if (TempRefSet.count(LRef->getSymbase())) {
      return nullptr;
    }

    assert(!DDG.empty() && "Empty DDG not expected!");

    for (DDEdge *E : DDG.incoming(LRef)) {
      if (E->isAnti() || E->isOutput()) {
        return nullptr;
      }
    }

    for (DDEdge *E : DDG.outgoing(LRef)) {
      if (E->isOutput()) {
        return nullptr;
      }
    }
  }

  return SingleLoadInst;
}

bool HIRLMM::canSinkSingleStore(HLLoop *Lp, RegDDRef *FirstRef,
                                MemRefGroup &Group,
                                SmallSet<unsigned, 32> &TempRefSet) const {
  if (Lp->getNumExits() > 1) {
    return false;
  }

  if (Group.size() != 1 || !FirstRef->isLval()) {
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

  assert(!DDG.empty() && "Empty DDG not expected!");

  for (DDEdge *E : DDG.outgoing(RRef)) {
    if (E->isAnti()) {
      return false;
    }
  }

  return true;
}

// Check whether we need a Load in the Loops' preheader:
// - If there is no load (store-only Group), no need for tmp in prehder
// - Scan the Group from beginning:
//  . 1st hit a load: need
//  . 1st hit a store on dom path: no need
//
static bool isLoadNeededInPrehder(HLLoop *Lp, const MemRefGroup &Group) {
  if (Group.isStoreOnly()) {
    return false;
  }

  const HLNode *LoopTail = Lp->getLastChild();

  for (const RegDDRef *CurRef : Group) {

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
                                                   MemRefGroup &Group) {
  if (Group.size() != 1 || !Ref->isRval()) {
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

// Call setDefinedAtLevel(LoopLevel -1) to setLinear on a given RegDDRef*
static void setLinear(DDRef *TmpRef, unsigned LoopLevel) {
  TmpRef->getSingleCanonExpr()->setDefinedAtLevel(LoopLevel - 1);

  if (auto BlobRef = dyn_cast<BlobDDRef>(TmpRef)) {
    BlobRef->getParentDDRef()->updateDefLevel();
  } else {
    assert(cast<RegDDRef>(TmpRef)->isTerminalRef() &&
           "Expecting a terminal ref");
  }
}

bool HIRLMM::hoistLoadsUsingExistingTemp(HLLoop *Lp, MemRefGroup &Group,
                                         SmallSet<unsigned, 32> &TempRefSet,
                                         OptReportBuilder &ORBuilder) {

  HLInst *SingleLoadInst = nullptr;

  if (!(SingleLoadInst =
            canHoistLoadsUsingExistingTemp(Lp, Group, TempRefSet))) {
    return false;
  }

  RegDDRef *TempRef = SingleLoadInst->getLvalDDRef();

  auto *OuterLp = Lp->getParentLoop();
  auto *ParentLp = SingleLoadInst->getParentLoop();

  // Ref can be hoisted outisde multiple loops in loopnest hoisting mode.
  while (ParentLp != OuterLp) {
    ParentLp->addLiveInTemp(TempRef->getSymbase());
    ParentLp = ParentLp->getParentLoop();
  }

  assert(!DDG.empty() && "Empty DDG not expected!");

  for (DDEdge *E : DDG.outgoing(TempRef)) {
    if (E->isFlow()) {
      DDRef *DDRefSink = E->getSink();
      setLinear(DDRefSink, LoopLevel);
    }
  }

  auto *LoadRef = SingleLoadInst->getRvalDDRef();

  // Replace all other refs in the group with linear temp.
  for (auto *Ref : Group) {
    if (Ref == LoadRef) {
      continue;
    }

    auto *TempRefClone = TempRef->clone();
    setLinear(TempRefClone, LoopLevel);
    Ref->getHLDDNode()->replaceOperandDDRef(Ref, TempRefClone);
  }

  HLNodeUtils::moveAsLastPreheaderNode(Lp, SingleLoadInst);

  LoadRef->updateDefLevel(LoopLevel - 1);

  // ID: 25563u, remark string: Load hoisted out of the loop
  ORBuilder(*Lp).addRemark(OptReportVerbosity::Low, 25563u);

  return true;
}

bool HIRLMM::sinkStoresUsingExistingTemp(HLLoop *Lp, RegDDRef *StoreRef,
                                         MemRefGroup &Group,
                                         SmallSet<unsigned, 32> &TempRefSet,
                                         OptReportBuilder &ORBuilder) {
  if (!canSinkSingleStore(Lp, StoreRef, Group, TempRefSet)) {
    return false;
  }

  HLDDNode *StoreDDNode = StoreRef->getHLDDNode();

  HLNodeUtils::moveAsFirstPostexitNode(Lp, StoreDDNode);

  RegDDRef *TempRef = StoreDDNode->getRvalDDRef();

  Lp->addLiveOutTemp(TempRef->getSymbase());

  StoreRef->updateDefLevel(LoopLevel - 1);
  TempRef->updateDefLevel(LoopLevel - 1);

  // ID: 25564u, remark string: Store sinked out of the loop
  ORBuilder(*Lp).addRemark(OptReportVerbosity::Low, 25564u);
  return true;
}

// Do LIMM promotion on a Group
//
// [Algorithm]
//
// For each candidate group (Group)
//
// [Preparation]
// Decide if a store is needed in Loop's postexit:
//  -yes: if there is at least 1 store in Group
//
// Decide if a load is needed in Loop's preheader
//  -details in isLoadNeededInPrehder()
//
// [Do LIMM Promotion]
//  - Create a load in preheader if needed
//  - Create a store in postexit if needed
//  - Replace each load/store in group with the Tmp
//
void HIRLMM::doLIMMRef(HLLoop *Lp, MemRefGroup &Group,
                       SmallSet<unsigned, 32> &TempRefSet) {
  bool NeedLoadInPrehdr = false, NeedStoreInPostexit = false;
  RegDDRef *TmpDDRef = nullptr;
  RegDDRef *FirstRef = Group[0];

  HLInst *LoadInPrehdr = nullptr;

  bool IsLoadOnly = Group.isLoadOnly();
  bool IsInsideLifetimeIntrinsics = Group.isInsideLifetimeIntrinsics();
  // Debug: Examine the Loop BEFORE transformation
  // LLVM_DEBUG(Lp->dump(););

  // *** Prepare LMM for the Group ***

  // Need a Store in postexit: if there is at least 1 store in Group
  NeedStoreInPostexit = !IsLoadOnly && !IsInsideLifetimeIntrinsics;

  // Need a Load in prehdr: check algorithm for details
  NeedLoadInPrehdr = !IsInsideLifetimeIntrinsics &&
                     (IsLoadOnly || isLoadNeededInPrehder(Lp, Group));

  OptReportBuilder &ORBuilder =
      Lp->getHLNodeUtils().getHIRFramework().getORBuilder();

  if (!IsInsideLifetimeIntrinsics &&
      (hoistLoadsUsingExistingTemp(Lp, Group, TempRefSet, ORBuilder) ||
       sinkStoresUsingExistingTemp(Lp, FirstRef, Group, TempRefSet,
                                   ORBuilder))) {
    return;
  }

  HLLoop *OuterLp = getOuterLoopCandidateForSingleLoad(Lp, FirstRef, Group);

  // ### Promote LIMM for the Group ###

  // Create a Load in prehdr if needed
  if (NeedLoadInPrehdr) {
    // Passing FirstRef's lexical parent loop as it can be different than Lp in
    // loopnest hoisting mode.
    LoadInPrehdr = createLoadInPreheader(FirstRef->getLexicalParentLoop(),
                                         FirstRef, OuterLp);

    TmpDDRef = LoadInPrehdr->getLvalDDRef();

    // ID: 25563u, remark string: Load hoisted out of the loop
    ORBuilder(*Lp).addRemark(OptReportVerbosity::Low, 25563u);
  }

  // Create a TempDDRef if needed
  if (!TmpDDRef) {
    TmpDDRef = HNU.createTemp(FirstRef->getDestType(), "limm");
  }

  TempRefSet.insert(TmpDDRef->getSymbase());

  // Create a Store in postexit if needed
  if (NeedStoreInPostexit) {
    RegDDRef *FirstStore = FirstRef;

    // If there is a need to generate a store in postexit, we try to identify
    // the first store ref from the group, and use it to clone for the store in
    // postexit.
    for (auto *Ref : Group) {
      if (Ref->isLval()) {
        FirstStore = Ref;
        break;
      }
    }

    createStoreInPostexit(Lp, FirstStore, TmpDDRef, NeedLoadInPrehdr);

    // ID: 25564u, remark string: Store sinked out of the loop
    ORBuilder(*Lp).addRemark(OptReportVerbosity::Low, 25564u);
  }

  // LMM process each Ref in Group
  for (auto *Ref : Group) {
    handleInLoopMemRef(OuterLp->getNestingLevel(), Ref, TmpDDRef, IsLoadOnly);
  }

  // Debug: Examine the Loop AFTER transformation
  // LLVM_DEBUG(Lp->dump(););
}

// Handle each loop-inv MemRef inside a group
//
// Special case: create a CopyInst and do proper replacement
// - LoadInst
// - StoreInst
//
// All Others: do regular replacement
//
void HIRLMM::handleInLoopMemRef(unsigned Level, RegDDRef *Ref, RegDDRef *TmpRef,
                                bool IsLoadOnly) {
  // Debug: Examine the Loop Before processing
  // LLVM_DEBUG(Lp->dump(););
  RegDDRef *TmpRefClone = TmpRef->clone();

  if (IsLoadOnly) {
    setLinear(TmpRefClone, Level);
  }

  HIRTransformUtils::replaceOperand(Ref, TmpRefClone);

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

// Creates a new LoadInst in preheader and returns it.
//
// Additional Work:
// - Mark the new Temp as live-in to loop
// - Mark the new Temp as linear, defined at level = looplevel -1
// - Call updateDefLevel() for the Rval of the new load
//
HLInst *HIRLMM::createLoadInPreheader(HLLoop *RefLp, RegDDRef *Ref,
                                      HLLoop *OuterLp) const {
  auto RvalRef = Ref->clone();
  HLInst *LoadInPrehdr = HNU.createLoad(RvalRef, "limm");

  RegDDRef *TmpRef = LoadInPrehdr->getLvalDDRef();
  unsigned TmpSB = TmpRef->getSymbase();

  insertInPreheader(OuterLp, LoadInPrehdr, RvalRef);

  unsigned OuterLpLevel = OuterLp->getNestingLevel();
  unsigned Level = RefLp->getNestingLevel();

  while (Level >= OuterLpLevel) {
    RefLp->addLiveInTemp(TmpSB);
    RefLp = RefLp->getParentLoop();
    Level--;
  }

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
    const RegDDRef *LHSRef = PrevIf->getLHSPredicateOperandDDRef(PredI);
    const RegDDRef *RHSRef = PrevIf->getRHSPredicateOperandDDRef(PredI);
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

template <typename IterTy>
static std::pair<RegDDRef *, bool>
findMemRefLoadInRange(IterTy Begin, IterTy End, RegDDRef *MemRef,
                      SmallSet<unsigned, 8> &DefinedTempSBs) {

  for (auto It = Begin; It != End; ++It) {

    // Give up on non-insts.
    if (!isa<HLInst>(*It)) {
      return {nullptr, true};
    }

    auto *Inst = cast<HLInst>(&*It);

    // Give up on calls.
    if (Inst->isCallInst()) {
      return {nullptr, true};
    }

    auto *LvalRef = Inst->getLvalDDRef();
    if (isa<LoadInst>(Inst->getLLVMInstruction()) &&
        DDRefUtils::areEqual(MemRef, Inst->getRvalDDRef())) {
      // Return nullptr if temp has been redefined.
      if (DefinedTempSBs.count(LvalRef->getSymbase())) {
        return {nullptr, true};
      } else {
        return {LvalRef->clone(), false};
      }
    }

    if (auto *LvalRef = Inst->getLvalDDRef()) {
      // Give up search on encountering aliasing store.
      if (LvalRef->isMemRef()) {
        if (LvalRef->getSymbase() == MemRef->getSymbase()) {
          return {nullptr, true};
        }
      } else {
        DefinedTempSBs.insert(LvalRef->getSymbase());
      }
    }
  }

  return {nullptr, false};
}

// Tries to find a load of \p MemRef in loop preheader or before it.
// Returns the lval temp of the load if found, else returns nullptr.
static RegDDRef *findMemRefLoadBeforeLoop(HLLoop *Lp, RegDDRef *MemRef) {
  SmallSet<unsigned, 8> DefinedTempSBs;
  RegDDRef *InitRef = nullptr;
  bool BailOut = false;

  std::tie(InitRef, BailOut) = findMemRefLoadInRange(
      Lp->pre_rbegin(), Lp->pre_rend(), MemRef, DefinedTempSBs);

  if (InitRef) {
    return InitRef;
  }

  HLNode *PrevNode = Lp->getPrevNode();
  if (!BailOut && PrevNode) {
    auto BegIt = PrevNode->getReverseIterator();
    auto *FirstLexChild =
        HLNodeUtils::getFirstLexicalChild(Lp->getParent(), Lp);
    auto EndIt = std::next(FirstLexChild->getReverseIterator());

    std::tie(InitRef, BailOut) =
        findMemRefLoadInRange(BegIt, EndIt, MemRef, DefinedTempSBs);
  }

  return InitRef;
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
  auto *StoreInPostexit = HNU.createStore(TmpRefClone, "limm", LvalRef);

  if (Lp->getNumExits() > 1) {
    // Collect all the Gotos if it is a multi-exit loop
    SmallVector<HLGoto *, 16> Gotos;
    Lp->populateEarlyExits(Gotos);
    bool TmpIsInitialized = NeedLoadInPrehdr;
    bool RequiresIVComparisonIf = true;

    for (auto &Goto : Gotos) {
      auto *StoreInst = StoreInPostexit->clone();

      if (HLNodeUtils::dominates(Ref->getHLDDNode(), Goto)) {
        HLNodeUtils::insertBefore(Goto, StoreInst);
      } else {
        // Create an inst like %t = 0 and insert it as last preheader inst of
        // the loop. If an existing load of this memref like this is found
        // before the loop- %ld = A[5];
        //
        // An copy of this form is generated in the preheader-
        // %t = %ld;
        //
        if (!TmpIsInitialized) {
          RegDDRef *LHS = TmpRefClone->clone();

          RegDDRef *RHS = findMemRefLoadBeforeLoop(Lp, Ref);

          if (!RHS) {
            RHS = HIRF.getDDRefUtils().createNullDDRef(LHS->getDestType());
          } else {
            RequiresIVComparisonIf = false;
          }
          auto *InitialTemp = HNU.createCopyInst(RHS, "copy", LHS);
          Lp->addLiveInTemp(LHS->getSymbase());
          HLNodeUtils::insertAsLastPreheaderNode(Lp, InitialTemp);
          TmpIsInitialized = true;
        }

        if (RequiresIVComparisonIf) {
          HLIf *IVCheckIf = getIVComparisonIf(Lp, Goto);
          HLNodeUtils::insertAsFirstThenChild(IVCheckIf, StoreInst);
        } else {
          HLNodeUtils::insertBefore(Goto, StoreInst);
        }
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

PreservedAnalyses HIRLMMPass::runImpl(llvm::Function &F,
                                      llvm::FunctionAnalysisManager &AM,
                                      HIRFramework &HIRF) {

#if INTEL_FEATURE_SW_DTRANS
  auto &MAMProxy = AM.getResult<ModuleAnalysisManagerFunctionProxy>(F);
#endif // INTEL_FEATURE_SW_DTRANS

  HIRLMM(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
         AM.getResult<HIRLoopStatisticsAnalysis>(F),
#if INTEL_FEATURE_SW_DTRANS
         MAMProxy.getCachedResult<DTransFieldModRefResult>(*F.getParent()),
#endif // INTEL_FEATURE_SW_DTRANS
         &AM.getResult<DominatorTreeAnalysis>(F),
         (LoopNestHoistingOnly || ForceLoopNestHoisting))
      .run();
  return PreservedAnalyses::all();
}

class HIRLMMLegacyPass : public HIRTransformPass {
public:
  static char ID;
  bool LoopNestHoistingOnly;

  HIRLMMLegacyPass(bool LoopNestHoistingOnly = false)
      : HIRTransformPass(ID), LoopNestHoistingOnly(LoopNestHoistingOnly) {
    initializeHIRLMMLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<DominatorTreeWrapperPass>();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
#if INTEL_FEATURE_SW_DTRANS
    AU.addRequiredTransitive<DTransFieldModRefResultWrapper>();
#endif // INTEL_FEATURE_SW_DTRANS
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRLMM(getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
                  getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
                  getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS(),
#if INTEL_FEATURE_SW_DTRANS
                  &getAnalysis<DTransFieldModRefResultWrapper>().getResult(),
#endif // INTEL_FEATURE_SW_DTRANS
                  &getAnalysis<DominatorTreeWrapperPass>().getDomTree(),
                  (LoopNestHoistingOnly || ForceLoopNestHoisting))
        .run();
  }
};

char HIRLMMLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRLMMLegacyPass, "hir-lmm", "HIR Loop Memory Motion",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
#if INTEL_FEATURE_SW_DTRANS
INITIALIZE_PASS_DEPENDENCY(DTransFieldModRefResultWrapper)
#endif // INTEL_FEATURE_SW_DTRANS
INITIALIZE_PASS_END(HIRLMMLegacyPass, "hir-lmm", "HIR Loop Memory Motion",
                    false, false)

FunctionPass *llvm::createHIRLMMPass(bool LoopNestHoistingOnly) {
  return new HIRLMMLegacyPass(LoopNestHoistingOnly);
}
