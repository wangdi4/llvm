// vim:ts=2:sw=2:et:

include "ref-opencl-builtin.td"
include "BLTRef.td"

// OpenCL 1.2 rev 15, Table 6.12
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def degrees       : REF_OclBuiltin<"degrees",     (outs gentype:$ret), (ins gentype:$radians), [], [], MathTemplate>;
def mix           : REF_OclBuiltin<"mix",         (outs gentype:$ret), (ins gentype:$x, gentype:$y, gentype:$a), [], [], TemplateAttr< [type0, vec0, vec2]> >;
def mix_s         : REF_OclBuiltin<"mix",         (outs gentype:$ret), (ins gentype:$x, gentype:$y, sgentype:$a), [], [], TemplateAttr< [type0, vec0, vec2]> >, Except<[v1f32, v1f64]>;
def min           : REF_OclBuiltin<"min",         (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], TemplateAttr< [type0, vec0, vec1]> >;
def min_s         : REF_OclBuiltin<"min",         (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], [], TemplateAttr< [type0, vec0, vec1]> >, Except<[v1f32, v1f64]>;
def max           : REF_OclBuiltin<"max",         (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], TemplateAttr< [type0, vec0, vec1]> >;
def max_s         : REF_OclBuiltin<"max",         (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], [], TemplateAttr< [type0, vec0, vec1]> >, Except<[v1f32, v1f64]>;
def clamp         : REF_OclBuiltin<"clamp",       (outs gentype:$ret), (ins gentype:$x, gentype:$y, gentype:$a), [], [], TemplateAttr< [type0, vec0, vec1]> >;
def clamp_s       : REF_OclBuiltin<"clamp",       (outs gentype:$ret), (ins gentype:$x, sgentype:$y, sgentype:$a), [], [], TemplateAttr< [type0, vec0, vec1]> >, Except<[v1f32, v1f64]>;
def radians       : REF_OclBuiltin<"radians",     (outs gentype:$ret), (ins gentype:$degrees), [], [], MathTemplate>;
def step          : REF_OclBuiltin<"step",        (outs gentype:$ret), (ins gentype:$edge, gentype:$x), [], [], TemplateAttr< [type0, vec0, vec1]> >;
def step_s        : REF_OclBuiltin<"step",        (outs gentype:$ret), (ins sgentype:$edge, gentype:$x), [], [], TemplateAttr< [type0, vec0, vec1]> >, Except<[v1f32, v1f64]>;
def smoothstep    : REF_OclBuiltin<"smoothstep",  (outs gentype:$ret), (ins gentype:$edge0, gentype:$edge1, gentype:$x), [], [], TemplateAttr< [type0, vec0, vec2]> >;
def smoothstep_s  : REF_OclBuiltin<"smoothstep",  (outs gentype:$ret), (ins sgentype:$edge0, sgentype:$edge1, gentype:$x), [], [], TemplateAttr< [type0, vec0, vec2]> >, Except<[v1f32, v1f64]>;
def sign          : REF_OclBuiltin<"sign",        (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
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

File Name:  BLTCommon.cpp

\*****************************************************************************/

#include "BLTCommon.h"
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
    BUILTINS_API void initOCLBuiltinsCommon() {
  }];

  code Epilog = [{
    }

} // extern "C"
  }];

}
  def inc : Generic;
