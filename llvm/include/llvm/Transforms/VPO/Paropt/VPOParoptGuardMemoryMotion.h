#if INTEL_COLLAB // -*- C++ -*-
//===------------------ VPOParoptGuardMemoryMotion ------------------------===//
//
//   Copyright (C) 2022 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This header file declares a pass that adds directives to guard memory motion
/// of OMP SIMD clause values across parent loop.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_PAROPT_GUARD_MEMORY_MOTION_H
#define LLVM_TRANSFORMS_VPO_PAROPT_GUARD_MEMORY_MOTION_H

#include "llvm/IR/PassManager.h"

namespace llvm {

/// Insert directives to guard memory motion of OMP SIMD clause variables (like
/// user-defined reductions) across parent loop. The guarded region is
/// prescribed by begin and end directives which are respectively added to loop
/// header and loop latch BBs. Values that need to be guarded in the region are
/// captured via QUAL.OMP.LIVEIN clauses.
class VPOParoptGuardMemoryMotionPass
    : public PassInfoMixin<VPOParoptGuardMemoryMotionPass> {
public:
  VPOParoptGuardMemoryMotionPass() = default;

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
  static bool isRequired() { return true; }
};
} // end namespace llvm
#endif // LLVM_TRANSFORMS_VPO_PAROPT_GUARD_MEMORY_MOTION_H
#endif // INTEL_COLLAB
