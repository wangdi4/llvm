// Copyright (c) 2006 Intel Corporation
// Miscellaneous Built-In Functions
// AVX2


/*****************************************************************\
 * shuffle:                                                      *
 *   gentypen shuffle (gentypem x, ugentypen mask)               *
 *   The shuffle built-in function constructs a                  *
 *     permutation of elements from input vector, returning a    *
 *     vector with the same element type as the input and        *
 *     length that is the same as the shuffle mask.              *
\*****************************************************************/
// shuffle with mask size 4 - long4
OclBuiltinImpl avx2_shuffle_4_v4u64 = OclBuiltinImpl<shuffle_4, [v4u64], 0, [{
    uint8 zeroOne = (uint8)(0,1,0,1,0,1,0,1);
    uint8 tmasklo = as_uint8($Arg1VarName) << (uint8)1;
    uint8 tmaskhi = as_uint8($Arg1VarName << ($Arg1Type)33) | zeroOne;
    uint8 tmask   = tmasklo | tmaskhi;
    return as_$ReturnType( shuffle(as_uint8($Arg0VarName), tmask) );
  }]>;
// shuffle with mask size 8 - int8
OclBuiltinImpl avx2_shuffle_8_v8u32 = OclBuiltinImpl<shuffle_8, [v8u32], 0, [{
    return as_$ReturnType( _mm256_permutevar8x32_epi32((__m256i)$Arg0VarName, (__m256i)$Arg1VarName) );
  }]>;
