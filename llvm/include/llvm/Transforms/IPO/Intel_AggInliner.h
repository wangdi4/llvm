//===----------- Intel_AggInliner.h ----------------------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {

using AggInlGetTLITy =
    std::function<const TargetLibraryInfo &(const Function &)>;
using AggInlGetLITy = std::function<const LoopInfo &(const Function &)>;
using AggInlGetSETy = std::function<ScalarEvolution &(const Function &)>;

// It handles actual analysis and results of Inline Aggressive analysis.
struct InlineAggressiveInfo {
  InlineAggressiveInfo(AggInlGetTLITy, AggInlGetLITy, AggInlGetSETy);
  InlineAggressiveInfo(InlineAggressiveInfo &&);
  InlineAggressiveInfo();
  ~InlineAggressiveInfo();
  InlineAggressiveInfo &operator=(const InlineAggressiveInfo &) = delete;

  static InlineAggressiveInfo runImpl(Module &M, WholeProgramInfo &WPI,
                                      AggInlGetTLITy GetTLI,
                                      AggInlGetLITy GetLI, AggInlGetSETy GetSE);
  bool analyzeModule(Module &MI);

private:
  AggInlGetTLITy GetTLI;
  AggInlGetLITy GetLI;
  AggInlGetSETy GetSE;

  // List of calls that are marked as AggInline.
  SetVector<CallBase *> AggInlCalls;

  bool setAggInlInfoForCallSite(CallBase &CB, bool Recursive);
  bool setAggInlInfoForCallSites(Function &F);
  bool trackUsesOfAGVs(std::vector<GlobalVariable *> &GVs);
  bool analyzeHugeMallocGlobalPointersHeuristic(Module &MI);
  bool analyzeSingleAccessFunctionGlobalVarHeuristic(Module &MI);
  void addInliningAttributes();
  void setNoRecurseOnTinyFunctions(Module &MI);
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
