//===- ArgNoAliasProp.cpp ---- Add noalias to function args -----*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
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

public:
  NoAliasProp(function_ref<AAResults &(Function &)> AAGetter,
              function_ref<DominatorTree &(Function &)> DTGetter)
      : AAGetter(AAGetter), DTGetter(DTGetter) {}

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

  AAResults &AA = AAGetter(F);
  DominatorTree &DT = DTGetter(F);

  // Check all function's call sites to see what values are passed to the
  // pointer arguments. We check if actual value that is passed to the call
  //  - is derived from a noalias pointer
  //  - is not captured before the call
  //  - does not alias with any other call's arguments
  // If that is true for all call sites we can add noalias attribute to the
  // function argument.
  // TODO: we may consider cloning functions if noalias can be added for some
  // call sites but not all.
  for (Use &U : F.uses()) {
    AbstractCallSite CS(&U);

    // Must be a direct or a callback call.
    if (!CS || !(CS.isDirectCall() || CS.isCallbackCall()))
      return false;

    for (auto &P : PtrArgs) {
      if (!P.second)
        continue;

      bool NoAlias = false;
      if (Value *PV = CS.getCallArgOperand(*P.first)) {
        Value *Origin = PV->stripPointerCasts();

        // So far we are checking for aliasing only the following values that
        // are passed to the call
        //  - Function arguments with noalias attribute
        //  - Function calls that return noalias pointer
        //  - Alloca instructions
        NoAlias = isNoAliasArgument(Origin) || isNoAliasCall(Origin) ||
                  isa<AllocaInst>(Origin);

        if (NoAlias)
          // Pointer that is passed to the call should not be captured because
          // otherwise we cannot guarantee that it does not alias with any other
          // pointer value that is used inside the function.
          NoAlias = !PointerMayBeCapturedBefore(PV, /*ReturnCaptures=*/true,
                                                /*StoreCaptures=*/true,
                                                CS.getInstruction(), &DT);

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

  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

  auto &CG = AM.getResult<CallGraphAnalysis>(M);

  auto AARGetter = [&](Function &F) -> AAResults & {
    return FAM.getResult<AAManager>(F);
  };

  auto DTGetter = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };

  if (!NoAliasProp(AARGetter, DTGetter).run(CG))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<AndersensAA>();
  PA.preserve<CallGraphAnalysis>();
  PA.preserve<DominatorTreeAnalysis>();
  PA.preserve<GlobalsAA>();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

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
    AU.addPreserved<AndersensAAWrapperPass>();
    AU.addPreserved<CallGraphWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
    getAAResultsAnalysisUsage(AU);
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    auto &CG = getAnalysis<CallGraphWrapperPass>().getCallGraph();

    auto DTGetter = [this](Function &F) -> DominatorTree & {
      return getAnalysis<DominatorTreeWrapperPass>(F).getDomTree();
    };

    return NoAliasProp(LegacyAARGetter(*this), DTGetter).run(CG);
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
INITIALIZE_PASS_END(ArgNoAliasProp, DEBUG_TYPE,
                    "Propagate noalias to function arguments", false, false)

ModulePass *llvm::createArgNoAliasPropPass() { return new ArgNoAliasProp(); }
