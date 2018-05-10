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
#include "llvm/PassRegistry.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"

using namespace llvm;

void llvm::initializeDTransPasses(PassRegistry& PR) {
  initializeDTransAnalysisWrapperPass(PR);
  initializeDTransAOSToSOAWrapperPass(PR);
  initializeDTransDeleteFieldWrapperPass(PR);
  initializeDTransReorderFieldsWrapperPass(PR);
}

void llvm::addDTransPasses(ModulePassManager &MPM) {
  MPM.addPass(dtrans::DeleteFieldPass());
  MPM.addPass(dtrans::AOSToSOAPass());
  MPM.addPass(dtrans::ReorderFieldsPass());
}

void llvm::addDTransLegacyPasses(legacy::PassManagerBase &PM) {
  PM.add(createDTransDeleteFieldWrapperPass());
  PM.add(createDTransAOSToSOAWrapperPass());
  PM.add(createDTransReorderFieldsWrapperPass());
}

// This is used by LinkAllPasses.h. The passes are never actually used when
// created this way.
void llvm::createDTransPasses() {
  (void) llvm::createDTransDeleteFieldWrapperPass();
  (void) llvm::createDTransAOSToSOAWrapperPass();
  (void) llvm::createDTransReorderFieldsWrapperPass();
  (void) llvm::createDTransAnalysisWrapperPass();
}
