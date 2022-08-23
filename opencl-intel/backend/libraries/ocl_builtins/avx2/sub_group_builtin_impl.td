// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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
// reductions
//

// all|any
OclBuiltinImpl sub_group_all_avx256v8iu32 = OclBuiltinImpl<sub_group_all_vec, [v8u32, v8i32], 0,
  [{
    __m256i is_zero = _mm256_cmpeq_epi32((__m256i)$Arg0VarName, _mm256_set_epi32(0,0,0,0,0,0,0,0));
    return _mm256_testz_si256(is_zero, (__m256i)$Arg1VarName);
  }]>;

OclBuiltinImpl sub_group_any_avx256v8iu32 = OclBuiltinImpl<sub_group_any_vec, [v8u32, v8i32], 0,
  [{
    return _mm256_testz_si256((__m256i)$Arg0VarName, (__m256i)$Arg1VarName) != 1;
  }]>;

// reduce add
OclBuiltinImpl sub_group_reduce_add_avx256ui32 = OclBuiltinImpl<sub_group_reduce_add_vec, [v8i32, v8u32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 &= arg1;
    $Arg0BaseType#4 temp0 = as_$Arg0BaseType#4(_mm_add_epi32((__m128i)arg0.lo, (__m128i)arg0.hi));
    $Arg0BaseType#4 temp1 = as_$Arg0BaseType#4(_mm_hadd_epi32((__m128i)temp0, (__m128i)temp0));
    return temp1[0] + temp1[1];
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx256f32 = OclBuiltinImpl<sub_group_reduce_add_vec, [v8f32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 &= arg1;
    $Arg0BaseType#4 temp0 = as_$Arg0BaseType#4(_mm_add_ps((__m128)arg0.lo, (__m128)arg0.hi));
    $Arg0BaseType#4 temp1 = as_$Arg0BaseType#4(_mm_hadd_ps((__m128)temp0, (__m128)temp0));
    return temp1[0] + temp1[1];
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx256i8 = OclBuiltinImpl<sub_group_reduce_add_vec_cs, [v8i8], 0,
  [{
    __m256i temp = _mm256_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_reduce_add(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx256u8 = OclBuiltinImpl<sub_group_reduce_add_vec_cs, [v8u8], 0,
  [{
    __m256i temp = _mm256_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_reduce_add(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx256i16 = OclBuiltinImpl<sub_group_reduce_add_vec_cs, [v8i16], 0,
  [{
    __m256i temp = _mm256_cvtepi16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_reduce_add(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx256u16 = OclBuiltinImpl<sub_group_reduce_add_vec_cs, [v8u16], 0,
  [{
    __m256i temp = _mm256_cvtepu16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_reduce_add(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx256ui64 = OclBuiltinImpl<sub_group_reduce_add_vec, [v8i64, v8u64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    arg0 &= arg1;
    $Arg0BaseType#4 temp0 = as_$Arg0BaseType#4(_mm256_add_epi64((__m256i)arg0.lo, (__m256i)arg0.hi));
    $Arg0BaseType#2 temp1 = as_$Arg0BaseType#2(_mm_add_epi64((__m128i)temp0.lo, (__m128i)temp0.hi));
    return temp1[0] + temp1[1];
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx256f64 = OclBuiltinImpl<sub_group_reduce_add_vec, [v8f64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    arg0 &= arg1;
    $Arg0BaseType#4 temp0 = as_$Arg0BaseType#4(_mm256_add_pd((__m256d)arg0.lo, (__m256d)arg0.hi));
    $Arg0BaseType#2 temp1 = as_$Arg0BaseType#2(_mm_add_pd((__m128d)temp0.lo, (__m128d)temp0.hi));
    return temp1[0] + temp1[1];
  }]>;


// reduce min
OclBuiltinImpl sub_group_reduce_min_avx256u32 = OclBuiltinImpl<sub_group_reduce_min_vec, [v8u32], 0,
  [{
     int8 arg0 = as_int8($Arg0VarName);
     int8 arg1 = as_int8($Arg1VarName);
     arg0 |= ~arg1;
     int4 temp0 = as_int4(_mm_min_epu32((__m128i)arg0.lo, (__m128i)arg0.hi));
     int4 temp1 = as_int4(_mm_min_epu32((__m128i)temp0, (__m128i)__builtin_shufflevector(temp0, temp0, 2, 3, 0, 1)));
     $Arg0BaseType#2 temp2 = as_$Arg0BaseType#2(temp1.lo);
     return min(temp2.lo, temp2.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx256i32 = OclBuiltinImpl<sub_group_reduce_min_vec, [v8i32], 0,
  [{
     int8 arg0 = as_int8($Arg0VarName);
     int8 arg1 = as_int8($Arg1VarName);
     arg0 &= arg1;
     arg1 = (~arg1) & INT_MAX;
     arg0 |= arg1;
     int4 temp0 = as_int4(_mm_min_epi32((__m128i)arg0.lo, (__m128i)arg0.hi));
     int4 temp1 = as_int4(_mm_min_epi32((__m128i)temp0, (__m128i)__builtin_shufflevector(temp0, temp0, 2, 3, 0, 1)));
     $Arg0BaseType#2 temp2 = as_$Arg0BaseType#2(temp1.lo);
     return min(temp2.lo, temp2.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx256f32 = OclBuiltinImpl<sub_group_reduce_min_vec, [v8f32], 0,
  [{
     int8 arg0 = as_int8($Arg0VarName);
     int8 arg1 = as_int8($Arg1VarName);
     arg0 &= arg1;
     arg1 = (~arg1) & as_int(INFINITY);
     arg0 |= arg1;
     int4 temp0 = as_int4(_mm_min_ps((__m128)arg0.lo, (__m128)arg0.hi));
     int4 temp1 = as_int4(_mm_min_ps((__m128)temp0, (__m128)__builtin_shufflevector(temp0, temp0, 2, 3, 0, 1)));
     $Arg0BaseType#2 temp2 = as_$Arg0BaseType#2(temp1.lo);
     return min(temp2.lo, temp2.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx256u64 = OclBuiltinImpl<sub_group_reduce_min_vec, [v8u64], 0,
  [{
     long8 arg0 = as_long8($Arg0VarName);
     long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
     arg0 |= ~arg1;
     long4 temp0;
     long4 mask4 = __builtin_convertvector(as_$Arg0BaseType#4(arg0.lo) > as_$Arg0BaseType#4(arg0.hi), long4);
     temp0 = arg0.lo & ~mask4;
     temp0 |= arg0.hi & mask4;
     long2 temp1;
     long2 mask2 = __builtin_convertvector(as_$Arg0BaseType#2(temp0.lo) > as_$Arg0BaseType#2(temp0.hi), long2);
     temp1 = temp0.lo & ~mask2;
     temp1 |= temp0.hi & mask2;
     $Arg0BaseType#2 temp2 = as_$Arg0BaseType#2(temp1);
     return min(temp2.lo, temp2.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx256i64 = OclBuiltinImpl<sub_group_reduce_min_vec, [v8i64], 0,
  [{
     long8 arg0 = as_long8($Arg0VarName);
     long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
     arg0 &= arg1;
     arg1 = (~arg1) & LONG_MAX;
     arg0 |= arg1;
     long4 temp0;
     long4 mask4 = as_long4( _mm256_cmpgt_epi64((__m256i)$Arg0VarName.lo, (__m256i)$Arg0VarName.hi));
     temp0 = arg0.lo & ~mask4;
     temp0 |= arg0.hi & mask4;
     long2 temp1;
     long2 mask2 = as_long2( _mm_cmpgt_epi64((__m128i)temp0.lo, (__m128i)temp0.hi));
     temp1 = temp0.lo & ~mask2;
     temp1 |= temp0.hi & mask2;
     $Arg0BaseType#2 temp2 = as_$Arg0BaseType#2(temp1);
     return min(temp2.lo, temp2.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx256f64 = OclBuiltinImpl<sub_group_reduce_min_vec, [v8f64], 0,
  [{
     long8 arg0 = as_long8($Arg0VarName);
     long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
     arg0 &= arg1;
     arg1 = (~arg1) & as_long((double)INFINITY);
     arg0 |= arg1;
     long4 temp0 = as_long4(_mm256_min_pd((__m256d)arg0.lo, (__m256d)arg0.hi));
     long2 temp1;
     long2 mask2 = as_long2(_mm_cmpgt_pd((__m128d)temp0.lo, (__m128d)temp0.hi));
     temp1 = temp0.lo & ~mask2;
     temp1 |= temp0.hi & mask2;
     $Arg0BaseType#2 temp2 = as_$Arg0BaseType#2(temp1);
     return min(temp2.lo, temp2.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx256i8 = OclBuiltinImpl<sub_group_reduce_min_vec_cs, [v8i8], 0,
  [{
    __m256i temp = _mm256_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_reduce_min(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx256u8 = OclBuiltinImpl<sub_group_reduce_min_vec_cs, [v8u8], 0,
  [{
    __m256i temp = _mm256_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_reduce_min(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx256i16 = OclBuiltinImpl<sub_group_reduce_min_vec_cs, [v8i16], 0,
  [{
    __m256i temp = _mm256_cvtepi16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_reduce_min(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx256u16 = OclBuiltinImpl<sub_group_reduce_min_vec_cs, [v8u16], 0,
  [{
    __m256i temp = _mm256_cvtepu16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_reduce_min(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

// reduce max
OclBuiltinImpl sub_group_reduce_max_avx256u32 = OclBuiltinImpl<sub_group_reduce_max_vec, [v8u32], 0,
  [{
     int8 arg0 = as_int8($Arg0VarName);
     int8 arg1 = as_int8($Arg1VarName);
     arg0 &= arg1;
     int4 temp0 = as_int4(_mm_max_epu32((__m128i)arg0.lo, (__m128i)arg0.hi));
     int4 temp1 = as_int4(_mm_max_epu32((__m128i)temp0, (__m128i)__builtin_shufflevector(temp0, temp0, 2, 3, 0, 1)));
     $Arg0BaseType#2 temp2 = as_$Arg0BaseType#2(temp1.lo);
     return max(temp2.lo, temp2.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx256i32 = OclBuiltinImpl<sub_group_reduce_max_vec, [v8i32], 0,
  [{
     int8 arg0 = as_int8($Arg0VarName);
     int8 arg1 = as_int8($Arg1VarName);
     arg0 &= arg1;
     arg1 = (~arg1) & INT_MIN;
     arg0 |= arg1;
     int4 temp0 = as_int4(_mm_max_epi32((__m128i)arg0.lo, (__m128i)arg0.hi));
     int4 temp1 = as_int4(_mm_max_epi32((__m128i)temp0, (__m128i)__builtin_shufflevector(temp0, temp0, 2, 3, 0, 1)));
     $Arg0BaseType#2 temp2 = as_$Arg0BaseType#2(temp1.lo);
     return max(temp2.lo, temp2.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx256f32 = OclBuiltinImpl<sub_group_reduce_max_vec, [v8f32], 0,
  [{
     int8 arg0 = as_int8($Arg0VarName);
     int8 arg1 = as_int8($Arg1VarName);
     arg0 &= arg1;
     arg1 = (~arg1) & as_int(-INFINITY);
     arg0 |= arg1;
     int4 temp0 = as_int4(_mm_max_ps((__m128)arg0.lo, (__m128)arg0.hi));
     int4 temp1 = as_int4(_mm_max_ps((__m128)temp0, (__m128)__builtin_shufflevector(temp0, temp0, 2, 3, 0, 1)));
     $Arg0BaseType#2 temp2 = as_$Arg0BaseType#2(temp1.lo);
     return max(temp2.lo, temp2.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx256u64 = OclBuiltinImpl<sub_group_reduce_max_vec, [v8u64], 0,
  [{
     long8 arg0 = as_long8($Arg0VarName);
     long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
     arg0 &= arg1;
     long4 temp0;
     long4 mask4 = __builtin_convertvector(as_$Arg0BaseType#4(arg0.lo) > as_$Arg0BaseType#4(arg0.hi), long4);
     temp0 = arg0.lo & mask4;
     temp0 |= arg0.hi & ~mask4;
     long2 temp1;
     long2 mask2 = __builtin_convertvector(as_$Arg0BaseType#2(temp0.lo) > as_$Arg0BaseType#2(temp0.hi), long2);
     temp1 = temp0.lo & mask2;
     temp1 |= temp0.hi & ~mask2;
     $Arg0BaseType#2 temp2 = as_$Arg0BaseType#2(temp1);
     return max(temp2.lo, temp2.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx256i64 = OclBuiltinImpl<sub_group_reduce_max_vec, [v8i64], 0,
  [{
     long8 arg0 = as_long8($Arg0VarName);
     long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
     arg0 &= arg1;
     arg1 = (~arg1) & LONG_MIN;
     arg0 |= arg1;
     long4 temp0;
     long4 mask4 = as_long4( _mm256_cmpgt_epi64((__m256i)arg0.lo, (__m256i)arg0.hi));
     temp0 = arg0.lo & mask4;
     temp0 |= arg0.hi & ~mask4;
     long2 temp1;
     long2 mask2 = as_long2( _mm_cmpgt_epi64((__m128i)temp0.lo, (__m128i)temp0.hi));
     temp1 = temp0.lo & mask2;
     temp1 |= temp0.hi & ~mask2;
     $Arg0BaseType#2 temp2 = as_$Arg0BaseType#2(temp1);
     return max(temp2.lo, temp2.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx256f64 = OclBuiltinImpl<sub_group_reduce_max_vec, [v8f64], 0,
  [{
     long8 arg0 = as_long8($Arg0VarName);
     long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
     arg0 &= arg1;
     arg1 = (~arg1) & as_long((double)-INFINITY);
     arg0 |= arg1;
     long4 temp0 = as_long4(_mm256_max_pd((__m256d)arg0.lo, (__m256d)arg0.hi));
     long2 temp1;
     long2 mask2 = as_long2(_mm_cmpgt_pd((__m128d)temp0.lo, (__m128d)temp0.hi));
     temp1 = temp0.lo & mask2;
     temp1 |= temp0.hi & ~mask2;
     $Arg0BaseType#2 temp2 = as_$Arg0BaseType#2(temp1);
     return max(temp2.lo, temp2.hi);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx256i8 = OclBuiltinImpl<sub_group_reduce_max_vec_cs, [v8i8], 0,
  [{
    __m256i temp = _mm256_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_reduce_max(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx256u8 = OclBuiltinImpl<sub_group_reduce_max_vec_cs, [v8u8], 0,
  [{
    __m256i temp = _mm256_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_reduce_max(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx256i16 = OclBuiltinImpl<sub_group_reduce_max_vec_cs, [v8i16], 0,
  [{
    __m256i temp = _mm256_cvtepi16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_reduce_max(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx256u16 = OclBuiltinImpl<sub_group_reduce_max_vec_cs, [v8u16], 0,
  [{
    __m256i temp = _mm256_cvtepu16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_reduce_max(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

// inclusive scan
// add
OclBuiltinImpl sub_group_scan_inclusive_add_avx256iu32 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [v8i32, v8u32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp[7] = 0;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_add_epi32((__m256i)$Arg0VarName, (__m256i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx256f32 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [v8f32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp[7] = 0;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_add_ps((__m256)$Arg0VarName, (__m256)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx256iu64 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [v8i64, v8u64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp[7] = 0;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName.lo = as_$Arg0BaseType#4(_mm256_add_epi64((__m256i)$Arg0VarName.lo, (__m256i)temp.lo));
      $Arg0VarName.hi = as_$Arg0BaseType#4(_mm256_add_epi64((__m256i)$Arg0VarName.hi, (__m256i)temp.hi));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx256f64 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [v8f64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp[7] = 0;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName.lo = as_$Arg0BaseType#4(_mm256_add_pd((__m256d)$Arg0VarName.lo, (__m256d)temp.lo));
      $Arg0VarName.hi = as_$Arg0BaseType#4(_mm256_add_pd((__m256d)$Arg0VarName.hi, (__m256d)temp.hi));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx256i8 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec_cs, [v8i8], 0,
  [{
    __m256i temp = _mm256_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_scan_inclusive_add(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx256u8 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec_cs, [v8u8], 0,
  [{
    __m256i temp = _mm256_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_scan_inclusive_add(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx256i16 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec_cs, [v8i16], 0,
  [{
    __m256i temp = _mm256_cvtepi16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_scan_inclusive_add(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx256u16 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec_cs, [v8u16], 0,
  [{
    __m256i temp = _mm256_cvtepu16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_scan_inclusive_add(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

// min
OclBuiltinImpl sub_group_scan_inclusive_min_avx256i32 =  OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [v8i32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & INT_MAX;
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp[7] = INT_MAX;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_min_epi32((__m256i)$Arg0VarName, (__m256i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx256u32 =  OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [v8u32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 |= ~arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp[7] = UINT_MAX;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_min_epu32((__m256i)$Arg0VarName, (__m256i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx256f32 =  OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [v8f32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & as_int(INFINITY);
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp[7] = INFINITY;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_min_ps((__m256)$Arg0VarName, (__m256)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx256i64 =  OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [v8i64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    arg0 &= arg1;
    arg1 = (~arg1) & LONG_MAX;
    arg0 |= arg1;
    long8 temp0 = arg0;

#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp0[7] = LONG_MAX;
      temp0 = __builtin_shufflevector(temp0, temp0, 7, 0, 1, 2, 3, 4, 5, 6);
      long4 mask4 = as_long4( _mm256_cmpgt_epi64((__m256i)arg0.lo, (__m256i)temp0.lo));
      arg0.lo &= ~mask4;
      arg0.lo |= temp0.lo & mask4;
      mask4 = as_long4( _mm256_cmpgt_epi64((__m256i)arg0.hi, (__m256i)temp0.hi));
      arg0.hi &= ~mask4;
      arg0.hi |= temp0.hi & mask4;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx256u64 =  OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [v8u64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    arg0 |= ~arg1;
    long8 temp0 = arg0;

#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp0[7] = as_long(ULONG_MAX);
      temp0 = __builtin_shufflevector(temp0, temp0, 7, 0, 1, 2, 3, 4, 5, 6);
      long8 mask8 = __builtin_convertvector(as_$Arg0Type(arg0) > as_$Arg0Type(temp0), long8);
      arg0 &= ~mask8;
      arg0 |= temp0 & mask8;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx256f64 =  OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [v8f64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    long INF = as_long((double)INFINITY);
    arg0 &= arg1;
    arg1 = (~arg1) & INF;
    arg0 |= arg1;
    long8 temp0 = arg0;

#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp0[7] = INF;
      temp0 = __builtin_shufflevector(temp0, temp0, 7, 0, 1, 2, 3, 4, 5, 6);
      arg0.lo = as_long4( _mm256_min_pd((__m256d)arg0.lo, (__m256d)temp0.lo));
      arg0.hi = as_long4( _mm256_min_pd((__m256d)arg0.hi, (__m256d)temp0.hi));
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx256i8 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec_cs, [v8i8], 0,
  [{
    __m256i temp = _mm256_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_scan_inclusive_min(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx256u8 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec_cs, [v8u8], 0,
  [{
    __m256i temp = _mm256_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_scan_inclusive_min(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx256i16 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec_cs, [v8i16], 0,
  [{
    __m256i temp = _mm256_cvtepi16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_scan_inclusive_min(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx256u16 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec_cs, [v8u16], 0,
  [{
    __m256i temp = _mm256_cvtepu16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_scan_inclusive_min(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

// max
OclBuiltinImpl sub_group_scan_inclusive_max_avx256i32 =  OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [v8i32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & INT_MIN;
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp[7] = INT_MIN;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_max_epi32((__m256i)$Arg0VarName, (__m256i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx256u32 =  OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [v8u32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp[7] = 0;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_max_epu32((__m256i)$Arg0VarName, (__m256i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx256f32 =  OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [v8f32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & as_int(-INFINITY);
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp[7] = -INFINITY;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_max_ps((__m256)$Arg0VarName, (__m256)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx256i64 =  OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [v8i64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    arg0 &= arg1;
    arg1 = (~arg1) & LONG_MIN;
    arg0 |= arg1;
    long8 temp0 = arg0;

#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp0[7] = LONG_MIN;
      temp0 = __builtin_shufflevector(temp0, temp0, 7, 0, 1, 2, 3, 4, 5, 6);
      long4 mask4 = as_long4( _mm256_cmpgt_epi64((__m256i)arg0.lo, (__m256i)temp0.lo));
      arg0.lo &= mask4;
      arg0.lo |= temp0.lo & ~mask4;
      mask4 = as_long4( _mm256_cmpgt_epi64((__m256i)arg0.hi, (__m256i)temp0.hi));
      arg0.hi &= mask4;
      arg0.hi |= temp0.hi & ~mask4;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx256u64 =  OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [v8u64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    arg0 &= arg1;
    long8 temp0 = arg0;

#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp0[7] = 0;
      temp0 = __builtin_shufflevector(temp0, temp0, 7, 0, 1, 2, 3, 4, 5, 6);
      long8 mask8 = __builtin_convertvector(as_$Arg0Type(arg0) > as_$Arg0Type(temp0), long8);
      arg0 &= mask8;
      arg0 |= temp0 & ~mask8;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx256f64 =  OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [v8f64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    long INF = as_long((double)-INFINITY);
    arg0 &= arg1;
    arg1 = (~arg1) & INF;
    arg0 |= arg1;
    long8 temp0 = arg0;
#pragma clang loop unroll(full)
    for (int i = 0; i < 7; i++) {
      temp0[7] = INF ;
      temp0 = __builtin_shufflevector(temp0, temp0, 7, 0, 1, 2, 3, 4, 5, 6);
      arg0.lo = as_long4( _mm256_max_pd((__m256d)arg0.lo, (__m256d)temp0.lo));
      arg0.hi = as_long4( _mm256_max_pd((__m256d)arg0.hi, (__m256d)temp0.hi));
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx256i8 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec_cs, [v8i8], 0,
  [{
    __m256i temp = _mm256_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_scan_inclusive_max(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx256u8 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec_cs, [v8u8], 0,
  [{
    __m256i temp = _mm256_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_scan_inclusive_max(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx256i16 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec_cs, [v8i16], 0,
  [{
    __m256i temp = _mm256_cvtepi16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_scan_inclusive_max(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx256u16 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec_cs, [v8u16], 0,
  [{
    __m256i temp = _mm256_cvtepu16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_scan_inclusive_max(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

// exclusive scan
// add
OclBuiltinImpl sub_group_scan_exclusive_add_avx256iu32 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [v8i32, v8u32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName[0] = 0;
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = 0;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_add_epi32((__m256i)$Arg0VarName, (__m256i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx256f32 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [v8f32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName[0] = 0;
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = 0;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_add_ps((__m256)$Arg0VarName, (__m256)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx256iu64 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [v8i64, v8u64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName[0] = 0;
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = 0;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName.lo = as_$Arg0BaseType#4(_mm256_add_epi64((__m256i)$Arg0VarName.lo, (__m256i)temp.lo));
      $Arg0VarName.hi = as_$Arg0BaseType#4(_mm256_add_epi64((__m256i)$Arg0VarName.hi, (__m256i)temp.hi));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx256f64 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [v8f64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    arg0 &= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName[0] = 0;
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = 0;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName.lo = as_$Arg0BaseType#4(_mm256_add_pd((__m256d)$Arg0VarName.lo, (__m256d)temp.lo));
      $Arg0VarName.hi = as_$Arg0BaseType#4(_mm256_add_pd((__m256d)$Arg0VarName.hi, (__m256d)temp.hi));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx256i8 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec_cs, [v8i8], 0,
  [{
    __m256i temp = _mm256_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_scan_exclusive_add(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx256u8 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec_cs, [v8u8], 0,
  [{
    __m256i temp = _mm256_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_scan_exclusive_add(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx256i16 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec_cs, [v8i16], 0,
  [{
    __m256i temp = _mm256_cvtepi16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_scan_exclusive_add(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx256u16 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec_cs, [v8u16], 0,
  [{
    __m256i temp = _mm256_cvtepu16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_scan_exclusive_add(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

//min
OclBuiltinImpl sub_group_scan_exclusive_min_avx256i32 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [v8i32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & INT_MAX;
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName[0] = INT_MAX;
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = INT_MAX;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_min_epi32((__m256i)$Arg0VarName, (__m256i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx256u32 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [v8u32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 |= ~arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName[0] = UINT_MAX;
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = UINT_MAX;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_min_epu32((__m256i)$Arg0VarName, (__m256i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx256f32 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [v8f32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & as_int(INFINITY);
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName[0] = INFINITY;
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = INFINITY;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_min_ps((__m256)$Arg0VarName, (__m256)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx256i64 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [v8i64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    arg0 &= arg1;
    arg1 = (~arg1) & LONG_MAX;
    arg0 |= arg1;

    arg0 = __builtin_shufflevector(arg0, arg0, 7, 0, 1, 2, 3, 4, 5, 6);
    arg0[0] = LONG_MAX;
    long8 temp = arg0;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = LONG_MAX;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      long4 mask4 = as_long4( _mm256_cmpgt_epi64((__m256i)arg0.lo, (__m256i)temp.lo));
      arg0.lo &= ~mask4;
      arg0.lo |= temp.lo & mask4;
      mask4 = as_long4( _mm256_cmpgt_epi64((__m256i)arg0.hi, (__m256i)temp.hi));
      arg0.hi &= ~mask4;
      arg0.hi |= temp.hi & mask4;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx256u64 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [v8u64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    arg0 |= ~arg1;

    arg0 = __builtin_shufflevector(arg0, arg0, 7, 0, 1, 2, 3, 4, 5, 6);
    arg0[0] = as_long(ULONG_MAX);
    long8 temp = arg0;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = as_long(ULONG_MAX);
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      long8 mask8 = __builtin_convertvector(as_$Arg0Type(arg0) > as_$Arg0Type(temp), long8);
      arg0 &= ~mask8;
      arg0 |= temp & mask8;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx256f64 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [v8f64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);

    long INF = as_long((double)INFINITY);
    arg0 &= arg1;
    arg1 = (~arg1) & INF;
    arg0 |= arg1;

    arg0 = __builtin_shufflevector(arg0, arg0, 7, 0, 1, 2, 3, 4, 5, 6);
    arg0[0] = INF;
    long8 temp = arg0;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = INF;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      arg0.lo = as_long4(_mm256_min_pd((__m256d)arg0.lo, (__m256d)temp.lo));
      arg0.hi = as_long4(_mm256_min_pd((__m256d)arg0.hi, (__m256d)temp.hi));
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx256i8 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec_cs, [v8i8], 0,
  [{
    __m256i temp = _mm256_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_scan_exclusive_min(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type_sat_rtp(temp1);
  }]>;  // round to positive infinity during conversion

OclBuiltinImpl sub_group_scan_exclusive_min_avx256u8 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec_cs, [v8u8], 0,
  [{
    __m256i temp = _mm256_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_scan_exclusive_min(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type_sat_rtp(temp1);
  }]>;  // round to positive infinity during conversion

OclBuiltinImpl sub_group_scan_exclusive_min_avx256i16 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec_cs, [v8i16], 0,
  [{
    __m256i temp = _mm256_cvtepi16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_scan_exclusive_min(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type_sat_rtp(temp1);
  }]>;  // round to positive infinity during conversion

OclBuiltinImpl sub_group_scan_exclusive_min_avx256u16 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec_cs, [v8u16], 0,
  [{
    __m256i temp = _mm256_cvtepu16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_scan_exclusive_min(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type_sat_rtp(temp1);
  }]>;  // round to positive infinity during conversion

//max
OclBuiltinImpl sub_group_scan_exclusive_max_avx256i32 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [v8i32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & INT_MIN;
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName[0] = INT_MIN;
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = INT_MIN;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_max_epi32((__m256i)$Arg0VarName, (__m256i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx256u32 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [v8u32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 |= ~arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName[0] = 0;
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = 0;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_max_epu32((__m256i)$Arg0VarName, (__m256i)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx256f32 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [v8f32], 0,
  [{
    int8 arg0 = as_int8($Arg0VarName);
    int8 arg1 = as_int8($Arg1VarName);
    arg0 &= arg1;
    arg1 = (~arg1) & as_int(-INFINITY);
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName[0] = -INFINITY;
    $Arg0Type temp = $Arg0VarName;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = -INFINITY;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      $Arg0VarName = as_$Arg0Type(_mm256_max_ps((__m256)$Arg0VarName, (__m256)temp));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx256i64 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [v8i64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    arg0 &= arg1;
    arg1 = (~arg1) & LONG_MIN;
    arg0 |= arg1;

    arg0 = __builtin_shufflevector(arg0, arg0, 7, 0, 1, 2, 3, 4, 5, 6);
    arg0[0] = LONG_MIN;
    long8 temp = arg0;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = LONG_MIN;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      long4 mask4 = as_long4( _mm256_cmpgt_epi64((__m256i)arg0.lo, (__m256i)temp.lo));
      arg0.lo &= mask4;
      arg0.lo |= temp.lo & ~mask4;
      mask4 = as_long4( _mm256_cmpgt_epi64((__m256i)arg0.hi, (__m256i)temp.hi));
      arg0.hi &= mask4;
      arg0.hi |= temp.hi & ~mask4;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx256u64 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [v8u64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);
    arg0 &= arg1;

    arg0 = __builtin_shufflevector(arg0, arg0, 7, 0, 1, 2, 3, 4, 5, 6);
    arg0[0] = 0;
    long8 temp = arg0;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = 0;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      long8 mask8 = __builtin_convertvector(as_$Arg0Type(arg0) > as_$Arg0Type(temp), long8);
      arg0 &= mask8;
      arg0 |= temp & ~mask8;
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx256f64 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [v8f64], 0,
  [{
    long8 arg0 = as_long8($Arg0VarName);
    long8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), long8);

    long INF = as_long((double)-INFINITY);
    arg0 &= arg1;
    arg1 = (~arg1) & INF;
    arg0 |= arg1;

    arg0 = __builtin_shufflevector(arg0, arg0, 7, 0, 1, 2, 3, 4, 5, 6);
    arg0[0] = INF;
    long8 temp = arg0;

#pragma clang loop unroll(full)
    for (int i = 0; i < 6; i++) {
      temp[7] = INF;
      temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
      arg0.lo = as_long4(_mm256_max_pd((__m256d)arg0.lo, (__m256d)temp.lo));
      arg0.hi = as_long4(_mm256_max_pd((__m256d)arg0.hi, (__m256d)temp.hi));
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx256i8 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec_cs, [v8i8], 0,
  [{
    __m256i temp = _mm256_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_scan_exclusive_max(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type_sat_rtn(temp1);
  }]>;  // round to negative infinity during conversion

OclBuiltinImpl sub_group_scan_exclusive_max_avx256u8 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec_cs, [v8u8], 0,
  [{
    __m256i temp = _mm256_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName), __m128i));
    int8 temp1 = sub_group_scan_exclusive_max(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type_sat_rtz(temp1);
  }]>;  // round to zero during conversion

OclBuiltinImpl sub_group_scan_exclusive_max_avx256i16 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec_cs, [v8i16], 0,
  [{
    __m256i temp = _mm256_cvtepi16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_scan_exclusive_max(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type_sat_rtn(temp1);
  }]>;  // round to negative infinity during conversion

OclBuiltinImpl sub_group_scan_exclusive_max_avx256u16 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec_cs, [v8u16], 0,
  [{
    __m256i temp = _mm256_cvtepu16_epi32(__builtin_astype($Arg0VarName, __m128i));
    int8 temp1 = sub_group_scan_exclusive_max(__builtin_astype(temp, int8), $Arg1VarName);
    return convert_$Arg0Type_sat_rtz(temp1);
  }]>;  // round to zero during conversion

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
OclBuiltinImpl sub_group_shuffle_avx256v8f32 = OclBuiltinImpl<sub_group_shuffle_avx2, [v8f32], 0,
  [{
    $Arg1VarName = $Arg1VarName & $Arg2VarName;
    return as_$ReturnType(_mm256_permutevar8x32_ps(__builtin_astype($Arg0VarName, __m256),
                                                   __builtin_astype($Arg1VarName, __m256i)));
  }]>;

// long and ulong
OclBuiltinImpl sub_group_shuffle_avx2v8ui64 = OclBuiltinImpl<sub_group_shuffle_avx2, [v8i64, v8u64, v8f64], 0,
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

// char, uchar, short, ushort
OclBuiltinImpl sub_group_shuffle_avx256v8ui8ui16 = OclBuiltinImpl<sub_group_shuffle_avx2, [v8i8, v8u8, v8i16, v8u16], 0,
  [{
    int8 temp = __builtin_convertvector($Arg0VarName, int8);
    temp = intel_sub_group_shuffle(temp, $Arg1VarName, $Arg2VarName);
    $Arg0VarName = __builtin_convertvector(temp, $Arg0Type);
    return $Arg0VarName;
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

// char2 and uchar2
OclBuiltinImpl sub_group_shuffle_avx256v16ui8 = OclBuiltinImpl<sub_group_shuffle_avx2, [v16i8, v16u8], 0,
  [{
    int16 temp = __builtin_convertvector($Arg0VarName, int16);
    temp = intel_sub_group_shuffle(temp, $Arg1VarName, $Arg2VarName);
    $Arg0VarName = __builtin_convertvector(temp, $Arg0Type);
    return $Arg0VarName;
  }]>;

// short2, ushort2, char4, uchar4
OclBuiltinImpl sub_group_shuffle_avx256v16iu16v32iu8 = OclBuiltinImpl<sub_group_shuffle_avx2, [v16i16, v16u16, v32i8, v32u8], 0,
  [{
    int8 temp = __builtin_astype($Arg0VarName, int8);
    temp = intel_sub_group_shuffle(temp, $Arg1VarName, $Arg2VarName);
    return __builtin_astype(temp, $ReturnType);
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

// short4, ushort4, char8, uchar8
OclBuiltinImpl sub_group_shuffle_avx256v32iu16v64iu8 = OclBuiltinImpl<sub_group_shuffle_avx2, [v32i16, v32u16, v64i8, v64u8], 0,
  [{
    int16 temp = __builtin_astype($Arg0VarName, int16);
    temp = intel_sub_group_shuffle(temp, $Arg1VarName, $Arg2VarName);
    return __builtin_astype(temp, $ReturnType);
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

// short8, ushort8, char16, uchar16
OclBuiltinImpl sub_group_shuffle_avx256v64iu16v128iu8 = OclBuiltinImpl<sub_group_shuffle_avx2, [v64i16, v64u16, v128i8, v128u8], 0,
  [{
    int32 temp = __builtin_astype($Arg0VarName, int32);
    temp = intel_sub_group_shuffle(temp, $Arg1VarName, $Arg2VarName);
    return __builtin_astype(temp, $ReturnType);
  }]>;

OclBuiltinImpl sub_group_shuffle_avx2_v128 = OclBuiltinImpl<sub_group_shuffle_avx2, [v128f32, v128i32, v128u32], 0,
[{
    $Arg1VarName = $Arg1VarName & $Arg2VarName;
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x8($Arg0VarName);

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

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x16($Arg0VarName);
    return $Arg0VarName;
}]>;

// short16, ushort16
OclBuiltinImpl sub_group_shuffle_avx256v128iu16 = OclBuiltinImpl<sub_group_shuffle_avx2, [v128i16, v128u16], 0,
  [{
    int64 temp = __builtin_astype($Arg0VarName, int64);
    temp = intel_sub_group_shuffle(temp, $Arg1VarName, $Arg2VarName);
    return __builtin_astype(temp, $ReturnType);
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

OclBuiltinImpl sub_group_shuffle_down_avx2_iu8 = OclBuiltinImpl<sub_group_shuffle_down_avx2,
        [v8i8, v8u8, v16i8, v16u8, v32i8, v32u8, v64i8, v64u8, v128i8, v128u8], 0,
  [{
      $Arg0Type res_cur, res_next;
      uint8 sg_indexes = { 0, 1, 2, 3, 4, 5, 6, 7 };
      const uint max_sg_size = 8;
      char$VecLength temp;
      sg_indexes = sg_indexes + $Arg2VarName;

      sg_indexes &= $Arg3VarName;

      int8 indexes_mask = sg_indexes < max_sg_size;
      int8 inv_indexes_mask = !indexes_mask;

      char$VecLength ext_mask =  __builtin_convertvector(__ocl_extend_mask_to_$VecLength(indexes_mask), char$VecLength);

      res_cur = intel_sub_group_shuffle($Arg0VarName, sg_indexes, *(uint8*)&indexes_mask);
      temp = *((char$VecLength*)&res_cur) & ext_mask;
      res_cur = *($Arg0Type*)&temp;

      res_next = intel_sub_group_shuffle($Arg1VarName, (sg_indexes - max_sg_size), *(uint8*)&inv_indexes_mask);
      temp = *((char$VecLength*)&res_next) & !ext_mask;
      res_next = *($Arg0Type*)&temp;

      return  res_cur | res_next;
  }]>;

OclBuiltinImpl sub_group_shuffle_down_avx2_iu16 = OclBuiltinImpl<sub_group_shuffle_down_avx2,
        [v8i16, v8u16, v16i16, v16u16, v32i16, v32u16, v64i16, v64u16, v128i16, v128u16], 0,
  [{
      $Arg0Type res_cur, res_next;
      uint8 sg_indexes = { 0, 1, 2, 3, 4, 5, 6, 7 };
      const uint max_sg_size = 8;
      short$VecLength temp;
      sg_indexes = sg_indexes + $Arg2VarName;

      sg_indexes &= $Arg3VarName;

      int8 indexes_mask = sg_indexes < max_sg_size;
      int8 inv_indexes_mask = !indexes_mask;

      short$VecLength ext_mask =  __builtin_convertvector(__ocl_extend_mask_to_$VecLength(indexes_mask), short$VecLength);

      res_cur = intel_sub_group_shuffle($Arg0VarName, sg_indexes, *(uint8*)&indexes_mask);
      temp = *((short$VecLength*)&res_cur) & ext_mask;
      res_cur = *($Arg0Type*)&temp;

      res_next = intel_sub_group_shuffle($Arg1VarName, (sg_indexes - max_sg_size), *(uint8*)&inv_indexes_mask);
      temp = *((short$VecLength*)&res_next) & !ext_mask;
      res_next = *($Arg0Type*)&temp;

      return  res_cur | res_next;
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

// char, uchar
OclBuiltinImpl sub_group_shuffle_up_avx2_iu8 = OclBuiltinImpl<sub_group_shuffle_up_avx2,
        [v8i8, v8u8, v16i8, v16u8, v32i8, v32u8, v64i8, v64u8, v128i8, v128u8], 0,
  [{
      $Arg0Type res_cur, res_next;
      char$VecLength temp;
      int8 sg_indexes = { 0, 1, 2, 3, 4, 5, 6, 7 };
      const int max_sg_size = 8;

      sg_indexes = sg_indexes - *(int8*)&$Arg2VarName;

      sg_indexes &= *(int8*)&$Arg3VarName;

      int8 indexes_mask = sg_indexes >= 0;
      int8 inv_indexes_mask = !indexes_mask;

      char$VecLength ext_mask =  __builtin_convertvector(__ocl_extend_mask_to_$VecLength(indexes_mask), char$VecLength);

      res_cur = intel_sub_group_shuffle($Arg1VarName, *(uint8*)&sg_indexes, *(uint8*)&indexes_mask);
      temp = *((char$VecLength*)&res_cur) & ext_mask;
      res_cur = *($Arg0Type*)&temp;

      int8 other_ind = sg_indexes + max_sg_size;
      res_next = intel_sub_group_shuffle($Arg0VarName, *(uint8*)&other_ind, *(uint8*)&inv_indexes_mask);
      temp = *((char$VecLength*)&res_next) & !ext_mask;
      res_next = *($Arg0Type*)&temp;

      return  res_cur | res_next;
  }]>;

// short, ushort
OclBuiltinImpl sub_group_shuffle_up_avx2_iu16 = OclBuiltinImpl<sub_group_shuffle_up_avx2,
        [v8i16, v8u16, v16i16, v16u16, v32i16, v32u16, v64i16, v64u16, v128i16, v128u16], 0,
  [{
      $Arg0Type res_cur, res_next;
      short$VecLength temp;
      int8 sg_indexes = { 0, 1, 2, 3, 4, 5, 6, 7 };
      const int max_sg_size = 8;

      sg_indexes = sg_indexes - *(int8*)&$Arg2VarName;

      sg_indexes &= *(int8*)&$Arg3VarName;

      int8 indexes_mask = sg_indexes >= 0;
      int8 inv_indexes_mask = !indexes_mask;

      short$VecLength ext_mask =  __builtin_convertvector(__ocl_extend_mask_to_$VecLength(indexes_mask), short$VecLength);

      res_cur = intel_sub_group_shuffle($Arg1VarName, *(uint8*)&sg_indexes, *(uint8*)&indexes_mask);
      temp = *((short$VecLength*)&res_cur) & ext_mask;
      res_cur = *($Arg0Type*)&temp;

      int8 other_ind = sg_indexes + max_sg_size;
      res_next = intel_sub_group_shuffle($Arg0VarName, *(uint8*)&other_ind, *(uint8*)&inv_indexes_mask);
      temp = *((short$VecLength*)&res_next) & !ext_mask;
      res_next = *($Arg0Type*)&temp;

      return  res_cur | res_next;
  }]>;

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

//
// Block read/write
//
code BlockRead1_8 =
  [{
    const $ReturnType res = vload8(0, $Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 1, 2, 3, 4, 5, 6, 7);
  }];
OclBuiltinImpl intel_sub_group_block_read1_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read1_8, [ v1u32 ], 0, BlockRead1_8>;
OclBuiltinImpl intel_sub_group_block_read_us1_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read_us1_8, intel_sub_group_block_read_write_us_types, 0, BlockRead1_8>;

code BlockRead2_8 =
  [{
    const $ReturnType res = vload16(0, $Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 8,
                                             1, 9,
                                             2, 10,
                                             3, 11,
                                             4, 12,
                                             5, 13,
                                             6, 14,
                                             7, 15);
  }];
OclBuiltinImpl intel_sub_group_block_read2_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read2_8, [ v1u32 ], 0, BlockRead2_8>;
OclBuiltinImpl intel_sub_group_block_read_us2_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read_us2_8, intel_sub_group_block_read_write_us_types, 0, BlockRead2_8>;

code BlockRead4_8 =
  [{
    $ReturnType res;
    res.lo  = vload16(0, $Arg0VarName);
    res.hi  = vload16(1, $Arg0VarName);
    return __ocl_shuffle_transpose_$ReturnBaseType#_8x4(res);
  }];
OclBuiltinImpl intel_sub_group_block_read4_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read4_8, [ v1u32 ], 0, BlockRead4_8>;
OclBuiltinImpl intel_sub_group_block_read_us4_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read_us4_8, intel_sub_group_block_read_write_us_types, 0, BlockRead4_8>;

code BlockRead8_8 =
  [{
    $ReturnType res;
    res.lo.lo  = vload16(0, $Arg0VarName);
    res.lo.hi  = vload16(1, $Arg0VarName);
    res.hi.lo  = vload16(2, $Arg0VarName);
    res.hi.hi  = vload16(3, $Arg0VarName);
    return __ocl_shuffle_transpose_$ReturnBaseType#_8x8(res);
  }];
OclBuiltinImpl intel_sub_group_block_read8_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read8_8, [ v1u32 ], 0, BlockRead8_8>;
OclBuiltinImpl intel_sub_group_block_read_us8_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read_us8_8, intel_sub_group_block_read_write_us_types, 0, BlockRead8_8>;

OclBuiltinImpl intel_sub_group_block_read_us16_8_avx2 = OclBuiltinImpl<intel_sub_group_block_read_us16_8, [v1u8], 0,
  [{
    $ReturnType res;
    res.lo.lo.lo  = vload16(0, $Arg0VarName);
    res.lo.lo.hi  = vload16(1, $Arg0VarName);
    res.lo.hi.lo  = vload16(2, $Arg0VarName);
    res.lo.hi.hi  = vload16(3, $Arg0VarName);
    res.hi.lo.lo  = vload16(4, $Arg0VarName);
    res.hi.lo.hi  = vload16(5, $Arg0VarName);
    res.hi.hi.lo  = vload16(6, $Arg0VarName);
    res.hi.hi.hi  = vload16(7, $Arg0VarName);
    return __ocl_shuffle_transpose_$ReturnBaseType#_8x16(res);
  }]>;

code BlockWrite1_8 =
  [{
    $Arg1BaseType#8 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 1, 2, 3, 4, 5, 6, 7);
    vstore8(res, 0, $Arg0VarName);
  }];
OclBuiltinImpl intel_sub_group_block_write1_8_avx2 = OclBuiltinImpl<intel_sub_group_block_write1_8, [ v1u32 ], 0, BlockWrite1_8>;
OclBuiltinImpl intel_sub_group_block_write_us1_8_avx2 = OclBuiltinImpl<intel_sub_group_block_write_us1_8, intel_sub_group_block_read_write_us_types, 0, BlockWrite1_8>;

code BlockWrite2_8 =
  [{
    $Arg1BaseType#16 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 2, 4, 6, 8, 10, 12, 14,
                                                                               1, 3, 5, 7, 9, 11, 13, 15);
    vstore16(res, 0, $Arg0VarName);
  }];
OclBuiltinImpl intel_sub_group_block_write2_8_avx2 = OclBuiltinImpl<intel_sub_group_block_write2_8, [ v1u32 ], 0, BlockWrite2_8>;
OclBuiltinImpl intel_sub_group_block_write_us2_8_avx2 = OclBuiltinImpl<intel_sub_group_block_write_us2_8, intel_sub_group_block_read_write_us_types, 0, BlockWrite2_8>;

code BlockWrite4_8 =
  [{
    $Arg1BaseType#32 res = __ocl_shuffle_transpose_$Arg1BaseType#_4x8($Arg1VarName);
    vstore16(res.lo, 0, $Arg0VarName);
    vstore16(res.hi, 1, $Arg0VarName);
  }];
OclBuiltinImpl intel_sub_group_block_write4_8_avx2 = OclBuiltinImpl<intel_sub_group_block_write4_8, [ v1u32 ], 0, BlockWrite4_8>;
OclBuiltinImpl intel_sub_group_block_write_us4_8_avx2 = OclBuiltinImpl<intel_sub_group_block_write_us4_8, intel_sub_group_block_read_write_us_types, 0, BlockWrite4_8>;

code BlockWrite8_8 =
  [{
    $Arg1BaseType#64 res = __ocl_shuffle_transpose_$Arg1BaseType#_8x8($Arg1VarName);
    vstore16(res.lo.lo, 0, $Arg0VarName);
    vstore16(res.lo.hi, 1, $Arg0VarName);
    vstore16(res.hi.lo, 2, $Arg0VarName);
    vstore16(res.hi.hi, 3, $Arg0VarName);
  }];
OclBuiltinImpl intel_sub_group_block_write8_8_avx2 = OclBuiltinImpl<intel_sub_group_block_write8_8, [ v1u32 ], 0, BlockWrite8_8>;
OclBuiltinImpl intel_sub_group_block_write_us8_8_avx2 = OclBuiltinImpl<intel_sub_group_block_write_us8_8, intel_sub_group_block_read_write_us_types, 0, BlockWrite8_8>;

OclBuiltinImpl intel_sub_group_block_write_us16_8_avx2 = OclBuiltinImpl<intel_sub_group_block_write_us16_8, [v1u8], 0,
  [{
    $Arg1BaseType#128 res = __ocl_shuffle_transpose_$Arg1BaseType#_16x8($Arg1VarName);
    vstore16(res.lo.lo.lo, 0, $Arg0VarName);
    vstore16(res.lo.lo.hi, 1, $Arg0VarName);
    vstore16(res.lo.hi.lo, 2, $Arg0VarName);
    vstore16(res.lo.hi.hi, 3, $Arg0VarName);
    vstore16(res.hi.lo.lo, 4, $Arg0VarName);
    vstore16(res.hi.lo.hi, 5, $Arg0VarName);
    vstore16(res.hi.hi.lo, 6, $Arg0VarName);
    vstore16(res.hi.hi.hi, 7, $Arg0VarName);
  }]>;
