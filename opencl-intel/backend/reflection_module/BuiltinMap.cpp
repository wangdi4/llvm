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

static bool startsWith(const llvm::StringRef s, const llvm::StringRef& prefix){
  return (s.substr(0, prefix.size()) == prefix);
}

/////////////////////////////////////////////////////////////////////////////
//Purpose: indicates whether a given name is the name of a conversion
//function.
/////////////////////////////////////////////////////////////////////////////
bool isConversionFunction(llvm::StringRef s){
  return s.startswith("convert_");
}

/////////////////////////////////////////////////////////////////////////////
//Purpose: get the 'core name' for the given function name; that is the name
//'representing' all the overloaded function of a given conversion.
//for example the core name of the functions convert_char, convert_char2, and
//convert_char3 is convert_char.
/////////////////////////////////////////////////////////////////////////////
static std::string getConversionCoreName(const std::string& s){
  assert(isConversionFunction(s) && "not a conversion function");
  //conversion functions has the following form:
  //convert_<type><type_width>?(_<suffix_name>)?
  std::string ret;
  size_t pos = s.find('_');
  assert(pos != std::string::npos && "failed to find 1st underscore");
  while (pos<s.length() && !isdigit(s[pos]))
    ++pos;
  ret = s.substr(0, pos);
  //deleting the with-digit, so conversion functions of all widths, will be in
  //the same cache-line
  while (pos<s.length() && isdigit(s[pos]))
    ++pos;
  size_t slen = s.length();
  if (slen > pos)
    ret.append(s.substr(pos, slen-pos));
  return ret;
}

static BuiltinMap::MapRange itToRange(
  NameToFDMultiMap::const_iterator it,
  NameToFDMultiMap::const_iterator end){
  if (end != it){
    const FunctionsVector& fv = it->getValue();
    return std::make_pair(fv.begin(), fv.end());
  } else {
    FunctionsVector empty;
    return std::make_pair(empty.begin(), empty.end());
  }
}

//
//BuiltinMap
//
BuiltinMap::MapRange BuiltinMap::equalRange (llvm::StringRef s)const{
  llvm::StringRef name;
  std::string strConversion;
  if (!isConversionFunction(s))
    name = s;
  else {
    strConversion = getConversionCoreName(s);
    name = strConversion;
  }
  NameToFDMultiMap::const_iterator it = m_nameToFd.find(name);
  return itToRange(it, m_nameToFd.end());
}

void BuiltinMap::insert (const FunctionDescriptor& fd){
  if (isConversionFunction(fd.name)){
    std::string name = getConversionCoreName(fd.name);
    m_nameToFd[name].append(1U, fd);
  } else
    m_nameToFd[fd.name].append(1U, fd);
}

bool
BuiltinMap::isInSameCacheLine(llvm::StringRef strLeft, llvm::StringRef strRight)const{
  if (strLeft == strRight)
    return true;
  
  if (isConversionFunction(strLeft) && isConversionFunction(strRight)){
    std::string strl = getConversionCoreName(strLeft);
    std::string strr = getConversionCoreName(strRight);
    return  strl == strr;
  }
  return false;
}

bool BuiltinMap::isSOAVersion(const std::string& s)const{
  return startsWith(s, "soa_");
}

}
