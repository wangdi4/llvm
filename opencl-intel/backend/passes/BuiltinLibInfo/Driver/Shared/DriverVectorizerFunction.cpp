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

#include "DriverVectorizerFunction.h"
#include "BuiltinKeeper.h"
#include "Logger.h"
#include "Mangler.h"
#include "VectorizerFunction.h"

#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/NameMangleAPI.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ParameterType.h"

using namespace Reflection;
using namespace llvm::reflection;
using namespace llvm::NameMangleAPI;

namespace intel {

DriverVectorizerFunction::DriverVectorizerFunction(const std::string &s)
    : m_name(s) {}

DriverVectorizerFunction::~DriverVectorizerFunction() {}

unsigned DriverVectorizerFunction::getWidth() const {
  assert(!isNull() && "Null function");
  const BuiltinKeeper *pKeeper = BuiltinKeeper::instance();
  if (!pKeeper->isBuiltin(m_name))
    return width::NONE;
  width::V allWidth[] = {width::SCALAR, width::TWO,   width::THREE,
                         width::FOUR,   width::EIGHT, width::SIXTEEN};
  for (size_t i = 0; i < width::OCL_VERSIONS; ++i) {
    PairSW sw = pKeeper->getVersion(m_name, allWidth[i]);
    if (m_name == sw.first)
      return sw.second;
  }
  assert((isMangled() || CompilationUtils::isPipeBuiltin(m_name)) &&
         "not a mangled name, cannot determine function width");
  // if we reached here, that means that function cannot be versioned, so our
  // best option is to apply the automatic width detection.
  FunctionDescriptor ret = demangle(m_name.c_str());
  ret.assignAutomaticWidth();
  return ret.Width;
}

bool DriverVectorizerFunction::isPacketizable() const {
  // all builtin version has a width 4 version is they are packetizable
  const BuiltinKeeper *pKeeper = BuiltinKeeper::instance();
  if (!pKeeper->isBuiltin(m_name))
    return false;
  PairSW version4 = pKeeper->getVersion(m_name, width::FOUR);
  return !isNullPair(version4);
}

bool DriverVectorizerFunction::isScalarizable() const {
  const BuiltinKeeper *pKeeper = BuiltinKeeper::instance();
  if (!pKeeper->isBuiltin(m_name))
    return false;
  PairSW sw = pKeeper->getVersion(m_name, width::SCALAR);
  return !isNullPair(sw);
}

std::string DriverVectorizerFunction::getVersion(unsigned index) const {
  // we need to comply with the 'wiered' indexing system of the interface
  const BuiltinKeeper *pKeeper = BuiltinKeeper::instance();
  if (!pKeeper->isBuiltin(m_name))
    return std::string(FunctionDescriptor::nullString());
  width::V w;
  switch (index) {
  case 0U:
    w = width::SCALAR;
    break;
  case 1U:
    w = width::TWO;
    break;
  case 2U:
    w = width::FOUR;
    break;
  case 3U:
    w = width::EIGHT;
    break;
  case 4U:
    w = width::SIXTEEN;
    break;
  case 5U:
    w = width::THREE;
    break;
  default:
    assert(false && "invalid index");
    return std::string(FunctionDescriptor::nullString());
  }
  PairSW sw = pKeeper->getVersion(m_name, w);
  assert(sw.second == w && "requested width doesn't match");
  return sw.first;
}

bool DriverVectorizerFunction::isNull() const {
  const BuiltinKeeper *pKeeper = BuiltinKeeper::instance();
  return !pKeeper->isBuiltin(m_name);
}

bool DriverVectorizerFunction::isMangled() const {
  const std::string prefix = "_Z";
  return (prefix == m_name.substr(0, prefix.size()));
}

} // namespace intel
