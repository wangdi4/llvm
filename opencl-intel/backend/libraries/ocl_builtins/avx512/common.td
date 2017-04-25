// vim:ts=2:sw=2:et:

// sign
OclBuiltinImpl sign_v16f32 = OclBuiltinImpl<sign,[v16f32] , 0, [{
    // According to the specification this built-in should return
    // 1.0 if x > 0, -1.0 if x < 0,
    // -0.0 if x = -0.0, +0.0 if x = +0.0, or 0.0 if x is a NaN.

    $ReturnType $ReturnVarName = $Arg0VarName;
    // Replace NaNs with 0 (check NaN != NaN)
    __mmask16 nanMask = _mm512_cmp_ps_mask($ReturnVarName, $ReturnVarName, _CMP_UNORD_Q);
    $ReturnVarName = _mm512_mask_and_ps($ReturnVarName, nanMask, $ReturnVarName, _mm512_setzero_ps());

    // We shouldn't do anything with zeros
    $ReturnType cleanSign = _mm512_and_ps($ReturnVarName,
                            _mm512_set1_ps(as_float(0x7FFFFFFF)));
    __mmask16 zeroMask = _mm512_cmp_ps_mask(cleanSign, _mm512_setzero_ps(), _CMP_EQ_OS);

    // Clean non-sign part of non-zero elements and replace it with 1.0f
    $ReturnVarName = _mm512_mask_and_ps($ReturnVarName, ~zeroMask, $ReturnVarName,
                                        _mm512_set1_ps(as_float(fsign_mask)));
    $ReturnVarName = _mm512_mask_or_ps($ReturnVarName, ~zeroMask, $ReturnVarName,
                                       _mm512_set1_ps(1.0f));

    return $ReturnVarName;
  }]>;

OclBuiltinImpl sign_v8f64 = OclBuiltinImpl<sign,[v8f64] , 0, [{
    // According to the specification this built-in should return
    // 1.0 if x > 0, -1.0 if x < 0,
    // -0.0 if x = -0.0, +0.0 if x = +0.0, or 0.0 if x is a NaN.

    $ReturnType $ReturnVarName = $Arg0VarName;
    // Replace NaNs with 0 (check NaN != NaN)
    __mmask8 nanMask = _mm512_cmp_pd_mask($ReturnVarName, $ReturnVarName, _CMP_UNORD_Q);
    $ReturnVarName = _mm512_mask_and_pd($ReturnVarName, nanMask, $ReturnVarName, _mm512_setzero_pd());

    // We shouldn't do anything with zeros
    $ReturnType cleanSign = _mm512_and_pd($ReturnVarName,
                            _mm512_set1_pd(as_double(0x7FFFFFFFFFFFFFFF)));
    __mmask8 zeroMask = _mm512_cmp_pd_mask(cleanSign, _mm512_setzero_pd(), _CMP_EQ_OS);

    // Clean non-sign part of non-zero elements and replace it with 1.0
    $ReturnVarName = _mm512_mask_and_pd($ReturnVarName, ~zeroMask, $ReturnVarName,
                                        _mm512_set1_pd(as_double(dsign_mask)));
    $ReturnVarName = _mm512_mask_or_pd($ReturnVarName, ~zeroMask, $ReturnVarName,
                                       _mm512_set1_pd(1.0));

    return $ReturnVarName;
  }]>;
