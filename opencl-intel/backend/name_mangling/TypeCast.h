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
  T* nullOrCast(Type*)const;
private:
  TypeEnum m_enumTy;
};

template <typename T>
T* cast(Type* pType){
  TypeCastVisitor typeCast;
  pType->accept(&typeCast);
  return typeCast.nullOrCast<T>(pType);
}

template <typename T>
T* TypeCastVisitor::nullOrCast(Type* pType)const{
  return (T::enumTy == m_enumTy) ?(T*)pType : NULL;
}

}//end namespace reflection
#endif//__TYPE_CAST_H__
