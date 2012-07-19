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

File Name: FunctionDescriptor.cpp

\****************************************************************************/

#include "FunctionDescriptor.h"
#include "Type.h"
#include <sstream>

namespace reflection{

std::string FunctionDescriptor::toString()const{
  std::stringstream stream;
  stream << "name: " << name << "\n";
  stream << "parameters:\n";
  std::vector<Type*>::const_iterator it = parameters.begin(),
  e = parameters.end();
  while (it != e)
    stream << "\t" << (*it++)->toString() << "\n";
  return stream.str();
}

}
