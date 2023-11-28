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

#ifndef __BUILTIN_KEEPER_H__
#define __BUILTIN_KEEPER_H__

#include "BuiltinMap.h"
#include "VersionStrategy.h"
#include "utils.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Transforms/SYCLTransforms/Utils/FunctionDescriptor.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ParameterType.h"
#include <exception>
#include <string>
#include <utility>

namespace Reflection {

///////////////////////////////////////////////////////////////////////////////
// Purpose: A singleton class which supplies reflection services on OCL builtins
// Those services includes:
// 1) The indication whether a given string represents a mangled for of an OCL
// built-in function.
// 2) The ability to receive a builtin function of width w, out of a given
// function with different width.
///////////////////////////////////////////////////////////////////////////////
class BuiltinKeeper {
  //
  // Type synonyms
  //
  typedef std::map<PairSW, VersionStrategy *> VersionCBMap;
  typedef llvm::ArrayRef<llvm::reflection::TypePrimitiveEnum> PrimitiveArray;
  typedef llvm::ArrayRef<llvm::StringRef> StringArray;
  typedef llvm::ArrayRef<llvm::reflection::width::V> VWidthArray;
  // A function descriptor factory method, for builtins with two parameters
  typedef llvm::reflection::FunctionDescriptor (*FDFactory)(
      const std::pair<
          std::pair<llvm::StringRef, llvm::reflection::TypePrimitiveEnum>,
          llvm::reflection::width::V> &,
      llvm::reflection::TypePrimitiveEnum PTy);

public:
  BuiltinKeeper(const BuiltinKeeper &) = delete;
  BuiltinKeeper &operator=(const BuiltinKeeper &) = delete;

  static const BuiltinKeeper *instance();

  // Purpose: indicates whether the given string represent the mangled name of a
  // known built-in function.
  // Assumption: the given mangled string is a valid. (asserted)
  bool isBuiltin(const std::string &mangledString) const;

  // Purpose: indicated whether the given function represents the prototype of a
  // known built-in function.
  bool isBuiltin(const llvm::reflection::FunctionDescriptor &) const;

  // Purpose: returns the function descriptor of the built-in with the given
  // name, and the given vector with. And returns the 'null descriptor', in case
  // the given name is not a mangled name for OpenCL built-in or there is no
  // match for the given built-in (since the given builtin doesn't have the
  // requested version). Parameters:
  //   name: the mangled name of the built-in function to be versioned.
  //   w: the width of the requested version
  // Assumptions: the given width is valid in OpenCL.
  // Returns: the function descriptor of the built-in in the requested width.
  //          And a 'null descriptor' when the given string isn't an OpenCL
  //          built-in function or if there is no match.
  PairSW getVersion(const std::string &mangledString,
                    llvm::reflection::width::V w) const;

private:
  BuiltinKeeper();
  ~BuiltinKeeper() {}

  void initNullStrategyEntries();
  void initSoaStrategyEntries();

  /////////////////////////////////////////////////////////////////////////////
  // Purpose: Adds the behavior expected from WI functions. (i.e., NULL strategy
  //  for all non-scalar widths, and identity strategy for scalar width.
  // Parameters:
  //   names: Builtins names.
  //   ty:    The type of the parameter of the builtins.
  /////////////////////////////////////////////////////////////////////////////
  void addExceptionToWIFunctions(const StringArray &names,
                                 llvm::reflection::TypePrimitiveEnum ty);

  /////////////////////////////////////////////////////////////////////////////
  // Purpose: a specialization for addConversionGroup with three parameter. The
  // third parameter is induced by the type of the second one.
  /////////////////////////////////////////////////////////////////////////////
  void addConversionGroup(const StringArray &names, const PrimitiveArray &types,
                          FDFactory fdFactory);

  /////////////////////////////////////////////////////////////////////////////
  // Purpose: adds a group of conversion functions to the 'exception group', for
  // which vectorization attempts will be denied. The group will be composed of
  // the Cartesian product of names X types X V X s,(where V is a group of all
  // vector sizes, and s is a singleton group of one type).
  // The prototype of the function will then be: name(tv, s). Each function will
  // then be added to all non-scalar vector widths, so it won't be vectorized.
  //
  // Parameters:
  //   names: A group of stripped function names.
  //   types: A group of primitive types
  //   s:     Type of scalar parameter(s).
  //   fdFactory: A function pointer that produces the FunctionDescriptor, given
  //   the quartet (name, vtype, width, stype).
  /////////////////////////////////////////////////////////////////////////////
  void addConversionGroup(const StringArray &names, const PrimitiveArray &types,
                          llvm::reflection::TypePrimitiveEnum s,
                          FDFactory fdFactory);

  /////////////////////////////////////////////////////////////////////////////
  // Purpose: add a 'line' of transposed functions to the exceptions map.
  // Parameters:
  //   aos: AOS versioned function descriptor of the functions to be transposed.
  //   transposedTargets: the widths array for which the vectorzier would like
  //   vectorize to.
  /////////////////////////////////////////////////////////////////////////////
  void addTransposGroup(const llvm::reflection::FunctionDescriptor &aos);

  /////////////////////////////////////////////////////////////////////////////
  // Purpose: populates the return-type map (which maps a function descriptor to
  // it return type
  /////////////////////////////////////////////////////////////////////////////
  void populateReturnTyMap();

  /////////////////////////////////////////////////////////////////////////////
  // Purpose: indicates whether the given bi name resides within the exception
  // map
  /////////////////////////////////////////////////////////////////////////////
  bool isInExceptionMap(const std::string &name) const;

  /////////////////////////////////////////////////////////////////////////////
  // Purpose: Searched the given name in the built-in repository.
  // Parameters: The built-in function to be searched.
  // Return:     true if the name was found, false otherwise.
  // Remarks:    This function should only be called after the cache (i.e.,
  // m_descriptorsMap) was searched with the name of the given function desc,
  // and returned as empty.
  /////////////////////////////////////////////////////////////////////////////
  bool searchAndCacheUpdate(const llvm::reflection::FunctionDescriptor &) const;

  // Cache for builtins. (contains builtin function which where previously
  // queried.
  mutable BuiltinMap m_descriptorsMap;
  //
  // Versioning strategies
  //
  NullDescriptorStrategy m_nullStrategy;
  SoaDescriptorStrategy m_soaStrategy;
  IdentityStrategy m_indentityStrategy;

  // Maps a function descriptor to its return type
  ReturnTypeMap m_fdToRetTy;

  // A map that holds 'abnormal versioning behaviours',
  // by mapping function descriptors and target width to a new
  //(non trivially expanded) function descriptor factory method.
  VersionCBMap m_exceptionsMap;
}; // End BuiltinKeeper

} // namespace Reflection

#endif //__BUILTIN_KEEPER_H__
