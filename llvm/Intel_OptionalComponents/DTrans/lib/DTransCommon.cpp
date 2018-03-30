//===----------------- DTransCommon.cpp - Shared DTrans code --------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file provides functions that are common to all DTrans passes.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/PassRegistry.h"

using namespace llvm;

// This option is mapped to user option -opt-mem-layout-trans and is set
// to 2 by default. DTransMemLayoutLevel is used to control transformations
// by not adding the transformation passes.
static cl::opt<unsigned> DTransMemLayoutLevel("dtrans-mem-layout-level",
                                         cl::init(2), cl::ReallyHidden);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::opt<bool> DumpModuleBeforeDTrans(
    "dump-module-before-dtrans", cl::init(false), cl::Hidden,
    cl::desc(
        "Dumps LLVM module to dbgs() before first DTRANS transformation"));

// Padded pointer propagation
static cl::opt<bool>
    EnablePaddedPtrProp("enable-padded-ptr-propagation", cl::init(true),
                        cl::Hidden,
                        cl::desc("Enable padded pointer property propagation"));
#else
constexpr bool DumpModuleBeforeDTrans = false;
constexpr bool EnablePaddedPtrProp = false;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

void llvm::initializeDTransPasses(PassRegistry& PR) {
  initializeDTransAnalysisWrapperPass(PR);
  initializeDTransAOSToSOAWrapperPass(PR);
  initializeDTransDeleteFieldWrapperPass(PR);
  initializeDTransPaddedMallocWrapperPass(PR);
  initializePaddedPtrPropWrapperPass(PR);
  initializeDTransReorderFieldsWrapperPass(PR);
  initializeDTransEliminateROFieldAccessWrapperPass(PR);

#if !INTEL_PRODUCT_RELEASE
  initializeDTransOptBaseTestWrapperPass(PR);
#endif // !INTEL_PRODUCT_RELEASE
}

void llvm::addDTransPasses(ModulePassManager &MPM) {
  if (DumpModuleBeforeDTrans) {
    MPM.addPass(PrintModulePass(dbgs(), "; Module Before DTrans\n"));
  }

  MPM.addPass(dtrans::DeleteFieldPass());
  MPM.addPass(dtrans::AOSToSOAPass());
  MPM.addPass(dtrans::ReorderFieldsPass());
  MPM.addPass(dtrans::EliminateROFieldAccessPass());
}

void llvm::addDTransLegacyPasses(legacy::PassManagerBase &PM) {
  if (DumpModuleBeforeDTrans) {
    PM.add(createPrintModulePass(dbgs(), "; Module Before DTrans\n"));
  }

  PM.add(createDTransDeleteFieldWrapperPass());
  PM.add(createDTransAOSToSOAWrapperPass());
  PM.add(createDTransReorderFieldsWrapperPass());
  PM.add(createDTransEliminateROFieldAccessWrapperPass());
}

void llvm::addLateDTransPasses(ModulePassManager &MPM) {
  MPM.addPass(dtrans::PaddedMallocPass());

  if (EnablePaddedPtrProp) {
    MPM.addPass(llvm::PaddedPtrPropPass());
  }
}

void llvm::addLateDTransLegacyPasses(legacy::PassManagerBase &PM) {
  PM.add(createDTransPaddedMallocWrapperPass());

  if (EnablePaddedPtrProp) {
    PM.add(createPaddedPtrPropWrapperPass());
  }
}

// This is used by LinkAllPasses.h. The passes are never actually used when
// created this way.
void llvm::createDTransPasses() {
  (void) llvm::createDTransDeleteFieldWrapperPass();
  (void) llvm::createDTransAOSToSOAWrapperPass();
  (void) llvm::createDTransReorderFieldsWrapperPass();
  (void) llvm::createDTransPaddedMallocWrapperPass();
  (void) llvm::createDTransEliminateROFieldAccessWrapperPass();
  (void) llvm::createPaddedPtrPropWrapperPass();
  (void) llvm::createDTransAnalysisWrapperPass();

#if !INTEL_PRODUCT_RELEASE
  (void) llvm::createDTransOptBaseTestWrapperPass();
#endif // !INTEL_PRODUCT_RELEASE
}
