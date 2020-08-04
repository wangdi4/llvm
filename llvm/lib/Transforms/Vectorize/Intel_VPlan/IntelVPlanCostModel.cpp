//===-- IntelVPlanCostModel.cpp -------------------------------------------===//
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements VPlan cost modeling.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanCostModel.h"
#include "IntelVPlan.h"
#include "IntelVPlanCallVecDecisions.h"
#include "IntelVPlanUtils.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/VectorUtils.h"

#include <numeric>

#define DEBUG_TYPE "vplan-cost-model"

using namespace loopopt;

/// A helper function that returns the alignment of load or store instruction.
static unsigned getMemInstAlignment(const Value *I) {
  assert((isa<LoadInst>(I) || isa<StoreInst>(I)) &&
         "Expected Load or Store instruction");
  if (auto *LI = dyn_cast<LoadInst>(I))
    return LI->getAlignment();
  return cast<StoreInst>(I)->getAlignment();
}

/// A helper function that returns the address space of the pointer operand of
/// load or store instruction.
static unsigned getMemInstAddressSpace(const Value *I) {
  assert((isa<LoadInst>(I) || isa<StoreInst>(I)) &&
         "Expected Load or Store instruction");
  if (auto *LI = dyn_cast<LoadInst>(I))
    return LI->getPointerAddressSpace();
  return cast<StoreInst>(I)->getPointerAddressSpace();
}

#if INTEL_CUSTOMIZATION
static const Instruction *getLLVMInstFromDDNode(const HLDDNode *Node) {
  const HLInst *HLInstruction = cast<HLInst>(Node);
  return HLInstruction->getLLVMInstruction();
}
#endif // INTEL_CUSTOMIZATION

namespace llvm {

namespace vpo {

#if INTEL_CUSTOMIZATION
uint64_t VPlanVLSCostModel::getInstructionCost(const OVLSInstruction *I) const {
  uint32_t ElemSize = I->getType().getElementSize();
  Type *ElemType = Type::getIntNTy(getContext(), ElemSize);
  if (isa<OVLSLoad>(I) || isa<OVLSStore>(I)) {
    VectorType *VecTy = FixedVectorType::get(ElemType, VPCM.VF);
    return TTI.getMemoryOpCost(
        isa<OVLSStore>(I) ? Instruction::Store : Instruction::Load, VecTy,
        // FIXME: Next values are not used in getMemoryOpCost(), however
        // that can change later.
        Align() /* Alignment */, 0 /* AddressSpace */);
  }
  if (auto Shuffle = dyn_cast<OVLSShuffle>(I)) {
    SmallVector<int, 16> Mask;
    Shuffle->getShuffleMask(Mask);
    VectorType *VecTy = FixedVectorType::get(ElemType, Mask.size());
    SmallVector<uint32_t, 16> UMask(Mask.begin(), Mask.end());
    return getShuffleCost(UMask, VecTy);
  }
  llvm_unreachable("Unexpected OVLSInstruction.");
}

uint64_t
VPlanVLSCostModel::getGatherScatterOpCost(const OVLSMemref &Memref) const {
  const auto *VPMemref = dyn_cast<VPVLSClientMemref>(&Memref);
  assert(VPMemref && "Wrong type of OVLSMemref is used.");
#if 0
  return VPCM.getLoadStoreCost(VPMemref->getInstruction());
#else
  Type *VecTy = getWidenedType(VPMemref->getInstruction()->getType(), VPCM.VF);
  // FIXME: Without proper decomposition it's impossible to call
  // getLoadStoreCost(), because opcode may not be valid in the VPInstruction.
  // Assume load instruction for non-memref opcode, because store instruction
  // cannot be composed.
  unsigned Opcode =
      VPMemref->getInstruction()->getOpcode() != Instruction::Store
          ? Instruction::Load
          : Instruction::Store;
  return TTI.getMemoryOpCost(Opcode, VecTy, Align(), 0);
#endif
}
#endif // INTEL_CUSTOMIZATION

Type *VPlanCostModel::getMemInstValueType(const VPInstruction *VPInst) {
  unsigned Opcode = VPInst->getOpcode();
  assert(Opcode == Instruction::Load || Opcode == Instruction::Store);
  return Opcode == Instruction::Load ?
    VPInst->getType() : VPInst->getOperand(0)->getType();
}

unsigned VPlanCostModel::getMemInstAddressSpace(const VPInstruction *VPInst) {
  unsigned Opcode = VPInst->getOpcode();
  (void)Opcode;
  assert(Opcode == Instruction::Load || Opcode == Instruction::Store);

  // TODO: getType() working without underlying Inst - seems we can return
  // address space too.

  if (const Value *Val = VPInst->getUnderlyingValue())
    return ::getMemInstAddressSpace(Val);

#if INTEL_CUSTOMIZATION
  if (!VPInst->HIR.isMaster())
    return 0; // CHECKME: Is that correct?
  const HLDDNode *DDNode = cast<HLDDNode>(VPInst->HIR.getUnderlyingNode());
  if (const Instruction *Inst = getLLVMInstFromDDNode(DDNode)) {
    if (isa<LoadInst>(Inst) || isa<StoreInst>(Inst))
      return ::getMemInstAddressSpace(Inst);

    // Handle cases such as a[i] = b + c, the store to a[i] will be the master
    // VPInst. However, Inst will be an add instruction.
    const RegDDRef *LvalRef = DDNode->getLvalDDRef();
    if (LvalRef && LvalRef->isMemRef())
      return LvalRef->getPointerAddressSpace();
  }
#endif // INTEL_CUSTOMIZATION

  return 0; // CHECKME: Is that correct?
}

unsigned
VPlanCostModel::getMemInstAlignment(const VPInstruction *VPInst) const {
  unsigned Opcode = VPInst->getOpcode();
  (void)Opcode;
  assert(Opcode == Instruction::Load || Opcode == Instruction::Store);

  // TODO: getType() working without underlying Inst - seems we can return
  // alignment too.

  if (const Instruction *Inst = VPInst->getInstruction())
    if (unsigned Align = ::getMemInstAlignment(Inst))
      return Align;

#if INTEL_CUSTOMIZATION
  if (VPInst->HIR.isMaster()) {
    const HLDDNode *DDNode = cast<HLDDNode>(VPInst->HIR.getUnderlyingNode());
    if (const Instruction *Inst = getLLVMInstFromDDNode(DDNode)) {
      if (isa<LoadInst>(Inst) || isa<StoreInst>(Inst)) {
        if (unsigned Align = ::getMemInstAlignment(Inst))
          return Align;
      } else {
        // Handle cases such as a[i] = b + c, the store to a[i] will be the
        // master VPInst. However, Inst will be an add instruction.
        const RegDDRef *LvalRef = DDNode->getLvalDDRef();
        if (LvalRef && LvalRef->isMemRef())
          if (unsigned Align = LvalRef->getAlignment())
            return Align;
      }
    }
  }
#endif // INTEL_CUSTOMIZATION

  // If underlying instruction had default alignment (0) we need to query
  // DataLayout what it is, because default alignment for the widened type will
  // be different.
  return DL->getABITypeAlignment(getMemInstValueType(VPInst));
}

bool VPlanCostModel::isUnitStrideLoadStore(const VPInstruction *VPInst,
                                           bool &NegativeStride) const {
  const VPValue *P = getLoadStorePointerOperand(VPInst);
  return Plan->getVPlanDA()->isUnitStridePtr(P, NegativeStride);
}

unsigned VPlanCostModel::getLoadStoreIndexSize(
  const VPInstruction *VPInst) const {
  assert((VPInst->getOpcode() == Instruction::Load ||
          VPInst->getOpcode() == Instruction::Store) &&
         "Expect 'VPInst' to be either a LoadInst or a StoreInst");

  const VPValue *Ptr = getLoadStorePointerOperand(VPInst);
  // Skip all NOP BitCasts/AddrSpaceCasts on the way to GEP.
  while ((VPInst = dyn_cast<VPInstruction>(Ptr)) &&
         (VPInst->getOpcode() == Instruction::BitCast ||
          VPInst->getOpcode() == Instruction::AddrSpaceCast) &&
         TTI->getCastInstrCost(VPInst->getOpcode(), VPInst->getType(),
                               VPInst->getOperand(0)->getType(),
                               TTI::CastContextHint::None) == 0)
    Ptr = VPInst->getOperand(0);

  const VPInstruction *VPAddrInst = dyn_cast<VPGEPInstruction>(Ptr);
  if (!VPAddrInst)
    VPAddrInst = dyn_cast<VPSubscriptInst>(Ptr);
  unsigned IndexSize = DL->getPointerSizeInBits();

  // Try to reduce index size from 64 bit (default for GEP) to 32. It is
  // essential for VF 16. Check that the base pointer (first operand) is the
  // same for all lanes, and that there's at most one variable index.
  if (IndexSize < 64 || !VPAddrInst ||
      getPointerOperand(VPAddrInst)->getType()->isVectorTy())
    return IndexSize;

  auto getTypeElementSize = [](const VPValue *V) -> unsigned {
    const Type *Ty = V->getType();
    if (auto *VecTy = dyn_cast<VectorType>(Ty))
      Ty = VecTy->getElementType();
    return Ty->getPrimitiveSizeInBits();
  };

  SmallVector<const VPValue *, 4> IndicesOperands;
  if (isa<VPGEPInstruction>(VPAddrInst))
    IndicesOperands.append(VPAddrInst->op_begin() + 1, VPAddrInst->op_end());
  else {
    const VPSubscriptInst *Subscript = cast<VPSubscriptInst>(VPAddrInst);
    // Add index operand for each dimension.
    for (unsigned Dim = 0; Dim < Subscript->getNumDimensions(); ++Dim)
      IndicesOperands.push_back(Subscript->getIndex(Dim));
  }

  unsigned NumOfVarIndices = 0;
  for (auto *Op : IndicesOperands) {
    const VPConstant *VPConst = dyn_cast<VPConstant>(Op);
    // We don't check that the Constant fits 32 bits as we expect CG is able to
    // pull the splat Constant into splat Base and form scalar base for gather/
    // scatter which is Base + Constant.
    //
    // TODO:
    // Currently it doesn't happen as uniform Base is not recognized by ISel.
    // Once we see a problem due to that we need to fix it in CG.
    if (VPConst && VPConst->isConstantInt())
      continue;

    const VPInstruction *VPIdx = dyn_cast<VPInstruction>(Op);
    if (!VPIdx || ++NumOfVarIndices > 1)
      return IndexSize; // Can't shrink.

    // SExt to 64 bits from 32 bits or less is allowed.
    if (getTypeElementSize(VPIdx) == 64 &&
        VPIdx->getOpcode() == Instruction::SExt &&
        isa<VPInstruction>(VPIdx->getOperand(0)))
      VPIdx = cast<VPInstruction>(VPIdx->getOperand(0));

    if (getTypeElementSize(VPIdx) > 32)
      return IndexSize; // Can't shrink.
  }
  return 32u;
}

unsigned VPlanCostModel::getArithmeticInstructionCost(const unsigned Opcode,
                                                      const VPValue *Op1,
                                                      const VPValue *Op2,
                                                      const Type *ScalarTy,
                                                      const unsigned VF) {
  assert(Op1 != nullptr && "First operand is expected.");
  if (!ScalarTy)
    return UnknownCost;
  Type *VecTy = getWidenedType(const_cast<Type *>(ScalarTy), VF);

  auto SetOperandValueFeatures = [](
    const VPValue *Val,
    TargetTransformInfo::OperandValueKind& OpVK,
    TargetTransformInfo::OperandValueProperties& OpVP) -> void {
    if (const VPConstant *VPConst = dyn_cast<VPConstant>(Val)) {
      OpVK = TargetTransformInfo::OK_UniformConstantValue;
      if (const ConstantInt *IntConst =
          dyn_cast<ConstantInt>(VPConst->getConstant())) {
        if (IntConst->getValue().isPowerOf2())
          OpVP = TargetTransformInfo::OP_PowerOf2;
        else if ((IntConst->getValue() + 1).isPowerOf2() ||
                 (IntConst->getValue() - 1).isPowerOf2())
          OpVP = TargetTransformInfo::OP_PowerOf2_PlusMinus1;
      }
    }
  };

  TargetTransformInfo::OperandValueKind Op1VK =
    TargetTransformInfo::OK_AnyValue;
  TargetTransformInfo::OperandValueKind Op2VK =
    TargetTransformInfo::OK_AnyValue;
  TargetTransformInfo::OperandValueProperties Op1VP =
    TargetTransformInfo::OP_None;
  TargetTransformInfo::OperandValueProperties Op2VP =
    TargetTransformInfo::OP_None;

  SetOperandValueFeatures(Op1, Op1VK, Op1VP);
  if (Op2)
    SetOperandValueFeatures(Op2, Op2VK, Op2VP);

  return TTI->getArithmeticInstrCost(Opcode, VecTy,
    TTI::TCK_RecipThroughput, Op1VK, Op2VK, Op1VP, Op2VP);
}

unsigned VPlanCostModel::getLoadStoreCost(const VPInstruction *VPInst) {
  Type *OpTy = getMemInstValueType(VPInst);
  assert(OpTy && "Can't get type of the load/store instruction!");

  unsigned Opcode = VPInst->getOpcode();
  unsigned Alignment = getMemInstAlignment(VPInst);
  unsigned AddrSpace = getMemInstAddressSpace(VPInst);
  Type *VecTy;
  unsigned Scale;

  // TODO: VF check in IsMasked might become redundant once a separate VPlan
  // is maintained for VF = 1 meaning that the cost calculation for scalar loop
  // is done over VPlan that doesn't undergo any vector transformations such as
  // predication.
  bool IsMasked = (VF > 1) && (VPInst->getParent()->getPredicate() != nullptr);

  // Aggregates are serialized.  If we see an aggregate type we set Scale to VF
  // and substitude VecTy with base aggregate type.
  //
  // TODO:
  // ScalarCost * VF is Zero order approximation of scalarization for aggregate
  // types.  Yet to be tuned further.
  if (isVectorizableTy(OpTy)) {
    Scale = 1;
    VecTy = getWidenedType(OpTy, VF);
  }
  else {
    Scale = VF;
    VecTy = OpTy;
  }

  // Call get[Masked]MemoryOpCost() for the following cases.
  // 1. VF = 1 VPlan even for vector OpTy.
  // 2. Unit stride load/store.
  // 3. Aggregate OpTy (they enter this code though Scale > 1 check of VF == 1
  //    check).
  bool NegativeStride = false;
  if (VF == 1 || Scale > 1 || isUnitStrideLoadStore(VPInst, NegativeStride)) {
    unsigned Cost = 0;

    // For negative stride we need to reverse elements in the vector after load
    // or before store.
    //
    // TODO:
    // In case of vector input type (re-vectorization case) reverse operation
    // might cost more than reversing vanilla vector and TTI interface doesn't
    // take into account the vector parts.  Once TTI is fixed VPlan should pass
    // particles type to TTI as well.
    if (NegativeStride) {
      assert(VF > 1 && Scale == 1 &&
             "Unexpected conditions for NegativeStride == true.");
      Cost += TTI->getShuffleCost(TTI::SK_Reverse, cast<VectorType>(VecTy));
    }

    Cost += IsMasked ?
      Scale * TTI->getMaskedMemoryOpCost(Opcode, VecTy, Align(Alignment), AddrSpace) :
      Scale * TTI->getMemoryOpCost(Opcode, VecTy,
                                   Alignment ? Align(Alignment): Align(),
                                   AddrSpace);
    return Cost;
  }

  // TODO:
  // Currently TTI doesn't add cost of index split and data join in case
  // gather/scatter operation is implemented with two HW gathers/scatters.
  return TTI->getGatherScatterOpCost(
    Opcode, VecTy, getLoadStoreIndexSize(VPInst),
    IsMasked, Alignment, AddrSpace);
}

unsigned VPlanCostModel::getInsertExtractElementsCost(
  unsigned Opcode, Type *Ty, unsigned VF) {
  assert((Opcode == Instruction::ExtractElement ||
          Opcode == Instruction::InsertElement) &&
         "Only Extract/InsertElement opcode is expected.");
  unsigned Cost = 0;
  Type *VecTy = getWidenedType(Ty, VF);
  for(unsigned Idx = 0; Idx < VF; Idx++)
    Cost += TTI->getVectorInstrCost(Opcode, VecTy, Idx);
  return Cost;
}

unsigned VPlanCostModel::getIntrinsicInstrCost(
  Intrinsic::ID ID, const CallBase &CB, unsigned VF,
  VPCallInstruction::CallVecScenariosTy VS) {

  // Intrinsics which have 0 cost are not lowered to actual code during ASM CG.
  // They are meant for intermediate analysis/transforms and will be deleted
  // before CG. Do not account the cost of serializing them.
  if (TTI->getIntrinsicInstrCost(IntrinsicCostAttributes(ID, CB, 1),
                                 TTI::TCK_RecipThroughput) == 0)
    return 0;

  // isTriviallyVectorizable(ID) is special cased in VPlan CG and
  // do not listen to VS settings.  Special case them here until
  // is fixed in CG and in VPlanCallVecDecisions::analyzeCall.
  //
  // Also special case VF == 1 as vectorization scenario may be not set.
  //
  // TODO:
  // We are introducing 'TrivialVectorIntrinsic' scenario for such intrinsics.
  // The code below has to be updated once the new scenario is available.
  if (isTriviallyVectorizable(ID) || VF == 1)
    VS = VPCallInstruction::CallVecScenariosTy::VectorVariant;

  switch (VS) {
    case VPCallInstruction::CallVecScenariosTy::Undefined:
      // The calls that missed the analysis have Unknown cost.
      return UnknownCost;
    case VPCallInstruction::CallVecScenariosTy::DoNotWiden:
      return
        TTI->getIntrinsicInstrCost(IntrinsicCostAttributes(ID, CB, 1),
                                   TTI::TCK_RecipThroughput);
    case VPCallInstruction::CallVecScenariosTy::Serialization:
      // For a serialized call, such as: float call @foo(double arg1, int arg2)
      // calculate the cost of vectorized code that way:
      // Cost of extracting VF double elements for arg1 +
      // Cost of extracting VF int elements for arg2 +
      // Cost of VF calls to scalar @foo +
      // Cost of inserting VF float elements for the result of foo.
      // TODO:
      // Here we ignore the fact that when serialized code feeds another
      // serialized code insert + extract in between can be optimized out.
      return
        // The sum of costs of 'devectorizing' all args of the call.
        std::accumulate(CB.arg_begin(), CB.arg_end(), 0,
          [=](unsigned Cost, const Use& Arg) {
            Type *ArgTy = Arg.get()->getType();
            // If Arg is not expected to be vectorized
            // (isVectorizableTy(ArgTy) is false) then it contributes 0.
            //
            // TODO:
            // In general there are can be call arguments that are not
            // vectorized.  SVA should help here.
            return Cost + (isVectorizableTy(ArgTy) ?
              getInsertExtractElementsCost(Instruction::ExtractElement,
                                           ArgTy, VF) : 0);
          }) +
        // The cost of VF calls to the scalar function.
        VF * TTI->getIntrinsicInstrCost(IntrinsicCostAttributes(ID, CB, 1),
                                        TTI::TCK_RecipThroughput) +
        // The cost of 'vectorizing' function's result if any.
        (isVectorizableTy(CB.getType()) && !CB.getType()->isVoidTy() ?
         getInsertExtractElementsCost(
           Instruction::InsertElement, CB.getType(), VF) : 0);
    default:
      break;
  }
  return TTI->getIntrinsicInstrCost(
    IntrinsicCostAttributes(ID, CB, VF), TTI::TCK_RecipThroughput);
}

unsigned VPlanCostModel::getCost(const VPInstruction *VPInst) {
  // TODO: For instruction that are not contained inside the loop we're
  // vectorizing, VF should not be considered. That includes the instructions
  // that are outside of any of the loops in the loopnest. However, before
  // support it in the cost model, we need to design how the current loop being
  // vectorized is represented in the VPlan itself, which also might result in
  // different VFs for different loop:
  //   Outer1
  //     Inner1, vectorize with VF_Inner1
  //     Inner2, vecotrize with VF_Inner2
  unsigned Opcode = VPInst->getOpcode();
  switch (Opcode) {
  default:
    return UnknownCost;
  case Instruction::GetElementPtr:
  case VPInstruction::Subscript: {
    // FIXME: First, there could be at least two ways to use this GEP's result
    // (maybe with some casts/geps in between):
    //   1) As a pointer operand of memory operation.
    //   2) As a non-pointer operand of memory operation, e.g. calculation of
    //      some pointers in a loop and storing them without dereferencing.
    // In the first case, the cost of the GEP might be easier to keep as a part
    // of that memory operation - because the way GEP would be handled depends
    // on how the corresponding memory instruction would be handled (e.g.
    // scalarized or not). We reflect that by just returning zero for such GEPs.
    //
    // Also, memory operation are not modeled correctly for now (just
    // VF*ScalarCost) so return the same zero cost for both scalar (VF==1) and
    // vector cases to remove any influence in GEPs cost modeling to
    // vectorization decisions.
    //
    // For the second case from the above we first need a way to distinguish
    // such two kinds of GEP in some prior analysis. As it's not yet done just
    // return the same zero cost too.
    //
    // Another note: getGEPCost in TTI uses GEP's operands (llvm::Value's) to
    // dedicde what will be the cost of the GEP. We don't necessarily have them
    // in VPlan so in future TTI interface should probably be extended with
    // VPValue-based operands overload.
    return 0;
  }
#if INTEL_CUSTOMIZATION
  case VPInstruction::Abs: {
    // Cost of Abs instruction is computed as cost of a compare followed by
    // a select for now.
    Type *OpTy = VPInst->getOperand(0)->getCMType();
    if (!OpTy)
      return UnknownCost;

    Type *VecOpTy = getWidenedType(OpTy, VF);
    Type *CmpTy = Type::getInt1Ty(*(Plan->getLLVMContext()));
    Type *VecCmpTy =
        getWidenedType(CmpTy, cast<VectorType>(VecOpTy)->getNumElements());

    unsigned CmpCost = TTI->getCmpSelInstrCost(Instruction::ICmp, VecOpTy);
    unsigned SelectCost =
        TTI->getCmpSelInstrCost(Instruction::Select, VecOpTy, VecCmpTy);
    return CmpCost + SelectCost;
  }

  // TODO - costmodel support for AllZeroCheck.
  case VPInstruction::AllZeroCheck:
    return 0;
  // This is a no-op - used to mark block predicate.
  case VPInstruction::Pred:
    return 0;
  // No-op terminator instruction.
  case Instruction::Br:
    return 0;
#endif // INTEL_CUSTOMIZATION
  case Instruction::Load:
  case Instruction::Store:
    return getLoadStoreCost(VPInst);
  case Instruction::Add:
  case Instruction::FAdd:
  case Instruction::Sub:
  case Instruction::FSub:
  case Instruction::Mul:
  case Instruction::FMul:
  case Instruction::UDiv:
  case Instruction::SDiv:
  case Instruction::FDiv:
  case Instruction::URem:
  case Instruction::SRem:
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor:
    return getArithmeticInstructionCost(
      Opcode, VPInst->getOperand(0), VPInst->getOperand(1),
      VPInst->getCMType(), VF);
  case VPInstruction::Not: // Treat same as Xor.
    return getArithmeticInstructionCost(
      Instruction::Xor, VPInst->getOperand(0), nullptr,
      VPInst->getCMType(), VF);
  case Instruction::FNeg:
    return getArithmeticInstructionCost(
      Opcode, VPInst->getOperand(0), nullptr, VPInst->getCMType(), VF);
  case Instruction::ICmp:
  case Instruction::FCmp: {
    // FIXME: Assuming all the compares are widened, which is obviously wrong
    // for trip count checks.
    Type *Ty = VPInst->getOperand(0)->getCMType();

    // FIXME: In the future VPValue will always have Type (VPType), but for now
    // it might be missing so handle such cases.
    if (!Ty)
      Ty = VPInst->getOperand(1)->getCMType();
    if (!Ty)
      return UnknownCost;

    Type *VectorTy = getWidenedType(Ty, VF);
    return TTI->getCmpSelInstrCost(Opcode, VectorTy);
  }
  case Instruction::Select: {
    // FIXME: Due to issues in VPlan creation VPInstruction with Select opcode
    // can have 4 operands. This is obviously wrong and is not related to the
    // cost modeling. Skip such cases.
    if (VPInst->getNumOperands() != 3)
      return UnknownCost;

    Type *CondTy = VPInst->getOperand(0)->getCMType();
    Type *OpTy = VPInst->getOperand(1)->getCMType();

    // FIXME: Remove once VPValue is known to always have type.
    if (!CondTy)
      return UnknownCost;

    if (!OpTy)
      OpTy = VPInst->getOperand(2)->getCMType();
    if (!OpTy)
      return UnknownCost;

    Type *VecCondTy = getWidenedType(CondTy, VF);
    Type *VecOpTy = getWidenedType(OpTy, VF);
    return TTI->getCmpSelInstrCost(Opcode, VecOpTy, VecCondTy);
  }
  case Instruction::ZExt:
  case Instruction::SExt:
  case Instruction::FPToUI:
  case Instruction::FPToSI:
  case Instruction::FPExt:
  case Instruction::PtrToInt:
  case Instruction::IntToPtr:
  case Instruction::SIToFP:
  case Instruction::UIToFP:
  case Instruction::Trunc:
  case Instruction::FPTrunc: {
    Type *BaseDstTy = VPInst->getCMType();
    Type *BaseSrcTy = VPInst->getOperand(0)->getCMType();

    if (!BaseDstTy || !BaseSrcTy)
      return UnknownCost;

    assert(!BaseDstTy->isVectorTy() &&
           "Vector base types are not yet implemented!");
    assert(!BaseDstTy->isAggregateType() && "Unexpected aggregate type!");

    Type *VecDstTy = getWidenedType(BaseDstTy, VF);
    Type *VecSrcTy = getWidenedType(BaseSrcTy, VF);
    // TODO: The following will report cost "1" for sext/zext in scalar case
    // because no Instruction* is passed to TTI and it is unable to analyze that
    // such a cast can be folded into the defining load for free. We should
    // consider adding an overload accepting VPInstruction for TTI to be able to
    // analyze that.
    return TTI->getCastInstrCost(Opcode, VecDstTy, VecSrcTy,
                                 TTI::CastContextHint::None);
  }
  case Instruction::Call: {
    auto *VPCall = cast<VPCallInstruction>(VPInst);
    auto *CI = VPCall->getUnderlyingCallInst();
    Intrinsic::ID ID = getIntrinsicForCallSite(*CI, TLI);

    if (ID == Intrinsic::not_intrinsic)
      return UnknownCost;

    return getIntrinsicInstrCost(ID, *CI, VF,
                                 VPCall->getVectorizationScenario());
  }
  }
}

unsigned VPlanCostModel::getCost(const VPBasicBlock *VPBB) {
  unsigned Cost = 0;
  for (const VPInstruction &VPInst : *VPBB) {
    unsigned InstCost = getCost(&VPInst);
    if (InstCost == UnknownCost)
      continue;
    Cost += InstCost;
  }

  return Cost;
}

unsigned VPlanCostModel::getCost() {
  // CallVecDecisions analysis invocation.
  VPlanCallVecDecisions CallVecDecisions(*const_cast<VPlan *>(Plan));
  CallVecDecisions.run(VF, TLI, TTI);

  // Compute SVA results for current VPlan in order to compute cost accurately
  // in CM.
  const_cast<VPlan *>(Plan)->runSVA(VF, TLI);

  unsigned Cost = 0;
  for (auto *Block : depth_first(Plan->getEntryBlock()))
    // FIXME: Use Block Frequency Info (or similar VPlan-specific analysis) to
    // correctly scale the cost of the basic block.
    Cost += getCost(Block);

  return Cost;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPlanCostModel::printForVPInstruction(
  raw_ostream &OS, const VPInstruction *VPInst) {
  OS << "  Cost " << getCostNumberString(getCost(VPInst)) << " for ";
  VPInst->printWithoutAnalyses(OS);
  OS << '\n';
}

void VPlanCostModel::printForVPBasicBlock(raw_ostream &OS,
                                          const VPBasicBlock *VPBB) {
  OS << "Analyzing VPBasicBlock " << VPBB->getName() << ", total cost: " <<
    getCostNumberString(getCost(VPBB)) << '\n';
  for (const VPInstruction &VPInst : *VPBB)
    if (PrintTerminatorInst || !isa<VPBranchInst>(VPInst))
      printForVPInstruction(OS, &VPInst);
}

void VPlanCostModel::print(raw_ostream &OS, const std::string &Header) {
  OS << "Cost Model for VPlan " << Header << " with VF = " << VF << ":\n";
  OS << "Total Cost: " << getCost() << '\n';
  LLVM_DEBUG(dbgs() << *Plan;);

  // TODO: match print order with "vector execution order".
  for (const VPBasicBlock *Block : depth_first(Plan->getEntryBlock()))
    printForVPBasicBlock(OS, Block);

  OS << '\n';
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

} // namespace vpo

} // namespace llvm
