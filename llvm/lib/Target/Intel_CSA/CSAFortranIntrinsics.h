//===-- llvm/lib/Target/Intel_CSA/CSAFortranIntrinsics.h --------*- C++ -*-===//
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
// This file contains the declaration for the FortranIntrinsics pass, which
// converts certain function calls to intrinsics to make it possible to access
// the intrinsics in Fortran
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAFORTRANINTRINSICS_H
#define LLVM_LIB_TARGET_CSA_CSAFORTRANINTRINSICS_H

namespace llvm {
class FunctionPass;
FunctionPass *createFortranIntrinsics();
} // namespace llvm

#endif
