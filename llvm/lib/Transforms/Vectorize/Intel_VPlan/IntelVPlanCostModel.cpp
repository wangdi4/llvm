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
#include "llvm/Analysis/TargetTransformInfo.h"

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
    VectorType *VecTy = VectorType::get(ElemType, VPCM.VF);
    return TTI.getMemoryOpCost(
        isa<OVLSStore>(I) ? Instruction::Store : Instruction::Load, VecTy,
        // FIXME: Next values are not used in getMemoryOpCost(), however
        // that can change later.
        MaybeAlign(0) /* Alignment */, 0 /* AddressSpace */);
  }
  if (auto Shuffle = dyn_cast<OVLSShuffle>(I)) {
    SmallVector<uint32_t, 16> Mask;
    Shuffle->getShuffleMask(Mask);
    VectorType *VecTy = VectorType::get(ElemType, Mask.size());
    return getShuffleCost(Mask, VecTy);
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
  Type *VecTy =
      VPCM.getVectorizedType(VPMemref->getInstruction()->getType(), VPCM.VF);
  // FIXME: Without proper decomposition it's impossible to call
  // getLoadStoreCost(), because opcode may not be valid in the VPInstruction.
  // Assume load instruction for non-memref opcode, because store instruction
  // cannot be composed.
  unsigned Opcode =
      VPMemref->getInstruction()->getOpcode() != Instruction::Store
          ? Instruction::Load
          : Instruction::Store;
  return TTI.getMemoryOpCost(Opcode, VecTy, MaybeAlign(0), 0);
#endif
}
#endif // INTEL_CUSTOMIZATION

// TODO: ideally this function should be moved into utils.
Type *VPlanCostModel::getVectorizedType(const Type *BaseTy, unsigned VF) {
  if (BaseTy->isVectorTy())
    VF *= BaseTy->getVectorNumElements();

  return VectorType::get(BaseTy->getScalarType(), VF);
}

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
  if (const Instruction *Inst = getLLVMInstFromDDNode(DDNode))
    return ::getMemInstAddressSpace(Inst);
#endif // INTEL_CUSTOMIZATION

  return 0; // CHECKME: Is that correct?
}

Value* VPlanCostModel::getGEP(const VPInstruction *VPInst) {
  unsigned Opcode = VPInst->getOpcode();
  (void)Opcode;
  assert(Opcode == Instruction::Load || Opcode == Instruction::Store);

  if (const Instruction *Inst =
          dyn_cast_or_null<Instruction>(VPInst->getUnderlyingValue())) {
    auto GEPInst = Opcode == Instruction::Load ? Inst->getOperand(0)
                                               : Inst->getOperand(1);
    if (dyn_cast_or_null<GetElementPtrInst>(GEPInst))
      return GEPInst;
  }

#if INTEL_CUSTOMIZATION
  if (!VPInst->HIR.isMaster())
    return nullptr;
  auto *HInst = dyn_cast<HLInst>(VPInst->HIR.getUnderlyingNode());
  auto RegDD = Opcode == Instruction::Load ? HInst->getOperandDDRef(1)
                                           : HInst->getLvalDDRef();

  return RegDD->getTempBaseValue();
#endif // INTEL_CUSTOMIZATION

  return nullptr;
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
    if (const Instruction *Inst = getLLVMInstFromDDNode(DDNode))
      if (unsigned Align = ::getMemInstAlignment(Inst))
        return Align;
  }
#endif // INTEL_CUSTOMIZATION

  // If underlying instruction had default alignment (0) we need to query
  // DataLayout what it is, because default alignment for the widened type will
  // be different.
  return DL->getABITypeAlignment(getMemInstValueType(VPInst));
}

unsigned VPlanCostModel::getLoadStoreCost(const VPInstruction *VPInst) const {
  Type *OpTy = getMemInstValueType(VPInst);
  assert(OpTy && "Can't get type of the load/store instruction!");

  unsigned Opcode = VPInst->getOpcode();
  unsigned Alignment = getMemInstAlignment(VPInst);
  unsigned AddrSpace = getMemInstAddressSpace(VPInst);
  // FIXME: Take into account masked case.
  // FIXME: Shouldn't use underlying IR, because at this point it can be
  // invalid. For instance, vectorizer may decide to generate 32-bit gather
  // instead of 64-bit, while GEP may have 64-bit index.
  // There're 2 options to consider:
  //  1. Rewrite getGatherScatterOpCost so that user will pass index size,
  //  rather then GEP
  //  2. Templatize TTI to use VPValue.
  if (auto GEPInst = getGEP(VPInst)) {
    Type *VecTy = getVectorizedType(OpTy, VF);
    return TTI->getGatherScatterOpCost(Opcode, VecTy, GEPInst,
                                       false /* Masked */, Alignment);
  }
  unsigned BaseCost =
      TTI->getMemoryOpCost(Opcode, OpTy, MaybeAlign(Alignment), AddrSpace);
  return VF*BaseCost;
}

unsigned VPlanCostModel::getCost(const VPInstruction *VPInst) const {
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
  case Instruction::GetElementPtr: {
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
  // TODO - costmodel support for AllZeroCheck.
  case VPInstruction::AllZeroCheck:
    return 0;
  // This is a no-op - used to mark block predicate.
  case VPInstruction::Pred:
    return 0;
#endif // INTEL_CUSTOMIZATION
  case Instruction::Load:
  case Instruction::Store:
    return VPlanCostModel::getLoadStoreCost(VPInst);
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
  case VPInstruction::Not: // Treat same as Xor.
  case Instruction::Xor: {
    TargetTransformInfo::OperandValueKind Op1VK =
        TargetTransformInfo::OK_AnyValue;
    TargetTransformInfo::OperandValueKind Op2VK =
        TargetTransformInfo::OK_AnyValue;
    TargetTransformInfo::OperandValueProperties Op1VP =
        TargetTransformInfo::OP_None;
    TargetTransformInfo::OperandValueProperties Op2VP =
        TargetTransformInfo::OP_None;

    // TODO: More precise kinds/properties for VPConstants. However, we'd need
    // to distinguish
    //   <i32 1, i32 1, i32 1, i32 1>
    // from
    //   <i32 1, i32 2, i32 3, i32 4>
    // that would have had the same underlying llvm::Constant (i32 1).

    Type *BaseTy = VPInst->getCMType();
    if (!BaseTy)
      return UnknownCost;
    Type *VecTy = getVectorizedType(BaseTy, VF);
    unsigned Cost =
        TTI->getArithmeticInstrCost(Opcode, VecTy, Op1VK, Op2VK, Op1VP, Op2VP);
    return Cost;
  }
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

    Type *VectorTy = getVectorizedType(Ty, VF);
    unsigned Cost = TTI->getCmpSelInstrCost(Opcode, VectorTy);
    return Cost;
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

    Type *VecCondTy = getVectorizedType(CondTy, VF);
    Type *VecOpTy = getVectorizedType(OpTy, VF);
    unsigned Cost = TTI->getCmpSelInstrCost(Opcode, VecOpTy, VecCondTy);
    return Cost;
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

    Type *VecDstTy = getVectorizedType(BaseDstTy, VF);
    Type *VecSrcTy = getVectorizedType(BaseSrcTy, VF);
    // TODO: The following will report cost "1" for sext/zext in scalar case
    // because no Instruction* is passed to TTI and it is unable to analyze that
    // such a cast can be folded into the defining load for free. We should
    // consider adding an overload accepting VPInstruction for TTI to be able to
    // analyze that.
    unsigned Cost = TTI->getCastInstrCost(Opcode, VecDstTy, VecSrcTy);

    return Cost;
  }
  }
}

unsigned VPlanCostModel::getCost(const VPBasicBlock *VPBB) const {
  unsigned Cost = 0;
  for (const VPInstruction &VPInst : VPBB->vpinstructions()) {
    unsigned InstCost = getCost(&VPInst);
    if (InstCost == UnknownCost)
      continue;
    Cost += InstCost;
  }

  return Cost;
}

unsigned VPlanCostModel::getCost(const VPBlockBase *VPBlock) const {
  if (auto Region = dyn_cast<VPRegionBlock>(VPBlock)) {
    unsigned Cost = 0;
    for (const VPBlockBase *Block : depth_first(Region->getEntry())) {
      // FIXME: Use Block Frequency Info (or similar VPlan-specific analysis) to
      // correctly scale the cost of the basic block.
      Cost += getCost(Block);
    }
    return Cost;
  }

  // TODO: swap the casts with the above?
  const auto *VPBB = cast<VPBasicBlock>(VPBlock);
  return getCost(VPBB);
}

unsigned VPlanCostModel::getCost() const {
  assert(Plan->getEntry()->getNumSuccessors() == 0 &&
         "VPlan Entry block must have no successors!");
  return getCost(Plan->getEntry());
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPlanCostModel::printForVPBlockBase(raw_ostream &OS,
                                         const VPBlockBase *VPBlock) const {
  // TODO: match print order with "vector execution order".
  if (auto Region = dyn_cast<VPRegionBlock>(VPBlock)) {
    for (const VPBlockBase *Block : depth_first(Region->getEntry()))
      printForVPBlockBase(OS, Block);
    return;
  }

  const auto *VPBB = cast<VPBasicBlock>(VPBlock);
  OS << "Analyzing VPBasicBlock " << VPBB->getName() << ", total cost: ";
  unsigned VPBBCost = getCost(VPBB);
  if (VPBBCost == UnknownCost)
    OS << "Unknown\n";
  else
    OS << VPBBCost << '\n';

  for (const VPInstruction &VPInst : VPBB->vpinstructions()) {
    unsigned Cost = getCost(&VPInst);
    if (Cost == UnknownCost)
      OS << "  Unknown cost for ";
    else
      OS << "  Cost " << Cost << " for ";
    VPInst.print(OS);
    OS << '\n';
  }
}

void VPlanCostModel::print(raw_ostream &OS) const {
  OS << "Cost Model for VPlan " << Plan->getName() << " with VF = " << VF
     << ":\n";
  OS << "Total Cost: " << getCost() << '\n';
  LLVM_DEBUG(dbgs() << *Plan;);

  // TODO: match print order with "vector execution order".
  for (const VPBlockBase *Block : depth_first(Plan->getEntry()))
    printForVPBlockBase(OS, Block);

  OS << '\n';
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

} // namespace vpo

} // namespace llvm
