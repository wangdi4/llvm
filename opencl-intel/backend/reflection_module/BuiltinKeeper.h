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

  File Name: BuiltinKeeper.h

\****************************************************************************/

#ifndef __BUILTIN_KEEPER_H__
#define __BUILTIN_KEEPER_H__

#include <string>
#include <utility>
#include <exception>
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "FunctionDescriptor.h"
#include "VersionStrategy.h"
#include "BuiltinMap.h"
#include "Type.h"
#include "utils.h"

namespace reflection{

class BuiltinKeeperException: public std::exception{
public:
  BuiltinKeeperException(const std::string&);
  ~BuiltinKeeperException()throw();
  const char* what()const throw();
private:
  std::string m_msg;
};

///////////////////////////////////////////////////////////////////////////////
//Purpose: A singleton class which supplies reflection services on OCL builtins
//Those services includes:
//1) The indication whether a given string represents a mangled for of an OCL
//built-in function.
//2) The ability to receive a builtin function of width w, out of a given
//function with different width.
///////////////////////////////////////////////////////////////////////////////
class BuiltinKeeper{
  //
  //Type synonyms
  //
  typedef std::map<PairSW,VersionStrategy*>     VersionCBMap;
  typedef llvm::ArrayRef<primitives::Primitive> PrimitiveArray;
  typedef llvm::ArrayRef<llvm::StringRef>       StringArray;
  typedef llvm::ArrayRef<width::V>              VWidthArray;
  //A function descriptor factory method, for builtins with two parameters
  typedef FunctionDescriptor (*FDFactory)(
  const std::pair<std::pair<llvm::StringRef,primitives::Primitive>,width::V>&,
  primitives::Primitive PTy);
public:
  static const BuiltinKeeper* instance();

  //Purpose: indicates whether the given string represent the mangled name of a
  //known built-in function.
  //Assumption: the given mangled string is a valid. (asserted)
  bool isBuiltin(const std::string& mangledString)const;

  //Purpose: indicated whether the given function represents the prototype of a
  //known built-in function.
  bool isBuiltin(const FunctionDescriptor&)const;

  //Purpose: returns the function descriptor of the built-in with the given
  //name, and the given vector with. In case there is no match (since the given
  //builtin doesn't have the requested version), the 'null descriptor' is
  //returned.
  //Parameters:
  //  name: the mangled name of the built-in function to be versioned.
  //  w: the width of the requested version
  //Assumptions: the given width is valid in OpenCL.
  //Returns: the function descriptor of the built-in in the requested width.
  //Exceptions: throws a BuiltinKeeperException when the given string isn't the
  //mangled name of a built-in function in OpenCL.
  PairSW getVersion(const std::string& mangledString, width::V w)const
#if !defined(_WIN32) && !defined(_WIN64)
  //cl compiler doesn't approve that, and issued a warning
  throw(BuiltinKeeperException)
#endif
  ;

private:
  BuiltinKeeper();
  ~BuiltinKeeper();
  BuiltinKeeper& operator= (const BuiltinKeeper&)const;
  BuiltinKeeper(const BuiltinKeeper&);

  void initNullStrategyEntries();
  void initSoaStrategyEntries();
  void initHardCodeStrategy();

  /////////////////////////////////////////////////////////////////////////////
  //Purpose: a specialization for addConversionGroup with three parameter. The
  //third parameter is induced by the type of the second one.
  /////////////////////////////////////////////////////////////////////////////
  void addConversionGroup (const StringArray& names,const PrimitiveArray& types,
  FDFactory fdFactory);

  /////////////////////////////////////////////////////////////////////////////
  //Purpose: adds a group of conversion functions to the 'exception group', for
  //which vectorization attempts will be denied. The group will be composed of
  //the cartesian product of names X types X V X s,(where V is a group of all
  //vector sizes, and s is a singleton group of one type).
  //The prototype of the function will then be: name(tv, s). Each function will
  //then be added to all non-scalar vector widths, so it won't be vectorized.
  //
  //Parameters:
  //  names: a group of stripped function names.
  //  types: a group of primitive types to be 
  //  s:     type of scalar parameter(s)
  //  fdFactory: a function pointer that produces the FunctionDescriptor, given
  //  the quartet (name, vtype, width, stype)
  /////////////////////////////////////////////////////////////////////////////
  void addConversionGroup (const StringArray& names, const PrimitiveArray& types
  , primitives::Primitive s, FDFactory fdFactory);

  /////////////////////////////////////////////////////////////////////////////
  //Purpose: add a 'line' of transposed functions to the exceptions map.
  //Parameters:
  //  aos: AOS versioned function descriptor of the functions to be transposed.
  //  transposedTargets: the widths array for which the vectorzier would like
  //  vectorize to.
  /////////////////////////////////////////////////////////////////////////////
  void addTransposGroup(const FunctionDescriptor& aos);

  /////////////////////////////////////////////////////////////////////////////
  //Purpose: populates the return-type map (which maps a function descriptor to
  //it return type
  /////////////////////////////////////////////////////////////////////////////
  void populateReturnTyMap();

  static BuiltinKeeper* Instance;
  //cache for builtins. (contains builtin function which where previously
  //queried.
  mutable BuiltinMap m_descriptorsMap;
  //
  //Versioning strategies
  //
  NullDescriptorStrategy m_nullStrategy;
  SoaDescriptorStrategy  m_soaStrategy;
  HardCodedVersionStrategy m_hardCodedStrategy;
  
  //Maps a function descriptor to its return type
  ReturnTypeMap m_fdToRetTy;

  //A map that holds 'abnormal versioning behaviours',
  //by mapping function descriptors and target width to a new
  //(non trivially expanded) function descriptor factory method.
  VersionCBMap m_exceptionsMap;
};//End BuiltinKeeper


}//end reflection

#endif//__BUILTIN_KEEPER_H__
