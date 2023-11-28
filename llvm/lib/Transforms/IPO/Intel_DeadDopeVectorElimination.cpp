//===------- Intel_DeadDopeVectorElimination.cpp
//--------------------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/Intel_DeadDopeVectorElimination.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_DopeVectorAnalysis.h"
#include "llvm/Analysis/Intel_DopeVectorTypeAnalysis.h"
#include "llvm/Analysis/Intel_OPAnalysisUtils.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Type.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;

using namespace dvanalysis;

#define DEBUG_TYPE "deaddopevectorelimination"

using ValueSet = SetVector<Value *>;

//
// For given value V collect all dependent values which can be safely removed.
// We can safely remove a dependent value if it is
//  1. in SafeTerminals set.
//  2. it has no side effects.
//
// Arguments:
//  Value* V - root node to start traversal
//  const ValueSet *SafeTerminals - a set of values known to be safe to delete
//  ValueSet *Dependencies - a pointer to ValueSet to store result
//
// Return value:
//  The function returns false if it finds dependencies preventing dope vector
//  to be removed. Otherwise it returns true and "Dependencies" parameter
//  holds all found dependencies which can be removed.
//
static bool collectRemovableDependencies(Value *V, ValueSet *SafeTerminals,
                                         ValueSet *Dependencies,
                                         ValueSet *Visited) {
  if (Visited->contains(V))
    return true;

  Visited->insert(V);

  bool IsSafe = false;
  if (isa<SwitchInst>(V) || isa<IndirectBrInst>(V))
    return false;
  else if (auto *BR = dyn_cast<BranchInst>(V)) {
    // In the case if branch depends on dopevector we need to analyze all
    // instruction in successors.
    for (auto S : BR->successors()) {
      for (auto &I : *S) {
        if (!collectRemovableDependencies(&I, SafeTerminals, Dependencies,
                                          Visited))
          return false;
      }
    }
    return true;
  } else if (auto *RI = dyn_cast<ReturnInst>(V)) {
    return RI->getType()->isVoidTy();
  } else if (auto *I = dyn_cast<Instruction>(V))
    IsSafe = !(I->mayHaveSideEffects() || I->getParent()->hasAddressTaken()) ||
             SafeTerminals->contains(V);
  else if (auto *GV = dyn_cast<GlobalValue>(V))
    IsSafe = SafeTerminals->contains(V);
  else if (isa<Operator>(V))
    IsSafe = true;

  if (!IsSafe)
    return false;

  Dependencies->insert(V);
  for (auto U : V->users()) {
    if (!collectRemovableDependencies(U, SafeTerminals, Dependencies, Visited))
      return false;
  }
  return true;
}

//
// The function extracts all terminal values known as accesses to a dope vector.
// Basically these values are:
//  1. GlobalValue representing dope vector
//  2. Calls to allocate and deallocate
//  3. Loads and stores based on dope vector fields.
//
// Arguments:
//  GlobalDopeVector *DV - dope vector to analyze
//
// Return value:
//  The function returns unique_ptr to set of values known as accesses to a dope
//  vector.
//
static std::unique_ptr<ValueSet> getSafeTerminalValues(GlobalDopeVector *DV) {
  auto Res = std::make_unique<ValueSet>();

  auto *DVInfo = DV->getGlobalDopeVectorInfo();
  Res->insert(DVInfo->getDVObject());

  auto &AS = DVInfo->getAllocSites();
  Res->insert(AS.begin(), AS.end());

  auto &DAS = DVInfo->getDeAllocSites();
  Res->insert(DAS.begin(), DAS.end());

  constexpr DopeVectorFieldType SingleValueFields[] = {
      DV_ArrayPtr, DV_ElementSize,      DV_Codim, DV_Flags, DV_Dimensions,
      DV_Reserved, DV_PerDimensionArray};

  for (auto Field : SingleValueFields) {
    auto &Loads = DVInfo->getDopeVectorField(Field)->getLoadsSet();
    Res->insert(Loads.begin(), Loads.end());
    auto &Stores = DVInfo->getDopeVectorField(Field)->getStoresSet();
    Res->insert(Stores.begin(), Stores.end());
  }

  constexpr DopeVectorFieldType DimFields[] = {DV_ExtentBase, DV_StrideBase,
                                               DV_LowerBoundBase};
  for (unsigned long I = 0; I < DVInfo->getRank(); I++) {
    for (auto Field : DimFields) {
      auto &Loads = DVInfo->getDopeVectorField(Field, I)->getLoadsSet();
      Res->insert(Loads.begin(), Loads.end());
      auto &Stores = DVInfo->getDopeVectorField(Field, I)->getStoresSet();
      Res->insert(Stores.begin(), Stores.end());
    }
  }

  return std::move(Res);
}

//
// A wrapper function to combine together:
//  std::unique_ptr<ValueSet> getSafeTerminalValues(GlobalDopeVector *DV)
//  bool collectRemovableDependencies(Value *V, ValueSet *SafeTerminals,
//                                    ValueSet *Dependencies)
//
static std::unique_ptr<ValueSet>
collectRemovableDependencies(GlobalDopeVector *DV) {
  auto Res = std::make_unique<ValueSet>();
  auto *DVInfo = DV->getGlobalDopeVectorInfo();
  auto *DVObj = DVInfo->getDVObject();
  auto STF = getSafeTerminalValues(DV);
  ValueSet Visited;
  if (!collectRemovableDependencies(DVObj, STF.get(), Res.get(), &Visited))
    return {};
  return std::move(Res);
}

//
// The function erases values passed to it as first argument
//
static void eraseValues(
    ValueSet *ValuesToRemove,
    const std::function<const PostDominatorTree &(Function *)> &GetPDT) {
  for (auto *V : *ValuesToRemove) {
    LLVM_DEBUG({
      dbgs() << "Removing: ";
      V->dump();
    });
    if (auto *BR = dyn_cast<BranchInst>(V)) {
      // Here we handle a special scenario where a conditional branch
      // depends on a dope vector. As the dope vector is removed,
      // both paths of execution become inactive, and we substitute
      // the conditional branch with a jump to a common post-dominator
      // of the branch's successors. This introduces new dangling
      // basic blocks, which are expected to be eliminated in subsequent
      // optimizations. Other types of branches do not pose a problem because:
      //  1. An unconditional branch does not depend on dope vectors and is
      //     therefore not eligible for removal by default.
      //  2. Indirect branches and switches are excluded by the
      //     collectRemovableDependencies function.
      if (BR->isConditional()) {
        auto &PDT = GetPDT(BR->getFunction());
        auto EndBB = PDT.findNearestCommonDominator(BR->getSuccessor(0),
                                                    BR->getSuccessor(1));
        IRBuilder<> IRB(BR->getParent());
        IRB.SetInsertPoint(BR);
        IRB.CreateBr(EndBB);
        BR->eraseFromParent();
      }
    } else if (auto *I = dyn_cast<Instruction>(V)) {
      I->replaceAllUsesWith(llvm::UndefValue::get(I->getType()));
      I->eraseFromParent();
    }
  }
}

static bool DeadDopeVectorEliminationPassImpl(
    Module &M, WholeProgramInfo &WPInfo, DopeVectorTypeInfo &DVTI,
    std::function<const TargetLibraryInfo &(Function &)> &GetTLI,
    const std::function<const PostDominatorTree &(Function *)> &GetPDT) {

  LLVM_DEBUG(dbgs() << "Start DeadDopeVectorElimination\n");
  if (!WPInfo.isWholeProgramSafe()) {
    LLVM_DEBUG(dbgs() << "DeadDopeVectorElimination: Not whole program safe\n");
    return false;
  }

  bool Modified = false;
  const DataLayout &DL = M.getDataLayout();
  SmallVector<GlobalValue *, 32> GlobalsToDelete;
  for (auto &G : M.globals()) {
    Type *GTy = G.getValueType();

    if (!DVTI.isDopeVectorType(GTy))
      continue;

    GlobalDopeVector DV(&G, GTy, GetTLI);
    DV.collectAndValidate(DL, /*ForDVCP=*/false);
    if (DV.getAnalysisResult() != GlobalDopeVector::AnalysisResult::AR_Pass)
      continue;

    auto ValuesToRemove = collectRemovableDependencies(&DV);
    if (!ValuesToRemove)
      continue;

    eraseValues(ValuesToRemove.get(), GetPDT);
    GlobalsToDelete.push_back(&G);
    Modified = true;
  }

  for (auto *G : GlobalsToDelete) {
    LLVM_DEBUG({
      dbgs() << "Global Dope Vector(";
      G->printAsOperand(dbgs(), false);
      dbgs() << ") was deleted\n";
    });
    G->eraseFromParent();
  }

  return Modified;
}

PreservedAnalyses
DeadDopeVectorEliminationPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);
  auto &DVTI = AM.getResult<DopeVectorTypeAnalysis>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  std::function<const TargetLibraryInfo &(Function &)> GetTLI =
      [&FAM](Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(F);
  };
  auto GetPDT = [&FAM](Function *F) -> PostDominatorTree & {
    return FAM.getResult<PostDominatorTreeAnalysis>(*F);
  };

  if (!DeadDopeVectorEliminationPassImpl(M, WPInfo, DVTI, GetTLI, GetPDT))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  PA.preserve<DopeVectorTypeAnalysis>();
  return PA;
}
