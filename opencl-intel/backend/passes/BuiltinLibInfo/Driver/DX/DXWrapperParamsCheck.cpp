// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

#include "DXWrapperParamsCheck.h"
#include "Mangler.h"

#include "llvm/IR/Module.h"

namespace intel {

bool DXWrapperParamsCheck::checkSameTypeScalar(Type* packetizeType,
                                                    Type* scalarizeType)
{
  // checks if params are the same
  if (packetizeType != scalarizeType) return false;
  // checks if param type is supported
  if (scalarizeType->isIntegerTy() ||
      scalarizeType->isFloatTy() ||
      scalarizeType->isPointerTy()){
    return true;
  }
  return false;
}

bool DXWrapperParamsCheck::checkSameTypeConstant(Type* packetizeType,
                                                    Value* scalarizeVal)
{
  // checks if params are the same
  if (packetizeType != scalarizeVal->getType()) return false;
  // return true iff parameter is constant
  if (isa<Constant>(scalarizeVal)) return true;
  return false;
}

bool DXWrapperParamsCheck::checkVec(Type* packetizeType,
                           Type* scalarizeType, unsigned packetWidth)
{
  bool scalarizeTypeOK = (scalarizeType->isIntegerTy() ||
                       scalarizeType->isFloatTy());
  return scalarizeTypeOK && (packetizeType == VectorType::get(
      scalarizeType, packetWidth));
}

bool DXWrapperParamsCheck::checkSOA(Type* packetizeType,
                           Type* scalarizeType, unsigned packetWidth)
{
  const VectorType* vType = dyn_cast<VectorType>(scalarizeType);
  if (vType){
    unsigned numElements = vType->getNumElements();
    Type* primitive = vType->getElementType(); 
    bool scalarizeTypeOK = (primitive->isIntegerTy(32) ||
                            primitive->isFloatTy());
    return scalarizeTypeOK && (packetizeType == ArrayType::get(
        VectorType::get(primitive, packetWidth),numElements));
  }
  return false;
}

DXWrapperParamsCheck::DXWrapperRetType DXWrapperParamsCheck::checkRet
  (Type* packetizeRetType, Type* scalarizeRetType, unsigned packetWidth)
{
  if ( packetizeRetType->isVoidTy() && scalarizeRetType->isVoidTy()){
    return DXWrapperParamsCheck::DX_RET_VOID;
  } else if (checkVec(packetizeRetType, scalarizeRetType, packetWidth)){
    return DXWrapperParamsCheck::DX_RET_VEC;
  } else if (checkSOA(packetizeRetType, scalarizeRetType, packetWidth)){
    return DXWrapperParamsCheck::DX_RET_SOA;
  } else {
    return DXWrapperParamsCheck::DX_RET_ILLEGAL;
  }
}

bool DXWrapperParamsCheck::isDXScalarWrapperMasked (CallInst* CI, const RuntimeServices* rtServices){
  assert (CI->getCalledFunction() && "Unexpected indirect function invocation");
  std::string name = CI->getCalledFunction()->getName().str();
  bool isMangled = Mangler::isMangledCall(name);
  if (!isMangled){
    return false;
  }
  name = Mangler::demangle(name);
  const Function *origFunc = rtServices->findInRuntimeModule(name);
  assert (origFunc && "scalar function not found in hash");
  return (CI->getNumArgOperands() != 
      origFunc->getFunctionType()->getNumParams());
}




}  // namespace
