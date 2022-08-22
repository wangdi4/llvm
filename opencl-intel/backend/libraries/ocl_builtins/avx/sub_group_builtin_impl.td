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
