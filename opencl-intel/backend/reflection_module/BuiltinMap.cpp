/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include "BuiltinMap.h"
#include "llvm/ADT/StringRef.h"
#include <utility>
#include <assert.h>
#include <cctype>
#include "ParameterType.h"

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
