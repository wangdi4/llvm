//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   IntelVPOCodeGen.cpp -- VPValue-based LLVM IR code generation from VPlan.
//
//===----------------------------------------------------------------------===//

#include "IntelLoopVectorizationCodeGen.h"
#include "IntelLoopVectorizationLegality.h"
#include "IntelVPlan.h"
#include "IntelVPlanUtils.h"
#include "IntelVPlanVLSAnalysis.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/IntrinsicUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include <tuple>

using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "vpo-ir-loop-vectorize"

///////// VPValue version of common vectorizer legality utilities /////////

/// Helper function to check if given VPValue has consecutive pointer stride.
// TODO: Need to use DA to identify unit-stridedness.
static int isVPValueConsecutivePtr(VPValue *V) { return 0; }

/// Helper function to check if given VPValue is uniform based on DA.
bool VPOCodeGen::isVPValueUniform(VPValue *V, const VPlan *Plan) {
  // TODO;TODO;TODO
  // At this point VPlan-to-VPlan transforms are not updating DA after
  // introducing new VPInstructions. Hence VPValues without underlying IR are
  // known to have incorrect DA results. This must be removed once this problem
  // is addressed.
  if (!V->getUnderlyingValue())
    return false;

  return !Plan->getVPlanDA()->isDivergent(*V);
}

/// Helper function to check if VPValue is linear and return linear step in \p
/// Step.
// TODO: Use new VPO legality infra to get this information.
static bool isVPValueLinear(VPValue *V, int *Step = nullptr) {
  if (Step)
    *Step = 0;
  return false;
}

/// Helper function to check if VPValue is unit step linear and return linear
/// step in \p Step and new scalar value in \p NewScalarV.
// TODO: Use new VPO legality infra to get this information.
static bool isVPValueUnitStepLinear(VPValue *V, int *Step = nullptr,
                                    Value **NewScalarV = nullptr) {
  if (Step)
    *Step = 0;
  if (NewScalarV)
    *NewScalarV = nullptr;
  return false;
}

///////////// VPValue version of cast and GEP utilities /////////////

template <class Type> struct VPlanOpcodeMapper;

template <> struct VPlanOpcodeMapper<llvm::BitCastInst> {
  static const unsigned Opcode = Instruction::BitCast;
};
template <> struct VPlanOpcodeMapper<llvm::AddrSpaceCastInst> {
  static const unsigned Opcode = Instruction::AddrSpaceCast;
};

/// A helper function that returns VPValue after skipping bitcast and
/// addrspacecast.
template <typename CastInstTy>
static VPValue *getVPPtrThruCast(VPValue *Ptr, const DataLayout &DL) {
  while (VPInstruction *PtrInst = dyn_cast<VPInstruction>(Ptr)) {
    if (PtrInst->getOpcode() != VPlanOpcodeMapper<CastInstTy>::Opcode)
      break;
    Type *DestTy = PtrInst->getType();
    Type *SrcTy = PtrInst->getOperand(0)->getType();
    if (!isa<PointerType>(DestTy) || !isa<PointerType>(SrcTy))
      break;
    Type *Pointee1Ty = cast<PointerType>(DestTy)->getPointerElementType();
    Type *Pointee2Ty = cast<PointerType>(SrcTy)->getPointerElementType();
    if (DL.getTypeSizeInBits(Pointee1Ty) != DL.getTypeSizeInBits(Pointee2Ty))
      break;
    Ptr = PtrInst->getOperand(0);
  }
  return Ptr;
}

/// Helper function that return VPGEP instruction and knows to skip bitcast or
/// addrspacecast.
// TODO: Make this utility recursive to handle combination of cast patterns.
static VPGEPInstruction *getGEPInstruction(VPValue *Ptr, const DataLayout &DL) {
  if (auto *VPGEP = dyn_cast<VPGEPInstruction>(Ptr))
    return VPGEP;
  VPGEPInstruction *GEP =
      dyn_cast<VPGEPInstruction>(getVPPtrThruCast<BitCastInst>(Ptr, DL));
  if (!GEP)
    GEP = dyn_cast<VPGEPInstruction>(
        getVPPtrThruCast<AddrSpaceCastInst>(Ptr, DL));
  return GEP;
}

/// Helper function that returns widened type of given type \p VPInstTy.
static Type *getVPInstVectorType(Type *VPInstTy, unsigned VF) {
  unsigned NumElts =
      VPInstTy->isVectorTy() ? VPInstTy->getVectorNumElements() * VF : VF;
  return VectorType::get(VPInstTy->getScalarType(), NumElts);
}

/// Helper function to generate and insert a scalar LLVM instruction from
/// VPInstruction based on its opcode and scalar versions of its operands.
// TODO: Currently we don't populate IR flags/metadata information for the
// instructions generated below. Update after VPlan has internal representation
// for them.
static Value *generateSerialInstruction(IRBuilder<> &Builder,
                                        VPInstruction *VPInst,
                                        ArrayRef<Value *> ScalarOperands) {
  SmallVector<Value *, 4> Ops(ScalarOperands.begin(), ScalarOperands.end());
  Value *SerialInst = nullptr;
  if (Instruction::isBinaryOp(VPInst->getOpcode())) {
    assert(ScalarOperands.size() == 2 &&
           "Binop VPInstruction has incorrect number of operands.");
    SerialInst = Builder.CreateBinOp(
        static_cast<Instruction::BinaryOps>(VPInst->getOpcode()), Ops[0],
        Ops[1]);
  } else if (VPInst->getOpcode() == Instruction::Load) {
    assert(ScalarOperands.size() == 1 &&
           "Load VPInstruction has incorrect number of operands.");
    SerialInst = Builder.CreateLoad(Ops[0]);
  } else if (VPInst->getOpcode() == Instruction::Call) {
    assert(ScalarOperands.size() > 0 &&
           "Call VPInstruction should have atleast one operand.");
    if (auto *ScalarF = dyn_cast<Function>(Ops.back())) {
      Ops.pop_back();
      SerialInst = Builder.CreateCall(ScalarF, Ops);
    } else {
      // Indirect call (via function pointer).
      Value *FuncPtr = Ops.back();
      Ops.pop_back();

      Type *FuncPtrTy = FuncPtr->getType();
      assert(isa<PointerType>(FuncPtrTy) &&
             "Function pointer operand is not pointer type.");
      auto *FT = cast<FunctionType>(
          cast<PointerType>(FuncPtrTy)->getPointerElementType());

      SerialInst = Builder.CreateCall(FT, FuncPtr, Ops);
    }
  } else if (VPGEPInstruction *VPGEP = dyn_cast<VPGEPInstruction>(VPInst)) {
    assert(ScalarOperands.size() > 1 &&
           "VPGEPInstruction should have atleast two operands.");
    Value *GepBasePtr = Ops[0];
    Ops.erase(Ops.begin());
    SerialInst = Builder.CreateGEP(GepBasePtr, Ops);
    cast<GetElementPtrInst>(SerialInst)->setIsInBounds(VPGEP->isInBounds());
  } else {
    LLVM_DEBUG(dbgs() << "VPInst: "; VPInst->dump());
    llvm_unreachable("Currently serialization of only binop instructions, "
                     "load, call, gep is supported.");
  }

  return SerialInst;
}

SmallVector<int, 16>
VPOCodeGen::getVPShuffleOriginalMask(const VPInstruction *VPI) {
  assert(VPI->getOpcode() == Instruction::ShuffleVector &&
         "getVPShuffleOriginalMask called on non-shuffle instruction.");
  // The last operand of shufflevector is the mask and it is expected to always
  // be a constant.
  VPConstant *ShufMask =
      cast<VPConstant>(VPI->getOperand(VPI->getNumOperands() - 1));
  Constant *ShufMaskConst = ShufMask->getConstant();
  SmallVector<int, 16> Result;

  unsigned NumElts = ShufMaskConst->getType()->getVectorNumElements();
  if (auto *CDS = dyn_cast<ConstantDataSequential>(ShufMaskConst)) {
    for (unsigned I = 0; I != NumElts; ++I)
      Result.push_back(CDS->getElementAsInteger(I));
    return Result;
  }
  for (unsigned I = 0; I != NumElts; ++I) {
    Constant *C = ShufMaskConst->getAggregateElement(I);
    Result.push_back(isa<UndefValue>(C) ? -1
                                        : cast<ConstantInt>(C)->getZExtValue());
  }

  return Result;
}

const VPValue *VPOCodeGen::getOrigSplatVPValue(const VPValue *V) {
  if (auto *C = dyn_cast<VPConstant>(V)) {
    if (isa<VectorType>(V->getType())) {
      Constant *SplatC =
          cast<Constant>(
              getScalarValue(const_cast<VPConstant *>(C), 0 /*Lane*/))
              ->getSplatValue();
      // We need to create a new VPConstant to represent a splat constant
      // vector.
      return const_cast<VPlan *>(Plan)->getVPConstant(SplatC);
    }
  }

  auto *VPInst = dyn_cast<VPInstruction>(V);
  if (!VPInst || VPInst->getOpcode() != Instruction::ShuffleVector)
    return nullptr;

  // All-zero or undef shuffle mask elements.
  if (any_of(getVPShuffleOriginalMask(VPInst),
             [](int MaskElt) -> bool { return MaskElt != 0 && MaskElt != -1; }))
    return nullptr;

  // The first shuffle source is 'insertelement' with index 0.
  auto *InsertEltVPInst = dyn_cast<VPInstruction>(VPInst->getOperand(0));
  if (!InsertEltVPInst ||
      InsertEltVPInst->getOpcode() != Instruction::InsertElement)
    return nullptr;

  if (auto *ConstIdx = dyn_cast<VPConstant>(InsertEltVPInst->getOperand(2))) {
    Value *ConstIdxV = getScalarValue(ConstIdx, 0 /*Lane*/);
    if (!isa<ConstantInt>(ConstIdxV) || !cast<ConstantInt>(ConstIdxV)->isZero())
      return nullptr;
  } else {
    return nullptr;
  }

  return InsertEltVPInst->getOperand(1);
}

template <typename CastInstTy>
void VPOCodeGen::vectorizeCast(
    typename std::enable_if<
        std::is_same<CastInstTy, BitCastInst>::value ||
            std::is_same<CastInstTy, AddrSpaceCastInst>::value,
        VPInstruction>::type *VPInst) {
  // TODO: Update code to handle loop privates.
  Value *VecOp = getVectorValue(VPInst->getOperand(0));
  Type *VecTy = getVPInstVectorType(VPInst->getType(), VF);

  if (std::is_same<CastInstTy, BitCastInst>::value)
    VPWidenMap[VPInst] = Builder.CreateBitCast(VecOp, VecTy);
  else
    VPWidenMap[VPInst] = Builder.CreateAddrSpaceCast(VecOp, VecTy);
}

void VPOCodeGen::vectorizeCallArgs(VPInstruction *VPCall,
                                   VectorVariant *VecVariant,
                                   SmallVectorImpl<Value *> &VecArgs,
                                   SmallVectorImpl<Type *> &VecArgTys) {
  std::vector<VectorKind> Parms;
  if (VecVariant) {
    Parms = VecVariant->getParameters();
  }

  Function *F = getCalledFunction(VPCall);
  assert(F && "Function not found for call instruction");
  StringRef FnName = F->getName();

  auto ProcessCallArg = [&](unsigned OrigArgIdx) -> Value * {
    if (isOpenCLWriteChannelSrc(FnName, OrigArgIdx)) {
      llvm_unreachable(
          "VPVALCG: OpenCL write channel vectorization not uplifted.");
    }

    if ((!VecVariant || Parms[OrigArgIdx].isVector()) &&
        !isScalarArgument(FnName, OrigArgIdx)) {
      // This is a vector call arg, so vectorize it.
      VPValue *Arg = VPCall->getOperand(OrigArgIdx);

      // Generate the right mask for OpenCL vector 'select' intrinsic
      if (isOpenCLSelectMask(FnName, OrigArgIdx)) {
        llvm_unreachable("VPVALCG: OpenCL select vector mask not uplifted.");
      }

      return getVectorValue(Arg);
    }
    // Linear and uniform parameters for simd functions must be passed as
    // scalars according to the vector function abi. CodeGen currently
    // vectorizes all instructions, so the scalar arguments for the vector
    // function must be extracted from them. For both linear and uniform
    // args, extract from lane 0. Linear args can use the value at lane 0
    // because this will be the starting value for which the stride will be
    // added. The same method applies to built-in functions for args that
    // need to be treated as uniform.

    assert(!isOpenCLSelectMask(FnName, OrigArgIdx) &&
           "OpenCL select mask parameter is linear/uniform?");

    VPValue *Arg = VPCall->getOperand(OrigArgIdx);
    Value *ScalarArg = getScalarValue(Arg, 0);

    return ScalarArg;
  };

  // TODO: For a VPInstruction representing Call, all the Call argument operands
  // are stored first. The last operand represents the called Function. Is this
  // true in all cases? What about indirect calls?
  unsigned NumArgOperands = VPCall->getNumOperands() - 1;

  for (unsigned OrigArgIdx = 0; OrigArgIdx < NumArgOperands; OrigArgIdx++) {
    if (isOpenCLReadChannelDest(FnName, OrigArgIdx))
      continue;

    Value *VecArg = ProcessCallArg(OrigArgIdx);
    VecArgs.push_back(VecArg);
    VecArgTys.push_back(VecArg->getType());
  }

  // We're done, unless we have an additional mask parameter to process that
  // wasn't part of the original (scalar) call.
  if (!VecVariant || !VecVariant->isMasked())
    return;

  Value *MaskToUse = MaskValue ? MaskValue
                               : Constant::getAllOnesValue(VectorType::get(
                                     Type::getInt1Ty(F->getContext()), VF));

  // Add the mask parameter for masked simd functions.
  // Mask should already be vectorized as i1 type.
  VectorType *MaskTy = cast<VectorType>(MaskToUse->getType());
  assert(MaskTy->getVectorElementType()->isIntegerTy(1) &&
         "Mask parameter is not vector of i1");

  // Incorrect code is generated by backend codegen when using i1 mask.
  // Therefore, the mask is promoted to the characteristic type of the
  // function, unless we're specifically told not to do so.
  if (Usei1MaskForSimdFunctions) {
    VecArgs.push_back(MaskToUse);
    VecArgTys.push_back(MaskTy);
    return;
  }

  // Promote to characteristic type.
  Type *CharacteristicType = calcCharacteristicType(*F, *VecVariant);
  unsigned CharacteristicTypeSize =
      CharacteristicType->getPrimitiveSizeInBits();

  // Promote the i1 to an integer type that has the same size as the
  // characteristic type.
  Type *ScalarToType =
      IntegerType::get(MaskTy->getContext(), CharacteristicTypeSize);
  VectorType *VecToType = VectorType::get(ScalarToType, VF);
  Value *MaskExt = Builder.CreateSExt(MaskToUse, VecToType, "maskext");

  // Bitcast if the promoted type is not the same as the characteristic
  // type.
  if (ScalarToType != CharacteristicType) {
    Type *MaskCastTy = VectorType::get(CharacteristicType, VF);
    Value *MaskCast = Builder.CreateBitCast(MaskExt, MaskCastTy, "maskcast");
    VecArgs.push_back(MaskCast);
    VecArgTys.push_back(MaskCastTy);
  } else {
    VecArgs.push_back(MaskExt);
    VecArgTys.push_back(VecToType);
  }
}

void VPOCodeGen::vectorizeSelectInstruction(VPInstruction *VPInst) {
  // If the selector is loop invariant we can create a select
  // instruction with a scalar condition. Otherwise, use vector-select.
  VPValue *Cond = VPInst->getOperand(0);
  Value *VCond = getVectorValue(Cond);
  Value *Op0 = getVectorValue(VPInst->getOperand(1));
  Value *Op1 = getVectorValue(VPInst->getOperand(2));

  // TODO: Using DA for loop invariance.
  bool UniformCond = isVPValueUniform(Cond, Plan);

  // The condition can be loop invariant  but still defined inside the
  // loop. This means that we can't just use the original 'cond' value.

  if (UniformCond) {
    // TODO: Handle uniform vector condition in selects.
    assert(!Cond->getType()->isVectorTy() &&
           "Uniform vector condition is not supported.");
    VCond = getScalarValue(Cond, 0);
  } else if (!Cond->getType()->isVectorTy() &&
             VPInst->getType()->isVectorTy()) {
    unsigned OriginalVL = VPInst->getType()->getVectorNumElements();
    // Widen the cond variable as following
    //                        <0, 1, 0, 1>
    //                             |
    //                             | VF = 4,
    //                             | OriginalVL = 2
    //                             |
    //                             V
    //                  <0, 0, 1, 1, 0, 0, 1, 1>

    VCond = replicateVectorElts(VCond, OriginalVL, Builder);
  }

  Value *NewSelect = Builder.CreateSelect(VCond, Op0, Op1);

  VPWidenMap[VPInst] = NewSelect;
}

unsigned
VPOCodeGen::getOriginalLoadStoreAlignment(const VPInstruction *VPInst) {
  assert((VPInst->getOpcode() == Instruction::Load ||
          VPInst->getOpcode() == Instruction::Store) &&
         "Alignment helper called on non load/store instruction.");
  unsigned Alignment = 1;
  // TODO: Peeking at underlying Value for alignment info.
  if (auto *UV = VPInst->getUnderlyingValue())
    Alignment = getLoadStoreAlignment(UV);

  if (!Alignment) {
    // An alignment of 0 means target abi alignment. We need to use the scalar's
    // target abi alignment in such a case.
    const DataLayout &DL = OrigLoop->getHeader()->getModule()->getDataLayout();
    // For store instructions alignment is determined by type of value operand.
    Type *OrigTy = VPInst->getOpcode() == Instruction::Load
                       ? VPInst->getType()
                       : VPInst->getOperand(0)->getType();

    Alignment = DL.getABITypeAlignment(OrigTy);
  }

  return Alignment;
}

Value *VPOCodeGen::getOrCreateWideLoadForGroup(OVLSGroup *Group) {
  auto FoundIter = VLSGroupLoadMap.find(Group);
  if (FoundIter != VLSGroupLoadMap.end())
    return FoundIter->second;

  // Check if the group is valid for this VF.
  assert(Group->getNumElems() == VF &&
         "Group number of elements must match VF");

  const VPInstruction *Leader =
      cast<VPVLSClientMemref>(Group->getFirstMemref())->getInstruction();

  Type *GroupType =
      getWidenedType(getLoadStoreType(Leader), VF * Group->size());

  Value *GatherAddress = getVectorValue(Leader->getOperand(0));
  assert(!MaskValue && "Scalar address may be invalid (masked out)");
  Value *ScalarAddress = Builder.CreateExtractElement(
      GatherAddress, (uint64_t)0, GatherAddress->getName() + "_0");
  auto AddressSpace =
      cast<PointerType>(ScalarAddress->getType())->getAddressSpace();
  Value *GroupPtr = Builder.CreateBitCast(
      ScalarAddress, GroupType->getPointerTo(AddressSpace), "groupPtr");
  unsigned Align = getOriginalLoadStoreAlignment(Leader);
  LoadInst *GroupLoad =
      Builder.CreateAlignedLoad(GroupType, GroupPtr, Align, "groupLoad");

  VLSGroupLoadMap.insert(std::make_pair(Group, GroupLoad));
  return GroupLoad;
}

Value *VPOCodeGen::vectorizeInterleavedLoad(VPInstruction *VPLoad,
                                            OVLSGroup *Group) {
  auto MemrefIter = find_if(*Group, [VPLoad](OVLSMemref *Iter) {
    return cast<VPVLSClientMemref>(Iter)->getInstruction() == VPLoad;
  });
  assert(MemrefIter != Group->end() &&
         "Instruction does not belong to the group");
  OVLSMemref *Memref = *MemrefIter;

  auto InterleaveIndex = computeInterleaveIndex(Memref, Group);
  auto InterleaveFactor = computeInterleaveFactor(Memref);

  Value *GroupLoad = getOrCreateWideLoadForGroup(Group);
  Constant *ShuffleMask =
      createStrideMask(Builder, InterleaveIndex, InterleaveFactor, VF);
  return Builder.CreateShuffleVector(GroupLoad,
                                     UndefValue::get(GroupLoad->getType()),
                                     ShuffleMask, "groupShuffle");
}

void VPOCodeGen::vectorizeLoadInstruction(VPInstruction *VPInst,
                                          bool EmitIntrinsic) {
  Type *LoadType = VPInst->getType();
  assert((!LoadType->isVectorTy() ||
          LoadType->getVectorElementType()->isSingleValueType()) &&
         "Re-vectorization supports simple vectors only!");

  // Pointer operand of Load is always the first operand.
  VPValue *Ptr = VPInst->getOperand(0);
  int LinStride = 0;

  // Handle vectorization of a linear value load.
  if (isVPValueLinear(Ptr, &LinStride)) {
    llvm_unreachable("VPVALCG: Vectorization of linear load not uplifted.");
#if 0
    vectorizeLinearLoad(Inst, LinStride);
    return;
#endif
  }

  // TODO: Using DA for loop invariance.
  if (isVPValueUniform(Ptr, Plan)) {
    if (MaskValue)
      serializePredicatedUniformLoad(VPInst);
    else
      serializeInstruction(VPInst);
    return;
  }

  unsigned OriginalVL =
      LoadType->isVectorTy() ? LoadType->getVectorNumElements() : 1;
  int ConsecutiveStride = isVPValueConsecutivePtr(Ptr);

  unsigned Alignment = getOriginalLoadStoreAlignment(VPInst);
  Value *NewLI = nullptr;

  // Handle consecutive loads.
  if (ConsecutiveStride) {
    llvm_unreachable("VPVALCG: Unit-strided load vectorization not uplifted.");
#if 0
    Value *VecPtr = createWidenedBasePtrConsecutiveLoadStore(
        LI, Ptr, ConsecutiveStride == -1);

    // Masking not needed for privates.
    if (MaskValue && !Legal->isLoopPrivate(Ptr)) {
      // Replicate the mask if VPInst is a vector instruction.
      Value *RepMaskValue = replicateVectorElts(MaskValue, OriginalVL, Builder,
                                                "replicatedMaskElts.");
      NewLI = Builder.CreateMaskedLoad(VecPtr, Alignment, RepMaskValue, nullptr,
                                       "wide.masked.load");
    } else
      NewLI = Builder.CreateAlignedLoad(VecPtr, Alignment, "wide.load");

    if (ConsecutiveStride == -1) //Reverse
      NewLI = reverseVector(NewLI);

    WidenMap[cast<Value>(Inst)] = NewLI;
#endif
  } else {

    // Try to do GATHER-to-SHUFFLE optimization.
    // TODO: VLS optimization is disabled in masked basic blocks so far. It
    // should be enabled for uniform masks, though.
    OVLSGroup *Group = MaskValue ? nullptr : VLSA->getGroupsFor(Plan, VPInst);
    // TODO: Enable VLS for vector types. We cannot generate correct shuffle
    // mask yet.
    if (Group && Group->size() > 1 && OriginalVL == 1) {
      Optional<int64_t> GroupStride = Group->getConstStride();
      assert(GroupStride && "Indexed loads are not supported");
      // Groups with gaps are not supported either.
      if (Group->getNByteAccessMask() == ~(UINT64_MAX << *GroupStride))
        NewLI = vectorizeInterleavedLoad(VPInst, Group);
    }

    // If VLS failed to emit a wide load, we have to emit a GATHER instruction.
    if (!NewLI) {
      // Replicate the mask if VPInst is a vector instruction originally.
      Value *RepMaskValue = nullptr;
      if (MaskValue)
        RepMaskValue = replicateVectorElts(MaskValue, OriginalVL, Builder,
                                           "replicatedMaskElts.");
      Value *GatherAddress = getWidenedAddressForScatterGather(VPInst);
      NewLI = Builder.CreateMaskedGather(GatherAddress, Alignment, RepMaskValue,
                                         nullptr, "wide.masked.gather");
    }
  }

  VPWidenMap[VPInst] = NewLI;
}

void VPOCodeGen::vectorizeInterleavedStore(VPInstruction *VPStore,
                                           OVLSGroup *Group) {
  // First, check if the wide store has been already generated.
  if (VLSGroupStoreMap.find(Group) != VLSGroupStoreMap.end())
    return;

  // If the wide store hasn't been generated yet, generate it.

  // FIXME: Currently VLS groups store instructions only if it is safe to move
  //        all the stores to the lexically first one (that means that all the
  //        stored values must be available at this point). That matches our
  //        current behavior in VPlan: we generate wide store when we encounter
  //        first store from a group. However, we may want to modify VLS
  //        analysis, and for example check if it is safe to move all group
  //        elements to the last store. If we do such change, the code below
  //        will be broken, as it will keep generating wide store in place of
  //        the first store.

  // Values to be stored.
  SmallVector<Value *, 8> GrpValues;

  // Interleave indexes of the values/memrefs.
  SmallVector<int, 8> GrpIndexes;

  // Populate GrpValues and GrpIndexes.
  for (OVLSMemref *Mrf : *Group) {
    VPValue *V = cast<VPVLSClientMemref>(Mrf)->getInstruction()->getOperand(0);
    GrpValues.push_back(getVectorValue(V));
    GrpIndexes.push_back(computeInterleaveIndex(Mrf, Group));
  }

  // For now indexes are assumed to be <0, 1, 2, ... N-1>. If it turns out not
  // true, we can sort the arrays and/or insert undefs instead of gaps.
  for (int i = 0, ie = GrpIndexes.size(); i < ie; ++i)
    assert(GrpIndexes[i] == i && "Unsupported memory references sequence");

  // Check that all memory references have the same interleave factor.
  OVLSMemref *MemRef = Group->getFirstMemref();
  assert(MemRef &&
         "Expect a non-null first memref to determine the Interleave factor.");
  auto InterleaveFactor = computeInterleaveFactor(MemRef);
  assert(all_of(*Group,
                [InterleaveFactor](OVLSMemref *x) {
                  return computeInterleaveFactor(x) == InterleaveFactor;
                }) &&
         "Cannot compute shuffle mask for groups with different access sizes");

  // Check if the group is valid for this VF.
  assert(Group->getNumElems() == VF &&
         "Group number of elements must match VF");

  // Concatenate all the values being stored into a single wide vector.
  Value *ConcatValue = concatenateVectors(Builder, GrpValues);

  // Shuffle scalar values into the correct order.
  Constant *ShuffleMask = createInterleaveMask(Builder, VF, InterleaveFactor);
  Value *StoredValue = Builder.CreateShuffleVector(
      ConcatValue, UndefValue::get(ConcatValue->getType()), ShuffleMask,
      "groupShuffle");

  // Compute address for the wide store.
  const VPInstruction *Leader =
      cast<VPVLSClientMemref>(MemRef)->getInstruction();
  Value *ScatterAddress = getVectorValue(Leader->getOperand(1));
  assert(!MaskValue && "Scalar address may be invalid (masked out)");
  Value *ScalarAddress = Builder.CreateExtractElement(
      ScatterAddress, (uint64_t)0, ScatterAddress->getName() + "_0");
  auto AddressSpace =
      cast<PointerType>(ScalarAddress->getType())->getAddressSpace();
  Type *GroupPtrTy = StoredValue->getType()->getPointerTo(AddressSpace);
  Value *GroupPtr =
      Builder.CreateBitCast(ScalarAddress, GroupPtrTy, "groupPtr");

  // Create the wide store.
  unsigned Align = getOriginalLoadStoreAlignment(VPStore);
  StoreInst *GroupStore =
      Builder.CreateAlignedStore(StoredValue, GroupPtr, Align);
  VLSGroupStoreMap.insert(std::make_pair(Group, GroupStore));
}

void VPOCodeGen::vectorizeStoreInstruction(VPInstruction *VPInst,
                                           bool EmitIntrinsic) {
  Type *StoreType = VPInst->getOperand(0)->getType();
  assert((!StoreType->isVectorTy() ||
          StoreType->getVectorElementType()->isSingleValueType()) &&
         "Re-vectorization supports simple vectors only!");

  // Pointer operand of Store will always be second operand.
  VPValue *Ptr = VPInst->getOperand(1);

  // Handle vectorization of a linear value store.
  if (isVPValueLinear(Ptr)) {
    llvm_unreachable("VPVALCG: Vectorization of linear store not uplifted.");
#if 0
    vectorizeLinearStore(Inst);
    return;
#endif
  }

  unsigned OriginalVL =
      StoreType->isVectorTy() ? StoreType->getVectorNumElements() : 1;
  int ConsecutiveStride = isVPValueConsecutivePtr(Ptr);

  unsigned Alignment = getOriginalLoadStoreAlignment(VPInst);
  Value *VecDataOp = getVectorValue(VPInst->getOperand(0));

  // Handle consecutive stores.
  if (ConsecutiveStride) {
    llvm_unreachable("VPVALCG: Unit-strided store vectorization not uplifted.");
#if 0
    bool StoreMaskValue = Legal->isCondLastPrivate(Ptr);
    Value *VecPtr = createWidenedBasePtrConsecutiveLoadStore(
        SI, Ptr, ConsecutiveStride == -1);

    if (ConsecutiveStride == -1) // Reverse
      // If we store to reverse consecutive memory locations, then we need
      // to reverse the order of elements in the stored value.
      VecDataOp = reverseVector(VecDataOp);

    if (MaskValue) {
      // Replicate the mask if VPInst is a vector instruction originally.
      Value *RepMaskValue = replicateVectorElts(MaskValue, OriginalVL, Builder,
                                                "replicatedMaskElts.");
      Builder.CreateMaskedStore(VecDataOp, VecPtr, Alignment, RepMaskValue);
    } else
      Builder.CreateAlignedStore(VecDataOp, VecPtr, Alignment);

    if (StoreMaskValue)
      storeMaskValue(MaskValue, LoopPrivateLastMask[getPtrThruCast<BitCastInst>(Ptr)], VF,
                     Builder);
#endif
  } else {
    // Try to do SCATTER-to-SHUFFLE optimization.
    // TODO: VLS optimization is disabled in masked basic blocks so far. It
    // should be enabled for uniform masks, though.
    OVLSGroup *Group = MaskValue ? nullptr : VLSA->getGroupsFor(Plan, VPInst);
    // TODO: Enable VLS for vector types. We cannot generate correct shuffle
    // mask yet.
    if (Group && Group->size() > 1 && OriginalVL == 1) {
      Optional<int64_t> GroupStride = Group->getConstStride();
      assert(GroupStride && "Indexed loads are not supported");
      // Groups with gaps are not supported either.
      if (Group->getNByteAccessMask() == ~(UINT64_MAX << *GroupStride)) {
        vectorizeInterleavedStore(VPInst, Group);
        return;
      }
    }

    // If VLS failed to emit a wide store, we have to emit a SCATTER
    // instruction.
    Value *ScatterPtr = getWidenedAddressForScatterGather(VPInst);
    Type *PtrToElemTy = ScatterPtr->getType()->getVectorElementType();
    Type *ElemTy = PtrToElemTy->getPointerElementType();
    VectorType *DesiredDataTy = VectorType::get(ElemTy, VF * OriginalVL);
    // TODO: Verify if this bitcast should be done this late. Maybe an earlier
    // transform can introduce it, if needed.
    VecDataOp = Builder.CreateBitCast(VecDataOp, DesiredDataTy, "cast");

    // Replicate the mask if VPInst is a vector instruction originally.
    Value *RepMaskValue = nullptr;
    if (MaskValue)
      RepMaskValue = replicateVectorElts(MaskValue, OriginalVL, Builder,
                                         "replicatedMaskElts.");
    Builder.CreateMaskedScatter(VecDataOp, ScatterPtr, Alignment, RepMaskValue);
  }
}

// This function returns computed addresses of memory locations which should be
// accessed in the vectorized code. These addresses, take the form of a GEP
// instruction, and this GEP is used as pointer operand of the resulting
// scatter/gather intrinsic.
Value *VPOCodeGen::getWidenedAddressForScatterGather(VPInstruction *VPI) {
  assert((VPI->getOpcode() == Instruction::Load ||
          VPI->getOpcode() == Instruction::Store) &&
         "Expect 'VPI' to be either a LoadInst or a StoreInst");

  // Vectorize BasePtr.
  VPValue *VPBasePtr = getPointerOperand(VPI);
  Value *BasePtr = getVectorValue(VPBasePtr);

  // No replication is needed for non-vector types.
  Type *LSIType = getLoadStoreType(VPI);
  if (!isa<VectorType>(LSIType))
    return BasePtr;

  unsigned AddrSpace =
      cast<PointerType>(VPBasePtr->getType())->getAddressSpace();

  // Cast the inner vector-type to it's elemental scalar type.
  // e.g. - <VF x <OriginalVL x Ty> addrspace(x)*>
  //                          |
  //                          |
  //                          V
  //                <VF x Ty addrspace(x)*>
  Value *TypeCastBasePtr = Builder.CreateBitCast(
      BasePtr,
      VectorType::get(LSIType->getVectorElementType()->getPointerTo(AddrSpace),
                      VF));
  // Replicate the base-address OriginalVL times
  //                <VF x Ty addrspace(x)*>
  //                          |
  //                          |
  //                          V
  //      < 0, 1, .., OriginalVL-1, ..., 0, 1, ..., OriginalVL-1>
  unsigned OriginalVL = LSIType->getVectorNumElements();
  Value *VecBasePtr =
      replicateVectorElts(TypeCastBasePtr, OriginalVL, Builder, "vecBasePtr.");

  // Create a vector of consecutive numbers from zero to OriginalVL-1 repeated
  // VF-times.
  SmallVector<Constant *, 32> Indices;
  for (unsigned J = 0; J < VF; ++J)
    for (unsigned I = 0; I < OriginalVL; ++I) {
      Indices.push_back(
          ConstantInt::get(Type::getInt64Ty(LSIType->getContext()), I));
    }

  // Add the consecutive indices to the vector value.
  Constant *Cv = ConstantVector::get(Indices);

  // Create a GEP that would return the address of each elements that is to be
  // accessed.
  Value *WidenedVectorGEP =
      Builder.CreateGEP(nullptr, VecBasePtr, Cv, "elemBasePtr.");
  return WidenedVectorGEP;
}

void VPOCodeGen::vectorizeCallInstruction(VPInstruction *VPCall) {
  SmallVector<Value *, 2> VecArgs;
  SmallVector<Type *, 2> VecArgTys;
  CallInst *UnderlyingCI = dyn_cast<CallInst>(VPCall->getUnderlyingValue());
  assert(UnderlyingCI &&
         "VPVALCG: Need underlying CallInst for call-site attributes.");
  Function *CalledFunc = getCalledFunction(VPCall);
  assert(CalledFunc && "Unexpected null called function.");
  bool IsMasked = (MaskValue != nullptr) ? true : false;

  // Don't attempt vector function matching for SVML or built-in functions.
  std::unique_ptr<VectorVariant> MatchedVariant;

  // OpenCL SinCos, would have a 'nullptr' MatchedVariant.
  if (isOpenCLSinCos(CalledFunc->getName())) {
    llvm_unreachable("VPVALCG: OpenCL sincos vectorization not uplifted.");
#if 0
    vectorizeOpenCLSinCos(Call, isMasked);
    return;
#endif
  }

  if (!TLI->isFunctionVectorizable(CalledFunc->getName()) &&
      !isOpenCLReadChannel(CalledFunc->getName()) &&
      !isOpenCLWriteChannel(CalledFunc->getName())) {
    // TLI is not used to check for SIMD functions for two reasons:
    // 1) A more sophisticated interface is needed to determine the most
    //    appropriate match.
    // 2) A SIMD function is not a library function.
    MatchedVariant = matchVectorVariant(UnderlyingCI, IsMasked);
    if (!MatchedVariant && !IsMasked) {
      // If non-masked version isn't available, try running the masked version
      // with all-ones mask.
      MatchedVariant = matchVectorVariant(UnderlyingCI, true);
      IsMasked = true;
    }
    assert(MatchedVariant && "Unexpected null matched vector variant");
    LLVM_DEBUG(dbgs() << "Matched Variant: " << MatchedVariant->encode()
                      << "\n");
  }

  vectorizeCallArgs(VPCall, MatchedVariant.get(), VecArgs, VecArgTys);

  Function *VectorF = getOrInsertVectorFunction(CalledFunc, VF, VecArgTys, TLI,
                                                Intrinsic::not_intrinsic,
                                                MatchedVariant.get(), IsMasked);
  assert(VectorF && "Can't create vector function.");
  CallInst *VecCall = Builder.CreateCall(VectorF, VecArgs);

  // TODO: investigate why attempting to copy fast math flags for __read_pipe
  // fails. For now, just don't do the copy.
  // TODO: Fast math flags are not represented in VPValue yet.
  if (isa<FPMathOperator>(VecCall) &&
      !isOpenCLReadChannel(CalledFunc->getName())) {
#if 0
    VecCall->copyFastMathFlags(Call);
#endif
  }

  // Make sure we don't lose attributes at the call site. E.g., IMF
  // attributes are taken from call sites in MapIntrinToIml to refine
  // SVML calls for precision.
  // TODO: Call attributes are not represented in VPValue yet. Peek at
  // underlying instruction.
  copyRequiredAttributes(UnderlyingCI, VecCall);

  // Set calling convention for SVML function calls.
  if (isSVMLFunction(TLI, CalledFunc->getName(), VectorF->getName()))
    VecCall->setCallingConv(CallingConv::SVML);

  // TODO: Need a VPValue based analysis for call arg memory references.
  // VPValue-based stride info also needed.
#if 0
  Loop *Lp = LI->getLoopFor(Call->getParent());
  analyzeCallArgMemoryReferences(Call, VecCall, TLI, PSE.getSE(), Lp);
#endif

  // No blending is required here for masked simd function calls as of now for
  // two reasons:
  //
  // 1) A select is already generated for call results that are live outside of
  //    the predicated region by using the predicated region's mask. See
  //    widenNonInductionPhi().
  //
  // 2) Currently, masked stores are always generated for call results stored
  //    to memory within a predicated region. See vectorizeStoreInstruction().

  if (isOpenCLReadChannel(CalledFunc->getName())) {
    llvm_unreachable(
        "VPVALCG: OpenCL read channel vectorization not uplifted.");
#if 0
    vectorizeOpenCLReadChannelDest(Call, VecCall, Call->getArgOperand(1));
#endif
  }

  VPWidenMap[VPCall] = VecCall;
}

Value *VPOCodeGen::getVectorValueUplifted(VPValue *V) {
  // If we have this scalar in the map, return it.
  if (VPWidenMap.count(V))
    return VPWidenMap[V];

  // Address of in memory private is needed. Construct a vector of addresses
  // on the fly.
  // TODO: Privates are not represented in VPlan yet. So we need to peek at
  // underlying instruction.
  if (V->getUnderlyingValue() &&
      Legal->isLoopPrivate(V->getUnderlyingValue())) {
    Value *VectorValue = getVectorPrivatePtrs(V->getUnderlyingValue());
    VPWidenMap[V] = VectorValue;
    return VectorValue;
  }

  // If the VPValue has not been vectorized, check if it has been scalarized
  // instead. If it has been scalarized, and we actually need the value in
  // vector form, we will construct the vector values on demand.
  if (VPScalarMap.count(V)) {
    // Use DA to check if VPValue is uniform.
    bool IsUniform = isVPValueUniform(V, Plan);

    Value *VectorValue = nullptr;
    IRBuilder<>::InsertPointGuard Guard(Builder);
    if (IsUniform) {
      Value *ScalarValue = VPScalarMap[V][0];
      // ScalarValue can be a constant, so insertion point setting is not needed
      // for that case.
      if (isa<Instruction>(ScalarValue))
        Builder.SetInsertPoint((cast<Instruction>(ScalarValue))->getNextNode());
      if (ScalarValue->getType()->isVectorTy()) {
        VectorValue =
            replicateVector(ScalarValue, VF, Builder,
                            "replicatedVal." + ScalarValue->getName());
      } else
        VectorValue = Builder.CreateVectorSplat(VF, ScalarValue, "broadcast");

      VPWidenMap[V] = VectorValue;
      return VectorValue;
    }

    if (V->getType()->isVectorTy()) {
      SmallVector<Value *, 8> Parts;
      for (unsigned Lane = 0; Lane < VF; ++Lane)
        Parts.push_back(VPScalarMap[V][Lane]);
      Value *ScalarValue = VPScalarMap[V][VF - 1];
      assert(isa<Instruction>(ScalarValue) &&
             "Expected instruction for scalar value");
      Builder.SetInsertPoint((cast<Instruction>(ScalarValue))->getNextNode());
      VectorValue = joinVectors(Parts, Builder);
    } else {
      VectorValue = UndefValue::get(VectorType::get(V->getType(), VF));
      for (unsigned Lane = 0; Lane < VF; ++Lane) {
        Value *ScalarValue = VPScalarMap[V][Lane];
        assert(isa<Instruction>(ScalarValue) &&
               "Expected instruction for scalar value");
        Builder.SetInsertPoint((cast<Instruction>(ScalarValue))->getNextNode());
        VectorValue = Builder.CreateInsertElement(VectorValue, ScalarValue,
                                                  Builder.getInt32(Lane));
      }
    }

    VPWidenMap[V] = VectorValue;
    return VectorValue;
  }

  // VPInstructions should already be handled, only external values are expected
  // here.
  assert((isa<VPExternalDef>(V) || isa<VPConstant>(V)) &&
         "Unknown external VPValue.");
  assert(isVPValueUniform(V, Plan) && "External value is not uniform.");
  Value *UnderlyingV = getScalarValue(V, 0 /*Lane*/);
  assert(UnderlyingV && "VPExternalDefs and VPConstants are expected to have "
                        "underlying IR value set.");

  // Broadcast V and save the value for future uses.
  if (V->getType()->isVectorTy()) {
    assert(V->getType()->getVectorElementType()->isSingleValueType() &&
           "Re-vectorization is supported for simple vectors only");
    // Widen the uniform vector variable as following
    //                        <i32 0, i32 1>
    //                             |
    //                             |VF = 4
    //                             |
    //                             V
    //          <i32 0, i32 1,i32 0, i32 1,i32 0, i32 1,i32 0, i32 1>
    VPWidenMap[V] = replicateVector(UnderlyingV, VF, Builder,
                                    "replicatedVal." + UnderlyingV->getName());
  } else
    VPWidenMap[V] = getBroadcastInstrs(UnderlyingV);

  return VPWidenMap[V];
}

Value *VPOCodeGen::getScalarValueUplifted(VPValue *V, unsigned Lane) {
  if (isa<VPExternalDef>(V) || isa<VPConstant>(V))
    return V->getUnderlyingValue();

  // If the VPValue is loop invariant and not a private, it is already a scalar.
  // TODO: Privates are not represented in VPlan, peek at underlying value.
  // TODO: Currently we are using DA for loop-invariance, but uniformity is not
  // always same as invariance.
  if (isVPValueUniform(V, Plan) && V->getUnderlyingValue() &&
      !Legal->isLoopPrivate(V->getUnderlyingValue())) {
    // Fall through, new LLVM scalar instruction will be generated below.
  }

  if (VPScalarMap.count(V)) {
    auto SV = VPScalarMap[V];
    if (isVPValueUniform(V, Plan))
      // For uniform instructions the mapping is updated for lane zero only.
      Lane = 0;

    if (SV.count(Lane))
      return SV[Lane];
  }

#if 0
  // TODO: This will be handled by reduction/induction cleanup patch.
  if (Legal->isInductionVariable(V))
    return buildScalarIVForLane(cast<PHINode>(V), Lane);
#endif

  // This code assumes that the widened vector, that we are extracting from has
  // data in AOS layout. If OriginalVL = 2, VF = 4 the widened value would be
  // Wide.Val = <v1_0, v2_0, v1_1, v2_1, v1_2, v2_2, v1_3, v2_3>.
  // getScalarValue(Wide.Val, 1) would return <v1_1, v2_1>

  if (V->getType()->isVectorTy() && VPWidenMap.count(V)) {
    Value *WidenedVar = VPWidenMap[V];
    unsigned OrigNumElts = V->getType()->getVectorNumElements();
    SmallVector<unsigned, 8> ShufMask;
    for (unsigned StartIdx = Lane * OrigNumElts,
                  EndIdx = (Lane * OrigNumElts) + OrigNumElts;
         StartIdx != EndIdx; ++StartIdx)
      ShufMask.push_back(StartIdx);

    Value *Shuff = Builder.CreateShuffleVector(
        WidenedVar, UndefValue::get(WidenedVar->getType()), ShufMask,
        "extractsubvec.");

    VPScalarMap[V][Lane] = Shuff;

    return Shuff;
  }

  Value *VecV = getVectorValue(V);
  IRBuilder<>::InsertPointGuard Guard(Builder);
  if (auto VecInst = dyn_cast<Instruction>(VecV)) {
    if (isa<PHINode>(VecInst))
      Builder.SetInsertPoint(&*(VecInst->getParent()->getFirstInsertionPt()));
    else
      Builder.SetInsertPoint(VecInst->getNextNode());
  }
  auto ScalarV = Builder.CreateExtractElement(VecV, Builder.getInt32(Lane));

  // Add to scalar map.
  VPScalarMap[V][Lane] = ScalarV;
  return ScalarV;
}

void VPOCodeGen::vectorizeExtractElement(VPInstruction *VPInst) {
  // Vector operand will be first operand of extractelement, index will be
  // second operand.
  Value *ExtrFrom = getVectorValue(VPInst->getOperand(0));
  VPValue *OrigIndexVal = VPInst->getOperand(1);

  // In case of a non-const index, we serialize the instruction.
  // We first get the actual index, for the vectorized data using
  // 'add', extract the element using the index and then finally insert it into
  // the narrower sub-vector.
  // Example -
  // %extract = extractelement <4 x float> %input, i32 %varidx
  // Vector IR for VF=2 -
  // %varidx1 = extractelement <2 x i32> %varidx.vec, i64 0
  // %offset1 = add i32 0, %varidx1
  // %res1 = extractelement <8 x float> %input.vec, i32 %offset1
  // %wide.extract1 = insertelement <2 x float> undef, float %res1, i64 0
  // %varidx2 = extractelement <2 x i32> %varidx.vec, i64 1
  // %offset2 = add i32 2, %varidx2
  // %res2 = extractelement <8 x float> %input.vec, i32 %offset2
  // %final = insertelement <2 x float> %wide.extract1, float %res2, i64 1
  if (!isa<VPConstant>(OrigIndexVal) ||
      !cast<VPConstant>(OrigIndexVal)->isConstantInt()) {
    Value *WideExtract =
        UndefValue::get(VectorType::get(VPInst->getType(), VF));
    Value *IndexValVec = getVectorValue(OrigIndexVal);
    for (unsigned VIdx = 0; VIdx < VF; ++VIdx) {
      Value *IndexVal = Builder.CreateExtractElement(IndexValVec, VIdx);
      Value *VectorIdx = Builder.CreateAdd(
          ConstantInt::get(IndexVal->getType(), VIdx * VF), IndexVal);
      WideExtract = Builder.CreateInsertElement(
          WideExtract, Builder.CreateExtractElement(ExtrFrom, VectorIdx), VIdx);
    }
    VPWidenMap[VPInst] = WideExtract;
    return;
  }

  VPConstant *OrigIndexVPConst = cast<VPConstant>(OrigIndexVal);
  assert(OrigIndexVPConst->isConstantInt() &&
         "Original index is not constant integer.");
  unsigned Index = OrigIndexVPConst->getZExtValue();

  // Extract subvector. The subvector should include VF elements.
  SmallVector<unsigned, 8> ShufMask;
  unsigned OriginalVL =
      VPInst->getOperand(0)->getType()->getVectorNumElements();
  unsigned WideNumElts = VF * OriginalVL;
  for (unsigned Idx = Index; Idx < WideNumElts; Idx += OriginalVL)
    ShufMask.push_back(Idx);
  Type *VTy = ExtrFrom->getType();
  VPWidenMap[VPInst] = Builder.CreateShuffleVector(
      ExtrFrom, UndefValue::get(VTy), ShufMask, "wide.extract");
}

void VPOCodeGen::vectorizeInsertElement(VPInstruction *VPInst) {
  Value *InsertTo = getVectorValue(VPInst->getOperand(0));
  Value *NewSubVec = getVectorValue(VPInst->getOperand(1));
  VPValue *OrigIndexVal = VPInst->getOperand(2);

  // In case of an non-const index, we serialize the instruction.
  // We first get the actual index, for the vectorized data using
  // 'add' and then insert that scalar into the index.

  if (!isa<VPConstant>(OrigIndexVal) ||
      !cast<VPConstant>(OrigIndexVal)->isConstantInt()) {
    assert(!MaskValue && "Masked insertelement vectorization for variable "
                         "index is not supported.");
    Value *WideInsert = InsertTo;
    Value *IndexValVec = getVectorValue(OrigIndexVal);
    for (unsigned VIdx = 0; VIdx < VF; ++VIdx) {
      Value *IndexVal = Builder.CreateExtractElement(IndexValVec, VIdx);
      Value *VectorIdx = Builder.CreateAdd(
          ConstantInt::get(IndexVal->getType(), VIdx * VF), IndexVal);
      // Insert the scalar value of second operand which can be vectorized
      // earlier.
      WideInsert = Builder.CreateInsertElement(
          WideInsert, getScalarValue(VPInst->getOperand(1), VIdx), VectorIdx);
    }
    VPWidenMap[VPInst] = WideInsert;
    return;
  }

  // TODO: Need more test coverage for vectorizing insertelement with const
  // index, especially masked insert scenarios.

  VPConstant *OrigIndexVPConst = cast<VPConstant>(OrigIndexVal);
  assert(OrigIndexVPConst->isConstantInt() &&
         "Original index is not constant integer.");
  unsigned Index = OrigIndexVPConst->getZExtValue();
  unsigned WideNumElts = InsertTo->getType()->getVectorNumElements();
  unsigned OriginalVL =
      VPInst->getOperand(0)->getType()->getVectorNumElements();

  // Widen the insert into an empty, undef-vector
  // E.g. For OriginalVL = 4 and VF = 2, the following code,
  // %add13 = add i32 %scalar, %scalar9
  // %assembled.vect = insertelement <4 x i32> undef, i32 %add13, i32 0
  //
  // is transformed into,
  // %6 = add <2 x i32> %Wide.Extract12, %Wide.Extract
  // %wide.insert = shufflevector <2 x i32> %6, <2 x i32> undef,
  //                              <8 x i32> <i32 0, i32 undef, i32 undef, i32
  //                                         undef,
  //                                         i32 1, i32 undef, i32 undef, i32
  //                                         undef>

  if (isa<UndefValue>(InsertTo)) {
    SmallVector<Constant *, 8> ShufMask;
    ShufMask.resize(WideNumElts, UndefValue::get(Builder.getInt32Ty()));
    for (size_t Lane = 0; Lane < VF; Lane++)
      ShufMask[Lane * OriginalVL + Index] = Builder.getInt32(Lane);

    Value *Shuf = Builder.CreateShuffleVector(
        NewSubVec, UndefValue::get(NewSubVec->getType()),
        ConstantVector::get(ShufMask), "wide.insert");
    VPWidenMap[VPInst] = Shuf;
    return;
  }

  // Generate two shuffles. The first one is extending the Subvector to the
  // width of the source and the second one is for blending in the actual
  // values.
  // In continuation of the example above, the following code,
  // %assembled.vect17 = insertelement <4 x i32> %assembled.vect, i32 %add14,
  //                                                              i32 1
  //
  // is transformed into,
  // %extended. = shufflevector <2 x i32> %7,
  //                            <2 x i32> undef,
  //                            <8 x i32> <i32 0, i32 1, i32 2, i32 2,
  //                                       i32 2, i32 2, i32 2, i32 2>
  // %wide.insert16 = shufflevector <8 x i32> %wide.insert,
  //                                <8 x i32> %extended.,
  //                                <8 x i32> <i32 0, i32 8, i32 2, i32 3,
  //                                           i32 4, i32 9, i32 6, i32 7>

  Value *ExtendSubVec =
      extendVector(NewSubVec, WideNumElts, Builder, NewSubVec->getName());
  SmallVector<unsigned, 8> ShufMask2;
  for (unsigned FirstVecIdx = 0, SecondVecIdx = WideNumElts;
       FirstVecIdx < WideNumElts; ++FirstVecIdx) {
    if ((FirstVecIdx % OriginalVL) == Index)
      ShufMask2.push_back(SecondVecIdx++);
    else
      ShufMask2.push_back(FirstVecIdx);
  }
  Value *SecondShuf = Builder.CreateShuffleVector(InsertTo, ExtendSubVec,
                                                  ShufMask2, "wide.insert");
  VPWidenMap[VPInst] = SecondShuf;
}

void VPOCodeGen::vectorizeShuffle(VPInstruction *VPInst) {
  unsigned OriginalVL =
      VPInst->getOperand(0)->getType()->getVectorNumElements();
  // Simple case - broadcast scalar elt into vector.
  if (getOrigSplatVPValue(VPInst)) {
    assert(isa<VPInstruction>(VPInst->getOperand(0)) &&
           "First operand of simple supported shuffle is not a VPInstruction.");
    assert(cast<VPInstruction>(VPInst->getOperand(0))->getOpcode() ==
               Instruction::InsertElement &&
           "First operand of simple supported shuffle is not insertelement");
    VPValue *SplVal = cast<VPInstruction>(VPInst->getOperand(0))->getOperand(1);
    Value *Vec = getVectorValue(SplVal);
    SmallVector<unsigned, 8> ShufMask;
    for (unsigned I = 0; I < OriginalVL; ++I)
      for (unsigned J = 0; J < VF; ++J)
        ShufMask.push_back(J);

    VPWidenMap[VPInst] = Builder.CreateShuffleVector(
        Vec, UndefValue::get(Vec->getType()), ShufMask);
    return;
  }

  Value *V0 = getVectorValue(VPInst->getOperand(0));

  // Mask of the shuffle is always the last operand is known to be constant.
  VPConstant *VPMask =
      cast<VPConstant>(VPInst->getOperand(VPInst->getNumOperands() - 1));
  Constant *Mask = cast<Constant>(getScalarValue(VPMask, 0 /*Lane*/));
  int InstVL = VPInst->getType()->getVectorNumElements();
  // All-zero mask case.
  if (isa<ConstantAggregateZero>(Mask)) {
    SmallVector<unsigned, 8> ShufMask;
    int Repeat = InstVL / OriginalVL;
    for (int K = 0; K < Repeat; K++)
      for (unsigned I = 0; I < OriginalVL; ++I)
        for (unsigned J = 0; J < VF; ++J)
          ShufMask.push_back(J + I);

    VPWidenMap[VPInst] = Builder.CreateShuffleVector(
        V0, UndefValue::get(V0->getType()), ShufMask);
    return;
  }
  // General case - whole mask should be recalculated.
  llvm_unreachable("Unsupported shuffle");
}

void VPOCodeGen::serializePredicatedUniformLoad(VPInstruction *VPInst) {
  assert(MaskValue->getType()->isVectorTy() &&
         MaskValue->getType()->getVectorNumElements() == VF &&
         "Unexpected Mask Type");
  // Emit not of all-zero check for mask
  Type *MaskTy = MaskValue->getType();
  Type *IntTy =
      IntegerType::get(MaskTy->getContext(), MaskTy->getPrimitiveSizeInBits());
  auto *MaskBitCast = Builder.CreateBitCast(MaskValue, IntTy);

  // Check if the bitcast value is not zero. The generated compare will be true
  // if atleast one of the i1 masks in <VF x i1> is true.
  auto *CmpInst =
      Builder.CreateICmpNE(MaskBitCast, Constant::getNullValue(IntTy));

  // Now create a scalar load, populating correct values for its operands.
  SmallVector<Value *, 4> ScalarOperands;
  for (unsigned Op = 0, e = VPInst->getNumOperands(); Op != e; ++Op) {
    auto *ScalarOp = getScalarValue(VPInst->getOperand(Op), 0 /*Lane*/);
    assert(ScalarOp && "Operand for serialized uniform load not found.");
    ScalarOperands.push_back(ScalarOp);
  }

  Value *SerialLoad =
      generateSerialInstruction(Builder, VPInst, ScalarOperands);
  VPScalarMap[VPInst][0] = SerialLoad;

  PredicatedInstructions.push_back(
      std::make_pair(cast<Instruction>(SerialLoad), CmpInst));
}

void VPOCodeGen::serializeWithPredication(VPInstruction *VPInst) {
  if (!MaskValue)
    return serializeInstruction(VPInst);

  assert(MaskValue->getType()->isVectorTy() &&
         MaskValue->getType()->getVectorNumElements() == VF &&
         "Unexpected Mask Type");

  for (unsigned Lane = 0; Lane < VF; ++Lane) {
    Value *Cmp = Builder.CreateExtractElement(MaskValue, Lane, "Predicate");
    Cmp = Builder.CreateICmp(ICmpInst::ICMP_EQ, Cmp,
                             ConstantInt::get(Cmp->getType(), 1));

    SmallVector<Value *, 4> ScalarOperands;
    // All operands to the serialized Instruction should be original loop
    // Values.
    for (unsigned Op = 0, e = VPInst->getNumOperands(); Op != e; ++Op) {
      auto *ScalarOp = getScalarValue(VPInst->getOperand(Op), Lane);
      assert(ScalarOp && "Operand for serialized instruction not found.");
      LLVM_DEBUG(dbgs() << "LVCG: Serialize scalar op: "; ScalarOp->dump());
      ScalarOperands.push_back(ScalarOp);
    }

    Value *SerialInst =
        generateSerialInstruction(Builder, VPInst, ScalarOperands);
    assert(SerialInst && "Instruction not serialized.");
    VPScalarMap[VPInst][Lane] = SerialInst;
    PredicatedInstructions.push_back(
        std::make_pair(cast<Instruction>(SerialInst), Cmp));
    LLVM_DEBUG(dbgs() << "LVCG: SerialInst: "; SerialInst->dump());
  }
}

void VPOCodeGen::serializeInstruction(VPInstruction *VPInst) {
  // TODO: Handle serialization of aggregate type instructions.
  assert(!VPInst->getType()->isAggregateType() &&
         "Can't serialize aggregate type instructions.");

  auto HasLoopInvariantOperands = [&](const VPInstruction *VPI) {
    return all_of(VPI->operands(),
                  [&](VPValue *V) { return isVPValueUniform(V, Plan); });
  };

  // TODO: Currently using DA for HasLoopInvariantOperands
  // TODO: Handle cases like - call i32 random_number_generator(void)
  unsigned Lanes =
      HasLoopInvariantOperands(VPInst) || isVPValueUniform(VPInst, Plan) ? 1
                                                                         : VF;

  for (unsigned Lane = 0; Lane < Lanes; ++Lane) {
    SmallVector<Value *, 4> ScalarOperands;
    // All operands to the serialized Instruction should be scalar Values.
    for (unsigned Op = 0, e = VPInst->getNumOperands(); Op != e; ++Op) {
      auto *ScalarOp = getScalarValue(VPInst->getOperand(Op), Lane);
      assert(ScalarOp && "Operand for serialized instruction not found.");
      LLVM_DEBUG(dbgs() << "LVCG: Serialize scalar op: "; ScalarOp->dump());
      ScalarOperands.push_back(ScalarOp);
    }

    Value *SerialInst =
        generateSerialInstruction(Builder, VPInst, ScalarOperands);
    assert(SerialInst && "Instruction not serialized.");
    VPScalarMap[VPInst][Lane] = SerialInst;
    LLVM_DEBUG(dbgs() << "LVCG: SerialInst: "; SerialInst->dump());
  }
}

void VPOCodeGen::vectorizeVPInstruction(VPInstruction *VPInst) {
  switch (VPInst->getOpcode()) {
  case Instruction::GetElementPtr: {
    // Consecutive Load/Store will clone the GEP.
    if (all_of(VPInst->users(),
               [&](VPUser *U) -> bool {
                 return getLoadStorePointerOperand(U) == VPInst;
               }) &&
        isVPValueConsecutivePtr(VPInst))
      break;
    if (all_of(VPInst->users(), [&](VPUser *U) -> bool {
          return getLoadStorePointerOperand(U) == VPInst &&
                 isVPValueUniform(U, Plan);
        })) {
      serializeInstruction(VPInst);
      return;
    }

    // Create the vector GEP, keeping all constant arguments scalar.
    bool AllGEPOpsUniform = false;
    VPGEPInstruction *GEP = cast<VPGEPInstruction>(VPInst);
    if (all_of(GEP->operands(), [&](VPValue *Op) -> bool {
          // TODO: Using DA for loop invariance.
          return isVPValueUniform(Op, Plan) && Op->getUnderlyingValue() &&
                 !Legal->isLoopPrivate(Op->getUnderlyingValue());
        })) {
      AllGEPOpsUniform = true;
    }

    SmallVector<Value *, 4> OpsV;

    for (VPValue *Op : GEP->operands())
      OpsV.push_back(AllGEPOpsUniform ? getScalarValue(Op, 0)
                                      : getVectorValue(Op));

    Value *GepBasePtr = OpsV[0];
    OpsV.erase(OpsV.begin());
    Value *VectorGEP = Builder.CreateGEP(GepBasePtr, OpsV, "mm_vectorGEP");
    cast<GetElementPtrInst>(VectorGEP)->setIsInBounds(GEP->isInBounds());

    // We need to bcast the scalar GEP to all lanes if all its operands were
    // uniform.
    if (AllGEPOpsUniform)
      VectorGEP = Builder.CreateVectorSplat(VF, VectorGEP);

    VPWidenMap[VPInst] = VectorGEP;
    return;
  }

  case Instruction::ZExt:
  case Instruction::SExt:
  case Instruction::FPToUI:
  case Instruction::FPToSI:
  case Instruction::FPExt:
  case Instruction::PtrToInt:
  case Instruction::IntToPtr:
  case Instruction::SIToFP:
  case Instruction::UIToFP:
  case Instruction::Trunc:
  case Instruction::FPTrunc: {
    auto Opcode = static_cast<Instruction::CastOps>(VPInst->getOpcode());

    /// Vectorize casts.
    Type *ScalTy = VPInst->getType();
    Type *VecTy = VectorType::get(ScalTy, VF);
    VPValue *ScalOp = VPInst->getOperand(0);
    Value *VecOp = getVectorValue(ScalOp);
    VPWidenMap[VPInst] = Builder.CreateCast(Opcode, VecOp, VecTy);

    // If the cast is a SExt/ZExt of a unit step linear item, add the cast value
    // to UnitStepLinears - so that we can use it to infer information about
    // unit stride loads/stores.
    Value *NewScalar;
    int LinStep;

    if ((Opcode == Instruction::SExt || Opcode == Instruction::ZExt) &&
        isVPValueUnitStepLinear(ScalOp, &LinStep, &NewScalar)) {
      // NewScalar is the scalar linear iterm corresponding to ScalOp - apply
      // cast.
      auto ScalCast = Builder.CreateCast(Opcode, NewScalar, ScalTy);
      (void)ScalCast;
      // TODO: Mark the new Value as unit-step linear.
#if 0
      addUnitStepLinear(Inst, ScalCast, LinStep);
#endif
    }
    return;
  }

  case Instruction::Add:
  case Instruction::FAdd:
  case Instruction::Sub:
  case Instruction::FSub:
  case Instruction::Mul:
  case Instruction::FMul:
  case Instruction::UDiv:
  case Instruction::SDiv:
  case Instruction::FDiv:
  case Instruction::URem:
  case Instruction::SRem:
  case Instruction::FRem:
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor: {

    if (isVPValueUniform(VPInst, Plan)) {
      serializeInstruction(VPInst);
      return;
    }
    // Widen binary operands.
    Value *A = getVectorValue(VPInst->getOperand(0));
    Value *B = getVectorValue(VPInst->getOperand(1));

    // Create wide instruction.
    auto BinOpCode = static_cast<Instruction::BinaryOps>(VPInst->getOpcode());
    Value *V = Builder.CreateBinOp(BinOpCode, A, B);

    // TODO: IR flags are not stored in VPInstruction (example FMF, wrapping
    // flags). Use underlying IR flags if any.
    if (VPInst->getUnderlyingValue()) {
      BinaryOperator *BinOp =
          cast<BinaryOperator>(VPInst->getUnderlyingValue());
      BinaryOperator *VecOp = cast<BinaryOperator>(V);
      VecOp->copyIRFlags(BinOp);
    }

    VPWidenMap[VPInst] = V;
    return;
  }

  case Instruction::ICmp: {
    Value *A = getVectorValue(VPInst->getOperand(0));
    Value *B = getVectorValue(VPInst->getOperand(1));
    auto *Cmp = cast<VPCmpInst>(VPInst);
    VPWidenMap[VPInst] = Builder.CreateICmp(Cmp->getPredicate(), A, B);
    return;
  }
  case Instruction::FCmp: {
    Value *A = getVectorValue(VPInst->getOperand(0));
    Value *B = getVectorValue(VPInst->getOperand(1));
    auto *FCmp = cast<VPCmpInst>(VPInst);
    Value *VecFCmp = Builder.CreateFCmp(FCmp->getPredicate(), A, B);
    // TODO: Copy fast math flags. Currently not represented in VPlan. Use
    // underlying IR flags if any.
    if (VPInst->getUnderlyingValue()) {
      FCmpInst *UnderlyingFCmp = cast<FCmpInst>(VPInst->getUnderlyingValue());
      cast<FCmpInst>(VecFCmp)->copyFastMathFlags(UnderlyingFCmp);
    }
    VPWidenMap[VPInst] = VecFCmp;
    return;
  }
  case Instruction::Load: {
    vectorizeLoadInstruction(VPInst, true);
    return;
  }
  case Instruction::Store: {
    vectorizeStoreInstruction(VPInst, true);
    return;
  }
  case Instruction::Select: {
    vectorizeSelectInstruction(VPInst);
    return;
  }
  case Instruction::BitCast: {
    vectorizeCast<BitCastInst>(VPInst);
    return;
  }
  case Instruction::AddrSpaceCast: {
    vectorizeCast<AddrSpaceCastInst>(VPInst);
    return;
  }
  case Instruction::ExtractElement: {
    vectorizeExtractElement(VPInst);
    return;
  }
  case Instruction::InsertElement: {
    vectorizeInsertElement(VPInst);
    return;
  }
  case Instruction::ShuffleVector: {
    vectorizeShuffle(VPInst);
    return;
  }

  case Instruction::Call: {
    CallInst *UnderlyingCI = dyn_cast<CallInst>(VPInst->getUnderlyingValue());
    assert(UnderlyingCI &&
           "VPVALCG: Need underlying CallInst for call-site attributes.");
    Function *F = getCalledFunction(VPInst);

    if (!F) {
      // Indirect calls.
      serializeWithPredication(VPInst);
      return;
    }

    assert(F && "Unexpected null called function");
    LLVM_DEBUG(dbgs() << "VPVALCG: Called Function: "; F->dump());
    StringRef CalledFunc = F->getName();
    bool isMasked = (MaskValue != nullptr) ? true : false;
    if (TLI->isFunctionVectorizable(CalledFunc, VF) ||
        ((matchVectorVariant(UnderlyingCI, isMasked) ||
          (!isMasked && matchVectorVariant(UnderlyingCI, true)))) ||
        (isOpenCLReadChannel(CalledFunc) || isOpenCLWriteChannel(CalledFunc))) {
      vectorizeCallInstruction(VPInst);
    } else {
      LLVM_DEBUG(dbgs() << "Function " << CalledFunc << " is serialized\n");
      serializeWithPredication(VPInst);
    }
    return;
  }

  case VPInstruction::AllZeroCheck: {
    Value *A = getVectorValue(VPInst->getOperand(0));
    Type *Ty = A->getType();

    // Bitcast <VF x i1> to an integer value VF bits long.
    Type *IntTy =
        IntegerType::get(Ty->getContext(), Ty->getPrimitiveSizeInBits());
    auto *BitCastInst = Builder.CreateBitCast(A, IntTy);

    // Compare the bitcast value to zero. The compare will be true if all
    // the i1 masks in <VF x i1> are false.
    auto *CmpInst =
        Builder.CreateICmpEQ(BitCastInst, Constant::getNullValue(IntTy));

    // Broadcast the compare and set as the widened value.
    auto *V = getBroadcastInstrs(CmpInst);
    VPWidenMap[VPInst] = V;
    return;
  }
  case VPInstruction::Not: {
    Value *A = getVectorValue(VPInst->getOperand(0));
    Value *V = Builder.CreateNot(A);
    VPWidenMap[VPInst] = V;
    return;
  }
  case VPInstruction::Pred: {
    // Pred instruction just marks the block mask.
    Value *A = getVectorValue(VPInst->getOperand(0));
    setMaskValue(A);
    return;
  }
  default: {
    LLVM_DEBUG(dbgs() << "VPInst: "; VPInst->dump());
    llvm_unreachable("VPVALCG: Opcode not uplifted yet.");
  }
  }
}
