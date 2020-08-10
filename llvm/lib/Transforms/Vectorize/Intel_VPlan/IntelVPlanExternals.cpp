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
  SmallSet<unsigned, 16> SymbaseSet;
  SmallSet<unsigned, 16> IVLevelSet;
  for (const auto &ExtDef : VPExternalDefsHIR) {
    const VPOperandHIR *HIROperand = ExtDef.getOperandHIR();

    // Deeper verification depending on the kind of the underlying HIR operand.
    if (const auto *Blob = dyn_cast<VPBlob>(HIROperand)) {
      // For blobs we check that the symbases are unique.
      unsigned Symbase = Blob->getBlob()->getSymbase();
      assert(!SymbaseSet.count(Symbase) && "Repeated blob VPExternalDef!");
      SymbaseSet.insert(Symbase);
    } else if (isa<VPCanonExpr>(HIROperand)) {
      assert(
          llvm::count_if(VPExternalDefsHIR,
                         [HIROperand](const VPExternalDef &ExtDef) {
                           return ExtDef.getOperandHIR()->isStructurallyEqual(
                               HIROperand);
                         }) == 1 &&
          "Repeated CanonExpr VPExternalDef!");
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
      if (Final = dyn_cast<FinalTy>(Val))
        FinalExternalUse = findExtUser(Final);
  }
  return std::make_tuple(Init, Final, FinalExternalUse);
}

void VPLiveInOutCreator::createInOutsInductions(
    const VPLoopEntityList *VPLEntityList) {

  VPExternalValues &ExtVals = Plan.getExternals();

  for (auto *Ind : VPLEntityList->vpinductions()) {
    if (Ind->getIsMemOnly())
      continue;
    VPInductionInit *IndInit;
    VPInductionFinal *IndFinal;
    VPExternalUse *IndFinalExternalUse;
    std::tie(IndInit, IndFinal, IndFinalExternalUse) =
        getInitFinal<VPInductionInit, VPInductionFinal, VPInduction>(Ind);
    assert((IndInit && IndFinal) && "Expected non-null init, final");
    bool NeedAddExtUse = IndFinalExternalUse == nullptr;
    // Inductions should always have outgoing value
    if (NeedAddExtUse) {
      IndFinalExternalUse = ExtVals.createVPExternalUseNoIR(IndInit->getType());
      IndFinalExternalUse->addOperand(IndFinal);
    }

    // Create live-in/out descriptors
    unsigned MergeId = IndFinalExternalUse->getMergeId();
    VPValue *StartV = IndInit->getStartValueOperand();
    VPLiveInValue *LIV = createLiveInValue(MergeId, StartV->getType());
    VPLiveOutValue *LOV = createLiveOutValue(MergeId, IndFinal);
    // Unlink External use
    IndFinalExternalUse->removeOperand(
        IndFinalExternalUse->getOperandIndex(IndFinal));
    // Put live-in/out to their lists
    if (NeedAddExtUse) {
      Plan.addLiveInValue(LIV);
      Plan.addLiveOutValue(LOV);
      ExtVals.addOriginalIncomingValue(StartV);
    } else {
      Plan.setLiveInValue(LIV, MergeId);
      Plan.setLiveOutValue(LOV, MergeId);
      ExtVals.setOriginalIncomingValue(StartV, MergeId);
    }
    // Replace start value with live-in
    IndInit->replaceStartValue(LIV);
    if (IndFinal->usesStartValue())
      IndFinal->replaceStartValue(LIV);
  }
}

void VPLiveInOutCreator::createInOutsReductions(
  const  VPLoopEntityList *VPLEntityList) {

  VPExternalValues &ExtVals = Plan.getExternals();

  for (auto *Red : VPLEntityList->vpreductions()) {
    if (Red->getIsMemOnly())
      continue;
    VPReductionInit *RedInit;
    VPReductionFinal *RedFinal;
    VPExternalUse *RedFinalExternalUse;
    std::tie(RedInit, RedFinal, RedFinalExternalUse) =
        getInitFinal<VPReductionInit, VPReductionFinal, VPReduction>(Red);
    if (!RedFinalExternalUse)
      // Can be possible for some auto-generated reductions, e.g. fake linear
      // index for min/max+index idiom.
      continue;
    assert((RedInit && RedFinal) && "Expected non-null init and final");
    // Create live-in/out descriptors
    int MergeId = RedFinalExternalUse->getMergeId();
    VPValue *StartV = nullptr;
    if (RedInit->usesStartValue())
      StartV = RedInit->getStartValueOperand();
    else
      StartV = RedFinal->getStartValueOperand();
    VPLiveInValue *LIV = createLiveInValue(MergeId, StartV->getType());
    VPLiveOutValue *LOV = createLiveOutValue(MergeId, RedFinal);
    // Unlink External use
    RedFinalExternalUse->removeOperand(
        RedFinalExternalUse->getOperandIndex(RedFinal));
    // Put live-in/out to their lists
    Plan.setLiveInValue(LIV, MergeId);
    Plan.setLiveOutValue(LOV, MergeId);
    ExtVals.setOriginalIncomingValue(StartV, MergeId);
    // Replace start value with live-in
    if (RedInit->usesStartValue())
      RedInit->replaceStartValue(LIV);
    if (RedFinal->usesStartValue())
      RedFinal->replaceStartValue(LIV);
  }
}

void VPLiveInOutCreator::createInOutValues() {
  const VPLoop *Loop = *Plan.getVPLoopInfo()->begin();
  if (!Loop->getUniqueExitBlock())
    return;

  VPExternalValues &ExtVals = Plan.getExternals();
  unsigned ExtUseCount = ExtVals.getLastMergeId();

  const VPLoopEntityList *VPLEntityList =
      Plan.getLoopEntities(Loop);

  // We need to allocate LiveIn/LiveOut lists here, along with
  // OriginalIncomingValues, due to we look through entities and the lookup is
  // not in the order of VPExternalUses creation but we need to place the items
  // in the needed order.
  ExtVals.allocateOriginalIncomingValues(ExtUseCount);
  Plan.allocateLiveInValues(ExtUseCount);
  Plan.allocateLiveOutValues(ExtUseCount);

  createInOutsInductions(VPLEntityList);
  createInOutsReductions(VPLEntityList);
}

void VPLiveInOutCreator::restoreLiveIns() {
  VPExternalValues &ExtVals = Plan.getExternals();
  for (VPLiveInValue *LIV : Plan.liveInValues())
    if (LIV) // Might be not created for some MergeId-s.
      LIV->replaceAllUsesWith(
          ExtVals.getOriginalIncomingValue(LIV->getMergeId()));
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
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

