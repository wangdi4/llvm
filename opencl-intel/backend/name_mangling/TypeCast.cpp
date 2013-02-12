/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "TypeCast.h"

namespace reflection{

template <>
Type* dyn_cast(Type* pType){
  return pType;
}

template <>
const Type* dyn_cast(const Type* pType){
  return pType;
}

void TypeCastVisitor::visit(const Type*){
  m_enumTy = Type::enumTy;
}

void TypeCastVisitor::visit(const Vector*){
  m_enumTy = Vector::enumTy;
}

void TypeCastVisitor::visit(const Pointer*){
  m_enumTy = Pointer::enumTy;
}

void TypeCastVisitor::visit(const UserDefinedTy*){
  m_enumTy = UserDefinedTy::enumTy;
}

}//end namespace
