//===--- IntelVPlanVLSTransform.cpp ---------------------------------------===//
//
//   Copyright (C) Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanVLSTransform.h"
#include "IntelVPlan.h"
#include "IntelVPlanBuilder.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanVLSAnalysis.h"

using namespace llvm;
using namespace llvm::vpo;

static LoopVPlanDumpControl VLSDumpControl("vls",
                                           "VPlan-to-VPlan VLS transformation");

#define DEBUG_TYPE "vplan-vls-transform"

/// Return the group granularity type (i.e. the one short enough to have all the
/// individual loads/offset expressed as a multiple of it) together with the
/// size of the group in terms of that type.
///
/// E.g. for %struct = type <{ i32, i16, double }>, and the group consisting of
/// i32 and double accesses only (i.e. with a gap), this routine should return
/// {i16, 2+1+4=7}.
static std::pair<Type *, int> getGroupGranularity(OVLSGroup *Group,
                                                  const DataLayout &DL) {
  Type *SomeLoadType = instruction(*Group->begin())->getValueType();
  Type *Result = SomeLoadType;
  for (const VPLoadStoreInst *MemrefInst : instructions(Group)) {
    auto *MemrefType = MemrefInst->getValueType();

    if (DL.getTypeSizeInBits(Result) > DL.getTypeSizeInBits(MemrefType)) {
      Result = MemrefType;
      continue;
    }

    if (DL.getTypeSizeInBits(Result) == DL.getTypeSizeInBits(MemrefType) &&
        MemrefType->isIntegerTy()) {
      // Prefer integer types over fp or pointers.
      Result = MemrefType;
      continue;
    }

    if (DL.getTypeSizeInBits(Result) == DL.getTypeSizeInBits(MemrefType) &&
        isa<VectorType>(Result)) {
      // Vector types need to be casted into wider iN types, so try to avoid
      // them.
      Result = MemrefType;
      continue;
    }
  }

  if (isa<VectorType>(Result))
    Result = IntegerType::getIntNTy(Result->getContext(),
                                    DL.getTypeSizeInBits(Result));

  int GroupSize = DL.getTypeSizeInBits(SomeLoadType) * Group->size() /
                  DL.getTypeSizeInBits(Result);

  return {Result, GroupSize};
}

static std::pair<VPVLSLoad *, unsigned /*Size*/>
createGroupLoad(OVLSGroup *Group, unsigned VF) {
  auto *LeaderMemref = cast<VPVLSClientMemref>(Group->getInsertPoint());
  auto *Leader = const_cast<VPLoadStoreInst *>(instruction(LeaderMemref));
  assert(Leader->getOpcode() == Instruction::Load &&
         "Expected VLS group for loads!");

  auto &Plan = *Leader->getParent()->getParent();
  VPBuilder Builder;
  Builder.setInsertPoint(Leader);
  int LeaderInterleaveIndex = computeInterleaveIndex(LeaderMemref, Group);
  auto *LeaderAddress = Leader->getPointerOperand();
  if (LeaderInterleaveIndex != 0) {
    LeaderAddress =
        Builder.createGEP(LeaderAddress,
                          {Plan.getVPConstant(-APInt(64, LeaderInterleaveIndex,
                                                     true /* Signed */))},
                          nullptr);
    LeaderAddress->setName(
        cast<VPInstruction>(LeaderAddress)->getOperand(0)->getName() +
        ".group.base.offset");
    Plan.getVPlanDA()->updateDivergence(*LeaderAddress);
  }

  // The alignment for the wide load needs to be set using the group's first
  // memory (lowest offset) reference.
  const auto *FirstGroupInst = instruction(Group->getFirstMemref());
  Type *VLSSmallestType;
  unsigned Size;
  std::tie(VLSSmallestType, Size) =
      getGroupGranularity(Group, *Plan.getDataLayout());

  Type *Ty = getWidenedType(VLSSmallestType, VF * llvm::NextPowerOf2(Size - 1));
  auto *WideLoad =
      Builder.create<VPVLSLoad>("vls.load", LeaderAddress, Ty, Size,
                                FirstGroupInst->getAlignment(), Group->size());
  Plan.getVPlanDA()->markUniform(*WideLoad);

  return {WideLoad, Size};
}

bool llvm::vpo::isTransformableVLSGroup(OVLSGroup *Group) {
  if (Group->size() <= 1) {
    LLVM_DEBUG(dbgs() << "Group doesn't contain enough elments (at least 2).";
               Group->dump(); dbgs() << '\n');
    return false;
  }

  Optional<int64_t> GroupStride = Group->getConstStride();
  if (!GroupStride) {
    LLVM_DEBUG(dbgs() << "Failing to transform OVLSGroup: Indexed "
                         "loads/stores are not supported. ";
               Group->dump(); dbgs() << '\n');
    return false;
  }

  auto *LeaderMemref = cast<VPVLSClientMemref>(Group->getInsertPoint());
  auto InterleaveFactor = computeInterleaveFactor(LeaderMemref);
  if (InterleaveFactor == 1) {
    LLVM_DEBUG(
        dbgs() << "No transformation for already unit-strided accesses. ";
        Group->dump(); dbgs() << '\n');
    return false;
  }

  if (*GroupStride > 64) {
    // TODO: Don't skip in LLVM IR case?
    LLVM_DEBUG(dbgs() << "HIR only supports up to 64 bits in mask, skipping. ";
               Group->dump(); dbgs() << '\n');
    return false;
  }

  APInt AccessMask = Group->computeByteAccessMask();
  if (!AccessMask.isAllOnesValue() ||
      AccessMask.getBitWidth() != *GroupStride) {
    LLVM_DEBUG(dbgs() << "Failing to transform OVLSGroup: groups with gaps "
                         "are not supported. ";
               Group->dump(); dbgs() << '\n');
    return false;
  }

  if (!std::equal(
          Group->begin() + 1, Group->end(), Group->begin(),
          [](const OVLSMemref *LHS, const OVLSMemref *RHS) {
            auto *Plan = instruction(LHS)->getParent()->getParent();
            auto *DL = Plan->getDataLayout();

            return DL->getTypeSizeInBits(instruction(LHS)->getValueType()) ==
                   DL->getTypeSizeInBits(instruction(RHS)->getValueType());
          })) {
    LLVM_DEBUG(
        dbgs() << "We don't handle groups with elements of different sizes.";
        Group->dump(); dbgs() << '\n');
    return false;
  }

  return true;
}

// Small example:
//   ; =================== Original VPlan ====================
//   ; <{ i32, i16, i64 }> size 14 bytes, or 7 i16 elements, VF = 4
//   i16 *%gep0 = getelementptr %base, (%iv * 7)
//   i32 *%gep0.cast = bitcast %gep0
//   i16 *%gep1 = getelementptr %gep0, 2
//   i16 *%gep2 = getelementptr %gep0, 3
//   i64 *%gep2.cast = bitcast %gep2
//   i32 %v0 = load %gep0.cast
//   i16 %v1 = load %gep1
//   i64 %v2 = load %gep2.cast
//
//   ; ========= After VLS transformation ====================
//
//   i16 *%gep0 = getlementptr %base, (%iv * 7)
//
//   ; Explicitly marked as uniform/producing "scalar" in DA/SVA
//   <32 x i16> %vls.load = VLSLoad %gep0, group_size=7
//
//   ; Divergent/Vector in DA/SVA. Ptr op is scalar in SVA
//   i32 %v0 = VLSExtract %vls.load, group_size=7, offset=0
//   i16 %v1 = VLSExtract %vls.load, group_size=7, offset=2
//   i64 %v2 = VLSExtract %vls.load, group_size=7, offset=3
void llvm::vpo::applyVLSTransform(VPlan &Plan, VPlanVLSAnalysis &VLSA,
                                  unsigned VF) {
  const DataLayout &DL = *Plan.getDataLayout();
  DenseSet<VPInstruction *> InstsToRemove;
  for (OVLSGroup *Group : VLSA.groups(&Plan)) {
    if (!isTransformableVLSGroup(Group))
      continue;

    auto *LeaderMemref = cast<VPVLSClientMemref>(Group->getInsertPoint());
    auto *Leader = const_cast<VPLoadStoreInst *>(instruction(LeaderMemref));

    if (Group->getAccessKind().isLoad()) {
      assert(Leader->getOpcode() == Instruction::Load &&
             "Expected VLS group for loads!");
      VPVLSLoad *WideLoad;
      unsigned Size;
      std::tie(WideLoad, Size) = createGroupLoad(Group, VF);

      VPBuilder Builder;
      Builder.setInsertPoint(WideLoad->getParent(), ++WideLoad->getIterator());
      for (auto MemrefIter : *Group) {
        auto *Memref = cast<VPVLSClientMemref>(MemrefIter);
        auto *OrigLoad = const_cast<VPLoadStoreInst *>(instruction(Memref));
        assert(OrigLoad->getOpcode() == Instruction::Load &&
               "Expected a load!");
        auto InterleaveIndex = computeInterleaveIndex(Memref, Group);
        auto InterleaveFactor = computeInterleaveFactor(Memref);
        (void)InterleaveFactor;
        assert(InterleaveIndex < InterleaveFactor &&
               "InterleaveIndex must be less than InterleaveFactor");
        assert(InterleaveFactor != 1 &&
               "No transformation for unit-strided loads is expected!");
        // We only support same-size elements.
        auto *Extract = Builder.create<VPVLSExtract>(
            OrigLoad->getName(), WideLoad, OrigLoad->getType(), Size,
            InterleaveIndex * getNumGroupEltsPerValue(DL, WideLoad->getType(),
                                                      OrigLoad->getType()));
        Extract->setDebugLocation(OrigLoad->getDebugLocation());
        OrigLoad->replaceAllUsesWith(Extract);
        InstsToRemove.insert(OrigLoad);
      }
      continue;
    }

    assert(Group->getAccessKind().isStore() && "Unexpected access kind!");
    assert(Leader->getOpcode() == Instruction::Store &&
           "Expected VLS group for stores!");

    Type *VLSSmallestType;
    unsigned Size;
    std::tie(VLSSmallestType, Size) =
        getGroupGranularity(Group, *Plan.getDataLayout());

    // Only same size elements are supported currently.
    Type *GroupTy =
        getWidenedType(VLSSmallestType, VF * llvm::NextPowerOf2(Size - 1));

    VPBuilder Builder;
    Builder.setInsertPoint(Leader);
    VPValue *WideValue = Plan.getUndef(GroupTy);

    for (OVLSMemref *Memref : *Group) {
      auto InterleaveIndex = computeInterleaveIndex(Memref, Group);
      auto *Store = const_cast<VPLoadStoreInst *>(instruction(Memref));
      InstsToRemove.insert(Store);
      VPValue *V = Store->getOperand(0);
      WideValue = Builder.create<VPVLSInsert>(
          "vls.insert", WideValue, V, Size,
          InterleaveIndex * getNumGroupEltsPerValue(DL, GroupTy, V->getType()));
      Plan.getVPlanDA()->markUniform(*WideValue);
    }

    auto *FirstMemrefInst =
        const_cast<VPLoadStoreInst *>(instruction(Group->getFirstMemref()));
    auto *BaseAddr = FirstMemrefInst->getOperand(1);
    Builder.create<VPVLSStore>("vls.store", WideValue, BaseAddr, Size,
                               FirstMemrefInst->getAlignment(), Group->size());
  }

  while (!InstsToRemove.empty()) {
    auto *Inst = *InstsToRemove.begin();
    InstsToRemove.erase(Inst);
    SmallVector<VPValue *, 8> Operands(Inst->operands());
    Inst->getParent()->eraseInstruction(Inst);

    for (auto *Op : Operands)
      if (auto *OpInst = dyn_cast<VPInstruction>(Op))
        if (OpInst->getNumUsers() == 0)
          InstsToRemove.insert(OpInst);
  }

  VPLAN_DUMP(VLSDumpControl, Plan);
}
