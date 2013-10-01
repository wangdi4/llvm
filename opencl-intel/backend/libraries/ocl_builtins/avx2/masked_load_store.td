// ****************************************************************************
//                                 char4x4
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_char_4x4_AVX2 = OclBuiltinImpl<__ocl_masked_load4x4, [v4i8],  0, [{
    char16* tm$Arg1VarName = (char16*)$Arg1VarName;
    *tm$Arg1VarName = as_char16(_mm_maskload_epi32((int*)$Arg0VarName, (__m128i)$Arg2VarName));
  }]>;

OclBuiltinImpl __ocl_masked_store_char_4x4_AVX2 = OclBuiltinImpl<__ocl_masked_store4x4, [v4i8],  0, [{
    _mm_maskstore_epi32((int*)$Arg0VarName, (__m128i) $Arg2VarName,*((__m128i*) $Arg1VarName));
  }]>;

// ****************************************************************************
//                                 char4x8
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_char_4x8_AVX2 = OclBuiltinImpl<__ocl_masked_load4x8, [v4i8],  0, [{
    ocl_char32* tm$Arg1VarName = (ocl_char32*)$Arg1VarName;
    *tm$Arg1VarName = (ocl_char32)_mm256_maskload_epi32((int*)$Arg0VarName, (__m256i)$Arg2VarName);
  }]>;

OclBuiltinImpl __ocl_masked_store_char_4x8_AVX2 = OclBuiltinImpl<__ocl_masked_store4x8, [v4i8],  0, [{
    _mm256_maskstore_epi32((int*)$Arg0VarName, (__m256i) $Arg2VarName,*((__m256i*) $Arg1VarName));
  }]>;

// ****************************************************************************
//                                 short4x4
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_short_4x4_AVX2 = OclBuiltinImpl<__ocl_masked_load4x4, [v4i16],  0, [{
    int8 extendedMask = $Arg2VarName.s00112233;

    short16* tm$Arg0VarName = (short16*)$Arg0VarName;
    short16* tm$Arg1VarName = (short16*)$Arg1VarName;
    *tm$Arg1VarName = as_short16( _mm256_maskload_epi32((int*)(tm$Arg0VarName), (__m256i)extendedMask) );
  }]>;

OclBuiltinImpl __ocl_masked_store_short_4x4_AVX2 = OclBuiltinImpl<__ocl_masked_store4x4, [v4i16],  0, [{
    int8 extendedMask = $Arg2VarName.s00112233;

    short16* tm$Arg0VarName = (short16*)$Arg0VarName;
    short16* tm$Arg1VarName = (short16*)$Arg1VarName;
    _mm256_maskstore_epi32((int*)(tm$Arg0VarName), (__m256i)extendedMask, *((__m256i*)tm$Arg1VarName));
  }]>;

// ****************************************************************************
//                                 short4x8
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_short_4x8_AVX2 = OclBuiltinImpl<__ocl_masked_load4x8, [v4i16],  0, [{
    int8 maskLow  = $Arg2VarName.s00112233;
    int8 maskHigh = $Arg2VarName.s44556677;

    short16* tm$Arg0VarName = (short16*)$Arg0VarName;
    short16* tm$Arg1VarName = (short16*)$Arg1VarName;
    tm$Arg1VarName[0] = as_short16( _mm256_maskload_epi32((int*)&(tm$Arg0VarName[0]), (__m256i)maskLow) );
    tm$Arg1VarName[1] = as_short16( _mm256_maskload_epi32((int*)&(tm$Arg0VarName[1]), (__m256i)maskHigh) );
  }]>;

OclBuiltinImpl __ocl_masked_store_short_4x8_AVX2 = OclBuiltinImpl<__ocl_masked_store4x8, [v4i16],  0, [{
    int8 maskLow  = $Arg2VarName.s00112233;
    int8 maskHigh = $Arg2VarName.s44556677;

    short16* tm$Arg0VarName = (short16*)$Arg0VarName;
    short16* tm$Arg1VarName = (short16*)$Arg1VarName;
    _mm256_maskstore_epi32((int*)&(tm$Arg0VarName[0]), (__m256i)maskLow,  (__m256i)tm$Arg1VarName[0]);
    _mm256_maskstore_epi32((int*)&(tm$Arg0VarName[1]), (__m256i)maskHigh, (__m256i)tm$Arg1VarName[1]);
  }]>;

// ****************************************************************************
//                                 int4x4
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_int_4x4_AVX2 = OclBuiltinImpl<__ocl_masked_load4x4, [v4i32],  0, [{
    int8 extendedMask = $Arg2VarName.s01230123;
    int8 maskLow = extendedMask.s00001111;
    int8 maskHigh = extendedMask.s22223333;

    int8* tm$Arg0VarName = (int8*)$Arg0VarName;
    int8* tm$Arg1VarName = (int8*)$Arg1VarName;
    tm$Arg1VarName[0] = as_int8(_mm256_maskload_epi32((int*)&(tm$Arg0VarName[0]), (__m256i)maskLow));
    tm$Arg1VarName[1] = as_int8(_mm256_maskload_epi32((int*)&(tm$Arg0VarName[1]), (__m256i)maskHigh));
  }]>;

OclBuiltinImpl __ocl_masked_store_int_4x4_AVX2 = OclBuiltinImpl<__ocl_masked_store4x4, [v4i32],  0, [{

    int8 extendedMask = $Arg2VarName.s01230123;
    int8 maskLow = extendedMask.s00001111;
    int8 maskHigh = extendedMask.s22223333;

    int8* tm$Arg0VarName = (int8*)$Arg0VarName;
    int8* tm$Arg1VarName = (int8*)$Arg1VarName;
    _mm256_maskstore_epi32((int*)&(tm$Arg0VarName[0]), (__m256i)maskLow, (__m256i)tm$Arg1VarName[0]);
    _mm256_maskstore_epi32((int*)&(tm$Arg0VarName[1]), (__m256i)maskHigh, (__m256i)tm$Arg1VarName[1]);
  }]>;

// ****************************************************************************
//                                 int4
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_int4_AVX2 = OclBuiltinImpl<__ocl_masked_load4, [v4i32],  0, [{
    return as_int4(_mm_maskload_epi32((int const *)$Arg0VarName, (__m128i) $Arg1VarName));
  }]>;

OclBuiltinImpl __ocl_masked_store_int4_AVX2 = OclBuiltinImpl<__ocl_masked_store4, [v4i32],  0, [{
    _mm_maskstore_epi32((int *)$Arg0VarName, (__m128i) $Arg2VarName, *((__m128i*) &$Arg1VarName));
  }]>;

// ****************************************************************************
//                                 int8
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_int8_AVX2 = OclBuiltinImpl<__ocl_masked_load8, [v8i32],  0, [{
    return as_int8(_mm256_maskload_epi32((int const *)$Arg0VarName, (__m256i) $Arg1VarName));
  }]>;

OclBuiltinImpl __ocl_masked_store_int8_AVX2 = OclBuiltinImpl<__ocl_masked_store8, [v8i32],  0, [{
    _mm256_maskstore_epi32((int *)$Arg0VarName, (__m256i) $Arg2VarName, *((__m256i*) &$Arg1VarName));
  }]>;

// ****************************************************************************
//                                 long4
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_long4_AVX2 = OclBuiltinImpl<__ocl_masked_load4, [v4i64],  0, [{
    long4 extMask = convert_long4($Arg1VarName);
    return as_long4(_mm256_maskload_epi64((const long long *)$Arg0VarName, (__m256i) extMask));
  }]>;

OclBuiltinImpl __ocl_masked_store_long4_AVX2 = OclBuiltinImpl<__ocl_masked_store4, [v4i64],  0, [{
    long4 extMask = convert_long4($Arg2VarName);
    _mm256_maskstore_epi64((long long *)$Arg0VarName, (__m256i) extMask, *((__m256i*) &$Arg1VarName));
  }]>;