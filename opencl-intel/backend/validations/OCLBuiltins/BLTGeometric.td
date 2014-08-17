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

Copyright (c) Intel Corporation (2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BLTGeometric.cpp

\*****************************************************************************/

#include "BLTGeometric.h"

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
BUILTINS_API void initOCLBuiltinsGeometric() {return;}
  }];

  code Epilog = [{
}

  }];

}
  def inc : Generic;
