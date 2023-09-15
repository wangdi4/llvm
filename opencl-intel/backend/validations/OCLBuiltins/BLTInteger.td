// vim:ts=2:sw=2:et:

include "ref-opencl-builtin.td"
include "BLTRef.td"

// OpenCL 1.2 rev 15, Table 6.10
let Types = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
              v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def abs       : REF_OclBuiltin<"abs",       (outs ugentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def abs_diff  : REF_OclBuiltin<"abs_diff",  (outs ugentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def add_sat   : REF_OclBuiltin<"add_sat",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def hadd      : REF_OclBuiltin<"hadd",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def rhadd     : REF_OclBuiltin<"rhadd",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def clz       : REF_OclBuiltin<"clz",       (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def ctz       : REF_OclBuiltin<"ctz",       (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def mad_hi    : REF_OclBuiltin<"mad_hi",    (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], [], MathTemplate>;
def mad_sat   : REF_OclBuiltin<"mad_sat",   (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], [], MathTemplate>;
def mul_hi    : REF_OclBuiltin<"mul_hi",    (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def rotate    : REF_OclBuiltin<"rotate",    (outs gentype:$ret), (ins gentype:$v, gentype:$i), [], [], MathTemplate>;
def sub_sat   : REF_OclBuiltin<"sub_sat",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def upsample  : REF_OclBuiltin<"upsample",  (outs up_gentype:$ret), (ins gentype:$hi, ugentype:$lo), [], [], MathTemplate>,
                Except<[v1i64, v2i64, v3i64, v4i64, v8i64, v16i64, v1u64, v2u64, v3u64, v4u64, v8u64, v16u64]>;
let NeedForwardDecl = 1 in /* work around the missing prototype in opencl.h */
def popcount  : REF_OclBuiltin<"popcount",  (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
}
let Types = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
              v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def clamp   : REF_OclBuiltin<"clampi", (outs gentype:$ret), (ins gentype:$x, gentype:$minval, gentype:$maxval), [], [], TemplateAttr< [type0, vec0, vec1]> >;
def clamp_s : REF_OclBuiltin<"clampi", (outs gentype:$ret), (ins gentype:$x, sgentype:$minval, sgentype:$maxval), [], [], TemplateAttr< [type0, vec0, vec1]> >,
              Except<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64]>;
def max     : REF_OclBuiltin<"maxi",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], TemplateAttr< [type0, vec0, vec1]> >;
def max_s   : REF_OclBuiltin<"maxi",   (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], [], TemplateAttr< [type0, vec0, vec1]> >,
              Except<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64]>;
def min     : REF_OclBuiltin<"mini",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], TemplateAttr< [type0, vec0, vec1]> >;
def min_s   : REF_OclBuiltin<"mini",   (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], [], TemplateAttr< [type0, vec0, vec1]> >,
              Except<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64]>;
}


// OpenCL 1.2 rev 15, Table 6.11
let Types = [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def mad24   : REF_OclBuiltin<"mad24", (outs gentype:$ret), (ins gentype:$x, gentype:$y, gentype:$z), [], [], MathTemplate>;
def mul24   : REF_OclBuiltin<"mul24", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
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

File Name:  BLTInteger.cpp

\*****************************************************************************/

#include "BLTInteger.h"
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

    template <> llvm::APInt ExtAPInt<int8_t>(llvm::APInt& a, unsigned width){return a.sext(width);}
    template <> llvm::APInt ExtAPInt<int16_t>(llvm::APInt& a, unsigned width){return a.sext(width);}
    template <> llvm::APInt ExtAPInt<int32_t>(llvm::APInt& a, unsigned width){return a.sext(width);}
    template <> llvm::APInt ExtAPInt<int64_t>(llvm::APInt& a, unsigned width){return a.sext(width);}
}
}

extern "C" {
  }];

  code EpilogInit = [{
    BUILTINS_API void initOCLBuiltinsInteger() {
  }];

  code Epilog = [{
    }

} // extern "C"
  }];

}
  def inc : Generic;
