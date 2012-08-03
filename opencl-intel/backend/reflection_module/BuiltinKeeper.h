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
#include <map>
#include <exception>
#include "FunctionDescriptor.h"

namespace llvm{ class StringRef;}

namespace reflection{

typedef std::multimap<std::string, FunctionDescriptor> BuiltinMap;

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
  //name, and the given vector with.
  //In case there is no match (either because there is no built-in with the
  //given name, or because there is no entry for the given width, an exception
  //is thrown.
  //Parameters:
  //  name: the mangled name of the built-in function to be versoined.
  //  w: the width of the requested version
  //Assumptions: the given width is valid in OpenCL.
  //Returns: the function descriptor of built-in in the requested width.
  //Exceptions: throws a BuiltinKeeperException when the requested version does
  //not exist.
  FunctionDescriptor
  getVersion(const std::string& mangledString, width::V w)const
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

  static BuiltinKeeper* Instance;
  //chache for builtins. (contains builtin function which where previously
  //queried.
  mutable std::multimap<std::string,FunctionDescriptor> m_descriptorsMap;
};//End BuiltinKeeper

}//end reflection

#endif//__BUILTIN_KEEPER_H__
