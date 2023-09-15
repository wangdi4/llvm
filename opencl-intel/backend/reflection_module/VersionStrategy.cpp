// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "VersionStrategy.h"
#include "llvm/Transforms/SYCLTransforms/Utils/NameMangleAPI.h"
#include <sstream>

using namespace llvm;
using namespace llvm::reflection;
using namespace llvm::NameMangleAPI;

namespace Reflection {

//
// PairSW
//

PairSW::PairSW(const std::pair<std::string, width::V> &sw)
    : std::pair<std::string, width::V>(sw) {}

bool PairSW::operator<(const PairSW &that) const {
  if (second != that.second)
    return second < that.second;
  // We need to find which object (if at all) contains wild-cards
  if (compareWild(first, that.first) || compareWild(that.first, first))
    return false;
  return first.compare(that.first) < 0;
}

bool PairSW::compareWild(const std::string &w, const std::string &s) const {
  if (w.find('*') == std::string::npos)
    return w == s;
  std::string::const_iterator wb = w.begin(), sb = s.begin();
  while (sb != s.end() && wb != w.end()) {
    if ('*' != *wb) {
      if (*wb != *sb)
        return false;
      ++wb;
      ++sb;
    } else {
      if (w.end() != (wb + 1) && *(wb + 1) == *sb)
        ++wb;
      else
        ++sb;
    }
  }
  if (s.end() != sb)
    return false;
  if (w.end() != wb && '*' == *wb)
    ++wb;
  return w.end() == wb;
}

VersionStrategy::~VersionStrategy() {}

//
// NullDescriptorStrategy
//

PairSW NullDescriptorStrategy::operator()(const PairSW &) const {
  return nullPair();
}

NullDescriptorStrategy::~NullDescriptorStrategy() {}

//
// SoaDescriptorStrategy
//

SoaDescriptorStrategy::SoaDescriptorStrategy() : m_pTypeMap(nullptr) {}

void SoaDescriptorStrategy::setTypeMap(const ReturnTypeMap *pMap) {
  m_pTypeMap = pMap;
}

SoaDescriptorStrategy::~SoaDescriptorStrategy() {}

void SoaDescriptorStrategy::visit(const PrimitiveType *) {
  m_transposeStrategy = &SoaDescriptorStrategy::scalarReturnTranspose;
}

void SoaDescriptorStrategy::visit(const AtomicType *) {
  m_transposeStrategy = &SoaDescriptorStrategy::scalarReturnTranspose;
}

void SoaDescriptorStrategy::visit(const BlockType *) {
  m_transposeStrategy = &SoaDescriptorStrategy::scalarReturnTranspose;
}

void SoaDescriptorStrategy::visit(const UserDefinedType *) {
  m_transposeStrategy = &SoaDescriptorStrategy::scalarReturnTranspose;
}

void SoaDescriptorStrategy::visit(const VectorType *) {
  m_transposeStrategy = &SoaDescriptorStrategy::vectorReturnTranspose;
}

void SoaDescriptorStrategy::visit(const PointerType *) {
  assert(false && "unreachable code");
}

PairSW SoaDescriptorStrategy::operator()(const PairSW &sw) const {
  assert(m_pTypeMap && "NULL type map! (not assigned?)");
  const std::string name = sw.first;
  width::V transposeWidth = sw.second;
  // If the given function is already the scalar version, we return it write
  // back, Otherwise we disallow the scalarization
  if (transposeWidth == width::SCALAR) {
    const std::string soa_prefix("soa_");
    if (soa_prefix == name.substr(0, soa_prefix.length()))
      return nullPair();
    FunctionDescriptor identity = demangle(name.c_str());
    identity.Width = width::SCALAR;
    return fdToPair(identity);
  }
  TypeVisitor *pVisitor = const_cast<SoaDescriptorStrategy *>(this);
  const FunctionDescriptor fdOrig = demangle(sw.first.c_str());
  ReturnTypeMap::const_iterator it = m_pTypeMap->find(fdOrig);
  assert(m_pTypeMap->end() != it &&
         "this type cannot be transposed (forgot to add it to transpose map?");
  RefParamType retTy = it->second;
  assert(retTy && "return type is NULL");
  // activating the double dispatach, so the transpose stategy will be applied
  retTy->accept(pVisitor);
  FunctionDescriptor transposed = (this->*m_transposeStrategy)(sw);
  transposed.Width = sw.second;
  return fdToPair(transposed);
}

// Note! all the parematers of the created function descriptors are allocated on
// the heap. Since there life span is that of the BuiltKeeper object that
// contains them, and that object is a singleton, they are never released.
FunctionDescriptor
SoaDescriptorStrategy::scalarReturnTranspose(const PairSW &sw) const {
  const FunctionDescriptor &orig = demangle(sw.first.c_str());
  width::V transposeWidth = sw.second;
  FunctionDescriptor fd = FunctionDescriptor::null();
  std::string Name;
  if (orig.Parameters.size() > 0) {
    std::stringstream nameBuilder;
    const VectorType *pVec = dyn_cast<VectorType>(orig.Parameters[0].get());
    int nameWidth = pVec ? pVec->getLength() : 1;
    nameBuilder << "soa_" << std::string(orig.Name) << nameWidth;
    Name = nameBuilder.str();
    fd.Name = Name;
  }
  for (size_t i = 0; i < orig.Parameters.size(); ++i) {
    const VectorType *pVector = dyn_cast<VectorType>(orig.Parameters[i].get());
    RefParamType pParam = orig.Parameters[i];
    width::V paramWidth = width::SCALAR;
    if (pVector) {
      paramWidth = static_cast<width::V>(pVector->getLength());
      pParam = pVector->getScalarType();
    }
    const PrimitiveType *pPrimitive = dyn_cast<PrimitiveType>(pParam.get());
    assert(pPrimitive && "Parameter has no primitive type");
    TypePrimitiveEnum p = pPrimitive->getPrimitive();
    for (unsigned j = 0; j < static_cast<unsigned>(paramWidth); ++j) {
      RefParamType scalar(new PrimitiveType(p));
      RefParamType transposedParam(
          new VectorType(scalar, (int)(transposeWidth)));
      fd.Parameters.push_back(transposedParam);
    }
  }
  fd.Width = width::V::NONE;
  return fd;
}

FunctionDescriptor
SoaDescriptorStrategy::vectorReturnTranspose(const PairSW &sw) const {
  const std::string name = sw.first;
  FunctionDescriptor fdOrig = demangle(name.c_str());
  // we now add the parameter for the return type, one vector pointer for each
  // transpose-size
  FunctionDescriptor fd = scalarReturnTranspose(sw);
  // we can safely downcast here, since the visitor's double-dispatch ensures us
  // that Vector is indeed the dynamic type of m_returnTy
  ReturnTypeMap::const_iterator it = m_pTypeMap->find(fdOrig);
  assert(m_pTypeMap->end() != it &&
         "this type cannot be transposed (forgot to add it to transpose map?");
  RefParamType retTy = it->second;
  // Calculating the OUT parameter type
  VectorType *retVecTy = dyn_cast<VectorType>(&*retTy);
  assert(retVecTy && "non-vector return type");
  RefParamType vOut(new VectorType(retVecTy->getScalarType(), sw.second));
  // All pointers for return data have private address space
  RefParamType ptr(new PointerType(vOut, {ATTR_PRIVATE}));
  for (int i = 0; i < retVecTy->getLength(); ++i)
    fd.Parameters.push_back(ptr);
  return fd;
}

//
// Identity strategy.
//
PairSW IdentityStrategy::operator()(const PairSW &sw) const { return sw; }
//
// Pair conversions
//

std::pair<std::string, width::V> fdToPair(const FunctionDescriptor &fd) {
  return std::make_pair(mangle(fd), fd.Width);
}

std::pair<std::string, width::V> nullPair() {
  return std::make_pair(std::string(FunctionDescriptor::nullString()),
                        width::NONE);
}

bool isNullPair(const std::pair<std::string, width::V> &sw) {
  return sw.first == FunctionDescriptor::nullString();
}
} // namespace Reflection
