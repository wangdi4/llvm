//===- SYCLPreprocessSPIRVFriendlyIR.cpp - DPC++ preprocessor on SPV-IR --===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/SYCLPreprocessSPIRVFriendlyIR.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-preprocess-spv-ir"

// Add "opencl.ocl.version" named metadata to SYCL module.
// SYCL (OpenCL CPP) implies OpenCL 2.0 implemententation for CPU backend.
// e.g.
// !opencl.ocl.version = !{!6}
// !6 = !{i32 2, i32 0}
static bool insertOpenCLVersionMetadata(Module &M) {
  if (!CompilationUtils::isGeneratedFromOCLCPP(M))
    return false;

  const char OCLVer[] = "opencl.ocl.version";
  if (M.getNamedMetadata(OCLVer))
    return false;

  auto *OCLVerMD = M.getOrInsertNamedMetadata(OCLVer);
  auto *Int32Ty = llvm::IntegerType::getInt32Ty(M.getContext());
  llvm::Metadata *OCLVerElts[] = {
      llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(Int32Ty, 2)),
      llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(Int32Ty, 0))};
  OCLVerMD->addOperand(llvm::MDNode::get(M.getContext(), OCLVerElts));
  return true;
}

bool SYCLPreprocessSPIRVFriendlyIRPass::runImpl(Module &M) {
  bool Changed = false;
  Changed |= insertOpenCLVersionMetadata(M);
  return Changed;
}

PreservedAnalyses
SYCLPreprocessSPIRVFriendlyIRPass::run(Module &M, ModuleAnalysisManager &) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  PA.preserve<CallGraphAnalysis>();
  return PA;
}
