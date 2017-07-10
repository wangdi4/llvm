// Copyright (c) 2006-2012 Intel Corporation
// Integers Built-In Functions
// AVX2

/*****************************************************************\
 * abs:                                                          *
 *   ugentype abs (gentype x)                                    *
 *   Returns | x |                                               *
\*****************************************************************/
// abs native
OclBuiltinImpl abs_avx2native = OclBuiltinImpl<abs, [v16i16, v8i32] , 0, [{
    return as_u$Arg0Type(_mm256_abs_epi$BitWidth((__m256i)$Arg0VarName));
  }]>;


/*****************************************************************\
 * add_sat:                                                      *
 *   gentype add_sat (gentype x, gentype y)                      *
 *   Returns x + y saturated                                     *
\*****************************************************************/
// add_sat on short16
OclBuiltinImpl add_sat_v16i8v8i16 = OclBuiltinImpl<add_sat, [v16i16], 0, [{
    return as_$ReturnType(_mm256_adds_epi16((__m256i)$Arg0VarName, (__m256i)$Arg1VarName));
  }]>;
// add_sat on ushort16
OclBuiltinImpl add_sat_v16u8v8u16 = OclBuiltinImpl<add_sat, [v16u16], 0, [{
    return as_$ReturnType(_mm256_adds_epu16((__m256i)$Arg0VarName, (__m256i)$Arg1VarName));
  }]>;


/*****************************************************************\
 * rhadd:                                                        *
 *   gentype rhadd (gentype x, gentype y)                        *
 *   Returns (x + y + 1) >> 1. The intermediate sum does         *
 *     not modulo overflow.                                      *
\*****************************************************************/
// rhadd fix for regression on int16
OclBuiltinImpl rhadd_16i32 = OclBuiltinImpl<rhadd, [v16i32] , 0, [{
    int16 LSB_mask_int16 = (int16)LSB_mask;
    int16 lsbx = $Arg0VarName & LSB_mask_int16;
    int16 lsby = $Arg1VarName & LSB_mask_int16;
    int16 lsb  = lsbx | lsby;
    $Arg0VarName = $Arg0VarName >> 1;
    $Arg1VarName = $Arg1VarName >> 1;
    return $Arg0VarName + $Arg1VarName + lsb;
  }]>;


/*****************************************************************\
 * mad_sat:                                                      *
 *   gentype mad_sat (gentype x , gentype y , gentype z)         *
 *   Returns min(max(x, y), z)                                   *
\*****************************************************************/
// mad_sat native ushort16
OclBuiltinImpl mad_sat_v16u16 = OclBuiltinImpl<mad_sat, [v16u16] , 0, [{
    $Arg0Type zero = ( $Arg0Type)0;
    uint8 lox = as_uint8(_mm256_unpacklo_epi16((__m256i)$Arg0VarName, (__m256i)zero));
    uint8 loy = as_uint8(_mm256_unpacklo_epi16((__m256i)$Arg1VarName, (__m256i)zero));
    uint8 loz = as_uint8(_mm256_unpacklo_epi16((__m256i)$Arg2VarName, (__m256i)zero));
    uint8 reslo = lox * loy;
    reslo = reslo + loz;
    reslo = min(reslo, (uint8)generic_max_ushort);
    uint8 hix = as_uint8(_mm256_unpackhi_epi16((__m256i)$Arg0VarName, (__m256i)zero));
    uint8 hiy = as_uint8(_mm256_unpackhi_epi16((__m256i)$Arg1VarName, (__m256i)zero));
    uint8 hiz = as_uint8(_mm256_unpackhi_epi16((__m256i)$Arg2VarName, (__m256i)zero));
    uint8 reshi = hix * hiy;
    reshi = reshi + hiz;
    reshi = min(reshi, (uint8)generic_max_ushort);
    $ReturnType res = as_$ReturnType(_mm256_packus_epi32((__m256i)reslo, (__m256i)reshi));
    return res;
  }]>;
// mad_sat native int8
OclBuiltinImpl mad_sat_v8i32 = OclBuiltinImpl<mad_sat, [v8i32] , 0, [{
    $Arg0Type loz = $Arg2VarName & as_$Arg0Type((long4)long_even_mask);
    $Arg0Type reslo = as_$Arg0Type(_mm256_mul_epi32((__m256i)$Arg0VarName, (__m256i)$Arg1VarName));
    reslo = reslo + loz;
    reslo = as_$Arg0Type(max(as_long4(reslo), (long)generic_min_int));
    reslo = as_$Arg0Type(min(as_long4(reslo), (long)generic_max_int));
    reslo = reslo & as_$Arg0Type((long4)long_even_mask);
    $Arg0VarName = as_$Arg0Type(as_long4($Arg0VarName) >> 32);
    $Arg1VarName = as_$Arg0Type(as_long4($Arg1VarName) >> 32);
    $Arg2VarName = as_$Arg0Type(as_long4($Arg2VarName) >> 32);
    $Arg0Type reshi = as_$Arg0Type(_mm256_mul_epi32((__m256i)$Arg0VarName, (__m256i)$Arg1VarName));
    reshi = reshi + $Arg2VarName;
    reshi = as_$Arg0Type(max(as_long4(reshi), (long)generic_min_int));
    reshi = as_$Arg0Type(min(as_long4(reshi), (long)generic_max_int));
    reshi = as_$Arg0Type(as_long4(reshi) << 32);
    return reshi | reslo;
  }]>;
// mad_sat native uint8
OclBuiltinImpl mad_sat_v8u32 = OclBuiltinImpl<mad_sat, [v8u32] , 0, [{
    $Arg0Type loz = $Arg2VarName & as_$Arg0Type((ulong4)long_even_mask);
    $Arg0Type reslo = as_$Arg0Type(_mm256_mul_epu32((__m256i)$Arg0VarName, (__m256i)$Arg1VarName));
    reslo = reslo + loz;
    reslo = as_$Arg0Type(min(as_ulong4(reslo), (ulong)generic_max_uint));
    reslo = reslo & as_$Arg0Type((ulong4)long_even_mask);
    $Arg0VarName = as_$Arg0Type(as_ulong4($Arg0VarName) >> 32);
    $Arg1VarName = as_$Arg0Type(as_ulong4($Arg1VarName) >> 32);
    $Arg2VarName = as_$Arg0Type(as_ulong4($Arg2VarName) >> 32);
    $Arg0Type reshi = as_$Arg0Type(_mm256_mul_epu32((__m256i)$Arg0VarName, (__m256i)$Arg1VarName));
    reshi = reshi + $Arg2VarName;
    reshi = as_$Arg0Type(min(as_ulong4(reshi), (ulong)generic_max_uint) << 32);
    return reshi | reslo;
  }]>;


/*****************************************************************\
 * max:                                                          *
 *   gentype max (gentype x , gentype y)                         *
 *   gentype max (gentype x , sgentype y)                        *
 *   Returns y if x < y, otherwise it returns x                  *
\*****************************************************************/
// max native short16, int8
OclBuiltinImpl max_avx2native_i = OclBuiltinImpl<max, [v16i16, v8i32] , 0, [{
    $ReturnType Mask = $Arg0VarName < $Arg1VarName;
    return as_$ReturnType(select($Arg0VarName, $Arg1VarName, Mask));
  }]>;
// max native ushort16, uint8
OclBuiltinImpl max_avx2native_u = OclBuiltinImpl<max, [v16u16, v8u32] , 0, [{
     $ReturnType Mask = as_$ReturnType($Arg0VarName < $Arg1VarName);
     return as_$ReturnType(select($Arg0VarName, $Arg1VarName, Mask)); 
  }]>;


/*****************************************************************\
 * min:                                                          *
 *   gentype min (gentype x , gentype y)                         *
 *   gentype min (gentype x , sgentype y)                        *
 *   Returns y if x < y, otherwise it returns x                  *
\*****************************************************************/
// min native short16, int8
OclBuiltinImpl min_avx2native_i = OclBuiltinImpl<min, [v16i16, v8i32] , 0, [{
     $ReturnType Mask = $Arg0VarName > $Arg1VarName;
     return as_$ReturnType(select($Arg0VarName, $Arg1VarName, Mask));
  }]>;
// min native ushort16, uint8
OclBuiltinImpl min_avx2native_u = OclBuiltinImpl<min, [v16u16, v8u32] , 0, [{
     $ReturnType Mask = as_$ReturnType($Arg0VarName > $Arg1VarName);
     return as_$ReturnType(select($Arg0VarName, $Arg1VarName, Mask));
  }]>;


/*****************************************************************\
 * mul_hi                                                        *
 *   gentype mul_hi (gentype x , gentype y)                      *
 *   Returns the high half of the product of (x * y)             *
\*****************************************************************/
// mul_hi native short16
OclBuiltinImpl mul_hi_v16i16 = OclBuiltinImpl<mul_hi, [v16i16] , 0, [{
    return as_$ReturnType(_mm256_mulhi_epi16((__m256i)$Arg0VarName, (__m256i)$Arg1VarName));
  }]>;
// mul_hi native ushort16
OclBuiltinImpl mul_hi_v16u16 = OclBuiltinImpl<mul_hi, [v16u16] , 0, [{
    return as_$ReturnType(_mm256_mulhi_epu16((__m256i)$Arg0VarName, (__m256i)$Arg1VarName));
  }]>;
// mul_hi native int8
OclBuiltinImpl mul_hi_v8i32 = OclBuiltinImpl<mul_hi, [v8i32] , 0, [{
    ulong4 reslo = as_ulong4(_mm256_mul_epi32((__m256i)$Arg0VarName, (__m256i)$Arg1VarName));
    reslo = reslo >> (ulong)32;
    $Arg0VarName = as_$Arg0Type(as_ulong4($Arg0VarName) >> (ulong)32);
    $Arg1VarName = as_$Arg1Type(as_ulong4($Arg1VarName) >> (ulong)32);
    ulong4 reshi = as_ulong4(_mm256_mul_epi32((__m256i)$Arg0VarName, (__m256i)$Arg1VarName));
    $ReturnType res = as_$ReturnType(_mm256_blend_epi32((__m256i)reslo, (__m256i)reshi, 0b10101010));
    return res;
  }]>;
// mul_hi native uint8
OclBuiltinImpl mul_hi_v8u32 = OclBuiltinImpl<mul_hi, [v8u32] , 0, [{
    ulong4 reslo = as_ulong4(_mm256_mul_epu32((__m256i)$Arg0VarName, (__m256i)$Arg1VarName));
    reslo = reslo >> (ulong)32;
    $Arg0VarName = as_$Arg0Type(as_ulong4($Arg0VarName) >> (ulong)32);
    $Arg1VarName = as_$Arg1Type(as_ulong4($Arg1VarName) >> (ulong)32);
    ulong4 reshi = as_ulong4(_mm256_mul_epu32((__m256i)$Arg0VarName, (__m256i)$Arg1VarName));
    $ReturnType res = as_$ReturnType(_mm256_blend_epi32((__m256i)reslo, (__m256i)reshi, 0b10101010));
    return res;
  }]>;
// mul_hi fix for regression on ulong16
OclBuiltinImpl mul_hi_v2u64 = OclBuiltinImpl<mul_hi, [v4u64] , 0, [{
    __attribute__((aligned(16))) ulong tempX[4];
    __attribute__((aligned(16))) ulong tempY[4];
    ulong destLow;
    ulong destHi;

    // Store to temporary buffer
    _mm256_store_si256((__m256i *)tempX,(__m256i) $Arg0VarName);
    _mm256_store_si256((__m256i *)tempY,(__m256i) $Arg1VarName);
    #pragma ivdep
    for(int i=0; i < 4; ++i)
    {
      multiply_unsigned_64_by_64( tempX[i], tempY[i], &destLow, &destHi );
      tempX[i] = destHi;
    }

    return as_ulong4(_mm256_load_si256((const __m256i *)tempX));
  }]>;


/*****************************************************************\
 * sub_sat:                                                      *
 *   gentype sub_sat (gentype x, gentype y)                      *
 *   Returns x - y saturated                                     *
\*****************************************************************/
// sub_sat on short16
OclBuiltinImpl sub_sat_v16i8v8i16 = OclBuiltinImpl<sub_sat, [v16i16], 0, [{
    return as_$ReturnType(_mm256_subs_epi16((__m256i)$Arg0VarName, (__m256i)$Arg1VarName));
  }]>;
// sub_sat on ushort16
OclBuiltinImpl sub_sat_v16u8v8u16 = OclBuiltinImpl<sub_sat, [v16u16], 0, [{
    return as_$ReturnType(_mm256_subs_epu16((__m256i)$Arg0VarName, (__m256i)$Arg1VarName));
  }]>;

