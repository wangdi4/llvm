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

OclBuiltinImpl sub_group_reduce_add_avx128i8 = OclBuiltinImpl<sub_group_reduce_add_vec_cs, [v4i8], 0,
  [{
    __m128i temp = _mm_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_reduce_add(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx128u8 = OclBuiltinImpl<sub_group_reduce_add_vec_cs, [v4u8], 0,
  [{
    __m128i temp = _mm_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_reduce_add(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx128i16 = OclBuiltinImpl<sub_group_reduce_add_vec_cs, [v4i16], 0,
  [{
    __m128i temp = _mm_cvtepi16_epi32(__builtin_astype((short8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_reduce_add(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx128u16 = OclBuiltinImpl<sub_group_reduce_add_vec_cs, [v4u16], 0,
  [{
    __m128i temp = _mm_cvtepu16_epi32(__builtin_astype((ushort8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_reduce_add(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
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

OclBuiltinImpl sub_group_reduce_min_avx128i8 = OclBuiltinImpl<sub_group_reduce_min_vec_cs, [v4i8], 0,
  [{
    __m128i temp = _mm_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_reduce_min(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx128u8 = OclBuiltinImpl<sub_group_reduce_min_vec_cs, [v4u8], 0,
  [{
    __m128i temp = _mm_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_reduce_min(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx128i16 = OclBuiltinImpl<sub_group_reduce_min_vec_cs, [v4i16], 0,
  [{
    __m128i temp = _mm_cvtepi16_epi32(__builtin_astype((short8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_reduce_min(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx128u16 = OclBuiltinImpl<sub_group_reduce_min_vec_cs, [v4u16], 0,
  [{
    __m128i temp = _mm_cvtepu16_epi32(__builtin_astype((ushort8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_reduce_min(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
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

OclBuiltinImpl sub_group_reduce_max_avx128i8 = OclBuiltinImpl<sub_group_reduce_max_vec_cs, [v4i8], 0,
  [{
    __m128i temp = _mm_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_reduce_max(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx128u8 = OclBuiltinImpl<sub_group_reduce_max_vec_cs, [v4u8], 0,
  [{
    __m128i temp = _mm_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_reduce_max(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx128i16 = OclBuiltinImpl<sub_group_reduce_max_vec_cs, [v4i16], 0,
  [{
    __m128i temp = _mm_cvtepi16_epi32(__builtin_astype((short8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_reduce_max(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx128u16 = OclBuiltinImpl<sub_group_reduce_max_vec_cs, [v4u16], 0,
  [{
    __m128i temp = _mm_cvtepu16_epi32(__builtin_astype((ushort8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_reduce_max(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
    for (int i = 0; i < 3; i++) {
      temp[3] = 0;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName.lo = as_$Arg0BaseType#2(_mm_add_pd((__m128d)$Arg0VarName.lo, (__m128d)temp.lo));
      $Arg0VarName.hi = as_$Arg0BaseType#2(_mm_add_pd((__m128d)$Arg0VarName.hi, (__m128d)temp.hi));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx128i8 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec_cs, [v4i8], 0,
  [{
    __m128i temp = _mm_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_inclusive_add(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx128u8 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec_cs, [v4u8], 0,
  [{
    __m128i temp = _mm_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_inclusive_add(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx128i16 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec_cs, [v4i16], 0,
  [{
    __m128i temp = _mm_cvtepi16_epi32(__builtin_astype((short8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_inclusive_add(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx128u16 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec_cs, [v4u16], 0,
  [{
    __m128i temp = _mm_cvtepu16_epi32(__builtin_astype((ushort8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_inclusive_add(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
    for (int i = 0; i < 3; i++) {
      temp0[3] = INF;
      temp0 = __builtin_shufflevector(temp0, temp0, 3, 0, 1, 2);
      arg0.lo = as_long2( _mm_min_pd((__m128d)arg0.lo, (__m128d)temp0.lo));
      arg0.hi = as_long2( _mm_min_pd((__m128d)arg0.hi, (__m128d)temp0.hi));
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx128i8 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec_cs, [v4i8], 0,
  [{
    __m128i temp = _mm_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_inclusive_min(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx128u8 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec_cs, [v4u8], 0,
  [{
    __m128i temp = _mm_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_inclusive_min(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx128i16 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec_cs, [v4i16], 0,
  [{
    __m128i temp = _mm_cvtepi16_epi32(__builtin_astype((short8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_inclusive_min(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx128u16 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec_cs, [v4u16], 0,
  [{
    __m128i temp = _mm_cvtepu16_epi32(__builtin_astype((ushort8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_inclusive_min(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
    for (int i = 0; i < 3; i++) {
      temp0[3] = INF;
      temp0 = __builtin_shufflevector(temp0, temp0, 3, 0, 1, 2);
      arg0.lo = as_long2( _mm_max_pd((__m128d)arg0.lo, (__m128d)temp0.lo));
      arg0.hi = as_long2( _mm_max_pd((__m128d)arg0.hi, (__m128d)temp0.hi));
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx128i8 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec_cs, [v4i8], 0,
  [{
    __m128i temp = _mm_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_inclusive_max(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx128u8 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec_cs, [v4u8], 0,
  [{
    __m128i temp = _mm_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_inclusive_max(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx128i16 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec_cs, [v4i16], 0,
  [{
    __m128i temp = _mm_cvtepi16_epi32(__builtin_astype((short8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_inclusive_max(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx128u16 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec_cs, [v4u16], 0,
  [{
    __m128i temp = _mm_cvtepu16_epi32(__builtin_astype((ushort8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_inclusive_max(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = 0;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      $Arg0VarName.hi = as_$Arg0BaseType#2(_mm_add_pd((__m128d)$Arg0VarName.hi, (__m128d)temp.hi));
    }
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx128i8 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec_cs, [v4i8], 0,
  [{
    __m128i temp = _mm_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_exclusive_add(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx128u8 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec_cs, [v4u8], 0,
  [{
    __m128i temp = _mm_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_exclusive_add(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx128i16 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec_cs, [v4i16], 0,
  [{
    __m128i temp = _mm_cvtepi16_epi32(__builtin_astype((short8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_exclusive_add(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx128u16 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec_cs, [v4u16], 0,
  [{
    __m128i temp = _mm_cvtepu16_epi32(__builtin_astype((ushort8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_exclusive_add(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type(temp1);
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = INF;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      arg0.hi = as_long2(_mm_min_pd((__m128d)arg0.hi, (__m128d)temp.hi));
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx128i8 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec_cs, [v4i8], 0,
  [{
    __m128i temp = _mm_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_exclusive_min(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type_sat_rtp(temp1);
  }]>;  // round to positive infinity during conversion

OclBuiltinImpl sub_group_scan_exclusive_min_avx128u8 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec_cs, [v4u8], 0,
  [{
    __m128i temp = _mm_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_exclusive_min(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type_sat_rtp(temp1);
  }]>;  // round to positive infinity during conversion

OclBuiltinImpl sub_group_scan_exclusive_min_avx128i16 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec_cs, [v4i16], 0,
  [{
    __m128i temp = _mm_cvtepi16_epi32(__builtin_astype((short8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_exclusive_min(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type_sat_rtp(temp1);
  }]>;  // round to positive infinity during conversion

OclBuiltinImpl sub_group_scan_exclusive_min_avx128u16 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec_cs, [v4u16], 0,
  [{
    __m128i temp = _mm_cvtepu16_epi32(__builtin_astype((ushort8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_exclusive_min(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type_sat_rtp(temp1);
  }]>;  // round to positive infinity during conversion

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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
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

#pragma clang loop unroll(full)
    for (int i = 0; i < 2; i++) {
      temp[3] = INF;
      temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
      arg0.hi = as_long2(_mm_max_pd((__m128d)arg0.hi, (__m128d)temp.hi));
    }
    return as_$Arg0Type(arg0);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx128i8 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec_cs, [v4i8], 0,
  [{
    __m128i temp = _mm_cvtepi8_epi32(__builtin_astype((char16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_exclusive_max(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type_sat_rtn(temp1);
  }]>;  // round to negative infinity during conversion

OclBuiltinImpl sub_group_scan_exclusive_max_avx128u8 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec_cs, [v4u8], 0,
  [{
    __m128i temp = _mm_cvtepu8_epi32(__builtin_astype((uchar16)($Arg0VarName, $Arg0VarName, $Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_exclusive_max(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type_sat_rtz(temp1);
  }]>;  // round to zero during conversion

OclBuiltinImpl sub_group_scan_exclusive_max_avx128i16 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec_cs, [v4i16], 0,
  [{
    __m128i temp = _mm_cvtepi16_epi32(__builtin_astype((short8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_exclusive_max(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type_sat_rtn(temp1);
  }]>;  // round to negative infinity during conversion

OclBuiltinImpl sub_group_scan_exclusive_max_avx128u16 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec_cs, [v4u16], 0,
  [{
    __m128i temp = _mm_cvtepu16_epi32(__builtin_astype((ushort8)($Arg0VarName, $Arg0VarName), __m128i));
    int4 temp1 = sub_group_scan_exclusive_max(__builtin_astype(temp, int4), $Arg1VarName);
    return convert_$Arg0Type_sat_rtz(temp1);
  }]>;  // round to zero during conversion

//
// Shuffle
//
// Scalar
OclBuiltinImpl sub_group_shuffle_avx128v4ui32 = OclBuiltinImpl<sub_group_shuffle_avx, [v4i32, v4u32, v4f32,
                                                                                       v4i8, v4u8, v4i16, v4u16,
                                                                                       v4i64, v4u64, v4f64], 0,
  [{
    $Arg1VarName = $Arg1VarName & $Arg2VarName;
    $ReturnType res;
#pragma clang loop unroll(full)
    for (int i =0; i < 4; ++i) {
      res[i] = $Arg0VarName[$Arg1VarName[i]];
    }
    return res;
  }]>;

// Vector types
OclBuiltinImpl sub_group_shuffle_avxv8uif32 = OclBuiltinImpl<sub_group_shuffle_avx, [v8u32, v8i32, v8f32], 0,
 [{
    long4 temp = __builtin_astype($Arg0VarName, long4);
    temp = intel_sub_group_shuffle(temp, $Arg1VarName, $Arg2VarName);
    return __builtin_astype(temp, $ReturnType);
 }]>;

// char2 and uchar2
OclBuiltinImpl sub_group_shuffle_avx128v8ui8 = OclBuiltinImpl<sub_group_shuffle_avx, [v8i8, v8u8], 0,
  [{
    short4 temp = __builtin_astype($Arg0VarName, short4);
    temp = intel_sub_group_shuffle(temp, $Arg1VarName, $Arg2VarName);
    return __builtin_astype(temp, $ReturnType);
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
    long4 temp = __builtin_astype($Arg0VarName, long4);
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

OclBuiltinImpl sub_group_shuffle_avxv12gentype = OclBuiltinImpl<sub_group_shuffle_avx, [v12i8, v12u8, v12i16, v12u16, v12f32,
                                                                                        v12i32, v12u32, v12i64, v12u64], 0,
  [{
     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x4($Arg0VarName);
     $Arg0BaseType#4* arg0 = ($Arg0BaseType#4*)&$Arg0VarName;

     $Arg0BaseType#4 tmp1 = intel_sub_group_shuffle(arg0[0], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#4 tmp2 = intel_sub_group_shuffle(arg0[1], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#4 tmp3 = intel_sub_group_shuffle(arg0[2], $Arg1VarName, $Arg2VarName);

     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_4x3(($Arg0Type)(tmp1, tmp2, tmp3));
     return $Arg0VarName;
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

OclBuiltinImpl sub_group_shuffle_down_avxv12gentype = OclBuiltinImpl<sub_group_shuffle_down_avx, [v12i8, v12u8, v12i16, v12u16, v12f32,
                                                                                                  v12i32, v12u32, v12i64, v12u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x4($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x4($Arg1VarName);

    $Arg0BaseType#4* arg0 = ($Arg0BaseType#4*)&$Arg0VarName;
    $Arg0BaseType#4* arg1 = ($Arg0BaseType#4*)&$Arg1VarName;

    $Arg0BaseType#4 tmp1 = intel_sub_group_shuffle_down(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#4 tmp2 = intel_sub_group_shuffle_down(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#4 tmp3 = intel_sub_group_shuffle_down(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_4x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
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

code khr_sub_group_shuffle_up_code = [{
    int$VecLength sg_indexes = $INDEXES;
    sg_indexes = sg_indexes - *(int$VecLength*)&$Arg1VarName;
    sg_indexes &= *(int$VecLength*)&$Arg2VarName;
    int$VecLength indexes_mask = sg_indexes >= 0;
    return sub_group_shuffle($Arg0VarName, *(uint$VecLength*)&sg_indexes, *(uint$VecLength*)&indexes_mask);
}];

OclBuiltinImpl khr_sub_group_shuffle_up_avx_gentype = OclBuiltinImpl<khr_sub_group_shuffle_up_avx, khr_sub_group_shuffle_up_avx.Types, 0, khr_sub_group_shuffle_up_code, [["$INDEXES", "{0, 1, 2, 3}"]]>;
OclBuiltinImpl khr_sub_group_shuffle_up_avx2_gentype = OclBuiltinImpl<khr_sub_group_shuffle_up_avx2, khr_sub_group_shuffle_up_avx2.Types, 0, khr_sub_group_shuffle_up_code, [["$INDEXES", "{0, 1, 2, 3, 4, 5, 6, 7}"]]>;
OclBuiltinImpl khr_sub_group_shuffle_up_avx512_gentype = OclBuiltinImpl<khr_sub_group_shuffle_up_avx512, khr_sub_group_shuffle_up_avx512.Types, 0, khr_sub_group_shuffle_up_code, [["$INDEXES", "{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}"]]>;
OclBuiltinImpl khr_sub_group_shuffle_up_x32_gentype = OclBuiltinImpl<khr_sub_group_shuffle_up_x32, khr_sub_group_shuffle_up_x32.Types, 0, khr_sub_group_shuffle_up_code, [["$INDEXES", "{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31}"]]>;
OclBuiltinImpl khr_sub_group_shuffle_up_x64_gentype = OclBuiltinImpl<khr_sub_group_shuffle_up_x64, khr_sub_group_shuffle_up_x64.Types, 0, khr_sub_group_shuffle_up_code, [["$INDEXES", "{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63}"]]>;

code khr_sub_group_shuffle_down_code = [{
    uint$VecLength sg_indexes = $INDEXES;
    sg_indexes = sg_indexes + $Arg1VarName;
    sg_indexes &= $Arg2VarName;
    int$VecLength indexes_mask = sg_indexes < $VecLength;
    return sub_group_shuffle($Arg0VarName, sg_indexes, *(uint$VecLength*)&indexes_mask);
}];

OclBuiltinImpl khr_sub_group_shuffle_down_avx_gentype = OclBuiltinImpl<khr_sub_group_shuffle_down_avx, khr_sub_group_shuffle_down_avx.Types, 0, khr_sub_group_shuffle_down_code, [["$INDEXES", "{0, 1, 2, 3}"]]>;
OclBuiltinImpl khr_sub_group_shuffle_down_avx2_gentype = OclBuiltinImpl<khr_sub_group_shuffle_down_avx2, khr_sub_group_shuffle_down_avx2.Types, 0, khr_sub_group_shuffle_down_code, [["$INDEXES", "{0, 1, 2, 3, 4, 5, 6, 7}"]]>;
OclBuiltinImpl khr_sub_group_shuffle_down_avx512_gentype = OclBuiltinImpl<khr_sub_group_shuffle_down_avx512, khr_sub_group_shuffle_down_avx512.Types, 0, khr_sub_group_shuffle_down_code, [["$INDEXES", "{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}"]]>;
OclBuiltinImpl khr_sub_group_shuffle_down_x32_gentype = OclBuiltinImpl<khr_sub_group_shuffle_down_x32, khr_sub_group_shuffle_down_x32.Types, 0, khr_sub_group_shuffle_down_code, [["$INDEXES", "{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31}"]]>;
OclBuiltinImpl khr_sub_group_shuffle_down_x64_gentype = OclBuiltinImpl<khr_sub_group_shuffle_down_x64, khr_sub_group_shuffle_down_x64.Types, 0, khr_sub_group_shuffle_down_code, [["$INDEXES", "{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63}"]]>;

OclBuiltinImpl khr_sub_group_shuffle_down_avx_gentype = OclBuiltinImpl<khr_sub_group_shuffle_down_avx,
  VFType<khr_shuffle_gentype, "_avx">.Tout, 0, [{
    uint4 sg_indexes = { 0, 1, 2, 3};
    sg_indexes = sg_indexes + $Arg1VarName;
    sg_indexes &= $Arg2VarName;
    int4 indexes_mask = sg_indexes < 4;
    return sub_group_shuffle($Arg0VarName, sg_indexes, *(uint4*)&indexes_mask);
  }]>;
OclBuiltinImpl khr_sub_group_shuffle_down_avx2_gentype = OclBuiltinImpl<khr_sub_group_shuffle_down_avx2,
  VFType<khr_shuffle_gentype, "_avx2">.Tout, 0, [{
    uint8 sg_indexes = { 0, 1, 2, 3, 4, 5, 6, 7};
    sg_indexes = sg_indexes + $Arg1VarName;
    sg_indexes &= $Arg2VarName;
    int8 indexes_mask = sg_indexes < 8;
    return sub_group_shuffle($Arg0VarName, sg_indexes, *(uint8*)&indexes_mask);
  }]>;
OclBuiltinImpl khr_sub_group_shuffle_down_avx512_gentype = OclBuiltinImpl<khr_sub_group_shuffle_down_avx512,
  VFType<khr_shuffle_gentype, "_avx512">.Tout, 0, [{
    uint16 sg_indexes = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    sg_indexes = sg_indexes + $Arg1VarName;
    sg_indexes &= $Arg2VarName;
    int16 indexes_mask = sg_indexes < 16;
    return sub_group_shuffle($Arg0VarName, sg_indexes, *(uint16*)&indexes_mask);
  }]>;

// char, uchar
OclBuiltinImpl sub_group_shuffle_up_avx_i8 = OclBuiltinImpl<sub_group_shuffle_up_avx,
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
OclBuiltinImpl sub_group_shuffle_up_avx_i16 = OclBuiltinImpl<sub_group_shuffle_up_avx,
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

OclBuiltinImpl sub_group_shuffle_up_avxv12gentype = OclBuiltinImpl<sub_group_shuffle_up_avx, [v12i8, v12u8, v12i16, v12u16, v12f32,
                                                                                              v12i32, v12u32, v12i64, v12u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x4($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x4($Arg1VarName);

    $Arg0BaseType#4* arg0 = ($Arg0BaseType#4*)&$Arg0VarName;
    $Arg0BaseType#4* arg1 = ($Arg0BaseType#4*)&$Arg1VarName;

    $Arg0BaseType#4 tmp1 = intel_sub_group_shuffle_up(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#4 tmp2 = intel_sub_group_shuffle_up(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#4 tmp3 = intel_sub_group_shuffle_up(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_4x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
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

OclBuiltinImpl intel_sub_group_block_read_us16_4_avx = OclBuiltinImpl<intel_sub_group_block_read_us16_4, [v1u8], 0,
  [{
    $ReturnType res;
    res.lo.lo  = vload16(0, $Arg0VarName);
    res.lo.hi  = vload16(1, $Arg0VarName);
    res.hi.lo  = vload16(2, $Arg0VarName);
    res.hi.hi  = vload16(3, $Arg0VarName);
    return __ocl_shuffle_transpose_$ReturnBaseType#_4x16(res);
  }]>;

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

OclBuiltinImpl intel_sub_group_block_write_us16_4_avx = OclBuiltinImpl<intel_sub_group_block_write_us16_4, [v1u8], 0,
  [{
    $Arg1BaseType#64 res = __ocl_shuffle_transpose_$Arg1BaseType#_16x4($Arg1VarName);
    vstore16(res.lo.lo, 0, $Arg0VarName);
    vstore16(res.lo.hi, 1, $Arg0VarName);
    vstore16(res.hi.lo, 2, $Arg0VarName);
    vstore16(res.hi.hi, 3, $Arg0VarName);
  }]>;

//
// non uniform logical
//
list<OclType> logical_vec_types = [v4i32, v8i32, v16i32];

// logical and
code reduce_logical_and = [{
  int$VecLength arg1 = as_int$VecLength($Arg1VarName);
  $Arg0VarName |= ~arg1;
  int res = 1;
#pragma clang loop unroll(full)
  for (int i=0; i < $VecLength; i++) {
    res = res && $Arg0VarName[i];
  }
  return res;
}];

OclBuiltinImpl sub_group_non_uniform_reduce_logical_and_all = OclBuiltinImpl<sub_group_non_uniform_reduce_logical_and_vec, logical_vec_types, 0, reduce_logical_and>;

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_and_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_and_vec, [v4i32], 0,
[{
  int4 arg1 = as_int4($Arg1VarName);
  $Arg0VarName |= ~arg1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 3; i++) {
    temp[3] = 1;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName = $Arg0VarName && temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_and_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_and_vec, [v8i32], 0,
[{
  int8 arg1 = as_int8($Arg1VarName);
  $Arg0VarName |= ~arg1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 7; i++) {
    temp[7] = 1;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName = $Arg0VarName && temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_and_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_and_vec, [v16i32], 0,
[{
  int16 arg1 = as_int16($Arg1VarName);
  $Arg0VarName |= ~arg1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 15; i++) {
    temp[15] = 1;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName = $Arg0VarName && temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_and_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_and_vec, [v4i32], 0,
[{
  int4 arg1 = as_int4($Arg1VarName);
  $Arg0VarName |= ~arg1;
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
  $Arg0VarName[0] = 1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 2; i++) {
    temp[3] = 1;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName = $Arg0VarName && temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_and_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_and_vec, [v8i32], 0,
[{
  int8 arg1 = as_int8($Arg1VarName);
  $Arg0VarName |= ~arg1;
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
  $Arg0VarName[0] = 1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 6; i++) {
    temp[7] = 1;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName = $Arg0VarName && temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_and_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_and_vec, [v16i32], 0,
[{
  int16 arg1 = as_int16($Arg1VarName);
  $Arg0VarName |= ~arg1;
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
  $Arg0VarName[0] = 1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 14; i++) {
    temp[15] = 1;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName = $Arg0VarName && temp;
  }
  return $Arg0VarName;
}]>;

// logical or, xor
code reduce_logical_or_xor = [{
  int$VecLength arg1 = as_int$VecLength($Arg1VarName);
  $Arg0VarName &= arg1;
  int res = 0;
#pragma clang loop unroll(full)
  for (int i=0; i < $VecLength; i++) {
    res = res $OP $Arg0VarName[i];
  }
  return res;
}];

OclBuiltinImpl sub_group_non_uniform_reduce_logical_or_all = OclBuiltinImpl<sub_group_non_uniform_reduce_logical_or_vec, logical_vec_types, 0, reduce_logical_or_xor, [["$OP", "||"]]>;
OclBuiltinImpl sub_group_non_uniform_reduce_logical_xor_all = OclBuiltinImpl<sub_group_non_uniform_reduce_logical_xor_vec, logical_vec_types, 0, reduce_logical_or_xor, [["$OP", "!="]]>;

code scan_inclusive_logical_or_xor_v4 = [{
  int4 arg1 = as_int4($Arg1VarName);
  $Arg0VarName &= arg1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 3; i++) {
    temp[3] = 0;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName = $Arg0VarName $OP temp;
  }
  return $Arg0VarName;
}];

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_or_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_or_vec, [v4i32], 0, scan_inclusive_logical_or_xor_v4, [["$OP", "||"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_xor_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_xor_vec, [v4i32], 0, scan_inclusive_logical_or_xor_v4, [["$OP", "!="]]>;

code scan_inclusive_logical_or_xor_v8 = [{
  int8 arg1 = as_int8($Arg1VarName);
  $Arg0VarName &= arg1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 7; i++) {
    temp[7] = 0;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName = $Arg0VarName $OP temp;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_or_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_or_vec, [v8i32], 0, scan_inclusive_logical_or_xor_v8, [["$OP", "||"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_xor_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_xor_vec, [v8i32], 0, scan_inclusive_logical_or_xor_v8, [["$OP", "!="]]>;

code scan_inclusive_logical_or_xor_v16 = [{
  int16 arg1 = as_int16($Arg1VarName);
  $Arg0VarName &= arg1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 15; i++) {
    temp[15] = 0;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName = $Arg0VarName $OP temp;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_or_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_or_vec, [v16i32], 0, scan_inclusive_logical_or_xor_v16, [["$OP", "||"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_xor_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_xor_vec, [v16i32], 0, scan_inclusive_logical_or_xor_v16, [["$OP", "!="]]>;

code scan_exclusive_logical_or_xor_v4 = [{
  int4 arg1 = as_int4($Arg1VarName);
  $Arg0VarName &= arg1;
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
  $Arg0VarName[0] = 0;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 2; i++) {
    temp[3] = 0;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName = $Arg0VarName $OP temp;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_or_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_or_vec, [v4i32], 0, scan_exclusive_logical_or_xor_v4, [["$OP", "||"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_xor_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_xor_vec, [v4i32], 0, scan_exclusive_logical_or_xor_v4, [["$OP", "!="]]>;

code scan_exclusive_logical_or_xor_v8 = [{
  int8 arg1 = as_int8($Arg1VarName);
  $Arg0VarName &= arg1;
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
  $Arg0VarName[0] = 0;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 6; i++) {
    temp[7] = 0;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName = $Arg0VarName $OP temp;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_or_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_or_vec, [v8i32], 0, scan_exclusive_logical_or_xor_v8, [["$OP", "||"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_xor_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_xor_vec, [v8i32], 0, scan_exclusive_logical_or_xor_v8, [["$OP", "!="]]>;

code scan_exclusive_logical_or_xor_v16 = [{
  int16 arg1 = as_int16($Arg1VarName);
  $Arg0VarName &= arg1;
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
  $Arg0VarName[0] = 0;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 14; i++) {
    temp[15] = 0;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName = $Arg0VarName $OP temp;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_or_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_or_vec, [v16i32], 0, scan_exclusive_logical_or_xor_v16, [["$OP", "||"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_xor_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_xor_vec, [v16i32], 0, scan_exclusive_logical_or_xor_v16, [["$OP", "!="]]>;


// non uniform bitwise
list<OclType> bitwise_vec_types = [v4i8, v4u8, v8i8, v8u8, v16i8, v16u8,
                                   v4i16, v4u16, v8i16, v8u16, v16i16, v16u16,
                                   v4i32, v4u32, v8i32, v8u32, v16i32, v16u32,
                                   v4i64, v4u64, v8i64, v8u64, v16i64, v16u64];
code reduce_and = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 |= ~arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0BaseType res = $Arg0VarName[0];
#pragma clang loop unroll(full)
  for (int i=1; i < $VecLength; i++) {
    res &= $Arg0VarName[i];
  }
  return res;
}];
OclBuiltinImpl sub_group_non_uniform_reduce_and_all = OclBuiltinImpl<sub_group_non_uniform_reduce_and_vec, bitwise_vec_types, 0, reduce_and>;

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_and_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_and_vec,
  [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64], 0, [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 |= ~arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 3; i++) {
    temp[3] = -1;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName &= temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_and_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_and_vec,
  [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64], 0, [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 |= ~arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 7; i++) {
    temp[7] = -1;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName &= temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_and_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_and_vec,
  [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64], 0, [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 |= ~arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 15; i++) {
    temp[15] = -1;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName &= temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_exclusive_and_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_and_vec,
  [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64], 0, [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 |= ~arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
  $Arg0VarName[0] = -1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 2; i++) {
    temp[3] = -1;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName &= temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_exclusive_and_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_and_vec,
  [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64], 0, [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 |= ~arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
  $Arg0VarName[0] = -1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 6; i++) {
    temp[7] = -1;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName &= temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_exclusive_and_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_and_vec,
  [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64], 0, [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 |= ~arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
  $Arg0VarName[0] = -1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 14; i++) {
    temp[15] = -1;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName &= temp;
  }
  return $Arg0VarName;
}]>;

code reduce_or_xor = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0BaseType res = $Arg0VarName[0];
#pragma clang loop unroll(full)
  for (int i=1; i < $VecLength; i++) {
    res = $Arg0VarName[i] $OP res;
  }
  return res;
}];
OclBuiltinImpl sub_group_non_uniform_reduce_or_all = OclBuiltinImpl<sub_group_non_uniform_reduce_or_vec, bitwise_vec_types, 0, reduce_or_xor, [["$OP", "|"]]>;
OclBuiltinImpl sub_group_non_uniform_reduce_xor_all = OclBuiltinImpl<sub_group_non_uniform_reduce_xor_vec, bitwise_vec_types, 0, reduce_or_xor, [["$OP", "^"]]>;

code scan_inclusive_or_xor_v4 = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 3; i++) {
    temp[3] = 0;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName = temp $OP $Arg0VarName;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_or_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_or_vec,
  [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64], 0, scan_inclusive_or_xor_v4, [["$OP", "|"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_xor_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_xor_vec,
  [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64], 0, scan_inclusive_or_xor_v4, [["$OP", "^"]]>;

code scan_inclusive_or_xor_v8 = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 7; i++) {
    temp[7] = 0;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName = temp $OP $Arg0VarName;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_or_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_or_vec,
  [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64], 0, scan_inclusive_or_xor_v8, [["$OP", "|"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_xor_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_xor_vec,
  [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64], 0, scan_inclusive_or_xor_v8, [["$OP", "^"]]>;

code scan_inclusive_or_xor_v16 = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 15; i++) {
    temp[15] = 0;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName = temp $OP $Arg0VarName;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_or_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_or_vec,
  [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64], 0, scan_inclusive_or_xor_v16, [["$OP", "|"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_xor_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_xor_vec,
  [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64], 0, scan_inclusive_or_xor_v16, [["$OP", "^"]]>;


code scan_exclusive_or_xor_v4 = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
  $Arg0VarName[0] = 0;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 2; i++) {
    temp[3] = 0;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName = temp $OP $Arg0VarName;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_or_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_or_vec,
  [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64], 0, scan_inclusive_or_xor_v4, [["$OP", "|"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_xor_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_xor_vec,
  [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64], 0, scan_inclusive_or_xor_v4, [["$OP", "^"]]>;

code scan_exclusive_or_xor_v8 = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
  $Arg0VarName[0] = 0;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 6; i++) {
    temp[7] = 0;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName = temp $OP $Arg0VarName;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_or_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_or_vec,
  [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64], 0, scan_inclusive_or_xor_v8, [["$OP", "|"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_xor_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_xor_vec,
  [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64], 0, scan_inclusive_or_xor_v8, [["$OP", "^"]]>;

code scan_exclusive_or_xor_v16 = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
  $Arg0VarName[0] = 0;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 14; i++) {
    temp[15] = 0;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName = temp $OP $Arg0VarName;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_or_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_or_vec,
  [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64], 0, scan_inclusive_or_xor_v16, [["$OP", "|"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_xor_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_xor_vec,
  [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64], 0, scan_inclusive_or_xor_v16, [["$OP", "^"]]>;


// non uniform arithmetic
list<OclType> arithmetic_vec_types = [v4i8, v4u8, v8i8, v8u8, v16i8, v16u8,
                                      v4i16, v4u16, v8i16, v8u16, v16i16, v16u16,
                                      v4i32, v4u32, v8i32, v8u32, v16i32, v16u32,
                                      v4i64, v4u64, v8i64, v8u64, v16i64, v16u64,
                                      v4f32, v8f32, v16f32, v4f64, v8f64, v16f64];

code reduce_mul = [{
  $MaskCastTy$VecLength mask0 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= mask0;
  $Arg0BaseType one = 1;
  arg0 |= (~mask0 & as_$MaskCastTy(one));
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0BaseType res = 1;
#pragma clang loop unroll(full)
  for (int i=0; i < $VecLength; i++) {
    res *= $Arg0VarName[i];
  }
  return res;
  }];
OclBuiltinImpl sub_group_non_uniform_reduce_mul_all = OclBuiltinImpl<sub_group_non_uniform_reduce_mul_vec, arithmetic_vec_types, 0, reduce_mul>;

code scan_inclusive_mul_v4 = [{
  $MaskCastTy#4 mask0 = __builtin_convertvector(as_int4($Arg1VarName), $MaskCastTy#4);
  $MaskCastTy#4 arg0 = as_$MaskCastTy#4($Arg0VarName);
  arg0 &= mask0;
  $Arg0BaseType one = 1;
  arg0 |= (~mask0 & as_$MaskCastTy(one));
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 3; i++) {
    temp[3] = 1;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName *= temp;
  }
  return $Arg0VarName;
}];
code scan_inclusive_mul_v8 = [{
  $MaskCastTy#8 mask0 = __builtin_convertvector(as_int8($Arg1VarName), $MaskCastTy#8);
  $MaskCastTy#8 arg0 = as_$MaskCastTy#8($Arg0VarName);
  arg0 &= mask0;
  $Arg0BaseType one = 1;
  arg0 |= (~mask0 & as_$MaskCastTy(one));
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 7; i++) {
    temp[7] = 1;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName *= temp;
  }
  return $Arg0VarName;
}];
code scan_inclusive_mul_v16 = [{
  $MaskCastTy#16 mask0 = __builtin_convertvector(as_int16($Arg1VarName), $MaskCastTy#16);
  $MaskCastTy#16 arg0 = as_$MaskCastTy#16($Arg0VarName);
  arg0 &= mask0;
  $Arg0BaseType one = 1;
  arg0 |= (~mask0 & as_$MaskCastTy(one));
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 15; i++) {
    temp[15] = 1;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName *= temp;
  }
  return $Arg0VarName;
}];

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_mul_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_mul_vec, [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4f32, v4f64, v4i64, v4u64], 0, scan_inclusive_mul_v4>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_mul_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_mul_vec, [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8f32, v8f64, v8i64, v8u64], 0, scan_inclusive_mul_v8>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_mul_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_mul_vec, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32, v16f64, v16i64, v16u64], 0, scan_inclusive_mul_v16>;

code scan_exclusive_mul_v4 = [{
  $MaskCastTy#4 mask0 = __builtin_convertvector(as_int4($Arg1VarName), $MaskCastTy#4);
  $MaskCastTy#4 arg0 = as_$MaskCastTy#4($Arg0VarName);
  arg0 &= mask0;
  $Arg0BaseType one = 1;
  arg0 |= (~mask0 & as_$MaskCastTy(one));
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
  $Arg0VarName[0] = 1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 2; i++) {
    temp[3] = 1;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName *= temp;
  }
  return $Arg0VarName;
}];
code scan_exclusive_mul_v8 = [{
  $MaskCastTy#8 mask0 = __builtin_convertvector(as_int8($Arg1VarName), $MaskCastTy#8);
  $MaskCastTy#8 arg0 = as_$MaskCastTy#8($Arg0VarName);
  arg0 &= mask0;
  $Arg0BaseType one = 1;
  arg0 |= (~mask0 & as_$MaskCastTy(one));
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
  $Arg0VarName[0] = 1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 6; i++) {
    temp[7] = 1;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName *= temp;
  }
  return $Arg0VarName;
}];
code scan_exclusive_mul_v16 = [{
  $MaskCastTy#16 mask0 = __builtin_convertvector(as_int16($Arg1VarName), $MaskCastTy#16);
  $MaskCastTy#16 arg0 = as_$MaskCastTy#16($Arg0VarName);
  arg0 &= mask0;
  $Arg0BaseType one = 1;
  arg0 |= (~mask0 & as_$MaskCastTy(one));
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
  $Arg0VarName[0] = 1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang loop unroll(full)
  for (int i = 0; i < 14; i++) {
    temp[15] = 1;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName *= temp;
  }
  return $Arg0VarName;
}];

OclBuiltinImpl sub_group_non_uniform_scan_exclusive_mul_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_mul_vec, [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4f32, v4f64, v4i64, v4u64], 0, scan_exclusive_mul_v4>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_mul_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_mul_vec, [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8f32, v8f64, v8i64, v8u64], 0, scan_exclusive_mul_v8>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_mul_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_mul_vec, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32, v16f64, v16i64, v16u64], 0, scan_exclusive_mul_v16>;


OclBuiltinImpl sub_group_broadcast_ext_avx_v8 =
  OclBuiltinImpl<sub_group_broadcast_extended_avx, [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64, v8f32, v8f64], 0,
  [{

     $Arg0BaseType#2 temp = *(($Arg0BaseType#2*)&$Arg0VarName + $Arg1VarName);
     return ($ReturnType)(temp, temp, temp, temp);
  }]>;

OclBuiltinImpl sub_group_broadcast_ext_avx_v12 =
  OclBuiltinImpl<sub_group_broadcast_extended_avx, [v12i8, v12u8, v12i16, v12u16, v12i32, v12u32, v12i64, v12u64, v12f32, v12f64], 0,
  [{
     $Arg0BaseType#3 temp = *(($Arg0BaseType#3*)&$Arg0VarName + $Arg1VarName);
     return ($ReturnType)(temp, temp, temp, temp);
  }]>;

OclBuiltinImpl sub_group_broadcast_ext_avx_v16 =
  OclBuiltinImpl<sub_group_broadcast_extended_avx, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64, v16f32, v16f64], 0,
  [{
     $Arg0BaseType#4 temp = *(($Arg0BaseType#4*)&$Arg0VarName + $Arg1VarName);
     return ($ReturnType)(temp, temp, temp, temp);
  }]>;

OclBuiltinImpl sub_group_broadcast_ext_avx_v32 =
  OclBuiltinImpl<sub_group_broadcast_extended_avx, [v32i8, v32u8, v32i16, v32u16, v32i32, v32u32, v32i64, v32u64, v32f32, v32f64], 0,
  [{
     $Arg0BaseType#8 temp = *(($Arg0BaseType#8*)&$Arg0VarName + $Arg1VarName);
     return ($ReturnType)(temp, temp, temp, temp);
  }]>;

OclBuiltinImpl sub_group_broadcast_ext_avx_v64 =
  OclBuiltinImpl<sub_group_broadcast_extended_avx, [v64i8, v64u8, v64i16, v64u16, v64i32, v64u32, v64i64, v64u64, v64f32, v64f64], 0,
  [{
     $Arg0BaseType#16 temp = *(($Arg0BaseType#16*)&$Arg0VarName + $Arg1VarName);
     return ($ReturnType)(temp, temp, temp, temp);
  }]>;

OclBuiltinImpl sub_group_broadcast_ext_avx2_v16 =
  OclBuiltinImpl<sub_group_broadcast_extended_avx2, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64, v16f32, v16f64], 0,
  [{

     $Arg0BaseType#2 temp = *(($Arg0BaseType#2*)&$Arg0VarName + $Arg1VarName);
     return ($ReturnType)(temp, temp, temp, temp, temp, temp, temp, temp);
  }]>;

OclBuiltinImpl sub_group_broadcast_ext_avx2_v24 =
  OclBuiltinImpl<sub_group_broadcast_extended_avx2, [v24i8, v24u8, v24i16, v24u16, v24i32, v24u32, v24i64, v24u64, v24f32, v24f64], 0,
  [{
     $Arg0BaseType#3 temp = *(($Arg0BaseType#3*)&$Arg0VarName + $Arg1VarName);
     return ($ReturnType)(temp, temp, temp, temp, temp, temp, temp, temp);
  }]>;

OclBuiltinImpl sub_group_broadcast_ext_avx2_v32 =
  OclBuiltinImpl<sub_group_broadcast_extended_avx2, [v32i8, v32u8, v32i16, v32u16, v32i32, v32u32, v32i64, v32u64, v32f32, v32f64], 0,
  [{
     $Arg0BaseType#4 temp = *(($Arg0BaseType#4*)&$Arg0VarName + $Arg1VarName);
     return ($ReturnType)(temp, temp, temp, temp, temp, temp, temp, temp);
  }]>;

OclBuiltinImpl sub_group_broadcast_ext_avx2_v64 =
  OclBuiltinImpl<sub_group_broadcast_extended_avx2, [v64i8, v64u8, v64i16, v64u16, v64i32, v64u32, v64i64, v64u64, v64f32, v64f64], 0,
  [{
     $Arg0BaseType#8 temp = *(($Arg0BaseType#8*)&$Arg0VarName + $Arg1VarName);
     return ($ReturnType)(temp, temp, temp, temp, temp, temp, temp, temp);
  }]>;

OclBuiltinImpl sub_group_broadcast_ext_avx2_v128 =
  OclBuiltinImpl<sub_group_broadcast_extended_avx2, [v128i8, v128u8, v128i16, v128u16, v128i32, v128u32, v128i64, v128u64, v128f32, v128f64], 0,
  [{
     $Arg0BaseType#16 temp = *(($Arg0BaseType#16*)&$Arg0VarName + $Arg1VarName);
     return ($ReturnType)(temp, temp, temp, temp, temp, temp, temp, temp);
  }]>;

OclBuiltinImpl sub_group_broadcast_ext_avx512_v32 =
  OclBuiltinImpl<sub_group_broadcast_extended_avx512, [v32i8, v32u8, v32i16, v32u16, v32i32, v32u32, v32i64, v32u64, v32f32, v32f64], 0,
  [{
     $Arg0BaseType#2 temp = *(($Arg0BaseType#2*)&$Arg0VarName + $Arg1VarName);
     return ($ReturnType)(temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp);
  }]>;

OclBuiltinImpl sub_group_broadcast_ext_avx512_v48 =
  OclBuiltinImpl<sub_group_broadcast_extended_avx512, [v48i8, v48u8, v48i16, v48u16, v48i32, v48u32, v48i64, v48u64, v48f32, v48f64], 0,
  [{
     $Arg0BaseType#3 temp = *(($Arg0BaseType#3*)&$Arg0VarName + $Arg1VarName);
     return ($ReturnType)(temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp);
  }]>;

OclBuiltinImpl sub_group_broadcast_ext_avx512_v64 =
  OclBuiltinImpl<sub_group_broadcast_extended_avx512, [v64i8, v64u8, v64i16, v64u16, v64i32, v64u32, v64i64, v64u64, v64f32, v64f64], 0,
  [{
     $Arg0BaseType#4 temp = *(($Arg0BaseType#4*)&$Arg0VarName + $Arg1VarName);
     return ($ReturnType)(temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp);
  }]>;

OclBuiltinImpl sub_group_broadcast_ext_avx512_v128 =
  OclBuiltinImpl<sub_group_broadcast_extended_avx512, [v128i8, v128u8, v128i16, v128u16, v128i32, v128u32, v128i64, v128u64, v128f32, v128f64], 0,
  [{
     $Arg0BaseType#8 temp = *(($Arg0BaseType#8*)&$Arg0VarName + $Arg1VarName);
     return ($ReturnType)(temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp);
  }]>;

OclBuiltinImpl sub_group_broadcast_ext_avx512_v256 =
  OclBuiltinImpl<sub_group_broadcast_extended_avx512, [v256i8, v256u8, v256i16, v256u16, v256i32, v256u32, v256i64, v256u64, v256f32, v256f64], 0,
  [{
     $Arg0BaseType#16 temp = *(($Arg0BaseType#16*)&$Arg0VarName + $Arg1VarName);
     return ($ReturnType)(temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp, temp);
  }]>;

defvar broadcast_ext_base_types = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64];
defvar broadcast_ext_code =
[{
  $Arg0BaseType$VSize temp = *(($Arg0BaseType$VSize*)&$Arg0VarName + $Arg1VarName);
  return ($ReturnType)$Result;
}];

defvar x32_count = Range<0, 32>.Tout;
defvar x32_result_vector = "(" # !interleave(!foreach(i, x32_count, "temp"), ", ")# ")";

defvar x32_v64_types = WidenTypeForList<VFType<broadcast_ext_base_types, "_x32">.Tout, 2>.Tout;
OclBuiltinImpl sub_group_broadcast_ext_x32_v64 =
  OclBuiltinImpl<sub_group_broadcast_extended_x32, x32_v64_types, 0, broadcast_ext_code, [["$VSize", "2"], ["$Result", x32_result_vector]]>;

defvar x32_v96_types = WidenTypeForList<VFType<broadcast_ext_base_types, "_x32">.Tout, 3>.Tout;
OclBuiltinImpl sub_group_broadcast_ext_x32_v96 =
  OclBuiltinImpl<sub_group_broadcast_extended_x32, x32_v96_types, 0, broadcast_ext_code, [["$VSize", "3"], ["$Result", x32_result_vector]]>;

defvar x32_v128_types = WidenTypeForList<VFType<broadcast_ext_base_types, "_x32">.Tout, 4>.Tout;
OclBuiltinImpl sub_group_broadcast_ext_x32_v128 =
  OclBuiltinImpl<sub_group_broadcast_extended_x32, x32_v128_types, 0, broadcast_ext_code, [["$VSize", "4"], ["$Result", x32_result_vector]]>;

defvar x32_v256_types = WidenTypeForList<VFType<broadcast_ext_base_types, "_x32">.Tout, 8>.Tout;
OclBuiltinImpl sub_group_broadcast_ext_x32_v256 =
  OclBuiltinImpl<sub_group_broadcast_extended_x32, x32_v256_types, 0, broadcast_ext_code, [["$VSize", "8"], ["$Result", x32_result_vector]]>;

defvar x32_v512_types = WidenTypeForList<VFType<broadcast_ext_base_types, "_x32">.Tout, 16>.Tout;
OclBuiltinImpl sub_group_broadcast_ext_x32_v512 =
  OclBuiltinImpl<sub_group_broadcast_extended_x32, x32_v512_types, 0, broadcast_ext_code, [["$VSize", "16"], ["$Result", x32_result_vector]]>;

defvar x64_count = Range<0, 64>.Tout;
defvar x64_result_vector = "(" # !interleave(!foreach(i, x64_count, "temp"), ", ")# ")";

defvar x64_v128_types = WidenTypeForList<VFType<broadcast_ext_base_types, "_x64">.Tout, 2>.Tout;
OclBuiltinImpl sub_group_broadcast_ext_x64_v128 =
  OclBuiltinImpl<sub_group_broadcast_extended_x64, x64_v128_types, 0, broadcast_ext_code, [["$VSize", "2"], ["$Result", x64_result_vector]]>;

defvar x64_v192_types = WidenTypeForList<VFType<broadcast_ext_base_types, "_x64">.Tout, 3>.Tout;
OclBuiltinImpl sub_group_broadcast_ext_x64_v192 =
  OclBuiltinImpl<sub_group_broadcast_extended_x64, x64_v192_types, 0, broadcast_ext_code, [["$VSize", "3"], ["$Result", x64_result_vector]]>;

defvar x64_v256_types = WidenTypeForList<VFType<broadcast_ext_base_types, "_x64">.Tout, 4>.Tout;
OclBuiltinImpl sub_group_broadcast_ext_x64_v256 =
  OclBuiltinImpl<sub_group_broadcast_extended_x64, x64_v256_types, 0, broadcast_ext_code, [["$VSize", "4"], ["$Result", x64_result_vector]]>;

defvar x64_v512_types = WidenTypeForList<VFType<broadcast_ext_base_types, "_x64">.Tout, 8>.Tout;
OclBuiltinImpl sub_group_broadcast_ext_x64_v512 =
  OclBuiltinImpl<sub_group_broadcast_extended_x64, x64_v512_types, 0, broadcast_ext_code, [["$VSize", "8"], ["$Result", x64_result_vector]]>;

defvar x64_v1024_types = WidenTypeForList<VFType<broadcast_ext_base_types, "_x64">.Tout, 16>.Tout;
OclBuiltinImpl sub_group_broadcast_ext_x64_v1024 =
  OclBuiltinImpl<sub_group_broadcast_extended_x64, x64_v1024_types, 0, broadcast_ext_code, [["$VSize", "16"], ["$Result", x64_result_vector]]>;

OclBuiltinImpl sub_group_shuffle_avx512v48gentype = OclBuiltinImpl<sub_group_shuffle_avx512, [v48i8, v48u8, v48i16, v48u16, v48f32,
                                                                                              v48i32, v48u32, v48i64, v48u64], 0,
  [{
     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x16($Arg0VarName);
     $Arg0BaseType#16* arg0 = ($Arg0BaseType#16*)&$Arg0VarName;

     $Arg0BaseType#16 tmp1 = intel_sub_group_shuffle(arg0[0], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#16 tmp2 = intel_sub_group_shuffle(arg0[1], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#16 tmp3 = intel_sub_group_shuffle(arg0[2], $Arg1VarName, $Arg2VarName);

     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x3(($Arg0Type)(tmp1, tmp2, tmp3));
     return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_avx2v24gentype = OclBuiltinImpl<sub_group_shuffle_avx2, [v24i8, v24u8, v24i16, v24u16, v24f32,
                                                                                        v24i32, v24u32, v24i64, v24u64], 0,
  [{
     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x8($Arg0VarName);
     $Arg0BaseType#8* arg0 = ($Arg0BaseType#8*)&$Arg0VarName;

     $Arg0BaseType#8 tmp1 = intel_sub_group_shuffle(arg0[0], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#8 tmp2 = intel_sub_group_shuffle(arg0[1], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#8 tmp3 = intel_sub_group_shuffle(arg0[2], $Arg1VarName, $Arg2VarName);

     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x3(($Arg0Type)(tmp1, tmp2, tmp3));
     return $Arg0VarName;
  }]>;

// shuffle up
OclBuiltinImpl sub_group_shuffle_up_avx512v48gentype = OclBuiltinImpl<sub_group_shuffle_up_avx512, [v48i8, v48u8, v48i16, v48u16, v48f32,
                                                                                                    v48i32, v48u32, v48i64, v48u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x16($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x16($Arg1VarName);

    $Arg0BaseType#16* arg0 = ($Arg0BaseType#16*)&$Arg0VarName;
    $Arg0BaseType#16* arg1 = ($Arg0BaseType#16*)&$Arg1VarName;

    $Arg0BaseType#16 tmp1 = intel_sub_group_shuffle_up(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#16 tmp2 = intel_sub_group_shuffle_up(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#16 tmp3 = intel_sub_group_shuffle_up(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_up_avx2v24gentype = OclBuiltinImpl<sub_group_shuffle_up_avx2, [v24i8, v24u8, v24i16, v24u16, v24f32,
                                                                                                v24i32, v24u32, v24i64, v24u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x8($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x8($Arg1VarName);

    $Arg0BaseType#8* arg0 = ($Arg0BaseType#8*)&$Arg0VarName;
    $Arg0BaseType#8* arg1 = ($Arg0BaseType#8*)&$Arg1VarName;

    $Arg0BaseType#8 tmp1 = intel_sub_group_shuffle_up(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#8 tmp2 = intel_sub_group_shuffle_up(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#8 tmp3 = intel_sub_group_shuffle_up(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;

// shuffle down
OclBuiltinImpl sub_group_shuffle_down_avx512v48gentype = OclBuiltinImpl<sub_group_shuffle_down_avx512, [v48i8, v48u8, v48i16, v48u16, v48f32,
                                                                                                        v48i32, v48u32, v48i64, v48u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x16($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x16($Arg1VarName);

    $Arg0BaseType#16* arg0 = ($Arg0BaseType#16*)&$Arg0VarName;
    $Arg0BaseType#16* arg1 = ($Arg0BaseType#16*)&$Arg1VarName;

    $Arg0BaseType#16 tmp1 = intel_sub_group_shuffle_down(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#16 tmp2 = intel_sub_group_shuffle_down(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#16 tmp3 = intel_sub_group_shuffle_down(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_down_avx2v24gentype = OclBuiltinImpl<sub_group_shuffle_down_avx2, [v24i8, v24u8, v24i16, v24u16, v24f32,
                                                                                                    v24i32, v24u32, v24i64, v24u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x8($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x8($Arg1VarName);

    $Arg0BaseType#8* arg0 = ($Arg0BaseType#8*)&$Arg0VarName;
    $Arg0BaseType#8* arg1 = ($Arg0BaseType#8*)&$Arg1VarName;

    $Arg0BaseType#8 tmp1 = intel_sub_group_shuffle_down(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#8 tmp2 = intel_sub_group_shuffle_down(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#8 tmp3 = intel_sub_group_shuffle_down(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;


// shuffle xor

OclBuiltinImpl sub_group_shuffle_xor_avx512v48gentype = OclBuiltinImpl<sub_group_shuffle_xor_avx512, [v48i8, v48u8, v48i16, v48u16, v48f32,
                                                                                                      v48i32, v48u32, v48i64, v48u64], 0,
  [{
    uint16 indexes = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    indexes = indexes ^ $Arg1VarName;
    return intel_sub_group_shuffle($Arg0VarName, indexes, $Arg2VarName);
  }]>;

OclBuiltinImpl sub_group_shuffle_xor_avx2v24gentype = OclBuiltinImpl<sub_group_shuffle_xor_avx2, [v24i8, v24u8, v24i16, v24u16, v24f32,
                                                                                                  v24i32, v24u32, v24i64, v24u64], 0,
  [{
    uint8 indexes = {0, 1, 2, 3, 4, 5, 6, 7};
    indexes = indexes ^ $Arg1VarName;
    return intel_sub_group_shuffle($Arg0VarName, indexes, $Arg2VarName);
  }]>;
