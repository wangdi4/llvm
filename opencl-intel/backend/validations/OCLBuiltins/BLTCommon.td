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

File Name:  BLTCommon.cpp

\*****************************************************************************/

#include "BLTCommon.h"

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
BUILTINS_API void initOCLBuiltinsCommon() {return;}
  }];

  code Epilog = [{
}

  }];

}
  def inc : Generic;
