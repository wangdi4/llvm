/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "DriverVectorizerFunction.h"
#include "VectorizerFunction.h"
#include "BuiltinKeeper.h"
#include "NameMangleAPI.h"
#include "Mangler.h"
#include "ParameterType.h"
#include "Logger.h"

#include "llvm/Constants.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Module.h"

using namespace reflection;
namespace intel {

DriverVectorizerFunction::DriverVectorizerFunction(const std::string& s): m_name(s){
}

DriverVectorizerFunction::~DriverVectorizerFunction() {  }

unsigned DriverVectorizerFunction::getWidth()const{
  assert(!isNull() && "Null function");
  const BuiltinKeeper* pKeeper = reflection::BuiltinKeeper::instance();
  if (!pKeeper->isBuiltin(m_name))
    return width::NONE;
  width::V allWidth[] = {width::SCALAR, width::TWO, width::THREE, width::FOUR,
  width::EIGHT, width::SIXTEEN};
  for (size_t i=0 ; i<width::OCL_VERSIONS ; ++i){
    PairSW sw = pKeeper->getVersion(m_name, allWidth[i]);
    if (m_name == sw.first)
      return sw.second;
  }
  assert (isMangled() && "not a mangled name, cannot determine function width");
  //if we reached here, that means that function cannot be versioned, so our
  //best option is to apply the automatic width detection.
  FunctionDescriptor ret = demangle(m_name.c_str());
  ret.assignAutomaticWidth();
  return ret.width;
}

bool DriverVectorizerFunction::isPacketizable()const{
  //all builtin version has a width 4 version is they are packetizable
  const BuiltinKeeper* pKeeper = BuiltinKeeper::instance();
  if (!pKeeper->isBuiltin(m_name))
    return false;
  PairSW version4 = pKeeper->getVersion(m_name, width::FOUR);
  return !isNullPair(version4);
}

bool DriverVectorizerFunction::isScalarizable()const{
  const BuiltinKeeper* pKeeper = reflection::BuiltinKeeper::instance();
  if (!pKeeper->isBuiltin(m_name))
    return false;
  PairSW sw = pKeeper->getVersion(m_name, width::SCALAR);
  return !isNullPair(sw);
}

std::string DriverVectorizerFunction::getVersion(unsigned index)const{
  //we need to comply with the 'wiered' indexing system of the interface
  const BuiltinKeeper* pKeeper = BuiltinKeeper::instance();
  if (!pKeeper->isBuiltin(m_name))
    return FunctionDescriptor::nullString();
  width::V w;
  switch(index){
    case 0U: w = width::SCALAR; break;
    case 1U: w = width::TWO; break;
    case 2U: w = width::FOUR; break;
    case 3U: w = width::EIGHT; break;
    case 4U: w = width::SIXTEEN; break;
    case 5U: w = width::THREE; break;
    default:
      assert(false && "invalid index");
      return FunctionDescriptor::nullString();
  }
  PairSW sw = pKeeper->getVersion(m_name, w);
  assert(sw.second == w && "requested width doesn't match");
  return sw.first;
}

bool DriverVectorizerFunction::isNull()const{
  const BuiltinKeeper* pKeeper = BuiltinKeeper::instance();
  return !pKeeper->isBuiltin(m_name);
}

bool DriverVectorizerFunction::isMangled()const{
  const std::string prefix = "_Z";
  return ( prefix == m_name.substr(0, prefix.size()) );
}

} // namespace

