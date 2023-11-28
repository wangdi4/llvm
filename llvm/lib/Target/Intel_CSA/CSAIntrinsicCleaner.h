//===-- llvm/lib/Target/Intel_CSA/CSAIntrinsicCleaner.h ---------*- C++ -*-===//
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
// This file contains the declaration for the CSAIntrinsicCleaner pass, which
// cleans up unused CSA intrinsics that shouldn't show up in the backend and
// detects iteration-local storage.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAINTRINSICCLEANER_H
#define LLVM_LIB_TARGET_CSA_CSAINTRINSICCLEANER_H

namespace llvm {
class Pass;
Pass *createCSAIntrinsicCleanerPass();
} // namespace llvm

#endif
