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

Copyright (C) 2013 Intel Corporation

This software and the related documents are Intel copyrighted materials, and
your use of them is governed by the express license under which they were
provided to you ("License"). Unless the License provides otherwise, you may
not use, modify, copy, publish, distribute, disclose or transmit this
software or the related documents without Intel's prior written permission.

This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

File Name:  BLTExplMemFenceOps.cpp

\*****************************************************************************/

#include "BLTExplMemFenceOps.h"
#include "llvm/Support/DynamicLibrary.h"

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
  }];

  code EpilogInit = [{
    BUILTINS_API void initOCLBuiltinsExplMemFenceOps() {
  }];

  code Epilog = [{
    }

} // extern "C"
  }];

}
  def inc : Generic;
