// vim:ts=2:sw=2:et:


// mad
OclBuiltinImpl mad_f32_v16 = OclBuiltinImpl<mad, [v16f32], 0, [{
    return _mm512_madd231_ps($Arg2VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_mad_f32_v16 = OclBuiltinImpl<mask_mad, [v16f32], 0, [{
    return _mm512_mask_madd231_ps($Arg3VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl mad_f32_promote = OclBuiltinImpl<mad, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote3>;

OclBuiltinImpl mad_f64_v8 = OclBuiltinImpl<mad, [v8f64], 0, [{
    return _mm512_madd231_pd($Arg2VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_mad_f64_v8 = OclBuiltinImpl<mask_mad, [v8f64], 0, [{
    return _mm512_mask_madd231_pd($Arg3VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl mad_f64_promote = OclBuiltinImpl<mad, [v1f64, v2f64, v3f64, v4f64], 0, MaskPromote3>;
OclBuiltinImpl mad_f64_expand = OclBuiltinImpl<mad, [v16f64], 0, Expand3>;

// fmax
OclBuiltinImpl fmax_f32_v16 = OclBuiltinImpl<fmax, [v16f32], 0, [{
    return _mm512_max_ps($Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_fmax_f32_v16 = OclBuiltinImpl<mask_fmax, [v16f32], 0, [{
    return _mm512_mask_max_ps($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl fmax_f32_promote = OclBuiltinImpl<fmax, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote2>;

OclBuiltinImpl fmax_f64_v8 = OclBuiltinImpl<fmax, [v8f64], 0, [{
    return _mm512_max_pd($Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_fmax_f64_v8 = OclBuiltinImpl<mask_fmax, [v8f64], 0, [{
    return _mm512_mask_max_pd($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl fmax_f64_promote = OclBuiltinImpl<fmax, [v1f64, v2f64, v3f64, v4f64], 0, MaskPromote2>;
OclBuiltinImpl fmax_f64_expand = OclBuiltinImpl<fmax, [v16f64], 0, Expand2>;

// fmin
OclBuiltinImpl fmin_f32_v16 = OclBuiltinImpl<fmin, [v16f32], 0, [{
    return _mm512_min_ps($Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_fmin_f32_v16 = OclBuiltinImpl<mask_fmin, [v16f32], 0, [{
    return _mm512_mask_min_ps($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl fmin_f32_promote = OclBuiltinImpl<fmin, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote2>;

OclBuiltinImpl fmin_f64_v8 = OclBuiltinImpl<fmin, [v8f64], 0, [{
    return _mm512_min_pd($Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_fmin_f64_v8 = OclBuiltinImpl<mask_fmin, [v8f64], 0, [{
    return _mm512_mask_min_pd($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl fmin_f64_promote = OclBuiltinImpl<fmin, [v1f64, v2f64, v3f64, v4f64], 0, MaskPromote2>;
OclBuiltinImpl fmin_f64_expand = OclBuiltinImpl<fmin, [v16f64], 0, Expand2>;

// native_cos
OclBuiltinImpl native_cos_f32_v16 = OclBuiltinImpl<native_cos, [v16f32], 0, [{
    return _mm512_cos_ps($Arg0VarName);
  }]>;
OclBuiltinImpl mask_native_cos_f32_v16 = OclBuiltinImpl<mask_native_cos, [v16f32], 0, [{
    return _mm512_mask_cos_ps($Arg1VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl native_cos_f32_promote = OclBuiltinImpl<native_cos, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote1>;

// native_divide
OclBuiltinImpl native_divide_f32_v16 = OclBuiltinImpl<native_divide, [v16f32], 0, [{
    return _mm512_div_ps($Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_native_divide_f32_v16 = OclBuiltinImpl<mask_native_divide, [v16f32], 0, [{
    return _mm512_mask_div_ps($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl native_divide_f32_promote = OclBuiltinImpl<native_divide, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote2>;

// native_exp
OclBuiltinImpl native_exp_f32_v16 = OclBuiltinImpl<native_exp, [v16f32], 0, [{
    return _mm512_exp_ps($Arg0VarName);
  }]>;
OclBuiltinImpl mask_native_exp_f32_v16 = OclBuiltinImpl<mask_native_exp, [v16f32], 0, [{
    return _mm512_mask_exp_ps($Arg1VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl native_exp_f32_promote = OclBuiltinImpl<native_exp, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote1>;

// native_exp2
OclBuiltinImpl native_exp2_f32_v16 = OclBuiltinImpl<native_exp2, [v16f32], 0, [{
    return _mm512_exp2_ps($Arg0VarName);
  }]>;
OclBuiltinImpl mask_native_exp2_f32_v16 = OclBuiltinImpl<mask_native_exp2, [v16f32], 0, [{
    return _mm512_mask_exp2_ps($Arg1VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl native_exp2_f32_promote = OclBuiltinImpl<native_exp2, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote1>;

// native_exp10
OclBuiltinImpl native_exp10_f32_v16 = OclBuiltinImpl<native_exp10, [v16f32], 0, [{
    return _mm512_exp10_ps($Arg0VarName);
  }]>;
OclBuiltinImpl mask_native_exp10_f32_v16 = OclBuiltinImpl<mask_native_exp10, [v16f32], 0, [{
    return _mm512_mask_exp10_ps($Arg1VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl native_exp10_f32_promote = OclBuiltinImpl<native_exp10, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote1>;

// native_log
OclBuiltinImpl native_log_f32_v16 = OclBuiltinImpl<native_log, [v16f32], 0, [{
    return _mm512_log_ps($Arg0VarName);
  }]>;
OclBuiltinImpl mask_native_log_f32_v16 = OclBuiltinImpl<mask_native_log, [v16f32], 0, [{
    return _mm512_mask_log_ps($Arg1VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl native_log_f32_promote = OclBuiltinImpl<native_log, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote1>;

// native_log2
OclBuiltinImpl native_log2_f32_v16 = OclBuiltinImpl<native_log2, [v16f32], 0, [{
    return _mm512_log2_ps($Arg0VarName);
  }]>;
OclBuiltinImpl mask_native_log2_f32_v16 = OclBuiltinImpl<mask_native_log2, [v16f32], 0, [{
    return _mm512_mask_log2_ps($Arg1VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl native_log2_f32_promote = OclBuiltinImpl<native_log2, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote1>;

// native_log10
OclBuiltinImpl native_log10_f32_v16 = OclBuiltinImpl<native_log10, [v16f32], 0, [{
    return _mm512_log10_ps($Arg0VarName);
  }]>;
OclBuiltinImpl mask_native_log10_f32_v16 = OclBuiltinImpl<mask_native_log10, [v16f32], 0, [{
    return _mm512_mask_log10_ps($Arg1VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl native_log10_f32_promote = OclBuiltinImpl<native_log10, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote1>;

// native_powr
OclBuiltinImpl native_powr_f32_v16 = OclBuiltinImpl<native_powr, [v16f32], 0, [{
    return _mm512_pow_ps($Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_native_powr_f32_v16 = OclBuiltinImpl<mask_native_powr, [v16f32], 0, [{
    return _mm512_mask_pow_ps($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl native_powr_f32_promote = OclBuiltinImpl<native_powr, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote2>;

// native_recip
OclBuiltinImpl native_recip_f32_v16 = OclBuiltinImpl<native_recip, [v16f32], 0, [{
    return _mm512_recip_ps($Arg0VarName);
  }]>;
OclBuiltinImpl mask_native_recip_f32_v16 = OclBuiltinImpl<mask_native_recip, [v16f32], 0, [{
    return _mm512_mask_recip_ps($Arg1VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl native_recip_f32_promote = OclBuiltinImpl<native_recip, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote1>;

// native_rsqrt
OclBuiltinImpl native_rsqrt_f32_v16 = OclBuiltinImpl<native_rsqrt, [v16f32], 0, [{
    return _mm512_invsqrt_ps($Arg0VarName);
  }]>;
OclBuiltinImpl mask_native_rsqrt_f32_v16 = OclBuiltinImpl<mask_native_rsqrt, [v16f32], 0, [{
    return _mm512_mask_invsqrt_ps($Arg1VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl native_rsqrt_f32_promote = OclBuiltinImpl<native_rsqrt, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote1>;

// native_sin
OclBuiltinImpl native_sin_f32_v16 = OclBuiltinImpl<native_sin, [v16f32], 0, [{
    return _mm512_sin_ps($Arg0VarName);
  }]>;
OclBuiltinImpl mask_native_sin_f32_v16 = OclBuiltinImpl<mask_native_sin, [v16f32], 0, [{
    return _mm512_mask_sin_ps($Arg1VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl native_sin_f32_promote = OclBuiltinImpl<native_sin, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote1>;

// native_sqrt
OclBuiltinImpl native_sqrt_f32_v16 = OclBuiltinImpl<native_sqrt, [v16f32], 0, [{
    return _mm512_sqrt_ps($Arg0VarName);
  }]>;
OclBuiltinImpl mask_native_sqrt_f32_v16 = OclBuiltinImpl<mask_native_sqrt, [v16f32], 0, [{
    return _mm512_mask_sqrt_ps($Arg1VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl native_sqrt_f32_promote = OclBuiltinImpl<native_sqrt, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote1>;

// native_tan
OclBuiltinImpl native_tan_f32_v16 = OclBuiltinImpl<native_tan, [v16f32], 0, [{
    return _mm512_tan_ps($Arg0VarName);
  }]>;
OclBuiltinImpl mask_native_tan_f32_v16 = OclBuiltinImpl<mask_native_tan, [v16f32], 0, [{
    return _mm512_mask_tan_ps($Arg1VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl native_tan_f32_promote = OclBuiltinImpl<native_tan, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote1>;
