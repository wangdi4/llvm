list<OclType> sub_group_types_all = [v1i32, v4i32, v8i32, v16i32,
                                     v1u32, v4u32, v8u32, v16u32,
                                     v1i64, v4i64, v8i64, v16i64,
                                     v1u64, v4u64, v8u64, v16u64,
                                     v1f32, v4f32, v8f32, v16f32,
                                     v1f64, v4f64, v8f64, v16f64];

list<OclType> sub_group_types_vec = [v4i32, v8i32, v16i32,
                                     v4u32, v8u32, v16u32,
                                     v4i64, v8i64, v16i64,
                                     v4u64, v8u64, v16u64,
                                     v4f32, v8f32, v16f32,
                                     v4f64, v8f64, v16f64];

//
// Shuffles
//

// Scalar
// int and uint
OclBuiltinImpl sub_group_shuffle_avx256v8ui32 = OclBuiltinImpl<sub_group_shuffle_avx2, [v8i32, v8u32], 0,
  [{
    $Arg1VarName = $Arg1VarName & $Arg2VarName;
    return as_$ReturnType(_mm256_permutevar8x32_epi32(__builtin_astype($Arg0VarName, __m256i),
                                                      __builtin_astype($Arg1VarName, __m256i)));
  }]>;

// float
OclBuiltinImpl sub_group_shuffle_avx256fv8f32 = OclBuiltinImpl<sub_group_shuffle_avx2, [v8f32], 0,
  [{
    $Arg1VarName = $Arg1VarName & $Arg2VarName;
    return as_$ReturnType(_mm256_permutevar8x32_ps(__builtin_astype($Arg0VarName, __m256),
                                                   __builtin_astype($Arg1VarName, __m256i)));
  }]>;

// long and ulong
OclBuiltinImpl sub_group_shuffle_avx512fv8ui64 = OclBuiltinImpl<sub_group_shuffle_avx2, [v8i64, v8u64, v8f64], 0,
 [{
    $Arg1VarName = $Arg1VarName & $Arg2VarName;
    $Arg0Type res;
    int8 mask = {0, 2, 4, 6, 1, 3, 5, 7};
    int8 back_mask = {0, 4, 1, 5, 2, 6, 3, 7};
    __m256 tmp_lo, tmp_hi;

    // 1. transpose
    $Arg0VarName.lo = as_$Arg0BaseType#4(_mm256_permutevar8x32_ps((__m256)$Arg0VarName.lo, (__m256i)mask));
    $Arg0VarName.hi = as_$Arg0BaseType#4(_mm256_permutevar8x32_ps((__m256)$Arg0VarName.hi, (__m256i)mask));
    tmp_lo = _mm256_permute2f128_ps((__m256)$Arg0VarName.lo, (__m256)$Arg0VarName.hi, 32);
    tmp_hi = _mm256_permute2f128_ps((__m256)$Arg0VarName.lo, (__m256)$Arg0VarName.hi, 49);

    // 2. shuffle
    tmp_lo = _mm256_permutevar8x32_ps(tmp_lo, __builtin_astype($Arg1VarName, __m256i));
    tmp_hi = _mm256_permutevar8x32_ps(tmp_hi, __builtin_astype($Arg1VarName, __m256i));

    // 3. transpose back
    res.lo = as_$Arg0BaseType#4(_mm256_permute2f128_ps(tmp_lo, tmp_hi, 32));
    res.hi = as_$Arg0BaseType#4(_mm256_permute2f128_ps(tmp_lo, tmp_hi, 49));
    res.lo = as_$Arg0BaseType#4(_mm256_permutevar8x32_ps((__m256)res.lo, (__m256i)back_mask));
    res.hi = as_$Arg0BaseType#4(_mm256_permutevar8x32_ps((__m256)res.hi, (__m256i)back_mask));

    return res;
 }]>;

// Vector
// int2, uint2, float2
OclBuiltinImpl sub_group_shuffle_avx2v16 = OclBuiltinImpl<sub_group_shuffle_avx2, [v16u32, v16i32, v16f32], 0,
  [{
    $Arg1VarName = $Arg1VarName & $Arg2VarName;
    $Arg0Type res;
    int8 mask = {0, 2, 4, 6, 1, 3, 5, 7};
    int8 back_mask = {0, 4, 1, 5, 2, 6, 3, 7};
    __m256 tmp_lo, tmp_hi;

    // 1. transpose
    $Arg0VarName.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps((__m256)$Arg0VarName.lo, (__m256i)mask));
    $Arg0VarName.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps((__m256)$Arg0VarName.hi, (__m256i)mask));
    tmp_lo = _mm256_permute2f128_ps((__m256)$Arg0VarName.lo, (__m256)$Arg0VarName.hi, 32);
    tmp_hi = _mm256_permute2f128_ps((__m256)$Arg0VarName.lo, (__m256)$Arg0VarName.hi, 49);

    // 2. shuffle
    tmp_lo = _mm256_permutevar8x32_ps(tmp_lo, __builtin_astype($Arg1VarName, __m256i));
    tmp_hi = _mm256_permutevar8x32_ps(tmp_hi, __builtin_astype($Arg1VarName, __m256i));

    // 3. transpose back
    res.lo = as_$Arg0BaseType#8(_mm256_permute2f128_ps(tmp_lo, tmp_hi, 32));
    res.hi = as_$Arg0BaseType#8(_mm256_permute2f128_ps(tmp_lo, tmp_hi, 49));
    res.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps((__m256)res.lo, (__m256i)back_mask));
    res.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps((__m256)res.hi, (__m256i)back_mask));

    return res;
  }]>;
OclBuiltinImpl intel_sub_group_block_write1_4_avx2 = OclBuiltinImpl<intel_sub_group_block_write1_4, [ v4i32 ], 0,
  [{
    uint4 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 1, 2, 3);
    _mm_store_epi32($Arg0VarName, res);
  }]>;

OclBuiltinImpl intel_sub_group_block_write2_4_avx2 = OclBuiltinImpl<intel_sub_group_block_write2_4, [ v8i32 ], 0,
  [{
    uint8 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 4, 1, 5, 2, 6, 3, 7);
    _mm256_store_epi32($Arg0VarName, res);
  }]>;

OclBuiltinImpl intel_sub_group_block_write4_4_avx2 = OclBuiltinImpl<intel_sub_group_block_write4_4, [ v16i32 ], 0,
  [{
    uint16 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15);
  }]>;

OclBuiltinImpl intel_sub_group_block_write8_4_avx2 = OclBuiltinImpl<intel_sub_group_block_write8_4, [ v32i32 ], 0,
  [{
    uint32 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 4, 8, 12, 16, 20, 24, 28, 1, 5, 9, 13, 17, 21, 25, 29, 2, 6, 10, 14, 18, 22, 26, 30, 3, 7, 11, 15, 19, 23, 27, 31);
  }]>;

OclBuiltinImpl intel_sub_group_block_write1_8_avx2 = OclBuiltinImpl<intel_sub_group_block_write1_8, [ v8i32 ], 0,
  [{
    uint8 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 1, 2, 3, 4, 5, 6, 7);
    _mm256_store_epi32($Arg0VarName, res);
  }]>;

OclBuiltinImpl intel_sub_group_block_write2_8_avx2 = OclBuiltinImpl<intel_sub_group_block_write2_8, [ v16i32 ], 0,
  [{
    uint16 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15);
  }]>;

OclBuiltinImpl intel_sub_group_block_write4_8_avx2 = OclBuiltinImpl<intel_sub_group_block_write4_8, [ v32i32 ], 0,
  [{
    uint32 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 8, 16, 24, 1, 9, 17, 25, 2, 10, 18, 26, 3, 11, 19, 27, 4, 12, 20, 28, 5, 13, 21, 29, 6, 14, 22, 30, 7, 15, 23, 31);
  }]>;

OclBuiltinImpl intel_sub_group_block_write8_8_avx2 = OclBuiltinImpl<intel_sub_group_block_write8_8, [ v64i32 ], 0,
  [{
    uint64 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 8, 16, 24, 32, 40, 48, 56, 1, 9, 17, 25, 33, 41, 49, 57, 2, 10, 18, 26, 34, 42, 50, 58, 3, 11, 19, 27, 35, 43, 51, 59, 4, 12, 20, 28, 36, 44, 52, 60, 5, 13, 21, 29, 37, 45, 53, 61, 6, 14, 22, 30, 38, 46, 54, 62, 7, 15, 23, 31, 39, 47, 55, 63);
    _mm512_store_epi32($Arg0VarName+0, *((uint16*)(*res) + 0));
  }]>;

OclBuiltinImpl intel_sub_group_block_write1_16_avx2 = OclBuiltinImpl<intel_sub_group_block_write1_16, [ v16i32 ], 0,
  [{
    uint16 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  }]>;

OclBuiltinImpl intel_sub_group_block_write2_16_avx2 = OclBuiltinImpl<intel_sub_group_block_write2_16, [ v32i32 ], 0,
  [{
    uint32 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23, 8, 24, 9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31);
  }]>;

OclBuiltinImpl intel_sub_group_block_write4_16_avx2 = OclBuiltinImpl<intel_sub_group_block_write4_16, [ v64i32 ], 0,
  [{
    uint64 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51, 4, 20, 36, 52, 5, 21, 37, 53, 6, 22, 38, 54, 7, 23, 39, 55, 8, 24, 40, 56, 9, 25, 41, 57, 10, 26, 42, 58, 11, 27, 43, 59, 12, 28, 44, 60, 13, 29, 45, 61, 14, 30, 46, 62, 15, 31, 47, 63);
    _mm512_store_epi32($Arg0VarName+0, *((uint16*)(*res) + 0));
  }]>;

OclBuiltinImpl intel_sub_group_block_write8_16_avx2 = OclBuiltinImpl<intel_sub_group_block_write8_16, [ v128i32 ], 0,
  [{
    uint128 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 16, 32, 48, 64, 80, 96, 112, 1, 17, 33, 49, 65, 81, 97, 113, 2, 18, 34, 50, 66, 82, 98, 114, 3, 19, 35, 51, 67, 83, 99, 115, 4, 20, 36, 52, 68, 84, 100, 116, 5, 21, 37, 53, 69, 85, 101, 117, 6, 22, 38, 54, 70, 86, 102, 118, 7, 23, 39, 55, 71, 87, 103, 119, 8, 24, 40, 56, 72, 88, 104, 120, 9, 25, 41, 57, 73, 89, 105, 121, 10, 26, 42, 58, 74, 90, 106, 122, 11, 27, 43, 59, 75, 91, 107, 123, 12, 28, 44, 60, 76, 92, 108, 124, 13, 29, 45, 61, 77, 93, 109, 125, 14, 30, 46, 62, 78, 94, 110, 126, 15, 31, 47, 63, 79, 95, 111, 127);
    _mm512_store_epi32($Arg0VarName+0, *((uint16*)(*res) + 0));
    _mm512_store_epi32($Arg0VarName+16, *((uint16*)(*res) + 16));
  }]>;

OclBuiltinImpl intel_sub_group_block_read1_4_avx2 = OclBuiltinImpl<intel_sub_group_block_read1_4, [ v4i32 ], 0,
  [{
    const uint4 res = _mm_load_epi32($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 1, 2, 3);
  }]>;

OclBuiltinImpl intel_sub_group_block_read2_4_avx2 = OclBuiltinImpl<intel_sub_group_block_read2_4, [ v8i32 ], 0,
  [{
    const uint8 res = _mm256_load_epi32($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 4, 1, 5, 2, 6, 3, 7);
  }]>;

OclBuiltinImpl intel_sub_group_block_read4_4_avx2 = OclBuiltinImpl<intel_sub_group_block_read4_4, [ v16i32 ], 0,
  [{
    const uint16 res = *(uint16*)($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15);
  }]>;

OclBuiltinImpl intel_sub_group_block_read8_4_avx2 = OclBuiltinImpl<intel_sub_group_block_read8_4, [ v32i32 ], 0,
  [{
    const uint32 res = *(uint32*)($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 4, 8, 12, 16, 20, 24, 28, 1, 5, 9, 13, 17, 21, 25, 29, 2, 6, 10, 14, 18, 22, 26, 30, 3, 7, 11, 15, 19, 23, 27, 31);
  }]>;

OclBuiltinImpl intel_sub_group_block_read1_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read1_8, [ v8i32 ], 0,
  [{
    const uint8 res = _mm256_load_epi32($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 1, 2, 3, 4, 5, 6, 7);
  }]>;

OclBuiltinImpl intel_sub_group_block_read2_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read2_8, [ v16i32 ], 0,
  [{
    const uint16 res = *(uint16*)($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15);
  }]>;

OclBuiltinImpl intel_sub_group_block_read4_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read4_8, [ v32i32 ], 0,
  [{
    const uint32 res = *(uint32*)($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 8, 16, 24, 1, 9, 17, 25, 2, 10, 18, 26, 3, 11, 19, 27, 4, 12, 20, 28, 5, 13, 21, 29, 6, 14, 22, 30, 7, 15, 23, 31);
  }]>;

OclBuiltinImpl intel_sub_group_block_read8_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read8_8, [ v64i32 ], 0,
  [{
    const uint64 res = *(uint64*)($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 8, 16, 24, 32, 40, 48, 56, 1, 9, 17, 25, 33, 41, 49, 57, 2, 10, 18, 26, 34, 42, 50, 58, 3, 11, 19, 27, 35, 43, 51, 59, 4, 12, 20, 28, 36, 44, 52, 60, 5, 13, 21, 29, 37, 45, 53, 61, 6, 14, 22, 30, 38, 46, 54, 62, 7, 15, 23, 31, 39, 47, 55, 63);
  }]>;

OclBuiltinImpl intel_sub_group_block_read1_16_avx2 = OclBuiltinImpl<intel_sub_group_block_read1_16, [ v16i32 ], 0,
  [{
    const uint16 res = *(uint16*)($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  }]>;

OclBuiltinImpl intel_sub_group_block_read2_16_avx2 = OclBuiltinImpl<intel_sub_group_block_read2_16, [ v32i32 ], 0,
  [{
    const uint32 res = *(uint32*)($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23, 8, 24, 9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31);
  }]>;

OclBuiltinImpl intel_sub_group_block_read4_16_avx2 = OclBuiltinImpl<intel_sub_group_block_read4_16, [ v64i32 ], 0,
  [{
    const uint64 res = *(uint64*)($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51, 4, 20, 36, 52, 5, 21, 37, 53, 6, 22, 38, 54, 7, 23, 39, 55, 8, 24, 40, 56, 9, 25, 41, 57, 10, 26, 42, 58, 11, 27, 43, 59, 12, 28, 44, 60, 13, 29, 45, 61, 14, 30, 46, 62, 15, 31, 47, 63);
  }]>;

OclBuiltinImpl intel_sub_group_block_read8_16_avx2 = OclBuiltinImpl<intel_sub_group_block_read8_16, [ v128i32 ], 0,
  [{
    const uint128 res = *(uint128*)($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 16, 32, 48, 64, 80, 96, 112, 1, 17, 33, 49, 65, 81, 97, 113, 2, 18, 34, 50, 66, 82, 98, 114, 3, 19, 35, 51, 67, 83, 99, 115, 4, 20, 36, 52, 68, 84, 100, 116, 5, 21, 37, 53, 69, 85, 101, 117, 6, 22, 38, 54, 70, 86, 102, 118, 7, 23, 39, 55, 71, 87, 103, 119, 8, 24, 40, 56, 72, 88, 104, 120, 9, 25, 41, 57, 73, 89, 105, 121, 10, 26, 42, 58, 74, 90, 106, 122, 11, 27, 43, 59, 75, 91, 107, 123, 12, 28, 44, 60, 76, 92, 108, 124, 13, 29, 45, 61, 77, 93, 109, 125, 14, 30, 46, 62, 78, 94, 110, 126, 15, 31, 47, 63, 79, 95, 111, 127);
  }]>;

