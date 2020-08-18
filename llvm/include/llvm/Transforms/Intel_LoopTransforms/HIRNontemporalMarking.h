//===------------------ HIRNontemporalMarking.h ----------------*- C++-*---===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// A pass for identifying loads and stores that could benefit from being
// nontemporal.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRNONTEMPORALMARKING_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRNONTEMPORALMARKING_H

#include "llvm/IR/PassManager.h"

namespace llvm {

namespace loopopt {

/// An HIR pass which performs merged load/store motion.
class HIRNontemporalMarkingPass : public PassInfoMixin<HIRNontemporalMarkingPass> {
public:
  PreservedAnalyses run(Function &, FunctionAnalysisManager &);
};

} // namespace loopopt

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRNONTEMPORALMARKING_H
