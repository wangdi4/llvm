// Copyright (c) 2006-2012 Intel Corporation
// Integers Built-In Functions
// SSE42

/*****************************************************************\
 * abs:                                                          *
 *   ugentype abs (gentype x)                                    *
 *   Returns | x |                                               *
\*****************************************************************/
// abs promote
OclBuiltinImpl abs_ipromote = OclBuiltinImpl<abs, [ v2i8,  v3i8,  v4i8,  v8i8,
                                                  v2i16, v3i16, v4i16,
                                                  v2i32, v3i32                      ], 0, Promote1>;
// abs expand
OclBuiltinImpl abs_iexpand  = OclBuiltinImpl<abs, [                            v16i16,
                                                                       v8i32, v16i32], 0, Expand1>;
// abs native
OclBuiltinImpl abs_inative = OclBuiltinImpl<abs, [v16i8, v8i16, v4i32] , 0, [{
    return (u$Arg0Type)_mm_abs_epi$BitWidth((__m128i)$Arg0VarName);
  }]>;


/*****************************************************************\
 * abs_diff:                                                     *
 *   ugentype abs_diff (gentype x, gentype y)                    *
 *   Returns | x - y |                                           *
\*****************************************************************/
// abs_diff on scalars
OclBuiltinImpl abs_diff_iv1 = OclBuiltinImpl<abs_diff, [ v1i8,  v1u8,
                                                       v1i16, v1u16,
                                                       v1i32, v1u32,
                                                       v1i64, v1u64] , 0, [{
    $ReturnType r = $Arg0VarName - $Arg1VarName;
    if( $Arg1VarName > $Arg0VarName )
        r = $Arg1VarName - $Arg0VarName;
    return r;
  }]>;
// abs_diff on signed vector
OclBuiltinImpl abs_diff_svector = OclBuiltinImpl<abs_diff, [ v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                            v2i16, v3i16, v4i16, v8i16, v16i16,
                                                            v2i32, v3i32, v4i32, v8i32, v16i32,
                                                            v2i64, v3i64, v4i64, v8i64, v16i64] , 0, [{
    $Arg0Type tmp1 = $Arg0VarName - $Arg1VarName;
    $Arg0Type tmp2 = $Arg1VarName - $Arg0VarName;
    $Arg0Type tmpc = $Arg1VarName > $Arg0VarName;
    $Arg0Type res = tmpc & tmp2;
    $Arg0Type reb = (~tmpc) & tmp1;
    return (u$Arg0Type)(res | reb);
  }]>;
// abs_diff on unsigned vector
OclBuiltinImpl abs_diff_uvector = OclBuiltinImpl<abs_diff, [ v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                            v2u16, v3u16, v4u16, v8u16, v16u16,
                                                            v2u32, v3u32, v4u32, v8u32, v16u32,
                                                            v2u64, v3u64, v4u64, v8u64, v16u64] , 0, [{
    $Arg0Type tmp1 = $Arg0VarName - $Arg1VarName;
    $Arg0Type tmp2 = $Arg1VarName - $Arg0VarName;
    $Arg0Type tmpc = ($Arg0Type)($Arg1VarName > $Arg0VarName);
    $Arg0Type res = tmpc & tmp2;
    $Arg0Type reb = (~tmpc) & tmp1;
    return (res | reb);
  }]>;


/*****************************************************************\
 * add_sat:                                                      *
 *   gentype add_sat (gentype x, gentype y)                      *
 *   Returns x + y saturated                                     *
\*****************************************************************/
// add_sat on char16, short8
OclBuiltinImpl add_sat_v16i8v8i16 = OclBuiltinImpl<add_sat, [v16i8, v8i16], 0, [{
    return ($ReturnType)_mm_adds_epi$BitWidth((__m128i)$Arg0VarName, (__m128i)$Arg1VarName);
  }]>;
// add_sat on uchar16, ushort8
OclBuiltinImpl add_sat_v16u8v8u16 = OclBuiltinImpl<add_sat, [v16u8, v8u16], 0, [{
    return ($ReturnType)_mm_adds_epu$BitWidth((__m128i)$Arg0VarName, (__m128i)$Arg1VarName);
  }]>;
// add_sat promote
OclBuiltinImpl add_sat_ssepromote = OclBuiltinImpl<add_sat, [ v2i8,  v3i8,  v4i8,  v8i8,
                                                              v2u8,  v3u8,  v4u8,  v8u8,
                                                             v2i16, v3i16, v4i16,
                                                             v2u16, v3u16, v4u16       ], 0, Promote2>;
// add_sat expand
OclBuiltinImpl add_sat_sseexpand  = OclBuiltinImpl<add_sat, [v16i16, v16u16], 0, Expand2>;


/*****************************************************************\
 * hadd:                                                         *
 *   gentype hadd (gentype x, gentype y)                         *
 *   Returns (x + y) >> 1. The intermediate sum does             *
 *     not modulo overflow.                                      *
\*****************************************************************/
// hadd on scalars
OclBuiltinImpl hadd_v1i8 = OclBuiltinImpl<hadd, [v1i8], 0, [{
    return ( (short)($Arg0VarName) + (short)($Arg1VarName) ) >> 1;
  }]>;
OclBuiltinImpl hadd_v1u8 = OclBuiltinImpl<hadd, [v1u8], 0, [{
    return ( (ushort)($Arg0VarName) + (ushort)($Arg1VarName) ) >> 1;
  }]>;
OclBuiltinImpl hadd_v1i16 = OclBuiltinImpl<hadd, [v1i16], 0, [{
    return ( (int)($Arg0VarName) + (int)($Arg1VarName) ) >> 1;
  }]>;
OclBuiltinImpl hadd_v1u16 = OclBuiltinImpl<hadd, [v1u16], 0, [{
    return ( (uint)($Arg0VarName) + (uint)($Arg1VarName) ) >> 1;
  }]>;
OclBuiltinImpl hadd_v1i32 = OclBuiltinImpl<hadd, [v1i32], 0, [{
    return ( (long)($Arg0VarName) + (long)($Arg1VarName) ) >> 1;
  }]>;
OclBuiltinImpl hadd_v1u32 = OclBuiltinImpl<hadd, [v1u32], 0, [{
    return ( (ulong)($Arg0VarName) + (ulong)($Arg1VarName) ) >> 1;
  }]>;
OclBuiltinImpl hadd_v1i64 = OclBuiltinImpl<hadd, [v1i64], 0, [{
    long overflow = ( $Arg0VarName & 0x1 ) + ( $Arg1VarName & 0x1 );
    return ( ( $Arg0VarName >> 1 ) + ( $Arg1VarName >> 1 ) ) + ( overflow >> 1 );
  }]>;
OclBuiltinImpl hadd_v1u64 = OclBuiltinImpl<hadd, [v1u64], 0, [{
    ulong overflow = ( $Arg0VarName & 0x1 ) + ( $Arg1VarName & 0x1 );
    return ( ( $Arg0VarName >> 1 ) + ( $Arg1VarName >> 1 ) ) + ( overflow >> 1 );
  }]>;


/*****************************************************************\
 * rhadd:                                                        *
 *   gentype rhadd (gentype x, gentype y)                        *
 *   Returns (x + y + 1) >> 1. The intermediate sum does         *
 *     not modulo overflow.                                      *
\*****************************************************************/
// rhadd on scalars
OclBuiltinImpl rhadd_v1i8 = OclBuiltinImpl<rhadd, [v1i8], 0, [{
    return ( (short)($Arg0VarName) + (short)($Arg1VarName) + 1 ) >> 1;
  }]>;
OclBuiltinImpl rhadd_v1u8 = OclBuiltinImpl<rhadd, [v1u8], 0, [{
    return ( (ushort)($Arg0VarName) + (ushort)($Arg1VarName) + 1 ) >> 1;
  }]>;
OclBuiltinImpl rhadd_v1i16 = OclBuiltinImpl<rhadd, [v1i16], 0, [{
    return ( (int)($Arg0VarName) + (int)($Arg1VarName) + 1 ) >> 1;
  }]>;
OclBuiltinImpl rhadd_v1u16 = OclBuiltinImpl<rhadd, [v1u16], 0, [{
    return ( (uint)($Arg0VarName) + (uint)($Arg1VarName) + 1 ) >> 1;
  }]>;
OclBuiltinImpl rhadd_v1i32 = OclBuiltinImpl<rhadd, [v1i32], 0, [{
    return ( (long)($Arg0VarName) + (long)($Arg1VarName) + 1) >> 1;
  }]>;
OclBuiltinImpl rhadd_v1u32 = OclBuiltinImpl<rhadd, [v1u32], 0, [{
    return ( (ulong)($Arg0VarName) + (ulong)($Arg1VarName) + 1) >> 1;
  }]>;
OclBuiltinImpl rhadd_v1i64 = OclBuiltinImpl<rhadd, [v1i64], 0, [{
    long overflow = ( $Arg0VarName | $Arg1VarName ) & 0x1;
    return ( ( $Arg0VarName >> 1 ) + ( $Arg1VarName >> 1 ) ) + overflow;
  }]>;
OclBuiltinImpl rhadd_v1u64 = OclBuiltinImpl<rhadd, [v1u64], 0, [{
    ulong overflow = ( $Arg0VarName | $Arg1VarName ) & 0x1;
    return ( ( $Arg0VarName >> 1 ) + ( $Arg1VarName >> 1 ) ) + overflow;
  }]>;


/*****************************************************************\
 * mad_sat:                                                      *
 *   gentype mad_sat (gentype x , gentype y , gentype z)         *
 *   Returns a * b + c and saturates the result.                 *
\*****************************************************************/
// mad_sat char4
OclBuiltinImpl mad_sat_v4i8 = OclBuiltinImpl<mad_sat, [v4i8], 0, [{
    int4 res;
    int4 cmin = (int)generic_min_char;
    int4 cmax = (int)generic_max_char;
    res =  convert_int4($Arg0VarName) * convert_int4($Arg1VarName) + convert_int4($Arg2VarName);
    return convert_char4( clamp ( res,  cmin , cmax ) );
  }]>;
// mad_sat uchar4
OclBuiltinImpl mad_sat_v4u8 = OclBuiltinImpl<mad_sat, [v4u8], 0, [{
    uint4 res;
    uint4 cmin = (uint)generic_min_uchar;
    uint4 cmax = (uint)generic_max_uchar;
    res =  convert_uint4($Arg0VarName) * convert_uint4($Arg1VarName) + convert_uint4($Arg2VarName);
    return convert_uchar4( clamp ( res,  cmin , cmax ) );
  }]>;
// mad_sat fix for char8, uchar8
OclBuiltinImpl mad_sat_v8i8u8 = OclBuiltinImpl<mad_sat, [v8i8, v8u8], 0, [{
    $NativeArg0Type a2, b2, c2, res;
    a2.s02468ACE = $Arg0VarName;
    b2.s02468ACE = $Arg1VarName;
    c2.s02468ACE = $Arg2VarName;
    res = $Func(a2, b2, c2);
    return res.s02468ACE;
  }]>;
// mad_sat promote
OclBuiltinImpl mad_sat_ipromote = OclBuiltinImpl<mad_sat, [ v2i8,  v3i8,
                                                          v2i16, v3i16, v4i16,
                                                          v2i32, v3i32,
                                                           v2u8,  v3u8,
                                                          v2u16, v3u16, v4u16,
                                                          v2u32, v3u32              ], 0, Promote3>;
// mad_sat expand
OclBuiltinImpl mad_sat_iexpand  = OclBuiltinImpl<mad_sat, [                            v16i16,
                                                                               v8i32, v16i32,
                                                          v2i64, v3i64, v4i64, v8i64, v16i64,
                                                                                      v16u16,
                                                                               v8u32, v16u32,
                                                          v2u64, v3u64, v4u64, v8u64, v16u64], 0, Expand3>;
// mad_sat native char16
OclBuiltinImpl mad_sat_v16i8 = OclBuiltinImpl<mad_sat, [v16i8], 0, [{
    char16 zero = (char16)0;
    short8 lox = (short8)_mm_unpacklo_epi8((__m128i)zero, (__m128i)$Arg0VarName);
    lox = lox >> (short)8;
    short8 loy = (short8)_mm_unpacklo_epi8((__m128i)zero, (__m128i)$Arg1VarName);
    loy = loy >> (short)8;
    short8 loz = (short8)_mm_unpacklo_epi8((__m128i)zero, (__m128i)$Arg2VarName);
    loz = loz >> (short)8;
    short8 reslo = lox * loy ;
    reslo = reslo + loz;
    reslo = max(reslo, (short)generic_min_char);
    reslo = min(reslo, (short)generic_max_char);
    short8 hix = (short8)_mm_unpackhi_epi8((__m128i)zero, (__m128i)$Arg0VarName);
    hix = hix >> (short)8;
    short8 hiy = (short8)_mm_unpackhi_epi8((__m128i)zero, (__m128i)$Arg1VarName);
    hiy = hiy >> (short)8;
    short8 hiz = (short8)_mm_unpackhi_epi8((__m128i)zero, (__m128i)$Arg2VarName);
    hiz = hiz >> (short)8;
    short8 reshi = hix * hiy;
    reshi = reshi + hiz;
    reshi = max(reshi, (short)generic_min_char);
    reshi = min(reshi, (short)generic_max_char);
    char16 res = (char16)_mm_packs_epi16((__m128i)reslo, (__m128i)reshi);
    return res;
  }]>;
// mad_sat native uchar16
OclBuiltinImpl mad_sat_v16u8 = OclBuiltinImpl<mad_sat, [v16u8], 0, [{
    uchar16 zero = (uchar)0;
    ushort8 lox = (ushort8)_mm_unpacklo_epi8((__m128i)$Arg0VarName, (__m128i)zero);
    ushort8 loy = (ushort8)_mm_unpacklo_epi8((__m128i)$Arg1VarName, (__m128i)zero);
    ushort8 loz = (ushort8)_mm_unpacklo_epi8((__m128i)$Arg2VarName, (__m128i)zero);
    ushort8 reslo = lox * loy;
    reslo = reslo + loz;
    reslo = min(reslo, (ushort)generic_max_uchar);
    ushort8 hix = (ushort8)_mm_unpackhi_epi8((__m128i)$Arg0VarName, (__m128i)zero);
    ushort8 hiy = (ushort8)_mm_unpackhi_epi8((__m128i)$Arg1VarName, (__m128i)zero);
    ushort8 hiz = (ushort8)_mm_unpackhi_epi8((__m128i)$Arg2VarName, (__m128i)zero);
    ushort8 reshi = hix * hiy;
    reshi = reshi + hiz;
    reshi = min(reshi, (ushort)generic_max_uchar);
    uchar16 res = (uchar16)_mm_packus_epi16((__m128i)reslo, (__m128i)reshi);
    return res;
  }]>;
// mad_sat native short8
OclBuiltinImpl mad_sat_v8i16 = OclBuiltinImpl<mad_sat, [v8i16] , 0, [{
    short8 zero = (short8)0;
    int4 lox = (int4)_mm_unpacklo_epi16((__m128i)zero, (__m128i)$Arg0VarName);
    lox = lox >> 16;
    int4 loy = (int4)_mm_unpacklo_epi16((__m128i)zero, (__m128i)$Arg1VarName);
    loy = loy >> 16;
    int4 loz = (int4)_mm_unpacklo_epi16((__m128i)zero, (__m128i)$Arg2VarName);
    loz = loz >> 16;
    int4 reslo = lox * loy;
    reslo = reslo + loz;
    reslo = max(reslo, (int)generic_min_short);
    reslo = min(reslo, (int)generic_max_short);
    int4 hix = (int4)_mm_unpackhi_epi16((__m128i)zero, (__m128i)$Arg0VarName);
    hix = hix >> 16;
    int4 hiy = (int4)_mm_unpackhi_epi16((__m128i)zero, (__m128i)$Arg1VarName);
    hiy = hiy >> 16;
    int4 hiz = (int4)_mm_unpackhi_epi16((__m128i)zero, (__m128i)$Arg2VarName);
    hiz = hiz >> 16;
    int4 reshi = hix * hiy;
    reshi = reshi + hiz;
    reshi = max(reshi, (int)generic_min_short);
    reshi = min(reshi, (int)generic_max_short);
    short8 res = (short8)_mm_packs_epi32((__m128i)reslo, (__m128i)reshi);
    return res;
  }]>;
// mad_sat native ushort8
OclBuiltinImpl mad_sat_v8u16 = OclBuiltinImpl<mad_sat, [v8u16] , 0, [{
    ushort8 zero = (ushort8)0;
    uint4 lox = (uint4)_mm_unpacklo_epi16((__m128i)$Arg0VarName, (__m128i)zero);
    uint4 loy = (uint4)_mm_unpacklo_epi16((__m128i)$Arg1VarName, (__m128i)zero);
    uint4 loz = (uint4)_mm_unpacklo_epi16((__m128i)$Arg2VarName, (__m128i)zero);
    uint4 reslo = lox * loy;
    reslo = reslo + loz;
    reslo = min(reslo, (uint4)generic_max_ushort);
    uint4 hix = (uint4)_mm_unpackhi_epi16((__m128i)$Arg0VarName, (__m128i)zero);
    uint4 hiy = (uint4)_mm_unpackhi_epi16((__m128i)$Arg1VarName, (__m128i)zero);
    uint4 hiz = (uint4)_mm_unpackhi_epi16((__m128i)$Arg2VarName, (__m128i)zero);
    uint4 reshi = hix * hiy;
    reshi = reshi + hiz;
    reshi = min(reshi, (uint4)generic_max_ushort);
    ushort8 res = (ushort8)_mm_packus_epi32((__m128i)reslo, (__m128i)reshi);
    return res;
  }]>;
// mad_sat native int4
OclBuiltinImpl mad_sat_v4i32 = OclBuiltinImpl<mad_sat, [v4i32] , 0, [{
    int4 loz = $Arg2VarName & as_int4((long2)long_even_mask);
    int4 reslo = (int4)_mm_mul_epi32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName);
    reslo = reslo + loz;
    reslo = as_int4(max(as_long2(reslo), (long)generic_min_int));
    reslo = as_int4(min(as_long2(reslo), (long)generic_max_int));
    reslo = reslo & as_int4((long2)long_even_mask);
    $Arg0VarName = as_int4(as_long2($Arg0VarName) >> 32);
    $Arg1VarName = as_int4(as_long2($Arg1VarName) >> 32);
    $Arg2VarName = as_int4(as_long2($Arg2VarName) >> 32);
    int4 reshi = (int4)_mm_mul_epi32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName);
    reshi = reshi + $Arg2VarName;
    reshi = as_int4(max(as_long2(reshi), (long)generic_min_int));
    reshi = as_int4(min(as_long2(reshi), (long)generic_max_int));
    reshi = as_int4(as_long2(reshi) << 32);
    int4 res = reshi | reslo;
    return res;
  }]>;
// mad_sat native uint4
OclBuiltinImpl mad_sat_v4u32 = OclBuiltinImpl<mad_sat, [v4u32] , 0, [{
    uint4 loz = $Arg2VarName & as_uint4((ulong2)long_even_mask);
    uint4 reslo = (uint4)_mm_mul_epi32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName);
    reslo = reslo + loz;
    reslo = as_uint4(min(as_ulong2(reslo), (ulong)generic_max_uint));
    reslo = reslo & as_uint4((ulong2)long_even_mask);
    $Arg0VarName = as_uint4(as_ulong2($Arg0VarName) >> 32);
    $Arg1VarName = as_uint4(as_ulong2($Arg1VarName) >> 32);
    $Arg2VarName = as_uint4(as_ulong2($Arg2VarName) >> 32);
    uint4 reshi = (uint4)_mm_mul_epi32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName);
    reshi = reshi + $Arg2VarName;
    reshi = as_uint4(min(as_ulong2(reshi), (ulong)generic_max_uint));
    reshi = as_uint4(as_ulong2(reshi) << 32);
    uint4 res = reshi | reslo;
    return res;
  }]>;


/*****************************************************************\
 * max:                                                          *
 *   gentype max (gentype x , gentype y)                         *
 *   gentype max (gentype x , sgentype y)                        *
 *   Returns y if x < y, otherwise it returns x                  *
\*****************************************************************/
// max native char16, short8, int4
OclBuiltinImpl max_native_i = OclBuiltinImpl<max, [v16i8, v8i16, v4i32], 0, [{
    return ($ReturnType)_mm_max_epi$BitWidth((__m128i)$Arg0VarName , (__m128i)$Arg1VarName);
  }]>;
// max native uchar16, ushort8, uint4
OclBuiltinImpl max_native_u = OclBuiltinImpl<max, [v16u8, v8u16, v4u32], 0, [{
    return ($ReturnType)_mm_max_epu$BitWidth((__m128i)$Arg0VarName , (__m128i)$Arg1VarName);
  }]>;
// max fix for char4
OclBuiltinImpl max_v4i8 = OclBuiltinImpl<max, [v4i8], 0, [{
    int4 i$Arg0VarName = convert_int4($Arg0VarName);
    int4 i$Arg1VarName = convert_int4($Arg1VarName);
    return convert_char4( $Func(i$Arg0VarName, i$Arg1VarName) );
  }]>;
// max fix for uchar4
OclBuiltinImpl max_v4u8 = OclBuiltinImpl<max, [v4u8], 0, [{
    uint4 i$Arg0VarName = convert_uint4($Arg0VarName);
    uint4 i$Arg1VarName = convert_uint4($Arg1VarName);
    return convert_uchar4( $Func(i$Arg0VarName, i$Arg1VarName) );
  }]>;
// max fix for char8, uchar8
OclBuiltinImpl max_v8i8u8 = OclBuiltinImpl<max, [v8i8, v8u8], 0, [{
    $NativeArg0Type x2, y2, res;
    x2.s02468ACE = $Arg0VarName;
    y2.s02468ACE = $Arg1VarName;
    res = $Func(x2,y2);
    return res.s02468ACE;
  }]>;
// max promote
OclBuiltinImpl max_ipromote = OclBuiltinImpl<max, [ v2i8,  v3i8,
                                                   v2u8,  v3u8,
                                                  v2i16, v3i16, v4i16,
                                                  v2u16, v3u16, v4u16,
                                                  v2i32, v3i32,
                                                  v2u32, v3u32                      ], 0, Promote2>;
// max expand
OclBuiltinImpl max_iexpand  = OclBuiltinImpl<max, [                            v16i16,
                                                                              v16u16,
                                                                       v8i32, v16i32,
                                                                       v8u32, v16u32], 0, Expand2>;

/*****************************************************************\
 * min:                                                          *
 *   gentype min (gentype x , gentype y)                         *
 *   gentype min (gentype x , sgentype y)                        *
 *   Returns y if y < x, otherwise it returns x                  *
\*****************************************************************/
// min native char16, short8, int4
OclBuiltinImpl min_native_i = OclBuiltinImpl<min, [v16i8, v8i16, v4i32], 0, [{
    return ($ReturnType)_mm_min_epi$BitWidth((__m128i)$Arg0VarName , (__m128i)$Arg1VarName);
  }]>;
// min native uchar16, ushort8, uint4
OclBuiltinImpl min_native_u = OclBuiltinImpl<min, [v16u8, v8u16, v4u32], 0, [{
    return ($ReturnType)_mm_min_epu$BitWidth((__m128i)$Arg0VarName , (__m128i)$Arg1VarName);
  }]>;
// min fix for char4
OclBuiltinImpl min_v4i8 = OclBuiltinImpl<min, [v4i8], 0, [{
    int4 i$Arg0VarName = convert_int4($Arg0VarName);
    int4 i$Arg1VarName = convert_int4($Arg1VarName);
    return convert_char4( $Func(i$Arg0VarName, i$Arg1VarName) );
  }]>;
// min fix for uchar4
OclBuiltinImpl min_v4u8 = OclBuiltinImpl<min, [v4u8], 0, [{
    uint4 i$Arg0VarName = convert_uint4($Arg0VarName);
    uint4 i$Arg1VarName = convert_uint4($Arg1VarName);
    return convert_uchar4( $Func(i$Arg0VarName, i$Arg1VarName) );
  }]>;
// min fix for char8, uchar8
OclBuiltinImpl min_v8i8u8 = OclBuiltinImpl<min, [v8i8, v8u8], 0, [{
    $NativeArg0Type x2, y2, res;
    x2.s02468ACE = x;
    y2.s02468ACE = y;
    res = $Func(x2,y2);
    return res.s02468ACE;
  }]>;
// min promote
OclBuiltinImpl min_ipromote = OclBuiltinImpl<min, [ v2i8,  v3i8,
                                                   v2u8,  v3u8,
                                                  v2i16, v3i16, v4i16,
                                                  v2u16, v3u16, v4u16,
                                                  v2i32, v3i32,
                                                  v2u32, v3u32                      ], 0, Promote2>;
// min expand
OclBuiltinImpl min_iexpand  = OclBuiltinImpl<min, [                            v16i16,
                                                                              v16u16,
                                                                       v8i32, v16i32,
                                                                       v8u32, v16u32], 0, Expand2>;


/*****************************************************************\
 * mul_hi:                                                       *
 *   gentype mul_hi (gentype x , gentype y)                      *
 *   Returns the high half of the product of (x * y)             *
\*****************************************************************/
// mul_hi promote
OclBuiltinImpl mul_hi_ipromote = OclBuiltinImpl<mul_hi, [ v2i8,  v3i8,  v4i8,  v8i8,
                                                        v2i16, v3i16, v4i16,
                                                        v2i32, v3i32,
                                                         v2u8,  v3u8,  v4u8,  v8u8,
                                                        v2u16, v3u16, v4u16,
                                                        v2u32, v3u32                      ], 0, Promote2>;
// mul_hi expand
OclBuiltinImpl mul_hi_iexpand  = OclBuiltinImpl<mul_hi, [                            v16i16,
                                                                             v8i32, v16i32,
                                                        v2i64, v3i64, v4i64, v8i64, v16i64,
                                                                                    v16u16,
                                                                             v8u32, v16u32,
                                                        v2u64, v3u64, v4u64, v8u64, v16u64], 0, Expand2>;
// mul_hi native char16
OclBuiltinImpl mul_hi_v16i8 = OclBuiltinImpl<mul_hi, [v16i8], 0, [{
    $Arg0Type zero = ($Arg0Type)0;
    short8 tempX = (short8)_mm_unpacklo_epi8((__m128i)zero, (__m128i)$Arg0VarName);
    short8 tempY = (short8)_mm_unpacklo_epi8((__m128i)zero, (__m128i)$Arg1VarName);
    tempX = tempX >> (short)8;
    tempY = tempY >> (short)8;
    short8 temp = tempX * tempY;
    short8 reslo = as_short8(as_ushort8(temp) >> (short)8);
    tempX = (short8)_mm_unpackhi_epi8((__m128i)zero, (__m128i)$Arg0VarName);
    tempY = (short8)_mm_unpackhi_epi8((__m128i)zero, (__m128i)$Arg1VarName);
    tempX = tempX >> (short)8;
    tempY = tempY >> (short)8;
    temp = tempX * tempY;
    short8 reshi = as_short8(as_ushort8(temp) >> (short)8);
    $ReturnType res = ($ReturnType)_mm_packus_epi16((__m128i)reslo, (__m128i)reshi);
    return res;
  }]>;
// mul_hi native uchar16
OclBuiltinImpl mul_hi_v16u8 = OclBuiltinImpl<mul_hi, [v16u8], 0, [{
    $Arg0Type zero = ($Arg0Type)0;
    ushort8 tempX = (ushort8)_mm_unpacklo_epi8((__m128i)$Arg0VarName, (__m128i)zero);
    ushort8 tempY = (ushort8)_mm_unpacklo_epi8((__m128i)$Arg1VarName, (__m128i)zero);
    ushort8 temp = tempX * tempY;
    ushort8 reslo = temp >> (short)8;
    tempX = (ushort8)_mm_unpackhi_epi8((__m128i)$Arg0VarName, (__m128i)zero);
    tempY = (ushort8)_mm_unpackhi_epi8((__m128i)$Arg1VarName, (__m128i)zero);
    temp = tempX * tempY;
    ushort8 reshi = temp >> (short)8;
    $ReturnType res = ($ReturnType)_mm_packus_epi16((__m128i)reslo, (__m128i)reshi);
    return res;
  }]>;
// mul_hi native short8
OclBuiltinImpl mul_hi_v8i16 = OclBuiltinImpl<mul_hi, [v8i16] , 0, [{
    return (short8) _mm_mulhi_epi16((__m128i)$Arg0VarName, (__m128i)$Arg1VarName);
  }]>;
// mul_hi native ushort8
OclBuiltinImpl mul_hi_v8u16 = OclBuiltinImpl<mul_hi, [v8u16] , 0, [{
    return (ushort8) _mm_mulhi_epu16((__m128i)$Arg0VarName, (__m128i)$Arg1VarName);
  }]>;
// mul_hi native int4
OclBuiltinImpl mul_hi_v4i32 = OclBuiltinImpl<mul_hi, [v4i32] , 0, [{
    ulong2 reslo = (ulong2)_mm_mul_epi32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName);
    reslo = reslo >> (ulong)32;
    $Arg0VarName = as_$Arg0Type(as_ulong2($Arg0VarName) >> (ulong)32);
    $Arg1VarName = as_$Arg1Type(as_ulong2($Arg1VarName) >> (ulong)32);
    ulong2 reshi = (ulong2)_mm_mul_epi32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName);
    $ReturnType res = ($ReturnType)_mm_blend_epi16 ((__m128i)reslo, (__m128i)reshi, 0b11001100);
    return res;
  }]>;
// mul_hi native uint4
OclBuiltinImpl mul_hi_v4u32 = OclBuiltinImpl<mul_hi, [v4u32] , 0, [{
    ulong2 reslo = (ulong2)_mm_mul_epu32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName);
    reslo = reslo >> (ulong)32;
    $Arg0VarName = as_$Arg0Type(as_ulong2($Arg0VarName) >> (ulong)32);
    $Arg1VarName = as_$Arg1Type(as_ulong2($Arg1VarName) >> (ulong)32);
    ulong2 reshi = (ulong2)_mm_mul_epu32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName);
    $ReturnType res = ($ReturnType)_mm_blend_epi16 ((__m128i)reslo, (__m128i)reshi, 0b11001100);
    return res;
  }]>;


/*****************************************************************\
 * sub_sat:                                                      *
 *   gentype sub_sat (gentype x, gentype y)                      *
 *   Returns x - y saturated                                     *
\*****************************************************************/
// sub_sat on char16, short8
OclBuiltinImpl sub_sat_v16i8v8i16 = OclBuiltinImpl<sub_sat, [v16i8, v8i16], 0, [{
    return ($ReturnType)_mm_subs_epi$BitWidth((__m128i)$Arg0VarName, (__m128i)$Arg1VarName);
  }]>;
// sub_sat on uchar16, ushort8
OclBuiltinImpl sub_sat_v16u8v8u16 = OclBuiltinImpl<sub_sat, [v16u8, v8u16], 0, [{
    return ($ReturnType)_mm_subs_epu$BitWidth((__m128i)$Arg0VarName, (__m128i)$Arg1VarName);
  }]>;
// sub_sat promote
OclBuiltinImpl sub_sat_ssepromote = OclBuiltinImpl<sub_sat, [ v2i8,  v3i8,  v4i8,  v8i8,
                                                              v2u8,  v3u8,  v4u8,  v8u8,
                                                             v2i16, v3i16, v4i16,
                                                             v2u16, v3u16, v4u16       ], 0, Promote2>;
// sub_sat expand
OclBuiltinImpl sub_sat_sseexpand  = OclBuiltinImpl<sub_sat, [v16i16, v16u16], 0, Expand2>;
