//===------- Intel_WP.h - Whole program Analysis -*------===//
//
// Copyright (C) 2016-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Whole Program Analysis
//===----------------------------------------------------------------------===//
//
#ifndef LLVM_ANALYSIS_INTELWP_H
#define LLVM_ANALYSIS_INTELWP_H

#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/Intel_WP_utils.h"

#include <queue>

namespace llvm {

namespace llvm_intel_wp_analysis {
// If it is true, compiler assumes that source files in the current
// compilation have entire program.
extern bool AssumeWholeProgram;
} // namespace llvm_intel_wp_analysis

// It handles actual analysis and results of whole program analysis.
class WholeProgramInfo {
private:
  // Set to true if all symbols have been resolved and not creating
  // shared library.
  bool WholeProgramSafe;

  // Set to true if all symbols have been resolved.
  bool WholeProgramSeen;

  // Set to true if the functions with IR were internalized, or identified
  // as LibFuncs. This is part of WholeProgramSeen.
  bool WholeProgramHidden;

  // Set to true if for each possible Level,
  //   TTI->isAdvancedOptLevelEnabled(Level)
  // is true for all Functions in the LTO unit with IR.
  bool IsAdvancedOptEnabled
      [TargetTransformInfo::AdvancedOptLevel::AO_TargetNumLevels];

  // True if the definition of main is seen in the IR
  bool MainDefSeen;

  // Go through the module M and check that all aliases were resolved
  bool analyzeAndResolveAliases(
      Module &M, std::function<const TargetLibraryInfo &(Function &F)> GetTLI);

  // Go through the module M and check that all Functions were resolved
  bool analyzeAndResolveFunctions(
      Module &M, std::function<const TargetLibraryInfo &(Function &F)> GetTLI);

  // Go through the callsites in Function F and check if the called
  // Functions are resolved
  bool collectAndResolveCallSites(
      const Function *F, std::queue<const Function *> &CallsitesFuncs,
      std::function<const TargetLibraryInfo &(Function &)> GetTLI);

  // Compute the values of IsAdvancedOptEnabled[].
  void computeIsAdvancedOptEnabled(
      Module &M, function_ref<TargetTransformInfo &(Function &)> GTTI);

  // Return true if the input Function has one of the following properties:
  //   * is main (or one of its form)
  //   * is not external (internalized symbol and is not main)
  //   * is a library function
  //   * is a linker added symbol (will be treated as libfunc)
  //   * is an intrinsic
  //   * is a branch funnel
  bool
  isValidFunction(const Function *F,
                  std::function<const TargetLibraryInfo &(Function &)> GetTLI);

  // Store the following global variables if they are available in the Module
  //   llvm.used
  //   llvm.compiler.used
  //   llvm.global_ctors
  //   llvm.global_dtors
  SetVector<const GlobalVariable *> LLVMSpecialGlobalVars;

  // Store the Functions that must be traversed
  SetVector<const Function *> FuncsCollected;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // SetVectors used for tracing the libfuncs
  // that were found and not found.
  SetVector<const Function *> LibFuncsFound;
  SetVector<const Function *> LibFuncsNotFound;

  // Store the aliases that weren't found
  SetVector<const GlobalAlias *> AliasesNotFound;

  // SetVector for storing the Functions with IR that are visible outside the
  // LTO module and weren't recognized as LibFuncs. The reason for storing
  // these functions in VisibleFunctions rather than LibFuncsNotFound is
  // to simplify the debugging process for transformations that require
  // internalization (e.g. devirtualization). A function inside
  // VisibleFunctions still a missing LibFunc.
  SetVector<const Function *> VisibleFunctions;

  // Keep track of unresolved calls
  size_t UnresolvedCallsCount;

  // Print the whole program trace
  void printWholeProgramTrace();
#endif // NDEBUG || LLVM_ENABLE_DUMP

public:

  // TODO: Make the Module and TargetLibraryInfo variables (M and GetTLI) as
  // fields of the class rather than pass them as function arguments.
  WholeProgramInfo();
  ~WholeProgramInfo();

  static WholeProgramInfo analyzeModule(
      Module &M, std::function<const TargetLibraryInfo &(Function &F)> GetTLI,
      function_ref<TargetTransformInfo &(Function &)> GTTI, CallGraph *CG);

  // Return true if the input GlobName is a form of main,
  // else return false.
  static bool isMainEntryPoint(llvm::StringRef GlobName);

  // Return true if all functions were resolved, linking for executable and
  // main was found, else return false
  bool isWholeProgramSafe();

  // Return true if all functions were resolved, else return false
  bool isWholeProgramSeen();

  // Return true if all Functions with IR were internalized or were identified
  // as LibFuncs, else return false.
  bool isWholeProgramHidden();

  // Return true if all symbols were resolved by the linker,
  // else return false.
  bool isWholeProgramRead();

  // Return true if the linker is linking an executable, else
  // return false.
  bool isLinkedAsExecutable();

  bool isAdvancedOptEnabled(TargetTransformInfo::AdvancedOptLevel AO);

  void wholeProgramAllExternsAreIntrins(
      Module &M, std::function<const TargetLibraryInfo &(Function &F)> GetTLI);

  // Return the Function* that points to main
  Function *getMainFunction(Module &M);
};

// Analysis pass providing a never-invalidated whole program analysis result.
class WholeProgramAnalysis : public AnalysisInfoMixin<WholeProgramAnalysis> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<WholeProgramAnalysis>;
  static char PassID;

public:
  typedef WholeProgramInfo Result;

  WholeProgramInfo run(Module &M, AnalysisManager<Module> &AM);
};

// Legacy wrapper pass to provide the WholeProgramInfo object.
class WholeProgramWrapperPass : public ModulePass {
  std::unique_ptr<WholeProgramInfo> Result;

public:
  static char ID;

  WholeProgramWrapperPass();

  WholeProgramInfo &getResult() { return *Result; }
  const WholeProgramInfo &getResult() const { return *Result; }

  bool runOnModule(Module &M) override;
  bool doFinalization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

ModulePass *createWholeProgramWrapperPassPass();

} // namespace llvm

#endif
