//===------- Intel_IPCloning.h - IP Cloning  -*------===//
//
// Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass performs IP cloning.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_IPCLONING_H
#define LLVM_TRANSFORMS_IPO_INTEL_IPCLONING_H

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// Pass to perform IP Cloning.
class IPCloningPass : public PassInfoMixin<IPCloningPass> {
public:
  IPCloningPass(bool AfterInl, bool IFSwitchHeuristic);
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);

private:
  // This flag helps to decide whether function addresses or other
  // constants need to be considered for cloning.
  bool AfterInl;
  // If 'true', enable cloning on routines with formals that feed a
  // large number of if tests and switches.
  bool IFSwitchHeuristic;
};

}
#endif // LLVM_TRANSFORMS_IPO_INTEL_IPCLONING_H
