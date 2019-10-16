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
// int, uint, and float
OclBuiltinImpl sub_group_shuffle_avx128v4ui32 = OclBuiltinImpl<sub_group_shuffle_avx, [v4i32, v4u32, v4f32], 0,
  [{
    $Arg1VarName = $Arg1VarName & $Arg2VarName;
    return as_$ReturnType(_mm_permutevar_ps(__builtin_astype($Arg0VarName, __m128),
                                            __builtin_astype($Arg1VarName, __m128i)));
  }]>;

// long and ulong
OclBuiltinImpl sub_group_shuffle_avx128v4ui64 = OclBuiltinImpl<sub_group_shuffle_avx, [v4i64, v4u64, v4f64], 0,
 [{
    $Arg1VarName = $Arg1VarName & $Arg2VarName;
    $Arg0Type res;
    int4 mask = {0, 2, 1, 3};
    __m128 tmp_lo, tmp_hi;

    $Arg0VarName.lo = as_$Arg0BaseType#2(_mm_permutevar_ps((__m128)$Arg0VarName.lo, (__m128i)mask));
    $Arg0VarName.hi = as_$Arg0BaseType#2(_mm_permutevar_ps((__m128)$Arg0VarName.hi, (__m128i)mask));
    tmp_lo = _mm_shuffle_ps((__m128)$Arg0VarName.lo, (__m128)$Arg0VarName.hi, 68);
    tmp_hi = _mm_shuffle_ps((__m128)$Arg0VarName.lo, (__m128)$Arg0VarName.hi, 238);

    tmp_lo = _mm_permutevar_ps(tmp_lo, (__m128i)$Arg1VarName);
    tmp_hi = _mm_permutevar_ps(tmp_hi, (__m128i)$Arg1VarName);

    res.lo = as_$Arg0BaseType#2(_mm_shuffle_ps(tmp_lo, tmp_hi, 68));
    res.hi = as_$Arg0BaseType#2(_mm_shuffle_ps(tmp_lo, tmp_hi, 238));
    res.lo = as_$Arg0BaseType#2(_mm_permutevar_ps((__m128)res.lo, (__m128i)mask));
    res.hi = as_$Arg0BaseType#2(_mm_permutevar_ps((__m128)res.hi, (__m128i)mask));

    return res;
 }]>;

// Vector types
OclBuiltinImpl sub_group_shuffle_avxv8uif32 = OclBuiltinImpl<sub_group_shuffle_avx, [v8u32, v8i32, v8f32], 0,
 [{
    $Arg1VarName = $Arg1VarName & $Arg2VarName;
    $Arg0Type res;
    int4 mask = {0, 2, 1, 3};
    __m128 tmp_lo, tmp_hi;

    $Arg0VarName.lo = as_$Arg0BaseType#4(_mm_permutevar_ps((__m128)$Arg0VarName.lo, (__m128i)mask));
    $Arg0VarName.hi = as_$Arg0BaseType#4(_mm_permutevar_ps((__m128)$Arg0VarName.hi, (__m128i)mask));
    tmp_lo = _mm_shuffle_ps((__m128)$Arg0VarName.lo, (__m128)$Arg0VarName.hi, 68);
    tmp_hi = _mm_shuffle_ps((__m128)$Arg0VarName.lo, (__m128)$Arg0VarName.hi, 238);

    tmp_lo = _mm_permutevar_ps(tmp_lo, (__m128i)$Arg1VarName);
    tmp_hi = _mm_permutevar_ps(tmp_hi, (__m128i)$Arg1VarName);

    res.lo = as_$Arg0BaseType#4(_mm_shuffle_ps(tmp_lo, tmp_hi, 68));
    res.hi = as_$Arg0BaseType#4(_mm_shuffle_ps(tmp_lo, tmp_hi, 238));
    res.lo = as_$Arg0BaseType#4(_mm_permutevar_ps((__m128)res.lo, (__m128i)mask));
    res.hi = as_$Arg0BaseType#4(_mm_permutevar_ps((__m128)res.hi, (__m128i)mask));

    return res;
 }]>;

OclBuiltinImpl sub_group_shuffle_avxv16uif32 = OclBuiltinImpl<sub_group_shuffle_avx, [v16u32, v16i32, v16f32], 0,
  [{
    $Arg0BaseType#4 t1 = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 0, 4, 8,  12);
    $Arg0BaseType#4 t2 = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 1, 5, 9,  13);
    $Arg0BaseType#4 t3 = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 2, 6, 10, 14);
    $Arg0BaseType#4 t4 = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 7, 11, 15);
    $Arg0BaseType#16 res;
    res.lo.lo = intel_sub_group_shuffle(t1, $Arg1VarName, $Arg2VarName);
    res.lo.hi = intel_sub_group_shuffle(t2, $Arg1VarName, $Arg2VarName);
    res.hi.lo = intel_sub_group_shuffle(t3, $Arg1VarName, $Arg2VarName);
    res.hi.hi = intel_sub_group_shuffle(t4, $Arg1VarName, $Arg2VarName);
    return res;
  }]>;

OclBuiltinImpl sub_group_shuffle_avxv32uif32 = OclBuiltinImpl<sub_group_shuffle_avx, [v32u32, v32i32, v32f32], 0,
  [{
    $Arg0BaseType#4 t1 = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 0, 8,  16, 24);
    $Arg0BaseType#4 t2 = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 1, 9,  17, 25);
    $Arg0BaseType#4 t3 = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 2, 10, 18, 26);
    $Arg0BaseType#4 t4 = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 11, 19, 27);
    $Arg0BaseType#4 t5 = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 4, 12, 20, 28);
    $Arg0BaseType#4 t6 = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 5, 13, 21, 29);
    $Arg0BaseType#4 t7 = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 6, 14, 22, 30);
    $Arg0BaseType#4 t8 = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 15, 23, 31);
    $Arg0BaseType#32 res;
    res.lo.lo.lo = intel_sub_group_shuffle(t1, $Arg1VarName, $Arg2VarName);
    res.lo.lo.hi = intel_sub_group_shuffle(t2, $Arg1VarName, $Arg2VarName);
    res.lo.hi.lo = intel_sub_group_shuffle(t3, $Arg1VarName, $Arg2VarName);
    res.lo.hi.hi = intel_sub_group_shuffle(t4, $Arg1VarName, $Arg2VarName);
    res.hi.lo.lo = intel_sub_group_shuffle(t5, $Arg1VarName, $Arg2VarName);
    res.hi.lo.hi = intel_sub_group_shuffle(t6, $Arg1VarName, $Arg2VarName);
    res.hi.hi.lo = intel_sub_group_shuffle(t7, $Arg1VarName, $Arg2VarName);
    res.hi.hi.hi = intel_sub_group_shuffle(t8, $Arg1VarName, $Arg2VarName);
    return res;
  }]>;

//
// Shuffle xor
//
OclBuiltinImpl sub_group_shuffle_xor_avx128v4ui32 = OclBuiltinImpl<sub_group_shuffle_xor_avx,
               [v4i32, v4u32, v4f32, v4i64, v4u64, v4f64, v8u32, v8i32, v8f32], 0,
  [{
    uint4 indexes = {0, 1, 2, 3};
    indexes = indexes ^ $Arg1VarName;
    return intel_sub_group_shuffle($Arg0VarName, indexes, $Arg2VarName);
  }]>;
