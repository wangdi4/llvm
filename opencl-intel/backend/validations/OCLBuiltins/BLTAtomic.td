// vim:ts=2:sw=2:et:

include "ref-opencl-builtin.td"
include "BLTRef.td"

// OpenCL 1.2 rev 15, Section 6.12.11 Explicit Conversions
//
// Atomic Functions
let Types = [v1i32, v1u32],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def atomic_add_global : REF_OclBuiltin<"atomic_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atomic_add_local  : REF_OclBuiltin<"atomic_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atomic_sub_global : REF_OclBuiltin<"atomic_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atomic_sub_local  : REF_OclBuiltin<"atomic_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atomic_inc_global : REF_OclBuiltin<"atomic_inc", (outs gentype:$ret), (ins pgentype:$p), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atomic_inc_local  : REF_OclBuiltin<"atomic_inc", (outs gentype:$ret), (ins pgentype:$p), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atomic_dec_global : REF_OclBuiltin<"atomic_dec", (outs gentype:$ret), (ins pgentype:$p), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atomic_dec_local  : REF_OclBuiltin<"atomic_dec", (outs gentype:$ret), (ins pgentype:$p), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atomic_cmpxchg_global : REF_OclBuiltin<"atomic_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atomic_cmpxchg_local  : REF_OclBuiltin<"atomic_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atomic_min_global : REF_OclBuiltin<"atomic_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atomic_min_local  : REF_OclBuiltin<"atomic_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atomic_max_global : REF_OclBuiltin<"atomic_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atomic_max_local  : REF_OclBuiltin<"atomic_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atomic_and_global : REF_OclBuiltin<"atomic_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atomic_and_local  : REF_OclBuiltin<"atomic_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atomic_or_global : REF_OclBuiltin<"atomic_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atomic_or_local  : REF_OclBuiltin<"atomic_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atomic_xor_global : REF_OclBuiltin<"atomic_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atomic_xor_local  : REF_OclBuiltin<"atomic_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
}

let Types = [v1i32, v1u32, v1f32],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def atomic_xchg_global : REF_OclBuiltin<"atomic_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atomic_xchg_local  : REF_OclBuiltin<"atomic_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
}

let Types = [v1i32, v1u32],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def atom_add_vol_global     : REF_OclBuiltin<"atom_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atom_add_global         : REF_OclBuiltin<"atom_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, GLOBAL;
def atom_sub_vol_global     : REF_OclBuiltin<"atom_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atom_sub_global         : REF_OclBuiltin<"atom_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, GLOBAL;
def atom_inc_vol_global     : REF_OclBuiltin<"atom_inc", (outs gentype:$ret), (ins pgentype:$p), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atom_inc_global         : REF_OclBuiltin<"atom_inc", (outs gentype:$ret), (ins pgentype:$p), [], [], TemplateAttr< [type0]> >, GLOBAL;
def atom_dec_vol_global     : REF_OclBuiltin<"atom_dec", (outs gentype:$ret), (ins pgentype:$p), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atom_dec_global         : REF_OclBuiltin<"atom_dec", (outs gentype:$ret), (ins pgentype:$p), [], [], TemplateAttr< [type0]> >, GLOBAL;
def atom_cmpxchg_vol_global : REF_OclBuiltin<"atom_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atom_cmpxchg_global     : REF_OclBuiltin<"atom_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], [], TemplateAttr< [type0]> >, GLOBAL;
def atom_min_vol_global     : REF_OclBuiltin<"atom_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atom_min_global         : REF_OclBuiltin<"atom_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, GLOBAL;
def atom_max_vol_global     : REF_OclBuiltin<"atom_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atom_max_global         : REF_OclBuiltin<"atom_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, GLOBAL;
def atom_and_vol_global     : REF_OclBuiltin<"atom_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atom_and_global         : REF_OclBuiltin<"atom_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, GLOBAL;
def atom_or_vol_global      : REF_OclBuiltin<"atom_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atom_or_global          : REF_OclBuiltin<"atom_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, GLOBAL;
def atom_xor_vol_global     : REF_OclBuiltin<"atom_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atom_xor_global         : REF_OclBuiltin<"atom_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, GLOBAL;

def atom_add_vol_local     : REF_OclBuiltin<"atom_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atom_add_local         : REF_OclBuiltin<"atom_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, LOCAL;
def atom_sub_vol_local     : REF_OclBuiltin<"atom_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atom_sub_local         : REF_OclBuiltin<"atom_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, LOCAL;
def atom_cmpxchg_vol_local : REF_OclBuiltin<"atom_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atom_cmpxchg_local     : REF_OclBuiltin<"atom_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], [], TemplateAttr< [type0]> >, LOCAL;
def atom_inc_vol_local     : REF_OclBuiltin<"atom_inc", (outs gentype:$ret), (ins pgentype:$p), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atom_inc_local         : REF_OclBuiltin<"atom_inc", (outs gentype:$ret), (ins pgentype:$p), [], [], TemplateAttr< [type0]> >, LOCAL;
def atom_dec_vol_local     : REF_OclBuiltin<"atom_dec", (outs gentype:$ret), (ins pgentype:$p), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atom_dec_local         : REF_OclBuiltin<"atom_dec", (outs gentype:$ret), (ins pgentype:$p), [], [], TemplateAttr< [type0]> >, LOCAL;
def atom_min_vol_local     : REF_OclBuiltin<"atom_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atom_min_local         : REF_OclBuiltin<"atom_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, LOCAL;
def atom_max_vol_local     : REF_OclBuiltin<"atom_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atom_max_local         : REF_OclBuiltin<"atom_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, LOCAL;
def atom_and_vol_local     : REF_OclBuiltin<"atom_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atom_and_local         : REF_OclBuiltin<"atom_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, LOCAL;
def atom_or_vol_local      : REF_OclBuiltin<"atom_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atom_or_local          : REF_OclBuiltin<"atom_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, LOCAL;
def atom_xor_vol_local     : REF_OclBuiltin<"atom_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atom_xor_local         : REF_OclBuiltin<"atom_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, LOCAL;
}

let Types = [v1i32, v1u32, v1f32],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def atom_xchg_vol_global    : REF_OclBuiltin<"atom_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_GLOBAL;
def atom_xchg_global        : REF_OclBuiltin<"atom_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, GLOBAL;
def atom_xchg_vol_local    : REF_OclBuiltin<"atom_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, VOLATILE_LOCAL;
def atom_xchg_local        : REF_OclBuiltin<"atom_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], TemplateAttr< [type0]> >, LOCAL;
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

File Name:  BLTAtomic.cpp

\*****************************************************************************/

#include "BLTAtomic.h"
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
    template<>
    llvm::GenericValue lle_X_atomic_xchg<float>(llvm::FunctionType *FT,
        llvm::ArrayRef<llvm::GenericValue> Args)
    {
        llvm::GenericValue R;
        llvm::GenericValue arg0 = Args[0];
        llvm::GenericValue arg1 = Args[1];
        float* p = static_cast<float*>(arg0.PointerVal);

        float old = *p;
        *p = getVal<float>(arg1);

        getRef<float>(R) = old;

        return R;
    }
    }
    }
extern "C" {
  }];

  code EpilogInit = [{
    BUILTINS_API void initOCLBuiltinsAtomic() {
  }];

  code Epilog = [{
    }

} // extern "C"
  }];

}
  def inc : Generic;
