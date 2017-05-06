//===------- Intel_AggInline.h - Aggressive Inline Analysis -*------===//
//
// Copyright (C) 2016-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Aggressive Inline Analysis
//===----------------------------------------------------------------------===//
//
#ifndef LLVM_ANALYSIS_INTELAGGINLINE_H
#define LLVM_ANALYSIS_INTELAGGINLINE_H

#include "llvm/ADT/StringSet.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Pass.h"

namespace llvm {

// It handles actual analysis and results of Inline Aggressive analysis.
struct InlineAggressiveInfo {
  InlineAggressiveInfo();
  ~InlineAggressiveInfo();

  static InlineAggressiveInfo runImpl(Module &M, WholeProgramInfo *WPI);
  bool analyzeModule(Module &MI);

  bool isCallInstInAggInlList(CallSite CS);

  bool isAggInlineOccured(void);


private:
  // List of calls that are marked as AggInline. 
  std::vector<WeakVH> AggInlCalls;

  bool propagateAggInlineInfo(Function *main_rtn);
  bool propagateAggInlineInfoCall(CallSite CS);
  bool setAggInlineInfo(CallSite CS);
  void setAggInlInfoForCallSite(CallSite CS);
  bool setAggInlineInfoForAllCallSites(Function *F);
  bool trackUsesofAllocatedGlobalVariables(
                             std::vector<GlobalVariable*> &Globals);
};

// Analysis pass providing a never-invalidated Inline Aggressive
// analysis result.
class InlineAggAnalysis : public AnalysisInfoMixin<InlineAggAnalysis> {
  static AnalysisKey Key;
  friend AnalysisInfoMixin<InlineAggAnalysis>;
  static char PassID;

public:
  typedef InlineAggressiveInfo Result;

  InlineAggressiveInfo run(Module &M, AnalysisManager<Module> &AM);
};

// Legacy wrapper pass to provide the InlineAggressiveInfo object.
class InlineAggressiveWrapperPass : public ModulePass {
  std::unique_ptr<InlineAggressiveInfo> Result;

public:
  static char ID;

  InlineAggressiveWrapperPass();

  InlineAggressiveInfo &getResult() { return *Result; }
  const InlineAggressiveInfo &getResult() const { return *Result; }

  bool runOnModule(Module &M) override;
  bool doFinalization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

ModulePass *createInlineAggressiveWrapperPassPass();

}

#endif
