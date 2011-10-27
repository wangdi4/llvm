/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/

#include "DXWrapperParamsCheck.h"
#include "Mangler.h"

namespace intel {

bool DXWrapperParamsCheck::checkSameTypeScalar(const Type* packetizeType,
                                                    const Type* scalarizeType)
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

bool DXWrapperParamsCheck::checkSameTypeConstant(const Type* packetizeType,
                                                    Value* scalarizeVal)
{
  // checks if params are the same
  if (packetizeType != scalarizeVal->getType()) return false;
  // return true iff parameter is constant
  if (isa<Constant>(scalarizeVal)) return true;
  return false;
}

bool DXWrapperParamsCheck::checkVec(const Type* packetizeType,
                           const Type* scalarizeType, unsigned packetWidth)
{
  bool scalarizeTypeOK = (scalarizeType->isIntegerTy() ||
                       scalarizeType->isFloatTy());
  return scalarizeTypeOK && (packetizeType == VectorType::get(
      scalarizeType, packetWidth));
}

bool DXWrapperParamsCheck::checkSOA(const Type* packetizeType,
                           const Type* scalarizeType, unsigned packetWidth)
{
  const VectorType* vType = dyn_cast<VectorType>(scalarizeType);
  if (vType){
    unsigned numElements = vType->getNumElements();
    const Type* primitive = vType->getElementType(); 
    bool scalarizeTypeOK = (primitive->isIntegerTy(32) ||
                            primitive->isFloatTy());
    return scalarizeTypeOK && (packetizeType == ArrayType::get(
        VectorType::get(primitive, packetWidth),numElements));
  }
  return false;
}

DXWrapperParamsCheck::DXWrapperRetType DXWrapperParamsCheck::checkRet
  (const Type* packetizeRetType, const Type* scalarizeRetType, unsigned packetWidth)
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
  std::string name = CI->getCalledFunction()->getNameStr();
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
