#if INTEL_COLLAB
//===------- VPOParoptPrepare.cpp - Paropt Prepare Pass for OpenMP --------===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation. and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// Authors:
// --------
// Xinmin Tian (xinmin.tian@intel.com)
//
// Major Revisions:
// ----------------
// July 2016: Initial Implementation of Paropt Prepare Pass (Xinmin Tian)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the ParOpt prepare pass interface to perform Prepare
/// transformation for OpenMP parallelization, simd and Offloading
///
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/InitializePasses.h"

#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"

#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Intel_VPO/VPOPasses.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptPrepare.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptTransform.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "VPOParoptPrepare"

INITIALIZE_PASS_BEGIN(VPOParoptPrepare, "vpo-paropt-prepare",
                     "VPO Paropt Prepare Function Pass", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_DEPENDENCY(WRegionInfoWrapperPass)
INITIALIZE_PASS_END(VPOParoptPrepare, "vpo-paropt-prepare",
                    "VPO Paropt Prepare Function Pass", false, false)

char VPOParoptPrepare::ID = 0;

FunctionPass *llvm::createVPOParoptPreparePass(unsigned Mode) {
  return new VPOParoptPrepare(Mode & ParPrepare);
}

VPOParoptPrepare::VPOParoptPrepare(unsigned MyMode)
    : FunctionPass(ID), Impl(MyMode) {
  LLVM_DEBUG(dbgs() << "\n\n====== Enter VPO Paropt Prepare ======\n\n");
  initializeVPOParoptPreparePass(*PassRegistry::getPassRegistry());
}

VPOParoptPreparePass::VPOParoptPreparePass(unsigned MyMode)
    : Mode(MyMode) {
  LLVM_DEBUG(dbgs() << "\n\n====== Enter VPO Paropt Prepare Pass ======\n\n");
}

void VPOParoptPrepare::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequiredID(LoopSimplifyID);
  AU.addRequired<WRegionInfoWrapperPass>();
}

bool VPOParoptPrepare::runOnFunction(Function &F) {

  LLVM_DEBUG(dbgs() << "\n=== VPOParoptPrepare Start: " << F.getName()
                    << " {\n");

  // TODO: need Front-End to set F.hasOpenMPDirective()
  if (F.isDeclaration()) { // if(!F.hasOpenMPDirective()))
    LLVM_DEBUG(dbgs() << "\n}=== VPOParoptPrepare End (no change): "
                      << F.getName() << "\n");
    return false;
  }

  WRegionInfo &WI = getAnalysis<WRegionInfoWrapperPass>().getWRegionInfo();

  bool Changed = Impl.runImpl(F, WI);

  LLVM_DEBUG(dbgs() << "\n}=== VPOParoptPrepare End: " << F.getName() << "\n");
  LLVM_DEBUG(dbgs() << "\n====== Exit VPO Paropt Prepare ======\n\n");

  return Changed;
}

bool VPOParoptPreparePass::runImpl(Function &F, WRegionInfo &WI) {
  bool Changed = false;

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
  auto && IsTargetCSA = [&F]() {
    Triple TT(F.getParent()->getTargetTriple());
    return TT.getArch() == Triple::ArchType::csa;
  };
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION

  if (Mode & ParPrepare) {
    LLVM_DEBUG(
        dbgs() << "VPOParoptPreparePass: Before Par Sections Transformation");
    LLVM_DEBUG(dbgs() << F << " \n");

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
    // For CSA, don't change OMP SECTIONS into a loop
    if (!IsTargetCSA())
#endif // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
    Changed = VPOUtils::parSectTransformer(&F, WI.getDomTree());

    LLVM_DEBUG(
        dbgs() << "VPOParoptPreparePass: After Par Sections Transformation");
    LLVM_DEBUG(dbgs() << F << " \n");
  }

  // Walk the W-Region Graph top-down, and create W-Region List
  WI.buildWRGraph(WRegionCollection::LLVMIR);

  LLVM_DEBUG(dbgs() << "\n=== W-Region Graph Build Done: " << F.getName()
                    << "\n");

  if (WI.WRGraphIsEmpty()) {
    LLVM_DEBUG(dbgs() << "\nNo WRegion Candidates for Parallelization \n");
  }

  LLVM_DEBUG(WI.print(dbgs()));

  LLVM_DEBUG(errs() << "VPOParoptPreparePass: ");
  LLVM_DEBUG(errs().write_escaped(F.getName()) << '\n');

  LLVM_DEBUG(
      dbgs() << "\n === VPOParoptPreparePass before Transformation === \n");

  // AUTOPAR | OPENMP | SIMD | OFFLOAD
  VPOParoptTransform VP(&F, &WI, WI.getDomTree(), WI.getLoopInfo(), WI.getSE(),
                        WI.getTargetTransformInfo(), WI.getAssumptionCache(),
                        WI.getTargetLibraryInfo(), WI.getAliasAnalysis(), Mode);
  Changed = Changed | VP.paroptTransforms();

  LLVM_DEBUG(
      dbgs() << "\n === VPOParoptPreparePass after Transformation === \n");

  // Remove calls to directive intrinsics since the LLVM back end does not
  // know how to translate them.
  // VPOUtils::stripDirectives(F);

  return Changed;
}

PreservedAnalyses VPOParoptPreparePass::run(Function &F,
                                            FunctionAnalysisManager &AM) {

  LLVM_DEBUG(dbgs() << "\n=== VPOParoptPreparePass Start: " << F.getName()
                    << " {\n");

  // TODO: need Front-End to set F.hasOpenMPDirective()
  if (F.isDeclaration()) { // if(!F.hasOpenMPDirective()))
    LLVM_DEBUG(dbgs() << "\n}=== VPOParoptPreparePass End (no change): "
                      << F.getName() << "\n");
    return PreservedAnalyses::all();
  }

  auto &WI = AM.getResult<WRegionInfoAnalysis>(F);

  bool Changed = runImpl(F, WI);

  LLVM_DEBUG(dbgs() << "\n}=== VPOParoptPreparePass End: " << F.getName()
                    << "\n");
  LLVM_DEBUG(dbgs() << "\n====== Exit VPO Paropt Prepare Pass======\n\n");

  if (!Changed)
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();;
}
#endif // INTEL_COLLAB
