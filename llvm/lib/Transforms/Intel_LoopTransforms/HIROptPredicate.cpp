//===--- HIROptPredicate.cpp - Implements OptPredicate class --------------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#define OPT_SWITCH "hir-opt-predicate"
#define OPT_DESC "HIR OptPredicate"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

STATISTIC(IfsUnswitched,
          "Number of HLIfs optimized (hoisted)");
// This will also count if the loop has been analyzed multiple times
// due to nested/multiple if.
STATISTIC(IfsAnalyzed, "Number of HLIfs analyzed for loop unswitching");

LLVM_CONSTEXPR unsigned DefaultNumPredicateThreshold = 3;

static cl::opt<unsigned> NumPredicateThreshold(
    OPT_SWITCH "-threshold", cl::init(DefaultNumPredicateThreshold), cl::Hidden,
    cl::desc("Don't opt predicate a loop which has been transformed greater"
             "than this threshold."));

static cl::opt<bool> DisableLoopUnswitch(
    "disable-" OPT_SWITCH, cl::init(false), cl::Hidden,
    cl::desc("Disable HIR Loop unswitching"));

static cl::opt<bool> DisableCostModel(
    "disable-" OPT_SWITCH "-cost-model", cl::init(false), cl::Hidden,
    cl::desc("Disable " OPT_DESC " cost model related checks"));

namespace {

/// Prints \p If header during the optimization process.
#ifndef NDEBUG
LLVM_DUMP_METHOD
static void dumpIf(const HLIf *If) {
  formatted_raw_ostream OS(dbgs());
  OS << "<" << If->getNumber() << "> ";
  If->printHeader(OS, 0);
}
#endif

struct HoistCandidate {
  HLIf *If;
  unsigned Level;
  SmallVector<HLIf *, 8> Clones;

  HoistCandidate(HLIf *If, unsigned Level) : If(If), Level(Level) {}
  HoistCandidate() : If(nullptr), Level(0) {}

#ifndef NDEBUG
  LLVM_DUMP_METHOD
  void dump() {
    dbgs() << "{";
    dumpIf(If);
    dbgs() << ", L: " << Level << ", [ ";
    for (HLIf *Clone : Clones) {
      dumpIf(Clone);
      dbgs() << " ";
    }
    dbgs() << "]}";
  }
#endif
};

class HIROptPredicate : public HIRTransformPass {
public:
  static char ID;

  HIROptPredicate() : HIRTransformPass(ID) {
    initializeHIROptPredicatePass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFramework>();
  }

private:
  struct CandidateLookup;
  class LoopUnswitchNodeMapper;

  SmallDenseMap<const HLLoop *, unsigned, 16> ThresholdMap;
  SmallVector<HoistCandidate, 16> Candidates;
  SmallDenseMap<const HLIf *, const HLIf *, 16> CloneOriginals;

  /// Sorts candidates in top sort number descending.
  /// They will be handled from right to left.
  void sortCandidates();

  /// \brief This routine will loop through the candidate loop to look
  /// for HLIf candidates. If all conditions are met, it will transform
  /// the loop. Returns true if transformation happened.
  bool processOptPredicate();

  /// This routine involves checking if the loop is supported by this
  /// transformation.
  bool isLoopSupported(const HLLoop *Loop) const;

  /// Returns the deepest level at which any of the If operands is defined.
  static unsigned getDefinedAtLevel(const HLIf *If);

  /// Searches HLLoop closest to \p Level which is suitable for un-switching of
  /// \p If.
  HLLoop *findTargetLoopAtLevel(const HLIf *If, HLLoop *ParentLoop,
                                unsigned Level) const;

  /// Extracts HLIf body and removes the node.
  void replaceWithBody(HLIf *If, bool ThenBody);

  /// Transform the original loop.
  void transformCandidate(HLLoop *TargetLoop, HoistCandidate &Candidate);

  /// Transform loop clones and create new candidate.
  bool transformClones(HLLoop *TargetLoop, HoistCandidate &Candidate,
                       HoistCandidate &NewCandidate);

  /// \brief Removes the Then and Else children of HLIf and stores them
  /// in the container.
  void removeThenElseChildren(HLIf *If, HLContainerTy *ThenContainer,
                              HLContainerTy *ElseContainer);

  /// Performs the OptPredicate transformation where condition is
  /// hoisted outside and loops are moved inside the condition and updates the
  /// DDRefs inside the HLIf.
  void hoistIf(HLIf *If, HLLoop *OrigLoop);

#ifndef NDEBUG
  LLVM_DUMP_METHOD
  void dumpCandidates() {
    for (HoistCandidate &Candidate : Candidates) {
      Candidate.dump();
      dbgs() << "\n";
    }
  }
#endif
};

class HIROptPredicate::LoopUnswitchNodeMapper : public HLNodeToNodeMapperImpl {
  const HLIf *If;

  decltype(Candidates) &CandidatesRef;
  decltype(CloneOriginals) &CloneOriginalsRef;

public:
  LoopUnswitchNodeMapper(const HLIf *If, decltype(Candidates) &CandidatesRef,
                         decltype(CloneOriginals) &CloneOriginalsRef)
      : If(If), CandidatesRef(CandidatesRef),
        CloneOriginalsRef(CloneOriginalsRef) {}

  void map(const HLNode *Node, HLNode *MappedNode) override {
    if (Node == If || isa<HLLabel>(Node)) {
      NodeMap[Node] = MappedNode;
      return;
    }

    // Maintain CloneOriginal map and populate Candidates clone map.
    if (const HLIf *If = dyn_cast<HLIf>(Node)) {
      auto OriginalI = CloneOriginalsRef.find(If);
      if (OriginalI == CloneOriginalsRef.end()) {
        return;
      }

      HLIf *CloneIf = cast<HLIf>(MappedNode);
      const HLIf *OriginalIf = OriginalI->getSecond();
      auto CandidateI =
          std::find_if(CandidatesRef.begin(), CandidatesRef.end(),
                       [OriginalIf](const HoistCandidate &Candidate) {
                         return Candidate.If == OriginalIf;
                       });

      // Original HLIf was found as a candidate, this means that we are still
      // interested in this node - have to save original node and update
      // candidate clone list.
      if (CandidateI != CandidatesRef.end()) {
        CloneOriginalsRef[CloneIf] = OriginalIf;
        CandidateI->Clones.push_back(CloneIf);
      }
    }
  }
};

struct HIROptPredicate::CandidateLookup final : public HLNodeVisitorBase {
  HLNode *SkipNode;
  HIROptPredicate &Pass;
  bool HasLabel;
  unsigned MinLevel;

  CandidateLookup(HIROptPredicate &Pass, unsigned MinLevel = 0)
      : SkipNode(nullptr), Pass(Pass), HasLabel(false), MinLevel(MinLevel) {}

  bool isCandidate(const RegDDRef *Ref) const;

  void visit(HLIf *If);
  void visit(HLLoop *Loop);
  void visit(const HLLabel *) { HasLabel = true; }

  bool skipRecursion(const HLNode *Node) const override {
    return Node == SkipNode;
  }

  void visit(const HLNode *Node) {}
  void postVisit(const HLNode *Node) {}
};

}

bool HIROptPredicate::CandidateLookup::isCandidate(const RegDDRef *Ref) const {
  // Only handle scalar references.
  if (!Ref->isTerminalRef()) {
    return false;
  }

  const CanonExpr *CE = Ref->getSingleCanonExpr();
  if (CE->isNonLinear()) {
    return false;
  }

  return true;
}

void HIROptPredicate::CandidateLookup::visit(HLIf *If) {
  SkipNode = If;

  HLLoop *ParentLoop = If->getParentLoop();
  if (!ParentLoop) {
    return;
  }

  // TODO: Think about erasing If when no children are present.
  // Usually, this will be safe when there are no call statements
  // associated with a predicate. Ignoring such If for now.
  if (!If->hasThenChildren() && !If->hasElseChildren()) {
    return;
  }

  bool IsCandidate = Pass.isLoopSupported(ParentLoop);

  if (!DisableCostModel) {
    if (!ParentLoop->isInnermost()) {
      IsCandidate = false;
    } else if (Pass.ThresholdMap[ParentLoop] >= NumPredicateThreshold) {
      IsCandidate = false;
    }
  }

  // Loop through predicates to check if they satisfy opt predicate
  // conditions.
  if (IsCandidate) {
    for (auto Iter = If->pred_begin(), E = If->pred_end(); Iter != E; ++Iter) {
      const RegDDRef *LHSRef = If->getPredicateOperandDDRef(Iter, true);
      const RegDDRef *RHSRef = If->getPredicateOperandDDRef(Iter, false);

      // Check if both DDRefs satisfy all the conditions.
      if (!isCandidate(LHSRef) || !isCandidate(RHSRef)) {
        IsCandidate = false;
        break;
      }
    }
  }

  // Skip candidates that are already at the outer most possible level.
  unsigned Level;

  if (IsCandidate) {
    Level = std::max(getDefinedAtLevel(If), MinLevel);
    if (Level == ParentLoop->getNestingLevel()) {
      IsCandidate = false;
    }
  } else {
    Level = std::max(ParentLoop->getNestingLevel(), MinLevel);
  }

  CandidateLookup Lookup(Pass, Level);
  HLNodeUtils::visitRange(Lookup, If->then_begin(), If->then_end());
  HLNodeUtils::visitRange(Lookup, If->else_begin(), If->else_end());
  if (!IsCandidate || Lookup.HasLabel) {
    return;
  }

  DEBUG(dbgs() << "Opportunity: ");
  DEBUG(dumpIf(If));
  DEBUG(dbgs() << " --> Level " << Level << "\n");

  Pass.Candidates.emplace_back(If, Level);

  // Setup identity mapping
  Pass.CloneOriginals[If] = If;

  Pass.ThresholdMap[ParentLoop]++;
}

void HIROptPredicate::CandidateLookup::visit(HLLoop *Loop) {
  SkipNode = Loop;

  CandidateLookup Lookup(Pass, MinLevel);
  HLNodeUtils::visitRange(Lookup, Loop->child_begin(), Loop->child_end());
}

char HIROptPredicate::ID = 0;
INITIALIZE_PASS_BEGIN(HIROptPredicate, OPT_SWITCH, OPT_DESC,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_END(HIROptPredicate, OPT_SWITCH, OPT_DESC,
                    false, false)

FunctionPass *llvm::createHIROptPredicatePass() {
  return new HIROptPredicate;
}

void HIROptPredicate::sortCandidates() {
  std::sort(Candidates.begin(), Candidates.end(),
            [](const HoistCandidate &A, const HoistCandidate &B) {
              return A.If->getTopSortNum() > B.If->getTopSortNum();
            });
}

bool HIROptPredicate::runOnFunction(Function &F) {
  if (skipFunction(F) || DisableLoopUnswitch) {
    return false;
  }

  DEBUG(dbgs() << "Opt Predicate for Function: " << F.getName() << "\n");

  HIRFramework &HIR = getAnalysis<HIRFramework>();
  for (HLNode &Node : make_range(HIR.hir_begin(), HIR.hir_end())) {
    HLRegion *Region = cast<HLRegion>(&Node);

    CandidateLookup Lookup(*this);
    HLNodeUtils::visit(Lookup, Region);
    sortCandidates();

    if (processOptPredicate()) {
      Region->setGenCode();
    }

    Candidates.clear();
    CloneOriginals.clear();
    ThresholdMap.clear();
  }

  return false;
}

void HIROptPredicate::releaseMemory() {}

unsigned HIROptPredicate::getDefinedAtLevel(const HLIf *If) {
  unsigned Level = 0;
  for (auto PI = If->pred_begin(), PE = If->pred_end(); PI != PE; ++PI) {
    CanonExpr *CEs[] = {
        If->getPredicateOperandDDRef(PI, true)->getSingleCanonExpr(),
        If->getPredicateOperandDDRef(PI, false)->getSingleCanonExpr()};

    for (const CanonExpr *CE : CEs) {
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

      Level = std::max(Level, IVMaxLevel);
      Level = std::max(Level, CE->getDefinedAtLevel());
    }
  }
  return Level;
}

HLLoop *HIROptPredicate::findTargetLoopAtLevel(const HLIf *If,
                                               HLLoop *ParentLoop,
                                               unsigned Level) const {
  assert(isLoopSupported(ParentLoop) && "Unsupported loop in candidates.");
  assert(Level < ParentLoop->getNestingLevel() &&
         "HLIf is somehow defined on deeper levels than parent's loop or "
         "HLIf added as a candidate, but the proper level is equal to the "
         "nesting level of the parent loop");

  while(Level < ParentLoop->getNestingLevel() - 1) {
    HLLoop *TargetLoop = ParentLoop->getParentLoop();

    ParentLoop = TargetLoop;
    assert(ParentLoop && "There always should be a parent loop");

    if (!isLoopSupported(TargetLoop)) {
      break;
    }
  }

  return ParentLoop;
}

/// processOptPredicate - Main routine to perform opt predicate transformation.
bool HIROptPredicate::processOptPredicate() {
  SmallPtrSet<HLLoop *, 8> ParentLoopsToInvalidate;
  SmallPtrSet<HLLoop *, 8> TargetLoopsToInvalidate;

  HoistCandidate NewCandidate;

  bool HasNewCandidate = false;

  while (!Candidates.empty()) {
    HoistCandidate &Candidate = Candidates.back();

    HLIf *If = Candidate.If;

    DEBUG(dbgs() << "Hoisting: ");
    DEBUG(Candidate.dump());
    DEBUG(dbgs() << "\n");

    IfsAnalyzed++;

    HLLoop *ParentLoop = If->getParentLoop();
    assert(ParentLoop && "Candidate should have a parent loop");

    // TODO: Implement partial hoisting: if (%b > 50 && i < 50),
    // %b > 50 may be hoisted.
    HLLoop *TargetLoop = findTargetLoopAtLevel(If, ParentLoop, Candidate.Level);
    assert(TargetLoop && "Target loop should always exist for the candidate");

    if (!HasNewCandidate) {
      ParentLoopsToInvalidate.insert(ParentLoop);
      TargetLoopsToInvalidate.insert(TargetLoop);
    }

    // TODO: check if candidate is defined in preheader
    TargetLoop->extractPreheader();

    // TransformLoop and its clones.
    transformCandidate(TargetLoop, Candidate);
    HasNewCandidate = transformClones(TargetLoop, Candidate, NewCandidate);

    DEBUG(dbgs() << "While " OPT_DESC ":\n");
    DEBUG(ParentLoop->getParentRegion()->dump());
    DEBUG(dbgs() << "\n");

    IfsUnswitched++;

    Candidates.pop_back();
    if (HasNewCandidate) {
      Candidates.push_back(NewCandidate);
    }

    DEBUG(dumpCandidates());
  }

  // Mark loop as modified in all the analysis.
  for (HLLoop *Loop : ParentLoopsToInvalidate) {
    HIRInvalidationUtils::invalidateBody(Loop);
  }
  for (HLLoop *Loop : TargetLoopsToInvalidate) {
    HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(Loop);
  }

  return !ParentLoopsToInvalidate.empty();
}

bool HIROptPredicate::isLoopSupported(const HLLoop *Loop) const {

  if (!Loop->isDo()) {
    return false;
  }

  return true;
}

void HIROptPredicate::replaceWithBody(HLIf *If, bool ThenBody) {
  if (ThenBody) {
    if (If->hasThenChildren()) {
      HLNodeUtils::moveAfter(If, If->then_begin(), If->then_end());
    }
  } else {
    if (If->hasElseChildren()) {
      HLNodeUtils::moveAfter(If, If->else_begin(), If->else_end());
    }
  }
  HLNodeUtils::remove(If);
}

// transformLoop - Perform the OptPredicate transformation for the given loop.
// There will be two loops after the transformation. One loop inside the
// If-Then and other inside the Else.
void HIROptPredicate::transformCandidate(HLLoop *TargetLoop,
                                         HoistCandidate &Candidate) {
  HLIf *If = Candidate.If;

  // Remove the Then and Else block as they will be inserted after cloning. This
  // is done to avoid cloning of children when creating the new loop.
  HLContainerTy ThenContainer;
  HLContainerTy ElseContainer;
  removeThenElseChildren(If, &ThenContainer, &ElseContainer);

  LoopUnswitchNodeMapper CloneMapper(If, Candidates, CloneOriginals);

  // Create the else loop by cloning the main loop.
  HLLoop *NewElseLoop = TargetLoop->clone(&CloneMapper);
  HLIf *ClonnedIf = CloneMapper.getMapped(If);

  // Insert then-case after the HLIf
  if (!ThenContainer.empty()) {
    HLNodeUtils::insertAfter(If, &ThenContainer);
  }
  // Insert else-case afther the cloned HLIf
  if (!ElseContainer.empty()) {
    // Update HLGotos to new cloned targets
    HLNodeUtils::remapLabelsRange(CloneMapper, &ElseContainer.front(),
                                  &ElseContainer.back());
    HLNodeUtils::insertAfter(ClonnedIf, &ElseContainer);
  }

  // Move the If condition outside.
  hoistIf(If, TargetLoop);
  HLNodeUtils::remove(ClonnedIf);

  HLNodeUtils::moveAsFirstChild(If, TargetLoop, true);
  HLNodeUtils::insertAsFirstChild(If, NewElseLoop, false);
}

bool HIROptPredicate::transformClones(HLLoop *TargetLoop,
                                      HoistCandidate &Candidate,
                                      HoistCandidate &NewCandidate) {
  HoistCandidate *NewCandidatePtr = nullptr;

  for (HLIf *Clone : Candidate.Clones) {
    if (HLNodeUtils::contains(Candidate.If, Clone, false)) {
      replaceWithBody(Clone, Candidate.If->isThenChild(Clone));
    } else {
      if (!NewCandidatePtr) {
        DEBUG(dbgs() << "Found new candidate: ");
        DEBUG(dumpIf(Clone));
        DEBUG(dbgs() << "\n");

        NewCandidate = std::move(HoistCandidate(Clone, Candidate.Level));
        NewCandidatePtr = &NewCandidate;
      } else {
        // For all clones set the original node.
        CloneOriginals[Clone] = NewCandidate.If;
        NewCandidate.Clones.push_back(Clone);
      }
    }
  }

  return NewCandidatePtr != nullptr;
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

void HIROptPredicate::hoistIf(HLIf *If, HLLoop *OrigLoop) {
  //TODO: remove loop live-ins

  // Hoist the If outside the loop.
  HLNodeUtils::moveBefore(OrigLoop, If);

  // Update the DDRefs inside the HLIf.
  for (auto Ref : make_range(If->ddref_begin(), If->ddref_end())) {
    Ref->updateDefLevel();
  }
}
