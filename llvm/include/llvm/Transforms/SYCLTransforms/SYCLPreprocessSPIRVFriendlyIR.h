//===- SYCLPreprocessSPIRVFriendlyIR.h - DPC++ preprocessor on SPV-IR ----===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_PREPROCESS_SPIRV_FRIENDLY_IR_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_PREPROCESS_SPIRV_FRIENDLY_IR_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// Preprocess the input SPIRV-Friendly-IR (AKA. SPV-IR) before translating it
/// into OCL-IR.
/// - Add "opencl.ocl.version" metadata.
/// - Materialize "spirv.SampleImage." opaque type name postfix.
class SYCLPreprocessSPIRVFriendlyIRPass
    : public PassInfoMixin<SYCLPreprocessSPIRVFriendlyIRPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  // Glue for old PM.
  bool runImpl(Module &M);
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_PREPROCESS_SPIRV_FRIENDLY_IR_H
