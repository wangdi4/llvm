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
#include <sstream>

namespace reflection {

///////////////////////////////////////////////////////////////////////////////
// Purpose: helps to determines the vector width of a given function descriptor
// Note: we could in fact use plain poly-morphism here, (no covariance problem
// here), but this will compel us to add the getWidth method throughout the Type
// inheritance tree, when it is only needed here.
///////////////////////////////////////////////////////////////////////////////
struct VWidthResolver : TypeVisitor {
  void visit(const PrimitiveType *) { m_width = width::SCALAR; }

  void visit(const VectorType *v) {
    m_width = static_cast<width::V>(v->getLength());
  }

  void visit(const PointerType *p) { p->getPointee()->accept(this); }

  void visit(const AtomicType *) { m_width = width::SCALAR; }

  void visit(const BlockType *) { m_width = width::SCALAR; }

  void visit(const UserDefinedType *) { m_width = width::SCALAR; }

  width::V width() const { return m_width; }

private:
  width::V m_width;
};

llvm::StringRef FunctionDescriptor::nullString() {
  return llvm::StringRef("<invalid>");
}

std::string FunctionDescriptor::toString() const {
  std::stringstream stream;
  if (isNull()) {
    stream << "null descriptor";
    return stream.str();
  }
  stream << name << "(";
  size_t paramCount = parameters.size();
  if (paramCount > 0) {
    for (size_t i = 0; i < paramCount - 1; ++i)
      stream << parameters[i]->toString() << ", ";
    stream << parameters[paramCount - 1]->toString();
  }
  stream << ")";
  return stream.str();
}

static bool equal(const TypeVector &l, const TypeVector &r) {
  if (&l == &r)
    return true;
  if (l.size() != r.size())
    return false;
  TypeVector::const_iterator itl = l.begin(), itr = r.begin(), endl = l.end();
  while (itl != endl) {
    if (!(*itl)->equals(*itr))
      return false;
    ++itl;
    ++itr;
  }
  return true;
}

//
// FunctionDescriptor
//

bool FunctionDescriptor::operator==(const FunctionDescriptor &that) const {
  if (this == &that)
    return true;
  if (name != that.name)
    return false;
  return equal(parameters, that.parameters);
}

bool FunctionDescriptor::operator<(const FunctionDescriptor &that) const {
  int strCmp = name.compare(that.name);
  if (strCmp)
    return (strCmp < 0);
  size_t len = parameters.size(), thatLen = that.parameters.size();
  if (len != thatLen)
    return len < thatLen;
  TypeVector::const_iterator it = parameters.begin(), e = parameters.end(),
                             thatit = that.parameters.begin();
  while (it != e) {
    int cmp = (*it)->toString().compare((*thatit)->toString());
    if (cmp)
      return (cmp < 0);
    ++thatit;
    ++it;
  }
  return false;
}

void FunctionDescriptor::assignAutomaticWidth() {
  VWidthResolver widthResolver;
  width::V w = width::SCALAR;
  size_t paramCount = parameters.size();
  for (size_t i = 0; i < paramCount; ++i) {
    parameters[i]->accept(&widthResolver);
    if (w < widthResolver.width())
      w = widthResolver.width();
  }
  width = w;
}

bool FunctionDescriptor::isNull() const {
  return (name.empty() && parameters.empty());
}

FunctionDescriptor FunctionDescriptor::null() {
  FunctionDescriptor fd;
  fd.name = "";
  fd.width = width::V::NONE;
  return fd;
}
}
