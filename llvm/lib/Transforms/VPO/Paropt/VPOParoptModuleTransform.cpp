#if INTEL_COLLAB
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===--- VPOParoptModuleTranform.cpp - Paropt Module Transforms --- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Authors:
// --------
// Xinmin Tian (xinmin.tian@intel.com)
//
// Major Revisions:
// ----------------
// Dec 2015: Initial Implementation of MT-code generation (Xinmin Tian)
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the ParOpt interface to perform module transformations
/// for OpenMP and Auto-parallelization
///
//===----------------------------------------------------------------------===//

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/GlobalStatus.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptModuleTransform.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTpv.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Transforms/Utils/InferAddressSpacesUtils.h"
#endif // INTEL_CUSTOMIZATION

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "VPOParopt"

#ifndef NDEBUG
static cl::opt<bool> VerifyIRAfterParopt(
    "vpo-paropt-verify-ir-after", cl::Hidden, cl::init(true),
    cl::desc("Enable IR verification after Paropt."));
#endif  // NDEBUG

static constexpr char LLVM_INTRIN_PREF0[] = "llvm.";

static cl::opt<bool> PreserveDeviceIntrin(
  "vpo-paropt-preserve-llvm-intrin", cl::Hidden, cl::init(false),
  cl::desc("Preserve LLVM intrinsics for device SIMD code generation"));

unsigned llvm::vpo::SpirvOffloadEntryAddSpace;
static cl::opt<unsigned, true> SpirvOffloadEntryAddSpaceOpt(
    "vpo-paropt-spirv-offload-entry-addrspace",
    cl::desc("Address space for offload entries on SPIR-V target"), cl::Hidden,
    cl::location(SpirvOffloadEntryAddSpace),
    cl::init(vpo::ADDRESS_SPACE_GLOBAL));

// This table is used to change math function names (left column) to the
// OCL builtin format (right column).
//
// Note that fabs, ceil and floor have corresponding LLVM intrinsics
// that have the same behavior (e.g. regarding setting errno), so
// clang will represent them with intrinsic calls, which implies
// llvm.*.ty mangling.  The intrinsics will be generated only for
// C; for C++ calls to fabs[f], etc. will be generated as-is.
std::unordered_map<std::string, std::string> llvm::vpo::OCLBuiltin = {
/////////////////////////////////////////////
//                 FLOAT                   //
/////////////////////////////////////////////
//  Basic functions
    {"_ZSt3absf",             "_Z16__spirv_ocl_fabsf"},
    {"_ZSt4fabsf",            "_Z16__spirv_ocl_fabsf"},

    {"_ZSt4fmodff",           "_Z16__spirv_ocl_fmodff"},

    {"_ZSt9remainderff",      "_Z21__spirv_ocl_remainderff"},

    {"_ZSt6remquoffPi",       "_Z18__spirv_ocl_remquoffPi"},

    {"llvm.fma.f32",          "_Z15__spirv_ocl_fmafff"},
    {"_ZSt3fmafff",           "_Z15__spirv_ocl_fmafff"},

    {"llvm.maxnum.f32",       "_Z16__spirv_ocl_fmaxff"},
    {"_ZSt4fmaxff",           "_Z16__spirv_ocl_fmaxff"},

    {"llvm.minnum.f32",       "_Z16__spirv_ocl_fminff"},
    {"_ZSt4fminff",           "_Z16__spirv_ocl_fminff"},

    {"_ZSt4fdimff",           "_Z16__spirv_ocl_fdimff"},

//  Exponential functions
    {"llvm.exp.f32",          "_Z15__spirv_ocl_expf"},
    {"_ZSt3expf",             "_Z15__spirv_ocl_expf"},

    {"llvm.exp2.f32",         "_Z16__spirv_ocl_exp2f"},
    {"_ZSt4exp2f",            "_Z16__spirv_ocl_exp2f"},

    {"_ZSt5expm1f",           "_Z17__spirv_ocl_expm1f"},

    {"llvm.log.f32",          "_Z15__spirv_ocl_logf"},
    {"_ZSt3logf",             "_Z15__spirv_ocl_logf"},

    {"llvm.log2.f32",         "_Z16__spirv_ocl_log2f"},
    {"_ZSt4log2f",            "_Z16__spirv_ocl_log2f"},

    {"llvm.log10.f32",        "_Z17__spirv_ocl_log10f"},
    {"_ZSt5log10f",           "_Z17__spirv_ocl_log10f"},

    {"_ZSt5log1pf",           "_Z17__spirv_ocl_log1pf"},

//  Power functions
    {"llvm.pow.f32",          "_Z15__spirv_ocl_powff"},
    {"_ZSt3powff",            "_Z15__spirv_ocl_powff"},

    {"llvm.sqrt.f32",         "_Z16__spirv_ocl_sqrtf"},
    {"_ZSt4sqrtf",            "_Z16__spirv_ocl_sqrtf"},

    {"_ZSt4cbrtf",            "_Z16__spirv_ocl_cbrtf"},
#ifdef _WIN32
    {"hypotf",                "_Z17__spirv_ocl_hypotff"},
#endif // _WIN32
    {"_ZSt5hypotff",          "_Z17__spirv_ocl_hypotff"},

    {"invsqrtf",              "_Z17__spirv_ocl_rsqrtf"},   // from mathimf.h

//  Trig & hyperbolic functions
    {"llvm.sin.f32",          "_Z15__spirv_ocl_sinf"},
    {"_ZSt3sinf",             "_Z15__spirv_ocl_sinf"},

    {"_ZSt4asinf",            "_Z16__spirv_ocl_asinf"},

    {"_ZSt5asinhf",           "_Z17__spirv_ocl_asinhf"},

    {"_ZSt4sinhf",            "_Z16__spirv_ocl_sinhf"},

    {"llvm.cos.f32",          "_Z15__spirv_ocl_cosf"},
    {"_ZSt3cosf",             "_Z15__spirv_ocl_cosf"},

    {"_ZSt4acosf",            "_Z16__spirv_ocl_acosf"},

    {"_ZSt5acoshf",           "_Z17__spirv_ocl_acoshf"},

    {"_ZSt4coshf",            "_Z16__spirv_ocl_coshf"},

    {"_ZSt3tanf",             "_Z15__spirv_ocl_tanf"},

    {"_ZSt4atanf",            "_Z16__spirv_ocl_atanf"},

    {"_ZSt5atanhf",           "_Z17__spirv_ocl_atanhf"},

    {"_ZSt4tanhf",            "_Z16__spirv_ocl_tanhf"},

    {"_ZSt5atan2ff",          "_Z17__spirv_ocl_atan2ff"},

//  Error & gamma functions
    {"_ZSt3erff",             "_Z15__spirv_ocl_erff"},

    {"_ZSt4erfcf",            "_Z16__spirv_ocl_erfcf"},

    {"_ZSt6tgammaf",          "_Z18__spirv_ocl_tgammaf"},

    {"_ZSt6lgammaf",          "_Z18__spirv_ocl_lgammaf"},

//  Rounding functions
    {"_ZSt4ceilf",            "_Z16__spirv_ocl_ceilf"},

    {"llvm.floor.f32",        "_Z17__spirv_ocl_floorf"},
    {"_ZSt5floorf",           "_Z17__spirv_ocl_floorf"},

    {"llvm.trunc.f32",        "_Z17__spirv_ocl_truncf"},
    {"_ZSt5truncf",           "_Z17__spirv_ocl_truncf"},

    {"llvm.round.f32",        "_Z17__spirv_ocl_roundf"},
    {"_ZSt5roundf",           "_Z17__spirv_ocl_roundf"},

//  Floating-point manipulation functions
#ifdef _WIN32
    {"frexpf",                "_Z17__spirv_ocl_frexpfPi"},
#endif // _WIN32
    {"_ZSt5frexpfPi",         "_Z17__spirv_ocl_frexpfPi"},

#ifdef _WIN32
    {"ldexpf",                "_Z17__spirv_ocl_ldexpfi"},
#endif // _WIN32
    {"_ZSt5ldexpfi",          "_Z17__spirv_ocl_ldexpfi"},

    {"_ZSt4modffPf",          "_Z16__spirv_ocl_modffPf"},

    {"_ZSt5ilogbf",           "_Z17__spirv_ocl_ilogbf"},

    {"_ZSt4logbf",            "_Z16__spirv_ocl_logbf"},

    {"_ZSt9nextafterff",      "_Z21__spirv_ocl_nextafterff"},

    {"llvm.copysign.f32",     "_Z20__spirv_ocl_copysignff"},
    {"_ZSt8copysignff",       "_Z20__spirv_ocl_copysignff"},

/////////////////////////////////////////////
//                 DOUBLE                  //
/////////////////////////////////////////////
//  Basic functions
    {"_ZSt3absd",             "_Z16__spirv_ocl_fabsd"},
    {"llvm.fma.f64",          "_Z15__spirv_ocl_fmaddd"},
    {"llvm.maxnum.f64",       "_Z16__spirv_ocl_fmaxdd"},
    {"llvm.minnum.f64",       "_Z16__spirv_ocl_fmindd"},

//  Exponential functions
    {"llvm.exp.f64",          "_Z15__spirv_ocl_expd"},
    {"llvm.exp2.f64",         "_Z16__spirv_ocl_exp2d"},
    {"llvm.log.f64",          "_Z15__spirv_ocl_logd"},
    {"llvm.log2.f64",         "_Z16__spirv_ocl_log2d"},
    {"llvm.log10.f64",        "_Z17__spirv_ocl_log10d"},

//  Power functions
    {"llvm.pow.f64",          "_Z15__spirv_ocl_powdd"},
    {"llvm.sqrt.f64",         "_Z16__spirv_ocl_sqrtd"},
    {"invsqrt",               "_Z17__spirv_ocl_rsqrtd"},   // from mathimf.h

//  Trig & hyperbolic functions
    {"llvm.sin.f64",          "_Z15__spirv_ocl_sind"},
    {"llvm.cos.f64",          "_Z15__spirv_ocl_cosd"},

//  Rounding functions
    {"llvm.ceil.f64",         "_Z16__spirv_ocl_ceild"},
    {"llvm.floor.f64",        "_Z17__spirv_ocl_floord"},
    {"llvm.trunc.f64",        "_Z17__spirv_ocl_truncd"},
    {"llvm.round.f64",        "_Z17__spirv_ocl_roundd"},

//  Floating-point manipulation functions
    {"llvm.copysign.f64",     "_Z20__spirv_ocl_copysigndd"},

/////////////////////////////////////////////
//                INTEGER                  //
/////////////////////////////////////////////
    {"abs",                   "_Z17__spirv_ocl_s_absi"},      // int abs(int)
    {"labs",                  "_Z17__spirv_ocl_s_absl"}};     // long labs(long)

// To support the SPIRV target compilation stage of the OpenMP compilation
// offloading to GPUs, we must translate the name of math functions (left
// column in OCLBuiltin) to their OCL builtin counterparts.
static bool replaceMathFnWithOCLBuiltin(Function &F) {
  bool Changed = false;
  StringRef OldName = F.getName();
  auto Map = OCLBuiltin.find(std::string(OldName));

  if (Map != OCLBuiltin.end()) {
    StringRef NewName = Map->second;
    if (!PreserveDeviceIntrin ||
        !OldName.consume_front(LLVM_INTRIN_PREF0)) {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Replacing " << OldName << " with "
                        << NewName << '\n');
      F.setName(NewName);
      Changed = true;
    }
  }
  return Changed;
}

// Given the original printf() declaration \p F coming in from clang:
//
//   declare dso_local spir_func i32 @printf(i8 addrspace(4)*, ...)
//
// Create the corresponding decl for OCL printf called from offload kernels:
//
//   declare dso_local spir_func i32
//     @_Z18__spirv_ocl_printfPU3AS2ci(i8 addrspace(2)*, ...)
//
// Save the former in \b PrintfDecl and the latter in \b OCLPrintfDecl
// in the \b VPOParoptModuleTransform class.
void VPOParoptModuleTransform::createOCLPrintfDecl(Function *F) {
  PrintfDecl = F;

  // Create FunctionType for OCLPrintfDecl
  Type *ReturnTy = Type::getInt32Ty(C);
  Type *Int8PtrTy = Type::getInt8PtrTy(C, ADDRESS_SPACE_CONSTANT /*=2*/);
  FunctionType *FnTy = FunctionType::get(ReturnTy, {Int8PtrTy},
                                         /* varargs= */ true);

  // Get the function prototype from the module symbol table.
  // If absent, create and insert it into the symbol table first.
  FunctionCallee FnC =
      M.getOrInsertFunction("_Z18__spirv_ocl_printfPU3AS2ci", FnTy);
  OCLPrintfDecl = cast<Function>(FnC.getCallee());
  OCLPrintfDecl->copyAttributesFrom(PrintfDecl);

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ":\nOld printf decl: " << *PrintfDecl
                    << "\nOCL printf decl: " << *OCLPrintfDecl << "\n");
}

// Given the original sincosf()/sincos() declaration \p F coming in from clang:
//
//   declare dso_local spir_func
//           void @sincosf(float, float addrspace(4)*, float addrspace(4)*)
//   declare dso_local spir_func
//           void @sincos(double, double addrspace(4)*, double addrspace(4)*)
//
// Create the corresponding decl for OCL sincos() called from offload kernels:
//
//   declare dso_local spir_func
//           float @_Z18__spirv_ocl_sincosfPf(float, float addrspace(4)*)
//   declare dso_local spir_func
//           double @_Z18__spirv_ocl_sincosdPd(double, double addrspace(4)*)
//
// Then replace all sincos and sincosf calls as follows:
//
//   old:
//        sincos(Opnd, SineVar, CosineVar)       // or sincosf
//   new:
//        %sine = _Z18__spirv_ocl_sincosdPd/fPf(opnd, CosineVar)
//        store %sine, SineVar
void VPOParoptModuleTransform::replaceSincosWithOCLBuiltin(Function *F,
                                                           bool IsDouble) {

  // First, create the function declaration for the OCL built-in versions

  Function *SincosDecl = F; // decl for sincos or sincosf
  Function *OCLSincosDecl;  // decl for _Z6sincosdPd or _Z6sincosfPf

  // Create the FunctionType
  Type *FpTy;    // floating-point type; can be double or float
  Type *FpPtrTy; // pointer to double or float
  StringRef NewName;
  if (IsDouble) {
    FpTy = Type::getDoubleTy(C);
    FpPtrTy = Type::getDoublePtrTy(C, ADDRESS_SPACE_GENERIC /*=4*/);
    NewName = "_Z18__spirv_ocl_sincosdPd";
  } else {
    FpTy = Type::getFloatTy(C);
    FpPtrTy = Type::getFloatPtrTy(C, ADDRESS_SPACE_GENERIC /*=4*/);
    NewName = "_Z18__spirv_ocl_sincosfPf";
  }
  FunctionType *FnTy = FunctionType::get(FpTy, {FpTy, FpPtrTy}, false);

  // Get or create the function prototype
  FunctionCallee FnC = M.getOrInsertFunction(NewName, FnTy);

  OCLSincosDecl = cast<Function>(FnC.getCallee());
  OCLSincosDecl->setDSOLocal(true);

  LLVM_DEBUG(dbgs() << __FUNCTION__ << ":\nOld sincos decl: " << *SincosDecl
                    << "\nOCL sincos decl: " << *OCLSincosDecl << "\n");

  // Then, replace all calls to the old function with calls to the OCL built-in
  SmallVector<Instruction *, 10> CallsToRemove;
  for (User *U : SincosDecl->users())
    if (CallInst *OldCall = dyn_cast<CallInst>(U)) {

      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": old sincos: " << *OldCall
                        << "\n");

      // Create the new call based on OCLSincosDecl and
      // insert it before the old call

      SmallVector<Value *, 3> FnArgs(OldCall->args());
      // arg[0]: Opnd;  arg[1]: SineVar;  arg[2]: CosineVar

      CallInst *NewCall = CallInst::Create(
          FnTy, OCLSincosDecl, {FnArgs[0], FnArgs[2]}, "sine", OldCall);
      VPOParoptUtils::setFuncCallingConv(NewCall, NewCall->getModule());

      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": OCL sincos: " << *NewCall
                        << "\n");

      // Store the return value into SineVar (which is FnArgs[1])
      StoreInst *StoreSine = new StoreInst(NewCall, FnArgs[1], OldCall);

      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Store for sine: " << *StoreSine
                        << "\n");
      (void) StoreSine;

      CallsToRemove.push_back(OldCall);
    }

  // Remove the old calls.
  for (Instruction *Inst : CallsToRemove)
    Inst->eraseFromParent();
}

// Perform paropt transformations for the module. Each module's function is
// transformed by a separate VPOParoptTransform instance which performs
// paropt transformations on a function level. Then, after tranforming all
// functions, create offload initialization code, emit offload entry table, and
// do necessary code cleanup (f.e. remove functions/globals which should not be
// generated for the target).
bool VPOParoptModuleTransform::doParoptTransforms(
    std::function<vpo::WRegionInfo &(Function &F, bool *Changed)>
        WRegionInfoGetter) {

  bool Changed = false;
  bool IsTargetSPIRV = VPOAnalysisUtils::isTargetSPIRV(&M);
  assert((!DisableOffload || !IsTargetSPIRV) &&
         "Compilation for SPIR-V target without -fopenmp-targets?");

  SmallPtrSet<Function *, 16> FuncsWithDummyBranchesRemoved;
  auto removeDummyBranchesFromBeginToEnd = [&WRegionInfoGetter,
                                            &FuncsWithDummyBranchesRemoved](
                                               Function *F) {
    if (FuncsWithDummyBranchesRemoved.count(F))
      return false;

    bool Changed = false;
    WRegionInfo &WI = WRegionInfoGetter(*F, &Changed);
    auto *DT = WI.getDomTree();
    auto *TLI = WI.getTargetLibraryInfo();
    if (VPOUtils::removeBranchesFromBeginToEndDirective(*F, TLI, DT)) {
      FuncsWithDummyBranchesRemoved.insert(F);
      LLVM_DEBUG(
          dbgs() << "=== After removing branches from Begin To End Directive:\n"
                 << *F);
      Changed = true;
    }
    return Changed;
  };

  processDeviceTriples();

  if (!DisableOffload) {
    OffloadEntries = VPOParoptUtils::loadOffloadMetadata(M);
    // This is the last point, where we want to read the offload metadata,
    // so erase it here.
    Changed |= VPOParoptUtils::eraseOffloadMetadata(M);
  }

  if (IsTargetSPIRV)
    // If Function F contains a "target" region, it will be extracted
    // as a SPIR kernel function. If F is also "declare target", then
    // its body will have to stay in IR, but calling the SPIR kernel
    // from it is illegal. Moreover, even if we outline the "target"
    // region as a normal SPIR function, the outlining will happen
    // after all code generation done for the "target" region,
    // which is only correct for "target" region, and invalid
    // for a "declare target" function compiled as if there is no
    // target region inside it. Here we create clones of Functions
    // that contain "target" region and are "declare target" themselves.
    // See details inside cloneDeclareTargetFunctions().
    Changed |= cloneDeclareTargetFunctions(removeDummyBranchesFromBeginToEnd);

  /// As new functions to be added, so we need to prepare the
  /// list of functions we want to work on in advance.
  std::vector<Function *> FnList;

  for (auto F = M.begin(), E = M.end(); F != E; ++F) {
    // TODO: need Front-End to set F->hasOpenMPDirective()
    if (F->isDeclaration()) { // if(!F->hasOpenMPDirective()))
      if (IsTargetSPIRV) {
        auto FuncName = F->getName();
        if (FuncName == "printf") {
          createOCLPrintfDecl(&*F);
          VPOParoptTransform::replacePrintfWithOCLBuiltin(&*F,
                                                          getOCLPrintfDecl());
          Changed = true;
        } else if (FuncName == "sincos") {
          replaceSincosWithOCLBuiltin(&*F, true);  // double sincos
          Changed = true;
        } else if (FuncName == "sincosf") {
          replaceSincosWithOCLBuiltin(&*F, false); // float sincosf
          Changed = true;
        } else
          Changed |= replaceMathFnWithOCLBuiltin(*F);
      }
      continue;
    }
    LLVM_DEBUG(dbgs() << "\n=== VPOParoptPass func: " << F->getName()
                      << " {\n");
    FnList.push_back(&*F);
  }

  // Iterate over all functions which OpenMP directives to perform Paropt
  // transformation and generate MT-code
  for (auto F : FnList) {

    LLVM_DEBUG(dbgs() << "\n=== VPOParoptPass Process func: " << F->getName()
                      << " {\n");

    if (Mode & ParTrans)
      Changed |= removeDummyBranchesFromBeginToEnd(F);

    WRegionInfo &WI = WRegionInfoGetter(*F, &Changed);
    // Walk the W-Region Graph top-down, and create W-Region List
    WI.buildWRGraph();

    if (WI.WRGraphIsEmpty()) {
      LLVM_DEBUG(dbgs() << "\nNo WRegion Candidates for Parallelization \n");
    }

    LLVM_DEBUG(WI.print(dbgs()));

    //
    // Set up a function pass manager so that we can run some cleanup
    // transforms on the LLVM IR after code gen.
    //
    // legacy::FunctionPassManager FPM(&M);

    LLVM_DEBUG(errs() << "VPOParoptPass: ");
    LLVM_DEBUG(errs().write_escaped(F->getName()) << '\n');

    LLVM_DEBUG(dbgs() << "\n=== VPOParoptPass before ParoptTransformer{\n");

    // AUTOPAR | OPENMP | SIMD | OFFLOAD
    VPOParoptTransform VP(
        this, F, &WI, WI.getDomTree(), WI.getLoopInfo(), WI.getSE(),
        WI.getTargetTransformInfo(), WI.getAssumptionCache(),
        WI.getTargetLibraryInfo(), WI.getAliasAnalysis(), Mode,
#if INTEL_CUSTOMIZATION
        ORVerbosity,
#endif // INTEL_CUSTOMIZATION
        WI.getORE(), OptLevel, DisableOffload);
    Changed = Changed | VP.paroptTransforms();

    LLVM_DEBUG(dbgs() << "\n}=== VPOParoptPass after ParoptTransformer\n");

    // Remove calls to directive intrinsics since the LLVM back end does not
    // know how to translate them.
    // VPOUtils::stripDirectives(*F);

    // It is possible that stripDirectives eliminates all instructions in a
    // basic block except for the branch instruction. Use CFG simplify to
    // eliminate them.
    // FPM.add(createCFGSimplificationPass());
    // FPM.run(*F);

    LLVM_DEBUG(dbgs() << "\n}=== VPOParopt end func: " << F->getName() << "\n");
  }

  if ((Mode & OmpPar) && (Mode & ParTrans))
    fixTidAndBidGlobals();

#if INTEL_CUSTOMIZATION
  if (IsTargetSPIRV)
    Changed |= InferAddrSpacesForGlobals(vpo::ADDRESS_SPACE_GENERIC, M);
#endif  // INTEL_CUSTOMIZATION

  if (!DisableOffload) {
    // Emit offload entries table.
    Changed |= genOffloadEntries();

    if (hasOffloadCompilation() && (Mode & ParTrans)) {
      removeTargetUndeclaredGlobals();
      if (IsTargetSPIRV) {
        // Add the metadata to indicate that the module is OpenCL C++ version.
        // enum SourceLanguage {
        //    SourceLanguageUnknown = 0,
        //    SourceLanguageESSL = 1,
        //    SourceLanguageGLSL = 2,
        //    SourceLanguageOpenCL_C = 3,
        //    SourceLanguageOpenCL_CPP = 4,
        //    SourceLanguageHLSL = 5,
        //    SourceLanguageMax = 0x7fffffff,
        // };
        // The compiler has to set the source type as SourceLanguageOpenCL_CPP.
        // Otherwise the spirv code generation will convert mangled
        // function name into OCL builtin function.

        if (!M.getNamedMetadata("spirv.Source")) {
          SmallVector<Metadata *, 8> opSource = {
              ConstantAsMetadata::get(ConstantInt::get(Type::getInt32Ty(C), 4)),
              ConstantAsMetadata::get(
                  ConstantInt::get(Type::getInt32Ty(C), 200000))};
          MDNode *srcMD = MDNode::get(C, opSource);
          M.getOrInsertNamedMetadata("spirv.Source")->addOperand(srcMD);
        }
      }
    }
  }

  // Thread private legacy mode implementation
  if (Mode & OmpTpv) {
    VPOParoptTpvLegacyPass VPTL;
    ModuleAnalysisManager DummyMAM;
    PreservedAnalyses PA = VPTL.run(M, DummyMAM);
    Changed = Changed | !PA.areAllPreserved();
  }

#ifndef NDEBUG
  if (Changed && VerifyIRAfterParopt) {
    bool BrokenDebugInfo = false;

      // Do not verify debug information for the time being.
    if (verifyModule(M, &dbgs(), &BrokenDebugInfo)) {
      LLVM_DEBUG(dbgs() << "ERROR: module verifier found errors "
                 "following VPOParoptModuleTransform:\n" << M << "\n");
      report_fatal_error("Module verifier found errors "
                         "following VPOParoptModuleTransform.  Use -mllvm "
                         "-debug-only=" DEBUG_TYPE " to get more information");
    }

    if (BrokenDebugInfo)
      LLVM_DEBUG(dbgs() << "ERROR: module verifier found debug info errors.\n");
  }
#endif  // NDEBUG

  LLVM_DEBUG(dbgs() << "\n====== End VPO ParoptPass ======\n\n");
  return Changed;
}

// Collect the uses of the given global variable.
void VPOParoptModuleTransform::collectUsesOfGlobals(
    Constant *PtrHolder, SmallVectorImpl<Instruction *> &RewriteIns) {
  for (auto IB = PtrHolder->user_begin(), IE = PtrHolder->user_end(); IB != IE;
       IB++) {
    if (Instruction *User = dyn_cast<Instruction>(*IB))
      RewriteIns.push_back(User);
  }
}

// Transform the use of the tid global into __kmpc_global_thread_num or the
// the use of the first argument of the OMP outlined function. The use of
// bid global is transformed accordingly.
void VPOParoptModuleTransform::fixTidAndBidGlobals() {
  Constant *TidPtrHolder =
      M.getOrInsertGlobal("@tid.addr", Type::getInt32Ty(C));
  SmallVector<Instruction *, 8> RewriteIns;

  collectUsesOfGlobals(TidPtrHolder, RewriteIns);
  processUsesOfGlobals(TidPtrHolder, RewriteIns, true);

  RewriteIns.clear();
  Constant *BidPtrHolder =
      M.getOrInsertGlobal("@bid.addr", Type::getInt32Ty(C));
  collectUsesOfGlobals(BidPtrHolder, RewriteIns);
  processUsesOfGlobals(BidPtrHolder, RewriteIns, false);
}

// The utility to transform the tid/bid global variable.
void VPOParoptModuleTransform::processUsesOfGlobals(
    Constant *PtrHolder, SmallVectorImpl<Instruction *> &RewriteIns,
    bool IsTid) {

  while (!RewriteIns.empty()) {
    Instruction *User = RewriteIns.pop_back_val();

    Function *F = User->getParent()->getParent();
    if (F->getAttributes().hasFnAttr("mt-func")) {
      auto IT = F->arg_begin();
      if (!IsTid)
        IT++;
      User->replaceUsesOfWith(PtrHolder, &*IT);
    } else if (IsTid && F->getAttributes().hasFnAttr("task-mt-func")) {
      BasicBlock *EntryBB = &F->getEntryBlock();
      IRBuilder<> Builder(EntryBB->getFirstNonPHI());
      AllocaInst *TidPtr =
          Builder.CreateAlloca(Type::getInt32Ty(C));
      Builder.CreateStore(&*(F->arg_begin()), TidPtr);
      User->replaceUsesOfWith(PtrHolder, TidPtr);
    } else {
      Type *I32Ty = Type::getInt32Ty(C);
      Align I32Align = F->getParent()->getDataLayout().getABITypeAlign(I32Ty);
      BasicBlock *EntryBB = &F->getEntryBlock();
      Instruction *Tid = nullptr;
      AllocaInst *TidPtr = nullptr;
      if (IsTid)
        Tid = VPOParoptUtils::findKmpcGlobalThreadNumCall(EntryBB);
      if (!Tid) {
        IRBuilder<> Builder(EntryBB->getFirstNonPHI());
        TidPtr = Builder.CreateAlloca(I32Ty);
        if (IsTid) {
          Tid = VPOParoptUtils::genKmpcGlobalThreadNumCall(F, TidPtr, nullptr);
          Tid->insertBefore(EntryBB->getFirstNonPHI());
        }
        StoreInst *SI = nullptr;
        if (IsTid)
          SI = new StoreInst(Tid, TidPtr, false /*volatile*/, I32Align);
        else
          SI = new StoreInst(ConstantInt::get(I32Ty, 0), TidPtr,
                             false /*volatile*/, I32Align);
        SI->insertAfter(TidPtr);
      } else {
        for (auto IB = Tid->user_begin(), IE = Tid->user_end(); IB != IE;
             IB++) {
          auto User = dyn_cast<Instruction>(*IB);
          if (User && User->getParent() == Tid->getParent()) {
            StoreInst *SI = dyn_cast<StoreInst>(User);
            if (SI) {
              Value *V = SI->getPointerOperand();
              TidPtr = dyn_cast<AllocaInst>(V);
              break;
            }
          }
        }
      }

      if (TidPtr == nullptr) {
        IRBuilder<> Builder(EntryBB->getFirstNonPHI());
        TidPtr = Builder.CreateAlloca(I32Ty);
        StoreInst *SI = new StoreInst(Tid, TidPtr, false, I32Align);
        SI->insertAfter(Tid);
      }
      User->replaceUsesOfWith(PtrHolder, TidPtr);
    }
  }
}

// Remove routines and global variables which has no target declare
// attribute.
void VPOParoptModuleTransform::removeTargetUndeclaredGlobals() {
  // Collect the set "used" values from the "llvm.used" and "llvm.compiler.used"
  // initializers. These objects need to be retained in the target IR.
  SmallVector<GlobalValue *, 16u> UsedVec;
  auto *UsedVar = collectUsedGlobalVariables(M, UsedVec, false);
  auto *CompilerUsedVar = collectUsedGlobalVariables(M, UsedVec, true);
  SmallPtrSet<GlobalValue *, 16u> UsedSet(UsedVec.begin(), UsedVec.end());

  SmallPtrSet<GlobalAlias *, 16u> DeadAlias; // Keep track of dead Alias
  for (GlobalAlias &A : M.aliases()) {
    if (isa<GlobalValue>(A.getAliasee())) {
      auto *F = dyn_cast<Function>(A.getAliasee());
      if (F && !UsedSet.count(F) &&
          !F->getAttributes().hasFnAttr("openmp-target-declare") &&
          !F->getAttributes().hasFnAttr("target.declare")) {
        DeadAlias.insert(&A);
      }
    }
  }

  for (GlobalAlias *DA : DeadAlias) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ << "Deleteing GlobalAlias "
                          << DA->getName());
    DA->eraseFromParent();
  }

  std::vector<GlobalVariable *> DeadGlobalVars; // Keep track of dead globals
  for (GlobalVariable &GV : M.globals()) {
    // Special globals "llvm.used" and "llvm.compiler.used" should be preserved.
    if ((UsedVar && UsedVar == &GV) ||
        (CompilerUsedVar && CompilerUsedVar == &GV))
      continue;

    // Keep global variables annotated as "used" in the target IR.
    if (UsedSet.count(&GV))
      continue;

    if (!GV.isTargetDeclare()) {
      DeadGlobalVars.push_back(&GV); // Keep track of dead globals
      // TODO  The check of use_empty will be removed after the frontend
      // generates target_declare attribute for the variable GV.
      if (GV.use_empty() && GV.hasInitializer()) {
        Constant *Init = GV.getInitializer();
        GV.setInitializer(nullptr);
        if (isSafeToDestroyConstant(Init))
          Init->destroyConstant();
      }
    }
  }

  std::vector<Function *> DeadFunctions;

  for (Function &F : M) {
    // Functions annotated as "used" should be preserved.
    if (UsedSet.count(&F)) {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Emit " << F.getName()
                        << ": Is 'used' or 'compiler.used'\n");
      continue;
    }

    // IsFETargetDeclare == true means that F has the
    //     "openmp-target-declare" attribute; i.e.,
    //     The FE found that it was
    //        (1) inside a DECLARE TARGET construct, or
    //        (2) called from a TARGET region, or
    //        (3) called recursively by another function with this attribute.
    //
    // IsBETargetDeclare == true means that F has the
    //     "target.declare" attribute; i.e., it was emitted by the BE
    //     for the target device, as a result of outlining a TARGET region.
    //
    // If F has neither of these attributes, then it is not needed by the
    // target device and we remove it here.
    bool IsFETargetDeclare = F.getAttributes().hasFnAttr("openmp-target-declare");
    bool IsBETargetDeclare = F.getAttributes().hasFnAttr("target.declare");
    // unused for now
    // bool HasTargetConstruct = F.getAttributes().hasFnAttr("contains-openmp-target");
    if (IsFETargetDeclare) {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Emit " << F.getName()
                        << ": IsFETargetDeclare == true\n");
      continue;
    }

    if (!IsBETargetDeclare) {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Remove " << F.getName() << "\n");
      DeadFunctions.push_back(&F);
      if (!F.isDeclaration())
        F.deleteBody();
    } else {
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Emit " << F.getName()
                        << ": IsBETargetDeclare == true\n");
#if INTEL_CUSTOMIZATION
      // This is a workaround for resolving the incompatibility issue
      // between the xmain compiler and IGC compiler. The IGC compiler
      // cannot accept the following instruction, which is generated
      // at compile time for helping infering the address space.
      //   %4 = bitcast [10000 x i32] addrspace(1)* %B to i8*
      // The instruction becomes dead after the inferring is done.
#endif // INTEL_CUSTOMIZATION
      for (BasicBlock &BB : F)
        for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E;) {
          Instruction *Inst = &*I++;
          if (isInstructionTriviallyDead(Inst)) {
            salvageDebugInfo(*Inst);
            BB.getInstList().erase(Inst);
          }
        }
    }
  }
  auto EraseUnusedGlobalValue = [&](GlobalValue *GV) {
    // TODO  The check of use_empty will be removed after the frontend
    // generates target_declare attribute for the variable GV.
    GV->removeDeadConstantUsers();
    if (!GV->use_empty())
      return;

    GV->eraseFromParent();
  };

  for (GlobalVariable *GV : DeadGlobalVars)
    EraseUnusedGlobalValue(GV);

  for (Function *F : DeadFunctions)
    EraseUnusedGlobalValue(F);
}

// Process the device information string into the triples.
void VPOParoptModuleTransform::processDeviceTriples() {
  auto TargetDevicesStr = M.getTargetDevices();
  std::string::size_type Pos = 0;
  while (true) {
    std::string::size_type Next = TargetDevicesStr.find(',', Pos);
    Triple TT(TargetDevicesStr.substr(Pos, Next - Pos));
    TgtDeviceTriples.push_back(TT);
    if (Next == std::string::npos)
      break;
    Pos = Next + 1;
  }
}

// Hold the struct type as follows.
//    struct __tgt_offload_entry {
//      void      *addr;       // The address of a global variable
//                             // or entry point in the host.
//      char      *name;       // Name of the symbol referring to the
//                             // global variable or entry point.
//      size_t     size;       // Size in bytes of the global variable or
//                             // zero if it is entry point.
//      int32_t    flags;      // Flags of the entry.
//      int32_t    reserved;   // Reserved by the runtime library.
//      size_t     name_size;  // SPIR-V specific: size of 'name' string,
//                             // including terminating null.
// };
StructType *VPOParoptModuleTransform::getTgtOffloadEntryTy() {
  if (TgtOffloadEntryTy)
    return TgtOffloadEntryTy;

  bool IsTargetSPIRV = VPOAnalysisUtils::isTargetSPIRV(&M);

  SmallVector<Type *, 6> TyArgs = {
    Type::getInt8PtrTy(C, IsTargetSPIRV ? vpo::ADDRESS_SPACE_GENERIC : 0),
    Type::getInt8PtrTy(C, IsTargetSPIRV ? vpo::ADDRESS_SPACE_CONSTANT : 0),
    GeneralUtils::getSizeTTy(&M),
    Type::getInt32Ty(C),
    Type::getInt32Ty(C)
  };

  if (IsTargetSPIRV)
    TyArgs.push_back(GeneralUtils::getSizeTTy(&M));

  TgtOffloadEntryTy =
      StructType::create(C, TyArgs, "struct.__tgt_offload_entry", false);
  return TgtOffloadEntryTy;
}

Constant* VPOParoptModuleTransform::registerTargetRegion(WRegionNode *W,
                                                         Constant *Func) {
  auto && getOffloadEntry = [&]() -> OffloadEntry* {
    // Find existing entry in the table.
    auto *Entry =
        VPOParoptUtils::getTargetRegionOffloadEntry(W, OffloadEntries);

    // Update outlined function name.
    Func->setName(Entry->getName());
    return Entry;
  };

  auto && genRegionID = [&]() {
    return new GlobalVariable(M, Type::getInt8Ty(C), true,
                              GlobalValue::WeakAnyLinkage,
                              Constant::getNullValue(Type::getInt8Ty(C)),
                              Func->getName() + ".region_id");
  };

  // Get offload entry for this target region.
  auto *Entry = getOffloadEntry();

  // Offload runtime needs an address which uniquely identifies the target
  // region. On the device side it has to be an address of the outlined
  // target region, but on the host side it can be anything which can uniquely
  // identify the region. Therefore, for the host compilation, we can just
  // create a variable which would serve as target region ID. Since we are not
  // taking address of the outlined function on the host side it can still be
  // inlined.
  if (hasOffloadCompilation()) {
    Entry->setAddress(Func);
    return Func;
  }

  // We can see the same target region in host compilation multiple times
  // because of inlining. Do not create new region ID if it has already been
  // created earlier.
  if (auto *ID = Entry->getAddress())
    return ID;
  auto *ID = genRegionID();
  Entry->setAddress(ID);
  return ID;
}

// Create offloading entry for the provided entry ID and address.
bool VPOParoptModuleTransform::genOffloadEntries() {
  if (OffloadEntries.empty())
    return false;

  bool Changed = false;
  bool IsTargetSPIRV = VPOAnalysisUtils::isTargetSPIRV(&M);

  for (auto *E : OffloadEntries) {
    if (auto *Var = dyn_cast<VarEntry>(E))
      // Emit entry for the variable only if it is a definition.
      if (Var->isDeclaration())
        continue;

    assert(E && "uninitialized offload entry");

    // A target region encountered by FE will have the corresponding
    // entry in OffloadEntries. If the corresponding offload entry
    // is optimized away (e.g. by DCE), then the OffloadEntries's
    // entry will have NULL address. We have to allow this, since
    // FE outlining also allows this.
    //
    // Note that not all libomptarget plugins handle NULL address
    // gracefully, e.g. CUDA plugin will fail with an error report.
    // OpenCL plugin will work just because we currently do not
    // run too many optimizations for SPIRV target, and the unused
    // offload entries remain in the target code. So OpenCL plugin
    // will be able to find the corresponding kernels by name,
    // but they will never be invoked, because the host __tgt_target
    // call was optimized.
    auto *EntryAddress = E->getAddress();
    StringRef Name = E->getName();
    Constant *StrInit = ConstantDataArray::getString(C, Name);

    GlobalVariable *Str = new GlobalVariable(
        M, StrInit->getType(), /*isConstant=*/true,
        GlobalValue::InternalLinkage, StrInit,
        ".omp_offloading.entry_name", /* InsertBefore */ nullptr,
        GlobalValue::ThreadLocalMode::NotThreadLocal,
        IsTargetSPIRV ? vpo::ADDRESS_SPACE_CONSTANT : 0);
    Str->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
    Str->setTargetDeclare(true);

    auto *EntryTy = getTgtOffloadEntryTy();

    SmallVector<Constant *, 5u> EntryInitBuffer;
    if ((!IsTargetSPIRV || isa<VarEntry>(E) ||
         isa<IndirectFunctionEntry>(E)) && EntryAddress)
      EntryInitBuffer.push_back(
          ConstantExpr::getPointerBitCastOrAddrSpaceCast(
              EntryAddress, EntryTy->getElementType(0)));
    else
      EntryInitBuffer.push_back(
          Constant::getNullValue(EntryTy->getElementType(0)));
    EntryInitBuffer.push_back(
        ConstantExpr::getBitCast(Str, EntryTy->getElementType(1)));
    EntryInitBuffer.push_back(
        ConstantInt::get(EntryTy->getElementType(2), E->getSize()));
    EntryInitBuffer.push_back(
        ConstantInt::get(EntryTy->getElementType(3), E->getFlags()));
    EntryInitBuffer.push_back(ConstantInt::get(EntryTy->getElementType(4), 0));

    if (IsTargetSPIRV) {
      // We need to know the length of the entry's name string
      // to be able to transfer it from the device to host.
      // We cannot change representation of the entry on the host,
      // since it will be incompatible with other implementations.
      auto *NameStrTy = Str->getValueType();
      uint64_t NameStrSize = NameStrTy->getArrayNumElements() *
          M.getDataLayout().getTypeAllocSize(NameStrTy->getArrayElementType());
      EntryInitBuffer.push_back(
          ConstantInt::get(EntryTy->getElementType(5), NameStrSize));
    }

    Constant *EntryInit = ConstantStruct::get(EntryTy, EntryInitBuffer);

    GlobalVariable *Entry = new GlobalVariable(
        M, EntryInit->getType(), /*isConstant=*/true,
        GlobalValue::WeakAnyLinkage, EntryInit, ".omp_offloading.entry." + Name,
        /*InsertBefore=*/nullptr, GlobalValue::ThreadLocalMode::NotThreadLocal,
        IsTargetSPIRV ? SpirvOffloadEntryAddSpace : 0);

    Entry->setTargetDeclare(true);
    Triple TT(M.getTargetTriple());
    if (TT.isOSWindows()) {
      // Align entries to their size, so that entries_end symbol
      // points to the end of 32-byte aligned chunk always,
      // otherwise libomptarget may read past the section.
      cast<GlobalObject>(Entry)->setAlignment(MaybeAlign(32));
      // By convention between Paropt and clang-offload-wrapper
      // the entries contribute into sections with suffix $B,
      // the entries_begin symbol is in the section with suffix $A,
      // the entries_end symbol is in the section suffix $C.
      Entry->setSection("omp_offloading_entries$B");
    } else {
      Entry->setSection("omp_offloading_entries");
    }

    if (IsTargetSPIRV &&
        (E->getFlags() & (RegionEntry::Ctor | RegionEntry::Dtor)) != 0) {
      // The constructor/destructor will be called as a target entry,
      // so we have to set its calling convention properly.
      //
      // An offload entry generated for constructing/destructing
      // global "declare target" objects cannot be optimized, so EntryAddress
      // can never be NULL here. Assert this explicitly with a verbose
      // message instead of relying on cast<>.
      assert(EntryAddress &&
             "global ctor/dtor offload entry with null address");
      auto *EntryFunction = cast<Function>(EntryAddress);
      EntryFunction->setCallingConv(CallingConv::SPIR_KERNEL);
    }

    Changed = true;
  }

  return Changed;
}

bool VPOParoptModuleTransform::cloneDeclareTargetFunctions(
    std::function<bool(Function *F)> DummyBranchDeleter) const {
  bool Changed = false;

  SmallVector<Function *, 128> FuncList;
  for (auto &F : M.functions())
    if (!F.isDeclaration())
      FuncList.push_back(&F);

  for (auto F : FuncList) {
    StringRef ContainsOmpTargetAttrName = "contains-openmp-target";
    StringRef OmpTargetDeclareAttrName = "openmp-target-declare";
    bool HasTargetConstruct = F->hasFnAttribute(ContainsOmpTargetAttrName);
    bool IsFETargetDeclare = F->hasFnAttribute(OmpTargetDeclareAttrName);

    if (!HasTargetConstruct || !IsFETargetDeclare)
      continue;

    // Function is both "declare target" and contains "target" region(s).
    ValueToValueMapTy VMap;
    Function *NewF = CloneFunction(F, VMap);
    // The new Function is just a placeholder for the "target" region(s),
    // and it should be removed from the Module after outlining.
    // Clear "openmp-target-declare" attribute from it.
    NewF->removeFnAttr(OmpTargetDeclareAttrName);
    // The original Function may be invoked from "target" region(s),
    // so we should compile it without "target" regions enclosed
    // into it.
    F->removeFnAttr(ContainsOmpTargetAttrName);
    // Since there would be dummy branches from the begin to end directives
    // present (added in Prepare pass using addBranchToEndDirective), we
    // need to delete those branches before removing any directives.
    Changed |= DummyBranchDeleter(F);
    VPOUtils::stripDirectives(*F, { DIR_OMP_TARGET, DIR_OMP_END_TARGET });
    // We should also delete any __kmpc_[begin/end]_spmd_[target/parallel] calls
    // in the function. These calls are used to simulate omp_get_num_threads
    // support in target regions for spir64 targets. We are deleting the target
    // directives here, and we also currently ignore parallel constructs in
    // declare-target functions during codegen, so we need to get rid of their
    // corresponding spmd calls as well.
    VPOParoptUtils::deleteKmpcBeginEndSpmdCalls(F);
    Changed = true;
  }

  return Changed;
}
#endif // INTEL_COLLAB
