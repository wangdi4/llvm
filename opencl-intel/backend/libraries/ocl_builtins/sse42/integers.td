// Copyright (c) 2006-2012 Intel Corporation
// Integers Built-In Functions
// SSE42

/*****************************************************************\
 * mul_hi:                                                       *
 *   gentype mul_hi (gentype x , gentype y)                      *
 *   Returns the high half of the product of (x * y)             *
\*****************************************************************/
// mul_hi native int4
OclBuiltinImpl mul_hi_v4i32 = OclBuiltinImpl<mul_hi, [v4i32] , 0, [{
    ulong2 reslo = as_ulong2(_mm_mul_epi32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName));
    reslo = reslo >> (ulong)32;
    $Arg0VarName = as_$Arg0Type(as_ulong2($Arg0VarName) >> (ulong)32);
    $Arg1VarName = as_$Arg1Type(as_ulong2($Arg1VarName) >> (ulong)32);
    ulong2 reshi = as_ulong2(_mm_mul_epi32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName));
    $ReturnType res = as_$ReturnType(_mm_blend_epi16 ((__m128i)reslo, (__m128i)reshi, 0b11001100));
    return res;
  }]>;

// mul_hi native uint4
OclBuiltinImpl mul_hi_v4u32 = OclBuiltinImpl<mul_hi, [v4u32] , 0, [{
    ulong2 reslo = as_ulong2(_mm_mul_epu32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName));
    reslo = reslo >> (ulong)32;
    $Arg0VarName = as_$Arg0Type(as_ulong2($Arg0VarName) >> (ulong)32);
    $Arg1VarName = as_$Arg1Type(as_ulong2($Arg1VarName) >> (ulong)32);
    ulong2 reshi = as_ulong2(_mm_mul_epu32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName));
    $ReturnType res = as_$ReturnType(_mm_blend_epi16 ((__m128i)reslo, (__m128i)reshi, 0b11001100));
    return res;
  }]>;

/*****************************************************************\
 * mad_sat:                                                      *
 *   gentype mad_sat (gentype x , gentype y , gentype z)         *
 *   Returns a * b + c and saturates the result.                 *
\*****************************************************************/
// mad_sat native int4
OclBuiltinImpl mad_sat_v4i32 = OclBuiltinImpl<mad_sat, [v4i32] , 0, [{
    int4 loz = $Arg2VarName & as_int4((long2)long_even_mask);
    int4 reslo = as_int4(_mm_mul_epi32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName));
    reslo = reslo + loz;
    reslo = as_int4(max(as_long2(reslo), (long)generic_min_int));
    reslo = as_int4(min(as_long2(reslo), (long)generic_max_int));
    reslo = reslo & as_int4((long2)long_even_mask);
    $Arg0VarName = as_int4(as_long2($Arg0VarName) >> 32);
    $Arg1VarName = as_int4(as_long2($Arg1VarName) >> 32);
    $Arg2VarName = as_int4(as_long2($Arg2VarName) >> 32);
    int4 reshi = as_int4(_mm_mul_epi32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName));
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
    uint4 reslo = as_uint4(_mm_mul_epi32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName));
    reslo = reslo + loz;
    reslo = as_uint4(min(as_ulong2(reslo), (ulong)generic_max_uint));
    reslo = reslo & as_uint4((ulong2)long_even_mask);
    $Arg0VarName = as_uint4(as_ulong2($Arg0VarName) >> 32);
    $Arg1VarName = as_uint4(as_ulong2($Arg1VarName) >> 32);
    $Arg2VarName = as_uint4(as_ulong2($Arg2VarName) >> 32);
    uint4 reshi = as_uint4(_mm_mul_epi32((__m128i)$Arg0VarName, (__m128i)$Arg1VarName));
    reshi = reshi + $Arg2VarName;
    reshi = as_uint4(min(as_ulong2(reshi), (ulong)generic_max_uint));
    reshi = as_uint4(as_ulong2(reshi) << 32);
    uint4 res = reshi | reslo;
    return res;
  }]>;

// mad_sat native ushort8
OclBuiltinImpl mad_sat_v8u16 = OclBuiltinImpl<mad_sat, [v8u16] , 0, [{
    ushort8 zero = (ushort8)0;
    uint4 lox = as_uint4(_mm_unpacklo_epi16((__m128i)$Arg0VarName, (__m128i)zero));
    uint4 loy = as_uint4(_mm_unpacklo_epi16((__m128i)$Arg1VarName, (__m128i)zero));
    uint4 loz = as_uint4(_mm_unpacklo_epi16((__m128i)$Arg2VarName, (__m128i)zero));
    uint4 reslo = lox * loy;
    reslo = reslo + loz;
    reslo = min(reslo, (uint4)generic_max_ushort);
    uint4 hix = as_uint4(_mm_unpackhi_epi16((__m128i)$Arg0VarName, (__m128i)zero));
    uint4 hiy = as_uint4(_mm_unpackhi_epi16((__m128i)$Arg1VarName, (__m128i)zero));
    uint4 hiz = as_uint4(_mm_unpackhi_epi16((__m128i)$Arg2VarName, (__m128i)zero));
    uint4 reshi = hix * hiy;
    reshi = reshi + hiz;
    reshi = min(reshi, (uint4)generic_max_ushort);
    ushort8 res = as_ushort8(_mm_packus_epi32((__m128i)reslo, (__m128i)reshi));
    return res;
  }]>;
  
  /*****************************************************************\
 * max:                                                          *
 *   gentype max (gentype x , gentype y)                         *
 *   gentype max (gentype x , sgentype y)                        *
 *   Returns y if x < y, otherwise it returns x                  *
\*****************************************************************/
// max native char16, short8, int4, uchar16, ushort8, uint4
OclBuiltinImpl max_native_i = OclBuiltinImpl<max, [v16i8, v8i16, v4i32, v16u8, v8u16, v4u32], 0, [{
    $ReturnType Mask = as_$ReturnType($Arg0VarName < $Arg1VarName);
    return select($Arg0VarName, $Arg1VarName, Mask);
  }]>;
  
/*****************************************************************\
 * min:                                                          *
 *   gentype min (gentype x , gentype y)                         *
 *   gentype min (gentype x , sgentype y)                        *
 *   Returns y if y < x, otherwise it returns x                  *
\*****************************************************************/
  // min native char16, short8, int4, uchar16, ushort8, uint4
OclBuiltinImpl min_native_i = OclBuiltinImpl<min, [v16i8, v8i16, v4i32, uchar16, ushort8, uint4], 0, [{
    $ReturnType Mask = as_$ReturnType($Arg0VarName > $Arg1VarName);
    return select($Arg0VarName, $Arg1VarName, Mask);
  }]>;

