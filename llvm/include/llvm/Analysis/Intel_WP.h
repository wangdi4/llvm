//===------- Intel_WP.h - Whole program Analysis -*------===//
//
// Copyright (C) 2016-2017 Intel Corporation. All rights reserved.
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

#include "llvm/ADT/StringSet.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Pass.h"

namespace llvm {

// It handles actual analysis and results of whole program analysis.
class WholeProgramInfo {
private:
  // Set to true if all symbols have been resolved and not creating
  // shared library.
  bool WholeProgramSafe;

  // Set to true if all symbols have been resolved.
  bool WholeProgramSeen;

public:
  WholeProgramInfo();
  //WholeProgramInfo(WholeProgramInfo &&Arg);
  ~WholeProgramInfo();

  static WholeProgramInfo analyzeModule(Module &M,
                                        const TargetLibraryInfo &TLI,
                                        CallGraph *CG);

  bool isWholeProgramSafe();
  bool isWholeProgramSeen();

  void wholeProgramAllExternsAreIntrins(Module &M,
                                        const TargetLibraryInfo &TLI);
  bool resolveAllLibFunctions(Module &M, const TargetLibraryInfo &TLI);
  bool resolveCallsInRoutine(const TargetLibraryInfo &TLI,
                             llvm::Function*, int*);
  void makeAllLocalToCompilationUnit(Module &M, CallGraph *CG);
  bool makeInternalize(GlobalValue &GV, const StringSet<> &AlwaysPreserved);
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
