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
#include "IntelVPlanLoopInfo.h"
#include "IntelVPlanBuilder.h"

#define DEBUG_TYPE "VConflictTransformation"

using namespace llvm;
using namespace llvm::vpo;

static LoopVPlanDumpControl
    VPlanLowerVConflictIdiomControl("optimize-vconflict-idiom",
                                    "VPlanOptimizeVConflictIdiom");
static LoopVPlanDumpControl
    VPlanLowerTreeConflictControl("lower-tree-conflict",
                                  "VPlanLowerTreeConflict");

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
  if (Opcode == Instruction::FAdd || Opcode == Instruction::FSub) {
    Mul = VPBldr.createFMul(Add, UpdateVal);
    cast<VPInstruction>(Mul)->setFastMathFlags(
        TreeConflict->getFastMathFlags());
  } else {
    assert((Opcode == Instruction::Add || Opcode == Instruction::Sub) &&
           "Expected add or sub opcode");
    Mul = VPBldr.createMul(Add, UpdateVal);
  }
  DA->markDivergent(*Mul);

  // Update VConflictLoad with the result using RednOpcode operation.
  auto *VConflictLoad = TreeConflict->getConflictLoad();
  VPValue *Res = nullptr;
  if (Opcode == Instruction::FAdd) {
    Res = VPBldr.createFAdd(VConflictLoad, Mul);
    cast<VPInstruction>(Res)->setFastMathFlags(
        TreeConflict->getFastMathFlags());
  } else if (Opcode == Instruction::FSub) {
    Res = VPBldr.createFSub(VConflictLoad, Mul);
    cast<VPInstruction>(Res)->setFastMathFlags(
        TreeConflict->getFastMathFlags());
  } else if (Opcode == Instruction::Add)
    Res = VPBldr.createAdd(VConflictLoad, Mul);
  else {
    assert(Opcode == Instruction::Sub && "Expected sub opcode");
    Res = VPBldr.createSub(VConflictLoad, Mul);
  }
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
  auto HasReductionOpcode = [ConflictRegion](VPInstruction *I) -> bool {
    switch (I->getOpcode()) {
    case Instruction::Add:
      return true;
    case Instruction::Sub:
      // sub not commutative, so enforce a[idx] = a[idx] - C.
      // Same for fsub below
      if (I->getOperand(1) == *ConflictRegion->getLiveIns().begin())
        return false;
      return true;
    case Instruction::FAdd:
      // We need fast math enabled in order to use the optimized histogram to
      // generate the following code
      // a[index] = a[index] + addVal * numConflicts
      return I->hasFastMathFlags();
    case Instruction::FSub: {
      // Here, we need the renamed value of VPConflictLoad. The first live-in of
      // the conflict region is the renamed value of VPConflictLoad.
      if (I->getOperand(1) == *ConflictRegion->getLiveIns().begin())
        return false;
      return I->hasFastMathFlags();
    }
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
  if (InsnInVConflictRegion->hasFastMathFlags())
    TreeConflict->setFastMathFlags(InsnInVConflictRegion->getFastMathFlags());
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

    // General Requirements met to enable TreeConflict lowering to double
    // permute tree reduction sequence. This happens just before codegen.
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
  return true;
}

static VPValue* convertValueType(VPValue *Val, Type *ToTy, VPBuilder &VPBldr) {
  Type *ValType = Val->getType();
  if (ValType == ToTy)
    return Val;
  if (ValType->getPrimitiveSizeInBits() == ToTy->getPrimitiveSizeInBits())
    return VPBldr.createNaryOp(Instruction::BitCast, ToTy, {Val});
  if (ValType->isIntegerTy() && ToTy->isIntegerTy())
    return VPBldr.createIntCast(Val, ToTy);
  if (ValType->isFloatingPointTy() && ToTy->isFloatingPointTy())
    return VPBldr.createFPCast(Val, ToTy);
  if (ValType->isIntegerTy() && ToTy->isFloatingPointTy())
    return VPBldr.createIntToFPCast(Val, ToTy);
  if (ValType->isFloatingPointTy() && ToTy->isIntegerTy())
    return VPBldr.createFPToIntCast(Val, ToTy);
  llvm_unreachable("Unsupported conversion operation");
}

// To lower TreeConflict to double permute tree reduction, the types of the
// control and result operands must have the same sizes because that's how the
// permute intrinsics are defined. Note: the type of the control operand is
// based on the type of the conflict idx. This function returns the size in bits
// that should be used for each operand of the permutes within the conflict
// loop. Decomposition of subscript instructions results in two possible type
// sizes for the conflict idx, the original type size and i64 since the
// decomposition currently promotes conflict idx to i64 for use in subscript
// instructions. Thus, we may have something like the following as input to
// this function.
//
// Original conflict idx type: i32
// Promoted conflict idx type: i64
// Reduction update op type (result type) : i32
//
// This function would return 32 for both the conflict idx and reduction
// update op type sizes. The corresponding conflict loop phis and instructions
// would then be based on this.
Optional<unsigned> getPermuteTypeSize(const VPTreeConflict *TreeConflict,
                                      unsigned VF) {
  auto *RednUpdateOpTy = TreeConflict->getRednUpdateOp()->getType();
  unsigned PermuteSize = RednUpdateOpTy->getScalarSizeInBits();

  // We don't yet support type sizes less than 32-bits. However, we do support
  // cases where we know at least one of the conflict idx or reduction op types
  // are at least 32-bit because the size returned will be 32. E.g., if conflict
  // idx is i16, but reduction op is i32, then we'll promote the conflict idx
  // for use in the permute intrinsics.
  if (PermuteSize < 32)
    return None;

  // The following cases are where we don't have a direct intrinsic mapping for
  // the size and VF. We'll need support for partial vectors and pumping.
  if (VF < 4)
    return None;

  if (PermuteSize == 64 && VF > 8)
    return None;

  if (PermuteSize == 32 && VF > 16)
    return None;

  return PermuteSize;
}

VPValue* createPermuteIntrinsic(StringRef Name, Type *Ty, VPValue *PermuteVals,
                                VPValue *Control, VPBuilder &VPBldr,
                                LLVMContext &C, unsigned VF,
                                VPlanDivergenceAnalysis *DA) {
  // Permute intrinsic for int where VF=4 doesn't exist, so bitcast to float
  // and use the float version. Cast back after permuting since users will
  // expect the original type.
  if (Ty->isIntegerTy(32) && VF == 4) {
    auto *Cast = VPBldr.createNaryOp(Instruction::BitCast, Type::getFloatTy(C),
                                     {PermuteVals});
    DA->markDivergent(*Cast);
    auto *Permute =
        VPBldr.create<VPPermute>(Name, Cast->getType(), Cast, Control);
    DA->markDivergent(*Permute);
    return VPBldr.createNaryOp(Instruction::BitCast, IntegerType::get(C, 32),
                               {Permute});
  } else
    return VPBldr.create<VPPermute>(Name, Ty, PermuteVals, Control);
}

// Do the actual tree conflict lowering using the double permute tree reduction
// algorithm described in <TODO: put link location here>. Due to the size of
// the IR generated, please refer to vplan_lower_tree_conflict*.ll tests. The
// Vlan dump can also be used with these tests through
// VPlanLowerTreeConflictControl (-vplan-print-after-lower-tree-conflict)
bool lowerTreeConflictsToDoublePermuteTreeReduction(VPlanVector *Plan,
                                                    unsigned VF, Function &Fn) {

  VPBuilder VPBldr;
  auto *DA = Plan->getVPlanDA();
  auto *VPLI = Plan->getVPLoopInfo();
  auto *C = Plan->getLLVMContext();
  SmallVector<VPTreeConflict *, 2> TreeConflicts;
  bool TreeConflictsLowered = false;

  for (auto &VPInst : vpinstructions(Plan))
    if (auto *TreeConflict = dyn_cast<VPTreeConflict>(&VPInst))
      TreeConflicts.push_back(TreeConflict);

  for (auto *TreeConflict : TreeConflicts) {
    assert(TreeConflict->getNumUsers() == 1 &&
           cast<VPInstruction>(*TreeConflict->users().begin())->getOpcode() ==
               Instruction::Store &&
               "VPTreeConflict can only have a store user.");

    auto *TreeConflictParent = TreeConflict->getParent();
    auto *Pred = TreeConflictParent->getPredicate();

    // Generate the conflict loop.
    auto SplitIt = TreeConflict->getIterator();
    auto *ConflictPreheader =
        VPBlockUtils::splitBlock(TreeConflictParent, SplitIt, VPLI,
                                 Plan->getDT(),
                                 Plan->getPDT());

    ++SplitIt;
    auto *PostConflict =
        VPBlockUtils::splitBlock(ConflictPreheader, SplitIt, VPLI,
                                 Plan->getDT(),
                                 Plan->getPDT());

    // OuterLoop latch; not referenced
    ++SplitIt;
    VPBlockUtils::splitBlock(PostConflict, SplitIt, VPLI, Plan->getDT(),
                             Plan->getPDT());

    auto *ConflictHeader =
        VPBlockUtils::splitBlockEnd(ConflictPreheader, VPLI, Plan->getDT(),
                                    Plan->getPDT());

    auto *ConflictExit =
        VPBlockUtils::splitBlockEnd(ConflictHeader, VPLI, Plan->getDT(),
                                    Plan->getPDT());

    VPLoop *ConflictLoop = VPLI->AllocateLoop();
    VPLoop *ParentLoop = VPLI->getLoopFor(TreeConflictParent);
    ParentLoop->addChildLoop(ConflictLoop);
    VPLI->changeLoopFor(ConflictHeader, ConflictLoop);
    ConflictLoop->addBlockEntry(ConflictHeader);

    // Begin inserting pre-conflict loop instructions.
    VPBldr.setInsertPoint(TreeConflictParent->getTerminator());

    Optional<unsigned> PermuteSize = getPermuteTypeSize(TreeConflict, VF);
    assert(PermuteSize != None && "Expected valid permute intrinsic size");
    auto *ConflictIdx = TreeConflict->getConflictIndex();
    auto *ConflictIdxTy = ConflictIdx->getType();
    Type *PermuteTy = IntegerType::get(*C, *PermuteSize);
    VPValue *RednUpdateOp = TreeConflict->getRednUpdateOp();
    Type *RednUpdateOpTy = RednUpdateOp->getType();

    auto *VConf = VPBldr.create<VPConflictInsn>(
        "vpconflict.intrinsic", ConflictIdxTy,
        cast<VPInstruction>(TreeConflict->getConflictIndex()));
    DA->markDivergent(*VConf);

    // Set llvm.ctlz is_zero_undef = 0. If VConf results in all zeros, then
    // the result of the intrinsic should be the number of bits in VConf. E.g.,
    // if VConf is 32-bits then the result of ctlz should be 32 and not undef.
    Type *Ty1 = Type::getInt1Ty(*C);
    VPConstant *Zero = Plan->getVPConstant(ConstantInt::get(Ty1, 0));

    auto *CtlzIntrin =
        Intrinsic::getDeclaration(Fn.getParent(), Intrinsic::ctlz,
                                  {VConf->getType()});
    auto *Vlzcnt =
        VPBldr.create<VPCallInstruction, const Twine &, VPValue *,
                      FunctionType *, ArrayRef<VPValue *>>(
        "vp.ctlz", Plan->getVPConstant(CtlzIntrin),
        CtlzIntrin->getFunctionType(), {VConf, Zero});
    Vlzcnt->setVectorizeWithIntrinsic(Intrinsic::ctlz);
    DA->markUniform(*Vlzcnt->getCalledValue());
    DA->markDivergent(*Vlzcnt);

    unsigned VPermControlTySize = ConflictIdxTy->getPrimitiveSizeInBits();
    // BitConst is 1 less the size in bits of ConflictIdx so that when
    // VPermControl = -1, mask will be updated to know that that
    // particular lane is off.
    VPConstant *BitConst =
        Plan->getVPConstant(ConstantInt::get(VConf->getType(),
                            VPermControlTySize - 1));
    auto *VPermControl =
        VPBldr.createNaryOp(Instruction::Sub, ConflictIdxTy,
                            {BitConst, Vlzcnt});

    auto *VPermControlConvert = convertValueType(VPermControl, PermuteTy,
                                                 VPBldr);
    DA->markDivergent(*VPermControlConvert);

    VPConstant *NegOne =
        Plan->getVPConstant(ConstantInt::get(PermuteTy, -1));
    VPInstruction *MaskTodo =
        VPBldr.createCmpInst(CmpInst::ICMP_NE, VPermControlConvert, NegOne,
                             "mask.todo");
    DA->markDivergent(*MaskTodo);

    // Mask of conflict loop anded with block-predicate where tree conflict
    // lowering is conditional. The final store of the result after the
    // conflict loop will be under this mask, so this is not strictly needed
    // for correctness. However, it will reduce the number of iterations of
    // the loop since the loop will exit once the mask is all-zero.
    if (Pred) {
      MaskTodo = VPBldr.createNaryOp(Instruction::And, MaskTodo->getType(),
                                     {MaskTodo, Pred});
      DA->markDivergent(*MaskTodo);
    }

    auto *Cond1 = VPBldr.createAllZeroCheck(MaskTodo, "conflict.top.test");
    DA->markUniform(*Cond1);
    TreeConflictParent->setTerminator(PostConflict,
                                      ConflictPreheader, Cond1);

    // Begin inserting conflict loop instructions.
    VPBldr.setInsertPoint(ConflictHeader, ConflictHeader->begin());

    VPPHINode *VPermControlPhi =
        VPBldr.createPhiInstruction(PermuteTy, "curr.vperm.control");
    VPermControlPhi->addIncoming(VPermControlConvert, ConflictPreheader);
    DA->markDivergent(*VPermControlPhi);

    VPPHINode *VResPhi =
        VPBldr.createPhiInstruction(RednUpdateOpTy, "curr.vres");
    VResPhi->addIncoming(RednUpdateOp, ConflictPreheader);
    DA->markDivergent(*VResPhi);

    VPPHINode *MaskTodoPhi =
        VPBldr.createPhiInstruction(MaskTodo->getType(), "curr.mask.todo");
    MaskTodoPhi->addIncoming(MaskTodo, ConflictPreheader);
    DA->markDivergent(*MaskTodoPhi);

    auto *VTmp = createPermuteIntrinsic("vtmp", VResPhi->getType(), VResPhi,
                                        VPermControlPhi, VPBldr, *C, VF, DA);
    DA->markDivergent(*VTmp);

    // HIR codegen needs operand 0 of select to be a cmp instruction due to how
    // HIR represents it. E.g., %val = select %op1 > %op2 ? %val0 : val1.
    VPConstant *One = Plan->getVPConstant(ConstantInt::get(Ty1, 1));
    auto *MaskICmp = VPBldr.createCmpInst(CmpInst::ICMP_EQ, MaskTodoPhi,
                                          One, "dummy.cmp");
    DA->markDivergent(*MaskICmp);

    // Permute intrinsics are unmasked and will remain so according to
    // the CodeGen team. This is apparently done to enable mask optimizations.
    // Use select after the permute instead.
    Zero = Plan->getVPConstant(Constant::getNullValue(VTmp->getType()));
    auto *VTmpSelect =
        VPBldr.createSelect(MaskICmp, VTmp, Zero, "vtmp.select");
    DA->markDivergent(*VTmpSelect);

    unsigned RednOpcode = TreeConflict->getRednOpcode();
    auto *VResUpdate = VPBldr.createNaryOp(RednOpcode, VResPhi->getType(),
                                           {VTmpSelect, VResPhi});
    DA->markDivergent(*VResUpdate);

    auto *VResNext = VPBldr.createSelect(MaskICmp, VResUpdate, VResPhi,
                                         "vres.next");
    VResPhi->addIncoming(VResNext, ConflictHeader);
    DA->markDivergent(*VResNext);

    auto *VPermControlNext =
        createPermuteIntrinsic("vperm.control.next", VPermControlPhi->getType(),
                               VPermControlPhi, VPermControlPhi, VPBldr, *C,
                               VF, DA);
    DA->markDivergent(*VPermControlNext);

    auto *VPermControlNextSelect =
        VPBldr.createSelect(MaskICmp, VPermControlNext, VPermControlPhi,
                            "vperm.control.select");
    VPermControlPhi->addIncoming(VPermControlNextSelect, ConflictHeader);
    DA->markDivergent(*VPermControlNextSelect);

    // Generate the mask for the next iteration of the conflict loop.
    NegOne = Plan->getVPConstant(
        ConstantInt::get(VPermControlNextSelect->getType(), -1));
    auto *MaskTodoNext =
        VPBldr.createCmpInst(CmpInst::ICMP_NE, VPermControlNextSelect, NegOne,
                             "mask.todo.next");
    MaskTodoPhi->addIncoming(MaskTodoNext, ConflictHeader);
    DA->markDivergent(*MaskTodoNext);

    // HLLoop verifier expects unknown loop backedge to be on the true
    // condition, so this is why the !allzero check is done here.
    auto *LatchCond = VPBldr.createAllZeroCheck(MaskTodoNext, "latch.cond");
    DA->markUniform(*LatchCond);
    auto *LatchCondNot =
        VPBldr.createNot(LatchCond, LatchCond->getName() + ".not");
    DA->markUniform(*LatchCondNot);
    ConflictHeader->setTerminator(ConflictHeader, ConflictExit, LatchCondNot);

    // Begin inserting/updating post-conflict loop instructions.
    VPBldr.setInsertPoint(PostConflict, PostConflict->begin());

    VPPHINode *FinalVResPhi =
        VPBldr.createPhiInstruction(RednUpdateOpTy, "final.result");
    FinalVResPhi->addIncoming(RednUpdateOp, TreeConflictParent);
    FinalVResPhi->addIncoming(VResNext, ConflictHeader);
    DA->markDivergent(*FinalVResPhi);

    // Store of the final result should be under the same mask before the block
    // splitting happened for the conflict loop. Necessary for conditional
    // tree conflict lowering.
    if (Pred) {
      auto *BlockPredInst = VPBldr.createPred(TreeConflictParent->getPredicate());
      PostConflict->setBlockPredicate(BlockPredInst);
    }

    auto *StoreVal =
        VPBldr.createNaryOp(RednOpcode, RednUpdateOpTy,
                            {TreeConflict->getConflictLoad(),
                            FinalVResPhi});
    DA->markDivergent(*StoreVal);

    TreeConflict->replaceAllUsesWith(StoreVal);
    TreeConflictParent->eraseInstruction(TreeConflict);
    TreeConflictsLowered = true;
  }
  VPLAN_DUMP(VPlanLowerTreeConflictControl, Plan);
  return TreeConflictsLowered;
}

} // namespace vpo
} // namespace llvm
