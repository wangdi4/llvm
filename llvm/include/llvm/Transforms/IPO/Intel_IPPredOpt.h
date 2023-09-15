#if INTEL_FEATURE_SW_ADVANCED
//===------- Intel_IPPredOpt.h ------------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// IP Predicate Optimization.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_IPPREDOPT_H
#define LLVM_TRANSFORMS_IPO_INTEL_IPPREDOPT_H

#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

///
/// Pass to perform IP Predicate Optimization.
///
class IPPredOptPass : public PassInfoMixin<IPPredOptPass> {
public:
  IPPredOptPass(void);
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm
#endif // LLVM_TRANSFORMS_IPO_INTEL_IPPREDOPT_H
#endif // INTEL_FEATURE_SW_ADVANCED
