// INTEL CONFIDENTIAL
//
// Copyright 2016-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#define DEBUG_TYPE "avx512resolver"

#include "AVX512Resolver.h"
#include "VectorizerUtils.h"
#include "Logger.h"
#include "OCLPassSupport.h"

#include "llvm/IR/InstIterator.h" 
#include "llvm/IR/Constants.h"
#include "llvm/Support/CommandLine.h"

#include <sstream>
#include <vector>

static bool isGatherScatterType(VectorType *VecTy) {
  return VecTy->getNumElements() == 16;
}

namespace intel {

char AVX512Resolver::ID = 0;

OCL_INITIALIZE_PASS(AVX512Resolver, "avx512resolve", "Resolves masked and vectorized function calls for AVX512", false, false)


static Value* getConsecutiveConstantVector(Type* type, unsigned count) {
  std::vector<Constant*> constList;
  uint64_t constVal = 0;

  for (unsigned j = 0; j < count; ++j) {
    constList.push_back(ConstantInt::get(type, constVal++));
  }
  return ConstantVector::get(ArrayRef<Constant*>(constList));
}


bool AVX512Resolver::TargetSpecificResolve(CallInst* caller) {
  Function* called = caller->getCalledFunction();
  V_ASSERT(called && "Unexpected indirect function invocation");
  std::string calledName = called->getName().str();
  V_PRINT(DEBUG_TYPE, "AVX512SpecificResolve Inspecting " << calledName << "\n");

  // Use name to decide what to do

  if (Mangler::isMangledGather(calledName)) {
    Value *Mask      = caller->getArgOperand(0);
    Value *Ptr       = caller->getArgOperand(1);
    Value *Index     = caller->getArgOperand(2);
    Value *ValidBits = caller->getArgOperand(3);
    Value *IsSigned  = caller->getArgOperand(4);

    FixBaseAndIndexIfNeeded(caller, Mask, ValidBits, IsSigned, Ptr, Index);
    CreateGatherScatterAndReplaceCall(caller, Mask, Ptr, Index, nullptr, Mangler::Gather);
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
    CreateGatherScatterAndReplaceCall(caller, Mask, Ptr, Index, nullptr, Mangler::GatherPrefetch);
    return true;
  }

  return false;
}

Instruction* AVX512Resolver::CreateGatherScatterAndReplaceCall(
    CallInst* caller, Value *Mask, Value *Ptr, Value *Index, Value *Data, Mangler::GatherScatterType type) {
  Module *pModule = caller->getParent()->getParent()->getParent();
  V_ASSERT((type == Mangler::GatherPrefetch || (Data ? Data->getType() : caller->getType())->isVectorTy())
    && "Data value type is not a vector");

  VectorType *dataTy = nullptr;

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
  VectorType *IndexType = cast<VectorType>(Index->getType());
  std::string name = Mangler::getGatherScatterName(isMasked, type, dataTy, IndexType);

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

void AVX512Resolver::FixBaseAndIndexIfNeeded(
                  CallInst* caller, Value *Mask, Value *ValidBits,
                  Value *IsSigned, Value *&Ptr, Value *&Index) {
  V_ASSERT(ValidBits && isa<ConstantInt>(ValidBits) && "ValidBits argument is not constant");
  V_ASSERT(IsSigned && isa<ConstantInt>(IsSigned) && "IsSigned argument is not constant");
  V_ASSERT(Ptr && Ptr->getType()->isPointerTy() && "Ptr type is not a pointer");

  unsigned int uValidBits = (unsigned int)cast<ConstantInt>(ValidBits)->getZExtValue();
  bool bIsSigned = !cast<Constant>(IsSigned)->isNullValue();

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

      Ptr = GetElementPtrInst::Create(nullptr, Ptr, PosMax32BitInt, "Base+safeNumFix", caller);
      Index = BinaryOperator::CreateAdd(Index, NegMax32BitInt, "Index-safeNumFix", caller);
      V_PRINT(gather_scatter_stat, "RESOLVER: Base+safeNumFix " << *Ptr <<
        "\t|\t RESOLVER: Index-safeNumFix " << *Index << "\n");
    }
    //return;
  }
}

bool AVX512Resolver::isBitMask(const VectorType& vecType) const {
  // float16, int16, double8, long8, double16, long16
  return (vecType.getBitWidth() >= 512) && (vecType.getNumElements() <= 16);
}


} // namespace

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
  FunctionPass* createAVX512ResolverPass() {
    return new intel::AVX512Resolver();
  }
}

