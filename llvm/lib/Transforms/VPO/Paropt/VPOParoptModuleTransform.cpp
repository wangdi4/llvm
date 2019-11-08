#if INTEL_COLLAB
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
#include "llvm/Transforms/VPO/Paropt/VPOParoptModuleTransform.h"

#include "llvm/Transforms/VPO/Paropt/VPOParoptTpv.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"

#include "llvm/Transforms/Utils/Local.h"

#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#endif // INTEL_CUSTOMIZATION

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "VPOParopt"

static cl::opt<bool> UseOffloadMetadata(
  "vpo-paropt-use-offload-metadata", cl::Hidden, cl::init(true),
  cl::desc("Use offload metadata created by clang in paropt lowering."));

// This table is used to change math function names (left column) to the
// OCL builtin format (right column).
//
// Note that fabs, ceil and floor have corresponding LLVM intrinsics
// that have the same behavior (e.g. regarding setting errno), so
// clang will represent them with intrinsic calls, which implies
// llvm.*.ty mangling.  The intrinsics will be generated only for
// C; for C++ calls to fabs[f], etc. will be generated as-is.
std::unordered_map<std::string, std::string> llvm::vpo::OCLBuiltin = {
    // float:
    {"asinf",                 "_Z4asinf"},
    {"asinhf",                "_Z5asinhf"},
    {"sinf",                  "_Z3sinf"},
    {"sinhf",                 "_Z4sinhf"},
    {"acosf",                 "_Z4acosf"},
    {"acoshf",                "_Z5acoshf"},
    {"cosf",                  "_Z3cosf"},
    {"coshf",                 "_Z4coshf"},
    {"atanf",                 "_Z4atanf"},
    {"atanhf",                "_Z5atanhf"},
    {"atan2f",                "_Z5atan2f"},
    {"tanf",                  "_Z3tanf"},
    {"tanhf",                 "_Z4tanhf"},
    {"erff",                  "_Z3erff"},
    {"expf",                  "_Z3expf"},
    {"logf",                  "_Z3logf"},
    {"log2f",                 "_Z4log2f"},
    {"powf",                  "_Z3powff"},
    {"sqrtf",                 "_Z4sqrtf"},
    {"invsqrtf",              "_Z5rsqrtf"},   // from mathimf.h
    {"fmaxf",                 "_Z4fmaxff"},
    {"llvm.maxnum.f32",       "_Z4fmaxff"},
    {"fminf",                 "_Z4fminff"},
    {"llvm.minnum.f32",       "_Z4fminff"},
    {"fabsf",                 "_Z4fabsf"},
    {"llvm.fabs.f32",         "_Z4fabsf"},
    {"ceilf",                 "_Z4ceilf"},
    {"llvm.ceil.f32",         "_Z4ceilf"},
    {"floorf",                "_Z5floorf"},
    {"llvm.floor.f32",        "_Z5floorf"},
    // double:
    {"asin",                  "_Z4asind"},
    {"asinh",                 "_Z5asinhd"},
    {"sin",                   "_Z3sind"},
    {"sinh",                  "_Z4sinhd"},
    {"acos",                  "_Z4acosd"},
    {"acosh",                 "_Z5acoshd"},
    {"cos",                   "_Z3cosd"},
    {"cosh",                  "_Z4coshd"},
    {"atan",                  "_Z4atand"},
    {"atanh",                 "_Z5atanhd"},
    {"atan2",                 "_Z5atan2d"},
    {"tan",                   "_Z3tand"},
    {"tanh",                  "_Z4tanhd"},
    {"erf",                   "_Z3erfd"},
    {"exp",                   "_Z3expd"},
    {"log",                   "_Z3logd"},
    {"log2",                  "_Z4log2d"},
    {"pow",                   "_Z3powdd"},
    {"sqrt",                  "_Z4sqrtd"},
    {"invsqrt",               "_Z5rsqrtd"},   // from mathimf.h
    {"fmax",                  "_Z4fmaxdd"},
    {"llvm.maxnum.f64",       "_Z4fmaxdd"},
    {"fmin",                  "_Z4fmindd"},
    {"llvm.minnum.f64",       "_Z4fmindd"},
    {"fabs",                  "_Z4fabsd"},
    {"llvm.fabs.f64",         "_Z4fabsd"},
    {"ceil",                  "_Z4ceild"},
    {"llvm.ceil.f64",         "_Z4ceild"},
    {"floor",                 "_Z5floord"},
    {"llvm.floor.f64",        "_Z5floord"}};

// To support the SPIRV target compilation stage of the OpenMP compilation
// offloading to GPUs, we must translate the name of math functions (left
// column in OCLBuiltin) to their OCL builtin counterparts.
static void replaceMathFnWithOCLBuiltin(Function &F) {
  StringRef OldName = F.getName();
  auto Map = OCLBuiltin.find(OldName);
  if (Map != OCLBuiltin.end()) {
    StringRef NewName = Map->second;
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Replacing " << OldName << " with "
                      << NewName << '\n');
    F.setName(NewName);
  }
}

// Given the original printf() declaration \p F coming in from clang:
//
//   declare dso_local spir_func i32 @printf(i8 addrspace(4)*, ...)
//
// Create the corresponding decl for OCL printf called from offload kernels:
//
//   declare dso_local spir_func i32
//     @_Z18__spirv_ocl_printfPU3AS2ci(i8 addrspace(1)*, ...)
//
// Save the former in \b PrintfDecl and the latter in \b OCLPrintfDecl
// in the \b VPOParoptModuleTransform class.
void VPOParoptModuleTransform::createOCLPrintfDecl(Function *F) {
  PrintfDecl = F;

  // Create FunctionType for OCLPrintfDecl
  Type *ReturnTy = Type::getInt32Ty(C);
  Type *Int8PtrTy = Type::getInt8PtrTy(C, ADDRESS_SPACE_GLOBAL /*=1*/);
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

void VPOParoptModuleTransform::collectMayHaveOMPCriticalFunctions(
    std::function<TargetLibraryInfo &(Function &F)> TLIGetter) {
  // Create call graph for the Module.
  CallGraph MCG(M);
  // Connect all orphan nodes to the entry node.
  CallGraphNode *EntryNode = MCG.getExternalCallingNode();
  for (auto &CGIt : MCG) {
    CallGraphNode *CGN = CGIt.second.get();
    if (CGN->getNumReferences() == 0 &&
        // Skip external null nodes.
        CGN->getFunction()) {
      EntryNode->addCalledFunction(nullptr, CGN);
    }
  }

  auto GetLibFunc = [&TLIGetter](Function *F) -> LibFunc {
    if (!F)
      return LibFunc::NotLibFunc;

    TargetLibraryInfo &TLI = TLIGetter(*F);
    LibFunc LF;
    if (!TLI.getLibFunc(*F, LF))
      return LibFunc::NotLibFunc;

    return LF;
  };

#ifndef NDEBUG
  for (auto &CGIt : MCG) {
    Function *CGF = CGIt.second.get()->getFunction();
    if (!CGF)
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": CallGraph Function: ";
                 dbgs() << "(null)";
                 dbgs() << "\n");
    else
      LLVM_DEBUG(dbgs() << __FUNCTION__ << ": CallGraph Function: ";
                 CGF->printAsOperand(dbgs());
                 if (GetLibFunc(CGF) != LibFunc::NotLibFunc)
                   dbgs() << " - is LibFunc";
                 dbgs() << "\n");

    // Print one level of the callees to get some information about the graph.
    for (auto &CalleeNode :
             make_range(CGIt.second.get()->begin(), CGIt.second.get()->end())) {
      Function *CalleeF = CalleeNode.second->getFunction();
      if (!CalleeF)
        LLVM_DEBUG(dbgs() << __FUNCTION__ << ":\tCallGraph callee Function: ";
                   dbgs() << "(null)";
                   dbgs() << "\n");
      else {
        LLVM_DEBUG(dbgs() << __FUNCTION__ << ":\tCallGraph callee Function: ";
                   CalleeF->printAsOperand(dbgs());
                   if (GetLibFunc(CalleeF) != LibFunc::NotLibFunc)
                     dbgs() << " - is LibFunc";
                   dbgs() << "\n");
      }
    }
  }

  // Print out the DFS post-order list.
  for (auto *CGN : post_order(&MCG)) {
    Function *CGF = CGN->getFunction();
    LLVM_DEBUG(dbgs() << __FUNCTION__ << ": Post-order: CallGraph Function: ";
               if (CGF) {
                 CGF->printAsOperand(dbgs());
                 if (GetLibFunc(CGF) != LibFunc::NotLibFunc)
                   dbgs() << " - is LibFunc";
               } else
                 dbgs() << "(null)";
               dbgs() << "\n");
  }
#endif  // NDEBUG

  // Mark __kmpc_critical() function.
  for (auto &CGIt : MCG) {
    Function *CGF = CGIt.second.get()->getFunction();
    if (GetLibFunc(CGF) == LibFunc_kmpc_critical)
      // We may probably break out of the loop here, but this will only
      // be correct if only one function name qualifies as
      // LibFunc_kmpc_critical.
      MayHaveOMPCritical.insert(CGF);
  }

  // Propagate may-have-openmp-critical attribute across the call graph.
  bool Updated;

  do {
    Updated = false;

    for (auto &CGN : post_order(&MCG)) {
      auto *CGF = CGN->getFunction();
      if (!CGF)
        // Skip external null nodes.
        continue;

      // Skip LibFunc's (e.g. sinf).
      // They will have external null callee, but we do not expect
      // OpenMP critical inside them.
      auto &TLI = TLIGetter(*CGF);
      LibFunc LF; // Not used.
      if (TLI.getLibFunc(*CGF, LF))
        continue;

      if (MayHaveOMPCritical.find(CGF) !=
          MayHaveOMPCritical.end())
        // Function is already marked.
        continue;

      // Look for callees.
      for (auto &Callee : make_range(CGN->begin(), CGN->end())) {
        auto *CalleeF = Callee.second->getFunction();
        // Check if there is an external null callee...
        if (!CalleeF ||
            // or the callee is marked already.
            MayHaveOMPCritical.find(CalleeF) != MayHaveOMPCritical.end()) {
          MayHaveOMPCritical.insert(CGF);
          Updated = true;
          break;
        }
      }
    }
  } while (Updated);

#ifndef NDEBUG
  for (auto *CGF : MayHaveOMPCritical) {
    LLVM_DEBUG(dbgs() << __FUNCTION__ <<
               ": Function marked as may-have-openmp-critical: ";
               CGF->printAsOperand(dbgs());
               dbgs() << "\n");
  }
#endif  // NDEBUG
}

// Perform paropt transformations for the module. Each module's function is
// transformed by a separate VPOParoptTransform instance which performs
// paropt transformations on a function level. Then, after tranforming all
// functions, create offload initialization code, emit offload entry table, and
// do necessary code cleanup (f.e. remove functions/globals which should not be
// generated for the target).
bool VPOParoptModuleTransform::doParoptTransforms(
    std::function<vpo::WRegionInfo &(Function &F)> WRegionInfoGetter,
    std::function<TargetLibraryInfo &(Function &F)> TLIGetter) {

  bool Changed = false;
  bool IsTargetSPIRV = VPOAnalysisUtils::isTargetSPIRV(&M) && !DisableOffload;

  processDeviceTriples();

  if (!DisableOffload) {
    loadOffloadMetadata();
  }

  if (IsTargetSPIRV)
    // For SPIR targets we have to know which functions may contain
    // "omp critical" inside them. We use this information to check
    // if an OpenMP region may call such a function.
    // If a work sharing OpenMP region contains such a call, then
    // the work sharing has to be done in a special way to make sure
    // that calls of __kmpc_critical are convergent across WIs
    // in the same sub-group, otherwise the program may experience
    // hangs.
    //
    // Note that we have to run this before the functions renaming
    // done in the loop below, otherwise the renamed library functions
    // will not be classified as such.
    // It is very unfortunate that we have to do the renaming,
    // since it changes the call-graph, and, in general, invalidates
    // any call-graph analysis done so far. Currently, we rely
    // on the fact that the library functions are assumed not to
    // use OpenMP critical, so the computed may-have-openmp-critical
    // information is not affected by the renaming.
    collectMayHaveOMPCriticalFunctions(TLIGetter);

  /// As new functions to be added, so we need to prepare the
  /// list of functions we want to work on in advance.
  std::vector<Function *> FnList;

  for (auto F = M.begin(), E = M.end(); F != E; ++F) {
    // TODO: need Front-End to set F->hasOpenMPDirective()
    if (F->isDeclaration()) { // if(!F->hasOpenMPDirective()))
      if (IsTargetSPIRV) {
        if (F->getName() == "printf")
          createOCLPrintfDecl(&*F);
        else
          replaceMathFnWithOCLBuiltin(*F);
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

    if ((Mode & OmpPar) && (Mode & ParTrans)) {
      Changed |= VPOUtils::removeBranchesFromBeginToEndDirective(*F);
      if (Changed)
        LLVM_DEBUG(
            dbgs()
            << "=== After removing branches from Begin To End Directive:\n"
            << *F);
    }

    // Walk the W-Region Graph top-down, and create W-Region List
    WRegionInfo &WI = WRegionInfoGetter(*F);
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
        WI.getORE(), OptLevel, SwitchToOffload, DisableOffload);
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

  if (!DisableOffload) {
    Triple TT(M.getTargetTriple());
    if (!TT.isOSWindows() && !hasOffloadCompilation())
      // Generate offload initialization code.
      genOffloadingBinaryDescriptorRegistration();

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
    if (F->getAttributes().hasAttribute(AttributeList::FunctionIndex,
                                        "mt-func")) {
      auto IT = F->arg_begin();
      if (!IsTid)
        IT++;
      User->replaceUsesOfWith(PtrHolder, &*IT);
    } else if (IsTid && F->getAttributes().hasAttribute(
                            AttributeList::FunctionIndex, "task-mt-func")) {
      BasicBlock *EntryBB = &F->getEntryBlock();
      IRBuilder<> Builder(EntryBB->getFirstNonPHI());
      AllocaInst *TidPtr =
          Builder.CreateAlloca(Type::getInt32Ty(C));
      Builder.CreateStore(&*(F->arg_begin()), TidPtr);
      User->replaceUsesOfWith(PtrHolder, TidPtr);
    } else {
      BasicBlock *EntryBB = &F->getEntryBlock();
      Instruction *Tid = nullptr;
      AllocaInst *TidPtr = nullptr;
      if (IsTid)
        Tid = VPOParoptUtils::findKmpcGlobalThreadNumCall(EntryBB);
      if (!Tid) {
        IRBuilder<> Builder(EntryBB->getFirstNonPHI());
        TidPtr = Builder.CreateAlloca(Type::getInt32Ty(C));
        if (IsTid) {
          Tid = VPOParoptUtils::genKmpcGlobalThreadNumCall(F, TidPtr, nullptr);
          Tid->insertBefore(EntryBB->getFirstNonPHI());
        }
        StoreInst *SI = nullptr;
        if (IsTid)
          SI = new StoreInst(Tid, TidPtr);
        else
          SI = new StoreInst(
              ConstantInt::get(Type::getInt32Ty(C), 0), TidPtr);
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
        TidPtr = Builder.CreateAlloca(Type::getInt32Ty(C));
        StoreInst *SI = new StoreInst(Tid, TidPtr);
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
  SmallPtrSet<GlobalValue *, 16u> UsedSet;
  auto *UsedVar = collectUsedGlobalVariables(M, UsedSet, false);
  auto *CompilerUsedVar = collectUsedGlobalVariables(M, UsedSet, true);

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
        if (!isa<GlobalValue>(Init) && !isa<ConstantData>(Init))
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
    bool IsFETargetDeclare = F.getAttributes().hasAttribute(
        AttributeList::FunctionIndex, "openmp-target-declare");
    bool IsBETargetDeclare = F.getAttributes().hasAttribute(
        AttributeList::FunctionIndex, "target.declare");
    // unused for now
    // bool HasTargetConstruct = F.getAttributes().hasAttribute(
    //     AttributeList::FunctionIndex, "contains-openmp-target");

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
          if (isInstructionTriviallyDead(Inst))
            BB.getInstList().erase(Inst);
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
// };
StructType *VPOParoptModuleTransform::getTgOffloadEntryTy() {
  if (TgOffloadEntryTy)
    return TgOffloadEntryTy;

  Type *TyArgs[] = {Type::getInt8PtrTy(C), Type::getInt8PtrTy(C),
                    GeneralUtils::getSizeTTy(&M), Type::getInt32Ty(C),
                    Type::getInt32Ty(C)};
  TgOffloadEntryTy =
      StructType::get(C, TyArgs, /* "struct.__tgt_offload_entry"*/false);
  return TgOffloadEntryTy;
}

// Hold the struct type as follows.
// struct __tgt_device_image{
//   void   *ImageStart;       // The address of the beginning of the
//                             // target code.
//   void   *ImageEnd;         // The address of the end of the target
//                             // code.
//   __tgt_offload_entry  *EntriesBegin;  // The first element of an array
//                                        // containing the globals and
//                                        // target entry points.
//   __tgt_offload_entry  *EntriesEnd;    // The last element of an array
//                                        // containing the globals and
//                                        // target entry points.
// };
StructType *VPOParoptModuleTransform::getTgDeviceImageTy() {
  if (TgDeviceImageTy)
    return TgDeviceImageTy;

  Type *TyArgs[] = {Type::getInt8PtrTy(C), Type::getInt8PtrTy(C),
                    PointerType::getUnqual(getTgOffloadEntryTy()),
                    PointerType::getUnqual(getTgOffloadEntryTy())};
  TgDeviceImageTy =
      StructType::get(C, TyArgs, /* "struct.__tgt_device_image" */false);
  return TgDeviceImageTy;
}

// Hold the struct type as follows.
// struct __tgt_bin_desc{
//   uint32_t              NumDevices;     // Number of device types i
//                                         // supported.
//   __tgt_device_image   *DeviceImages;   // A pointer to an array of
//                                         // NumDevices elements.
//   __tgt_offload_entry  *EntriesBegin;   // The first element of an array
//                                         // containing the globals and
//                                         // target entry points.
//   __tgt_offload_entry  *EntriesEnd;     // The last element of an array
//                                         // containing the globals and
//                                         // target entry points.
// };
//
StructType *VPOParoptModuleTransform::getTgBinaryDescriptorTy() {
  if (TgBinaryDescriptorTy)
    return TgBinaryDescriptorTy;

  Type *TyArgs[] = {Type::getInt32Ty(C),
                    PointerType::getUnqual(getTgDeviceImageTy()),
                    PointerType::getUnqual(getTgOffloadEntryTy()),
                    PointerType::getUnqual(getTgOffloadEntryTy())};
  TgBinaryDescriptorTy =
      StructType::get(C, TyArgs, /* "struct.__tgt_bin_desc" */false);
  return TgBinaryDescriptorTy;
}

// Return/Create a variable that binds the atexit to this shared
// object.
GlobalVariable *VPOParoptModuleTransform::getDsoHandle() {
  if (DsoHandle)
    return DsoHandle;

  DsoHandle = M.getGlobalVariable("__dso_handle");
  if (!DsoHandle) {
    DsoHandle = new GlobalVariable(M, Type::getInt8Ty(C), false,
                                   GlobalValue::ExternalLinkage, nullptr,
                                   "__dso_handle");
    DsoHandle->setVisibility(GlobalValue::HiddenVisibility);
  }

  return DsoHandle;
}

// Register the offloading binary descriptors.
void VPOParoptModuleTransform::genOffloadingBinaryDescriptorRegistration() {
  if (OffloadEntries.empty())
    return;

  auto OffloadEntryTy = getTgOffloadEntryTy();
  GlobalVariable *HostEntriesBegin = new GlobalVariable(
      M, OffloadEntryTy, /*isConstant=*/true, GlobalValue::ExternalLinkage,
      /*Initializer=*/nullptr, ".omp_offloading.entries_begin");
  GlobalVariable *HostEntriesEnd = new GlobalVariable(
      M, OffloadEntryTy, /*isConstant=*/true,
      llvm::GlobalValue::ExternalLinkage, /*Initializer=*/nullptr,
      ".omp_offloading.entries_end");

  SmallVector<Constant*, 8u> DeviceImagesInit;
  for (const auto &T : TgtDeviceTriples) {
    const auto &N = T.getTriple();

    auto *ImgBegin = new GlobalVariable(
      M, Type::getInt8Ty(C), /*isConstant=*/true,
      GlobalValue::ExternalWeakLinkage,
      /*Initializer=*/nullptr, Twine(".omp_offloading.img_start.") + Twine(N));
    auto *ImgEnd = new GlobalVariable(
      M, Type::getInt8Ty(C), /*isConstant=*/true,
      GlobalValue::ExternalWeakLinkage,
      /*Initializer=*/nullptr, Twine(".omp_offloading.img_end.") + Twine(N));

    SmallVector<Constant*, 4> DevInitBuffer;
    DevInitBuffer.push_back(ImgBegin);
    DevInitBuffer.push_back(ImgEnd);
    DevInitBuffer.push_back(HostEntriesBegin);
    DevInitBuffer.push_back(HostEntriesEnd);

    Constant *DevInit = ConstantStruct::get(getTgDeviceImageTy(), DevInitBuffer);
    DeviceImagesInit.push_back(DevInit);
  }

  Constant *DevArrayInit = ConstantArray::get(
      ArrayType::get(getTgDeviceImageTy(), DeviceImagesInit.size()),
      DeviceImagesInit);

  GlobalVariable *DeviceImages =
      new GlobalVariable(M, DevArrayInit->getType(),
                         /*isConstant=*/true, GlobalValue::InternalLinkage,
                         DevArrayInit, ".omp_offloading.device_images");
  DeviceImages->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);

  Constant *Index[] = { Constant::getNullValue(Type::getInt32Ty(C)),
                        Constant::getNullValue(Type::getInt32Ty(C)) };

  SmallVector<Constant *, 16> DescInitBuffer;
  DescInitBuffer.push_back(
      ConstantInt::get(Type::getInt32Ty(C), TgtDeviceTriples.size()));
  DescInitBuffer.push_back(ConstantExpr::getGetElementPtr(
      DeviceImages->getValueType(), DeviceImages, Index));
  DescInitBuffer.push_back(HostEntriesBegin);
  DescInitBuffer.push_back(HostEntriesEnd);

  Constant *DescInit =
      ConstantStruct::get(getTgBinaryDescriptorTy(), DescInitBuffer);
  GlobalVariable *Desc =
      new GlobalVariable(M, DescInit->getType(),
                         /*isConstant=*/true, GlobalValue::InternalLinkage,
                         DescInit, ".omp_offloading.descriptor");
  if (hasOffloadCompilation())
    return;
  Function *TgDescUnregFn = createTgDescUnregisterLib(Desc);
  Function *TgDescRegFn = createTgDescRegisterLib(TgDescUnregFn, Desc);

  // It is sufficient to call offload registration code once per unique
  // combination of target triples. Therefore create a comdat group for the
  // registration/unregistration functions and associated data. Registration
  // function name serves as a key for this comdat group (it includes sorted
  // offload target triple names).
  auto *ComdatKey = M.getOrInsertComdat(TgDescRegFn->getName());
  TgDescRegFn->setLinkage(GlobalValue::LinkOnceAnyLinkage);
  TgDescRegFn->setVisibility(GlobalValue::HiddenVisibility);
  TgDescRegFn->setComdat(ComdatKey);
  TgDescUnregFn->setComdat(ComdatKey);
  DeviceImages->setComdat(ComdatKey);
  Desc->setComdat(ComdatKey);
  appendToGlobalCtors(M, TgDescRegFn, 0, TgDescRegFn);
}

// Create the function .omp_offloading.descriptor_unreg.
Function *VPOParoptModuleTransform::createTgDescUnregisterLib(
    GlobalVariable *Desc) {
  Type *Params[] = { Type::getInt8PtrTy(C) };
  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), Params, false);

  Function *Fn = Function::Create(FnTy, GlobalValue::InternalLinkage,
                                  ".omp_offloading.descriptor_unreg", M);
  Fn->setCallingConv(CallingConv::C);

  BasicBlock *EntryBB = BasicBlock::Create(C, "entry", Fn);

  DominatorTree DT;
  DT.recalculate(*Fn);

  IRBuilder<> Builder(EntryBB);

  Builder.CreateRetVoid();
  VPOParoptUtils::genTgtUnregisterLib(Desc, EntryBB->getTerminator());
  Fn->setSection(".text.startup");

  return Fn;
}

// Create the function .omp_offloading.descriptor_reg
Function *VPOParoptModuleTransform::createTgDescRegisterLib(
    Function *TgDescUnregFn, GlobalVariable *Desc) {
  FunctionType *FnTy = FunctionType::get(Type::getVoidTy(C), false);

  SmallVector<StringRef, 4u> DeviceNames(TgtDeviceTriples.size());
  transform(TgtDeviceTriples, DeviceNames.begin(),
            [](const Triple &T) -> const std::string& {
               return T.getTriple();
            });
  sort(DeviceNames.begin(), DeviceNames.end());
  SmallString<128u> FnName;
  {
    raw_svector_ostream OS(FnName);
    OS << ".omp_offloading.descriptor_reg";
    for (auto &T : DeviceNames)
      OS << "." << T;
  }

  Function *Fn = Function::Create(FnTy, GlobalValue::InternalLinkage,
                                  FnName, M);
  Fn->setCallingConv(CallingConv::C);

  BasicBlock *EntryBB = BasicBlock::Create(C, "entry", Fn);

  DominatorTree DT;
  DT.recalculate(*Fn);

  IRBuilder<> Builder(EntryBB);

  Builder.CreateRetVoid();

  VPOParoptUtils::genTgtRegisterLib(Desc, EntryBB->getTerminator());
  VPOParoptUtils::genCxaAtExit(TgDescUnregFn, Desc, getDsoHandle(),
                               EntryBB->getTerminator());

  Fn->setSection(".text.startup");
  return Fn;
}

void VPOParoptModuleTransform::loadOffloadMetadata() {
  if (!UseOffloadMetadata)
    return;

  auto *MD = M.getNamedMetadata("omp_offload.info");
  if (!MD)
    return;

  // Helper for adding offload entries - resizes entries containter as needed.
  auto && addEntry = [&](OffloadEntry *E, size_t Idx) {
    auto NewSize = Idx + 1u;
    if (OffloadEntries.size() < NewSize)
      OffloadEntries.resize(NewSize);
    assert(!OffloadEntries[Idx] && "more than one entry with the same index");
    OffloadEntries[Idx] = E;
  };

  // Populate offload entries using information from the offload metadata.
  for (auto *Node : MD->operands()) {
    auto && getMDInt = [Node](unsigned I) {
      auto *V = cast<ConstantAsMetadata>(Node->getOperand(I));
      return cast<ConstantInt>(V->getValue())->getZExtValue();
    };

    auto && getMDString = [Node](unsigned I) {
      auto *V = cast<MDString>(Node->getOperand(I));
      return V->getString();
    };

    switch (getMDInt(0)) {
      case OffloadEntry::EntryKind::RegionKind: {
        auto Device = getMDInt(1u);
        auto File = getMDInt(2u);
        auto Parent = getMDString(3u);
        auto Line = getMDInt(4u);
        auto Idx = getMDInt(5u);
        auto Flags = getMDInt(6u);

        switch (Flags) {
          case RegionEntry::Region: {
            // Compose name.
            SmallString<64u> Name;
            llvm::raw_svector_ostream(Name) << "__omp_offloading"
              << llvm::format("_%x", Device) << llvm::format("_%x_", File)
              << Parent << "_l" << Line;
            addEntry(new RegionEntry(Name, Flags), Idx);
            break;
          }
          case RegionEntry::Ctor:
          case RegionEntry::Dtor: {
            auto *GV = M.getNamedValue(Parent);
            assert(GV && "no value for ctor/dtor offload entry");
            addEntry(new RegionEntry(GV, Flags), Idx);
            break;
          }
          default:
            llvm_unreachable("unexpected entry kind");
        }
        break;
      }
      case OffloadEntry::EntryKind::VarKind: {
        auto Name = getMDString(1u);
        auto Flags = getMDInt(2u);
        auto Idx = getMDInt(3u);

        auto *Var = M.getGlobalVariable(Name, true);
        assert(Var && "no global variable with given name");
        assert(Var->isTargetDeclare() && "must be a target declare variable");

        addEntry(new VarEntry(Var, Flags), Idx);
        break;
      }
      default:
        llvm_unreachable("unexpected metadata!");
    }
  }

  // Remove offload metadata from the module after parsing.
  MD->eraseFromParent();
}

Constant* VPOParoptModuleTransform::registerTargetRegion(WRegionNode *W,
                                                         Constant *Func) {
  auto && getOffloadEntry = [&]() -> OffloadEntry* {
    if (!UseOffloadMetadata) {
      // Old behavior where offload entries are created on the fly.
      auto *Entry = new RegionEntry(Func->getName(), RegionEntry::Region);
      OffloadEntries.push_back(Entry);
      return Entry;
    }

    // Find existing entry in the table.
    int Idx = W->getOffloadEntryIdx();
    assert(Idx >= 0 && "target region with no entry index");
    auto *Entry = OffloadEntries[Idx];
    assert(Entry && "entry index with no entry");

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
  Type *VoidStarTy = Type::getInt8PtrTy(C);
  Type *SizeTy = GeneralUtils::getSizeTTy(&M);
  Type *Int32Ty = Type::getInt32Ty(C);

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
        GlobalValue::InternalLinkage, StrInit, ".omp_offloading.entry_name");
    Str->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
    Str->setTargetDeclare(true);

    SmallVector<Constant *, 5u> EntryInitBuffer;
    if (!IsTargetSPIRV && EntryAddress)
      EntryInitBuffer.push_back(
        ConstantExpr::getBitCast(EntryAddress, VoidStarTy));
    else
      EntryInitBuffer.push_back(Constant::getNullValue(VoidStarTy));
    EntryInitBuffer.push_back(ConstantExpr::getBitCast(Str, VoidStarTy));
    EntryInitBuffer.push_back(ConstantInt::get(SizeTy, E->getSize()));
    EntryInitBuffer.push_back(ConstantInt::get(Int32Ty, E->getFlags()));
    EntryInitBuffer.push_back(ConstantInt::get(Int32Ty, 0));

    Constant *EntryInit =
        ConstantStruct::get(getTgOffloadEntryTy(), EntryInitBuffer);

    GlobalVariable *Entry =
        new GlobalVariable(M, EntryInit->getType(),
                           /*isConstant=*/true, GlobalValue::WeakAnyLinkage,
                           EntryInit, ".omp_offloading.entry." + Name);

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

bool VPOParoptModuleTransform::mayHaveOMPCritical(const Function *F) const {
  return MayHaveOMPCritical.find(F) != MayHaveOMPCritical.end();
}
#endif // INTEL_COLLAB
