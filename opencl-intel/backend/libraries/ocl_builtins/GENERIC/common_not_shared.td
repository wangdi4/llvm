// Copyright (C) 2014 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

// vim:ts=2:sw=2:et:

// degrees

OclBuiltinImpl degrees_f64_generic_not_shared = OclBuiltinImpl<degrees,
  [v3f64] , 0, degrees_f64_code>;

// radians

OclBuiltinImpl radians_f64_generic_not_shared = OclBuiltinImpl<radians,
  [v3f64] , 0, radians_f64_code>;

// step
OclBuiltinImpl step_f32_generic_not_shared = OclBuiltinImpl<step,
  [v8f32, v16f32] , 0, [{
    return ($Arg1VarName < $Arg0VarName) ? ($ReturnType)0.0f : ($ReturnType)1.0f;
  }]>;
OclBuiltinImpl step_f64_generic_not_shared = OclBuiltinImpl<step,
  [v3f64, v4f64, v8f64] , 0, [{
    return ($Arg1VarName < $Arg0VarName) ? ($ReturnType)0.0 : ($ReturnType)1.0;
  }]>;

// smoothstep
OclBuiltinImpl smoothstep_f64_generic_not_shared = OclBuiltinImpl<smoothstep,
  [v3f64] , 0, smoothstep_f64_code>;

// sign
OclBuiltinImpl sign_f32_generic_not_shared = OclBuiltinImpl<sign,
  [v8f32, v16f32] , 0, [{
    return isnan($Arg0VarName) ? ($ReturnType)0.0f :
           $Arg0VarName > 0.0f ? ($ReturnType)1.0f :
           $Arg0VarName < 0.0f ? ($ReturnType)-1.0f : $Arg0VarName;
  }]>;
OclBuiltinImpl sign_f64_generic_not_shared = OclBuiltinImpl<sign,
  [v3f64, v4f64, v8f64] , 0, [{
    return isnan($Arg0VarName) ? ($ReturnType)0.0 :
           $Arg0VarName > 0.0 ?  ($ReturnType)1.0 :
           $Arg0VarName < 0.0 ? ($ReturnType)-1.0 : $Arg0VarName;
  }]>;
