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
#include "IntelVPlanExternals.h"
#include "IntelVPlanScalarEvolution.h"
#include "VPlanHIR/IntelVPlanScalarEvolutionHIR.h"

#define DEBUG_TYPE "VPlanCFGMerger"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool, true>
    EmitPushPopVFOpt("vplan-enable-pushvf", cl::location(EmitPushPopVF),
                     cl::Hidden,
                     cl::desc("Emit pushvf and popvf VPInstrucitons."));

static cl::opt<bool> NeedPeelForSafety(
    "vplan-peel-for-safety", cl::Hidden,
    cl::desc("flag to emit peel for safety (e.g. in search loops)"));

namespace llvm {
namespace vpo {
bool EmitPushPopVF = false;
} // namespace vpo
} // namespace llvm

static LoopVPlanDumpControl MergeNewPlansDumpControl("create-in-merge",
                                                     "creation during merge");
static LoopVPlanDumpControl
    MergeSkeletonDumpControl("merge-skeleton", "merge skeleton creation");

static LoopVPlanDumpControl
    MergePass2DumpControl("merge-pass2", "final merge pass");

static LoopVPlanDumpControl CfgMergeDumpControl("cfg-merge",
                                                "CFG merge before CG");

// Forward declaration.
static bool isMergeBlock(VPBasicBlock *BB);

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

  if (UseLiveIn) {
    updateMergeBlockIncomings(*Descr.Plan, MergeBlock, SplitBlock, UseLiveIn);
    return;
  }

  VPBasicBlock *Src = SplitBlock;
  while (Src && Src != Descr.LastBB && !isMergeBlock(Src))
    Src = Src->getSinglePredecessor();

  DenseMap<unsigned, VPValue *> MergeVals;
  VPValue *AdapterInst = nullptr;
  if (Src == Descr.LastBB) {
    if (Descr.Type != LT::LTMain) {
      // Go through phi nodes in the MergeBlock and set their incoming values
      // from SplitBlock to VPlan adapter.
      auto Adapter = llvm::find_if(*Descr.FirstBB, [](const VPInstruction &I) {
        return isa<VPlanAdapter>(I);
      });
      assert(Adapter != Descr.FirstBB->end() && "expected non-null adapter");
      AdapterInst = &*Adapter;
    } else {
      for (auto &PN : MergeBlock->getVPPhis())
        MergeVals[PN.getMergeId()] = Plan.getLiveOutValue(PN.getMergeId());
    }
  } else {
    // It's a merge block after VPlan. Make its phis feeding the MergeBlock
    for (auto &PN : Src->getVPPhis())
      MergeVals[PN.getMergeId()] = &PN;
  }

  for (auto &Phi : MergeBlock->getVPPhis()) {
    unsigned MergeId = Phi.getMergeId();
    if (MergeId == VPExternalUse::UndefMergeId)
      llvm_unreachable("Unexpected instruction in a merge block");

    VPValue *InVal = AdapterInst ? AdapterInst : MergeVals[MergeId];
    if (Phi.getBlockIndex(SplitBlock) == -1) {
      Phi.addIncoming(InVal, SplitBlock);
      continue;
    }
    assert(Phi.getIncomingValue(SplitBlock) == InVal &&
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
  VPBasicBlock *BB = &Plan.getEntryBlock();
  for (; BB && BB->terminator() == BB->begin(); BB = BB->getSingleSuccessor())
    ;
  assert(BB && "Non-empty VPlan expected");
  return BB;
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

// Dummy no-op method to accept HLLoop and enable templatized
// ScalarPeelOrRemainderVPlanFabBase::runImpl.
static Use *getExitBBUse(loopopt::HLLoop *Loop) { return nullptr; }

template <typename VPlanType, typename VPInstructionType>
class ScalarPeelOrRemainderVPlanFabBase {
  virtual void addRemainderLiveIn(ScalarInOutDescr *Descr,
                                  VPInstructionType *I) {}
  virtual void addRemainderLiveIn(ScalarInOutDescrHIR *Descr,
                                  VPInstructionType *I) {}
  // Will be no-op for HIR specialized fabs.
  virtual void updateLoopExit(VPInstructionType *I, VPValue *Blk, Use *Val) {}
  virtual VPValue *generateOrigLiveOut(VPBuilder &Builder,
                                       ScalarInOutDescr *Descr,
                                       VPInstructionType *I) {
    return nullptr;
  }
  virtual VPValue *generateOrigLiveOut(VPBuilder &Builder,
                                       ScalarInOutDescrHIR *Descr,
                                       VPInstructionType *I) {
    return nullptr;
  }
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
  template <class LoopTy>
  VPlanType *runImpl(VPlan &MainPlan, LoopTy *OrigLoop) {
    NewPlan =
        new VPlanType(MainPlan.getExternals(), MainPlan.getUnlinkedVPInsts());
    setPlanName(MainPlan);

    // Copy PrintingEnabled flag
    NewPlan->setPrintingEnabled(MainPlan.isPrintingEnabled());

    // Create live-in and live-out lists.
    auto *ScalarInOuts = NewPlan->getExternals().getScalarLoopInOuts(OrigLoop);

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
      LiveOuts[MergeId] =
          generateOrigLiveOut(Builder, ScalarDescr, ScalarLoopInstr);
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

  virtual VPValue *generateOrigLiveOut(VPBuilder &Builder,
                                       ScalarInOutDescr *Descr,
                                       VPInstructionType *I) override {
    return Builder.create<VPPeelOrigLiveOut>(
        "orig.liveout", Descr->getValueType(), I, Descr->getLiveOut(),
        Descr->getId());
  }

public:
  VPlanType *create(VPlan &MainPlan, Loop *OrigLoop) {
    return runImpl(MainPlan, OrigLoop);
  }
};

// Peel specialization for HIR
template <bool>
class ScalarPeelOrRemainderVPlanFabHIR
    : public ScalarPeelOrRemainderVPlanFabBase<VPlanScalarPeel,
                                               VPScalarPeelHIR> {
  using VPlanType = VPlanScalarPeel;
  using VPInstructionType = VPScalarPeelHIR;

  virtual const char *getFirstBlockName() override { return "PeelBlk"; }
  virtual void setPlanName(VPlan &MainPlan) override {
    NewPlan->setName(MainPlan.getName() + ".ScalarPeel");
  }

  virtual VPValue *generateOrigLiveOut(VPBuilder &Builder,
                                       ScalarInOutDescrHIR *Descr,
                                       VPInstructionType *I) override {
    return Builder.create<VPPeelOrigLiveOutHIR>(
        "orig.liveout", Descr->getValueType(), I, Descr->getHIRRef(),
        Descr->getId());
  }

public:
  VPlanType *create(VPlan &MainPlan, loopopt::HLLoop *OrigLoop) {
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

  virtual VPValue *generateOrigLiveOut(VPBuilder &Builder,
                                       ScalarInOutDescr *Descr,
                                       VPInstructionType *I) override {
    return Builder.create<VPRemainderOrigLiveOut>(
        "orig.liveout", Descr->getValueType(), I, Descr->getLiveOut(),
        Descr->getId());
  }

public:
  VPlanType *create(VPlan &MainPlan, Loop *OrigLoop) {
    return runImpl(MainPlan, OrigLoop);
  }
};

// Remainder specialization for HIR
template <>
class ScalarPeelOrRemainderVPlanFabHIR<false>
    : public ScalarPeelOrRemainderVPlanFabBase<VPlanScalarRemainder,
                                               VPScalarRemainderHIR> {
  using VPlanType = VPlanScalarRemainder;
  using VPInstructionType = VPScalarRemainderHIR;

  virtual const char *getFirstBlockName() override { return "RemBlk"; }
  virtual void setPlanName(VPlan &MainPlan) override {
    NewPlan->setName(MainPlan.getName() + ".ScalarRemainder");
  }

  // Overriden method to create temp initialization map for the scalar remainder
  // loop.
  virtual void addRemainderLiveIn(ScalarInOutDescrHIR *Descr,
                                  VPInstructionType *I) override {
    loopopt::HLLoop *OrigLp = I->getLoop();
    auto *HIRRef = Descr->getHIRRef();

    // If the temp is not main loop IV and not live-in then ignore.
    // TODO: Consider uplifting this property to ScalarInOutDescr.
    if (!Descr->isMainLoopIV() && !OrigLp->isLiveIn(HIRRef->getSymbase()))
      return;

    int Id = Descr->getId();
    auto *LiveIn = const_cast<VPLiveInValue *>(NewPlan->getLiveInValue(Id));

    // For main loop IV we don't have a temp to initialize the live-in. Create a
    // new temp to track lower-bound of loop.
    if (Descr->isMainLoopIV()) {
      loopopt::RegDDRef *LBTmp =
          OrigLp->getHLNodeUtils().createTemp(OrigLp->getIVType(), "lb.tmp");
      I->setLowerBoundTemp(LiveIn, LBTmp);
      return;
    }

    // For other descriptors we simply update temp-init map.
    I->addLiveIn(LiveIn, const_cast<loopopt::DDRef *>(HIRRef));
  }

  virtual VPValue *generateOrigLiveOut(VPBuilder &Builder,
                                       ScalarInOutDescrHIR *Descr,
                                       VPInstructionType *I) override {
    return Builder.create<VPRemainderOrigLiveOutHIR>(
        "orig.liveout", Descr->getValueType(), I, Descr->getHIRRef(),
        Descr->getId());
  }

public:
  VPlanType *create(VPlan &MainPlan, loopopt::HLLoop *OrigLoop) {
    return runImpl(MainPlan, OrigLoop);
  }
};

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

void VPlanCFGMerger::insertPushPopVF(VPlan &P, unsigned VF, unsigned UF) {
  VPBasicBlock *FirstBB = &P.getEntryBlock();
  VPBuilder Builder;
  Builder.setInsertPoint(FirstBB, FirstBB->begin());
  VPInstruction *PushVF =
      Builder.create<VPPushVF>("pushvf", P.getLLVMContext(), VF, UF);

  auto LastBB = &*P.getExitBlock();
  Builder.setInsertPoint(LastBB);
  VPValue *PopVF = Builder.createNaryOp(
      VPInstruction::PopVF, Type::getVoidTy(*P.getLLVMContext()), {});

  if (auto *DA = dyn_cast<VPlanDivergenceAnalysis>(P.getVPlanDA())) {
    DA->markUniform(*PopVF);
    DA->markUniform(*PushVF);
  }
}

template <class LoopTy>
void VPlanCFGMerger::createPlans(LoopVectorizationPlanner &Planner,
                                 const SingleLoopVecScenario &Scen,
                                 std::list<CfgMergerPlanDescr> &PlanDescrs,
                                 LoopTy *OrigLoop, VPlan &MainPlan,
                                 VPAnalysesFactoryBase &VPAF) {

  using ScalarPeelVPlanFab =
      typename std::conditional<std::is_same<LoopTy, loopopt::HLLoop>::value,
                                ScalarPeelOrRemainderVPlanFabHIR<true>,
                                ScalarPeelOrRemainderVPlanFab<true>>::type;
  using ScalarRemainderVPlanFab =
      typename std::conditional<std::is_same<LoopTy, loopopt::HLLoop>::value,
                                ScalarPeelOrRemainderVPlanFabHIR<false>,
                                ScalarPeelOrRemainderVPlanFab<false>>::type;

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
  assert(NewMainPlan && "Unexpected null main plan.");

  VPlan *CurPlan = NewMainPlan;
  UsedPlans.insert(CurPlan);

  switch (Scen.getPeelKind()) {
  case LK::LKNone:
    break;
  case LK::LKScalar: {
    bool ScalarUsed = llvm::any_of(Scen.remainders(), [](auto &PlanDescr) {
      return PlanDescr.Kind == LK::LKScalar;
    });
    ScalarPeelVPlanFab Fab;
    auto ScalarPlan = Fab.create(MainPlan, OrigLoop);
    // Update the NeedClone flag in scalar peel. We use the original
    // loop for remainder and create a clone for peel if that's needed.
    ScalarPlan->setNeedCloneOrigLoop(ScalarUsed);
    CurPlan = Planner.addAuxiliaryVPlan(*ScalarPlan);
    PlanDescrs.push_front({LT::LTPeel, 1, CurPlan});
    dumpNewVPlan(CurPlan);
    break;
  }
  case LK::LKMasked:
    CurPlan = Planner.getMaskedVPlanForVF(Scen.getPeelVF());
    assert(CurPlan && "Unexpected null current plan.");
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
  for (auto Rem : Scen.remainders())
    switch (Rem.Kind) {
    case LK::LKNone:
      break;
    case LK::LKScalar: {
      ScalarRemainderVPlanFab Fab;
      auto ScalarPlan = Fab.create(MainPlan, OrigLoop);
      CurPlan = Planner.addAuxiliaryVPlan(*ScalarPlan);
      PlanDescrs.push_front({LT::LTRemainder, 1, CurPlan});
      dumpNewVPlan(CurPlan);
      break;
    }
    case LK::LKVector:
      CurPlan = Planner.getVPlanForVF(Rem.VF);
      assert(CurPlan && "Unexpected null current plan.");
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
      assert(CurPlan && "Unexpected null current plan.");
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

template void VPlanCFGMerger::createPlans<Loop>(LoopVectorizationPlanner &,
                                                const SingleLoopVecScenario &,
                                                std::list<CfgMergerPlanDescr> &,
                                                Loop *, VPlan &,
                                                VPAnalysesFactoryBase &);
template void VPlanCFGMerger::createPlans<loopopt::HLLoop>(
    LoopVectorizationPlanner &, const SingleLoopVecScenario &,
    std::list<CfgMergerPlanDescr> &, loopopt::HLLoop *, VPlan &,
    VPAnalysesFactoryBase &);

void VPlanCFGMerger::createAdapterBB(PlanDescr &Descr,
                                     VPBasicBlock *InsertBefore,
                                     VPBasicBlock *Succ) {
  using LT = CfgMergerPlanDescr::LoopType;
  // The basic block with vplan adapter can be inserted only before
  // a merge block which has single predecessor or has no predecessors (when we
  // insert peel). The Succ can be different, e.g. when we insert the block for
  // masked remainder but before that we have inserted a scalar remainder. In
  // this case, we need to "jump over" scalar remainder after executing masked
  // vector one. The reachability of the scalar remainder in such cases should
  // be ensured by linking the merge block with other check-blocks.
  assert(isMergeBlock(InsertBefore) && "expected merge block");
  assert(isMergeBlock(Succ) && "expected merge block");
  assert((InsertBefore->getNumPredecessors() == 0 ||
          InsertBefore->getSinglePredecessor()) &&
         "expected none or one predecessor");

  // Create a new block before insertion point.
  auto *NewBB = new VPBasicBlock(VPlanUtils::createUniqueName("BB"), &Plan);
  VPBlockUtils::insertBlockBefore(NewBB, InsertBefore);
  NewBB->setTerminator(Succ);

  VPBuilder Builder;
  Builder.setInsertPoint(NewBB);
  VPInstruction *AdapterI;
  if (Descr.Type == LT::LTPeel)
    AdapterI =
        Builder.create<VPlanPeelAdapter>("vplan.peel.adapter", *Descr.Plan);
  else
    AdapterI = Builder.create<VPlanAdapter>("vplan.adapter", *Descr.Plan);
  Plan.getVPlanDA()->markUniform(*AdapterI);

  Descr.FirstBB = NewBB;
  Descr.LastBB = NewBB;
}

template <class LoopTy>
void VPlanCFGMerger::createMergedCFG(SingleLoopVecScenario &Scen,
                                     std::list<CfgMergerPlanDescr> &Plans,
                                     LoopTy *OrigLoop) {
  Plan.invalidateAnalyses({VPAnalysisID::SVA});

  MainVF = Scen.getMainVF();
  MainUF = Scen.getMainUF();
  emitSkeleton(Plans, OrigLoop);
  mergeVPlans(Plans);
  VPLAN_DUMP(CfgMergeDumpControl, Plan);
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
    // For non-main VPlan we need to clone its upper bound and surround it
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
  // according to the check. This is not needed if we are dealing with a
  // simple main vector, remainder scalar const trip count scenario.
  // Here we can use VectorUB directly as the new block is dominated by VPlan.
  VPBuilder Builder;
  Builder.setInsertPoint(TestBB);

  updateMergeBlockIncomings(Descr, PrevDescr.MergeBefore, TestBB,
                            false /* UseLiveIn */);
  if (!IsSimpleConstTCScenario) {
    auto *RemTCCheck = Builder.createCmpInst(CmpInst::ICMP_EQ, PrevUB, VectorUB,
                                             "remtc.check");
    Plan.getVPlanDA()->markUniform(*RemTCCheck);
    TestBB->setTerminator(PrevDescr.PrevMerge, PrevDescr.MergeBefore,
                          RemTCCheck);
    updateMergeBlockIncomings(Descr, PrevDescr.PrevMerge, TestBB,
                              false /* UseLiveIn */);
  } else
    TestBB->setTerminator(PrevDescr.MergeBefore);
}

VPCmpInst *VPlanCFGMerger::createPeelCntVFCheck(VPValue *UB, VPBuilder &Builder,
                                                CmpInst::Predicate Pred,
                                                unsigned VF) {
  // Create the check for VF+PeelCount is greater then UB.
  VPValue *PeelCnt = Builder.createIntCast(PeelCount, UB->getType());
  if (PeelCnt != PeelCount)
    Plan.getVPlanDA()->markUniform(*PeelCnt);
  VPValue *VFUF = Plan.getVPConstant(ConstantInt::get(UB->getType(), VF));
  PeelCnt = Builder.createAdd(PeelCnt, VFUF);
  Plan.getVPlanDA()->markUniform(*PeelCnt);
  VPCmpInst *Cmp = Builder.createCmpInst(CmpInst::ICMP_UGT, PeelCnt, UB,
                                         "peel.vec.tc.check");
  Plan.getVPlanDA()->markUniform(*Cmp);
  return Cmp;
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
                                            VPBasicBlock *SuccNe, VPlan *Peel,
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
  //  for the case with peel:
  //     LHS = peel_cnt + VF*UF
  //     RHS = orig_ub
  //     cmp = ugt
  //  for the case w/o peel:
  //     LHS = 0
  //     RHS = vector_ub
  //     cmp = eq
  //
  VPValue *Cmp;
  if (Peel) {
    Cmp = createPeelCntVFCheck(OrigUB, Builder, CmpInst::ICMP_UGT, VF);
  } else {
    auto *VectorUB =
        cast<VPVectorTripCountCalculation>(findVectorUB(*VecPlan)->clone());
    VectorUB->setOperand(0, OrigUB);
    insertVectorUBInst(VectorUB, TestBB, VF, VecPlan == &Plan);
    auto *Zero =
        Plan.getVPConstant(ConstantInt::getNullValue(VectorUB->getType()));
    Cmp =
        Builder.createCmpInst(CmpInst::ICMP_EQ, Zero, VectorUB, "vec.tc.check");
    Plan.getVPlanDA()->markUniform(*Cmp);
  }
  TestBB->setTerminator(SuccEq, SuccNe, Cmp);
  return TestBB;
}

void VPlanCFGMerger::createTCCheckBeforeMain(PlanDescr *Peel,
                                             PlanDescr &MainDescr,
                                             PlanDescr *PRemDescr,
                                             PlanDescr *PPrevDescr) {
  assert((MainDescr.Type == CfgMergerPlanDescr::LoopType::LTMain &&
          MainDescr.Plan == &Plan) &&
         "expected main vplan");
  if (Peel)
    assert((isa<VPlanMasked>(Peel->Plan) || isa<VPlanScalar>(Peel->Plan)) &&
           "unexpected peel VPlan");

  if (isa<VPlanMasked>(MainDescr.Plan)) {
    // For masked mode loop we don't need any checks
    assert(!Peel && "unsupported peel for masked mode loop");
    return;
  }

  if (!PRemDescr) {
    // The situation with only unmasked vplan (known TC evenly divisible by VF).
    // Nothing to generate.
    return;
  }

  // We do not need the top test for simple main vector, remainder scalar
  // scenario of constant trip count loops.
  if (IsSimpleConstTCScenario)
    return;

  VPBasicBlock *TopTest = createTopTest(
      MainDescr.Plan, MainDescr.FirstBB, MainDescr.PrevMerge,
      MainDescr.FirstBB, Peel ? Peel->Plan : nullptr, MainDescr.VF);
  if (Peel)
    updateMergeBlockIncomings(*Peel, MainDescr.PrevMerge, TopTest,
                              false /* UseLiveIn */);
  else
    updateMergeBlockIncomings(MainDescr, MainDescr.PrevMerge, TopTest,
                              true /* UseLiveIn */);
  if (isa<VPlanNonMasked>(PRemDescr->Plan)) {
    // If there is a pre-pre vplan (e.g. scalar remainder after vectorized one)
    // we generate the toptest for the previous vplan (vectorized remainder)
    // before the previous check.
    VPBasicBlock *MergeBB =
        PPrevDescr ? PPrevDescr->MergeBefore : PRemDescr->PrevMerge;
    VPBasicBlock *TopTest2 =
        createTopTest(PRemDescr->Plan, TopTest, MergeBB, TopTest,
                      Peel ? Peel->Plan : nullptr, PRemDescr->VF);
    if (Peel)
      updateMergeBlockIncomings(*Peel, MergeBB, TopTest2,
                                false /* UseLiveIn */);
    else
      updateMergeBlockIncomings(MainDescr, MergeBB, TopTest2,
                                true /* UseLiveIn */);
  }
}

template <class LoopTy>
VPValue *
VPlanCFGMerger::emitDynamicPeelCount(VPlanDynamicPeeling &DP, VPValue *BasePtr,
                                     VPBuilder &Builder, LoopTy *OrigLoop) {
  // We compute the peel-count using the formula below.
  // Quotient = BasePtr / DP.RequiredAlignment;
  // Divisor = DP.TargetAlignment / DP.RequiredAlignment;
  // PeelCount = (Quotient * Multiplier) % Divisor;
  Type *Ty = Type::getIntNTy(*Plan.getLLVMContext(),
                             Plan.getDataLayout()->getPointerSizeInBits());

  VPConstant *ReqAlignment =
      Plan.getVPConstant(ConstantInt::get(Ty, DP.requiredAlignment().value()));
  VPConstant *Multiplier =
      Plan.getVPConstant(ConstantInt::get(Ty, DP.multiplier()));
  unsigned DivisorVal =
      DP.targetAlignment().value() / DP.requiredAlignment().value();
  VPConstant *Divisor = Plan.getVPConstant(ConstantInt::get(Ty, DivisorVal));

  if (!BasePtr)
    BasePtr = emitPeelBasePtr(DP, Builder, OrigLoop);

  VPInstruction *IntBase =
      Builder.createNaryOp(Instruction::PtrToInt, Ty, {BasePtr});
  IntBase->setName("baseptr.int");
  Plan.getVPlanDA()->markUniform(*IntBase);

  // Quotient = InvariantBase / RequiredAlignment;
  VPInstruction *Quotient =
      Builder.createNaryOp(Instruction::UDiv, Ty, {IntBase, ReqAlignment});
  Quotient->setName("quotient");
  Plan.getVPlanDA()->markUniform(*Quotient);

  // PeelCount = (Quotient * Multiplier) % Divisor;
  VPInstruction *QuotientTimesMultiplier =
      Builder.createNaryOp(Instruction::Mul, Ty, {Quotient, Multiplier});
  QuotientTimesMultiplier->setName("qmultiplier");
  Plan.getVPlanDA()->markUniform(*QuotientTimesMultiplier);

  VPInstruction *PeelCnt =
      Builder.createNaryOp(Instruction::URem, Ty,
                           {QuotientTimesMultiplier, Divisor});
  VPValue *Ret = Builder.createIntCast(PeelCnt, OrigUB->getType());
  Ret->setName("peel.count");
  Plan.getVPlanDA()->markUniform(*Ret);
  return Ret;
}

template <>
VPInvSCEVWrapper *
VPlanCFGMerger::emitPeelBasePtr<loopopt::HLLoop>(VPlanDynamicPeeling &Peeling,
                                                 VPBuilder &Builder,
                                                 loopopt::HLLoop *OrigLoop) {
  (void) OrigLoop;
  auto *BasePtr = Peeling.invariantBase();
  auto *AddRecHIR = VPlanScalarEvolutionHIR::toVPlanAddRecHIR(BasePtr);
  auto *Ty = AddRecHIR->Base->getDestType();
  auto *Ret = Builder.create<VPInvSCEVWrapper>("peel.base.ptr", BasePtr, Ty,
                                               false /* IsSCEV */);
  Plan.getVPlanDA()->markUniform(*Ret);
  return Ret;
}

template <>
VPInvSCEVWrapper *
VPlanCFGMerger::emitPeelBasePtr<Loop>(VPlanDynamicPeeling &Peeling,
                                      VPBuilder &Builder, Loop *OrigLoop) {
  (void) OrigLoop;
  auto *BasePtr = Peeling.invariantBase();
  auto *Ty = VPlanScalarEvolutionLLVM::toSCEV(BasePtr)->getType();
  auto Ret = Builder.create<VPInvSCEVWrapper>("peel.base.ptr", BasePtr, Ty);
  Plan.getVPlanDA()->markUniform(*Ret);
  return Ret;
}

template <class LoopTy>
void VPlanCFGMerger::createPeelPtrCheck(VPlanDynamicPeeling &Peeling,
                                        VPBasicBlock *InsertBefore,
                                        VPBasicBlock *NonZeroMerge, VPlan &P,
                                        VPValue *&PeelBasePtr,
                                        LoopTy *OrigLoop) {
  // See full comment in the *.h file.
  // The following sequence is generated to check the lower bits of pointer are
  // not zero:
  //
  //  %ptr = Peeling->getMemRef()->getPointerOperand();
  //  %ptr2int = ptrtoint i64 %ptr
  //  %and = and %ptr2int, (Peeling->getTargetAlignment().value()-1)
  //  %cmp = icmp eq %and, 0
  //  br %cmp, %InsertBefore, %NonZeroMerge
  //
  VPBasicBlock *TestBB = new VPBasicBlock(
      VPlanUtils::createUniqueName("peel.checkl"), &Plan);
  VPBlockUtils::insertBlockBefore(TestBB, InsertBefore);
  VPBuilder Builder;
  Builder.setInsertPoint(TestBB);
  // Create ptrtoint and "and" with low bits mask.
  PeelBasePtr = emitPeelBasePtr(Peeling, Builder, OrigLoop);
  Type *VTy = Type::getIntNTy(*Plan.getLLVMContext(),
                              Plan.getDataLayout()->getPointerSizeInBits());
  VPInstruction *PtrToInt =
      Builder.createNaryOp(Instruction::PtrToInt, VTy, {PeelBasePtr});
  Plan.getVPlanDA()->markUniform(*PtrToInt);
  // Create low bits mask and "and"
  auto *LowBitMask = Plan.getVPConstant(
      ConstantInt::get(VTy, Peeling.requiredAlignment().value() - 1));
  auto BitAnd = Builder.createAnd(PtrToInt, LowBitMask, "peel.lowbit.and");
  Plan.getVPlanDA()->markUniform(*BitAnd);
  // Compare with 0.
  VPConstant *Zero = Plan.getVPConstant(ConstantInt::get(VTy, 0));
  VPCmpInst *Cmp = Builder.createCmpInst(CmpInst::ICMP_EQ, Zero, BitAnd,
                                         "peel.lowbitzero.check");
  Plan.getVPlanDA()->markUniform(*Cmp);
  // set successors
  TestBB->setTerminator(InsertBefore, NonZeroMerge, Cmp);
  updateMergeBlockIncomings(Plan, NonZeroMerge, TestBB, true /* UseLiveIn */);
}

template <class LoopTy>
void VPlanCFGMerger::insertPeelCntAndChecks(PlanDescr &P,
                                            VPBasicBlock *FinalRemainderMerge,
                                            VPBasicBlock *RemainderMerge,
                                            LoopTy *OrigLoop) {

  assert(P.Type == CfgMergerPlanDescr::LoopType::LTPeel &&
         "expected peel loop");
  // Emit peel count instruction.
  VPBasicBlock *TestBB =
      new VPBasicBlock(VPlanUtils::createUniqueName("peel.checkz"), &Plan);
  VPBlockUtils::insertBlockBefore(TestBB, P.FirstBB);
  VPBuilder Builder;
  Builder.setInsertPoint(TestBB);
  VPlanPeelingVariant *PeelVariant = Plan.getPreferredPeeling(MainVF);

  // Create the needed checks
  auto StaticPeel = dyn_cast<VPlanStaticPeeling>(PeelVariant);
  if (StaticPeel) {
    // No check for static peel count
    assert(StaticPeel->peelCount() && "unexpected zero peel count");
    PeelCount = Plan.getVPConstant(
        ConstantInt::get(OrigUB->getType(), StaticPeel->peelCount()));
    TestBB->setTerminator(P.FirstBB);
  } else {
    assert(RemainderMerge && "expected remainder");
    auto *Peeling = cast<VPlanDynamicPeeling>(PeelVariant);
    VPValue *PeelBasePtr = nullptr;
    VPLoadStoreInst *PeelMemref = cast<VPLoadStoreInst>(Peeling->memref());
    if (PeelMemref->getAlignment() < Peeling->requiredAlignment()) {
      // If alignment of peeled memref is unknown create the check for low bits
      // of the peeled pointer
      // If we need peel for safety then we can't execute vectorized code and
      // should goto scalar remainder thus select either FinalRemainderMerge
      // of the marge block before main loop.
      VPBasicBlock *UnalignedMerge =
          needPeelForSafety() ? FinalRemainderMerge : RemainderMerge;
      createPeelPtrCheck(*Peeling, TestBB, UnalignedMerge, *P.Plan, PeelBasePtr,
                         OrigLoop);
    }
    PeelCount = emitDynamicPeelCount(*Peeling, PeelBasePtr, Builder, OrigLoop);
    // Then insert check for peel count is zero
    auto *Zero =
        Plan.getVPConstant(ConstantInt::getNullValue(PeelCount->getType()));
    VPCmpInst *Cmp = Builder.createCmpInst(CmpInst::ICMP_EQ, Zero, PeelCount,
                                           "peel.zero.check");
    Plan.getVPlanDA()->markUniform(*Cmp);
    // go to merge after peel or to peel
    TestBB->setTerminator(P.PrevMerge, P.FirstBB, Cmp);
    // Update merge block incoming values
    updateMergeBlockIncomings(Plan, P.PrevMerge, TestBB, true /* UseLiveIn */);
  }

  // Update peel adaptor's upper bound.
  auto Adapter = llvm::find_if(*P.FirstBB, [](const VPInstruction &I) {
    return isa<VPlanPeelAdapter>(I);
  });
  assert(Adapter != P.FirstBB->end() && "expected peel adapter");
  // Set upper bound of the peel. We need to adjust it subtracting one if the
  // the loop is marked as having not exact UB. E.g. it's  latch condition looks
  // like below:
  //   %c = icmp ule %ind_var, %UB
  //   br %c, label %header, label %exit
  //
  // In this case to execute e.g. 3 iterations, we should set UB to 2 as
  // induction always starts with 0.
  VPLoop *VLoop = Plan.getMainLoop(true);
  VPValue *PeelCnt = PeelCount;
  if (!VLoop->exactUB()) {
    Type *Ty = PeelCount->getType();
    if (StaticPeel) {
      PeelCnt =
          Plan.getVPConstant(ConstantInt::get(Ty, StaticPeel->peelCount() - 1));
    } else {
      auto *I = cast<VPInstruction>(PeelCount);
      Builder.setInsertPoint(I->getParent(), std::next(I->getIterator()));
      VPConstant *One = Plan.getVPConstant(ConstantInt::get(Ty, 1));
      PeelCnt = Builder.createNaryOp(Instruction::Sub, Ty, {PeelCount, One});
    }
  }
  cast<VPlanPeelAdapter>(Adapter)->setUpperBound(PeelCnt);

  // Merge block after peel needs live out values.
  updateMergeBlockIncomings(P, P.PrevMerge, P.FirstBB, false /* UseLiveIn */);

  if (!FinalRemainderMerge) {
    // No remainder. A bit strange with peel but that can happen when TC is
    // known and static peel. Will not generate anything.
    assert(StaticPeel && "remainder is expected with non-static peel");
    return;
  }

  // Emit the check for (peel-count + main_VF) is greater than original upper
  // bound, jumping to remainder if so.
  VPBasicBlock *TestBB2 =
      new VPBasicBlock(VPlanUtils::createUniqueName("peel.checkv"), &Plan);
  VPBlockUtils::insertBlockBefore(TestBB2, P.FirstBB);
  Builder.setInsertPoint(TestBB2);
  VPCmpInst *Cmp =
      createPeelCntVFCheck(OrigUB, Builder, CmpInst::ICMP_UGE, MainVF * MainUF);
  Plan.getVPlanDA()->markUniform(*Cmp);
  // goto merge before remainder or to peel
  TestBB2->setTerminator(FinalRemainderMerge, P.FirstBB, Cmp);
  updateMergeBlockIncomings(Plan, FinalRemainderMerge, TestBB2,
                            true /* UseLiveIn */);
}

void VPlanCFGMerger::updateAdapterOperands(VPBasicBlock *AdapterBB,
                                           VPBasicBlock *MergeBB) {

  assert(MergeBB->getSingleSuccessor() == AdapterBB && "unexpected successor");
  assert(AdapterBB->getSinglePredecessor() == MergeBB &&
         "unexpected predecessor");
  assert((isMergeBlock(MergeBB) && MergeBB->size() > 1) &&
         "expected non-empty merge block");

  auto AdapterI = llvm::find_if(*AdapterBB, [](const VPInstruction &I) {
    return isa<VPlanAdapter>(I);
  });
  assert(AdapterI != AdapterBB->end() && "expected basic block with adapter");
  assert(!isa<VPlanPeelAdapter>(AdapterI) && "peel adapter is not expected");

  auto *Adapter = cast<VPlanAdapter>(&*AdapterI);
  for (auto &I : *MergeBB)
    if (isa<VPPHINode>(I))
      Adapter->addOperand(&I);
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
// |  |  |          +-----------------------+
// |  |  |          | Merge btw main/vecrem |
// |  |  |          +-----------------------+
// |  |  |                   |
// |  |  |                   |
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
//
// The skeleton emission is optimized for the simple scenario of constant
// trip count loops with a main vector and scalar remainder to enable
// better downstream optimizations. The main and remainder loops are
// emitted as straight line code without any checks for such a scenario.
//
template <class LoopTy>
void VPlanCFGMerger::emitSkeleton(std::list<PlanDescr> &Plans,
                                  LoopTy *OrigLoop) {
  using LT = CfgMergerPlanDescr::LoopType;

  VPBasicBlock *FinalMerge, *LastMerge;
  VPBasicBlock *FinalRemainderMerge = nullptr;
  auto LastVPBB = &*Plan.getExitBlock();

  // Find original upper bound of the main loop. We need it to generate trip
  // count checks.
  updateOrigUB();

  // Insert push/popVF around the original main VPlan, guarding its body.
  insertPushPopVF(Plan, MainVF, MainUF);

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

      // Insert PushVF/PopVF around VPlan for non-main plan.
      // For main plan we inserted them already at beginnnig of the routine.
      insertPushPopVF(*P.Plan, P.VF, 1);
    } else {
      // Special case for main loop. We don't create basic block with adaptor,
      // just setting First/Last basic blocks.
      assert(P.Plan == &Plan && "incorrect main VPlan");
      P.FirstBB = findFirstNonEmptyBB();
      P.LastBB = LastVPBB;
      Succ = LastMerge;
    }
    P.PrevMerge = Succ;

    if (!IsFirst && !P.isMaskedOrScalarRemainder()) {
      assert(Succ != FinalMerge && "not expected check");
      if (P.Type == LT::LTPeel) {
        assert(!IsFirst && "peel loop can't be the first in chain");
        // After peel we should have at least two loops created, main and
        // remainder. The situation with one loop (e.g. masked mode loop) is not
        // supported.
        auto PrevP = std::prev(Iter, 1); // main VPlan
        // Get a remainder VPlan that was placed before main VPlan. It's not
        // required in case when we have static peel and a known trip count. All
        // the needed assertions are done in the called routine.
        PlanDescr *RemDescr = nullptr;
        if (PrevP != Plans.begin())
          RemDescr = &*std::prev(Iter, 2);

        // Create peel count instruction, updating the upper bound of the
        // peel and insert the needed checks before peel.
        insertPeelCntAndChecks(P, FinalRemainderMerge, PrevP->PrevMerge,
                               OrigLoop);

        // Create trip count checks before main loop.
        // Check whether we have a remainder, and it has an additional
        // remainder. If so we need to pass that additional previous
        // remainder to form the correct cfg
        PlanDescr *PPrevRem = nullptr;
        if (RemDescr && RemDescr->isNonMaskedVecRemainder()) {
          auto PIter = std::prev(Iter, 2);
          if (PIter != Plans.begin())
            PPrevRem = &*(std::prev(Iter, 3));
        }
        createTCCheckBeforeMain(&P, *PrevP, RemDescr, PPrevRem);
      } else {
        // Create the needed trip count checks after VPlan when needed.
        // See Check6, Check7 on the diagram above.
        createTCCheckAfter(P, *std::prev(Iter, 1));
        if (P.isNonMaskedVecRemainder()) {
          auto *SingleSucc = P.LastBB->getSingleSuccessor();
          assert(SingleSucc && "Non-null successor expected.");
          // We need to create a merge block between main loop and non-masked
          // vectorized remainder.
          auto MergeBlk =
              createMergeBlockBefore(SingleSucc);
          updateMergeBlockIncomings(P, MergeBlk, P.LastBB,
                                    false /* UseLiveIn */);
          // Need to relink uses of outgoing values to the phis from new block.
          auto NextBB = MergeBlk->getSingleSuccessor();
          for (VPInstruction &Inst : *MergeBlk) {
            auto *MergePhi = dyn_cast<VPPHINode>(&Inst);
            // Can have VPBranchInst
            if (!MergePhi)
              continue;
            for (VPValue *Op : MergePhi->operands()) {
              // The replaceUsesWithIf() does not work here as we have
              // (intentionally and temporary at this stage) type inconsistency
              // between phi and its operands. Operands at this stage are
              // VPlanAdapter which are of type token.
              SmallVector<VPUser *, 2> UsersToUpdate(make_filter_range(
                  Op->users(), [MergePhi, Op, NextBB](VPUser *U) {
                    auto *Phi = dyn_cast<VPPHINode>(U);
                    return Phi && Phi != MergePhi &&
                           Phi->getMergeId() == MergePhi->getMergeId() &&
                           Phi->getIncomingBlock(Op) == NextBB;
                  }));
              for (VPUser *U : UsersToUpdate)
                U->replaceUsesOfWith(Op, MergePhi);
            }
          }
          P.PrevMerge = MergeBlk;
        }
      }
    } else {
      // First or masked or scalar remainder.
      updateMergeBlockIncomings(P, Succ, P.LastBB, false /* UseLiveIn */);
    }

    if (std::next(Iter, 1) != Plans.end()) {
      // If we need to insert more VPlans we create a merge block before the
      // current one.
      LastMerge = createMergeBlockBefore(P.FirstBB);
      P.MergeBefore = LastMerge;

      if (P.Type != LT::LTMain)
        updateAdapterOperands(P.FirstBB, LastMerge);

      if (!FinalRemainderMerge && P.Type == LT::LTRemainder &&
          !isa<VPlanNonMasked>(P.Plan))
        FinalRemainderMerge = LastMerge;
    } else if (P.Type != LT::LTPeel) {
      // If that is the last non-peel VPlan it should be main VPlan and
      // we need to generate the needed trip count check before it.
      assert(P.Type == LT::LTMain && "expected main loop");
      PlanDescr *PrevD = IsFirst ? nullptr : &*(std::prev(Iter, 1));
      // PPrevD is a remainder after non-masked mode remainder.
      PlanDescr *PPrevD = nullptr;
      if (PrevD && PrevD->isNonMaskedVecRemainder()) {
        auto PIter = std::prev(Iter, 1);
        if (PIter != Plans.begin())
          PPrevD = &*(std::prev(Iter, 2));
      }
      // Check4, Check5 on the diagram above
      createTCCheckBeforeMain(nullptr, P, PrevD, PPrevD);
    }
  }

  // Set the merge-phis from FinalMerge as operands of VPExternalUses.
  updateExternalUsesOperands(FinalMerge);

  // Now insert push/popVF once again around the whole merged CFG. We need this
  // for correct pre-main-loop tests (peel and top test).
  insertPushPopVF(Plan, MainVF, MainUF);

  // Hoist the original upper bound (if it's a VPInstruction) to the first
  // non-empty block. We use it in several places of CFG and need it to be in
  // the most dominating block.
  moveOrigUBToBegin();

  // Update every VPVectorTripCountCalculation to account peeling.
  if (PeelCount) {
    SmallVector<VPUser *, 8> OrigUbUsers(
        make_filter_range(OrigUB->users(), [](auto *U) {
          return isa<VPVectorTripCountCalculation>(U);
        }));
    for (auto UbUse : OrigUbUsers) {
      auto VTC = cast<VPVectorTripCountCalculation>(UbUse);
      assert(VTC->getNumOperands() == 1 && "unexpected VTC operand");
      VPBuilder Builder;
      Builder.setInsertPoint(VTC);
      VPValue *Casted = Builder.createIntCast(PeelCount, OrigUB->getType());
      if (Casted != PeelCount)
        Plan.getVPlanDA()->markUniform(*Casted);
      VTC->addOperand(Casted);
    }
  }
  VPLAN_DUMP(MergeSkeletonDumpControl, Plan);
}

// TODO: Implement the check. It should return true if we create peel loop
// for safety, e.g. in the search loop.
bool VPlanCFGMerger::needPeelForSafety() const {
  return NeedPeelForSafety;
}

void VPlanCFGMerger::updateOrigUB() {
  // Get original upper bound using VPlan main loop.
  if (isa<VPlanNonMasked>(Plan))
    OrigUB = findVectorUB(Plan)->getOperand(0);
  else {
    // Get the UB from the latch condition (its invariant operand).
    // Note that if the original loop did not have normalized loop IV, here
    // we know that we emitted one. Thus we rely on that.
    VPLoop *L = *cast<VPlanMasked>(Plan).getVPLoopInfo()->begin();
    std::tie(OrigUB, std::ignore) =
        L->getLoopUpperBound(/*AssumeNormalizedIV*/ true);

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

void VPlanCFGMerger::copyDA(std::list<PlanDescr> &Plans) {
  using LT = CfgMergerPlanDescr::LoopType;
  VPlanDivergenceAnalysis *DA = Plan.getVPlanDA();
  for (auto P : Plans) {
    if (P.Type == LT::LTMain)
      continue;

    if (isa<VPlanScalar>(P.Plan)) {
      auto *ScalarDA = cast<VPlanDivergenceAnalysisScalar>(P.Plan->getVPlanDA());
      auto ScalarShapes = vplan_da_shapes(P.Plan, ScalarDA);
      DA->copyShapes(ScalarShapes.begin(), ScalarShapes.end());
    } else {
      // Specialize for non-scalar to have a faster version.
      auto *VectorDA = cast<VPlanDivergenceAnalysis>(P.Plan->getVPlanDA());
      auto VectorShapes = VectorDA->shapes();
      DA->copyShapes(VectorShapes.begin(), VectorShapes.end());
    }
  }
}

// Replace live-in values of VPlan \p P by ones passed in \p Range.
template <class IterRange>
static void updateVPlanLiveIns(VPlan *P, IterRange &Range) {
  for (auto &Op : Range) {
    if (isa<VPBranchInst>(Op))
      continue;
    auto *Phi = dyn_cast<VPPHINode>(&Op);
    assert(Phi && "expected a VPPHINode");

    unsigned MergeId = Phi->getMergeId();
    assert(MergeId != VPExternalUse::UndefMergeId &&
           "Unexpected instruction in a merge block");
    if (auto *LI = P->getLiveInValue(MergeId))
      const_cast<VPLiveInValue *>(LI)->replaceAllUsesWith(Phi);
  }
}

void VPlanCFGMerger::replaceAdapterUses(VPlanAdapter *Adapter, VPlan &P) {

  VPBasicBlock *PlanEnd = &*find_if(
      P, [](const VPBasicBlock &BB) { return BB.getNumSuccessors() == 0; });

  VPBasicBlock *AdapterParent = Adapter->getParent();
  SmallVector<VPUser *, 4> AdapterUsers(Adapter->users());
  for (auto U : AdapterUsers) {
    auto *Phi = cast<VPPHINode>(U);
    unsigned MergeId = Phi->getMergeId();
    if (MergeId == VPExternalUse::UndefMergeId)
      llvm_unreachable("Unexpected instruction in a merge block");

    VPValue *NewOp;
    if (auto *LO = P.getLiveOutValue(MergeId))
      NewOp = LO->getOperand(0);
    else
      NewOp = Plan.getVPConstant(UndefValue::get(Phi->getType()));

    int Idx = Phi->getBlockIndex(AdapterParent);
    if (Idx >= 0) {
      assert(Phi->getIncomingValue(Idx) == Adapter &&
             "expected adapter as operand");
      Phi->setIncomingValue(Idx, NewOp);
      Phi->setIncomingBlock(Idx, PlanEnd);
    } else {
      Phi->replaceUsesOfWith(Adapter, NewOp);
    }
  }
}

void VPlanCFGMerger::updateVPlansIncomings(std::list<PlanDescr> &Plans) {
  using LT = CfgMergerPlanDescr::LoopType;
  for (auto Iter = Plans.begin(); Iter != Plans.end(); Iter++) {
    auto &P = *Iter;

    // Find VPlan adapter.
    auto AdapterI = P.FirstBB->end();
    if (P.Type != LT::LTMain) {
      AdapterI = llvm::find_if(*P.FirstBB, [](const VPInstruction &I) {
        return isa<VPlanAdapter>(I);
      });
      assert(AdapterI != P.FirstBB->end() && "expected vplan adapter");
    }

    if (std::next(Iter, 1) == Plans.end()) {
      assert((P.Type == LT::LTPeel || P.Type == LT::LTMain) &&
             "expected peel or main vplan");
      // The last inserted VPlan (peel or main one), i.e. the most upper one in
      // CFG, always uses original incoming values.
      VPLiveInOutCreator LICreator(*P.Plan);
      LICreator.restoreLiveIns();
      if (P.Type != LT::LTMain)
        // Update uses of Adapter by VPlan's outigoing values.
        replaceAdapterUses(cast<VPlanAdapter>(&*AdapterI), *P.Plan);
      continue;
    }

    if (P.Type == LT::LTMain) {
      // If main VPlan has a predecessor VPlan in CFG then we update its
      // incoming values from the predecessor merge block.
      VPBasicBlock *MergeBB = P.MergeBefore;
      assert((MergeBB && isMergeBlock(MergeBB)) &&
             "expected non-null merge block");
      updateVPlanLiveIns(P.Plan, *MergeBB);
      continue;
    }

    // Now update VPlan incomings from adapter's operands.
    // Need re-map pointers to references to align with VPBasicBlock iterator in
    // the previous call to updateVPlanLiveIns.
    auto OpRange = map_range(AdapterI->operands(),
                             [](VPValue *Op) -> VPValue & { return *Op; });
    updateVPlanLiveIns(P.Plan, OpRange);

    // Update uses of Adapter by VPlan's outigoing values.
    replaceAdapterUses(cast<VPlanAdapter>(&*AdapterI), *P.Plan);
  }
}

// Merge VPLoopInfo from \p P into main vplan.
// The merging includes:
//    - copying of the loop structure to the upper level of the VPLoopInfo of
//      the main VPlan.
//    - adding the basic blocks from the old loops to the new ones
// Of course, no clonning is done as the blocks from \p P should be in the main
// VPlan.
//
void VPlanCFGMerger::mergeLoopInfo(VPlanVector &P) {
  VPLoopInfo *DestLI = Plan.getVPLoopInfo();
  VPLoopInfo *SrcLI = P.getVPLoopInfo();

  auto CopyLoop = [DestLI, SrcLI, this](VPLoop *L,
                                        VPLoop *ParentL) -> VPLoop * {
    VPLoop *NewLoop = DestLI->AllocateLoop();
    if (ParentL)
      ParentL->addChildLoop(NewLoop);
    else
      DestLI->addTopLevelLoop(NewLoop);

    NewLoop->copyHasNormalizedInductionFlag(L);
    NewLoop->setOptReport(L->getOptReport());
    ExtVals.setOptRptStatsForLoop(NewLoop,
                                  ExtVals.getOrCreateOptRptStatsForLoop(L));

    // Add all of the blocks in L to the new loop.
    for (auto BB : L->getBlocks())
      if (SrcLI->getLoopFor(BB) == L)
        NewLoop->addBasicBlockToLoop(BB, *DestLI);
    return NewLoop;
  };
  DenseMap<VPLoop * /*Src*/, VPLoop * /*Dest*/> LoopMap;
  SmallVector<VPLoop *, 4> SrcLoops = SrcLI->getLoopsInPreorder();
  for (auto L : SrcLoops) {
    VPLoop *ParentL = L->getParentLoop();
    if (ParentL)
      ParentL = LoopMap[ParentL];
    LoopMap[L] = CopyLoop(L, ParentL);
  }
}

void VPlanCFGMerger::mergeVPlanBodies(std::list<PlanDescr> &Plans) {
  using LT = CfgMergerPlanDescr::LoopType;
  for (auto P : Plans) {
    if (P.Type == LT::LTMain)
      continue;
    // Move blocks from inner VPlan into main VPlan.
    VPBasicBlock *Begin = &P.Plan->getEntryBlock();
    VPBasicBlock *End = &*find_if(*P.Plan, [](const VPBasicBlock &BB) {
      return BB.getNumSuccessors() == 0;
    });
    VPBasicBlock *VPFirstBB = P.FirstBB;
    Plan.getBasicBlockList().splice(VPFirstBB->getIterator(),
                                    P.Plan->getBasicBlockList());
    // Relink blocks in CFG.
    VPFirstBB->getSinglePredecessor()->replaceSuccessor(VPFirstBB, Begin);
    End->setTerminator(VPFirstBB->getSingleSuccessor());
    VPFirstBB->setTerminator();

    // Add the instructions we're about to remove into the UnlinkedVPInsns.
    for (auto &VPI : make_early_inc_range(reverse(*VPFirstBB)))
      VPFirstBB->eraseInstruction(&VPI);

    Plan.getBasicBlockList().erase(VPFirstBB);
    if (auto VecPlan = dyn_cast<VPlanVector>(P.Plan))
      mergeLoopInfo(*VecPlan);
  }
  VPLAN_DUMP(MergePass2DumpControl, Plan);
}

void VPlanCFGMerger::mergeVPlans(std::list<CfgMergerPlanDescr> &Plans) {

  VPLoop *VLoop = Plan.getMainLoop(true);
  (void) VLoop;

  copyDA(Plans);
  updateVPlansIncomings(Plans);
  mergeVPlanBodies(Plans);

  Plan.setExplicitRemainderUsed();

  // Sanity check.
  assert(VLoop == *Plan.getVPLoopInfo()->begin() &&
         "Unexpected change of top loop");

  // Invalidate SVA results as VPlan has been changed.
  Plan.invalidateAnalyses({VPAnalysisID::SVA});

  Plan.computeDT();
  Plan.computePDT();
}

template void
VPlanCFGMerger::createMergedCFG<Loop>(SingleLoopVecScenario &Scen,
                                      std::list<CfgMergerPlanDescr> &Plans,
                                      Loop *OrigLoop);
template void VPlanCFGMerger::createMergedCFG<loopopt::HLLoop>(
    SingleLoopVecScenario &Scen, std::list<CfgMergerPlanDescr> &Plans,
    loopopt::HLLoop *OrigLoop);

template void VPlanCFGMerger::emitSkeleton<Loop>(std::list<PlanDescr> &Plans,
                                                 Loop *OrigLoop);
template void
VPlanCFGMerger::emitSkeleton<loopopt::HLLoop>(std::list<PlanDescr> &Plans,
                                              loopopt::HLLoop *OrigLoop);

template void VPlanCFGMerger::insertPeelCntAndChecks<Loop>(
    PlanDescr &PeelDescr, VPBasicBlock *FinalRemainderMerge,
    VPBasicBlock *RemainderMerge, Loop *OrigLoop);
template void VPlanCFGMerger::insertPeelCntAndChecks<loopopt::HLLoop>(
    PlanDescr &PeelDescr, VPBasicBlock *FinalRemainderMerge,
    VPBasicBlock *RemainderMerge, loopopt::HLLoop *OrigLoop);

template void
VPlanCFGMerger::createPeelPtrCheck<Loop>(VPlanDynamicPeeling &Peeling,
                                         VPBasicBlock *InsertBefore,
                                         VPBasicBlock *NonZeroMerge, VPlan &P,
                                         VPValue *&PeelBasePtr, Loop *OrigLoop);
template void VPlanCFGMerger::createPeelPtrCheck<loopopt::HLLoop>(
    VPlanDynamicPeeling &Peeling, VPBasicBlock *InsertBefore,
    VPBasicBlock *NonZeroMerge, VPlan &P, VPValue *&PeelBasePtr,
    loopopt::HLLoop *OrigLoop);

template VPValue *
VPlanCFGMerger::emitDynamicPeelCount<Loop>(VPlanDynamicPeeling &DP,
                                           VPValue *BasePtr, VPBuilder &Builder,
                                           Loop *OrigLoop);
template VPValue *VPlanCFGMerger::emitDynamicPeelCount<loopopt::HLLoop>(
    VPlanDynamicPeeling &DP, VPValue *BasePtr, VPBuilder &Builder,
    loopopt::HLLoop *OrigLoop);
