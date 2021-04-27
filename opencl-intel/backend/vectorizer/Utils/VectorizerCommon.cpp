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

extern cl::opt<VectorVariant::ISAClass> CPUIsaEncodingOverride;

namespace Intel {
namespace VectorizerCommon {

VectorVariant::ISAClass getCPUIdISA(
    const Intel::OpenCL::Utils::CPUDetect *CPUId /*=nullptr*/) {
  if (CPUIsaEncodingOverride.getNumOccurrences())
    return CPUIsaEncodingOverride.getValue();

  assert(CPUId && "Valid CPUDetect is expected!");
  if (CPUId->HasAVX512Core())
    return VectorVariant::ZMM;
  if (CPUId->HasAVX2())
    return VectorVariant::YMM2;
  if (CPUId->HasAVX1())
    return VectorVariant::YMM1;
  return VectorVariant::XMM;
}

} // namespace VectorizerCommon
} // namespace Intel
