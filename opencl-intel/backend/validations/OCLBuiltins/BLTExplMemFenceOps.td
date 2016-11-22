// vim:ts=2:sw=2:et:

include "ref-opencl-builtin.td"
include "BLTRef.td"

// OpenCL 1.2, Table 6.12.9
// Explicit Memory Fence Functions
let Types = [v1i64],
    Attrs = [OVERLOADABLE] in {
  def mem_fence       : REF_OclBuiltin<"mem_fence",      (outs), (ins cl_mem_fence_flags:$flags), [], [], TemplateAttr< [] > >;
  def read_mem_fence  : REF_OclBuiltin<"read_mem_fence", (outs), (ins cl_mem_fence_flags:$flags), [], [], TemplateAttr< [] > >;
  def write_mem_fence : REF_OclBuiltin<"write_mem_fence",(outs), (ins cl_mem_fence_flags:$flags), [], [], TemplateAttr< [] > >;
}

class Generic {
  code Prolog = [{
/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BLTExplMemFenceOps.cpp

\*****************************************************************************/

#include "BLTExplMemFenceOps.h"

using namespace llvm;
using std::string;
using std::vector;
using namespace Validation::OCLBuiltins;

#ifndef BUILTINS_API
   #if defined(_WIN32)
      #define BUILTINS_API __declspec(dllexport)
   #else
      #define BUILTINS_API
   #endif
#endif


namespace Validation {
namespace OCLBuiltins {

GenericValue lle_X_mem_fence(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  GenericValue gv;
  return gv;
}

GenericValue lle_X_read_mem_fence(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  GenericValue gv;
  return gv;
}

GenericValue lle_X_write_mem_fence(FunctionType *FT,
                         llvm::ArrayRef<llvm::GenericValue> Args) {
  GenericValue gv;
  return gv;
}


} // namespace OCLBuiltins
} // namespace Validation

extern "C" {
BUILTINS_API void initOCLBuiltinsExplMemFenceOps() {return;}
  }];

  code Epilog = [{
}

  }];

}
  def inc : Generic;
