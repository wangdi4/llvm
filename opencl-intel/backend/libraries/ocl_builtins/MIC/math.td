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

// native_fdim
OclBuiltinImpl native_fdim_f32_v16 = OclBuiltinImpl<native_fdim, [v16f32], 0, [{
    __m512 ret = _mm512_sub_ps($Arg0VarName, $Arg1VarName);
    return _mm512_max_ps(ret, _mm512_setzero_ps());
  }]>;
OclBuiltinImpl mask_native_fdim_f32_v16 = OclBuiltinImpl<mask_native_fdim, [v16f32], 0, [{
    __m512 ret = _mm512_mask_sub_ps($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
    return _mm512_mask_max_ps(ret, $Arg0VarName, ret, _mm512_setzero_ps());
  }]>;
OclBuiltinImpl native_fdim_f32_promote = OclBuiltinImpl<native_fdim, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote2>;

OclBuiltinImpl native_fdim_f64_v8 = OclBuiltinImpl<native_fdim, [v8f64], 0, [{
    __m512d ret = _mm512_sub_pd($Arg0VarName, $Arg1VarName);
    return _mm512_max_pd(ret, _mm512_setzero_pd());
  }]>;
OclBuiltinImpl mask_native_fdim_f64_v8 = OclBuiltinImpl<mask_native_fdim, [v8f64], 0, [{
    __m512d ret = _mm512_mask_sub_pd($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
    return _mm512_mask_max_pd(ret, $Arg0VarName, ret, _mm512_setzero_pd());
  }]>;
OclBuiltinImpl native_fdim_f64_promote = OclBuiltinImpl<native_fdim, [v1f64, v2f64, v3f64, v4f64], 0, MaskPromote2>;
OclBuiltinImpl native_fdim_f64_expand = OclBuiltinImpl<native_fdim, [v16f64], 0, Expand2>;

// native_fmax
OclBuiltinImpl native_fmax_f32_v16 = OclBuiltinImpl<native_fmax, [v16f32], 0, [{
    return _mm512_max_ps($Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_native_fmax_f32_v16 = OclBuiltinImpl<mask_native_fmax, [v16f32], 0, [{
    return _mm512_mask_max_ps($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl native_fmax_f32_promote = OclBuiltinImpl<native_fmax, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote2>;

OclBuiltinImpl native_fmax_f64_v8 = OclBuiltinImpl<native_fmax, [v8f64], 0, [{
    return _mm512_max_pd($Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_native_fmax_f64_v8 = OclBuiltinImpl<mask_native_fmax, [v8f64], 0, [{
    return _mm512_mask_max_pd($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl native_fmax_f64_promote = OclBuiltinImpl<native_fmax, [v1f64, v2f64, v3f64, v4f64], 0, MaskPromote2>;
OclBuiltinImpl native_fmax_f64_expand = OclBuiltinImpl<native_fmax, [v16f64], 0, Expand2>;

// native_fmin
OclBuiltinImpl native_fmin_f32_v16 = OclBuiltinImpl<native_fmin, [v16f32], 0, [{
    return _mm512_min_ps($Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_native_fmin_f32_v16 = OclBuiltinImpl<mask_native_fmin, [v16f32], 0, [{
    return _mm512_mask_min_ps($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl native_fmin_f32_promote = OclBuiltinImpl<native_fmin, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote2>;

OclBuiltinImpl native_fmin_f64_v8 = OclBuiltinImpl<native_fmin, [v8f64], 0, [{
    return _mm512_min_pd($Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_native_fmin_f64_v8 = OclBuiltinImpl<mask_native_fmin, [v8f64], 0, [{
    return _mm512_mask_min_pd($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl native_fmin_f64_promote = OclBuiltinImpl<native_fmin, [v1f64, v2f64, v3f64, v4f64], 0, MaskPromote2>;
OclBuiltinImpl native_fmin_f64_expand = OclBuiltinImpl<native_fmin, [v16f64], 0, Expand2>;

// native_fmod
OclBuiltinImpl native_fmod_f32_v16 = OclBuiltinImpl<native_fmod, [v16f32], 0, [{
    __m512 ret = _mm512_trunc_ps(_mm512_div_ps($Arg0VarName, $Arg1VarName));
    return _mm512_msubr132_ps(ret, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_native_fmod_f32_v16 = OclBuiltinImpl<mask_native_fmod, [v16f32], 0, [{
    __m512 ret = _mm512_mask_trunc_ps($Arg1VarName, $Arg0VarName, _mm512_mask_div_ps($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName));
    return _mm512_mask_msubr132_ps(ret, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl native_fmod_f32_promote = OclBuiltinImpl<native_fmod, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote2>;

OclBuiltinImpl native_fmod_f64_v8 = OclBuiltinImpl<native_fmod, [v8f64], 0, [{
    __m512d ret = _mm512_trunc_pd(_mm512_div_pd($Arg0VarName, $Arg1VarName));
    return _mm512_msubr132_pd(ret, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_native_fmod_f64_v8 = OclBuiltinImpl<mask_native_fmod, [v8f64], 0, [{
    __m512d ret = _mm512_mask_trunc_pd($Arg1VarName, $Arg0VarName, _mm512_mask_div_pd($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName));
    return _mm512_mask_msubr132_pd(ret, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl native_fmod_f64_promote = OclBuiltinImpl<native_fmod, [v1f64, v2f64, v3f64, v4f64], 0, MaskPromote2>;
OclBuiltinImpl native_fmod_f64_expand = OclBuiltinImpl<native_fmod, [v16f64], 0, Expand2>;

// native_fract
code native_fract_f32_v16 = [{
    float16 f = _mm512_floor_ps($Arg0VarName);
    *($Arg1NoASType)$Arg1VarName = f;
    return _mm512_min_ps(_mm512_sub_ps($Arg0VarName, f), _mm512_set_1to16_ps(0x1.fffffep-1f));
  }];
code mask_native_fract_f32_v16 = [{
    float16 nx; nx$Pattern = $Arg0VarName;
    float16 f = _mm512_mask_floor_ps(nx, $Mask, nx);
    *($Arg1NoASType)$Arg1VarName = f$Pattern;
    float16 nret = _mm512_mask_min_ps(nx, $Mask, _mm512_mask_sub_ps(nx, $Mask, nx, f), _mm512_set_1to16_ps(0x1.fffffep-1f));
    return nret$Pattern;
  }];
OclBuiltinImpl native_fract_f32_v16_pas = OclBuiltinImpl<native_fract_pas, [v16f32], 0, native_fract_f32_v16>;
OclBuiltinImpl native_fract_f32_v16_las = OclBuiltinImpl<native_fract_las, [v16f32], 0, native_fract_f32_v16>;
OclBuiltinImpl native_fract_f32_v16_gas = OclBuiltinImpl<native_fract_gas, [v16f32], 0, native_fract_f32_v16>;
OclBuiltinImpl mask_native_fract_f32_v16_pas = OclBuiltinImpl<native_fract_pas, [v1f32, v2f32,v3f32, v4f32, v8f32], 0, mask_native_fract_f32_v16>;
OclBuiltinImpl mask_native_fract_f32_v16_las = OclBuiltinImpl<native_fract_las, [v1f32, v2f32,v3f32, v4f32, v8f32], 0, mask_native_fract_f32_v16>;
OclBuiltinImpl mask_native_fract_f32_v16_gas = OclBuiltinImpl<native_fract_gas, [v1f32, v2f32,v3f32, v4f32, v8f32], 0, mask_native_fract_f32_v16>;

code native_fract_f64_v8 = [{
    double8 f = _mm512_floor_pd($Arg0VarName);
    *($Arg1NoASType)$Arg1VarName = f;
    return _mm512_min_pd(_mm512_sub_pd($Arg0VarName, f), _mm512_set_1to8_pd(0x1.fffffep-1));
  }];
code mask_native_fract_f64_v8 = [{
    double8 nx; nx$Pattern = $Arg0VarName;
    double8 f = _mm512_mask_floor_pd(nx, $Mask, nx);
    *($Arg1NoASType)$Arg1VarName = f$Pattern;
    double8 nret = _mm512_mask_min_pd(nx, $Mask, _mm512_mask_sub_pd(nx, $Mask, nx, f), _mm512_set_1to8_pd(0x1.fffffep-1f));
    return nret$Pattern;
  }];
code expand_native_fract_f64_v16 = [{
    double16 ret;
    ret$ExpandLoPattern = native_fract($Arg0VarName$ExpandLoPattern, ((double8*)$Arg1VarName) + 0);
    ret$ExpandHiPattern = native_fract($Arg0VarName$ExpandHiPattern, ((double8*)$Arg1VarName) + 1);
    return ret;
  }];
OclBuiltinImpl native_fract_f64_v8_pas = OclBuiltinImpl<native_fract_pas, [v8f64], 0, native_fract_f64_v8>;
OclBuiltinImpl native_fract_f64_v8_las = OclBuiltinImpl<native_fract_las, [v8f64], 0, native_fract_f64_v8>;
OclBuiltinImpl native_fract_f64_v8_gas = OclBuiltinImpl<native_fract_gas, [v8f64], 0, native_fract_f64_v8>;
OclBuiltinImpl mask_native_fract_f64_v8_pas = OclBuiltinImpl<native_fract_pas, [v1f64, v2f64,v3f64, v4f64], 0, mask_native_fract_f64_v8>;
OclBuiltinImpl mask_native_fract_f64_v8_las = OclBuiltinImpl<native_fract_las, [v1f64, v2f64,v3f64, v4f64], 0, mask_native_fract_f64_v8>;
OclBuiltinImpl mask_native_fract_f64_v8_gas = OclBuiltinImpl<native_fract_gas, [v1f64, v2f64,v3f64, v4f64], 0, mask_native_fract_f64_v8>;
OclBuiltinImpl expand_native_fract_f64_v8_pas = OclBuiltinImpl<native_fract_pas, [v16f64], 0, expand_native_fract_f64_v16>;
OclBuiltinImpl expand_native_fract_f64_v8_las = OclBuiltinImpl<native_fract_las, [v16f64], 0, expand_native_fract_f64_v16>;
OclBuiltinImpl expand_native_fract_f64_v8_gas = OclBuiltinImpl<native_fract_gas, [v16f64], 0, expand_native_fract_f64_v16>;

// native_ilogb
OclBuiltinImpl native_ilogb_f32_v16 = OclBuiltinImpl<native_ilogb, [v16f32], 0, [{
    __m512 e = _mm512_getexp_ps($Arg0VarName);
    return as_$ReturnType(_mm512_cvtfxpnt_round_adjustps_epi32(e, _MM_FROUND_CUR_DIRECTION, _MM_EXPADJ_NONE));
  }]>;
OclBuiltinImpl mask_native_ilogb_f32_v16 = OclBuiltinImpl<mask_native_ilogb, [v16f32], 0, [{
    __m512 e = _mm512_mask_getexp_ps($Arg1VarName, $Arg0VarName, $Arg1VarName);
    return as_$ReturnType(_mm512_mask_cvtfxpnt_round_adjustps_epi32(_mm512_undefined_epi32(), $Arg0VarName, e, _MM_FROUND_CUR_DIRECTION, _MM_EXPADJ_NONE));
  }]>;
OclBuiltinImpl native_ilogb_f32_promote = OclBuiltinImpl<native_ilogb, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote1>;

OclBuiltinImpl native_ilogb_f64_v8 = OclBuiltinImpl<native_ilogb, [v8f64], 0, [{
    __m512d e = _mm512_getexp_pd($Arg0VarName);
    return as_int16(_mm512_cvtfxpnt_roundpd_epi32lo(e, _MM_FROUND_CUR_DIRECTION)).lo;
  }]>;
OclBuiltinImpl mask_native_ilogb_f64_v8 = OclBuiltinImpl<mask_native_ilogb, [v8f64], 0, [{
    __m512d e = _mm512_mask_getexp_pd($Arg1VarName, $Arg0VarName, $Arg1VarName);
    return as_int16(_mm512_mask_cvtfxpnt_roundpd_epi32lo(_mm512_undefined_epi32(), $Arg0VarName, e, _MM_FROUND_CUR_DIRECTION)).lo;
  }]>;
code MaskPromote1ilogb_f64 = [{
    double8 n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    int8 $ReturnVarName = mask_$NativeFunc($Mask, n$Arg0VarName);
    return $ReturnVarName$Pattern;
  }];
OclBuiltinImpl native_ilogb_f64_promote = OclBuiltinImpl<native_ilogb, [v1f64, v2f64, v3f64, v4f64], 0, MaskPromote1ilogb_f64>;
OclBuiltinImpl native_ilogb_f64_expand = OclBuiltinImpl<native_ilogb, [v16f64], 0, Expand1>;

// native_logb
OclBuiltinImpl native_logb_f32_v16 = OclBuiltinImpl<native_logb, [v16f32], 0, [{
    return _mm512_getexp_ps($Arg0VarName);
  }]>;
OclBuiltinImpl mask_native_logb_f32_v16 = OclBuiltinImpl<mask_native_logb, [v16f32], 0, [{
    return _mm512_mask_getexp_ps($Arg1VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl native_logb_f32_promote = OclBuiltinImpl<native_logb, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote1>;

OclBuiltinImpl native_logb_f64_v8 = OclBuiltinImpl<native_logb, [v8f64], 0, [{
    return _mm512_getexp_pd($Arg0VarName);
  }]>;
OclBuiltinImpl mask_native_logb_f64_v8 = OclBuiltinImpl<mask_native_logb, [v8f64], 0, [{
    return _mm512_mask_getexp_pd($Arg1VarName, $Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl native_logb_f64_promote = OclBuiltinImpl<native_logb, [v1f64, v2f64, v3f64, v4f64], 0, MaskPromote1>;
OclBuiltinImpl native_logb_f64_expand = OclBuiltinImpl<native_logb, [v16f64], 0, Expand1>;

// ilogb
OclBuiltinImpl ilogb_svml_f32_v3 = OclBuiltinImpl<ilogb, [v3f32], 0, [{
    float16 in;
    in.s012 = $Arg0VarName;
    int16 out = __ocl_svml_$Target_ilogbf16(in);
    return as_$ReturnType(out.s012);
  }]>;

OclBuiltinImpl ilogb_svml_f32_v8 = OclBuiltinImpl<ilogb, [v8f32], 0, [{
    float16 in;
    in.lo = $Arg0VarName;
    int16 out = __ocl_svml_$Target_ilogbf16(in);
    return as_$ReturnType(out.lo);
  }]>;

// ldexp
OclBuiltinImpl ldexp_svml_v8 = OclBuiltinImpl<ldexp, [v8f32], 0, [{
    float16 f;
    f.lo = $Arg0VarName;
    int16 i;
    i.lo = $Arg1VarName;
    float16 out = __ocl_svml_$Target_ldexpf16(f, i);
    return as_$ReturnType(out.lo);
  }]>;

// pown
OclBuiltinImpl pown_svml_v8 = OclBuiltinImpl<pown, [v8f32], 0, [{
    float16 f;
    f.lo = $Arg0VarName;
    int16 i;
    i.lo = $Arg1VarName;
    float16 out = __ocl_svml_$Target_pownf16(f, i);
    return as_$ReturnType(out.lo);
  }]>;

// rootn
OclBuiltinImpl rootn_svml_v8 = OclBuiltinImpl<rootn, [v8f32], 0, [{
    float16 f;
    f.lo = $Arg0VarName;
    int16 i;
    i.lo = $Arg1VarName;
    float16 out = __ocl_svml_$Target_rootnf16(f, i);
    return as_$ReturnType(out.lo);
  }]>;

// modf
code workaround_modf = [{
    float16 f;
    $Arg1NoASType pf = ($Arg1NoASType)(&f);
    $Arg0NoASType res = __ocl_svml_$Target_modf$SVMLSuffix($Arg0VarName, pf);
    *$Arg1VarName = *pf;
    return res;
}];
OclBuiltinImpl workaround_modf_pas = OclBuiltinImpl<modf_pas, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64], 0, workaround_modf>;
OclBuiltinImpl workaround_modf_gas = OclBuiltinImpl<modf_gas, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64], 0, workaround_modf>;
OclBuiltinImpl workaround_modf_las = OclBuiltinImpl<modf_las, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64], 0, workaround_modf>;

// fract
code workaround_fract = [{
    float16 f;
    $Arg1NoASType pf = ($Arg1NoASType)(&f);
    $Arg0NoASType res = __ocl_svml_$Target_fract$SVMLSuffix($Arg0VarName, pf);
    *$Arg1VarName = *pf;
    return res;
}];
OclBuiltinImpl workaround_fract_pas = OclBuiltinImpl<fract_pas, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64], 0, workaround_fract>;
OclBuiltinImpl workaround_fract_gas = OclBuiltinImpl<fract_gas, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64], 0, workaround_fract>;
OclBuiltinImpl workaround_fract_las = OclBuiltinImpl<fract_las, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64], 0, workaround_fract>;

// sincos
code workaround_sincos = [{
    float16 f;
    $Arg1NoASType pf = ($Arg1NoASType)(&f);
    $Arg0NoASType res = __ocl_svml_$Target_sincos$SVMLSuffix($Arg0VarName, pf);
    *$Arg1VarName = *pf;
    return res;
}];
OclBuiltinImpl workaround_sincos_pas = OclBuiltinImpl<sincos_pas, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64], 0, workaround_sincos>;
OclBuiltinImpl workaround_sincos_gas = OclBuiltinImpl<sincos_gas, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64], 0, workaround_sincos>;
OclBuiltinImpl workaround_sincos_las = OclBuiltinImpl<sincos_las, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64], 0, workaround_sincos>;

// frexp
code workaround_frexp = [{
    float16 f;
    $Arg1NoASType pf = ($Arg1NoASType)(&f);
    $Arg0NoASType res = __ocl_svml_$Target_frexp$SVMLSuffix($Arg0VarName, pf);
    *$Arg1VarName = *pf;
    return res;
}];
OclBuiltinImpl workaround_frexp_pas = OclBuiltinImpl<frexp_pas, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64, v8f64], 0, workaround_frexp>;
OclBuiltinImpl workaround_frexp_gas = OclBuiltinImpl<frexp_gas, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64, v8f64], 0, workaround_frexp>;
OclBuiltinImpl workaround_frexp_las = OclBuiltinImpl<frexp_las, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64, v8f64], 0, workaround_frexp>;

// remquo
code workaround_remquo = [{
    float16 f;
    $Arg2NoASType pf = ($Arg2NoASType)(&f);
    $Arg0NoASType res = __ocl_svml_$Target_remquo$SVMLSuffix($Arg0VarName, $Arg1VarName, pf);
    *$Arg2VarName = *pf;
    return res;
}];
OclBuiltinImpl workaround_remquo_pas = OclBuiltinImpl<remquo_pas, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64, v8f64], 0, workaround_remquo>;
OclBuiltinImpl workaround_remquo_gas = OclBuiltinImpl<remquo_gas, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64, v8f64], 0, workaround_remquo>;
OclBuiltinImpl workaround_remquo_las = OclBuiltinImpl<remquo_las, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64, v8f64], 0, workaround_remquo>;

// lgamma_r
code workaround_lgamma_r = [{
    float16 f;
    $Arg1NoASType pf = ($Arg1NoASType)(&f);
    $Arg0NoASType res = __ocl_svml_$Target_lgamma_r$SVMLSuffix($Arg0VarName, pf);
    *$Arg1VarName = *pf;
    return res;
}];
OclBuiltinImpl workaround_lgamma_r_pas = OclBuiltinImpl<lgamma_r_pas, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64, v8f64], 0, workaround_lgamma_r>;
OclBuiltinImpl workaround_lgamma_r_gas = OclBuiltinImpl<lgamma_r_gas, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64, v8f64], 0, workaround_lgamma_r>;
OclBuiltinImpl workaround_lgamma_r_las = OclBuiltinImpl<lgamma_r_las, [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64, v8f64], 0, workaround_lgamma_r>;
