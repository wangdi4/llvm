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


// int4, uint4, float4
OclBuiltinImpl sub_group_shuffle_avx2v32 = OclBuiltinImpl<sub_group_shuffle_avx2, [v32u32, v32i32, v32f32], 0,
  [{
    $Arg1VarName = $Arg1VarName & $Arg2VarName;
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_4x8($Arg0VarName);

    $Arg0BaseType#8 tmp1 = $Arg0VarName.lo.lo;
    $Arg0BaseType#8 tmp2 = $Arg0VarName.lo.hi;
    $Arg0BaseType#8 tmp3 = $Arg0VarName.hi.lo;
    $Arg0BaseType#8 tmp4 = $Arg0VarName.hi.hi;

    $Arg0VarName.lo.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp1, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.lo.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp2, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.hi.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp3, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.hi.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp4, __m256), __builtin_astype($Arg1VarName, __m256i)));

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x4($Arg0VarName);
    return $Arg0VarName;
  }]>;

// int8, uint8, float8
OclBuiltinImpl sub_group_shuffle_avx2v64 = OclBuiltinImpl<sub_group_shuffle_avx2, [v64f32, v64i32, v64u32], 0,
[{
    $Arg1VarName = $Arg1VarName & $Arg2VarName;
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x8($Arg0VarName);

    $Arg0BaseType#8 tmp1 = $Arg0VarName.lo.lo.lo;
    $Arg0BaseType#8 tmp2 = $Arg0VarName.lo.lo.hi;
    $Arg0BaseType#8 tmp3 = $Arg0VarName.lo.hi.lo;
    $Arg0BaseType#8 tmp4 = $Arg0VarName.lo.hi.hi;
    $Arg0BaseType#8 tmp5 = $Arg0VarName.hi.lo.lo;
    $Arg0BaseType#8 tmp6 = $Arg0VarName.hi.lo.hi;
    $Arg0BaseType#8 tmp7 = $Arg0VarName.hi.hi.lo;
    $Arg0BaseType#8 tmp8 = $Arg0VarName.hi.hi.hi;

    $Arg0VarName.lo.lo.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp1, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.lo.lo.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp2, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.lo.hi.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp3, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.lo.hi.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp4, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.hi.lo.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp5, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.hi.lo.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp6, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.hi.hi.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp7, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.hi.hi.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp8, __m256), __builtin_astype($Arg1VarName, __m256i)));

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x8($Arg0VarName);
    return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_shuffle_avx2_v128 = OclBuiltinImpl<sub_group_shuffle_avx2, [v128f32, v128i32, v128u32], 0,
[{
    $Arg1VarName = $Arg1VarName & $Arg2VarName;
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x16($Arg0VarName);

    $Arg0BaseType#8 tmp1  = $Arg0VarName.lo.lo.lo.lo;
    $Arg0BaseType#8 tmp2  = $Arg0VarName.lo.lo.lo.hi;
    $Arg0BaseType#8 tmp3  = $Arg0VarName.lo.lo.hi.lo;
    $Arg0BaseType#8 tmp4  = $Arg0VarName.lo.lo.hi.hi;
    $Arg0BaseType#8 tmp5  = $Arg0VarName.lo.hi.lo.lo;
    $Arg0BaseType#8 tmp6  = $Arg0VarName.lo.hi.lo.hi;
    $Arg0BaseType#8 tmp7  = $Arg0VarName.lo.hi.hi.lo;
    $Arg0BaseType#8 tmp8  = $Arg0VarName.lo.hi.hi.hi;
    $Arg0BaseType#8 tmp9  = $Arg0VarName.hi.lo.lo.lo;
    $Arg0BaseType#8 tmp10 = $Arg0VarName.hi.lo.lo.hi;
    $Arg0BaseType#8 tmp11 = $Arg0VarName.hi.lo.hi.lo;
    $Arg0BaseType#8 tmp12 = $Arg0VarName.hi.lo.hi.hi;
    $Arg0BaseType#8 tmp13 = $Arg0VarName.hi.hi.lo.lo;
    $Arg0BaseType#8 tmp14 = $Arg0VarName.hi.hi.lo.hi;
    $Arg0BaseType#8 tmp15 = $Arg0VarName.hi.hi.hi.lo;
    $Arg0BaseType#8 tmp16 = $Arg0VarName.hi.hi.hi.hi;

    $Arg0VarName.lo.lo.lo.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype( tmp1, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.lo.lo.lo.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype( tmp2, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.lo.lo.hi.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype( tmp3, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.lo.lo.hi.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype( tmp4, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.lo.hi.lo.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype( tmp5, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.lo.hi.lo.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype( tmp6, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.lo.hi.hi.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype( tmp7, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.lo.hi.hi.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype( tmp8, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.hi.lo.lo.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype( tmp9, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.hi.lo.lo.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp10, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.hi.lo.hi.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp11, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.hi.lo.hi.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp12, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.hi.hi.lo.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp13, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.hi.hi.lo.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp14, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.hi.hi.hi.lo = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp15, __m256), __builtin_astype($Arg1VarName, __m256i)));
    $Arg0VarName.hi.hi.hi.hi = as_$Arg0BaseType#8(_mm256_permutevar8x32_ps(__builtin_astype(tmp16, __m256), __builtin_astype($Arg1VarName, __m256i)));

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x8($Arg0VarName);
    return $Arg0VarName;
}]>;

//
// Shuffle xor
//
OclBuiltinImpl sub_group_shuffle_xor_avx2_gen = OclBuiltinImpl<sub_group_shuffle_xor_avx2,
               [ v8i32,  v8u32,  v8f32,   v8i64,   v8u64,   v8f64,
                v16u32, v16i32, v16f32,  v32u32,  v32i32,  v32f32,
                v64u32, v64i32, v64f32, v128u32, v128i32, v128f32 ], 0,
  [{
    uint8 indexes = {0, 1, 2, 3, 4, 5, 6, 7};
    indexes = indexes ^ $Arg1VarName;
    return intel_sub_group_shuffle($Arg0VarName, indexes, $Arg2VarName);
  }]>;

//
// Shuffle down
//
OclBuiltinImpl sub_group_shuffle_down_avx2_gen = OclBuiltinImpl<sub_group_shuffle_down_avx2,
        [v8i32, v8u32, v8f32, v16i32, v16u32, v16f32, v32i32, v32u32, v32f32,
         v64i32, v64u32, v64f32, v128i32, v128u32, v128f32], 0,
  [{
      $Arg0Type res_cur, res_next;
      int$VecLength temp;
      uint8 sg_indexes = { 0, 1, 2, 3, 4, 5, 6, 7 };
      const uint max_sg_size = 8;

      sg_indexes = sg_indexes + $Arg2VarName;

      sg_indexes &= $Arg3VarName;

      int8 indexes_mask = sg_indexes < max_sg_size;
      int8 inv_indexes_mask = !indexes_mask;

      int$VecLength ext_mask = __ocl_extend_mask_to_$VecLength(indexes_mask);

      res_cur = intel_sub_group_shuffle($Arg0VarName, sg_indexes, *(uint8*)&indexes_mask);
      temp = *((int$VecLength*)&res_cur) & ext_mask;
      res_cur = *($Arg0Type*)&temp;

      res_next = intel_sub_group_shuffle($Arg1VarName, (sg_indexes - max_sg_size), *(uint8*)&inv_indexes_mask);
      temp = *((int$VecLength*)&res_next) & !ext_mask;
      res_next = *($Arg0Type*)&temp;

      temp = (*(int$VecLength*)&res_cur | *(int$VecLength*)&res_next);
      return *($Arg0Type*)&temp;
    }]>;


OclBuiltinImpl sub_group_shuffle_down_avx2_v8d64 = OclBuiltinImpl<sub_group_shuffle_down_avx2,
        [v8i64, v8u64, v8f64], 0,
  [{
      $Arg0Type res_cur, res_next;
      int16 temp;
      uint8 sg_indexes = { 0, 1, 2, 3, 4, 5, 6, 7 };
      const uint max_sg_size = 8;

      sg_indexes = sg_indexes + $Arg2VarName;

      sg_indexes &= $Arg3VarName;

      int8 indexes_mask = sg_indexes < max_sg_size;
      int8 inv_indexes_mask = !indexes_mask;

      int16 ext_mask = __ocl_extend_mask_to_16(indexes_mask);

      res_cur = intel_sub_group_shuffle($Arg0VarName, sg_indexes, *(uint8*)&indexes_mask);
      temp = *((int16*)&res_cur) & ext_mask;
      res_cur = *($Arg0Type*)&temp;

      res_next = intel_sub_group_shuffle($Arg1VarName, (sg_indexes - max_sg_size), *(uint8*)&inv_indexes_mask);
      temp = *((int16*)&res_next) & !ext_mask;
      res_next = *($Arg0Type*)&temp;

      temp = (*(int16*)&res_cur | *(int16*)&res_next);
      return *($Arg0Type*)&temp;
    }]>;

//
// Shuffle up
//
OclBuiltinImpl sub_group_shuffle_up_avx2_gen = OclBuiltinImpl<sub_group_shuffle_up_avx2,
            [v8i32, v8u32, v8f32, v16i32, v16u32, v16f32, v32i32, v32u32, v32f32,
             v64i32, v64u32, v64f32, v128i32, v128u32, v128f32], 0,
[{
     $Arg0Type res_cur, res_next;
     int$VecLength temp;
     int8 sg_indexes = { 0, 1, 2, 3, 4, 5, 6, 7 };
     const int max_sg_size = 8;

     sg_indexes = sg_indexes - *(int8*)&$Arg2VarName;

     sg_indexes &= *(int8*)&$Arg3VarName;

     int8 indexes_mask = sg_indexes >= 0;
     int8 inv_indexes_mask = !indexes_mask;

     int$VecLength ext_mask = __ocl_extend_mask_to_$VecLength(indexes_mask);

     res_cur = intel_sub_group_shuffle($Arg1VarName, *(uint8*)&sg_indexes, *(uint8*)&indexes_mask);
     temp = *((int$VecLength*)&res_cur) & ext_mask;
     res_cur = *($Arg0Type*)&temp;

     int8 other_ind = sg_indexes + max_sg_size;
     res_next = intel_sub_group_shuffle($Arg0VarName, *(uint8*)&other_ind, *(uint8*)&inv_indexes_mask);
     temp = *((int$VecLength*)&res_next) & !ext_mask;
     res_next = *($Arg0Type*)&temp;

     temp = (*(int$VecLength*)&res_cur | *(int$VecLength*)&res_next);
     return *($Arg0Type*)&temp;
}]>;


OclBuiltinImpl sub_group_shuffle_up_avx2_v8d64 = OclBuiltinImpl<sub_group_shuffle_up_avx2,
        [v8i64, v8u64, v8f64], 0,
[{
     $Arg0Type res_cur, res_next;
     int16 temp;
     int8 sg_indexes = { 0, 1, 2, 3, 4, 5, 6, 7 };
     const int max_sg_size = 8;

     sg_indexes = sg_indexes - *(int8*)&$Arg2VarName;

     sg_indexes &= *(int8*)&$Arg3VarName;

     int8 indexes_mask = sg_indexes >= 0;
     int8 inv_indexes_mask = !indexes_mask;

     int16 ext_mask = __ocl_extend_mask_to_16(indexes_mask);

     res_cur = intel_sub_group_shuffle($Arg1VarName, *(uint8*)&sg_indexes, *(uint8*)&indexes_mask);
     temp = *((int16*)&res_cur) & ext_mask;
     res_cur = *($Arg0Type*)&temp;

     int8 other_ind = sg_indexes + max_sg_size;
     res_next = intel_sub_group_shuffle($Arg0VarName, *(uint8*)&other_ind, *(uint8*)&inv_indexes_mask);
     temp = *((int16*)&res_next) & !ext_mask;
     res_next = *($Arg0Type*)&temp;

     temp = (*(int16*)&res_cur | *(int16*)&res_next);
     return *($Arg0Type*)&temp;
}]>;

OclBuiltinImpl intel_sub_group_block_read1_4_avx2 = OclBuiltinImpl<intel_sub_group_block_read1_4, [ v1u32 ], 0,
  [{
    const uint4 res = *(uint4*)($Arg0VarName);
    return ($ReturnType)(__builtin_shufflevector(res, res, 0, 1, 2, 3));
  }]>;

OclBuiltinImpl intel_sub_group_block_read2_4_avx2 = OclBuiltinImpl<intel_sub_group_block_read2_4, [ v1u32 ], 0,
  [{
    const uint8 res = *(uint8*)($Arg0VarName);
    return ($ReturnType)(__builtin_shufflevector(res, res, 0, 4, 1, 5, 2, 6, 3, 7));
  }]>;

OclBuiltinImpl intel_sub_group_block_read4_4_avx2 = OclBuiltinImpl<intel_sub_group_block_read4_4, [ v1u32 ], 0,
  [{
    const uint16 res = *(uint16*)($Arg0VarName);
    return ($ReturnType)(__builtin_shufflevector(res, res, 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15));
  }]>;

OclBuiltinImpl intel_sub_group_block_read8_4_avx2 = OclBuiltinImpl<intel_sub_group_block_read8_4, [ v1u32 ], 0,
  [{
    const uint32 res = *(uint32*)($Arg0VarName);
    return ($ReturnType)(__builtin_shufflevector(res, res, 0, 4, 8, 12, 16, 20, 24, 28, 1, 5, 9, 13, 17, 21, 25, 29, 2, 6, 10, 14, 18, 22, 26, 30, 3, 7, 11, 15, 19, 23, 27, 31));
  }]>;

OclBuiltinImpl intel_sub_group_block_read1_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read1_8, [ v1u32 ], 0,
  [{
    const uint8 res = *(uint8*)($Arg0VarName);
    return ($ReturnType)(__builtin_shufflevector(res, res, 0, 1, 2, 3, 4, 5, 6, 7));
  }]>;

OclBuiltinImpl intel_sub_group_block_read2_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read2_8, [ v1u32 ], 0,
  [{
    const uint16 res = *(uint16*)($Arg0VarName);
    return ($ReturnType)(__builtin_shufflevector(res, res, 0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15));
  }]>;

OclBuiltinImpl intel_sub_group_block_read4_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read4_8, [ v1u32 ], 0,
  [{
    const uint32 res = *(uint32*)($Arg0VarName);
    return ($ReturnType)(__builtin_shufflevector(res, res, 0, 8, 16, 24, 1, 9, 17, 25, 2, 10, 18, 26, 3, 11, 19, 27, 4, 12, 20, 28, 5, 13, 21, 29, 6, 14, 22, 30, 7, 15, 23, 31));
  }]>;

OclBuiltinImpl intel_sub_group_block_read8_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read8_8, [ v1u32 ], 0,
  [{
    const uint64 res = *(uint64*)($Arg0VarName);
    return ($ReturnType)(__builtin_shufflevector(res, res, 0, 8, 16, 24, 32, 40, 48, 56, 1, 9, 17, 25, 33, 41, 49, 57, 2, 10, 18, 26, 34, 42, 50, 58, 3, 11, 19, 27, 35, 43, 51, 59, 4, 12, 20, 28, 36, 44, 52, 60, 5, 13, 21, 29, 37, 45, 53, 61, 6, 14, 22, 30, 38, 46, 54, 62, 7, 15, 23, 31, 39, 47, 55, 63));
  }]>;

OclBuiltinImpl intel_sub_group_block_write1_4_avx512f = OclBuiltinImpl<intel_sub_group_block_write1_4, [ v1u32 ], 0,
  [{
    uint4 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 1, 2, 3);
    *(uint4 *)($Arg0VarName) = res;
  }]>;

OclBuiltinImpl intel_sub_group_block_write2_4_avx512f = OclBuiltinImpl<intel_sub_group_block_write2_4, [ v1u32 ], 0,
  [{
    uint8 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 4, 1, 5, 2, 6, 3, 7);
    *(uint8 *)($Arg0VarName) = res;
  }]>;

OclBuiltinImpl intel_sub_group_block_write4_4_avx512f = OclBuiltinImpl<intel_sub_group_block_write4_4, [ v1u32 ], 0,
  [{
    uint16 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15);
    *(uint16 *)($Arg0VarName) = res;
  }]>;

OclBuiltinImpl intel_sub_group_block_write8_4_avx512f = OclBuiltinImpl<intel_sub_group_block_write8_4, [ v1u32 ], 0,
  [{
    uint32 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 4, 8, 12, 16, 20, 24, 28, 1, 5, 9, 13, 17, 21, 25, 29, 2, 6, 10, 14, 18, 22, 26, 30, 3, 7, 11, 15, 19, 23, 27, 31);
    *(uint32 *)($Arg0VarName) = res;
  }]>;

OclBuiltinImpl intel_sub_group_block_write1_8_avx512f = OclBuiltinImpl<intel_sub_group_block_write1_8, [ v1u32 ], 0,
  [{
    uint8 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 1, 2, 3, 4, 5, 6, 7);
    *(uint8 *)($Arg0VarName) = res;
  }]>;

OclBuiltinImpl intel_sub_group_block_write2_8_avx512f = OclBuiltinImpl<intel_sub_group_block_write2_8, [ v1u32 ], 0,
  [{
    uint16 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15);
    *(uint16 *)($Arg0VarName) = res;
  }]>;

OclBuiltinImpl intel_sub_group_block_write4_8_avx512f = OclBuiltinImpl<intel_sub_group_block_write4_8, [ v1u32 ], 0,
  [{
    uint32 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 8, 16, 24, 1, 9, 17, 25, 2, 10, 18, 26, 3, 11, 19, 27, 4, 12, 20, 28, 5, 13, 21, 29, 6, 14, 22, 30, 7, 15, 23, 31);
    *(uint32 *)($Arg0VarName) = res;
  }]>;

OclBuiltinImpl intel_sub_group_block_write8_8_avx512f = OclBuiltinImpl<intel_sub_group_block_write8_8, [ v1u32 ], 0,
  [{
    uint64 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 8, 16, 24, 32, 40, 48, 56, 1, 9, 17, 25, 33, 41, 49, 57, 2, 10, 18, 26, 34, 42, 50, 58, 3, 11, 19, 27, 35, 43, 51, 59, 4, 12, 20, 28, 36, 44, 52, 60, 5, 13, 21, 29, 37, 45, 53, 61, 6, 14, 22, 30, 38, 46, 54, 62, 7, 15, 23, 31, 39, 47, 55, 63);
    *(uint64 *)($Arg0VarName) = res;
  }]>;

