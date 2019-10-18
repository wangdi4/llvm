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

// SOA-to-AOS Prepare transformation.
static cl::opt<bool> EnableSOAToAOSPrepare("enable-dtrans-soatoaos-prepare",
                                    cl::init(false), cl::Hidden,
                                    cl::desc("Enable DTrans SOAToAOSPrepare"));
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

// Valid values for the dump-module-after-dtrans and dump-module-before-dtrans
// options:
//   early -> dump before/after early DTrans passes
//   late -> dump before/after late DTrans passes
//   <passname> -> dump before/after specific pass
enum DumpModuleDTransValues {
  early,
  resolvetypes,
  transpose,
  soatoaosprepare,
  soatoaos,
  weakalign,
  deletefield,
  meminittrimdown,
  reorderfields,
  aostosoa,
  elimrofieldaccess,
  dynclone,
  annotatorcleaner,
  late
};

static const char *DumpModuleDTransNames[] = {"Early",
                                              "ResolveTypes",
                                              "Transpose",
                                              "SOAToAOSPrepare",
                                              "SOAToAOS",
                                              "WeakAlign",
                                              "DeleteField",
                                              "MemInitTrimDown",
                                              "ReorderFields",
                                              "AOSToSOA",
                                              "EliminateROFieldAccess",
                                              "DynClone",
                                              "AnnotatorCleaner",
                                              "Late"};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static cl::list<DumpModuleDTransValues> DumpModuleBeforeDTrans(
    "dump-module-before-dtrans", cl::ReallyHidden,
    cl::desc("Dumps LLVM module to dbgs() before DTRANS transformation"),
    cl::values(
        clEnumVal(early, "Dump LLVM Module before early DTRANS passes"),
        clEnumVal(resolvetypes, "Dump LLVM Module before ResolveTypes pass"),
        clEnumVal(transpose, "Dump LLVM Module before Transpose pass"),
        clEnumVal(soatoaosprepare,
                  "Dump LLVM Module before SOA-to-AOS Prepare pass"),
        clEnumVal(soatoaos, "Dump LLVM Module before SOA-to-AOS pass"),
        clEnumVal(weakalign, "Dump LLVM Module before WeakAlign pass"),
        clEnumVal(deletefield, "Dump LLVM Module before DeleteField pass"),
        clEnumVal(meminittrimdown,
                  "Dump LLVM Module before MemInitTrimDown pass"),
        clEnumVal(reorderfields, "Dump LLVM Module before ReorderFields pass"),
        clEnumVal(aostosoa, "Dump LLVM Module before AOS-to-SOA pass"),
        clEnumVal(elimrofieldaccess,
                  "Dump LLVM Module before EliminateROFieldAccess pass"),
        clEnumVal(dynclone, "Dump LLVM Module before DynClone pass"),
        clEnumVal(annotatorcleaner,
                  "Dump LLVM Module before AnnotatorCleaner pass"),
        clEnumVal(late, "Dump LLVM Module before late DTRANS passes")));

static bool hasDumpModuleBeforeDTransValue(DumpModuleDTransValues V) {
  return std::find(DumpModuleBeforeDTrans.begin(), DumpModuleBeforeDTrans.end(),
                   V) != DumpModuleBeforeDTrans.end();
}

static cl::list<DumpModuleDTransValues> DumpModuleAfterDTrans(
    "dump-module-after-dtrans", cl::ReallyHidden,
    cl::desc("Dumps LLVM module to dbgs() after DTRANS transformation"),
    cl::values(
        clEnumVal(early, "Dump LLVM Module after early DTRANS passes"),
        clEnumVal(resolvetypes, "Dump LLVM Module after ResolveTypes pass"),
        clEnumVal(transpose, "Dump LLVM Module after Transpose pass"),
        clEnumVal(soatoaosprepare,
                  "Dump LLVM Module after SOA-to-AOS Prepare pass"),
        clEnumVal(soatoaos, "Dump LLVM Module after SOA-to-AOS pass"),
        clEnumVal(weakalign, "Dump LLVM Module after WeakAlign pass"),
        clEnumVal(deletefield, "Dump LLVM Module after DeleteField pass"),
        clEnumVal(meminittrimdown,
                  "Dump LLVM Module after MemInitTrimDown pass"),
        clEnumVal(reorderfields, "Dump LLVM Module after ReorderFields pass"),
        clEnumVal(aostosoa, "Dump LLVM Module after AOS-to-SOA pass"),
        clEnumVal(elimrofieldaccess,
                  "Dump LLVM Module after EliminateROFieldAccess pass"),
        clEnumVal(dynclone, "Dump LLVM Module after DynClone pass"),
        clEnumVal(annotatorcleaner,
                  "Dump LLVM Module after AnnotatorCleaner pass"),
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
  initializeDTransSOAToAOSPrepareWrapperPass(PR);
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

// Add a new pass manager type pass. Add module dumps before/after
// the pass, if requested.
template <typename PassT>
static void addPass(ModulePassManager &MPM, DumpModuleDTransValues Phase,
                    PassT P) {
  assert(Phase <= late && "Phase value out of range");
  if (hasDumpModuleBeforeDTransValue(Phase))
    MPM.addPass(PrintModulePass(
        dbgs(),
        "; Module Before " + std::string(DumpModuleDTransNames[Phase]) + "\n"));

  MPM.addPass(P);

  if (hasDumpModuleAfterDTransValue(Phase))
    MPM.addPass(PrintModulePass(
        dbgs(),
        "; Module After " + std::string(DumpModuleDTransNames[Phase]) + "\n"));
}

void llvm::addDTransPasses(ModulePassManager &MPM) {
  if (hasDumpModuleBeforeDTransValue(early))
    MPM.addPass(PrintModulePass(dbgs(), "; Module Before Early DTrans\n"));

  // This must run before any other pass that depends on DTransAnalysis.
  if (EnableResolveTypes)
    addPass(MPM, resolvetypes, dtrans::ResolveTypesPass());

  if (EnableTranspose)
    addPass(MPM, transpose, dtrans::TransposePass());
  if (EnableSOAToAOSPrepare)
    addPass(MPM, soatoaosprepare, dtrans::SOAToAOSPreparePass());
  if (EnableSOAToAOS)
    addPass(MPM, soatoaos, dtrans::SOAToAOSPass());
  addPass(MPM, weakalign, dtrans::WeakAlignPass());
  if (EnableDeleteFields)
    addPass(MPM, deletefield, dtrans::DeleteFieldPass());
  if (EnableMemInitTrimDown)
    addPass(MPM, meminittrimdown, dtrans::MemInitTrimDownPass());
  addPass(MPM, reorderfields, dtrans::ReorderFieldsPass());
  addPass(MPM, aostosoa, dtrans::AOSToSOAPass());
  addPass(MPM, elimrofieldaccess, dtrans::EliminateROFieldAccessPass());
  addPass(MPM, dynclone, dtrans::DynClonePass());
  addPass(MPM, annotatorcleaner, dtrans::AnnotatorCleanerPass());

  if (hasDumpModuleAfterDTransValue(early))
    MPM.addPass(PrintModulePass(dbgs(), "; Module After Early DTrans\n"));
}

// Add a legacy pass manager type pass. Add module dumps before/after
// the pass, if requested.
static void addPass(legacy::PassManagerBase &PM, DumpModuleDTransValues Phase,
                    Pass *P) {
  assert(Phase <= late && "Phase value out of range");
  if (hasDumpModuleBeforeDTransValue(Phase))
    PM.add(createPrintModulePass(
        dbgs(),
        "; Module Before " + std::string(DumpModuleDTransNames[Phase]) + "\n"));

  PM.add(P);

  if (hasDumpModuleAfterDTransValue(Phase))
    PM.add(createPrintModulePass(
        dbgs(),
        "; Module After " + std::string(DumpModuleDTransNames[Phase]) + "\n"));
}

void llvm::addDTransLegacyPasses(legacy::PassManagerBase &PM) {
  if (hasDumpModuleBeforeDTransValue(early))
    PM.add(createPrintModulePass(dbgs(), "; Module Before Early DTrans\n"));

  // This must run before any other pass that depends on DTransAnalysis.
  if (EnableResolveTypes)
    addPass(PM, resolvetypes, createDTransResolveTypesWrapperPass());

  if (EnableTranspose)
    addPass(PM, transpose, createDTransTransposeWrapperPass());
  if (EnableSOAToAOSPrepare)
    addPass(PM, soatoaosprepare, createDTransSOAToAOSPrepareWrapperPass());
  if (EnableSOAToAOS)
    addPass(PM, soatoaos, createDTransSOAToAOSWrapperPass());
  addPass(PM, weakalign, createDTransWeakAlignWrapperPass());
  if (EnableDeleteFields)
    addPass(PM, deletefield, createDTransDeleteFieldWrapperPass());
  if (EnableMemInitTrimDown)
    addPass(PM, meminittrimdown, createDTransMemInitTrimDownWrapperPass());
  addPass(PM, reorderfields, createDTransReorderFieldsWrapperPass());
  addPass(PM, aostosoa, createDTransAOSToSOAWrapperPass());
  addPass(PM, elimrofieldaccess,
          createDTransEliminateROFieldAccessWrapperPass());
  addPass(PM, dynclone, createDTransDynCloneWrapperPass());
  addPass(PM, annotatorcleaner, createDTransAnnotatorCleanerWrapperPass());

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
  (void)llvm::createDTransSOAToAOSPrepareWrapperPass();
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
