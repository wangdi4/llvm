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

  File Name: NameMangleAPI.h

  \****************************************************************************/

#include "FunctionDescriptor.h"
#include <string>

//
//Purpose: converts the given string to function descriptor, that represents
//the function's prototype.
//In case of failures, an exception is thrown.
reflection::FunctionDescriptor demangle(const char* rawstring);


//
//Purpose: converts the given function descriptor to string that represents
//the function's prototype.
//The mangling algorithm is based on Itanium mangling algorithm
//(http://sourcery.mentor.com/public/cxx-abi/abi.html#mangling), with SPIR
//extensions.
std::string mangle(const reflection::FunctionDescriptor&);

