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
#include <algorithm>

using namespace llvm;

// This option is mapped to user option -opt-mem-layout-trans and is set
// to 2 by default. DTransMemLayoutLevel is used to control transformations
// by not adding the transformation passes.
static cl::opt<unsigned> DTransMemLayoutLevel("dtrans-mem-layout-level",
                                              cl::init(2), cl::ReallyHidden);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Valid values: early -> dump before early DTrans passes
//               late -> dump before late DTrans passes
enum DumpModuleBeforeDTransValues { early, late };

static cl::list<DumpModuleBeforeDTransValues> DumpModuleBeforeDTrans(
    "dump-module-before-dtrans", cl::ReallyHidden,
    cl::desc("Dumps LLVM module to dbgs() before DTRANS transformation"),
    cl::values(clEnumVal(early, "Dump LLVM Module before early DTRANS passes"),
               clEnumVal(late, "Dump LLVM Module before late DTRANS passes")));

static bool hasDumpModuleBeforeDTransValue(DumpModuleBeforeDTransValues V) {
  return std::find(DumpModuleBeforeDTrans.begin(), DumpModuleBeforeDTrans.end(),
                   V) != DumpModuleBeforeDTrans.end();
}

// Padded pointer propagation
static cl::opt<bool>
    EnablePaddedPtrProp("enable-padded-ptr-propagation", cl::init(true),
                        cl::Hidden,
                        cl::desc("Enable padded pointer property propagation"));
#else

#define hasDumpModuleBeforeDTransValue(x) (false)
constexpr bool EnablePaddedPtrProp = false;

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

void llvm::initializeDTransPasses(PassRegistry &PR) {
  initializeDTransAnalysisWrapperPass(PR);
  initializeDTransAOSToSOAWrapperPass(PR);
  initializeDTransDeleteFieldWrapperPass(PR);
  initializeDTransPaddedMallocWrapperPass(PR);
  initializePaddedPtrPropWrapperPass(PR);
  initializeDTransReorderFieldsWrapperPass(PR);
  initializeDTransResolveTypesWrapperPass(PR);
  initializeDTransEliminateROFieldAccessWrapperPass(PR);
  initializeDTransDynCloneWrapperPass(PR);
  initializeDTransSOAToAOSWrapperPass(PR);

#if !INTEL_PRODUCT_RELEASE
  initializeDTransOptBaseTestWrapperPass(PR);
#endif // !INTEL_PRODUCT_RELEASE
}

void llvm::addDTransPasses(ModulePassManager &MPM) {
  if (hasDumpModuleBeforeDTransValue(early))
    MPM.addPass(PrintModulePass(dbgs(), "; Module Before Early DTrans\n"));

  // This must run before any other pass that depends on DTransAnalysis.
  MPM.addPass(dtrans::ResolveTypesPass());

  MPM.addPass(dtrans::DeleteFieldPass());
  MPM.addPass(dtrans::ReorderFieldsPass());
  MPM.addPass(dtrans::AOSToSOAPass());
  MPM.addPass(dtrans::EliminateROFieldAccessPass());
  MPM.addPass(dtrans::DynClonePass());
  MPM.addPass(dtrans::SOAToAOSPass());
}

void llvm::addDTransLegacyPasses(legacy::PassManagerBase &PM) {
  if (hasDumpModuleBeforeDTransValue(early))
    PM.add(createPrintModulePass(dbgs(), "; Module Before Early DTrans\n"));

  // This must run before any other pass that depends on DTransAnalysis.
  PM.add(createDTransResolveTypesWrapperPass());

  PM.add(createDTransDeleteFieldWrapperPass());
  PM.add(createDTransReorderFieldsWrapperPass());
  PM.add(createDTransAOSToSOAWrapperPass());
  PM.add(createDTransEliminateROFieldAccessWrapperPass());
  PM.add(createDTransDynCloneWrapperPass());
  PM.add(createDTransSOAToAOSWrapperPass());
}

void llvm::addLateDTransPasses(ModulePassManager &MPM) {
  if (hasDumpModuleBeforeDTransValue(late))
    MPM.addPass(PrintModulePass(dbgs(), "; Module Before Late DTrans\n"));

  if (EnablePaddedPtrProp) {
    MPM.addPass(llvm::PaddedPtrPropPass());
  }

  MPM.addPass(dtrans::PaddedMallocPass());
}

void llvm::addLateDTransLegacyPasses(legacy::PassManagerBase &PM) {
  if (hasDumpModuleBeforeDTransValue(late))
    PM.add(createPrintModulePass(dbgs(), "; Module Before Late DTrans\n"));

  if (EnablePaddedPtrProp) {
    PM.add(createPaddedPtrPropWrapperPass());
  }

  PM.add(createDTransPaddedMallocWrapperPass());
}

// This is used by LinkAllPasses.h. The passes are never actually used when
// created this way.
void llvm::createDTransPasses() {
  (void)llvm::createDTransDeleteFieldWrapperPass();
  (void)llvm::createDTransAOSToSOAWrapperPass();
  (void)llvm::createDTransReorderFieldsWrapperPass();
  (void)llvm::createDTransPaddedMallocWrapperPass();
  (void)llvm::createDTransEliminateROFieldAccessWrapperPass();
  (void)llvm::createPaddedPtrPropWrapperPass();
  (void)llvm::createDTransSOAToAOSWrapperPass();
  (void)llvm::createDTransAnalysisWrapperPass();
  (void)llvm::createDTransDynCloneWrapperPass();

#if !INTEL_PRODUCT_RELEASE
  (void)llvm::createDTransOptBaseTestWrapperPass();
#endif // !INTEL_PRODUCT_RELEASE
}
