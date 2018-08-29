// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "ParameterType.h"
#include "Utils.h"
#include <assert.h>
#include <cctype>
#include <sstream>

namespace reflection {
//
// Primitive Type
//

PrimitiveType::PrimitiveType(TypePrimitiveEnum primitive)
    : ParamType(TYPE_ID_PRIMITIVE), m_primitive(primitive) {}

void PrimitiveType::accept(TypeVisitor *visitor) const { visitor->visit(this); }

std::string PrimitiveType::toString() const {
  assert((m_primitive >= PRIMITIVE_FIRST && m_primitive <= PRIMITIVE_LAST) &&
         "illegal primitive");
  std::stringstream myName;
  myName << readablePrimitiveString(m_primitive);
  return myName.str();
}

bool PrimitiveType::equals(const ParamType *type) const {
  if (!type)
    return false;
  const PrimitiveType *p = reflection::dyn_cast<PrimitiveType>(type);
  return p && (m_primitive == p->m_primitive);
}

void PrimitiveType::setPrimitive(TypePrimitiveEnum primitive) {
  assert((primitive >= PRIMITIVE_FIRST && primitive <= PRIMITIVE_LAST) &&
         "illegal primitive");
  m_primitive = primitive;
}

//
// Pointer Type
//

PointerType::PointerType(const RefParamType type)
    : ParamType(TYPE_ID_POINTER), m_pType(type) {}

PointerType::PointerType(const RefParamType type,
                         std::vector<TypeAttributeEnum> &&attrs)
    : ParamType(TYPE_ID_POINTER), m_pType(type),
      m_attributes(std::move(attrs)) {}

void PointerType::accept(TypeVisitor *visitor) const { visitor->visit(this); }

std::string PointerType::toString() const {
  std::stringstream myName;
  for (unsigned int i = m_attributes.size(); i > 0; --i) {
    myName << getReadableAttribute(m_attributes[i - 1]) << " ";
  }
  myName << getPointee()->toString() << " *";
  return myName.str();
}

bool PointerType::equals(const ParamType *type) const {
  if (!type)
    return false;
  const PointerType *p = reflection::dyn_cast<PointerType>(type);
  if (!p || p->getAttributes().size() != getAttributes().size()) {
    return false;
  }
  for (unsigned int i = 0; i < getAttributes().size(); ++i) {
    if (getAttributes()[i] != p->getAttributes()[i]) {
      return false;
    }
  }
  return (*getPointee()).equals(&*(p->getPointee()));
}

bool PointerType::convertAddrSpaceAttribute(TypeAttributeEnum from,
                                            TypeAttributeEnum to) {
  assert((from >= ATTR_ADDR_SPACE_FIRST && from <= ATTR_ADDR_SPACE_LAST) &&
         "Only addr space attribute can be replaced!");
  assert((to >= ATTR_ADDR_SPACE_FIRST && to <= ATTR_ADDR_SPACE_LAST) &&
         "Only addr space attribute can be applied!");
  for (std::vector<TypeAttributeEnum>::iterator attr_it = m_attributes.begin(),
                                                attr_end = m_attributes.end();
       attr_it != attr_end; attr_it++) {
    if (*attr_it == from) {
      *attr_it = to;
      return true;
    }
  }
  return false;
}

//
// Vector Type
//

VectorType::VectorType(const RefParamType type, int len)
    : ParamType(TYPE_ID_VECTOR), m_pType(type), m_len(len) {}

void VectorType::accept(TypeVisitor *visitor) const { visitor->visit(this); }

std::string VectorType::toString() const {
  std::stringstream myName;
  myName << getScalarType()->toString();
  myName << m_len;
  return myName.str();
}

bool VectorType::equals(const ParamType *type) const {
  if (!type)
    return false;
  const VectorType *pVec = reflection::dyn_cast<VectorType>(type);
  return pVec && (m_len == pVec->m_len) &&
         (*getScalarType()).equals(&*(pVec->getScalarType()));
}

//
// Atomic Type
//

AtomicType::AtomicType(const RefParamType type)
    : ParamType(TYPE_ID_ATOMIC), m_pType(type) {}

void AtomicType::accept(TypeVisitor *visitor) const { visitor->visit(this); }

std::string AtomicType::toString() const {
  std::stringstream myName;
  myName << "atomic_" << getBaseType()->toString();
  return myName.str();
}

bool AtomicType::equals(const ParamType *type) const {
  if (!type)
    return false;
  const AtomicType *a = reflection::dyn_cast<AtomicType>(type);
  return (a && (*getBaseType()).equals(&*(a->getBaseType())));
}

//
// Block Type
//

BlockType::BlockType() : ParamType(TYPE_ID_BLOCK) {}

void BlockType::accept(TypeVisitor *visitor) const { visitor->visit(this); }

std::string BlockType::toString() const {
  std::stringstream myName;
  myName << "void (";
  for (unsigned int i = 0; i < getNumOfParams(); ++i) {
    if (i > 0)
      myName << ", ";
    myName << m_params[i]->toString();
  }
  myName << ")*";
  return myName.str();
}

bool BlockType::equals(const ParamType *type) const {
  if (!type)
    return false;
  const BlockType *pBlock = reflection::dyn_cast<BlockType>(type);
  if (!pBlock || getNumOfParams() != pBlock->getNumOfParams()) {
    return false;
  }
  for (unsigned int i = 0; i < getNumOfParams(); ++i) {
    if (!getParam(i)->equals(&*pBlock->getParam(i))) {
      return false;
    }
  }
  return true;
}

//
// User Defined Type
//
//
UserDefinedType::UserDefinedType(const std::string &name)
    : ParamType(TYPE_ID_STRUCTURE), m_name(name) {}

void UserDefinedType::accept(TypeVisitor *visitor) const {
  visitor->visit(this);
}

std::string UserDefinedType::toString() const {
  std::stringstream myName;
  myName << m_name;
  return myName.str();
}

bool UserDefinedType::equals(const ParamType *pType) const {
  if (!pType)
    return false;
  const UserDefinedType *pTy = reflection::dyn_cast<UserDefinedType>(pType);
  return pTy && (m_name == pTy->m_name);
}

//
// static enums
//
const TypeEnum PrimitiveType::enumTy = TYPE_ID_PRIMITIVE;
const TypeEnum PointerType::enumTy = TYPE_ID_POINTER;
const TypeEnum VectorType::enumTy = TYPE_ID_VECTOR;
const TypeEnum AtomicType::enumTy = TYPE_ID_ATOMIC;
const TypeEnum BlockType::enumTy = TYPE_ID_BLOCK;
const TypeEnum UserDefinedType::enumTy = TYPE_ID_STRUCTURE;

} // namespace reflection {
