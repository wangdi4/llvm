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
// the code after Decomposer will be:
//
// i64 %vp.vconflict.index = sext i32 %vp.load.B to i64
// i32* %A.subscript = subscript inbounds i32* %A i64 %vp.vconflict.index
// i32 %vp.load.A = load i32* %A.subscript
// i64 %B.sext = sext i32 %vp.load.B to i64
// i32* %A.subscript = subscript inbounds i32* %A i64 %B.sext
// i32 %vp.general.mem.opt.conflict = vp-general-mem-opt-conflict i64
// %vp.vconflict.index void %vp.conflict.region i32 %vp.load.A i32 100 ->
// VConflictRegion (i32 %vp.live.in0 i32 %vp.live.in1 ) {
//  value : none
//  mask : none
//  live-in : i32 %vp.live.in0
//  live-in : i32 %vp.live.in1
//  Region:
//  VConflictBB
//  i32 %vp3660 = add i32 %vp.live.in0 i32 %vp.live.in1
//  live-out : i32 %vp3660 = add i32 %vp.live.in0 i32 %vp.live.in1
//  }
// store i32 %vp.general.mem.opt.conflict i32* %A.subscript
//
// and the code after optimizing VConflict idiom will be:
//
// i64 %vp.vconflict.index = sext i32 %vp.load.B to i64
// i32* %A.subscript = subscript inbounds i32* %A i64 %vp.vconflict.index
// i32 %vp.load.A = load i32* %A.subscript
// i64 %B.sext = sext i32 %vp.load.B to i64
// i32* %A.subscript = subscript inbounds i32* %A i64 %B.sext
// VConlfict instruction finds the conflicting lanes in array A:
// i64 %vp.vpconfict.intrinsic = vpconflict-insn i64 %vp.vconflict.index
// VPopcnt finds the number of conflicts in each lane:
// i64 %vp.pop.count = call i64 %vp.vpconfict.intrinsic llvm.ctpop.v2i64 [x 1]
// i32 %trunc = trunc i64 %vp.pop.count to i32
// VConflict does not count the current lane. Hence, we have to add 1 to the
// number of conflicts:
// i32 %add = add i32 %trunc i32 1
// Multiply the number of conflicts with 100:
// i32 %mul = mul i32 %add1 i32 100
// Add the result to A[index]:
// i32 %res = add i32 %vp.load.A i32 %vp62140
// Store the result:
// store i32 %res i32* %A.subscript
//
static bool lowerHistogram(VPGeneralMemOptConflict *VPConflict, Function &Fn) {
  assert(VPConflict->getNumOperands() == 4 && "Histogram has 4 operands.");
  VPBuilder VPBldr;
  VPBldr.setInsertPoint(VPConflict);
  VPInstruction *ConflictIndex =
      cast<VPInstruction>(VPConflict->getConflictIndex());

  SmallVector<VPValue *, 2> EmittedInsns;
  auto *ConflictInst = VPBldr.create<VPConflictInsn>(
      "vpconfict.intrinsic", ConflictIndex->getType(), ConflictIndex);
  EmittedInsns.push_back(ConflictInst);
  // Emit pop count intrinsic.
  VPlan *Plan = VPConflict->getParent()->getParent();
  auto *PopCountCall = VPBldr.create<VPCallInstruction, const Twine &,
                                     FunctionCallee, ArrayRef<VPValue *>>(
      "vp.pop.count",
      Intrinsic::getDeclaration(Fn.getParent(), Intrinsic::ctpop,
                                {ConflictInst->getType()}),
      {ConflictInst}, Plan);
  Plan->getVPlanDA()->markUniform(*PopCountCall->getCalledValue());
  PopCountCall->setVectorizeWithIntrinsic(Intrinsic::ctpop);
  EmittedInsns.push_back(PopCountCall);

  // Get the value that we need to multiply.
  auto *MulVal = VPConflict->getOperand(3);

  auto CalcTy = MulVal->getType();
  VPValue *PopCountCallCast = nullptr;
  if (PopCountCall->getType() != CalcTy) {
    if (CalcTy->isFloatingPointTy()) {
      PopCountCallCast = VPBldr.createSIToFp(PopCountCall, CalcTy);
    } else {
      assert(CalcTy->getPrimitiveSizeInBits() > 4 &&
             "Truncate does not work for smaller data types.");
      PopCountCallCast = VPBldr.createZExtOrTrunc(PopCountCall, CalcTy);
    }
    EmittedInsns.push_back(PopCountCallCast);
  }

  // Add 1 to pop count.
  VPValue *Add = nullptr;
  if (CalcTy->isFloatingPointTy())
    Add = VPBldr.createFAdd(
        PopCountCallCast ? PopCountCallCast : PopCountCall,
        Plan->getVPConstant(ConstantFP::get(MulVal->getType(), 1)));
  else
    Add = VPBldr.createAdd(
        PopCountCallCast ? PopCountCallCast : PopCountCall,
        Plan->getVPConstant(ConstantInt::get(MulVal->getType(), 1)));
  EmittedInsns.push_back(Add);

  // Historgram has only one instruction in its region.
  VPInstruction *InsnInVConflictRegion =
      &*(*VPConflict->getRegion()->getBBs().begin())->begin();
  unsigned Opcode = InsnInVConflictRegion->getOpcode();

  // Multiply the Add with the uniform value(MulVal).
  VPValue *Mul = nullptr;
  if (Opcode == Instruction::FAdd)
    Mul = VPBldr.createFMul(Add, MulVal);
  else
    Mul = VPBldr.createMul(Add, MulVal);
  EmittedInsns.push_back(Mul);

  // Add the result to VConflictLoad.
  auto *VConflictLoad = VPConflict->getOperand(2);
  VPValue *Res = nullptr;
  if (Opcode == Instruction::FAdd)
    Res = VPBldr.createFAdd(VConflictLoad, Mul);
  else
    Res = VPBldr.createAdd(VConflictLoad, Mul);
  EmittedInsns.push_back(Res);

  // Update VConflictStore with the outcome of VConflict pattern.
  auto *VConflictStore = *VPConflict->users().begin();
  assert(VPConflict->getNumUsers() == 1 &&
         "VPConflict can have only one user.");
  VConflictStore->setOperand(0, Res);
  // Remove VPConflict instruction.
  auto *CurrentBB = VPConflict->getParent();
  CurrentBB->eraseInstruction(VPConflict);
  // Update DA of the new instructions.
  for (auto &VPI : EmittedInsns)
    Plan->getVPlanDA()->markDivergent(*VPI);

  return true;
}

// Checks what kind of VConflict type we have and it lowers it accordingly.
// TODO: Add support for all kinds of general conflict, general conflict
// optimized, tree-conflict, multiple histograms and histogram with loop.
bool processVConflictIdiom(VPGeneralMemOptConflict *VPConflict, Function &Fn) {
  // Check if the opcode of the instrution in Histogram is a reduction
  // operation. This is a requirement for both tree-conflict and histogram.
  SmallVector<VPInstruction *, 1> RgnInsns;
  for (auto *VPBB : VPConflict->getRegion()->getBBs())
    for (auto &VPI : *VPBB)
      RgnInsns.push_back(&VPI);

  if (RgnInsns.size() != 1)
    return false;

  bool HasReductionOpcode = llvm::any_of(RgnInsns, [](VPInstruction *I) {
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
    return false;
  });

  if (!HasReductionOpcode) {
    LLVM_DEBUG(dbgs() << "VConflict idiom has unsupported opcode.\n");
    return false;
  }

  VPlan *Plan = VPConflict->getParent()->getParent();
  auto *DA = Plan->getVPlanDA();
  bool HasUniformValue =
      llvm::all_of(VPConflict->getliveins(),
                   [DA](VPValue *Val) { return DA->isUniform(*Val); });

  // The VConflict idiom is histogram when its region has only live-in which is
  // uniform.
  if (HasUniformValue)
    return lowerHistogram(VPConflict, Fn);

  return false;
}

bool processVConflictIdiom(VPlan &Plan, Function &Fn) {
  for (auto &VPInst : make_early_inc_range(vpinstructions(&Plan)))
    if (auto *VPConflict = dyn_cast<VPGeneralMemOptConflict>(&VPInst))
      if (!processVConflictIdiom(VPConflict, Fn))
        return false;

  VPLAN_DUMP(VPlanLowerVConflictIdiomControl, Plan);
  return true;
}
} // namespace vpo
} // namespace llvm
