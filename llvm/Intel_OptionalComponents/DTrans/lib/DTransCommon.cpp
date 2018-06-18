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

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::opt<bool> DumpModuleBeforeDTrans(
    "dump-module-before-dtrans", cl::init(false), cl::Hidden,
    cl::desc(
        "Dumps LLVM module to dbgs() before first DTRANS transformation"));
#else
constexpr bool DumpModuleBeforeDTrans = false;
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

void llvm::initializeDTransPasses(PassRegistry& PR) {
  initializeDTransAnalysisWrapperPass(PR);
  initializeDTransAOSToSOAWrapperPass(PR);
  initializeDTransDeleteFieldWrapperPass(PR);
  initializeDTransPaddedMallocWrapperPass(PR);
  initializeDTransReorderFieldsWrapperPass(PR);

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
}

void llvm::addDTransLegacyPasses(legacy::PassManagerBase &PM) {
  if (DumpModuleBeforeDTrans) {
    PM.add(createPrintModulePass(dbgs(), "; Module Before DTrans\n"));
  }

  PM.add(createDTransDeleteFieldWrapperPass());
  PM.add(createDTransAOSToSOAWrapperPass());
  PM.add(createDTransReorderFieldsWrapperPass());
}

void llvm::addLateDTransPasses(ModulePassManager &MPM) {
  MPM.addPass(dtrans::PaddedMallocPass());
}

void llvm::addLateDTransLegacyPasses(legacy::PassManagerBase &PM) {
  PM.add(createDTransPaddedMallocWrapperPass());
}

// This is used by LinkAllPasses.h. The passes are never actually used when
// created this way.
void llvm::createDTransPasses() {
  (void) llvm::createDTransDeleteFieldWrapperPass();
  (void) llvm::createDTransAOSToSOAWrapperPass();
  (void) llvm::createDTransReorderFieldsWrapperPass();
  (void) llvm::createDTransPaddedMallocWrapperPass();
  (void) llvm::createDTransAnalysisWrapperPass();

#if !INTEL_PRODUCT_RELEASE
  (void) llvm::createDTransOptBaseTestWrapperPass();
#endif // !INTEL_PRODUCT_RELEASE
}
