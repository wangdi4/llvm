/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __BUILTIN_MAP_H__
#define __BUILTIN_MAP_H__

#include <map>
#include <string>
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/SmallVector.h"
#include "FunctionDescriptor.h"

namespace reflection{

typedef llvm::SmallVector<FunctionDescriptor, 6> FunctionsVector;

typedef llvm::StringMap<FunctionsVector> NameToFDMultiMap;

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
  typedef FunctionsVector::const_iterator const_iterator;

  typedef std::pair<const_iterator, const_iterator> MapRange;

  //////////////////////////////////////////////////////////////////////////////
  //Purpose: returns all the builtin overloads associated to the given
  //(stripped) name.
  /////////////////////////////////////////////////////////////////////////////
  MapRange equalRange(llvm::StringRef) const;

  /////////////////////////////////////////////////////////////////////////////
  //Purpose: inserts the given function descriptor to the map
  /////////////////////////////////////////////////////////////////////////////
  void insert (const FunctionDescriptor&);

  /////////////////////////////////////////////////////////////////////////////
  //Purpose: indicates whether the two given (stripped) built-in function names
  //  should reside in the same cache line.
  //Return: true if so, false otherwise.
  /////////////////////////////////////////////////////////////////////////////
  bool isInSameCacheLine(llvm::StringRef, llvm::StringRef)const;

private:

  bool isSOAVersion(const std::string&)const;

  //a multi map which associates a stripped name to its function descriptor
  //overloads.
  NameToFDMultiMap m_nameToFd;
};

}

#endif//_BUILTIN_MAP_H__
