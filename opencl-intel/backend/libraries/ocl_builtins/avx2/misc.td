// Copyright (c) 2006-2012 Intel Corporation
// Misc Built-In Functions
// AVX2


/*****************************************************************\
 * shuffle:                                                      *
 *   gentypen shuffle (gentypem x, ugentypen mask)               *
 *   The shuffle built-in function constructs a                  *
 *     permutation of elements from input vector, returning a    *
 *     vector with the same element type as the input and        *
 *     length that is the same as the shuffle mask.              *
\*****************************************************************/
// shuffle with mask size 2 - int8
OclBuiltinImpl shuffle_2_v8u32 = OclBuiltinImpl<shuffle_2, [v8u32], 0, [{
    uint8 tMask;
    tMask.s01 = $Arg1VarName;
    return shuffle(x, tMask).s01;
  }]>;
// shuffle with mask size 4 - int8
OclBuiltinImpl shuffle_4_v8u32 = OclBuiltinImpl<shuffle_4, [v8u32], 0, [{
    uint8 tMask;
    tMask.lo = $Arg1VarName;
    return shuffle($Arg0VarName, tMask).lo;
  }]>;
// shuffle with mask size 8 - int2
OclBuiltinImpl shuffle_8_v2u32 = OclBuiltinImpl<shuffle_8, [v2u32], 0, [{
    uint8 tx;
    tx.s01 = $Arg0VarName;
    return shuffle(tx, $Arg1VarName);
  }]>;
// shuffle with mask size 8 - int4
OclBuiltinImpl shuffle_8_v4u32 = OclBuiltinImpl<shuffle_8, [v4u32], 0, [{
    uint8 tx;
    tx.lo = $Arg0VarName;
    return shuffle(tx, $Arg1VarName);
  }]>;
// shuffle with mask size 8 - int8
OclBuiltinImpl shuffle_8_v8u32 = OclBuiltinImpl<shuffle_8, [v8u32], 0, [{
    return as_$ReturnType( _mm256_permutevar8x32_epi32((__m256i)$Arg0VarName, (__m256i)$Arg1VarName) );
  }]>;
// shuffle with mask size 16 - int4
OclBuiltinImpl shuffle_16_v4u32 = OclBuiltinImpl<shuffle_16, [v4u32], 0, [{
    $ReturnType res;
    uint8 tx;
    tx.lo = $Arg0VarName;
    res.lo = shuffle(tx, $Arg1VarName.lo);
    res.hi = shuffle(tx, $Arg1VarName.hi);
    return res;
  }]>;
// shuffle with mask size 16 - int8
OclBuiltinImpl shuffle_16_v8u32 = OclBuiltinImpl<shuffle_16, [v8u32], 0, [{
    $ReturnType res;
    res.lo = shuffle($Arg0VarName, $Arg1VarName.lo);
    res.hi = shuffle($Arg0VarName, $Arg1VarName.hi);
    return res;
  }]>;
// shuffle with mask size 4 - long4
OclBuiltinImpl shuffle_4_v4u64 = OclBuiltinImpl<shuffle_4, [v4u64], 0, [{
    $ReturnType res;
    uint8 tmaskhi;
    uint8 tmask;
    uint8 tmasklo = as_uint8($Arg1VarName);
    tmasklo = tmasklo + tmasklo;
    tmaskhi = tmasklo + 1;
    tmaskhi = as_uint8( as_ulong4(tmaskhi) << (ulong4)32 );
    tmask = tmasklo | tmaskhi;
    res = as_ulong4( _mm256_permutevar8x32_epi32( (__m256i) $Arg0VarName, (__m256i) tmask) );
    return res;
  }]>;
// shuffle with mask size 8 - long4
OclBuiltinImpl shuffle_8_v4u64 = OclBuiltinImpl<shuffle_8, [v4u64], 0, [{
    $ReturnType res;
    res.lo = shuffle ($Arg0VarName, $Arg1VarName.lo);
    res.hi = shuffle ($Arg0VarName, $Arg1VarName.hi);
    return res;
  }]>;
// shuffle with mask size 16 - long4
OclBuiltinImpl shuffle_16_v4u64 = OclBuiltinImpl<shuffle_16, [v4u64], 0, [{
    $ReturnType res;
    res.lo = shuffle ($Arg0VarName, $Arg1VarName.lo);
    res.hi = shuffle ($Arg0VarName, $Arg1VarName.hi);
    return res;
  }]>;
// shuffle with mask size 4 - float8
OclBuiltinImpl shuffle_4_v8f32 = OclBuiltinImpl<shuffle_4, [v8f32], 0, [{
    uint8 tmask = (uint8)0;
    float8 res;
    tmask.lo = $Arg1VarName;
    res = as_float8( _mm256_permutevar8x32_ps ((__m256) $Arg0VarName, (__m256)tmask) );
    return res.lo;
  }]>;
// shuffle with mask size 8 - float8
OclBuiltinImpl shuffle_8_v8f32 = OclBuiltinImpl<shuffle_8, [v8f32], 0, [{
    return as_float8( _mm256_permutevar8x32_ps ((__m256) $Arg0VarName, (__m256)$Arg1VarName) );
  }]>;
// shuffle with mask size 16 - float8
OclBuiltinImpl shuffle_16_v8f32 = OclBuiltinImpl<shuffle_16, [v8f32], 0, [{
    float16 res;
    res.lo = as_float8( shuffle($Arg0VarName, $Arg1VarName.lo) );
    res.hi = as_float8( shuffle($Arg0VarName, $Arg1VarName.hi) );
    return res; 
  }]>;

