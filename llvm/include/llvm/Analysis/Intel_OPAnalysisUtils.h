//===-------------------- Intel_OPAnalysisUtils.h -------------------------===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Analysis routines helpful in working with IR that uses opaque pointers.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_OPANALYSISUTILS_H
#define LLVM_ANALYSIS_INTEL_OPANALYSISUTILS_H

#include "llvm/IR/Instructions.h"

namespace llvm {

//
// If the type of 'Arg' is a pointer type, infer and return its pointer element
// type, or return 'nullptr'.
//
extern Type *inferPtrElementType(Argument &Arg);

} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_OPANALYSISUTILS_H
