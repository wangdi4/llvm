//===-- IntelVPlanCFGBuilder.cpp ------------------------------------------===//
//
//   Copyright (C) 2020 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// TODO
///
//===----------------------------------------------------------------------===//

#include "IntelVPlanCFGBuilder.h"
#include "llvm/Analysis/LoopIterator.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vplan-cfg-builder"

template <class CFGBuilder>
void VPlanCFGBuilderBase<CFGBuilder>::fixPhiNodes() {
  for (auto *Phi : PhisToFix) {
    assert(IRDef2VPValue.count(Phi) && "Missing VPInstruction for PHINode.");
    VPValue *VPVal = IRDef2VPValue[Phi];
    assert(isa<VPPHINode>(VPVal) && "Expected VPPHINode for phi node.");
    auto *VPPhi = cast<VPPHINode>(VPVal);
    assert(VPPhi->getNumOperands() == 0 &&
           "Expected VPInstruction with no operands.");

    for (unsigned I = 0, E = Phi->getNumIncomingValues(); I != E; ++I)
      VPPhi->addIncoming(getOrCreateVPOperand(Phi->getIncomingValue(I)),
                         getOrCreateVPBB(Phi->getIncomingBlock(I)));
  }
}

template <class CFGBuilder>
VPBasicBlock *VPlanCFGBuilderBase<CFGBuilder>::getOrCreateVPBB(BasicBlock *BB) {

  VPBasicBlock *VPBB;
  auto BlockIt = BB2VPBB.find(BB);

  if (BlockIt == BB2VPBB.end()) {
    // New VPBB
    LLVM_DEBUG(dbgs() << "Creating VPBasicBlock for " << BB->getName() << "\n");
    VPBB = new VPBasicBlock(VPlanUtils::createUniqueName("BB"), Plan);
    BB2VPBB[BB] = VPBB;
    VPBB->setOriginalBB(BB);
    Plan->insertAtBack(VPBB);
  } else {
    // Retrieve existing VPBB
    VPBB = BlockIt->second;
  }

  return VPBB;
}

template <class CFGBuilder>
bool VPlanCFGBuilderBase<CFGBuilder>::isExternalDef(Value *Val) const {
  assert(!isa<Constant>(Val) &&
         "Constants should have been processed separately.");
  assert(!isa<MetadataAsValue>(Val) &&
         "MetadataAsValue should have been processed separately.");
  // All the Values that are not Instructions are considered external
  // definitions for now.
  Instruction *Inst = dyn_cast<Instruction>(Val);
  if (!Inst)
    return true;

  // Check whether Instruction definition is within the region being imported.
  return !contains(Inst);
}

template <class CFGBuilder>
void VPlanCFGBuilderBase<CFGBuilder>::addExternalUses(Value *Val,
                                                      VPValue *NewVPInst) {
  for (User *U : Val->users())
    if (auto Inst = dyn_cast<Instruction>(U))
      if (!contains(Inst)) {
        // Uses are required to be through a single PHI node, e.g. LLVM IR loop
        // (for loop vectorization) must be in LCSSA form.
        //
        // TODO: As region vectorization infrastructure evolves, we might
        // consider weakening the restriction, but having a single use does
        // looks very handy, and a phi is a natural way to represent single use.
        VPExternalUse *User = Plan->getVPExternalUse(cast<PHINode>(Inst));
        User->addOperandWithUnderlyingValue(NewVPInst, Val);
      }
}

// Create a new VPValue or retrieve an existing one for the Instruction's
// operand \p IROp. This function must only be used to create/retrieve VPValues
// for *Instruction's operands* and not to create regular VPInstruction's. For
// the latter, please, look at 'createVPInstructionsForVPBB'.
template <class CFGBuilder>
VPValue *VPlanCFGBuilderBase<CFGBuilder>::getOrCreateVPOperand(Value *IRVal) {
  // Constant operand
  if (Constant *IRConst = dyn_cast<Constant>(IRVal))
    return Plan->getVPConstant(IRConst);

  if (MetadataAsValue *MDAsValue = dyn_cast<MetadataAsValue>(IRVal))
    return Plan->getVPMetadataAsValue(MDAsValue);

  auto VPValIt = IRDef2VPValue.find(IRVal);
  if (VPValIt != IRDef2VPValue.end())
    // Operand has an associated VPInstruction or VPValue that was previously
    // created.
    return VPValIt->second;

  // Operand is not Constant or MetadataAsValue and doesn't have a previously
  // created VPInstruction/VPValue. This means that operand is:
  //   A) a definition external to VPlan,
  //   B) any other Value without specific representation in VPlan.
  // For now, we use VPValue to represent A and B and classify both as external
  // definitions. We may introduce specific VPValue subclasses for them in the
  // future.
  assert(isExternalDef(IRVal) && "Expected external definition as operand.");
  // A and B: Create VPValue and add it to the pool of external definitions and
  // to the Value->VPValue map.
  VPExternalDef *ExtDef = Plan->getVPExternalDef(IRVal);
  IRDef2VPValue[IRVal] = ExtDef;
  return ExtDef;
}

template <class CFGBuilder>
VPInstruction *
VPlanCFGBuilderBase<CFGBuilder>::createVPInstruction(Instruction *Inst) {
  if (auto *Br = dyn_cast<BranchInst>(Inst)) {
    // Branch instruction is not explicitly represented in VPlan but we need
    // to represent its condition bit when it's conditional.
    if (Br->isConditional())
      getOrCreateVPOperand(Br->getCondition());

    // Skip the rest of the Instruction processing for Branch instructions.
    return nullptr;
  }

  if (auto *Call = dyn_cast<CallInst>(Inst)) {
    if (Function *F = Call->getCalledFunction()) {
      if (F->getName() == "llvm.vplan.laneid") {
        LLVMContext &Ctx = F->getContext();
        auto *Zero = ConstantInt::getSigned(Type::getInt32Ty(Ctx), 0);
        auto *One = ConstantInt::getSigned(Type::getInt32Ty(Ctx), 1);
        return VPIRBuilder.createInductionInit(
            getOrCreateVPOperand(Zero), getOrCreateVPOperand(One),
            Instruction::Add, Call->getName());
      }
    }
  }

  VPInstruction *NewVPInst{nullptr};
  if (auto *Phi = dyn_cast<PHINode>(Inst)) {
    // Phi node's operands may have not been visited at this point. We create
    // an empty VPInstruction that we will fix once the whole plain CFG has
    // been built.
    NewVPInst = cast<VPInstruction>(VPIRBuilder.createPhiInstruction(Inst));
    PhisToFix.push_back(Phi);
  } else {
    // Translate LLVM-IR operands into VPValue operands and set them in the
    // new VPInstruction.
    SmallVector<VPValue *, 4> VPOperands;
    for (Value *Op : Inst->operands())
      VPOperands.push_back(getOrCreateVPOperand(Op));

    if (CmpInst *CI = dyn_cast<CmpInst>(Inst)) {
      assert(VPOperands.size() == 2 && "Expected 2 operands in CmpInst.");
      NewVPInst = VPIRBuilder.createCmpInst(VPOperands[0], VPOperands[1], CI);
    } else if (auto *GEP = dyn_cast<GetElementPtrInst>(Inst)) {
      // Build VPGEPInstruction to represent GEP instructions
      SmallVector<VPValue *, 3> IdxList(VPOperands.begin() + 1,
                                        VPOperands.end());
      if (GEP->isInBounds())
        NewVPInst = VPIRBuilder.createInBoundsGEP(VPOperands[0], IdxList, Inst);
      else
        NewVPInst = VPIRBuilder.createGEP(VPOperands[0], IdxList, Inst);
    } else if (auto *Call = dyn_cast<CallInst>(Inst)) {
      // Build VPCallInstruction to represent Call instructions.
      SmallVector<VPValue *, 3> ArgList(VPOperands.begin(),
                                        VPOperands.end() - 1);
      VPValue *CalledValue = getOrCreateVPOperand(Call->getCalledOperand());
      NewVPInst = VPIRBuilder.createCall(CalledValue, ArgList, Call);
    } else
      // Build VPInstruction for any arbitraty Instruction without specific
      // representation in VPlan.
      NewVPInst = cast<VPInstruction>(VPIRBuilder.createNaryOp(
          Inst->getOpcode(), Inst->getType(), VPOperands, Inst));
  }

  // Import underlying debug location and Operator attributes attached to this
  // instruction.
  NewVPInst->setDebugLocation(Inst->getDebugLoc());
  NewVPInst->copyOperatorFlagsFrom(Inst);
  return NewVPInst;
}

// Create new VPInstructions in a VPBasicBlock, given its BasicBlock
// counterpart. This function must be invoked in RPO so that the operands of a
// VPInstruction in \p BB have been visited before. VPInstructions representing
// Phi nodes are created without operands to honor the RPO traversal. They will
// be fixed later by 'fixPhiNodes'.
template <class CFGBuilder>
void VPlanCFGBuilderBase<CFGBuilder>::createVPInstructionsForVPBB(
    VPBasicBlock *VPBB, BasicBlock *BB) {
  VPIRBuilder.setInsertPoint(VPBB);
  for (Instruction &InstRef : *BB) {
    // There shouldn't be any VPValue for Inst at this point. Otherwise, we
    // visited Inst when we shouldn't, breaking the RPO traversal order.
    assert(!IRDef2VPValue.count(&InstRef) &&
           "Instruction shouldn't have been visited.");
    VPInstruction *NewVPInst = createVPInstruction(&InstRef);
    // createVPInstruction can return nullptr in case of a BranchInst.
    if (NewVPInst) {
      if (contains(&InstRef))
        addExternalUses(&InstRef, NewVPInst);

      IRDef2VPValue[&InstRef] = NewVPInst;
    }
  }
}

template <class CFGBuilder>
void VPlanCFGBuilderBase<CFGBuilder>::processBB(BasicBlock *BB) {
  // Create or retrieve the VPBasicBlock for this BB and create its
  // VPInstructions.
  VPBasicBlock *VPBB = getOrCreateVPBB(BB);
  createVPInstructionsForVPBB(VPBB, BB);

  // Set VPBB successors. We create empty VPBBs for successors if they don't
  // exist already. Instructions will be created when the successor is visited
  // during the RPO traversal.
  Instruction *TI = BB->getTerminator();
  assert(TI && "Terminator expected");
  unsigned NumSuccs = TI->getNumSuccessors();

  if (NumSuccs == 1) {
    VPBasicBlock *SuccVPBB = getOrCreateVPBB(TI->getSuccessor(0));
    assert(SuccVPBB && "VPBB Successor not found");
    VPBB->setOneSuccessor(SuccVPBB);
    VPBB->setCBlock(BB);
    VPBB->setTBlock(TI->getSuccessor(0));
  } else if (NumSuccs == 2) {
    VPBasicBlock *SuccVPBB0 = getOrCreateVPBB(TI->getSuccessor(0));
    assert(SuccVPBB0 && "Successor 0 not found");
    VPBasicBlock *SuccVPBB1 = getOrCreateVPBB(TI->getSuccessor(1));
    assert(SuccVPBB1 && "Successor 1 not found");

    // Set VPBB's condition bit.
    assert(isa<BranchInst>(TI) && "Unsupported terminator!");
    auto *Br = cast<BranchInst>(TI);
    Value *BrCond = Br->getCondition();
    VPValue *VPCondBit;
    if (Constant *ConstBrCond = dyn_cast<Constant>(BrCond))
      // Create new VPConstant for constant branch condition.
      VPCondBit = Plan->getVPConstant(ConstBrCond);
    else {
      // Look up the branch condition to get the corresponding VPValue
      // representing the condition bit in VPlan (which may be in another
      // VPBB).
      assert(IRDef2VPValue.count(BrCond) &&
             "Missing condition bit in IRDef2VPValue!");
      VPCondBit = IRDef2VPValue[BrCond];
    }
    VPBB->setTwoSuccessors(VPCondBit, SuccVPBB0, SuccVPBB1);

    VPBB->setCBlock(BB);
    VPBB->setTBlock(TI->getSuccessor(0));
    VPBB->setFBlock(TI->getSuccessor(1));

  } else if (NumSuccs == 0) {
    assert(!cast<ReturnInst>(TI)->getReturnValue() && "Expected void return!");
  } else {
    llvm_unreachable("Number of successors not supported");
  }
}

void VPlanLoopCFGBuilder::buildCFG() {
  // 1. Scan the body of the loop in a topological order to visit each basic
  // block after having visited its predecessor basic blocks.Create a VPBB for
  // each BB and link it to its successor and predecessor VPBBs. Note that
  // predecessors must be set in the same order as they are in the incomming IR.
  // Otherwise, there might be problems with existing phi nodes and algorithms
  // based on predecessors traversal.

  // Create loop PH. PH needs to be explicitly processed since it's not taken
  // into account by LoopBlocksDFS below. Since the loop PH may contain any
  // Instruction, related or not to the loop nest, we do not create
  // VPInstructions for them. Those Instructions used within the loop nest will
  // be modeled as external definitions.
  BasicBlock *PreheaderBB = TheLoop->getLoopPreheader();
  assert((PreheaderBB->getTerminator()->getNumSuccessors() == 1) &&
         "Unexpected loop preheader");
  VPBasicBlock *PreheaderVPBB = getOrCreateVPBB(PreheaderBB);
  // Create empty VPBB for Loop H so that we can link PH->H. H's VPInstructions
  // will be created during RPO traversal.
  VPBasicBlock *HeaderVPBB = getOrCreateVPBB(TheLoop->getHeader());
  // Preheader's predecessors will be set during the loop RPO traversal below.
  PreheaderVPBB->setOneSuccessor(HeaderVPBB);

  LoopBlocksRPO RPO(TheLoop);
  RPO.perform(LI);

  for (BasicBlock *BB : RPO) {
    processBB(BB);
  }

  // 2. Process outermost loop exit. We created an empty VPBB for the loop
  // exit BBs during the RPO traversal of the loop nest but their predecessors
  // have to be properly set. Since a loop exit may contain any Instruction,
  // related or not to the loop nest, we do not create VPInstructions for them.
  SmallVector<BasicBlock *, 2> LoopExits;
  TheLoop->getUniqueExitBlocks(LoopExits);

  // 3. The whole CFG has been built at this point so all the input Values must
  // have a VPlan couterpart. Fix VPlan phi nodes by adding their corresponding
  // VPlan operands.
  fixPhiNodes();

  // 4. Set the EntryBlock and the ExitBlock of SESE region.
  // Create EntryBlock.
  VPBasicBlock *PlanEntryBB =
      new VPBasicBlock(VPlanUtils::createUniqueName("BB"), Plan);
  Plan->insertAtFront(PlanEntryBB);
  PlanEntryBB->appendSuccessor(PreheaderVPBB);

  // Create ExitBlock.
  VPBasicBlock *NewPlanExitBB =
      new VPBasicBlock(VPlanUtils::createUniqueName("BB"), Plan);
  Plan->insertAtBack(NewPlanExitBB);

  // Update CFG for ExitBlock.
  if (LoopExits.size() == 1) {
    VPBasicBlock *LoopExitVPBB = BB2VPBB[LoopExits.front()];
    LoopExitVPBB->appendSuccessor(NewPlanExitBB);
  } else {
    // If there are multiple exits in the outermost loop, we need another dummy
    // block as landing pad for all of them.
    assert(LoopExits.size() > 1 && "Wrong number of exit blocks");

    VPBasicBlock *LandingPad =
        new VPBasicBlock(VPlanUtils::createUniqueName("BB"), Plan);
    LandingPad->insertBefore(NewPlanExitBB);

    // Connect multiple exits to landing pad
    for (auto ExitBB : make_range(LoopExits.begin(), LoopExits.end())) {
      VPBasicBlock *ExitVPBB = BB2VPBB[ExitBB];
      ExitVPBB->appendSuccessor(LandingPad);
    }

    // Connect landing pad to ExitBlock.
    LandingPad->appendSuccessor(NewPlanExitBB);
  }

  return;
}

void VPlanFunctionCFGBuilder::buildCFG() {
  ReversePostOrderTraversal<BasicBlock *> RPOT(&F.getEntryBlock());
  assert(count_if(F,
                  [](const BasicBlock &BB) {
                    return !isa<BranchInst>(BB.getTerminator());
                  }) == 1 &&
         "Unsupported function for region vectorization!");

  for (BasicBlock *BB : RPOT)
    processBB(BB);

  fixPhiNodes();

  // Fix exit block location.
  for (auto &BB : F) {
    if (isa<ReturnInst>(BB.getTerminator())) {
      assert(cast<ReturnInst>(BB.getTerminator())->getNumOperands() == 0 &&
             "Only void return is supported for region vectorization!");
      VPBasicBlock *VPBB = BB2VPBB[&BB];

      Plan->getVPBasicBlockList().remove(VPBB->getIterator());
      Plan->insertAtBack(VPBB);
      break;
    }
  }

  return;
}

// Implicit instantiation above only creates definitions for methods used in
// this compilation unit, but others that are supposed to be available through
// the derived class' interface have to be forced through explicit
// instantiation.
template class llvm::vpo::VPlanCFGBuilderBase<VPlanLoopCFGBuilder>;
template class llvm::vpo::VPlanCFGBuilderBase<VPlanFunctionCFGBuilder>;
