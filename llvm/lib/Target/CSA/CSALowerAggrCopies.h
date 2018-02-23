//===-- llvm/lib/Target/CSA/CSALowerAggrCopies.h ------------*- C++ -*-===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the NVIDIA specific lowering of
// aggregate copies
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSALOWERAGGRCOPIES_H
#define LLVM_LIB_TARGET_CSA_CSALOWERAGGRCOPIES_H

namespace llvm {
class FunctionPass;
FunctionPass *createLowerAggrCopies();
} // namespace llvm

#endif
