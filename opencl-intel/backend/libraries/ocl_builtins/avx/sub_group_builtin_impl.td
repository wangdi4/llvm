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

// broadcast
OclBuiltinImpl sub_group_broadcast_avx128v4gentype = OclBuiltinImpl<sub_group_broadcast_vec, [v4u32, v4i32, v4f32,
                                                                                              v4i64, v4u64, v4f64], 0,
  [{
    return $Arg0VarName[$Arg1VarName];
  }]>;

//
// reductions
//

// all|any
OclBuiltinImpl sub_group_all_avx128v4iu32 = OclBuiltinImpl<sub_group_all_vec, [v4u32, v4i32], 0,
  [{
    __m128i is_zero = _mm_cmpeq_epi32((__m128i)$Arg0VarName, _mm_set_epi32(0,0,0,0));
    return _mm_testz_si128(is_zero, (__m128i)$Arg1VarName);
  }]>;

OclBuiltinImpl sub_group_any_avx128v4iu32 = OclBuiltinImpl<sub_group_any_vec, [v4u32, v4i32], 0,
  [{
    return _mm_testz_si128((__m128i)$Arg0VarName, (__m128i)$Arg1VarName) != 1;
  }]>;

// reduce add
OclBuiltinImpl sub_group_reduce_add_avx128ui32 = OclBuiltinImpl<sub_group_reduce_add_vec, [v4i32, v4u32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 &= arg1;
    $Arg0Type temp = as_$Arg0Type(_mm_hadd_epi32((__m128i)arg0, (__m128i)arg0));
    return temp[0] + temp[1];
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx128f32 = OclBuiltinImpl<sub_group_reduce_add_vec, [v4f32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 &= arg1;
    $Arg0Type temp = as_$Arg0Type(_mm_hadd_ps((__m128)arg0, (__m128)arg0));
    return temp[0] + temp[1];
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx128ui64 = OclBuiltinImpl<sub_group_reduce_add_vec, [v4i64, v4u64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    arg0 &= arg1;
    $Arg0BaseType#2 temp = as_$Arg0BaseType#2(_mm_add_epi64((__m128i)arg0.lo, (__m128i)arg0.hi));
    return temp[0] + temp[1];
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx128f64 = OclBuiltinImpl<sub_group_reduce_add_vec, [v4f64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    arg0 &= arg1;
    $Arg0BaseType#2 temp = as_$Arg0BaseType#2(_mm_add_pd((__m128d)arg0.lo, (__m128d)arg0.hi));
    return temp[0] + temp[1];
  }]>;

// reduce min
OclBuiltinImpl sub_group_reduce_min_avx128u32 = OclBuiltinImpl<sub_group_reduce_min_vec, [v4u32], 0,
  [{
     int4 arg0 = as_int4($Arg0VarName);
     int4 arg1 = as_int4($Arg1VarName);
     arg0 |= ~arg1;
     int4 temp0 = as_int4(_mm_min_epu32((__m128i)arg0, (__m128i)__builtin_shufflevector(arg0, arg0, 2, 3, 0, 1)));
     $Arg0Type temp1 = as_$Arg0Type(temp0);
     return min(temp1[0], temp1[1]);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx128i32 = OclBuiltinImpl<sub_group_reduce_min_vec, [v4i32], 0,
  [{
     int4 arg0 = as_int4($Arg0VarName);
     int4 arg1 = as_int4($Arg1VarName);
     arg0 &= arg1;
     arg1 = (~arg1) & INT_MAX;
     arg0 |= arg1;
     int4 temp0 = as_int4(_mm_min_epi32((__m128i)arg0, (__m128i)__builtin_shufflevector(arg0, arg0, 2, 3, 0, 1)));
     $Arg0Type temp1 = as_$Arg0Type(temp0);
     return min(temp1[0], temp1[1]);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx128f32 = OclBuiltinImpl<sub_group_reduce_min_vec, [v4f32], 0,
  [{
     int4 arg0 = as_int4($Arg0VarName);
     int4 arg1 = as_int4($Arg1VarName);
     arg0 &= arg1;
     arg1 = (~arg1) & as_int(INFINITY);
     arg0 |= arg1;
     int4 temp0 = as_int4(_mm_min_ps((__m128)arg0, (__m128)__builtin_shufflevector(arg0, arg0, 2, 3, 0, 1)));
     $Arg0Type temp1 = as_$Arg0Type(temp0);
     return min(temp1[0], temp1[1]);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx128u64 = OclBuiltinImpl<sub_group_reduce_min_vec, [v4u64], 0,
  [{
     long4 arg0 = as_long4($Arg0VarName);
     long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
     arg0 |= ~arg1;
     long2 temp0;
     long2 mask2 = __builtin_convertvector(as_$Arg0BaseType#2(arg0.lo) > as_$Arg0BaseType#2(arg0.hi), long2);
     temp0 = arg0.lo & ~mask2;
     temp0 |= arg0.hi & mask2;
     $Arg0BaseType#2 temp1 = as_$Arg0BaseType#2(temp0);
     return min(temp1.lo, temp1.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx128i64 = OclBuiltinImpl<sub_group_reduce_min_vec, [v4i64], 0,
  [{
     long4 arg0 = as_long4($Arg0VarName);
     long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
     arg0 &= arg1;
     arg1 = (~arg1) & LONG_MAX;
     arg0 |= arg1;
     long2 temp0;
     long2 mask2 = as_long2( _mm_cmpgt_epi64((__m128i)arg0.lo, (__m128i)arg0.hi));
     temp0 = arg0.lo & ~mask2;
     temp0 |= arg0.hi & mask2;
     $Arg0BaseType#2 temp1 = as_$Arg0BaseType#2(temp0);
     return min(temp1.lo, temp1.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx128f64 = OclBuiltinImpl<sub_group_reduce_min_vec, [v4f64], 0,
  [{
     long4 arg0 = as_long4($Arg0VarName);
     long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
     arg0 &= arg1;
     arg1 = (~arg1) & as_long((double)INFINITY);
     arg0 |= arg1;
     long2 temp0;
     long2 mask2 = as_long2(_mm_cmpgt_pd((__m128d)arg0.lo, (__m128d)arg0.hi));
     temp0 = arg0.lo & ~mask2;
     temp0 |= arg0.hi & mask2;
     $Arg0BaseType#2 temp1 = as_$Arg0BaseType#2(temp0);
     return min(temp1.lo, temp1.hi);
  }]>;

// reduce max
OclBuiltinImpl sub_group_reduce_max_avx128u32 = OclBuiltinImpl<sub_group_reduce_max_vec, [v4u32], 0,
  [{
     int4 arg0 = as_int4($Arg0VarName);
     int4 arg1 = as_int4($Arg1VarName);
     arg0 &= arg1;
     int4 temp0 = as_int4(_mm_max_epu32((__m128i)arg0, (__m128i)__builtin_shufflevector(arg0, arg0, 2, 3, 0, 1)));
     $Arg0Type temp1 = as_$Arg0Type(temp0);
     return max(temp1[0], temp1[1]);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx128i32 = OclBuiltinImpl<sub_group_reduce_max_vec, [v4i32], 0,
  [{
     int4 arg0 = as_int4($Arg0VarName);
     int4 arg1 = as_int4($Arg1VarName);
     arg0 &= arg1;
     arg1 = (~arg1) & INT_MIN;
     arg0 |= arg1;
     int4 temp0 = as_int4(_mm_max_epi32((__m128i)arg0, (__m128i)__builtin_shufflevector(arg0, arg0, 2, 3, 0, 1)));
     $Arg0Type temp1 = as_$Arg0Type(temp0);
     return max(temp1[0], temp1[1]);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx128f32 = OclBuiltinImpl<sub_group_reduce_max_vec, [v4f32], 0,
  [{
     int4 arg0 = as_int4($Arg0VarName);
     int4 arg1 = as_int4($Arg1VarName);
     arg0 &= arg1;
     arg1 = (~arg1) & as_int(-INFINITY);
     arg0 |= arg1;
     int4 temp0 = as_int4(_mm_max_ps((__m128)arg0, (__m128)__builtin_shufflevector(arg0, arg0, 2, 3, 0, 1)));
     $Arg0Type temp1 = as_$Arg0Type(temp0);
     return max(temp1[0], temp1[1]);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx128u64 = OclBuiltinImpl<sub_group_reduce_max_vec, [v4u64], 0,
  [{
     long4 arg0 = as_long4($Arg0VarName);
     long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
     arg0 &= arg1;
     long2 temp0;
     long2 mask2 = __builtin_convertvector(as_$Arg0BaseType#2(arg0.lo) > as_$Arg0BaseType#2(arg0.hi), long2);
     temp0 = arg0.lo & mask2;
     temp0 |= arg0.hi & ~mask2;
     $Arg0BaseType#2 temp1 = as_$Arg0BaseType#2(temp0);
     return max(temp1.lo, temp1.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx128i64 = OclBuiltinImpl<sub_group_reduce_max_vec, [v4i64], 0,
  [{
     long4 arg0 = as_long4($Arg0VarName);
     long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
     arg0 &= arg1;
     arg1 = (~arg1) & LONG_MIN;
     arg0 |= arg1;
     long2 temp0;
     long2 mask2 = as_long2( _mm_cmpgt_epi64((__m128i)arg0.lo, (__m128i)arg0.hi));
     temp0 = arg0.lo & mask2;
     temp0 |= arg0.hi & ~mask2;
     $Arg0BaseType#2 temp1 = as_$Arg0BaseType#2(temp0);
     return max(temp1.lo, temp1.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx128f64 = OclBuiltinImpl<sub_group_reduce_max_vec, [v4f64], 0,
  [{
     long4 arg0 = as_long4($Arg0VarName);
     long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
     arg0 &= arg1;
     arg1 = (~arg1) & as_long((double)-INFINITY);
     arg0 |= arg1;
     long2 temp0;
     long2 mask2 = as_long2(_mm_cmpgt_pd((__m128d)arg0.lo, (__m128d)arg0.hi));
     temp0 = arg0.lo & mask2;
     temp0 |= arg0.hi & ~mask2;
     $Arg0BaseType#2 temp1 = as_$Arg0BaseType#2(temp0);
     return max(temp1.lo, temp1.hi);
  }]>;

// inclusive scan
// add
OclBuiltinImpl sub_group_scan_inclusive_add_avx128iu32 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [v4i32, v4u32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp[3] = 0;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_add_epi32((__m128i)$Arg0VarName, (__m128i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx128f32 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [v4f32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp[3] = 0;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_add_ps((__m128)$Arg0VarName, (__m128)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx128iu64 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [v4i64, v4u64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp[3] = 0;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName.lo = as_$Arg0BaseType#2(_mm_add_epi64((__m128i)$Arg0VarName.lo, (__m128i)temp.lo));
      $Arg0VarName.hi = as_$Arg0BaseType#2(_mm_add_epi64((__m128i)$Arg0VarName.hi, (__m128i)temp.hi));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx128f64 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [v4f64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp[3] = 0;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName.lo = as_$Arg0BaseType#2(_mm_add_pd((__m128d)$Arg0VarName.lo, (__m128d)temp.lo));
      $Arg0VarName.hi = as_$Arg0BaseType#2(_mm_add_pd((__m128d)$Arg0VarName.hi, (__m128d)temp.hi));
    }
    return $Arg0VarName;
  }]>;

// min
OclBuiltinImpl sub_group_scan_inclusive_min_avx128i32 =  OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [v4i32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & INT_MAX;
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp[3] = INT_MAX;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_min_epi32((__m128i)$Arg0VarName, (__m128i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx128u32 =  OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [v4u32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 |= ~arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp[3] = UINT_MAX;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_min_epu32((__m128i)$Arg0VarName, (__m128i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx128f32 =  OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [v4f32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & as_int(INFINITY);
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp[3] = INFINITY;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_min_ps((__m128)$Arg0VarName, (__m128)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx128i64 =  OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [v4i64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    arg0 &= arg1;
    arg1 = (~arg1) & LONG_MAX;
    arg0 |= arg1;
    long4 temp0 = arg0;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp0[3] = LONG_MAX;
      temp0 = __builtin_shufflevector(temp0, temp0, 3, 0, 1, 2);
      long2 mask2 = as_long2( _mm_cmpgt_epi64((__m128i)arg0.lo, (__m128i)temp0.lo));
      arg0.lo &= ~mask2;
      arg0.lo |= temp0.lo & mask2;
      mask2 = as_long2( _mm_cmpgt_epi64((__m128i)arg0.hi, (__m128i)temp0.hi));
      arg0.hi &= ~mask2;
      arg0.hi |= temp0.hi & mask2;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx128u64 =  OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [v4u64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    arg0 |= ~arg1;
    long4 temp0 = arg0;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp0[3] = as_long(ULONG_MAX);
      temp0 = __builtin_shufflevector(temp0, temp0, 3, 0, 1, 2);
      long4 mask4 = __builtin_convertvector(as_$Arg0Type(arg0) > as_$Arg0Type(temp0), long4);
      arg0 &= ~mask4;
      arg0 |= temp0 & mask4;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx128f64 =  OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [v4f64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    long INF = as_long((double)INFINITY);
    arg0 &= arg1;
    arg1 = (~arg1) & INF;
    arg0 |= arg1;
    long4 temp0 = arg0;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp0[3] = INF;
      temp0 = __builtin_shufflevector(temp0, temp0, 3, 0, 1, 2);
      arg0.lo = as_long2( _mm_min_pd((__m128d)arg0.lo, (__m128d)temp0.lo));
      arg0.hi = as_long2( _mm_min_pd((__m128d)arg0.hi, (__m128d)temp0.hi));
    }
    return as_$Arg0Type(arg0);
  }]>;

// max
OclBuiltinImpl sub_group_scan_inclusive_max_avx128i32 =  OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [v4i32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & INT_MIN;
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp[3] = INT_MIN;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_max_epi32((__m128i)$Arg0VarName, (__m128i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx128u32 =  OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [v4u32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp[3] = 0;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_max_epu32((__m128i)$Arg0VarName, (__m128i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx128f32 =  OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [v4f32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & as_int(-INFINITY);
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp[3] = -INFINITY;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_max_ps((__m128)$Arg0VarName, (__m128)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx128i64 =  OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [v4i64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    arg0 &= arg1;
    arg1 = (~arg1) & LONG_MIN;
    arg0 |= arg1;
    long4 temp0 = arg0;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp0[3] = LONG_MIN;
      temp0 = __builtin_shufflevector(temp0, temp0, 3, 0, 1, 2);
      long2 mask2 = as_long2( _mm_cmpgt_epi64((__m128i)arg0.lo, (__m128i)temp0.lo));
      arg0.lo &= mask2;
      arg0.lo |= temp0.lo & ~mask2;
      mask2 = as_long2( _mm_cmpgt_epi64((__m128i)arg0.hi, (__m128i)temp0.hi));
      arg0.hi &= mask2;
      arg0.hi |= temp0.hi & ~mask2;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx128u64 =  OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [v4u64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    arg0 &= arg1;
    long4 temp0 = arg0;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp0[3] = 0;
      temp0 = __builtin_shufflevector(temp0, temp0, 3, 0, 1, 2);
      long4 mask4 = __builtin_convertvector(as_$Arg0Type(arg0) > as_$Arg0Type(temp0), long4);
      arg0 &= mask4;
      arg0 |= temp0 & ~mask4;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx128f64 =  OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [v4f64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    long INF = as_long((double)-INFINITY);
    arg0 &= arg1;
    arg1 = (~arg1) & INF;
    arg0 |= arg1;
    long4 temp0 = arg0;

#pragma clang unroll(full)
    for (int i = 0; i < 3; i++) {
      temp0[3] = INF;
      temp0 = __builtin_shufflevector(temp0, temp0, 3, 0, 1, 2);
      arg0.lo = as_long2( _mm_max_pd((__m128d)arg0.lo, (__m128d)temp0.lo));
      arg0.hi = as_long2( _mm_max_pd((__m128d)arg0.hi, (__m128d)temp0.hi));
    }
    return as_$Arg0Type(arg0);
  }]>;

// exclusive scan
// add
OclBuiltinImpl sub_group_scan_exclusive_add_avx128iu32 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [v4i32, v4u32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
    $Arg0VarName[0] = 0;
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = 0;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_add_epi32((__m128i)$Arg0VarName, (__m128i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx128f32 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [v4f32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
    $Arg0VarName[0] = 0;
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = 0;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_add_ps((__m128)$Arg0VarName, (__m128)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx128iu64 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [v4i64, v4u64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
    $Arg0VarName[0] = 0;
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = 0;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName.hi = as_$Arg0BaseType#2(_mm_add_epi64((__m128i)$Arg0VarName.hi, (__m128i)temp.hi));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx128f64 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [v4f64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
    $Arg0VarName[0] = 0;
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = 0;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName.hi = as_$Arg0BaseType#2(_mm_add_pd((__m128d)$Arg0VarName.hi, (__m128d)temp.hi));
    }
    return $Arg0VarName;
  }]>;

//min
OclBuiltinImpl sub_group_scan_exclusive_min_avx128i32 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [v4i32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & INT_MAX;
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
    $Arg0VarName[0] = INT_MAX;
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = INT_MAX;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_min_epi32((__m128i)$Arg0VarName, (__m128i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx128u32 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [v4u32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 |= ~arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
    $Arg0VarName[0] = UINT_MAX;
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = UINT_MAX;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_min_epu32((__m128i)$Arg0VarName, (__m128i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx128f32 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [v4f32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & as_int(INFINITY);
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
    $Arg0VarName[0] = INFINITY;
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = INFINITY;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_min_ps((__m128)$Arg0VarName, (__m128)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx128i64 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [v4i64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    arg0 &= arg1;
    arg1 = (~arg1) & LONG_MAX;
    arg0 |= arg1;

    arg0 = __builtin_shufflevector(arg0, arg0, 3, 0, 1, 2);
    arg0[0] = LONG_MAX;
    long4 temp = arg0;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = LONG_MAX;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      long2 mask2 = as_long2( _mm_cmpgt_epi64((__m128i)arg0.hi, (__m128i)temp.hi));
      arg0.hi &= ~mask2;
      arg0.hi |= temp.hi & mask2;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx128u64 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [v4u64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    arg0 |= ~arg1;

    arg0 = __builtin_shufflevector(arg0, arg0, 3, 0, 1, 2);
    arg0[0] = as_long(ULONG_MAX);
    long4 temp = arg0;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = as_long(ULONG_MAX);
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      long4 mask4 = __builtin_convertvector(as_$Arg0Type(arg0) > as_$Arg0Type(temp), long4);
      arg0 &= ~mask4;
      arg0 |= temp & mask4;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx128f64 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [v4f64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);

    long INF = as_long((double)INFINITY);
    arg0 &= arg1;
    arg1 = (~arg1) & INF;
    arg0 |= arg1;

    arg0 = __builtin_shufflevector(arg0, arg0, 3, 0, 1, 2);
    arg0[0] = INF;
    long4 temp = arg0;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = INF;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      arg0.hi = as_long2(_mm_min_pd((__m128d)arg0.hi, (__m128d)temp.hi));
    }
    return as_$Arg0Type(arg0);
  }]>;

//max
OclBuiltinImpl sub_group_scan_exclusive_max_avx128i32 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [v4i32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & INT_MIN;
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
    $Arg0VarName[0] = INT_MIN;
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = INT_MIN;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_max_epi32((__m128i)$Arg0VarName, (__m128i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx128u32 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [v4u32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 |= ~arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
    $Arg0VarName[0] = 0;
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = 0;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_max_epu32((__m128i)$Arg0VarName, (__m128i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx128f32 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [v4f32], 0,
  [{
    int4 arg0 = as_int4($Arg0VarName);
    int4 arg1 = as_int4($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & as_int(-INFINITY);
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
    $Arg0VarName[0] = -INFINITY;
    $Arg0Type temp = $Arg0VarName;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = -INFINITY;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName = as_$Arg0Type(_mm_max_ps((__m128)$Arg0VarName, (__m128)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx128i64 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [v4i64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    arg0 &= arg1;
    arg1 = (~arg1) & LONG_MIN;
    arg0 |= arg1;

    arg0 = __builtin_shufflevector(arg0, arg0, 3, 0, 1, 2);
    arg0[0] = LONG_MIN;
    long4 temp = arg0;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = LONG_MIN;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      long2 mask2 = as_long2( _mm_cmpgt_epi64((__m128i)arg0.hi, (__m128i)temp.hi));
      arg0.hi &= mask2;
      arg0.hi |= temp.hi & ~mask2;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx128u64 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [v4u64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);
    arg0 |= ~arg1;

    arg0 = __builtin_shufflevector(arg0, arg0, 3, 0, 1, 2);
    arg0[0] = 0;
    long4 temp = arg0;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = 0;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      long4 mask4 = __builtin_convertvector(as_$Arg0Type(arg0) > as_$Arg0Type(temp), long4);
      arg0 &= mask4;
      arg0 |= temp & ~mask4;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx128f64 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [v4f64], 0,
  [{
    long4 arg0 = as_long4($Arg0VarName);
    long4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), long4);

    long INF = as_long((double)-INFINITY);
    arg0 &= arg1;
    arg1 = (~arg1) & INF;
    arg0 |= arg1;

    arg0 = __builtin_shufflevector(arg0, arg0, 3, 0, 1, 2);
    arg0[0] = INF;
    long4 temp = arg0;

#pragma clang unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = INF;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      arg0.hi = as_long2(_mm_max_pd((__m128d)arg0.hi, (__m128d)temp.hi));
    }
    return as_$Arg0Type(arg0);
  }]>;


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

// char, uchar, short, ushort
OclBuiltinImpl sub_group_shuffle_avx128v4ui8ui16 = OclBuiltinImpl<sub_group_shuffle_avx, [v4i8, v4u8, v4i16, v4u16], 0,
  [{
    int4 temp = __builtin_convertvector($Arg0VarName, int4);
    temp = intel_sub_group_shuffle(temp, $Arg1VarName, $Arg2VarName);
    $Arg0VarName = __builtin_convertvector(temp, $Arg0Type);
    return $Arg0VarName;
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

// char2 and uchar2
OclBuiltinImpl sub_group_shuffle_avx128v8ui8 = OclBuiltinImpl<sub_group_shuffle_avx, [v8i8, v8u8], 0,
  [{
    int8 temp = __builtin_convertvector($Arg0VarName, int8);
    temp = intel_sub_group_shuffle(temp, $Arg1VarName, $Arg2VarName);
    $Arg0VarName = __builtin_convertvector(temp, $Arg0Type);
    return $Arg0VarName;
  }]>;

// short2, ushort2, char4, uchar4
OclBuiltinImpl sub_group_shuffle_avx128v8iu16v16iu8 = OclBuiltinImpl<sub_group_shuffle_avx, [v8i16, v8u16, v16i8, v16u8], 0,
  [{
    int4 temp = __builtin_astype($Arg0VarName, int4);
    temp = intel_sub_group_shuffle(temp, $Arg1VarName, $Arg2VarName);
    return __builtin_astype(temp, $ReturnType);
  }]>;

OclBuiltinImpl sub_group_shuffle_avxv16uif32 = OclBuiltinImpl<sub_group_shuffle_avx, [v16u32, v16i32, v16f32], 0,
  [{
    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15);

    $Arg0BaseType#4 t1 = $Arg0VarName.lo.lo;
    $Arg0BaseType#4 t2 = $Arg0VarName.lo.hi;
    $Arg0BaseType#4 t3 = $Arg0VarName.hi.lo;
    $Arg0BaseType#4 t4 = $Arg0VarName.hi.hi;
    $Arg0BaseType#16 res;
    res.lo.lo = intel_sub_group_shuffle(t1, $Arg1VarName, $Arg2VarName);
    res.lo.hi = intel_sub_group_shuffle(t2, $Arg1VarName, $Arg2VarName);
    res.hi.lo = intel_sub_group_shuffle(t3, $Arg1VarName, $Arg2VarName);
    res.hi.hi = intel_sub_group_shuffle(t4, $Arg1VarName, $Arg2VarName);
    res = __builtin_shufflevector(res, res, 0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15);
    return res;
  }]>;

// short4, ushort4, char8, uchar8
OclBuiltinImpl sub_group_shuffle_avx128v16iu16v32iu8 = OclBuiltinImpl<sub_group_shuffle_avx, [v16i16, v16u16, v32i8, v32u8], 0,
  [{
    int8 temp = __builtin_astype($Arg0VarName, int8);
    temp = intel_sub_group_shuffle(temp, $Arg1VarName, $Arg2VarName);
    return __builtin_astype(temp, $ReturnType);
  }]>;

OclBuiltinImpl sub_group_shuffle_avxv32uif32 = OclBuiltinImpl<sub_group_shuffle_avx, [v32u32, v32i32, v32f32], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x4($Arg0VarName);
    $Arg0BaseType#4 t1 = $Arg0VarName.lo.lo.lo;
    $Arg0BaseType#4 t2 = $Arg0VarName.lo.lo.hi;
    $Arg0BaseType#4 t3 = $Arg0VarName.lo.hi.lo;
    $Arg0BaseType#4 t4 = $Arg0VarName.lo.hi.hi;
    $Arg0BaseType#4 t5 = $Arg0VarName.hi.lo.lo;
    $Arg0BaseType#4 t6 = $Arg0VarName.hi.lo.hi;
    $Arg0BaseType#4 t7 = $Arg0VarName.hi.hi.lo;
    $Arg0BaseType#4 t8 = $Arg0VarName.hi.hi.hi;
    $Arg0BaseType#32 res;
    res.lo.lo.lo = intel_sub_group_shuffle(t1, $Arg1VarName, $Arg2VarName);
    res.lo.lo.hi = intel_sub_group_shuffle(t2, $Arg1VarName, $Arg2VarName);
    res.lo.hi.lo = intel_sub_group_shuffle(t3, $Arg1VarName, $Arg2VarName);
    res.lo.hi.hi = intel_sub_group_shuffle(t4, $Arg1VarName, $Arg2VarName);
    res.hi.lo.lo = intel_sub_group_shuffle(t5, $Arg1VarName, $Arg2VarName);
    res.hi.lo.hi = intel_sub_group_shuffle(t6, $Arg1VarName, $Arg2VarName);
    res.hi.hi.lo = intel_sub_group_shuffle(t7, $Arg1VarName, $Arg2VarName);
    res.hi.hi.hi = intel_sub_group_shuffle(t8, $Arg1VarName, $Arg2VarName);
    res = __ocl_shuffle_transpose_$Arg0BaseType_4x8(res);
    return res;
  }]>;

// short8, ushort8, char16, uchar16
OclBuiltinImpl sub_group_shuffle_avx128v32iu16v64iu8 = OclBuiltinImpl<sub_group_shuffle_avx, [v32i16, v32u16, v64i8, v64u8], 0,
  [{
    int16 temp = __builtin_astype($Arg0VarName, int16);
    temp = intel_sub_group_shuffle(temp, $Arg1VarName, $Arg2VarName);
    return __builtin_astype(temp, $ReturnType);
  }]>;

OclBuiltinImpl sub_group_shuffle_avxv64uif32 = OclBuiltinImpl<sub_group_shuffle_avx, [v64u32, v64i32, v64f32], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x4($Arg0VarName);
    $Arg0BaseType#4 t1  = $Arg0VarName.lo.lo.lo.lo;
    $Arg0BaseType#4 t2  = $Arg0VarName.lo.lo.lo.hi;
    $Arg0BaseType#4 t3  = $Arg0VarName.lo.lo.hi.lo;
    $Arg0BaseType#4 t4  = $Arg0VarName.lo.lo.hi.hi;
    $Arg0BaseType#4 t5  = $Arg0VarName.lo.hi.lo.lo;
    $Arg0BaseType#4 t6  = $Arg0VarName.lo.hi.lo.hi;
    $Arg0BaseType#4 t7  = $Arg0VarName.lo.hi.hi.lo;
    $Arg0BaseType#4 t8  = $Arg0VarName.lo.hi.hi.hi;
    $Arg0BaseType#4 t9  = $Arg0VarName.hi.lo.lo.lo;
    $Arg0BaseType#4 t10 = $Arg0VarName.hi.lo.lo.hi;
    $Arg0BaseType#4 t11 = $Arg0VarName.hi.lo.hi.lo;
    $Arg0BaseType#4 t12 = $Arg0VarName.hi.lo.hi.hi;
    $Arg0BaseType#4 t13 = $Arg0VarName.hi.hi.lo.lo;
    $Arg0BaseType#4 t14 = $Arg0VarName.hi.hi.lo.hi;
    $Arg0BaseType#4 t15 = $Arg0VarName.hi.hi.hi.lo;
    $Arg0BaseType#4 t16 = $Arg0VarName.hi.hi.hi.hi;
    $Arg0BaseType#64 res;
    res.lo.lo.lo.lo = intel_sub_group_shuffle(t1,  $Arg1VarName, $Arg2VarName);
    res.lo.lo.lo.hi = intel_sub_group_shuffle(t2,  $Arg1VarName, $Arg2VarName);
    res.lo.lo.hi.lo = intel_sub_group_shuffle(t3,  $Arg1VarName, $Arg2VarName);
    res.lo.lo.hi.hi = intel_sub_group_shuffle(t4,  $Arg1VarName, $Arg2VarName);
    res.lo.hi.lo.lo = intel_sub_group_shuffle(t5,  $Arg1VarName, $Arg2VarName);
    res.lo.hi.lo.hi = intel_sub_group_shuffle(t6,  $Arg1VarName, $Arg2VarName);
    res.lo.hi.hi.lo = intel_sub_group_shuffle(t7,  $Arg1VarName, $Arg2VarName);
    res.lo.hi.hi.hi = intel_sub_group_shuffle(t8,  $Arg1VarName, $Arg2VarName);
    res.hi.lo.lo.lo = intel_sub_group_shuffle(t9,  $Arg1VarName, $Arg2VarName);
    res.hi.lo.lo.hi = intel_sub_group_shuffle(t10, $Arg1VarName, $Arg2VarName);
    res.hi.lo.hi.lo = intel_sub_group_shuffle(t11, $Arg1VarName, $Arg2VarName);
    res.hi.lo.hi.hi = intel_sub_group_shuffle(t12, $Arg1VarName, $Arg2VarName);
    res.hi.hi.lo.lo = intel_sub_group_shuffle(t13, $Arg1VarName, $Arg2VarName);
    res.hi.hi.lo.hi = intel_sub_group_shuffle(t14, $Arg1VarName, $Arg2VarName);
    res.hi.hi.hi.lo = intel_sub_group_shuffle(t15, $Arg1VarName, $Arg2VarName);
    res.hi.hi.hi.hi = intel_sub_group_shuffle(t16, $Arg1VarName, $Arg2VarName);
    res = __ocl_shuffle_transpose_$Arg0BaseType_4x16(res);
    return res;
  }]>;

// short16, ushort16
OclBuiltinImpl sub_group_shuffle_avx128v64iu16 = OclBuiltinImpl<sub_group_shuffle_avx, [v64i16, v64u16], 0,
  [{
    int32 temp = __builtin_astype($Arg0VarName, int32);
    temp = intel_sub_group_shuffle(temp, $Arg1VarName, $Arg2VarName);
    return __builtin_astype(temp, $ReturnType);
  }]>;

//
// Shuffle xor
//
OclBuiltinImpl sub_group_shuffle_xor_avx128v4ui32 = OclBuiltinImpl<sub_group_shuffle_xor_avx,
               [v4i32, v4u32, v4f32, v4i64, v4u64, v4f64, v4i8, v4u8, v4i16, v4u16,
                v8u32, v8i32, v8f32, v8i8, v8u8, v8i16, v8u16,
                v16i32, v16u32, v16f32, v16i8, v16u8, v16i16, v16u16,
                v32i32, v32u32, v32f32, v32i8, v32u8, v32i16, v32u16,
                v64i32, v64u32, v64f32, v64i8, v64u8, v64i16, v64u16], 0,
  [{
    uint4 indexes = {0, 1, 2, 3};
    indexes = indexes ^ $Arg1VarName;
    return intel_sub_group_shuffle($Arg0VarName, indexes, $Arg2VarName);
  }]>;

//
// Shuffle down
//

OclBuiltinImpl sub_group_shuffle_down_avx_gen = OclBuiltinImpl<sub_group_shuffle_down_avx,
               [v4i32, v4u32,  v4f32,
                v8i32, v16i32, v32i32, v64i32,
                v8u32, v16u32, v32u32, v64u32,
                v8f32, v16f32, v32f32, v64f32], 0,
  [{
    $Arg0Type res_cur, res_next;
    int$VecLength temp;
    uint4 sg_indexes = { 0, 1, 2, 3 };
    const uint max_sg_size = 4;

    sg_indexes = sg_indexes + $Arg2VarName;

    sg_indexes &= $Arg3VarName;

    int4 indexes_mask = sg_indexes < max_sg_size;
    int4 inv_indexes_mask = !indexes_mask;

    int$VecLength ext_mask = __ocl_extend_mask_to_$VecLength(indexes_mask);

    res_cur = intel_sub_group_shuffle($Arg0VarName, sg_indexes, *(uint4*)&indexes_mask);
    temp = *((int$VecLength*)&res_cur) & ext_mask;
    res_cur = *($Arg0Type*)&temp;

    res_next = intel_sub_group_shuffle($Arg1VarName, (sg_indexes - max_sg_size), *(uint4*)&inv_indexes_mask);
    temp = *((int$VecLength*)&res_next) & !ext_mask;
    res_next = *($Arg0Type*)&temp;

    temp = (*(int$VecLength*)&res_cur | *(int$VecLength*)&res_next);
    return *($Arg0Type*)&temp;
  }]>;

// char, uchar
OclBuiltinImpl sub_group_shuffle_down_avx_i8 = OclBuiltinImpl<sub_group_shuffle_down_avx,
               [v4i8, v4u8, v8i8, v8u8, v16i8, v16u8, v32i8, v32u8, v64i8, v64u8], 0,
  [{
    $Arg0Type res_cur, res_next;
    char$VecLength temp;
    uint4 sg_indexes = { 0, 1, 2, 3 };
    const uint max_sg_size = 4;

    sg_indexes = sg_indexes + $Arg2VarName;

    sg_indexes &= $Arg3VarName;

    int4 indexes_mask = sg_indexes < max_sg_size;
    int4 inv_indexes_mask = !indexes_mask;

    char$VecLength ext_mask = __builtin_convertvector(__ocl_extend_mask_to_$VecLength(indexes_mask), char$VecLength);

    res_cur = intel_sub_group_shuffle($Arg0VarName, sg_indexes, *(uint4*)&indexes_mask);
    temp = *((char$VecLength*)&res_cur) & ext_mask;
    res_cur = *($Arg0Type*)&temp;

    res_next = intel_sub_group_shuffle($Arg1VarName, (sg_indexes - max_sg_size), *(uint4*)&inv_indexes_mask);
    temp = *((char$VecLength*)&res_next) & !ext_mask;
    res_next = *($Arg0Type*)&temp;

    return res_cur | res_next;
  }]>;

// short, ushort
OclBuiltinImpl sub_group_shuffle_down_avx_i16 = OclBuiltinImpl<sub_group_shuffle_down_avx,
               [v4i16, v4u16, v8i16, v8u16, v16i16, v16u16, v32i16, v32u16, v64i16, v64u16], 0,
  [{
    $Arg0Type res_cur, res_next;
    short$VecLength temp;
    uint4 sg_indexes = { 0, 1, 2, 3 };
    const uint max_sg_size = 4;

    sg_indexes = sg_indexes + $Arg2VarName;

    sg_indexes &= $Arg3VarName;

    int4 indexes_mask = sg_indexes < max_sg_size;
    int4 inv_indexes_mask = !indexes_mask;

    short$VecLength ext_mask = __builtin_convertvector(__ocl_extend_mask_to_$VecLength(indexes_mask), short$VecLength);

    res_cur = intel_sub_group_shuffle($Arg0VarName, sg_indexes, *(uint4*)&indexes_mask);
    temp = *((short$VecLength*)&res_cur) & ext_mask;
    res_cur = *($Arg0Type*)&temp;

    res_next = intel_sub_group_shuffle($Arg1VarName, (sg_indexes - max_sg_size), *(uint4*)&inv_indexes_mask);
    temp = *((short$VecLength*)&res_next) & !ext_mask;
    res_next = *($Arg0Type*)&temp;

    return res_cur | res_next;
  }]>;

OclBuiltinImpl sub_group_shuffle_down_avx_v4d64 = OclBuiltinImpl<sub_group_shuffle_down_avx, [v4i64, v4u64, v4f64], 0,
  [{
    $Arg0Type res_cur, res_next;

    int8 temp;
    uint4 sg_indexes = { 0, 1, 2, 3 };
    const uint max_sg_size = 4;

    sg_indexes = sg_indexes + $Arg2VarName;

    sg_indexes &= $Arg3VarName;

    int4 indexes_mask = sg_indexes < max_sg_size;
    int4 inv_indexes_mask = !indexes_mask;

    int8 ext_mask = __ocl_extend_mask_to_8(indexes_mask);

    res_cur = intel_sub_group_shuffle($Arg0VarName, sg_indexes, *(uint4*)&indexes_mask);
    temp = *((int8*)&res_cur) & ext_mask;
    res_cur = *($Arg0Type*)&temp;

    res_next = intel_sub_group_shuffle($Arg1VarName, (sg_indexes - max_sg_size), *(uint4*)&inv_indexes_mask);
    temp = *((int8*)&res_next) & !ext_mask;
    res_next = *($Arg0Type*)&temp;

    temp = (*(int8*)&res_cur | *(int8*)&res_next);
    return *($Arg0Type*)&temp;
  }]>;

//
// Shuffle up
//

OclBuiltinImpl sub_group_shuffle_up_avx_gen = OclBuiltinImpl<sub_group_shuffle_up_avx,
              [v4i32, v4u32, v4f32,
               v8i32, v16i32, v32i32, v64i32,
               v8u32, v16u32, v32u32, v64u32,
               v8f32, v16f32, v32f32, v64f32], 0,
  [{
    $Arg0Type res_cur, res_next;
    int$VecLength temp;
    int4 sg_indexes = { 0, 1, 2, 3};
    const int max_sg_size = 4;

    // Calculate indices
    sg_indexes = sg_indexes - *(int4*)&$Arg2VarName;

    // Apply vec_mask
    sg_indexes &= *(int4*)&$Arg3VarName;

    // Get the mask for elements which >= 0
    int4 indexes_mask = sg_indexes >= 0;
    int4 inv_indexes_mask = !indexes_mask;

    int$VecLength ext_mask = __ocl_extend_mask_to_$VecLength(indexes_mask);

    // Call masked shuffle 2 times: first for elements from current, second for elements from next
    // Apply extended mask to get rid of unnecessary elements
    res_cur = intel_sub_group_shuffle($Arg1VarName, *(uint4*)&sg_indexes, *(uint4*)&indexes_mask);
    temp = *((int$VecLength*)&res_cur) & ext_mask;
    res_cur = *($Arg0Type*)&temp;

    int4 other_ind = sg_indexes + max_sg_size;
    res_next = intel_sub_group_shuffle($Arg0VarName, *(uint4*)&other_ind, *(uint4*)&inv_indexes_mask);
    temp = *((int$VecLength*)&res_next) & !ext_mask;
    res_next = *($Arg0Type*)&temp;

    temp = (*(int$VecLength*)&res_cur | *(int$VecLength*)&res_next);
    return *($Arg0Type*)&temp;
  }]>;

// char, uchar
OclBuiltinImpl sub_group_shuffle_up_avx_gen = OclBuiltinImpl<sub_group_shuffle_up_avx,
               [v4i8, v4u8, v8i8, v8u8, v16i8, v16u8, v32i8, v32u8, v64i8, v64u8], 0,
  [{
    $Arg0Type res_cur, res_next;
    char$VecLength temp;
    int4 sg_indexes = { 0, 1, 2, 3};
    const int max_sg_size = 4;

    // Calculate indices
    sg_indexes = sg_indexes - *(int4*)&$Arg2VarName;

    // Apply vec_mask
    sg_indexes &= *(int4*)&$Arg3VarName;

    // Get the mask for elements which >= 0
    int4 indexes_mask = sg_indexes >= 0;
    int4 inv_indexes_mask = !indexes_mask;

    char$VecLength ext_mask = __builtin_convertvector(__ocl_extend_mask_to_$VecLength(indexes_mask), char$VecLength);

    // Call masked shuffle 2 times: first for elements from current, second for elements from next
    // Apply extended mask to get rid of unnecessary elements
    res_cur = intel_sub_group_shuffle($Arg1VarName, *(uint4*)&sg_indexes, *(uint4*)&indexes_mask);
    temp = *((char$VecLength*)&res_cur) & ext_mask;
    res_cur = *($Arg0Type*)&temp;

    int4 other_ind = sg_indexes + max_sg_size;
    res_next = intel_sub_group_shuffle($Arg0VarName, *(uint4*)&other_ind, *(uint4*)&inv_indexes_mask);
    temp = *((char$VecLength*)&res_next) & !ext_mask;
    res_next = *($Arg0Type*)&temp;

    return res_cur | res_next;
  }]>;

// short, ushort
OclBuiltinImpl sub_group_shuffle_up_avx_gen = OclBuiltinImpl<sub_group_shuffle_up_avx,
               [v4i16, v4u16, v8i16, v8u16, v16i16, v16u16, v32i16, v32u16, v64i16, v64u16], 0,
  [{
    $Arg0Type res_cur, res_next;
    short$VecLength temp;
    int4 sg_indexes = { 0, 1, 2, 3};
    const int max_sg_size = 4;

    // Calculate indices
    sg_indexes = sg_indexes - *(int4*)&$Arg2VarName;

    // Apply vec_mask
    sg_indexes &= *(int4*)&$Arg3VarName;

    // Get the mask for elements which >= 0
    int4 indexes_mask = sg_indexes >= 0;
    int4 inv_indexes_mask = !indexes_mask;

    short$VecLength ext_mask = __builtin_convertvector(__ocl_extend_mask_to_$VecLength(indexes_mask), short$VecLength);

    // Call masked shuffle 2 times: first for elements from current, second for elements from next
    // Apply extended mask to get rid of unnecessary elements
    res_cur = intel_sub_group_shuffle($Arg1VarName, *(uint4*)&sg_indexes, *(uint4*)&indexes_mask);
    temp = *((short$VecLength*)&res_cur) & ext_mask;
    res_cur = *($Arg0Type*)&temp;

    int4 other_ind = sg_indexes + max_sg_size;
    res_next = intel_sub_group_shuffle($Arg0VarName, *(uint4*)&other_ind, *(uint4*)&inv_indexes_mask);
    temp = *((short$VecLength*)&res_next) & !ext_mask;
    res_next = *($Arg0Type*)&temp;

    return res_cur | res_next;
  }]>;

OclBuiltinImpl sub_group_shuffle_up_avx_v4d64 = OclBuiltinImpl<sub_group_shuffle_up_avx,
              [v4i64, v4u64, v4f64], 0,
  [{
    $Arg0Type res_cur, res_next;
    int8 temp;
    int4 sg_indexes = { 0, 1, 2, 3};
    const int max_sg_size = 4;

    // Calculate indexes
    sg_indexes = sg_indexes - *(int4*)&$Arg2VarName;

    // Apply vec_mask
    sg_indexes &= *(int4*)&$Arg3VarName;

    // Get the mask for elements which >= 0
    int4 indexes_mask = sg_indexes >= 0;
    int4 inv_indexes_mask = !indexes_mask;

    int8 ext_mask = __ocl_extend_mask_to_8(indexes_mask);

    // Call masked shuffle 2 times: first for elements from current, second for elements from next
    // Apply extended mask to get rid of unnecessary elements
    res_cur = intel_sub_group_shuffle($Arg1VarName, *(uint4*)&sg_indexes, *(uint4*)&indexes_mask);
    temp = *((int8*)&res_cur) & ext_mask;
    res_cur = *($Arg0Type*)&temp;

    int4 other_ind = sg_indexes + max_sg_size;
    res_next = intel_sub_group_shuffle($Arg0VarName, *(uint4*)&other_ind, *(uint4*)&inv_indexes_mask);
    temp = *((int8*)&res_next) & !ext_mask;
    res_next = *($Arg0Type*)&temp;

    temp = (*(int8*)&res_cur | *(int8*)&res_next);
    return *($Arg0Type*)&temp;
  }]>;

//
// Block read write
//
list<OclType> intel_sub_group_block_read_write_us_types = [ v1u64, v1u32, v1u16, v1u8 ];

code BlockRead1_4 =
  [{
    const $ReturnType res = vload4(0, $Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 1, 2, 3);
  }];
OclBuiltinImpl intel_sub_group_block_read1_4_avx = OclBuiltinImpl<intel_sub_group_block_read1_4, [ v1u32 ], 0, BlockRead1_4>;
OclBuiltinImpl intel_sub_group_block_read_us1_4_avx = OclBuiltinImpl<intel_sub_group_block_read_us1_4, intel_sub_group_block_read_write_us_types, 0, BlockRead1_4>;

code BlockRead2_4 =
  [{
    const $ReturnType res = vload8(0, $Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 4,
                                             1, 5,
                                             2, 6,
                                             3, 7);
  }];
OclBuiltinImpl intel_sub_group_block_read2_4_avx = OclBuiltinImpl<intel_sub_group_block_read2_4, [ v1u32 ], 0, BlockRead2_4>;
OclBuiltinImpl intel_sub_group_block_read_us2_4_avx = OclBuiltinImpl<intel_sub_group_block_read_us2_4, intel_sub_group_block_read_write_us_types, 0, BlockRead2_4>;

code BlockRead4_4 =
  [{
    const $ReturnType res = vload16(0, $Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 4, 8,  12,
                                             1, 5, 9,  13,
                                             2, 6, 10, 14,
                                             3, 7, 11, 15);
  }];
OclBuiltinImpl intel_sub_group_block_read4_4_avx = OclBuiltinImpl<intel_sub_group_block_read4_4, [ v1u32 ], 0, BlockRead4_4>;
OclBuiltinImpl intel_sub_group_block_read_us4_4_avx = OclBuiltinImpl<intel_sub_group_block_read_us4_4, intel_sub_group_block_read_write_us_types, 0, BlockRead4_4>;

code BlockRead8_4 =
  [{
    $ReturnType res;
    res.lo  = vload16(0, $Arg0VarName);
    res.hi  = vload16(1, $Arg0VarName);
    return __ocl_shuffle_transpose_$ReturnBaseType#_4x8(res);
  }];
OclBuiltinImpl intel_sub_group_block_read8_4_avx = OclBuiltinImpl<intel_sub_group_block_read8_4, [ v1u32 ], 0, BlockRead8_4>;
OclBuiltinImpl intel_sub_group_block_read_us8_4_avx = OclBuiltinImpl<intel_sub_group_block_read_us8_4, intel_sub_group_block_read_write_us_types, 0, BlockRead8_4>;

code BlockWrite1_4 =
  [{
    $Arg1BaseType#4 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 1, 2, 3);
    vstore4(res, 0, $Arg0VarName);
  }];
OclBuiltinImpl intel_sub_group_block_write1_4_avx = OclBuiltinImpl<intel_sub_group_block_write1_4, [ v1u32 ], 0, BlockWrite1_4>;
OclBuiltinImpl intel_sub_group_block_write_us1_4_avx = OclBuiltinImpl<intel_sub_group_block_write_us1_4, intel_sub_group_block_read_write_us_types, 0, BlockWrite1_4>;

code BlockWrite2_4 =
  [{
    $Arg1BaseType#8 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 2, 4, 6,
                                                                              1, 3, 5, 7);
    vstore8(res, 0, $Arg0VarName);
  }];
OclBuiltinImpl intel_sub_group_block_write2_4_avx = OclBuiltinImpl<intel_sub_group_block_write2_4, [ v1u32 ], 0, BlockWrite2_4>;
OclBuiltinImpl intel_sub_group_block_write_us2_4_avx = OclBuiltinImpl<intel_sub_group_block_write_us2_4, intel_sub_group_block_read_write_us_types, 0, BlockWrite2_4>;

code BlockWrite4_4 =
  [{
    $Arg1BaseType#16 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 4, 8,  12,
                                                                               1, 5, 9,  13,
                                                                               2, 6, 10, 14,
                                                                               3, 7, 11, 15);
    vstore16(res, 0, $Arg0VarName);
  }];
OclBuiltinImpl intel_sub_group_block_write4_4_avx = OclBuiltinImpl<intel_sub_group_block_write4_4, [ v1u32 ], 0, BlockWrite4_4>;
OclBuiltinImpl intel_sub_group_block_write_us4_4_avx = OclBuiltinImpl<intel_sub_group_block_write_us4_4, intel_sub_group_block_read_write_us_types, 0, BlockWrite4_4>;

code BlockWrite8_4 =
  [{
    $Arg1BaseType#32 res = __ocl_shuffle_transpose_$Arg1BaseType#_8x4($Arg1VarName);
    vstore16(res.lo, 0, $Arg0VarName);
    vstore16(res.hi, 1, $Arg0VarName);
  }];
OclBuiltinImpl intel_sub_group_block_write8_4_avx = OclBuiltinImpl<intel_sub_group_block_write8_4, [ v1u32 ], 0, BlockWrite8_4>;
OclBuiltinImpl intel_sub_group_block_write_us8_4_avx = OclBuiltinImpl<intel_sub_group_block_write_us8_4, intel_sub_group_block_read_write_us_types, 0, BlockWrite8_4>;

//
// Block read/write for image
//

list<OclType> intel_sub_group_block_read_write_us_types = [ v1u64, v1u32, v1u16, v1u8 ];

code BlockReadImg1_4 =
  [{
    $ReturnType* p = ($ReturnType*)extract_pixel_in_byte($Arg0VarName, $Arg1VarName);
    return *p;
  }];
OclBuiltinImpl intel_sub_group_block_read1_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_read1_4_img, [ v1u32 ], 0, BlockReadImg1_4>;
OclBuiltinImpl intel_sub_group_block_read1_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_read1_4_img_rw, [ v1u32 ], 0, BlockReadImg1_4>;
OclBuiltinImpl intel_sub_group_block_read_us1_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_read_us1_4_img, intel_sub_group_block_read_write_us_types, 0, BlockReadImg1_4>;
OclBuiltinImpl intel_sub_group_block_read_us1_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_read_us1_4_img_rw, intel_sub_group_block_read_write_us_types, 0, BlockReadImg1_4>;

code BlockWriteImg1_4 =
  [{
    $Arg2Type* p = ($Arg2Type*)extract_pixel_in_byte($Arg0VarName, $Arg1VarName);
    *p = $Arg2VarName;
  }];
OclBuiltinImpl intel_sub_group_block_write1_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_write1_4_img, [ v1u32 ], 0, BlockWriteImg1_4>;
OclBuiltinImpl intel_sub_group_block_write1_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_write1_4_img_rw, [ v1u32 ], 0, BlockWriteImg1_4>;
OclBuiltinImpl intel_sub_group_block_write_us1_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_write_us1_4_img, intel_sub_group_block_read_write_us_types, 0, BlockWriteImg1_4>;
OclBuiltinImpl intel_sub_group_block_write_us1_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_write_us1_4_img_rw, intel_sub_group_block_read_write_us_types, 0, BlockWriteImg1_4>;

code BlockReadImg2_4 =
  [{
     $ReturnType res;
     $ReturnBaseType#4* p1 = ($ReturnBaseType#4 *)extract_pixel_in_byte($Arg0VarName, $Arg1VarName);
     $Arg1VarName.hi += 1;
     $ReturnBaseType#4* p2 = ($ReturnBaseType#4 *)extract_pixel_in_byte($Arg0VarName, $Arg1VarName);
     res.lo = *p1;
     res.hi = *p2;
     return ($ReturnType)(__builtin_shufflevector(res, res, 0,  4,
                                                            1,  5,
                                                            2,  6,
                                                            3,  7));
  }];
OclBuiltinImpl intel_sub_group_block_read2_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_read2_4_img, [ v1u32 ], 0, BlockReadImg2_4>;
OclBuiltinImpl intel_sub_group_block_read2_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_read2_4_img_rw, [ v1u32 ], 0, BlockReadImg2_4>;
OclBuiltinImpl intel_sub_group_block_read_us2_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_read_us2_4_img, intel_sub_group_block_read_write_us_types, 0, BlockReadImg2_4>;
OclBuiltinImpl intel_sub_group_block_read_us2_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_read_us2_4_img_rw, intel_sub_group_block_read_write_us_types, 0, BlockReadImg2_4>;

code BlockWriteImg2_4 =
  [{
     $Arg2BaseType#4* p1 = ($Arg2BaseType#4 *)extract_pixel_in_byte($Arg0VarName, $Arg1VarName);
     $Arg1VarName.hi += 1;
     $Arg2BaseType#4* p2 = ($Arg2BaseType#4 *)extract_pixel_in_byte($Arg0VarName, $Arg1VarName);
     $Arg2Type d = __builtin_shufflevector($Arg2VarName, $Arg2VarName, 0, 2, 4, 6,
                                                                       1, 3, 5, 7);
     *p1 = d.lo;
     *p2 = d.hi;
  }];
OclBuiltinImpl intel_sub_group_block_write2_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_write2_4_img, [ v1u32 ], 0, BlockWriteImg2_4>;
OclBuiltinImpl intel_sub_group_block_write2_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_write2_4_img_rw, [ v1u32 ], 0, BlockWriteImg2_4>;
OclBuiltinImpl intel_sub_group_block_write_us2_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_write_us2_4_img, intel_sub_group_block_read_write_us_types, 0, BlockWriteImg2_4>;
OclBuiltinImpl intel_sub_group_block_write_us2_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_write_us2_4_img_rw, intel_sub_group_block_read_write_us_types, 0, BlockWriteImg2_4>;

code BlockReadImg4_4 =
  [{
     $ReturnType res;
     $ReturnBaseType *r = ($ReturnBaseType*)&res;
     $ReturnBaseType#4* p;
#pragma clang unroll(full)
     for (int i=0; i < 4; ++i) {
       p = ($ReturnBaseType#4 *)extract_pixel_in_byte($Arg0VarName, $Arg1VarName);
       *($ReturnBaseType#4*)&r[i * 4] = *p;
       $Arg1VarName.hi += 1;
     }
    return __builtin_shufflevector(res,res, 0, 4, 8, 12,
                                            1, 5, 9, 13,
                                            2, 6, 10, 14,
                                            3, 7, 11, 15);
  }];
OclBuiltinImpl intel_sub_group_block_read4_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_read4_4_img, [ v1u32 ], 0, BlockReadImg4_4>;
OclBuiltinImpl intel_sub_group_block_read4_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_read4_4_img_rw, [ v1u32 ], 0, BlockReadImg4_4>;
OclBuiltinImpl intel_sub_group_block_read_us4_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_read_us4_4_img, intel_sub_group_block_read_write_us_types, 0, BlockReadImg4_4>;
OclBuiltinImpl intel_sub_group_block_read_us4_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_read_us4_4_img_rw, intel_sub_group_block_read_write_us_types, 0, BlockReadImg4_4>;

code BlockWriteImg4_4 =
  [{
     $Arg2Type d = __builtin_shufflevector($Arg2VarName, $Arg2VarName, 0, 4, 8, 12,
                                                                       1, 5, 9, 13,
                                                                       2, 6, 10, 14,
                                                                       3, 7, 11, 15);
     $Arg2BaseType *r = ($Arg2BaseType*)&d;
     $Arg2BaseType#4* p;
#pragma clang unroll(full)
     for (int i=0; i < 4; ++i) {
       p = ($Arg2BaseType#4 *)extract_pixel_in_byte($Arg0VarName, $Arg1VarName);
       *p = *($Arg2BaseType#4 *)&r[i * 4];
       $Arg1VarName.hi += 1;
     }
  }];
OclBuiltinImpl intel_sub_group_block_write4_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_write4_4_img, [ v1u32 ], 0,BlockWriteImg4_4>;
OclBuiltinImpl intel_sub_group_block_write4_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_write4_4_img_rw, [ v1u32 ], 0,BlockWriteImg4_4>;
OclBuiltinImpl intel_sub_group_block_write_us4_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_write_us4_4_img, intel_sub_group_block_read_write_us_types, 0,BlockWriteImg4_4>;
OclBuiltinImpl intel_sub_group_block_write_us4_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_write_us4_4_img_rw, intel_sub_group_block_read_write_us_types, 0,BlockWriteImg4_4>;

code BlockReadImg8_4 =
  [{
     $ReturnType res;
     $ReturnBaseType *r = ($ReturnBaseType*)&res;
     $ReturnBaseType#4* p;
#pragma clang unroll(full)
     for (int i=0; i < 8; ++i) {
       p = ($ReturnBaseType#4 *)extract_pixel_in_byte($Arg0VarName, $Arg1VarName);
       *($ReturnBaseType#4*)&r[i * 4] = *p;
       $Arg1VarName.hi += 1;
     }
    return __ocl_shuffle_transpose_$ReturnBaseType#_4x8(res);
  }];
OclBuiltinImpl intel_sub_group_block_read8_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_read8_4_img, [ v1u32 ], 0, BlockReadImg8_4>;
OclBuiltinImpl intel_sub_group_block_read8_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_read8_4_img_rw, [ v1u32 ], 0, BlockReadImg8_4>;
OclBuiltinImpl intel_sub_group_block_read_us8_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_read_us8_4_img, intel_sub_group_block_read_write_us_types, 0, BlockReadImg8_4>;
OclBuiltinImpl intel_sub_group_block_read_us8_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_read_us8_4_img_rw, intel_sub_group_block_read_write_us_types, 0, BlockReadImg8_4>;

code BlockWriteImg8_4 =
  [{
     $Arg2Type d = __ocl_shuffle_transpose_$Arg2BaseType#_8x4($Arg2VarName);
     $Arg2BaseType *r = ($Arg2BaseType*)&d;
     $Arg2BaseType#4* p;
#pragma clang unroll(full)
     for (int i=0; i < 8; ++i) {
       p = ($Arg2BaseType#4 *)extract_pixel_in_byte($Arg0VarName, $Arg1VarName);
       *p = *($Arg2BaseType#4 *)&r[i * 4];
       $Arg1VarName.hi += 1;
     }
  }];
OclBuiltinImpl intel_sub_group_block_write8_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_write8_4_img, [ v1u32 ], 0, BlockWriteImg8_4>;
OclBuiltinImpl intel_sub_group_block_write8_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_write8_4_img_rw, [ v1u32 ], 0, BlockWriteImg8_4>;
OclBuiltinImpl intel_sub_group_block_write_us8_4_img_avx =  OclBuiltinImpl<intel_sub_group_block_write_us8_4_img, intel_sub_group_block_read_write_us_types, 0, BlockWriteImg8_4>;
OclBuiltinImpl intel_sub_group_block_write_us8_4_img_rw_avx =  OclBuiltinImpl<intel_sub_group_block_write_us8_4_img_rw, intel_sub_group_block_read_write_us_types, 0, BlockWriteImg8_4>;
