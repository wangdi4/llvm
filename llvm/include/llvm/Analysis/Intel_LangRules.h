#if INTEL_FEATURE_SW_ADVANCED
//===-------------------- Intel_LangRules.h -------------------------------===//
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
// Predicates based on language rules.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LANGRULES_H
#define LLVM_ANALYSIS_INTEL_LANGRULES_H

namespace llvm {

bool getLangRuleOutOfBoundsOK(void);

} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_LANGRULES_H
#endif // INTEL_FEATURE_SW_ADVANCED
