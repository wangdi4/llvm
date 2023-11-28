//===-- CSAMemopOrdering.h - Standard memop ordering pass -------*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
///===---------------------------------------------------------------------===//
/// \file
///
/// This file contains the interface for the standard CSA memop ordering pass.
///
///===---------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_MEMOPORDERING_CSAMEMOPORDERING_H
#define LLVM_LIB_TARGET_CSA_MEMOPORDERING_CSAMEMOPORDERING_H

#include "CSAMemopOrderingBase.h"

namespace llvm {

class CSATargetMachine;

/// Instantiates a CSAMemopOrderingPass. The full class definition is in
/// CSAMemopOrdering.cpp.
Pass *createStandardCSAMemopOrderingPass(const CSATargetMachine *);

void initializeCSAMemopOrderingPass(PassRegistry &);

} // namespace llvm

#endif
