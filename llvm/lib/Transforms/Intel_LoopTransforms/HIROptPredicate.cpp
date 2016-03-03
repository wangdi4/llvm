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
// 5. Hoist the If condition to the outermost loop as possible. Currently, it
//    will only hoist them outside the innermost loop.
// 6. Add opt report messages.

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Analysis/Intel_LoopAnalysis/DDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRFramework.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

#define DEBUG_TYPE "hir-opt-predicate"

using namespace llvm;
using namespace llvm::loopopt;

STATISTIC(LoopsOptPredicated,
          "Number of HIR loops if optimized (opt predicated)");
// This will also count if the loop has been analyzed multiple times
// due to nested/multiple if.
STATISTIC(LoopsAnalyzed, "Number of HIR loops analyzed for opt predicate");

static cl::opt<unsigned> OptPredTripThreshold(
    "optpred-trip-threshold", cl::init(4), cl::Hidden,
    cl::desc("Don't opt predicate a loop which has been transformed greater"
             "than this threshold."));

namespace {

class HIROptPredicate : public HIRTransformPass {
public:
  static char ID;

  HIROptPredicate(int T = -1) : HIRTransformPass(ID) {
    initializeHIROptPredicatePass(*PassRegistry::getPassRegistry());

    CurOptPredTripThreshold = (T == -1) ? OptPredTripThreshold : unsigned(T);
  }

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFramework>();
    AU.addRequiredTransitive<DDAnalysis>();
  }

private:
  // DD Analysis pointer.
  DDAnalysis *DD;
  unsigned CurOptPredTripThreshold;
  bool IsOptPredTriggered;
  SmallVector<HLLoop *, 64> CandidateLoops;

  // Maintains the number of times a loop was opt predicated.
  // We want to opt predicate only certain times to prevent extremely large code
  // size.
  SmallDenseMap<const HLLoop *, unsigned, 64> LoopPredTripMap;

  /// \brief Determines if the HLIf is a candidate for OptPred transformation.
  /// If this routine finds such as case, it will set the return parameter
  /// and return true. This is an internal helper function used by isSupported.
  bool analyzeIf(const HLLoop *Loop, HLIf **CandidateIf, unsigned *IfPos) const;

  /// \brief This routine will loop through the candidate loop to look
  /// for HLIf candidates. If all conditions are met, it will transform
  /// the loop.
  void processOptPredicate();

  /// \brief Determines if the loop is profitable for transformation.
  /// This check if the loops has not been transformed too many times.
  bool isProfitable(const HLLoop *Loop);

  /// \brief Determines if all the conditions are met for an If inside the
  /// loop. If such an HLIf exist, it will set it in the parameter and
  /// return true. This routine involves checking if the loop and 'if' are
  /// supported by this transformation.
  bool isSupported(const HLLoop *Loop, HLIf **CandidateIf,
                   unsigned *IfPos) const;

  /// \brief Returns true if the loop is opt predicated trips
  /// is greater than threshold.
  bool exceedPredCount(const HLLoop *Loop);

  /// \brief Main routine to transform the loop for opt predicate.
  void transformLoop(HLLoop *Loop, HLIf *If, unsigned IfPos);

  /// \brief Create the new then loop for the HLIf when OptPredicate occurs.
  HLLoop *createThenLoop(HLLoop *OrigLoop, HLIf *If);

  /// \brief Inserts the Then Children before the If for transformation.
  void insertThenChildren(HLIf *If, HLContainerTy *ThenContainer);

  /// \brief Inserts the Else children of HLIf before the condition during
  /// OptPredicate transformation. This will search for the HLIf using IfPos
  /// in the cloned loop.
  void insertElseChildren(HLLoop *NewElseLoop, unsigned IfPos,
                          HLContainerTy *ElseContainer);

  /// \brief Removes the Then and Else children of HLIf and stores them
  /// in the container.
  void removeThenElseChildren(HLIf *If, HLContainerTy *ThenContainer,
                              HLContainerTy *ElseContainer);

  /// \brief Updates the DDRefs inside the HLIf. The updates are specifically
  /// to mark the CE inside DDRef as non-linear during the transformation.
  void updateIfRef(HLIf *If);

  /// \brief Performs the OptPredicate transformation where condition is
  /// hoisted outside and loops are moved inside the condition.
  void hoistIf(HLIf *If, HLLoop *OrigLoop, HLLoop *NewThenLoop);
};
}

char HIROptPredicate::ID = 0;
INITIALIZE_PASS_BEGIN(HIROptPredicate, "hir-opt-predicate", "HIR OptPredicate",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFramework)
INITIALIZE_PASS_DEPENDENCY(DDAnalysis)
INITIALIZE_PASS_END(HIROptPredicate, "hir-opt-predicate", "HIR OptPredicate",
                    false, false)

FunctionPass *llvm::createHIROptPredicatePass(int Threshold) {
  return new HIROptPredicate(Threshold);
}

bool HIROptPredicate::runOnFunction(Function &F) {
  DEBUG(dbgs() << "Opt Predicate for Function : " << F.getName() << "\n");

  DD = &getAnalysis<DDAnalysis>();
  IsOptPredTriggered = false;

  // Gather the innermost loops as candidates.
  HLNodeUtils::gatherInnermostLoops(CandidateLoops);

  processOptPredicate();

  return IsOptPredTriggered;
}

void HIROptPredicate::releaseMemory() {
  CandidateLoops.clear();
  LoopPredTripMap.clear();
}

/// processOptPredicate - Main routine to perform opt predicate transformation.
void HIROptPredicate::processOptPredicate() {

  // Visit each candidate loop to run cost analysis.
  // Note, end is not stored as CandidateLoops gets updated for
  // nested/multiple if.
  for (unsigned Index = 0; Index != CandidateLoops.size(); ++Index) {

    HLLoop *Loop = CandidateLoops[Index];
    DEBUG(dbgs() << "Opt Pred Visiting Loop:\n");
    DEBUG(Loop->dump());

    // Perform a cost/profitability analysis on the loop
    // If all conditions are met, apply opt predicate to it.
    HLIf *CandidateIf = nullptr;
    // IfPosition is used to track the location of CandidateIf, which
    // will be used later during transformation of loop.
    unsigned CandidateIfPos = 0;
    if (isProfitable(Loop) &&
        isSupported(Loop, &CandidateIf, &CandidateIfPos)) {
      assert(CandidateIf && "Candidate If is null when opt predicate is "
                            "possible.");
      assert(CandidateIfPos &&
             " Candidate If position should be greater than 0.");
      transformLoop(Loop, CandidateIf, CandidateIfPos);
      IsOptPredTriggered = true;
      LoopsOptPredicated++;
    }
    LoopsAnalyzed++;
  }
}

namespace {
// Visitor used to find a candidate If for OptPredicate.
class IfVisitor final : public HLNodeVisitorBase {

public:
  IfVisitor(unsigned Level)
      : LoopLevel(Level), LabelExist(false), IsOptPredCandidate(false),
        CandidateIf(nullptr), CandidateIfPos(0), NodeCount(0) {}

  void visit(const HLIf *If);
  void visit(const HLLabel *L) {
    LabelExist = true;
    NodeCount++;
  }
  void visit(const HLNode *Node) { NodeCount++; }
  void postVisit(const HLNode *Node) {}
  bool isDone() const override {
    // TODO: When goto/label are supported, return IsOptPredicate.
    return LabelExist;
  }
  bool hasCandidate() const { return (IsOptPredCandidate && !LabelExist); }

  const HLIf *getCandidateIf(unsigned *Pos) {
    if (Pos) {
      *Pos = CandidateIfPos;
    }
    return CandidateIf;
  }

private:
  unsigned LoopLevel;
  bool LabelExist;
  bool IsOptPredCandidate;
  const HLIf *CandidateIf;
  unsigned CandidateIfPos;
  unsigned NodeCount;

  bool isCandidate(const RegDDRef *RegDD) const;
};
}

bool IfVisitor::isCandidate(const RegDDRef *RegDD) const {

  // Only handle scalar references.
  if (!RegDD->isTerminalRef()) {
    return false;
  }

  const CanonExpr *CE = RegDD->getSingleCanonExpr();

  // TODO: Handle IV for If.
  // Currently, handling only invariant cases.
  if (!CE->isInvariantAtLevel(LoopLevel)) {
    return false;
  }

  return true;
}

void IfVisitor::visit(const HLIf *If) {

  NodeCount++;

  DEBUG(dbgs() << "\n Analyzing:");
  DEBUG(If->dump());

  // TODO: Think about erasing If when no children are present.
  // Usually, this will be safe when there are no call statements
  // associated with a predicate. Ignoring such If for now.
  if (!If->hasThenChildren() && !If->hasElseChildren()) {
    return;
  }

  // Candidate If is already found.
  // When labels is supported we can call isDone() once
  // candidate is found.
  if (hasCandidate()) {
    return;
  }

  IsOptPredCandidate = true;
  // Loop through predicates to check if they satisfy opt predicate conditions.
  for (auto Iter = If->pred_begin(), E = If->pred_end(); Iter != E; ++Iter) {

    const RegDDRef *LHSRef = If->getPredicateOperandDDRef(Iter, true);
    const RegDDRef *RHSRef = If->getPredicateOperandDDRef(Iter, false);

    // Check if both DDRefs satisfy all the conditions.
    if (!isCandidate(LHSRef) || !isCandidate(RHSRef)) {
      IsOptPredCandidate = false;
      break;
    }
  }

  if (IsOptPredCandidate) {
    CandidateIf = If;
    CandidateIfPos = NodeCount;
  } else {
    DEBUG(dbgs() << "\n If is not a candidate");
  }
}

bool HIROptPredicate::analyzeIf(const HLLoop *Loop, HLIf **CandidateIf,
                                unsigned *IfPos) const {

  IfVisitor IfVisit(Loop->getNestingLevel());
  HLNodeUtils::visit(IfVisit, Loop);
  if (IfVisit.hasCandidate()) {
    *CandidateIf = const_cast<HLIf *>(IfVisit.getCandidateIf(IfPos));
    return true;
  }

  return false;
}

bool HIROptPredicate::exceedPredCount(const HLLoop *Loop) {

  auto Iter = LoopPredTripMap.find(Loop);

  // Loop is visited first time.
  if (Iter == LoopPredTripMap.end()) {
    LoopPredTripMap[Loop] = 0;
    return false;
  }

  // Loop exceeds threshold.
  if ((Iter->second) > CurOptPredTripThreshold) {
    return true;
  }

  return false;
}

bool HIROptPredicate::isProfitable(const HLLoop *Loop) {

  // Check if loop has been opt predicated meets the threshold
  // number of times.
  return !exceedPredCount(Loop);
}

// isSupported - Check if the loop has an if which is supported for
// opt predicate transformation.
bool HIROptPredicate::isSupported(const HLLoop *Loop, HLIf **CandidateIf,
                                  unsigned *IfPos) const {

  // TODO: Preheader and PostExit not handled currently.
  if (Loop->hasPreheader() || Loop->hasPostexit()) {
    return false;
  }

  // Loop should be normalized before this pass
  // TODO: Decide whether we can remove this, just to save compile time.
  if (!Loop->isNormalized() || Loop->isUnknown()) {
    return false;
  }

  // TODO: Handle multi-exit loops later, since they will need to
  // update the internal gotos and label.
  if (Loop->isDoMultiExit()) {
    return false;
  }

  // Ignore loops which switch statement.
  if (!analyzeIf(Loop, CandidateIf, IfPos)) {
    return false;
  }

  return true;
}

// transformLoop - Perform the OptPredicate transformation for the given loop.
// There will be two loops after the transformation. One loop inside the
// If-Then and other inside the Else.
void HIROptPredicate::transformLoop(HLLoop *OrigLoop, HLIf *If,
                                    unsigned IfPos) {

  // Remove the Then and Else block as they will be inserted
  // after cloning. This is done to avoid cloning of children
  // when creating the new loop.
  HLContainerTy ThenContainer;
  HLContainerTy ElseContainer;
  removeThenElseChildren(If, &ThenContainer, &ElseContainer);

  // Create the main IfElse loop.
  HLLoop *NewElseLoop = OrigLoop->clone();

  // Insert the Then Children of main loop.
  insertThenChildren(If, &ThenContainer);

  // Move the If condition outside and insert the loops.
  hoistIf(If, OrigLoop, NewElseLoop);

  // Insert the Else children in the new loop.
  insertElseChildren(NewElseLoop, IfPos, &ElseContainer);

  // Add the two loops as candidate for nested/multipe Ifs.
  CandidateLoops.push_back(OrigLoop);
  CandidateLoops.push_back(NewElseLoop);

  // Update the OptPred Trips to prevent massive code size.
  assert(LoopPredTripMap.count(OrigLoop) && " Loop not found in trip map.");
  unsigned NewTrip = LoopPredTripMap[OrigLoop] + 1;
  LoopPredTripMap[NewElseLoop] = NewTrip;
  LoopPredTripMap[OrigLoop] = NewTrip;

  // Mark loop as modified in all the analysis.
  HIRInvalidationUtils::invalidateBody(OrigLoop);

  // Set the code gen for modified region
  assert(OrigLoop->getParentRegion() && " Loop does not have a parent region.");
  OrigLoop->getParentRegion()->setGenCode();
}

void HIROptPredicate::removeThenElseChildren(HLIf *If,
                                             HLContainerTy *ThenContainer,
                                             HLContainerTy *ElseContainer) {

  // Collect Then Children.
  if (If->hasThenChildren()) {
    HLNodeUtils::remove(ThenContainer, If->getFirstThenChild(),
                        If->getLastThenChild());
  }

  // Collect Then Children.
  if (If->hasElseChildren()) {
    HLNodeUtils::remove(ElseContainer, If->getFirstElseChild(),
                        If->getLastElseChild());
  }
}

void HIROptPredicate::insertThenChildren(HLIf *If,
                                         HLContainerTy *ThenContainer) {

  // No else children were present in the If.
  if (ThenContainer->empty()) {
    return;
  }

  HLNodeUtils::insertBefore(If, ThenContainer);
}

// Visitor used to find a candidate If for OptPredicate.
class IfSearchVisitor final : public HLNodeVisitorBase {

private:
  unsigned IfPos;
  unsigned NodeCount;
  HLIf *If;

public:
  IfSearchVisitor(unsigned Pos) : IfPos(Pos), NodeCount(0), If(nullptr) {}

  void visit(HLNode *Node) {
    NodeCount++;
    if (NodeCount == IfPos) {
      If = dyn_cast<HLIf>(Node);
      // HLIf should have been found at IfPos.
      assert(If && " No HLIf found at specified position");
    }
  }
  void postVisit(const HLNode *Node) {}
  bool isDone() const override { return If; }

  HLIf *getIf() { return If; }
};

void HIROptPredicate::insertElseChildren(HLLoop *NewElseLoop, unsigned IfPos,
                                         HLContainerTy *ElseContainer) {

  // Visit NewElseLoop to search for If using its position.
  // We use position marker to find where the If existed in the original loop.
  IfSearchVisitor IfSearchVisit(IfPos);
  HLNodeUtils::visit(IfSearchVisit, NewElseLoop);
  HLIf *NewIf = IfSearchVisit.getIf();
  assert(NewIf && " No If found in the newly created else loop.");

  // Insert else children only if they were present.
  if (!ElseContainer->empty()) {
    HLNodeUtils::insertBefore(NewIf, ElseContainer);
  }

  // Remove the If as it is no longer necessary in the else loop.
  // The If was already hoisted out in the Then loop.
  HLNodeUtils::erase(NewIf);
  NewIf = nullptr;
}

void HIROptPredicate::updateIfRef(HLIf *If) {
  for (auto Iter = If->ddref_begin(), End = If->ddref_end(); Iter != End;
       ++Iter) {
    (*Iter)->updateDefLevel();
  }
}

void HIROptPredicate::hoistIf(HLIf *If, HLLoop *OrigLoop, HLLoop *NewElseLoop) {

  // Hoist the If outside the loop.
  HLNodeUtils::moveBefore(OrigLoop, If);

  // Update the DDRefs inside the HLIf.
  updateIfRef(If);

  // Move loop to then part.
  HLNodeUtils::moveAsFirstChild(If, OrigLoop, true);

  // Add the NewElseLoop to Else.
  HLNodeUtils::insertAsFirstChild(If, NewElseLoop, false);
}
