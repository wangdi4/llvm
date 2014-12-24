#undef DEBUG_TYPE
#define DEBUG_TYPE "micresolver"
#include "MICResolver.h"
#include "VectorizerUtils.h"
#include "Logger.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Constants.h"

#include "OCLPassSupport.h"
#include <vector>
#include <sstream>

static bool isGatherScatterType(VectorType *VecTy) {
  return VecTy->getNumElements() == 16;
}

namespace intel {

char MICResolver::ID = 0;

OCL_INITIALIZE_PASS(MICResolver, "micresolve", "Resolves masked and vectorized function calls on MIC", false, false)


static Value* getConsecutiveConstantVector(Type* type, unsigned count) {
  std::vector<Constant*> constList;
  uint64_t constVal = 0;

  for (unsigned j=0; j < count; ++j) {
    constList.push_back(ConstantInt::get(type, constVal++));
  }
  return ConstantVector::get(ArrayRef<Constant*>(constList));
}


bool MICResolver::TargetSpecificResolve(CallInst* caller) {
  Function* called = caller->getCalledFunction();
  std::string calledName = called->getName().str();
  V_PRINT(DEBUG_TYPE, "MICSpecificResolve Inspecting "<<calledName<<"\n");

  // Use name to decide what to do
  if (Mangler::isMangledLoad(calledName)) {
    Value *Mask = caller->getArgOperand(0);
    Value *Ptr  = caller->getArgOperand(1);
    assert(Mask->getType()->getScalarSizeInBits() == 1 && "Invalid mask size");
    assert(Ptr->getType()->isPointerTy() && "Ptr is not a pointer!");
    VectorType *MaskTy = dyn_cast<VectorType>(Mask->getType());
    VectorType *RetTy = dyn_cast<VectorType>(caller->getType());
    if (!MaskTy || !RetTy) {
      V_PRINT(DEBUG_TYPE, "Non vector type unsupported for gather "<<calledName<<"\n");
      if (!MaskTy && !RetTy) {
        V_PRINT(gather_scatter_stat, "RESOLVER: SCALAR FOR GATHER " << *caller << "\n");
      } else {
        V_PRINT(gather_scatter_stat, "RESOLVER: NON VECTOR TYPE FOR GATHER " << *caller << "\n");
      }
      return false;
    }

    assert(MaskTy->getNumElements() == RetTy->getNumElements() &&
      "mismatch between mask and data num elements");
    PointerType *PtrTy = cast<PointerType>(Ptr->getType());
    assert(PtrTy->getElementType() == RetTy && "mismatch between ptr and retval");

    if (!isGatherScatterType(RetTy)) {
      V_PRINT(DEBUG_TYPE, "Type unsupported for gather "<<calledName<<"\n");
      V_PRINT(gather_scatter_stat, "RESOLVER: UNSUPPORTED TYPE FOR GATHER " << *caller << "\n");
      return false;
    }

    V_PRINT(DEBUG_TYPE, "Generating gather for "<<calledName<<"\n");
    assert(8 * Mangler::getMangledLoadAlignment(calledName) >=
           RetTy->getScalarSizeInBits() && "Not naturally aligned!");

    // Remove address space from pointer type
    PtrTy = PointerType::get(RetTy->getScalarType(), 0);
    Ptr = new BitCastInst(Ptr, PtrTy, "ptrTypeCast", caller);

    Type *IndTy = IntegerType::get(caller->getContext(), 32);

    Value *Index = getConsecutiveConstantVector(IndTy, RetTy->getNumElements());

    CreateGatherScatterAndReplaceCall(caller, Mask, Ptr, Index, NULL, Mangler::Gather);
    return true;
  }

  if (Mangler::isMangledStore(calledName)) {
    Value *Mask = caller->getArgOperand(0);
    Value *Ptr  = caller->getArgOperand(2);
    Value *Data = caller->getArgOperand(1);
    assert(Mask->getType()->getScalarSizeInBits() == 1 && "Invalid mask size");
    assert(Ptr->getType()->isPointerTy() && "Ptr is not a pointer!");
    VectorType *MaskTy = dyn_cast<VectorType>(Mask->getType());
    VectorType *DataTy = dyn_cast<VectorType>(Data->getType());
    if (!MaskTy || !DataTy) {
      V_PRINT(DEBUG_TYPE, "Non vector type unsupported for scatter "<<calledName<<"\n");
      if (!MaskTy && !DataTy) {
        V_PRINT(gather_scatter_stat, "RESOLVER: SCALAR FOR SCATTER " << *caller << "\n");
      } else {
        V_PRINT(gather_scatter_stat, "RESOLVER: NON VECTOR TYPE FOR SCATTER " << *caller << "\n");
      }
      return false;
    }

    assert(DataTy && "mangled store of non vector type data");
    assert(MaskTy->getNumElements() == DataTy->getNumElements() &&
      "mismatch between mask and data num elements");
    PointerType *PtrTy  = cast<PointerType>(Ptr->getType());
    assert(PtrTy->getElementType() == DataTy &&
      "mismatch between ptr and retval");

    if (!isGatherScatterType(DataTy)) {
      V_PRINT(DEBUG_TYPE, "Type unsupported for scatter "<<calledName<<"\n");
      V_PRINT(gather_scatter_stat, "RESOLVER: UNSUPPORTED TYPE FOR SCATTER " << *caller << "\n");
      return false;
    }

    V_PRINT(DEBUG_TYPE, "Generating scatter for "<<calledName<<"\n");
    assert(8 * Mangler::getMangledStoreAlignment(calledName) >=
           DataTy->getScalarSizeInBits() && "Not naturally aligned!");

    // Remove address space from pointer type
    PtrTy = PointerType::get(DataTy->getScalarType(), 0);
    Ptr = new BitCastInst(Ptr, PtrTy, "ptrTypeCast", caller);

    Type *IndTy = IntegerType::get(caller->getContext(), 32);

    Value *Index = getConsecutiveConstantVector(IndTy, DataTy->getNumElements());

    CreateGatherScatterAndReplaceCall(caller, Mask, Ptr, Index, Data, Mangler::Scatter);
    return true;
  }

  if (Mangler::isMangledGather(calledName)) {
    Value *Mask      = caller->getArgOperand(0);
    Value *Ptr       = caller->getArgOperand(1);
    Value *Index     = caller->getArgOperand(2);
    Value *ValidBits = caller->getArgOperand(3);
    Value *IsSigned  = caller->getArgOperand(4);

    FixBaseAndIndexIfNeeded(caller, Mask, ValidBits, IsSigned, Ptr, Index);
    CreateGatherScatterAndReplaceCall(caller, Mask, Ptr, Index, NULL, Mangler::Gather);
    return true;
  }

  if (Mangler::isMangledScatter(calledName)) {
    Value *Mask      = caller->getArgOperand(0);
    Value *Ptr       = caller->getArgOperand(1);
    Value *Index     = caller->getArgOperand(2);
    Value *Data      = caller->getArgOperand(3);
    Value *ValidBits = caller->getArgOperand(4);
    Value *IsSigned  = caller->getArgOperand(5);

    FixBaseAndIndexIfNeeded(caller, Mask, ValidBits, IsSigned, Ptr, Index);
    CreateGatherScatterAndReplaceCall(caller, Mask, Ptr, Index, Data, Mangler::Scatter);
    return true;
  }

  if (Mangler::isMangeledGatherPrefetch(calledName)) {
    Value *Mask      = caller->getArgOperand(0);
    Value *Ptr       = caller->getArgOperand(1);
    Value *Index     = caller->getArgOperand(2);
    Value *ValidBits = caller->getArgOperand(3);
    Value *IsSigned  = caller->getArgOperand(4);

    FixBaseAndIndexIfNeeded(caller, Mask, ValidBits, IsSigned, Ptr, Index);
    CreateGatherScatterAndReplaceCall(caller, Mask, Ptr, Index, NULL, Mangler::GatherPrefetch);
    return true;
  }

  V_PRINT(DEBUG_TYPE, "Unhandled call: not mangled load nor store "<<calledName<<"\n");
  return false;
}

Instruction* MICResolver::CreateGatherScatterAndReplaceCall(CallInst* caller, Value *Mask, Value *Ptr, Value *Index, Value *Data, Mangler::GatherScatterType type) {
  Module *pModule = caller->getParent()->getParent()->getParent();
  V_ASSERT((type == Mangler::GatherPrefetch || (Data ? Data->getType() : caller->getType())->isVectorTy()) && "Data value type is not a vector");

  VectorType *dataTy = NULL;

  switch (type) {
  case Mangler::GatherPrefetch: {
    Type * ElemTy = cast<PointerType>(Ptr->getType())->getElementType();
    dataTy = VectorType::get(ElemTy, 16);
    break;
  }
  case Mangler::Gather:
    dataTy = cast<VectorType>(caller->getType());
    break;
  case Mangler::Scatter:
    dataTy = cast<VectorType>(Data->getType());
    break;
  default:
    V_ASSERT(false && "Illegal GatheScatterType ");
  }

  const bool isUniformMask = !(Mask->getType()->isVectorTy());
  const bool isMasked = !(isUniformMask && isa<Constant>(Mask) && cast<Constant>(Mask)->isAllOnesValue());

  // Get Gather/Scatter function name
  std::string name = Mangler::getGatherScatterName(isMasked, type, dataTy);

  if(isMasked && isUniformMask) {
    // We have uniform mask (not known to be 1), need to broadcast it
    const unsigned int packetWidth = dataTy->getNumElements();
    Mask = VectorizerUtils::createBroadcast(Mask, packetWidth, caller);
  }
  // Initialize function arguments
  SmallVector<Value *, 4> args;
  if(isMasked) args.push_back(Mask);
  args.push_back(Ptr);
  args.push_back(Index);
  if(type == Mangler::Scatter) args.push_back(Data);

  // Create new gather/scatter caller instruction
  Instruction *newCaller = VectorizerUtils::createFunctionCall(pModule, name, caller->getType(), args,
        SmallVector<Attribute::AttrKind, 4>(), caller);

  // Replace caller with new gather/scatter caller instruction
  caller->replaceAllUsesWith(newCaller);
  // Remove and erase caller from function
  caller->eraseFromParent();

  V_PRINT(DEBUG_TYPE, "Generated "<<(type == Mangler::Gather ? "gather " : type == Mangler::Scatter ? "scatter " : "prefetch gather " )<<*newCaller<<"\n");
  return newCaller;
}

void MICResolver::FixBaseAndIndexIfNeeded(
                  CallInst* caller, Value *Mask, Value *ValidBits,
                  Value *IsSigned, Value *&Ptr, Value *&Index) {
  V_ASSERT(ValidBits && isa<ConstantInt>(ValidBits) && "ValidBits argument is not constant");
  V_ASSERT(IsSigned && isa<ConstantInt>(IsSigned) && "IsSigned argument is not constant");
  V_ASSERT(Ptr && Ptr->getType()->isPointerTy() && "Ptr type is not a pointer");

  unsigned int uValidBits = (unsigned int)cast<ConstantInt>(ValidBits)->getZExtValue();
  bool bIsSigned = !cast<Constant>(IsSigned)->isNullValue();
  const bool bIsUniformMask = !(Mask->getType()->isVectorTy());

  VectorType *IndexType = cast<VectorType>(Index->getType());

  Type *i32Ty = Type::getInt32Ty(caller->getContext());
  Type *i32Vec = VectorType::get(i32Ty, IndexType->getNumElements());

  // Calculate the safe valid bits in index that allows using gather/scatter without modifications
  // This number is refferring to signed index, for unsigned the safe index is one bit less.
  const unsigned int safeValidBits = 32;// - VectorizerUtils::getLOG(dataSizeInBytes);
  if(uValidBits <= safeValidBits) {
    // In this case it is safe to Bitcast index to 32bit
    Index = BitCastInst::CreateIntegerCast(Index, i32Vec, bIsSigned, "IntegerCaseToi32", caller);
    V_PRINT(gather_scatter_stat, "RESOLVER: TRUNC " << *Index << "\n");
    if((uValidBits ==  safeValidBits) && !bIsSigned) {
      // In this case index is unsigned 32bit (we need to assure value is no higher than 2^31)
      const unsigned int MaxSignedPosValidNum = 0x7FFFFFFF;
      Value *PosMax32BitInt = ConstantInt::get(i32Ty, MaxSignedPosValidNum);
      Constant *NegMax32BitIntConst = ConstantInt::get(i32Ty, -MaxSignedPosValidNum);
      SmallVector<Constant*, 16> VecNegMax32BitIntConst(IndexType->getNumElements(), NegMax32BitIntConst);
      Value *NegMax32BitInt = ConstantVector::get(ArrayRef<Constant*>(VecNegMax32BitIntConst));

      Ptr = GetElementPtrInst::Create(Ptr, PosMax32BitInt, "Base+safeNumFix", caller);
      Index = BinaryOperator::CreateAdd(Index, NegMax32BitInt, "Index-safeNumFix", caller);
      V_PRINT(gather_scatter_stat, "RESOLVER: Base+safeNumFix " << *Ptr <<
        "\t|\t RESOLVER: Index-safeNumFix " << *Index << "\n");
    }
    return;
  }

  //Reaching here means we have Index with more than 32bit. (Thus, we assume Index type is 64bit)
  //V_ASSERT(IndexType->getElementType()->isIntegerTy(64) && "index element type is something other than 32bit or 64bit");

  // The following code will chose a valide index (such one with mask bit 1), and will perform the following
  //   newBase = Base + validIndex
  //   newVecIndex = VecIndex - brodcast(validIndex)
  // As a result we can assure that newIndex is limited to 32bit
  // ***Since buffer size is <= 2^31***, and all work items that access the memory
  // has distance of +-2^31 at most.


  Constant *ConstZero = ConstantInt::get(i32Ty, 0);
  // if masked idx = number of MSB bit in mask that is set to zero
  // Otherwise idx = 0
  Value *Idx = ConstZero;
  if(!bIsUniformMask) {
    Module *pModule = caller->getParent()->getParent()->getParent();
    V_ASSERT(Mask->getType()->isVectorTy() && "Mask assumed to be vector type at this point");
    VectorType *MaskTy = cast<VectorType>(Mask->getType());
    V_ASSERT(MaskTy->getElementType()->isIntegerTy(1) && "Mask assumed to be vector of type i1");

    // Get cttz intrinsics name
    std::stringstream sname;
    sname << "llvm.cttz.i" << MaskTy->getNumElements();
    //sname << "llvm.x86.mic.bsff." << MaskTy->getNumElements();
    Type *MaskCombinedTy = Type::getIntNTy(caller->getContext(), MaskTy->getNumElements());
    Value *CombinedMask = new BitCastInst(Mask, MaskCombinedTy, "16xi1Toi16", caller);

    // Initialize function arguments
    SmallVector<Value *, 4> args;
    args.push_back(CombinedMask);
    args.push_back(ConstantInt::get(Type::getInt1Ty(caller->getContext()),0));

    // Call cttz intrinsics name
    Instruction *CttzCaller = VectorizerUtils::createFunctionCall(pModule, sname.str(), MaskCombinedTy, args,
        SmallVector<Attribute::AttrKind, 4>(), caller);
    // Convert cttz result to i32 before using it as index parameter for extract element instruction.
    CttzCaller = BitCastInst::CreateIntegerCast(CttzCaller, i32Ty, false,  "ZExti16Toi32", caller);
    // Mask with (Vector-width-1), this is needed for the case of zero mask!
    Value *MaskVecWidth = ConstantInt::get(i32Ty, MaskTy->getNumElements()-1);
    CttzCaller = BinaryOperator::CreateAnd(CttzCaller, MaskVecWidth,  "ModuloVecWidthMask", caller);

    // Set Idx to cttz return value
    Idx = CttzCaller;
  }
  // Ptr' = Ptr + Index[idx]
  Value *Index0 = ExtractElementInst::Create(Index, Idx, "ExtractIndex0", caller);
  Ptr = GetElementPtrInst::Create(Ptr, Index0, "Ptr+Index[0]", caller);

#if 0
  // Index' = truncate(Index - broadcast(Index[idx]), <16x32>)
  Index0 = VectorizerUtils::createBroadcast(Index0, IndexType->getNumElements(), caller);
  Index = BinaryOperator::CreateNUWSub(Index, Index0, "Index-Index[0]", caller);
  Index = BitCastInst::CreateIntegerCast(Index, i32Vec, true, "trunc64To32", caller);
#else
  // This sequance is more efficient because subtract on 32bit values
  // costs much less than subtract on 64bit values on MIC.
  // Index = truncate(Index, <16x32>)
  // Index' = Index - broadcast(Index[idx])
  Index = BitCastInst::CreateIntegerCast(Index, i32Vec, true, "trunc64To32", caller);
  Index0 = ExtractElementInst::Create(Index, Idx, "ExtractTruncIndex0", caller);
  Index0 = VectorizerUtils::createBroadcast(Index0, IndexType->getNumElements(), caller);
  Index = BinaryOperator::CreateSub(Index, Index0, "Index-Index[0]", caller);
#endif
}

} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  FunctionPass* createGatherScatterResolverPass() {
    return new intel::MICResolver();
  }
}
