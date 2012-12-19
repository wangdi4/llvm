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
#include "Refcount.h"
#include "llvm/ADT/StringRef.h"

#ifndef __FDESCRIPTOR_H__
#define __FDESCRIPTOR_H__

namespace reflection {

namespace width{

enum V{
  NONE = 0,
  SCALAR = 1,
  TWO = 2,
  THREE = 3,
  FOUR = 4,
  EIGHT = 8,
  SIXTEEN = 16
};
const size_t OCL_VERSIONS = 6;
}

struct Type;

#define INVALID_ENTRY "<invalid>"

typedef std::vector<intel::RefCount<Type> > TypeVector;

struct FunctionDescriptor{
  //
  //returns: a human readable string representation of the function's
  //prototype.
  std::string toString()const;
  //The name of the function (stripped).
  std::string name;
  //Parameter list of the function
  TypeVector parameters;
  //'version width'; the width to which this function is suitable for
  width::V width;

  bool operator == (const FunctionDescriptor&)const;
  
  //enables function descriptors to serve as keys in stl maps.
  bool operator < (const FunctionDescriptor&)const;
  void assignAutomaticWidth();
  bool isNull()const;

  //create a singular value, that represents a 'null' FunctionDescriptor
  static FunctionDescriptor null();

  static llvm::StringRef nullString();
};
template <typename T>
std::ostream& operator<< (T& o, const reflection::FunctionDescriptor& fd){
  o << fd.toString();
  return o;
}
}//end reflection

#endif
