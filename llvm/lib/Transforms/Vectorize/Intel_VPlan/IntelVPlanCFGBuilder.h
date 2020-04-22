//===-- IntelVPlanCFGBuilder.h ----------------------------------*- C++ -*-===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines VPlanCFGBuilder class that is used to build a CFG in
/// VPlan from the incoming LLVM IR CFG.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_CFG_BUILDER_H
#define LLVM_TRANSFORMS_VECTORIZE_INTEL_VPLAN_INTEL_VPLAN_CFG_BUILDER_H

#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanDominatorTree.h"
#include "llvm/ADT/SmallVector.h"
namespace llvm {
namespace vpo {
template <class CFGBuilder>
class VPlanCFGBuilderBase {
  bool contains(Instruction *Inst) const {
    return static_cast<const CFGBuilder *>(this)->contains(Inst);
  }

protected:
  VPlan *Plan;

  // Builder of the VPlan instruction-level representation.
  VPBuilder VPIRBuilder;

  // NOTE: The following maps are intentionally destroyed after the plain CFG
  // construction because subsequent VPlan-to-VPlan transformation may
  // invalidate them.
  // Map incoming BasicBlocks to their newly-created VPBasicBlocks.
  DenseMap<BasicBlock *, VPBasicBlock *> BB2VPBB;
  // Map incoming Value definitions to their newly-created VPValues.
  DenseMap<Value *, VPValue *> IRDef2VPValue;
  /// Map the branches to the condition VPInstruction they are controlled by
  /// (Possibly at a different VPBB).
  DenseMap<Value *, VPValue *> BranchCondMap;

  // Hold phi node's that need to be fixed once the plain CFG has been built.
  SmallVector<PHINode *, 8> PhisToFix;

  // Set operands to VPInstructions representing phi nodes from the input IR.
  // VPlan Phi nodes were created without operands in a previous step of the
  // H-CFG construction because those operands might not have been created in
  // VPlan at that time despite the RPO traversal. This function expects all the
  // instructions to have a representation in VPlan so operands of VPlan phis
  // can be properly set.
  void fixPhiNodes();

  // Create a new empty VPBasicBlock for an incoming BasicBlock or retrieve an
  // existing one if it was already created.
  VPBasicBlock *getOrCreateVPBB(BasicBlock *BB);

  // Return true if \p Val is considered an external definition in the context
  // of the plain CFG construction.
  //
  // An external definition is either:
  // 1. A Value that is neither a Constant nor an Instruction.
  // 2. An Instruction that is outside of the CFG snippet represented in VPlan.
  bool isExternalDef(Value *Val) const;

  // Check whether Val has uses outside the region being imported and create
  // VPExternalUse-s for NewVPInst accordingly.
  void addExternalUses(Value *Val, VPValue *NewVPInst);

  void createVPInstructionsForVPBB(VPBasicBlock *VPBB, BasicBlock *BB);

  // Create a VPInstruction based on the input IR instruction.
  VPInstruction *createVPInstruction(Instruction *Inst);

  // Reset the insertion point.
  void resetInsertPoint() { VPIRBuilder.clearInsertionPoint(); }

  VPlanCFGBuilderBase(VPlan *Plan) : Plan(Plan) {}

  void processBB(BasicBlock *BB);

  VPValue *getOrCreateVPOperand(Value *IRVal);
};

class VPlanLoopCFGBuilder : public VPlanCFGBuilderBase<VPlanLoopCFGBuilder> {
protected:
  Loop *TheLoop;
  LoopInfo *LI;

public:
  bool contains(Instruction *Inst) const {
    return TheLoop->contains(Inst);
  }

  VPlanLoopCFGBuilder(VPlan *Plan, Loop *Lp, LoopInfo *LI)
      : VPlanCFGBuilderBase<VPlanLoopCFGBuilder>(Plan), TheLoop(Lp), LI(LI) {}

  void buildCFG();
};

class VPlanFunctionCFGBuilder
    : public VPlanCFGBuilderBase<VPlanFunctionCFGBuilder> {
  Function &F;

public:
  bool contains(Instruction *Inst) const {
    assert(Inst->getParent()->getParent() == &F && "Use from another function?");
    return true;
  }

  VPlanFunctionCFGBuilder(VPlan *Plan, Function &F)
      : VPlanCFGBuilderBase<VPlanFunctionCFGBuilder>(Plan), F(F) {}

  void buildCFG();
};
} // namespace vpo
} // namespace llvm
#endif
