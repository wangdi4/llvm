//===----------------- Intel_VPOParoptOptimizeDataSharing -----------------===//
//
//   Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements optimization pass that transforms OpenMP clauses
/// to optimize shared data accesses across threads.
///
/// 1. Optimize address space for PRIVATE/FIRSTPRIVATE variables for SPIR-V
/// targets.
///
/// If the optimization proves that PRIVATE/FIRSTPRIVATE item can be made WI
/// private, it adds "WILOCAL" modifier for the PRIVATE/FIRSTPRIVATE clause
/// specifying this item. VPOParoptTransform will use the modifier
/// to choose the optimal address space for the item.
///
/// 2. Optimize REDUCTION clauses on TEAMS regions for SPIR-V targets.
///
/// If all reads/writes from/to the reduction variable on the TEAMS region
/// are done within regions enclosed into the TEAMS region, and all such
/// regions (or their ancestors also enclosed into the TEAMS region)
/// have the same variable in their reduction clauses, then we can make
/// the variables SHARED for the TEAMS region. This means that the descendant
/// regions (if any) will access the original reduction item during
/// the reduction updates.
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Paropt/Intel_VPOParoptOptimizeDataSharing.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/VPO/VPOParoptConstants.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptUtils.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#endif  // INTEL_CUSTOMIZATION

using namespace llvm;
using namespace llvm::vpo;

// Boolean control to enable/disable the optimization.
static cl::opt<bool> EnableDataSharingOpt(
    "vpo-paropt-opt-data-sharing", cl::Hidden, cl::init(true),
    cl::desc("Optimize OpenMP clauses to optimize shared data access"));

// Boolean control for optimizing PRIVATE/FIRSTPRIVATE clauses.
static cl::opt<bool> EnableDataSharingOptForPrivate(
    "vpo-paropt-opt-data-sharing-for-private", cl::Hidden, cl::init(true),
    cl::desc("Optimize PRIVATE/FIRSTPRIVATE OpenMP clauses."));

// Boolean control for optimizing REDUCTION clauses.
// Ignored for non-SPIR-V targets.
static cl::opt<bool> EnableDataSharingOptForReduction(
    "vpo-paropt-opt-data-sharing-for-reduction", cl::Hidden, cl::init(false),
    cl::desc("Optimize REDUCTION OpenMP clauses."));

// FIXME: remove
// Temporary switch for IGC to reproduce the problem with
// parallel regions.
static cl::opt<bool> ForceDataSharingOptForReduction(
    "vpo-paropt-force-opt-data-sharing-for-reduction", cl::Hidden,
    cl::init(false), cl::desc("Engineering switch."));

// Integer control to limit the number of optimized items
// for each function.
static cl::opt<int> DataSharingOptNumCase(
    "vpo-paropt-opt-data-sharing-num-case", cl::Hidden, cl::init(-1),
    cl::desc("Maximum number of optimized clause items"));

// Defined in VPOParoptTransform.cpp
extern cl::opt<bool> AtomicFreeReduction;

#define DEBUG_TYPE "vpo-paropt-optimize-data-sharing"
#define PASS_NAME "VPO Paropt Optimize Data Sharing"

namespace {
class VPOParoptOptimizeDataSharing : public FunctionPass {
public:
  static char ID;

  VPOParoptOptimizeDataSharing() : FunctionPass(ID) {
    initializeVPOParoptOptimizeDataSharingPass(
        *PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return PASS_NAME; }

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<WRegionInfoWrapperPass>();
    AU.addRequired<OptimizationRemarkEmitterWrapperPass>();
  }
};

static bool optimizeDataSharing(
    Function &F, WRegionInfo &WI, OptimizationRemarkEmitter &ORE) {
  bool Changed = false;

  if (!EnableDataSharingOpt)
    return Changed;

  // Walk the W-Region Graph top-down, and create W-Region List
  WI.buildWRGraph();

  if (WI.WRGraphIsEmpty()) {
    LLVM_DEBUG(dbgs() << "\nNo WRegion Candidates for Parallelization \n");
    return Changed;
  }

  LLVM_DEBUG(WI.print(dbgs()));
  LLVM_DEBUG(dbgs() << PASS_NAME << " for Function: ");
  LLVM_DEBUG(dbgs().write_escaped(F.getName()) << '\n');

  VPOParoptTransform VP(nullptr, &F, &WI, WI.getDomTree(), WI.getLoopInfo(),
                        WI.getSE(), WI.getTargetTransformInfo(),
                        WI.getAssumptionCache(), WI.getTargetLibraryInfo(),
                        WI.getAliasAnalysis(), OmpNoFECollapse,
#if INTEL_CUSTOMIZATION
                        OptReportVerbosity::None,
#endif  // INTEL_CUSTOMIZATION
                        ORE, 2, false);

  BBToWRNMapTy BBToWRNMap;
  // Track the number of optimized items for DataSharingOptNumCase control.
  int NumOptimizedItems = 0;

  // TODO: expose just one entry point in VPOParoptTransform,
  //       and make these functions private to this file.
  //       We need to get rid of WRegionList dependency inside
  //       these functions.
  Changed |= VP.optimizeDataSharingForPrivateItems(BBToWRNMap,
                                                   NumOptimizedItems);
  Changed |= VP.optimizeDataSharingForReductionItems(BBToWRNMap,
                                                     NumOptimizedItems);

  return Changed;
}
} // end anonymous namespace

char VPOParoptOptimizeDataSharing::ID = 0;
INITIALIZE_PASS_BEGIN(VPOParoptOptimizeDataSharing, DEBUG_TYPE, PASS_NAME,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(OptimizationRemarkEmitterWrapperPass)
INITIALIZE_PASS_END(VPOParoptOptimizeDataSharing, DEBUG_TYPE, PASS_NAME,
                    false, false)

FunctionPass *llvm::createVPOParoptOptimizeDataSharingPass() {
  return new VPOParoptOptimizeDataSharing();
}

bool VPOParoptOptimizeDataSharing::runOnFunction(Function &F) {
  WRegionInfo &WI = getAnalysis<WRegionInfoWrapperPass>().getWRegionInfo();
  auto &ORE = getAnalysis<OptimizationRemarkEmitterWrapperPass>().getORE();

  LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");
  bool Changed = optimizeDataSharing(F, WI, ORE);
  LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");
  return Changed;
}

PreservedAnalyses VPOParoptOptimizeDataSharingPass::run(
    Function &F, FunctionAnalysisManager &AM) {
  WRegionInfo &WI = AM.getResult<WRegionInfoAnalysis>(F);
  auto &ORE = AM.getResult<OptimizationRemarkEmitterAnalysis>(F);

  PreservedAnalyses PA;

  LLVM_DEBUG(dbgs() << "\n\n====== Enter " << PASS_NAME << " ======\n\n");
  if (optimizeDataSharing(F, WI, ORE))
    PA = PreservedAnalyses::all();
  else
    PA = PreservedAnalyses::none();
  LLVM_DEBUG(dbgs() << "\n\n====== Exit  " << PASS_NAME << " ======\n\n");

  return PA;
}

// FEs sometimes put variables both into MAP and [FIRST]PRIVATE
// clauses. Sometimes the map type is not compatible with
// [FIRST]PRIVATE, so trying to rely on [FIRST]PRIVATE:WILOCAL
// will be completely incorrect.
//
// For example if foo() below is a member function of
// some class, and _p is a member of this class, CFE will
// put 'this' pointer into a FIRSTPRIVATE clause and
// a MAP clause with map type (OMP_TGT_MAPTYPE_TO |
// OMP_TGT_MAPTYPE_FROM | OMP_TGT_MAPTYPE_TARGET_PARAM |
// OMP_TGT_MAPTYPE_IMPLICIT).
//
// void foo(int *p) {
// #pragma omp target
//   _p = p;
// }
//
// In general, Paropt does not do privatization for
// [FIRST]PRIVATE items that are also referenced in MAP clauses,
// but in some cases we do want to apply privatization, e.g.
// when it is a WILOCAL [FIRST]PRIVATE item. By replacing
// all references inside the target region to the new private
// copy we make all the accesses to the object explicit,
// providing better disambiguation with accesses to other
// objects. The private copy will be registerized, in general,
// so it has relatively low cost.
//
// But we cannot apply privatization for the case above
// just based on the fact that the item is WILOCAL [FIRST]PRIVATE.
// In order to avoid checking the MAP clauses at the point
// of privatization we just avoid setting WILOCAL for items
// that are referenced in MAP clauses with conflicting map types.
// I believe [FIRST]PRIVATE pointers may only be
// referenced by chain heads, so we need to check
// the map type of the chain head.
template <typename ItemTy> bool isInMapWithoutPrivateBit(ItemTy *I) {
  MapItem *MapI = I->getInMap();
  if (!MapI)
    return false;

  assert(MapI->getIsMapChain() && "Paropt only supports map chains now.");

  MapAggrTy *AggrHead = MapI->getMapChain()[0];
  // Only handle MAPs with explicit map types,
  // otherwise, be conservative.
  if (!AggrHead->hasExplicitMapType())
    return true;

  uint64_t MapType = AggrHead->getMapType();
  if (MapType & TGT_MAP_PRIVATE)
    return false;

  // If map type does not have TGT_MAP_PRIVATE bit, then
  // do not optimize such items.
  LLVM_DEBUG(dbgs() << "Item '" << *I->getOrig()
                    << "' is in a map clause without map-type PRIVATE.\n");
  return true;
}

// Optimize address space for PRIVATE/FIRSTPRIVATE variables for SPIR-V targets.
//
// Example:
//   int *a;
//   #pragma omp target teams map(a[:N])
//   #pragma distribute parallel for
//   for (int i = 0; i < 100; ++i)
//     a[i] = ...;
//
// FEs will create a "target" private variable 'a.addr', which will hold
// the 'a' address passed to the outlined offload routine. The store of
// 'a' into 'a.addr' will happen inside the "target" region. In general,
// this requires globalizing 'a.addr' and guarding the store to it
// with the master thread check. On the other hand, if there are no other
// stores to 'a.addr' inside the regions enclosed into 'target',
// we can make it WI private (i.e. put it into addrspace(0)), and execute
// the store in each WI. The WI private variable will have the same value
// in all WIs, and this value will always be available to all WIs.
//
// If the optimization proves that an item can be made WI private,
// it adds "WILOCAL" modifier for the PRIVATE/FIRSTPRIVATE clause
// specifying this item. VPOParoptTransform will use the modifier
// to choose the optimal address space for the item.
bool VPOParoptTransform::optimizeDataSharingForPrivateItems(
    BBToWRNMapTy &BBToWRNMap, int &NumOptimizedItems) {
  bool Changed = false;

  if (!EnableDataSharingOptForPrivate)
    return Changed;

  initializeBlocksToRegionsMap(BBToWRNMap);

  // InnerW is one of descendants of OuterW. The lambda returns true,
  // if value V is privatized by any descendant of OuterW, which is not
  // a descendant of InnerW. In other words, if value V is privatized
  // by InnerW or any of its ancestors (not including OuterW and
  // the ancestors of OuterW), then the lambda will return true.
  auto ValueIsPrivatizedByInnerRegion = [&](WRegionNode *OuterW,
                                            WRegionNode *InnerW,
                                            Value *V) {
    if (OuterW == InnerW)
      return false;

    do {
      assert(InnerW && "Expected region not found in ancestors.");

      if (InnerW->canHavePrivate() &&
          std::any_of(InnerW->getPriv().items().begin(),
                      InnerW->getPriv().items().end(),
                      [V](PrivateItem *PrivI) {
                        return V == PrivI->getOrig();
                      }))
        return true;

      if (InnerW->canHaveFirstprivate() &&
          std::any_of(InnerW->getFpriv().items().begin(),
                      InnerW->getFpriv().items().end(),
                      [V](FirstprivateItem *FprivI) {
                        return V == FprivI->getOrig();
                      }))
        return true;

      if (InnerW->getIsOmpLoop()) {
        auto &LI = InnerW->getWRNLoopInfo();
        for (unsigned Idx = 0; Idx < LI.getNormUBSize(); ++Idx)
          if (V == LI.getNormUB(Idx) || V == LI.getNormIV(Idx))
            return true;
      }

      InnerW = InnerW->getParent();
    } while (InnerW != OuterW);

    return false;
  };

  // Return true, if the memory pointed by the given point V
  // may be modified by any region enclosed into the given region W.
  // Note that modifications of the memory inside region W itself
  // are ignored, and they do not cause 'true' result.
  auto MaybeModifiedByEnclosedRegion = [&](WRegionNode *W, Value *V) {

    if (isa<Constant>(V))
      // TODO: we do not handle constants yet.
      return true;

    std::queue<Value *> Derivatives;
    Derivatives.push(V);

    while (!Derivatives.empty()) {
      Value *DV = Derivatives.front();
      Derivatives.pop();

      SmallVector<Instruction *, 8> Users;
      WRegionUtils::findUsersInRegion(W, DV, &Users, false);
      for (auto *U : Users) {
        if (auto *II = dyn_cast<IntrinsicInst>(U)) {
        // Ignore uses by directive entry calls.
          if (II->getIntrinsicID() == Intrinsic::directive_region_entry)
            continue;

          // Not captured if the callee is readonly, doesn't return
          // a copy through its return value and doesn't unwind
          // (a readonly function can leak bits by throwing an exception
          // or not depending on the input value).
          //
          // FIXME: this is copied from llvm::PointerMayBeCaptured().
          //        We should try to reuse it. There are cases that
          //        we currently handle incorrectly, e.g. pointer
          //        comparison.
          if (II->onlyReadsMemory() && II->doesNotThrow() &&
              II->getType()->isVoidTy())
            continue;

          // Recognize the lifetime markers explicitly,
          // since they do not classify as onlyReadsMemory().
          if (II->getIntrinsicID() == Intrinsic::lifetime_end ||
              II->getIntrinsicID() == Intrinsic::lifetime_start)
            continue;
        }

        auto *BB = U->getParent();
        auto I = BBToWRNMap.find(BB);
        if (I == BBToWRNMap.end()) {
          // This should not happen, but we just return true conservatively.
          return true;
        }
        // Get the owning WRegionNode for the using instruction.
        WRegionNode *UseWRN = I->second;

        // Any use of DV inside a descendant region is "safe", if the value
        // is privatized by any region that is a descendant of W and is
        // an ancestor of UseWRN, or if the value is privated by UseWRN itself.
        //
        // We do not need to consider such uses as derivatives or analyze
        // them at all.
        if (UseWRN != W && ValueIsPrivatizedByInnerRegion(W, UseWRN, DV))
          continue;

        if (auto *GEP = dyn_cast<GEPOperator>(U))
          Derivatives.push(GEP);
        else if (auto *BCO = dyn_cast<BitCastOperator>(U))
          Derivatives.push(BCO);
        else if (Operator::getOpcode(U) == Instruction::AddrSpaceCast)
          Derivatives.push(U);
        else if (auto *SI = dyn_cast<SelectInst>(U))
          Derivatives.push(SI);
        else if (auto *LI = dyn_cast<LoadInst>(U)) {
          if (LI->isVolatile()) {
            // In case of volatile load, we probably cannot change
            // the variable address space. We only allow the optimization
            // if the volatile load is inside an enclosed region,
            // where the variable is private.

            // If the volatile load is inside the region itself:
            // Returning true here is not actually the right answer
            // to the query made by this lambda, but we want to return
            // true to avoid optimizing for the given pointer V.
            //
            // If the volatile load is inside the enclosed region.
            // We have already checked that the location is not privatized
            // by an enclosed region. So this use is not safe for optimization.
            return true;
          }
        } else if (auto *SI = dyn_cast<StoreInst>(U)) {
          if (DV != SI->getPointerOperand())
            // This may be a capture.
            return true;
          // Stores inside the given region W are OK.
          if (UseWRN == W)
            continue;
          // The store is inside the enclosed region.
          // We have already checked that the location is not privatized
          // by an enclosed region. So this use is a modification.
          return true;
        } else {
          // Unknown user, so assume the worst.
          return true;
        }
      }
    }

    return false;
  };

  for (auto I = WRegionList.begin(), E = WRegionList.end(); I != E; ++I) {
    WRegionNode *W = *I;

    if (!isa<WRNTargetNode>(W) && !isa<WRNTeamsNode>(W) &&
        !WRegionUtils::isDistributeNode(W))
      // Optimize PRIVATE/FIRSTPRIVATE clauses of "target", "teams" and
      // "distribute" regions.
      continue;

    // No need to reset the BBSets afterwards.
    W->populateBBSet();

    SmallPtrSet<Value *, 8> PrivOptimizableItems;
    SmallPtrSet<Value *, 8> FprivOptimizableItems;

    if (W->canHavePrivate())
      for (auto *Item : W->getPriv().items())
        if (!isInMapWithoutPrivateBit(Item) &&
            !MaybeModifiedByEnclosedRegion(W, Item->getOrig()))
          PrivOptimizableItems.insert(Item->getOrig());

    if (W->canHaveFirstprivate())
      for (auto *Item : W->getFpriv().items())
        if (!isInMapWithoutPrivateBit(Item) &&
            !MaybeModifiedByEnclosedRegion(W, Item->getOrig()))
          FprivOptimizableItems.insert(Item->getOrig());

    if (PrivOptimizableItems.empty() && FprivOptimizableItems.empty())
      // Nothing to do.
      continue;

    if (!PrivOptimizableItems.empty())
      LLVM_DEBUG(
          dbgs() << "PRIVATE optimizable items:\n";
          for (auto *V : PrivOptimizableItems)
            dbgs() << *V << "\n");
    if (!FprivOptimizableItems.empty())
      LLVM_DEBUG(
          dbgs() << "FIRSTPRIVATE optimizable items:\n";
          for (auto *V : FprivOptimizableItems)
            dbgs() << *V << "\n");

    // Add LOCAL modifiers to PRIVATE/FIRSTPRIVATE clauses.
    SmallVector<OperandBundleDef, 16> OpBundles;
    auto *EntryCI = cast<CallInst>(W->getEntryDirective());
    EntryCI->getOperandBundlesAsDefs(OpBundles);
    SmallVector<OperandBundleDef, 16> NewOpBundles;
    bool ModifierAdded = false;

    for (const auto &Bundle : OpBundles) {
      StringRef ClauseString = Bundle.getTag();
      SmallPtrSet<Value *, 8> *OptimizableItems = nullptr;
      bool CheckOnlyFirstBundleOperand = false;

      if (VPOAnalysisUtils::isOpenMPClause(ClauseString)) {
        ClauseSpecifier ClauseInfo(ClauseString);
        int ClauseId = ClauseInfo.getId();

        switch (ClauseId) {
        case QUAL_OMP_PRIVATE:
          OptimizableItems = &PrivOptimizableItems;
          break;
        case QUAL_OMP_FIRSTPRIVATE:
          OptimizableItems = &FprivOptimizableItems;
          break;
        }

        // TODO: figure out how to optimize these cases.
        if (ClauseInfo.getIsByRef() || ClauseInfo.getIsF90DopeVector() ||
            ClauseInfo.getIsF90NonPod())
          OptimizableItems = nullptr;

        // NONPOD and TYPED [FIRST]PRIVATE clauses have multiple
        // inputs in their operand bundles, but only the first
        // input is the pointer value that we may treat as WILOCAL.
        // The rest of the inputs are auxiliary values that do not affect
        // WILOCAL property, so we do not need to check them agains
        // OptimizableItems set. We just need to preserve them as-is.
        if (ClauseInfo.getIsNonPod() || ClauseInfo.getIsTyped())
          CheckOnlyFirstBundleOperand = true;
      }

      // If this clause is not optimizable or if this bundle is not a clause,
      // then just add it as is.
      if (!OptimizableItems) {
        NewOpBundles.emplace_back(Bundle);
        continue;
      }

      // If the clause already has some modifiers, then we need to use '.'
      // separator, otherwise, add a new modifier using ':' separator.
      const char *WILocal =
          ClauseString.find(':') != StringRef::npos ? ".WILOCAL" : ":WILOCAL";

      // Create new bundles for this clause preserving the original items order.
      SmallVector<Value *, 8> NewItems;
      const char *NewModifier = nullptr;
      auto AddBundle = [&]() {
        std::string NewTag = ClauseString.str();
        if (NewModifier) {
          NewTag += NewModifier;
          ModifierAdded = true;
        }
        NewOpBundles.emplace_back(NewTag, NewItems);
        NewItems.clear();
        NewModifier = nullptr;
      };
      for (Value *ClauseItem : Bundle.inputs()) {
        if (CheckOnlyFirstBundleOperand && !NewItems.empty()) {
          // Preserve the auxiliary inputs as-is (e.g. for NONPOD/TYPED
          // clauses).
          NewItems.push_back(ClauseItem);
          continue;
        }

        const char *ItemModifier = nullptr;
        if (OptimizableItems->contains(ClauseItem)) {
          LLVM_DEBUG(dbgs() << "Will transform: " << *ClauseItem << "\n");
          ItemModifier = WILocal;
        }
        if (!NewItems.empty() && NewModifier != ItemModifier)
          AddBundle();
        NewItems.push_back(ClauseItem);
        NewModifier = ItemModifier;
      }
      if (!NewItems.empty())
        AddBundle();
    }

    if (!ModifierAdded)
      // We have not added a modifier to any of the clauses,
      // so we may keep the original directive call.
      continue;

    // Recreate the directive call.
    SmallVector<Value *, 8> Args(EntryCI->arg_begin(), EntryCI->arg_end());
    auto *NewEntryCI = CallInst::Create(EntryCI->getFunctionType(),
                                        EntryCI->getCalledOperand(), Args,
                                        NewOpBundles, "", EntryCI);
    NewEntryCI->takeName(EntryCI);
    NewEntryCI->setCallingConv(EntryCI->getCallingConv());
    NewEntryCI->setAttributes(EntryCI->getAttributes());
    NewEntryCI->setDebugLoc(EntryCI->getDebugLoc());
    NewEntryCI->copyMetadata(*EntryCI);
    EntryCI->replaceAllUsesWith(NewEntryCI);
    EntryCI->eraseFromParent();
    W->setEntryDirective(NewEntryCI);
  }

  return Changed;
}

// Optimize REDUCTION clauses on TEAMS regions for SPIR-V targets.
//
// Example:
//   float sum = 0.0f;
//   #pragma omp target teams map(to: data) reduction(+: sum)
//   #pragma omp distribute parallel for reduction(+: sum)
//   for (int i = 0; i < 100; ++i) {
//     sum += data[i];
//   }
//
// If all reads/writes from/to the reduction variable on the TEAMS region
// are done within regions enclosed into the TEAMS region, and all such
// regions (or their ancestors also enclosed into the TEAMS region)
// have the same variable in their reduction clauses, then we can make
// the variables SHARED for the TEAMS region. This means that the descendant
// regions (if any) will access the original reduction item during
// the reduction updates.
//
// In general, reducing into a team-local storage, and then reducing
// to the global storage should be better, since each team accesses
// the global storage only once. But implementing this for SPIR-V
// target requires a barrier after the enclosed region, and this
// results in up to 3x slowdowns (CMPLRLLVM-20537).
bool VPOParoptTransform::optimizeDataSharingForReductionItems(
    BBToWRNMapTy &BBToWRNMap, int &NumOptimizedItems) {
  bool Changed = false;

  if (!EnableDataSharingOptForReduction || !isTargetSPIRV())
    return Changed;

  // We cannot perform data sharing opt when GPU fast reduction local update
  // loop is generated because it needs TEAMS local reductions values to stay
  // and their respective atomic updates to be emiited. Otherwise ParLoop's
  // local updates will update cross-WG accumulators in a non-thread-safe
  // manner causing dataraces.
  if (AtomicFreeReduction) {
    OptimizationRemark R(
        "openmp", "Ignored OptDataSharing as fast GPU reduction is enabled", F);
    R << " OptDataSharing as fast GPU reduction is enabled";
    F->getContext().diagnose(R);
    return Changed;
  }

  initializeBlocksToRegionsMap(BBToWRNMap);

  // InnerW is one of descendants of OuterW. The lambda returns true,
  // if value V is privatized or reduced by any descendant of OuterW,
  // which is not a descendant of InnerW. In other words, if value V
  // is privatized or reduced by InnerW or any of its ancestors
  // (not including OuterW and the ancestors of OuterW), then the lambda
  // will return true.
  auto ValueIsPrivatizedOrReducedByInnerRegion = [&](WRegionNode *OuterW,
                                                     WRegionNode *InnerW,
                                                     Value *V) {
    if (OuterW == InnerW)
      return false;

    do {
      assert(InnerW && "Expected region not found in ancestors.");

      if (InnerW->canHavePrivate() &&
          WRegionUtils::wrnSeenAsPrivate(InnerW, V))
        return true;

      if (InnerW->canHaveFirstprivate() &&
          WRegionUtils::wrnSeenAsFirstprivate(InnerW, V))
        return true;

      if (!isa<WRNVecLoopNode>(InnerW) &&
          // Temporary disable this for inner parallel regions.
          // There seems to be a bug in IGC (JIRA TBD).
          (!isa<WRNParallelNode>(InnerW) ||
           !ForceDataSharingOptForReduction)) {
        // Reduction update for SIMD loops is not atomic,
        // so we cannot assume that a SIMD loop may reduce
        // correct value to the global storage.
        if (InnerW->canHaveReduction() &&
            WRegionUtils::wrnSeenAsReduction(InnerW, V))
          return true;
      }

      if (InnerW->getIsOmpLoop()) {
        auto &LI = InnerW->getWRNLoopInfo();
        for (unsigned Idx = 0; Idx < LI.getNormUBSize(); ++Idx)
          if (V == LI.getNormUB(Idx) || V == LI.getNormIV(Idx))
            return true;
      }

      InnerW = InnerW->getParent();
    } while (InnerW != OuterW);

    return false;
  };

  // Return true, if the reduction value cannot be computed solely by
  // the inner regions. The lambda traces the value uses within
  // the given region and checks several conditions (see below).
  auto CannotBeReducedByInnerRegions = [&](WRegionNode *W, Value *V) {

    if (isa<Constant>(V))
      // TODO: we do not handle constants yet.
      return true;

    std::queue<Value *> Derivatives;
    Derivatives.push(V);

    while (!Derivatives.empty()) {
      Value *DV = Derivatives.front();
      Derivatives.pop();

      SmallVector<Instruction *, 8> Users;
      WRegionUtils::findUsersInRegion(W, DV, &Users, false);
      for (auto *U : Users) {
        if (auto *GEP = dyn_cast<GEPOperator>(U))
          Derivatives.push(GEP);
        else if (auto *BCO = dyn_cast<BitCastOperator>(U))
          Derivatives.push(BCO);
        else if (Operator::getOpcode(U) == Instruction::AddrSpaceCast)
          Derivatives.push(U);
        else if (auto *LI = dyn_cast<LoadInst>(U)) {
          auto *BB = LI->getParent();
          auto MapIt = BBToWRNMap.find(BB);
          if (MapIt == BBToWRNMap.end()) {
            // This should not happen, but we just return true conservatively.
            LLVM_DEBUG(dbgs() << "WARNING: BasicBlock '";
                       BB->printAsOperand(dbgs());
                       dbgs() << "' does not belong to any region.\n");
            return true;
          }
          // Get the owning WRegionNode for the load instruction.
          WRegionNode *LoadWRN = MapIt->second;
          if (LoadWRN == W)
            // #pragma omp target teams reduction(+: sum)
            // {
            // #pragma omp parallel for reduction(+: sum)
            //   for (int i = 0; i < 100; ++i) { ... }
            //
            //   ... = sum;
            // }
            //
            // The read of sum inside the teams region assumes that the team
            // local value is read, so we have to handle reduction for
            // the teams region. If we make it shared, then the read of sum
            // will return some intermediate reduction value computed across
            // the teams.
            return true;

          if (!ValueIsPrivatizedOrReducedByInnerRegion(W, LoadWRN, DV))
            // The value is not privatized by any region enclosed
            // into W, so we cannot optimize it.
            return true;
        } else if (auto *SI = dyn_cast<StoreInst>(U)) {
          if (DV != SI->getPointerOperand())
            // This may be a capture.
            return true;

          auto *BB = SI->getParent();
          auto MapIt = BBToWRNMap.find(BB);
          if (MapIt == BBToWRNMap.end()) {
            // This should not happen, but we just return true conservatively.
            LLVM_DEBUG(dbgs() << "WARNING: BasicBlock '";
                       BB->printAsOperand(dbgs());
                       dbgs() << "' does not belong to any region.\n");
            return true;
          }
          // Get the owning WRegionNode for the store instruction.
          WRegionNode *StoreWRN = MapIt->second;
          if (StoreWRN == W)
            return true;
          if (!ValueIsPrivatizedOrReducedByInnerRegion(W, StoreWRN, DV))
            return true;
        } else if (auto *II = dyn_cast<IntrinsicInst>(U)) {
          if (II->getIntrinsicID() != Intrinsic::directive_region_entry)
            return true;
        } else {
          // Unknown user, so assume the worst.
          return true;
        }
      }
    }

    return false;
  };

  for (auto *W : WRegionList) {

    if (!isa<WRNTeamsNode>(W))
      // Optimize REDUCTION clauses of "teams" regions.
      continue;

    // No need to reset the BBSets afterwards.
    W->populateBBSet();

    SmallPtrSet<Value *, 8> OptimizableItems;

    if (W->canHaveReduction()) {
      for (auto *Item : W->getRed().items()) {
        if (Item->getIsArraySection())
          // TODO: figure out handling for array reductions.
          continue;

        if (Item->getType() == ReductionItem::WRNReductionUdr)
          // Do not optimize UDR.
          continue;

        Type *AllocaTy;
        Value *NumElements;
        std::tie(AllocaTy, NumElements, std::ignore) =
            VPOParoptUtils::getItemInfo(Item);

        Type *ScalarTy = AllocaTy->getScalarType();

        // FIXME: remove ForceDataSharingOptForReduction check,
        //        when IGC is fixed.
        if (!ForceDataSharingOptForReduction &&
            !ScalarTy->isDoubleTy())
          // FIXME: optimize only 'double' reductions now, which
          //        require critical section.
          //        Other types may use atomic updates,
          //        and hierarchical reductions show better results
          //        for them (on micro kernels).
          //        Some non-double reductions may still require critical
          //        section, so we need to figure out how to distinguish
          //        them here.
          continue;

        if (!CannotBeReducedByInnerRegions(W, Item->getOrig()))
          OptimizableItems.insert(Item->getOrig());
      }
    }

    if (OptimizableItems.empty())
      // Nothing to do.
      continue;

    LLVM_DEBUG(dbgs() << "REDUCTION optimizable items:\n";
               for (auto *V : OptimizableItems)
                 dbgs() << *V << "\n");

    // Add LOCAL modifiers to PRIVATE/FIRSTPRIVATE clauses.
    SmallVector<OperandBundleDef, 16> OpBundles;
    auto *EntryCI = cast<CallInst>(W->getEntryDirective());
    EntryCI->getOperandBundlesAsDefs(OpBundles);
    SmallVector<OperandBundleDef, 16> NewOpBundles;
    bool ClauseModified = false;
    StringRef SharedClauseString =
        VPOAnalysisUtils::getClauseString(QUAL_OMP_SHARED);

    for (unsigned OI = 0; OI < OpBundles.size(); ++OI) {
      StringRef ClauseString = OpBundles[OI].getTag();

      // FIXME: we currently handle clauses with just one item.
      //        This also means that array reductions are not supported.
      if (OpBundles[OI].input_size() == 1) {
        Value *ClauseItem = OpBundles[OI].inputs()[0];

        ClauseSpecifier ClauseInfo(ClauseString);
        int ClauseId = ClauseInfo.getId();

        if (VPOAnalysisUtils::isReductionClause(ClauseId) &&
            OptimizableItems.count(ClauseItem) != 0 &&
            (DataSharingOptNumCase < 0 ||
             NumOptimizedItems < DataSharingOptNumCase)) {
          // This is a REDUCTION clause with an optimizable item.
          LLVM_DEBUG(dbgs() << "Will transform to SHARED: " << *ClauseItem <<
                     "\n");
          ClauseString = SharedClauseString;
          ++NumOptimizedItems;
          Changed = true;
          ClauseModified = true;
        }
      }

      OperandBundleDef B(ClauseString.str(), OpBundles[OI].inputs());
      NewOpBundles.push_back(B);
    }

    if (!ClauseModified)
      continue;

    // Recreate the directive call.
    SmallVector<Value *, 8> Args(EntryCI->arg_begin(), EntryCI->arg_end());
    auto *NewEntryCI = CallInst::Create(EntryCI->getFunctionType(),
                                        EntryCI->getCalledOperand(), Args,
                                        NewOpBundles, "", EntryCI);
    NewEntryCI->takeName(EntryCI);
    NewEntryCI->setCallingConv(EntryCI->getCallingConv());
    NewEntryCI->setAttributes(EntryCI->getAttributes());
    NewEntryCI->setDebugLoc(EntryCI->getDebugLoc());
    EntryCI->replaceAllUsesWith(NewEntryCI);
    EntryCI->eraseFromParent();
    W->setEntryDirective(NewEntryCI);
  }

  return Changed;
}

// Create a map between the BasicBlocks and the corresponding
// innermost WRegionNodes owning the blocks.
// The side effect of this function is that it computes WRegionList.
void VPOParoptTransform::initializeBlocksToRegionsMap(
    BBToWRNMapTy &BBToWRNMap) {
  // The map is kept valid across transformations, so we do not recompute
  // it, if it was computed before.
  if (!BBToWRNMap.empty())
    return;

  bool NeedTID, NeedBID;
  gatherWRegionNodeList(NeedTID, NeedBID);

  for (auto I = WRegionList.begin(), E = WRegionList.end(); I != E; ++I) {
    WRegionNode *W = *I;
    assert(W->isBBSetEmpty() &&
           "WRNs should not have BBSet populated initially.");

    SmallVector<BasicBlock *, 16> BBSet;
    GeneralUtils::collectBBSet(W->getEntryBBlock(), W->getExitBBlock(), BBSet);
    for (auto *BB : BBSet)
      BBToWRNMap.emplace(BB, W);
  }
}
