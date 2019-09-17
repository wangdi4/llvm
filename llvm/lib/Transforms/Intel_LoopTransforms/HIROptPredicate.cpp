//===--- HIROptPredicate.cpp - Implements OptPredicate class --------------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements HIROptPredicate class which performs if switching for
// a HIR loop.
//
// Before:
// for(i=0; i<n; i++) {
//   A;
//   if(Cond1) {
//    B;
//   }
//   C;
// }
//
// After:
// if(Cond1) {
//  for(i=0; i<n; i++) {
//   A; B; C;
//  }
// } else {
//  for(i=0; i<n; i++) {
//   A; C;
//  }
// }
//
//===----------------------------------------------------------------------===//

// TODO:
// 1. Handle loops with internal branches and update gotos and label within
//    the parent loop of opt-predicate.
// 2. Handle IV's inside the predicate. This will need to create separate blocks
//    of ranged loops depending on the loop condition.
// 3. Do deep predicate analysis where conditions can be proved to be always
//    true or always false. There may be many other mathematical simplification
//    possible and extensions.
// 4. Possible future extensions would be to support memory references inside
//    the HLIf condition for OptPredicate.
// 5. Add opt report messages.

#include "llvm/Transforms/Intel_LoopTransforms/HIROptPredicate.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/Triple.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNodeMapper.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/CanonExprUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/ForEach.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#define OPT_SWITCH "hir-opt-predicate"
#define OPT_DESC "HIR OptPredicate"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

STATISTIC(IfsUnswitched, "Number of HLIfs optimized (hoisted)");
STATISTIC(IfsPUUnswitched, "Number of HLIfs partially optimized (hoisted)");
// This will also count if the loop has been analyzed multiple times
// due to nested/multiple if.
STATISTIC(IfsAnalyzed, "Number of HLIfs analyzed for loop unswitching");

constexpr unsigned DefaultNumPredicateThreshold = 3;

static cl::opt<unsigned> NumPredicateThreshold(
    OPT_SWITCH "-threshold", cl::init(DefaultNumPredicateThreshold), cl::Hidden,
    cl::desc("Don't opt predicate a loop which has been transformed greater"
             "than this threshold."));

static cl::opt<bool>
    DisableLoopUnswitch("disable-" OPT_SWITCH, cl::init(false), cl::Hidden,
                        cl::desc("Disable HIR Loop unswitching"));

static cl::opt<bool> DisableCostModel("disable-" OPT_SWITCH "-cost-model",
                                      cl::init(false), cl::Hidden,
                                      cl::desc("Disable " OPT_DESC
                                               " cost model related checks"));

static cl::opt<bool> DisablePartialUnswitch("disable-" OPT_SWITCH "-pu",
                                            cl::init(false), cl::Hidden,
                                            cl::desc("Disable " OPT_DESC
                                                     " partial unswitch"));

static cl::opt<bool>
    KeepLoopnestPerfectOption(OPT_SWITCH "-keep-perfect", cl::init(false),
                              cl::Hidden,
                              cl::desc(OPT_DESC " keep loopnests perfect"));

namespace {

// Partial Unswitch context
struct PUCandidate {
  bool IsRequired = false;

  SmallPtrSet<HLInst *, 8> Instructions;

  bool IsUpdatedInThenBranch = false;
  bool IsUpdatedInElseBranch = false;

public:
  SmallPtrSetImpl<HLInst *> &getInstructions() { return Instructions; }

  bool isPUCandidate() const {
    return !(IsUpdatedInThenBranch && IsUpdatedInElseBranch);
  }

  bool isPURequired() const { return IsRequired; }

  void setPURequired() { IsRequired = true; }

  PUCandidate() {}

  PUCandidate(const PUCandidate &Arg, const HLNodeMapper &Mapper)
      : IsRequired(Arg.IsRequired),
        IsUpdatedInThenBranch(Arg.IsUpdatedInThenBranch),
        IsUpdatedInElseBranch(Arg.IsUpdatedInElseBranch) {
    for (HLInst *Inst : Arg.Instructions) {
      HLInst *ClonedInst = Mapper.getMapped(Inst);
      assert(ClonedInst && "Nullptr instruction");
      Instructions.insert(ClonedInst);
    }
  }

#ifndef NDEBUG
  LLVM_DUMP_METHOD
  void dump() {
    dbgs() << "[ ";

    dbgs() << (IsUpdatedInThenBranch ? "T" : "F");
    dbgs() << "/";
    dbgs() << (IsUpdatedInElseBranch ? "T" : "F");
    dbgs() << " ";

    for (HLInst *Inst : Instructions) {
      dbgs() << "<" << Inst->getNumber() << "> ";
    }
    dbgs() << "]";
  }
#endif
};

struct PUContext : PUCandidate {
  SmallPtrSet<const RegDDRef *, 8> VisitedRefs;
};

struct HoistCandidate {
  HLIf *If;
  unsigned Level;

  PUCandidate PUC;

  HoistCandidate(HLIf *If, unsigned Level, const PUCandidate &PUC)
      : If(If), Level(Level), PUC(PUC) {}

  HoistCandidate(const HoistCandidate &Orig, HLIf *CloneIf,
                 const HLNodeMapper &Mapper)
      : If(CloneIf), Level(Orig.Level), PUC(Orig.PUC, Mapper) {}

  HoistCandidate() : Level(0) {}

  HLIf *getIf() const { return If; }

  bool operator==(const HoistCandidate &Arg) const { return If == Arg.If; }

#ifndef NDEBUG
  LLVM_DUMP_METHOD
  void dump() {
    dbgs() << "{";
    getIf()->dumpHeader();
    dbgs() << ", L: " << Level << ", PU: ";
    PUC.dump();
    dbgs() << "}";
  }
#endif
};

class HIROptPredicate {
public:
  static char ID;

  HIROptPredicate(HIRFramework &HIRF, HIRDDAnalysis &DDA,
                  bool EnablePartialUnswitch, bool KeepLoopnestPerfect)
      : HIRF(HIRF), DDA(DDA),
        EnablePartialUnswitch(EnablePartialUnswitch && !DisablePartialUnswitch),
        KeepLoopnestPerfect(KeepLoopnestPerfect || KeepLoopnestPerfectOption) {}

  bool run();

private:
  HIRFramework &HIRF;
  HIRDDAnalysis &DDA;

  bool EnablePartialUnswitch;
  bool KeepLoopnestPerfect;

  struct CandidateLookup;
  class LoopUnswitchNodeMapper;

  SmallDenseMap<const HLLoop *, unsigned, 16> ThresholdMap;
  SmallVector<HoistCandidate, 16> Candidates;

  /// Returns true if the non-linear rval Ref of the If condition may be
  /// unswitched together with its definition.
  bool isPUCandidate(const HLIf *If, const RegDDRef *Ref, PUContext &PU) const;

  /// Returns true of false whatever \p Edge prevents unswitching of non-linear
  /// condition. Populates PU and RefsStack.
  bool processPUEdge(const HLIf *If, DDEdge *Edge, PUContext &PU,
                     SmallVectorImpl<const RegDDRef *> &RefsStack,
                     DDGraph &DDG) const;

  /// Sorts candidates in top sort number descending.
  /// They will be handled from right to left.
  void sortCandidates();

  /// This routine will loop through the candidate loop to look
  /// for HLIf candidates. If all conditions are met, it will transform
  /// the loop. Returns true if transformation happened.
  bool processOptPredicate(bool &HasMultiexitLoop);

  /// Returns the deepest level at which any of the If/Ref operands is defined.
  unsigned getPossibleDefLevel(const HLIf *If, PUContext &PUC);
  unsigned getPossibleDefLevel(const HLIf *If, const RegDDRef *Ref,
                               PUContext &PUC);

  /// Returns the possible level where CE is defined. NonLinearBlob is set true
  /// whenever CE contains a non-linear blob.
  unsigned getPossibleDefLevel(const CanonExpr *CE, bool &NonLinearBlob);

  /// Transform the original loop.
  void transformCandidate(HLLoop *TargetLoop, HoistCandidate &Candidate,
                          unsigned &VNum,
                          SmallPtrSetImpl<HLLoop *> &OptReportVisitedSet,
                          SmallPtrSetImpl<HLIf *> &IfVisitedSet);

  /// Removes the Then and Else children of HLIf and stores them
  /// in the container.
  void removeThenElseChildren(HLIf *If, HLContainerTy *ThenContainer,
                              HLContainerTy *ElseContainer);

  /// Performs the OptPredicate transformation where condition is
  /// hoisted outside and loops are moved inside the condition and updates the
  /// DDRefs inside the HLIf.
  void removeOrHoistIf(HoistCandidate &Candidate, HLLoop *TargetLoop,
                       HLIf *FirstIf, HLIf *If, HLIf *&PivotIf);
  void hoistIf(HLIf *&If, HLLoop *OrigLoop);

  void addPredicateOptReport(HLLoop *TargetLoop, HLIf *If,
                             SmallPtrSetImpl<HLLoop *> &OptReportVisitedSet,
                             SmallPtrSetImpl<HLIf *> &IfVisitedSet,
                             unsigned &VNum);

#ifndef NDEBUG
  LLVM_DUMP_METHOD
  void dumpCandidates() {
    dbgs() << "Candidates, count: " << Candidates.size() << "\n";
    for (HoistCandidate &Candidate : Candidates) {
      Candidate.dump();
      dbgs() << "\n";
    }
  }
#endif
};

class HIROptPredicate::LoopUnswitchNodeMapper : public HLNodeToNodeMapperImpl {
  SmallPtrSetImpl<HLNode *> &TrackedNodes;
  decltype(Candidates) &CandidatesRef;
  decltype(Candidates) NewCandidates;

public:
  LoopUnswitchNodeMapper(SmallPtrSetImpl<HLNode *> &TrackedNodes,
                         decltype(Candidates) &CandidatesRef)
      : TrackedNodes(TrackedNodes), CandidatesRef(CandidatesRef) {}

  void map(const HLNode *Node, HLNode *MappedNode) override {
    const HLInst *Inst = dyn_cast<HLInst>(Node);

    if (Inst && TrackedNodes.count(Inst)) {
      NodeMap[Node] = MappedNode;
      return;
    }

    const HLIf *If = dyn_cast<HLIf>(Node);

    if ((If && TrackedNodes.count(If)) || isa<HLLabel>(Node)) {
      NodeMap[Node] = MappedNode;
      return;
    }

    if (!If) {
      return;
    }

    HLIf *CloneIf = cast<HLIf>(MappedNode);
    auto CandidateI = std::find_if(CandidatesRef.begin(), CandidatesRef.end(),
                                   [If](const HoistCandidate &Candidate) {
                                     return Candidate.getIf() == If;
                                   });

    // Original HLIf was found as a candidate, this means that we are still
    // interested in this node - have to save original node and update
    // candidate clone list.
    if (CandidateI != CandidatesRef.end()) {
      NewCandidates.emplace_back(*CandidateI, CloneIf, *this);

      LLVM_DEBUG(dbgs() << "Found new candidate: ");
      LLVM_DEBUG(NewCandidates.back().dump());
      LLVM_DEBUG(dbgs() << "\n");
    }
  }

  decltype(Candidates) &getNewCandidates() { return NewCandidates; }
};

struct HIROptPredicate::CandidateLookup final : public HLNodeVisitorBase {
  HLNode *SkipNode;
  HIROptPredicate &Pass;
  unsigned MinLevel;
  bool TransformLoop;

  CandidateLookup(HIROptPredicate &Pass, bool TransformLoop = true,
                  unsigned MinLevel = 0)
      : SkipNode(nullptr), Pass(Pass), MinLevel(MinLevel),
        TransformLoop(TransformLoop) {}

  void visit(HLIf *If);
  void visit(HLLoop *Loop);

  bool skipRecursion(const HLNode *Node) const { return Node == SkipNode; }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
};
} // namespace

bool HIROptPredicate::processPUEdge(
    const HLIf *If, DDEdge *Edge, PUContext &PU,
    SmallVectorImpl<const RegDDRef *> &RefsStack, DDGraph &DDG) const {
  if (!Edge->isFLOWdep()) {
    // May ignore non-flow edges.
    return true;
  }

  RegDDRef *SrcRef = cast<RegDDRef>(Edge->getSrc());

  // Source of the flow edge is always an Inst.
  HLInst *Inst = cast<HLInst>(SrcRef->getHLDDNode());

  if (!Edge->isForwardDep()) {
    // VRef is updated from the previous loop iteration

    if (If->isThenChild(Inst)) {
      PU.IsUpdatedInThenBranch = true;
    } else if (If->isElseChild(Inst)) {
      PU.IsUpdatedInElseBranch = true;
    } else {
      // Is updated outside of the If construct
      return false;
    }

    // Is updated in both If branches
    if (PU.IsUpdatedInThenBranch && PU.IsUpdatedInElseBranch) {
      return false;
    }

    return true;
  }

  if (!SrcRef->isTerminalRef()) {
    // To support memory references we also have to take into account output
    // edges.
    // Here is an example:
    //
    // char *%a, int *%b, float *%c
    //
    // DO i1 = 0, %n, 1
    // |   (%c)[i1] = 1.000000e+00;
    // |   (%a)[0] = 1;
    // |   if ((%b)[0] == 100) {
    // |      (%b)[0] = 1;
    // |   }
    // + END LOOP
    //
    // %a-%c, %a-%b may alias
    // %c-%b may not alias
    //
    // To unswitch this loop, both %c and %a assignments should be hoisted.
    return false;
  }

  if (SrcRef->isLiveOutOfParentLoop()) {
    // This is conservative check to handle cases where the flow edge from 2nd
    // definition of t1 may not exist because it's killed by the first
    // definition.
    //
    // DO i1 = 0, %n, 1
    // |   t1 = ...
    // |   if (t1) {
    // |      ...
    // |   }
    // |   if (...) {
    // |      t1 = ...
    // |   }
    // + END LOOP
    return false;
  }

  if (Inst->isUnsafeSideEffectsCallInst()) {
    // Unsafe to speculate
    return false;
  }

  if (SrcRef->isFakeLval()) {
    // Do not handle cases when the condition may be changed by a call.
    return false;
  }

  if (!HLNodeUtils::dominates(Inst, If)) {
    // Handle only simple CFG.
    return false;
  }

  for (auto &Edge : DDG.outgoing(SrcRef)) {
    if (Edge->isOUTPUTdep()) {
      return false;
    }
  }

  PU.getInstructions().insert(Inst);

  auto RefsRange = concat<RegDDRef *>(
      make_range(Inst->rval_op_ddref_begin(), Inst->rval_op_ddref_end()),
      make_range(Inst->rval_fake_ddref_begin(), Inst->rval_fake_ddref_end()));

  for (RegDDRef *RVal : RefsRange) {
    if (!PU.VisitedRefs.count(RVal) && !RVal->isConstant()) {
      RefsStack.push_back(RVal);
    }
  }

  return true;
}

// The HLIf may be unswitched partially if one of
// the branches may update value of the condition.
//
// DO i1 = 0, 5
//   %0 = a[0]
//   if (%0 == 1) {
//     {code}
//   } else {
//     a[i] = ...
//   }
// END DO
//
// %0 = a[0]
// if (%0 == 1) {
//   DO i1 = 0, 5
//     {code}
//   END DO
// } else {
//   DO i1 = 0, 5
//     %0 = a[0]
//     if (%0 == 1) {
//       {code}
//     } else {
//       a[i] = ...
//     }
//   END DO
// }
//
bool HIROptPredicate::isPUCandidate(const HLIf *If, const RegDDRef *Ref,
                                    PUContext &PU) const {
  assert(isa<HLIf>(Ref->getHLDDNode()) && "Ref parent is not an HLIf");
  assert(Ref->getHLDDNode() == If && "HLIf should be a parent of Ref");

  if (!EnablePartialUnswitch) {
    LLVM_DEBUG(dbgs() << "LOOPOPT_OPTREPORT: skipping <" << If->getNumber()
                      << "> candidate due to optlevel\n");
    return false;
  }

  // Try to hoist non-linear condition with all its definition instructions.

  HLLoop *ParentLoop = If->getParentLoop();
  unsigned ParentLoopLevel = ParentLoop->getNestingLevel();
  DDGraph DDG = DDA.getGraph(ParentLoop);

#ifndef NDEBUG
  static HLLoop *DDGShownForLoop = nullptr;
  if (DDGShownForLoop != ParentLoop) {
    LLVM_DEBUG(DDG.dump());
    DDGShownForLoop = ParentLoop;
  }
#endif

  SmallVector<const RegDDRef *, 32> RefsStack;

  RefsStack.push_back(Ref);
  while (!RefsStack.empty()) {
    const RegDDRef *VRef = RefsStack.pop_back_val();
    PU.VisitedRefs.insert(VRef);

    if (VRef->isMemRef() && VRef->isVolatile()) {
      return false;
    }

    // Check that the VRef value is independent from ParentLoop's IV.
    if (VRef->hasIV(ParentLoopLevel)) {
      return false;
    }

    // Look for all incoming flow edges.

    for (const BlobDDRef *BDDRef :
         make_range(VRef->blob_begin(), VRef->blob_end())) {
      for (auto &Edge : DDG.incoming(BDDRef)) {
        if (!processPUEdge(If, Edge, PU, RefsStack, DDG)) {
          return false;
        }
      }
    }

    for (auto &Edge : DDG.incoming(VRef)) {
      if (!processPUEdge(If, Edge, PU, RefsStack, DDG)) {
        return false;
      }
    }
  }

  return true;
}

class UnsafeCallFinder : public HLNodeVisitorBase {
  bool HasUnsafeCall = false;
  HLNode *SkipNode;

public:
  UnsafeCallFinder(HLNode *SkipNode = nullptr) : SkipNode(SkipNode) {}

  void visit(const HLNode *) {}
  void postVisit(const HLNode *) {}

  void visit(const HLInst *Inst) {
    if (auto *CInst = Inst->getCallInst()) {
      if (!CInst->onlyReadsMemory() && HLInst::hasUnknownAliasing(CInst)) {
        HasUnsafeCall = true;
      }
    }
  }

  bool skipRecursion(const HLNode *Node) const { return SkipNode == Node; }
  bool isDone() const { return HasUnsafeCall; }

  bool hasUnsafeCall() const { return HasUnsafeCall; }
};

void HIROptPredicate::CandidateLookup::visit(HLIf *If) {
  HLLoop *ParentLoop = If->getParentLoop();
  if (!ParentLoop || ParentLoop->getBottomTest() == If) {
    return;
  }

  SkipNode = If;

  // Partial unswitch context
  PUContext PUC;

  bool IsCandidate = TransformLoop && !If->isUnswitchDisabled();

  // Skip candidates that are already at the outer most possible level.
  unsigned Level;

  if (IsCandidate) {
    // Determine target level to unswitch.
    Level = std::max(Pass.getPossibleDefLevel(If, PUC), MinLevel);

    if (Level < ParentLoop->getNestingLevel()) {
      // Check if condition does not depend on both T/F branches at the same
      // time.
      IsCandidate = PUC.isPUCandidate();
    } else {
      IsCandidate = false;
    }
  } else {
    Level = ParentLoop->getNestingLevel();
  }

  if (IsCandidate && Pass.KeepLoopnestPerfect && Level != 0) {
    // Check if unswitching breaks the existing loopnest perfectness.
    IsCandidate = If->getParentLoopAtLevel(Level)->getNumChildren() > 1;
  }

  if (IsCandidate && PUC.isPURequired()) {
    // HLIf should be unconditionally executed to unswitch.
    IsCandidate = HLNodeUtils::postDominates(If, ParentLoop->getFirstChild());
  }

  // Check for unsafe calls in branches that can modify the condition.
  // TODO: Do we need HIRStatistics instead of LoopStatistics?
  if (IsCandidate && PUC.isPURequired()) {
    UnsafeCallFinder LoopUnsafe(If), ThenUnsafe, ElseUnsafe;
    HLNodeUtils::visitRange(LoopUnsafe, ParentLoop->child_begin(),
                            ParentLoop->child_end());
    HLNodeUtils::visitRange(ThenUnsafe, If->then_begin(), If->then_end());
    HLNodeUtils::visitRange(ElseUnsafe, If->else_begin(), If->else_end());
    PUC.IsUpdatedInThenBranch = PUC.IsUpdatedInThenBranch |
                                ThenUnsafe.hasUnsafeCall() |
                                LoopUnsafe.hasUnsafeCall();
    PUC.IsUpdatedInElseBranch = PUC.IsUpdatedInElseBranch |
                                ElseUnsafe.hasUnsafeCall() |
                                LoopUnsafe.hasUnsafeCall();
    IsCandidate = PUC.isPUCandidate();
  }

  LLVM_DEBUG(dbgs() << "Opportunity: ");
  LLVM_DEBUG(If->dumpHeader());
  LLVM_DEBUG(dbgs() << " --> Level " << Level << ", Candidate: " << IsCandidate
                    << (PUC.isPURequired() ? "(PU)" : "") << "\n");

  // Tell inner candidates that parent candidate will not be unswitched.
  // Do not unswitch inner candidates in case of partial unswitching.
  bool WillUnswitchParent = IsCandidate && !PUC.isPURequired();

  // Collect candidates within HLIf branches.
  CandidateLookup Lookup(Pass, WillUnswitchParent, Level);
  HLNodeUtils::visitRange(Lookup, If->then_begin(), If->then_end());
  HLNodeUtils::visitRange(Lookup, If->else_begin(), If->else_end());

  if (!IsCandidate) {
    return;
  }

  Pass.Candidates.emplace_back(If, Level, PUC);
}

void HIROptPredicate::CandidateLookup::visit(HLLoop *Loop) {
  SkipNode = Loop;

  bool TransformLoop = true;

  if (!DisableCostModel && !Loop->isInnermost()) {
    TransformLoop = false;
  }

  if (Loop->isSIMD()) {
    TransformLoop = false;
  }

  CandidateLookup Lookup(Pass, TransformLoop, MinLevel);
  Loop->getHLNodeUtils().visitRange(Lookup, Loop->child_begin(),
                                    Loop->child_end());
}

void HIROptPredicate::sortCandidates() {
  std::sort(Candidates.begin(), Candidates.end(),
            [](const HoistCandidate &A, const HoistCandidate &B) {
              return B.getIf()->getTopSortNum() < A.getIf()->getTopSortNum();
            });
}

bool HIROptPredicate::run() {
  if (DisableLoopUnswitch) {
    return false;
  }

#if INTEL_FEATURE_CSA
  // When compiling for offload target = CSA:
  //
  // If the user does not specify the option -mllvm
  // disable-hir-opt-predicate at all on the command line, then
  // HIROptPredicate (i.e., loop unswitching) will be disabled by
  // default. HIROptPredicate will be enabled only when the user
  // specifies -mllvm disable-hir-opt-predicate=false (or = 0) on the
  // command line.

  bool IsCsaTarget =
      Triple(HIRF.getFunction().getParent()->getTargetTriple()).getArch() ==
      Triple::ArchType::csa;

  if (IsCsaTarget && (DisableLoopUnswitch.getNumOccurrences() == 0)) {
    return false;
  }
#endif // INTEL_FEATURE_CSA

  LLVM_DEBUG(dbgs() << "Opt Predicate for Function: "
                    << HIRF.getFunction().getName() << "\n");

  for (HLNode &Node : make_range(HIRF.hir_begin(), HIRF.hir_end())) {
    HLRegion *Region = cast<HLRegion>(&Node);

    LLVM_DEBUG(dbgs() << "Region: " << Region->getNumber() << ":\n");

    CandidateLookup Lookup(*this);
    HLNodeUtils::visit(Lookup, Region);
    sortCandidates();

    LLVM_DEBUG(dumpCandidates());

    bool HasMultiexitLoop;
    if (processOptPredicate(HasMultiexitLoop)) {
      Region->setGenCode();
      HLNodeUtils::removeRedundantNodes(Region, false);

      if (HasMultiexitLoop) {
        HLNodeUtils::updateNumLoopExits(Region);
      }
    }

    Candidates.clear();
    ThresholdMap.clear();
  }

  return false;
}

unsigned HIROptPredicate::getPossibleDefLevel(const CanonExpr *CE,
                                              bool &NonLinearBlob) {
  unsigned IVMaxLevel = 0;
  unsigned IVLevel = 0;
  for (auto I = CE->iv_begin(), E = CE->iv_end(); I != E; ++I) {
    // TODO: Traverse in reverse order and break as soon as a non-zero
    // constant coeff found. Also, the IV level can be accessed using
    // CE->getLevel(I). But before it will be possible getIVConstCoeff()
    // and getLevel() should support reverse iterators.
    IVLevel++;
    if (CE->getIVConstCoeff(I)) {
      IVMaxLevel = IVLevel;
    }
  }

  unsigned DefLevel = CE->getDefinedAtLevel();
  if (DefLevel == NonLinearLevel) {
    // The caller uses max IV level if there was non linear blobs.
    NonLinearBlob = true;
    return IVMaxLevel;
  }

  return std::max(IVMaxLevel, DefLevel);
}

unsigned HIROptPredicate::getPossibleDefLevel(const HLIf *If,
                                              const RegDDRef *Ref,
                                              PUContext &PUC) {
  unsigned Level = 0;
  bool NonLinearRef = false;
  bool HasGEPInfo = Ref->hasGEPInfo();

  if (HasGEPInfo) {
    Level = getPossibleDefLevel(Ref->getBaseCE(), NonLinearRef);
  }

  for (unsigned I = 1, NumDims = Ref->getNumDimensions(); I <= NumDims; ++I) {
    Level = std::max(
        Level, getPossibleDefLevel(Ref->getDimensionIndex(I), NonLinearRef));

    if (HasGEPInfo) {
      Level = std::max(
          Level, getPossibleDefLevel(Ref->getDimensionLower(I), NonLinearRef));
      Level = std::max(
          Level, getPossibleDefLevel(Ref->getDimensionStride(I), NonLinearRef));
    }
  }

  if (If->getNodeLevel() == Level) {
    // Reference is dependent on If's nesting level. Ex.: a[i1] or i1 + %b
    return Level;
  }

  if (NonLinearRef || Ref->isMemRef()) {
    // Return current level of attachment.
    Level = If->getNodeLevel();

    if (isPUCandidate(If, Ref, PUC)) {
      PUC.setPURequired();

      // May hoist one level up only.
      Level -= 1;
    }
  }

  return Level;
}

unsigned HIROptPredicate::getPossibleDefLevel(const HLIf *If, PUContext &PUC) {
  unsigned Level = 0;

  for (auto PI = If->pred_begin(), PE = If->pred_end(); PI != PE; ++PI) {
    const RegDDRef *Ref1 = If->getPredicateOperandDDRef(PI, true);
    const RegDDRef *Ref2 = If->getPredicateOperandDDRef(PI, false);

    Level = std::max(Level, getPossibleDefLevel(If, Ref1, PUC));
    if (Level == NonLinearLevel) {
      return Level;
    }

    Level = std::max(Level, getPossibleDefLevel(If, Ref2, PUC));
    if (Level == NonLinearLevel) {
      return Level;
    }
  }

  return Level;
}

/// processOptPredicate - Main routine to perform opt predicate
/// transformation.
bool HIROptPredicate::processOptPredicate(bool &HasMultiexitLoop) {
  SmallPtrSet<HLLoop *, 8> ParentLoopsToInvalidate;
  SmallPtrSet<HLLoop *, 8> TargetLoopsToInvalidate;

  // Chunk number
  unsigned VNum = 1;

  // Sets including visited TargetLoop, visited NewElseLoop and If line nums
  SmallPtrSet<HLLoop *, 8> OptReportVisitedSet;
  SmallPtrSet<HLIf *, 8> IfVisitedSet;

  while (!Candidates.empty()) {
    HoistCandidate &Candidate = Candidates.back();

    HLIf *PilotIf = Candidate.getIf();

    IfsAnalyzed++;

    HLLoop *ParentLoop = PilotIf->getParentLoop();
    assert(ParentLoop && "Candidate should have a parent loop");

    LLVM_DEBUG(dbgs() << "Unswitching loop <" << ParentLoop->getNumber()
                      << ">:\n");

    // TODO: Implement partial predicate hoisting: if (%b > 50 && i < 50),
    // %b > 50 may be hoisted.
    HLLoop *TargetLoop = PilotIf->getParentLoopAtLevel(Candidate.Level + 1);
    assert(TargetLoop && "Target loop should always exist for the candidate");

    if (ThresholdMap[TargetLoop] >= NumPredicateThreshold) {
      LLVM_DEBUG(dbgs() << "Skipped due to NumPredicateThreshold\n");
      Candidates.pop_back();
      PilotIf->setUnswitchDisabled();
      continue;
    }

    ParentLoopsToInvalidate.insert(ParentLoop);
    TargetLoopsToInvalidate.insert(TargetLoop);

    // TODO: check if candidate is defined in preheader
    TargetLoop->extractZtt();
    TargetLoop->extractPreheader();

    // TransformLoop and its clones.
    transformCandidate(TargetLoop, Candidate, VNum, OptReportVisitedSet,
                       IfVisitedSet);

    LLVM_DEBUG(dbgs() << "While " OPT_DESC ":\n");
    LLVM_DEBUG(ParentLoop->getParentRegion()->dump());
    LLVM_DEBUG(dbgs() << "\n");

    // Calculate statistics
    if (Candidate.PUC.isPURequired()) {
      IfsPUUnswitched++;
    }

    IfsUnswitched++;

    LLVM_DEBUG(dumpCandidates());
  }

  HasMultiexitLoop = false;

  // Mark loop as modified in all the analysis.
  for (HLLoop *Loop : ParentLoopsToInvalidate) {
    if (Loop->isMultiExit()) {
      // It will be used in the caller to update exit counts.
      HasMultiexitLoop = true;
    }

    HIRInvalidationUtils::invalidateBody(Loop);
  }
  for (HLLoop *Loop : TargetLoopsToInvalidate) {
    HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(Loop);
  }

  return !ParentLoopsToInvalidate.empty();
}

// Conservatively add Lval symbase as live-in to the loop.
static void addLvalAsLivein(const RegDDRef *LvalRef, HLLoop *Loop) {
  assert(LvalRef->isLval());
  if (LvalRef->isTerminalRef()) {
    Loop->addLiveInTemp(LvalRef->getSymbase());
  }
}

static bool hasEqualParentIf(HLIf *FromIf, HLLoop *ToLoop) {
  for (HLNode *Node = FromIf->getParent(); Node != ToLoop;
       Node = Node->getParent()) {
    assert(Node && "FromIf should be a child of ToLoop");
    if (HLIf *ParentIf = dyn_cast<HLIf>(Node)) {
      if (HLNodeUtils::areEqual(FromIf, ParentIf)) {
        return true;
      }
    }
  }
  return false;
}

// transformLoop - Perform the OptPredicate transformation for the given loop.
// There will be two loops after the transformation. One loop inside the
// If-Then and other inside the Else.
void HIROptPredicate::transformCandidate(
    HLLoop *TargetLoop, HoistCandidate &Candidate, unsigned &VNum,
    SmallPtrSetImpl<HLLoop *> &OptReportVisitedSet,
    SmallPtrSetImpl<HLIf *> &IfVisitedSet) {

  SmallDenseMap<HLIf *, std::pair<HLContainerTy, HLContainerTy>, 8> Containers;
  SmallPtrSet<HLNode *, 32> TrackClonedNodes;

  // Find other equivalent candidates that may be handled under the same
  // TargetLoop. We only need to hoist PilotIf as the others are equivalent to
  // being sibling ifs inside the same loop.

  std::function<bool(const HoistCandidate &)> IsEquivCandidate =
      [TargetLoop, &Candidate](const HoistCandidate &C) {
        if (C == Candidate) {
          return true;
        }

        bool IsEquiv = C.Level == Candidate.Level &&
                       // Have same condition
                       HLNodeUtils::areEqual(C.getIf(), Candidate.getIf()) &&
                       // And are in the same target loop
                       HLNodeUtils::contains(TargetLoop, C.getIf(), false);
        if (!IsEquiv) {
          return false;
        }

        // Can not handle candidate if there is an equal parent If.
        return !hasEqualParentIf(C.getIf(), TargetLoop);
      };

  auto EquivCandidatesI = std::stable_partition(
      Candidates.begin(), Candidates.end(), std::not1(IsEquivCandidate));

  for (auto Iter = EquivCandidatesI, E = Candidates.end(); Iter != E; ++Iter) {
    LLVM_DEBUG(dbgs() << "H: ");
    LLVM_DEBUG(Iter->dump());
    LLVM_DEBUG(dbgs() << "\n");

    // Disable further unswitching in case of pass will be called more than
    // once.
    Iter->getIf()->setUnswitchDisabled();

    // Set HLIfs that will be tracked during the cloning.
    TrackClonedNodes.insert(Iter->getIf());
  }

  for (auto &C : Candidates) {
    // Also track the condition definition instructions.
    TrackClonedNodes.insert(C.PUC.getInstructions().begin(),
                            C.PUC.getInstructions().end());
  }

  Containers.reserve(std::distance(EquivCandidatesI, Candidates.end()));

  // Remove the Then and Else block as they will be inserted after cloning.
  // This is done to avoid cloning of children when creating the new loop.
  for (auto Iter = EquivCandidatesI, E = Candidates.end(); Iter != E; ++Iter) {
    HoistCandidate &C = *Iter;
    auto &Pair = Containers[C.getIf()];
    removeThenElseChildren(C.getIf(), &Pair.first, &Pair.second);
  }

  // Create the else loop by cloning the main loop.
  LoopUnswitchNodeMapper CloneMapper(TrackClonedNodes, Candidates);
  HLLoop *NewElseLoop = TargetLoop->clone(&CloneMapper);

  HLNodeUtils::insertAfter(TargetLoop, NewElseLoop);

  ThresholdMap[NewElseLoop] = ++ThresholdMap[TargetLoop];

  HLIf *PivotIf = nullptr;

  HLIf *FirstIf = Candidate.getIf();
  HLIf *FirstIfClone = CloneMapper.getMapped(FirstIf);

  // if (...) {  << PivotIf
  //   DO i1     << TargetLoop
  //     if(...) << FirstIf (Original)
  //     if(...)
  //     if(...)
  //   END DO
  // } else {
  //   DO i1     << NewElseLoop
  //     if(...) << FirstIf (Clone)
  //     if(...)
  //     if(...)
  //   END DO
  // }

  // Unswitch every equivalent candidate - a pair of HLIfs. First we handle
  // original If and then its clone. removeOrHoistIf() will move HLIf before
  // the loops and make it a pivot or will remove it as needed.
  for (auto Iter = EquivCandidatesI, E = Candidates.end(); Iter != E; ++Iter) {
    HoistCandidate &C = *Iter;
    HLIf *If = C.getIf();
    auto &Pair = Containers[If];
    auto &ThenContainer = Pair.first;
    auto &ElseContainer = Pair.second;

    // Handle TargetLoop first

    if (C.PUC.IsUpdatedInThenBranch) {
      // Place original HLIf block
      HLNodeUtils::insertAsFirstThenChildren(If, &ThenContainer);
      HLNodeUtils::insertAsFirstElseChildren(If, &ElseContainer);
    } else {
      // Place *then* branch unconditionally.
      if (!ThenContainer.empty()) {
        HLContainerTy *ThenContainerPtr = &ThenContainer;
        HLContainerTy CloneContainer;
        if (C.PUC.IsUpdatedInElseBranch) {
          // If true then nodes will be needed for second loop.
          HLNodeUtils::cloneSequence(&CloneContainer, &ThenContainer.front(),
                                     &ThenContainer.back());
          ThenContainerPtr = &CloneContainer;
        }

        HLNodeUtils::insertAfter(If, ThenContainerPtr);
      }

      removeOrHoistIf(C, TargetLoop, FirstIf, If, PivotIf);

      for (HLInst *RedundantInst : C.PUC.getInstructions()) {
        // Remove instruction if it's still attached. May be removed by
        // another candidate.
        if (RedundantInst->isAttached()) {
          addLvalAsLivein(RedundantInst->getLvalDDRef(), TargetLoop);
          HLNodeUtils::remove(RedundantInst);
        }
      }
    }

    addPredicateOptReport(TargetLoop, If, OptReportVisitedSet, IfVisitedSet,
                          VNum);

    // Handle second loop - NewElseLoop

    if (!ThenContainer.empty()) {
      HIRTransformUtils::remapLabelsRange(CloneMapper, &ThenContainer.front(),
                                          &ThenContainer.back());
    }

    if (ElseContainer.empty() && If->hasElseChildren()) {
      // If true than ElseContainer was used for the first loop and we will
      // need it for the second loop.
      HLNodeUtils::cloneSequence(&ElseContainer, If->getFirstElseChild(),
                                 If->getLastElseChild());
    }

    if (!ElseContainer.empty()) {
      HIRTransformUtils::remapLabelsRange(CloneMapper, &ElseContainer.front(),
                                          &ElseContainer.back());
    }

    HLIf *ClonedIf = CloneMapper.getMapped(If);
    assert(ClonedIf && "Can not get mapped If");
    if (C.PUC.IsUpdatedInElseBranch) {
      // Place original HLIf block
      HLNodeUtils::insertAsFirstThenChildren(ClonedIf, &ThenContainer);
      HLNodeUtils::insertAsFirstElseChildren(ClonedIf, &ElseContainer);
    } else {
      // Place *else* branch unconditionally.
      if (!ElseContainer.empty()) {
        HLNodeUtils::insertAfter(ClonedIf, &ElseContainer);
      }

      removeOrHoistIf(C, TargetLoop, FirstIfClone, ClonedIf, PivotIf);

      for (HLInst *RedundantInst : C.PUC.getInstructions()) {
        // Remove instruction if it's still attached. May be removed by
        // another candidate.
        HLInst *RedundantInstClone = CloneMapper.getMapped(RedundantInst);
        if (RedundantInstClone && RedundantInstClone->isAttached()) {
          addLvalAsLivein(RedundantInstClone->getLvalDDRef(), NewElseLoop);
          HLNodeUtils::remove(RedundantInstClone);
        }
      }
    }

    addPredicateOptReport(NewElseLoop, If, OptReportVisitedSet, IfVisitedSet,
                          VNum);
  }

  assert(PivotIf && "should be defined");

  // Allow further unswitching of the top *If* statement.
  PivotIf->setUnswitchDisabled(false);

  // Place TargetLoop and NewElseLoop under PivotIf.
  HLNodeUtils::moveAsFirstThenChild(PivotIf, TargetLoop);
  HLNodeUtils::moveAsFirstElseChild(PivotIf, NewElseLoop);

  Candidates.erase(EquivCandidatesI, Candidates.end());

  // Append new candidates.
  if (!CloneMapper.getNewCandidates().empty()) {
    Candidates.append(CloneMapper.getNewCandidates().begin(),
                      CloneMapper.getNewCandidates().end());
    sortCandidates();
  }
}

void HIROptPredicate::removeThenElseChildren(HLIf *If,
                                             HLContainerTy *ThenContainer,
                                             HLContainerTy *ElseContainer) {

  // Collect Then Children.
  if (If->hasThenChildren()) {
    HLNodeUtils::remove(ThenContainer, If->getFirstThenChild(),
                        If->getLastThenChild());
  }

  // Collect Else Children.
  if (If->hasElseChildren()) {
    HLNodeUtils::remove(ElseContainer, If->getFirstElseChild(),
                        If->getLastElseChild());
  }
}

// The method is called for a series of HLIfs with the same conditions and
// same TargetLoop. FirstIf is the first HLIf in the series. PivotIf is a
// reference to HLIf * that is used to store hoisted HLIf instance.
void HIROptPredicate::removeOrHoistIf(HoistCandidate &Candidate,
                                      HLLoop *TargetLoop, HLIf *FirstIf,
                                      HLIf *If, HLIf *&PivotIf) {
  if (!PivotIf && If == FirstIf) {
    // Move the If condition outside.

    SmallVector<HLInst *, 8> DefInstructions(
        Candidate.PUC.getInstructions().begin(),
        Candidate.PUC.getInstructions().end());

    std::sort(DefInstructions.begin(), DefInstructions.end(),
              [](const HLInst *A, const HLInst *B) {
                return A->getTopSortNum() < B->getTopSortNum();
              });

    for (HLInst *DefInst : DefInstructions) {
      HLNodeUtils::insertBefore(TargetLoop, DefInst->clone());
    }

    hoistIf(FirstIf, TargetLoop);
    PivotIf = If;
  } else {
    HLNodeUtils::remove(If);
  }
}

void HIROptPredicate::hoistIf(HLIf *&If, HLLoop *OrigLoop) {
  // TODO: remove loop live-ins

  // Hoist the If outside the loop.
  HLNodeUtils::moveBefore(OrigLoop, If);

  unsigned Level = OrigLoop->getNestingLevel();

  // Update the DDRefs inside the HLIf.
  for (auto Ref : make_range(If->ddref_begin(), If->ddref_end())) {
    Ref->updateDefLevel(Level - 1);
  }
}

void HIROptPredicate::addPredicateOptReport(
    HLLoop *TargetLoop, HLIf *If,
    SmallPtrSetImpl<HLLoop *> &OptReportVisitedSet,
    SmallPtrSetImpl<HLIf *> &IfVisitedSet, unsigned &VNum) {

  LoopOptReportBuilder &LORBuilder =
      TargetLoop->getHLNodeUtils().getHIRFramework().getLORBuilder();

  bool IsReportOn = LORBuilder.isLoopOptReportOn();

  if (!IsReportOn) {
    return;
  }

  if (!OptReportVisitedSet.count(TargetLoop)) {
    LORBuilder(*TargetLoop).addOrigin("Predicate Optimized v%d", VNum);
    VNum++;
    OptReportVisitedSet.insert(TargetLoop);
  }

  if (IfVisitedSet.count(If)) {
    return;
  }

  SmallString<32> IfNum;
  unsigned LineNum;
  raw_svector_ostream VOS(IfNum);

  if (If->getDebugLoc()) {
    LineNum = If->getDebugLoc().getLine();
    VOS << " at line ";
    VOS << LineNum;
  }

  LORBuilder(*TargetLoop)
      .addRemark(OptReportVerbosity::Low,
                 "Invariant Condition%s hoisted out of this loop", IfNum);
  IfVisitedSet.insert(If);
}

PreservedAnalyses HIROptPredicatePass::run(llvm::Function &F,
                                           llvm::FunctionAnalysisManager &AM) {
  HIROptPredicate(AM.getResult<HIRFrameworkAnalysis>(F),
                  AM.getResult<HIRDDAnalysisPass>(F), EnablePartialUnswitch,
                  KeepLoopnestPerfect)
      .run();

  return PreservedAnalyses::all();
}

class HIROptPredicateLegacyPass : public HIRTransformPass {
  bool EnablePartialUnswitch;
  bool KeepLoopnestPerfect;

public:
  static char ID;

  HIROptPredicateLegacyPass(bool EnablePartialUnswitch = true,
                            bool KeepLoopnestPerfect = false)
      : HIRTransformPass(ID), EnablePartialUnswitch(EnablePartialUnswitch),
        KeepLoopnestPerfect(KeepLoopnestPerfect) {
    initializeHIROptPredicateLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRDDAnalysisWrapperPass>();
    // Loop Statistics is not used by this pass directly but it used by
    // HLNodeUtils::dominates() utility. This is a workaround to keep the pass
    // manager from freeing it.
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
  }

  bool runOnFunction(Function &F) {
    if (skipFunction(F)) {
      return false;
    }

    return HIROptPredicate(getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
                           getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
                           EnablePartialUnswitch, KeepLoopnestPerfect)
        .run();
  }
};

char HIROptPredicateLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIROptPredicateLegacyPass, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIROptPredicateLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIROptPredicatePass(bool EnablePartialUnswitch,
                                              bool KeepLoopnestPerfect) {
  return new HIROptPredicateLegacyPass(EnablePartialUnswitch,
                                       KeepLoopnestPerfect);
}
