// isnan
OclBuiltinImpl isnan_v16f32 = OclBuiltinImpl<isnan, [v16f32], 0, [{
    return as_$ReturnType(_mm512_cmp_ps($Arg0VarName, $Arg0VarName, _CMP_UNORD_Q));
  }]>;

OclBuiltinImpl isnan_v8f64 = OclBuiltinImpl<isnan, [v8f64], 0, [{
    return as_$ReturnType(_mm512_cmp_pd($Arg0VarName, $Arg0VarName, _CMP_UNORD_Q));
  }]>;


