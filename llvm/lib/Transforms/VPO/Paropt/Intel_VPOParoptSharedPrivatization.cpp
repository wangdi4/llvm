//===------------ Intel_VPOParoptSharedPrivatization.cpp ------------------===//
//
//   Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements optimization pass that privatizes shared items in work
/// regions where it is safe to do so.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Paropt/Intel_VPOParoptSharedPrivatization.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-paropt-shared-privatization"
#define PASS_NAME "VPO Paropt Shared Privatization Pass"

// If true check if map clause for variables of structure type can be changed
// to a firstprivate.
static cl::opt<bool> CheckMapForStructs(
    "vpo-paropt-sp-check-map-for-structs", cl::Hidden, cl::init(true),
    cl::desc("Check if map clause for variables of structure type can be "
             "changed to a firstprivate"));

static cl::opt<bool>
    CleanupRedundantClauses("vpo-paropt-sp-cleanup-redundant-clauses",
                            cl::Hidden, cl::init(true),
                            cl::desc("Change redundant clauses into private"));

static bool privatizeSharedItems(Function &F, WRegionInfo &WI,
                                 OptimizationRemarkEmitter &ORE,
                                 OptReportVerbosity::Level ORVerbosity,
                                 unsigned Mode) {
  bool Changed = false;

  // Walk the W-Region Graph top-down, and create W-Region List
  WI.buildWRGraph();

  if (WI.WRGraphIsEmpty()) {
    LLVM_DEBUG(dbgs() << "\nNo WRegion Candidates for Shared Privatization \n");
    return Changed;
  }

  LLVM_DEBUG(WI.print(dbgs()));
  LLVM_DEBUG(dbgs() << PASS_NAME << " for Function: ");
  LLVM_DEBUG(dbgs().write_escaped(F.getName()) << '\n');

  VPOParoptTransform VP(nullptr, &F, &WI, WI.getDomTree(), WI.getLoopInfo(),
                        WI.getSE(), WI.getTargetTransformInfo(),
                        WI.getAssumptionCache(), WI.getTargetLibraryInfo(),
                        WI.getAliasAnalysis(), Mode & OmpOffload, ORVerbosity,
                        ORE, 2, false);

  Changed |= VP.privatizeSharedItems();

  return Changed;
}

PreservedAnalyses
VPOParoptSharedPrivatizationPass::run(Function &F,
                                      FunctionAnalysisManager &AM) {
  WRegionInfo &WI = AM.getResult<WRegionInfoAnalysis>(F);
  auto &ORE = AM.getResult<OptimizationRemarkEmitterAnalysis>(F);
  auto ORVerbosity = AM.getResult<OptReportOptionsAnalysis>(F).getVerbosity();

  PreservedAnalyses PA;

  LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");
  if (!privatizeSharedItems(F, WI, ORE, ORVerbosity, Mode))
    PA = PreservedAnalyses::all();
  else
    PA = PreservedAnalyses::none();
  LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");

  return PA;
}

namespace {

class VPOParoptSharedPrivatization : public FunctionPass {
public:
  static char ID;

  explicit VPOParoptSharedPrivatization(unsigned Mode = 0u)
      : FunctionPass(ID), Mode(Mode) {
    initializeVPOParoptSharedPrivatizationPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    WRegionInfo &WI = getAnalysis<WRegionInfoWrapperPass>().getWRegionInfo();
    auto &ORE = getAnalysis<OptimizationRemarkEmitterWrapperPass>().getORE();
    auto ORVerbosity = getAnalysis<OptReportOptionsPass>().getVerbosity();

    LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");
    bool Changed = privatizeSharedItems(F, WI, ORE, ORVerbosity, Mode);
    LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");
    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<WRegionInfoWrapperPass>();
    AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
    AU.addRequired<OptReportOptionsPass>();
  }

private:
  unsigned Mode;
};

} // end anonymous namespace

char VPOParoptSharedPrivatization::ID = 0;
INITIALIZE_PASS_BEGIN(VPOParoptSharedPrivatization, DEBUG_TYPE, PASS_NAME,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(OptimizationRemarkEmitterWrapperPass)
INITIALIZE_PASS_DEPENDENCY(OptReportOptionsPass)
INITIALIZE_PASS_END(VPOParoptSharedPrivatization, DEBUG_TYPE, PASS_NAME, false,
                    false)

FunctionPass *llvm::createVPOParoptSharedPrivatizationPass(unsigned Mode) {
  return new VPOParoptSharedPrivatization(Mode);
}

// Returns true if all users of value V and derrived bitcasts/GEPs do not
// capture or modify the value inside BBs.
static bool isReadonlyAndNotCaptured(Value *V,
                                     const SmallPtrSetImpl<BasicBlock *> &BBs) {
  SmallVector<Use *, 8u> Worklist;
  SmallPtrSet<Use *, 8u> Visited;

  auto AddUses = [&Worklist, &Visited, &BBs](Value *V) {
    for (Use &U : V->uses()) {
      if (Visited.contains(&U))
        continue;

      // if user is not an instruction conservatively assume that value is
      // modified/captured.
      auto *I = dyn_cast<Instruction>(U.getUser());
      if (!I)
        return false;

      // Consider only those instructions that are inside BBs.
      if (!BBs.contains(I->getParent()))
        continue;

      Worklist.push_back(&U);
      Visited.insert(&U);
    }
    return true;
  };

  // Verify that all users of value V or derrived bitcasts/GEPs in collected
  // blocks are either load or function call arguments with readonly/readnone
  // and nocapture attributes.
  if (!AddUses(V))
    return false;
  while (!Worklist.empty()) {
    Use *U = Worklist.pop_back_val();
    Value *I = U->getUser();

    if (isa<BitCastInst>(I) || isa<GetElementPtrInst>(I)) {
      if (!AddUses(I))
        return false;
      continue;
    }
    if (auto *Load = dyn_cast<LoadInst>(I)) {
      if (Load->isVolatile())
        return false;
      continue;
    }
    if (auto *Call = dyn_cast<CallBase>(I)) {
      if (auto *MI = dyn_cast<MemIntrinsic>(Call))
        if (MI->isVolatile())
          return false;
      if (!Call->isDataOperand(U) ||
          !Call->doesNotCapture(Call->getDataOperandNo(U)) ||
          !Call->onlyReadsMemory(Call->getDataOperandNo(U)))
        return false;
      continue;
    }
    return false;
  }
  return true;
}

#ifndef NDEBUG
static void reportSkipped(Value *V, const Twine &Msg) {
  dbgs() << "skipping '" << V->getName() << "' - " << Msg << "\n";
}
#endif // NDEBUG

static bool isPrivatizationCandidate(AllocaInst *AI,
                                     const SmallPtrSetImpl<BasicBlock *> &BBs,
                                     AAResults *AA, bool AllowStructs) {
  // Do not attempt to promote arrays or structures.
  if (AI->isArrayAllocation() ||
      !(AI->getAllocatedType()->isSingleValueType() ||
        (AI->getAllocatedType()->isStructTy() && AllowStructs))) {
    LLVM_DEBUG(reportSkipped(AI, "unsupported value type"));
    return false;
  }
  Optional<TypeSize> Size =
      AI->getAllocationSizeInBits(AI->getModule()->getDataLayout());
  if (!Size) {
    LLVM_DEBUG(reportSkipped(AI, "unknown size"));
    return false;
  }

  // Check if item's memory is modified inside the region. If not then it
  // should be safe to privatize it.
  MemoryLocation Loc{AI, LocationSize::precise(*Size)};
  if (any_of(BBs, [&](BasicBlock *BB) {
        for (const Instruction &I : *BB) {
          // Ignore fences when checking if memory location is modified by
          // the instruction since fences do not really modify it though
          // AA assumes they do.
          if (isa<FenceInst>(&I))
            continue;
          if (isModOrRefSet(AA->getModRefInfo(&I, Loc) & ModRefInfo::Mod)) {
            LLVM_DEBUG(reportSkipped(AI, "is modified by");
                       dbgs() << I << "\n";);
            return true;
          }
        }
        return false;
      }))
    return false;
  return true;
}

template <typename ClauseTy>
static bool containsValue(const ClauseTy *C, Value *V) {
  return any_of(C->items(), [V](auto *I) { return I->getOrig() == V; });
}

// Returns private item for value V if it is private in the work region W, or
// nullptr otherwise.
static PrivateItem *getWRNPrivate(WRegionNode *W, Value *V) {
  if (PrivateClause *C = W->getPrivIfSupported())
    for (auto *I : C->items())
      if (I->getOrig() == V)
        return I;
  return nullptr;
}

// Returns true if value V is private in the work region W.
static bool isWRNPrivate(WRegionNode *W, Value *V) {
  return getWRNPrivate(W, V);
}

// Returns true if value V is first-private in the work region W.
static bool isWRNFirstprivate(WRegionNode *W, Value *V) {
  if (FirstprivateClause *C = W->getFprivIfSupported())
    if (containsValue(C, V))
      return true;
  return false;
}

// Returns true if value V is last-private in the work region W.
static bool isWRNLastprivate(WRegionNode *W, Value *V) {
  if (LastprivateClause *C = W->getLprivIfSupported())
    if (containsValue(C, V))
      return true;
  return false;
}

// Collects set of work region blocks where we will be checking if item is used
// or items's memory is modified. This set does not include blocks with
// directives (all directives have their own basic blocks now) and nested
// regions where item is private or first-private.
static SmallPtrSet<BasicBlock *, 16u> findWRNBlocks(WRegionNode *W, Value *V) {
  // Build set of work region blocks where we will be checking if item's
  // memory is modified. Exclude blocks with directives (all directives have
  // their own basic blocks now).
  SmallPtrSet<BasicBlock *, 16u> BBs;
  for (BasicBlock *BB : W->blocks())
    if (!VPOAnalysisUtils::isBeginOrEndDirective(BB))
      BBs.insert(BB);

  // Then exclude nested regions where item will be privatized, unless items is
  // also in the last-private list which means that item will be modified by the
  // region. Any modifications in these regions should not inhibit privatization
  // because it will be done on a private instance.
  SmallVector<WRegionNode *, 8u> Worklist{W};
  do {
    WRegionNode *W = Worklist.pop_back_val();
    for (WRegionNode *CW : W->getChildren()) {
      if ((isWRNPrivate(CW, V) || isWRNFirstprivate(CW, V)) &&
          !isWRNLastprivate(CW, V)) {
        for_each(CW->blocks(), [&BBs](BasicBlock *BB) { BBs.erase(BB); });
        continue;
      }
      Worklist.push_back(CW);
    }
  } while (!Worklist.empty());

  return BBs;
}

// Return the <ElementType, NumElements> pair, that should be used when
// creating a PRIVATE clause for the typed item I.
ElementTypeAndNumElements getTypedClauseInfoForTypedItem(Item *I) {
  assert(I->getIsTyped() && "Item is not typed.");
  assert((isa<SharedItem>(I) || isa<FirstprivateItem>(I) ||
          isa<LastprivateItem>(I)) &&
         "Unexpected clause item.");
  assert(!I->getIsPointerToPointer() &&
         "Typed ptr-to-ptrs are not supported yet.");
  assert(!I->getIsNonPod() && "Typed NONPODs are not supported yet.");
#if INTEL_CUSTOMIZATION
  assert(!I->getIsF90NonPod() && "Typed F90_NONPODs are not supported yet.");
  assert(!I->getIsF90DopeVector() && "Typed F90_DVs are not supported yet.");
#endif // INTEL_CUSTOMIZATION

  Type *ElementTy = I->getOrigItemElementTypeFromIR();
  Value *NumElements = I->getNumElements();

  if (!I->getIsByRef())
    return {ElementTy, NumElements};

  // When privatizing a BYREF operand, the analysis is done for the operand
  // itself, so the PRIVATE should be added on the operand itself, not its
  // byref pointee (i.e. without a BYREF modifier).
  Type *OrigTy = I->getOrig()->getType();
  Type *ByrefVarTy =
      PointerType::get(ElementTy, OrigTy->getPointerAddressSpace());
  return {ByrefVarTy, ConstantInt::get(NumElements->getType(), 1)};
}

// Return true if value V has uses inside work region W excluding
// sub-regions where it is private.
static bool hasWRNUses(WRegionNode *W, Value *V) {
  // Conservatively assume that globals have uses.
  if (GeneralUtils::isOMPItemGlobalVAR(V))
    return true;

  // Collect set of WRN blocks where value can be used. Start with the
  // region's set of blocks excluding begin/end directives.
  SmallPtrSet<BasicBlock *, 16u> BBs;
  for (BasicBlock *BB : W->blocks())
    if (!VPOAnalysisUtils::isBeginOrEndDirective(BB))
      BBs.insert(BB);

  // Then exclude blocks from nested regions where item is private.
  std::queue<WRegionNode *> Worklist;
  Worklist.push(W);
  do {
    WRegionNode *W = Worklist.front();
    Worklist.pop();

    for (WRegionNode *CW : W->getChildren()) {
      // If item is private on the nested child exclude its blocks from
      // the set.
      if (PrivateItem *I = getWRNPrivate(CW, V)) {
#if INTEL_CUSTOMIZATION
        // Dope-vectors, even if they are private, use the original item for
        // initializing private instance. So such items have uses in the region.
        if (I->getIsF90DopeVector())
          return true;
#endif // INTEL_CUSTOMIZATION
        for_each(CW->blocks(), [&BBs](BasicBlock *BB) { BBs.erase(BB); });
        continue;
      }

      // If item is not private check if item is used by the directive
      // itself (there could be uses for example in 'thread_limit' or 'if'
      // clauses). If there are such uses then consider it being used.
      if (any_of(CW->getEntryDirective()->operands(),
                 [V](Value *O) { return O == V; }))
        return true;

      Worklist.push(CW);
    }
  } while (!Worklist.empty());

  // Check if value V has any uses in these blocks.
  for (const User *U : V->users())
    if (auto *I = dyn_cast<Instruction>(U)) {
      if (I->isLifetimeStartOrEnd())
        continue;
      if (is_contained(BBs, I->getParent()))
        return true;
    }

  return false;
}

bool VPOParoptTransform::privatizeSharedItems(WRegionNode *W) {
  if (!W->canHaveShared() || !W->needsOutlining())
    return false;

  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::privatizeSharedItems: "
                    << W->getName() << "\n");

  // Returns true if value V is captured by any nested 'omp task' construct.
  auto IsCapturedByNestedTask = [&, W](Value *V) {
    SmallVector<WRegionNode *, 8u> Worklist{W};
    do {
      WRegionNode *W = Worklist.pop_back_val();
      for (WRegionNode *CW : W->getChildren()) {
        if (isWRNPrivate(CW, V) || isWRNFirstprivate(CW, V))
          continue;
        if (CW->getIsTask() && containsValue(&CW->getShared(), V)) {
          LLVM_DEBUG(reportSkipped(V, "is captured by a nested task"));
          return true;
        }
        Worklist.push_back(CW);
      }
    } while (!Worklist.empty());
    return false;
  };

  // Replaces all uses of value From with To within work region W.
  auto ReplaceWRNUsesOfWith = [W](Value *From, Value *To) {
    SmallVector<Instruction *, 8> Users;
    if (WRegionUtils::findUsersInRegion(W, From, &Users))
      for (auto *User : Users)
        User->replaceUsesOfWith(From, To);
  };

  // Find "shared" candidates that can be privatized.
  SmallVector<AllocaInst *, 8> ToPrivatize;
  for (SharedItem *I : W->getShared().items()) {
    Value *V = I->getOrig();
    if (!V)
      continue;

    if (auto *AI = dyn_cast<AllocaInst>(V)) {
      // Do not do privatization for a shared item if it is captured by a nested
      // task.
      //
      //  PARALLEL SHARED(X)         =>      PARALLEL FIRSTPRIVATE(X)
      //    TASK SHARED(X)                     TASK SHARED(X)
      //
      // Such transformation is illegal since it may cause thread executing task
      // to access X allocated on the stack of thread that created the task when
      // its stack frame is freed.
      if (IsCapturedByNestedTask(AI))
        continue;

      auto BBs = findWRNBlocks(W, AI);

      if (!isPrivatizationCandidate(AI, BBs, AA, /*AllowStructs=*/false) ||
          !isReadonlyAndNotCaptured(AI, BBs))
        continue;

      ToPrivatize.push_back(AI);
      continue;
    }

    // Check if shared item is a bitcasted alloca instruction that can be
    // privatized. If so move bitcast into the work region and change shared
    // item to alloca instruction.
    if (auto *BCI = dyn_cast<BitCastInst>(V))
      if (auto *AI = dyn_cast<AllocaInst>(BCI->getOperand(0))) {

        // TODO: If the BC's region is nested, we cannot make the alloca
        // live-into the inner region without fixing up all the outer
        // regions with map and/or explicit shared. Just disable for now.
        if (W->getParent() && W->needsOutlining())
          continue;

        if (IsCapturedByNestedTask(BCI))
          continue;

        auto BBs = findWRNBlocks(W, AI);

        if (!isPrivatizationCandidate(AI, BBs, AA, /*AllowStructs=*/false) ||
            !isReadonlyAndNotCaptured(BCI, BBs))
          continue;

        // Change shared value to alloca instruction.
        auto *EntryDir = cast<IntrinsicInst>(W->getEntryDirective());
        EntryDir->replaceUsesOfWith(BCI, AI);
        I->setOrig(AI);

        // And move bitcast into the region.
        BasicBlock *EntrySuccessor = W->getEntryBBlock()->getSingleSuccessor();
        assert(EntrySuccessor && "Entry block must have a single successor");
        auto *NewBCI = cast<BitCastInst>(BCI->clone());
        NewBCI->insertBefore(EntrySuccessor->getFirstNonPHI());

        ReplaceWRNUsesOfWith(BCI, NewBCI);

        // Add alloca to the privatization list.
        ToPrivatize.push_back(AI);
        continue;
      }

    LLVM_DEBUG(reportSkipped(V, "not a local pointer"));
  }

  if (ToPrivatize.empty())
    return false;

  // Create separate block for alloca and load/store instructions.
  BasicBlock *EntryBB = W->getEntryBBlock();
  BasicBlock *NewBB = SplitBlock(EntryBB, EntryBB->getTerminator(), DT, LI);
  Instruction *InsPt = NewBB->getTerminator();

  // Create private instances for variables collected earlier.
  for (AllocaInst *AI : ToPrivatize) {
    LLVM_DEBUG(dbgs() << "privatizing '" << AI->getName() << "'\n");

    // Allocate space for the private copy.
    auto *NewAI = cast<AllocaInst>(AI->clone());
    NewAI->setName(AI->getName() + ".fp");
    NewAI->insertBefore(InsPt);

    // Copy variable value from the original location to the private instance.
    new StoreInst(
        new LoadInst(AI->getAllocatedType(), AI, AI->getName() + ".v", InsPt),
        NewAI, InsPt);

    // And replace all uses of the original variable in the region with the
    // private one.
    ReplaceWRNUsesOfWith(AI, NewAI);
  }

  // Need to refresh BBSet because CFG has been changed.
  W->populateBBSet(/*Always=*/true);

  return true;
}

// Remove item's uses from the clause bundles.
template <typename ItemTy>
static bool cleanupItem(
    WRegionNode *W, ItemTy *Item, int ClauseID,
    MapVector<Value *, llvm::Optional<ElementTypeAndNumElements>> &ToPrivatize,
    Function *F, WRegionListTy &WRegionList, OptReportBuilder &ORBuilder,
    bool hasOffloadCompilation) {
  bool Changed = false;
  Value *V = Item->getOrig();

  StringRef ClauseName = VPOAnalysisUtils::getOmpClauseName(ClauseID);
  LLVM_DEBUG(dbgs() << ClauseName << " clause for '" << V->getName() << "' on '"
                    << W->getName() << "' construct is redundant\n");

  // Emit diagnostic in the host compilation only to avoid duplicating
  // remarks.
  if (!hasOffloadCompilation)
    F->getContext().diagnose(
        OptimizationRemarkAnalysis("openmp", "optimization note",
                                   W->getEntryDirective())
        << ClauseName << " clause for variable '" << V->getName() << "' on '"
        << W->getName() << "' construct is redundant");

  // Do not change target regions so far because we cannot be sure that
  // both host and device sides will be changed the same way.
  if (isa<WRNTargetNode>(W) || !CleanupRedundantClauses) {
    ORBuilder(*W, WRegionList)
        .addRemark(OptReportVerbosity::Low,
                   (Twine(ClauseName) + " clause for variable '" +
                    V->getName() + "' is redundant")
                       .str());
    return Changed;
  }

  ORBuilder(*W, WRegionList)
      .addRemark(OptReportVerbosity::Low,
                 (Twine(ClauseName) + " clause for variable '" + V->getName() +
                  "' has been changed to " +
                  VPOAnalysisUtils::getOmpClauseName(QUAL_OMP_PRIVATE))
                     .str());

  // Add V to the list of items to be privatized.
  if (!ToPrivatize.count(V)) {
    if (Item->getIsTyped())
      ToPrivatize.insert({V, getTypedClauseInfoForTypedItem(Item)});
    else if (!V->getType()->isOpaquePointerTy())
      ToPrivatize.insert({V, llvm::None});
    else if (auto *AI = dyn_cast<AllocaInst>(V))
      ToPrivatize.insert({AI, VPOUtils::getTypedClauseInfoForAlloca(AI)});
    else
      llvm_unreachable("Need TYPED clause item to support opaque pointers.");
  }

  // And replace its uses in the clause list with null.
  auto *Entry = cast<CallInst>(W->getEntryDirective());
  for (auto &BOI : make_range(std::next(Entry->bundle_op_info_begin()),
                              Entry->bundle_op_info_end())) {
    // Get clause ID and check if it is a clause of interest.
    ClauseSpecifier CS(BOI.Tag->getKey());
    if (CS.getId() != ClauseID)
      continue;

    // Check bundle operand uses and zap the ones matching given value.
    for (unsigned I = BOI.Begin, E = BOI.End; I < E; ++I) {
      Use &U = Entry->getOperandUse(I);
      if (V != U.get())
        continue;

      // Change item's value to null.
      Value *NewV = Constant::getNullValue(U->getType());
      U.set(NewV);
      Item->setOrig(nullptr);
      Changed = true;
    }
  }

  return Changed;
}

bool VPOParoptTransform::addPrivateClausesToRegion(
    WRegionNode *W,
    ArrayRef<std::pair<Value *, llvm::Optional<ElementTypeAndNumElements>>>
        ToPrivatize) {

  if (ToPrivatize.empty())
    return false;

  assert(W->canHavePrivate() && "Region cannot have private clauses.");
  PrivateClause &PrivC = W->getPriv();

  StringRef PrivateClauseStr =
      VPOAnalysisUtils::getClauseString(QUAL_OMP_PRIVATE);
  std::string TypedPrivateClauseStr = PrivateClauseStr.str() + ":TYPED";

  SmallVector<std::pair<StringRef, SmallVector<Value *, 3>>, 8> PrivateBundles;
  PrivateBundles.reserve(ToPrivatize.size());

  for (const auto &I : ToPrivatize) {
    Value *V = I.first;

    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Adding private clause for '";
               V->printAsOperand(dbgs()); dbgs() << "'.\n");

    if (!I.second.has_value()) {
      PrivateBundles.push_back({PrivateClauseStr, {V}});
      PrivC.add(V);
      continue;
    }

    Type *ElementTy = nullptr;
    Value *NumElements = nullptr;
    std::tie(ElementTy, NumElements) = I.second.value();
    PrivateBundles.push_back(
        {TypedPrivateClauseStr,
         {V, Constant::getNullValue(ElementTy), NumElements}});

    PrivC.add(V);
    PrivateItem *PI = PrivC.back();
    PI->setIsTyped(true);
    PI->setOrigItemElementTypeFromIR(ElementTy);
    PI->setNumElements(NumElements);
  }

  CallInst *NewEntry = VPOUtils::addOperandBundlesInCall(
      cast<CallInst>(W->getEntryDirective()), makeArrayRef(PrivateBundles));
  W->setEntryDirective(NewEntry);
  return true;
}

bool VPOParoptTransform::simplifyRegionClauses(WRegionNode *W) {
  LLVM_DEBUG(dbgs() << "\nEnter VPOParoptTransform::simplifyRegionClauses: "
                    << W->getName() << "\n");

  // Special handling for a target constructs on the host side. Check map
  // clauses for scalars that are readonly inside the target region. Emit
  // optimization report that such variables can be changed to firstprivate
  // to reduce overhead.
  // Such change cannot be done by the compiler now because we cannot guarantee
  // that both host and target compilations will do this convertsion, but it has
  // to be done on both sides because it changes signature of the outlined
  // target region.
  if (isa<WRNTargetNode>(W) && !hasOffloadCompilation()) {
    // Return true if given map item maps scalar value.
    auto IsScalarMapItem = [this](const MapItem *MI) {
      assert(MI->getIsMapChain() && "map chain is expected");

      const MapChainTy &MC = MI->getMapChain();
      if (MC.size() > 1)
        return false;

      MapAggrTy *MA = MC[0];
      if (MA->getMapper())
        return false;

      if (MA->getBasePtr() != MA->getSectionPtr())
        return false;

      auto Size = dyn_cast<ConstantInt>(MA->getSize());
      if (!Size)
        return false;

      Type *ItemTy = nullptr;
      Value *NumElements = nullptr;
      // TODO: this will stop working with opaque pointers, because
      //       we do not plan to have type information for MAP clauses yet.
      std::tie(ItemTy, NumElements, std::ignore) =
          VPOParoptUtils::getItemInfo(MI);
      if (NumElements)
        return false;
      TypeSize ElemSize =
          F->getParent()->getDataLayout().getTypeAllocSize(ItemTy);
      if (Size->getValue() != ElemSize)
        return false;

      return true;
    };

    // Returns true if given pointer may be mapped before the work region.
    // TODO: this code just checks if there are any dominating work regions
    // which may map given value, which may result in false positives. More
    // precise analysis would require building live ranges for corresponding
    // items in device data environment which are created by the dominating
    // constructs with map clauses and checking if the corresponding device
    // item is alive at the work region's entry.
    auto MayBeMappedBefore = [this](AllocaInst *AI, WRegionNode *W) {
      // If pointer is captured we cannot guarantee that it was not mapped
      // somewhere else.
      if (PointerMayBeCapturedBefore(AI, /*ReturnCaptures=*/true,
                                     /*StoreCaptures=*/true,
                                     W->getEntryDirective(), DT))
        return true;

      // Check if there are dominating work regions in this routine which can
      // map this pointer.
      for (WRegionNode *N : WRegionList) {
        if (N == W)
          continue;

        if (!N->canHaveMap())
          continue;

        if (!DT->dominates(N->getEntryBBlock(), W->getEntryBBlock()))
          continue;

        for (const MapItem *MI : N->getMap().items())
          if (!AA->isNoAlias(MI->getOrig(), AI))
            return true;
      }
      return false;
    };

    auto IsMapTo = [](const MapItem *MI) {
      uint64_t MapType = MI->getMapChain()[0]->getMapType();
      return (MapType & TGT_MAP_TO) &&
             !((MapType & TGT_MAP_PRIVATE) || (MapType & TGT_MAP_LITERAL));
    };
    auto IsMapFrom = [](const MapItem *MI) {
      uint64_t MapType = MI->getMapChain()[0]->getMapType();
      return (MapType & TGT_MAP_FROM);
    };
    auto IsMapTofrom = [&](const MapItem *MI) {
      return IsMapTo(MI) && IsMapFrom(MI);
    };

    auto GetMapName = [&](const MapItem *MI) {
      if (IsMapTofrom(MI))
        return "MAP:TOFROM";
      if (IsMapTo(MI))
        return "MAP:TO";
      if (IsMapFrom(MI))
        return "MAP:FROM";
      return "MAP";
    };

    for (const MapItem *MI : W->getMap().items()) {
      auto *AI = dyn_cast<AllocaInst>(MI->getOrig());
      if (!AI) {
        LLVM_DEBUG(reportSkipped(MI->getOrig(), "not a local pointer"));
        continue;
      }

      if (MayBeMappedBefore(AI, W)) {
        LLVM_DEBUG(reportSkipped(AI, "may be mapped before work region"));
        continue;
      }

      if (!MI->getIsMapChain()) {
        LLVM_DEBUG(reportSkipped(AI, "not a map chain"));
        continue;
      }

      if ((IsMapTo(MI) || IsMapFrom(MI)) && !hasWRNUses(W, AI)) {
        LLVM_DEBUG(dbgs() << GetMapName(MI) << " clause for '" << AI->getName()
                          << "' on '" << W->getName()
                          << "' construct is redundant\n");

        F->getContext().diagnose(
            OptimizationRemarkAnalysis("openmp", "optimization note",
                                       W->getEntryDirective())
            << GetMapName(MI) << " clause for variable '" << AI->getName()
            << "' on '" << W->getName() << "' construct is redundant");

        ORBuilder(*W, WRegionList)
            .addRemark(OptReportVerbosity::Low,
                       (Twine(GetMapName(MI)) + " clause for variable '" +
                        AI->getName() + "' is redundant")
                           .str());
        continue;
      }

      if (!IsMapTo(MI)) {
        LLVM_DEBUG(reportSkipped(MI->getOrig(), "is not a map to/tofrom"));
        continue;
      }

      if (!IsScalarMapItem(MI)) {
        LLVM_DEBUG(reportSkipped(MI->getOrig(), "is not a scalar item"));
        continue;
      }

      auto BBs = findWRNBlocks(W, AI);

      if (!isPrivatizationCandidate(AI, BBs, AA, CheckMapForStructs) ||
          !isReadonlyAndNotCaptured(AI, BBs))
        continue;

      LLVM_DEBUG(dbgs() << GetMapName(MI) << " clause for '" << AI->getName()
                        << "' on '" << W->getName()
                        << "' construct can be changed to FIRSTPRIVATE\n");

      F->getContext().diagnose(
          OptimizationRemarkAnalysis("openmp", "optimization note",
                                     W->getEntryDirective())
          << GetMapName(MI) << " clause for variable '" << AI->getName()
          << "' on '" << W->getName()
          << "' construct can be changed to FIRSTPRIVATE to reduce mapping "
             "overhead");

      ORBuilder(*W, WRegionList)
          .addRemark(
              OptReportVerbosity::Low,
              (Twine(GetMapName(MI)) + " clause for variable '" +
               AI->getName() + "' can be changed to " +
               VPOAnalysisUtils::getOmpClauseName(QUAL_OMP_FIRSTPRIVATE) +
               " to reduce mapping overhead")
                  .str());
    }
  }

  if (!W->canHavePrivate())
    return false;

  bool Changed = false;
  MapVector<Value *, llvm::Optional<ElementTypeAndNumElements>> ToPrivatize;

  auto CleanupRedundantItems = [this, W, &ToPrivatize](auto *Clause) {
    bool Changed = false;
    for (auto *Item : Clause->items()) {
      // Do not try to simplify nonPOD items.
      if (Item->getIsNonPod())
        continue;

      Value *V = Item->getOrig();
      if (!V || hasWRNUses(W, V))
        continue;

      // Special handling for the schedule chunk that is loaded from a shared
      // pointer that has no uses inside the region
      //
      // void foo(int N, int C) {
      // #pragma omp parallel for schedule(static, C)
      //   for (int I = 0; I < N; ++I) {}
      // }
      //
      // generated code looks as follows
      //
      // %.capture_expr.0 = alloca i32, align 4
      // ...
      // %5 = load i32, i32* %.capture_expr.0
      // %6 = call token @llvm.directive.region.entry() [
      //         "DIR.OMP.PARALLEL.LOOP"(),
      //         "QUAL.OMP.SCHEDULE.STATIC"(i32 %5),
      //         "QUAL.OMP.SHARED"(i32* %.capture_expr.0),
      //         ...
      //
      // The load gets moved into the region later by paropt transform while
      // lowering the loop, so turning shared %.capture_expr.0 into private
      // leads to invalid results.
      if (isa<SharedItem>(Item) && W->canHaveSchedule())
        if (auto *Chunk =
                dyn_cast_or_null<LoadInst>(W->getSchedule().getChunkExpr()))
          if (Chunk->getPointerOperand() == V)
            continue;

      // Item's value is not used inside the region, so it should be safe to
      // turn it into a private. Print a diagnostic that clause is redundant
      // and remove item's uses from clause bundles.
      Changed |= cleanupItem(W, Item, Clause->getClauseID(), ToPrivatize, F,
                             WRegionList, ORBuilder, hasOffloadCompilation());

      // Special case first-private items which can also be listed in the
      // last-private clause. Item has to be cleaned up in both clauses.
      if (isa<FirstprivateItem>(Item))
        if (LastprivateClause *LPClause = W->getLprivIfSupported())
          for (auto *LPItem : LPClause->items())
            if (LPItem->getOrig() == V)
              Changed |= cleanupItem(W, LPItem, LPClause->getClauseID(),
                                     ToPrivatize, F, WRegionList, ORBuilder,
                                     hasOffloadCompilation());
    }
    return Changed;
  };

  if (auto *Clause = W->getFprivIfSupported())
    Changed |= CleanupRedundantItems(Clause);
  if (auto *Clause = W->getSharedIfSupported())
    Changed |= CleanupRedundantItems(Clause);
  if (auto *Clause = W->getLprivIfSupported())
    Changed |= CleanupRedundantItems(Clause);

  Changed |= addPrivateClausesToRegion(W, ToPrivatize.takeVector());

  // This routine does not change CFG, so there is no need to be refresh BB set.
  return Changed;
}

bool VPOParoptTransform::simplifyLastprivateClauses(WRegionNode *W) {
  LLVM_DEBUG(
      dbgs() << "\nEnter VPOParoptTransform::simplifyLastprivateClauses: "
             << W->getName() << "\n");

  if (!W->canHavePrivate() || !W->canHaveLastprivate() || W->getLpriv().empty())
    return false;

  bool Changed = false;
  MapVector<Value *, llvm::Optional<ElementTypeAndNumElements>> ToPrivatize;

  for (LastprivateItem *Item : W->getLpriv().items()) {
    // Do not try to simplify byref items because existing analysis is not
    // sufficient for them.
    if (Item->getIsByRef())
      continue;

    // Also skip nonPOD items.
    if (Item->getIsNonPod())
      continue;

    Value *V = Item->getOrig();
    if (!V)
      continue;

    // So far we will try to simplify locally allocated scalars only.
    auto *AI = dyn_cast<AllocaInst>(V->stripPointerCasts());
    if (!AI || AI->isArrayAllocation() ||
        !AI->getAllocatedType()->isSingleValueType())
      continue;

    Optional<TypeSize> Size =
        AI->getAllocationSizeInBits(AI->getModule()->getDataLayout());
    if (!Size)
      continue;

    // Collect set of basic blocks where we will be checking if value is used.
    SmallPtrSet<BasicBlock *, 8u> BBs;
    if (WRegionNode *PW = W->getParent()) {
      // If there is a parent work region check how that value is being used. If
      // value is not privatized by the region assume that it is being
      // propagated outside of the region.
      if (!isWRNPrivate(PW, V) &&
          !(isWRNLastprivate(PW, V) && !isWRNFirstprivate(PW, V)))
        continue;

      // Start with adding parent region's block to the list excluding its
      // entry/exit blocks and the blocks which cannot be reached from the work
      // region (i.e. lastprivate defined by it cannot be used by those blocks).
      for (BasicBlock *BB : PW->blocks()) {
        if (BB == PW->getEntryBBlock() || BB == PW->getExitBBlock())
          continue;
        if (!isPotentiallyReachable(W->getExitBBlock(), BB, nullptr, DT, LI))
          continue;
        BBs.insert(BB);
      }

      // Also exclude child region's blocks except entry blocks. Entries are
      // needed for checking how value is used by the child region.
      for (WRegionNode *CW : PW->getChildren())
        for (BasicBlock *BB : CW->blocks())
          if (BB != CW->getEntryBBlock())
            BBs.erase(BB);
    } else {
      // If there is no parent region add all function's blocks to the list
      // exluding ones that cannot be reached from the directive.
      for (BasicBlock &BB : *F) {
        if (!isPotentiallyReachable(W->getExitBBlock(), &BB, nullptr, DT, LI))
          continue;
        BBs.insert(&BB);
      }

      // And similarly exclude all child region's blocks except entries.
      for (WRegionNode *CW : WRegionList) {
        if (CW->getParent())
          continue;
        for (BasicBlock *BB : CW->blocks())
          if (BB != CW->getEntryBBlock())
            BBs.erase(BB);
      }
    }

    MemoryLocation Loc{AI, LocationSize::precise(*Size)};
    if (none_of(BBs, [&](BasicBlock *BB) {
          if (VPOAnalysisUtils::isBeginDirective(&BB->front())) {
            // Check if directive uses value in any way other than private or
            // lastprivate. If so, considered the item being used.
            auto *Entry = cast<CallInst>(&BB->front());
            if (any_of(Entry->bundle_op_infos(),
                       [this, Entry, V](const CallBase::BundleOpInfo &BOI) {
                         for (unsigned I = BOI.Begin, E = BOI.End; I < E; ++I) {
                           Use &U = Entry->getOperandUse(I);
                           if (AA->isNoAlias(V, U.get()))
                             continue;

                           ClauseSpecifier CS(BOI.Tag->getKey());
                           if (CS.getId() != QUAL_OMP_PRIVATE &&
                               CS.getId() != QUAL_OMP_LASTPRIVATE)
                             return true;
                         }
                         return false;
                       }))
              return true;
            return false;
          }

          // Check if any block's instruction references the item's memory.
          for (const Instruction &I : *BB) {
            if (I.isLifetimeStartOrEnd() || isa<FenceInst>(I))
              continue;
            if (isRefSet(AA->getModRefInfo(&I, Loc)))
              return true;
          }
          return false;
        }))
      Changed |=
          cleanupItem(W, Item, W->getLpriv().getClauseID(), ToPrivatize, F,
                      WRegionList, ORBuilder, hasOffloadCompilation());
  }

  Changed |= addPrivateClausesToRegion(W, ToPrivatize.takeVector());

  // This routine does not change CFG, so there is no need to be refresh BB set.
  return Changed;
}

bool VPOParoptTransform::privatizeSharedItems() {
  bool NeedTID, NeedBID;
  gatherWRegionNodeList(NeedTID, NeedBID);

  bool Changed = false;
  for (auto *W : WRegionList) {
    W->populateBBSet();
    switch (W->getWRegionKindID()) {
    case WRegionNode::WRNTarget:
      Changed |= simplifyRegionClauses(W);
      break;
    case WRegionNode::WRNTeams:
    case WRegionNode::WRNParallel:
      Changed |= simplifyRegionClauses(W);
      Changed |= privatizeSharedItems(W);
      break;
    case WRegionNode::WRNParallelSections:
    case WRegionNode::WRNParallelLoop:
    case WRegionNode::WRNDistributeParLoop:
      Changed |= simplifyRegionClauses(W);
      Changed |= privatizeSharedItems(W);
      break;
    case WRegionNode::WRNTask:
    case WRegionNode::WRNTaskloop:
    case WRegionNode::WRNSections:
    case WRegionNode::WRNSingle:
    case WRegionNode::WRNWksLoop:
    case WRegionNode::WRNDistribute:
    case WRegionNode::WRNVecLoop:
      Changed |= simplifyRegionClauses(W);
      break;
    }
  }

  for (auto *W : reverse(WRegionList))
    Changed |= simplifyLastprivateClauses(W);

  return Changed;
}
