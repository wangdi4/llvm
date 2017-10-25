//===-------- VPOParopt.cpp - Paropt Pass for Auto-Par and OpenMP ---------===//
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
// Nov 2015: Initial Implementation of Paropt Pass (Xinmin Tian)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the ParOpt pass interface to perform transformation
/// for OpenMP and Auto-parallelization
///
//===----------------------------------------------------------------------===//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/InitializePasses.h"

#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/Intel_VPO/WRegionInfo/WRegionInfo.h"

#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Intel_VPO/VPOPasses.h"
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptTransform.h"
#include "llvm/Transforms/Intel_VPO/Paropt/VPOParoptTpv.h"
#define DEBUG_TYPE "VPOParopt"


using namespace llvm;
using namespace llvm::vpo;

INITIALIZE_PASS_BEGIN(VPOParopt, "vpo-paropt", "VPO Paropt Module Pass", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
INITIALIZE_PASS_DEPENDENCY(WRegionInfo)
INITIALIZE_PASS_END(VPOParopt, "vpo-paropt", "VPO Paropt Module Pass", false,
                    false)

char VPOParopt::ID = 0;

ModulePass *llvm::createVPOParoptPass(unsigned Mode) { 
  return new VPOParopt((ParTrans | OmpPar | OmpVec | OmpTpv | OmpTbb) & Mode);
}

VPOParopt::VPOParopt(unsigned MyMode)
    : ModulePass(ID), Mode(MyMode) {
  DEBUG(dbgs() << "\n\n====== Start VPO Paropt Pass ======\n\n");
  initializeVPOParoptPass(*PassRegistry::getPassRegistry());
}

void VPOParopt::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequiredID(LoopSimplifyID);
  AU.addRequired<WRegionInfo>();
}

bool VPOParopt::runOnModule(Module &M) {
  if (skipModule(M))
    return false;

  bool Changed = false;

  /// \brief As new functions to be added, so we need to prepare the
  /// list of functions we want to work on in advance.
  std::vector<Function *> FnList;

  for (auto F = M.begin(), E = M.end(); F != E; ++F) {
    // TODO: need Front-End to set F->hasOpenMPDirective()
    if (F->isDeclaration()) // if(!F->hasOpenMPDirective()))
      continue;
    DEBUG(dbgs() << "\n=== VPOParopt func: " << F->getName() <<" {\n");
    FnList.push_back(&*F);
  }

  // Iterate over all functions which OpenMP directives to perform Paropt
  // transformation and generate MT-code
  for (auto F : FnList) {

    DEBUG(dbgs() << "\n=== VPOParopt begin func: " << F->getName() <<" {\n");

    // Walk the W-Region Graph top-down, and create W-Region List
    WRegionInfo &WI = getAnalysis<WRegionInfo>(*F);
    WI.buildWRGraph(WRegionCollection::LLVMIR);

    if (WI.WRGraphIsEmpty()) {
      DEBUG(dbgs() << "\nNo WRegion Candidates for Parallelization \n");
    }

    DEBUG(WI.dump());

    //
    // Set up a function pass manager so that we can run some cleanup
    // transforms on the LLVM IR after code gen.
    //
    // legacy::FunctionPassManager FPM(&M);

    DEBUG(errs() << "VPOParopt Pass: ");
    DEBUG(errs().write_escaped(F->getName()) << '\n');

    DEBUG(dbgs() << "\n=== VPOParopt before ParoptTransformer{\n");

    // AUTOPAR | OPENMP | SIMD | OFFLOAD
    VPOParoptTransform VP(F, &WI, WI.getDomTree(), WI.getLoopInfo(), WI.getSE(),
                          Mode);
    Changed = Changed | VP.paroptTransforms();

    DEBUG(dbgs() << "\n}=== VPOParopt after ParoptTransformer\n");

    // Remove calls to directive intrinsics since the LLVM back end does not
    // know how to translate them.
    // VPOUtils::stripDirectives(*F);

    // It is possible that stripDirectives eliminates all instructions in a
    // basic block except for the branch instruction. Use CFG simplify to
    // eliminate them.
    // FPM.add(createCFGSimplificationPass());
    // FPM.run(*F);

    DEBUG(dbgs() << "\n}=== VPOParopt end func: " << F->getName() <<"\n");
  }

  // Thread private legacy mode implementation
  if (Mode & OmpTpv) {
    VPOParoptTpvLegacyPass VPTL;
    ModuleAnalysisManager DummyMAM;
    PreservedAnalyses PA = VPTL.run(M, DummyMAM);
    Changed = Changed | !PA.areAllPreserved();
  }

  DEBUG(dbgs() << "\n====== End VPO Paropt Pass ======\n\n");
  return Changed;
}
