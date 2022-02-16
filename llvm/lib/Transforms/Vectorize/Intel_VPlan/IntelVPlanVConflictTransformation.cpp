//===-- IntelVPlanVConflictIdiomTransformation.cpp ------------------------===//
//
//   Copyright (C) Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements VConflict idiom recognition. In addition, it lowers
/// each VConflict idiom to the corresponding LLVM-IR.
///
//===----------------------------------------------------------------------===//

#include "IntelVPlanVConflictTransformation.h"
#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"

#define DEBUG_TYPE "VConflictTransformation"

using namespace llvm;
using namespace llvm::vpo;

static LoopVPlanDumpControl
    VPlanLowerVConflictIdiomControl("optimize-vconflict-idiom",
                                    "VPlanOptimizeVConflictIdiom");

namespace llvm {
namespace vpo {

// For the following Histogram example:
//
//   for (int i=0; i<N; i++){
//     index = B[i];
//     A[index] = A[index] + 100;
//   }
//
// To calculate the conflicts in the array A, we use vpconflict and vpopcnt
// instructions. First, vpconflict detects the conflicts in the array. For each
// element of the array, vpconflict finds how many times the current value
// appears in the previous elements. Next, vpopcnt calculates the number of
// conflicts for each lane. The outcome of vpopcnt is incremented across all
// lanes by 1. This is because all lanes should be counted and vpconflict
// excludes the first (non-conflict) lane. Next, we multiply the number of
// conflicts by the uniform value. This is better than incrementing each value
// each time. After that, we load array A and add it to the outcome of the
// multiplication. The result is stored back to A by using scatter. Scatter
// updates each lane separately and for conflict indexes, it stores the latest
// value.
//
// The code after processVConflictIdiom will be:
//
// i64 %vp.vconflict.index = sext i32 %vp.load.B to i64
// i32* %A.subscript = subscript inbounds i32* %A i64 %vp.vconflict.index
// i32 %vp.load.A = load i32* %A.subscript
// i64 %B.sext = sext i32 %vp.load.B to i64
// i32* %A.subscript = subscript inbounds i32* %A i64 %B.sext
// i32 %vp.tree.conflict = tree-conflict i64 %vp.vconflict.index i32 %vp.load.A
//                          i32 100 { Redux Opcode: add }
// store i32 %vp.tree.conflict i32* %A.subscript
//
// and the code after optimizing VConflict idiom to Histogram will be:
//
// i64 %vp.vconflict.index = sext i32 %vp.load.B to i64
// i32* %A.subscript = subscript inbounds i32* %A i64 %vp.vconflict.index
// i32 %vp.load.A = load i32* %A.subscript
// i64 %B.sext = sext i32 %vp.load.B to i64
// i32* %A.subscript = subscript inbounds i32* %A i64 %B.sext
//
// VConflict instruction finds the conflicting lanes in array A:
// i64 %vp.vpconfict.intrinsic = vpconflict-insn i64 %vp.vconflict.index
//
// VPopcnt finds the number of conflicts in each lane:
// i64 %vp.pop.count = call i64 %vp.vpconfict.intrinsic llvm.ctpop.v2i64 [x 1]
// i32 %trunc = trunc i64 %vp.pop.count to i32
//
// VConflict does not count the current lane. Hence, we have to add 1 to the
// number of conflicts:
// i32 %add = add i32 %trunc i32 1
//
// Multiply the number of conflicts with 100:
// i32 %mul = mul i32 %add1 i32 100
//
// Add the result to A[index]:
// i32 %res = add i32 %vp.load.A i32 %vp62140
//
// Store the result:
// store i32 %res i32* %A.subscript
//
static bool lowerHistogram(VPTreeConflict *TreeConflict, Function &Fn) {
  VPlan *Plan = TreeConflict->getParent()->getParent();
  auto *DA = Plan->getVPlanDA();

  VPBuilder VPBldr;
  VPBldr.setInsertPoint(TreeConflict);
  VPInstruction *ConflictIndex =
      cast<VPInstruction>(TreeConflict->getConflictIndex());

  // Detect conflicts.
  auto *ConflictInst = VPBldr.create<VPConflictInsn>(
      "vpconfict.intrinsic", ConflictIndex->getType(), ConflictIndex);
  DA->markDivergent(*ConflictInst);

  // Emit pop count intrinsic.
  auto *PopCountFn = Intrinsic::getDeclaration(Fn.getParent(), Intrinsic::ctpop,
                                               {ConflictInst->getType()});
  auto *PopCountCall =
      VPBldr.create<VPCallInstruction, const Twine &, VPValue *, FunctionType *,
                    ArrayRef<VPValue *>>(
          "vp.pop.count", Plan->getVPConstant(PopCountFn),
          PopCountFn->getFunctionType(), {ConflictInst});
  DA->markUniform(*PopCountCall->getCalledValue());
  PopCountCall->setVectorizeWithIntrinsic(Intrinsic::ctpop);
  DA->markDivergent(*PopCountCall);

  // Get the uniform reduction update value.
  VPValue *UpdateVal = TreeConflict->getRednUpdateOp();
  assert(DA->isUniform(*UpdateVal) &&
         "Live-in value for histogram expected to be uniform.");

  auto CalcTy = UpdateVal->getType();
  VPValue *PopCountCallCast = nullptr;
  if (PopCountCall->getType() != CalcTy) {
    if (CalcTy->isFloatingPointTy()) {
      PopCountCallCast = VPBldr.createSIToFp(PopCountCall, CalcTy);
    } else {
      assert(CalcTy->getPrimitiveSizeInBits() > 4 &&
             "Truncate does not work for smaller data types.");
      PopCountCallCast = VPBldr.createZExtOrTrunc(PopCountCall, CalcTy);
    }
    DA->markDivergent(*PopCountCallCast);
  }

  // Add 1 to pop count.
  VPValue *Add = nullptr;
  if (CalcTy->isFloatingPointTy())
    Add = VPBldr.createFAdd(
        PopCountCallCast ? PopCountCallCast : PopCountCall,
        Plan->getVPConstant(ConstantFP::get(UpdateVal->getType(), 1)));
  else
    Add = VPBldr.createAdd(
        PopCountCallCast ? PopCountCallCast : PopCountCall,
        Plan->getVPConstant(ConstantInt::get(UpdateVal->getType(), 1)));
  DA->markDivergent(*Add);

  // Multiply the Add with the uniform value(UpdateVal).
  VPValue *Mul = nullptr;
  unsigned Opcode = TreeConflict->getRednOpcode();
  if (Opcode == Instruction::FAdd)
    Mul = VPBldr.createFMul(Add, UpdateVal);
  else
    Mul = VPBldr.createMul(Add, UpdateVal);
  DA->markDivergent(*Mul);

  // Update VConflictLoad with the result using RednOpcode operation.
  auto *VConflictLoad = TreeConflict->getConflictLoad();
  VPValue *Res = nullptr;
  if (Opcode == Instruction::FAdd)
    Res = VPBldr.createFAdd(VConflictLoad, Mul);
  else
    Res = VPBldr.createAdd(VConflictLoad, Mul);
  DA->markDivergent(*Res);

  // Update VConflictStore with the outcome of VConflict pattern. TODO: Can we
  // use RAUW instead?
  auto *VConflictStore = *TreeConflict->users().begin();
  assert(TreeConflict->getNumUsers() == 1 &&
         "VPTreeConflict can have only one user.");
  VConflictStore->setOperand(0, Res);
  // Remove VPTreeConflict instruction.
  auto *CurrentBB = TreeConflict->getParent();
  CurrentBB->eraseInstruction(TreeConflict);

  return true;
}

VPTreeConflict *
tryReplaceWithTreeConflict(VPGeneralMemOptConflict *VPConflict) {
  VPRegion *ConflictRegion = VPConflict->getRegion();

  // Tree-conflict idiom is expected to have only one BB in conflict region.
  if (ConflictRegion->getSize() != 1)
    return nullptr;

  auto *VPBB = *(ConflictRegion->getBBs().begin());
  // Tree-conflict idiom is expected to have only one instruction in conflict
  // region.
  if (VPBB->size() != 1)
    return nullptr;
  VPInstruction *InsnInVConflictRegion = &*VPBB->begin();

  // Check if the opcode of the single instruction in VConflict region is a
  // reduction operation. This is a requirement for both tree-conflict and
  // histogram.
  auto HasReductionOpcode = [](VPInstruction *I) -> bool {
    switch (I->getOpcode()) {
    case Instruction::Add:
      return true;
    case Instruction::FAdd:
      // We need fast math enabled in order to use the optimized histogram to
      // generate the following code
      // a[index] = a[index] + addVal * numConflicts
      return I->hasFastMathFlags();
      // TODO: Add support for other reduction operations.
    default:
      return false;
    }
  };

  if (!HasReductionOpcode(InsnInVConflictRegion)) {
    LLVM_DEBUG(dbgs() << "Tree-conflict idiom has unsupported opcode.\n");
    return nullptr;
  }

  unsigned RednOpcode = InsnInVConflictRegion->getOpcode();
  VPValue *RednUpdateOp = nullptr;
  if (VPConflict->getNumOperands() == 3) {
    // If there is no live-in value, then we have a constant or external def
    // updating value.
    VPValue *Op0 = InsnInVConflictRegion->getOperand(0);
    VPValue *Op1 = InsnInVConflictRegion->getOperand(1);
    RednUpdateOp = Op0 == *ConflictRegion->getLiveIns().begin() ? Op1 : Op0;
    assert(isa<VPConstant>(RednUpdateOp) ||
           isa<VPExternalDef>(RednUpdateOp) &&
               "Constant or external definition is expected.");
  } else {
    assert(VPConflict->getNumOperands() == 4 &&
           "Unexpected number of operands for tree-conflict.");
    // If the conflict instruction has a live-in value, then it is the operand
    // updating the reduction.
    RednUpdateOp = *VPConflict->getLiveIns().begin();
  }

  // Create VPTreeConflict instruction.
  VPlan *Plan = VPConflict->getParent()->getParent();
  auto *DA = Plan->getVPlanDA();
  auto *TreeConflict =
      VPBuilder()
          .setInsertPoint(VPConflict)
          .create<VPTreeConflict>(
              "vp.tree.conflict", VPConflict->getConflictIndex(),
              VPConflict->getConflictLoad(), RednUpdateOp, RednOpcode);
  VPConflict->replaceAllUsesWith(TreeConflict);
  // Update DA for the new tree-conflict instruction.
  DA->updateDivergence(*TreeConflict);

  // Remove VPConflict instruction.
  auto *CurrentBB = VPConflict->getParent();
  CurrentBB->eraseInstruction(VPConflict);

  return TreeConflict;
}

// Checks what kind of VConflict type we have and it lowers it accordingly.
bool processVConflictIdiom(VPGeneralMemOptConflict *VPConflict, Function &Fn) {
  auto *DA = VPConflict->getParent()->getParent()->getVPlanDA();
  if (auto *TreeConflict = tryReplaceWithTreeConflict(VPConflict)) {
    // Try to lower optimized tree-conflict idioms to histogram. This is
    // feasible only if the reduction updating operand is uniform.
    if (DA->isUniform(*TreeConflict->getRednUpdateOp()))
      return lowerHistogram(TreeConflict, Fn);

    return true;
  }

  // TODO: Add support for all kinds of general conflict, general conflict
  // optimized, multiple histograms (via CSE) and histogram with loop.
  return false;
}

bool processVConflictIdiom(VPlan &Plan, Function &Fn) {
  for (auto &VPInst : make_early_inc_range(vpinstructions(&Plan)))
    if (auto *VPConflict = dyn_cast<VPGeneralMemOptConflict>(&VPInst))
      if (!processVConflictIdiom(VPConflict, Fn))
        return false;

  VPLAN_DUMP(VPlanLowerVConflictIdiomControl, Plan);

  // TODO: Remove below loop after adding lowering and CG support for
  // VPTreeConflict.
  for (auto &VPInst : make_early_inc_range(vpinstructions(&Plan)))
    if (isa<VPTreeConflict>(&VPInst))
      return false;

  return true;
}
} // namespace vpo
} // namespace llvm
