// step
OclBuiltinImpl step_v16f32 = OclBuiltinImpl<step, [v16f32] , 0, [{
    // According to the specification this built-in should return
    // 0.0 if $Arg1VarName < $Arg0VarName, else 1.0

    __mmask16 cmpMask = _mm512_cmp_ps_mask($Arg1VarName, $Arg0VarName, _CMP_GE_OS);
    return _mm512_maskz_mov_ps(cmpMask, _mm512_set1_ps(1.0f));
  }]>;

OclBuiltinImpl step_v8f64 = OclBuiltinImpl<step, [v8f64] , 0, [{
    // According to the specification this built-in should return
    // 0.0 if $Arg1VarName < $Arg0VarName, else 1.0,

    __mmask8 cmpMask = _mm512_cmp_pd_mask($Arg1VarName, $Arg0VarName, _CMP_GE_OS);
    return _mm512_maskz_mov_pd(cmpMask, _mm512_set1_pd(1.0));
  }]>;
