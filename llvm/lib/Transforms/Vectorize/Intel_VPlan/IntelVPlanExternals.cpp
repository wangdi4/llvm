//===- IntelVPlanExternals.cpp - VPlan externals storage   ----------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is the VPlan externals storage implementation.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanExternals.h"
#include "IntelVPlan.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"

#define DEBUG_TYPE "VPlanExternals"

using namespace llvm;
using namespace llvm::vpo;

static LoopVPlanDumpControl
    LiveInOutListsDumpControl("live-inout-list", "live in/out lists creation");

void VPExternalValues::verifyVPConstants() const {
  SmallPtrSet<const Constant *, 16> ConstantSet;
  for (const auto &Pair : VPConstants) {
    const Constant *KeyConst = Pair.first;
    assert(KeyConst == Pair.second->getConstant() &&
           "Value key and VPConstant's underlying Constant must be the same!");
    // Checking that an element is repeated in a map is unnecessary but it
    // will catch bugs if the data structure is changed in the future.
    assert(!ConstantSet.count(KeyConst) && "Repeated VPConstant!");
    ConstantSet.insert(KeyConst);
  }
}

void VPExternalValues::verifyVPExternalDefs() const {
  SmallPtrSet<const Value *, 16> ValueSet;
  for (const auto &Def : VPExternalDefs) {
    const Value *KeyVal = Def->getUnderlyingValue();
    assert(!ValueSet.count(KeyVal) && "Repeated VPExternalDef!");
    ValueSet.insert(KeyVal);
  }
}

void VPExternalValues::verifyVPExternalDefsHIR() const {
  SmallSet<unsigned, 16> IVLevelSet;
  for (const auto &ExtDef : VPExternalDefsHIR) {
    const VPOperandHIR *HIROperand = ExtDef.getOperandHIR();

    // Deeper verification depending on the kind of the underlying HIR operand.
    if (isa<VPBlob>(HIROperand) || isa<VPCanonExpr>(HIROperand) ||
        isa<VPIfCond>(HIROperand)) {
      // For blobs and CEs check that they are structurally unique.
      assert(
          llvm::count_if(VPExternalDefsHIR,
                         [HIROperand](const VPExternalDef &ExtDef) {
                           return ExtDef.getOperandHIR()->isStructurallyEqual(
                               HIROperand);
                         }) == 1 &&
          "Repeated Blob/CanonExpr VPExternalDef!");
    } else {
      // For IVs we check that the IV levels are unique.
      const auto *IV = cast<VPIndVar>(HIROperand);
      unsigned IVLevel = IV->getIVLevel();
      assert(!IVLevelSet.count(IVLevel) && "Repeated IV VPExternalDef!");
      IVLevelSet.insert(IVLevel);
    }
  }
}

void VPExternalValues::verifyVPMetadataAsValues() const {
  SmallPtrSet<const MetadataAsValue *, 16> MDAsValueSet;
  for (const auto &Pair : VPMetadataAsValues) {
    const MetadataAsValue *KeyMD = Pair.first;
    assert(KeyMD == Pair.second->getMetadataAsValue() &&
           "Value key and VPMetadataAsValue's underlying MetadataAsValue must "
           "be the same!");
    // Checking that an element is repeated in a map is unnecessary but it
    // will catch bugs if the data structure is changed in the future.
    assert(!MDAsValueSet.count(KeyMD) && "Repeated MetadataAsValue!");
    MDAsValueSet.insert(KeyMD);
  }
}

template <class InitTy, class FinalTy, class EntityTy>
static std::tuple<InitTy *, FinalTy *>
getInitFinal(EntityTy *E, VPLiveInOutCreator::VPExtUseList &UseList) {
  InitTy *Init = nullptr;
  FinalTy *Final = nullptr;

  for (VPValue *Val : E->getLinkedVPValues()) {
    if (!Init)
      Init = dyn_cast<InitTy>(Val);
    if (!Final)
      if ((Final = dyn_cast<FinalTy>(Val))) {
        for (auto *U : Final->users())
          if (auto *EU = dyn_cast<VPExternalUse>(U))
            UseList.push_back(EU);
      }
  }
  return std::make_tuple(Init, Final);
}

template <class InitTy, class FinalTy>
void VPLiveInOutCreator::addInOutValues(InitTy *Init, FinalTy *Final,
                                        VPExtUseList &ExtUseList,
                                        bool ExtUseAdded, VPValue *StartV) {
  VPExternalValues &ExtVals = Plan.getExternals();

  int Index = 0;
  VPLiveInValue *LIV = nullptr;
  assert(!ExtUseAdded ||
         ExtUseList.size() == 1 && "Expected one added external use");
  for (VPExternalUse *ExtUse : ExtUseList) {
    // Create live-in/out descriptors
    int MergeId = ExtUse->getMergeId();
    if (Index == 0)
      LIV = createLiveInValue(MergeId, StartV->getType());
    VPLiveOutValue *LOV = createLiveOutValue(MergeId, Final);
    // Unlink External use
    ExtUse->removeOperand(ExtUse->getOperandIndex(Final));
    // Put live-in/out to their lists
    if (ExtUseAdded) {
      if (Index == 0)
        Plan.addLiveInValue(LIV);
      Plan.addLiveOutValue(LOV);
      ExtVals.addOriginalIncomingValue(StartV);
    } else {
      if (Index == 0)
        Plan.setLiveInValue(LIV, MergeId);
      Plan.setLiveOutValue(LOV, MergeId);
      ExtVals.setOriginalIncomingValue(StartV, MergeId);
    }
    Index++;
  }
  // Replace start value with live-in
  if (Init->usesStartValue())
    Init->replaceStartValue(LIV);
  if (Final->usesStartValue())
    Final->replaceStartValue(LIV);
}

void VPLiveInOutCreator::addOriginalLiveInOut(
    const VPLoopEntityList *VPLEntityList, Loop *OrigLoop, VPLoopEntity *E,
    VPExtUseList &ExtUseList, ScalarInOutList &ScalarInOuts) {
  VPPHINode *VPPhi = VPLEntityList->getRecurrentVPHINode(*E);
  PHINode *Phi = nullptr;
  if (VPPhi) {
    Phi = cast<PHINode>(VPPhi->getUnderlyingValue());
    assert(Phi->getParent() == OrigLoop->getHeader() &&
           "Unexpected recurrent phi");
  }
  int StartValOpNum =
      Phi ? Phi->getBasicBlockIndex(OrigLoop->getLoopPreheader()) : -1;
  int Index = 0;
  for (auto *ExtUse : ExtUseList) {
    const Value *LiveOut = nullptr;
    if (ExtUse->hasUnderlying())
      // If the external use exists and it has underlying value, we use it to
      // set scalar live out. External use or its underlying value may not
      // exist, e.g. for non-liveout inductions.
      LiveOut = ExtUse->getUnderlyingOperand(0);
    else {
      // Otherwise use phi node incoming operand that comes from latch.
      assert((Phi && Phi->getNumOperands() == 2) && "Unexpected recurrent Phi");
      assert(Index == 0 && "Expected only for one liveout");
      LiveOut = Phi->getOperand(StartValOpNum ^ 1);
    }
    if (Index == 0)
      ScalarInOuts.add(Phi, StartValOpNum, LiveOut, ExtUse->getMergeId());
    else
      ScalarInOuts.add(nullptr, -1, LiveOut, ExtUse->getMergeId());
    Index++;
  }
}

void VPLiveInOutCreator::addOriginalLiveInOut(
    const VPLoopEntityList *, loopopt::HLLoop *OrigLoop, VPLoopEntity *E,
    VPExtUseList &ExtUseList, ScalarInOutListHIR &ScalarInOuts) {
  // External use w/o underlying operand is created only for main loop IV.
  // TODO: Is this valid?
  for (auto ExtUse : ExtUseList) {
    bool IsMainLoopIV = !ExtUse->hasUnderlying();
    // Carry over knowledge of MainLoopIV
    // to explicit VPInstructions in CFG.
    if (auto *Ind = dyn_cast<VPInduction>(E)) {
      VPInductionInit *Init = nullptr;
      VPInductionInitStep *InitStep = nullptr;
      for (VPValue *Val : Ind->getLinkedVPValues()) {
        if (!Init)
          Init = dyn_cast<VPInductionInit>(Val);
        if (!InitStep)
          InitStep = dyn_cast<VPInductionInitStep>(Val);
      }
      assert((Init && InitStep) &&
             "Expected non-null init and init-step for every induction.");
      Init->setIsMainLoopIV(IsMainLoopIV);
      InitStep->setIsMainLoopIV(IsMainLoopIV);
    }
    // For a given VPEntity a single temp is expected to be live-in, live-out or
    // both. We rely purely on VPExternalUse here to capture the temp associated
    // with current loop entity. For main loop IV we track only the final value
    // of IV as liveout which should be loop's UB.
    auto *HIROperand = ExtUse->getOperandHIR();
    const loopopt::DDRef *HIRTemp = !IsMainLoopIV
                                        ? cast<VPBlob>(HIROperand)->getBlob()
                                        : OrigLoop->getUpperDDRef();
    LLVM_DEBUG(dbgs() << "HIR addOriginalLiveInOut: "
                      << "HIRTemp: ";
               HIRTemp->dump();
               dbgs() << ", MainLoopIV: " << IsMainLoopIV << "\n");
    ScalarInOuts.add(HIRTemp, IsMainLoopIV, ExtUse->getMergeId());
  }
}

template <class LoopTy>
void VPLiveInOutCreator::createInOutsInductions(
    const VPLoopEntityList *VPLEntityList, LoopTy *OrigLoop) {

  VPExternalValues &ExtVals = Plan.getExternals();
  auto &ScalarInOuts = *ExtVals.getOrCreateScalarLoopInOuts(OrigLoop);

  for (auto *Ind : VPLEntityList->vpinductions()) {
    if (Ind->getIsMemOnly())
      continue;
    VPInductionInit *IndInit = nullptr;
    VPInductionFinal *IndFinal = nullptr;
    VPExtUseList IndFinalExternalUse;
    std::tie(IndInit, IndFinal) =
        getInitFinal<VPInductionInit, VPInductionFinal, VPInduction>(
            Ind, IndFinalExternalUse);
    assert((IndInit && IndFinal) && "Expected non-null init, final");
    VPValue *StartV = IndInit->getStartValueOperand();
    bool ExtUseAdded = false;
    // Inductions should always have outgoing value
    if (IndFinalExternalUse.empty()) {
      if (auto ExtDef = dyn_cast<VPExternalDef>(StartV))
        if (auto VBlob = dyn_cast_or_null<VPBlob>(ExtDef->getOperandHIR())) {
          auto ExtUse =
              ExtVals.getOrCreateVPExternalUseForDDRef(VBlob->getBlob());
          ExtUse->addOperand(IndFinal);
          IndFinalExternalUse.push_back(ExtUse);
        }
      if (IndFinalExternalUse.empty()) {
        // That means we have a constant start value. Should be main loop IV,
        // create ExternalUse without underlying IR.
        auto ExtUse = ExtVals.createVPExternalUseNoIR(IndInit->getType());
        ExtUse->addOperand(IndFinal);
        IndFinalExternalUse.push_back(ExtUse);
      }
      ExtUseAdded = true;
    }
    addInOutValues(IndInit, IndFinal, IndFinalExternalUse, ExtUseAdded,
                   IndInit->getStartValueOperand());
    addOriginalLiveInOut(VPLEntityList, OrigLoop, Ind, IndFinalExternalUse,
                         ScalarInOuts);
  }
}

template void
VPLiveInOutCreator::createInOutsInductions<Loop>(const VPLoopEntityList *,
                                                 Loop *);
template void VPLiveInOutCreator::createInOutsInductions<loopopt::HLLoop>(
    const VPLoopEntityList *, loopopt::HLLoop *);

template <class LoopTy>
void VPLiveInOutCreator::createInOutsReductions(
    const VPLoopEntityList *VPLEntityList, LoopTy *OrigLoop) {

  VPExternalValues &ExtVals = Plan.getExternals();
  auto &ScalarInOuts = *ExtVals.getOrCreateScalarLoopInOuts(OrigLoop);

  for (auto *Red : VPLEntityList->vpreductions()) {
    // Ignore in-memory reductions, they are neither live-in/live-out.
    if (Red->getIsMemOnly())
      continue;
    VPReductionInit *RedInit = nullptr;
    VPReductionFinal *RedFinal = nullptr;
    VPExtUseList RedFinalExternalUse;
    std::tie(RedInit, RedFinal) =
        getInitFinal<VPReductionInit, VPReductionFinal, VPReduction>(
            Red, RedFinalExternalUse);
    assert((RedInit && RedFinal) && "Expected non-null init and final");
    VPValue *StartV = nullptr;
    if (RedInit->usesStartValue())
      StartV = RedInit->getStartValueOperand();
    else
      StartV = RedFinal->getStartValueOperand();
    assert(StartV && "StartV is expected to be non-null here.");
    bool ExtUseAdded = false;
    if (RedFinalExternalUse.empty()) {
      // No external use can be possible for some auto-generated reductions,
      // e.g. fake linear index for min/max+index idiom or for reductions that
      // are not used outside of the loop or in-memory reductions. Then we
      // create a fake external use, with symbase equal to symbase of start
      // value.
      // We don't need to create an external use in cases when it's a fake
      // linear index. In other cases we need to keep it's value after peel/main
      // loop.
      if (RedFinal->isLinearIndex())
        continue;
      if (auto ExtDef = dyn_cast<VPExternalDef>(StartV))
        if (auto VBlob = dyn_cast_or_null<VPBlob>(ExtDef->getOperandHIR())) {
          auto ExtUse =
              ExtVals.getOrCreateVPExternalUseForDDRef(VBlob->getBlob());
          ExtUse->addOperand(RedFinal);
          RedFinalExternalUse.push_back(ExtUse);
        }
      if (RedFinalExternalUse.empty()) {
        // That means we have a constant start value.
        auto ExtUse = ExtVals.createVPExternalUseNoIR(RedInit->getType());
        ExtUse->addOperand(RedFinal);
        RedFinalExternalUse.push_back(ExtUse);
      }
      ExtUseAdded = true;
    }
    addInOutValues(RedInit, RedFinal, RedFinalExternalUse, ExtUseAdded, StartV);
    addOriginalLiveInOut(VPLEntityList, OrigLoop, Red, RedFinalExternalUse,
                         ScalarInOuts);
  }
}

template void
VPLiveInOutCreator::createInOutsReductions<Loop>(const VPLoopEntityList *,
                                                 Loop *);
template void VPLiveInOutCreator::createInOutsReductions<loopopt::HLLoop>(
    const VPLoopEntityList *, loopopt::HLLoop *);

template <class LoopTy>
void VPLiveInOutCreator::createInOutsPrivates(
    const VPLoopEntityList *VPLEntityList, LoopTy *OrigLoop) {

  VPExternalValues &ExtVals = Plan.getExternals();
  auto &ScalarInOuts = *ExtVals.getOrCreateScalarLoopInOuts(OrigLoop);
  for (auto *Priv : VPLEntityList->vpprivates()) {
    if (Priv->getIsMemOnly())
      continue;
    if (!Priv->isLast())
      continue;

    for (VPValue *Val : Priv->getLinkedVPValues()) {
      auto PrivFinal = dyn_cast<VPInstruction>(Val);
      if (!PrivFinal)
        continue;
      VPExtUseList PrivFinalExternalUse;
      if (PrivFinal->getOpcode() == VPInstruction::PrivateFinalUncond ||
          PrivFinal->getOpcode() == VPInstruction::PrivateFinalCond) {
        for (auto *U : PrivFinal->users())
          if (auto *EU = dyn_cast<VPExternalUse>(U))
            PrivFinalExternalUse.push_back(EU);
      } else {
        continue;
      }

      assert(!PrivFinalExternalUse.empty() && "Expected non-null external use");
      // Create live-in/out descriptors
      for (auto *ExtUse : PrivFinalExternalUse) {
        int MergeId = ExtUse->getMergeId();
        VPLiveOutValue *LOV = createLiveOutValue(MergeId, PrivFinal);
        // Unlink External use
        ExtUse->removeOperand(ExtUse->getOperandIndex(PrivFinal));
        // Put live-in/out to their lists
        Plan.setLiveOutValue(LOV, MergeId);
      }
      VPValue *StartV;
      if (auto *PrivInst = dyn_cast<VPPrivateFinalCond>(PrivFinal))
        StartV = PrivInst->getOrig();
      else {
        // For unconditional privates add "undef" incoming value.
        StartV = Plan.getVPConstant(UndefValue::get(PrivFinal->getType()));
      }
      int MergeId = PrivFinalExternalUse[0]->getMergeId();
      VPLiveInValue *LIV = createLiveInValue(MergeId, StartV->getType());
      Plan.setLiveInValue(LIV, MergeId);
      ExtVals.setOriginalIncomingValue(StartV, MergeId);
      if (auto *PrivInst = dyn_cast<VPPrivateFinalCond>(PrivFinal))
        PrivInst->setOrig(LIV);
      addOriginalLiveInOut(VPLEntityList, OrigLoop, Priv, PrivFinalExternalUse,
                           ScalarInOuts);
    }
  }
}

template void
VPLiveInOutCreator::createInOutsPrivates<Loop>(const VPLoopEntityList *,
                                               Loop *);
template void VPLiveInOutCreator::createInOutsPrivates<loopopt::HLLoop>(
    const VPLoopEntityList *, loopopt::HLLoop *);

template <>
void VPLiveInOutCreator::createInOutsCompressExpandIdioms(
    const VPLoopEntityList *VPLEntityList, Loop *) {

  assert(VPLEntityList->vpceidioms().empty() &&
         "Compress/expand idiom support for LLVM IR is not implemented.");
}

template <>
void VPLiveInOutCreator::createInOutsCompressExpandIdioms(
    const VPLoopEntityList *VPLEntityList, loopopt::HLLoop *OrigLoop) {

  VPExternalValues &ExtVals = Plan.getExternals();
  auto &ScalarInOuts = *ExtVals.getOrCreateScalarLoopInOuts(OrigLoop);
  for (VPCompressExpandIdiom *CEIdiom : VPLEntityList->vpceidioms()) {

    VPExtUseList ExtUseList;
    VPCompressExpandFinal *Final = CEIdiom->getFinal();
    for (VPUser *User : Final->users())
      if (VPExternalUse *ExtUse = dyn_cast<VPExternalUse>(User))
        ExtUseList.push_back(ExtUse);

    bool NeedAddExtUse = ExtUseList.empty();
    if (NeedAddExtUse) {

      VPExternalDef *ExtDef = cast<VPExternalDef>(CEIdiom->getLiveIn());
      const VPBlob *VBlob = cast<VPBlob>(ExtDef->getOperandHIR());
      const loopopt::DDRef *DDRef = VBlob->getBlob();

      const loopopt::RegDDRef *RegDDRef = dyn_cast<loopopt::RegDDRef>(DDRef);
      if (!RegDDRef) {
        const loopopt::BlobDDRef *BlobRef = cast<loopopt::BlobDDRef>(DDRef);
        RegDDRef = BlobRef->getDDRefUtils().createScalarRegDDRef(
            BlobRef->getSymbase(), BlobRef->getSingleCanonExpr()->clone());
      }

      VPExternalUse *ExtUse =
          ExtVals.getOrCreateVPExternalUseForDDRef(RegDDRef);
      ExtUse->addOperand(Final);
      ExtUseList.push_back(ExtUse);
    }

    addInOutValues(CEIdiom->getInit(), Final, ExtUseList, NeedAddExtUse,
                   CEIdiom->getLiveIn());
    addOriginalLiveInOut(VPLEntityList, OrigLoop, CEIdiom, ExtUseList,
                         ScalarInOuts);
  }
}

template <class LoopTy>
void VPLiveInOutCreator::createInOutValues(LoopTy *OrigLoop) {
  VPlanVector &VecPlan = cast<VPlanVector>(Plan);
  const VPLoop *VLoop = *VecPlan.getVPLoopInfo()->begin();
  if (!VLoop->getUniqueExitBlock())
    return;

  VPExternalValues &ExtVals = Plan.getExternals();
  unsigned ExtUseCount = ExtVals.getLastMergeId();

  const VPLoopEntityList *VPLEntityList = VecPlan.getLoopEntities(VLoop);
  assert(VPLEntityList && "VPLEntityList is expected to be non-null here.");

  // We need to allocate LiveIn/LiveOut lists here, along with
  // OriginalIncomingValues, due to we look through entities and the lookup is
  // not in the order of VPExternalUses creation but we need to place the
  // items in the needed order.
  ExtVals.allocateOriginalIncomingValues(ExtUseCount);
  Plan.allocateLiveInValues(ExtUseCount);
  Plan.allocateLiveOutValues(ExtUseCount);

  createInOutsInductions(VPLEntityList, OrigLoop);
  createInOutsReductions(VPLEntityList, OrigLoop);
  createInOutsPrivates(VPLEntityList, OrigLoop);
  createInOutsCompressExpandIdioms(VPLEntityList, OrigLoop);

  VPLAN_DUMP(LiveInOutListsDumpControl, Plan);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (LiveInOutListsDumpControl.dumpPlain())
    Plan.getExternals().dumpScalarInOuts(outs(), OrigLoop);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
}

template void VPLiveInOutCreator::createInOutValues<Loop>(Loop *);
template void
VPLiveInOutCreator::createInOutValues<loopopt::HLLoop>(loopopt::HLLoop *);

void VPLiveInOutCreator::restoreLiveIns() {
  VPExternalValues &ExtVals = Plan.getExternals();
  for (VPLiveInValue *LIV : Plan.liveInValues()) {
    if (LIV) { // might be not created for some MergeId-s.
      // OrigValue = nullptr for live-ins that are not initialized.
      auto *OrigValue = ExtVals.getOriginalIncomingValue(LIV->getMergeId());
      if (OrigValue)
        LIV->replaceAllUsesWith(OrigValue);
    }
  }
}

template <typename InOutListTy>
void VPLiveInOutCreator::createLiveInsForScalarVPlan(
    const InOutListTy &ScalarInOuts, int Count) {

  Plan.allocateLiveInValues(Count);
  for (auto Item : ScalarInOuts.list()) {
    int MergeId = Item->getId();
    VPLiveInValue *LIV = createLiveInValue(MergeId, Item->getValueType());
    Plan.setLiveInValue(LIV, MergeId);
  }
}

template void VPLiveInOutCreator::createLiveInsForScalarVPlan<ScalarInOutList>(
    const ScalarInOutList &, int);
template void
VPLiveInOutCreator::createLiveInsForScalarVPlan<ScalarInOutListHIR>(
    const ScalarInOutListHIR &, int);

template <typename InOutListTy>
void VPLiveInOutCreator::createLiveOutsForScalarVPlan(
    const InOutListTy &ScalarInOuts, int Count,
    DenseMap<int, VPValue *> &Outgoing) {

  Plan.allocateLiveOutValues(Count);
  for (auto Item : ScalarInOuts.list()) {
    int MergeId = Item->getId();
    VPValue *Val = Outgoing[MergeId];
    assert(Val && "Expected non-null outgoing value");
    VPLiveOutValue *LOV = createLiveOutValue(MergeId, Val);
    Plan.setLiveOutValue(LOV, MergeId);
  }
}

template void VPLiveInOutCreator::createLiveOutsForScalarVPlan<ScalarInOutList>(
    const ScalarInOutList &, int, DenseMap<int, VPValue *> &);
template void
VPLiveInOutCreator::createLiveOutsForScalarVPlan<ScalarInOutListHIR>(
    const ScalarInOutListHIR &, int, DenseMap<int, VPValue *> &);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void VPExternalValues::dumpExternalDefs(raw_ostream &FOS) const {
  if (VPExternalDefsHIR.size() == 0)
    return;
  FOS << "External Defs Start:\n";
  for (auto &Def : VPExternalDefsHIR)
    Def.printDetail(FOS);
  FOS << "External Defs End:\n";
}

void VPExternalValues::dumpExternalUses(raw_ostream &FOS,
                                        const VPlan *Plan) const {
  if (VPExternalUses.empty())
    return;
  FOS << "External Uses:\n";
  for (auto &ExtUse : VPExternalUses) {
    const VPLiveOutValue *LO =
        Plan ? Plan->getLiveOutValue(ExtUse->getMergeId()) : nullptr;
    VPValue *Operand = LO ? LO->getOperand(0) : nullptr;
    ExtUse->print(FOS, Operand);
    FOS << "\n";
  }
}

template <class LoopTy>
void VPExternalValues::dumpScalarInOuts(raw_ostream &FOS,
                                        const LoopTy *L) const {
  auto *ScalarLoopInOuts = getScalarLoopInOuts(L);
  if (ScalarLoopInOuts)
     ScalarLoopInOuts->dump(FOS);
}

template void VPExternalValues::dumpScalarInOuts<Loop>(raw_ostream &,
                                                       const Loop *) const;
template void VPExternalValues::dumpScalarInOuts<loopopt::HLLoop>(
    raw_ostream &, const loopopt::HLLoop *) const;

void ScalarInOutDescr::dump(raw_ostream &OS) const {
  OS.indent(2) << "Id: " << MergeId << "\n";
  if (Phi)
    OS.indent(4) << "Phi: " << *Phi;
  else
    OS.indent(4) << "Phi: " << "nullptr";
  OS.indent(4) << "Start op: " << StartValOpNum << "\n";
  OS.indent(4) << "Live-Out: " << *LiveOut;
}

void ScalarInOutDescrHIR::print(raw_ostream &OS) const {
  formatted_raw_ostream FOS(OS);
  FOS.indent(2) << "Id: " << MergeId << "\n";
  FOS.indent(4) << "HIR Temp: ";
  HIRRef->print(FOS);
  FOS.indent(4) << "IsMainLoopIV: " << IsMainLoopIV;
}

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

