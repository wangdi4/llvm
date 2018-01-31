//===-- VPlanCostModel.cpp ------------------------------------------------===//
//
//   Copyright (C) 2018 Intel Corporation. All rights reserved.
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

#include "VPlanCostModel.h"
#include "VPlan.h"
#include <llvm/Analysis/TargetTransformInfo.h>

#define DEBUG_TYPE "vplan-cost-model"

using namespace llvm;
using namespace vpo;

namespace {
// FIXME: The following helper functions have multiple implementations
// in the project. They can be effectively organized in a common Load/Store
// utilities unit. This copy is from the LoopVectorize.cpp.

/// A helper function that returns the type of loaded or stored value.
Type *getMemInstValueType(const Value *I) {
  assert((isa<LoadInst>(I) || isa<StoreInst>(I)) &&
         "Expected Load or Store instruction");
  if (auto *LI = dyn_cast<LoadInst>(I))
    return LI->getType();
  return cast<StoreInst>(I)->getValueOperand()->getType();
}

/// A helper function that returns the alignment of load or store instruction.
unsigned getMemInstAlignment(const Value *I) {
  assert((isa<LoadInst>(I) || isa<StoreInst>(I)) &&
         "Expected Load or Store instruction");
  if (auto *LI = dyn_cast<LoadInst>(I))
    return LI->getAlignment();
  return cast<StoreInst>(I)->getAlignment();
}

/// A helper function that returns the address space of the pointer operand of
/// load or store instruction.
unsigned getMemInstAddressSpace(const Value *I) {
  assert((isa<LoadInst>(I) || isa<StoreInst>(I)) &&
         "Expected Load or Store instruction");
  if (auto *LI = dyn_cast<LoadInst>(I))
    return LI->getPointerAddressSpace();
  return cast<StoreInst>(I)->getPointerAddressSpace();
}
} // end anonymous namespace

static const Instruction *getLLVMInstFromDDNode(const HLDDNode *Node) {
  const HLInst *HLInstruction = cast<HLInst>(Node);
  return HLInstruction->getLLVMInstruction();
}

Type *VPlanCostModel::getMemInstValueType(const VPInstruction *VPInst) {
  unsigned Opcode = VPInst->getOpcode();
  assert(Opcode == Instruction::Load || Opcode == Instruction::Store);

  bool IsLoad = Opcode == Instruction::Load;
  if (Type *Result = IsLoad ? VPInst->getOperand(0)->getType()
                            : VPInst->getOperand(1)->getType())
    return Result;

  // FIXME: This is temporal until decomposition is in place - we might end up
  // in operand without underlying HIR instruction currently. The code below
  // workarounds it by accessing the operands of the original load/store which
  // goes beyond just accessing the type of the underlying IR.

  // This path seems to be covered by the one above.
  if (const Instruction *Inst = VPInst->Inst)
    return ::getMemInstValueType(Inst);

  if (!VPInst->HIRData)
    return nullptr;

  HLDDNode *Node = cast<VPInstructionDataHIR>(VPInst->HIRData)->getInstruction();

  if (const Instruction *Inst = getLLVMInstFromDDNode(Node))
    return ::getMemInstValueType(Inst);

  RegDDRef *LvalDDRef = Node->getLvalDDRef();
  // FIXME: Is that correct?
  return LvalDDRef->getDestType();
}

unsigned VPlanCostModel::getMemInstAlignment(const VPInstruction *VPInst) {
  unsigned Opcode = VPInst->getOpcode(); (void)Opcode;
  assert(Opcode == Instruction::Load || Opcode == Instruction::Store);

  // TODO: getType() working without underlying Inst - seems we can return
  // alignment too.

  if (const Instruction *Inst = VPInst->Inst)
    return ::getMemInstAlignment(Inst);

  if (!VPInst->HIRData)
    return 0; // CHECKME: Is that correct?

  HLDDNode *Node = cast<VPInstructionDataHIR>(VPInst->HIRData)->getInstruction();
  if (const Instruction *Inst = getLLVMInstFromDDNode(Node))
    return ::getMemInstAlignment(Inst);

  return 0; // CHECKME: Is that correct?
}

unsigned VPlanCostModel::getMemInstAddressSpace(const VPInstruction *VPInst) {
  unsigned Opcode = VPInst->getOpcode(); (void)Opcode;
  assert(Opcode == Instruction::Load || Opcode == Instruction::Store);

  // TODO: getType() working without underlying Inst - seems we can return
  // address space too.

  if (const Instruction *Inst = VPInst->Inst)
    return ::getMemInstAddressSpace(Inst);

  if (!VPInst->HIRData)
    return 0; // CHECKME: Is that correct?

  HLDDNode *Node = cast<VPInstructionDataHIR>(VPInst->HIRData)->getInstruction();
  if (const Instruction *Inst = getLLVMInstFromDDNode(Node))
    return ::getMemInstAddressSpace(Inst);

  return 0; // CHECKME: Is that correct?
}

unsigned VPlanCostModel::getCost(const VPInstruction *VPInst) {
  unsigned Opcode = VPInst->getOpcode();
  switch (Opcode) {
  default:
    return UnknownCost;
  case Instruction::Load:
  case Instruction::Store: {
    Type *OpTy = getMemInstValueType(VPInst);

    // FIXME: That should be removed later.
    if (!OpTy)
      return UnknownCost;

    assert(OpTy && "Can't get type of the load/store instruction!");
    unsigned Alignment = getMemInstAlignment(VPInst);
    unsigned AddrSpace = getMemInstAddressSpace(VPInst);

    unsigned ScalarCost =
        TTI->getMemoryOpCost(Opcode, OpTy, Alignment, AddrSpace);
    // FIXME: In order to do something smarter we would need:
    //   1) isLinear (and also the case for consecutive stride)
    //   2) Changes in TTI so that getGatherScatterOpCost could work without
    //      'Value *Ptr' (if that could be reasonable)
    return VF*ScalarCost;
  }
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

    Type *BaseTy = VPInst->getType();
    Type *VecTy = VectorType::get(BaseTy, VF);
    unsigned Cost =
        TTI->getArithmeticInstrCost(Opcode, VecTy, Op1VK, Op2VK, Op1VP, Op2VP);
    return Cost;
  }
  }
}

unsigned VPlanCostModel::getCost(const VPBasicBlock *VPBB) {
  unsigned Cost = 0;
  for (const VPRecipeBase &Recipe : *VPBB) {
    const VPInstruction *VPInst = dyn_cast<VPInstruction>(&Recipe);
    // FIXME: cost of other recipes?
    if (!VPInst)
      continue;

    unsigned InstCost = getCost(VPInst);
    if (InstCost == UnknownCost)
      return UnknownCost;
    Cost += InstCost;
  }

  return Cost;
}

void VPlanCostModel::printForVPBlockBase(raw_ostream &OS, const VPBlockBase *VPBlock) {
  // TODO: match print order with "vector execution order".
  if (auto Region = dyn_cast<VPRegionBlock>(VPBlock)) {
    for (const VPBlockBase *Block : depth_first(Region->getEntry()))
      printForVPBlockBase(OS, Block);
    return;
  }

  const VPBasicBlock *VPBB = cast<VPBasicBlock>(VPBlock);
  OS << "Analyzing VPBasicBlock " << VPBB->getName() << ", total cost: ";
  unsigned VPBBCost = getCost(VPBB);
  if (VPBBCost == UnknownCost)
    OS << "Unknown\n";
  else
    OS << VPBBCost << '\n';

  for (const VPRecipeBase &Recipe : *VPBB) {
    const VPInstruction *VPInst = dyn_cast<VPInstruction>(&Recipe);
    // FIXME: cost of other recipes?
    if (!VPInst)
      continue;

    unsigned Cost = getCost(VPInst);
    if (Cost == UnknownCost)
      OS << "  Unknown cost for ";
    else
      OS << "  Cost " << Cost << " for ";
    VPInst->print(OS);
    OS << '\n';
  }
}

void VPlanCostModel::print(raw_ostream &OS) {
  OS << "Cost Model for VPlan: " << Plan->getName() << '\n';
  DEBUG(dbgs() << *Plan;);

  // TODO: match print order with "vector execution order".
  for (const VPBlockBase *Block : depth_first(Plan->getEntry()))
    printForVPBlockBase(OS, Block);
}
