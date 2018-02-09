//==----- Passes.h - Constructors for OpenCL transformations -*- C++ -*-----==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
#ifndef INTEL_OPENCL_TRANSFORMS_PASSES_H
#define INTEL_OPENCL_TRANSFORMS_PASSES_H

namespace llvm {

class FunctionPass;

/// Splits lvm.fmuladd to "fmul + fadd" pair of instructions.
FunctionPass* createFMASplitterPass();

} // namespace llvm

#endif // INTEL_OPENCL_TRANSFORMS_PASSES_H
