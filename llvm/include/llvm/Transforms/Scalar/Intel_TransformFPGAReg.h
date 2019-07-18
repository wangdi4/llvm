//==--- Intel_TransformFPGAReg.h - Header file of TransformFPGAReg pass -*- C++ -*---==//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef __TRANSFORM_FPGA_REG_H__
#define __TRANSFORM_FPGA_REG_H__

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Module.h"

namespace llvm {

/// TransformFPGAReg class transforms llvm.(ptr.)annotation.* representation
/// of __fpga_reg builtin to the proprietary builtin representation, namely the
/// llvm.fpga.reg.* intrinsic calls
class TransformFPGARegPass :
  public PassInfoMixin<TransformFPGARegPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &);
};
} // end namespace llvm

#endif // __TRANSFORM_FPGA_REG_H__
