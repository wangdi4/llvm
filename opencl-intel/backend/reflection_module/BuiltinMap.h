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

  File Name: BuiltinMap.h

\****************************************************************************/

#ifndef __BUILTIN_MAP_H__
#define __BUILTIN_MAP_H__

#include <map>
#include <string>
#include "FunctionDescriptor.h"

namespace reflection{

typedef std::multimap<std::string, FunctionDescriptor> NameToFDMultiMap;

///////////////////////////////////////////////////////////////////////////////
//Purpose:  maps a 'stripped name' (i.e., the pre-mangled name of the
//function) to the all its overloaded formats. (More concretely, to their
//function descriptors). This feature is preserved for conversion functions,
//so (example given), convert_uchar4(char4) is clustred together with
//convert_uchar8(char8), convert_uchar16(char16) etc..., even thought their
//stripped names vary.
///////////////////////////////////////////////////////////////////////////////
class BuiltinMap{
public:
  typedef NameToFDMultiMap::const_iterator const_iterator;

  typedef std::pair<const_iterator, const_iterator> MapRange;

  /////////////////////////////////////////////////////////////////////////////
  //Purpose: returns all the builtin overloads associated to the given
  //(stripped) name.
  /////////////////////////////////////////////////////////////////////////////
  MapRange equalRange(const std::string&) const;

  /////////////////////////////////////////////////////////////////////////////
  //Purpose: inserts the given function descriptor to the map
  /////////////////////////////////////////////////////////////////////////////
  void insert (const FunctionDescriptor&);

  /////////////////////////////////////////////////////////////////////////////
  //Purpose: indicates whether the two given (stripped) built-in function names
  //  should reside in the same cache line.
  //Return: true if so, false otherwise.
  /////////////////////////////////////////////////////////////////////////////
  bool isInSameCacheLine(const std::string&, const std::string&)const;

private:

  /////////////////////////////////////////////////////////////////////////////
  //Purpose: get the 'core name' for the given function name, that is the name
  //'representing' all the overloaded function of a given conversion.
  //for example the core name of the functions convert_char, convert_char2, and
  //convert_char3 is convert_char.
  /////////////////////////////////////////////////////////////////////////////
  std::string getConversionCoreName(std::string)const;

  /////////////////////////////////////////////////////////////////////////////
  //Purpose: indicates whether a given name is the name of a conversion
  //function.
  /////////////////////////////////////////////////////////////////////////////
  bool isConversionFunction(const std::string&) const;

  bool isSOAVersion(const std::string&)const;

  //a multi map which associates a stripped name to its function descriptor
  //overloads.
  NameToFDMultiMap m_nameToFd;
};

}

#endif//_BUILTIN_MAP_H__
