//===- IntelVPlanIdioms.h -------------------------------------------------===//
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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

namespace loopopt {
class RegDDRef;
class HLIf;
}

namespace vpo {

class VPlanIdioms {
public:
  enum Opcode {
    Unknown,
    Unsafe,
    // Search loop idioms:
    SearchLoop,
    SearchLoopStrEq,
    SearchLoopPtrEq,
    // End of search loop idioms
  };

  /// Return \p Opcode that describes specific search loop idiom, or return
  /// VPlanIdioms::Unsafe if unsafe instructions are found in the pattern, or
  /// VPlanIdioms::Unknown if the loop doesn't fit into any known pattern. If
  /// the identified pattern requires peeling of an array for aligned accesses,
  /// then return it in \p PeelArrayRef.
  static Opcode isSearchLoop(const VPlanVector *Plan,
                             const bool CheckSafety,
                             loopopt::RegDDRef *&PeelArrayRef);
  static bool isAnySearchLoop(const VPlanVector *Plan,
                              const bool CheckSafety);

  static bool isAnySearchLoop(const VPlanIdioms::Opcode Opcode) {
    return Opcode == VPlanIdioms::SearchLoopStrEq ||
           Opcode == VPlanIdioms::SearchLoopPtrEq ||
           Opcode == VPlanIdioms::SearchLoop;
  }

private:
  static bool isSafeLatchBlockForSearchLoop(const VPBasicBlock *Block);
  static Opcode isStrEqSearchLoop(const VPBasicBlock *Block,
                                  const bool AllowMemorySpeculation);
  static Opcode isPtrEqSearchLoop(const VPBasicBlock *Block,
                                  const bool AllowMemorySpeculation,
                                  loopopt::RegDDRef *&PeelArrayRef);
  static bool checkPtrEqThenNodes(const loopopt::HLIf *If,
                                  const loopopt::RegDDRef *ListItemRef);
  static bool isSafeExitBlockForSearchLoop(const VPBasicBlock *Block);
};
} // namespace vpo

} // namespace llvm

#endif // LLVM_TRANSFORM_VECTORIZE_INTEL_VPLAN_INTELVPLANIDIOMS_H
