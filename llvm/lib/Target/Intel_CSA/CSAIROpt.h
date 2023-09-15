//===-- llvm/lib/Target/Intel_CSA/CSAIROpt.h --------------------*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration for the CSAIRReductionOpt pass
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAIROPT_H
#define LLVM_LIB_TARGET_CSA_CSAIROPT_H

namespace llvm {
class Pass;
Pass *createCSAIRReductionOptPass();
} // namespace llvm

#endif
