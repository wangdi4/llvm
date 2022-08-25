//===-- IntelVPlanCostModel.cpp -------------------------------------------===//
//
//   Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements VPlan cost modeling.
//
//===----------------------------------------------------------------------===//

#include "IntelVPlanCostModel.h"
#include "IntelVPlan.h"
#include "IntelVPlanCallVecDecisions.h"
#include "IntelVPlanScalVecAnalysis.h"
#include "IntelVPlanUtils.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Support/SaveAndRestore.h"

#include <numeric>

#define DEBUG_TYPE "vplan-cost-model"

using namespace loopopt;

static const constexpr unsigned DefaultCacheLineSize = 64;

// TODO: TTI should return proper data.
static cl::opt<unsigned>
    CMCacheLineSize("vplan-cm-cache-line-size", cl::init(DefaultCacheLineSize),
                    cl::Hidden,
                    cl::desc("Defines size of a cache line (in bytes)"));

// Cost of the store should be 1.5x greater than the cost of a load.
// Store has two stages: allocate/read cache line(s) and place data to it. The
// load reads data from cache line(s). Moving the data to the cache buffer is
// more expensive than moving it from the buffer.
static cl::opt<float> CMStoreCostAdjustment(
    "vplan-cm-store-cost-adjustment", cl::init(1.0f), cl::Hidden,
    cl::desc("Store cost adjustment on top of TTI value"));

static cl::opt<float>
    CMLoadCostAdjustment("vplan-cm-load-cost-adjustment", cl::init(.5f),
                         cl::Hidden,
                         cl::desc("Load cost adjustment on top of TTI value"));

/// A helper function that returns the alignment of load or store instruction.
static Align getMemInstAlignment(const Value *I) {
  assert((isa<LoadInst>(I) || isa<StoreInst>(I)) &&
         "Expected Load or Store instruction");
  if (auto *LI = dyn_cast<LoadInst>(I))
    return LI->getAlign();
  return cast<StoreInst>(I)->getAlign();
}

#if INTEL_CUSTOMIZATION
static const Instruction *getLLVMInstFromDDNode(const HLDDNode *Node) {
  const HLInst *HLInstruction = cast<HLInst>(Node);
  return HLInstruction->getLLVMInstruction();
}
#endif // INTEL_CUSTOMIZATION

namespace llvm {

namespace vpo {

#if INTEL_CUSTOMIZATION
uint64_t VPlanVLSCostModel::getInstructionCost(const OVLSInstruction *I) const {
  uint32_t ElemSize = I->getType().getElementSize();
  Type *ElemType = Type::getIntNTy(getContext(), ElemSize);
  if (isa<OVLSLoad>(I) || isa<OVLSStore>(I)) {
    VectorType *VecTy = FixedVectorType::get(ElemType, VF);
    return *TTI.getMemoryOpCost(
                   isa<OVLSStore>(I) ? Instruction::Store : Instruction::Load,
                   VecTy,
                   // FIXME: Next values are not used in getMemoryOpCost(),
                   // however that can change later.
                   Align() /* Alignment */, 0 /* AddressSpace */)
                .getValue();
  }
  if (auto Shuffle = dyn_cast<OVLSShuffle>(I)) {
    SmallVector<int, 16> Mask;
    Shuffle->getShuffleMask(Mask);
    VectorType *VecTy = FixedVectorType::get(ElemType, Mask.size());
    SmallVector<uint32_t, 16> UMask(Mask.begin(), Mask.end());
    return getShuffleCost(UMask, VecTy);
  }
  llvm_unreachable("Unexpected OVLSInstruction.");
}

uint64_t
VPlanVLSCostModel::getGatherScatterOpCost(const OVLSMemref &Memref) const {
  const auto *VPMemref = dyn_cast<VPVLSClientMemref>(&Memref);
  assert(VPMemref && "Wrong type of OVLSMemref is used.");
  Type *VecTy = getWidenedType(VPMemref->getInstruction()->getType(), VF);
  // FIXME: Without proper decomposition it's impossible to call
  // getLoadStoreCost(), because opcode may not be valid in the VPInstruction.
  // Assume load instruction for non-memref opcode, because store instruction
  // cannot be composed.
  unsigned Opcode =
      VPMemref->getInstruction()->getOpcode() != Instruction::Store
          ? Instruction::Load
          : Instruction::Store;
  return *TTI.getMemoryOpCost(Opcode, VecTy, Align(), 0).getValue();
}
#endif // INTEL_CUSTOMIZATION

Align VPlanTTICostModel::getMemInstAlignment(
    const VPLoadStoreInst *LoadStore) const {
  // getMemInstAlignment is invoked from getLoadStoreCost() when no Alignment
  // is passed to getLoadStoreCost(), which means getLoadStoreCost() is invoked
  // during getCost() pass though every Instruction. In such scenario
  // DefaultPeelingVariant is expected to be set.
  assert(DefaultPeelingVariant && "PeelingVariant is not set.");
  bool NegativeStride = false;
  if (Plan->getVPlanSVA()->instNeedsVectorCode(LoadStore) &&
      isUnitStrideLoadStore(LoadStore, NegativeStride)) {
    // VPAA method takes alignment from IR as a base.
    // Alignment computed by VPAA in most cases is not guaranteed if we skip
    // the peel loop at runtime.
    return VPAA.getAlignmentUnitStride(*LoadStore, DefaultPeelingVariant);
  }

  // TODO:
  // Whole code below this line is expected to be replaced with call
  //   return VPAA.getAlignment(*LS).value();
  // once VPAA.getAlignment is ready.

  if (const Instruction *Inst = LoadStore->getInstruction())
    return ::getMemInstAlignment(Inst);

#if INTEL_CUSTOMIZATION
  if (LoadStore->HIR().isMaster()) {
    const HLDDNode *DDNode = cast<HLDDNode>(LoadStore->HIR().getUnderlyingNode());
    if (const Instruction *Inst = getLLVMInstFromDDNode(DDNode)) {
      if (isa<LoadInst>(Inst) || isa<StoreInst>(Inst)) {
        return ::getMemInstAlignment(Inst);
      } else {
        // Handle cases such as a[i] = b + c, the store to a[i] will be the
        // master VPInst. However, Inst will be an add instruction.
        const RegDDRef *LvalRef = DDNode->getLvalDDRef();
        if (LvalRef && LvalRef->isMemRef())
          if (unsigned Alignment = LvalRef->getAlignment())
            return Align(Alignment);
      }
    }
  }
#endif // INTEL_CUSTOMIZATION

  // If underlying instruction had default alignment (0) we need to query
  // DataLayout what it is, because default alignment for the widened type will
  // be different.
  return DL->getABITypeAlign(LoadStore->getValueType());
}

bool VPlanTTICostModel::isUniformLoadStore(
  const VPLoadStoreInst *LoadStore) const {
  assert (Plan->getVPlanDA() && "DA is not established.");
  return Plan->getVPlanDA()->isAlwaysUniform(*LoadStore->getPointerOperand());
}

bool VPlanTTICostModel::isUnitStrideLoadStore(const VPLoadStoreInst *LoadStore,
                                              bool &NegativeStride) const {
  assert (Plan->getVPlanDA() && "DA is not established.");
  return Plan->getVPlanDA()->isUnitStridePtr(LoadStore->getPointerOperand(),
                                             LoadStore->getValueType(),
                                             NegativeStride);
}

unsigned VPlanTTICostModel::getLoadStoreIndexSize(
  const VPLoadStoreInst *LoadStore) const {

  const VPValue *Ptr = LoadStore->getPointerOperand();
  const VPInstruction *VPInst;
  // Skip all NOP BitCasts/AddrSpaceCasts on the way to GEP.
  while ((VPInst = dyn_cast<VPInstruction>(Ptr)) &&
         (VPInst->getOpcode() == Instruction::BitCast ||
          VPInst->getOpcode() == Instruction::AddrSpaceCast) &&
         TTI.getCastInstrCost(VPInst->getOpcode(), VPInst->getType(),
                              VPInst->getOperand(0)->getType(),
                              TTI::CastContextHint::None) == 0)
    Ptr = VPInst->getOperand(0);

  const VPInstruction *VPAddrInst = dyn_cast<VPGEPInstruction>(Ptr);
  if (!VPAddrInst)
    VPAddrInst = dyn_cast<VPSubscriptInst>(Ptr);
  unsigned IndexSize = DL->getPointerSizeInBits();

  // Try to reduce index size from 64 bit (default for GEP) to 32. It is
  // essential for VF 16. Check that the base pointer (first operand) is the
  // same for all lanes, and that there's at most one variable index.
  if (IndexSize < 64 || !VPAddrInst ||
      getPointerOperand(VPAddrInst)->getType()->isVectorTy())
    return IndexSize;

  auto getTypeElementSize = [](const VPValue *V) -> unsigned {
    const Type *Ty = V->getType();
    if (auto *VecTy = dyn_cast<VectorType>(Ty))
      Ty = VecTy->getElementType();
    return Ty->getPrimitiveSizeInBits();
  };

  SmallVector<const VPValue *, 4> IndicesOperands;
  if (isa<VPGEPInstruction>(VPAddrInst))
    IndicesOperands.append(VPAddrInst->op_begin() + 1, VPAddrInst->op_end());
  else {
    const VPSubscriptInst *Subscript = cast<VPSubscriptInst>(VPAddrInst);
    // Add index operand for each dimension.
    for (unsigned Dim = 0; Dim < Subscript->getNumDimensions(); ++Dim)
      IndicesOperands.push_back(Subscript->dim(Dim).Index);
  }

  unsigned NumOfVarIndices = 0;
  for (auto *Op : IndicesOperands) {
    const VPConstant *VPConst = dyn_cast<VPConstant>(Op);
    // We don't check that the Constant fits 32 bits as we expect CG is able to
    // pull the splat Constant into splat Base and form scalar base for gather/
    // scatter which is Base + Constant.
    //
    // TODO:
    // Currently it doesn't happen as uniform Base is not recognized by ISel.
    // Once we see a problem due to that we need to fix it in CG.
    if (VPConst && VPConst->isConstantInt())
      continue;

    const VPInstruction *VPIdx = dyn_cast<VPInstruction>(Op);
    if (!VPIdx || ++NumOfVarIndices > 1)
      return IndexSize; // Can't shrink.

    // SExt to 64 bits from 32 bits or less is allowed.
    if (getTypeElementSize(VPIdx) == 64 &&
        VPIdx->getOpcode() == Instruction::SExt &&
        isa<VPInstruction>(VPIdx->getOperand(0)))
      VPIdx = cast<VPInstruction>(VPIdx->getOperand(0));

    if (getTypeElementSize(VPIdx) > 32)
      return IndexSize; // Can't shrink.
  }
  return 32u;
}

VPInstructionCost VPlanTTICostModel::getArithmeticInstructionCost(
  const unsigned Opcode,
  const VPValue *Op1,
  const VPValue *Op2,
  const Type *Ty,
  const unsigned VF) {
  assert(Op1 != nullptr && "First operand is expected.");
  Type *VecTy = getWidenedType(Ty, VF);

  auto SetOperandValueFeatures = [](
    const VPValue *Val,
    TargetTransformInfo::OperandValueKind& OpVK,
    TargetTransformInfo::OperandValueProperties& OpVP) -> void {
    if (const VPConstant *VPConst = dyn_cast<VPConstant>(Val)) {
      OpVK = TargetTransformInfo::OK_UniformConstantValue;
      if (const ConstantInt *IntConst =
          dyn_cast<ConstantInt>(VPConst->getConstant())) {
        if (IntConst->getValue().isPowerOf2())
          OpVP = TargetTransformInfo::OP_PowerOf2;
        else if ((IntConst->getValue() + 1).isPowerOf2() ||
                 (IntConst->getValue() - 1).isPowerOf2())
          OpVP = TargetTransformInfo::OP_PowerOf2_PlusMinus1;
      }
    }
  };

  TargetTransformInfo::OperandValueKind Op1VK =
    TargetTransformInfo::OK_AnyValue;
  TargetTransformInfo::OperandValueKind Op2VK =
    TargetTransformInfo::OK_AnyValue;
  TargetTransformInfo::OperandValueProperties Op1VP =
    TargetTransformInfo::OP_None;
  TargetTransformInfo::OperandValueProperties Op2VP =
    TargetTransformInfo::OP_None;

  SetOperandValueFeatures(Op1, Op1VK, Op1VP);
  if (Op2)
    SetOperandValueFeatures(Op2, Op2VK, Op2VP);

  return TTI.getArithmeticInstrCost(Opcode, VecTy,
    TargetTransformInfo::TCK_RecipThroughput, {Op1VK, Op1VP}, {Op2VK, Op2VP});
}

VPInstructionCost VPlanTTICostModel::getLoadStoreCost(
  const VPLoadStoreInst *LoadStore, unsigned VF) const {
  Align Alignment = getMemInstAlignment(LoadStore);
  return getLoadStoreCost(LoadStore, Alignment, VF);
}

VPInstructionCost VPlanTTICostModel::getLoadStoreCost(
  const VPLoadStoreInst *LoadStore, Align Alignment, unsigned VF) const {
  // TODO: VF check in IsMasked might become redundant once a separate VPlan
  // is maintained for VF = 1 meaning that the cost calculation for scalar loop
  // is done over VPlan that doesn't undergo any vector transformations such as
  // predication.
  bool IsMasked = (VF > 1) && (LoadStore->getParent()->getPredicate() != nullptr);

  // Aggregates are serialized.  If we see an aggregate type we set Scale to VF
  // and substitude VecTy with base aggregate type.
  //
  // TODO:
  // ScalarCost * VF is Zero order approximation of scalarization for aggregate
  // types.  Yet to be tuned further.
  Type *VecTy;
  unsigned Scale;

  Type *OpTy = LoadStore->getValueType();
  if (isVectorizableTy(OpTy)) {
    Scale = 1;
    VecTy = getWidenedType(OpTy, VF);
  }
  else {
    Scale = VF;
    VecTy = OpTy;
  }

  unsigned Opcode = LoadStore->getOpcode();
  unsigned AddrSpace = LoadStore->getPointerAddressSpace();

  if (Opcode == VPInstruction::CompressStore ||
      Opcode == VPInstruction::CompressStoreNonu ||
      Opcode == VPInstruction::ExpandLoad ||
      Opcode == VPInstruction::ExpandLoadNonu)
    return getCompressExpandLoadStoreCost(LoadStore, VF);

  // Special case uniform loads/stores as we issue scalar load/store
  // instructions for them plus broadcast for loads.
  if (VF > 1 && isVectorizableTy(OpTy) && isUniformLoadStore(LoadStore)) {
    VPInstructionCost Cost = IsMasked ?
      TTI.getMaskedMemoryOpCost(Opcode, OpTy, Alignment, AddrSpace) :
      getMemoryOpCost(Opcode, OpTy, Alignment, AddrSpace);

    // TODO:
    // Once SVA is checked during CM the cost of vector->scalar and
    // scalar->vector conversions is expected to be accounted for each
    // instructions and the code below should be removed.
    Cost += (Opcode == Instruction::Load) ?
      TTI.getShuffleCost(TTI::SK_Broadcast, cast<VectorType>(VecTy)) :
      TTI.getVectorInstrCost(Instruction::ExtractElement, VecTy, VF - 1);

    return Cost;
  }

  // Call get[Masked]MemoryOpCost() for the following cases.
  // 1. VF = 1 VPlan even for vector OpTy.
  // 2. Unit stride load/store.
  // 3. Aggregate OpTy (they enter this code though Scale > 1 check of VF == 1
  //    check).
  bool NegativeStride = false;
  if (VF == 1 || Scale > 1 ||
      isUnitStrideLoadStore(LoadStore, NegativeStride)) {
    VPInstructionCost Cost = 0;

    // For negative stride we need to reverse elements in the vector after load
    // or before store.
    //
    // TODO:
    // In case of vector input type (re-vectorization case) reverse operation
    // might cost more than reversing vanilla vector and TTI interface doesn't
    // take into account the vector parts.  Once TTI is fixed VPlan should pass
    // particles type to TTI as well.
    if (NegativeStride) {
      assert(VF > 1 && Scale == 1 &&
             "Unexpected conditions for NegativeStride == true.");
      Cost += TTI.getShuffleCost(TTI::SK_Reverse, cast<VectorType>(VecTy));
    }

    Cost += IsMasked ?
      Scale * TTI.getMaskedMemoryOpCost(Opcode, VecTy, Alignment, AddrSpace) :
      Scale * getMemoryOpCost(Opcode, VecTy, Alignment, AddrSpace);
    return Cost;
  }

  // TODO:
  // Currently TTI doesn't add cost of index split and data join in case
  // gather/scatter operation is implemented with two HW gathers/scatters.
  return TTI.getGatherScatterOpCost(
    Opcode, VecTy, getLoadStoreIndexSize(LoadStore),
    IsMasked, Alignment.value(), AddrSpace);
}

VPInstructionCost VPlanTTICostModel::getCompressExpandLoadStoreCost(
    const VPLoadStoreInst *LoadStore, unsigned VF) const {

  Intrinsic::ID IntrinsicId;
  unsigned Opcode = LoadStore->getOpcode();
  switch (Opcode) {
  case VPInstruction::CompressStore:
    IntrinsicId = Intrinsic::masked_compressstore;
    break;
  case VPInstruction::ExpandLoad:
    IntrinsicId = Intrinsic::masked_expandload;
    break;
  case VPInstruction::CompressStoreNonu:
    IntrinsicId = Intrinsic::x86_avx512_mask_compress;
    break;
  case VPInstruction::ExpandLoadNonu:
    IntrinsicId = Intrinsic::x86_avx512_mask_expand;
    break;
  default:
    llvm_unreachable("Compress/Expand Store/Load opcode is expected.");
  }

  // compress/expand intrinsic cost.
  Type *VecType = getWidenedType(LoadStore->getValueType(), VF);
  VPInstructionCost Cost = TTI.getIntrinsicInstrCost(
      IntrinsicCostAttributes(IntrinsicId, VecType, {VecType}),
      TTI::TCK_RecipThroughput);

  if (Opcode == VPInstruction::CompressStore ||
      Opcode == VPInstruction::ExpandLoad)
    return Cost;

  // scatter/gather cost.
  Cost += TTI.getGatherScatterOpCost(
      Opcode == VPInstruction::CompressStoreNonu ? Instruction::Store
                                                 : Instruction::Load,
      VecType, getLoadStoreIndexSize(LoadStore), true,
      LoadStore->getAlignment().value(), LoadStore->getPointerAddressSpace());

  // Mask generation cost.
  Type *IntTy = IntegerType::get(*Plan->getLLVMContext(), VF);
  Type *MaskTy =
      FixedVectorType::get(IntegerType::getInt1Ty(*Plan->getLLVMContext()), VF);
  Cost += TTI.getCastInstrCost(Instruction::BitCast, IntTy, MaskTy,
                               TTI::CastContextHint::None);
  Cost += TTI.getIntrinsicInstrCost(
      IntrinsicCostAttributes(Intrinsic::ctpop, IntTy, {IntTy}),
      TTI::TCK_RecipThroughput);
  Cost += TTI.getArithmeticInstrCost(
      Instruction::Shl, IntTy, TargetTransformInfo::TCK_RecipThroughput,
      {TargetTransformInfo::OK_UniformConstantValue,
       TargetTransformInfo::OP_None},
      {TargetTransformInfo::OK_AnyValue, TargetTransformInfo::OP_None});
  Cost += TTI.getArithmeticInstrCost(
      Instruction::Xor, IntTy, TargetTransformInfo::TCK_RecipThroughput,
      {TargetTransformInfo::OK_AnyValue, TargetTransformInfo::OP_None},
      {TargetTransformInfo::OK_UniformConstantValue,
       TargetTransformInfo::OP_None});
  Cost += TTI.getCastInstrCost(Instruction::BitCast, MaskTy, IntTy,
                               TTI::CastContextHint::None);
  return Cost;
}

VPInstructionCost VPlanTTICostModel::getInsertExtractElementsCost(
  unsigned Opcode, Type *Ty, unsigned VF) {
  assert((Opcode == Instruction::ExtractElement ||
          Opcode == Instruction::InsertElement) &&
         "Only Extract/InsertElement opcode is expected.");
  VPInstructionCost Cost = 0;
  Type *VecTy = getWidenedType(Ty, VF);
  for(unsigned Idx = 0; Idx < VF; Idx++)
    Cost += TTI.getVectorInstrCost(Opcode, VecTy, Idx);
  return Cost;
}

Intrinsic::ID
VPlanTTICostModel::getIntrinsicForLibFuncCall(
  const VPCallInstruction *VPCall) const {

  assert(VPCall->getCalledFunction() && "Value should not be nullptr!");
  const CallInst *UnderlyingCI = VPCall->getUnderlyingCallInst();
  assert(UnderlyingCI && "Underlying call instruction expected here");

  LibFunc Func = NotLibFunc;
  if (!TLI->getLibFunc(*UnderlyingCI, Func) || !TLI->has(Func))
    return Intrinsic::not_intrinsic;

  // Table to provide alternate similar intrinsics for given library function.
  // NOTE: LLORG has predefined LibFunc_sinpi/LibFunc_cospi for __sinpi/__cospi
  // instead of sinpi/cospi that is used by xmain. Hence new LibFunc definitions
  // - LibFunc_intel_sinpi/LibFunc_intel_cospi - are introduced and used in this
  // table.
  switch (Func) {
  case LibFunc_intel_sinpi:
  case LibFunc_intel_sinpif:
    return Intrinsic::sin;
  case LibFunc_intel_cospi:
  case LibFunc_intel_cospif:
    return Intrinsic::cos;
  default:
    return Intrinsic::not_intrinsic;
  }
}

VPInstructionCost VPlanTTICostModel::getIntrinsicInstrCost(
  Intrinsic::ID ID, const VPCallInstruction *VPCall, unsigned VF) {

  auto *CallInst = VPCall->getUnderlyingCallInst();
  assert(CallInst && "Variable cannot be nullptr.");
  const CallBase &CB = *CallInst;
  VPCallInstruction::CallVecScenariosTy VS = VPCall->getVectorizationScenario();

  // Intrinsics which have 0 cost are not lowered to actual code during ASM CG.
  // They are meant for intermediate analysis/transforms and will be deleted
  // before CG. Do not account the cost of serializing them.
  if (TTI.getIntrinsicInstrCost(
        IntrinsicCostAttributes(ID, CB), TTI::TCK_RecipThroughput) == 0)
    return 0;

  switch (VS) {
    case VPCallInstruction::CallVecScenariosTy::Undefined:
      // For VF = 1 vectorization scenario is not set by design.
      if (VF == 1)
        break;
      // The calls that missed the analysis have Unknown cost.
      return VPInstructionCost::getUnknown();
    case VPCallInstruction::CallVecScenariosTy::DoNotWiden:
      return TTI.getIntrinsicInstrCost(
        IntrinsicCostAttributes(ID, CB), TTI::TCK_RecipThroughput);
    case VPCallInstruction::CallVecScenariosTy::Serialization: {
      // For a serialized call, such as: float call @foo(double arg1, int arg2)
      // calculate the cost of vectorized code that way:
      // Cost of extracting VF double elements for arg1 +
      // Cost of extracting VF int elements for arg2 +
      // Cost of VF calls to scalar @foo +
      // Cost of inserting VF float elements for the result of foo.
      // TODO:
      // Here we ignore the fact that when serialized code feeds another
      // serialized code insert + extract in between can be optimized out.
      VPInstructionCost Cost =
          // The sum of costs of 'devectorizing' all args of the call.
          std::accumulate(
            CB.arg_begin(), CB.arg_end(), VPInstructionCost(0),
            [=](VPInstructionCost Cost, const Use &Arg) {
              Type *ArgTy = Arg.get()->getType();
              // If Arg is not expected to be vectorized
              // (isVectorizableTy(ArgTy) is false) then it contributes 0.
              //
              // TODO:
              // In general there are can be call arguments that are not
              // vectorized.  SVA should help here.
              return Cost + (isVectorizableTy(ArgTy) ?
                             getInsertExtractElementsCost(
                               Instruction::ExtractElement, ArgTy, VF) : 0);
            }) +
          // The cost of VF calls to the scalar function.
          VF * TTI.getIntrinsicInstrCost(
                 IntrinsicCostAttributes(ID, CB), TTI::TCK_RecipThroughput) +
          // The cost of 'vectorizing' function's result if any.
          (isVectorizableTy(CB.getType()) && !CB.getType()->isVoidTy() ?
           getInsertExtractElementsCost(Instruction::InsertElement,
                                        CB.getType(), VF) : 0);
      return Cost;
    }

    case VPCallInstruction::CallVecScenariosTy::LibraryFunc:
      // Catch Library intrinsics with non void return type and special case
      // them.
      //
      // TODO: we need a new TTI interface for SVML calls.  The new interface
      // should not require intrin ID as not all calls that can be mapped to
      // SVML calls are intrinsics.  Until that, keep this customization to
      // handle at least intrinsics that are vectorized using SVML. Other
      // SVML-vectorized library calls will be handled later.
      if (TLI->isSVMLEnabled() && VF > 1 && !CB.getType()->isVoidTy())
        return TTI.getNumberOfParts(getWidenedType(CB.getType(), VF)) *
          getIntrinsicInstrCost(ID, VPCall, 1);
      break;

    default:
      break;
  }

  // Factor in VF into return type and Args type when it is vectorizable.
  auto MaybeVectorizeType = [](bool NeedsVectorCode, Type *Ty,
                               unsigned VF) -> Type * {
    if (VF == 1)
      return Ty;
    if (NeedsVectorCode && isVectorizableTy(Ty) && !Ty->isVoidTy())
      return getWidenedType(Ty, VF);
    return Ty;
  };

  Type *RetTy = MaybeVectorizeType(
    Plan->getVPlanSVA()->retValNeedsVectorCode(VPCall), CB.getType(), VF);
  FastMathFlags FMF;
  if (VPCall->hasFastMathFlags())
    FMF = VPCall->getFastMathFlags();

  SmallVector<Type *> ParamTys;
  for (auto Arg : enumerate(VPCall->arg_operands())) {
    Type *Ty = MaybeVectorizeType(
        Plan->getVPlanSVA()->operandNeedsVectorCode(VPCall, Arg.index()),
        Arg.value()->getType(), VF);
    ParamTys.push_back(Ty);
  }

  return TTI.getIntrinsicInstrCost(
    IntrinsicCostAttributes(ID, RetTy, ParamTys, FMF,
                            dyn_cast<IntrinsicInst>(&CB)),
    TTI::TCK_RecipThroughput);
}

VPInstructionCost VPlanTTICostModel::getTTICost(const VPInstruction *VPInst) {
  return getTTICostForVF(VPInst, VF);
}

VPInstructionCost VPlanTTICostModel::getAllZeroCheckInstrCost(Type *VecSrcTy,
                                                              Type *DestTy) {
  VPInstructionCost CastCost =
    TTI.getCastInstrCost(Instruction::BitCast, DestTy, VecSrcTy,
                         TTI::CastContextHint::None);
  VPInstructionCost CmpCost = TTI.getCmpSelInstrCost(
    Instruction::ICmp, DestTy, nullptr /* CondTy */,
    CmpInst::BAD_ICMP_PREDICATE, TTI::TCK_RecipThroughput);
  return CastCost + CmpCost;
}

VPInstructionCost VPlanTTICostModel::getTTICostForVF(
  const VPInstruction *VPInst, unsigned VF) {
  // TODO: For instruction that are not contained inside the loop we're
  // vectorizing, VF should not be considered. That includes the instructions
  // that are outside of any of the loops in the loopnest. However, before
  // support it in the cost model, we need to design how the current loop being
  // vectorized is represented in the VPlan itself, which also might result in
  // different VFs for different loop:
  //   Outer1
  //     Inner1, vectorize with VF_Inner1
  //     Inner2, vecotrize with VF_Inner2
  unsigned Opcode = VPInst->getOpcode();
  switch (Opcode) {
  default:
    return VPInstructionCost::getUnknown();
  case Instruction::GetElementPtr:
  case VPInstruction::Subscript: {
    // FIXME: First, there could be at least two ways to use this GEP's result
    // (maybe with some casts/geps in between):
    //   1) As a pointer operand of memory operation.
    //   2) As a non-pointer operand of memory operation, e.g. calculation of
    //      some pointers in a loop and storing them without dereferencing.
    // In the first case, the cost of the GEP might be easier to keep as a part
    // of that memory operation - because the way GEP would be handled depends
    // on how the corresponding memory instruction would be handled (e.g.
    // scalarized or not). We reflect that by just returning zero for such GEPs.
    //
    // Also, memory operation are not modeled correctly for now (just
    // VF*ScalarCost) so return the same zero cost for both scalar (VF==1) and
    // vector cases to remove any influence in GEPs cost modeling to
    // vectorization decisions.
    //
    // For the second case from the above we first need a way to distinguish
    // such two kinds of GEP in some prior analysis. As it's not yet done just
    // return the same zero cost too.
    //
    // Another note: getGEPCost in TTI uses GEP's operands (llvm::Value's) to
    // dedicde what will be the cost of the GEP. We don't necessarily have them
    // in VPlan so in future TTI interface should probably be extended with
    // VPValue-based operands overload.
    return 0;
  }
#if INTEL_CUSTOMIZATION
  case VPInstruction::Abs: {
    // Cost of Abs instruction is computed as cost of a compare followed by
    // a select for now.
    Type *VecOpTy = getWidenedType(VPInst->getOperand(0)->getType(), VF);
    Type *CmpTy = Type::getInt1Ty(*(Plan->getLLVMContext()));
    Type *VecCmpTy =
        getWidenedType(CmpTy, cast<FixedVectorType>(VecOpTy)->getNumElements());

    VPInstructionCost CmpCost = TTI.getCmpSelInstrCost(
      Instruction::ICmp, VecOpTy, nullptr /* CondTy */,
      CmpInst::BAD_ICMP_PREDICATE, TTI::TCK_RecipThroughput);
    VPInstructionCost SelectCost = TTI.getCmpSelInstrCost(
      Instruction::Select, VecOpTy, VecCmpTy,
      CmpInst::BAD_ICMP_PREDICATE, TTI::TCK_RecipThroughput);
    return CmpCost + SelectCost;
  }

  case VPInstruction::AllZeroCheck: {
    // AVX512 targets where zmm usage=high results in the following asm:
    // kortestb %k1, %k1
    //
    // AVX512 targets zmm usage=low register results in the following asm:
    // kshiftlb $4, %k2, %k0
    // korb     %k0, %k1, %k0
    // kortestb %k0, %k0
    //
    // AVX2 targets results in the following asm:
    // vmovmskpd %ymm3, %eax
    // testl     %eax, %eax
    //
    // VPlan CG generates bitcast of <i1 x VF> to int of VF size, followed
    // by comparison to 0. The cost of these two instructions seems
    // reasonable for each of the above targets, so this is what is
    // modeled.
    Type *OpTy = VPInst->getOperand(0)->getType();
    Type *VecSrcTy = getWidenedType(OpTy, VF);
    Type *DestTy = IntegerType::get(VPInst->getType()->getContext(), VF);
    return getAllZeroCheckInstrCost(VecSrcTy, DestTy);
  }

  // This is a no-op - used to mark block predicate.
  case VPInstruction::Pred:
    return 0;
  // No-op terminator instruction.
  case Instruction::Br:
    return 0;
#endif // INTEL_CUSTOMIZATION
  case Instruction::Load:
  case Instruction::Store:
    return getLoadStoreCost(cast<VPLoadStoreInst>(VPInst), VF);
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
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor:
    return getArithmeticInstructionCost(
      Opcode, VPInst->getOperand(0), VPInst->getOperand(1),
      VPInst->getType(), VF);
  case VPInstruction::Not: // Treat same as Xor.
    return getArithmeticInstructionCost(
      Instruction::Xor, VPInst->getOperand(0), nullptr,
      VPInst->getType(), VF);
  case Instruction::FNeg:
    return getArithmeticInstructionCost(
      Opcode, VPInst->getOperand(0), nullptr, VPInst->getType(), VF);
  case Instruction::ICmp:
  case Instruction::FCmp: {
    Type *Ty = VPInst->getOperand(0)->getType();

    // FIXME: Assuming all the compares are widened, which is obviously wrong
    // for trip count checks.
    Type *VectorTy = getWidenedType(Ty, VF);
    return TTI.getCmpSelInstrCost(
      Opcode, VectorTy, nullptr /* CondTy */,
      CmpInst::BAD_ICMP_PREDICATE, TTI::TCK_RecipThroughput);
  }
  case Instruction::Select: {
    Type *CondTy = VPInst->getOperand(0)->getType();
    Type *OpTy = VPInst->getOperand(1)->getType();
    Type *VecCondTy = getWidenedType(CondTy, VF);

    if(!isVectorizableTy(OpTy)) {
      /*  %1 = icmp sgt i32 ...
       *  %15 = select <i1> %1, <STy> %2, <STy> %3
       *  (to)
       *  %1 = icmp sgt <2 x i32> ...
       *  %1.1 = extractelement <2 x i1> %1, i32 1
       *  %1.0 = extractelement <2 x i1> %1, i32 0
       *  %15 = select i1 %1.1, <STy> %4, <STy> %5
       *  %16 = select i1 %1.0, <STy> %2, <STy> %3
       *  %15.1 = extractvalue <STy> %15, 0
       *  %16.1 = extractvalue <STy> %16, 0
       */
      VPInstructionCost SelectCost = 0, ExtractCost = 0;
      if (StructType *STy = dyn_cast<StructType>(OpTy)) {
        // Cost of extracting cond from vec cond
        for(unsigned Idx = 0; Idx < VF; Idx++)
          ExtractCost += TTI.getVectorInstrCost(
            Instruction::ExtractElement, VecCondTy, Idx);

        /* Cost of single select instruction with struct type operands
         * Here the cost is calculated based on the assumption that all fields
         * of the struct will eventually be used. i.e. Cost of select with
         * struct type operands <= Sum of Cost of selecting each element. In
         * current CM framework, we don't cost based on use. If this changes in
         * the future, we will update the cost calculation logic here.
         */
        for (auto *EltTy : STy->elements()) {
          // TODO: Handle complex struct types.
          // getCostImpl skips instructions with UnknownCost, so we will
          // follow the same logic here.
          if(!isVectorizableTy(EltTy)) continue;
          SelectCost += TTI.getCmpSelInstrCost(
            Opcode, EltTy, CondTy, CmpInst::BAD_ICMP_PREDICATE,
            TTI::TCK_RecipThroughput);
        }
        // TotalCost = Cost of n extracts from VecCondTy +
        //             Cost of n select instructions
        // where n = VF
        return ExtractCost + VF * SelectCost;
      }
      return VPInstructionCost::getUnknown();
    }

    Type *VecOpTy = getWidenedType(OpTy, VF);
    return TTI.getCmpSelInstrCost(
      Opcode, VecOpTy, VecCondTy, CmpInst::BAD_ICMP_PREDICATE,
      TTI::TCK_RecipThroughput);
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
    Type *BaseDstTy = VPInst->getCMType();
    Type *BaseSrcTy = VPInst->getOperand(0)->getCMType();

    if (!BaseDstTy || !BaseSrcTy)
      return VPInstructionCost::getUnknown();

    assert(!BaseDstTy->isAggregateType() && "Unexpected aggregate type!");

    Type *VecDstTy = getWidenedType(BaseDstTy, VF);
    Type *VecSrcTy = getWidenedType(BaseSrcTy, VF);
    // TODO: The following will report cost "1" for sext/zext in scalar case
    // because no Instruction* is passed to TTI and it is unable to analyze that
    // such a cast can be folded into the defining load for free. We should
    // consider adding an overload accepting VPInstruction for TTI to be able to
    // analyze that.
    return TTI.getCastInstrCost(Opcode, VecDstTy, VecSrcTy,
                                  TTI::CastContextHint::None);
  }
  case Instruction::Call: {
    auto *VPCall = cast<VPCallInstruction>(VPInst);
    auto *CI = VPCall->getUnderlyingCallInst();
    // Calls that construct/destruct non-POD private memory are not expected to
    // have any underlying CallInst. Return UnknownCost for such cases.
    if (!CI)
      return VPInstructionCost::getUnknown();
    Intrinsic::ID ID = getIntrinsicForCallSite(*CI, TLI);

    // If call is expected to be vectorized using SVML then obtain alternate
    // intrinsic version (if available) which will be used for cost computation
    // purpose.
    if (ID == Intrinsic::not_intrinsic && VPCall->getCalledFunction())
      ID = getIntrinsicForLibFuncCall(VPCall);

    if (ID == Intrinsic::not_intrinsic)
      return VPInstructionCost::getUnknown();

    return getIntrinsicInstrCost(ID, VPCall, VF);
  }
  case VPInstruction::ConflictInsn: {
    // TODO:
    // The code calculating the cost of Conflict HW instruction is temporarily
    // is in VPlan cost model. Long term plan is to establish llvm intrinsic
    // corresponding to the instruction and delegate cost calculation of the
    // intrinsic to TTI module.
    //
    // For time being and for consistency we take uops number of vpconflictd/q
    // instruction, which are:
    // VPCONFLICTD xmm, xmm 15 (VF = 4)
    // VPCONFLICTD ymm, ymm 22 (VF = 8)
    // VPCONFLICTD zmm, zmm 37 (VF = 16)
    // VPCONFLICTQ xmm, xmm 3  (VF = 2)
    // VPCONFLICTQ ymm, ymm 15 (VF = 4)
    // VPCONFLICTQ zmm, zmm 22 (VF = 8)
    //
    // VF = 1 might be seen if we make cost estimation for scalar VPlan late
    // enough so Conflict idiom is recognized and lowered already. Thereby
    // return 0 for VF = 1 for now. The long term plan is either to move Scalar
    // VPlan cost calculation earlier, before vector specific transformations
    // happen or clone and keep Scalar VPlan separately from other VPlans.
    if (VF == 1)
      return 0;

    const Type *Ty = VPInst->getOperand(0)->getType();
    assert(dyn_cast<VectorType>(Ty) == nullptr &&
           "revectorization of ConflictInst is not supported.");

    unsigned ElementSizeBits = Ty->getPrimitiveSizeInBits();

    assert((ElementSizeBits == 32 || ElementSizeBits == 64) &&
           "Unsupported element size for VPCONFLICT.");

    // Check for unsupported by CG cases and return high cost if the case is
    // unsupported to impede the vectorization.
    // CM doesn't have machinery to disable the vectorization and VF can be
    // enforced with a knob. 'High cost' approach is tolerated as a temporal
    // solution.
    auto *VPConflictInst = cast<VPConflictInsn>(VPInst);
    if (VPConflictInst->getConflictIntrinsic(VF, ElementSizeBits) ==
        Intrinsic::not_intrinsic)
      return 1000;

    return getConflictInsnCost(VF, ElementSizeBits);
  }
  case VPInstruction::GeneralMemOptConflict: {
    // General memory conflict is not handled in VPlan CG thus it is not
    // supported in CM for VF > 1. We still can see in VF = 1 Plan if
    // cost modelling is performed before general conflict optimization
    // into vectorizable forms of conflict.
    assert(VF == 1 && "GeneralMemOptConflict supported for VF = 1 CM only.");
    // The cost of GeneralMemOptConflict is determined as the sum of costs
    // of all instructions within the Region of general conflict instruction.
    //
    // NOTE:
    // We silently assume here that conflict region either consists of a single
    // block or linear sequence of blocks, which doesn't have if's and loops.
    // And we don't need to apply any heuristics on the code inside the region.
    // The code has to be adjusted once the assumption doesn't hold anymore.
    auto *VPConflict = cast<VPGeneralMemOptConflict>(VPInst);
    VPInstructionCost Cost = 0;
    for (const VPBasicBlock *RegionBlk : VPConflict->getRegion()->getBBs())
      for (const VPInstruction &RegionInst : *RegionBlk) {
        VPInstructionCost InstCost =
          VPlanTTICostModel::getTTICostForVF(&RegionInst, VF);
        if (InstCost.isUnknown())
          continue;
        Cost += InstCost;
      }

    return Cost;
  }
  case VPInstruction::TreeConflict: {
    auto *TreeConflict = cast<VPTreeConflict>(VPInst);
    auto *ConflictIndexTy = TreeConflict->getConflictIndex()->getType();
    auto *ConflictIndexWidenedTy = getWidenedType(ConflictIndexTy, VF);
    auto *RednUpdateWidenedTy =
        getWidenedType(TreeConflict->getRednUpdateOp()->getType(), VF);

    // Cost for pre-conflict loop instructions
    VPInstructionCost PreLoopCost =
        getConflictInsnCost(VF, ConflictIndexTy->getPrimitiveSizeInBits());
    SmallVector<Type *> ParamTys =
        { ConflictIndexWidenedTy, ConflictIndexTy /* flag */ };
    PreLoopCost +=
        TTI.getIntrinsicInstrCost(
            IntrinsicCostAttributes(Intrinsic::ctlz, ConflictIndexWidenedTy,
                                    ParamTys),
            TTI::TCK_RecipThroughput);
    PreLoopCost +=
        TTI.getArithmeticInstrCost(Instruction::Sub, ConflictIndexWidenedTy,
                                   TargetTransformInfo::TCK_RecipThroughput);

    // all-zero check before conflict loop
    Type *DestTy = IntegerType::get(ConflictIndexTy->getContext(), VF);
    PreLoopCost += getAllZeroCheckInstrCost(ConflictIndexWidenedTy, DestTy);

    // Cost model branch misprediction of all-zero check. Similar to value
    // used in icc
    PreLoopCost += 3;

    // Conflict loop cost - conflict loop mainly consists of 2 permute calls,
    // the add used for the running sum, and the all-zero check exit out of the
    // loop. The permute intrinsics are modeled here as shuffles, because
    // shuffles are lowered to equivalent vperm calls in CG. See AVX512ShuffleTbl
    // in X86TargetTransformInfo.cpp
    VPInstructionCost LoopCost =
        TTI.getShuffleCost(TTI::SK_PermuteSingleSrc,
        cast<VectorType>(RednUpdateWidenedTy));
    LoopCost +=
        TTI.getArithmeticInstrCost(Instruction::Add, RednUpdateWidenedTy,
                                   TargetTransformInfo::TCK_RecipThroughput);
    LoopCost += TTI.getShuffleCost(TTI::SK_PermuteSingleSrc,
                                   cast<VectorType>(ConflictIndexWidenedTy));
    LoopCost += getAllZeroCheckInstrCost(ConflictIndexWidenedTy, DestTy);
    // Assume half of the lanes are in conflict - e.g., approximate number of
    // iterations of the conflict loop.
    LoopCost *= VF / 2; // assume half of lanes are in conflict
    return PreLoopCost + LoopCost;
  }
  case VPInstruction::UMinSeq:
  case VPInstruction::SMin:
  case VPInstruction::UMin:
  case VPInstruction::UMax:
  case VPInstruction::SMax: {
    assert(VPInst->getNumOperands() == 2 &&
           "Unsupported form of [S|U][Min|Max]");
    // UMinSeq instruction is lowered as:
    // %cond.or = (%op1 == 0) || (%op2 == 0)
    // select %cond.or == true ? 0 : UMin(%op1, %op2)
    //
    // Initialize Cost with vanilla UMin/SMin/UMax/SMax cost which is
    // lowered into select.
    Type *OpTy = VPInst->getOperand(0)->getType();
    Type *VecOpTy = getWidenedType(OpTy, VF);

    // Pick any condition code here frpm possible conditions for operation in
    // question for the simplicity. We expect the cost doesn't depend on it.
    VPInstructionCost Cost = TTI.getCmpSelInstrCost(
      Instruction::Select, VecOpTy, VecOpTy, CmpInst::ICMP_ULT,
      TTI::TCK_RecipThroughput);

    // TODO: The entry condition to be extended once more Sequential operations
    // are introduced.
    if (Opcode == VPInstruction::UMinSeq) {
      // Add Cost of extra instructions for Sequential operation.
      Cost += TTI.getCmpSelInstrCost(
        Instruction::ICmp, VecOpTy, VecOpTy, CmpInst::ICMP_EQ,
        TTI::TCK_RecipThroughput) * VPInst->getNumOperands();

      Type *VecInt1Ty =
        getWidenedType(Type::getInt1Ty(*Plan->getLLVMContext()), VF);

      // Select is used to implement logical (non commutative) OR operation.
      Cost += TTI.getCmpSelInstrCost(
        Instruction::Select, VecInt1Ty, VecInt1Ty, CmpInst::ICMP_EQ,
        TTI::TCK_RecipThroughput);

      Cost += TTI.getCmpSelInstrCost(
        Instruction::Select, VecOpTy, VecInt1Ty, CmpInst::ICMP_EQ,
        TTI::TCK_RecipThroughput);
    }

    return Cost;
  }
  case VPInstruction::InductionInit: {
    // Add based InductionInit lowering scheme:
    // 1. StartValBCast = bcast(StartVal)
    // 2. StepValBCast = bcast(StepVal)
    // 3. VectorStep = StepValBCast * [0, 1, .. VF - 1]
    // 4. RetVal = StartValBCast + VectorStep
    //
    // Mul based InductionInit lowering scheme:
    // 1. StartValBCast = bcast(StartVal)
    // 2. VectorStep = insert(StepVal, 0)
    // 3. StepVal = StepVal * StepVal
    // 4. VectorStep = insert(StepVal, 1)
    //    ...
    // VF+1. StepVal = StepVal * StepVal
    // VF+2. VectorStep = insert(StepVal, VF - 1)
    // VF+3. RetVal = StartValBCast * VectorStep
    //
    // The last operation for RetVal turns into GEP for pointer type
    // Inductions.
    //
    // Note that when StartVal and/or StepVal are constant values some
    // operations can be folded and result is used inside the loop directly
    // yielding zero cost of InductionInit instruction.
    VPInstructionCost Cost = 0;
    auto *VPInd = cast<VPInductionInit>(VPInst);
    unsigned Opc = VPInd->getBinOpcode();
    bool IsAdd = Opc == Instruction::Add || Opc == Instruction::FAdd ||
                 Opc == Instruction::Sub || Opc == Instruction::FSub ||
                 Opc == Instruction::GetElementPtr;
    bool IsFloat = VPInst->getType()->isFloatingPointTy();
    bool IsPtr = VPInst->getType()->isPointerTy() ||
                 Opc == Instruction::GetElementPtr;

    auto *StepScalTy = VPInst->getOperand(1)->getType();
    auto *StepVecTy = getWidenedType(StepScalTy, VF);
    auto *StartVal = VPInd->getOperand(0);

    if (dyn_cast<VPLiveInValue>(StartVal))
      StartVal = Plan->getExternals().getOriginalIncomingValue(
                   cast<VPLiveInValue>(StartVal)->getMergeId());

    // StartValBCast cost.
    if (!isa<VPConstant>(StartVal))
      Cost += TTI.getShuffleCost(TTI::SK_Broadcast,
                                 getWidenedType(StartVal->getType(), VF));

    // StepValBCast and VectorStep costs.
    if (!isa<VPConstant>(VPInd->getOperand(1))) {
      if (IsAdd)
        Cost +=
          TTI.getShuffleCost(TTI::SK_Broadcast, StepVecTy) +
          TTI.getArithmeticInstrCost(
            IsFloat ? Instruction::FMul : Instruction::Mul, StepVecTy,
            TargetTransformInfo::TCK_RecipThroughput);
      else {
        for (unsigned Index = 0; Index < VF; Index++)
          Cost += TTI.getVectorInstrCost(Instruction::InsertElement,
                                         StepVecTy, Index);
        Cost += (VF - 1) * TTI.getArithmeticInstrCost(
          Opc, StepScalTy, TargetTransformInfo::TCK_RecipThroughput);
      }
    }

    // RetVal cost.
    if (!isa<VPConstant>(StartVal) || !isa<VPConstant>(VPInd->getOperand(1)))
      // TODO: TTI interface for GEP cost yet to be implemented.
      if (!IsPtr)
        Cost += TTI.getArithmeticInstrCost(
          Opc, StepVecTy, TargetTransformInfo::TCK_RecipThroughput);

    return Cost;
  }

  case VPInstruction::InductionInitStep: {
    // Add based InductionInitStep lowering scheme:
    // 1. ScaledStep = StepVal * VF
    // 2. VectorStep = bcast(ScaledStep)
    //
    // Mul based InductionInitStep lowering scheme:
    // 1. ScaledStep = StepVal * StepVal
    // 2. ScaledStep = ScaledStep * ScaledStep
    //    ...
    // Log(VF).   ScaledStep = ScaledStep * ScaledStep
    // Log(VF)+1. VectorStep = bcast(ScaledStep)
    //
    // Note that when StepVal is a constant value the operations are folded
    // resulting zero cost of InductionInitStep instruction.
    auto *StepVal = VPInst->getOperand(0);
    if (isa<VPConstant>(StepVal))
      return 0;

    VPInstructionCost Cost = 0;
    auto *VPIndStep = cast<VPInductionInitStep>(VPInst);
    unsigned Opc = VPIndStep->getBinOpcode();
    bool IsAdd = Opc == Instruction::Add || Opc == Instruction::FAdd ||
                 Opc == Instruction::Sub || Opc == Instruction::FSub ||
                 Opc == Instruction::GetElementPtr;
    bool IsFloat = VPInst->getType()->isFloatingPointTy();
    auto *StepScalTy = StepVal->getType();

    // ScaledStep cost.
    if (IsAdd)
      Cost += TTI.getArithmeticInstrCost(
          IsFloat ? Instruction::FMul : Instruction::Mul, StepScalTy,
          TargetTransformInfo::TCK_RecipThroughput,
          {TargetTransformInfo::OK_AnyValue, TargetTransformInfo::OP_None},
          {TargetTransformInfo::OK_UniformConstantValue,
           TargetTransformInfo::OP_PowerOf2});
    else
      Cost += TTI.getArithmeticInstrCost(
        IsFloat ? Instruction::FMul : Instruction::Mul, StepScalTy,
        TargetTransformInfo::TCK_RecipThroughput) * log2(VF);

    Cost += TTI.getShuffleCost(TTI::SK_Broadcast,
                               getWidenedType(StepScalTy, VF));

    return Cost;
  }

  case VPInstruction::InductionFinal: {
    auto *IndInit = VPInst->getOperand(0);
    auto *IndInitScalTy = IndInit->getType();

    // One operand - extract from vector
    if (VPInst->getNumOperands() == 1)
      return TTI.getShuffleCost(TTI::SK_ExtractSubvector,
                                getWidenedType(IndInitScalTy, VF), VF - 1);

    VPInstructionCost Cost = 0;
    // Otherwise (two operands) calculate by formulas
    //  for post increment liveouts LV = start + step*upper_bound,
    //  for pre increment liveouts LV = start + step*(upper_bound-1)
    //
    bool IsFloat = VPInst->getType()->isFloatingPointTy();
    unsigned Opc = cast<VPInductionFinal>(VPInst)->getBinOpcode();
    bool IsPtr = VPInst->getType()->isPointerTy() ||
                 Opc == Instruction::GetElementPtr;

    unsigned StepOpc = IsFloat ? Instruction::FMul : Instruction::Mul;
    auto *VPIndFinal = cast<VPInductionFinal>(VPInst);

    // The code below to find the loop is copy-pasted from CG routine
    // VPOCodeGen::vectorizeInductionFinal().
    VPLoop *L = nullptr;
    VPBasicBlock *VPIndFinalBB =
      *VPInst->getParent()->getPredecessors().begin();
    L = Plan->getVPLoopInfo()->getLoopFor(VPIndFinalBB);
    while (!L) {
      VPIndFinalBB = *VPIndFinalBB->getPredecessors().begin();
      L = Plan->getVPLoopInfo()->getLoopFor(VPIndFinalBB);
    }
    bool ExactUB = L->exactUB();
    VPCmpInst *Cond = L->getLatchComparison();
    VPValue *VPTripCnt = nullptr;
    if (Cond)
      VPTripCnt = L->isDefOutside(Cond->getOperand(0)) ?
        Cond->getOperand(0) : Cond->getOperand(1);

    auto *VPStep = VPInst->getOperand(1);
    auto *VPStepScalTy = VPStep->getType();
    TargetTransformInfo::OperandValueKind TripCountVK =
      TargetTransformInfo::OK_AnyValue;
    VPValue *VPTripCntVal = nullptr;

    // If Cond is nullptr we assume TripCount is known and constant.
    // !L->getTripCountInfo().IsEstimated is also indication of that
    // the loop has constant TC even if corresponding to TC VPInstructions
    // are not found.
    if (!VPTripCnt || !L->getTripCountInfo().IsEstimated)
      TripCountVK = TargetTransformInfo::OK_UniformConstantValue;
    else if (dyn_cast<VPVectorTripCountCalculation>(VPTripCnt))
      VPTripCntVal = cast<VPInstruction>(VPTripCnt)->getOperand(0);

    if (VPTripCntVal && isa<VPConstant>(VPTripCntVal))
      TripCountVK = TargetTransformInfo::OK_UniformConstantValue;

    // Take into account +/- 1 adjustments if TC is not constant and they
    // do not annihilate each other.
    if (VPTripCntVal && TripCountVK == TargetTransformInfo::OK_AnyValue) {
      if (VPIndFinal->isLastValPreIncrement() && ExactUB)
        // Subtruct one.
        Cost += TTI.getArithmeticInstrCost(
            Instruction::Sub, VPTripCntVal->getType(),
            TargetTransformInfo::TCK_RecipThroughput,
            {TargetTransformInfo::OK_AnyValue, TargetTransformInfo::OP_None},
            {TargetTransformInfo::OK_UniformConstantValue,
             TargetTransformInfo::OP_None});

      if (!ExactUB && !VPIndFinal->isLastValPreIncrement())
        // Add one.
        Cost += TTI.getArithmeticInstrCost(
            Instruction::Add, VPTripCntVal->getType(),
            TargetTransformInfo::TCK_RecipThroughput,
            {TargetTransformInfo::OK_AnyValue, TargetTransformInfo::OP_None},
            {TargetTransformInfo::OK_UniformConstantValue,
             TargetTransformInfo::OP_None});

      // TODO: int32 -> int64 or other cast are possibly inserted by CG
      // but it is not cost modelled as CastInst::getCastOpcode() needs
      // TripCnt Value on input.
      //
      // For simplicity we catch the only common case here:
      // int32 -> int64 cast.
      if (VPTripCnt->getType()->isIntegerTy(32) &&
          VPStepScalTy->isIntegerTy(64))
        Cost += TTI.getCastInstrCost(
          Instruction::SExt, VPStepScalTy, VPTripCntVal->getType(),
          TTI::CastContextHint::None);
    }

    // TODO: Possible PowerOfTwo for known Start/TripCount Values are not
    // cost modelled properly here.

    TargetTransformInfo::OperandValueKind StepVK =
      TargetTransformInfo::OK_AnyValue;
    TargetTransformInfo::OperandValueProperties StepVP =
      TargetTransformInfo::OP_None;

    TargetTransformInfo::OperandValueKind MultVK =
      TargetTransformInfo::OK_UniformConstantValue;

    // Model multiply by 1 case by hand as it is common but TTI has no
    // meaning to model it.
    bool StepValIsOne = false;
    if (isa<VPConstant>(VPStep)) {
      StepVK = TargetTransformInfo::OK_UniformConstantValue;
      if (const ConstantInt *IntConst =
          dyn_cast<ConstantInt>(cast<VPConstant>(VPStep)->getConstant())) {
        if (IntConst->getValue().isOne())
          StepValIsOne = true;
        else if (IntConst->getValue().isPowerOf2())
          StepVP = TargetTransformInfo::OP_PowerOf2;
      }
    }

    if (StepVK == TargetTransformInfo::OK_AnyValue ||
        (TripCountVK == TargetTransformInfo::OK_AnyValue && !StepValIsOne)) {
      Cost += TTI.getArithmeticInstrCost(
          StepOpc, VPStepScalTy, TargetTransformInfo::TCK_RecipThroughput,
          {StepVK, StepVP}, {TripCountVK, TargetTransformInfo::OP_None});
      MultVK = TargetTransformInfo::OK_AnyValue;
    }

    TargetTransformInfo::OperandValueKind IndInitVK =
      TargetTransformInfo::OK_AnyValue;

    if (isa<VPConstant>(IndInit))
      IndInitVK = TargetTransformInfo::OK_UniformConstantValue;

    if (IndInitVK == TargetTransformInfo::OK_AnyValue ||
        MultVK == TargetTransformInfo::OK_AnyValue)
      // TODO: GEP is to be modelled once supported in TTI.
      if (!IsPtr)
        Cost += TTI.getArithmeticInstrCost(
            Opc, IndInitScalTy, TargetTransformInfo::TCK_RecipThroughput,
            {IndInitVK, TargetTransformInfo::OP_None},
            {MultVK, TargetTransformInfo::OP_None});

    return Cost;
  }

  case VPInstruction::CompressExpandIndexInit:
    return TTI.getVectorInstrCost(Instruction::InsertElement,
                                  getWidenedType(VPInst->getType(), VF), 0);

  case VPInstruction::CompressExpandIndexInc: {
    Type *ElType = VPInst->getType();
    VectorType *VecType = getWidenedType(ElType, VF);
    VPInstructionCost Cost = 0;
    Cost += TTI.getIntrinsicInstrCost(
        IntrinsicCostAttributes(Intrinsic::vector_reduce_add, ElType,
                                {VecType}),
        TTI::TCK_RecipThroughput);
    Cost += TTI.getVectorInstrCost(Instruction::InsertElement, VecType, 0);
    return Cost;
  }

  case VPInstruction::CompressExpandIndex: {
    VectorType *VecType = getWidenedType(VPInst->getType(), VF);
    VPInstructionCost Cost = 0;
    Cost += TTI.getShuffleCost(TTI::SK_Broadcast, VecType);
    Cost += TTI.getArithmeticInstrCost(Instruction::Add, VecType);
    return Cost;
  }

  case VPInstruction::CompressStore:
  case VPInstruction::ExpandLoad:
  case VPInstruction::CompressStoreNonu:
  case VPInstruction::ExpandLoadNonu:
    return getCompressExpandLoadStoreCost(cast<VPLoadStoreInst>(VPInst), VF);
  }
}

// Estimate probability ([0.0f; 1.0f] range) of a memref to cross a cache line
// boundary.
// Alignment    - alignment of memory reference;
// RefBytes     - how many bytes is references, a power of 2 value;
// BytesCross   - how many bytes crosses cache line (number M of bytes).
//               (chunk of memory that crosses the the first cache lane);
// \/-------cache-line---------\/-------cache-line---------\/
// ||------|-----||------|-----||------|-----||------|-----||
//                       |---memref----|
//                       |--N--||--M---|
static VPInstructionCost cacheLineCrossingProbability(
  Align Alignment, uint64_t RefBytes, unsigned &BytesCross) {
  unsigned CacheLineSize;
  // Enforce CacheLineSize to be a power of 2 value.
  switch (CMCacheLineSize) {
  case 16:
  case 32:
  case 64:
  case 128:
  case 256:
  case 512:
    CacheLineSize = CMCacheLineSize;
    break;
  default:
    CacheLineSize = DefaultCacheLineSize;
  }

  // This method returns only powers of 2 values.
  uint64_t Base = Alignment.value();

  if (RefBytes <= CacheLineSize) {
    if (Base >= CacheLineSize) {
      // \/-cache-line-\/-cache-line-\/-cache-line-\/-cache-line-\/
      // ||------|-----||------|-----||------|-----||------|-----||
      // /\-------alignment----------/\-------alignment----------/\
      // ||-ref1-|                   ||-ref2-|
      BytesCross = 0;
      return 0;
    } else { // Base < CacheLineSize
      if (RefBytes <= Base) {
        // In this case we have natural alignment.
        // \/-------cache-line---------\/-------cache-line---------\/
        // ||------|-----||------|-----||------|-----||------|-----||
        // /\--alignment-/\--alignment-/\--alignment-/\--alignment-/\
        // ||----ref1----||----ref2----||----ref3----||----ref4----||
        BytesCross = 0;
        return 0;
      } else { // Base < CacheLineSize && RefBytes > Base
        // It is impossible to estimate the number of bytes crossing with
        // certainty:
        // \/-----------------------cache-line---------------------\/
        // ||------|-----||------|-----||------|-----||------|-----||
        // /\--alignment-/\--alignment-/\--alignment-/\--alignment-/\
        // ||------------ref-----------|
        //               ||------------ref-----------|
        //                             ||------------ref-----------|
        //                                           ||------------ref----...|

        // We have N possible placements of RefBytes with specified alignment
        // within cache line. Both are powers of 2.
        unsigned N = CacheLineSize / Base;

        // Out of those N placementes K do not result in cache line crossing.
        // [0, 1, ..., K] first placements do not cross;
        // [K + 1, ..., N] do result in crossing.
        // 0 < K < N.
        // The greatest natural K to satisfy
        //   Base * K + RefBytes <= CacheLineSize
        // is:
        unsigned K = (CacheLineSize - RefBytes) / Base;

        // Number of possibilities to read RefBytes within cache line,
        // since K is an index that starts from 0.
        unsigned AlignPossibilites = K + 1;

        // Min/Max number of bytes crossing the cache line.
        unsigned MinCrossBytes =
          Base * AlignPossibilites + RefBytes - CacheLineSize;
        unsigned MaxCrossBytes =
          Base * (N - 1) + RefBytes - CacheLineSize;
        // Return an average, as the best guess is uniform distribution of
        // memory accesses.
        BytesCross = (MinCrossBytes + MaxCrossBytes) / 2;

        // Probability of an unaligned access would be
        // 1 - AlignPossibilites / N.
        return 1.f - static_cast<VPInstructionCost>(AlignPossibilites) /
                     static_cast<VPInstructionCost>(N);
      }
    }
  } else { // RefBytes > CacheLineSize
    // Base >= CacheLineSize.
    // \/-cache-line-\/-cache-line-\/-cache-line-\/-cache-line-\/
    // ||------|-----||------|-----||------|-----||------|-----||
    // /\-------alignment----------/\-------alignment----------/\
    // ||-------------------------ref--------------------------|

    // RefBytes > CacheLineSize && Base < CacheLineSize.
    // \/-----------------------cache-line---------------------\/
    // ||------|-----||------|-----||------|-----||------|-----||
    // /\--alignment-/\--alignment-/\--alignment-/\--alignment-/\
    // ...----------------ref--------------------|
    // OR:
    //               ||-----------------ref-------------------...
    BytesCross = RefBytes - CacheLineSize;
    return 1;
  }
}

VPInstructionCost VPlanTTICostModel::getNonMaskedMemOpCostAdj(
  unsigned Opcode, Type *SrcTy, Align Alignment) const {
  // Non-vector types are handled using default costs.
  VectorType *VecTy = cast<VectorType>(SrcTy);

  // Number of parts for this Type.
  unsigned NumReg = TTI.getNumberOfParts(VecTy);

  // TTI model doesn't support vector types/registers.  Don't bother evaluating
  // cache split cost for such targets.
  if (NumReg == 0)
    return 0;

  assert(VecTy->getScalarType()->isSized() && "Expect only sizable types");

  uint64_t TypeSizeInBits = 0;
  if (VecTy->getScalarType()->isPointerTy())
    TypeSizeInBits = DL->getPointerTypeSizeInBits(VecTy);
  else {
    TypeSizeInBits = DL->getTypeStoreSizeInBits(VecTy);
  }

  uint64_t SizeOfWholeVector = TypeSizeInBits / 8;
  uint64_t SizeOfMemRef = SizeOfWholeVector / NumReg;

  unsigned BytesCross = 0;
  bool IsStore = Opcode == Instruction::Store;

  // TODO: tune the cost model once peel/rem loops can be generated.
  // For now the change should be miniscule enough to not have
  // noticeable regressions. Consider a multiplier for Load/Store cost.
  VPInstructionCost Cost = IsStore ? VPInstructionCost(CMStoreCostAdjustment)
                                   : VPInstructionCost(CMLoadCostAdjustment);

  VPInstructionCost CrossProbability =
      cacheLineCrossingProbability(Alignment, SizeOfMemRef, BytesCross);
  return Cost * CrossProbability * NumReg;
}

VPInstructionCost VPlanTTICostModel::getMemoryOpCost(
    unsigned Opcode, Type *Src, Align Alignment, unsigned AddressSpace,
    TTI::TargetCostKind CostKind, const Instruction *I) const {
  auto TTICost =
      TTI.getMemoryOpCost(Opcode, Src, Alignment, AddressSpace, CostKind,
                          {TTI::OK_AnyValue, TTI::OP_None}, I);
  LLVM_DEBUG(dbgs() << "TTICost: " << TTICost << '\n';);

  // Return not adjusted scaled up cost for non-vector types.
  if (!isa<FixedVectorType>(Src) ||
      cast<FixedVectorType>(Src)->getNumElements() == 1)
    return TTICost;

  auto AdjustedCost =
    TTICost + getNonMaskedMemOpCostAdj(Opcode, Src, Alignment);
  LLVM_DEBUG(dbgs() << "AdjustedCost: " << AdjustedCost << '\n';);

  return AdjustedCost;
}

VPInstructionCost VPlanTTICostModel::getConflictInsnCost(
    unsigned VF, unsigned ElementSizeBits) {
    if (ElementSizeBits == 32)
      switch (VF) {
      // VF = 2 for 32-bit element type can be implemented with
      // 4 elements VPCONFLICTD.
      case 2:
      case 4:
        return 15;
      case 8:
        return 22;
      case 16:
        return 37;
      case 32:
        return 37 * 2;
      default:
        llvm_unreachable("Unsupported number of elements for VPCONFLICTD.");
      }
    else
      switch (VF) {
      case 2:
        return 3;
      case 4:
        return 15;
      case 8:
        return 22;
      case 16:
        return 22 * 2;
      case 32:
        return 22 * 4;
      default:
        llvm_unreachable("Unsupported number of elements for VPCONFLICTQ.");
      }

    llvm_unreachable("Unreachable code during VPCONFLICT handling.");
}


} // namespace vpo

} // namespace llvm
