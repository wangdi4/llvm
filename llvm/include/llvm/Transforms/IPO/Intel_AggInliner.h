//===----------- Intel_AggInliner.h ----------------------------------===//
//
// Copyright (C) 2020-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Aggressive Inliner
//
// Mark callsites with the "prefer-inline-aggressive" attribute. This
// attribute is checked during inline cost analysis, actual inlining is done
// by the inliner.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_AGGINLINER_H
#define LLVM_TRANSFORMS_IPO_INTEL_AGGINLINER_H

#include "llvm/ADT/StringSet.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

using AggInlGetTLITy =
    std::function<const TargetLibraryInfo &(const Function &)>;

// It handles actual analysis and results of Inline Aggressive analysis.
struct InlineAggressiveInfo {
  InlineAggressiveInfo(AggInlGetTLITy);
  InlineAggressiveInfo(InlineAggressiveInfo &&);
  InlineAggressiveInfo();
  ~InlineAggressiveInfo();

  static InlineAggressiveInfo runImpl(Module &M, WholeProgramInfo &WPI,
                                      AggInlGetTLITy GetTLI);
  bool analyzeModule(Module &MI);

private:
  AggInlGetTLITy GetTLI;

  // List of calls that are marked as AggInline.
  std::vector<CallBase *> AggInlCalls;

  bool propagateAggInlineInfo(Function *main_rtn);
  bool propagateAggInlineInfoCall(CallBase &CB);
  bool setAggInlineInfo(CallBase &CB);
  bool isCallInstInAggInlList(CallBase &CB);
  void setAggInlInfoForCallSite(CallBase &CB);
  bool setAggInlineInfoForAllCallSites(Function *F);
  bool
  trackUsesofAllocatedGlobalVariables(std::vector<GlobalVariable *> &Globals);
  bool analyzeHugeMallocGlobalPointersHeuristic(Module &MI);
  bool analyzeSingleAccessFunctionGlobalVarHeuristic(Module &MI);
  void addInliningAttributes();
};

///
/// Pass to mark callsites for aggressive inlining with the
/// "prefer-inline-aggressive" attribute.
///
class AggInlinerPass : public PassInfoMixin<AggInlinerPass> {
  static char PassID;

public:
  AggInlinerPass(void);
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm
#endif // LLVM_TRANSFORMS_IPO_INTEL_AGGINLINER_H
