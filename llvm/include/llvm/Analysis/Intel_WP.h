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

  // Set to 'true' if LibIRC is enabled for all functions with IR.
  bool IsLibIRCAllowedEverywhere;

  // True if the definition of main is seen in the IR
  bool MainDefSeen;

  // Go through the module M and check that all aliases were resolved
  bool analyzeAndResolveAliases();

  // Go through the module M and check that all Functions were resolved
  bool analyzeAndResolveFunctions();

  // Collect the following globals from the Module M and store them in the
  // SetVector LLVMSpecialGlobalVars:
  //
  //   llvm.used:          Treat the symbol as if it needs to be retained in the
  //                       module, even if it appears it could be removed.
  //   llvm.compiler.used: Same as llvm.used but the compiler shouldn't touch the
  //                         function
  //   llvm.global_ctors:  Everything executed before main
  //   llvm.global_dtors:  Everything executed after main
  //
  // These globals are arrays that contain Functions needed by the whole program
  // analysis but we can't reach them from main.
  void collectLLVMSpecialGlobalVars(
      SetVector<const GlobalVariable *> &LLVMSpecialGlobalVars);

  // Go through the callsites in Function F and check if the called
  // Functions are resolved
  bool collectAndResolveCallSites(
      const Function *F, std::queue<const Function *> &CallsitesFuncs);

  // Compute the values of IsAdvancedOptEnabled[].
  void computeIsAdvancedOptEnabled();

  // Compute the value of IsLibIRCAllowedEverywhere.
  void computeIsLibIRCAllowedEverywhere();

  // Given a GlobalAlias GA, return true if the alias was resolved, else return
  // false. An alias is identified as resolved if:
  //
  //   * All functions in the alias must be internal functions or libfuncs
  //   * All aliases in the alias must be resolved too
  //   * There is no recursion in the alias
  //
  // The GlobalAlias OriginalGA is used for checking any recursion (Alias A
  // points to Alias B which points to Alias A).
  bool isValidAlias(const GlobalAlias *GA, const GlobalAlias *OriginalGA);

  // Return true if the input Function has one of the following properties:
  //   * is main (or one of its form)
  //   * is not external (internalized symbol and is not main)
  //   * is a library function
  //   * is a linker added symbol (will be treated as libfunc)
  //   * is an intrinsic
  //   * is a branch funnel
  bool isValidFunction(const Function *F);

  // Store the following global variables if they are available in the Module
  //   llvm.used
  //   llvm.compiler.used
  //   llvm.global_ctors
  //   llvm.global_dtors
  SetVector<const GlobalVariable *> LLVMSpecialGlobalVars;

  // Store the Functions that must be traversed
  SetVector<const Function *> FuncsCollected;

  // Store the aliases that were resolved
  SetVector<const GlobalAlias*> AliasesFound;

  // Current Module
  Module *M;

  // Lambda function used for collecting library function
  std::function<const TargetLibraryInfo &(Function &)> GetTLI;

  // Lambda function used for collecting information from
  // TargetTransformInfo analysis pass.
  function_ref<TargetTransformInfo &(Function &)> GTTI;

  // Store the information collected from the linker like symbol
  // resolution, linking an executable, etc.
  WholeProgramUtils *WPUtils;

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

  WholeProgramInfo(Module *M,
      std::function<const TargetLibraryInfo &(Function &F)> GetTLI,
      function_ref<TargetTransformInfo &(Function &)> GTTI,
      WholeProgramUtils *WPUtils);
  ~WholeProgramInfo();

  void analyzeModule();

  // Return true if the input GlobName is a form of main,
  // else return false.
  bool isMainEntryPoint(llvm::StringRef GlobName);

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

  void wholeProgramAllExternsAreIntrins();

  // Returns 'true' if use of LibIRC is allowed for all functions with IR.
  bool isLibIRCAllowedEverywhere();

  // Return the information collected from the linker
  WholeProgramUtils *getWholeProgramLinkerUtils() { return WPUtils; }

  // Return the Function* that points to main
  Function *getMainFunction();

  // Handle the invalidation of this information.
  // Once we have determined whole program status, it should be persistent
  // for the remainder of the compilation. So we return false here. This
  // is similar to the case of ProfileSummaryInfo.
  bool invalidate(Module &, const PreservedAnalyses &,
                  ModuleAnalysisManager::Invalidator &) {
    return false;
  }
};

// Analysis pass providing a never-invalidated whole program analysis result.
class WholeProgramAnalysis : public AnalysisInfoMixin<WholeProgramAnalysis> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<WholeProgramAnalysis>;
  static char PassID;

  // Store the information related to the linker like symbols resolution
  WholeProgramUtils WPUtils;

public:
  typedef WholeProgramInfo Result;

  explicit WholeProgramAnalysis(WholeProgramUtils WPUtils) :
      WPUtils(std::move(WPUtils)) { }
  WholeProgramInfo run(Module &M, AnalysisManager<Module> &AM);
};

// Legacy wrapper pass to provide the WholeProgramInfo object.
class WholeProgramWrapperPass : public ModulePass {
  std::unique_ptr<WholeProgramInfo> Result;

public:
  static char ID;

  // We must provide a default constructor for the pass due to the build
  // but it should not be used. Please use the constructor that supports
  // the WPUtils.
  WholeProgramWrapperPass();
  explicit WholeProgramWrapperPass(WholeProgramUtils WPUtils);

  WholeProgramInfo &getResult() { return *Result; }
  const WholeProgramInfo &getResult() const { return *Result; }

  bool runOnModule(Module &M) override;
  bool doFinalization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  // Store the information related to the linker like symbols resolution
  WholeProgramUtils WPUtils;
};

ModulePass *createWholeProgramWrapperPassPass(WholeProgramUtils WPUtils);

} // namespace llvm

#endif
