// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "InstCounter.h"
#include "VecConfig.h"

#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"

namespace  intel {

class VectorKernelDiscard : public llvm::ModulePass {
public:
  static char ID;

  VectorKernelDiscard(const OptimizerConfig *Config = nullptr);

  bool runOnModule(llvm::Module &M) override;

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
    AU.setPreservesAll();
  }

private:
  WeightedInstCounter *
  addPassesToCalculateCost(llvm::legacy::FunctionPassManager &FPM,
                           TargetMachine *TM, TargetLibraryInfoImpl &TLI,
                           ArrayRef<Module *> BuiltinModules, bool IsScalar);

  const OptimizerConfig* Config;
};

} // namespace intel
