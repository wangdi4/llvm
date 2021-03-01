//===-- IntelVPlanMaskedVModeLoop.h ---------------------------------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file creates a masked variant for the current loop. This is done by
// wrapping the loop body under an if statement as it is shown below. The if
// statement triggers the predicator which emits the masked code. The following
// example shows how the following code is transformed:
//
// Code before transformation:
// entry:
//   br label %header
// header:
//   %iv = phi i32 [ 0, %preheader ], [ %iv.next, %latch ]
//   %add.phi = phi i32 [ 0, %preheader ], [ %add, %latch ]
//   %iv.next = add nsw i32 %iv, 1
//   br label %latch
// latch:
//   %add = add nsw i32 %add.phi, 1
//   %bottom_test = icmp lt i32 %iv.next, 128
//   br i1 %bottom_test, label %header, label %loopexit
// loopexit:
//   %lcssa.phi = phi i32 [ %add, %latch ]
//   %add.final = add nsw i32 %lcssa.phi, 1
//   br label %exit
// exit:
//   ret void
//
// The following steps transforms the code:
// 1. If it is needed, the induction increment is moved to the latch. Next,
// the latch is split just before the induction increment.
// 2. The header is split after the last phi.
// 3. A new comparison and branch is emitted in the header. The comparison will
// be updated by later pass.
// 4. For each recurrent value and live-out, a phi node is emitted in the new
// loop latch.
//
// Code after transformation:
// entry:
//   br label %header
// header:
//   %iv = phi i32 [ 0, %preheader ], [ %iv.next, %new_latch ]
//   %add.phi = phi i32 [ 0, %preheader ], [ %new.add.phi, %new_latch ]
//   %cond = icmp lt i32 %iv, 128
//   br i1 %cond, label %bb1, label %new_latch
// bb1:
//   %iv.next = add nsw i32 %iv, 1
//   br label %latch
// latch:
//   %add = add nsw i32 %add.phi, 1
//   br label %new_latch
// new_latch:
//   %new.add.phi = phi i32 [ %add, %latch ], [ %add.phi, %header ]
//   %bottom_test = icmp lt i32 %iv.next, 128
//   br i1 %bottom_test, label %header, label %loopexit
// loopexit:
//   %lcssa.phi = phi i32 [ %new.add.phi, %new_latch ]
//   %add.final = add nsw i32 %lcssa.phi, 1
//   br label %exit
// exit:
//   ret void
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANMASKEDMODELOOP_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANMASKEDMODELOOP_H

#include "llvm/Support/CommandLine.h"
#include <memory>

extern llvm::cl::opt<bool> EnableMaskedVariant;

namespace llvm {
namespace vpo {

class VPlanNonMasked;
class VPlanMasked;
class VPAnalysesFactory;
class VPInstruction;
class VPLoop;

class MaskedModeLoopCreator {
  VPlanNonMasked *OrigVPlan = nullptr;
  VPAnalysesFactory &VPAF;

public:
  MaskedModeLoopCreator(VPlanNonMasked *OrigVPlan, VPAnalysesFactory &VPAF)
      : OrigVPlan(OrigVPlan), VPAF(VPAF) {}

  // Emit masked mode loop.
  std::shared_ptr<VPlanMasked> createMaskedModeLoop();

private:
  // Checks whether masked mode loop is needed.
  bool mayUseMaskedMode();

  // Returns loop's induction. Currently, we do not process outer loops that are
  // not for-loops.
  VPInstruction *getInductionVariable(VPLoop *TopVPLoop);
};
} // namespace vpo
} // namespace llvm

#endif // LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTELVPLANMASKEDMODELOOP_H
