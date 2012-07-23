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

  File Name: FunctionDescriptor.h

  \****************************************************************************/

#include <string>
#include <vector>

#ifndef __FDESCRIPTOR_H__
#define __FDESCRIPTOR_H__

namespace reflection {

struct Type;

struct FunctionDescriptor{
  //
  //returns: a human readable string representation of the function's
  //prototype.
  std::string toString()const;
  //The name of the function (stripped).
  std::string name;
  //Parameter list of the function
  std::vector<Type*> parameters;
};
}//end reflection
#endif
