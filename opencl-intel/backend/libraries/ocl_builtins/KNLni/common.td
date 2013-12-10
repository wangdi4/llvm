// max
OclBuiltinImpl max_v16f32 = OclBuiltinImpl<max, [v16f32], 0, [{
    return _mm512_max_ps($Arg0VarName, $Arg1VarName);
  }]>;

OclBuiltinImpl max_v8f64 = OclBuiltinImpl<max, [v8f64], 0, [{
    return _mm512_max_pd($Arg0VarName, $Arg1VarName);
  }]>;

// min
OclBuiltinImpl min_v16f32 = OclBuiltinImpl<min, [v16f32], 0, [{
    return _mm512_min_ps($Arg0VarName, $Arg1VarName);
  }]>;

OclBuiltinImpl min_v8f64 = OclBuiltinImpl<min, [v8f64], 0, [{
    return _mm512_min_pd($Arg0VarName, $Arg1VarName);
  }]>;

// sign
OclBuiltinImpl sign_f32_generic = OclBuiltinImpl<sign,
  [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32] , 0, [{
    return isnan($Arg0VarName) ? ($ReturnType)0.0f :
           $Arg0VarName > 0.0f ? ($ReturnType)1.0f :
           $Arg0VarName < 0.0f ? ($ReturnType)-1.0f : $Arg0VarName;
  }]>;
OclBuiltinImpl sign_f64_generic = OclBuiltinImpl<sign,
  [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64] , 0, [{
    return isnan($Arg0VarName) ? ($ReturnType)0.0 :
           $Arg0VarName > 0.0 ?  ($ReturnType)1.0 :
           $Arg0VarName < 0.0 ? ($ReturnType)-1.0 : $Arg0VarName;
  }]>;
