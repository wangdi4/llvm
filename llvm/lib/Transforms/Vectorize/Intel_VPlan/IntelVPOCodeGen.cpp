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

static cl::opt<bool> VPlanUseDAForUnitStride(
    "vplan-use-da-unit-stride-accesses", cl::init(true), cl::Hidden,
    cl::desc("Use DA knowledge in VPlan for unit-stride accesses."));

///////// VPValue version of common vectorizer legality utilities /////////

/// Helper function to check if given VPValue has consecutive pointer stride and
/// return the stride value.
static Optional<int> getVPValueConsecutivePtrStride(const VPValue *Ptr,
                                                    const VPlan *Plan) {
  VPVectorShape *PtrShape = Plan->getVPlanDA()->getVectorShape(Ptr);
  if (PtrShape->isUnitStridePtr())
    return PtrShape->getStrideVal();

  return None;
}

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
      if (isOpenCLSelectMask(FnName, OrigArgIdx))
        return getOpenCLSelectVectorMask(Arg);

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
  // TODO: Peeking at underlying Value for alignment info.
  auto *UV = VPInst->getUnderlyingValue();
  if (!UV)
    return 1;

  const DataLayout &DL = OrigLoop->getHeader()->getModule()->getDataLayout();
  // For store instructions alignment is determined by type of value operand.
  Type *OrigTy = VPInst->getOpcode() == Instruction::Load
                     ? VPInst->getType()
                     : VPInst->getOperand(0)->getType();

 // Absence of alignment means target abi alignment. We need to use the scalar's
 // target abi alignment in such a case.
  return DL.getValueOrABITypeAlignment(getLoadStoreAlignment(UV), OrigTy)
      .value();
}

Value *VPOCodeGen::getOrCreateWideLoadForGroup(OVLSGroup *Group) {
  auto FoundIter = VLSGroupLoadMap.find(Group);
  if (FoundIter != VLSGroupLoadMap.end())
    return FoundIter->second;

  // Check if the group is valid for this VF.
  assert(Group->getNumElems() == VF &&
         "Group number of elements must match VF");

  OVLSMemref *InsertPoint = Group->getInsertPoint();
  int InterleaveIndex = computeInterleaveIndex(InsertPoint, Group);
  const VPInstruction *Leader =
      cast<VPVLSClientMemref>(InsertPoint)->getInstruction();

  Type *GroupType =
      getWidenedType(getLoadStoreType(Leader), VF * Group->size());

  Value *GatherAddress = getVectorValue(Leader->getOperand(0));
  assert(!MaskValue && "Scalar address may be invalid (masked out)");
  Value *ScalarAddress = Builder.CreateExtractElement(
      GatherAddress, (uint64_t)0, GatherAddress->getName() + "_0");

  // If the group leader (lexically first load) does not access the lowest
  // memory address address (InterleaveIndex != 0), we should adjust
  // ScalarAddress to make it point to the beginning of the Group.
  if (InterleaveIndex != 0)
    ScalarAddress = Builder.CreateConstInBoundsGEP1_64(
        ScalarAddress, -InterleaveIndex, "groupStart");

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

  assert((Group->getInsertPoint() == Memref ||
          VLSGroupLoadMap.find(Group) != VLSGroupLoadMap.end()) &&
         "Wide load must be emitted at the group insertion point");
  Value *GroupLoad = getOrCreateWideLoadForGroup(Group);

  // Extract a proper widened value from the wide load. A bit more sophisticated
  // shuffle mask is required if the input type is itself a vector type.
  Type *Ty = VPLoad->getType();
  unsigned OriginalVL = Ty->isVectorTy() ? Ty->getVectorNumElements() : 1;
  Constant *ShuffleMask = createVectorStrideMask(
      Builder, InterleaveIndex, InterleaveFactor, VF, OriginalVL);
  return Builder.CreateShuffleVector(GroupLoad,
                                     UndefValue::get(GroupLoad->getType()),
                                     ShuffleMask, "groupShuffle");
}

Value *VPOCodeGen::vectorizeUnitStrideLoad(VPInstruction *VPInst, int StrideVal,
                                           bool IsPvtPtr) {
  Value *WideLoad = nullptr;
  VPValue *Ptr = getLoadStorePointerOperand(VPInst);
  Type *LoadType = getLoadStoreType(VPInst);
  unsigned OriginalVL =
      LoadType->isVectorTy() ? LoadType->getVectorNumElements() : 1;
  unsigned Alignment = getOriginalLoadStoreAlignment(VPInst);
  Value *VecPtr = createWidenedBasePtrConsecutiveLoadStore(Ptr, StrideVal < 0);

  // Masking not needed for privates.
  if (MaskValue && !IsPvtPtr) {
    // Replicate the mask if VPInst is a vector instruction.
    Value *RepMaskValue = replicateVectorElts(MaskValue, OriginalVL, Builder,
                                              "replicatedMaskElts.");
    WideLoad = Builder.CreateMaskedLoad(VecPtr, Alignment, RepMaskValue,
                                        nullptr, "wide.masked.load");
  } else
    WideLoad = Builder.CreateAlignedLoad(VecPtr, Alignment, "wide.load");

  if (StrideVal < 0) // Reverse
    WideLoad = reverseVector(WideLoad);

  return WideLoad;
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

  if (isa<VPAllocatePrivate>(Ptr)) {
    // TODO. Need to handshake with VLS. No sure if VLS currently operates
    // on privates. At least it should account SOA property of private.
    Value *VecPtr = getVectorValue(Ptr);
    unsigned Alignment = 0;
    if (auto PrivAlloca = dyn_cast<AllocaInst>(VecPtr))
      Alignment = PrivAlloca->getAlignment();
    if (MaskValue)
      VPWidenMap[VPInst] =
          Builder.CreateMaskedLoad(VecPtr, Alignment, MaskValue,
                                   nullptr /*PassThru*/, "wide.masked.load");
    else
      VPWidenMap[VPInst] =
          Builder.CreateAlignedLoad(VecPtr, Alignment, "wide.load");
    return;
  }

  unsigned OriginalVL =
      LoadType->isVectorTy() ? LoadType->getVectorNumElements() : 1;

  unsigned Alignment = getOriginalLoadStoreAlignment(VPInst);
  Value *NewLI = nullptr;

  // Try to handle consecutive loads without VLS.
  if (VPlanUseDAForUnitStride) {
    Optional<int> ConsecutiveStride = getVPValueConsecutivePtrStride(Ptr, Plan);
    // TODO: VPVALCG: Unit strided load from private pointer not implemented
    // yet.
    bool IsPvtPtr = Ptr->getUnderlyingValue() &&
                    Legal->isLoopPrivate(Ptr->getUnderlyingValue());

    if (ConsecutiveStride && !IsPvtPtr) {
      NewLI = vectorizeUnitStrideLoad(VPInst, ConsecutiveStride.getValue(),
                                      IsPvtPtr);
      VPWidenMap[VPInst] = NewLI;
      return;
    }
  }

  // Try to do GATHER-to-SHUFFLE optimization.
  // TODO: VLS optimization is disabled in masked basic blocks so far. It
  // should be enabled for uniform masks, though.
  OVLSGroup *Group = MaskValue ? nullptr : VLSA->getGroupsFor(Plan, VPInst);
  if (Group && Group->size() > 1) {
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

  VPWidenMap[VPInst] = NewLI;
}

void VPOCodeGen::vectorizeInterleavedStore(VPInstruction *VPStore,
                                           OVLSGroup *Group) {
  // Don't do anything unless we're vectorizing an instruction that is the group
  // insertion point.
  auto *Memref = cast<VPVLSClientMemref>(Group->getInsertPoint());
  if (Memref->getInstruction() != VPStore)
    return;

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

  // Shuffle values into the correct order. A bit more sophisticated shuffle
  // mask is required if the original type is itself a vector type.
  Type *Ty = VPStore->getOperand(0)->getType();
  unsigned OriginalVL = Ty->isVectorTy() ? Ty->getVectorNumElements() : 1;
  Constant *ShuffleMask =
      createVectorInterleaveMask(Builder, VF, InterleaveFactor, OriginalVL);
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
  Builder.CreateAlignedStore(StoredValue, GroupPtr, Align);
}

void VPOCodeGen::vectorizeUnitStrideStore(VPInstruction *VPInst, int StrideVal,
                                          bool IsPvtPtr) {
  VPValue *Ptr = getLoadStorePointerOperand(VPInst);
  Value *VecDataOp = getVectorValue(VPInst->getOperand(0));
  Type *StoreType = getLoadStoreType(VPInst);
  unsigned OriginalVL =
      StoreType->isVectorTy() ? StoreType->getVectorNumElements() : 1;
  unsigned Alignment = getOriginalLoadStoreAlignment(VPInst);
  Value *VecPtr = createWidenedBasePtrConsecutiveLoadStore(Ptr, StrideVal < 0);

  if (StrideVal < 0) // Reverse
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

  if (isa<VPAllocatePrivate>(Ptr)) {
    // TODO. Need to handshake with VLS. No sure if VLS currently operates
    // on privates. At least it should account SOA property of private.
    Value *VecPtr = getVectorValue(Ptr);
    unsigned Alignment = 0;
    if (auto PrivAlloca = dyn_cast<AllocaInst>(VecPtr))
      Alignment = PrivAlloca->getAlignment();
    Value *VecDataOp = getVectorValue(VPInst->getOperand(0));
    if (MaskValue)
      Builder.CreateMaskedStore(VecDataOp, VecPtr, Alignment, MaskValue);
    else
      Builder.CreateAlignedStore(VecDataOp, VecPtr, Alignment);
    return;
  } else if (isa<VPExternalDef>(Ptr) ||
             (isa<VPConstant>(Ptr) && Ptr->getType()->isPointerTy())) {
    // Scalar store
    Value *ScalarPtr = getScalarValue(Ptr, 0);
    Value *ScalarVal = getScalarValue(VPInst->getOperand(0), 0);
    Builder.CreateAlignedStore(ScalarVal, ScalarPtr, 0);
    return;
  }

  unsigned OriginalVL =
      StoreType->isVectorTy() ? StoreType->getVectorNumElements() : 1;

  unsigned Alignment = getOriginalLoadStoreAlignment(VPInst);
  Value *VecDataOp = getVectorValue(VPInst->getOperand(0));

  // Try to handle consecutive stores without VLS.
  if (VPlanUseDAForUnitStride) {
    Optional<int> ConsecutiveStride = getVPValueConsecutivePtrStride(Ptr, Plan);
    // TODO: VPVALCG: Unit strided store to private pointer not implemented yet.
    // Special handling for mask value is also needed for conditional last
    // privates.
    bool IsPvtPtr = Ptr->getUnderlyingValue() &&
                    Legal->isLoopPrivate(Ptr->getUnderlyingValue());
    if (ConsecutiveStride && !IsPvtPtr) {
      vectorizeUnitStrideStore(VPInst, ConsecutiveStride.getValue(), IsPvtPtr);
      return;
    }
  }

  // Try to do SCATTER-to-SHUFFLE optimization.
  // TODO: VLS optimization is disabled in masked basic blocks so far. It
  // should be enabled for uniform masks, though.
  OVLSGroup *Group = MaskValue ? nullptr : VLSA->getGroupsFor(Plan, VPInst);
  if (Group && Group->size() > 1) {
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

// This function return an appropriate BasePtr for cases where we are dealing
// with load/store to consecutive memory locations
Value *VPOCodeGen::createWidenedBasePtrConsecutiveLoadStore(VPValue *Ptr,
                                                            bool Reverse) {
  Type *VecTy = Ptr->getType()->getPointerElementType();
  unsigned AddrSpace = Ptr->getType()->getPointerAddressSpace();
  Type *WideDataTy = getWidenedType(VecTy, VF);

  // TODO: Peeking at underlying IR for privates.
  // TODO: Support loop private pointers here.
  assert(!(Ptr->getUnderlyingValue() &&
           Legal->isLoopPrivate(Ptr->getUnderlyingValue())) &&
         "VPVALCG: Widened base ptr for private unit stride load/store not "
         "uplifted yet.");
  // We do not care whether the 'Ptr' operand comes from a GEP or any other
  // source. We just fetch the first element and then create a
  // bitcast  which assumes the 'consecutive-ness' property and return the
  // correct operand for widened load/store.
  Value *VecPtr = getScalarValue(Ptr, 0);

  VecPtr = Reverse
               ? Builder.CreateGEP(
                     VecPtr,
                     Builder.getInt32(1 - WideDataTy->getVectorNumElements()))
               : VecPtr;
  VecPtr = Builder.CreateBitCast(VecPtr, WideDataTy->getPointerTo(AddrSpace));
  return VecPtr;
}

// Return the right vector mask for a OpenCL vector select built-in.
//
// Definition of OpenCL select intrinsic:
//   gentype select ( gentype a, gentype b, igentype c)
//
//   For each component of a vector type, result[i] = if MSB of c[i] is set ?
//   b[i] : a[i] For scalar type, result = c ? b : a.
//
// Scalar select built-in uses integer mask (integer != 0 means true). However,
// vector select built-in uses the MSB of each vector element.
//
// Returned vector mask depends on ScalarMask as follows:
//   1) if ScalarMask == ZExt(i1), return widened SExt.
//   2) if ScalarMask == SExt(i1), return widened SExt.
//   3) Otherwise, return SExt(VectorMask != 0).
//
Value *VPOCodeGen::getOpenCLSelectVectorMask(VPValue *ScalarMask) {

  Type *ScTy = ScalarMask->getType();
  assert(!ScTy->isVectorTy() && ScTy->isIntegerTy() &&
         "Scalar integer type expected.");
  Type *VecTy = getWidenedType(ScTy, VF);

  // Special cases for i1 type.
  VPInstruction *VPInst = dyn_cast<VPInstruction>(ScalarMask);
  if (VPInst && VPInst->isCast() &&
      VPInst->getOperand(0)->getType()->isIntegerTy(1 /*i1*/)) {
    if (VPInst->getOpcode() == Instruction::SExt) {
      // SExt mask doesn't need to be fixed.
      return getVectorValue(ScalarMask);
    } else if (VPInst->getOpcode() == Instruction::ZExt) {
      // ZExt is replaced by an SExt.
      Value *Val = getVectorValue(VPInst->getOperand(0));
      return Builder.CreateSExt(Val, VecTy);
    }
  }

  // General case. We generate a SExt(VectorMask != 0).
  // TODO: Look at Volcano vectorizer, file OCLBuiltinPreVectorizationPass.cpp.
  // It is doing something different, creating a fake call to a built-in
  // intrinsic. I don't know if that approach is applicable here at this point.
  Value *VectorMask = getVectorValue(ScalarMask);
  Constant *Zero = Constant::getNullValue(VecTy);

  // Only integer mask is supported.
  Value *Cmp = Builder.CreateICmpNE(VectorMask, Zero);
  return Builder.CreateSExt(Cmp, VecTy);
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

  // Get the scalar value by extracting from the vector instruction based on the
  // requested lane.
  Value *VecV = getVectorValue(V);

  // This code assumes that the widened vector, that we are extracting from has
  // data in AOS layout. If OriginalVL = 2, VF = 4 the widened value would be
  // Wide.Val = <v1_0, v2_0, v1_1, v2_1, v1_2, v2_2, v1_3, v2_3>.
  // getScalarValue(Wide.Val, 1) would return <v1_1, v2_1>
  if (V->getType()->isVectorTy()) {
    unsigned OrigNumElts = V->getType()->getVectorNumElements();
    SmallVector<unsigned, 8> ShufMask;
    for (unsigned StartIdx = Lane * OrigNumElts,
                  EndIdx = (Lane * OrigNumElts) + OrigNumElts;
         StartIdx != EndIdx; ++StartIdx)
      ShufMask.push_back(StartIdx);

    Value *Shuff = Builder.CreateShuffleVector(
        VecV, UndefValue::get(VecV->getType()), ShufMask,
        "extractsubvec.");

    VPScalarMap[V][Lane] = Shuff;

    return Shuff;
  }

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

  unsigned Lanes =
      !VPInst->mayHaveSideEffects() && isVPValueUniform(VPInst, Plan) ? 1 : VF;

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
    // For consecutive load/store we create a scalar GEP.
    // TODO: Extend support for private pointers and VLS-based unit-stride
    // optimization.
    bool IsPvtPtr = VPInst->getUnderlyingValue() &&
                    Legal->isLoopPrivate(VPInst->getUnderlyingValue());
    if (all_of(VPInst->users(),
               [&](VPUser *U) -> bool {
                 return getLoadStorePointerOperand(U) == VPInst;
               }) &&
        getVPValueConsecutivePtrStride(VPInst, Plan) &&
        VPlanUseDAForUnitStride && !IsPvtPtr) {
      SmallVector<Value *, 6> ScalarOperands;
      for (unsigned Op = 0; Op < VPInst->getNumOperands(); ++Op) {
        auto *ScalarOp = getScalarValue(VPInst->getOperand(Op), 0 /*Lane*/);
        assert(ScalarOp && "Operand for scalar GEP not found.");
        ScalarOperands.push_back(ScalarOp);
      }

      Value *ScalarGep =
          generateSerialInstruction(Builder, VPInst, ScalarOperands);
      ScalarGep->setName("scalar.gep");
      VPScalarMap[VPInst][0] = ScalarGep;
      break;
    }
    // Serialize if all users of GEP are uniform load/store.
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
    // flags). Use underlying IR flags if any
    if (auto *IRValue = VPInst->getUnderlyingValue()) {
      BinaryOperator *BinOp = cast<BinaryOperator>(IRValue);
      BinaryOperator *VecOp = cast<BinaryOperator>(V);
      VecOp->copyIRFlags(BinOp);
    }
    VPWidenMap[VPInst] = V;
    // TODO: Need to check for scalar code generation for the most of
    // VPInstructions.
    if (needScalarCode(VPInst)) {
      Value *AScal = getScalarValue(VPInst->getOperand(0), 0);
      Value *BScal = getScalarValue(VPInst->getOperand(1), 0);
      Value *VScal = Builder.CreateBinOp(
          static_cast<Instruction::BinaryOps>(VPInst->getOpcode()), AScal,
          BScal);
      VPScalarMap[VPInst][0] = VScal;
      if (auto Underlying = VPInst->getUnderlyingValue()) {
        if (auto Inst = dyn_cast<Instruction>(VScal))
          Inst->copyIRFlags(Underlying);
      }
    }
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

    if (MaskValue) {
      // Consider the inner loop that is executed under the mask, after loop CFU
      // transformation and predication it looks something like this:
      //
      //   REGION: loop19 (BP: NULL)
      //   BB16 (BP: NULL) :
      //    i1 %vp24064 = block-predicate i1 %loop_incoming_mask
      //   SUCCESSORS(1):BB11
      //   no PREDECESSORS

      //   BB11 (BP: NULL) :
      //    i1 %vp24384 = block-predicate i1 %loop_incoming_mask
      //    i64 %vp54864 = phi  [ i64 %vp20544, BB23 ],  [ i64 1, BB16 ]
      //    i32 %vp55072 = phi  [ i32 %vp20704, BB23 ],  [ i32 %vp49616, BB16 ]
      //    i1 %inner_loop_specific_mask =
      //         phi  [ i1 true, BB16 ],
      //              [ i1 %inner_loop_specific_mask.next, BB23 ]
      //   SUCCESSORS(1):mask_region24
      //   PREDECESSORS(2): BB16 BB23

      //   REGION: mask_region24 (BP: NULL)
      //   BB21 (BP: NULL) :
      //    i1 %vp24544 = block-predicate i1 %loop_incoming_mask
      //   SUCCESSORS(1):BB22
      //   no PREDECESSORS

      //   BB22 (BP: NULL) :
      //    i1 %real_mask = and i1 %loop_incoming_mask i1, %inner_loop_specific_mask
      //    i1 %real_mask_predicate = block-predicate i1 %real_mask
      //    i32 addrspace(1)* %vp38496 =
      //        getelementptr inbounds i32 addrspace(1)* %input i64 %vp54864
      //    i32 %ld = load i32 addrspace(1)* %vp38496
      //    i1 %vp55440 = icmp i32 %vp55072 i32 %ld
      //    i32 %vp55616 =  select i1 %vp55440 i32 %ld i32 %vp55072
      //    i64 %vp55888 = add i64 %vp54864 i64 1
      //   SUCCESSORS(1):BB17
      //   PREDECESSORS(1): BB21

      //   BB17 (BP: NULL) :
      //    i1 %vp25472 = not i1 %inner_loop_specific_mask
      //    i1 %vp25632 = and i1 %loop_incoming_mask i1 %vp25472
      //    i1 %vp25920 = block-predicate i1 %loop_incoming_mask
      //    i1 %vp56048 = icmp i64 %vp55888 i64 %vp1824
      //
      //    ;; This "not" is because original latch was at false successor
      //    i1 %continue_cond = not i1 %vp56048
      //    i1 %inner_loop_specific_mask.next =
      //       and i1 %continue_cond i1  %inner_loop_specific_mask
      //
      //    ;; Live-outs updates
      //
      //   i1 %vp20864 = all-zero-check i1 %inner_loop_specific_mask.next
      //   no SUCCESSORS
      //   PREDECESSORS(1): BB22
      //
      // After vectorizing the loop above we create something like
      //
      //    %wide.ld = gather %vector_gep, %real_mask, undef_vector
      //
      // All-zero-check is dependent on the result of this gather, including the
      // lanes that were masked out by %real_mask (which includes
      // %loop_incoming_mask). That means that for lanes masked out by
      // %loop_incoming_mask we can only have undef values and naive
      //
      //   %vp1 bitcast <VF x i1> %innerl_loop_specific_mask.next to iVF
      //   %should_exit %cmp %vp1, 0
      //   br i1 %should_exit, %exit_bb, %header_bb
      //
      // would result in branching based on that undef, which is UB. To avoid
      // this, use only the active lanes when calculating the all-zero-check.
      // Note, that technically we can use either %loop_incoming_mask or
      // %real_mask. The former is easily available, so use it. Also,
      //
      //   and undef, %vpval
      //
      // semantics isn't immediately obvious for the reader.
      //
      // Another approach to this issue is to modify Predicator/LoopCFU to have
      // a single phi/value for the mask, but it looks like much more work.
      // Changing the semantics of the all-zero-check VPInstruction to reflect
      // the mask (similar to loads/stores/calls) doesn't seem to have any
      // drawbacks and is much easier to do.
      A = Builder.CreateAnd(A, MaskValue);
    }

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
    if (needScalarCode(VPInst)) {
      Value *AScal = getScalarValue(VPInst->getOperand(0), 0);
      Value *VScal = Builder.CreateNot(AScal);
      VPScalarMap[VPInst][0] = VScal;
    }
    return;
  }
  case VPInstruction::Pred: {
    // Pred instruction just marks the block mask.
    Value *A = getVectorValue(VPInst->getOperand(0));
    setMaskValue(A);
    return;
  }
  case VPInstruction::ReductionInit: {
    // Generate a broadcast/splat of reduction's identity. If a start value is
    // specified for reduction, then insert into lane 0 after broadcast.
    // Example -
    // i32 %vp0 = reduction-init i32 0 i32 %red.init
    //
    // Generated instructions-
    // %0 = insertelement <4 x i32> zeroinitializer, i32 %red.init, i32 0

    Value *Identity = getVectorValue(VPInst->getOperand(0));
    if (VPInst->getNumOperands() > 1) {
      auto *StartVPVal = VPInst->getOperand(1);
      assert((isa<VPExternalDef>(StartVPVal) || isa<VPConstant>(StartVPVal)) &&
             "Unsupported reduction StartValue");
      auto *StartVal = getScalarValue(StartVPVal, 0);
      Identity = Builder.CreateInsertElement(
          Identity, StartVal, Builder.getInt32(0), "red.init.insert");
    }
    VPWidenMap[VPInst] = Identity;
    return;
  }
  case VPInstruction::ReductionFinal: {
    vectorizeReductionFinal(cast<VPReductionFinal>(VPInst));
    return;
  }
  case VPInstruction::InductionInit: {
    vectorizeInductionInit(cast<VPInductionInit>(VPInst));
    return;
  }
  case VPInstruction::InductionInitStep: {
    vectorizeInductionInitStep(cast<VPInductionInitStep>(VPInst));
    return;
  }
  case VPInstruction::InductionFinal: {
    vectorizeInductionFinal(cast<VPInductionFinal>(VPInst));
    return;
  }
  case VPInstruction::AllocatePrivate: {
    vectorizeAllocatePrivate(cast<VPAllocatePrivate>(VPInst));
    return;
  }
  default: {
    LLVM_DEBUG(dbgs() << "VPInst: "; VPInst->dump());
    llvm_unreachable("VPVALCG: Opcode not uplifted yet.");
  }
  }
}

void VPOCodeGen::vectorizeReductionPHI(VPPHINode *VPPhi,
                                       PHINode *UnderlyingPhi) {
  Type *ScalarTy = VPPhi->getType();
  assert(!ScalarTy->isAggregateType() && "Unexpected reduction type");
  assert(VPPhi->getParent()->getNumPredecessors() == 2 &&
         "Unexpected reduction phi placement");
  assert(!VPPhi->getBlend() && "Unexpected blend on reduction phi");
  Type *VecTy = VectorType::get(ScalarTy, VF);
  PHINode *VecPhi = PHINode::Create(VecTy, 2, "vec.phi",
                                    &*LoopVectorBody->getFirstInsertionPt());
  if (UnderlyingPhi)
    // TODO. Remove after switching to VPValue-based code gen.
    WidenMap[UnderlyingPhi] = VecPhi;
  VPWidenMap[VPPhi] = VecPhi;
  if (EnableVPValueCodegen)
    // In this case we need an additional fixup.
    PhisToFix[VPPhi] = VecPhi;
}

void VPOCodeGen::vectorizeVPPHINode(VPPHINode *VPPhi) {
  assert(EnableVPValueCodegen && "This call is unexpected");

  if (VPEntities && VPEntities->getReduction(VPPhi))
    // Handle reductions.
    vectorizeReductionPHI(VPPhi);
  else
    widenNonInductionPhi(VPPhi);
}

void VPOCodeGen::vectorizeReductionFinal(VPReductionFinal *RedFinal) {
  Value *VecValue = getVectorValue(RedFinal->getOperand(0));
  Intrinsic::ID Intrin = RedFinal->getVectorReduceIntrinsic();
  Type *ElType = RedFinal->getOperand(0)->getType();
  if (isa<VectorType>(ElType))
    // TODO: can implement as shufle/OP sequence for vectors.
    llvm_unreachable("Unsupported vector data type in reduction");

  auto *StartVPVal =
      RedFinal->getNumOperands() > 1 ? RedFinal->getOperand(1) : nullptr;
  Value *Acc = nullptr;
  if (StartVPVal) {
    assert((isa<VPExternalDef>(StartVPVal) || isa<VPConstant>(StartVPVal)) &&
           "Unsupported reduction StartValue");
    Acc = getScalarValue(StartVPVal, 0);
  }
  Value *Ret = nullptr;
  // TODO: Need meaningful processing for Acc for FP reductions, and NoNan
  // parameter.
  switch (Intrin) {
  case Intrinsic::experimental_vector_reduce_v2_fadd:
    assert(Acc && "Expected initial value");
    Ret = Builder.CreateFAddReduce(Acc, VecValue);
    Acc = nullptr;
    break;
  case Intrinsic::experimental_vector_reduce_v2_fmul:
    assert(Acc && "Expected initial value");
    Ret = Builder.CreateFMulReduce(Acc, VecValue);
    Acc = nullptr;
    break;
  case Intrinsic::experimental_vector_reduce_add:
    Ret = Builder.CreateAddReduce(VecValue);
    break;
  case Intrinsic::experimental_vector_reduce_mul:
    Ret = Builder.CreateMulReduce(VecValue);
    break;
  case Intrinsic::experimental_vector_reduce_and:
    Ret = Builder.CreateAndReduce(VecValue);
    break;
  case Intrinsic::experimental_vector_reduce_or:
    Ret = Builder.CreateOrReduce(VecValue);
    break;
  case Intrinsic::experimental_vector_reduce_xor:
    Ret = Builder.CreateXorReduce(VecValue);
    break;
  case Intrinsic::experimental_vector_reduce_umax:
    assert(!Acc && "Unexpected initial value");
    Ret = Builder.CreateIntMaxReduce(VecValue, false);
    break;
  case Intrinsic::experimental_vector_reduce_smax:
    assert(!Acc && "Unexpected initial value");
    Ret = Builder.CreateIntMaxReduce(VecValue, true);
    break;
  case Intrinsic::experimental_vector_reduce_umin:
    assert(!Acc && "Unexpected initial value");
    Ret = Builder.CreateIntMinReduce(VecValue, false);
    break;
  case Intrinsic::experimental_vector_reduce_smin:
    assert(!Acc && "Unexpected initial value");
    Ret = Builder.CreateIntMinReduce(VecValue, true);
    break;
  case Intrinsic::experimental_vector_reduce_fmax:
    assert(!Acc && "Unexpected initial value");
    Ret = Builder.CreateFPMaxReduce(VecValue, /*NoNan*/ false);
    break;
  case Intrinsic::experimental_vector_reduce_fmin:
    assert(!Acc && "Unexpected initial value");
    Ret = Builder.CreateFPMinReduce(VecValue, /*NoNaN*/ false);
    break;
  default:
    llvm_unreachable("unsupported reduction");
    break;
  }
  if (Acc)
    Ret = Builder.CreateBinOp(
        static_cast<Instruction::BinaryOps>(RedFinal->getBinOpcode()), Acc, Ret,
        "final.red");
  VPScalarMap[RedFinal][0] = Ret;

  const VPLoopEntity *Entity = VPEntities->getReduction(RedFinal);
  assert(Entity && "Unexpected: reduction last value is not for entity");
  EntitiesLastValMap[Entity] = Ret;
}

void VPOCodeGen::vectorizeAllocatePrivate(VPAllocatePrivate *V) {
  // Private memory is a pointer. We need to get element type
  // and allocate VF elements.
  Type *OrigTy = V->getType()->getPointerElementType();
  assert((!OrigTy->isAggregateType() || !V->isSOALayout()) &&
         "SOA is not supported for aggregate types yet");

  Type *VecTyForAlloca;
  // TODO. We should handle the case when original alloca has the size argument,
  // e.g. it's like alloca i32, i32 4.
  if (OrigTy->isAggregateType())
    VecTyForAlloca = ArrayType::get(OrigTy, VF);
  else {
    // For non-aggregate types create a vector type.
    Type *EltTy = OrigTy;
    unsigned NumEls = VF;
    if (OrigTy->isVectorTy()) {
      EltTy = OrigTy->getVectorElementType();
      NumEls *= OrigTy->getVectorNumElements();
    }
    VecTyForAlloca = VectorType::get(EltTy, NumEls);
  }

  // Create an alloca in the appropriate block
  IRBuilder<>::InsertPointGuard Guard(Builder);
  Function *F = OrigLoop->getHeader()->getParent();
  BasicBlock &FirstBB = F->front();
  Builder.SetInsertPoint(&*FirstBB.getFirstInsertionPt());

  AllocaInst *WidenedPrivArr =
      Builder.CreateAlloca(VecTyForAlloca, nullptr, "private.mem");
  const DataLayout &DL = OrigLoop->getHeader()->getModule()->getDataLayout();
  WidenedPrivArr->setAlignment(
      MaybeAlign(DL.getPrefTypeAlignment(VecTyForAlloca)));

  VPWidenMap[V] = WidenedPrivArr;
}

// InductionInit has two arguments {Start, Step} and keeps the operation
// opcode. We generate
// For +/-   : broadcast(start) +/GEP step*{0, 1,..,VL-1} (GEP for pointers)
// For */div : broadcast(start) * pow(step,{0, 1,..,VL-1})
// In the current version, pow() is replaced with a series of multiplications.
void VPOCodeGen::vectorizeInductionInit(VPInductionInit *VPInst) {
  auto *StartVPVal = VPInst->getOperand(0);
  auto *StartVal = getScalarValue(StartVPVal, 0);
  Value *BcastStart =
      createVectorSplat(StartVal, VF, Builder, "ind.start.bcast");

  auto *StepVPVal = VPInst->getOperand(1);
  Value *StepVal = getScalarValue(StepVPVal, 0);
  unsigned Opc = VPInst->getBinOpcode();
  bool isMult = Opc == Instruction::Mul || Opc == Instruction::FMul ||
                Opc == Instruction::SDiv || Opc == Instruction::UDiv ||
                Opc == Instruction::FDiv;
  bool IsFloat = VPInst->getType()->isFloatingPointTy();
  int StartConst = isMult ? 1 : 0;
  Constant *StartCoeff =
      IsFloat ? ConstantFP::get(VPInst->getType(), StartConst)
              : ConstantInt::getSigned(StepVal->getType(), StartConst);
  Value *VectorStep;
  if (isMult) {
    // Generate series of mult and insert operations, to avoid calling pow(),
    // forming the following vector
    // {StartCoeff, StartCoeff*Step, StartCoeff*Step*Step, ...,
    //  StartCoeff{*Step}{VF-1 times}}
    unsigned StepOpc = IsFloat ? Instruction::FMul : Instruction::Mul;
    Value *Val = StartCoeff;
    VectorStep = createVectorSplat(UndefValue::get(Val->getType()), VF, Builder,
                                   "ind.step.vec");
    unsigned I = 0;
    for (I = 0; I < VF - 1; I++) {
      VectorStep = Builder.CreateInsertElement(VectorStep, Val, I);
      Val = Builder.CreateBinOp(static_cast<Instruction::BinaryOps>(StepOpc),
                                Val, StepVal);
    }
    // Here I = VF - 1.
    VectorStep = Builder.CreateInsertElement(VectorStep, Val, I);
  } else {
    // Generate sequence of vector operations:
    // %i_seq = {0, 1, 2, 3, ...VF-1}
    // %bcst_step = broadcast step
    // %vector_step = mul %i_seq, %bcst_step
    SmallVector<Constant *, 32> IndStep;
    IndStep.push_back(StartCoeff);
    for (unsigned I = 1; I < VF; I++) {
      Constant *ConstVal = IsFloat
                               ? ConstantFP::get(VPInst->getType(), I)
                               : ConstantInt::getSigned(StepVal->getType(), I);
      IndStep.push_back(ConstVal);
    }
    Value *VecConst = ConstantVector::get(IndStep);
    Value *BcstStep = createVectorSplat(StepVal, VF, Builder, "ind.step.vec");
    VectorStep = Builder.CreateBinOp(
        IsFloat ? Instruction::FMul : Instruction::Mul, BcstStep, VecConst);

    if (auto BinOp = dyn_cast<BinaryOperator>(VectorStep))
      // May be a constant.
      if (BinOp->getOpcode() == Instruction::FMul) {
        FastMathFlags Flags;
        Flags.setFast();
        BinOp->setFastMathFlags(Flags);
      }

  }
  Value *Ret =
      (VPInst->getType()->isPointerTy() || Opc == Instruction::GetElementPtr)
          ? Builder.CreateInBoundsGEP(BcastStart, {VectorStep}, "vector_gep")
          : Builder.CreateBinOp(static_cast<Instruction::BinaryOps>(Opc),
                                BcastStart, VectorStep);
  VPWidenMap[VPInst] = Ret;
  if (needScalarCode(VPInst)) {
    VPScalarMap[VPInst][0] = StartVal;
  }
}

void VPOCodeGen::vectorizeInductionInitStep(VPInductionInitStep *VPInst) {
  unsigned Opc = VPInst->getBinOpcode();
  bool isMult = Opc == Instruction::Mul || Opc == Instruction::FMul ||
                Opc == Instruction::SDiv || Opc == Instruction::UDiv ||
                Opc == Instruction::FDiv;
  bool IsFloat = VPInst->getType()->isFloatingPointTy();
  auto *StartVPVal = VPInst->getOperand(0);
  auto *StartVal = getScalarValue(StartVPVal, 0);

  unsigned StepOpc = IsFloat ? Instruction::FMul : Instruction::Mul;
  Value *MulVF = StartVal;
  if (isMult) {
    for (unsigned I = 1; I < VF; I *= 2)
      MulVF = Builder.CreateBinOp(static_cast<Instruction::BinaryOps>(StepOpc),
                                  MulVF, MulVF);
  } else {
    Constant *VFVal = IsFloat ? ConstantFP::get(VPInst->getType(), VF)
                              : ConstantInt::getSigned(StartVal->getType(), VF);
    MulVF = Builder.CreateBinOp(static_cast<Instruction::BinaryOps>(StepOpc),
                                MulVF, VFVal);
  }
  Value *Ret = createVectorSplat(MulVF, VF, Builder, "ind.step.init");
  VPWidenMap[VPInst] = Ret;

  if (needScalarCode(VPInst)) {
    VPScalarMap[VPInst][0] = MulVF;
  }
}

void VPOCodeGen::vectorizeInductionFinal(VPInductionFinal *VPInst) {
  Value *LastValue = nullptr;
  const VPLoopEntity *Entity = VPEntities->getInduction(VPInst);
  assert(Entity && "Induction last value is not for entity");
  if (VPInst->getNumOperands() == 1) {
    // One operand - extract from vector
    Value *VecVal = getVectorValue(VPInst->getOperand(0));
    LastValue = Builder.CreateExtractElement(VecVal, Builder.getInt32(VF - 1));
  } else {
    // Otherwise calculate by formulas
    //  for post increment liveouts LV = start + step*rounded_tc,
    //  for pre increment liveouts LV = start + step*(rounded_tc-1)
    //
    assert(VPInst->getNumOperands() == 2 && "Incorrect number of operands");
    unsigned Opc = VPInst->getBinOpcode();
    assert(!(Opc == Instruction::Mul || Opc == Instruction::FMul ||
             Opc == Instruction::SDiv || Opc == Instruction::UDiv ||
             Opc == Instruction::FDiv) &&
           "Unsupported induction final form");

    bool IsFloat = VPInst->getType()->isFloatingPointTy();
    auto *VPStep = VPInst->getOperand(1);
    auto *Step = getScalarValue(VPStep, 0);

    unsigned StepOpc = IsFloat ? Instruction::FMul : Instruction::Mul;
    Type *StepType = Step->getType();
    Value *TripCnt = VectorTripCount;
    if (VPEntities->isInductionLastValPreInc(cast<VPInduction>(Entity)))
      TripCnt =
          Builder.CreateSub(TripCnt, ConstantInt::get(TripCnt->getType(), 1));
    Instruction::CastOps CastOp =
        CastInst::getCastOpcode(TripCnt, true, StepType, true);
    Value *CRD = Builder.CreateCast(CastOp, TripCnt, StepType, "cast.crd");
    Value *MulV = Builder.CreateBinOp(
        static_cast<Instruction::BinaryOps>(StepOpc), Step, CRD);
    auto *VPStart = VPInst->getOperand(0);
    auto *Start = getScalarValue(VPStart, 0);
    LastValue =
        (VPInst->getType()->isPointerTy() || Opc == Instruction::GetElementPtr)
            ? Builder.CreateInBoundsGEP(Start, {MulV}, "final_gep")
            : Builder.CreateBinOp(static_cast<Instruction::BinaryOps>(Opc),
                                  Start, MulV);
  }
  // The value is scalar
  VPScalarMap[VPInst][0] = LastValue;
  EntitiesLastValMap[Entity] = LastValue;
}

void VPOCodeGen::fixOutgoingValues() {
  for (auto &LastValPair : EntitiesLastValMap) {
    if (auto *Reduction = dyn_cast<VPReduction>(LastValPair.first))
      fixReductionLastVal(*Reduction, LastValPair.second);
    if (auto *Induction = dyn_cast<VPInduction>(LastValPair.first))
      fixInductionLastVal(*Induction, LastValPair.second);
  }
}

void VPOCodeGen::fixLiveOutValues(const VPLoopEntity &Entity, Value *LastVal) {
  for (auto *Linked : Entity.getLinkedVPValues())
    for (auto *User : Linked->users())
      if (isa<VPExternalUse>(User)) {
        Value *ExtVal = User->getUnderlyingValue();
        if (auto Phi = dyn_cast<PHINode>(ExtVal)) {
          int Ndx = Phi->getBasicBlockIndex(LoopMiddleBlock);
          if (Ndx == -1)
            Phi->addIncoming(LastVal, LoopMiddleBlock);
          else
            Phi->setIncomingValue(Ndx, LastVal);
        } else {
          int Ndx = User->getOperandIndex(Linked);
          assert(Ndx != -1 && "Operand not found in User");
          Value *Operand = const_cast<Value *>(
              cast<VPExternalUse>(User)->getUnderlyingOperand(Ndx));
          cast<Instruction>(ExtVal)->replaceUsesOfWith(Operand, LastVal);
        }
      }
}

void VPOCodeGen::createLastValPhiAndUpdateOldStart(Value *OrigStartValue,
                                                   PHINode *Phi,
                                                   const Twine &NameStr,
                                                   Value *LastVal) {
  PHINode *BCBlockPhi = PHINode::Create(OrigStartValue->getType(), 2, NameStr,
                                        LoopScalarPreHeader->getTerminator());
  for (unsigned I = 0, E = LoopBypassBlocks.size(); I != E; ++I)
    BCBlockPhi->addIncoming(OrigStartValue, LoopBypassBlocks[I]);
  BCBlockPhi->addIncoming(LastVal, LoopMiddleBlock);

  // Fix the scalar loop reduction variable.
  int IncomingEdgeBlockIdx = Phi->getBasicBlockIndex(OrigLoop->getLoopLatch());
  assert(IncomingEdgeBlockIdx >= 0 && "Invalid block index");
  // Pick the other block.
  int SelfEdgeBlockIdx = (IncomingEdgeBlockIdx ? 0 : 1);
  Phi->setIncomingValue(SelfEdgeBlockIdx, BCBlockPhi);
}

void VPOCodeGen::fixReductionLastVal(const VPReduction &Red, Value *LastVal) {
  if (Red.getIsMemOnly()) {
#if 0
    // TODO: Implement last value fixing for in-memory reductions.
    auto OrigPtr = VPEntities->getOrigMemoryPtr(&Red);
    assert(OrigPtr && "Unexpected nullptr original memory");
    auto ScalarPtr = OrigPtr->getUnderlyingValue();
    Builder.SetInsertPoint(LoopScalarPreHeader->getTerminator());
    MergedVal = Builder.CreateLoad(ScalarPtr, ScalarPtr->getName() + ".reload");
#endif
  } else {
    VPValue *VPStart = Red.getRecurrenceStartValue();
    Value *OrigStartValue = VPStart->getUnderlyingValue();
    VPPHINode *VPHi = VPEntities->getRecurrentVPHINode(Red);
    assert(VPHi && "nullptr is not expected");
    PHINode *Phi = cast<PHINode>(VPHi->getUnderlyingValue());
    createLastValPhiAndUpdateOldStart(OrigStartValue, Phi, "bc.merge.reduction",
                                      LastVal);
    fixLiveOutValues(Red, LastVal);
  }
}

void VPOCodeGen::fixInductionLastVal(const VPInduction &Ind, Value *LastVal) {
  if (Ind.getIsMemOnly()) {
    // TODO: Implement last value fixing for in-memory inductions.
  } else {
    VPValue *VPStart = Ind.getStartValue();
    Value *OrigStartValue = VPStart->getUnderlyingValue();
    VPPHINode *VPHi = VPEntities->getRecurrentVPHINode(Ind);
    assert(VPHi && "nullptr is not expected");
    PHINode *Phi = cast<PHINode>(VPHi->getUnderlyingValue());
    createLastValPhiAndUpdateOldStart(OrigStartValue, Phi, "bc.resume.val",
                                      LastVal);
    fixLiveOutValues(Ind, LastVal);
  }
}

void VPOCodeGen::fixNonInductionVPPhis() {
  std::function<void(DenseMap<VPPHINode *, PHINode *> &)> fixInductions =
      [&](DenseMap<VPPHINode *, PHINode *> &Table) -> void {
    bool IsScalar = &Table == &ScalarPhisToFix;
    for (auto PhiToFix : Table) {
      auto *VPPhi = PhiToFix.first;
      auto *Phi = PhiToFix.second;
      const unsigned NumPhiValues = VPPhi->getNumIncomingValues();
      for (unsigned I = 0; I < NumPhiValues; ++I) {
        auto *VPVal = VPPhi->getIncomingValue(I);
        auto *VPBB = VPPhi->getIncomingBlock(I);
        Value *IncValue =
            IsScalar ? getScalarValue(VPVal, 0) : getVectorValue(VPVal);
        Phi->addIncoming(IncValue, State->CFG.VPBB2IRBB[VPBB]);
      }
    }
    return;
  };
  fixInductions(ScalarPhisToFix);
  fixInductions(PhisToFix);
}


