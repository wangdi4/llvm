//==--- VectorizerCommon.cpp - Common Vectorizer helpers  -*- C++ -*--------==//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "VectorizerCommon.h"

#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/WGBoundDecoder.h"

extern cl::opt<llvm::VFISAKind> IsaEncodingOverride;

namespace Intel {
namespace VectorizerCommon {

llvm::VFISAKind getCPUIdISA(
    const Intel::OpenCL::Utils::CPUDetect *CPUId /*=nullptr*/) {
  if (IsaEncodingOverride.getNumOccurrences())
    return IsaEncodingOverride.getValue();

  assert(CPUId && "Valid CPUDetect is expected!");
  if (CPUId->HasAVX512Core())
    return llvm::VFISAKind::AVX512;
  if (CPUId->HasAVX2())
    return llvm::VFISAKind::AVX2;
  if (CPUId->HasAVX1())
    return llvm::VFISAKind::AVX;
  return llvm::VFISAKind::SSE;
}

bool skipFunction(Function *F) {
  return !F || F->isIntrinsic() || F->isDeclaration() ||
         llvm::WGBoundDecoder::isWGBoundFunction(F->getName().str());
}

} // namespace VectorizerCommon
} // namespace Intel
