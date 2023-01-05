//===- FunctionDescriptor.cpp - Function descriptor utilities -------------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/FunctionDescriptor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/ParameterType.h"

using namespace llvm;
using namespace llvm::reflection;

namespace {
///////////////////////////////////////////////////////////////////////////////
// Purpose: helps to determines the vector width of a given function descriptor
// Note: we could in fact use plain poly-morphism here, (no covariance problem
// here), but this will compel us to add the getWidth method throughout the Type
// inheritance tree, when it is only needed here.
///////////////////////////////////////////////////////////////////////////////
struct VWidthResolver : TypeVisitor {
  void visit(const PrimitiveType *) override { Width = width::SCALAR; }

  void visit(const VectorType *v) override {
    Width = static_cast<width::V>(v->getLength());
  }

  void visit(const PointerType *p) override { p->getPointee()->accept(this); }

  void visit(const AtomicType *) override { Width = width::SCALAR; }

  void visit(const BlockType *) override { Width = width::SCALAR; }

  void visit(const UserDefinedType *) override { Width = width::SCALAR; }

  width::V width() const { return Width; }

private:
  width::V Width = width::NONE;
};

} // namespace

StringRef FunctionDescriptor::nullString() { return StringRef("<invalid>"); }

std::string FunctionDescriptor::toString() const {
  std::string Str;
  raw_string_ostream S(Str);
  if (isNull()) {
    S << "null descriptor";
    return Str;
  }
  S << Name << "(";
  size_t ParamCount = Parameters.size();
  if (ParamCount > 0) {
    for (size_t i = 0; i < ParamCount - 1; ++i)
      S << Parameters[i]->toString() << ", ";
    S << Parameters[ParamCount - 1]->toString();
  }
  S << ")";
  return Str;
}

static bool equal(const TypeVector &l, const TypeVector &r) {
  if (&l == &r)
    return true;
  if (l.size() != r.size())
    return false;
  TypeVector::const_iterator itl = l.begin(), itr = r.begin(), endl = l.end();
  while (itl != endl) {
    if (!(*itl)->equals(itr->get()))
      return false;
    ++itl;
    ++itr;
  }
  return true;
}

bool FunctionDescriptor::operator==(const FunctionDescriptor &that) const {
  if (this == &that)
    return true;
  if (Name != that.Name)
    return false;
  return ::equal(Parameters, that.Parameters);
}

bool FunctionDescriptor::operator<(const FunctionDescriptor &that) const {
  int Cmp = Name.compare(that.Name);
  if (Cmp)
    return (Cmp < 0);
  size_t Len = Parameters.size(), ThatLen = that.Parameters.size();
  if (Len != ThatLen)
    return Len < ThatLen;
  TypeVector::const_iterator It = Parameters.begin(), E = Parameters.end(),
                             ThatIt = that.Parameters.begin();
  while (It != E) {
    int cmp = (*It)->toString().compare((*ThatIt)->toString());
    if (cmp)
      return (cmp < 0);
    ++ThatIt;
    ++It;
  }
  return false;
}

void FunctionDescriptor::assignAutomaticWidth() {
  VWidthResolver Resolver;
  width::V W = width::SCALAR;
  size_t ParamCount = Parameters.size();
  for (size_t i = 0; i < ParamCount; ++i) {
    Parameters[i]->accept(&Resolver);
    if (W < Resolver.width())
      W = Resolver.width();
  }
  Width = W;
}

bool FunctionDescriptor::isNull() const {
  return (Name.empty() && Parameters.empty());
}

FunctionDescriptor FunctionDescriptor::null() {
  FunctionDescriptor FD;
  FD.Name = "";
  FD.Width = width::V::NONE;
  return FD;
}
