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

let RModes = [ defRM ] in {
// conversion to integer has satured mode.
def convert_u16         : REF_Conversions<"convert_$ReturnType",         (outs u16gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 0>;
def convert_i16         : REF_Conversions<"convert_$ReturnType",         (outs i16gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 0>;
def convert_u16_sat     : REF_Conversions<"convert_$ReturnType_sat",     (outs u16gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 1>;
def convert_i16_sat     : REF_Conversions<"convert_$ReturnType_sat",     (outs i16gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 1>;
}
// conversions from floating to integer also honor rounding mode as well as saturated mode.
let ExceptionTypes = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                       v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                      v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                      v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                      v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                      v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                      v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
                      v1u64, v2u64, v3u64, v4u64, v8u64, v16u64] in {
let RModes = [ rte, rtz, rtp, rtn] in {
def convert_u16_r     : REF_Conversions<"convert_$ReturnType$Saturate$RMode",     (outs u16gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 0>;
def convert_u16_sat_r : REF_Conversions<"convert_$ReturnType$Saturate$RMode", (outs u16gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 1>;
def convert_i16_r     : REF_Conversions<"convert_$ReturnType$Saturate$RMode",     (outs i16gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 0>;
def convert_i16_sat_r : REF_Conversions<"convert_$ReturnType$Saturate$RMode", (outs i16gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 1>;
}
}

// the following conversions make no sense but are added for completeness.
//
// conversions from int to integer with rounding mode and/or saturated mode.
let ExceptionTypes = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
                      v1f64, v2f64, v3f64, v4f64, v8f64, v16f64] in {
let RModes = [ rte, rtz, rtp, rtn] in {
def _convert_u16_r     : REF_Conversions<"convert_$ReturnType$Saturate$RMode",     (outs u16gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 0>;
def _convert_u16_sat_r : REF_Conversions<"convert_$ReturnType$Saturate$RMode", (outs u16gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 1>;
def _convert_i16_r     : REF_Conversions<"convert_$ReturnType$Saturate$RMode",     (outs i16gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 0>;
def _convert_i16_sat_r : REF_Conversions<"convert_$ReturnType$Saturate$RMode", (outs i16gentype:$ret), (ins gentype:$x), [], [], ConvTemplate, 1>;
}
}
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

File Name:  BLTConversionShort.cpp

\*****************************************************************************/

#include "BLTConversion.h"
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
    BUILTINS_API void initOCLBuiltinsConvShort() {
  }];

  code Epilog = [{
    }

} // extern "C"
  }];

}
  def inc : Generic;
