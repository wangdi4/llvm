// ****************************************************************************
//                                 char4x4
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_char_4x4_AVX = OclBuiltinImpl<__ocl_masked_load4x4, [v4i8], 0, [{
    __private char16* tmpLoadedValues = (__private char16*)$Arg1VarName;
    *tmpLoadedValues = as_char16(_mm_maskload_ps((float*)$Arg0VarName, (__m128i)$Arg2VarName));
  }]>;

OclBuiltinImpl __ocl_masked_store_char_4x4_AVX = OclBuiltinImpl<__ocl_masked_store4x4, [v4i8], 0, [{
    _mm_maskstore_ps((float*)$Arg0VarName, (__m128i) $Arg2VarName,*((__m128*) $Arg1VarName));
  }]>;

// ****************************************************************************
//                                 char4x8
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_char_4x8_AVX = OclBuiltinImpl<__ocl_masked_load4x8, [v4i8], 0, [{
    float8 res = as_float8(_mm256_maskload_ps((float*)$Arg0VarName, (__m256i)$Arg2VarName));
    __private char16* tmpLoadedValues = (__private char16*)$Arg1VarName;
    tmpLoadedValues[0] = as_char16(res.lo);
    tmpLoadedValues[1] = as_char16(res.hi);
  }]>;

OclBuiltinImpl __ocl_masked_store_char_4x8_AVX = OclBuiltinImpl<__ocl_masked_store4x8, [v4i8], 0, [{
    _mm256_maskstore_ps((float*)$Arg0VarName, (__m256i) $Arg2VarName,*((__m256*) $Arg1VarName));
  }]>;

// ****************************************************************************
//                                 short4x4
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_short_4x4_AVX = OclBuiltinImpl<__ocl_masked_load4x4, [v4i16], 0, [{
    int8 extendedMask = $Arg2VarName.s00112233;

    __private short16* tmpLoadAdd = (__private short16*)$Arg0VarName;
    __private short16* tmpLoadedValues = (__private short16*)$Arg1VarName;
    *tmpLoadedValues = as_short16( _mm256_maskload_ps((float*)(tmpLoadAdd), (__m256i)extendedMask) );
  }]>;

OclBuiltinImpl __ocl_masked_store_short_4x4_AVX = OclBuiltinImpl<__ocl_masked_store4x4, [v4i16], 0, [{
    int8 extendedMask = $Arg2VarName.s00112233;

    __private short16* tmpStoreAdd = (__private short16*)$Arg0VarName;
    __private short16* tmpValuesToStore = (__private short16*)$Arg1VarName;
    _mm256_maskstore_ps((float*)(tmpStoreAdd), (__m256i)extendedMask, *((__m256*)tmpValuesToStore));
  }]>;

// ****************************************************************************
//                                 short4x8
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_short_4x8_AVX = OclBuiltinImpl<__ocl_masked_load4x8, [v4i16], 0, [{
    int8 maskLow  = $Arg2VarName.s00112233;
    int8 maskHigh = $Arg2VarName.s44556677;

    __private short16* tmpLoadAdd = (__private short16*)$Arg0VarName;
    __private short16* tmpLoadedValues = (__private short16*)$Arg1VarName;
    tmpLoadedValues[0] = as_short16( _mm256_maskload_ps((float*)&(tmpLoadAdd[0]), (__m256i)maskLow) );
    tmpLoadedValues[1] = as_short16( _mm256_maskload_ps((float*)&(tmpLoadAdd[1]), (__m256i)maskHigh) );
  }]>;

OclBuiltinImpl __ocl_masked_store_short_4x8_AVX = OclBuiltinImpl<__ocl_masked_store4x8, [v4i16], 0, [{
    int8 maskLow  = $Arg2VarName.s00112233;
    int8 maskHigh = $Arg2VarName.s44556677;

    __private short16* tmpStoreAdd = (__private short16*)$Arg0VarName;
    __private short16* tmpValuesToStore = (__private short16*)$Arg1VarName;
    _mm256_maskstore_ps((float*)&(tmpStoreAdd[0]), (__m256i)maskLow,  (__m256)tmpValuesToStore[0]);
    _mm256_maskstore_ps((float*)&(tmpStoreAdd[1]), (__m256i)maskHigh, (__m256)tmpValuesToStore[1]);
  }]>;

// ****************************************************************************
//                                 int4x4
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_int_4x4_AVX = OclBuiltinImpl<__ocl_masked_load4x4, [v4i32], 0, [{
    float8 extendedMask = as_float8($Arg2VarName.s01230123);
    float8 maskLow = extendedMask.s00001111;
    float8 maskHigh = extendedMask.s22223333;
    __private float8* tmpLoadAdd = (__private float8*)$Arg0VarName;
    __private float8* tmpLoadedValues = (__private float8*)$Arg1VarName;
    tmpLoadedValues[0] = as_float8(_mm256_maskload_ps((float*)&(tmpLoadAdd[0]), (__m256i)maskLow));
    tmpLoadedValues[1] = as_float8(_mm256_maskload_ps((float*)&(tmpLoadAdd[1]), (__m256i)maskHigh));
  }]>;

OclBuiltinImpl __ocl_masked_store_int_4x4_AVX = OclBuiltinImpl<__ocl_masked_store4x4, [v4i32], 0, [{
    __ocl_masked_store_float_4x4((__private float4*)$Arg0VarName, (__private float4*)$Arg1VarName, $Arg2VarName);
  }]>;

// ****************************************************************************
//                            int4x8 & float4x8
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_int_float_4x8_AVX = OclBuiltinImpl<__ocl_masked_load4x8, [v4i32, v4f32], 0, [{
    __ocl_masked_load_$Arg0BaseType_4x4(&($Arg0VarName[0]), &($Arg1VarName[0]), $Arg2VarName.lo);
    __ocl_masked_load_$Arg0BaseType_4x4(&($Arg0VarName[4]), &($Arg1VarName[4]), $Arg2VarName.hi);
  }]>;

OclBuiltinImpl __ocl_masked_store_int_float_4x8_AVX = OclBuiltinImpl<__ocl_masked_store4x8, [v4i32, v4f32], 0, [{
    __ocl_masked_store_$Arg0BaseType_4x4(&($Arg0VarName[0]), &($Arg1VarName[0]), $Arg2VarName.lo);
    __ocl_masked_store_$Arg0BaseType_4x4(&($Arg0VarName[4]), &($Arg1VarName[4]), $Arg2VarName.hi);
  }]>;

// ****************************************************************************
//                                 float4x4
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_float_4x4_AVX = OclBuiltinImpl<__ocl_masked_load4x4, [v4f32], 0, [{
    *((__private float16*)$Arg1VarName) = 0;                                        // to avoid denormals for the non loaded floats
    float8 extendedMask = as_float8($Arg2VarName.s01230123);
    float8 maskLow = extendedMask.s00001111;
    float8 maskHigh = extendedMask.s22223333;
    __private float8* tmpLoadAdd = (__private float8*)$Arg0VarName;
    __private float8* tmpLoadedValues = (__private float8*)$Arg1VarName;
    tmpLoadedValues[0] = as_float8(_mm256_maskload_ps((float*)&(tmpLoadAdd[0]), (__m256i)maskLow));
    tmpLoadedValues[1] = as_float8(_mm256_maskload_ps((float*)&(tmpLoadAdd[1]), (__m256i)maskHigh));
  }]>;

OclBuiltinImpl __ocl_masked_store_float_4x4_AVX = OclBuiltinImpl<__ocl_masked_store4x4, [v4f32], 0, [{
    float8 extendedMask = as_float8($Arg2VarName.s01230123);
    float8 maskLow = extendedMask.s00001111;
    float8 maskHigh = extendedMask.s22223333;

    __private float8* tmpStoreAdd = (__private float8*)$Arg0VarName;
    __private float8* tmpValuesToStore = (__private float8*)$Arg1VarName;
    _mm256_maskstore_ps((float*)&(tmpStoreAdd[0]), (__m256i)maskLow, (__m256)tmpValuesToStore[0]);
    _mm256_maskstore_ps((float*)&(tmpStoreAdd[1]), (__m256i)maskHigh, (__m256)tmpValuesToStore[1]);
  }]>;

// ****************************************************************************
//                            int4 & float4
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_int4_float4_AVX = OclBuiltinImpl<__ocl_masked_load4, [v4i32, v4f32], 0, [{
    return as_$ReturnType(_mm_maskload_ps ((float const *)$Arg0VarName, (__m128i) $Arg1VarName));
  }]>;

OclBuiltinImpl __ocl_masked_store_int4_float4_AVX = OclBuiltinImpl<__ocl_masked_store4, [v4i32, v4f32], 0, [{
    _mm_maskstore_ps((float *)$Arg0VarName, (__m128i) $Arg2VarName, *((__m128*) &$Arg1VarName));
  }]>;

// ****************************************************************************
//                            int8 and float8
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_int8_AVX = OclBuiltinImpl<__ocl_masked_load8, [v8i32, v8f32], 0, [{
    return as_$ReturnType(_mm256_maskload_ps((float const *)$Arg0VarName, (__m256i) $Arg1VarName));
  }]>;

OclBuiltinImpl __ocl_masked_store_int8_AVX = OclBuiltinImpl<__ocl_masked_store8, [v8i32, v8f32], 0, [{
    _mm256_maskstore_ps((float *)$Arg0VarName, (__m256i) $Arg2VarName, *((__m256*) &$Arg1VarName));
  }]>;

// ****************************************************************************
//                             long4 & double4
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_long4_double4_AVX = OclBuiltinImpl<__ocl_masked_load4, [v4i64, v4f64], 0, [{
    long4 extMask = convert_long4($Arg1VarName);

    return as_$ReturnType(_mm256_maskload_pd((double *)$Arg0VarName, (__m256i) extMask));
  }]>;

OclBuiltinImpl __ocl_masked_store_long4_double4_AVX = OclBuiltinImpl<__ocl_masked_store4, [v4i64, v4f64], 0, [{
    long4 extMask = convert_long4($Arg2VarName);

    _mm256_maskstore_pd((double *)$Arg0VarName, (__m256i) extMask, *((__m256d*) &$Arg1VarName));
  }]>;

// ****************************************************************************
//                                 long8
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_long8_AVX = OclBuiltinImpl<__ocl_masked_load8, [v8i64], 0, [{
    long8 res;
    res.lo = __ocl_masked_load_long4((__private long4*) $Arg0VarName, $Arg1VarName.lo);
    res.hi = __ocl_masked_load_long4(((__private long4*) $Arg0VarName)+1, $Arg1VarName.hi);
    return res;
  }]>;

OclBuiltinImpl __ocl_masked_store_long8_AVX = OclBuiltinImpl<__ocl_masked_store8, [v8i64], 0, [{
    __ocl_masked_store_long4((__private long4*) $Arg0VarName, $Arg1VarName.lo, $Arg2VarName.lo);
    __ocl_masked_store_long4(((__private long4*) $Arg0VarName)+1, $Arg1VarName.hi, $Arg2VarName.hi);
  }]>;

// ****************************************************************************
//                                 double8
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_double8_AVX = OclBuiltinImpl<__ocl_masked_load8, [v8f64], 0, [{
    double8 res;
    res.lo = __ocl_masked_load_double4((__private double4*) $Arg0VarName, $Arg1VarName.lo);
    res.hi = __ocl_masked_load_double4(((__private double4*) $Arg0VarName)+1, $Arg1VarName.hi);
    return res;
  }]>;

OclBuiltinImpl __ocl_masked_store_double8_AVX = OclBuiltinImpl<__ocl_masked_store8, [v8f64], 0, [{
    __ocl_masked_store_double4((__private double4*) $Arg0VarName, $Arg1VarName.lo, $Arg2VarName.lo);
    __ocl_masked_store_double4(((__private double4*) $Arg0VarName)+1, $Arg1VarName.hi, $Arg2VarName.hi);
  }]>;
