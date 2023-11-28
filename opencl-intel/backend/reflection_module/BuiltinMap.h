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

#ifndef __BUILTIN_MAP_H__
#define __BUILTIN_MAP_H__

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Transforms/SYCLTransforms/Utils/FunctionDescriptor.h"
#include <map>
#include <string>

namespace Reflection {

typedef llvm::SmallVector<llvm::reflection::FunctionDescriptor, 6>
    FunctionsVector;

typedef llvm::StringMap<FunctionsVector> NameToFDMultiMap;

///////////////////////////////////////////////////////////////////////////////
// Purpose:  maps a 'stripped name' (i.e., the pre-mangled name of the
// function) to the all its overloaded formats. (More concretely, to their
// function descriptors). This feature is preserved for conversion functions,
// so (example given), convert_uchar4(char4) is clustred together with
// convert_uchar8(char8), convert_uchar16(char16) etc..., even thought their
// stripped names vary.
///////////////////////////////////////////////////////////////////////////////
class BuiltinMap {
public:
  typedef FunctionsVector::const_iterator const_iterator;

  typedef std::pair<const_iterator, const_iterator> MapRange;

  //////////////////////////////////////////////////////////////////////////////
  // Purpose: returns all the builtin overloads associated to the given
  //(stripped) name.
  /////////////////////////////////////////////////////////////////////////////
  MapRange equalRange(llvm::StringRef) const;

  /////////////////////////////////////////////////////////////////////////////
  // Purpose: inserts the given function descriptor to the map
  /////////////////////////////////////////////////////////////////////////////
  void insert(const llvm::reflection::FunctionDescriptor &);

  /////////////////////////////////////////////////////////////////////////////
  // Purpose: indicates whether the two given (stripped) built-in function names
  //   should reside in the same cache line.
  // Return: true if so, false otherwise.
  /////////////////////////////////////////////////////////////////////////////
  bool isInSameCacheLine(llvm::StringRef, llvm::StringRef) const;

private:
  bool isSOAVersion(const std::string &) const;

  // a multi map which associates a stripped name to its function descriptor
  // overloads.
  NameToFDMultiMap m_nameToFd;
};

} // namespace Reflection

#endif //_BUILTIN_MAP_H__
