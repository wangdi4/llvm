// vim:ts=2:sw=2:et:

// clamp on floating
OclBuiltinImpl clamp_fgeneric = OclBuiltinImpl<clamp,
  [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
   v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return fmax(fmin($Arg0VarName, $Arg2VarName), $Arg1VarName);
  }]>;

// degrees
OclBuiltinImpl degrees_f32_generic = OclBuiltinImpl<degrees,
  [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32] , 0, [{
    return ($Arg0Type)generic_inv_pi_180f * $Arg0VarName;
  }]>;
OclBuiltinImpl degrees_f64_generic = OclBuiltinImpl<degrees,
  [v1f64, v2f64, v4f64, v8f64, v16f64] , 0, [{
    return ($Arg0Type)generic_inv_pi_180 * $Arg0VarName;
  }]>;

// max
OclBuiltinImpl max_fgeneric = OclBuiltinImpl<max,
  [v1f32, v2f32, v3f32, v4f32,
   v1f64, v2f64, v16f64], 0, [{
    $Arg0igentype mask = isless($Arg0VarName, $Arg1VarName);
    return mask ? $Arg1VarName : $Arg0VarName;
  }]>;

// min
OclBuiltinImpl min_fgeneric = OclBuiltinImpl<min,
  [v1f32, v2f32, v3f32, v4f32,
   v1f64, v2f64, v16f64], 0, [{
    $Arg0igentype mask = isgreater($Arg0VarName, $Arg1VarName);
    return mask ? $Arg1VarName : $Arg0VarName;
  }]>;

// mix
OclBuiltinImpl mix_generic = OclBuiltinImpl<mix, 
  [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
   v1f64, v2f64, v4f64, v8f64, v16f64], 0, [{
    return $Arg0VarName + ($Arg1VarName - $Arg0VarName) * $Arg2VarName;
  }]>;

// radians
OclBuiltinImpl radians_f32_generic = OclBuiltinImpl<radians,
  [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32] , 0, [{
    return ($Arg0Type)generic_pi_180f * $Arg0VarName;
  }]>;
OclBuiltinImpl radians_f64_generic = OclBuiltinImpl<radians,
  [v1f64, v2f64, v4f64, v8f64, v16f64] , 0, [{
    return ($Arg0Type)generic_pi_180 * $Arg0VarName;
  }]>;

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
OclBuiltinImpl smoothstep_f32_generic = OclBuiltinImpl<smoothstep,
  [v1f32, v2f32, /*v3f32,*/ v4f32, v8f32, v16f32] , 0, [{
    $ReturnType t = clamp(($Arg2VarName - $Arg0VarName) / ($Arg1VarName - $Arg0VarName), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
  }]>;
/* workaround an issue in LLVM/clang, where float3 is widen to double2 in
 * caller and shrunk back to float3 in callee. This stop constant folding in
 * current LLVM/clang. That still work with '*.ll' files but '*.bc' would cause
 * a crash in LLVM as the way where the constant expression is constructed.
 */
OclBuiltinImpl smoothstep_v3f32_workaround = OclBuiltinImpl<smoothstep, [v3f32], 0, [{
    float4 v4$Arg0VarName; v4$Arg0VarName$Pattern = $Arg0VarName;
    float4 v4$Arg1VarName; v4$Arg1VarName$Pattern = $Arg1VarName;
    float4 v4$Arg2VarName; v4$Arg2VarName$Pattern = $Arg2VarName;
    float4 $ReturnVarName = smoothstep(v4$Arg0VarName, v4$Arg1VarName, v4$Arg2VarName);
    return $ReturnVarName$Pattern;
  }]>;
OclBuiltinImpl smoothstep_f64_generic = OclBuiltinImpl<smoothstep,
  [v1f64, v2f64, v4f64, v8f64, v16f64] , 0, [{
    $ReturnType t = clamp(($Arg2VarName - $Arg0VarName) / ($Arg1VarName - $Arg0VarName), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
  }]>;

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
