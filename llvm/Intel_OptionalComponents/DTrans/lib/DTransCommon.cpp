//===----------------- DTransCommon.cpp - Shared DTrans code --------------===//
//
// Copyright (C) 2018-2022 Intel Corporation. All rights reserved.
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
#include "Intel_DTrans/DTransPasses.h"
#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Analysis/DTransImmutableAnalysis.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/CommandLine.h"
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

// Initial Memory management transformation.
static cl::opt<bool> EnableMemManageTrans("enable-dtrans-memmanagetrans",
                                  cl::init(true), cl::Hidden,
                                  cl::desc("Enable DTrans MemoryManageTrans"));

// SOA-to-AOS Prepare transformation.
static cl::opt<bool> EnableSOAToAOSPrepare("enable-dtrans-soatoaos-prepare",
                                    cl::init(true), cl::Hidden,
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
// Reuse fields transformation.
static cl::opt<bool> EnableReuseFields("enable-dtrans-reusefield",
                                       cl::init(true), cl::Hidden,
                                       cl::desc("Enable DTrans reuse field"));
// Valid values for the dump-module-after-dtrans and dump-module-before-dtrans
// options:
//   early -> dump before/after early DTrans passes
//   late -> dump before/after late DTrans passes
//   <passname> -> dump before/after specific pass
enum DumpModuleDTransValues {
  early,
  deadmdremover,
  normalize,
  resolvetypes,
  transpose,
  commutecond,
  soatoaosprepare,
  soatoaos,
  memmanagetrans,
  codealign,
  weakalign,
  reusefield,
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
                                              "DeadMDRemover",
                                              "Normalize",
                                              "ResolveTypes",
                                              "Transpose",
                                              "CommuteCond",
                                              "SOAToAOSPrepare",
                                              "SOAToAOS",
                                              "MemManageTrans",
                                              "CodeAlign",
                                              "WeakAlign",
                                              "ReuseField",
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
        clEnumVal(deadmdremover,
                  "Dump LLVM Module before removing dead DTrans MD"),
        clEnumVal(normalize, "Dump LLVM Module before normalizing for DTrans"),
        clEnumVal(resolvetypes, "Dump LLVM Module before ResolveTypes pass"),
        clEnumVal(transpose, "Dump LLVM Module before Transpose pass"),
        clEnumVal(commutecond,
                  "Dump LLVM Module before CommuteCond pass"),
        clEnumVal(soatoaosprepare,
                  "Dump LLVM Module before SOA-to-AOS Prepare pass"),
        clEnumVal(soatoaos, "Dump LLVM Module before SOA-to-AOS pass"),
        clEnumVal(memmanagetrans,
                  "Dump LLVM Module before MemManageTrans pass"),
        clEnumVal(codealign, "Dump LLVM Module before CodeAlign pass"),
        clEnumVal(weakalign, "Dump LLVM Module before WeakAlign pass"),
        clEnumVal(reusefield, "Dump LLVM Module before ReuseField pass"),
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
        clEnumVal(deadmdremover,
                  "Dump LLVM Module after removing dead DTrans MD"),
        clEnumVal(normalize, "Dump LLVM Module after normalizing for DTrans"),
        clEnumVal(resolvetypes, "Dump LLVM Module after ResolveTypes pass"),
        clEnumVal(transpose, "Dump LLVM Module after Transpose pass"),
        clEnumVal(commutecond,
                  "Dump LLVM Module after CommuteCond pass"),
        clEnumVal(soatoaosprepare,
                  "Dump LLVM Module after SOA-to-AOS Prepare pass"),
        clEnumVal(soatoaos, "Dump LLVM Module after SOA-to-AOS pass"),
        clEnumVal(memmanagetrans,
                  "Dump LLVM Module after MemManageTrans pass"),
        clEnumVal(codealign, "Dump LLVM Module after CodeAlign pass"),
        clEnumVal(weakalign, "Dump LLVM Module after WeakAlign pass"),
        clEnumVal(reusefield, "Dump LLVM Module after ReuseField pass"),
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
  initializeDTransFieldModRefAnalysisWrapperPass(PR);
  initializeDTransFieldModRefOPAnalysisWrapperPass(PR);
  initializeDTransFieldModRefResultWrapperPass(PR);
  initializeDTransSafetyAnalyzerWrapperPass(PR);
  initializeDTransImmutableAnalysisWrapperPass(PR);
  initializeDTransPaddedMallocWrapperPass(PR);
  initializeDTransPaddedMallocOPWrapperPass(PR);
  initializePaddedPtrPropWrapperPass(PR);
  initializePaddedPtrPropOPWrapperPass(PR);
  initializeDTransResolveTypesWrapperPass(PR);
  initializeDTransSOAToAOSPrepareWrapperPass(PR);
  initializeDTransSOAToAOSOPPrepareWrapperPass(PR);
  initializeDTransSOAToAOSWrapperPass(PR);
  initializeDTransSOAToAOSOPWrapperPass(PR);
  initializeDTransReuseFieldWrapperPass(PR);
  initializeDTransDeleteFieldWrapperPass(PR);
  initializeDTransDeleteFieldOPWrapperPass(PR);
  initializeDTransReorderFieldsWrapperPass(PR);
  initializeDTransReorderFieldsOPWrapperPass(PR);
  initializeDTransAOSToSOAWrapperPass(PR);
  initializeDTransAOSToSOAOPWrapperPass(PR);
  initializeDTransEliminateROFieldAccessWrapperPass(PR);
  initializeDTransEliminateROFieldAccessOPWrapperPass(PR);
  initializeDTransDynCloneWrapperPass(PR);
  initializeDTransDynCloneOPWrapperPass(PR);
  initializeDTransAnnotatorCleanerWrapperPass(PR);
  initializeDTransWeakAlignWrapperPass(PR);
  initializeDTransMemInitTrimDownWrapperPass(PR);
  initializeDTransMemInitTrimDownOPWrapperPass(PR);
  initializeDTransMemManageTransWrapperPass(PR);
  initializeDTransCodeAlignWrapperPass(PR);
  initializeDTransCodeAlignOPWrapperPass(PR);
  initializeDTransTransposeWrapperPass(PR);
  initializeDTransCommuteCondWrapperPass(PR);
  initializeDTransCommuteCondOPWrapperPass(PR);
  initializeRemoveAllDTransTypeMetadataWrapperPass(PR);
  initializeRemoveDeadDTransTypeMetadataWrapperPass(PR);
  initializeDTransForceInlineWrapperPass(PR);
  initializeDTransForceInlineOPWrapperPass(PR);
  initializeDTransNormalizeOPWrapperPass(PR);

#if !INTEL_PRODUCT_RELEASE
  initializeDTransOPOptBaseTestWrapperPass(PR);
  initializeDTransOptBaseTestWrapperPass(PR);
  initializeDTransTypeMetadataReaderTestWrapperPass(PR);
  initializeDTransPtrTypeAnalyzerTestWrapperPass(PR);
#endif // !INTEL_PRODUCT_RELEASE
}

// Add a new pass manager type pass. Add module dumps before/after
// the pass, if requested.
template <typename PassT>
static void addPass(ModulePassManager &MPM, DumpModuleDTransValues Phase,
                    PassT &&P) {
  assert(Phase <= late && "Phase value out of range");
  if (hasDumpModuleBeforeDTransValue(Phase))
    MPM.addPass(PrintModulePass(
        dbgs(),
        "; Module Before " + std::string(DumpModuleDTransNames[Phase]) + "\n"));

  MPM.addPass(std::forward<PassT>(P));

  if (hasDumpModuleAfterDTransValue(Phase))
    MPM.addPass(PrintModulePass(
        dbgs(),
        "; Module After " + std::string(DumpModuleDTransNames[Phase]) + "\n"));
}

void llvm::addDTransPasses(ModulePassManager &MPM) {
  if (hasDumpModuleBeforeDTransValue(early))
    MPM.addPass(PrintModulePass(dbgs(), "; Module Before Early DTrans\n"));

  // Try to run the typed-pointer passes. These passes should be no-ops when
  // opaque pointers are in use.
  // TODO: These will be removed when only opaque pointers are supported.

  // Start of typed pointer passes
  // This must run before any other pass that depends on DTransAnalysis.
  if (EnableResolveTypes)
    addPass(MPM, resolvetypes, dtrans::ResolveTypesPass());

  if (EnableTranspose)
    addPass(MPM, transpose, dtrans::TransposePass());
  addPass(MPM, commutecond, dtrans::CommuteCondPass());
  if (EnableMemInitTrimDown)
    addPass(MPM, meminittrimdown, dtrans::MemInitTrimDownPass());
  if (EnableSOAToAOSPrepare)
    addPass(MPM, soatoaosprepare, dtrans::SOAToAOSPreparePass());
  if (EnableSOAToAOS)
    addPass(MPM, soatoaos, dtrans::SOAToAOSPass());
  if (EnableMemManageTrans)
    addPass(MPM, memmanagetrans, dtrans::MemManageTransPass());
  addPass(MPM, codealign, dtrans::CodeAlignPass());
  addPass(MPM, weakalign, dtrans::WeakAlignPass());
  if (EnableDeleteFields)
    addPass(MPM, deletefield, dtrans::DeleteFieldPass());
  addPass(MPM, reorderfields, dtrans::ReorderFieldsPass());
  addPass(MPM, aostosoa, dtrans::AOSToSOAPass());
  if (EnableReuseFields) {
    addPass(MPM, reusefield, dtrans::ReuseFieldPass());
    addPass(MPM, deletefield, dtrans::DeleteFieldPass());
  }
  addPass(MPM, elimrofieldaccess, dtrans::EliminateROFieldAccessPass());
  addPass(MPM, dynclone, dtrans::DynClonePass());
  // End of typed pointer passes

  // Try to run the opaque pointer passes
  addPass(MPM, deadmdremover, dtransOP::RemoveDeadDTransTypeMetadataPass());
  addPass(MPM, normalize, dtransOP::DTransNormalizeOPPass());
  addPass(MPM, commutecond, dtransOP::CommuteCondOPPass());
  addPass(MPM, meminittrimdown, dtransOP::MemInitTrimDownOPPass());
  addPass(MPM, soatoaosprepare, dtransOP::SOAToAOSOPPreparePass());
  if (EnableMemManageTrans)
    addPass(MPM, memmanagetrans, dtransOP::MemManageTransOPPass());
  addPass(MPM, codealign, dtransOP::CodeAlignPass());
  addPass(MPM, deletefield, dtransOP::DeleteFieldOPPass());
  addPass(MPM, reorderfields, dtransOP::ReorderFieldsOPPass());
  addPass(MPM, aostosoa, dtransOP::AOSToSOAOPPass());

  if (EnableReuseFields) {
    addPass(MPM, reusefield, dtransOP::ReuseFieldOPPass());
    addPass(MPM, deletefield, dtransOP::DeleteFieldOPPass());
  }

  addPass(MPM, elimrofieldaccess, dtransOP::EliminateROFieldAccessPass());
  addPass(MPM, dynclone, dtransOP::DynClonePass());

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

  // Try to run the typed-pointer passes. These passes should be no-ops when
  // opaque pointers are in use.
  // TODO: These will be removed when only opaque pointers are supported.

  // Start of typed pointer passes
  // This must run before any other pass that depends on DTransAnalysis.
  if (EnableResolveTypes)
    addPass(PM, resolvetypes, createDTransResolveTypesWrapperPass());

  if (EnableTranspose)
    addPass(PM, transpose, createDTransTransposeWrapperPass());
  addPass(PM, commutecond, createDTransCommuteCondWrapperPass());
  if (EnableMemInitTrimDown)
    addPass(PM, meminittrimdown, createDTransMemInitTrimDownWrapperPass());
  if (EnableSOAToAOSPrepare)
    addPass(PM, soatoaosprepare, createDTransSOAToAOSPrepareWrapperPass());
  if (EnableSOAToAOS)
    addPass(PM, soatoaos, createDTransSOAToAOSWrapperPass());
  if (EnableMemManageTrans)
    addPass(PM, memmanagetrans, createDTransMemManageTransWrapperPass());
  addPass(PM, codealign, createDTransCodeAlignWrapperPass());
  addPass(PM, weakalign, createDTransWeakAlignWrapperPass());
  if (EnableDeleteFields)
    addPass(PM, deletefield, createDTransDeleteFieldWrapperPass());
  addPass(PM, reorderfields, createDTransReorderFieldsWrapperPass());
  addPass(PM, aostosoa, createDTransAOSToSOAWrapperPass());
  if (EnableReuseFields) {
    addPass(PM, reusefield, createDTransReuseFieldWrapperPass());
    addPass(PM, deletefield, createDTransDeleteFieldWrapperPass());
  }

  addPass(PM, elimrofieldaccess,
          createDTransEliminateROFieldAccessWrapperPass());
  addPass(PM, dynclone, createDTransDynCloneWrapperPass());
  // End of typed pointer passes

  // Try to run the opaque pointer passes

  addPass(PM, deadmdremover, createRemoveDeadDTransTypeMetadataWrapperPass());
  addPass(PM, normalize, createDTransNormalizeOPWrapperPass());
  addPass(PM, commutecond, createDTransCommuteCondOPWrapperPass());
  addPass(PM, meminittrimdown, createDTransMemInitTrimDownOPWrapperPass());
  addPass(PM, soatoaosprepare, createDTransSOAToAOSOPPrepareWrapperPass());
  addPass(PM, codealign, createDTransCodeAlignOPWrapperPass());
  addPass(PM, deletefield, createDTransDeleteFieldOPWrapperPass());
  addPass(PM, reorderfields, createDTransReorderFieldsOPWrapperPass());
  addPass(PM, aostosoa, createDTransAOSToSOAOPWrapperPass());
  addPass(PM, elimrofieldaccess,
          createDTransEliminateROFieldAccessOPWrapperPass());
  addPass(PM, dynclone, createDTransDynCloneOPWrapperPass());

  addPass(PM, annotatorcleaner, createDTransAnnotatorCleanerWrapperPass());
  if (hasDumpModuleAfterDTransValue(early))
    PM.add(createPrintModulePass(dbgs(), "; Module After Early DTrans\n"));
}

void llvm::addLateDTransPasses(ModulePassManager &MPM) {
  if (hasDumpModuleBeforeDTransValue(late))
    MPM.addPass(PrintModulePass(dbgs(), "; Module Before Late DTrans\n"));

  // Try to run the typed-pointer passes. These passes should be no-ops when
  // opaque pointers are in use.
  // TODO: These will be removed when only opaque pointers are supported.
  // Start of typed pointer passes
  if (EnablePaddedPtrProp)
    MPM.addPass(dtrans::PaddedPtrPropPass());
  MPM.addPass(dtrans::PaddedMallocPass());
  // End of typed pointer passes

  // Try to run the opaque pointer passes
  if (EnablePaddedPtrProp)
    MPM.addPass(dtransOP::PaddedPtrPropOPPass());
  MPM.addPass(dtransOP::PaddedMallocOPPass());

  if (hasDumpModuleAfterDTransValue(late))
    MPM.addPass(PrintModulePass(dbgs(), "; Module After Late DTrans\n"));
}

void llvm::addLateDTransLegacyPasses(legacy::PassManagerBase &PM) {
  if (hasDumpModuleBeforeDTransValue(late))
    PM.add(createPrintModulePass(dbgs(), "; Module Before Late DTrans\n"));

  // Try to run the typed-pointer passes. These passes should be no-ops when
  // opaque pointers are in use.
  // TODO: These will be removed when only opaque pointers are supported.
  // Start of typed pointer passes
  if (EnablePaddedPtrProp)
    PM.add(createPaddedPtrPropWrapperPass());
  PM.add(createDTransPaddedMallocWrapperPass());
  // End of typed pointer passes

  // Try to run the opaque pointer passes
  if (EnablePaddedPtrProp)
    PM.add(createPaddedPtrPropOPWrapperPass());
  PM.add(createDTransPaddedMallocOPWrapperPass());

  if (hasDumpModuleAfterDTransValue(late))
    PM.add(createPrintModulePass(dbgs(), "; Module After Late DTrans\n"));
}

// This is used by LinkAllPasses.h. The passes are never actually used when
// created this way.
void llvm::createDTransPasses() {
  (void)llvm::createDTransReuseFieldWrapperPass();
  (void)llvm::createDTransDeleteFieldWrapperPass();
  (void)llvm::createDTransDeleteFieldOPWrapperPass();
  (void)llvm::createDTransAOSToSOAWrapperPass();
  (void)llvm::createDTransAOSToSOAOPWrapperPass();
  (void)llvm::createDTransAnnotatorCleanerWrapperPass();
  (void)llvm::createDTransReorderFieldsWrapperPass();
  (void)llvm::createDTransReorderFieldsOPWrapperPass();
  (void)llvm::createDTransPaddedMallocWrapperPass();
  (void)llvm::createDTransPaddedMallocOPWrapperPass();
  (void)llvm::createDTransEliminateROFieldAccessWrapperPass();
  (void)llvm::createDTransEliminateROFieldAccessOPWrapperPass();
  (void)llvm::createPaddedPtrPropWrapperPass();
  (void)llvm::createPaddedPtrPropOPWrapperPass();
  (void)llvm::createDTransSOAToAOSPrepareWrapperPass();
  (void)llvm::createDTransSOAToAOSOPPrepareWrapperPass();
  (void)llvm::createDTransSOAToAOSWrapperPass();
  (void)llvm::createDTransSOAToAOSOPWrapperPass();
  (void)llvm::createDTransAnalysisWrapperPass();
  (void)llvm::createDTransImmutableAnalysisWrapperPass();
  (void)llvm::createDTransSafetyAnalyzerTestWrapperPass();
  (void)llvm::createDTransFieldModRefAnalysisWrapperPass();
  (void)llvm::createDTransFieldModRefOPAnalysisWrapperPass();
  (void)llvm::createDTransFieldModRefResultWrapperPass();
  (void)llvm::createDTransDynCloneWrapperPass();
  (void)llvm::createDTransDynCloneOPWrapperPass();
  (void)llvm::createDTransWeakAlignWrapperPass();
  (void)llvm::createDTransMemInitTrimDownWrapperPass();
  (void)llvm::createDTransMemManageTransWrapperPass();
  (void)llvm::createDTransCodeAlignWrapperPass();
  (void)llvm::createDTransCodeAlignOPWrapperPass();
  (void)llvm::createDTransTransposeWrapperPass();
  (void)llvm::createDTransCommuteCondWrapperPass();
  (void)llvm::createDTransCommuteCondOPWrapperPass();
  (void)llvm::createRemoveAllDTransTypeMetadataWrapperPass();
  (void)llvm::createRemoveDeadDTransTypeMetadataWrapperPass();
  (void)llvm::createDTransNormalizeOPWrapperPass();

#if !INTEL_PRODUCT_RELEASE
  (void)llvm::createDTransOptBaseTestWrapperPass();
  (void)llvm::createDTransMetadataReaderTestWrapperPass();
  (void)llvm::createDTransPtrTypeAnalyzerTestWrapperPass();
#endif // !INTEL_PRODUCT_RELEASE
}
