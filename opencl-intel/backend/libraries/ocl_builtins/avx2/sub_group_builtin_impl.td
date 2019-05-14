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
OclBuiltinImpl sub_group_shuffle_avx256v8ui32 = OclBuiltinImpl<sub_group_shuffle, [v8i32, v8u32], 0,
  [{
    return as_$ReturnType(_mm256_permutevar8x32_epi32(__builtin_astype($Arg0VarName, __m256i),
                                                      __builtin_astype($Arg1VarName, __m256i)));
  }]>;

// float
OclBuiltinImpl sub_group_shuffle_avx256fv8f32 = OclBuiltinImpl<sub_group_shuffle, [v8f32], 0,
  [{
    return as_$ReturnType(_mm256_permutevar8x32_ps(__builtin_astype($Arg0VarName, __m256),
                                                   __builtin_astype($Arg1VarName, __m256i)));
  }]>;

// long and ulong
OclBuiltinImpl sub_group_shuffle_avx512fv8ui64 = OclBuiltinImpl<sub_group_shuffle, [v8i64, v8u64], 0,
 [{
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

// int2, uint2
//OclBuiltinImpl sub_group_shuffle_avx512f = OclBuiltinImpl<sub_group_shuffle, [v16u32, v16i64], 0,
//  [{
//    return $Argv0VarName;
//  }]>;
