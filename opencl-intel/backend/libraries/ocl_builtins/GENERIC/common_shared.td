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

// clamp on floating
OclBuiltinImpl clamp_fgeneric = OclBuiltinImpl<clamp,
  ExpandTypesByVFAndFlatten<[v1f32, v1f64], [1, 2, 3, 4, 8, 16]>.Tout, 0, clamp_f_code>;

// step
OclBuiltinImpl step_f32_generic = OclBuiltinImpl<step,
  [v1f32, v2f32, v3f32, v4f32] , 0, [{
    return ($Arg1VarName < $Arg0VarName) ? ($ReturnType)0.0f : ($ReturnType)1.0f;
  }]>;
OclBuiltinImpl step_f64_generic = OclBuiltinImpl<step,
  [v1f64, v2f64, v16f64] , 0, [{
    return ($Arg1VarName < $Arg0VarName) ? ($ReturnType)0.0 : ($ReturnType)1.0;
  }]>;

// smoothstep
/* workaround an issue in LLVM/clang, where float3 is widen to double2 in
 * caller and shrunk back to float3 in callee. This stop constant folding in
 * current LLVM/clang. That still work with '*.ll' files but '*.bc' would cause
 * a crash in LLVM as the way where the constant expression is constructed.
 */
OclBuiltinImpl smoothstep_v3f32_workaround = OclBuiltinImpl<smoothstep, [v3f32], 0, Promote3>;

// sign
OclBuiltinImpl sign_f32_generic = OclBuiltinImpl<sign,
  [v1f32, v2f32, v3f32, v4f32] , 0, [{
    return isnan($Arg0VarName) ? ($ReturnType)0.0f :
           $Arg0VarName > 0.0f ? ($ReturnType)1.0f :
           $Arg0VarName < 0.0f ? ($ReturnType)-1.0f : $Arg0VarName;
  }]>;
OclBuiltinImpl sign_f64_generic = OclBuiltinImpl<sign,
  [v1f64, v2f64, v16f64] , 0, [{
    return isnan($Arg0VarName) ? ($ReturnType)0.0 :
           $Arg0VarName > 0.0 ?  ($ReturnType)1.0 :
           $Arg0VarName < 0.0 ? ($ReturnType)-1.0 : $Arg0VarName;
  }]>;
