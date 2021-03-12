//===- IntelVPlanExternals.cpp - VPlan externals storage   ----------------===//
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

using namespace llvm;
using namespace llvm::vpo;

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
    if (isa<VPBlob>(HIROperand) || isa<VPCanonExpr>(HIROperand)) {
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
static std::tuple<InitTy *, FinalTy *, VPExternalUse *>
getInitFinal(EntityTy *E) {
  InitTy *Init = nullptr;
  FinalTy *Final = nullptr;
  VPExternalUse *FinalExternalUse = nullptr;

  auto findExtUser = [](VPInstruction *I) -> VPExternalUse * {
    auto Iter = llvm::find_if(I->users(),
                              [](VPUser *U) { return isa<VPExternalUse>(U); });
    if (Iter == I->user_end())
      return nullptr;
    else
      return cast<VPExternalUse>(*Iter);
  };

  for (VPValue *Val : E->getLinkedVPValues()) {
    if (!Init)
      Init = dyn_cast<InitTy>(Val);
    if (!Final)
      if ((Final = dyn_cast<FinalTy>(Val)))
        FinalExternalUse = findExtUser(Final);
  }
  return std::make_tuple(Init, Final, FinalExternalUse);
}

template <class InitTy, class FinalTy>
void VPLiveInOutCreator::addInOutValues(InitTy *Init, FinalTy *Final,
                                        VPExternalUse *ExtUse, bool ExtUseAdded,
                                        VPValue *StartV) {
  VPExternalValues &ExtVals = Plan.getExternals();

  // Create live-in/out descriptors
  int MergeId = ExtUse->getMergeId();
  VPLiveInValue *LIV = createLiveInValue(MergeId, StartV->getType());
  VPLiveOutValue *LOV = createLiveOutValue(MergeId, Final);
  // Unlink External use
  ExtUse->removeOperand(ExtUse->getOperandIndex(Final));
  // Put live-in/out to their lists
  if (ExtUseAdded) {
    Plan.addLiveInValue(LIV);
    Plan.addLiveOutValue(LOV);
    ExtVals.addOriginalIncomingValue(StartV);
  } else {
    Plan.setLiveInValue(LIV, MergeId);
    Plan.setLiveOutValue(LOV, MergeId);
    ExtVals.setOriginalIncomingValue(StartV, MergeId);
  }
  // Replace start value with live-in
  if (Init->usesStartValue())
    Init->replaceStartValue(LIV);
  if (Final->usesStartValue())
    Final->replaceStartValue(LIV);
}

void VPLiveInOutCreator::addOriginalLiveInOut(
    const VPLoopEntityList *VPLEntityList, Loop *OrigLoop, VPLoopEntity *E,
    VPExternalUse *ExtUse, ScalarInOutList &ScalarInOuts) {
  if (!OrigLoop)
    // TODO: implement for HIR input
    return;
  VPPHINode *VPPhi = VPLEntityList->getRecurrentVPHINode(*E);
  assert(VPPhi && "Unexpected null recurrent VPPhi");
  PHINode *Phi = cast<PHINode>(VPPhi->getUnderlyingValue());
  assert(Phi->getParent() == OrigLoop->getHeader() &&
         "Unexpected recurrent phi");
  int StartValOpNum = Phi->getBasicBlockIndex(OrigLoop->getLoopPreheader());
  const Value *LiveOut = nullptr;
  if (ExtUse->hasUnderlying())
    // If the external use exists and it has underlying value, we use it to
    // set scalar live out. External use or its underlying value may not
    // exist, e.g. for non-liveout inductions.
    LiveOut = ExtUse->getUnderlyingOperand(0);
  else {
    // Otherwise use phi node incoming operand that comes from latch.
    assert(Phi->getNumOperands() == 2 && "Unexpected recurrent Phi");
    LiveOut = Phi->getOperand(StartValOpNum ^ 1);
  }
  ScalarInOuts.add(Phi, StartValOpNum, LiveOut, ExtUse->getMergeId());
}

void VPLiveInOutCreator::createInOutsInductions(
    const VPLoopEntityList *VPLEntityList, Loop *OrigLoop) {

  VPExternalValues &ExtVals = Plan.getExternals();
  ScalarInOutList &ScalarInOuts = *ExtVals.getOrCreateScalarLoopInOuts(OrigLoop);

  for (auto *Ind : VPLEntityList->vpinductions()) {
    if (Ind->getIsMemOnly())
      continue;
    VPInductionInit *IndInit = nullptr;
    VPInductionFinal *IndFinal = nullptr;
    VPExternalUse *IndFinalExternalUse = nullptr;
    std::tie(IndInit, IndFinal, IndFinalExternalUse) =
        getInitFinal<VPInductionInit, VPInductionFinal, VPInduction>(Ind);
    assert((IndInit && IndFinal) && "Expected non-null init, final");
    bool NeedAddExtUse = IndFinalExternalUse == nullptr;
    // Inductions should always have outgoing value
    if (NeedAddExtUse) {
      IndFinalExternalUse = ExtVals.createVPExternalUseNoIR(IndInit->getType());
      IndFinalExternalUse->addOperand(IndFinal);
    }
    addInOutValues(IndInit, IndFinal, IndFinalExternalUse, NeedAddExtUse,
                   IndInit->getStartValueOperand());
    addOriginalLiveInOut(VPLEntityList, OrigLoop, Ind, IndFinalExternalUse,
                         ScalarInOuts);
  }
}

void VPLiveInOutCreator::createInOutsReductions(
    const VPLoopEntityList *VPLEntityList, Loop *OrigLoop) {

  VPExternalValues &ExtVals = Plan.getExternals();
  ScalarInOutList &ScalarInOuts = *ExtVals.getOrCreateScalarLoopInOuts(OrigLoop);

  for (auto *Red : VPLEntityList->vpreductions()) {
    VPReductionInit *RedInit = nullptr;
    VPReductionFinal *RedFinal = nullptr;
    VPExternalUse *RedFinalExternalUse = nullptr;
    std::tie(RedInit, RedFinal, RedFinalExternalUse) =
        getInitFinal<VPReductionInit, VPReductionFinal, VPReduction>(Red);
    assert((RedInit && RedFinal) && "Expected non-null init and final");
    VPValue *StartV = nullptr;
    if (RedInit->usesStartValue())
      StartV = RedInit->getStartValueOperand();
    else
      StartV = RedFinal->getStartValueOperand();
    assert(StartV && "StartV is expected to be non-null here.");
    bool ExtUseAdded = false;
    if (!RedFinalExternalUse) {
      // No external use can be possible for some auto-generated reductions,
      // e.g. fake linear index for min/max+index idiom or for reductions that
      // are not used outside of the loop or in-memory reductions. Then we
      // create a fake external use, with symbase equal to symbase of start
      // value.
      // TODO: We don't need to create an external use in cases when it's a fake
      // linear index. In other cases we need to keep it's value after peel/main
      // loop.
      if (auto ExtDef = dyn_cast<VPExternalDef>(StartV))
        if (auto VBlob = dyn_cast_or_null<VPBlob>(ExtDef->getOperandHIR())) {
          RedFinalExternalUse =
              ExtVals.getOrCreateVPExternalUseForDDRef(VBlob->getBlob());
          RedFinalExternalUse->addOperand(RedFinal);
        }
      if (!RedFinalExternalUse) {
        // That means we have a constant start value.
        RedFinalExternalUse =
            ExtVals.createVPExternalUseNoIR(RedInit->getType());
        RedFinalExternalUse->addOperand(RedFinal);
      }
      ExtUseAdded = true;
    }
    addInOutValues(RedInit, RedFinal, RedFinalExternalUse, ExtUseAdded, StartV);
    addOriginalLiveInOut(VPLEntityList, OrigLoop, Red, RedFinalExternalUse,
                         ScalarInOuts);
  }
}

void VPLiveInOutCreator::createInOutValues(Loop *OrigLoop) {
  VPlanVector &VecPlan = cast<VPlanVector>(Plan);
  const VPLoop *Loop = *VecPlan.getVPLoopInfo()->begin();
  if (!Loop->getUniqueExitBlock())
    return;

  VPExternalValues &ExtVals = Plan.getExternals();
  unsigned ExtUseCount = ExtVals.getLastMergeId();

  const VPLoopEntityList *VPLEntityList = VecPlan.getLoopEntities(Loop);
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
}

void VPLiveInOutCreator::restoreLiveIns() {
  VPExternalValues &ExtVals = Plan.getExternals();
  for (VPLiveInValue *LIV : Plan.liveInValues())
    if (LIV) // Might be not created for some MergeId-s.
      LIV->replaceAllUsesWith(
          ExtVals.getOriginalIncomingValue(LIV->getMergeId()));
}

void VPLiveInOutCreator::createLiveInsForScalarVPlan(
    const ScalarInOutList &ScalarInOuts, int Count) {

  Plan.allocateLiveInValues(Count);
  for (auto Item : ScalarInOuts.list()) {
    int MergeId = Item->getId();
    VPLiveInValue *LIV = createLiveInValue(MergeId, Item->getPhi()->getType());
    Plan.setLiveInValue(LIV, MergeId);
  }
}

void VPLiveInOutCreator::createLiveOutsForScalarVPlan(
    const ScalarInOutList &ScalarInOuts, int Count,
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

void VPExternalValues::dumpScalarInOuts(raw_ostream &FOS, const Loop *L) const {
  const ScalarInOutList* ScalarLoopInOuts = getScalarLoopInOuts(L);
  if (ScalarLoopInOuts)
     ScalarLoopInOuts->dump(FOS);
}

void ScalarInOutDescr::dump(raw_ostream &OS) const {
  OS.indent(2) << "Id: " << MergeId << "\n";
  OS.indent(4) << "Phi: " << *Phi;
  OS.indent(4) << "Start op: " << StartValOpNum << "\n";
  OS.indent(4) << "Live-Out: " << *LiveOut;
}

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

