// max
OclBuiltinImpl max_v16f32 = OclBuiltinImpl<max, [v16f32], 0, [{
    int16 mask = isless($Arg0VarName, $Arg1VarName);
    return mask ? $Arg1VarName : $Arg0VarName;
  }]>;

OclBuiltinImpl max_v8f64 = OclBuiltinImpl<max, [v8f64], 0, [{
    long8 mask = isless($Arg0VarName, $Arg1VarName);
    return mask ? $Arg1VarName : $Arg0VarName;
  }]>;

// min
OclBuiltinImpl min_v16f32 = OclBuiltinImpl<min, [v16f32], 0, [{
    int16 mask = isgreater($Arg0VarName, $Arg1VarName);
    return mask ? $Arg1VarName : $Arg0VarName;
  }]>;

OclBuiltinImpl min_v8f64 = OclBuiltinImpl<min, [v8f64], 0, [{
    long8 mask = isgreater($Arg0VarName, $Arg1VarName);
    return mask ? $Arg1VarName : $Arg0VarName;
  }]>;

// step
OclBuiltinImpl step_v16f32 = OclBuiltinImpl<step, [v16f32] , 0, [{
    // According to the specification this built-in should return
    // 0.0 if $Arg1VarName < $Arg0VarName, else 1.0

    // TODO: Using _mm512_maskz_mov_ps instead of _mm512_maskz_set1_epi32
    // may be more profitable. Check it after upgrade to llvm supported it
    __mmask16 cmpMask = _mm512_cmp_ps_mask($Arg1VarName, $Arg0VarName, _CMP_GE_OS);
    return as_$ReturnType(_mm512_maskz_set1_epi32(cmpMask, as_int(1.0f)));
  }]>;

OclBuiltinImpl step_v8f64 = OclBuiltinImpl<step, [v8f64] , 0, [{
    // According to the specification this built-in should return
    // 0.0 if $Arg1VarName < $Arg0VarName, else 1.0,

    // TODO: Using _mm512_maskz_mov_pd instead of _mm512_maskz_set1_epi64
    // may be more profitable. Check it after upgrade to llvm supported it
    __mmask8 cmpMask = _mm512_cmp_pd_mask($Arg1VarName, $Arg0VarName, _CMP_GE_OS);
    return as_$ReturnType(_mm512_maskz_set1_epi64(cmpMask, as_long(1.0)));
  }]>;
