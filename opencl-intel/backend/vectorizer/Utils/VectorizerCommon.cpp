//==--- VectorizerCommon.cpp - Common Vectorizer helpers  -*- C++ -*--------==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "VectorizerCommon.h"

#include "LoopHandler/CLWGBoundDecoder.h"
#include "llvm/IR/Function.h"

extern cl::opt<VectorVariant::ISAClass> IsaEncodingOverride;

namespace Intel {
namespace VectorizerCommon {

VectorVariant::ISAClass getCPUIdISA(
    const Intel::OpenCL::Utils::CPUDetect *CPUId /*=nullptr*/) {
  if (IsaEncodingOverride.getNumOccurrences())
    return IsaEncodingOverride.getValue();

  assert(CPUId && "Valid CPUDetect is expected!");
  if (CPUId->HasAVX512Core())
    return VectorVariant::ZMM;
  if (CPUId->HasAVX2())
    return VectorVariant::YMM2;
  if (CPUId->HasAVX1())
    return VectorVariant::YMM1;
  return VectorVariant::XMM;
}

bool skipFunction(Function *F) {
  return !F || F->isIntrinsic() || F->isDeclaration() ||
    intel::CLWGBoundDecoder::isWGBoundFunction(
      F->getName().str());
}

} // namespace VectorizerCommon
} // namespace Intel
