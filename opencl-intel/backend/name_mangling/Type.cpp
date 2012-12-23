/****************************************************************************
Copyright (c) Intel Corporation (2012,2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN AS IS BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name: Type.cpp

\****************************************************************************/

#include "Type.h"
#include "TypeCast.h"
#include "Utils.h"
#include <assert.h>
#include <cctype>
#include <sstream>
#include <algorithm>

namespace reflection{
//compares two string vectors.
//returns if the two vectors has the same length and contents, or false
//otehrwise.
bool compare(const std::vector<std::string>& left,
             const std::vector<std::string>& right){
  if (left.size() != right.size())
    return false;
  std::vector<std::string>::const_iterator leftIt = left.begin(),
  rightIt = right.begin();
  while (leftIt != left.end() && rightIt != right.end()) {
    if (*leftIt != *rightIt)
      return false;
    leftIt++;
    rightIt++;
  }
  return true;
}

//
//Type
//

Type::Type(primitives::Primitive p): m_primitive(p){
}

Type::~Type(){}

std::string Type::toString()const{
  assert
  (
    (m_primitive>primitives::FIRST && m_primitive<=primitives::NONE) &&
    "illegal primitive"
  );
  return readableString(this);
}

Type* Type::clone()const{
  Type* ret = new Type(m_primitive);
  ret->m_attributes = m_attributes;
  return ret;
}

void Type::addAttribute(const std::string& a){
  m_attributes.push_back(a);
}

std::vector<std::string>::const_iterator Type::beginAttributes()const{
  return m_attributes.begin();
}

std::vector<std::string>::const_iterator Type::endAttributes()const{
  return m_attributes.end();
}

std::vector<std::string>::const_reverse_iterator
Type::rbeginAttributes()const{
  return m_attributes.rbegin();
}

std::vector<std::string>::const_reverse_iterator
Type::rendAttributes()const{
  return m_attributes.rend();
}

bool Type::equals(const Type* t)const{
  if (this == t)
    return true;
  return eq(t) && t->eq(this);
}

void Type::accept(TypeVisitor* v)const{
  v->visit(this);
}

bool Type::eq(const Type* t)const{
  return m_primitive == t->m_primitive;
}

bool Type::isPrimiteTy()const{
  return true;
}

//
//Pointer
//

Pointer::Pointer(const Type* t):Type(t->getPrimitive()), m_pType(t->clone()){
  m_attributes.insert(
    m_attributes.begin(),
    t->beginAttributes(),
    t->endAttributes()
  );
}

Pointer::~Pointer(){
  assert(m_pType && "pointer pointing to NULL!");
  delete m_pType;
}

std::string Pointer::toString()const{
  std::stringstream ret;
  for(std::vector<std::string>::const_iterator it = m_attributes.begin();
    it != m_attributes.end();
    it++)
    ret << *it << " ";
  ret << m_pType->toString() << " *";
  return ret.str();
}

Pointer* Pointer::clone()const{
  Pointer* p = new Pointer(m_pType);
  p->m_attributes = m_attributes;
  return p;
}

void Pointer::accept(TypeVisitor* v)const{
  v->visit(this);
}

bool Pointer::eq(const Type* t)const{
  const Pointer* p = cast<Pointer>((Type*)t);
  return p && m_pType->equals(p->m_pType);
}

const Type* Pointer::getPointee()const{
  return m_pType;
}

bool Pointer::isPrimiteTy()const{
  return false;
}
//
//Vector
//

Vector::Vector(const Type* t, int l): Type(t->getPrimitive()), m_len(l){
}

Vector::Vector(const Vector& v ): Type(v.m_primitive), m_len(v.m_len){
  m_attributes = v.m_attributes;
}


int Vector::getLen()const{
  return m_len;
}

std::string Vector::toString()const{
  std::stringstream myName;
  assert
  (
    (m_primitive>primitives::FIRST && m_primitive<=primitives::NONE) &&
    "illegal primitive"
  );
  myName << readableString(this);
  myName << m_len;
  return myName.str();
}

Vector* Vector::clone()const{
  Vector* ret = new Vector(*this);
  return ret;
}

void Vector::accept(TypeVisitor* v)const{
  v->visit(this);
}

bool Vector::eq(const Type* t)const{
  const Vector* pVec = cast<Vector>((Type*)t);
  return pVec && (m_len == pVec->m_len) && (m_primitive == pVec->m_primitive);
}

bool Vector::isPrimiteTy()const{
  return false;
}
//
//UserDefinedTy
//
//
UserDefinedTy::UserDefinedTy(const std::string& name):
Type(primitives::NONE), m_name(name){
}

Type* UserDefinedTy::clone()const{
  return new UserDefinedTy(m_name);
}

void UserDefinedTy::accept(TypeVisitor* v)const{
  v->visit(this);
}

std::string UserDefinedTy::toString()const{
  return m_name;
}

primitives::Primitive UserDefinedTy::getPrimitive()const{
  return primitives::NONE;
}

bool UserDefinedTy::eq(const Type* t)const{
  const UserDefinedTy* pTy = cast<UserDefinedTy>((Type*)t);
  return pTy && (m_name == pTy->m_name);
}

bool UserDefinedTy::isPrimiteTy()const{
  return false;
}
//
//static enums
//
TypeEnum Type::enumTy = PrimitveTy;
TypeEnum Pointer::enumTy   = PointerTy;
TypeEnum Vector::enumTy    = VectorTy;
TypeEnum UserDefinedTy::enumTy = StructTy;

}//end reflection
