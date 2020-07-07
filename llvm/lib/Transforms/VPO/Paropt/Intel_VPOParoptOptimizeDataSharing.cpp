//===----------------- Intel_VPOParoptOptimizeDataSharing -----------------===//
//
//   Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
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
/// to minimize shared data accesses across threads.
///
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/VPO/WRegionInfo/WRegionInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/VPO/Paropt/Intel_VPOParoptOptimizeDataSharing.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/VPO/VPOPasses.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#endif  // INTEL_CUSTOMIZATION

using namespace llvm;
using namespace llvm::vpo;

// Boolean control to enable/disable the optimization.
static cl::opt<bool> EnableDataSharingOpt(
    "vpo-paropt-opt-data-sharing", cl::Hidden, cl::init(true),
    cl::desc("Optimize OpenMP clauses to reduce shared data access"));

// Integer control to limit the number of optimized items
// for each function.
static cl::opt<int> DataSharingOptNumCase(
    "vpo-paropt-opt-data-sharing-num-case", cl::Hidden, cl::init(-1),
    cl::desc("Maximum number of optimized clause items"));

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
                        ORE, 2, false, false);

  Changed |= VP.optimizeDataSharing();

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

// The data sharing optimization is supposed to modify OpenMP clauses
// for some items to reduce the amount of accesses to shared memory.
// For example,
//   #pragma omp parallel for shared(c)
//   for (int i = 0; i < 100; ++i)
//     a[i] = i * c;
//
// The scalar 'c' will be shared by default, so multiple threads will access
// the same shared memory. It may also be hard for the optimizer to prove
// that it is legal to hoist 'c' load from the loop. We can make 'c'
// firstprivate, which will make the code easier to optimize and reduce
// number of times 'c' is read within the loop. Such an optimization
// is safe, if we can prove that a pointer to 'c' cannot be passed
// into the region in any other form (e.g. via capturing it in another
// variable).
//
// This is just one example of things we can do in this optimization.
// Currently, the implementation is only able to optimize address space
// of variables for SPIR-V targets.
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
bool VPOParoptTransform::optimizeDataSharing() {
  bool Changed = false;

  if (!EnableDataSharingOpt)
    return Changed;

  bool NeedTID, NeedBID;
  gatherWRegionNodeList(NeedTID, NeedBID);

  // Create a map between the BasicBlocks and the corresponding
  // innermost WRegionNodes owning the blocks.
  std::unordered_map<const BasicBlock *, WRegionNode *> BBToWRNMap;

  for (auto I = WRegionList.begin(), E = WRegionList.end(); I != E; ++I) {
    WRegionNode *W = *I;
    assert(W->isBBSetEmpty() &&
           "WRNs should not have BBSet populated initially.");

    SmallVector<BasicBlock *, 16> BBSet;
    GeneralUtils::collectBBSet(W->getEntryBBlock(), W->getExitBBlock(), BBSet);
    for (auto *BB : BBSet)
      BBToWRNMap.emplace(BB, W);
  }

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
        if (auto *GEP = dyn_cast<GEPOperator>(U))
          Derivatives.push(GEP);
        else if (auto *BCO = dyn_cast<BitCastOperator>(U))
          Derivatives.push(BCO);
        else if (Operator::getOpcode(U) == Instruction::AddrSpaceCast)
          Derivatives.push(U);
        else if (auto *LI = dyn_cast<LoadInst>(U)) {
          if (LI->isVolatile()) {
            // In case of volatile load, we probably cannot change
            // the variable address space. We only allow the optimization
            // if the volatile load is inside an enclosed region,
            // where the variable is private.
            auto *BB = LI->getParent();
            auto I = BBToWRNMap.find(BB);
            if (I == BBToWRNMap.end())
              // This should not happen, but we just return true conservatively.
              return true;
            // Get the owning WRegionNode for the load instruction.
            WRegionNode *LoadWRN = I->second;
            if (LoadWRN == W)
              // The volatile load is inside the region itself.
              // Returning true here is not actually the right answer
              // to the query made by this lambda, but we want to return
              // true to avoid optimizing for the given pointer V.
              return true;

            if (!ValueIsPrivatizedByInnerRegion(W, LoadWRN, DV))
              // The value is not privatized by any region enclosed
              // into W, so we cannot optimize it.
              return true;
          }
        } else if (auto *SI = dyn_cast<StoreInst>(U)) {
          if (DV != SI->getPointerOperand())
            // This may be a capture.
            return true;

          auto *BB = SI->getParent();
          auto I = BBToWRNMap.find(BB);
          if (I == BBToWRNMap.end())
              // This should not happen, but we just return true conservatively.
            return true;
          // Get the owning WRegionNode for the store instruction.
          WRegionNode *StoreWRN = I->second;
          // Stores inside the given region W are OK.
          // If the store is inside the enclosed region, we have to check
          // whether the store is actually made into the private copy
          // of the variable.
          if (StoreWRN != W &&
              !ValueIsPrivatizedByInnerRegion(W, StoreWRN, DV))
            return true;
        } else if (auto *II = dyn_cast<IntrinsicInst>(U)) {
          if (II->getIntrinsicID() != Intrinsic::directive_region_entry)
            return true;
        } else
          // Unknown user, so assume the worst.
          return true;
      }
    }

    return false;
  };

  // Track the number of optimized items for DataSharingOptNumCase control.
  int NumOptimizedItems = 0;

  for (auto I = WRegionList.begin(), E = WRegionList.end(); I != E; ++I) {
    WRegionNode *W = *I;

    if (!isa<WRNTargetNode>(W) && !isa<WRNTeamsNode>(W))
      // Optimize PRIVATE/FIRSTPRIVATE clauses of "target" regions.
      continue;

    // No need to reset the BBSets afterwards.
    W->populateBBSet();

    SmallPtrSet<Value *, 8> PrivOptimizableItems;
    SmallPtrSet<Value *, 8> FprivOptimizableItems;

    if (W->canHavePrivate()) {
      for (auto *Item : W->getPriv().items())
        if (!MaybeModifiedByEnclosedRegion(W, Item->getOrig()))
          PrivOptimizableItems.insert(Item->getOrig());
    }
    if (W->canHaveFirstprivate()) {
      for (auto *Item : W->getFpriv().items())
        if (!MaybeModifiedByEnclosedRegion(W, Item->getOrig()))
          FprivOptimizableItems.insert(Item->getOrig());
    }

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

    for (unsigned OI = 0; OI < OpBundles.size(); ++OI) {
      StringRef ClauseString = OpBundles[OI].getTag();
      const char *Modifier = "";
      // Some clauses, e.g. PRIVATE, may contain more than one item.
      // In order to keep IR untouched as much as possible, we want
      // to keep the original order of items. To do this for
      // clauses with multiple items we need to break them
      // into multiple clauses with single items. We do not support
      // this by the initial implementation.
      if (OpBundles[OI].input_size() == 1) {
        Value *ClauseItem = OpBundles[OI].inputs()[0];

        if (VPOAnalysisUtils::getClauseID(ClauseString) == QUAL_OMP_PRIVATE &&
            PrivOptimizableItems.count(ClauseItem) != 0) {
          // This is a PRIVATE clause with an optimizable item.
          LLVM_DEBUG(dbgs() << "Will transform: " << *ClauseItem << "\n");
          // If the clause already has some modifiers, then we need
          // to use '.' separator, otherwise, add a new modifier
          // using ':' separator.
          if (ClauseString.find(':') != StringRef::npos)
            Modifier = ".WILOCAL";
          else
            Modifier = ":WILOCAL";
        } else if (VPOAnalysisUtils::getClauseID(ClauseString) ==
                       QUAL_OMP_FIRSTPRIVATE &&
                   FprivOptimizableItems.count(ClauseItem) != 0) {
          // This is a FIRSTPRIVATE clause with an optimizable item.
          LLVM_DEBUG(dbgs() << "Will transform: " << *ClauseItem << "\n");
          if (ClauseString.find(':') != StringRef::npos)
            Modifier = ".WILOCAL";
          else
            Modifier = ":WILOCAL";
        }

        if (Modifier[0] != '\0')
          if (DataSharingOptNumCase >= 0 &&
              NumOptimizedItems >= DataSharingOptNumCase)
            Modifier = "";
          else {
            ++NumOptimizedItems;
            ModifierAdded = true;
          }
      }

      OperandBundleDef B(ClauseString.str() + Modifier,
                         OpBundles[OI].inputs());
      NewOpBundles.push_back(B);
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
    EntryCI->replaceAllUsesWith(NewEntryCI);
    EntryCI->eraseFromParent();
    W->setEntryDirective(NewEntryCI);
  }

  Changed = (NumOptimizedItems > 0);
  return Changed;
}
