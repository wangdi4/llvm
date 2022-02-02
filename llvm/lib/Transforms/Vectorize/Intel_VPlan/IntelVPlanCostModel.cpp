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

/// A helper function that returns the alignment of load or store instruction.
static unsigned getMemInstAlignment(const Value *I) {
  assert((isa<LoadInst>(I) || isa<StoreInst>(I)) &&
         "Expected Load or Store instruction");
  if (auto *LI = dyn_cast<LoadInst>(I))
    return LI->getAlignment();
  return cast<StoreInst>(I)->getAlignment();
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

unsigned
VPlanTTICostModel::getMemInstAlignment(const VPLoadStoreInst *LoadStore) const {
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
    return VPAA.getAlignmentUnitStride(*LoadStore, DefaultPeelingVariant)
        .value();
  }

  // TODO:
  // Whole code below this line is expected to be replaced with call
  //   return VPAA.getAlignment(*LS).value();
  // once VPAA.getAlignment is ready.

  if (const Instruction *Inst = LoadStore->getInstruction())
    if (unsigned Align = ::getMemInstAlignment(Inst))
      return Align;

#if INTEL_CUSTOMIZATION
  if (LoadStore->HIR().isMaster()) {
    const HLDDNode *DDNode = cast<HLDDNode>(LoadStore->HIR().getUnderlyingNode());
    if (const Instruction *Inst = getLLVMInstFromDDNode(DDNode)) {
      if (isa<LoadInst>(Inst) || isa<StoreInst>(Inst)) {
        if (unsigned Align = ::getMemInstAlignment(Inst))
          return Align;
      } else {
        // Handle cases such as a[i] = b + c, the store to a[i] will be the
        // master VPInst. However, Inst will be an add instruction.
        const RegDDRef *LvalRef = DDNode->getLvalDDRef();
        if (LvalRef && LvalRef->isMemRef())
          if (unsigned Align = LvalRef->getAlignment())
            return Align;
      }
    }
  }
#endif // INTEL_CUSTOMIZATION

  // If underlying instruction had default alignment (0) we need to query
  // DataLayout what it is, because default alignment for the widened type will
  // be different.
  return DL->getABITypeAlignment(LoadStore->getValueType());
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
         VPTTI.getCastInstrCost(VPInst->getOpcode(), VPInst->getType(),
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
  const Type *ScalarTy,
  const unsigned VF) {
  assert(Op1 != nullptr && "First operand is expected.");
  if (!ScalarTy)
    return VPInstructionCost::getUnknown();
  Type *VecTy = getWidenedType(const_cast<Type *>(ScalarTy), VF);

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

  return VPTTI.getArithmeticInstrCost(Opcode, VecTy,
    TargetTransformInfo::TCK_RecipThroughput, Op1VK, Op2VK, Op1VP, Op2VP);
}

VPInstructionCost VPlanTTICostModel::getLoadStoreCost(
  const VPLoadStoreInst *LoadStore, unsigned VF) const {
  unsigned Alignment = getMemInstAlignment(LoadStore);
  return getLoadStoreCost(LoadStore, Align(Alignment), VF);
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
      Cost += VPTTI.getShuffleCost(TTI::SK_Reverse, cast<VectorType>(VecTy));
    }

    Cost += IsMasked ?
      Scale * VPTTI.getMaskedMemoryOpCost(Opcode, VecTy, Alignment, AddrSpace) :
      Scale * VPTTI.getMemoryOpCost(Opcode, VecTy, Alignment, AddrSpace);
    return Cost;
  }

  // TODO:
  // Currently TTI doesn't add cost of index split and data join in case
  // gather/scatter operation is implemented with two HW gathers/scatters.
  return VPTTI.getGatherScatterOpCost(
    Opcode, VecTy, getLoadStoreIndexSize(LoadStore),
    IsMasked, Alignment.value(), AddrSpace);
}

VPInstructionCost VPlanTTICostModel::getInsertExtractElementsCost(
  unsigned Opcode, Type *Ty, unsigned VF) {
  assert((Opcode == Instruction::ExtractElement ||
          Opcode == Instruction::InsertElement) &&
         "Only Extract/InsertElement opcode is expected.");
  VPInstructionCost Cost = 0;
  Type *VecTy = getWidenedType(Ty, VF);
  for(unsigned Idx = 0; Idx < VF; Idx++)
    Cost += VPTTI.getVectorInstrCost(Opcode, VecTy, Idx);
  return Cost;
}

Intrinsic::ID
VPlanTTICostModel::getIntrinsicForSVMLCall(
  const VPCallInstruction *VPCall) const {
  assert(VPCall->getVectorizationScenario() ==
             VPCallInstruction::CallVecScenariosTy::LibraryFunc &&
         "Expected library function call here.");
  auto *CalledFunc = VPCall->getCalledFunction();
  assert(CalledFunc && "Value should not be nullptr!");
  assert(isSVMLFunction(TLI, CalledFunc->getName(),
                        VPCall->getVectorLibraryFunc()) &&
         "Expected SVML function call.");
  (void)CalledFunc;

  LibFunc Func;
  if (!TLI->getLibFunc(*VPCall->getUnderlyingCallInst(), Func))
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
  if (VPTTI.getIntrinsicInstrCost(
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
      return VPTTI.getIntrinsicInstrCost(
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
            CB.arg_begin(), CB.arg_end(), VPInstructionCost(),
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
          VF * VPTTI.getIntrinsicInstrCost(
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
        return VPTTI.getNumberOfParts(getWidenedType(CB.getType(), VF)) *
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

  return VPTTI.getIntrinsicInstrCost(
    IntrinsicCostAttributes(ID, RetTy, ParamTys, FMF,
                            dyn_cast<IntrinsicInst>(&CB)),
    TTI::TCK_RecipThroughput);
}

VPInstructionCost VPlanTTICostModel::getTTICost(const VPInstruction *VPInst) {
  return getTTICostForVF(VPInst, VF);
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
    Type *OpTy = VPInst->getOperand(0)->getCMType();
    if (!OpTy)
      return VPInstructionCost::getUnknown();

    Type *VecOpTy = getWidenedType(OpTy, VF);
    Type *CmpTy = Type::getInt1Ty(*(Plan->getLLVMContext()));
    Type *VecCmpTy =
        getWidenedType(CmpTy, cast<VectorType>(VecOpTy)->getNumElements());

    VPInstructionCost CmpCost =
      VPTTI.getCmpSelInstrCost(Instruction::ICmp, VecOpTy);
    VPInstructionCost SelectCost =
        VPTTI.getCmpSelInstrCost(Instruction::Select, VecOpTy, VecCmpTy);
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
    VPInstructionCost CastCost =
      VPTTI.getCastInstrCost(Instruction::BitCast, DestTy, VecSrcTy,
                             TTI::CastContextHint::None);
    VPInstructionCost CmpCost =
      VPTTI.getCmpSelInstrCost(Instruction::ICmp, DestTy);
    return CastCost + CmpCost;
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
      VPInst->getCMType(), VF);
  case VPInstruction::Not: // Treat same as Xor.
    return getArithmeticInstructionCost(
      Instruction::Xor, VPInst->getOperand(0), nullptr,
      VPInst->getCMType(), VF);
  case Instruction::FNeg:
    return getArithmeticInstructionCost(
      Opcode, VPInst->getOperand(0), nullptr, VPInst->getCMType(), VF);
  case Instruction::ICmp:
  case Instruction::FCmp: {
    // FIXME: Assuming all the compares are widened, which is obviously wrong
    // for trip count checks.
    Type *Ty = VPInst->getOperand(0)->getCMType();

    // FIXME: In the future VPValue will always have Type (VPType), but for now
    // it might be missing so handle such cases.
    if (!Ty)
      Ty = VPInst->getOperand(1)->getCMType();
    if (!Ty)
      return VPInstructionCost::getUnknown();

    Type *VectorTy = getWidenedType(Ty, VF);
    return VPTTI.getCmpSelInstrCost(Opcode, VectorTy);
  }
  case Instruction::Select: {
    // FIXME: Due to issues in VPlan creation VPInstruction with Select opcode
    // can have 4 operands. This is obviously wrong and is not related to the
    // cost modeling. Skip such cases.
    if (VPInst->getNumOperands() != 3)
      return VPInstructionCost::getUnknown();

    Type *CondTy = VPInst->getOperand(0)->getCMType();
    Type *OpTy = VPInst->getOperand(1)->getCMType();

    // FIXME: Remove once VPValue is known to always have type.
    if (!CondTy)
      return VPInstructionCost::getUnknown();

    if (!OpTy)
      OpTy = VPInst->getOperand(2)->getCMType();
    if (!OpTy)
      return VPInstructionCost::getUnknown();

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
          ExtractCost += VPTTI.getVectorInstrCost(
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
          SelectCost += VPTTI.getCmpSelInstrCost(Opcode, EltTy, CondTy);
        }
        // TotalCost = Cost of n extracts from VecCondTy +
        //             Cost of n select instructions
        // where n = VF
        return ExtractCost + VF * SelectCost;
      }
      return VPInstructionCost::getUnknown();
    }

    Type *VecOpTy = getWidenedType(OpTy, VF);
    return VPTTI.getCmpSelInstrCost(Opcode, VecOpTy, VecCondTy);
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
    return VPTTI.getCastInstrCost(Opcode, VecDstTy, VecSrcTy,
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
    if (ID == Intrinsic::not_intrinsic &&
        VPCall->getVectorizationScenario() ==
            VPCallInstruction::CallVecScenariosTy::LibraryFunc) {
      // Filter out SVML function calls, we currently allow some OCL builtins to
      // be vectorized via LibraryFunc scenario too.
      auto *CalledFunc = VPCall->getCalledFunction();
      assert(CalledFunc && "Value should not be nullptr!");
      if (isSVMLFunction(TLI, CalledFunc->getName(),
                         VPCall->getVectorLibraryFunc()))
        ID = getIntrinsicForSVMLCall(VPCall);
    }

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

    // Check for unsupported by CG cases and return high cost if the case is
    // unsupported to impede the vectorization.
    // CM doesn't have machinery to disable the vectorization and VF can be
    // enforced with a knob. 'High cost' approach is tolerated as a temporal
    // solution.
    if (cast<VPConflictInsn>(VPInst)->getConflictIntrinsic(VF) ==
        Intrinsic::not_intrinsic)
      return VPlanTTIWrapper::Multiplier * 1000;

    const Type *Ty = VPInst->getOperand(0)->getType();
    assert(dyn_cast<VectorType>(Ty) == nullptr &&
           "revectorization of ConflictInst is not supported.");

    unsigned NumberOfElements = VF;
    unsigned ElementSizeBits = Ty->getPrimitiveSizeInBits();

    assert((ElementSizeBits == 32 || ElementSizeBits == 64) &&
           "Unsupported element size for VPCONFLICT.");

    if (ElementSizeBits == 32)
      switch (NumberOfElements) {
      // VF = 2 for 32-bit element type can be implemented with
      // 4 elements VPCONFLICTD.
      case 2:
      case 4:
        return VPlanTTIWrapper::Multiplier * 15;
      case 8:
        return VPlanTTIWrapper::Multiplier * 22;
      case 16:
        return VPlanTTIWrapper::Multiplier * 37;
      case 32:
        return VPlanTTIWrapper::Multiplier * 37 * 2;
      default:
        llvm_unreachable("Unsupported number of elements for VPCONFLICTD.");
      }
    else
      switch (NumberOfElements) {
      case 2:
        return VPlanTTIWrapper::Multiplier * 3;
      case 4:
        return VPlanTTIWrapper::Multiplier * 15;
      case 8:
        return VPlanTTIWrapper::Multiplier * 22;
      case 16:
        return VPlanTTIWrapper::Multiplier * 22 * 2;
      case 32:
        return VPlanTTIWrapper::Multiplier * 22 * 4;
      default:
        llvm_unreachable("Unsupported number of elements for VPCONFLICTQ.");
      }

    llvm_unreachable("Unreachable code during VPCONFLICT handling.");
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
  }
}

} // namespace vpo

} // namespace llvm
