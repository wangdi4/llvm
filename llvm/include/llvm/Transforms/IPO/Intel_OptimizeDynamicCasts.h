//===- Intel_OptimizeDynamicCasts.h - Optimize dynamic_cast calls  -------*- C++
//-*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_INTEL_OPTIMIZEDYNAMICCASTS_H
#define LLVM_TRANSFORMS_IPO_INTEL_OPTIMIZEDYNAMICCASTS_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class Module;
class WholeProgramInfo;
class TargetLibraryInfo;

class OptimizeDynamicCastsPass
    : public PassInfoMixin<OptimizeDynamicCastsPass> {
private:
  typedef std::map<GlobalVariable *, bool> TypeInfoMap;
  TypeInfoMap TypeInfoAnalysis;

  bool isTransformationApplicable(CallInst *);

public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);

  PreservedAnalyses
  runImpl(Module &M, WholeProgramInfo &WPI,
          std::function<const TargetLibraryInfo &(Function &F)> GetTLI);
};

ModulePass *createOptimizeDynamicCastsWrapperPass();

} // namespace llvm

#endif // LLVM_TRANSFORMS_IPO_INTEL_OPTIMIZEDYNAMICCASTS_H
