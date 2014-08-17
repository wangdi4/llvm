// vim:ts=2:sw=2:et:

include "ref-opencl-builtin.td"
include "BLTRef.td"

// OpenCL 1.2 rev 15, Section 6.2.3 Explicit Conversions
//
// destType convert_destType<_sat><_roundingMode>(sourceType)
//
let Types = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
              v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64,
             v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
// conversion to floating may not has saturated mode.
let RModes = [ rte, rtz, rtp, rtn, defRM] in {
def convert_f32         : REF_Conversions<"convert_$ReturnType$Saturate$RMode",         (outs f32gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 0>;
def convert_f64         : REF_Conversions<"convert_$ReturnType$Saturate$RMode",         (outs f64gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 0>;
// conversion to floating make nosense with saturated mode.
def _convert_f32_sat     : REF_Conversions<"convert_$ReturnType$Saturate$RMode",     (outs f32gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 1>;
def _convert_f64_sat     : REF_Conversions<"convert_$ReturnType$Saturate$RMode",     (outs f64gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 1>;
}
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

File Name:  BLTConversionFPoint.cpp

\*****************************************************************************/

#include "BLTConversion.h"

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
BUILTINS_API void initOCLBuiltinsConvFPoint() {return;}
}];

  code Epilog = [{
}

  }];

}
  def inc : Generic;
