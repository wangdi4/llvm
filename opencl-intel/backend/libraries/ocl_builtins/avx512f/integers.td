// Copyright (c) 2006 Intel Corporation
// Integers Built-In Functions
// AVX-512

/*****************************************************************\
 * abs:                                                          *
 *   ugentype abs (gentype x)                                    *
 *   Returns | x |                                               *
\*****************************************************************/
// abs native
OclBuiltinImpl abs_avx512_native = OclBuiltinImpl<abs, [v16i32, v8i64] , 0, [{
    return as_u$Arg0Type(_mm512_abs_epi$BitWidth((__m512i)$Arg0VarName));
  }]>;

/*****************************************************************\
 * abs_diff:                                                     *
 *   ugentype abs_diff (gentype x, gentype y)                    *
 *   Returns | x - y |                                           *
\*****************************************************************/
OclBuiltinImpl abs_diff_iavx512 = OclBuiltinImpl<abs_diff,
               [v16i32, v8i64, v16u32, v8u64], 0, [{
    return as_$ReturnType(($Arg0Type)(max($Arg0VarName, $Arg1VarName)
      - min($Arg0VarName, $Arg1VarName)));
  }]>;

/*****************************************************************\
 * mad_sat:                                                      *
 *   gentype mad_sat (gentype x , gentype y , gentype z)         *
 *   Returns x * y + z and saturates the result.                 *
\*****************************************************************/
// mad_sat native int16
OclBuiltinImpl mad_sat_v16i32 = OclBuiltinImpl<mad_sat, [v16i32] , 0, [{
    $Arg0Type loz = $Arg2VarName & as_$Arg0Type((long8)long_even_mask);
    $Arg0Type reslo = as_$Arg0Type(_mm512_mul_epi32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
    reslo = reslo + loz;
    reslo = as_$Arg0Type(max(as_long8(reslo), (long)generic_min_int));
    reslo = as_$Arg0Type(min(as_long8(reslo), (long)generic_max_int));
    reslo = reslo & as_$Arg0Type((long8)long_even_mask);
    $Arg0VarName = as_$Arg0Type(as_long8($Arg0VarName) >> 32);
    $Arg1VarName = as_$Arg0Type(as_long8($Arg1VarName) >> 32);
    $Arg0Type hiz = as_$Arg0Type(_mm512_srli_epi64((__m512i)$Arg2VarName, 32));
    $Arg0Type reshi = as_$Arg0Type(_mm512_mul_epi32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
    reshi = reshi + hiz;
    reshi = as_$Arg0Type(max(as_long8(reshi), (long)generic_min_int));
    reshi = as_$Arg0Type(min(as_long8(reshi), (long)generic_max_int));
    reshi = as_$Arg0Type(as_long8(reshi) << 32);
    return reshi | reslo;
  }]>;
  
// mad_sat native uint8
OclBuiltinImpl mad_sat_v16u32 = OclBuiltinImpl<mad_sat, [v16u32] , 0, [{
    $Arg0Type loz = $Arg2VarName & as_$Arg0Type((ulong8)long_even_mask);
    $Arg0Type reslo = as_$Arg0Type(_mm512_mul_epu32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
    reslo = reslo + loz;
    reslo = as_$Arg0Type(min(as_ulong8(reslo), (ulong)generic_max_uint));
    reslo = reslo & as_$Arg0Type((ulong8)long_even_mask);
    $Arg0VarName = as_$Arg0Type(as_ulong8($Arg0VarName) >> 32);
    $Arg1VarName = as_$Arg0Type(as_ulong8($Arg1VarName) >> 32);
    $Arg2VarName = as_$Arg0Type(as_ulong8($Arg2VarName) >> 32);
    $Arg0Type reshi = as_$Arg0Type(_mm512_mul_epu32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
    reshi = reshi + $Arg2VarName;
    reshi = as_$Arg0Type(min(as_ulong8(reshi), (ulong)generic_max_uint) << 32);
    return reshi | reslo;
  }]>;


/*****************************************************************\
 * mul_hi                                                        *
 *   gentype mul_hi (gentype x , gentype y)                      *
 *   Returns the high half of the product of (x * y)             *
\*****************************************************************/
// mul_hi native int16
OclBuiltinImpl mul_hi_v16i32 = OclBuiltinImpl<mul_hi, [v16i32] , 0, [{
    ulong8 reslo = as_ulong8(_mm512_mul_epi32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
    reslo = reslo >> (ulong)32;
    $Arg0VarName = as_$Arg0Type(as_ulong8($Arg0VarName) >> (ulong)32);
    $Arg1VarName = as_$Arg1Type(as_ulong8($Arg1VarName) >> (ulong)32);
    ulong8 reshi = as_ulong8(_mm512_mul_epi32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
    $ReturnType res = as_$ReturnType(_mm512_mask_blend_epi32((__mmask16)0b1010101010101010, (__m512i)reslo, (__m512i)reshi));
    return res;
  }]>;
// mul_hi native uint16
OclBuiltinImpl mul_hi_v16u32 = OclBuiltinImpl<mul_hi, [v16u32] , 0, [{
    ulong8 reslo = as_ulong8(_mm512_mul_epu32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
    reslo = reslo >> (ulong)32;
    $Arg0VarName = as_$Arg0Type(as_ulong8($Arg0VarName) >> (ulong)32);
    $Arg1VarName = as_$Arg1Type(as_ulong8($Arg1VarName) >> (ulong)32);
    ulong8 reshi = as_ulong8(_mm512_mul_epu32((__m512i)$Arg0VarName, (__m512i)$Arg1VarName));
    $ReturnType res = as_$ReturnType(_mm512_mask_blend_epi32((__mmask16)0b1010101010101010, (__m512i)reslo, (__m512i)reshi));
    return res;
  }]>;

 
// mul_hi for ulong8
OclBuiltinImpl mul_hi_v8u64 = OclBuiltinImpl<mul_hi, [v8u64] , 0, [{
    __attribute__((aligned(64))) ulong tempX[8];
    __attribute__((aligned(64))) ulong tempY[8];
    ulong destLow;
    ulong destHi;

    // Store to temporary buffer
    *(__m512i *) tempX = (__m512i) $Arg0VarName;
    *(__m512i *) tempY = (__m512i) $Arg1VarName;
#if defined(__GNUC__)
#pragma GCC ivdep
#elif defined(__clang__)
#pragma clang loop vectorize(enable)
#endif
    for(int i=0; i < 8; ++i)
    {
      multiply_unsigned_64_by_64( tempX[i], tempY[i], &destLow, &destHi );
      tempX[i] = destHi;
    }

    return as_ulong8(*(__m512i *)tempX);
  }]>;

// mul_hi for long8
OclBuiltinImpl mul_hi_v8i64 = OclBuiltinImpl<mul_hi, [v8i64] , 0, [{
    __attribute__((aligned(64))) ulong tempX[8];
    __attribute__((aligned(64))) ulong tempY[8];
    ulong destLow;
    ulong destHi;

    // Store to temporary buffer
    *(__m512i *) tempX = (__m512i) $Arg0VarName;
    *(__m512i *) tempY = (__m512i) $Arg1VarName;
#if defined(__GNUC__)
#pragma GCC ivdep
#elif defined(__clang__)
#pragma clang loop vectorize(enable)
#endif
    for(int i=0; i < 8; ++i)
    {
      multiply_signed_64_by_64( tempX[i], tempY[i], &destLow, &destHi );
      tempX[i] = destHi;
    }
    return as_long8(*(__m512i *)tempX);
  }]>;
