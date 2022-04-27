// ===---------- SetPreferVectorWidth.h - Class definition -*- C++ -*-----=== //
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// Set 'prefer-vector-width' function attribute based on CPU architecture.
///
// ===--------------------------------------------------------------------=== //
#ifndef SET_MIN_LEGAL_WIDTH_H
#define SET_MIN_LEGAL_WIDTH_H

#include "cl_cpu_detect.h"
#include "llvm/Pass.h"

using namespace llvm;

using CPUIDTy = Intel::OpenCL::Utils::CPUDetect;

namespace intel {

class SetPreferVectorWidth : public ModulePass {
public:
  SetPreferVectorWidth(const CPUIDTy *CPUID = nullptr);

  bool runOnModule(Module &) override;

  static char ID;

private:
  const CPUIDTy *CPUID;
};

} // namespace intel
#endif // SET_MIN_LEGAL_WIDTH_H
