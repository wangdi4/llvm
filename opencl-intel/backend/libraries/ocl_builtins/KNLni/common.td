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

