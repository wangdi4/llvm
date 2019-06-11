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
    return as_$ReturnType(_mm_permutevar_ps(__builtin_astype($Arg0VarName, __m128),
                                            __builtin_astype($Arg1VarName, __m128i)));
  }]>;

// long and ulong
OclBuiltinImpl sub_group_shuffle_avx128v4ui64 = OclBuiltinImpl<sub_group_shuffle_avx, [v4i64, v4u64], 0,
 [{
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
