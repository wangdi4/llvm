// vim:ts=2:sw=2:et:


// abs
OclBuiltinImpl abs_v16_i8i16 = OclBuiltinImpl<abs, [v16i8, v16i16], 0, [{
    return convert_$ReturnType(abs(convert_int16($Arg0VarName)));
  }]>;
OclBuiltinImpl abs_v16_i32 = OclBuiltinImpl<abs, [v16i32], 0, [{
    ushort m16 = _mm512_cmplt_epi32((__m512i)$Arg0VarName, (__m512i)(int16)0); // $Arg0VarName < 0
    __m512i r = _mm512_mask_mov_epi32((__m512i)$Arg0VarName, m16, (__m512i)-$Arg0VarName); // $Arg0VarName < 0 ? -$Arg0VarName : $Arg0VarName
    return as_$ReturnType(r);
  }]>;
OclBuiltinImpl abs_promote = OclBuiltinImpl<abs, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,/*  v16i8,*/
                                                  v1i16, v2i16, v3i16, v4i16, v8i16,/* v16i16,*/
                                                  v1i32, v2i32, v3i32, v4i32, v8i32,/* v16i32,*/
                                                  v1i64, v2i64, v3i64, v4i64/*, v8i64, v16i64*/], 0, NonMaskPromote1>;
OclBuiltinImpl abs_expand = OclBuiltinImpl<abs, [v16i64], 0, Expand1>;

// add_sat
OclBuiltinImpl add_sat_v16_i8 = OclBuiltinImpl<add_sat, [v16i8], 0, [{
    int16 t0 = convert_int16($Arg0VarName);
    int16 t1 = convert_int16($Arg1VarName);
    return convert_$ReturnType(clamp(add_sat(t0, t1), CHAR_MIN, CHAR_MAX));
  }]>;
OclBuiltinImpl add_sat_v16_u8 = OclBuiltinImpl<add_sat, [v16u8], 0, [{
    uint16 t0 = convert_uint16($Arg0VarName);
    uint16 t1 = convert_uint16($Arg1VarName);
    return convert_$ReturnType(clamp(add_sat(t0, t1), (uint)0, (uint)UCHAR_MAX));
  }]>;
OclBuiltinImpl add_sat_v16_i16 = OclBuiltinImpl<add_sat, [v16i16], 0, [{
    int16 t0 = convert_int16($Arg0VarName);
    int16 t1 = convert_int16($Arg1VarName);
    return convert_$ReturnType(clamp(add_sat(t0, t1), SHRT_MIN, SHRT_MAX));
  }]>;
OclBuiltinImpl add_sat_v16_u16 = OclBuiltinImpl<add_sat, [v16u16], 0, [{
    uint16 t0 = convert_uint16($Arg0VarName);
    uint16 t1 = convert_uint16($Arg1VarName);
    return convert_$ReturnType(clamp(add_sat(t0, t1), (uint)0, (uint)USHRT_MAX));
  }]>;
OclBuiltinImpl add_sat_v16_i32 = OclBuiltinImpl<add_sat, [v16i32], 0, [{
    ushort s16;
    __m512i r = _mm512_addsets_epi32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName, &s16);
    ushort p16 = _mm512_cmplt_epi32((__m512i)(int16)0, (__m512i)($Arg0VarName | $Arg1VarName)) & s16; // both $Arg0VarName and $Arg1VarName > 0 but sign bit is set
    ushort n16 = _mm512_cmplt_epi32((__m512i)($Arg0VarName & $Arg1VarName), (__m512i)(int16)0) & (~s16); // both $Arg0VarName and $Arg1VarName < 0 but sign bit is not set
    r = _mm512_mask_mov_epi32(_mm512_mask_mov_epi32(r, p16, (__m512i)(int16)INT_MAX), n16, (__m512i)(int16)INT_MIN);
    return as_$ReturnType(r);
  }]>;
OclBuiltinImpl add_sat_v16_u32 = OclBuiltinImpl<add_sat, [v16u32], 0, [{
    ushort c16;
    __m512i r = _mm512_addsetc_epi32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName, &c16);
    r = _mm512_mask_mov_epi32(r, c16, (__m512i)(uint16)UINT_MAX);
    return as_$ReturnType(r);
  }]>;
OclBuiltinImpl add_sat_v8_i64 = OclBuiltinImpl<add_sat, [v8i64], 0, [{
    long8 r = $Arg0VarName + $Arg1VarName;
    long8 p8 = (($Arg0VarName | $Arg1VarName) > 0) & (r < 0);
    long8 n8 = (($Arg0VarName & $Arg1VarName) < 0) & (r > 0);
    return  (n8 == 0) ?
                (p8 == 0) ? r : (long8)LONG_MAX :
                (long8)LONG_MIN;
  }]>;
OclBuiltinImpl add_sat_v8_u64 = OclBuiltinImpl<add_sat, [v8u64], 0, [{
    __m512i x_lo = (__m512i)_mm512_shuffle((__m512)$Arg0VarName, _MM_PERM_CCAA);
    __m512i y_lo = (__m512i)_mm512_shuffle((__m512)$Arg1VarName, _MM_PERM_CCAA);
    __m512i x_hi = (__m512i)_mm512_shuffle((__m512)$Arg0VarName, _MM_PERM_DDBB);
    __m512i y_hi = (__m512i)_mm512_shuffle((__m512)$Arg1VarName, _MM_PERM_DDBB);

    ushort c_lo = 0, c_hi = 0;
    __m512i sum_lo = _mm512_addsetc_epi32(x_lo, y_lo, &c_lo);
    __m512i sum_hi = _mm512_adc_epi32(x_hi, c_lo, y_hi, &c_hi); 
    __m512i sum    = _mm512_mask_mov_epi32(sum_lo, 0xAAAA, sum_hi);
    sum = _mm512_mask_mov_epi32(sum, c_hi, (__m512i)(uint16)UINT_MAX);
    return as_$ReturnType(sum);
  }]>;
OclBuiltinImpl add_sat_promote = OclBuiltinImpl<add_sat, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8/*,  v16i8*/,
                                                           v1u8,  v2u8,  v3u8,  v4u8,  v8u8/*,  v16u8*/,
                                                          v1i16, v2i16, v3i16, v4i16, v8i16/*, v16i16*/,
                                                          v1u16, v2u16, v3u16, v4u16, v8u16/*, v16u16*/,
                                                          v1i32, v2i32, v3i32, v4i32, v8i32/*, v16i32*/,
                                                          v1u32, v2u32, v3u32, v4u32, v8u32/*, v16u32*/,
                                                          v1i64, v2i64, v3i64, v4i64/*, v8i64, v16i64*/,
                                                          v1u64, v2u64, v3u64, v4u64/*, v8u64, v16u64*/], 0, [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName);
    return $ReturnVarName$Pattern;
  }]>;
OclBuiltinImpl add_sat_expand = OclBuiltinImpl<add_sat, [v16i64, v16u64], 0, Expand2>;


// clz
OclBuiltinImpl clz_v16_u8u16 = OclBuiltinImpl<clz, [v16u8, v16u16], 0, [{
    return convert_$ReturnType(clz(convert_uint16($Arg0VarName)) - (32 - $BitWidth));
  }]>;
OclBuiltinImpl clz_v16_u32 = OclBuiltinImpl<clz, [v16u32], 0, [{
    __m512i t = _mm512_andnot_epi32((__m512i)($Arg0VarName >> 1), (__m512i)$Arg0VarName);
    __m512 f = _mm512_cvtfxpnt_round_adjustepu32_ps(t, _MM_FROUND_CUR_DIRECTION, _MM_EXPADJ_NONE);
    uint16 r = 158 - (as_uint16(f) >> 23);
    return as_$ReturnType(_mm512_min_epi32((__m512i)r, (__m512i)(uint16)32));
  }]>;
OclBuiltinImpl clz_v8_u64 = OclBuiltinImpl<clz, [v8u64], 0, [{
    __m512i lz = (__m512i)clz(as_uint16($Arg0VarName));
    ushort m16 = _mm512_mask_cmplt_epi32(0xAAAA, lz, (__m512i)(uint16)32);
    __m512i r = _mm512_mask_add_epi32((__m512i)(uint16)0, 0x5555, lz, _mm512_swizzle_epi32(lz, _MM_SWIZ_REG_CDAB));
    r = _mm512_mask_mov_epi32(r, m16 >> 1, _mm512_swizzle_epi32(lz, _MM_SWIZ_REG_CDAB));
    return as_$ReturnType(r);
  }]>;
OclBuiltinImpl clz_promote = OclBuiltinImpl<clz, [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8/*,  v16u8*/,
                                                  v1u16, v2u16, v3u16, v4u16, v8u16/*, v16u16*/,
                                                  v1u32, v2u32, v3u32, v4u32, v8u32/*, v16u32*/,
                                                  v1u64, v2u64, v3u64, v4u64/*, v8u64, v16u64*/], 0, NonMaskPromote1>;
OclBuiltinImpl clz_expand = OclBuiltinImpl<clz, [v16u64], 0, Expand1>;

// mad_sat
OclBuiltinImpl mad_sat_v16_i8 = OclBuiltinImpl<mad_sat, [v16i8], 0, [{
    int16 t0 = convert_int16($Arg0VarName);
    int16 t1 = convert_int16($Arg1VarName);
    int16 t2 = convert_int16($Arg2VarName);
    return convert_$ReturnType(clamp(mad_sat(t0, t1, t2), CHAR_MIN, CHAR_MAX));
  }]>;
OclBuiltinImpl mad_sat_v16_u8 = OclBuiltinImpl<mad_sat, [v16u8], 0, [{
    uint16 t0 = convert_uint16($Arg0VarName);
    uint16 t1 = convert_uint16($Arg1VarName);
    uint16 t2 = convert_uint16($Arg2VarName);
    return convert_$ReturnType(clamp(mad_sat(t0, t1, t2), (uint)0, (uint)UCHAR_MAX));
  }]>;
OclBuiltinImpl mad_sat_v16_i16 = OclBuiltinImpl<mad_sat, [v16i16], 0, [{
    int16 t0 = convert_int16($Arg0VarName);
    int16 t1 = convert_int16($Arg1VarName);
    int16 t2 = convert_int16($Arg2VarName);
    return convert_$ReturnType(clamp(mad_sat(t0, t1, t2), SHRT_MIN, SHRT_MAX));
  }]>;
OclBuiltinImpl mad_sat_v16_u16 = OclBuiltinImpl<mad_sat, [v16u16], 0, [{
    uint16 t0 = convert_uint16($Arg0VarName);
    uint16 t1 = convert_uint16($Arg1VarName);
    uint16 t2 = convert_uint16($Arg2VarName);
    return convert_$ReturnType(clamp(mad_sat(t0, t1, t2), (uint)0, (uint)USHRT_MAX));
  }]>;
OclBuiltinImpl mad_sat_v16_i32 = OclBuiltinImpl<mad_sat, [v16i32], 0, [{
    __m512i pl = _mm512_mullo_epi32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName);
    __m512i ph = _mm512_mulhi_epi32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName);
    ushort c16;
    __m512i s = _mm512_addsetc_epi32((__m512i)$Arg2VarName, pl, &c16);
    ph = _mm512_adc_epi32(ph, c16, (__m512i)(int16)0, &c16);
    ushort p16 = _mm512_cmplt_epi32((__m512i)(int16)1, ph) |
                 (_mm512_cmpeq_epi32(ph, (__m512i)(int16)1) & _mm512_cmplt_epi32(s, (__m512i)(int16)0));
    ushort n16 = _mm512_cmplt_epi32(ph, (__m512i)(int16)-1) |
                 (_mm512_cmpeq_epi32(ph, (__m512i)(int16)-1) & _mm512_cmplt_epi32((__m512i)(int16)0, s));
    __m512i r = _mm512_mask_mov_epi32(_mm512_mask_mov_epi32(s, p16, (__m512i)(int16)INT_MAX), n16, (__m512i)(int16)INT_MIN);
    return as_$ReturnType(r);
  }]>;
OclBuiltinImpl mad_sat_v16_u32 = OclBuiltinImpl<mad_sat, [v16u32], 0, [{
    __m512i pl = _mm512_mullo_epi32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName);
    __m512i ph = _mm512_mulhi_epu32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName);
    ushort c16;
    __m512i s = _mm512_addsetc_epi32((__m512i)$Arg2VarName, pl, &c16);
    ph = _mm512_adc_epi32(ph, c16, (__m512i)(uint16)0, &c16);
    c16 = _mm512_cmplt_pu((__m512i)(uint16)0, ph);
    __m512i r = _mm512_mask_mov_epi32(s, c16, (__m512i)(uint16)UINT_MAX);
    return as_$ReturnType(r);
  }]>;
OclBuiltinImpl mad_sat_v8_i64 = OclBuiltinImpl<mad_sat, [v8i64], 0, [{
    long8 pl = _mm512_mullo_epi64($Arg0VarName, $Arg1VarName);
    long8 ph = mul_hi($Arg0VarName, $Arg1VarName);
    ushort c16 = 0;
    long8 s = _mm512_mask_addsetc_epi32($Arg2VarName, 0x5555, c16, pl, &c16);
    s = _mm512_mask_adc_epi32(s, 0xAAAA, c16 << 1, pl, &c16);
    __m512i r = _mm512_mask_adc_epi32((__m512i)ph, 0x5555, c16, (__m512i)(long8)0, &c16);
    ph = _mm512_mask_adc_epi32(r, 0xAAAA, c16 << 1, (__m512i)(long8)0, &c16);
    uchar p8 = _mm512_cmplt_epi64((__m512i)(long8)1, (__m512i)ph) |
               (_mm512_cmpeq_epi64((__m512i)ph, (__m512i)(long8)1) & _mm512_cmplt_epi64((__m512i)s, (__m512i)(long8)0));
    uchar n8 = _mm512_cmplt_epi64((__m512i)ph, (__m512i)(long8)-1) |
               (_mm512_cmpeq_epi64((__m512i)ph, (__m512i)(long8)-1) & _mm512_cmplt_epi64((__m512i)(long8)0, (__m512i)s));
    return _mm512_mask_mov_epi64(_mm512_mask_mov_epi64((__m512i)s, p8, (__m512i)(long8)LONG_MAX), n8, (__m512i)(long8)LONG_MIN);
  }]>;
OclBuiltinImpl mad_sat_v8_u64 = OclBuiltinImpl<mad_sat, [v8u64], 0, [{
    __m512i zero_v = (__m512i)(uint16)0;

    // X*Y = (Xh+Xl)*(Yh+Yl) = Xh*Yh + Xl*Yl + Xh*Yl + Xl*Yh 

    // fill the saturation mask
    // if the both hi dwords are not equal to zero simulatneously
    __m512i Xh = (__m512i)_mm512_shuffle((__m512)$Arg0VarName, _MM_PERM_DDBB);
    __m512i Yh = (__m512i)_mm512_shuffle((__m512)$Arg1VarName, _MM_PERM_DDBB);
    ushort sat_mask = _mm512_cmpneq_epi32(zero_v, Xh);
    sat_mask = _mm512_mask_cmpneq_epi32(sat_mask, zero_v, Yh);
    // if mulhi of Xh*Yl or Xl*Yh is not equal to zero
    __m512 mulhi_XhYl_XlYh = (__m512)_mm512_mulhi_epu32((__m512i)$Arg0VarName, _mm512_swizzle_epi32((__m512i)$Arg1VarName, _MM_SWIZ_REG_CDAB));
    sat_mask |= _mm512_cmpneq_epi32(zero_v, (__m512i)_mm512_shuffle(mulhi_XhYl_XlYh, _MM_PERM_CCAA));
    sat_mask |= _mm512_cmpneq_epi32(zero_v, (__m512i)_mm512_shuffle(mulhi_XhYl_XlYh, _MM_PERM_DDBB));

    // calculate the result
    __m512 mullo_XhYl_XlYh = (__m512)_mm512_mullo_epi32((__m512i)$Arg0VarName, _mm512_swizzle_epi32((__m512i)$Arg1VarName, _MM_SWIZ_REG_CDAB));
    // mullo_XhYl_XlYh = (Xh*Yl, Xl*Yh, ...)
    __m512i mul = _mm512_mask_addsetc_epi32((__m512i)_mm512_shuffle(mullo_XhYl_XlYh, _MM_PERM_CCAA), ~sat_mask, sat_mask, (__m512i)_mm512_shuffle(mullo_XhYl_XlYh, _MM_PERM_DDBB), &sat_mask); 
    __m512i Xl = (__m512i)_mm512_shuffle((__m512)$Arg0VarName, _MM_PERM_CCAA);
    __m512i Yl = (__m512i)_mm512_shuffle((__m512)$Arg1VarName, _MM_PERM_CCAA);
    __m512i mullo_XlYl = _mm512_mullo_epi32(Xl, Yl);
    __m512i mulhi_XlYl = _mm512_mulhi_epu32(Xl, Yl);
    mul = _mm512_mask_addsetc_epi32(mul, ~sat_mask, sat_mask, mulhi_XlYl, &sat_mask); 
    mul = _mm512_mask_mov_epi32(mul, 0x5555, mullo_XlYl); 
    mul = _mm512_mask_mov_epi32(mul, sat_mask, (__m512i)(uint16)UINT_MAX); 
    
    return add_sat(as_ulong8(mul), $Arg2VarName);
  }]>;
OclBuiltinImpl mad_sat_promote = OclBuiltinImpl<mad_sat, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8/*,  v16i8*/,
                                                               v1u8,  v2u8,  v3u8,  v4u8,  v8u8/*,  v16u8*/,
                                                              v1i16, v2i16, v3i16, v4i16, v8i16/*, v16i16*/,
                                                              v1u16, v2u16, v3u16, v4u16, v8u16/*, v16u16*/,
                                                              v1i32, v2i32, v3i32, v4i32, v8i32/*, v16i32*/,
                                                              v1u32, v2u32, v3u32, v4u32, v8u32/*, v16u32*/,
                                                              v1i64, v2i64, v3i64, v4i64/*, v8i64, v16i64*/,
                                                              v1u64, v2u64, v3u64, v4u64/*, v8u64, v16u64*/], 0, NonMaskPromote3>;
OclBuiltinImpl mad_sat_expand = OclBuiltinImpl<mad_sat, [v16i64, v16u64], 0, Expand3>;

// max
OclBuiltinImpl max_v16_i8i16 = OclBuiltinImpl<max, [v16i8, v16i16], 0, [{
    int16 t0 = convert_int16($Arg0VarName);
    int16 t1 = convert_int16($Arg1VarName);
    return convert_$ReturnType(max(t0, t1));
  }]>;
OclBuiltinImpl max_v16_u8u16 = OclBuiltinImpl<max, [v16u8, v16u16], 0, [{
    uint16 t0 = convert_uint16($Arg0VarName);
    uint16 t1 = convert_uint16($Arg1VarName);
    return convert_$ReturnType(max(t0, t1));
  }]>;
OclBuiltinImpl max_v16_i32 = OclBuiltinImpl<max, [v16i32], 0, [{
    return as_$ReturnType(_mm512_max_epi32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
  }]>;
OclBuiltinImpl max_v16_u32 = OclBuiltinImpl<max, [v16u32], 0, [{
    return as_$ReturnType(_mm512_max_epu32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
  }]>;
OclBuiltinImpl max_v8_i64 = OclBuiltinImpl<max, [v8i64], 0, [{
    return as_$ReturnType(_mm512_max_epi64((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
  }]>;
OclBuiltinImpl max_v8_u64 = OclBuiltinImpl<max, [v8u64], 0, [{
    return as_$ReturnType(_mm512_max_epu64((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
  }]>;
OclBuiltinImpl max_ipromote = OclBuiltinImpl<max, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8/*,  v16i8*/,
                                                    v1u8,  v2u8,  v3u8,  v4u8,  v8u8/*,  v16u8*/,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16/*, v16i16*/,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16/*, v16u16*/,
                                                   v1i32, v2i32, v3i32, v4i32, v8i32/*, v16i32*/,
                                                   v1u32, v2u32, v3u32, v4u32, v8u32/*, v16u32*/,
                                                   v1i64, v2i64, v3i64, v4i64/*, v8i64, v16i64*/,
                                                   v1u64, v2u64, v3u64, v4u64/*, v8u64, v16u64*/], 0, NonMaskPromote2>;
OclBuiltinImpl max_iexpand = OclBuiltinImpl<max, [v16i64, v16u64], 0, Expand2>;

// min
OclBuiltinImpl min_v16_i8i16 = OclBuiltinImpl<min, [v16i8, v16i16], 0, [{
    int16 t0 = convert_int16($Arg0VarName);
    int16 t1 = convert_int16($Arg1VarName);
    return convert_$ReturnType(min(t0, t1));
  }]>;
OclBuiltinImpl min_v16_u8u16 = OclBuiltinImpl<min, [v16u8, v16u16], 0, [{
    uint16 t0 = convert_uint16($Arg0VarName);
    uint16 t1 = convert_uint16($Arg1VarName);
    return convert_$ReturnType(min(t0, t1));
  }]>;
OclBuiltinImpl min_v16_i32 = OclBuiltinImpl<min, [v16i32], 0, [{
    return as_$ReturnType(_mm512_min_epi32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
  }]>;
OclBuiltinImpl min_v16_u32 = OclBuiltinImpl<min, [v16u32], 0, [{
    return as_$ReturnType(_mm512_min_epu32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
  }]>;
OclBuiltinImpl min_v8_i64 = OclBuiltinImpl<min, [v8i64], 0, [{
    return as_$ReturnType(_mm512_min_epi64((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
  }]>;
OclBuiltinImpl min_v8_u64 = OclBuiltinImpl<min, [v8u64], 0, [{
    return as_$ReturnType(_mm512_min_epu64((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
  }]>;
OclBuiltinImpl min_ipromote = OclBuiltinImpl<min, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8/*,  v16i8*/,
                                                    v1u8,  v2u8,  v3u8,  v4u8,  v8u8/*,  v16u8*/,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16/*, v16i16*/,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16/*, v16u16*/,
                                                   v1i32, v2i32, v3i32, v4i32, v8i32/*, v16i32*/,
                                                   v1u32, v2u32, v3u32, v4u32, v8u32/*, v16u32*/,
                                                   v1i64, v2i64, v3i64, v4i64/*, v8i64, v16i64*/,
                                                   v1u64, v2u64, v3u64, v4u64/*, v8u64, v16u64*/], 0, NonMaskPromote2>;
OclBuiltinImpl min_iexpand = OclBuiltinImpl<min, [v16i64, v16u64], 0, Expand2>;

// mul_hi
OclBuiltinImpl mul_hi_v16_i8i16 = OclBuiltinImpl<mul_hi, [v16i8, v16i16], 0, [{
    int16 t0 = convert_int16($Arg0VarName);
    int16 t1 = convert_int16($Arg1VarName);
    return convert_$ReturnType(as_int16(_mm512_sra_epi32(_mm512_mullo_epi32((__m512i)t0, (__m512i)t1), (__m512i)(int16)$BitWidth)));
  }]>;
OclBuiltinImpl mul_hi_v16_u8u16 = OclBuiltinImpl<mul_hi, [v16u8, v16u16], 0, [{
    uint16 t0 = convert_uint16($Arg0VarName);
    uint16 t1 = convert_uint16($Arg1VarName);
    return convert_$ReturnType(as_uint16(_mm512_srl_epi32(_mm512_mullo_epi32((__m512i)t0, (__m512i)t1), (__m512i)(int16)$BitWidth)));
  }]>;
OclBuiltinImpl mul_hi_v16_i32 = OclBuiltinImpl<mul_hi, [v16i32], 0, [{
    return as_$ReturnType(_mm512_mulhi_epi32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
  }]>;
OclBuiltinImpl mul_hi_v16_u32 = OclBuiltinImpl<mul_hi, [v16u32], 0, [{
    return as_$ReturnType(_mm512_mulhi_epu32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
  }]>;
OclBuiltinImpl mul_hi_v8_i64 = OclBuiltinImpl<mul_hi, [v8i64], 0, [{
    ulong8 ures = mul_hi(as_ulong8($Arg0VarName), as_ulong8($Arg1VarName));
    __mmask8 x_sign = 0, y_sign = 0;
    x_sign = _mm512_cmplt_epi64((__m512i)$Arg0VarName, (__m512i)(long8)0);
    long8 res = _mm512_mask_sub_epi64((__m512i)ures, x_sign, (__m512i)ures, (__m512i)$Arg1VarName);
    y_sign = _mm512_cmplt_epi64((__m512i)$Arg1VarName, (__m512i)(long8)0);
    res = _mm512_mask_sub_epi64((__m512i)res, y_sign, (__m512i)res, (__m512i)$Arg0VarName);
    return as_$ReturnType(res);
  }]>;
OclBuiltinImpl mul_hi_v8_u64 = OclBuiltinImpl<mul_hi, [v8u64], 0, [{
    // x = x_h, x_l; y = y_h, y_l
    // p0_h = H(x_l * y_l); p0_l = L(x_l * y_l)
    // p3_h = H(x_h * y_h); p3_l = L(x_h * y_h)
    // p1_h = H(x_h * y_l); p1_l = L(x_h * y_l)
    // p2_h = H(x_l * y_h); p2_l = L(x_l * y_h)
    //
    //             p0_h, p0_l
    //       p1_h, p1_l
    //       p2_h, p2_l
    // p3_h, p3_l
    // ----------------------
    // hi = ((p0_h + p1_l + p2_l) + ((p1_h + p2_h + p3_l) << 32) + (p3_h << 64)) >> 32
    __m512i p30l = _mm512_mullo_epi32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName);
    __m512i p30h = _mm512_mulhi_epu32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName);
    __m512i p21l = _mm512_mullo_epi32((__m512i)$Arg0VarName, _mm512_swizzle_epi32((__m512i)$Arg1VarName, _MM_SWIZ_REG_CDAB));
    __m512i p21h = _mm512_mulhi_epu32((__m512i)$Arg0VarName, _mm512_swizzle_epi32((__m512i)$Arg1VarName, _MM_SWIZ_REG_CDAB));
    ushort c0, c1;
    __m512i ah, al;
    c0 = c1 = 0;
    // = p1_l + p0_h
    al = _mm512_mask_addsetc_epi32(p30h, 0x5555, c0, p21l, &c0);
    // = p2_l + a0
    ah = _mm512_mask_addsetc_epi32(al, 0x5555, c1, _mm512_swizzle_epi32(p21l, _MM_SWIZ_REG_CDAB), &c1);
    // al = p2_h + p3_l + c0
    al = _mm512_mask_adc_epi32(p30l, 0xAAAA, c0 << 1, p21h, &c0);
    // al = p1_h + al + c1
    al = _mm512_mask_adc_epi32(al, 0xAAAA, c1 << 1, _mm512_swizzle_epi32(p21h, _MM_SWIZ_REG_CDAB), &c1);
    // ah = p3_h + c0 + c1
    ah = _mm512_mask_adc_epi32(p30h, 0xAAAA, c0, (__m512i)(uint16)0, &c0);
    ah = _mm512_mask_adc_epi32(ah, 0xAAAA, c1, (__m512i)(uint16)0, &c1);
    // (ah, al)
    return as_$ReturnType(_mm512_mask_mov_epi32(ah, 0x5555, _mm512_swizzle_epi32(al, _MM_SWIZ_REG_CDAB)));
  }]>;
OclBuiltinImpl mul_hi_promote = OclBuiltinImpl<mul_hi, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8/*,  v16i8*/,
                                                         v1u8,  v2u8,  v3u8,  v4u8,  v8u8/*,  v16u8*/,
                                                        v1i16, v2i16, v3i16, v4i16, v8i16/*, v16i16*/,
                                                        v1u16, v2u16, v3u16, v4u16, v8u16/*, v16u16*/,
                                                        v1i32, v2i32, v3i32, v4i32, v8i32/*, v16i32*/,
                                                        v1u32, v2u32, v3u32, v4u32, v8u32/*, v16u32*/,
                                                        v1i64, v2i64, v3i64, v4i64/*, v8i64, v16i64*/,
                                                        v1u64, v2u64, v3u64, v4u64/*, v8u64, v16u64*/], 0, NonMaskPromote2>;
OclBuiltinImpl mul_hi_expand = OclBuiltinImpl<mul_hi, [v16i64, v16u64], 0, Expand2>;

// rotate
OclBuiltinImpl rotate_v16_u8u16 = OclBuiltinImpl<rotate, [v16u8, v16u16], 0, [{
    uint16 t0 = convert_uint16($Arg0VarName);
    uint16 t1 = convert_uint16($Arg1VarName);
    t1 = as_uint16(_mm512_and_epi32((__m512i)t1, (__m512i)(uint16)($BitWidth - 1)));
    uint16 r = as_uint16(_mm512_sll_epi32((__m512i)t0, (__m512i)t1)) |
               as_uint16(_mm512_srl_epi32((__m512i)t0, (__m512i)((uint16)$BitWidth - t1)));
    return convert_$ReturnType(r);
  }]>;
OclBuiltinImpl rotate_v16_u32 = OclBuiltinImpl<rotate, [v16u32], 0, [{
    __m512i t1 = _mm512_and_epi32((__m512i)$Arg1VarName, (__m512i)(uint16)($BitWidth - 1));
    return as_$ReturnType(_mm512_sll_epi32((__m512i)$Arg0VarName, t1)) |
           as_$ReturnType(_mm512_srl_epi32((__m512i)$Arg0VarName, (__m512i)((uint16)32 - as_uint16(t1))));
  }]>;
OclBuiltinImpl rotate_v8_u64 = OclBuiltinImpl<rotate, [v8u64], 0, [{
    __m512i t1 = _mm512_and_epi64((__m512i)$Arg1VarName, (__m512i)(ulong8)($BitWidth - 1));
    return as_$ReturnType(_mm512_sll_epi64((__m512i)$Arg0VarName, t1)) |
           as_$ReturnType(_mm512_srl_epi64((__m512i)$Arg0VarName, (__m512i)((ulong8)64 - as_ulong8(t1))));
  }]>;
OclBuiltinImpl rotate_promote = OclBuiltinImpl<rotate, [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,/*  v16u8,*/
                                                        v1u16, v2u16, v3u16, v4u16, v8u16,/* v16u16,*/
                                                        v1u32, v2u32, v3u32, v4u32, v8u32,/* v16u32,*/
                                                        v1u64, v2u64, v3u64, v4u64/*, v8u64, v16u64*/], 0, NonMaskPromote2>;
OclBuiltinImpl rotate_expand = OclBuiltinImpl<rotate, [v16u64], 0, Expand2>;

// upsample
OclBuiltinImpl upsample_v16_u8u16 = OclBuiltinImpl<upsample, [v16u8, v16u16], 0, [{
    uint16 t0 = convert_uint16($Arg0VarName);
    uint16 t1 = convert_uint16($Arg1VarName);
    uint16 r = as_uint16(_mm512_or_epi32(_mm512_sll_epi32((__m512i)t0, (__m512i)(int16)$BitWidth), (__m512i)t1));
    return convert_$ReturnType(r);
  }]>;
// upsample for v8u32
OclBuiltinImpl upsample_v8_u32 = OclBuiltinImpl<upsample, [v8u32], 0, [{
    uint16 val;
    val.s02468ace = $Arg1VarName;
    val.s13579bdf = $Arg0VarName;
    return as_$ReturnType(val);
  }]>;
OclBuiltinImpl upsample_promote_u8u16 = OclBuiltinImpl<upsample, [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8/*,  v16u8*/,
                                                                  v1u16, v2u16, v3u16, v4u16, v8u16/*, v16u16*/], 0, NonMaskPromote2>;
OclBuiltinImpl upsample_promote_u32   = OclBuiltinImpl<upsample, [v1u32, v2u32, v3u32, v4u32/*, v8u32, v16u32*/], 0, [{
    uint8 n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    uint8 n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    ulong8 $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName);
    return $ReturnVarName$Pattern;
  }]>;
OclBuiltinImpl upsample_expand = OclBuiltinImpl<upsample, [v16u32], 0, Expand2>;

// sub_sat
OclBuiltinImpl sub_sat_v16_i8 = OclBuiltinImpl<sub_sat, [v16i8], 0, [{
    int16 t0 = convert_int16($Arg0VarName);
    int16 t1 = convert_int16($Arg1VarName);
    return convert_$ReturnType(clamp(sub_sat(t0, t1), CHAR_MIN, CHAR_MAX));
  }]>;
OclBuiltinImpl sub_sat_v16_i16 = OclBuiltinImpl<sub_sat, [v16i16], 0, [{
    int16 t0 = convert_int16($Arg0VarName);
    int16 t1 = convert_int16($Arg1VarName);
    return convert_$ReturnType(clamp(sub_sat(t0, t1), SHRT_MIN, SHRT_MAX));
  }]>;
OclBuiltinImpl sub_sat_v16_i32 = OclBuiltinImpl<sub_sat, [v16i32], 0, [{
    $Arg0Type r = $Arg0VarName - $Arg1VarName;
    $Arg0Type p8 = ($Arg1VarName < 0) & (r < $Arg0VarName);
    $Arg0Type n8 = ($Arg1VarName > 0) & (r > $Arg0VarName);

    return  (n8 == 0) ?
                (p8 == 0) ? r : (int16)INT_MAX :
                (int16)INT_MIN;
  }]>;
OclBuiltinImpl sub_sat_v16_i64 = OclBuiltinImpl<sub_sat, [v8i64], 0, [{
    $Arg0Type r = $Arg0VarName - $Arg1VarName;
    $Arg0Type p8 = ($Arg1VarName < 0) & (r < $Arg0VarName);
    $Arg0Type n8 = ($Arg1VarName > 0) & (r > $Arg0VarName);

    return  (n8 == 0) ?
                (p8 == 0) ? r : (long8)LONG_MAX :
                (long8)LONG_MIN;
  }]>;
OclBuiltinImpl sub_sat_v16_u8 = OclBuiltinImpl<sub_sat, [v16u8], 0, [{
    uint16 t0 = convert_uint16($Arg0VarName);
    uint16 t1 = convert_uint16($Arg1VarName);
    return convert_$ReturnType(clamp(sub_sat(t0, t1), (uint)0, (uint)UCHAR_MAX));
  }]>;
OclBuiltinImpl sub_sat_v16_u16 = OclBuiltinImpl<sub_sat, [v16u16], 0, [{
    uint16 t0 = convert_uint16($Arg0VarName);
    uint16 t1 = convert_uint16($Arg1VarName);
    return convert_$ReturnType(clamp(sub_sat(t0, t1), (uint)0, (uint)USHRT_MAX));
  }]>;
OclBuiltinImpl sub_sat_v16_u32 = OclBuiltinImpl<sub_sat, [v16u32, v8u64], 0, [{
    $Arg0Type r = $Arg0VarName - $Arg1VarName;
    return ($Arg0VarName < r) ? 0 : r;
  }]>;
OclBuiltinImpl sub_sat_promote = OclBuiltinImpl<sub_sat, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8/*,  v16i8*/,
                                                           v1u8,  v2u8,  v3u8,  v4u8,  v8u8/*,  v16u8*/,
                                                          v1i16, v2i16, v3i16, v4i16, v8i16/*, v16i16*/,
                                                          v1u16, v2u16, v3u16, v4u16, v8u16/*, v16u16*/,
                                                          v1i32, v2i32, v3i32, v4i32, v8i32/*, v16i32*/,
                                                          v1u32, v2u32, v3u32, v4u32, v8u32/*, v16u32*/,
                                                          v1i64, v2i64, v3i64, v4i64/*, v8i64, v16i64*/,
                                                          v1u64, v2u64, v3u64, v4u64/*, v8u64, v16u64*/], 0, [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName);
    return $ReturnVarName$Pattern;
  }]>;
OclBuiltinImpl sub_sat_expand = OclBuiltinImpl<sub_sat, [v16i64, v16u64], 0, Expand2>;

// popcount
OclBuiltinImpl mask_popcount_v16_u8u16 = OclBuiltinImpl<mask_popcount, [v16u8, v16u16], 0, [{
    return convert_$ReturnType(mask_popcount($Arg0VarName, convert_uint16($Arg1VarName)));
  }]>;
OclBuiltinImpl mask_popcount_v16_u32 = OclBuiltinImpl<mask_popcount, [v16u32], 0, [{
    uint t[16] __attribute__((aligned(64)));

    // The following 'switch' will be removed during incluing this function
    // with constant mask. As a result, the final builtin on narrow vector
    // won't have branches.
    _mm512_mask_extstore_epi32(&t[0], $Arg0VarName, (__m512i)$Arg1VarName, _MM_DOWNCONV_EPI32_NONE, _MM_HINT_NONE);
    switch ($Arg0VarName) {
        case 0xFFFF:
            t[15] = _mm_countbits_32(t[15]);
            t[14] = _mm_countbits_32(t[14]);
            t[13] = _mm_countbits_32(t[13]);
            t[12] = _mm_countbits_32(t[12]);
            t[11] = _mm_countbits_32(t[11]);
            t[10] = _mm_countbits_32(t[10]);
            t[ 9] = _mm_countbits_32(t[ 9]);
            t[ 8] = _mm_countbits_32(t[ 8]);
        case 0x00FF:
            t[ 7] = _mm_countbits_32(t[ 7]);
            t[ 6] = _mm_countbits_32(t[ 6]);
            t[ 5] = _mm_countbits_32(t[ 5]);
            t[ 4] = _mm_countbits_32(t[ 4]);
        case 0x000F:
            t[ 3] = _mm_countbits_32(t[ 3]);
        case 0x0007:
            t[ 2] = _mm_countbits_32(t[ 2]);
        case 0x0003:
            t[ 1] = _mm_countbits_32(t[ 1]);
        case 0x0001:
            t[ 0] = _mm_countbits_32(t[ 0]);
        default:
            break;
    }
    return as_$ReturnType(_mm512_extload_epi32(&t[0], _MM_UPCONV_EPI32_NONE, _MM_BROADCAST32_NONE, _MM_HINT_NONE));
  }]>;
OclBuiltinImpl mask_popcount_v8_u64 = OclBuiltinImpl<mask_popcount, [v8u64], 0, [{
    ulong t[8] __attribute__((aligned(64)));

    // The following 'switch' will be removed during incluing this function
    // with constant mask. As a result, the final builtin on narrow vector
    // won't have branches.
    _mm512_mask_extstore_epi64(&t[0], $Arg0VarName, (__m512i)$Arg1VarName, _MM_DOWNCONV_EPI64_NONE, _MM_HINT_NONE);
    switch ($Arg0VarName) {
        case 0xFF:
            t[7] = _mm_countbits_64(t[7]);
            t[6] = _mm_countbits_64(t[6]);
            t[5] = _mm_countbits_64(t[5]);
            t[4] = _mm_countbits_64(t[4]);
        case 0x0F:
            t[3] = _mm_countbits_64(t[3]);
        case 0x07:
            t[2] = _mm_countbits_64(t[2]);
        case 0x03:
            t[1] = _mm_countbits_64(t[1]);
        case 0x01:
            t[0] = _mm_countbits_64(t[0]);
        default:
            break;
    }
    return as_$ReturnType(_mm512_extload_epi64(&t[0], _MM_UPCONV_EPI64_NONE, _MM_BROADCAST64_NONE, _MM_HINT_NONE));
  }]>;
OclBuiltinImpl popcount_promote = OclBuiltinImpl<popcount, [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                            v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                                            v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                                                            v1u64, v2u64, v3u64, v4u64, v8u64/*, v16u64*/], 0, MaskPromote1>;
OclBuiltinImpl popcount_expand = OclBuiltinImpl<popcount, [v16u64], 0, Expand1>;

// mad24
OclBuiltinImpl mad24_v16_ui32 = OclBuiltinImpl<mad24, [v16i32, v16u32], 0, [{
    __m512i r = _mm512_madd231_epi32((__m512i)$Arg2VarName, (__m512i)$Arg0VarName, (__m512i)$Arg1VarName);
    return as_$ReturnType(r);
  }]>;
OclBuiltinImpl mad24_promote = OclBuiltinImpl<mad24, [v1i32, v2i32, v3i32, v4i32, v8i32/*, v16i32*/,
                                                      v1u32, v2u32, v3u32, v4u32, v8u32/*, v16u32*/], 0, NonMaskPromote3>;

// mul24
OclBuiltinImpl mul24_v16_ui32 = OclBuiltinImpl<mul24, [v16i32, v16u32], 0, [{
    __m512i r = _mm512_mullo_epi32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName);
    return as_$ReturnType(r);
  }]>;
OclBuiltinImpl mul24_promote = OclBuiltinImpl<mul24, [v1i32, v2i32, v3i32, v4i32, v8i32/*, v16i32*/,
                                                      v1u32, v2u32, v3u32, v4u32, v8u32/*, v16u32*/], 0, NonMaskPromote2>;
