#if INTEL_FEATURE_SW_ADVANCED
//===------- Intel_IPCloning.h - IP Cloning  -*------===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
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
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

/// Pass to perform IP Cloning.
class IPCloningPass : public PassInfoMixin<IPCloningPass> {
public:
  IPCloningPass(bool AfterInl, bool EnableDTrans);
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);

private:
  // This flag helps to decide whether function addresses or other
  // constants need to be considered for cloning.
  bool AfterInl;
  // If 'true' we are doing specialized cloning generally applicable
  // when we are running DTrans.
  bool EnableDTrans;
};

}
#endif // LLVM_TRANSFORMS_IPO_INTEL_IPCLONING_H
#endif // INTEL_FEATURE_SW_ADVANCED
