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
#include "IntelLoopVectorizationPlanner.h"
#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanDivergenceAnalysis.h"

#define DEBUG_TYPE "VPlanCFGMerger"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool, true>
    EmitPushPopVFOpt("vplan-enable-pushvf", cl::location(EmitPushPopVF),
                     cl::Hidden,
                     cl::desc("Emit pushvf and popvf VPInstrucitons."));

namespace llvm {
namespace vpo {
bool EmitPushPopVF = false;
} // namespace vpo
} // namespace llvm

static LoopVPlanDumpControl MergeNewPlansDumpControl("create-in-merge",
                                                     "creation during merge");
static LoopVPlanDumpControl
    MergeSkeletonDumpControl("merge-skeleton", "merge skeleton creation");

VPBasicBlock *VPlanCFGMerger::createMergeBlock(VPBasicBlock *InsertAfter,
                                               VPBasicBlock *SplitBlock,
                                               bool UseLiveIn) {
  // Create new block after insertion point
  VPBasicBlock *MergeBlock = VPBlockUtils::splitBlockEnd(
      InsertAfter, Plan.getVPLoopInfo(), nullptr /*DT*/, nullptr /*PDT*/);
  // Leaving for a separate change, too many tests updated.
  // MergeBlock->setName(VPlanUtils::createUniqueName("merge.blk"));
  return createMergePhis(MergeBlock, SplitBlock, UseLiveIn);
}

VPBasicBlock *VPlanCFGMerger::createMergePhis(VPBasicBlock *MergeBlock,
                                              VPBasicBlock *SplitBlock,
                                              bool UseLiveIn) {
  VPBuilder Builder;
  Builder.setInsertPoint(MergeBlock);
  // Create phi nodes for each liveout value.
  for (auto LiveOut : Plan.liveOutValues()) {
    unsigned MergeId = LiveOut->getMergeId();
    // New phis are created with merge id corresponding to the liveout.
    VPPHINode *NewMerge = new VPPHINode(MergeId, LiveOut->getType());
    Builder.insert(NewMerge);
    Plan.getVPlanDA()->markUniform(*NewMerge);
    if (SplitBlock) {
      // Add phi operand coming from split block. It can be either an original
      // incoming value or a liveout. Remember, liveout is symbolic and is
      // replaced by its operand.
      VPValue *InVal =
          UseLiveIn ? ExtVals.getOriginalIncomingValue(MergeId) : LiveOut;
      NewMerge->addIncoming(InVal, SplitBlock);
    }
  }
  return MergeBlock;
}

VPBasicBlock *
VPlanCFGMerger::createMergeBlockBefore(VPBasicBlock *InsertBefore) {
  VPBasicBlock *MergeBlock =
      new VPBasicBlock(VPlanUtils::createUniqueName("merge.blk"), &Plan);
  VPBlockUtils::insertBlockBefore(MergeBlock, InsertBefore);
  return createMergePhis(MergeBlock);
}

void VPlanCFGMerger::updateMergeBlockIncomings(VPlan &P,
                                               VPBasicBlock *MergeBlock,
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
                               : P.getLiveOutValue(MergeId);
    if (Node.getBlockIndex(SplitBlock) == -1) {
      if (!InVal)
        InVal = Plan.getVPConstant(UndefValue::get(Node.getType()));
      Node.addIncoming(InVal, SplitBlock);
      continue;
    }
    assert(Node.getIncomingValue(SplitBlock) == InVal &&
           "Unexpected incoming replacement");
  }
}

void VPlanCFGMerger::updateMergeBlockIncomings(PlanDescr &Descr,
                                               VPBasicBlock *MergeBlock,
                                               VPBasicBlock *SplitBlock,
                                               bool UseLiveIn) {
  using LT = CfgMergerPlanDescr::LoopType;

  if (UseLiveIn || Descr.Type == LT::LTMain) {
    updateMergeBlockIncomings(*Descr.Plan, MergeBlock, SplitBlock, UseLiveIn);
    return;
  }

  // Go through phi nodes in the MergeBlock and set their incoming values from
  // SplitBlock to VPlan adapter.
  auto Adapter = llvm::find_if(*Descr.FirstBB, [](const VPInstruction &I) {
    return isa<VPlanAdapter>(I);
  });
  assert(Adapter != Descr.FirstBB->end() && "expected non-null adapter");

  for (auto &Phi : MergeBlock->getVPPhis()) {
    unsigned MergeId = Phi.getMergeId();
    if (MergeId == VPExternalUse::UndefMergeId)
      llvm_unreachable("Unexpected instruction in a merge block");

    if (Phi.getBlockIndex(SplitBlock) == -1) {
      Phi.addIncoming(&*Adapter, SplitBlock);
      continue;
    }
    assert(Phi.getIncomingValue(SplitBlock) == &*Adapter &&
           "Unexpected incoming replacement");
  }
}

VPVectorTripCountCalculation *
VPlanCFGMerger::findVectorTCInst(VPBasicBlock *BB) const {
  auto findInBB = [](VPBasicBlock *B) -> VPVectorTripCountCalculation * {
    auto Iter = llvm::find_if(*B, [](VPInstruction &I) {
      return isa<VPVectorTripCountCalculation>(I);
    });

    if (Iter == B->end())
      return nullptr;
    return cast<VPVectorTripCountCalculation>(&*Iter);
  };
  VPVectorTripCountCalculation *Ret = nullptr;
  for (; BB; BB = BB->getSinglePredecessor())
    if ((Ret = findInBB(BB)))
      break;

  assert(Ret && "Can't find vector TC");
  return Ret;
}

VPVectorTripCountCalculation *VPlanCFGMerger::findVectorUB(VPlan &P) const {
  VPlanNonMasked &VecPlan = cast<VPlanNonMasked>(P);
  if (VectorUBs.count(&VecPlan))
    return VectorUBs[&VecPlan];

  VPLoop *Loop = *VecPlan.getVPLoopInfo()->begin();
  VPBasicBlock *Preheader = Loop->getLoopPreheader();
  assert(Preheader && "Loop preheader is expected to exist.");

  VPVectorTripCountCalculation *Ret = findVectorTCInst(Preheader);
  VectorUBs[&VecPlan] = Ret;
  return Ret;
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
                                     nullptr /*DT*/, nullptr /*PDT*/);
  FirstExecutableBB->setName("vector.ph");
  // Find and move vector trip count and original trip count instructions to
  // the new block.
  VPBasicBlock *Preheader = Loop->getLoopPreheader();
  assert(Preheader && "Loop preheader is expected to exist.");

  VPVectorTripCountCalculation *VectorTC = findVectorTCInst(Preheader);

  VPBuilder Builder;
  Builder.setInsertPoint(VectorTopTestBB);
  if (EmitPushPopVF) {
    VPValue *PushVF = Builder.create<VPPushVF>("pushvf", Plan.getLLVMContext(),
                                               MainVF, MainUF);
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
      InsertAfter, Plan.getVPLoopInfo(), nullptr /*DT*/, nullptr /*PDT*/);
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

static Use *getExitBBUse(Loop *Loop) {
  BasicBlock *Latch = Loop->getLoopLatch();
  BasicBlock *Header = Loop->getHeader();
  auto *Br = cast<BranchInst>(Latch->getTerminator());
  return Br->getOperand(1) == Header ? &Br->getOperandUse(2)
                                     : &Br->getOperandUse(1);
}

VPBasicBlock *VPlanCFGMerger::createScalarRemainder(Loop *OrigLoop,
                                                    VPBasicBlock *InsertAfter,
                                                    VPBasicBlock *FinalBB) {
  assert(isMergeBlock(InsertAfter) && "Expected merge block");
  // Create a new block for LoopReuse
  VPBasicBlock *RemainderBB = VPBlockUtils::splitBlockEnd(
      InsertAfter, Plan.getVPLoopInfo(), nullptr /*DT*/, nullptr /*PDT*/);
  VPBuilder Builder;
  Builder.setInsertPoint(RemainderBB);
  auto *Remainder =
      Builder.create<VPScalarRemainder>("orig.loop", OrigLoop, false);
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
    if (OrigPhi)
      Remainder->addLiveIn(
          &I, &OrigPhi->getOperandUse(ScalarDescr->getStartOpNum()));
  }
  // Add info to replace the successor in scalar exit block.
  Remainder->addLiveIn(FinalBB, getExitBBUse(OrigLoop));
  return RemainderBB;
}

template <typename VPlanType, typename VPInstructionType>
class ScalarPeelOrRemainderVPlanFabBase {
  virtual void addRemainderLiveIn(ScalarInOutDescr *Descr,
                                  VPInstructionType *I) {}
  virtual void updateLoopExit(VPInstructionType *I, VPValue *Blk, Use *Val) = 0;
  virtual void setPlanName(VPlan &MainPlan) = 0;
  virtual const char *getFirstBlockName() = 0;

protected:
  virtual ~ScalarPeelOrRemainderVPlanFabBase() {}
  VPlanType *NewPlan;

  // Worker routine to create a body of a brand new VPlanScalarPeel
  // or VPlanScalarRemainder:
  //  - Create livein/liveout lists
  //  - Create two basic blocks. The first one contains either
  //    VPScalarRemainder or VPScalarPeel instruction (which does represent
  //    the loop) and a set of VPOrigLiveOut instructions.
  //    The second block is empty. It will serve as landing pad for the scalar
  //    loop (one behind the instruction). Exit form the loop is redirected
  //    to the pad block.
  //  - Liveins for the VPScalar{Peel,Remainder} instruction are also filled
  //    in, except the peel count for a peel loop.
  //  - Create a new DA instance.
  VPlanType *runImpl(VPlan &MainPlan, Loop *OrigLoop) {
    NewPlan =
        new VPlanType(MainPlan.getExternals(), MainPlan.getUnlinkedVPInsts());
    setPlanName(MainPlan);

    // Create live-in and live-out lists.
    const ScalarInOutList *ScalarInOuts =
        NewPlan->getExternals().getScalarLoopInOuts(OrigLoop);

    // First create live-ins
    VPLiveInOutCreator LICreator(*NewPlan);
    LICreator.createLiveInsForScalarVPlan(*ScalarInOuts,
                                          MainPlan.getLiveInValuesSize());
    // Create the code
    auto *FirstBB = new VPBasicBlock(
        VPlanUtils::createUniqueName(getFirstBlockName()), NewPlan);
    NewPlan->insertAtBack(FirstBB);
    FirstBB->setTerminator();

    VPBuilder Builder;
    Builder.setInsertPoint(FirstBB);
    auto *ScalarLoopInstr =
        Builder.create<VPInstructionType>("orig.loop", OrigLoop, false);

    // Go through ScalarInOuts and add scalar remainder liveins (no liveins
    // for peel) and VPOrigLiveOuts for original loop live out values. The
    // needed scalar values are taken from the ScalarInOuts descriptor.
    DenseMap<int, VPValue *> LiveOuts;
    for (auto ScalarDescr : ScalarInOuts->list()) {
      int MergeId = ScalarDescr->getId();
      auto LO = Builder.create<VPOrigLiveOut>(
          "orig.liveout", ScalarLoopInstr, ScalarDescr->getLiveOut(), MergeId);
      LiveOuts[MergeId] = LO;
      addRemainderLiveIn(ScalarDescr, ScalarLoopInstr); // No-op for peel
    }
    // Create live out list.
    LICreator.createLiveOutsForScalarVPlan(
        *ScalarInOuts, MainPlan.getLiveOutValuesSize(), LiveOuts);

    // Add info to replace the successor in scalar exit block.
    // For that we create one more basic block in the scalar VPlan and set it
    // as the exit block in the scalar loop.
    auto *FinalBB =
        new VPBasicBlock(VPlanUtils::createUniqueName("BB"), NewPlan);
    FinalBB->insertAfter(FirstBB);
    FinalBB->setTerminator();
    FirstBB->setTerminator(FinalBB);

    // Set the basic block where to jump after the scalar loop.
    // For that we update the operand of the branch instruction that
    // corresponds to the loop exit, taking either one its operand use or
    // another. We have different interfaces for peel and remainder (see
    // details in updateLoopExit specializations). Peel has restrictions on
    // its arguments. For remainder we do that directly. For peel - using
    // special interface.
    updateLoopExit(ScalarLoopInstr, FinalBB, getExitBBUse(OrigLoop));

    // Finnaly, create DA.
    auto VPDA = std::make_unique<VPlanDivergenceAnalysisScalar>();
    NewPlan->setVPlanDA(std::move(VPDA));
    return NewPlan;
  }
};

// Peel specialization
template <bool>
class ScalarPeelOrRemainderVPlanFab
    : public ScalarPeelOrRemainderVPlanFabBase<VPlanScalarPeel, VPScalarPeel> {
  using VPlanType = VPlanScalarPeel;
  using VPInstructionType = VPScalarPeel;

  virtual const char *getFirstBlockName() override { return "PeelBlk"; }
  virtual void setPlanName(VPlan &MainPlan) override {
    NewPlan->setName(MainPlan.getName() + ".ScalarPeel");
  }
  virtual void updateLoopExit(VPInstructionType *I, VPValue *Blk,
                              Use *Val) override {
    I->setTargetLabel(Blk, Val);
  }

public:
  VPlanType *create(VPlan &MainPlan, Loop *OrigLoop) {
    return runImpl(MainPlan, OrigLoop);
  }
};

// Remainder specialization
template <>
class ScalarPeelOrRemainderVPlanFab<false>
    : public ScalarPeelOrRemainderVPlanFabBase<VPlanScalarRemainder,
                                               VPScalarRemainder> {
  using VPlanType = VPlanScalarRemainder;
  using VPInstructionType = VPScalarRemainder;

  virtual const char *getFirstBlockName() override { return "RemBlk"; }
  virtual void setPlanName(VPlan &MainPlan) override {
    NewPlan->setName(MainPlan.getName() + ".ScalarRemainder");
  }

  virtual void addRemainderLiveIn(ScalarInOutDescr *Descr,
                                  VPInstructionType *I) override {
    int Id = Descr->getId();
    if (PHINode *OrigPhi = Descr->getPhi())
      I->addLiveIn(const_cast<VPLiveInValue *>(NewPlan->getLiveInValue(Id)),
                   &OrigPhi->getOperandUse(Descr->getStartOpNum()));
  }
  virtual void updateLoopExit(VPInstructionType *I, VPValue *Blk,
                              Use *Val) override {
    I->addLiveIn(Blk, Val);
  }

public:
  VPlanType *create(VPlan &MainPlan, Loop *OrigLoop) {
    return runImpl(MainPlan, OrigLoop);
  }
};

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
  updateMergeBlockIncomings(Plan, PostExitBB, VectorTopTest,
                            true /*UseLiveIn*/);

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
  updateMergeBlockIncomings(Plan, FinalBB, RemTCBB, false /*UseLiveIn*/);

  // Set the merge-phis from FinalBB as operands of VPExternalUses.
  updateExternalUsesOperands(FinalBB);

  Plan.setExplicitRemainderUsed();

  // Invalidate SVA results as VPlan has been changed.
  Plan.invalidateAnalyses({VPAnalysisID::SVA});

  Plan.computeDT();
  Plan.computePDT();
}

void VPlanCFGMerger::createPlans(LoopVectorizationPlanner &Planner,
                                 const SingleLoopVecScenario &Scen,
                                 std::list<CfgMergerPlanDescr> &PlanDescrs,
                                 Loop *OrigLoop, VPlan &MainPlan,
                                 VPAnalysesFactory &VPAF) {

  using ScalarPeelVPlanFab = ScalarPeelOrRemainderVPlanFab<true>;
  using ScalarRemainderVPlanFab = ScalarPeelOrRemainderVPlanFab<false>;

  auto dumpNewVPlan = [](const VPlan *P) -> void {
    VPLAN_DUMP(MergeNewPlansDumpControl, P);
  };
  auto dumpExistingVPlan = [](const VPlan *P) -> void {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    VPLAN_DUMP(MergeNewPlansDumpControl.dumpPlain(),
               "adding existing one during merge", P);
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  };
  using LK = SingleLoopVecScenario::AuxLoopKind;
  using LT = CfgMergerPlanDescr::LoopType;

  // The list of original VPlans that are already passed to the PlanDescrs.
  // We can't use VPlans twice and clone them when it's needed.
  SmallSet<VPlan *, 4> UsedPlans;

  LK NewMainKind = Scen.getMainKind();
  assert((NewMainKind == LK::LKVector || NewMainKind == LK::LKMasked) &&
         "unexpected main loop kind");
  unsigned MainVF = Scen.getMainVF();
  VPlan *NewMainPlan = NewMainKind == LK::LKVector
                           ? &MainPlan
                           : Planner.getMaskedVPlanForVF(MainVF);

  VPlan *CurPlan = NewMainPlan;
  UsedPlans.insert(CurPlan);

  bool ScalarUsed = false;

  switch (Scen.getPeelKind()) {
  case LK::LKNone:
    break;
  case LK::LKScalar: {
    ScalarUsed = true;
    ScalarPeelVPlanFab Fab;
    auto ScalarPlan = Fab.create(MainPlan, OrigLoop);
    CurPlan = Planner.addAuxiliaryVPlan(*ScalarPlan);
    PlanDescrs.push_front({LT::LTPeel, 1, CurPlan});
    dumpNewVPlan(CurPlan);
    break;
  }
  case LK::LKMasked:
    CurPlan = Planner.getMaskedVPlanForVF(Scen.getPeelVF());
    if (UsedPlans.count(CurPlan)) {
      auto Clone = cast<VPlanMasked>(CurPlan)->clone(
          VPAF, VPlanVector::UpdateDA::CloneDA);
      CurPlan = Planner.addAuxiliaryVPlan(*Clone);
      dumpNewVPlan(CurPlan);
    } else {
      dumpExistingVPlan(CurPlan);
    }
    UsedPlans.insert(CurPlan);
    PlanDescrs.push_front({LT::LTPeel, Scen.getPeelVF(), CurPlan});
    break;
  case LK::LKVector:
    llvm_unreachable("unsupported peel kind");
    break;
  }

  CurPlan = NewMainPlan;
  unsigned MainUF = Scen.getMainUF();
  PlanDescrs.push_front({LT::LTMain, MainVF * MainUF, CurPlan});
  dumpExistingVPlan(CurPlan);

  // The order of insertion of remainders in the list is important. We insert
  // scalar remainder at front while others before the anchor. So the scalar
  // remainder is inserted in CFG first.
  auto AnchorIter = PlanDescrs.begin();
  for (auto Rem : make_range(Scen.rem_begin(), Scen.rem_end()))
    switch (Rem.Kind) {
    case LK::LKNone:
      break;
    case LK::LKScalar: {
      ScalarRemainderVPlanFab Fab;
      auto ScalarPlan = Fab.create(MainPlan, OrigLoop);
      // Update the NeedClone flag in scalar remainder. We use the original
      // loop first for peel and create a clone for remainder if that happen.
      ScalarPlan->setNeedCloneOrigLoop(ScalarUsed);
      CurPlan = Planner.addAuxiliaryVPlan(*ScalarPlan);
      PlanDescrs.push_front({LT::LTRemainder, 1, CurPlan});
      dumpNewVPlan(CurPlan);
      break;
    }
    case LK::LKVector:
      CurPlan = Planner.getVPlanForVF(Rem.VF);
      if (UsedPlans.count(CurPlan)) {
        auto Clone = cast<VPlanNonMasked>(CurPlan)->clone(
            VPAF, VPlanVector::UpdateDA::CloneDA);
        CurPlan = Planner.addAuxiliaryVPlan(*Clone);
        dumpNewVPlan(CurPlan);
      } else {
        dumpExistingVPlan(CurPlan);
      }
      PlanDescrs.insert(AnchorIter, {LT::LTRemainder, Rem.VF, CurPlan});
      UsedPlans.insert(CurPlan);
      break;
    case LK::LKMasked:
      CurPlan = Planner.getMaskedVPlanForVF(Rem.VF);
      if (UsedPlans.count(CurPlan)) {
        auto Clone = cast<VPlanMasked>(CurPlan)->clone(
            VPAF, VPlanVector::UpdateDA::CloneDA);
        CurPlan = Planner.addAuxiliaryVPlan(*Clone);
        dumpNewVPlan(CurPlan);
      } else {
        dumpExistingVPlan(CurPlan);
      }
      PlanDescrs.insert(AnchorIter, {LT::LTRemainder, Rem.VF, CurPlan});
      UsedPlans.insert(CurPlan);
    }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (PlanDescrs.size() && MergeNewPlansDumpControl.dumpPlain()) {
    dbgs() << "List of VPlans added for merging:\n";
    for (auto P : PlanDescrs)
      P.dump();
  }
#endif // !NDEBUG || LLVM_ENABLE_DUMP
}

void VPlanCFGMerger::createAdapterBB(PlanDescr &Descr,
                                     VPBasicBlock *InsertBefore,
                                     VPBasicBlock *Succ) {
  // The basic block with vplan adapter can be inserted only before
  // a merge block which has single predecessor. The Succ can be different,
  // e.g. when we insert the block for masked remainder but before that
  // we have inserted a scalar remainder. In this case, we need to "jump over"
  // scalar remainder after executing masked vector one. The reachability
  // of the scalar remainder in such cases should be ensured by linking the
  // merge block with other check-blocks.
  assert(isMergeBlock(InsertBefore) && "expected merge block");
  assert(isMergeBlock(Succ) && "expected merge block");
  assert(InsertBefore->getSinglePredecessor() && "expected one predecessor");

  // Create a new block before insertion point.
  auto *NewBB = new VPBasicBlock(VPlanUtils::createUniqueName("BB"), &Plan);
  VPBlockUtils::insertBlockBefore(NewBB, InsertBefore);
  NewBB->setTerminator(Succ);

  VPBuilder Builder;
  Builder.setInsertPoint(NewBB);
  auto *AdapterI  = Builder.create<VPlanAdapter>("vplan.adapter", *Descr.Plan);
  Plan.getVPlanDA()->markUniform(*AdapterI);
  Descr.FirstBB = NewBB;
  Descr.LastBB = NewBB;
}

void VPlanCFGMerger::createMergedCFG(SingleLoopVecScenario &Scen,
                                     std::list<CfgMergerPlanDescr> &Plans) {
  Plan.invalidateAnalyses({VPAnalysisID::SVA});

  MainVF = Scen.getMainVF();
  MainUF = Scen.getMainUF();
  emitSkeleton(Plans);

  VPLAN_DUMP(MergeSkeletonDumpControl, Plan);
}

void VPlanCFGMerger::createTCCheckAfter(PlanDescr &Descr,
                                        PlanDescr &PrevDescr) {

  assert(isa<VPlanNonMasked>(Descr.Plan) && "expected VPlanNonMasked");
  VPVectorTripCountCalculation *VectorUB = findVectorUB(*Descr.Plan);
  VPValue *PrevUB;

  // Create a new block after insertion point.
  auto *TestBB =
      new VPBasicBlock(VPlanUtils::createUniqueName("BB"), &Plan);
  VPBlockUtils::insertBlockAfter(TestBB, Descr.LastBB);

  // At this point we have non-masked VPlan (assert at the first function line).
  if (Descr.Plan != &Plan) {
    // For non-main VPlan we need to clone its upper bound and surrond it
    // with pushVF/popVF.
    VectorUB = cast<VPVectorTripCountCalculation>(VectorUB->clone());
    VectorUB->setOperand(0, OrigUB);
    insertVectorUBInst(VectorUB, TestBB, Descr.VF, false);
  }
  if (isa<VPlanScalar>(PrevDescr.Plan) || isa<VPlanMasked>(PrevDescr.Plan)) {
    // Scalar and masked mode loops have original upper bound
    PrevUB = OrigUB;
  } else {
    // Clone upper bound of the previous loop and update its operand.
    auto *VecUB = cast<VPVectorTripCountCalculation>(
        findVectorUB(*PrevDescr.Plan)->clone());
    VecUB->setOperand(0, OrigUB);
    insertVectorUBInst(VecUB, TestBB, PrevDescr.VF, PrevDescr.Plan == &Plan);
    PrevUB = VecUB;
  }

  // Generate a check for vector tc is not equal original tc and branch
  // according to the check.
  // Here we can use VectorUB directly as the new block is dominated by VPlan.
  VPBuilder Builder;
  Builder.setInsertPoint(TestBB);
  auto *RemTCCheck =
      Builder.createCmpInst(CmpInst::ICMP_EQ, PrevUB, VectorUB, "remtc.check");
  Plan.getVPlanDA()->markUniform(*RemTCCheck);

  TestBB->setTerminator(PrevDescr.PrevMerge, PrevDescr.MergeBefore, RemTCCheck);
  updateMergeBlockIncomings(Descr, PrevDescr.MergeBefore, TestBB,
                            false /* UseLiveIn */);
  updateMergeBlockIncomings(Descr, PrevDescr.PrevMerge, TestBB,
                            false /* UseLiveIn */);
}

void VPlanCFGMerger::insertVectorUBInst(VPVectorTripCountCalculation *VectorUB,
                                        VPBasicBlock *BB, unsigned VF,
                                        bool IsMain) {
  // Insert VectorTripCounCalculation. We surround it with VPPushVF/VPPopVF so
  // the VF is taken correctly. If we insert UB for main VPlan we don't need
  // push/pop. That is indicated by VF == 0.
  VPBuilder Builder;
  Builder.setInsertPoint(BB);
  if (!IsMain) {
    auto *PushVF =
        Builder.create<VPPushVF>("pushvf", Plan.getLLVMContext(), VF, 1);
    Plan.getVPlanDA()->markUniform(*PushVF);
  }
  BB->addInstruction(VectorUB);

  if (!IsMain) {
    auto *PopVF = Builder.createNaryOp(
        VPInstruction::PopVF, Type::getVoidTy(*Plan.getLLVMContext()), {});
    Plan.getVPlanDA()->markUniform(*PopVF);
  }
}

VPBasicBlock *VPlanCFGMerger::createTopTest(VPlan *VecPlan,
                                            VPBasicBlock *InsertBefore,
                                            VPBasicBlock *SuccEq,
                                            VPBasicBlock *SuccNe,
                                            unsigned VF) {

  // Create a new block before insertion point.
  auto *TestBB = new VPBasicBlock(VPlanUtils::createUniqueName("BB"), &Plan);
  VPBlockUtils::insertBlockBefore(TestBB, InsertBefore);
  VPBuilder Builder;
  Builder.setInsertPoint(TestBB);
  // We will generate
  //    %c = icmp cmp LHS, RHS
  //    br %c, label %SuccEq, label %SuccNe
  //
  // where:
  //  both Succ* are parameters and
  //     LHS = 0
  //     RHS = vector_ub
  //     cmp = eq
  //
  VPValue *Cmp;
  auto *VectorUB =
      cast<VPVectorTripCountCalculation>(findVectorUB(*VecPlan)->clone());
  VectorUB->setOperand(0, OrigUB);
  insertVectorUBInst(VectorUB, TestBB, VF, VecPlan == &Plan);
  auto *Zero =
      Plan.getVPConstant(ConstantInt::getNullValue(VectorUB->getType()));
  Cmp = Builder.createCmpInst(CmpInst::ICMP_EQ, Zero, VectorUB, "vec.tc.check");
  Plan.getVPlanDA()->markUniform(*Cmp);
  TestBB->setTerminator(SuccEq, SuccNe, Cmp);
  return TestBB;
}

void VPlanCFGMerger::createTCCheckBeforeMain(PlanDescr &MainDescr,
                                             PlanDescr *PRemDescr) {
  assert((MainDescr.Type == CfgMergerPlanDescr::LoopType::LTMain &&
          MainDescr.Plan == &Plan) &&
         "expected main vplan");
  if (!PRemDescr) {
    // The situation with only unmasked vplan (known TC evenly divisible by VF).
    // Nothing to generate.
    return;
  }

  VPBasicBlock *TopTest = createTopTest(
      MainDescr.Plan, MainDescr.FirstBB, MainDescr.PrevMerge,
      MainDescr.FirstBB, MainDescr.VF);

  updateMergeBlockIncomings(Plan, MainDescr.PrevMerge, TopTest,
                              true /* UseLiveIn */);
  if (isa<VPlanNonMasked>(PRemDescr->Plan)) {
    // If there is a pre-pre vplan (e.g. scalar remainder after vectorized one)
    // we generate the toptest for the previous vplan (vectorized remainder)
    // before the previous check.
    VPBasicBlock *TopTest2 =
        createTopTest(PRemDescr->Plan, TopTest, PRemDescr->PrevMerge,
                      TopTest, PRemDescr->VF);
    updateMergeBlockIncomings(Plan, PRemDescr->PrevMerge, TopTest2,
                              false /* UseLiveIn */);
  }
}

// Example CFG skeleton for scenario:
//     peel + non-masked main + non-masked remainder + scalar remainder
//       where RemVF < MainVF
// OrigUB is an original loop upper bound.
// MainUB == AlignFloor(OrigUB, mainVF)
// RemUB == AlignFloor(OrigUB, remVF)
//   where AlignFloor(X, Y) is (X & (Y - 1)) and Y is power of 2
//
//                         Entry
//                           |
//   ___false- (Base pointer vector element aligned ? )    : Check1
//  /                      true
// |                         |
// |                 ( PeelCount == 0 ? ) -true_______     : Check2
// |                       false                      \
// |                         |                         |
// |    ____true- ( (PeelCount + mainVF) > OrigUB ? )  |   : Check3
// |   /                   false                       |
// |  |                      |                         |
// |  |             +-------------------+              |
// |  |             |   Peel loop       |              |
// |  |             +-------------------+              |
// |  |                      |                         |
// |  |    ____true- ( RemUB == 0 ? )                  |   : Check4
// |  |   /                false                       |
// |  |  |                   |                         |
// |  |  |     __true- ( MainUB == 0 ? )               |   : Check5
// |  |  |    /             false     ________________/
// |  |  |   |               |      /
// |  |  |   |      +-----------------------+
// |  |  |   |      |   Merge before main   |
// |  |  |   |      +-----------------------+  MainVF
// |  |  |   |      |    Main vector loop   |  MainUB
// |  |  |   |      +-----------------------+
// |  |  |   |               |
// |  |  |   |     ( MainUB == RemUB ? ) -true______       : Check6
// |  |  |    \            false                    \
// |  |  |     \_________    |                       |
// |  |  |               \   |                       |
// |  |  |          +-----------------------+        |
// |  |  |          | Merge before vec rem  |        |
// |  |  |          +-----------------------+  RemVF |
// |  |  |          | Vector remainder loop |  RemUB |
// |  |  |          +-----------------------+        |
// |  |  |                   |       _______________/
// |  |  |                   |      /
// |  |  |         ( RemUB == OrigUB ? )  -true____        : Check7
// |  |   \                false                   \
//  \  \   \____________     |                      \
//   \  \______________  \   |                       \
//    \_______________ \  \  |                        |
//                    \ \  \ |                        |
//                  +-----------------------+         |
//                  | Merge before scal rem |         |
//                  +-----------------------+         |
//                  | Scalar remainder loop |  OrigUB |
//                  +-----------------------+         |
//                           |       ________________/
//                           |      /
//                  +-----------------------+
//                  |     Final Merge       |
//                  +-----------------------+
//                           |
//                         Exit
void VPlanCFGMerger::emitSkeleton(std::list<PlanDescr> &Plans) {
  using LT = CfgMergerPlanDescr::LoopType;

  VPBasicBlock *FinalMerge, *LastMerge;
  auto LastVPBB = &*find_if(
      Plan, [](const VPBasicBlock &BB) { return BB.getNumSuccessors() == 0; });

  // Find original upper bound of the main loop. We need it to generate trip
  // count checks.
  updateOrigUB();

  FinalMerge = LastMerge = createMergeBlock(LastVPBB);
  FinalMerge->setName("final.merge");

  // TODO: In some cases we need scalar loop not as remainder but for
  // safety/profitability reasons, e.g. due to data dependency checks. That is
  // not implemented/accounted yet.

  // Note: The VPlans in the list are placed so we go from the end of CFG to the
  // beginning. I.e. first we process remainder(s), then main loop, and then
  // peel. So for the diagram above we will process in the following order:
  //    ScalarRemainder->VectorRemainder->MainLoop->Peel
  // The selected order of processing is more convenient because going this way
  // the merge blocks that are needed as targets for different checks are
  // already created, we don't create back-edges.
  for (auto Iter = Plans.begin(); Iter != Plans.end(); Iter++) {
    auto &P = *Iter;
    bool IsFirst = Iter == Plans.begin();

    VPBasicBlock *Succ;
    if (P.Type != LT::LTMain) {
      // Create basic block with VPlanAdaptor.
      // We need to link it to FinalMerge in case it's the first one or
      // is masked remainder.
      Succ = (IsFirst || P.isMaskedRemainder()) ? FinalMerge : LastMerge;
      createAdapterBB(P, LastMerge, Succ);
    } else {
      // Special case for main loop. We don't create basic block with adaptor,
      // just setting First/Last basic blocks.
      P.FirstBB = findFirstNonEmptyBB();
      P.LastBB = LastVPBB;
      Succ = LastMerge;
    }
    P.PrevMerge = Succ;

    if (!IsFirst && !P.isMaskedOrScalarRemainder()) {
      // Create the needed trip count checks after VPlan when needed.
      assert(Succ != FinalMerge && "not expected check");
      // See Check6, Check7 on the diagram above.
      createTCCheckAfter(P, *std::prev(Iter, 1));
    } else {
      updateMergeBlockIncomings(P, Succ, P.LastBB, false /* UseLiveIn */);
    }

    if (std::next(Iter, 1) != Plans.end()) {
      // If we need to insert more VPlans we create a merge block before the
      // current one.
      LastMerge = createMergeBlockBefore(P.FirstBB);
      P.MergeBefore = LastMerge;
    } else { //if (P.Type != LT::LTPeel) {
      // If that is the last non-peel VPlan it should be main VPlan and
      // we need to generate the needed trip count check before it.
      assert(P.Type == LT::LTMain && "expected main loop");
      PlanDescr *PrevD = IsFirst ? nullptr : &*(std::prev(Iter, 1));
      // Check4, Check5 on the diagram above
      createTCCheckBeforeMain(P, PrevD);
    }
  }
  // Hoist the original upper bound (if it's a VPInstruction) to the first
  // non-empty block. We use it in several places of CFG and need it to be in
  // the most dominating block.
  moveOrigUBToBegin();
}

void VPlanCFGMerger::updateOrigUB() {
  // Get original upper bound using VPlan main loop.
  if (isa<VPlanNonMasked>(Plan))
    OrigUB = findVectorUB(Plan)->getOperand(0);
  else {
    // Get the UB from the latch condition (its invariant operand).
    VPLoop *L = *cast<VPlanMasked>(Plan).getVPLoopInfo()->begin();
    VPBasicBlock *Latch = L->getLoopLatch();
    auto BI = Latch->getTerminator();
    assert((BI && BI->isConditional()) &&
           "expected conditional branch instruction");
    VPCmpInst *Cond = L->getLatchComparison();
    assert(Cond && "expected comparison instruction");
    auto Op = Cond->getOperand(0);
    if (isa<VPInstruction>(Op) && L->contains(cast<VPInstruction>(Op)))
      OrigUB = Cond->getOperand(1);
    else
      OrigUB = Cond->getOperand(0);
    // TODO: remove this when masked vplan creation is fixed to use original UB
    // in the latch.
    if (auto OrigUBInst = dyn_cast<VPVectorTripCountCalculation>(OrigUB))
      OrigUB = OrigUBInst->getOperand(0);
  }
}

void VPlanCFGMerger::moveOrigUBToBegin() {
  auto UBInst = dyn_cast<VPInstruction>(OrigUB);
  if (!UBInst)
    return;
  // Gather all instructions that original UB depends on, putting them in the
  // order of direct dependencies. The duplicated items are intentionally added
  // to the list. We move them in the reverse order of insertion, inserting
  // before a fixed point. So the latest of the duplicates will be
  // moved/inserted first, and other instructions will be inserted before it.
  // E.g. suppose we have
  //   %1 = %ext_def1 + %ext_def2
  //   %2 = %1 + %ext_def3
  //   %3 = %2 + %ext_def3
  //   %4 = %3 + %1
  //   %UB = %2 + %4
  //
  // Then, in the end of gathering our list will contain
  //     %UB, %2, %4, %3, %1, %2, %1
  // We will insert them in the reverse order, skipping already inserted, and
  // all before same point. So we will have
  //     %1
  //     %2
  //     %3
  //     %4
  //     %UB
  std::list<VPInstruction *> WorkList;
  SmallVector<VPInstruction *, 2> ToMove;
  WorkList.push_back(UBInst);
  while (!WorkList.empty()) {
    VPInstruction *I = WorkList.front();
    WorkList.pop_front();
    ToMove.push_back(I);
    for (auto *Op : I->operands())
      if (auto *OpI = dyn_cast<VPInstruction>(Op))
        WorkList.push_back(OpI);
  }
  // Move the gathered VPInstructions to the new location.
  VPBasicBlock *FirstBB = findFirstNonEmptyBB();
  auto MovePos = FirstBB->begin();
  while (MovePos != FirstBB->end() && isa<VPPHINode>(*MovePos))
    ++MovePos;
  SmallSet<VPInstruction*, 4> Moved;
  auto REnd = ToMove.rend();
  for (auto I = ToMove.rbegin(); I != REnd; I++) {
    if (Moved.count(*I))
      continue;
    (*I)->moveBefore(*FirstBB, MovePos);
    Moved.insert(*I);
  }
}
