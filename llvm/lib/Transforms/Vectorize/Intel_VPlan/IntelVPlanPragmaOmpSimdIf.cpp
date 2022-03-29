//=------------- IntelVPlanPragmaOmpSimdIf.cpp -*- C++ -*--------=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Vectorize/IntelVPlanPragmaOmpSimdIf.h"
#include "llvm/InitializePasses.h"

#define DEBUG_TYPE "VPlanPragmaOmpSimdIf"

using namespace llvm;
using namespace llvm::vpo;

INITIALIZE_PASS_BEGIN(VPlanPragmaOmpSimdIf, "vplan-pragma-omp-simd-if",
                      "Pragma omp simd if clause reduction to simdlen(1)",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(VPlanPragmaOmpSimdIf, "vplan-pragma-omp-simd-if",
                    "Pragma omp simd if clause reduction to simdlen(1)", false,
                    false)

bool VPlanPragmaOmpSimdIfImpl::runImpl(Function &F, DominatorTree *DT,
                                       LoopInfo *LI) {

  LLVM_DEBUG(dbgs() << "Pragma omp simd if clause reduction to simdlen(1)\n");

  // Find all the intrinsic calls representing DIR_OMP_SIMD begin directive that
  // have QUAL_OMP_IF clause and store them in BeginSimdDirs array and if clause
  // conditions in SimdIfClauses array
  SmallVector<CallInst *> BeginSimdDirs;
  SmallVector<Value *> SimdIfConds;
  for (auto IterB = df_begin(&F), IterEndB = df_end(&F); IterB != IterEndB;
       ++IterB)
    for (BasicBlock::iterator IterI = IterB->begin(), IterEndI = IterB->end();
         IterI != IterEndI; ++IterI) {
      Instruction *I = &*IterI;
      if (VPOAnalysisUtils::getDirectiveID(I) != DIR_OMP_SIMD)
        continue;
      CallInst *Call = cast<CallInst>(I);
      // Search for QUAL_OMP_IF clause
      unsigned i, NumOB = Call->getNumOperandBundles();
      // Index i start from 1 (not 0) because we want to skip the first
      // OperandBundle, which is the directive name.
      for (i = 1; i < NumOB; ++i) {
        // BU is the ith OperandBundle, which represents a clause
        OperandBundleUse BU = Call->getOperandBundleAt(i);
        if (ClauseSpecifier(BU.getTagName()).getId() == QUAL_OMP_IF) {
          assert(BU.Inputs.size() == 1 &&
                 "QUAL_OMP_IF clause have wrong number of inputs");
          // Now we found DIR_OMP_SIMD directive that have QUAL_OMP_IF clause
          BeginSimdDirs.push_back(Call);
          SimdIfConds.push_back(BU.Inputs[0]);
          break;
        }
      }
    }

  // Go through BeginSimdDirs to process if clause in them by cloning region and
  // loop inside it and changing code as follows:
  //
  // #pragma omp simd ... if(C)
  // for (A) B
  //
  //   ->
  //
  // if (C) {
  //   #pragma omp simd ...
  //   for (A) B
  // } else {
  //   #pragma omp simd ... simdlen(1)
  //   for (A) B
  // }
  int i = SimdIfConds.size();
  for (CallInst *Call : reverse(BeginSimdDirs)) {
    // Determine entry and exit basic blocks to clone
    BasicBlock *EntryBB = Call->getParent();
    BasicBlock *ExitBB = VPOAnalysisUtils::getEndRegionDirBB(Call);
    // Map tht will contain relation between original and cloned objects
    ValueToValueMapTy VMap;
    // Array of basic blocks to clone (will be calculated as basic blocks
    // between EntryBB and ExitBB if empty)
    SmallVector<BasicBlock *> BBSet;
    // Remove if clause before we do region cloning
    Call = VPOUtils::removeOpenMPClausesFromCall(Call, {QUAL_OMP_IF});
    // Clone region
    VPOUtils::singleRegionMultiVersioning(EntryBB, ExitBB, BBSet, VMap,
                                          SimdIfConds[--i], DT, LI);
    // Update simdlen clause in the cloned region
    CallInst *ClonedCall = cast<CallInst>(VMap[Call]);
    ClonedCall =
        VPOUtils::removeOpenMPClausesFromCall(ClonedCall, {QUAL_OMP_SIMDLEN});
    // TODO: Implement a check for no privates are declared and remove the omp
    // simd directive completely from the cloned region.
    IRBuilder<> Builder(ClonedCall);
    VPOUtils::addOperandBundlesInCall(
        ClonedCall, {{"QUAL.OMP.SIMDLEN", {Builder.getInt32(1)}}});
  }

  // Returns true if IR was changed
  return BeginSimdDirs.empty() ? false : true;
}

char VPlanPragmaOmpSimdIf::ID = 0;

VPlanPragmaOmpSimdIf::VPlanPragmaOmpSimdIf() : FunctionPass(ID) {
  initializeVPlanPragmaOmpSimdIfPass(*PassRegistry::getPassRegistry());
}

bool VPlanPragmaOmpSimdIf::runOnFunction(Function &F) {
  if (VPOAnalysisUtils::skipFunctionForOpenmp(F) && skipFunction(F))
    return false;

  auto DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  auto LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  return Impl.runImpl(F, DT, LI);
}

void VPlanPragmaOmpSimdIf::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
}

PreservedAnalyses VPlanPragmaOmpSimdIfPass::run(Function &F,
                                                FunctionAnalysisManager &AM) {
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  auto &LI = AM.getResult<LoopAnalysis>(F);

  bool Changed = Impl.runImpl(F, &DT, &LI);

  if (!Changed)
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}

Pass *llvm::createVPlanPragmaOmpSimdIfPass() {
  return new VPlanPragmaOmpSimdIf();
}
