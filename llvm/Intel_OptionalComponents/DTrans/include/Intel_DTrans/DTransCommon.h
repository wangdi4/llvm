//===------------------ DTransCommon.h - Shared DTrans code ---------------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file declares functions that are common to all DTrans passes.
//
//===----------------------------------------------------------------------===//

#if !INTEL_FEATURE_SW_DTRANS
#error DTransCommon.h include in an non-INTEL_FEATURE_SW_DTRANS build.
#endif

#ifndef INTEL_DTRANS_DTRANS_COMMON_H
#define INTEL_DTRANS_DTRANS_COMMON_H

#include "llvm/IR/PassManager.h"

namespace llvm {
void addDTransPasses(ModulePassManager &MPM);
void addLateDTransPasses(ModulePassManager &MPM);
} // namespace llvm

#endif // INTEL_DTRANS_DTRANS_COMMON_H
