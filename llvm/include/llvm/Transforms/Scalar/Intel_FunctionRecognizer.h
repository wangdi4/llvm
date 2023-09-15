#if INTEL_FEATURE_SW_ADVANCED
//===- Intel_FunctionRecognizer.h - Function Recognition --------------===//
////
//// Copyright (C) 2020 Intel Corporation. All rights reserved.
////
//// The information and source code contained herein is the exclusive
//// property of Intel Corporation and may not be disclosed, examined
//// or reproduced in whole or in part without explicit written authorization
//// from the company.
////
////===-------------------------------------------------------------------===//
////
//// This pass performs Function recognition. See implementation file for more
//// details.
////
////===-------------------------------------------------------------------===//
//

#ifndef LLVM_TRANSFORMS_SCALAR_INTEL_FUNCTION_RECOGNIZER_H
#define LLVM_TRANSFORMS_SCALAR_INTEL_FUNCTION_RECOGNIZER_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class FunctionRecognizerPass : public PassInfoMixin<FunctionRecognizerPass> {

public:
  explicit FunctionRecognizerPass(void) {}
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_INTEL_FUNCTION_RECOGNIZER_H
#endif // INTEL_FEATURE_SW_ADVANCED
