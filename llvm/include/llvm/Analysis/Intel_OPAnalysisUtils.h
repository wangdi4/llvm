//===-------------------- Intel_OPAnalysisUtils.h -------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
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

#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"

namespace llvm {
//
// If the type of 'V' is a pointer type, infer and return its pointer element
// type, or return 'nullptr'. If 'FunctionOnly', search only the Function
// containing 'F', do not descend down the call tree through CallBases.
//
extern Type *inferPtrElementType(Value &V, bool FunctionOnly = false);

} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_OPANALYSISUTILS_H
