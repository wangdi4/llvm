//===------- Intel_WP.h - Whole program Analysis -*------===//
//
// Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
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

namespace llvm {

namespace llvm_intel_wp_analysis {
// If it is true, compiler assumes that source files in the current
// compilation have entire program.
extern cl::opt<bool> AssumeWholeProgram;
}

void setWholeProgramRead(bool ProgramRead);
void setLinkingExecutable(bool LinikingExe);

// Set if the LTO process found that all symbols
// have hidden visibility
void setVisibilityHidden(bool AllSymbolsHidden);

// Store the symbols that are visible outside the LTO unit
void storeVisibleSymbols(StringRef SymbolName);

// It handles actual analysis and results of whole program analysis.
class WholeProgramInfo {
private:
  // Set to true if all symbols have been resolved and not creating
  // shared library.
  bool WholeProgramSafe;

  // Set to true if all symbols have been resolved.
  bool WholeProgramSeen;

  // Set to true if for each possible Level,
  //   TTI->isAdvancedOptLevelEnabled(Level)
  // is true for all Functions in the LTO unit with IR.
  bool IsAdvancedOptEnabled[
      TargetTransformInfo::AdvancedOptLevel::AO_TargetNumLevels];

  size_t UnresolvedCallsCount;

  // SetVectors used for tracing the libfuncs
  // that were found and not found.
  SetVector<const Function *> LibFuncsFound;
  SetVector<const Function *> LibFuncsNotFound;
  SetVector<const Function *> ExternalSymbols;

  // Return true if all symbols have hidden visibility, else
  // return false.
  bool isWholeProgramHidden();

  // Compute the values of IsAdvancedOptEnabled[].
  void computeIsAdvancedOptEnabled(Module &M,
      function_ref<TargetTransformInfo &(Function &)> GTTI);

public:
  WholeProgramInfo();
  //WholeProgramInfo(WholeProgramInfo &&Arg);
  ~WholeProgramInfo();

  static WholeProgramInfo analyzeModule(Module &M,
                                        const TargetLibraryInfo &TLI,
                                        function_ref<TargetTransformInfo
                                            &(Function &)> GTTI,
                                        CallGraph *CG);

  // Fold the intrinsic llvm.intel.wholeprogramsafe
  // into true or false depending on the result of the analysis
  void foldIntrinsicWholeProgramSafe(Module &M);

  // Return true if the input GlobName is a form of main,
  // else return false.
  static bool isMainEntryPoint(llvm::StringRef GlobName);

  bool isWholeProgramSafe();
  bool isWholeProgramSeen();
  bool isAdvancedOptEnabled(TargetTransformInfo::AdvancedOptLevel AO);

  void wholeProgramAllExternsAreIntrins(Module &M,
                                        const TargetLibraryInfo &TLI);
  bool resolveAllLibFunctions(Module &M, const TargetLibraryInfo &TLI);
  bool resolveCallsInRoutine(const TargetLibraryInfo &TLI, llvm::Function *F);
  bool resolveCalledValue(const TargetLibraryInfo &TLI, const Value *Arg,
                          const Function *Caller);
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

}

#endif
