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
// with the most branches (called "principal global") as a starting point. Bail
// out if the number of branches is too small.
// * Assume an invariant for each of the two boolean values of the principal
// global. If it may be modified inside the function, try to find other
// invariants that make the modification unreachable. Do this recursively until
// we've built a set of invariants (InvariantSet) that can be assumed throughout
// the function if all of the invariants are satisfied at the entry of function.
// As the starting global may take 2 values, we try to build 2 InvariantSets.
// * If an instruction may modify an invariant, but we can't find a new
// invariant to exclude it, we'll then try to shrink the scope of
// multiversioning to avoid that instruction. Bail out if this still doesn't
// work.
// * Do the actual multiversioning by cloning the function body and generating
// dispatch code based on the InvariantSets. The InvariantSets are applied to
// the new clones and some quick constant folding is done.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_GVBasedMultiVersioning.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
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

namespace {

// Structure to store information of an invariant temporarily
struct InvariantDesc {
  GlobalVariable *GV;
  bool Value;
  InvariantDesc(GlobalVariable *GV, bool Value) : GV(GV), Value(Value) {}
};

// A Scope represents a set of instructions inside a function with single-entry,
// single-exit control flow.
// It's identified by an entry point and exit point. For a scope to be valid,
// the entry must dominate the exit, and the exit must post-dominate the entry.
// However we don't have a good way to check for validity defined above because
// the "post-dominate" part have caveats: We allow unreachables in a scope. But
// they're also included in LLVM post-dominator tree contains. Because there is
// no successor to these blocks, they basically mess up the entire pdom tree as
// they're not executed in runtime, and thus not useful to us. It seems the
// community have no desire to change this (D12676, D119760). For now,
// unreachables are handled with some special casing.
// The trouble with unreachables also means we need to store all blocks
// contained inside the scope because when doing the cloning transformation, we
// can't reliably determine whether a basic block or instruction is in the scope
// with post-dominator queries.
// The exit point we store here is not contained by the scope. Doing so is
// useful when the scope ends just before a basic block that has multiple
// predecessors, but the first instruction of that block has properties that we
// don't want (e.g. side effect). We can also create an intermediate block to
// deal with such cases, but since we use scopes in the analysis phase, mutation
// to IR is not appropriate.
struct ScopeTy {
  Instruction *Entry;
  // If Exit is nullptr, the scope terminates when the function returns.
  Instruction *Exit;
  // All basic blocks inside the scope, stored to help computing liveouts in
  // transformation phase.
  // Note the parent block of Entry and Exit is not in the vector. It's not a
  // good idea to store them as they can get split during the transformation.
  DenseSet<BasicBlock *> Blocks;
  // Create a scope covering the entire function F.
  ScopeTy(Function &F) : Entry(&F.getEntryBlock().front()), Exit(nullptr) {
    for (auto &BB : F)
      Blocks.insert(&BB);
    // Remove entry block because it's the parent block of entry.
    Blocks.erase(&F.getEntryBlock());
  }
  ScopeTy(Instruction *Entry, Instruction *Exit,
          const DenseSet<BasicBlock *> &Blocks)
      : Entry(Entry), Exit(Exit), Blocks(Blocks) {
    assert(Entry && "Entry can't be nullptr");
    assert(Blocks.find(Entry->getParent()) == Blocks.end() &&
           (!Exit || Blocks.find(Exit->getParent()) == Blocks.end()) &&
           "Parent block of Entry and Exit shouldn't be in Blocks.");
  }
  bool coversWholeFunction() const {
    return Entry == &Entry->getFunction()->getEntryBlock().front() &&
           Exit == nullptr;
  }
  bool operator==(const ScopeTy &Other) const {
    return std::tie(Entry, Exit) == std::tie(Other.Entry, Other.Exit);
  }
  bool operator!=(const ScopeTy &Other) const { return !(*this == Other); }
};

raw_ostream &operator<<(raw_ostream &OS, const ScopeTy &Scope) {
  if (Scope.coversWholeFunction()) {
    dbgs() << "entire function";
    return OS;
  }
  OS << "from " << *Scope.Entry << " to ";
  if (Scope.Exit)
    OS << *Scope.Exit;
  else
    OS << "function exit";
  return OS;
}

// Use MapVector to generate consistent code for testing the invariants.
using InvariantSet = MapVector<GlobalVariable *, bool>;

// Represent multiversioning restricted to a scope. Note that the scope can be
// the entire function, as the constructor defaults to.
struct ScopedInvariantSet {
  InvariantSet Invariants;
  ScopeTy Scope;
  ScopedInvariantSet(Function &F) : Scope(F) {}
};

// Data that is helpful for the scope shrinking algorithm, derived from
// analysis.
struct ScopeShrinkInfoTy {
  // Some patterns can make scope shrinking hard (e.g. irreducible control
  // flow). They're checked to set this field.
  bool CanDoScopeShrink = false;
  // Set to the function's unique return instruction (if there's only one).
  Instruction *UniqueExit = nullptr;
  // All loads of principal globals used in conditional branches.
  DenseSet<LoadInst *> LoadsForPrincipalGlobalInBranches;
};

class GVBasedMultiVersioning {
  Function *F;
  DominatorTree *DT;
  AAManager::Result *AAR;
  std::function<LoopInfo &()> GetLI;
  std::function<PostDominatorTree &()> GetPDT;
  std::function<TargetTransformInfo &()> GetTTI;
  // One-to-many mapping from i1 global variables to every conditional branch in
  // F based on that variable.
  // MapVector is used because we need to deterministically select a branch with
  // the highest number of branches.
  MapVector<GlobalVariable *, SmallVector<BranchInst *, 4>> GlobalBranches;
  // Inverse mapping of GlobalBranches. Used only to determine whether a branch
  // is a eligible branch for an invariant quickly.
  DenseMap<BranchInst *, GlobalVariable *> BranchToGlobal;
  GlobalVariable *PrincipalGlobal = nullptr;
  // Because scope shrinking may be executed for multiple times or not executed
  // at all, this is lazily computed and memorized.
  std::optional<ScopeShrinkInfoTy> ScopeShrinkInfo;

  // Given a BB, try to find an invariant that must hold for the BB to be
  // executed.
  // The current implementation returns the "nearest" invariant as it searches
  // for an invariant by walking upwards the dominator tree from BB.
  std::optional<InvariantDesc> findInvariantForBlock(BasicBlock *BB);

  // Try to create a new scope, that is a subset of OldScope, but doesn't
  // contain InstrToExclude. Returns std::nullopt if fails to do so.
  // All basic blocks removed from OldScope will also be removed from BBs set.
  std::optional<ScopeTy> tryShrinkScope(ScopeTy OldScope,
                                        Instruction *InstrToExclude,
                                        DenseSet<BasicBlock *> &BBs);

  // Given an initial invariant, try to build a ScopedInvariantSet (which is
  // guaranteed to include the initial one), so that when all the invariants in
  // the set are satisfied at the entry of the scope, they're also guaranteed to
  // hold at the exit of scope. (i.e. all global variables of these invariants
  // are not modified throughout the scope)
  // Return value has empty InvariantSet if failed to do so.
  ScopedInvariantSet
  buildInvariantSetFromInvariant(const InvariantDesc &InitialInvariant);

  // Analyze F and try to build a few ScopedInvariantSets. By doing
  // multiversioning based on them, conditional branches on globals can be
  // reduced. The results are pushed to Out.
  void buildInvariantSetsForMultiversioning(
      SmallVectorImpl<ScopedInvariantSet> &Out);

  // Attempt to do multiversioning on F for the given ScopedInvariantSets.
  // The signature allows some really complex inputs such as multiple
  // ScopedInvariantSets with non-identical overlapping scopes. So there is no
  // promise that every input is actually used.
  void doTransformation(ArrayRef<ScopedInvariantSet> BranchInvariantSets);

  // Return the cached ScopeShrinkInfo, do the analysis to compute it if not
  // yet.
  const ScopeShrinkInfoTy &getScopeShrinkInfo();

public:
  GVBasedMultiVersioning(Function *F, DominatorTree *DT, AAManager::Result *AAR,
                         std::function<LoopInfo &()> GetLI,
                         std::function<PostDominatorTree &()> GetPDT,
                         std::function<TargetTransformInfo &()> GetTTI)
      : F(F), DT(DT), AAR(AAR), GetLI(GetLI), GetPDT(GetPDT), GetTTI(GetTTI) {}
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

// Utility to remove all blocks satisfying Pred from BBs.
static void filterBBsWithPred(function_ref<bool(BasicBlock *)> Pred,
                              DenseSet<BasicBlock *> &BBs) {
  SmallVector<BasicBlock *, 4> BBsToRemove;
  for (auto BB : BBs)
    if (Pred(BB))
      BBsToRemove.push_back(BB);
  for (auto BB : BBsToRemove)
    BBs.erase(BB);
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

  filterBBsWithPred(
      [DT, Edges](BasicBlock *BB) {
        return std::any_of(Edges.begin(), Edges.end(),
                           [DT, BB](const BasicBlockEdge &Edge) {
                             return DT->dominates(Edge, BB);
                           });
      },
      BlocksSet);
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

// Returns the function's only exit if there is only one. Returns nullptr
// otherwise.
static Instruction *getFunctionUniqueExit(Function *F, PostDominatorTree &PDT) {
  // Attempt to identify the unique exit by checking whether post dominator tree
  // has a root BasicBlock.
  if (auto ExitBlock = PDT.getRootNode()->getBlock())
    return ExitBlock->getTerminator();

  // If the PDT approach failed, maybe the PDT is tainted by unreachables. Check
  // every terminator manually.
  Instruction *Result = nullptr;
  unsigned NumReturnInst = 0;
  for (auto &BB : *F) {
    Instruction *Term = BB.getTerminator();
    // Only handle the most common terminators for simplicity
    if (!(isa<BranchInst>(Term) || isa<SwitchInst>(Term) ||
          isa<ReturnInst>(Term) || isa<UnreachableInst>(Term)))
      return nullptr;
    if (isa<ReturnInst>(Term)) {
      NumReturnInst += 1;
      Result = Term;
    }
  }
  return NumReturnInst == 1 ? Result : nullptr;
}

const ScopeShrinkInfoTy &GVBasedMultiVersioning::getScopeShrinkInfo() {
  if (ScopeShrinkInfo.has_value())
    return ScopeShrinkInfo.value();

  ScopeShrinkInfo = ScopeShrinkInfoTy();
  ScopeShrinkInfoTy &Result = ScopeShrinkInfo.value();

  // The scope shrinking heuristic (see tryShrinkScope() below) needs to split a
  // scope into two. Irreducible cycles and multiple-exit functions make it
  // harder.
  ReversePostOrderTraversal<BasicBlock *> RPOT(&F->getEntryBlock());
  Result.UniqueExit = getFunctionUniqueExit(F, GetPDT());
  Result.CanDoScopeShrink =
      !containsIrreducibleCFG<BasicBlock *>(RPOT, GetLI()) && Result.UniqueExit;
  if (!Result.CanDoScopeShrink) {
    LLVM_DEBUG(dbgs() << "GVMV analysis for " << F->getName()
                      << ": shrink scope not supported for this function\n");
    return Result;
  }

  for (BranchInst *BI : GlobalBranches[PrincipalGlobal])
    Result.LoadsForPrincipalGlobalInBranches.insert(
        cast<LoadInst>(BI->getCondition()));

  return Result;
}

std::optional<ScopeTy>
GVBasedMultiVersioning::tryShrinkScope(ScopeTy OldScope,
                                       Instruction *InstrToExclude,
                                       DenseSet<BasicBlock *> &BBs) {
  // There is no guarantee that a valid and optimal result can be found with
  // acceptable cost. Here we just implement some heuristics for common cases.
  if (InstrToExclude->isTerminator())
    return std::nullopt;
  const ScopeShrinkInfoTy &SSI = getScopeShrinkInfo();
  if (!SSI.CanDoScopeShrink)
    return std::nullopt;

  // Because all instructions in a scope must be continuous in control flow. If
  // we want to produce a new scope that is a sub-scope of OldScope and doesn't
  // contain InstrToExclude, the result must be either a subset of the control
  // flow from OldScope.Entry to InstrToExclude, or subset of control flow from
  // InstrToExclude to OldScope.Exit.
  // To get a clear cut of OldScope by InstrToExclude, InstrExclude can't be
  // located inside a loop (Irreducibles are already taken care of in
  // CanDoScopeShrink).
  if (GetLI().getLoopFor(InstrToExclude->getParent()))
    return std::nullopt;

  // A simple profitability check is performed below to choose whether the new
  // scope would be before or after InstrToExclude, depending on whether
  // InstrToExclude dominates all loads of principal global (that are used for
  // conditional branches), or post-dominates all loads. Note it requires "all
  // loads" to be consistent in the domination queries, we can do better e.g.
  // checking which side has more than 80% loads, but this is simpler and works
  // well for our cases.
  bool InstrDominatesAllLoads = true;
  bool InstrPostDominatesAllLoads = true;
  for (LoadInst *Load : SSI.LoadsForPrincipalGlobalInBranches) {
    if (!DT->dominates(InstrToExclude, Load))
      InstrDominatesAllLoads = false;
    if (!GetPDT().dominates(InstrToExclude, Load))
      InstrPostDominatesAllLoads = false;
  }

  // If either InstrDominatesAllLoads or InstrPostDominatesAllLoads is true, it
  // means InstrToExclude partitions the scope into two fragments: one with all
  // the loads of globals, and another without.
  // If InstrDominatesAllLoads is true, what we care about is in the latter
  // half, and we can try shrinking the scope of multiversioning by setting the
  // entry of the scope to the next instruction of InstrToExclude. This also
  // works vice versa: If InstrToExclude post-dominates all loads, use the
  // former half.
  BasicBlock *UniqueExitBB = SSI.UniqueExit->getParent();
  BasicBlock *SplitBB = InstrToExclude->getParent();
  DenseSet<BasicBlock *> BBsInScope(OldScope.Blocks);

  if (InstrDominatesAllLoads) {
    Instruction *NewEntry = InstrToExclude->getNextNode();
    auto CheckNewEntryDominatesOldExit = [this, &OldScope, NewEntry, SplitBB,
                                          UniqueExitBB]() {
      return OldScope.Exit ? DT->dominates(NewEntry, OldScope.Exit)
                           : DT->dominates(SplitBB, UniqueExitBB);
    };
    auto CheckOldExitPostDominatesNewEntry = [this, &OldScope, NewEntry,
                                              UniqueExitBB]() {
      // A nullptr value of Exit points to the exit of the function, which
      // certainly post-dominates everything inside the function.
      if (!OldScope.Exit)
        return true;
      // Check with PDT
      if (GetPDT().dominates(OldScope.Exit, NewEntry))
        return true;
      // If the PDT check failed, maybe the function has unreachables and PDT is
      // unreliable. But the DT is still good. We can utilize the function exit
      // point (UniqueExitBB) as a proxy to check for a special case, where such
      // A chain of domination relationship exists: NewEntry => OldScope.Exit
      // => Function exit. And given we already know the function exit always
      // post-dominates NewEntry, we can infer OldScope.Exit also post-dominates
      // NewEntry.
      return DT->dominates(NewEntry, OldScope.Exit) &&
             DT->dominates(OldScope.Exit->getParent(), UniqueExitBB);
    };

    if (CheckNewEntryDominatesOldExit() &&
        CheckOldExitPostDominatesNewEntry()) {
      auto Pred = [this, SplitBB](BasicBlock *BB) {
        return !DT->dominates(SplitBB, BB);
      };
      filterBBsWithPred(Pred, BBs);
      filterBBsWithPred(Pred, BBsInScope);
      BBsInScope.erase(NewEntry->getParent());
      return ScopeTy(NewEntry, OldScope.Exit, BBsInScope);
    }
  }

  if (InstrPostDominatesAllLoads &&
      DT->dominates(OldScope.Entry, InstrToExclude) &&
      GetPDT().dominates(InstrToExclude, OldScope.Entry)) {
    auto Pred = [this, SplitBB](BasicBlock *BB) {
      return !GetPDT().dominates(SplitBB, BB);
    };
    filterBBsWithPred(Pred, BBs);
    filterBBsWithPred(Pred, BBsInScope);
    BBsInScope.erase(InstrToExclude->getParent());
    return ScopeTy(OldScope.Entry, InstrToExclude, BBsInScope);
  }

  // Another special case to deal with the PDT issues with unreachables, but
  // tries to set the Exit to InstrToExclude.
  // Unlike the one in CheckOldExitPostDominatesNewEntry, we need the value of
  // InstrPostDominatesAllLoads, which is not correct. But if InstrToExclude is
  // in UniqueExitBB, we can safely say it post-dominates all instructions (sans
  // the ones after it in UniqueExitBB), and post-dominates all relevant loads
  // (as UniqueExitBB is guaranteed to have a "return" as terminator).
  if (InstrToExclude->getParent() == UniqueExitBB &&
      DT->dominates(OldScope.Entry, InstrToExclude)) {
    assert((!OldScope.Exit || (OldScope.Exit->getParent() == UniqueExitBB &&
                               DT->dominates(InstrToExclude, OldScope.Exit))) &&
           "Old exit should be after InstrToExclude");
    BBsInScope.erase(UniqueExitBB);
    return ScopeTy(OldScope.Entry, InstrToExclude, BBsInScope);
  }

  return std::nullopt;
}

ScopedInvariantSet GVBasedMultiVersioning::buildInvariantSetFromInvariant(
    const InvariantDesc &InitialInvariant) {
  LLVM_DEBUG(dbgs() << "Populating InvariantSet for "
                    << InitialInvariant.GV->getName() << " = "
                    << InitialInvariant.Value << "\n");
  ScopedInvariantSet Result(*F);
  SmallVector<InvariantDesc, 4> NewInvariants;
  auto AddInvariant = [&Result, &NewInvariants](InvariantDesc Invariant) {
    NewInvariants.push_back(Invariant);
    Result.Invariants[Invariant.GV] = Invariant.Value;
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
    bool ScopeChanged = false;
    do {
      ScopeChanged = false;
      for (auto *BB : BlocksToCheck) {
        // The loop below is an extension of AAResults::canBasicBlockModify()
        // with support of Scope. It avoids scanning instructions before the
        // entry of a scope, or after its exit.
        bool BBViolatesInvariant = false;
        Instruction *AliasedInstr = nullptr;
        Instruction *Begin = BB == Result.Scope.Entry->getParent()
                                 ? Result.Scope.Entry
                                 : &BB->front();
        Instruction *End =
            Result.Scope.Exit && BB == Result.Scope.Exit->getParent()
                ? Result.Scope.Exit
                : nullptr;
        for (Instruction *I = Begin; I != End; I = I->getNextNode())
          if (isModOrRefSet(AAR->getModRefInfo(I, Loc) & ModRefInfo::Mod)) {
            BBViolatesInvariant = true;
            AliasedInstr = I;
            break;
          }
        if (!BBViolatesInvariant)
          continue;

        std::optional<InvariantDesc> MaybeNewInvariant =
            findInvariantForBlock(BB);

        // If we can't exclude the infringing instruction with a new invariant,
        // try scope shrinking.
        if (!MaybeNewInvariant.has_value()) {
          std::optional<ScopeTy> MaybeShrunkScope =
              tryShrinkScope(Result.Scope, AliasedInstr, BlocksToCheck);
          if (!MaybeShrunkScope.has_value()) {
            LLVM_DEBUG(dbgs()
                       << "Cannot find invariant or shrink scope for block "
                       << *BB << "\n");
            return ScopedInvariantSet(*F);
          }
          Result.Scope = MaybeShrunkScope.value();
          ScopeChanged = true;
          break;
        }

        InvariantDesc NewInvariant = MaybeNewInvariant.value();

        // BB may modify Invariant.GV. To keep Invariant.GV constant, we need BB
        // not to be executed, so invert the value of NewInvariant.
        NewInvariant.Value = !NewInvariant.Value;

        auto It = Result.Invariants.find(NewInvariant.GV);
        if (It != Result.Invariants.end()) {
          // The GV we found already exists in the InvariantSet, but the new
          // invariant needs a value for this GV that's different from that in
          // the InvariantSet. We can't satisfy two conflicting values for the
          // same GV, bail out.
          if (It->second != NewInvariant.Value)
            return ScopedInvariantSet(*F);
        } else {
          // This invariant is new, add to worklist to find what may modify its
          // GV in later iterations.
          AddInvariant(NewInvariant);
        }
      }
    } while (ScopeChanged);
  }

  return Result;
}

void GVBasedMultiVersioning::buildInvariantSetsForMultiversioning(
    SmallVectorImpl<ScopedInvariantSet> &Out) {
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
  unsigned NumUses = 0;
  for (const auto &[GV, Branches] : GlobalBranches) {
    unsigned NumBranches = Branches.size();
    if (NumBranches <= NumUses)
      continue;
    PrincipalGlobal = GV;
    NumUses = NumBranches;
  }

  // Bail out if we can't find any eligible branch, or the number of branches is
  // too few.
  if (!PrincipalGlobal) {
    LLVM_DEBUG(dbgs() << "GVMV analysis for " << F->getName()
                      << " bails out, no principal global found\n");
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
  // PrincipalGlobal = true and PrincipalGlobal = false.
  auto TryBuildAndAddInvariantSetWithInitialValue = [this,
                                                     &Out](bool InitialValue) {
    ScopedInvariantSet Result = buildInvariantSetFromInvariant(
        InvariantDesc(PrincipalGlobal, InitialValue));
    if (Result.Invariants.empty())
      return;
    if (Result.Invariants.size() > GVBasedMultiversionMaxNumInvariants) {
      LLVM_DEBUG(dbgs() << "GVMV analysis for " << F->getName()
                        << " created invariant set with "
                        << Result.Invariants.size()
                        << " invariants, too many for multiversioning.\n");
      return;
    }
    LLVM_DEBUG(dbgs() << "GVMV analysis for " << F->getName()
                      << " created invariant set: ";
               for (const auto &Invariant
                    : Result.Invariants) dbgs()
               << "(" << Invariant.first->getName() << " = " << Invariant.second
               << ") ";
               dbgs() << "Scope: " << Result.Scope << "\n");
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

// Given an InvariantSet, create instructions to check all the invariants.
// Returns an i1 Value.
// Variables from the invariants are fetched by calling MakeLoadInst, to allow
// reuse of instruction sequences among multiple invariants.
template <typename MakeLoadInstTy>
Value *buildConditionForInvariantSet(const InvariantSet &Invariants,
                                     IRBuilder<> &Builder,
                                     MakeLoadInstTy MakeLoadInst) {
  assert(!Invariants.empty());
  auto *Int1Ty = IntegerType::getInt1Ty(Builder.getContext());
  if (Invariants.empty())
    return ConstantInt::get(Int1Ty, 1);
  SmallVector<Value *, 4> Values;
  for (auto &Invariant : Invariants) {
    Value *Load = MakeLoadInst(Invariant.first, Builder);
    Values.push_back(Invariant.second ? Load : Builder.CreateNot(Load));
  }
  return Builder.CreateAnd(Values);
}

// Inside a region cloned from the original function, substitute uses of all
// variables in the InvariantSet with the constant stored in the corresponding
// invariant.
static void
applyInvariantSetToClone(const InvariantSet &Invariants,
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
}

// Clone all blocks in BBs and apply Invariants to the new clone. Mapping
// between the old BBs and cloned ones is returned in VMap.
static void cloneBBsWithInvariants(ArrayRef<BasicBlock *> BBs,
                                   const InvariantSet &Invariants,
                                   ValueToValueMapTy &VMap) {
  if (BBs.empty())
    return;
  Function *F = BBs[0]->getParent();
  SmallVector<BasicBlock *, 32> CloneBBs;
  for (auto *OrigBB : BBs) {
    auto CloneBB = CloneBasicBlock(OrigBB, VMap, ".clone", F);
    VMap[OrigBB] = CloneBB;
    CloneBBs.push_back(CloneBB);
  }
  remapInstructionsInBlocks(CloneBBs, VMap);
  applyInvariantSetToClone(Invariants, CloneBBs);
  return;
}

static void
cloneForUnscopedInvariantSets(Function *F,
                              ArrayRef<InvariantSet> UnscopedInvariantSets) {
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

  // First, do cloning for the unscoped invariant sets.
  // Clone the function body for every InvariantSet and generate dispatch code
  // to each clone. Reuse the original code if possible.
  const bool ReuseOriginalCode = canReuseOriginalCode(UnscopedInvariantSets);
  for (const auto &Invariants : UnscopedInvariantSets) {
    if (ReuseOriginalCode && &Invariants == &UnscopedInvariantSets.back()) {
      LLVM_DEBUG(dbgs() << "GVMV transformation for " << F->getName() << ": "
                        << "Reusing the original code.\n");
      applyInvariantSetToClone(Invariants, OrigBBs);
      continue;
    }

    ValueToValueMapTy VMap;
    cloneBBsWithInvariants(OrigBBs, Invariants, VMap);
    // Create a new BB to check the InvariantSet and jump to the new clone. Then
    // use it as the new successor of GlobalLoadsBB.
    BasicBlock *CondBB = BasicBlock::Create(F->getParent()->getContext(),
                                            "mv.cond", F, BBAfterLoadsBB);
    IRBuilder<> Builder(CondBB);

    Type *Int1Ty = IntegerType::getInt1Ty(F->getContext());
    auto BuildOrReuseGlobalLoadInst = [&LoadsForGlobals, EntryBr,
                                       Int1Ty](GlobalVariable *GV,
                                               IRBuilder<> &Builder) {
      auto It = LoadsForGlobals.find(GV);
      if (It == LoadsForGlobals.end())
        It = LoadsForGlobals
                 .insert(std::make_pair(
                     GV, new LoadInst(Int1Ty, GV, "mv.load." + GV->getName(),
                                      EntryBr)))
                 .first;
      return It->second;
    };
    Value *Condition = buildConditionForInvariantSet(
        Invariants, Builder, BuildOrReuseGlobalLoadInst);

    Builder.CreateCondBr(Condition, cast<BasicBlock>(VMap.lookup(OrigEntryBB)),
                         BBAfterLoadsBB);
    EntryBr->setSuccessor(0, CondBB);
    BBAfterLoadsBB = CondBB;
  }
}

static void cloneForScopedInvariantSet(Function *F, ScopedInvariantSet SIS) {
  // First, split the parent block of Scope.Entry, the block before Entry will
  // contain the conditional branch for dispatch, the block after Entry
  // (including Entry) will be part of the cloned scope.
  BasicBlock *CondBB = SIS.Scope.Entry->getParent();
  BasicBlock *ScopeEntryBB =
      SplitBlock(CondBB, SIS.Scope.Entry, (DomTreeUpdater *)nullptr, nullptr,
                 nullptr, "mv.scope.entry");
  // Remove the unconditional branch between CondBB and ScopeEntryBB added by
  // SplitBlock so we can add the conditional branch later.
  cast<BranchInst>(CondBB->getTerminator())->eraseFromParent();

  // Do the same split for Scope.Exit. ScopeExitBB doesn't contain Scope.Exit
  // and will be part of the scope. The two variants of code in scope will
  // branch to BBAfterScopeExit.
  // If Scope.Exit is nullptr, there is no need to have these blocks because
  // the function exits as the control flow leaves the scope.
  BasicBlock *ScopeExitBB =
      SIS.Scope.Exit ? SIS.Scope.Exit->getParent() : nullptr;
  BasicBlock *BBAfterScopeExit =
      ScopeExitBB
          ? SplitBlock(ScopeExitBB, SIS.Scope.Exit, (DomTreeUpdater *)nullptr,
                       nullptr, nullptr, "mv.scope.exit")
          : nullptr;

  // Build a set of blocks in the scope, that includes Scope.Blocks,
  // ScopeEntryBB and ScopeExitBB.
  DenseSet<BasicBlock *> OrigBBsInScopeSet = SIS.Scope.Blocks;
  OrigBBsInScopeSet.insert(ScopeEntryBB);
  if (ScopeExitBB)
    OrigBBsInScopeSet.insert(ScopeExitBB);

  // Build a list of blocks in the scope, by filtering all blocks in the
  // function with OrigBBsInScopeSet.
  // This is not mandatory for the transformation, however we'd like to retain
  // the overall order of BBs in the cloned IR which is usually easier to read
  // for convenience of debugging.
  SmallVector<BasicBlock *, 32> OrigBBsInScopeList;
  for (auto &BB : *F)
    if (OrigBBsInScopeSet.find(&BB) != OrigBBsInScopeSet.end())
      OrigBBsInScopeList.push_back(&BB);

  // Compute live-out values for the scope by checking whether there is any
  // user not in scope for every instruction inside the scope.
  // Alternatively, we can first do the cloning and check whether every
  // instruction dominates its uses, saving the work of computing
  // Scope.Blocks. But that requires updating the dominator tree first, which
  // is also cumbersome.
  SmallVector<Value *, 32> LiveOuts;
  if (ScopeExitBB)
    for (auto Block : OrigBBsInScopeList)
      for (auto &Inst : *Block) {
        auto Users = Inst.users();
        if (std::any_of(Users.begin(), Users.end(), [&](User *U) {
              return OrigBBsInScopeSet.find(
                         cast<Instruction>(U)->getParent()) ==
                     OrigBBsInScopeSet.end();
            }))
          LiveOuts.push_back(&Inst);
      }

  // Clone code in scope and generate dispatch code
  ValueToValueMapTy VMap;
  cloneBBsWithInvariants(OrigBBsInScopeList, SIS.Invariants, VMap);

  IRBuilder<> Builder(CondBB);
  Type *Int1Ty = IntegerType::getInt1Ty(F->getContext());
  Value *Condition = buildConditionForInvariantSet(
      SIS.Invariants, Builder,
      [Int1Ty](GlobalVariable *GV, IRBuilder<> &Builder) {
        return Builder.CreateLoad(Int1Ty, GV,
                                  "mv.scoped.load." + GV->getName());
      });
  Builder.CreateCondBr(Condition, cast<BasicBlock>(VMap.lookup(ScopeEntryBB)),
                       ScopeEntryBB);

  if (ScopeExitBB) {
    // Wire the clone of ScopeExitBB to BBAfterScopeExit
    BasicBlock *ScopeExitBBClone = cast<BasicBlock>(VMap.lookup(ScopeExitBB));
    cast<BranchInst>(ScopeExitBBClone->getTerminator())
        ->setSuccessor(0, BBAfterScopeExit);

    // Create PHIs for live-outs
    Builder.SetInsertPoint(&BBAfterScopeExit->front());
    for (auto LiveOut : LiveOuts) {
      PHINode *PHI = Builder.CreatePHI(LiveOut->getType(), 2,
                                       LiveOut->getName() + ".mv.liveout");
      for (auto U : LiveOut->users())
        if (OrigBBsInScopeSet.find(cast<Instruction>(U)->getParent()) ==
            OrigBBsInScopeSet.end())
          U->replaceUsesOfWith(LiveOut, PHI);
      PHI->addIncoming(LiveOut, ScopeExitBB);
      PHI->addIncoming(VMap.lookup(LiveOut), ScopeExitBBClone);
    }
  }
}

void GVBasedMultiVersioning::doTransformation(
    ArrayRef<ScopedInvariantSet> BranchInvariantSets) {
  // Each entry of the input is handled in different ways, depending on whether
  // its scope is the entire function.
  SmallVector<InvariantSet, 4> UnscopedInvariantSets;
  // At most one scoped invariant set is supported.
  std::optional<ScopedInvariantSet> MaybeScopedInvariantSet;
  for (auto &ScopedInvariants : BranchInvariantSets)
    if (ScopedInvariants.Scope.coversWholeFunction())
      UnscopedInvariantSets.push_back(ScopedInvariants.Invariants);
    else if (!MaybeScopedInvariantSet.has_value())
      MaybeScopedInvariantSet = ScopedInvariants;

  // There is no need to support both scoped and unscoped invariant sets present
  // simultaneously yet.
  if (UnscopedInvariantSets.size())
    cloneForUnscopedInvariantSets(F, UnscopedInvariantSets);
  else if (MaybeScopedInvariantSet.has_value())
    cloneForScopedInvariantSet(F, MaybeScopedInvariantSet.value());

  // Run some CFG simplification transformations to constant fold branches on
  // the globals.
  // Currently it's done globally. This is okay because it's rarely triggered.
  // We can limit it to a subset of related BBs but that'll complicate the code
  // significantly.
  bool Changed = true;
  while (Changed) {
    Changed = false;
    for (Function::iterator It = F->begin(); It != F->end();) {
      BasicBlock &BB = *It++;
      Changed |= simplifyCFG(&BB, GetTTI());
    }
  }
  removeUnreachableBlocks(*F);
}

bool GVBasedMultiVersioning::run() {
  SmallVector<ScopedInvariantSet, 2> InvariantSets;
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

} // namespace

PreservedAnalyses GVBasedMultiVersioningPass::run(Function &F,
                                                  FunctionAnalysisManager &AM) {
  if (DisableGVBasedMultiversioning)
    return PreservedAnalyses::all();

  auto &AAR = AM.getResult<AAManager>(F);
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);

  GVBasedMultiVersioning GVMV(
      &F, &DT, &AAR,
      [&AM, &F ]() -> auto & { return AM.getResult<LoopAnalysis>(F); },
      [&AM, &
       F ]() -> auto & { return AM.getResult<PostDominatorTreeAnalysis>(F); },
      [&AM, &F ]() -> auto & { return AM.getResult<TargetIRAnalysis>(F); });
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
