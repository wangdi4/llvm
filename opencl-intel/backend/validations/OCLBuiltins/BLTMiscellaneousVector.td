// vim:ts=2:sw=2:et:

include "ref-opencl-builtin.td"
include "BLTRef.td"

// OpenCL 1.2 rev 15, Table 6.20
let Types = [ v2i8,  v4i8,  v8i8,  v16i8,
              v2u8,  v4u8,  v8u8,  v16u8,
             v2i16, v4i16, v8i16, v16i16,
             v2u16, v4u16, v8u16, v16u16,
             v2i32, v4i32, v8i32, v16i32,
             v2u32, v4u32, v8u32, v16u32,
             v2i64, v4i64, v8i64, v16i64,
             v2u64, v4u64, v8u64, v16u64,
             v2f32, v4f32, v8f32, v16f32,
             v2f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def shuffle_2   : REF_OclBuiltin<"shuffle",   (outs gentype2:$ret), (ins gentype:$x, ugentype2:$mask), [], [], TemplateAttr< [type0, type1, vec1]> >;
def shuffle_4   : REF_OclBuiltin<"shuffle",   (outs gentype4:$ret), (ins gentype:$x, ugentype4:$mask), [], [], TemplateAttr< [type0, type1, vec1]> >;
def shuffle_8   : REF_OclBuiltin<"shuffle",   (outs gentype8:$ret), (ins gentype:$x, ugentype8:$mask), [], [], TemplateAttr< [type0, type1, vec1]> >;
def shuffle_16  : REF_OclBuiltin<"shuffle",   (outs gentype16:$ret), (ins gentype:$x, ugentype16:$mask), [], [], TemplateAttr< [type0, type1, vec1]> >;
def shuffle2_2  : REF_OclBuiltin<"shuffle2",  (outs gentype2:$ret), (ins gentype:$x, gentype:$y, ugentype2:$mask), [], [], TemplateAttr< [type0, type2, vec2]> >;
def shuffle2_4  : REF_OclBuiltin<"shuffle2",  (outs gentype4:$ret), (ins gentype:$x, gentype:$y, ugentype4:$mask), [], [], TemplateAttr< [type0, type2, vec2]> >;
def shuffle2_8  : REF_OclBuiltin<"shuffle2",  (outs gentype8:$ret), (ins gentype:$x, gentype:$y, ugentype8:$mask), [], [], TemplateAttr< [type0, type2, vec2]> >;
def shuffle2_16 : REF_OclBuiltin<"shuffle2",  (outs gentype16:$ret), (ins gentype:$x, gentype:$y, ugentype16:$mask), [], [], TemplateAttr< [type0, type2, vec2]> >;
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

File Name:  BLTMiscellaneousVector.cpp

\*****************************************************************************/

#include "BLTMiscellaneousVector.h"
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
    BUILTINS_API void initOCLBuiltinsMisc() {
  }];

  code Epilog = [{
    }

} // extern "C"
  }];

}
  def inc : Generic;
