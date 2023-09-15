//===-- llvm/lib/Target/Intel_CSA/CSALoopIntrinsicExpander.h ----*- C++ -*-===//
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
// This file contains the declaration for the CSALoopIntrinsicExpander pass,
// which expands CSA-specific loop intrinsics into their underlying
// representations.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSALOOPINTRINSICEXPANDER_H
#define LLVM_LIB_TARGET_CSA_CSALOOPINTRINSICEXPANDER_H

namespace llvm {
class Pass;
Pass *createCSALoopIntrinsicExpanderPass();
} // namespace llvm

#endif
