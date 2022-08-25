//==---------------------- - Common  helpers  -*- C++ -*--------------------==//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

// this file includes naming conventions and constant shared by the vectorizer passes
// this file should NOT include any environment specific data

#include "cl_cpu_detect.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/Function.h"

// Maximum width (in elements) supported as input
// An AMX tile has a maximum size of 16 rows x 64 bytes. It could be flatten to
// a <1024 x i8> vector.
#define MAX_INPUT_VECTOR_WIDTH 1024

// Define estimated amount of instructions in function
#define ESTIMATED_INST_NUM 128

// Maximum supported packetization width
#define MAX_PACKET_WIDTH 16

namespace Intel {
namespace VectorizerCommon {

// Get ISAClass from CPUDetect object, use command line toggle if
// it is not available.
llvm::VFISAKind getCPUIdISA(
  const Intel::OpenCL::Utils::CPUDetect *CPUId = nullptr);

// Skip function when traversing CallGraph in VectorVariant passes.
bool skipFunction(Function *F);

} // namespace VectorizerCommon
} // namespace Intel
