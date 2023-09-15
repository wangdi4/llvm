// Copyright (c) 2006 Intel Corporation
// Integers Built-In Functions
// SSSE3

/*****************************************************************\
 * abs:                                                          *
 *   ugentype abs (gentype x)                                    *
 *   Returns | x |                                               *
\*****************************************************************/
// abs promote
OclBuiltinImpl abs_ipromote = OclBuiltinImpl<abs, [ v2i8,  v3i8,  v4i8,  v8i8,
                                                    v2i16, v3i16, v4i16,
                                                    v2i32, v3i32                      ], 0, Promote1>;
// abs native
OclBuiltinImpl abs_inative = OclBuiltinImpl<abs, [v16i8, v8i16, v4i32] , 0, [{
    return as_u$Arg0Type(_mm_abs_epi$BitWidth((__m128i)$Arg0VarName));
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
OclBuiltinImpl abs_diff_svector = OclBuiltinImpl<abs_diff, [ v2i8,  v3i8, v4i8,  v8i8,  v16i8,
                                                            v2i16, v3i16, v4i16, v8i16, 
                                                            v2i32, v3i32, v4i32,
                                                            v2i64, v3i64, v16i64] , 0, [{
    $Arg0Type tmp1 = $Arg0VarName - $Arg1VarName;
    $Arg0Type tmp2 = $Arg1VarName - $Arg0VarName;
    $Arg0Type tmpc = $Arg1VarName > $Arg0VarName;
    $Arg0Type res = tmpc & tmp2;
    $Arg0Type reb = (~tmpc) & tmp1;
    return as_u$Arg0Type(res | reb);
  }]>;
// abs_diff on unsigned vector
OclBuiltinImpl abs_diff_uvector = OclBuiltinImpl<abs_diff, [ v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                            v2u16, v3u16, v4u16, v8u16,
                                                            v2u32, v3u32, v4u32,
                                                            v2u64, v3u64, v16u64] , 0, [{
    $Arg0Type tmp1 = $Arg0VarName - $Arg1VarName;
    $Arg0Type tmp2 = $Arg1VarName - $Arg0VarName;
    $Arg0Type tmpc = as_$Arg0Type($Arg1VarName > $Arg0VarName);
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
    return as_$ReturnType(_mm_adds_epi$BitWidth((__m128i)$Arg0VarName, (__m128i)$Arg1VarName));
  }]>;
// add_sat on uchar16, ushort8
OclBuiltinImpl add_sat_v16u8v8u16 = OclBuiltinImpl<add_sat, [v16u8, v8u16], 0, [{
    return as_$ReturnType(_mm_adds_epu$BitWidth((__m128i)$Arg0VarName, (__m128i)$Arg1VarName));
  }]>;
// add_sat promote
OclBuiltinImpl add_sat_ssepromote = OclBuiltinImpl<add_sat, [ v2i8,  v3i8,  v4i8,  v8i8,
                                                              v2u8,  v3u8,  v4u8,  v8u8,
                                                              v2i16, v3i16, v4i16,
                                                              v2u16, v3u16, v4u16       ], 0, Promote2>;


/*****************************************************************\
 * sub_sat:                                                      *
 *   gentype sub_sat (gentype x, gentype y)                      *
 *   Returns x - y saturated                                     *
\*****************************************************************/
// sub_sat on char16, short8
OclBuiltinImpl sub_sat_v16i8v8i16 = OclBuiltinImpl<sub_sat, [v16i8, v8i16], 0, [{
    return as_$ReturnType(_mm_subs_epi$BitWidth((__m128i)$Arg0VarName, (__m128i)$Arg1VarName));
  }]>;
// sub_sat on uchar16, ushort8
OclBuiltinImpl sub_sat_v16u8v8u16 = OclBuiltinImpl<sub_sat, [v16u8, v8u16], 0, [{
    return as_$ReturnType(_mm_subs_epu$BitWidth((__m128i)$Arg0VarName, (__m128i)$Arg1VarName));
  }]>;
// sub_sat promote
OclBuiltinImpl sub_sat_ssepromote = OclBuiltinImpl<sub_sat, [ v2i8,  v3i8,  v4i8,  v8i8,
                                                              v2u8,  v3u8,  v4u8,  v8u8,
                                                              v2i16, v3i16, v4i16,
                                                              v2u16, v3u16, v4u16       ], 0, Promote2>;


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
    res = convert_int4($Arg0VarName) * convert_int4($Arg1VarName) + convert_int4($Arg2VarName);
    return convert_char4( clamp ( res,  cmin , cmax ) );
  }]>;
// mad_sat uchar4
OclBuiltinImpl mad_sat_v4u8 = OclBuiltinImpl<mad_sat, [v4u8], 0, [{
    uint4 res;
    uint4 cmin = (uint)generic_min_uchar;
    uint4 cmax = (uint)generic_max_uchar;
    res = convert_uint4($Arg0VarName) * convert_uint4($Arg1VarName) + convert_uint4($Arg2VarName);
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
OclBuiltinImpl mad_sat_iexpand  = OclBuiltinImpl<mad_sat, [                             v16i16,
                                                            v2i64, v3i64, v4i64, v8i64, v16i64,
                                                            v2u64, v3u64, v4u64, v8u64, v16u64], 0, Expand3>;
// mad_sat native char16
OclBuiltinImpl mad_sat_v16i8 = OclBuiltinImpl<mad_sat, [v16i8], 0, [{
    char16 zero = (char16)0;
    short8 lox = as_short8(_mm_unpacklo_epi8((__m128i)zero, (__m128i)$Arg0VarName));
    lox = lox >> (short)8;
    short8 loy = as_short8(_mm_unpacklo_epi8((__m128i)zero, (__m128i)$Arg1VarName));
    loy = loy >> (short)8;
    short8 loz = as_short8(_mm_unpacklo_epi8((__m128i)zero, (__m128i)$Arg2VarName));
    loz = loz >> (short)8;
    short8 reslo = lox * loy ;
    reslo = reslo + loz;
    reslo = max(reslo, (short)generic_min_char);
    reslo = min(reslo, (short)generic_max_char);
    short8 hix = as_short8(_mm_unpackhi_epi8((__m128i)zero, (__m128i)$Arg0VarName));
    hix = hix >> (short)8;
    short8 hiy = as_short8(_mm_unpackhi_epi8((__m128i)zero, (__m128i)$Arg1VarName));
    hiy = hiy >> (short)8;
    short8 hiz = as_short8(_mm_unpackhi_epi8((__m128i)zero, (__m128i)$Arg2VarName));
    hiz = hiz >> (short)8;
    short8 reshi = hix * hiy;
    reshi = reshi + hiz;
    reshi = max(reshi, (short)generic_min_char);
    reshi = min(reshi, (short)generic_max_char);
    char16 res = as_char16(_mm_packs_epi16((__m128i)reslo, (__m128i)reshi));
    return res;
  }]>;
// mad_sat native uchar16
OclBuiltinImpl mad_sat_v16u8 = OclBuiltinImpl<mad_sat, [v16u8], 0, [{
    uchar16 zero = (uchar)0;
    ushort8 lox = as_ushort8(_mm_unpacklo_epi8((__m128i)$Arg0VarName, (__m128i)zero));
    ushort8 loy = as_ushort8(_mm_unpacklo_epi8((__m128i)$Arg1VarName, (__m128i)zero));
    ushort8 loz = as_ushort8(_mm_unpacklo_epi8((__m128i)$Arg2VarName, (__m128i)zero));
    ushort8 reslo = lox * loy;
    reslo = reslo + loz;
    reslo = min(reslo, (ushort)generic_max_uchar);
    ushort8 hix = as_ushort8(_mm_unpackhi_epi8((__m128i)$Arg0VarName, (__m128i)zero));
    ushort8 hiy = as_ushort8(_mm_unpackhi_epi8((__m128i)$Arg1VarName, (__m128i)zero));
    ushort8 hiz = as_ushort8(_mm_unpackhi_epi8((__m128i)$Arg2VarName, (__m128i)zero));
    ushort8 reshi = hix * hiy;
    reshi = reshi + hiz;
    reshi = min(reshi, (ushort)generic_max_uchar);
    uchar16 res = as_uchar16(_mm_packus_epi16((__m128i)reslo, (__m128i)reshi));
    return res;
  }]>;
// mad_sat native short8
OclBuiltinImpl mad_sat_v8i16 = OclBuiltinImpl<mad_sat, [v8i16] , 0, [{
    short8 zero = (short8)0;
    int4 lox = as_int4(_mm_unpacklo_epi16((__m128i)zero, (__m128i)$Arg0VarName));
    lox = lox >> 16;
    int4 loy = as_int4(_mm_unpacklo_epi16((__m128i)zero, (__m128i)$Arg1VarName));
    loy = loy >> 16;
    int4 loz = as_int4(_mm_unpacklo_epi16((__m128i)zero, (__m128i)$Arg2VarName));
    loz = loz >> 16;
    int4 reslo = lox * loy;
    reslo = reslo + loz;
    reslo = max(reslo, (int)generic_min_short);
    reslo = min(reslo, (int)generic_max_short);
    int4 hix = as_int4(_mm_unpackhi_epi16((__m128i)zero, (__m128i)$Arg0VarName));
    hix = hix >> 16;
    int4 hiy = as_int4(_mm_unpackhi_epi16((__m128i)zero, (__m128i)$Arg1VarName));
    hiy = hiy >> 16;
    int4 hiz = as_int4(_mm_unpackhi_epi16((__m128i)zero, (__m128i)$Arg2VarName));
    hiz = hiz >> 16;
    int4 reshi = hix * hiy;
    reshi = reshi + hiz;
    reshi = max(reshi, (int)generic_min_short);
    reshi = min(reshi, (int)generic_max_short);
    short8 res = as_short8(_mm_packs_epi32((__m128i)reslo, (__m128i)reshi));
    return res;
  }]>;

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
OclBuiltinImpl mul_hi_iexpand  = OclBuiltinImpl<mul_hi, [v2i64, v3i64, v4i64, v16i64,
                                                         v2u64, v3u64, v16u64], 0, Expand2>;
// mul_hi native char16
OclBuiltinImpl mul_hi_v16i8 = OclBuiltinImpl<mul_hi, [v16i8], 0, [{
    $Arg0Type zero = ($Arg0Type)0;
    short8 tempX = as_short8(_mm_unpacklo_epi8((__m128i)zero, (__m128i)$Arg0VarName));
    short8 tempY = as_short8(_mm_unpacklo_epi8((__m128i)zero, (__m128i)$Arg1VarName));
    tempX = tempX >> (short)8;
    tempY = tempY >> (short)8;
    short8 temp = tempX * tempY;
    short8 reslo = as_short8(as_ushort8(temp) >> (short)8);
    tempX = as_short8(_mm_unpackhi_epi8((__m128i)zero, (__m128i)$Arg0VarName));
    tempY = as_short8(_mm_unpackhi_epi8((__m128i)zero, (__m128i)$Arg1VarName));
    tempX = tempX >> (short)8;
    tempY = tempY >> (short)8;
    temp = tempX * tempY;
    short8 reshi = as_short8(as_ushort8(temp) >> (short)8);
    $ReturnType res = as_$ReturnType(_mm_packus_epi16((__m128i)reslo, (__m128i)reshi));
    return res;
  }]>;
// mul_hi native uchar16
OclBuiltinImpl mul_hi_v16u8 = OclBuiltinImpl<mul_hi, [v16u8], 0, [{
    $Arg0Type zero = ($Arg0Type)0;
    ushort8 tempX = as_ushort8(_mm_unpacklo_epi8((__m128i)$Arg0VarName, (__m128i)zero));
    ushort8 tempY = as_ushort8(_mm_unpacklo_epi8((__m128i)$Arg1VarName, (__m128i)zero));
    ushort8 temp = tempX * tempY;
    ushort8 reslo = temp >> (short)8;
    tempX = as_ushort8(_mm_unpackhi_epi8((__m128i)$Arg0VarName, (__m128i)zero));
    tempY = as_ushort8(_mm_unpackhi_epi8((__m128i)$Arg1VarName, (__m128i)zero));
    temp = tempX * tempY;
    ushort8 reshi = temp >> (short)8;
    $ReturnType res = as_$ReturnType(_mm_packus_epi16((__m128i)reslo, (__m128i)reshi));
    return res;
  }]>;
// mul_hi native short8
OclBuiltinImpl mul_hi_v8i16 = OclBuiltinImpl<mul_hi, [v8i16] , 0, [{
    return as_short8(_mm_mulhi_epi16((__m128i)$Arg0VarName, (__m128i)$Arg1VarName));
  }]>;
// mul_hi native ushort8
OclBuiltinImpl mul_hi_v8u16 = OclBuiltinImpl<mul_hi, [v8u16] , 0, [{
    return as_ushort8(_mm_mulhi_epu16((__m128i)$Arg0VarName, (__m128i)$Arg1VarName));
  }]>;

/*****************************************************************\
 * ctz:                                                          *
 *   gentype ctz (gentype x)                                     *
 *   Returns the count of trailing 0-bits in x.                  *
\*****************************************************************/
// ctz on signed integer - call unsigned
OclBuiltinImpl ctz_igeneric = OclBuiltinImpl<ctz, [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                   v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                                   v1i64, v2i64, v3i64, v4i64, v8i64, v16i64], 0, [{
    return as_$ReturnType(ctz(as_u$Arg0Type($Arg0VarName)));
  }]>;
// ctz on unsigned integer for all vectors but 3 demensional
OclBuiltinImpl ctz_ugeneric = OclBuiltinImpl<ctz, [ v1u8,  v2u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v4u16, v8u16, v16u16,
                                                   v1u32, v2u32, v4u32, v8u32, v16u32,
                                                   v1u64, v2u64, v4u64, v8u64, v16u64], 0, [{
    return __ocl_helper_ctz_$Arg0Symgentype($Arg0VarName);
  }]>;
// ctz on unsigned 3 demensional vector promoted to 4 dementional
OclBuiltinImpl ctz_promote = OclBuiltinImpl<ctz, [v3u8, v3u16, v3u32, v3u64], 0, Promote1_v3_to_v4>;


