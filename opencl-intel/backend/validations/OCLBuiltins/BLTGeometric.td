// vim:ts=2:sw=2:et:

include "ref-opencl-builtin.td"
include "BLTRef.td"

// OpenCL 1.2 rev 15, Table 6.13
let Types = [v1f32, v2f32, v3f32, v4f32,
             v1f64, v2f64, v3f64, v4f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def cross     : REF_OclBuiltin<"cross",     (outs gentype:$ret), (ins gentype:$p0, gentype:$p1), [], [], MathTemplate>,
                Except<[v1f32, v2f32, v1f64, v2f64]>;
def dot       : REF_OclBuiltin<"dot",       (outs sgentype:$ret), (ins gentype:$p0, gentype:$p1), [], [], MathTemplate>;
def distance  : REF_OclBuiltin<"distance",  (outs sgentype:$ret), (ins gentype:$p0, gentype:$p1), [], [], MathTemplate>;
def length    : REF_OclBuiltin<"length",    (outs sgentype:$ret), (ins gentype:$p), [], [], MathTemplate>;
def normalize : REF_OclBuiltin<"normalize", (outs gentype:$ret), (ins gentype:$p), [], [], MathTemplate>;
}
let Types = [v1f32, v2f32, v3f32, v4f32],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def fast_distance  : REF_OclBuiltin<"fast_distance",  (outs sgentype:$ret), (ins gentype:$p0, gentype:$p1), [], [], MathTemplate>;
def fast_length    : REF_OclBuiltin<"fast_length",    (outs sgentype:$ret), (ins gentype:$p), [], [], MathTemplate>;
def fast_normalize : REF_OclBuiltin<"fast_normalize", (outs gentype:$ret), (ins gentype:$p), [], [], MathTemplate>;
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

File Name:  BLTGeometric.cpp

\*****************************************************************************/

#include "BLTGeometric.h"
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
    BUILTINS_API void initOCLBuiltinsGeometric() {
  }];

  code Epilog = [{
    }

} // extern "C"
  }];

}
  def inc : Generic;
