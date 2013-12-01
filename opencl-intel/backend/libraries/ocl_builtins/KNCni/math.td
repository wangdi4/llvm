// vim:ts=2:sw=2:et:

list<OclType> mask_f32_types = [v1f32, v2f32, v3f32, v4f32, v8f32];
list<OclType> mask_f64_types = [v1f64, v2f64, v3f64, v4f64];
list<OclType> mask_types = [v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64];

// According to OpenCL 1.2 ch. 6.12.2 if arguments to fmin (or fmax) are
// equal then the first argument has to be returned. Signed zeroes -0.
// and +0. are considered to be equal. But the standard states fmin and
// fmax should behave as defined by C99 standard. Which in turn says that
// "ideally" fmin(+0., -0.) and fmax(-0, +0) whould be sensitive to the sign
// of zero and could return -0. and +0. respectively. The following
// implementation of fmin and fmax behaves exactly so.

// fmax
OclBuiltinImpl knc_fmax = OclBuiltinImpl<fmax, [v16f32, v8f64], 0, [{
    $Arg0Symm512gentype arg0 = ($Arg0Symm512gentype)$Arg0VarName;
    $Arg1Symm512gentype arg1 = ($Arg1Symm512gentype)$Arg1VarName;
    $Arg1Symm512gentype ret = _mm512_max_$Arg0Sympspdgentype(arg0, arg1);
    return as_$ReturnType(ret);
  }]>;

OclBuiltinImpl knc_fmax_mask = OclBuiltinImpl<fmax, mask_types, 0, [{
    $NativeArg0Type arg;
    arg$Pattern = $Arg0VarName;
    $Arg0Symm512gentype arg0 = ($Arg0Symm512gentype)arg;
    arg$Pattern = $Arg1VarName;
    $Arg1Symm512gentype arg1 = ($Arg1Symm512gentype)arg;
    $ReturnSymm512gentype ret = _mm512_mask_max_$Arg0Sympspdgentype(arg0, $Mask, arg0, arg1);
    return as_$NativeReturnType(ret)$Pattern;
  }]>;
OclBuiltinImpl knc_fmax_expand = OclBuiltinImpl<fmax, [v16f64], 0, Expand2>;

// fmin
OclBuiltinImpl knc_fmin = OclBuiltinImpl<fmin, [v16f32, v8f64], 0, [{
    $Arg0Symm512gentype arg0 = ($Arg0Symm512gentype)$Arg0VarName;
    $Arg1Symm512gentype arg1 = ($Arg1Symm512gentype)$Arg1VarName;
    $Arg1Symm512gentype ret = _mm512_min_$Arg0Sympspdgentype(arg0, arg1);
    return as_$ReturnType(ret);
  }]>;

OclBuiltinImpl knc_fmin_mask = OclBuiltinImpl<fmin, mask_types, 0, [{
    $NativeArg0Type arg;
    arg$Pattern = $Arg0VarName;
    $Arg0Symm512gentype arg0 = ($Arg0Symm512gentype)arg;
    arg$Pattern = $Arg1VarName;
    $Arg1Symm512gentype arg1 = ($Arg1Symm512gentype)arg;
    $ReturnSymm512gentype ret = _mm512_mask_min_$Arg0Sympspdgentype(arg0, $Mask, arg0, arg1);
    return as_$NativeReturnType(ret)$Pattern;
  }]>;
OclBuiltinImpl knc_fmin_expand = OclBuiltinImpl<fmin, [v16f64], 0, Expand2>;

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
    *(__private $Arg1NoASType)$Arg1VarName = f;
    return _mm512_min_ps(_mm512_sub_ps($Arg0VarName, f), _mm512_set_1to16_ps(0x1.fffffep-1f));
  }];
code mask_native_fract_f32_v16 = [{
    float16 nx; nx$Pattern = $Arg0VarName;
    float16 f = _mm512_mask_floor_ps(nx, $Mask, nx);
    *(__private $Arg1NoASType)$Arg1VarName = f$Pattern;
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
    *(__private $Arg1NoASType)$Arg1VarName = f;
    return _mm512_min_pd(_mm512_sub_pd($Arg0VarName, f), _mm512_set_1to8_pd(0x1.fffffep-1));
  }];
code mask_native_fract_f64_v8 = [{
    double8 nx; nx$Pattern = $Arg0VarName;
    double8 f = _mm512_mask_floor_pd(nx, $Mask, nx);
    *(__private $Arg1NoASType)$Arg1VarName = f$Pattern;
    double8 nret = _mm512_mask_min_pd(nx, $Mask, _mm512_mask_sub_pd(nx, $Mask, nx, f), _mm512_set_1to8_pd(0x1.fffffep-1f));
    return nret$Pattern;
  }];
code expand_native_fract_f64_v16 = [{
    double16 ret;
    ret.lo = native_fract($Arg0VarName.lo, ($Arg1gentype8)$Arg1VarName);
    ret.hi = native_fract($Arg0VarName.hi, ($Arg1gentype8)$Arg1VarName + 1);
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

// one argument code
code knc_code_1 = [{
    $ReturnSymm512gentype res =
        _mm512_$Func_$Arg0Sympspdgentype(($Arg0Symm512gentype)$Arg0VarName);
    return as_$NativeReturnType(res)$Pattern;
  }];
code knc_mask_code_1 = [{
    $NativeArg0Type arg0; arg0$Pattern = $Arg0VarName;
    $ReturnSymm512gentype res =
        _mm512_mask_$Func_$Arg0Sympspdgentype(_mm512_undefined_$Arg0Sympspdgentype(), $Mask, ($Arg0Symm512gentype)arg0);
    return as_$NativeReturnType(res)$Pattern;
  }];

// two arguments code
code knc_code_2 = [{
    $NativeArg1Type arg1; arg1$Pattern = $Arg1VarName;
    $ReturnSymm512gentype res =
        _mm512_$Func_$Arg0Sympspdgentype(($Arg0Symm512gentype)$Arg0VarName, ($Arg1Symm512gentype)arg1);
    return as_$NativeReturnType(res)$Pattern;
  }];
code knc_mask_code_2 = [{
    $NativeArg0Type arg0; arg0$Pattern = $Arg0VarName;
    $NativeArg1Type arg1; arg1$Pattern = $Arg1VarName;
    $ReturnSymm512gentype res =
        _mm512_mask_$Func_$Arg0Sympspdgentype(_mm512_undefined_$Arg0Sympspdgentype(), $Mask, ($Arg0Symm512gentype)arg0, ($Arg1Symm512gentype)arg1);
    return as_$NativeReturnType(res)$Pattern;
  }];

// three arguments code
code knc_code_3 = [{
    $ReturnSymm512gentype res =
        _mm512_$Func_$Arg0Sympspdgentype(($Arg0Symm512gentype)$Arg0VarName, ($Arg1Symm512gentype)$Arg1VarName, ($Arg2Symm512gentype)$Arg2VarName);
    return as_$NativeReturnType(res)$Pattern;
  }];
code knc_mask_code_3 = [{
    $NativeArg0Type arg0; arg0$Pattern = $Arg0VarName;
    $NativeArg1Type arg1; arg1$Pattern = $Arg1VarName;
    $NativeArg2Type arg2; arg2$Pattern = $Arg2VarName;
    $ReturnSymm512gentype res =
         _mm512_mask_$Func_$Arg0Sympspdgentype(($Arg0Symm512gentype)arg0, $Mask, ($Arg1Symm512gentype)arg1, ($Arg2Symm512gentype)arg2);
    return as_$NativeReturnType(res)$Pattern;
  }];

// WORKAROUND: CSSD100016256, direct SVML call for double16 works up to 70% faster
//             then two calls to double8 versions
code svml_v16f64_1 = [{
    // forward declaration to supress warnings 
    $ReturnType __ocl_svml_$Target_$Func$SVMLSuffix($Arg0Type);

    return __ocl_svml_$Target_$Func$SVMLSuffix($Arg0VarName);
  }];
code svml_v16f64_2 = [{
    // forward declaration to supress warnings 
    $ReturnType __ocl_svml_$Target_$Func$SVMLSuffix($Arg0Type, $Arg1Type);

    return __ocl_svml_$Target_$Func$SVMLSuffix($Arg0VarName, $Arg1VarName);
  }];

// acos
OclBuiltinImpl knc_acos         = OclBuiltinImpl<acos, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_acos    = OclBuiltinImpl<acos, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_acos  = OclBuiltinImpl<acos, [v16f64], 0, svml_v16f64_1>;

// acosh
OclBuiltinImpl knc_acosh         = OclBuiltinImpl<acosh, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_acosh    = OclBuiltinImpl<acosh, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_acosh  = OclBuiltinImpl<acosh, [v16f64], 0, svml_v16f64_1>;

// asin
OclBuiltinImpl knc_asin        = OclBuiltinImpl<asin, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_asin   = OclBuiltinImpl<asin, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_asin = OclBuiltinImpl<asin, [v16f64], 0, svml_v16f64_1>;

// asinh
OclBuiltinImpl knc_asinh        = OclBuiltinImpl<asinh, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_asinh   = OclBuiltinImpl<asinh, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_asinh = OclBuiltinImpl<asinh, [v16f64], 0, svml_v16f64_1>;

// atan
OclBuiltinImpl knc_atan        = OclBuiltinImpl<atan, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_atan   = OclBuiltinImpl<atan, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_atan = OclBuiltinImpl<atan, [v16f64], 0, svml_v16f64_1>;

// atan2
OclBuiltinImpl knc_atan2        = OclBuiltinImpl<atan2, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_atan2   = OclBuiltinImpl<atan2, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_atan2 = OclBuiltinImpl<atan2, [v16f64], 0, svml_v16f64_2>;

// atanh
OclBuiltinImpl knc_atanh        = OclBuiltinImpl<atanh, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_atanh   = OclBuiltinImpl<atanh, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_atanh = OclBuiltinImpl<atanh, [v16f64], 0, svml_v16f64_1>;

// cbrt
OclBuiltinImpl knc_cbrt        = OclBuiltinImpl<cbrt, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_cbrt   = OclBuiltinImpl<cbrt, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_cbrt = OclBuiltinImpl<cbrt, [v16f64], 0, svml_v16f64_1>;

// ceil
OclBuiltinImpl knc_ceil        = OclBuiltinImpl<ceil, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_ceil   = OclBuiltinImpl<ceil, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_ceil = OclBuiltinImpl<ceil, [v16f64], 0, Expand1>;

// cos
OclBuiltinImpl knc_cos        = OclBuiltinImpl<cos, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_cos   = OclBuiltinImpl<cos, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_cos = OclBuiltinImpl<cos, [v16f64], 0, svml_v16f64_1>;

// cosh
OclBuiltinImpl knc_cosh        = OclBuiltinImpl<cosh, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_cosh   = OclBuiltinImpl<cosh, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_cosh = OclBuiltinImpl<cosh, [v16f64], 0, svml_v16f64_1>;

// floor
OclBuiltinImpl knc_floor        = OclBuiltinImpl<floor, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_floor   = OclBuiltinImpl<floor, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_floor = OclBuiltinImpl<floor, [v16f64], 0, Expand1>;

// hypot
OclBuiltinImpl knc_hypot        = OclBuiltinImpl<hypot, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_hypot   = OclBuiltinImpl<hypot, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_hypot = OclBuiltinImpl<hypot, [v16f64], 0, svml_v16f64_2>;

// erfc
OclBuiltinImpl knc_erfc        = OclBuiltinImpl<erfc, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_erfc   = OclBuiltinImpl<erfc, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_erfc = OclBuiltinImpl<erfc, [v16f64], 0, Expand1>;

// erf
OclBuiltinImpl knc_erf        = OclBuiltinImpl<erf, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_erf   = OclBuiltinImpl<erf, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_erf = OclBuiltinImpl<erf, [v16f64], 0, Expand1>;

// exp
OclBuiltinImpl knc_exp        = OclBuiltinImpl<exp, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_exp   = OclBuiltinImpl<exp, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_exp = OclBuiltinImpl<exp, [v16f64], 0, svml_v16f64_1>;

// exp2
OclBuiltinImpl knc_exp2        = OclBuiltinImpl<exp2, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_exp2   = OclBuiltinImpl<exp2, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_exp2 = OclBuiltinImpl<exp2, [v16f64], 0, svml_v16f64_1>;

// exp10
OclBuiltinImpl knc_exp10        = OclBuiltinImpl<exp10, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_exp10   = OclBuiltinImpl<exp10, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_exp10 = OclBuiltinImpl<exp10, [v16f64], 0, svml_v16f64_1>;

// expm1
OclBuiltinImpl knc_expm1        = OclBuiltinImpl<expm1, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_expm1   = OclBuiltinImpl<expm1, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_expm1 = OclBuiltinImpl<expm1, [v16f64], 0, svml_v16f64_1>;

// fma
OclBuiltinImpl knc_fma        = OclBuiltinImpl<fma, [v16f32, v8f64], 0, knc_code_3, ["$Func"->"madd213"]>;
OclBuiltinImpl knc_mask_fma   = OclBuiltinImpl<fma, mask_types, 0, knc_mask_code_3, ["$Func"->"madd213"]>;
OclBuiltinImpl knc_expand_fma = OclBuiltinImpl<fma, [v16f64], 0, Expand3, ["$Func"->"madd213"]>;

// log
OclBuiltinImpl knc_log        = OclBuiltinImpl<log, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_log   = OclBuiltinImpl<log, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_log = OclBuiltinImpl<log, [v16f64], 0, svml_v16f64_1>;

// log2
OclBuiltinImpl knc_log2        = OclBuiltinImpl<log2, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_log2   = OclBuiltinImpl<log2, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_log2 = OclBuiltinImpl<log2, [v16f64], 0, svml_v16f64_1>;

// log10
OclBuiltinImpl knc_log10        = OclBuiltinImpl<log10, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_log10   = OclBuiltinImpl<log10, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_log10 = OclBuiltinImpl<log10, [v16f64], 0, svml_v16f64_1>;

// log1p
OclBuiltinImpl knc_log1p        = OclBuiltinImpl<log1p, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_log1p   = OclBuiltinImpl<log1p, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_log1p = OclBuiltinImpl<log1p, [v16f64], 0, svml_v16f64_1>;

// logb
OclBuiltinImpl knc_logb        = OclBuiltinImpl<logb, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_logb   = OclBuiltinImpl<logb, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_logb = OclBuiltinImpl<logb, [v16f64], 0, svml_v16f64_1>;

// pow
OclBuiltinImpl knc_pow        = OclBuiltinImpl<pow, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_pow   = OclBuiltinImpl<pow, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_pow = OclBuiltinImpl<pow, [v16f64], 0, svml_v16f64_2>;

// rint
OclBuiltinImpl knc_rint        = OclBuiltinImpl<rint, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_rint   = OclBuiltinImpl<rint, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_rint = OclBuiltinImpl<rint, [v16f64], 0, svml_v16f64_1>;

// sin
OclBuiltinImpl knc_sin        = OclBuiltinImpl<sin, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_sin   = OclBuiltinImpl<sin, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_sin = OclBuiltinImpl<sin, [v16f64], 0, svml_v16f64_1>;

// sinh
OclBuiltinImpl knc_sinh        = OclBuiltinImpl<sinh, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_sinh   = OclBuiltinImpl<sinh, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_sinh = OclBuiltinImpl<sinh, [v16f64], 0, svml_v16f64_1>;

// sqrt
OclBuiltinImpl knc_sqrt        = OclBuiltinImpl<sqrt, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_sqrt   = OclBuiltinImpl<sqrt, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_sqrt = OclBuiltinImpl<sqrt, [v16f64], 0, Expand1>;

// tan
OclBuiltinImpl knc_tan        = OclBuiltinImpl<tan, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_tan   = OclBuiltinImpl<tan, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_tan = OclBuiltinImpl<tan, [v16f64], 0, Expand1>;

// tanh
OclBuiltinImpl knc_tanh        = OclBuiltinImpl<tanh, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_tanh   = OclBuiltinImpl<tanh, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_tanh = OclBuiltinImpl<tanh, [v16f64], 0, svml_v16f64_1>;

// trunc
OclBuiltinImpl knc_trunc        = OclBuiltinImpl<trunc, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_trunc   = OclBuiltinImpl<trunc, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_trunc = OclBuiltinImpl<trunc, [v16f64], 0, Expand1>;

// acospi
OclBuiltinImpl knc_acospi        = OclBuiltinImpl<acospi, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_acospi   = OclBuiltinImpl<acospi, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_acospi = OclBuiltinImpl<acospi, [v16f64], 0, svml_v16f64_1>;

// asinpi
OclBuiltinImpl knc_asinpi        = OclBuiltinImpl<asinpi, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_asinpi   = OclBuiltinImpl<asinpi, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_asinpi = OclBuiltinImpl<asinpi, [v16f64], 0, svml_v16f64_1>;
 
// atanpi
OclBuiltinImpl knc_atanpi        = OclBuiltinImpl<atanpi, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_atanpi   = OclBuiltinImpl<atanpi, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_atanpi = OclBuiltinImpl<atanpi, [v16f64], 0, svml_v16f64_1>;
 
// atan2pi
OclBuiltinImpl knc_atan2pi        = OclBuiltinImpl<atan2pi, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_atan2pi   = OclBuiltinImpl<atan2pi, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_atan2pi = OclBuiltinImpl<atan2pi, [v16f64], 0, svml_v16f64_2>;
 
// cospi
OclBuiltinImpl knc_cospi        = OclBuiltinImpl<cospi, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_cospi   = OclBuiltinImpl<cospi, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_cospi = OclBuiltinImpl<cospi, [v16f64], 0, svml_v16f64_1>;
 
// fmod
OclBuiltinImpl knc_fmod        = OclBuiltinImpl<fmod, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_fmod   = OclBuiltinImpl<fmod, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_fmod = OclBuiltinImpl<fmod, [v16f64], 0, svml_v16f64_2>;
 
// ilogb
OclBuiltinImpl knc_ilogb_v8f64  = OclBuiltinImpl<ilogb, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_ilogb   = OclBuiltinImpl<ilogb, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_ilogb = OclBuiltinImpl<ilogb, [v16f64], 0, svml_v16f64_1>;

// ldexp
// ldexp_s will call ldexp with the second argument casted to the vector
OclBuiltinImpl knc_ldexp_v8f64  = OclBuiltinImpl<ldexp, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_ldexp   = OclBuiltinImpl<ldexp, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_ldexp = OclBuiltinImpl<ldexp, [v16f64], 0, svml_v16f64_2>;

// lgamma
OclBuiltinImpl knc_lgamma        = OclBuiltinImpl<lgamma, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_lgamma   = OclBuiltinImpl<lgamma, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_lgamma = OclBuiltinImpl<lgamma, [v16f64], 0, svml_v16f64_1>;

// nextafter
OclBuiltinImpl knc_nextafter        = OclBuiltinImpl<nextafter, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_nextafter   = OclBuiltinImpl<nextafter, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_nextafter = OclBuiltinImpl<nextafter, [v16f64], 0, svml_v16f64_2>;

// pown
OclBuiltinImpl knc_pown_v8f64  = OclBuiltinImpl<pown, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_pown   = OclBuiltinImpl<pown, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_pown = OclBuiltinImpl<pown, [v16f64], 0, svml_v16f64_2>;

// powr
OclBuiltinImpl knc_powr        = OclBuiltinImpl<powr, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_powr   = OclBuiltinImpl<powr, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_powr = OclBuiltinImpl<powr, [v16f64], 0, svml_v16f64_2>;

// remainder
OclBuiltinImpl knc_remainder        = OclBuiltinImpl<remainder, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_remainder   = OclBuiltinImpl<remainder, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_remainder = OclBuiltinImpl<remainder, [v16f64], 0, svml_v16f64_2>;

// rootn
OclBuiltinImpl knc_rootn_v8f64  = OclBuiltinImpl<rootn, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_rootn   = OclBuiltinImpl<rootn, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_rootn = OclBuiltinImpl<rootn, [v16f64], 0, svml_v16f64_2>;

// rsqrt
OclBuiltinImpl knc_rsqrt        = OclBuiltinImpl<rsqrt, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_rsqrt   = OclBuiltinImpl<rsqrt, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_rsqrt = OclBuiltinImpl<rsqrt, [v16f64], 0, svml_v16f64_1>;

// sinpi
OclBuiltinImpl knc_sinpi        = OclBuiltinImpl<sinpi, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_sinpi   = OclBuiltinImpl<sinpi, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_sinpi = OclBuiltinImpl<sinpi, [v16f64], 0, svml_v16f64_1>;

// tanpi
OclBuiltinImpl knc_tanpi        = OclBuiltinImpl<tanpi, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_tanpi   = OclBuiltinImpl<tanpi, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_tanpi = OclBuiltinImpl<tanpi, [v16f64], 0, svml_v16f64_1>;

// tgamma
OclBuiltinImpl knc_tgamma        = OclBuiltinImpl<tgamma, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_tgamma   = OclBuiltinImpl<tgamma, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_tgamma = OclBuiltinImpl<tgamma, [v16f64], 0, svml_v16f64_1>;

// round
OclBuiltinImpl knc_round_mask_f32 = OclBuiltinImpl<round, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, [{
    float16 arg0; arg0$Pattern = $Arg0VarName;
    __m512 rounded_to_zero =
    _mm512_mask_round_ps(arg0, $Mask, arg0, _MM_FROUND_TO_ZERO, _MM_EXPADJ_NONE);
    __m512 diff = _mm512_mask_sub_ps(arg0, $Mask, arg0, rounded_to_zero);
    __mmask16 to_pos_inf = _mm512_mask_cmpnlt_ps($Mask, diff, (__m512)(float16)(+.5f));
    __mmask16 to_neg_inf = _mm512_mask_cmple_ps($Mask, diff, (__m512)(float16)(-.5f));
    __m512 res = _mm512_mask_round_ps(rounded_to_zero, to_pos_inf, arg0, _MM_FROUND_TO_POS_INF, _MM_EXPADJ_NONE);
    res = _mm512_mask_round_ps(res, to_neg_inf, arg0, _MM_FROUND_TO_NEG_INF, _MM_EXPADJ_NONE);
    return as_float16(res)$Pattern;
  }]>;

OclBuiltinImpl knc_round_f32 = OclBuiltinImpl<round, [v16f32], 0, [{
    __m512 arg0 = (__m512)$Arg0VarName;
    __m512 rounded_to_zero = _mm512_round_ps(arg0, _MM_FROUND_TO_ZERO, _MM_EXPADJ_NONE);
    __m512 diff = _mm512_sub_ps(arg0, rounded_to_zero);
    __mmask16 to_pos_inf = _mm512_cmpnlt_ps(diff, (__m512)(float16)(+.5f));
    __mmask16 to_neg_inf = _mm512_cmple_ps(diff, (__m512)(float16)(-.5f));
    __m512 res = _mm512_mask_round_ps(rounded_to_zero, to_pos_inf, arg0, _MM_FROUND_TO_POS_INF, _MM_EXPADJ_NONE);
    res = _mm512_mask_round_ps(res, to_neg_inf, arg0, _MM_FROUND_TO_NEG_INF, _MM_EXPADJ_NONE);
    return as_float16(res);
  }]>;

OclBuiltinImpl knc_round_v8_f64 = OclBuiltinImpl<round, [v8f64], 0, knc_code_1, ["$Func"->"svml_round"]>;
OclBuiltinImpl knc_round_mask_f64 = OclBuiltinImpl<round, mask_f64_types, 0, knc_mask_code_1, ["$Func"->"svml_round"]>;
OclBuiltinImpl knc_round_expand = OclBuiltinImpl<round, [v16f64], 0, svml_v16f64_1>;

code knc_code_2p = [{
    $Arg1Symm512gentype arg1; 
    $ReturnType ret = _mm512_$Func_$Arg0Sympspdgentype($Arg0VarName, &arg1);
    *$Arg1VarName = as_$Arg1Symmicnativegentype(arg1)$Pattern;
    return ret;
  }];

code knc_mask_code_2p = [{
    $NativeArg0Type arg0; arg0$Pattern = $Arg0VarName;
    $Arg1Symm512gentype arg1;
    $NativeReturnType ret = _mm512_mask_$Func_$Arg0Sympspdgentype(_mm512_undefined_$Arg0Sympspdgentype(), $Mask, arg0, &arg1);
    *$Arg1VarName = as_$Arg1Symmicnativegentype(arg1)$Pattern;
    return ret$Pattern;
  }];

code knc_expand_code_2p = [{
     int16 lo, hi;
     __private $Arg1BaseType#8 * p_lo = (__private $Arg1BaseType#8 *)&lo;
     __private $Arg1BaseType#8 * p_hi = (__private $Arg1BaseType#8 *)&hi;
     $ReturnBaseType#16 $ReturnVarName = 
         ($ReturnBaseType#16)($Func($Arg0VarName.lo, p_lo), $Func($Arg0VarName.hi, p_hi));
     *$Arg1VarName = ($Arg1BaseType#16)(*p_lo, *p_hi);
     return $ReturnVarName;
  }];

// fract
OclBuiltinImpl knc_fract_pas   = OclBuiltinImpl<fract_pas, [v16f32, v8f64], 0, knc_code_2p>;
OclBuiltinImpl knc_fract_las   = OclBuiltinImpl<fract_las, [v16f32, v8f64], 0, knc_code_2p>;
OclBuiltinImpl knc_fract_gas   = OclBuiltinImpl<fract_gas, [v16f32, v8f64], 0, knc_code_2p>;

OclBuiltinImpl knc_mask_fract_pas   = OclBuiltinImpl<fract_pas, mask_types, 0, knc_mask_code_2p>;
OclBuiltinImpl knc_mask_fract_las   = OclBuiltinImpl<fract_las, mask_types, 0, knc_mask_code_2p>;
OclBuiltinImpl knc_mask_fract_gas   = OclBuiltinImpl<fract_gas, mask_types, 0, knc_mask_code_2p>;

OclBuiltinImpl knc_expand_fract_pas   = OclBuiltinImpl<fract_pas, [v16f64], 0, knc_expand_code_2p>;
OclBuiltinImpl knc_expand_fract_las   = OclBuiltinImpl<fract_las, [v16f64], 0, knc_expand_code_2p>;
OclBuiltinImpl knc_expand_fract_gas   = OclBuiltinImpl<fract_gas, [v16f64], 0, knc_expand_code_2p>;

// frexp
OclBuiltinImpl knc_frexp_pas   = OclBuiltinImpl<frexp_pas, [v16f32, v8f64], 0, knc_code_2p>;
OclBuiltinImpl knc_frexp_las   = OclBuiltinImpl<frexp_las, [v16f32, v8f64], 0, knc_code_2p>;
OclBuiltinImpl knc_frexp_gas   = OclBuiltinImpl<frexp_gas, [v16f32, v8f64], 0, knc_code_2p>;

OclBuiltinImpl knc_mask_frexp_pas   = OclBuiltinImpl<frexp_pas, mask_types, 0, knc_mask_code_2p>;
OclBuiltinImpl knc_mask_frexp_las   = OclBuiltinImpl<frexp_las, mask_types, 0, knc_mask_code_2p>;
OclBuiltinImpl knc_mask_frexp_gas   = OclBuiltinImpl<frexp_gas, mask_types, 0, knc_mask_code_2p>;

OclBuiltinImpl knc_expand_frexp_pas   = OclBuiltinImpl<frexp_pas, [v16f64], 0, knc_expand_code_2p>;
OclBuiltinImpl knc_expand_frexp_las   = OclBuiltinImpl<frexp_las, [v16f64], 0, knc_expand_code_2p>;
OclBuiltinImpl knc_expand_frexp_gas   = OclBuiltinImpl<frexp_gas, [v16f64], 0, knc_expand_code_2p>;

// lgamma_r
OclBuiltinImpl knc_lgamma_r_pas   = OclBuiltinImpl<lgamma_r_pas, [v16f32, v8f64], 0, knc_code_2p>;
OclBuiltinImpl knc_lgamma_r_las   = OclBuiltinImpl<lgamma_r_las, [v16f32, v8f64], 0, knc_code_2p>;
OclBuiltinImpl knc_lgamma_r_gas   = OclBuiltinImpl<lgamma_r_gas, [v16f32, v8f64], 0, knc_code_2p>;

OclBuiltinImpl knc_mask_lgamma_r_pas   = OclBuiltinImpl<lgamma_r_pas, mask_types, 0, knc_mask_code_2p>;
OclBuiltinImpl knc_mask_lgamma_r_las   = OclBuiltinImpl<lgamma_r_las, mask_types, 0, knc_mask_code_2p>;
OclBuiltinImpl knc_mask_lgamma_r_gas   = OclBuiltinImpl<lgamma_r_gas, mask_types, 0, knc_mask_code_2p>;

OclBuiltinImpl knc_expand_lgamma_r_pas   = OclBuiltinImpl<lgamma_r_pas, [v16f64], 0, knc_expand_code_2p>;
OclBuiltinImpl knc_expand_lgamma_r_las   = OclBuiltinImpl<lgamma_r_las, [v16f64], 0, knc_expand_code_2p>;
OclBuiltinImpl knc_expand_lgamma_r_gas   = OclBuiltinImpl<lgamma_r_gas, [v16f64], 0, knc_expand_code_2p>;

// modf
OclBuiltinImpl knc_modf_pas   = OclBuiltinImpl<modf_pas, [v16f32, v8f64], 0, knc_code_2p>;
OclBuiltinImpl knc_modf_las   = OclBuiltinImpl<modf_las, [v16f32, v8f64], 0, knc_code_2p>;
OclBuiltinImpl knc_modf_gas   = OclBuiltinImpl<modf_gas, [v16f32, v8f64], 0, knc_code_2p>;

OclBuiltinImpl knc_mask_modf_pas   = OclBuiltinImpl<modf_pas, mask_types, 0, knc_mask_code_2p>;
OclBuiltinImpl knc_mask_modf_las   = OclBuiltinImpl<modf_las, mask_types, 0, knc_mask_code_2p>;
OclBuiltinImpl knc_mask_modf_gas   = OclBuiltinImpl<modf_gas, mask_types, 0, knc_mask_code_2p>;

OclBuiltinImpl knc_expand_modf_pas   = OclBuiltinImpl<modf_pas, [v16f64], 0, knc_expand_code_2p>;
OclBuiltinImpl knc_expand_modf_las   = OclBuiltinImpl<modf_las, [v16f64], 0, knc_expand_code_2p>;
OclBuiltinImpl knc_expand_modf_gas   = OclBuiltinImpl<modf_gas, [v16f64], 0, knc_expand_code_2p>;

// sincos
code sincos_mask_code = [{
    $NativeArg0Type arg0; arg0$Pattern = $Arg0VarName;
    $Arg1Symmicnativegentype arg1; 
    $NativeReturnType ret = _mm512_mask_$Func_$Arg0Sympspdgentype(_mm512_undefined_$Arg0Sympspdgentype(), _mm512_undefined_$Arg0Sympspdgentype(), $Mask, arg0, &arg1);
    *$Arg1VarName = arg1$Pattern;
    return ret$Pattern;
  }];
OclBuiltinImpl knc_sincos_pas   = OclBuiltinImpl<sincos_pas, [v16f32, v8f64], 0, knc_code_2p>;
OclBuiltinImpl knc_sincos_las   = OclBuiltinImpl<sincos_las, [v16f32, v8f64], 0, knc_code_2p>;
OclBuiltinImpl knc_sincos_gas   = OclBuiltinImpl<sincos_gas, [v16f32, v8f64], 0, knc_code_2p>;

OclBuiltinImpl knc_mask_sincos_pas   = OclBuiltinImpl<sincos_pas, mask_types, 0, sincos_mask_code>;
OclBuiltinImpl knc_mask_sincos_las   = OclBuiltinImpl<sincos_las, mask_types, 0, sincos_mask_code>;
OclBuiltinImpl knc_mask_sincos_gas   = OclBuiltinImpl<sincos_gas, mask_types, 0, sincos_mask_code>;

OclBuiltinImpl knc_expand_sincos_pas   = OclBuiltinImpl<sincos_pas, [v16f64], 0, knc_expand_code_2p>;
OclBuiltinImpl knc_expand_sincos_las   = OclBuiltinImpl<sincos_las, [v16f64], 0, knc_expand_code_2p>;
OclBuiltinImpl knc_expand_sincos_gas   = OclBuiltinImpl<sincos_gas, [v16f64], 0, knc_expand_code_2p>;

code knc_code_3p = [{
    $Arg2Symm512gentype arg2; 
    $ReturnType ret = _mm512_$Func_$Arg0Sympspdgentype($Arg0VarName, $Arg1VarName, &arg2);
    *$Arg2VarName = as_$Arg2Symmicnativegentype(arg2)$Pattern;
    return ret;
  }];

code knc_mask_code_3p = [{
    $NativeArg0Type arg0; arg0$Pattern = $Arg0VarName;
    $NativeArg1Type arg1; arg1$Pattern = $Arg1VarName;
    $Arg2Symm512gentype arg2;
    $NativeReturnType ret = _mm512_mask_$Func_$Arg0Sympspdgentype(_mm512_undefined_$Arg0Sympspdgentype(), $Mask, arg0, arg1, &arg2);
    *$Arg2VarName = as_$Arg2Symmicnativegentype(arg2)$Pattern;
    return ret$Pattern;
  }];

code knc_expand_code_3p = [{
     int16 lo, hi;
     $Arg2BaseType#8 * p_lo = ($Arg2BaseType#8 *)&lo;
     $Arg2BaseType#8 * p_hi = ($Arg2BaseType#8 *)&hi;
     $ReturnBaseType#16 $ReturnVarName = 
         ($ReturnBaseType#16)($Func($Arg0VarName.lo, $Arg1VarName.lo, p_lo), $Func($Arg0VarName.hi, $Arg1VarName.hi, p_hi));
     *$Arg2VarName = ($Arg2BaseType#16)(*p_lo, *p_hi);
     return $ReturnVarName;
  }];

// remquo
OclBuiltinImpl knc_remquo_pas   = OclBuiltinImpl<remquo_pas, [v16f32, v8f64], 0, knc_code_3p>;
OclBuiltinImpl knc_remquo_las   = OclBuiltinImpl<remquo_las, [v16f32, v8f64], 0, knc_code_3p>;
OclBuiltinImpl knc_remquo_gas   = OclBuiltinImpl<remquo_gas, [v16f32, v8f64], 0, knc_code_3p>;

OclBuiltinImpl knc_mask_remquo_pas   = OclBuiltinImpl<remquo_pas, mask_types, 0, knc_mask_code_3p>;
OclBuiltinImpl knc_mask_remquo_las   = OclBuiltinImpl<remquo_las, mask_types, 0, knc_mask_code_3p>;
OclBuiltinImpl knc_mask_remquo_gas   = OclBuiltinImpl<remquo_gas, mask_types, 0, knc_mask_code_3p>;

OclBuiltinImpl knc_expand_remquo_pas   = OclBuiltinImpl<remquo_pas, [v16f64], 0, knc_expand_code_3p>;
OclBuiltinImpl knc_expand_remquo_las   = OclBuiltinImpl<remquo_las, [v16f64], 0, knc_expand_code_3p>;
OclBuiltinImpl knc_expand_remquo_gas   = OclBuiltinImpl<remquo_gas, [v16f64], 0, knc_expand_code_3p>;

// mad, maxmag, minmag, nan, copysign are implemented in GENERIC/math.td 

// half builtins
// half_cos
OclBuiltinImpl knc_half_cos         = OclBuiltinImpl<half_cos, [v16f32], 0, knc_code_1>;
OclBuiltinImpl knc_mask_half_cos    = OclBuiltinImpl<half_cos, mask_f32_types, 0, knc_mask_code_1>;

// half_exp10
OclBuiltinImpl knc_half_exp10         = OclBuiltinImpl<half_exp10, [v16f32], 0, knc_code_1>;
OclBuiltinImpl knc_mask_half_exp10    = OclBuiltinImpl<half_exp10, mask_f32_types, 0, knc_mask_code_1>;

// half_log10
OclBuiltinImpl knc_half_log10         = OclBuiltinImpl<half_log10, [v16f32], 0, knc_code_1>;
OclBuiltinImpl knc_mask_half_log10    = OclBuiltinImpl<half_log10, mask_f32_types, 0, knc_mask_code_1>;

// half_sin
OclBuiltinImpl knc_half_sin         = OclBuiltinImpl<half_sin, [v16f32], 0, knc_code_1>;
OclBuiltinImpl knc_mask_half_sin    = OclBuiltinImpl<half_sin, mask_f32_types, 0, knc_mask_code_1>;

// half_sqrt
OclBuiltinImpl knc_half_sqrt         = OclBuiltinImpl<half_sqrt, [v16f32], 0, knc_code_1>;
OclBuiltinImpl knc_mask_half_sqrt    = OclBuiltinImpl<half_sqrt, mask_f32_types, 0, knc_mask_code_1>;

// half_tan
OclBuiltinImpl knc_half_tan         = OclBuiltinImpl<half_tan, [v16f32], 0, knc_code_1>;
OclBuiltinImpl knc_mask_half_tan    = OclBuiltinImpl<half_tan, mask_f32_types, 0, knc_mask_code_1>;

// half_recip
OclBuiltinImpl knc_half_recip         = OclBuiltinImpl<half_recip, [v16f32], 0, knc_code_1>;
OclBuiltinImpl knc_mask_half_recip    = OclBuiltinImpl<half_recip, mask_f32_types, 0, knc_mask_code_1>;

// half_rsqrt
OclBuiltinImpl knc_half_rsqrt         = OclBuiltinImpl<half_rsqrt, [v16f32], 0, knc_code_1>;
OclBuiltinImpl knc_mask_half_rsqrt    = OclBuiltinImpl<half_rsqrt, mask_f32_types, 0, knc_mask_code_1>;

// half_exp
OclBuiltinImpl knc_half_exp         = OclBuiltinImpl<half_exp, [v16f32], 0, knc_code_1>;
OclBuiltinImpl knc_mask_half_exp    = OclBuiltinImpl<half_exp, mask_f32_types, 0, knc_mask_code_1>;

// half_exp2
OclBuiltinImpl knc_half_exp2         = OclBuiltinImpl<half_exp2, [v16f32], 0, knc_code_1>;
OclBuiltinImpl knc_mask_half_exp2    = OclBuiltinImpl<half_exp2, mask_f32_types, 0, knc_mask_code_1>;

// half_log
OclBuiltinImpl knc_half_log         = OclBuiltinImpl<half_log, [v16f32], 0, knc_code_1>;
OclBuiltinImpl knc_mask_half_log    = OclBuiltinImpl<half_log, mask_f32_types, 0, knc_mask_code_1>;

// half_divide 
OclBuiltinImpl knc_half_divide         = OclBuiltinImpl<half_divide, [v16f32], 0, knc_code_2>;
OclBuiltinImpl knc_mask_half_divide    = OclBuiltinImpl<half_divide, mask_f32_types, 0, knc_mask_code_2>;

// half_log2
OclBuiltinImpl knc_half_log2         = OclBuiltinImpl<half_log2, [v16f32], 0, knc_code_1>;
OclBuiltinImpl knc_mask_half_log2    = OclBuiltinImpl<half_log2, mask_f32_types, 0, knc_mask_code_1>;

// half_powr 
OclBuiltinImpl knc_half_powr         = OclBuiltinImpl<half_powr, [v16f32], 0, knc_code_2>;
OclBuiltinImpl knc_mask_half_powr    = OclBuiltinImpl<half_powr, mask_f32_types, 0, knc_mask_code_2>;

// native builtins
// native_acos
OclBuiltinImpl knc_native_acos        = OclBuiltinImpl<native_acos, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_acos   = OclBuiltinImpl<native_acos, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_acos = OclBuiltinImpl<native_acos, [v16f64], 0, Expand1>;

// native_acosh
OclBuiltinImpl knc_native_acosh        = OclBuiltinImpl<native_acosh, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_acosh   = OclBuiltinImpl<native_acosh, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_acosh = OclBuiltinImpl<native_acosh, [v16f64], 0, Expand1>;

// native_acospi
OclBuiltinImpl knc_native_acospi        = OclBuiltinImpl<native_acospi, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_acospi   = OclBuiltinImpl<native_acospi, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_acospi = OclBuiltinImpl<native_acospi, [v16f64], 0, Expand1>;

// native_asin
OclBuiltinImpl knc_native_asin        = OclBuiltinImpl<native_asin, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_asin   = OclBuiltinImpl<native_asin, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_asin = OclBuiltinImpl<native_asin, [v16f64], 0, Expand1>;

// native_asinh
OclBuiltinImpl knc_native_asinh        = OclBuiltinImpl<native_asinh, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_asinh   = OclBuiltinImpl<native_asinh, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_asinh = OclBuiltinImpl<native_asinh, [v16f64], 0, Expand1>;

// native_asinpi
OclBuiltinImpl knc_native_asinpi        = OclBuiltinImpl<native_asinpi, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_asinpi   = OclBuiltinImpl<native_asinpi, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_asinpi = OclBuiltinImpl<native_asinpi, [v16f64], 0, Expand1>;

// native_atan
OclBuiltinImpl knc_native_atan        = OclBuiltinImpl<native_atan, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_atan   = OclBuiltinImpl<native_atan, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_atan = OclBuiltinImpl<native_atan, [v16f64], 0, Expand1>;

// native_atan2
OclBuiltinImpl knc_native_atan2        = OclBuiltinImpl<native_atan2, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_native_atan2   = OclBuiltinImpl<native_atan2, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_native_atan2 = OclBuiltinImpl<native_atan2, [v16f64], 0, Expand2>;

// native_atan2pi
OclBuiltinImpl knc_native_atan2pi        = OclBuiltinImpl<native_atan2pi, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_native_atan2pi   = OclBuiltinImpl<native_atan2pi, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_native_atan2pi = OclBuiltinImpl<native_atan2pi, [v16f64], 0, Expand2>;

// native_atanh
OclBuiltinImpl knc_native_atanh        = OclBuiltinImpl<native_atanh, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_atanh   = OclBuiltinImpl<native_atanh, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_atanh = OclBuiltinImpl<native_atanh, [v16f64], 0, Expand1>;

// native_atanpi
OclBuiltinImpl knc_native_atanpi        = OclBuiltinImpl<native_atanpi, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_atanpi   = OclBuiltinImpl<native_atanpi, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_atanpi = OclBuiltinImpl<native_atanpi, [v16f64], 0, Expand1>;

// native_cbrt
OclBuiltinImpl knc_native_cbrt        = OclBuiltinImpl<native_cbrt, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_cbrt   = OclBuiltinImpl<native_cbrt, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_cbrt = OclBuiltinImpl<native_cbrt, [v16f64], 0, Expand1>;

// native_cos
OclBuiltinImpl knc_native_cos        = OclBuiltinImpl<native_cos, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_cos   = OclBuiltinImpl<native_cos, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_cos = OclBuiltinImpl<native_cos, [v16f64], 0, Expand1>;

// native_cosh
OclBuiltinImpl knc_native_cosh        = OclBuiltinImpl<native_cosh, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_cosh   = OclBuiltinImpl<native_cosh, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_cosh = OclBuiltinImpl<native_cosh, [v16f64], 0, Expand1>;

// native_cospi
OclBuiltinImpl knc_native_cospi        = OclBuiltinImpl<native_cospi, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_cospi   = OclBuiltinImpl<native_cospi, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_cospi = OclBuiltinImpl<native_cospi, [v16f64], 0, Expand1>;

// native_divide
OclBuiltinImpl knc_native_divide        = OclBuiltinImpl<native_divide, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_native_divide   = OclBuiltinImpl<native_divide, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_native_divide = OclBuiltinImpl<native_divide, [v16f64], 0, Expand2>;

// native_erf
OclBuiltinImpl knc_native_erf        = OclBuiltinImpl<native_erf, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_erf   = OclBuiltinImpl<native_erf, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_erf = OclBuiltinImpl<native_erf, [v16f64], 0, Expand1>;

// native_erfc
OclBuiltinImpl knc_native_erfc        = OclBuiltinImpl<native_erfc, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_erfc   = OclBuiltinImpl<native_erfc, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_erfc = OclBuiltinImpl<native_erfc, [v16f64], 0, Expand1>;

// native_exp
OclBuiltinImpl knc_native_exp        = OclBuiltinImpl<native_exp, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_exp   = OclBuiltinImpl<native_exp, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_exp = OclBuiltinImpl<native_exp, [v16f64], 0, Expand1>;

// native_exp10
OclBuiltinImpl knc_native_exp10        = OclBuiltinImpl<native_exp10, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_exp10   = OclBuiltinImpl<native_exp10, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_exp10 = OclBuiltinImpl<native_exp10, [v16f64], 0, Expand1>;

// native_exp2
OclBuiltinImpl knc_native_exp2        = OclBuiltinImpl<native_exp2, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_exp2   = OclBuiltinImpl<native_exp2, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_exp2 = OclBuiltinImpl<native_exp2, [v16f64], 0, Expand1>;

// native_expm1
OclBuiltinImpl knc_native_expm1        = OclBuiltinImpl<native_expm1, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_expm1   = OclBuiltinImpl<native_expm1, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_expm1 = OclBuiltinImpl<native_expm1, [v16f64], 0, Expand1>;

// native_hypot
OclBuiltinImpl knc_native_hypot        = OclBuiltinImpl<native_hypot, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_native_hypot   = OclBuiltinImpl<native_hypot, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_native_hypot = OclBuiltinImpl<native_hypot, [v16f64], 0, Expand2>;

// native_log
OclBuiltinImpl knc_native_log        = OclBuiltinImpl<native_log, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_log   = OclBuiltinImpl<native_log, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_log = OclBuiltinImpl<native_log, [v16f64], 0, Expand1>;

// native_log10
OclBuiltinImpl knc_native_log10        = OclBuiltinImpl<native_log10, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_log10   = OclBuiltinImpl<native_log10, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_log10 = OclBuiltinImpl<native_log10, [v16f64], 0, Expand1>;

// native_log1p
OclBuiltinImpl knc_native_log1p        = OclBuiltinImpl<native_log1p, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_log1p   = OclBuiltinImpl<native_log1p, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_log1p = OclBuiltinImpl<native_log1p, [v16f64], 0, Expand1>;

// native_log2
OclBuiltinImpl knc_native_log2        = OclBuiltinImpl<native_log2, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_log2   = OclBuiltinImpl<native_log2, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_log2 = OclBuiltinImpl<native_log2, [v16f64], 0, Expand1>;

// native_pow
OclBuiltinImpl knc_native_pow        = OclBuiltinImpl<native_pow, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_native_pow   = OclBuiltinImpl<native_pow, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_native_pow = OclBuiltinImpl<native_pow, [v16f64], 0, Expand2>;

// native_pown
OclBuiltinImpl knc_native_pown        = OclBuiltinImpl<native_pown, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_native_pown   = OclBuiltinImpl<native_pown, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_native_pown = OclBuiltinImpl<native_pown, [v16f64], 0, Expand2>;

// native_powr
OclBuiltinImpl knc_native_powr        = OclBuiltinImpl<native_powr, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_native_powr   = OclBuiltinImpl<native_powr, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_native_powr = OclBuiltinImpl<native_powr, [v16f64], 0, Expand2>;

// native_rootn
OclBuiltinImpl knc_native_rootn        = OclBuiltinImpl<native_rootn, [v16f32, v8f64], 0, knc_code_2>;
OclBuiltinImpl knc_mask_native_rootn   = OclBuiltinImpl<native_rootn, mask_types, 0, knc_mask_code_2>;
OclBuiltinImpl knc_expand_native_rootn = OclBuiltinImpl<native_rootn, [v16f64], 0, Expand2>;

// native_rsqrt
OclBuiltinImpl knc_native_rsqrt        = OclBuiltinImpl<native_rsqrt, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_rsqrt   = OclBuiltinImpl<native_rsqrt, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_rsqrt = OclBuiltinImpl<native_rsqrt, [v16f64], 0, Expand1>;

// native_sin
OclBuiltinImpl knc_native_sin        = OclBuiltinImpl<native_sin, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_sin   = OclBuiltinImpl<native_sin, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_sin = OclBuiltinImpl<native_sin, [v16f64], 0, Expand1>;

// native_sinh
OclBuiltinImpl knc_native_sinh        = OclBuiltinImpl<native_sinh, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_sinh   = OclBuiltinImpl<native_sinh, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_sinh = OclBuiltinImpl<native_sinh, [v16f64], 0, Expand1>;

// native_sinpi
OclBuiltinImpl knc_native_sinpi        = OclBuiltinImpl<native_sinpi, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_sinpi   = OclBuiltinImpl<native_sinpi, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_sinpi = OclBuiltinImpl<native_sinpi, [v16f64], 0, Expand1>;

// native_sqrt
OclBuiltinImpl knc_native_sqrt        = OclBuiltinImpl<native_sqrt, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_sqrt   = OclBuiltinImpl<native_sqrt, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_sqrt = OclBuiltinImpl<native_sqrt, [v16f64], 0, Expand1>;

// native_tan
OclBuiltinImpl knc_native_tan        = OclBuiltinImpl<native_tan, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_tan   = OclBuiltinImpl<native_tan, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_tan = OclBuiltinImpl<native_tan, [v16f64], 0, Expand1>;

// native_tanh
OclBuiltinImpl knc_native_tanh        = OclBuiltinImpl<native_tanh, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_tanh   = OclBuiltinImpl<native_tanh, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_tanh = OclBuiltinImpl<native_tanh, [v16f64], 0, Expand1>;

// native_tanpi
OclBuiltinImpl knc_native_tanpi        = OclBuiltinImpl<native_tanpi, [v16f32, v8f64], 0, knc_code_1>;
OclBuiltinImpl knc_mask_native_tanpi   = OclBuiltinImpl<native_tanpi, mask_types, 0, knc_mask_code_1>;
OclBuiltinImpl knc_expand_native_tanpi = OclBuiltinImpl<native_tanpi, [v16f64], 0, Expand1>;

// native_recip
// WORKAROUND: CSSD100016998
//             Remove then PCG is capable to distinguish calls to native SVML functions
OclBuiltinImpl knc_native_recip        = OclBuiltinImpl<native_recip, native_recip.Types, 0, [{
    return native_divide(1, $Arg0VarName);
  }]>;
//OclBuiltinImpl knc_native_recip        = OclBuiltinImpl<native_recip, [v16f32, v8f64], 0, >;
//OclBuiltinImpl knc_mask_native_recip   = OclBuiltinImpl<native_recip, mask_types, 0, knc_mask_code_1>;
//OclBuiltinImpl knc_expand_native_recip = OclBuiltinImpl<native_recip, [v16f64], 0, Expand1>;

// native_rcbrt
OclBuiltinImpl workaround_mask_native_rcbrt = OclBuiltinImpl<native_rcbrt, native_rcbrt.Types, 0, [{
    return native_recip(native_cbrt($Arg0VarName));
  }]>;

// native_sincos
OclBuiltinImpl knc_native_sincos_pas   = OclBuiltinImpl<native_sincos_pas, [v16f32, v8f64], 0, knc_code_2p>;
OclBuiltinImpl knc_native_sincos_las   = OclBuiltinImpl<native_sincos_las, [v16f32, v8f64], 0, knc_code_2p>;
OclBuiltinImpl knc_native_sincos_gas   = OclBuiltinImpl<native_sincos_gas, [v16f32, v8f64], 0, knc_code_2p>;

OclBuiltinImpl knc_mask_native_sincos_pas   = OclBuiltinImpl<native_sincos_pas, mask_types, 0, sincos_mask_code>;
OclBuiltinImpl knc_mask_native_sincos_las   = OclBuiltinImpl<native_sincos_las, mask_types, 0, sincos_mask_code>;
OclBuiltinImpl knc_mask_native_sincos_gas   = OclBuiltinImpl<native_sincos_gas, mask_types, 0, sincos_mask_code>;

OclBuiltinImpl knc_expand_native_sincos_pas   = OclBuiltinImpl<native_sincos_pas, [v16f64], 0, knc_expand_code_2p>;
OclBuiltinImpl knc_expand_native_sincos_las   = OclBuiltinImpl<native_sincos_las, [v16f64], 0, knc_expand_code_2p>;
OclBuiltinImpl knc_expand_native_sincos_gas   = OclBuiltinImpl<native_sincos_gas, [v16f64], 0, knc_expand_code_2p>;
