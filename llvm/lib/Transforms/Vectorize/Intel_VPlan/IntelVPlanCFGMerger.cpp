//===-- IntelVPlanCFGMerger.cpp -----------------------------------------===//
//
//   Copyright (C) 2021 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements the algorithm that creates auxiliary loops (peel/remainder)
/// and merges them into one flattened CFG.
///
//===----------------------------------------------------------------------===//
#include "IntelVPlanCFGMerger.h"
#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanDivergenceAnalysis.h"

#define DEBUG_TYPE "VPlanCFGMerger"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool>
    EmitPushPopVF("vplan-enable-pushvf", cl::init(false), cl::Hidden,
                  cl::desc("Emit pushvf and popvf VPInstrucitons."));

VPBasicBlock *VPlanCFGMerger::createMergeBlock(VPBasicBlock *InsertAfter,
                                               VPBasicBlock *SplitBlock,
                                               bool UseLiveIn) {
  // Create new block after insertion point
  VPBasicBlock *MergeBlock =
      VPBlockUtils::splitBlockEnd(InsertAfter, Plan.getVPLoopInfo(),
                                  Plan.getDT(), Plan.getPDT());
  VPBuilder Builder;
  Builder.setInsertPoint(MergeBlock);
  // Create phi nodes for each liveout value.
  for (auto LiveOut : Plan.liveOutValues()) {
    unsigned MergeId = LiveOut->getMergeId();
    // New phis are created with merge id corresponding to the liveout.
    VPPHINode *NewMerge =
        new VPPHINode(MergeId, LiveOut->getType());
    Builder.insert(NewMerge);
    Plan.getVPlanDA()->markUniform(*NewMerge);
    if (SplitBlock) {
      // Add phi operand coming from split block. It can be either an original
      // incoming value or a liveout. Remember, liveout is symbolic and is
      // replaced by its operand.
      VPValue *InVal = UseLiveIn ? ExtVals.getOriginalIncomingValue(MergeId)
                                 : LiveOut;
      NewMerge->addIncoming(InVal, SplitBlock);
    }
  }
  return MergeBlock;
}

void VPlanCFGMerger::updateMergeBlockIncomings(VPBasicBlock *MergeBlock,
                                               VPBasicBlock *SplitBlock,
                                               bool UseLiveIn) {
  // Go through phi nodes in the MergeBlock and set their incoming values from
  // SplitBlock, the incoming values are either liveouts or original incoming
  // values.
  for (auto &Node : MergeBlock->getVPPhis()) {
    unsigned MergeId = Node.getMergeId();
    if (MergeId == VPExternalUse::UndefMergeId)
      llvm_unreachable("Unexpected instruction in a merge block");

    VPValue *InVal = UseLiveIn ? ExtVals.getOriginalIncomingValue(MergeId)
                               : Plan.getLiveOutValue(MergeId);
    if (Node.getBlockIndex(SplitBlock) == -1) {
      Node.addIncoming(InVal, SplitBlock);
      continue;
    }
    assert(Node.getIncomingValue(SplitBlock) == InVal &&
           "Unexpected incoming replacement");
  }
}

VPVectorTripCountCalculation *
VPlanCFGMerger::findVectorTCInst(VPBasicBlock *StartBB) {
  if (VectorTripCount)
    return VectorTripCount;

  auto findInBB = [](VPBasicBlock *BB) -> VPVectorTripCountCalculation * {
    auto Iter = llvm::find_if(*BB, [](VPInstruction &I) {
      return isa<VPVectorTripCountCalculation>(I);
    });

    if (Iter == BB->end())
      return nullptr;
    return cast<VPVectorTripCountCalculation>(&*Iter);
  };

  for (; StartBB; StartBB = StartBB->getSinglePredecessor())
    if ((VectorTripCount = findInBB(StartBB)))
      break;

  assert(VectorTripCount && "Can't find vector TC");
  return VectorTripCount;
}

VPBasicBlock *VPlanCFGMerger::findFirstNonEmptyBB() const {
  VPBasicBlock *BB = Plan.getEntryBlock();
  for (; BB && BB->terminator() == BB->begin(); BB = BB->getSingleSuccessor())
    ;
  assert(BB && "Non-empty VPlan expected");
  return BB;
}

VPBasicBlock *
VPlanCFGMerger::createVPlanLoopTopTest(VPBasicBlock *FallThroughMergeBlock) {

  VPLoop *Loop = *Plan.getVPLoopInfo()->begin();
  // Create new basic block before first non-empty block.
  VPBasicBlock *VectorTopTestBB = findFirstNonEmptyBB();
  VPBasicBlock *FirstExecutableBB =
      VPBlockUtils::splitBlockBegin(VectorTopTestBB, Plan.getVPLoopInfo(),
                                    Plan.getDT(), Plan.getPDT());
  FirstExecutableBB->setName("vector.ph");
  // Find and move vector trip count and original trip count instructions to
  // the new block.
  VPBasicBlock *Preheader = Loop->getLoopPreheader();
  assert(Preheader && "Loop preheader is expected to exist.");

  VPVectorTripCountCalculation *VectorTC = findVectorTCInst(Preheader);

  VPBuilder Builder;
  Builder.setInsertPoint(VectorTopTestBB);
  if (EmitPushPopVF) {
    VPValue *PushVF =
        Builder.create<VPPushVF>("pushvf", Plan.getLLVMContext(), VF, UF);
    Plan.getVPlanDA()->markUniform(*PushVF);
  }

  VPValue *OrigTC = VectorTC->getOperand(0);
  if (auto *OrigTCInst = dyn_cast<VPInstruction>(OrigTC))
    OrigTCInst->moveBefore(*VectorTopTestBB, VectorTopTestBB->terminator());
  VectorTC->moveBefore(*VectorTopTestBB, VectorTopTestBB->terminator());

  // Generate the check for vector TC is 0 and branch according to the check.
  auto *Zero =
      Plan.getVPConstant(ConstantInt::getNullValue(VectorTC->getType()));
  auto *VectorTopTest =
      Builder.createCmpInst(CmpInst::ICMP_EQ, Zero, VectorTC, "vec.tc.check");
  Plan.getVPlanDA()->markUniform(*VectorTopTest);
  VectorTopTestBB->setTerminator(FallThroughMergeBlock, FirstExecutableBB, VectorTopTest);

  if (EmitPushPopVF) {
    VPLoop *Loop = *Plan.getVPLoopInfo()->begin();
    VPBasicBlock *ExitBB = Loop->getUniqueExitBlock();
    assert(ExitBB && "Expecting a unique exit block.");
    Builder.setInsertPoint(ExitBB);
    VPValue *PopVF = Builder.createNaryOp(
        VPInstruction::PopVF, Type::getVoidTy(*Plan.getLLVMContext()), {});
    Plan.getVPlanDA()->markUniform(*PopVF);
  }
  return VectorTopTestBB;
}

VPBasicBlock *VPlanCFGMerger::createRemainderTopTest(VPBasicBlock *InsertAfter,
                                     VPBasicBlock *RemPreheader,
                                     VPBasicBlock *FinalMergeBlock) {
  VPLoop *Loop = *Plan.getVPLoopInfo()->begin();
  VPBasicBlock *Preheader = Loop->getLoopPreheader();

  // Find vector tc and original tc instructions
  VPVectorTripCountCalculation *VectorTC = findVectorTCInst(Preheader);
  VPValue *OrigTC = VectorTC->getOperand(0);
  // Create a new block after insertion point.
  VPBasicBlock *RemIterTestBB = VPBlockUtils::splitBlockEnd(
      InsertAfter, Plan.getVPLoopInfo(), Plan.getDT(), Plan.getPDT());
  RemIterTestBB->setName("middle.block");

  // Generate a check for vector tc is not equal original tc and branch
  // accoring to the check.
  VPBuilder Builder;
  Builder.setInsertPoint(RemIterTestBB);
  auto *RemTCCheck =
      Builder.createCmpInst(CmpInst::ICMP_NE, OrigTC, VectorTC, "remtc.check");
  Plan.getVPlanDA()->markUniform(*RemTCCheck);

  RemIterTestBB->setTerminator(RemPreheader, FinalMergeBlock, RemTCCheck);

  return RemIterTestBB;
}

static bool isMergeBlock(VPBasicBlock *BB) {
  // Merge block can contain only merge phi and terminator.
  if (BB->empty() || isa<VPBranchInst>(BB->begin()))
    return false;
  return llvm::all_of(*BB, [](VPInstruction &I) {
    return isa<VPBranchInst>(I) ||
           (isa<VPPHINode>(I) &&
            cast<VPPHINode>(I).getMergeId() != VPExternalUse::UndefMergeId);
  });
}

VPBasicBlock *VPlanCFGMerger::createScalarRemainder(Loop *OrigLoop,
                                                    VPBasicBlock *InsertAfter,
                                                    VPBasicBlock *FinalBB) {
  assert(isMergeBlock(InsertAfter) && "Expected merge block");
  // Create a new block for LoopReuse
  VPBasicBlock *RemainderBB = VPBlockUtils::splitBlockEnd(
      InsertAfter, Plan.getVPLoopInfo(), Plan.getDT(), Plan.getPDT());
  VPBuilder Builder;
  Builder.setInsertPoint(RemainderBB);
  auto *Remainder = Builder.create<VPReuseLoop>("orig.loop", OrigLoop);
  Plan.getVPlanDA()->markUniform(*Remainder);
  const ScalarInOutList &ScalarInOuts =
      *Plan.getExternals().getScalarLoopInOuts(OrigLoop);
  // InsertAfter is a merge block. Go through its phis and add LoopReuse liveins
  // and OrigLiveOuts for original loop live out values. The needed scalar
  // values are taken from the ScalarInOuts descriptor.
  for (auto &I : *InsertAfter) {
    if (!isa <VPPHINode>(I))
      continue;
    unsigned Id = cast<VPPHINode>(I).getMergeId();
    const ScalarInOutDescr *ScalarDescr = ScalarInOuts.getDescr(Id);
    assert(ScalarDescr && "InOutDescr not found");
    auto LO = Builder.create<VPOrigLiveOut>("orig.liveout", Remainder,
                                  ScalarDescr->getLiveOut(), Id);
    Plan.getVPlanDA()->markUniform(*LO);
    PHINode *OrigPhi = ScalarDescr->getPhi();
    Remainder->addLiveIn(&I,
                         &OrigPhi->getOperandUse(ScalarDescr->getStartOpNum()));
  }
  BasicBlock *OrigLatch = OrigLoop->getLoopLatch();
  BasicBlock *OrigHeader = OrigLoop->getHeader();
  auto *Br = cast<BranchInst>(OrigLatch->getTerminator());

  // Add info to replace the successor in scalar exit block.
  Remainder->addLiveIn(FinalBB, Br->getOperand(1) == OrigHeader
                                    ? &Br->getOperandUse(2)
                                    : &Br->getOperandUse(1));
  return RemainderBB;
}

void VPlanCFGMerger::updateMergeBlockByScalarLiveOuts(VPBasicBlock *BB,
                                                      VPBasicBlock *InBlock) {
  assert(isMergeBlock(BB) && "Expected merge block");
  DenseMap<unsigned, VPPHINode *> MergePhis;
  for (auto &PN: BB->getVPPhis())
    MergePhis[PN.getMergeId()] = &PN;

  for (auto &I : *InBlock)
    if (auto *OrigLI = dyn_cast<VPOrigLiveOut>(&I))
      MergePhis[OrigLI->getMergeId()]->addIncoming(OrigLI, InBlock);
}

void VPlanCFGMerger::updateExternalUsesOperands(VPBasicBlock *FinalBB) {
  assert(isMergeBlock(FinalBB) && "Expected merge block");
  // Go through FinalBB phi nodes and set them as operands of
  // the corresponding external uses.
  for (auto &I : *FinalBB)
    if (auto MNode = dyn_cast<VPPHINode>(&I)) {
      VPExternalUse *EUse = ExtVals.getVPExternalUse(MNode->getMergeId());
      if (EUse->hasUnderlying()) {
        assert(EUse->getNumOperands() == 0 && "Unexpected operand");
        EUse->addOperand(MNode);
      }
    }
}

void VPlanCFGMerger::createSimpleVectorRemainderChain(Loop *OrigLoop) {
  VPLoop *Loop = *Plan.getVPLoopInfo()->begin();
  VPBasicBlock *ExitBB = Loop->getUniqueExitBlock();
  assert(ExitBB && "Expecting a unique exit block.");
  VPBasicBlock *PostExitBB = ExitBB->getSingleSuccessor();

  // Ensure that first merge point is created, adding incoming values from loop
  // exit.
  if (!PostExitBB || !isMergeBlock(PostExitBB))
    // This will create a block with merge-phis which have live-outs as operands.
    //   ExitBB:  ---- this one exists
    //      i32 [[VP_SUM_07_RED_FINAL:%.*]] = reduction-final{u_add} i32 [[VP_ADD]]
    //      i64 [[VP_INDVARS_IV_IND_FINAL:%.*]] = induction-final{add} i64 live-in1 i64 1
    //      br PostExitBB
    //
    //   PostExitBB: # preds: ExitBB ----- the new block
    //      i32 merge.phi0 = phi-merge  [ i32 live-out0, ExitBB ]
    //      i64 merge.phi1 = phi-merge  [ i64 live-out1, ExitBB ]
    //      br <External Block>
    PostExitBB = createMergeBlock(ExitBB, ExitBB, false /*UseLiveIn*/);
  PostExitBB->setName("scalar.ph");

  // Create vector top test:
  // %c = icmp eq i64 %VecTC, 0
  // br i1 %c, label %post.exit, label %vec.preheader
  VPBasicBlock *VectorTopTest = createVPlanLoopTopTest(PostExitBB);

  // Update incoming values from true path of top test.
  updateMergeBlockIncomings(PostExitBB, VectorTopTest, true /*UseLiveIn*/);

  // Create final merge point, after remainder. This creates
  // a list of merge-phis w/o operands. The operands are added
  // later in the next steps.
  VPBasicBlock *FinalBB = createMergeBlock(PostExitBB);

  // Insert scalar remainder after merge point, having as a successor
  // the FinalBB.
  // After this point we have the following CFG.
  //           VectorTopTest
  //              /     \
  //             /    VectorPreheader |
  //            /        |            |
  //           +      LoopBody        | existing VPlan blocks
  //           |         |            |
  //           |      ExitBB          |
  //           |      /
  //           |     /  ------ the remainder toptest is inserted here
  //           |    /          on the next step
  //       PostExitBB
  //           |
  //       RemainderBB
  //              \
  //              FinalBB
  //
  VPBasicBlock *RemainderBB =
      createScalarRemainder(OrigLoop, PostExitBB, FinalBB);

  // Update final merge point with values coming from remainder.
  updateMergeBlockByScalarLiveOuts(FinalBB, RemainderBB);

  // Insert remainning iterations check after loop exit, with two successors,
  // post exit and final block.
  // %c = icmp ne i64 %VecTC, %OrigTC
  // br i1 %c, label %post.exit, label %final.bb
  VPBasicBlock *RemTCBB = createRemainderTopTest(ExitBB, PostExitBB, FinalBB);

  // Update final merge point with values coming from remainder.
  updateMergeBlockIncomings(FinalBB, RemTCBB, false /*UseLiveIn*/);

  // Set the merge-phis from FinalBB as operands of VPExternalUses.
  updateExternalUsesOperands(FinalBB);

  Plan.setExplicitRemainderUsed();

  // Invalidate SVA results as VPlan has been changed.
  Plan.invalidateAnalyses({VPAnalysisID::SVA});

  Plan.computeDT();
  Plan.computePDT();
}
