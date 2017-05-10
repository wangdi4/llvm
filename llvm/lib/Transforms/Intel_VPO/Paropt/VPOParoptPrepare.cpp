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
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/InitializePasses.h"

#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"

#include "llvm/Transforms/Scalar.h"
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
INITIALIZE_PASS_DEPENDENCY(WRegionInfo)
INITIALIZE_PASS_END(VPOParoptPrepare, "vpo-paropt-prepare", 
                    "VPO Paropt Prepare Function Pass", false, false)

char VPOParoptPrepare::ID = 0;

FunctionPass *llvm::createVPOParoptPreparePass(unsigned Mode) {
  return new VPOParoptPrepare(ParPrepare & Mode);
}

VPOParoptPrepare::VPOParoptPrepare(unsigned MyMode)
    : FunctionPass(ID), Mode(MyMode) {
  DEBUG(dbgs() << "\n\n====== Enter VPO Paropt Prepare Pass ======\n\n");
  initializeVPOParoptPreparePass(*PassRegistry::getPassRegistry());
}

void VPOParoptPrepare::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequiredID(LoopSimplifyID);
  AU.addRequired<WRegionInfo>();
}

bool VPOParoptPrepare::runOnFunction(Function &F) {
  bool Changed = false;

  DEBUG(dbgs() << "\n=== VPOParoptPrepare Start: " << F.getName() <<" {\n");

  // TODO: need Front-End to set F.hasOpenMPDirective()
  if (F.isDeclaration()) { // if(!F.hasOpenMPDirective()))
    DEBUG(dbgs() << "\n}=== VPOParoptPrepare End (no change): " 
                                                     << F.getName() <<"\n");
    return Changed;
  }

  // Walk the W-Region Graph top-down, and create W-Region List
  WRegionInfo &WI = getAnalysis<WRegionInfo>();
  WI.buildWRGraph(WRegionCollection::LLVMIR);

  DEBUG(dbgs() << "\n=== W-Region Graph Build Done: " << F.getName() <<"\n");

  if (WI.WRGraphIsEmpty()) {
    DEBUG(dbgs() << "\nNo WRegion Candidates for Parallelization \n");
  }

  DEBUG(WI.dump());

  DEBUG(errs() << "VPOParoptPrepare Pass: ");
  DEBUG(errs().write_escaped(F.getName()) << '\n');

  DEBUG(dbgs() << "\n === VPOParoptPrepare Pass before Transformation === \n");

  // AUTOPAR | OPENMP | SIMD | OFFLOAD
  VPOParoptTransform VP(&F, &WI, 
                        WI.getDomTree(), WI.getLoopInfo(), WI.getSE(), Mode);
  Changed = Changed | VP.paroptTransforms();

  DEBUG(dbgs() << "\n === VPOParoptPrepare Pass after Transformation === \n");

  // Remove calls to directive intrinsics since the LLVM back end does not
  // know how to translate them.
  // VPOUtils::stripDirectives(F);

  DEBUG(dbgs() << "\n}=== VPOParoptPrepare End: " << F.getName() <<"\n");
  DEBUG(dbgs() << "\n====== Exit VPO Paropt Prepare Pass ======\n\n");
  return Changed;
}
