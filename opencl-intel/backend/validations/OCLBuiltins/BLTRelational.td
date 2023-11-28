// vim:ts=2:sw=2:et:

include "ref-opencl-builtin.td"
include "BLTRef.td"

// OpenCL 1.2 rev 15, Table 6.14
let Types = [v1f32, v1f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
//def isequal         : REF_OclBuiltin<"isequal",         (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], TypeTemplate>;
//def isnotequal      : REF_OclBuiltin<"isnotequal",      (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], TypeTemplate>;
//def isgreater       : REF_OclBuiltin<"isgreater",       (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], TypeTemplate>;
//def isgreaterequal  : REF_OclBuiltin<"isgreaterequal",  (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], TypeTemplate>;
//def isless          : REF_OclBuiltin<"isless",          (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], TypeTemplate>;
//def islessequal     : REF_OclBuiltin<"islessequal",     (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], TypeTemplate>;
def islessgreater   : REF_OclBuiltin<"islessgreater",   (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], TypeTemplate>;
def isfinite        : REF_OclBuiltin<"isfinite",        (outs rf2igentype:$ret), (ins gentype:$x), [], [], TypeTemplate>;
def isinf           : REF_OclBuiltin<"isinf",           (outs rf2igentype:$ret), (ins gentype:$x), [], [], TypeTemplate>;
def isnan           : REF_OclBuiltin<"isnan",           (outs rf2igentype:$ret), (ins gentype:$x), [], [], TypeTemplate>;
def isnormal        : REF_OclBuiltin<"isnormal",        (outs rf2igentype:$ret), (ins gentype:$x), [], [], TypeTemplate>;
def isordered       : REF_OclBuiltin<"isordered",       (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], TypeTemplate>;
def isunordered     : REF_OclBuiltin<"isunordered",     (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], TypeTemplate>;
def signbit         : REF_OclBuiltin<"signbit",         (outs rf2igentype:$ret), (ins gentype:$x), [], [], TypeTemplate>;
}

let Types = [v2f32, v3f32, v4f32, v8f32, v16f32,
             v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
//def isequal_v         : REF_OclBuiltin<"isequal",         (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
//def isnotequal_v      : REF_OclBuiltin<"isnotequal",      (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
//def isgreater_v       : REF_OclBuiltin<"isgreater",       (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
//def isgreaterequal_v  : REF_OclBuiltin<"isgreaterequal",  (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
//def isless_v          : REF_OclBuiltin<"isless",          (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
//def islessequal_v     : REF_OclBuiltin<"islessequal",     (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def islessgreater_v   : REF_OclBuiltin<"islessgreater",   (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def isfinite_v        : REF_OclBuiltin<"isfinite",        (outs rf2igentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def isinf_v           : REF_OclBuiltin<"isinf",           (outs rf2igentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def isnan_v           : REF_OclBuiltin<"isnan",           (outs rf2igentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def isnormal_v        : REF_OclBuiltin<"isnormal",        (outs rf2igentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def isordered_v       : REF_OclBuiltin<"isordered",       (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def isunordered_v     : REF_OclBuiltin<"isunordered",     (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def signbit_v         : REF_OclBuiltin<"signbit",         (outs rf2igentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
}

let Types = [v1i8, v1i16, v1i32, v1i64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def any : REF_OclBuiltin<"any", (outs si32gentype:$ret), (ins gentype:$x), [], [], TypeTemplate>;
def all : REF_OclBuiltin<"all", (outs si32gentype:$ret), (ins gentype:$x), [], [], TypeTemplate>;
}

let Types = [v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
             v2i16, v3i16, v4i16, v8i16, v16i16,
             v2i32, v3i32, v4i32, v8i32, v16i32,
             v2i64, v3i64, v4i64, v8i64, v16i64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def any_v : REF_OclBuiltin<"any", (outs si32gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def all_v : REF_OclBuiltin<"all", (outs si32gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
}

let Types = [ v1i8, v1u8, v1i16, v1u16, v1i32, v1u32,
             v1i64, v1u64, v1f32, v1f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def bitselect   : REF_OclBuiltin<"bitselect", (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], [], TypeTemplate>;
def iselect     : REF_OclBuiltin<"select",    (outs gentype:$ret), (ins gentype:$a, gentype:$b, igentype:$c), [], [], TemplateAttr< [type0, type2]> >;
def uselect     : REF_OclBuiltin<"select",    (outs gentype:$ret), (ins gentype:$a, gentype:$b, ugentype:$c), [], [], TemplateAttr< [type0, type2]> >;
}

let Types = [ v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
              v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
             v2i16, v3i16, v4i16, v8i16, v16i16,
             v2u16, v3u16, v4u16, v8u16, v16u16,
             v2i32, v3i32, v4i32, v8i32, v16i32,
             v2u32, v3u32, v4u32, v8u32, v16u32,
             v2i64, v3i64, v4i64, v8i64, v16i64,
             v2u64, v3u64, v4u64, v8u64, v16u64,
             v2f32, v3f32, v4f32, v8f32, v16f32,
             v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def bitselect_v   : REF_OclBuiltin<"bitselect", (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], [], MathTemplate>;
def iselect_v     : REF_OclBuiltin<"select",    (outs gentype:$ret), (ins gentype:$a, gentype:$b, igentype:$c), [], [], TemplateAttr< [type0, type2, vec0]> >;
def uselect_v     : REF_OclBuiltin<"select",    (outs gentype:$ret), (ins gentype:$a, gentype:$b, ugentype:$c), [], [], TemplateAttr< [type0, type2, vec0]> >;
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

File Name:  BLTRelational.cpp

\*****************************************************************************/

#include "BLTRelational.h"
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

extern "C" {
  }];

  code EpilogInit = [{
    BUILTINS_API void initOCLBuiltinsRelational() {
  }];

  code Epilog = [{
    }

} // extern "C"

namespace Validation {
namespace OCLBuiltins {
template<>
    llvm::GenericValue localBitselect( float inA, float inB, float inC )
    {
        llvm::GenericValue R;
        union {uint32_t u; float f;} a, b, c, out;
        a.f = inA;
        b.f = inB;
        c.f = inC;
        out.u = ( a.u & ~c.u ) | ( b.u & c.u );
        getRef<float>(R) = out.f;
        return R;
    }
    template<>
    llvm::GenericValue localBitselect( double inA, double inB, double inC )
    {
        llvm::GenericValue R;
        union {uint64_t u; double f;} a, b, c, out;
        a.f = inA;
        b.f = inB;
        c.f = inC;
        out.u = ( a.u & c.u ) | ( b.u & c.u );
        getRef<double>(R) = out.f;
        return R;
    }

    template<>
    llvm::GenericValue selectResult( float inC )
    {
        llvm::GenericValue R;
        getRef<float>(R) = inC;
        return R;
    }
    template<>
    llvm::GenericValue selectResult( double inC )
    {
        llvm::GenericValue R;
        getRef<double>(R) = inC;
        return R;
    }

}
}
  }];

}
  def inc : Generic;
