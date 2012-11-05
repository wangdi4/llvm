#include "TypeCast.h"

namespace reflection{

template <>
Type* cast(Type* pType){
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
