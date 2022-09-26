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
#include "llvm/ADT/STLExtras.h"

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vplan-vls-transform"

static LoopVPlanDumpControl VLSDumpControl("vls",
                                           "VPlan-to-VPlan VLS transformation");
namespace {
class VLSTransform {
public:
  VLSTransform(OVLSGroup *Group, VPlanVector &Plan, unsigned VF);

  const char *getFailureReason() const { return FailureReason; }

  /// Make the transformation for the OVLSGroup used to create an instance of
  /// this transformer.
  void run(DenseSet<VPInstruction *> &InstsToRemove);

private:
  // Setup metadata and HIR symbases for the newly created VLSLoad/VLSStore.
  template <class VLSMemoryOpTy>
  void setMemOpProperties(VLSMemoryOpTy *VLSMemoryOp);

  /// Return the group granularity type (i.e. the one short enough to have all
  /// the individual loads/offset expressed as a multiple of it) together with
  /// the size of the group in terms of that type.
  ///
  /// E.g. for %struct = type <{ i32, i16, double }>, and the group consisting
  /// of i32 and double accesses only (i.e. with a gap), this routine should
  /// return {i16, 2+1+4=7}.
  ///
  /// However, current VLSAnalysis implementation isn't able to support such
  /// generic cases and the transformation code exploits some of those
  /// limitation. As such, the transformation itself (caller of this routine)
  /// has it's own bailout. Note that some parts of the code are written in a
  /// way that would work when such limitation are lifted yet others still rely
  /// on it.
  std::pair<Type *, int> getGroupGranularity();

  /// E.g. for {i16, i16, i32} with VF=2. Group type would be <8 x i16> and
  /// ExtractInsert element type for the i32 element would be <2 x i16>.
  Type *getExtractInsertEltType(Type *EltType);

  /// Return the position of the \p Memref as used in the operand for the
  /// VLSInsert/VLSExtract instruction.
  unsigned getExtractInsertEltOffset(OVLSMemref *Memref);

  void processLoadGroup(DenseSet<VPInstruction *> &InstsToRemove);
  void processStoreGroup(DenseSet<VPInstruction *> &InstsToRemove);

  /// Helper function to handle different casts from VLSInsert/VLSExtract to the
  /// desired value type of the original memory operation. We lower it into
  /// explicit casts as part of the transformation to simplify CGs as the
  /// lowering isn't trivial. For example, if the group consists of floating
  /// point values and pointers we can't cast fp-value directly to the pointer
  /// and have to go through intermediate integer bitcast.
  VPValue *createCast(VPBuilder &Builder, VPValue *From, Type *ToTy);

  /// Utilities to perform the adjustments (reversal) for the case when the
  /// group has negative stride. Basically, for
  /// {i32 ld0, i32 ld1, i32 ld2} the "group" value after/before
  /// VLSLoad/VLSStore is (VF == 2):
  ///
  /// Positive stride:
  ///     (ld0, ld1, ld2), (ld0, ld1, ld2), (mask-out, mask-out)
  ///     |   lane0            lane1
  ///    AdjustedBase == Base
  ///
  /// Negative stride:
  ///     (ld0, ld1, ld2), (ld 0, ld1, ld2), (mask-out, mask-out)
  ///     |   lane1        |   lane0
  ///    AdjustedBase      Base
  ///
  /// This routine converts one order into another and is used after/before the
  /// load/store so that the "shape" of the value is "positive" during compute
  /// and "negative" during the memory operation itself.
  ///
  /// These routines are no-op if the stride is positive.
  VPValue *adjustBasePtrForReverse(VPValue *Base, VPBuilder &Builder);
  VPValue *adjustGroupValForReverse(VPBuilder &Builder, VPValue *GroupVal);

private:
  OVLSGroup *Group;
  VPlanVector &Plan;
  const DataLayout &DL;
  VPlanDivergenceAnalysis &DA;
  unsigned VF;

  const char *FailureReason = nullptr;

  // Rest of the state is inaccessible if the group itself isn't transformable.
  Optional<int64_t> GroupStride; // In bytes.
  /// Group-wide memop should be performed at that position.
  VPVLSClientMemref *InsertPointMemref;
  /// VPInstruction associated with the insert point above.
  VPLoadStoreInst *InsertPointInst;
  /// Memref with lowest offset. We rely on the fact that the group actually
  /// starts with it (i.e. no gap in the beginning of the group).
  VPLoadStoreInst *FirstMemrefInst;
  /// SIMD-wide load type is
  ///    <GroupSizeInGranularityElements x GroupGranularityType>
  /// extended to the next power of two. See getGroupGranularity for the
  /// detailed documentation.
  Type *GroupGranularityType;
  unsigned GroupSizeInGranularityElements;
  // Includes the spacing at the end for non-power-of two sizes.
  FixedVectorType *GroupTy;
};
} // namespace

VLSTransform::VLSTransform(OVLSGroup *Group, VPlanVector &Plan, unsigned VF)
    : Group(Group), Plan(Plan), DL(*Plan.getDataLayout()),
      DA(*Plan.getVPlanDA()), VF(VF) {
  if (Group->size() <= 1) {
    FailureReason = "Group doesn't contain enough elments (at least 2).";
    return;
  }

  GroupStride = Group->getConstStride();
  if (!GroupStride) {
    FailureReason = "Failing to transform OVLSGroup: Indexed loads/stores are "
                    "not supported.";
    return;
  }
  for (OVLSMemref *Memref: *Group) {
    auto ElementSizeInBits = Memref->getType().getElementSize();
    if (*GroupStride % (ElementSizeInBits / 8)) {
      FailureReason = "Stride not a multiple of element size, skipping.";
      return;
    }
  }
  if (std::abs(*GroupStride) > 64) {
    // TODO: Don't skip in LLVM IR case?
    FailureReason = "HIR only supports up to 64 bits in mask, skipping.";
    return;
  }

  InsertPointMemref = cast<VPVLSClientMemref>(Group->getInsertPoint());
  InsertPointInst =
      const_cast<VPLoadStoreInst *>(instruction(InsertPointMemref));
  if (computeInterleaveFactor(InsertPointMemref) == 1) {
    // Not sure if possible at all. Maybe two identical unit-stride loads might
    // theoretically result in that?
    FailureReason = "Leader is unit-strided.";
    return;
  }

  FirstMemrefInst =
      const_cast<VPLoadStoreInst *>(instruction(Group->getFirstMemref()));

  APInt AccessMask = Group->computeByteAccessMask();
  if (!AccessMask.isAllOnesValue() ||
      AccessMask.getBitWidth() != std::abs(*GroupStride)) {
    FailureReason =
        "Failing to transform OVLSGroup: groups with gaps are not supported.";
    return;
  }

  if (!std::equal(
          Group->begin() + 1, Group->end(), Group->begin(),
          [this](const OVLSMemref *LHS, const OVLSMemref *RHS) {
            return DL.getTypeSizeInBits(instruction(LHS)->getValueType()) ==
                   DL.getTypeSizeInBits(instruction(RHS)->getValueType());
          })) {
    FailureReason = "We don't handle groups with elements of different sizes.";
    return;
  }

  if (any_of(instructions(Group), [](const VPLoadStoreInst *Memref) {
        return Memref->getValueType()->isAggregateType();
      })) {
    // While we could load the data via scalar type (like iN or <M x iN> after
    // widening), we'd need to cast the data back to vector of structs which
    // doesn't exist. As such, support for aggregate types in VLS would require
    // a mix of widening and serialization which isn't implemented yet (and it's
    // unclear how beneficial it would be).
    //
    // Also, in many cases aggregate accesses are split into individual scalar
    // type loads/stores, so the issue might be not that common.
    FailureReason = "Aggregate type in the group.";
    return;
  }

  // Initialize whole group specific properties.
  std::tie(GroupGranularityType, GroupSizeInGranularityElements) =
      getGroupGranularity();
  GroupTy = cast<FixedVectorType>(getWidenedType(
      GroupGranularityType,
      VF * llvm::NextPowerOf2(GroupSizeInGranularityElements - 1)));
}

template <class VLSMemoryOpTy>
void VLSTransform::setMemOpProperties(VLSMemoryOpTy *VLSMemoryOp) {
  static_assert(std::is_same<VLSMemoryOpTy, VPVLSLoad>::value ||
                    std::is_same<VLSMemoryOpTy, VPVLSStore>::value,
                "Unexpected type!");
  unsigned PreservedMDKinds[] = {
      LLVMContext::MD_tbaa,        LLVMContext::MD_alias_scope,
      LLVMContext::MD_noalias,     LLVMContext::MD_fpmath,
      LLVMContext::MD_nontemporal, LLVMContext::MD_invariant_load};
  for (unsigned Kind : PreservedMDKinds) {
    auto Range = instructions(Group);
    auto It = Range.begin();
    auto End = Range.end();
    MDNode *ResultMD = (*(It++))->getMetadata(Kind);
    for (const VPLoadStoreInst *Inst : make_range(It, End)) {
      MDNode *MD = Inst->getMetadata(Kind);
      switch (Kind) {
      case LLVMContext::MD_tbaa:
        ResultMD = MDNode::getMostGenericTBAA(ResultMD, MD);
        break;
      case LLVMContext::MD_alias_scope:
        ResultMD = MDNode::getMostGenericAliasScope(ResultMD, MD);
        break;
      case LLVMContext::MD_fpmath:
        ResultMD = MDNode::getMostGenericFPMath(ResultMD, MD);
        break;
      case LLVMContext::MD_noalias:
      case LLVMContext::MD_nontemporal:
      case LLVMContext::MD_invariant_load:
        ResultMD = MDNode::intersect(ResultMD, MD);
        break;
      default:
        llvm_unreachable("unhandled metadata");
      }
    }
    VLSMemoryOp->setMetadata(Kind, ResultMD);
  }

  VLSMemoryOp->HIR().setSymbase(FirstMemrefInst->HIR().getSymbase());
  for (const VPLoadStoreInst *Inst : instructions(Group))
    VLSMemoryOp->HIR().addFakeSymbase(Inst->HIR().getSymbase());
}

std::pair<Type *, int> VLSTransform::getGroupGranularity() {
  Type *SomeLoadType = instruction(*Group->begin())->getValueType();
  Type *Result = SomeLoadType;
  for (const VPLoadStoreInst *MemrefInst : instructions(Group)) {
    auto *MemrefType = MemrefInst->getValueType();
    assert(DL.getTypeSizeInBits(Result) == DL.getTypeSizeInBits(MemrefType) &&
           "Generic support isn't fully implemented yet (GroupSize).");
    // TODO: Support for gaps, including gap size less than any element

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

  assert(!Result->isAggregateType() &&
         "Aggregate types should be filtered out in legality checks!");

  if (isa<VectorType>(Result))
    Result = IntegerType::getIntNTy(Result->getContext(),
                                    DL.getTypeSizeInBits(Result));

  // AddrSpaceCast can be a no-op cast or a complex value modification,
  // depending on the target and the address space pair.
  //
  // We do require a no-op casting so have to go through non-pointer type. Might
  // as well require it to be the group granularity type here.
  if (auto *PtrTy = dyn_cast<PointerType>(Result))
    if (any_of(instructions(Group), [AS = PtrTy->getAddressSpace()](
                                        const VPLoadStoreInst *MemrefInst) {
          auto *Ty = dyn_cast<PointerType>(MemrefInst->getValueType());
          return Ty && Ty->getAddressSpace() != AS;
        })) {
      Result = IntegerType::getIntNTy(Result->getContext(),
                                      DL.getTypeSizeInBits(Result));
    }

  int GroupSize = DL.getTypeSizeInBits(SomeLoadType) * Group->size() /
                  DL.getTypeSizeInBits(Result);

  return {Result, GroupSize};
}

VPValue *VLSTransform::createCast(VPBuilder &Builder, VPValue *From,
                                  Type *ToTy) {
  auto *FromTy = From->getType();
  if (FromTy == ToTy)
    return From;

  auto IsFromPtr = isa<PointerType>(FromTy);
  auto IsToPtr = isa<PointerType>(ToTy);

  if (!IsFromPtr && !IsToPtr)
    return Builder.createNaryOp(Instruction::BitCast, ToTy, {From});

  if (IsFromPtr && IsToPtr) {
    assert(cast<PointerType>(FromTy)->getAddressSpace() ==
               cast<PointerType>(ToTy)->getAddressSpace() &&
           "Groups consisting of pointers to different addrspaces should be "
           "loaded/stored as non-pointer data!");
    return Builder.createNaryOp(Instruction::BitCast, ToTy, {From});
  }

  if (IsFromPtr) {
    if (isa<IntegerType>(ToTy))
      return Builder.createNaryOp(Instruction::PtrToInt, ToTy, {From});

    Type *IntermediateType =
        Type::getIntNTy(ToTy->getContext(), ToTy->getPrimitiveSizeInBits());
    return Builder.createNaryOp(
        Instruction::BitCast, ToTy,
        {Builder.createNaryOp(Instruction::PtrToInt, IntermediateType,
                              {From})});
  }

  if (isa<IntegerType>(FromTy))
    return Builder.createNaryOp(Instruction::IntToPtr, ToTy, {From});

  Type *IntermediateType =
      Type::getIntNTy(FromTy->getContext(), FromTy->getPrimitiveSizeInBits());
  return Builder.createNaryOp(
      Instruction::IntToPtr, ToTy,
      {Builder.createNaryOp(Instruction::BitCast, IntermediateType, {From})});
}

Type *VLSTransform::getExtractInsertEltType(Type *EltType) {
  auto *GroupEltTy = GroupTy->getElementType();
  auto NumGroupEltsPerValue = getNumGroupEltsPerValue(DL, GroupTy, EltType);
  if (NumGroupEltsPerValue == 1)
    return GroupEltTy;

  return FixedVectorType::get(GroupEltTy, NumGroupEltsPerValue);
}

unsigned VLSTransform::getExtractInsertEltOffset(OVLSMemref *Memref) {
  auto InterleaveIndex = computeInterleaveIndex(Memref, Group);
  auto InterleaveFactor = computeInterleaveFactor(Memref);
  (void)InterleaveFactor;
  assert(InterleaveIndex < std::abs(InterleaveFactor) &&
         "InterleaveIndex must be less than InterleaveFactor");
  assert(InterleaveFactor != 1 &&
         "No transformation for unit-strided accesses is expected!");
  return InterleaveIndex *
         getNumGroupEltsPerValue(DL, GroupTy,
                                 instruction(Memref)->getValueType());
}

void VLSTransform::processLoadGroup(DenseSet<VPInstruction *> &InstsToRemove) {
  assert(InsertPointInst->getOpcode() == Instruction::Load &&
         "Expected VLS group for loads!");

  VPBuilder Builder;
  Builder.setInsertPoint(InsertPointInst);

  // That interface basically means we can't support generic different-size
  // elements inside the group. E.g. { i16, i32, i16 } wouldn't be supported
  // because interleave index for i32 would be 0.5.
  int LeaderInterleaveIndex = computeInterleaveIndex(InsertPointMemref, Group);
  auto *LeaderAddress = InsertPointInst->getPointerOperand();
  if (LeaderInterleaveIndex != 0) {
    // Insert point for loads happens at the lexically first load which might
    // not be the first memref in the group (sorted by adddress). As such, we
    // can't simply use the address of the first memref as its def might be
    // unavailable here.
    LeaderAddress =
        Builder.createGEP(InsertPointInst->getValueType(),
                          InsertPointInst->getValueType(), LeaderAddress,
                          {Plan.getVPConstant(-APInt(64, LeaderInterleaveIndex,
                                                     true /* Signed */))},
                          nullptr);
    LeaderAddress->setName(
        cast<VPInstruction>(LeaderAddress)->getOperand(0)->getName() +
        ".group.base.offset");
    DA.updateDivergence(*LeaderAddress);
  }

  LeaderAddress = adjustBasePtrForReverse(LeaderAddress, Builder);

  // The alignment for the wide load needs to be set using the group's first
  // memory (lowest offset) reference. Note that it is true because we don't
  // have a gap at the group start (or if we would start the load from the
  // address of non-gap element in case of gap presence).
  auto *WideLoad = Builder.create<VPVLSLoad>(
      "vls.load", LeaderAddress, GroupTy, GroupSizeInGranularityElements,
      FirstMemrefInst->getAlignment(), Group->size());
  DA.markUniform(*WideLoad);
  setMemOpProperties(WideLoad);

  auto *ReverseAdjusted = adjustGroupValForReverse(Builder, WideLoad);

  for (OVLSMemref *Memref : *Group) {
    auto *OrigLoad = const_cast<VPLoadStoreInst *>(instruction(Memref));
    auto ExtractTy = getExtractInsertEltType(OrigLoad->getType());
    auto *Extract = Builder.create<VPVLSExtract>(
        OrigLoad->getName(), ReverseAdjusted, ExtractTy,
        GroupSizeInGranularityElements, getExtractInsertEltOffset(Memref));
    auto *ExtractCast =
        cast<VPInstruction>(createCast(Builder, Extract, OrigLoad->getType()));
    ExtractCast->setDebugLocation(OrigLoad->getDebugLocation());
    OrigLoad->replaceAllUsesWith(ExtractCast);
    InstsToRemove.insert(OrigLoad);
  }
}

void VLSTransform::processStoreGroup(DenseSet<VPInstruction *> &InstsToRemove) {
  assert(InsertPointInst->getOpcode() == Instruction::Store &&
         "Expected VLS group for stores!");

  VPBuilder Builder;
  Builder.setInsertPoint(InsertPointInst);
  VPValue *WideValue = Plan.getUndef(GroupTy);

  for (OVLSMemref *Memref : *Group) {
    auto *Store = const_cast<VPLoadStoreInst *>(instruction(Memref));
    InstsToRemove.insert(Store);
    VPValue *V = Store->getOperand(0);
    Type *InsertTy = getExtractInsertEltType(V->getType());
    auto *Casted = createCast(Builder, V, InsertTy);
    WideValue = Builder.create<VPVLSInsert>("vls.insert", WideValue, Casted,
                                            GroupSizeInGranularityElements,
                                            getExtractInsertEltOffset(Memref));
    DA.markUniform(*WideValue);
  }

  WideValue = adjustGroupValForReverse(Builder, WideValue);

  // Insert point for stores is the last store, so the def for the address of
  // the first store is known to be available.
  auto *BaseAddr = FirstMemrefInst->getOperand(1);
  BaseAddr = adjustBasePtrForReverse(BaseAddr, Builder);
  auto *WideStore = Builder.create<VPVLSStore>(
      "vls.store", WideValue, BaseAddr, GroupSizeInGranularityElements,
      FirstMemrefInst->getAlignment(), Group->size());
  setMemOpProperties(WideStore);
}

void VLSTransform::run(DenseSet<VPInstruction *> &InstsToRemove) {
  assert(FailureReason == nullptr && "Transformation is impossible!");
  if (Group->getAccessKind().isLoad()) {
    processLoadGroup(InstsToRemove);
  } else {
    processStoreGroup(InstsToRemove);
  }
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
void llvm::vpo::applyVLSTransform(VPlanVector &Plan, VPlanVLSAnalysis &VLSA,
                                  unsigned VF) {
  DenseSet<VPInstruction *> InstsToRemove;
  for (auto *Group : VLSA.groups(&Plan)) {
    VLSTransform Transform(Group, Plan, VF);
    if (auto *FailureReason = Transform.getFailureReason()) {
      LLVM_DEBUG(dbgs() << FailureReason << '\n'; Group->dump();
                 dbgs() << '\n');
      continue;
    }

    Transform.run(InstsToRemove);
  }

  while (!InstsToRemove.empty()) {
    auto *Inst = *InstsToRemove.begin();
    Inst->invalidateUnderlyingIR(); // For HIR.
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

bool llvm::vpo::isTransformableVLSGroup(OVLSGroup *Group) {
  auto Instructions = instructions(Group);
  auto *FirstInst = const_cast<VPLoadStoreInst *>(&*(*Instructions.begin()));
  VPlanVector &Plan = cast<VPlanVector>(*FirstInst->getParent()->getParent());
  VLSTransform Transform(Group, Plan, 1 /*VF*/);
  return Transform.getFailureReason() == nullptr;
}

VPValue *VLSTransform::adjustBasePtrForReverse(VPValue *Base,
                                               VPBuilder &Builder) {
  if (*GroupStride > 0)
    return Base;

  auto *BaseTy = cast<PointerType>(Base->getType());
  if (BaseTy->isOpaque()) {
    auto *Result = Builder.createGEP(
        GroupGranularityType, GroupGranularityType, Base,
        {Plan.getVPConstant(-APInt(
            64, GroupSizeInGranularityElements * (VF - 1), true /* Signed */))},
        nullptr /* Underlying Instruction */);
    Result->setName(Base->getName() + ".reverse.adjust");
    return Result;
  }

  auto *Ty = BaseTy->getPointerElementType();
  // We rely on no gaps and equal sizes here.
  assert(DL.getTypeSizeInBits(GroupTy->getElementType()) ==
             DL.getTypeSizeInBits(Ty) &&
         "Type combination isn't supported yet.");
  unsigned Scale = 1;
  if (auto *VecTy = dyn_cast<FixedVectorType>(Ty))
    Scale = VecTy->getNumElements();

  // TODO: API boundaries are wacky here.
  auto *Result = Builder.createGEP(
      FirstMemrefInst->getValueType(),
      FirstMemrefInst->getValueType(),
      Base,
      {Plan.getVPConstant(
          -APInt(64, GroupSizeInGranularityElements * (VF - 1) / Scale,
                 true /* Signed */))},
      nullptr /* Underlying Instruction */);
  Result->setName(Base->getName() + ".reverse.adjust");
  return Result;
}

VPValue *VLSTransform::adjustGroupValForReverse(VPBuilder &Builder,
                                                VPValue *GroupVal) {
  if (*GroupStride > 0)
    return GroupVal;

  auto &Ctx = *Plan.getLLVMContext();
  SmallVector<Constant *, 16> Mask;
  for (unsigned i = 0; i < VF; ++i) {
    for (unsigned j = 0; j < GroupSizeInGranularityElements; ++j)
      Mask.push_back(ConstantInt::get(
          Ctx, APInt(64, (VF - i - 1) * GroupSizeInGranularityElements + j)));
  }
  auto *Undef = UndefValue::get(Mask[0]->getType());
  for (unsigned i = VF * GroupSizeInGranularityElements;
       i < GroupTy->getNumElements(); ++i)
    Mask.push_back(Undef);

  // NOTE: we rely on the fact that all CGs would emit it unmasked, even though
  // it's not guaranteed.
  auto Shuffle = Builder.createNaryOp(
      Instruction::ShuffleVector, GroupTy,
      {GroupVal, GroupVal, Plan.getVPConstant(ConstantVector::get(Mask))});
  DA.markUniform(*Shuffle);
  Shuffle->setName(GroupVal->getName() + ".reverse");

  return Shuffle;
}
