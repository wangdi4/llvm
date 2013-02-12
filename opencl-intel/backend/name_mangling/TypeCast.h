/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __TYPE_CAST_H__
#define __TYPE_CAST_H__

#include "Type.h"

namespace reflection{

struct TypeCastVisitor : TypeVisitor{
  void visit(const Type*);
  void visit(const Vector*);
  void visit(const Pointer*);
  void visit(const UserDefinedTy*);
  
  template <typename T>
  const T* nullOrCast(const Type*)const;

  template <typename T>
  T* nullOrCast(Type*)const;
private:
  TypeEnum m_enumTy;
};

template <typename T>
T* dyn_cast(Type* pType){
  TypeCastVisitor typeCast;
  pType->accept(&typeCast);
  return typeCast.nullOrCast<T>(pType);
}

template <typename T>
const T* dyn_cast(const Type* pType){
  TypeCastVisitor typeCast;
  pType->accept(&typeCast);
  return typeCast.nullOrCast<T>(pType);
}

template <typename T>
T* TypeCastVisitor::nullOrCast(Type* pType)const{
  return (T::enumTy == m_enumTy) ?(T*)pType : NULL;
}

template <typename T>
const T* TypeCastVisitor::nullOrCast(const Type* pType)const{
  return (T::enumTy == m_enumTy) ?(T*)pType : NULL;
}

}//end namespace reflection
#endif//__TYPE_CAST_H__
