//===----------------- DTransCommon.cpp - Shared DTrans code --------------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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

// Initial Memory allocation Trim down transformation.
static cl::opt<bool> EnableMemInitTrimDown("enable-dtrans-meminittrimdown",
                                    cl::init(true), cl::Hidden,
                                    cl::desc("Enable DTrans MemInitTrimDown"));

// SOA-to-AOS transformation.
static cl::opt<bool> EnableSOAToAOS("enable-dtrans-soatoaos", cl::init(true),
                                    cl::Hidden,
                                    cl::desc("Enable DTrans SOAToAOS"));

static cl::opt<bool> EnableTranspose("enable-dtrans-transpose", cl::init(true),
                                     cl::Hidden,
                                     cl::desc("Enable DTrans Transpose"));
// Delete fields transformation.
static cl::opt<bool> EnableDeleteFields("enable-dtrans-deletefield",
                                        cl::init(true), cl::Hidden,
                                        cl::desc("Enable DTrans delete field"));

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Valid values: early -> dump before early DTrans passes
//               late -> dump before late DTrans passes
enum DumpModuleDTransValues { early, late };

static cl::list<DumpModuleDTransValues> DumpModuleBeforeDTrans(
    "dump-module-before-dtrans", cl::ReallyHidden,
    cl::desc("Dumps LLVM module to dbgs() before DTRANS transformation"),
    cl::values(clEnumVal(early, "Dump LLVM Module before early DTRANS passes"),
               clEnumVal(late, "Dump LLVM Module before late DTRANS passes")));

static bool hasDumpModuleBeforeDTransValue(DumpModuleDTransValues V) {
  return std::find(DumpModuleBeforeDTrans.begin(), DumpModuleBeforeDTrans.end(),
                   V) != DumpModuleBeforeDTrans.end();
}

static cl::list<DumpModuleDTransValues> DumpModuleAfterDTrans(
    "dump-module-after-dtrans", cl::ReallyHidden,
    cl::desc("Dumps LLVM module to dbgs() after DTRANS transformation"),
    cl::values(clEnumVal(early, "Dump LLVM Module after early DTRANS passes"),
               clEnumVal(late, "Dump LLVM Module after late DTRANS passes")));

static bool hasDumpModuleAfterDTransValue(DumpModuleDTransValues V) {
  return std::find(DumpModuleAfterDTrans.begin(), DumpModuleAfterDTrans.end(),
                   V) != DumpModuleAfterDTrans.end();
}

// Padded pointer propagation
static cl::opt<bool>
    EnablePaddedPtrProp("enable-padded-ptr-propagation", cl::init(true),
                        cl::Hidden,
                        cl::desc("Enable padded pointer property propagation"));

static cl::opt<bool>
    EnableResolveTypes("enable-resolve-types", cl::init(true),
                        cl::Hidden,
                        cl::desc("Enable pre-dtrans type resolution"));

#else

#define hasDumpModuleBeforeDTransValue(x) (false)
#define hasDumpModuleAfterDTransValue(x) (false)
constexpr bool EnablePaddedPtrProp = true;
constexpr bool EnableResolveTypes = true;

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

void llvm::initializeDTransPasses(PassRegistry &PR) {
  initializeDTransAnalysisWrapperPass(PR);
  initializeDTransPaddedMallocWrapperPass(PR);
  initializePaddedPtrPropWrapperPass(PR);
  initializeDTransResolveTypesWrapperPass(PR);
  initializeDTransSOAToAOSWrapperPass(PR);
  initializeDTransDeleteFieldWrapperPass(PR);
  initializeDTransReorderFieldsWrapperPass(PR);
  initializeDTransAOSToSOAWrapperPass(PR);
  initializeDTransEliminateROFieldAccessWrapperPass(PR);
  initializeDTransDynCloneWrapperPass(PR);
  initializeDTransAnnotatorCleanerWrapperPass(PR);
  initializeDTransWeakAlignWrapperPass(PR);
  initializeDTransMemInitTrimDownWrapperPass(PR);
  initializeDTransTransposeWrapperPass(PR);

#if !INTEL_PRODUCT_RELEASE
  initializeDTransOptBaseTestWrapperPass(PR);
#endif // !INTEL_PRODUCT_RELEASE
}

void llvm::addDTransPasses(ModulePassManager &MPM) {
  if (hasDumpModuleBeforeDTransValue(early))
    MPM.addPass(PrintModulePass(dbgs(), "; Module Before Early DTrans\n"));

  // This must run before any other pass that depends on DTransAnalysis.
  if (EnableResolveTypes)
    MPM.addPass(dtrans::ResolveTypesPass());

  if (EnableTranspose)
    MPM.addPass(dtrans::TransposePass());
  if (EnableSOAToAOS)
    MPM.addPass(dtrans::SOAToAOSPass());
  MPM.addPass(dtrans::WeakAlignPass());
  if (EnableDeleteFields)
    MPM.addPass(dtrans::DeleteFieldPass());
  if (EnableMemInitTrimDown)
    MPM.addPass(dtrans::MemInitTrimDownPass());
  MPM.addPass(dtrans::ReorderFieldsPass());
  MPM.addPass(dtrans::AOSToSOAPass());
  MPM.addPass(dtrans::EliminateROFieldAccessPass());
  MPM.addPass(dtrans::DynClonePass());
  MPM.addPass(dtrans::AnnotatorCleanerPass());

  if (hasDumpModuleAfterDTransValue(early))
    MPM.addPass(PrintModulePass(dbgs(), "; Module After Early DTrans\n"));
}

void llvm::addDTransLegacyPasses(legacy::PassManagerBase &PM) {
  if (hasDumpModuleBeforeDTransValue(early))
    PM.add(createPrintModulePass(dbgs(), "; Module Before Early DTrans\n"));

  // This must run before any other pass that depends on DTransAnalysis.
  if (EnableResolveTypes)
    PM.add(createDTransResolveTypesWrapperPass());

  if (EnableTranspose)
    PM.add(createDTransTransposeWrapperPass());
  if (EnableSOAToAOS)
    PM.add(createDTransSOAToAOSWrapperPass());
  PM.add(createDTransWeakAlignWrapperPass());
  if (EnableDeleteFields)
    PM.add(createDTransDeleteFieldWrapperPass());
  if (EnableMemInitTrimDown)
    PM.add(createDTransMemInitTrimDownWrapperPass());
  PM.add(createDTransReorderFieldsWrapperPass());
  PM.add(createDTransAOSToSOAWrapperPass());
  PM.add(createDTransEliminateROFieldAccessWrapperPass());
  PM.add(createDTransDynCloneWrapperPass());
  PM.add(createDTransAnnotatorCleanerWrapperPass());

  if (hasDumpModuleAfterDTransValue(early))
    PM.add(createPrintModulePass(dbgs(), "; Module After Early DTrans\n"));
}

void llvm::addLateDTransPasses(ModulePassManager &MPM) {
  if (hasDumpModuleBeforeDTransValue(late))
    MPM.addPass(PrintModulePass(dbgs(), "; Module Before Late DTrans\n"));

  if (EnablePaddedPtrProp) {
    MPM.addPass(llvm::PaddedPtrPropPass());
  }

  MPM.addPass(dtrans::PaddedMallocPass());

  if (hasDumpModuleAfterDTransValue(late))
    MPM.addPass(PrintModulePass(dbgs(), "; Module After Late DTrans\n"));
}

void llvm::addLateDTransLegacyPasses(legacy::PassManagerBase &PM) {
  if (hasDumpModuleBeforeDTransValue(late))
    PM.add(createPrintModulePass(dbgs(), "; Module Before Late DTrans\n"));

  if (EnablePaddedPtrProp) {
    PM.add(createPaddedPtrPropWrapperPass());
  }

  PM.add(createDTransPaddedMallocWrapperPass());

  if (hasDumpModuleAfterDTransValue(late))
    PM.add(createPrintModulePass(dbgs(), "; Module After Late DTrans\n"));
}

// This is used by LinkAllPasses.h. The passes are never actually used when
// created this way.
void llvm::createDTransPasses() {
  (void)llvm::createDTransDeleteFieldWrapperPass();
  (void)llvm::createDTransAOSToSOAWrapperPass();
  (void)llvm::createDTransAnnotatorCleanerWrapperPass();
  (void)llvm::createDTransReorderFieldsWrapperPass();
  (void)llvm::createDTransPaddedMallocWrapperPass();
  (void)llvm::createDTransEliminateROFieldAccessWrapperPass();
  (void)llvm::createPaddedPtrPropWrapperPass();
  (void)llvm::createDTransSOAToAOSWrapperPass();
  (void)llvm::createDTransAnalysisWrapperPass();
  (void)llvm::createDTransDynCloneWrapperPass();
  (void)llvm::createDTransWeakAlignWrapperPass();
  (void)llvm::createDTransMemInitTrimDownWrapperPass();
  (void)llvm::createDTransTransposeWrapperPass();

#if !INTEL_PRODUCT_RELEASE
  (void)llvm::createDTransOptBaseTestWrapperPass();
#endif // !INTEL_PRODUCT_RELEASE
}
