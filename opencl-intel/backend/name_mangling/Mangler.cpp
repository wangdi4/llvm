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

#include "FunctionDescriptor.h"
#include "ParameterType.h"
#include "Utils.h"
#include <algorithm>
#include <assert.h>
#include <map>
#include <sstream>
#include <string>

//
// Implementation of an API.
// Mangle the given function descriptor to a mangled name.
// The mangling algorithm is intendent to match the Itanium mangling altorithm.
// More concretly, it is designed to match clang 3.0 itanium mangling.
//

typedef std::vector<reflection::TypeAttributeEnum> TypeAttrVec;

class MangleVisitor : public reflection::TypeVisitor {
public:
  MangleVisitor(std::stringstream &s) : m_stream(s) {}

  void operator()(const reflection::ParamType *t) { t->accept(this); }

  // visit methods
  void visit(const reflection::PrimitiveType *t) {
    int typeIndex = getTypeIndex(t);
    if (-1 != typeIndex) {
      m_stream << reflection::getDuplicateString(typeIndex);
      return;
    }
    m_stream << reflection::mangledPrimitiveString(t->getPrimitive());
    if (t->getPrimitive() >= reflection::PRIMITIVE_STRUCT_FIRST &&
        t->getPrimitive() <= reflection::PRIMITIVE_LAST)
      m_dupList.push_back((reflection::ParamType *)t);
  }

  void visit(const reflection::PointerType *p) {
    int typeIndex = getTypeIndex(p);
    if (-1 != typeIndex) {
      m_stream << reflection::getDuplicateString(typeIndex);
      return;
    }
    m_stream << "P";
    for (unsigned int i = 0; i < p->getAttributes().size(); ++i) {
      m_stream << getMangledAttribute(p->getAttributes()[i]);
    }
    p->getPointee()->accept(this);

    assert(p->getAttributes().size() > 0 &&
           "Pointers always have attributes (at least address space)");

    // Add dummy type to preserve substitutions order
    // TODO: implement correct way to handle substitutions with qualifiers
    // See more details in DemangleParser.cpp
    m_dupList.push_back(reflection::RefParamType());

    m_dupList.push_back((reflection::ParamType *)p);
  }

  void visit(const reflection::VectorType *v) {
    int typeIndex = getTypeIndex(v);
    if (-1 != typeIndex) {
      m_stream << reflection::getDuplicateString(typeIndex);
      return;
    }
    m_stream << "Dv" << v->getLength() << "_";
    v->getScalarType()->accept(this);
    m_dupList.push_back((reflection::ParamType *)v);
  }

  void visit(const reflection::AtomicType *p) {
    int typeIndex = getTypeIndex(p);
    if (-1 != typeIndex) {
      m_stream << reflection::getDuplicateString(typeIndex);
      return;
    }
    m_stream << "U"
             << "7_Atomic";
    p->getBaseType()->accept(this);
    m_dupList.push_back((reflection::ParamType *)p);
  }

  void visit(const reflection::BlockType *p) {
    int typeIndex = getTypeIndex(p);
    if (-1 != typeIndex) {
      m_stream << reflection::getDuplicateString(typeIndex);
      return;
    }
    m_stream << "U"
             << "13block_pointerFv";
    for (unsigned int i = 0; i < p->getNumOfParams(); ++i) {
      p->getParam(i)->accept(this);
    }
    m_dupList.push_back((reflection::ParamType *)p);
    m_stream << "E";
  }

  void visit(const reflection::UserDefinedType *pTy) {
    int typeIndex = getTypeIndex(pTy);
    if (-1 != typeIndex) {
      m_stream << reflection::getDuplicateString(typeIndex);
      return;
    }
    std::string name = pTy->toString();
    m_stream << name.size() << name;
    m_dupList.push_back((reflection::ParamType *)pTy);
  }

private:
  int getTypeIndex(const reflection::ParamType *t) const {
    for (unsigned int i = 0; i < m_dupList.size(); ++i) {
      if (t->equals(m_dupList[i])) {
        return i;
      }
    }
    return -1;
  }

  // holds the mangled string representing the prototype of the function
  std::stringstream &m_stream;
  // list of types 'seen' so far
  reflection::DuplicatedTypeList m_dupList;
};

std::string mangle(const reflection::FunctionDescriptor &fd) {
  if (fd.isNull())
    return reflection::FunctionDescriptor::nullString();
  std::stringstream ret;
  ret << "_Z" << fd.name.length() << fd.name;
  MangleVisitor visitor(ret);
  for (unsigned int i = 0; i < fd.parameters.size(); ++i) {
    fd.parameters[i]->accept(&visitor);
  }
  return ret.str();
}
