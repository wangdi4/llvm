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

  File Name: BuiltinKeeper.cpp
\****************************************************************************/
#include "BuiltinMap.h"
#include "llvm/ADT/StringRef.h"
#include <utility>
#include <assert.h>
#include <cctype>

namespace reflection{
//
//BuiltinMap
//
BuiltinMap::MapRange BuiltinMap::equalRange (const std::string& s)const{
  if (!isConversionFunction(s))
    return m_nameToFd.equal_range(s);
  std::string coreName = getConversionCoreName(s);
  return m_nameToFd.equal_range(coreName);
}

void BuiltinMap::insert (const FunctionDescriptor& fd){
  const std::string fdName = fd.name;
  if (isConversionFunction(fdName))
    m_nameToFd.insert(std::make_pair(getConversionCoreName(fdName), fd));
  else
    m_nameToFd.insert(std::make_pair(fdName, fd));
}

bool BuiltinMap::isInSameCacheLine(
  const std::string& strLeft,
  const std::string& strRight)const{
  if (strLeft == strRight)
    return true;
  if (isConversionFunction(strLeft) && isConversionFunction(strRight))
    return getConversionCoreName(strLeft) == getConversionCoreName(strRight);
  return false;
}

std::string BuiltinMap::getConversionCoreName(std::string s)const{
  assert(isConversionFunction(s) && "not a conversion function");
  //conversion functions has the following form:
  //convert_<type><type_width>?(_<suffix_name>)?
  size_t pos = s.find('_');
  assert(pos != std::string::npos && "failed to find 1st underscore");
  while (pos<s.length() && !isdigit(s[pos]))
    ++pos;
  //deleting the with-digit, so conversion functions of all widths, will be in
  //the same cache-line
  while (pos<s.length() && isdigit(s[pos]))
    s.erase(pos, 1);
  return s;
}

static bool startsWith(const llvm::StringRef s, const llvm::StringRef& prefix){
  return (s.substr(0, prefix.size()) == prefix);
}

bool BuiltinMap::isConversionFunction(const std::string& s) const{
  return startsWith(s, "convert_");
}

bool BuiltinMap::isSOAVersion(const std::string& s)const{
  return startsWith(s, "soa_");
}


}
