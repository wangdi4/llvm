//===- ArgNoAliasProp.cpp ---- Add noalias to function args -----*- C++ -*-===//
//
// Copyright (C) 2020-2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This pass adds noalias attribute to function arguments which have pointer
/// types where it is safe to do so.
///
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_ArgNoAliasProp.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_XmainOptLevelPass.h"
#include "llvm/Analysis/PhiValues.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/AbstractCallSite.h"
#include "llvm/IR/Dominators.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;

#define DEBUG_TYPE "arg-noalias-prop"

STATISTIC(NumUpdatedArgs, "Number of updated arguments");

namespace {

class NoAliasProp {
  function_ref<AAResults &(Function &)> AAGetter;
  function_ref<DominatorTree &(Function &)> DTGetter;
  const unsigned OptLevel;

public:
  NoAliasProp(function_ref<AAResults &(Function &)> AAGetter,
              function_ref<DominatorTree &(Function &)> DTGetter,
              unsigned OptLevel)
      : AAGetter(AAGetter), DTGetter(DTGetter), OptLevel(OptLevel) {}

  bool run(CallGraph &CG);

private:
  bool propagateNoAliasToArgs(Function &F);
};

} // anonymous namespace

bool NoAliasProp::propagateNoAliasToArgs(Function &F) {
  // We need to know all callers for the function therefore it has to be local.
  if (!F.hasLocalLinkage() || F.use_empty())
    return false;

  // Select function arguments with pointer types.
  DenseMap<Argument *, bool> PtrArgs;
  for (Argument &A : F.args())
    if (isa<PointerType>(A.getType()) && !A.hasNoAliasAttr())
      PtrArgs[&A] = true;
  if (PtrArgs.empty())
    return false;

  // Make a pass of the Function's uses in order to look through ConstantExpr
  // cast uses. The AbstractCallSite constructor can do this for cases where
  // the ConstantExpr has a single use, but doing it for the constructor allows
  // ConstantExpr use which themselves have multiple uses to be handled.
  SmallVector<Use *, 4> FuncUses;
  for (Use &U : F.uses()) {
    ConstantExpr *CE = dyn_cast<ConstantExpr>(U.getUser());
    if (CE && !CE->hasOneUse() && CE->isCast())
      for (Use &CEUse : CE->uses())
        FuncUses.push_back(&CEUse);
    else
      FuncUses.push_back(&U);
  }

  // Check all function's call sites to see what values are passed to the
  // pointer arguments. We check if actual value that is passed to the call
  //  - is derived from a noalias pointer
  //  - is not captured before the call
  //  - does not alias with any other call's arguments
  // If that is true for all call sites we can add noalias attribute to the
  // function argument.
  // TODO: we may consider cloning functions if noalias can be added for some
  // call sites but not all.
  for (Use *U : FuncUses) {
    AbstractCallSite CS(U);

    // Must be a direct or a callback call.
    if (!CS || !(CS.isDirectCall() || CS.isCallbackCall()))
      return false;

    /* If OptLevel > 2, spend time exploring more uses. */
    const unsigned MaxUsesToExplore =
        OptLevel > 2 ? 80 : getDefaultMaxUsesToExploreForCaptureTracking();

    // Obtain alias and dominator analyses for the function containing this
    // call site of F. (We are not concerned with the implementation of F.)
    Function *CallingFunc = CS.getInstruction()->getFunction();
    AAResults &AA = AAGetter(*CallingFunc);
    DominatorTree &DT = DTGetter(*CallingFunc);

    for (auto &P : PtrArgs) {
      if (!P.second)
        continue;

      bool NoAlias = false;
      if (Value *PV = CS.getCallArgOperand(*P.first)) {
        Value *Origin = PV->stripPointerCasts();

        // Returns true if the Value is both derived from a function-local
        // noalias pointer and not captured before the call site.
        auto IsLocalAndUncaptured = [&DT, &CS,
                                     MaxUsesToExplore](const Value *V) -> bool {
          if (!isIdentifiedFunctionLocal(V))
            return false;

          return !PointerMayBeCapturedBefore(
              V, /*ReturnCaptures=*/true,
              /*StoreCaptures=*/true, CS.getInstruction(), &DT,
              /*IncludeI=*/false, MaxUsesToExplore);
        };

        // Returns true if the Value is derived exclusively from local objects
        // which have not escaped. For example, the Value may be a PHI choosing
        // between multiple allocas.
        auto IsDerivedFromUncaptured =
            [&IsLocalAndUncaptured](Value *V) -> bool {
          SmallVector<const Value *, 4> Pointers;
          getUnderlyingObjects(V, Pointers);

          return all_of(Pointers, IsLocalAndUncaptured);
        };

        NoAlias = IsDerivedFromUncaptured(Origin);

        if (NoAlias)
          // Actual argument should not alias with any other pointer values that
          // are passed to the call.
          NoAlias = none_of(F.args(), [&P, &CS, &AA, PV](Argument &A) {
            if (!isa<PointerType>(A.getType()) || &A == P.first)
              return false;
            Value *AV = CS.getCallArgOperand(A);
            if (!AV)
              return false;
            return !AA.isNoAlias(AV, PV);
          });
      }

      P.second &= NoAlias;
    }
  }

  // Update arguments.
  bool Changed = false;
  for (auto &P : PtrArgs)
    if (P.second) {
      P.first->addAttr(Attribute::NoAlias);
      ++NumUpdatedArgs;
      Changed = true;
    }

  if (Changed)
    LLVM_DEBUG(dbgs() << "Added noalias to " << F.getName() << "arguments\n");
  return Changed;
}

bool NoAliasProp::run(CallGraph &CG) {
  // Traverse SCCs in topological order.
  SmallVector<std::vector<CallGraphNode *>, 8u> SCCs;
  for (auto I = scc_begin(&CG), E = scc_end(&CG); I != E; ++I)
    SCCs.emplace_back(*I);

  bool Changed = false;
  for (const auto &SCC : reverse(SCCs)) {
    bool LocalChange;
    do {
      LocalChange = false;
      for (const CallGraphNode *CGN : SCC) {
        Function *F = CGN->getFunction();
        if (!F || F->isDeclaration())
          continue;
        LocalChange |= propagateNoAliasToArgs(*F);
      }
    } while (LocalChange);
    Changed |= LocalChange;
  }
  return Changed;
}

PreservedAnalyses ArgNoAliasPropPass::run(Module &M,
                                          ModuleAnalysisManager &AM) {

  SmallPtrSet<Function *, 32> BasicAARecomputed;
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  auto &CG = AM.getResult<CallGraphAnalysis>(M);

  auto AARGetter = [&](Function &F) -> AAResults & {
    // BasicAA uses phi values analysis only if it is available, so force it to
    // be run on the function.
    // Invalidate cached BasicAA so it gets recomputed with PHiValuesAnalysis
    // BasicAARecomputed set is used to stop recomputing BasicAA multiple times.
    if (FAM.getCachedResult<BasicAA>(F) && !BasicAARecomputed.count(&F)) {
      auto PA = PreservedAnalyses::all();
      // Abandoning BasicAA doesn’t help recomputing BasicAA.
      // DominatorTreeAnalysis is dependent analysis for BasicAA.
      // Abandoning DominatorTreeAnalysis here so that BasicAA is recomputed.
      PA.abandon<DominatorTreeAnalysis>();
      FAM.invalidate(F, PA);
      BasicAARecomputed.insert(&F);
    }
    FAM.getResult<PhiValuesAnalysis>(F);
    return FAM.getResult<AAManager>(F);
  };

  auto DTGetter = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };

  unsigned OptLevel = AM.getResult<XmainOptLevelAnalysis>(M).getOptLevel();

  if (!NoAliasProp(AARGetter, DTGetter, OptLevel).run(CG))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<AndersensAA>();
  PA.preserve<CallGraphAnalysis>();
  PA.preserve<DominatorTreeAnalysis>();
  PA.preserve<GlobalsAA>();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}
<<<<<<< HEAD

namespace {

struct ArgNoAliasProp : public ModulePass {
  static char ID;

  ArgNoAliasProp() : ModulePass(ID) {
    initializeArgNoAliasPropPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<CallGraphWrapperPass>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<XmainOptLevelWrapperPass>();
    AU.addPreserved<AndersensAAWrapperPass>();
    AU.addPreserved<CallGraphWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
    // Commenting this out uintil we remove the legacy pass manager code.
    // getAAResultsAnalysisUsage(AU);
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    auto &CG = getAnalysis<CallGraphWrapperPass>().getCallGraph();

    auto DTGetter = [this](Function &F) -> DominatorTree & {
      return getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };

    using AAData = struct {
      std::unique_ptr<PhiValues> PV;
      std::unique_ptr<BasicAAResult> BAR;
      std::unique_ptr<AAResults> AAR;
    };
    DenseMap<Function *, AAData> F2AAData;

    auto AARGetter = [this, &DTGetter, &F2AAData](Function &F) -> AAResults & {
      auto P = F2AAData.insert({&F, AAData()});
      if (P.second) {
        P.first->second.PV = std::make_unique<PhiValues>(F);
        P.first->second.BAR = std::make_unique<BasicAAResult>(
            F.getParent()->getDataLayout(), F,
            getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F),
            getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F),
            &DTGetter(F), getAnalysis<XmainOptLevelWrapperPass>().getOptLevel(),
            P.first->second.PV.get());
        P.first->second.AAR = std::make_unique<AAResults>(
            createLegacyPMAAResults(*this, F, *P.first->second.BAR));
      }
      return *P.first->second.AAR;
    };

    unsigned OptLevel = getAnalysis<XmainOptLevelWrapperPass>().getOptLevel();

    return NoAliasProp(AARGetter, DTGetter, OptLevel).run(CG);
  }
};

} // end anonymous namespace

char ArgNoAliasProp::ID = 0;

INITIALIZE_PASS_BEGIN(ArgNoAliasProp, DEBUG_TYPE,
                      "Propagate noalias to function arguments", false, false)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(XmainOptLevelWrapperPass)
INITIALIZE_PASS_END(ArgNoAliasProp, DEBUG_TYPE,
                    "Propagate noalias to function arguments", false, false)

ModulePass *llvm::createArgNoAliasPropPass() { return new ArgNoAliasProp(); }
=======
>>>>>>> d70143a36e1b2b5ac4b66ba698f8b74763c4fc66
