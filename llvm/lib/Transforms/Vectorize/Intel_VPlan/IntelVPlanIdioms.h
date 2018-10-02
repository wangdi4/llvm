//===- IntelVPlanIdioms.h -------------------------------------------------===//
//
//   Copyright (C) 2018 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// TODO:
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLANIDIOMS_H
#define LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLANIDIOMS_H

#include "IntelVPlan.h"

namespace llvm {

namespace vpo {

class VPlanIdioms {
public:
  enum Opcode {
    Unknown,
    Unsafe,
    // Search loop idioms:
    SearchLoop,
    SearchLoopStrEq,
    // End of search loop idioms
  };

  /// Return \p Opcode that describes specific search loop idiom, or return
  /// VPlanIdioms::Unsafe if unsafe instructions are found in the pattern, or
  /// VPlanIdioms::Unknown if the loop doesn't fit into any known pattern.
  static Opcode isSearchLoop(const VPlan *Plan, const unsigned VF,
                             const bool CheckSafity);
  static bool isAnySearchLoop(const VPlan *Plan, const unsigned VF,
                              const bool CheckSafity);

private:
  static bool isSafeLatchBlockForSearchLoop(const VPBasicBlock *Block);
  static Opcode isStrEqSearchLoop(const VPBasicBlock *Block,
                                  const bool AllowMemorySpeculation);
  static bool isSafeBlockForSearchLoop(const VPBasicBlock *Block);
  static bool isSafeExitBlockForSearchLoop(const VPBasicBlock *Block);
};
} // namespace vpo

} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLANIDIOMS_H
