//===- Intel_GVBasedMultiVersioning.cpp -----------------------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// Global-variable-based Multiversioning tries to create clones of function body
// with less conditional branches on values of global variables. In some
// applications, interesting regions of code is separated by irrelevant code
// such as debug logs and error reporting, or different modes of operation are
// supported by the same function. Such code is typically guarded by some
// variables, e.g.:
//
// for (...)
//   HOTLOOP 1
// if (LOGGING_ENABLED)
//   log("Step 1 done.");
// for (...)
//   HOTLOOP 2
//
// By identifying and multiversioning on those variables, and thus removing the
// interfering code, we can potentially make the hot regions closer, reduce side
// effect and facilitate optimization. Currently it only handles i1 global
// variables.
//
// This pass works by:
//
// * First find all eligible branches. An eligible branch is a conditional
// branch based on an i1 global variable. We then choose the global variable
// with the most branches as a starting point. Bail out if the number of
// branches is too small.
// * Assume an invariant for each of the two boolean values of the global
// variable selected. If the global may be modified inside the function, try to
// find other invariants that make the modification unreachable. Do this
// recursively until we've built a set of invariants (InvariantSet) that can be
// assumed throughout the function if all of the invariants are satisfied at the
// entry of function. As the starting global may take 2 values, we try to build
// 2 InvariantSets. Bail out if we can't build any InvariantSet.
// * Do the actual multiversioning by cloning the function body and generating
// dispatch code based on the InvariantSets. The InvariantSets are applied to
// the new clones and some quick constant folding is done.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_GVBasedMultiVersioning.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include <optional>

#define DEBUG_TYPE "gvbased-multiversioning"

using namespace llvm;

static cl::opt<bool> DisableGVBasedMultiversioning(
    "disable-gvbased-multiversion", cl::Hidden,
    cl::desc("Do not perform GVBasedMultiversioning"));

static cl::opt<unsigned int> GVBasedMultiversionMinNumBranches(
    "gvbased-multiversion-min-num-branches", cl::init(8), cl::Hidden,
    cl::desc("Minimum number of conditional branches based on a global "
             "variable to trigger GVBasedMultiversioning for that global"));

// If an InvariantSet has too many conditions, it's possible that satisfying
// them all is hard at runtime.
static cl::opt<unsigned int> GVBasedMultiversionMaxNumInvariants(
    "gvbased-multiversion-max-num-invariants", cl::init(8), cl::Hidden,
    cl::desc(
        "Maximum number of invariants allowed for GVBasedMultiversioning."));

// Structure to store information of an invariant temporarily
struct InvariantDesc {
  GlobalVariable *GV;
  bool Value;
  InvariantDesc(GlobalVariable *GV, bool Value) : GV(GV), Value(Value) {}
};

// Use MapVector to generate consistent code for testing the invariants.
using InvariantSet = MapVector<GlobalVariable *, bool>;

class GVBasedMultiVersioning {
  Function *F;
  DominatorTree *DT;
  AAManager::Result *AAR;
  // One-to-many mapping from i1 global variables to every conditional branch in
  // F based on that variable.
  // MapVector is used because we need to deterministically select a branch with
  // the highest number of branches.
  MapVector<GlobalVariable *, SmallVector<BranchInst *, 4>> GlobalBranches;
  // Inverse mapping of GlobalBranches. Used only to determine whether a branch
  // is a eligible branch for an invariant quickly.
  DenseMap<BranchInst *, GlobalVariable *> BranchToGlobal;

  // Given a BB, try to find an invariant that must hold for the BB to be
  // executed.
  // The current implementation returns the "nearest" invariant as it searches
  // for an invariant by walking upwards the dominator tree from BB.
  std::optional<InvariantDesc> findInvariantForBlock(BasicBlock *BB);

  // Given an initial invariant, try to build a InvariantSet (which is
  // guaranteed to include the initial one), so that when all the invariants in
  // the set are satisfied at the entry of F, they're also guaranteed to hold at
  // the exit of F. (i.e. all global variables of these invariants are not
  // modified throughout the function)
  // Returns empty set if failed InvariantSet.
  InvariantSet
  buildInvariantSetFromInvariant(const InvariantDesc &InitialInvariant);

  // Analyze F and try to build a few InvariantSets. By doing multiversioning
  // based on the returned InvariantSets, conditional branches on globals can be
  // reduced. The result inveriant sets are pushed to Out.
  void buildInvariantSetsForMultiversioning(SmallVectorImpl<InvariantSet> &Out);
  // Do multiversioning on F for the given InvariantSets.
  void doTransformation(ArrayRef<InvariantSet> BranchInvariantSets);

public:
  GVBasedMultiVersioning(Function *F, DominatorTree *DT, AAManager::Result *AAR)
      : F(F), DT(DT), AAR(AAR) {}
  bool run();
};

// Given a conditional branch and a boolean, select one from the two edges
// originating from the branch based on the boolean value.
static BasicBlockEdge getEdge(const BranchInst *Branch, bool Value) {
  assert(Branch->isConditional() && "Expect a conditional branch");
  return BasicBlockEdge(Branch->getParent(), Branch->getSuccessor(1 - Value));
}

// If a branch is a conditional branch based on an i1 global variable, returns
// that variable. Returns nullptr if the branch is not eligible.
static GlobalVariable *getGlobalVariableForBranch(BranchInst *Branch) {
  if (!Branch->isConditional())
    return nullptr;
  auto *Load = dyn_cast<LoadInst>(Branch->getCondition());
  if (!Load)
    return nullptr;
  auto *GV = dyn_cast<GlobalVariable>(Load->getPointerOperand());
  return GV;
}

// Given a set of BBs and an invariant, remove all BBs that is unreachable if
// the invariant is true from the set.
static void removeUnreachableBasicBlocksForInvariant(
    DenseSet<BasicBlock *> &BlocksSet, InvariantDesc Invariant,
    DominatorTree *DT,
    const MapVector<GlobalVariable *, SmallVector<BranchInst *, 4>>
        &GlobalBranches) {
  SmallVector<BasicBlockEdge, 4> Edges;
  // Find all CFG edges associated with the opposite side of the invariant, and
  // exclude all BBs dominated by these edges.
  auto InvariantBranchIt = GlobalBranches.find(Invariant.GV);
  assert(InvariantBranchIt != GlobalBranches.end() && "Missing branch");
  for (auto *Branch : InvariantBranchIt->second) {
    BasicBlockEdge BBE = getEdge(Branch, !Invariant.Value);
    if (DT->dominates(BBE, BBE.getEnd()))
      Edges.push_back(BBE);
  }

  SmallVector<BasicBlock *, 4> BBsToRemove;
  for (auto *BB : BlocksSet)
    for (const auto &Edge : Edges)
      if (DT->dominates(Edge, BB))
        BBsToRemove.push_back(BB);

  for (auto *BB : BBsToRemove)
    BlocksSet.erase(BB);
}

std::optional<InvariantDesc>
GVBasedMultiVersioning::findInvariantForBlock(BasicBlock *BB) {
  auto *DTNode = (*DT)[BB]->getIDom();
  while (DTNode != nullptr) {
    // Walk upwards the DomTree, until the terminator of the current BB is a
    // branch on global.
    BranchInst *Branch =
        dyn_cast<BranchInst>(DTNode->getBlock()->getTerminator());
    auto BranchToGlobalIt = BranchToGlobal.find(Branch);
    if (BranchToGlobalIt == BranchToGlobal.end()) {
      DTNode = DTNode->getIDom();
      continue;
    }

    // The branch is what we're finding, if one and only one of its successors
    // dominates BB
    BasicBlockEdge TrueEdge = getEdge(Branch, true),
                   FalseEdge = getEdge(Branch, false);
    bool Value;
    if (DT->dominates(TrueEdge, BB)) {
      assert(!DT->dominates(FalseEdge, BB) &&
             "Two successors can't dominate the same block");
      Value = true;
    } else if (DT->dominates(FalseEdge, BB)) {
      assert(!DT->dominates(TrueEdge, BB) &&
             "Two successors can't dominate the same block");
      Value = false;
    } else {
      DTNode = DTNode->getIDom();
      continue;
    }
    return InvariantDesc(BranchToGlobalIt->second, Value);
  }
  return std::nullopt;
}

InvariantSet GVBasedMultiVersioning::buildInvariantSetFromInvariant(
    const InvariantDesc &InitialInvariant) {
  LLVM_DEBUG(dbgs() << "Populating InvariantSet for "
                    << InitialInvariant.GV->getName() << " = "
                    << InitialInvariant.Value << "\n");
  InvariantSet Result;
  SmallVector<InvariantDesc, 4> NewInvariants;
  auto AddInvariant = [&Result, &NewInvariants](InvariantDesc Invariant) {
    NewInvariants.push_back(Invariant);
    Result[Invariant.GV] = Invariant.Value;
  };
  AddInvariant(InitialInvariant);

  DenseSet<BasicBlock *> BlocksToCheck;
  for (auto &BB : *F)
    BlocksToCheck.insert(&BB);

  while (!NewInvariants.empty()) {
    InvariantDesc Invariant = NewInvariants.pop_back_val();
    // Remove all BBs not reachable for this condition. Then analyze all
    // remaining BBs for potential modification of the GV.
    removeUnreachableBasicBlocksForInvariant(BlocksToCheck, Invariant, DT,
                                             GlobalBranches);
    MemoryLocation Loc(Invariant.GV, LocationSize::precise(1));
    for (auto *BB : BlocksToCheck) {
      if (!AAR->canBasicBlockModify(*BB, Loc))
        continue;

      std::optional<InvariantDesc> MaybeNewInvariant =
          findInvariantForBlock(BB);
      if (!MaybeNewInvariant.has_value()) {
        LLVM_DEBUG(dbgs() << "Cannot find invariant for block " << *BB << "\n");
        return InvariantSet();
      }
      InvariantDesc NewInvariant = MaybeNewInvariant.value();

      // BB may modify Invariant.GV. To keep Invariant.GV constant, we need BB
      // not to be executed, so invert the value of NewInvariant.
      NewInvariant.Value = !NewInvariant.Value;

      auto It = Result.find(NewInvariant.GV);
      if (It != Result.end()) {
        // The GV we found already exists in the InvariantSet, but the new
        // invariant needs a value for this GV that's different from that in the
        // InvariantSet. We can't satisfy two conflicting values for the same
        // GV, bail out.
        if (It->second != NewInvariant.Value)
          return InvariantSet();
      } else {
        // This invariant is new, add to worklist to find what may modify its GV
        // in later iterations.
        AddInvariant(NewInvariant);
      }
    }
  }

  return Result;
}

void GVBasedMultiVersioning::buildInvariantSetsForMultiversioning(
    SmallVectorImpl<InvariantSet> &Out) {
  // Scan terminators of all BBs to find all conditional branches based on
  // global variables.
  for (BasicBlock &BB : *F) {
    auto *Branch = dyn_cast<BranchInst>(BB.getTerminator());
    if (!Branch)
      continue;
    auto *GV = getGlobalVariableForBranch(Branch);
    if (!GV)
      continue;

    GlobalBranches[GV].push_back(Branch);
    BranchToGlobal[Branch] = GV;
  }

  // Find the global variable that's most frequently used in conditional
  // branches.
  GlobalVariable *GlobalWithMostBranches = nullptr;
  unsigned NumUses = 0;
  for (const auto &[GV, Branches] : GlobalBranches) {
    unsigned NumBranches = Branches.size();
    if (NumBranches <= NumUses)
      continue;
    GlobalWithMostBranches = GV;
    NumUses = NumBranches;
  }

  // Bail out if we can't find any eligible branch, or the number of branches is
  // too few.
  if (!GlobalWithMostBranches) {
    LLVM_DEBUG(dbgs() << "GVMV analysis for " << F->getName()
                      << " bails out, no eligible branch\n");
    return;
  }
  if (NumUses < GVBasedMultiversionMinNumBranches) {
    LLVM_DEBUG(
        dbgs()
        << "GVMV analysis for " << F->getName() << " bails out, only "
        << NumUses
        << " eligible branches found, not adequate to trigger multiversion\n");
    return;
  }
  LLVM_DEBUG(
      dbgs() << "GVMV analysis for " << F->getName() << ": " << NumUses
             << " eligible branches found, proceed to build invariant sets\n");

  // Attempt to build two InvariantSets with the initial invariants:
  // GlobalWithMostBranches = true and GlobalWithMostBranches = false.
  auto TryBuildAndAddInvariantSetWithInitialValue =
      [this, &Out, GlobalWithMostBranches](bool InitialValue) {
        InvariantSet Result = buildInvariantSetFromInvariant(
            InvariantDesc(GlobalWithMostBranches, InitialValue));
        if (Result.empty())
          return;
        if (Result.size() > GVBasedMultiversionMaxNumInvariants) {
          LLVM_DEBUG(dbgs() << "GVMV analysis for " << F->getName()
                            << " created invariant set with " << Result.size()
                            << " invariants, too many for multiversioning.\n");
          return;
        }
        LLVM_DEBUG(dbgs() << "GVMV analysis for " << F->getName()
                          << " invariant set: ";
                   for (const auto &Invariant
                        : Result) dbgs()
                   << "(" << Invariant.first->getName() << " = "
                   << Invariant.second << ") ";
                   dbgs() << "\n");
        Out.push_back(Result);
      };

  TryBuildAndAddInvariantSetWithInitialValue(true);
  TryBuildAndAddInvariantSetWithInitialValue(false);
}

// Determine whether the invariant sets are exhaustive so that we don't need a
// fallback path and can reuse the original code for one of the clones.
// NOTE: The current implementation only covers a special case: The input
// contains 2 invariant sets with only 1 global, the value for one of them is
// true and the other is false.
static bool canReuseOriginalCode(ArrayRef<InvariantSet> BranchInvariantSets) {
  if (BranchInvariantSets.size() != 2)
    return false;
  const auto &FirstSet = BranchInvariantSets[0];
  const auto &SecondSet = BranchInvariantSets[1];
  // Ensure both InvariantSets have only 1 Invariant
  if (FirstSet.size() != FirstSet.size() || FirstSet.size() != 1)
    return false;
  // Check the global variable and value of both invariants
  const auto First = FirstSet.front();
  const auto Second = SecondSet.front();
  if (First.first != Second.first || First.second != !Second.second)
    return false;
  return true;
}

void GVBasedMultiVersioning::doTransformation(
    ArrayRef<InvariantSet> BranchInvariantSets) {
  BasicBlock *OrigEntryBB = &F->getEntryBlock();
  SmallVector<BasicBlock *, 32> OrigBBs;
  for (auto &BB : *F)
    OrigBBs.push_back(&BB);

  // Create a new BB to hold loads of globals used in the invariants. A global
  // variable can be used in multiple InvariantSets, since all invariants in the
  // same InvariantSet are preserved throughout the function, we can reuse the
  // same load of that global.
  // The new BB is the new entry of the function. We'll create more BBs
  // succeeding it to check these loads.
  BasicBlock *GlobalLoadsBB = BasicBlock::Create(
      F->getParent()->getContext(), "mv.global.loads", F, OrigEntryBB);
  DenseMap<GlobalVariable *, LoadInst *> LoadsForGlobals;

  BasicBlock *BBAfterLoadsBB = OrigEntryBB;
  auto *EntryBr = BranchInst::Create(OrigEntryBB, GlobalLoadsBB);

  // Given an InvariantSet, create instructions to check all the invariants.
  // Returns an i1 Value.
  auto BuildConditionForInvariantSet =
      [&LoadsForGlobals, EntryBr](const InvariantSet &Invariants,
                                  IRBuilder<> &Builder) -> Value * {
    assert(!Invariants.empty());
    auto *Int1Ty = IntegerType::getInt1Ty(Builder.getContext());
    if (Invariants.empty())
      return ConstantInt::get(Int1Ty, 1);
    SmallVector<Value *, 4> Values;
    for (auto &Invariant : Invariants) {
      auto *GV = Invariant.first;
      auto It = LoadsForGlobals.find(GV);
      if (It == LoadsForGlobals.end())
        It = LoadsForGlobals
                 .insert(std::make_pair(
                     GV, new LoadInst(Int1Ty, GV, "mv.load." + GV->getName(),
                                      EntryBr)))
                 .first;
      Value *Load = It->second;
      Values.push_back(Invariant.second ? Load : Builder.CreateNot(Load));
    }
    return Builder.CreateAnd(Values);
  };

  // Inside a region cloned from the original function, substitute uses of all
  // variables in the InvariantSet with the constant stored in the corresponding
  // invariant.
  auto ApplyInvariantSetToClone =
      [](const InvariantSet &Invariants,
         const SmallVector<BasicBlock *, 32> &Blocks) {
        for (auto *BB : Blocks) {
          SmallVector<Instruction *, 4> InstsToRemove;
          for (auto &I : *BB) {
            auto *Load = dyn_cast<LoadInst>(&I);
            if (!Load)
              continue;
            auto *GV = dyn_cast<GlobalVariable>(Load->getPointerOperand());
            if (!GV)
              continue;
            auto It = Invariants.find(GV);
            if (It != Invariants.end()) {
              Load->replaceAllUsesWith(ConstantInt::get(
                  IntegerType::getInt1Ty(BB->getContext()), It->second));
              InstsToRemove.push_back(Load);
            }
          }
          for (auto *I : InstsToRemove)
            I->eraseFromParent();
        }
      };

  // Clone the function body for every InvariantSet and generate dispatch code
  // to each clone. Reuse the original code if possible.
  const bool ReuseOriginalCode = canReuseOriginalCode(BranchInvariantSets);
  for (const auto &Invariants : BranchInvariantSets) {
    if (ReuseOriginalCode && &Invariants == &BranchInvariantSets.back()) {
      LLVM_DEBUG(dbgs() << "GVMV transformation for " << F->getName() << ": "
                        << "Reusing the original code.\n");
      ApplyInvariantSetToClone(Invariants, OrigBBs);
      continue;
    }

    SmallVector<BasicBlock *, 32> CloneBBs;
    ValueToValueMapTy VMap;
    for (auto *OrigBB : OrigBBs) {
      auto CloneBB = CloneBasicBlock(OrigBB, VMap, ".clone", F);
      VMap[OrigBB] = CloneBB;
      CloneBBs.push_back(CloneBB);
    }
    remapInstructionsInBlocks(CloneBBs, VMap);
    ApplyInvariantSetToClone(Invariants, CloneBBs);

    // Create a new BB to check the InvariantSet and jump to the new clone. Then
    // use it as the new successor of GlobalLoadsBB.
    BasicBlock *CondBB = BasicBlock::Create(F->getParent()->getContext(),
                                            "mv.cond", F, BBAfterLoadsBB);
    IRBuilder<> Builder(CondBB);
    Value *Condition = BuildConditionForInvariantSet(Invariants, Builder);
    Builder.CreateCondBr(Condition, cast<BasicBlock>(VMap.lookup(OrigEntryBB)),
                         BBAfterLoadsBB);
    EntryBr->setSuccessor(0, CondBB);
    BBAfterLoadsBB = CondBB;
  }

  // Run some CFG simplification transformations to constant fold branches on
  // the globals.
  // Currently it's done globally. This is okay because it's rarely triggered.
  // We can limit it to a subset of related BBs but that'll complicate the code
  // significantly.
  removeUnreachableBlocks(*F);
  for (auto &BB : make_early_inc_range(*F))
    MergeBlockIntoPredecessor(&BB);
}

bool GVBasedMultiVersioning::run() {
  SmallVector<InvariantSet, 2> InvariantSets;
  buildInvariantSetsForMultiversioning(InvariantSets);
  if (InvariantSets.empty()) {
    LLVM_DEBUG(
        dbgs() << "GVMV analysis for " << F->getName()
               << " bails out, unable to build a valid invariant set.\n");
    return false;
  }
  doTransformation(InvariantSets);
  return true;
}

PreservedAnalyses GVBasedMultiVersioningPass::run(Function &F,
                                                  FunctionAnalysisManager &AM) {
  if (DisableGVBasedMultiversioning)
    return PreservedAnalyses::all();

  auto &AAR = AM.getResult<AAManager>(F);
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);

  GVBasedMultiVersioning GVMV(&F, &DT, &AAR);
  if (GVMV.run()) {
    auto PA = PreservedAnalyses();
    PA.preserve<WholeProgramAnalysis>();
    PA.preserve<GlobalsAA>();
    PA.preserve<TargetIRAnalysis>();
    PA.preserve<AndersensAA>();
    return PA;
  } else {
    return PreservedAnalyses::all();
  }
}
