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
#include "Intel_DTrans/Analysis/DTransImmutableAnalysis.h"
#include "Intel_DTrans/DTransPasses.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IRPrinter/IRPrintingPasses.h"
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
static cl::opt<bool>
    EnableMemManageTrans("enable-dtrans-memmanagetrans", cl::init(false),
                         cl::Hidden,
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

#else

#define hasDumpModuleBeforeDTransValue(x) (false)
#define hasDumpModuleAfterDTransValue(x) (false)
constexpr bool EnablePaddedPtrProp = true;

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

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

#if INTEL_PRODUCT_RELEASE
  // Set the "Intel Proprietary" module flag when any DTrans passes are run
  MPM.addPass(dtrans::SetIntelPropPass());
#endif // INTEL_PRODUCT_RELEASE

  if (EnableTranspose)
    addPass(MPM, transpose, dtrans::TransposePass());
  addPass(MPM, deadmdremover, dtransOP::RemoveDeadDTransTypeMetadataPass());
  addPass(MPM, normalize, dtransOP::DTransNormalizeOPPass());
  addPass(MPM, commutecond, dtransOP::CommuteCondOPPass());
  if (EnableMemInitTrimDown)
    addPass(MPM, meminittrimdown, dtransOP::MemInitTrimDownOPPass());
  if (EnableSOAToAOSPrepare)
    addPass(MPM, soatoaosprepare, dtransOP::SOAToAOSOPPreparePass());
  if (EnableSOAToAOS)
    addPass(MPM, soatoaos, dtransOP::SOAToAOSOPPass());
  if (EnableMemManageTrans)
    addPass(MPM, memmanagetrans, dtransOP::MemManageTransOPPass());
  addPass(MPM, codealign, dtransOP::CodeAlignPass());
  addPass(MPM, weakalign, dtrans::WeakAlignPass());
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

void llvm::addLateDTransPasses(ModulePassManager &MPM) {
  if (hasDumpModuleBeforeDTransValue(late))
    MPM.addPass(PrintModulePass(dbgs(), "; Module Before Late DTrans\n"));

  if (EnablePaddedPtrProp)
    MPM.addPass(dtransOP::PaddedPtrPropOPPass());
  MPM.addPass(dtransOP::PaddedMallocOPPass());

  if (hasDumpModuleAfterDTransValue(late))
    MPM.addPass(PrintModulePass(dbgs(), "; Module After Late DTrans\n"));
}
