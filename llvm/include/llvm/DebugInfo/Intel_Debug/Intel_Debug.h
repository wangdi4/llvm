//===- Intel_Debug.h - Intel Target-Specific Debug Pass -------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass performs Intel-specific modifications to the debug information.
// These modifications may be target specific. The debug information exists
// in the metadata, so this pass is designed to operate on the entire module.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_DEBUGINFO_INTEL_DEBUG_H
#define LLVM_DEBUGINFO_INTEL_DEBUG_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class TargetMachine;

class Intel_DebugPass : public PassInfoMixin<Intel_DebugPass> {
private:
  TargetMachine *TM;

public:
  Intel_DebugPass(TargetMachine *TM);

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  static bool isRequired() { return true; }
};

} // namespace llvm

#endif // LLVM_DEBUGINFO_INTEL_DEBUG_H
