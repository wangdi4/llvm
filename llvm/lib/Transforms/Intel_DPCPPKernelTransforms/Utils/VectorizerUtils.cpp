//===-- VectorizerUtils.cpp - Vectorizer utilities --------------*- C++ -*-===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/VectorizerUtils.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/LoopUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/NameMangleAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/RuntimeService.h"

namespace llvm {

using namespace CompilationUtils;

namespace {

class ConversionVisitor : public reflection::TypeVisitor {
  Type *LLVMTy{nullptr};
  LLVMContext &Ctx;

  bool isAddressSpace(const reflection::TypeAttributeEnum Attr) {
    return (Attr >= reflection::ATTR_ADDR_SPACE_FIRST &&
            Attr <= reflection::ATTR_ADDR_SPACE_LAST);
  }

  unsigned convertAddressSpace(const reflection::TypeAttributeEnum Attr) {
    switch (Attr) {
    case reflection::ATTR_PRIVATE:
      return 0U;
    case reflection::ATTR_GLOBAL:
      return 1U;
    case reflection::ATTR_CONSTANT:
      return 2U;
    case reflection::ATTR_LOCAL:
      return 3U;
    case reflection::ATTR_GENERIC:
      return 4U;
    default:
      llvm_unreachable("Invalid TypeAttributeEnum");
    }
  }

public:
  ConversionVisitor(LLVMContext &Ctx) : Ctx(Ctx) {}

  virtual void visit(const reflection::PrimitiveType *Ty) override {
    switch (Ty->getPrimitive()) {
    case reflection::PRIMITIVE_BOOL:
      LLVMTy = IntegerType::get(Ctx, 1U);
      break;
    case reflection::PRIMITIVE_UCHAR:
    case reflection::PRIMITIVE_CHAR:
      LLVMTy = IntegerType::get(Ctx, 8U);
      break;
    case reflection::PRIMITIVE_USHORT:
    case reflection::PRIMITIVE_SHORT:
      LLVMTy = IntegerType::get(Ctx, 16U);
      break;
    case reflection::PRIMITIVE_UINT:
    case reflection::PRIMITIVE_INT:
      LLVMTy = IntegerType::get(Ctx, 32U);
      break;
    case reflection::PRIMITIVE_ULONG:
    case reflection::PRIMITIVE_LONG:
      LLVMTy = IntegerType::get(Ctx, 64U);
      break;
    case reflection::PRIMITIVE_HALF:
      LLVMTy = Type::getHalfTy(Ctx);
      break;
    case reflection::PRIMITIVE_FLOAT:
      LLVMTy = Type::getFloatTy(Ctx);
      break;
    case reflection::PRIMITIVE_DOUBLE:
      LLVMTy = Type::getDoubleTy(Ctx);
      break;
    case reflection::PRIMITIVE_VOID:
      LLVMTy = Type::getVoidTy(Ctx);
      break;
    case reflection::PRIMITIVE_IMAGE_1D_T:
    case reflection::PRIMITIVE_IMAGE_2D_T:
    case reflection::PRIMITIVE_IMAGE_2D_DEPTH_T:
    case reflection::PRIMITIVE_IMAGE_3D_T:
    case reflection::PRIMITIVE_IMAGE_1D_BUFFER_T:
    case reflection::PRIMITIVE_IMAGE_1D_ARRAY_T:
    case reflection::PRIMITIVE_IMAGE_2D_ARRAY_T:
    case reflection::PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_T:
    case reflection::PRIMITIVE_EVENT_T:
    case reflection::PRIMITIVE_CLK_EVENT_T:
    case reflection::PRIMITIVE_QUEUE_T:
    case reflection::PRIMITIVE_PIPE_RO_T:
    case reflection::PRIMITIVE_PIPE_WO_T: {
      std::string Name = reflection::llvmPrimitiveString(Ty->getPrimitive());
      LLVMTy = StructType::create(Ctx, Name);
    } break;
    case reflection::PRIMITIVE_SAMPLER_T:
      LLVMTy = IntegerType::get(Ctx, 32U);
      break;
    default:
      llvm_unreachable("Unexpected primitive type");
      break;
    }
  }

  virtual void visit(const reflection::VectorType *VTy) override {
    VTy->getScalarType()->accept(this);
    LLVMTy = FixedVectorType::get(LLVMTy, VTy->getLength());
  }

  virtual void visit(const reflection::PointerType *PTy) override {
    PTy->getPointee()->accept(this);
    unsigned AS = 0U;
    for (const reflection::TypeAttributeEnum &Attr : PTy->getAttributes()) {
      if (isAddressSpace(Attr)) {
        AS = convertAddressSpace(Attr);
        break;
      }
    }
    LLVMTy = PointerType::get(LLVMTy, AS);
  }

  void visit(const reflection::AtomicType * /*ATy*/) override {
    assert(false && "need to support Atomic Parameter type");
  }

  void visit(const reflection::BlockType * /*BTy*/) override {
    assert(false && "need to support Block Parameter type");
  }

  virtual void visit(const reflection::UserDefinedType *UdTy) override {
    std::string Name = UdTy->toString();
    LLVMTy = StructType::create(Ctx, Name);
  }

  const Type *getType() const { return LLVMTy; }

  Type *getType() { return LLVMTy; }
};

} // namespace

static Type *reflectionToLLVM(LLVMContext &Ctx,
                              const reflection::RefParamType &Ty) {
  ConversionVisitor V(Ctx);
  Ty->accept(&V);
  return V.getType();
}

/// checks if it is possible to convert V into real type using shuffle vector
/// instruction.
static Instruction *convertUsingShuffle(Value *V, const Type *RealTy,
                                        Instruction *Loc) {
  // In order to convert using shuffle both V and RealTy need to be vectors
  // with the same element type.
  const FixedVectorType *DestTy = dyn_cast<FixedVectorType>(RealTy);
  FixedVectorType *VTy = dyn_cast<FixedVectorType>(V->getType());
  if (!DestTy || !VTy)
    return nullptr;
  const Type *DestElTy = DestTy->getElementType();
  const Type *VElTy = VTy->getElementType();
  if (VElTy != DestElTy)
    return nullptr;

  // Generate the shuffle vector Mask.
  unsigned DestNElts = DestTy->getNumElements();
  unsigned VNElts = VTy->getNumElements();
  std::vector<Constant *> Constants;
  unsigned MinWidth = DestNElts > VNElts ? VNElts : DestNElts;
  LLVMContext &Ctx = V->getContext();
  for (unsigned I = 0; I < MinWidth; ++I)
    Constants.push_back(ConstantInt::get(Ctx, APInt(32, I)));
  for (unsigned I = MinWidth; I < DestNElts; ++I)
    Constants.push_back(UndefValue::get(IntegerType::get(Ctx, 32)));
  Constant *Mask = ConstantVector::get(Constants);

  // Return shuffle instruction.
  UndefValue *UndefVect = UndefValue::get(VTy);
  Instruction *Shuffle = new ShuffleVectorInst(V, UndefVect, Mask, "", Loc);
  Shuffle->setDebugLoc(Loc->getDebugLoc());
  return Shuffle;
}

/// Generate type-conversion and place in given location.
static Instruction *bitCastValToType(Value *Orig, Type *TargetType,
                                     Instruction *InsertPoint) {
  LLVMContext &Ctx = InsertPoint->getContext();
  Type *CurrType = Orig->getType();
  assert(CurrType != TargetType && "should get here in case of same type");
  unsigned CurrSize = CurrType->getPrimitiveSizeInBits();
  unsigned RootSize = TargetType->getPrimitiveSizeInBits();
  Value *RetVal;
  IRBuilder<> B(InsertPoint);

  if (CurrSize == RootSize) {
    // just bitcast from one to the other
    RetVal = B.CreateBitCast(Orig, TargetType, "cast_val");
  } else if (Instruction *ShuffleConvert =
                 convertUsingShuffle(Orig, TargetType, InsertPoint)) {
    return ShuffleConvert;
  } else {
    Value *OrigInt = Orig;
    // if Orig is not integer bitcast it into integer
    if (!Orig->getType()->isIntegerTy())
      OrigInt = B.CreateBitCast(Orig, IntegerType::get(Ctx, CurrSize), "cast1");

    // zext / trunc to the TargetType size
    RetVal =
        B.CreateZExtOrTrunc(OrigInt, IntegerType::get(Ctx, RootSize), "conv");

    // if target is not integer bitcast to target type
    if (!TargetType->isIntegerTy())
      RetVal = B.CreateBitCast(RetVal, TargetType, "cast_val");
  }
  return cast<Instruction>(RetVal);
}

/// checks if any of the values in ValInChain can be coverted into RealTy using
/// shuffle vector instruction.
static Value *canRootInputByShuffle(SmallVector<Value *, 4> &ValInChain,
                                    const Type *RealTy, Instruction *Loc) {
  // Run over the chain in reverse order so we try earlier values first.
  unsigned DestSize = RealTy->getPrimitiveSizeInBits();
  for (Value *CurVal : ValInChain) {
    // Argumetns can be converted only if the root is smaller than RealTy.
    unsigned CurSize = CurVal->getType()->getPrimitiveSizeInBits();
    assert(CurSize >= DestSize && "root is bigger than the value");
    if (CurSize < DestSize)
      continue;

    // Try rooting using shuffle.
    if (Instruction *Shuffle = convertUsingShuffle(CurVal, RealTy, Loc)) {
      return Shuffle;
    }
  }
  return nullptr;
}

/// Check if ShuffleVector instruction is used to artificially extend a vector.
static Value *isExtendedByShuffle(ShuffleVectorInst *SVI, Type *RealTy) {
  assert(SVI && "Expected ShuffleVector instruction as input");

  // The "proper" input is supposed to be in the first vector input,
  // and the first WIDTH shuffle values (locations) are the ordered components
  // of that input. No assumptions are made on the second input, or the trailing
  // shuffled elements. That is legal, as the consumer of the shuffleInst is
  // known to be using only the lower WIDTH elements of the vector. WIDTH is
  // calculated as number of vector Elements of RealTy.
  FixedVectorType *DesiredVectorTy = dyn_cast<FixedVectorType>(RealTy);
  if (!DesiredVectorTy)
    return nullptr;
  unsigned RealWidth = DesiredVectorTy->getNumElements();

  // RealWidth must be smaller-or-equal to the width of the shuffleInst result.
  if (RealWidth > cast<FixedVectorType>(SVI->getType())->getNumElements())
    return nullptr;

  // Check that the shuffle components correspond to the input vector.
  for (unsigned I = 0; I < RealWidth; ++I) {
    unsigned MaskValue = SVI->getMaskValue(I);
    if (MaskValue != I)
      return nullptr;
  }
  return SVI->getOperand(0);
}

/// Check if I is obtained by series of insert element instructions to it's
/// start. rooting a sequence like this:
///   %v0 = insertelement <4 x type> undef, type %scalar.0, i32 0
///   %v1 = insertelement <4 x type> %v0,   type %scalar.1, i32 1
/// into
///   %u0 = insertelement <2 x type> undef, type %scalar.0, i32 0
///   %u1 = insertelement <2 x type> %v0,   type %scalar.1, i32 1
static Value *isInsertEltExtend(Instruction *I, Type *RealTy) {
  // If I is an extension of vector by insert element then both I and the real
  // type are vectors with the same element type.
  const FixedVectorType *OrigTy = dyn_cast<FixedVectorType>(I->getType());
  const FixedVectorType *DestTy = dyn_cast<FixedVectorType>(RealTy);
  if (!OrigTy || !DestTy)
    return nullptr;
  const Type *OrigElTy = OrigTy->getElementType();
  unsigned OrigNElts = OrigTy->getNumElements();
  const Type *DestElTy = DestTy->getElementType();
  unsigned DestNElts = DestTy->getNumElements();
  if (OrigElTy != DestElTy || OrigNElts <= DestNElts)
    return nullptr;

  // If I is an extension of vector by insert element then the vector should
  // be created by sequence of insert element instructions to the head of the
  // vector.
  SmallVector<Value *, 16> InsertedVals;
  InsertedVals.assign(DestNElts, nullptr);
  Value *Val = I;
  while (!isa<UndefValue>(Val)) {
    // Val is insert element.
    InsertElementInst *IEI = dyn_cast<InsertElementInst>(Val);
    if (!IEI)
      return nullptr;

    // Index of insertion is constant < destination type number of elements.
    Value *Index = IEI->getOperand(2);
    ConstantInt *C = dyn_cast<ConstantInt>(Index);
    if (!C)
      return nullptr;
    unsigned Idx = (unsigned)C->getZExtValue();
    if (Idx >= DestNElts)
      return nullptr;

    // Consider only the last insertion to Idx.
    if (!InsertedVals[Idx]) {
      InsertedVals[Idx] = IEI->getOperand(1);
    }

    // Continue to the next iteration with the vector operand.
    Val = IEI->getOperand(0);
  }

  // Reconstruct the vector right after the original insert element.
  assert(I != I->getParent()->getTerminator() &&
         "Insert element can not be a terminator of basic block");
  Instruction *Loc = &*(++BasicBlock::iterator(I));
  Value *GatherdVals = UndefValue::get(RealTy);
  LLVMContext &Ctx = Val->getContext();
  for (unsigned Idx = 0; Idx < DestNElts; ++Idx) {
    Value *Val = InsertedVals[Idx];
    if (!Val)
      continue;
    ConstantInt *Index = ConstantInt::get(Ctx, APInt(32, Idx));
    GatherdVals = InsertElementInst::Create(GatherdVals, Val, Index, "", Loc);
  }
  return GatherdVals;
}

/// Check if ShuffleVector instruction is used to artificially truncate a vector
/// result.
static bool isShuffleVectorTruncate(ShuffleVectorInst *SVI) {
  if (!SVI)
    return false;
  // The "proper" input is supposed to be in the first vector input,
  // and the shuffle values (locations) are the ordered components of that
  // Input.
  FixedVectorType *InputType =
      dyn_cast<FixedVectorType>(SVI->getOperand(0)->getType());
  assert(InputType && "ShuffleVector is expected to have vector Inputs!");
  unsigned InputWidth = InputType->getNumElements();
  unsigned ResultWidth =
      cast<FixedVectorType>(SVI->getType())->getNumElements();
  if (ResultWidth > InputWidth)
    return false;

  for (unsigned I = 0; I < ResultWidth; I++) {
    unsigned MaskValue = SVI->getMaskValue(I);
    if (MaskValue != I)
      return false;
  }
  return true;
}

namespace VectorizerUtils {

bool CanVectorize::canVectorizeForVPO(Function &F, FuncSet &UnsupportedFuncs,
                                      bool EnableDirectCallVectorization,
                                      bool EnableSGDirectCallVectorization) {
  if (!EnableDirectCallVectorization) {
    auto KIMD = DPCPPKernelMetadataAPI::KernelInternalMetadataAPI(&F);
    bool HasSG =
        KIMD.KernelHasSubgroups.hasValue() && KIMD.KernelHasSubgroups.get();
    if (!(EnableSGDirectCallVectorization && HasSG))
      if (UnsupportedFuncs.count(&F))
        return false;
  }

  return true;
}

FuncSet CanVectorize::getNonInlineUnsupportedFunctions(Module &M) {
  using namespace llvm::CompilationUtils;

  // Add all kernels to root functions.
  // Kernels assumes to have implicit barrier.
  auto Kernels = DPCPPKernelMetadataAPI::KernelList(&M);
  FuncSet Roots;
  Roots.insert(Kernels.begin(), Kernels.end());

  // Find all functions that contains synchronize/get_local_id/get_global_id to
  // root functions.

  // Get all synchronize built-ins declared in module.
  FuncSet FSet = getAllSyncBuiltinsDecls(M);

  // Get get_local_id built-in if declared in module.
  if (Function *LID = M.getFunction(mangledGetLID())) {
    FSet.insert(LID);
  }

  // Get get_global_id built-in if declared in module.
  if (Function *GID = M.getFunction(mangledGetGID())) {
    FSet.insert(GID);
  }

  for (Function *F : FSet) {
    for (User *U : F->users())
      if (CallInst *CI = dyn_cast<CallInst>(U))
        Roots.insert(CI->getCaller());
  }

  // Fill UnsupportedFuncs set with all functions that calls directly or
  // undirectly functions from the root functions set.
  FuncSet UnsupportedFuncs;
  LoopUtils::fillFuncUsersSet(Roots, UnsupportedFuncs);

  return UnsupportedFuncs;
}

Instruction *createBroadcast(Value *V, unsigned VectorWidth,
                             Instruction *InsertBefore) {
  Constant *Zero = ConstantInt::get(Type::getInt32Ty(V->getContext()), 0);
  Constant *ZeroVector =
      ConstantVector::getSplat(ElementCount::getFixed(VectorWidth), Zero);
  auto *UndefVec =
      UndefValue::get(FixedVectorType::get(V->getType(), VectorWidth));
  auto *IEI =
      InsertElementInst::Create(UndefVec, V, Zero, "insert", InsertBefore);
  auto *Shuffle =
      new ShuffleVectorInst(IEI, UndefVec, ZeroVector, "vector", InsertBefore);

  if (Instruction *I = dyn_cast<Instruction>(V)) {
    const auto &Loc = I->getDebugLoc();
    IEI->setDebugLoc(Loc);
    Shuffle->setDebugLoc(Loc);
  }

  return Shuffle;
}

Instruction *extendValToType(Value *Orig, Type *TargetType,
                             Instruction *InsertPoint) {
  assert(Orig->getType()->getPrimitiveSizeInBits() <=
             TargetType->getPrimitiveSizeInBits() &&
         "expanding when souce is bigger than target");
  return bitCastValToType(Orig, TargetType, InsertPoint);
}

bool isOpaquePtrPair(Type *X, Type *Y) {
  PointerType *XPtr = dyn_cast<PointerType>(X);
  PointerType *YPtr = dyn_cast<PointerType>(Y);
  if (XPtr && YPtr) {
    StructType *XStructEl = dyn_cast<StructType>(XPtr->getElementType());
    StructType *YStructEl = dyn_cast<StructType>(YPtr->getElementType());
    if (XStructEl && YStructEl) {
      return ( // in apple the samplers have slightly differnet function names
               // between rt module and kernels IR so I skip checking that name
               // is the same.
               // XStructEl->getName() == YStructEl->getName() && // have the
               // same name
          XStructEl->isEmptyTy() && // X is emptY
          YStructEl->isEmptyTy());  // Y is emptY
    }
  }
  return false;
}

Value *rootInputArgument(Value *Arg, Type *RootTy, CallInst *CI) {
  LLVMContext &Ctx = CI->getContext();
  // Is the argument already in the correct type?
  Type *ArgTy = Arg->getType();
  if (ArgTy == RootTy)
    return Arg;

  if (isOpaquePtrPair(ArgTy, RootTy)) {
    // incase of pointer to opaque type bitcast
    return CastInst::CreatePointerCast(Arg, RootTy, "bitcast.opaque.ptr", CI);
  }

  if (isa<PointerType>(ArgTy)) {
    // If the function argument is in Pointer type, we expect to find the origin
    // of the pointer as an alloca instruction with 2 users: a store (of the
    // original value) and the CALL Inst. Any other formation will fail the
    // rooting effort. This has one exception (a hack). If the desired type is a
    // vector of width 3, Apple's clang jumps through all sorts of hoops, and
    // creates a shuffle-bitcast-store pattern.
    AllocaInst *Allocator = dyn_cast<AllocaInst>(Arg);
    if (!Allocator || Allocator->getAllocatedType() != RootTy ||
        Allocator->isArrayAllocation() || !Allocator->hasNUses(2)) {
      return nullptr;
    }

    const bool Is3Vector =
        (RootTy->isVectorTy() &&
         cast<FixedVectorType>(RootTy)->getNumElements() == 3);

    // Check the 2 users are really a store and the function call.
    Value *RetVal = nullptr;
    for (auto *U : Allocator->users()) {
      // Check for store instruction
      if (StoreInst *SI = dyn_cast<StoreInst>(U)) {
        // Only a single store is expected...
        if (RetVal)
          return nullptr;
        // Keep the value which is being stored.
        RetVal = SI->getOperand(0);
        // the stored value should be of the expected type
        if (RetVal->getType() != RootTy)
          return nullptr;
      }
      // Support the bitcast-shuffle-store pattern (for width-3 vectors)
      // [LLVM 3.6 UPGRADE] TODO: add support for addrspacecast instruction.
      else if (Is3Vector && isa<BitCastInst>(U)) {
        // Only a single store is expected...
        if (RetVal)
          return nullptr;

        BitCastInst *BCI = cast<BitCastInst>(U);
        // The bitcast must have one user, which is a store.
        if (!BCI->hasOneUse())
          return nullptr;
        StoreInst *SI = dyn_cast<StoreInst>(BCI->user_back());
        if (!SI)
          return nullptr;

        // The store value must be the result of a shuffle.
        ShuffleVectorInst *SVI = dyn_cast<ShuffleVectorInst>(SI->getOperand(0));
        if (!SVI)
          return nullptr;

        // Check that shuffle is extending operand of desired (root) type.
        RetVal = isExtendedByShuffle(SVI, RootTy);
        if (!RetVal || RetVal->getType() != RootTy)
          return nullptr;
      }
      // Else check for the call instruction.
      else if (CallInst *UserCI = dyn_cast<CallInst>(U)) {
        // check that the call Inst is the one we started with.
        if (CI != UserCI)
          return nullptr;
      } else {
        // Unexpected consumer of Alloca.
        return nullptr;
      }
    }
    assert(RetVal && "RetVal must have been set by now");
    return RetVal;
  }

  // Arg was passed as a value (not pointer) but of incorrect type. Climb up
  // over instruction's use-def chain, until the value's root is found, or until
  // reaching a non-instruction.
  Value *CurrVal = Arg;
  Instruction *I;
  SmallVector<Value *, 4> ValInChain;
  while (CurrVal->getType() != RootTy && (I = dyn_cast<Instruction>(CurrVal))) {
    ValInChain.push_back(CurrVal);
    // Check for the "simple" BitCast and ZExt/SExt cases.
    if ((I = dyn_cast<BitCastInst>(CurrVal)) ||
        (I = dyn_cast<AddrSpaceCastInst>(CurrVal)) ||
        (I = dyn_cast<ZExtInst>(CurrVal)) ||
        (I = dyn_cast<SExtInst>(CurrVal))) {
      // Climb up to the input of the cast.
      CurrVal = I->getOperand(0);
    }
    // Check for ExtractElement.
    else if (ExtractElementInst *EE = dyn_cast<ExtractElementInst>(CurrVal)) {
      // ExtractElement is allowed in a single case:
      // ExtractElement <1 x Type>, 0
      CurrVal = EE->getVectorOperand();
      if (cast<FixedVectorType>(EE->getVectorOperandType())->getNumElements() !=
          1)
        return canRootInputByShuffle(ValInChain, RootTy, CI);
    }
    // Check for the more-complicated ShuffleVector cast.
    else if (ShuffleVectorInst *SV = dyn_cast<ShuffleVectorInst>(CurrVal)) {
      CurrVal = isExtendedByShuffle(SV, RootTy);
      if (!CurrVal)
        return canRootInputByShuffle(ValInChain, RootTy, CI);
    } else if (InsertElementInst *IE = dyn_cast<InsertElementInst>(CurrVal)) {
      CurrVal = isInsertEltExtend(IE, RootTy);
      if (!CurrVal)
        return canRootInputByShuffle(ValInChain, RootTy, CI);
    } else {
      return canRootInputByShuffle(ValInChain, RootTy, CI);
    }
  }

  // Check if "desired" type was reached.
  if (CurrVal->getType() == RootTy)
    return CurrVal;

  // CurrVal is not an instruction, so its a constant, or global, or kernel
  // argument. So simply cast it to the desired type.
  unsigned SourceSize = CurrVal->getType()->getPrimitiveSizeInBits();
  unsigned TargetSize = RootTy->getPrimitiveSizeInBits();

  if (Constant *ConstVal = dyn_cast<Constant>(CurrVal)) {
    // Check if both types are of the same size.
    if (SourceSize != TargetSize) {
      // The type sizes mismatch. BitCast to int and resize.
      ConstVal =
          ConstantExpr::getBitCast(ConstVal, IntegerType::get(Ctx, SourceSize));
      ConstVal = ConstantExpr::getIntegerCast(
          ConstVal, IntegerType::get(Ctx, TargetSize), false);
    }
    // Now the sizes match. Bitcast to the desired type.
    CurrVal = ConstantExpr::getBitCast(ConstVal, RootTy);
  } else {
    // Value may be an input argument, or global of some sort.
    // Cast it at the head of the function to the required type.
    Function *CurrFunc = CI->getParent()->getParent();
    CurrVal = bitCastValToType(CurrVal, RootTy, &*inst_begin(CurrFunc));
  }
  return CurrVal;
}

Value *rootInputArgumentBySignature(Value *Arg, unsigned int ParamNum,
                                    CallInst *CI) {
  assert(ParamNum <= CI->arg_size() &&
         "Requested type of parameter that does not exist");

  // Get the (reflection) type from the mangled name.
  assert(CI->getCalledFunction() && "Unexpected indirect function invocation");
  StringRef MangledName = CI->getCalledFunction()->getName();
  reflection::FunctionDescriptor FD = NameMangleAPI::demangle(MangledName);
  return rootInputArgument(
      Arg, reflectionToLLVM(CI->getContext(), FD.Parameters[ParamNum]), CI);
}

Value *rootReturnValue(Value *RetVal, Type *RootType, CallInst *CI) {
  LLVMContext &Ctx = CI->getContext();
  // Check maybe the return value is of the right type - no need for rooting.
  if (RetVal->getType() == RootType)
    return RetVal;

  if (isa<PointerType>(RetVal->getType())) {
    // If the retval is in Pointer type (return by reference), we expect to find
    // the origin of the pointer as an alloca with 2 users: the CALL inst, and a
    // following LOAD of the data. Any other formation will fail the rooting
    // effort.
    AllocaInst *AI = dyn_cast<AllocaInst>(RetVal);
    if (!AI || AI->isArrayAllocation() || AI->getAllocatedType() != RootType ||
        !AI->hasNUses(2))
      return nullptr;

    // Check the 2 users are really a load and the function call.
    Value *RootRetVal = nullptr;
    for (auto *U : AI->users()) {
      if (LoadInst *LI = dyn_cast<LoadInst>(U)) {
        RootRetVal = LI;
        // Check if the loaded value has the expected type.
        if (RootRetVal->getType() != RootType)
          return nullptr;
      } else if (CallInst *UserCI = dyn_cast<CallInst>(U)) {
        // Check that we didnt reach a different call instruction.
        if (UserCI != CI)
          return nullptr;
      } else {
        // Any other instruction is unsupported.
        return nullptr;
      }
    }
    assert(RootRetVal && "Must have rooted the RetVal by now");
    return RootRetVal;
  }

  // retval was passed as a value (not pointer) but of incorrect type.
  assert(dyn_cast<Instruction>(RetVal) == CI &&
         "RetVal should be the return of the CALL");

  if (CI->use_begin() == CI->use_end()) {
    return RetVal;
  }
  // Collect all the users of the retval (thru def-use crawling). Collect only
  // values that have users other than conversion instructions (bitcasts,
  // truncate, etc).
  SmallPtrSet<Instruction *, 8> InstructionsToCrawl, RetvalUsers;
  // start crawling by inspecting the users of the CALL instruction.
  InstructionsToCrawl.insert(CI);
  while (!InstructionsToCrawl.empty()) {
    // Extract next value to inspect.
    Instruction *InstToTest = *(InstructionsToCrawl.begin());
    InstructionsToCrawl.erase(InstToTest);

    // Scan all descendants, looking for retval users.
    for (auto *U : InstToTest->users()) {
      Instruction *UserInst = dyn_cast<Instruction>(U);
      assert(UserInst &&
             "Instruction's user is not an instruction. Unexpected");
      if (isa<BitCastInst>(UserInst) || isa<TruncInst>(UserInst) ||
          isShuffleVectorTruncate(dyn_cast<ShuffleVectorInst>(UserInst))) {
        // User is another conversion instruction. Add it to the crawling list
        InstructionsToCrawl.insert(UserInst);
      } else {
        // User is a "proper" user of retval. Add inspected instruction to the
        // users list
        RetvalUsers.insert(InstToTest);
      }
    }
  }
  assert(!RetvalUsers.empty() && "retval expected to have at least one user");
  unsigned SrcSize = CI->getType()->getPrimitiveSizeInBits();
  unsigned DstSize = RootType->getPrimitiveSizeInBits();
  // Fail if retval is not a primitive type which has a measurable size
  if (0 == SrcSize || 0 == DstSize)
    return nullptr;
  // Fail if the real retval is smaller than the desired size
  if (SrcSize < DstSize)
    return nullptr;

  // If the CALL instruction is in the RetvalUsers list, create a dummy inst and
  // replace all users of the inst with the dummy val. This is needed now, so
  // the new conversion from the CALL value will be the only user of the CALL.
  Instruction *DummyInstruction = nullptr;
  if (RetvalUsers.count(CI)) {
    Type *PtrTy = PointerType::get(CI->getType(), 0);
    Constant *SubExpr = ConstantExpr::getIntToPtr(
        ConstantInt::get(Type::getInt32Ty(Ctx), APInt(32, 0xdeadbeef)), PtrTy);
    DummyInstruction =
        new LoadInst(CI->getType(), SubExpr, "", /*volatile*/ false, Align());
    DummyInstruction->insertAfter(CI);
    CI->replaceAllUsesWith(DummyInstruction);
    RetvalUsers.erase(CI);
    RetvalUsers.insert(DummyInstruction);
  }

  // Generate a conversion from the retval to its "proper" type, and place after
  // the CALL inst The conversion may have up-to 3 stages: bitcast to int,
  // truncate, bitcast to required type.
  Instruction *ConvertedVal = CI;
  if (!isa<IntegerType>(CI->getType())) {
    // Cast retval to an integer.
    Instruction *CastToInt =
        new BitCastInst(ConvertedVal, IntegerType::get(Ctx, SrcSize));
    CastToInt->insertAfter(ConvertedVal);
    CastToInt->setDebugLoc(CI->getDebugLoc());
    ConvertedVal = CastToInt;
  }
  if (SrcSize > DstSize) {
    // Shrink retval to the desired size.
    Instruction *Shrink =
        new TruncInst(ConvertedVal, IntegerType::get(Ctx, DstSize));
    Shrink->insertAfter(ConvertedVal);
    Shrink->setDebugLoc(CI->getDebugLoc());
    ConvertedVal = Shrink;
  }
  if (ConvertedVal->getType() != RootType) {
    assert(ConvertedVal->getType()->getPrimitiveSizeInBits() == DstSize &&
           "cast size error");
    // Bitcast to desired type.
    Instruction *CastToDesired = new BitCastInst(ConvertedVal, RootType);
    CastToDesired->insertAfter(ConvertedVal);
    CastToDesired->setDebugLoc(CI->getDebugLoc());
    ConvertedVal = CastToDesired;
  }
  assert(ConvertedVal->getType() == RootType && "Cast retval failed");

  // Go over all the retval users (from RetvalUsers list) can connect the to the
  // ConvertedVal. In case of type mismatch, cast the ConvertedVal to the
  // desired type first.
  for (Instruction *I : RetvalUsers) {
    if (I->getType() == RootType)
      I->replaceAllUsesWith(ConvertedVal);
    else
      I->replaceAllUsesWith(bitCastValToType(ConvertedVal, I->getType(), I));
  }
  // Erase the dummy inst if existed.
  if (DummyInstruction) {
    assert(DummyInstruction->use_empty() &&
           "Did not disconnect all dummy users!");
    DummyInstruction->eraseFromParent();
  }
  return ConvertedVal;
}

} // namespace VectorizerUtils
} // namespace llvm
