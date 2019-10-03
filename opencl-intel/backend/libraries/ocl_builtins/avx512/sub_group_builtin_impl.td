//
// Ballot
//

OclBuiltinImpl intel_sub_group_ballot_avx512fvf16 = OclBuiltinImpl<intel_sub_group_ballot_vf16, [v16i32], 0,
  [{
    // This implementation only applicable as long as CPU supports sub group size <= 32,
    // anything above that would have to extend to the second element of uint4 return vector.
    __mmask16 mask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    uint16 res = (uint16)(uint)_mm512_mask_cmp_epi32_mask(mask, $Arg0VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    uint16 zero = (uint16) 0;
    return __builtin_shufflevector(res, zero, 0,  16, 16, 16,
                                              1,  16, 16, 16,
                                              2,  16, 16, 16,
                                              3,  16, 16, 16,
                                              4,  16, 16, 16,
                                              5,  16, 16, 16,
                                              6,  16, 16, 16,
                                              7,  16, 16, 16,
                                              8,  16, 16, 16,
                                              9,  16, 16, 16,
                                              10, 16, 16, 16,
                                              11, 16, 16, 16,
                                              12, 16, 16, 16,
                                              13, 16, 16, 16,
                                              14, 16, 16, 16,
                                              15, 16, 16, 16);
  }]>;

OclBuiltinImpl intel_sub_group_ballot_avx512fvf8 = OclBuiltinImpl<intel_sub_group_ballot_vf8, [v8i32], 0,
  [{
     // This implementation only applicable as long as CPU supports sub group size <= 32,
     // anything above that would have to extend to the second element of uint4 return vector.
     __mmask8 mask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
     uint8 res  = (uint8)(uint)_mm256_mask_cmp_epi32_mask(mask, $Arg0VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
     uint8 zero = (uint8) 0;
     return __builtin_shufflevector(res, zero, 0, 8, 8, 8,
                                               1, 8, 8, 8,
                                               2, 8, 8, 8,
                                               3, 8, 8, 8,
                                               4, 8, 8, 8,
                                               5, 8, 8, 8,
                                               6, 8, 8, 8,
                                               7, 8, 8, 8);
  }]>;

OclBuiltinImpl intel_sub_group_ballot_avx512fvf4 = OclBuiltinImpl<intel_sub_group_ballot_vf4, [v4i32], 0,
  [{
    // This implementation only applicable as long as CPU supports sub group size <= 32,
    // anything above that would have to extend to the second element of uint4 return vector.
    uint8 temp_mask;
    temp_mask.hi = 0;
    temp_mask.lo = $Arg1VarName;
    int8 temp_pred;
    temp_pred.lo = $Arg0VarName;
    __mmask8 mask = _mm256_cmp_epi32_mask(temp_mask, _mm256_setzero_ps(), _MM_CMPINT_NE);
    uint8 res  = (uint8)(uint)_mm256_mask_cmp_epi32_mask(mask, temp_pred, _mm256_setzero_ps(), _MM_CMPINT_NE);
    uint4 zero = (uint4) 0;
    return __builtin_shufflevector(res.lo, zero, 0, 4, 4, 4,
                                                 1, 4, 4, 4,
                                                 2, 4, 4, 4,
                                                 3, 4, 4, 4);
  }]>;

//
// Broadcast
//

OclBuiltinImpl sub_group_broadcast_vector = OclBuiltinImpl<sub_group_broadcast_vec, [v4i32, v8i32, v16i32,
                                                                                     v4u32, v8u32, v16u32,
                                                                                     v4i64, v8i64, v16i64,
                                                                                     v4u64, v8u64, v16u64,
                                                                                     v4f32, v8f32, v16f32,
                                                                                     v4f64, v8f64, v16f64],  0,
  [{
    return $Arg0VarName[$Arg1VarName];
  }]>;

//
// Reductions
//

// any/all begin
// VF 16 begin

OclBuiltinImpl sub_group_all_avx512fiu32 = OclBuiltinImpl<sub_group_all_vec, [ v16i32 ], 0,
  [{
    __mmask16 mask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __mmask16 test = _mm512_mask_cmp_epi32_mask(mask, $Arg0VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    unsigned short res = _mm512_kxnor(mask, test);
    return (int16)(res == 65535);
  }]>;

OclBuiltinImpl sub_group_any_avx512fiu32 = OclBuiltinImpl<sub_group_any_vec, [ v16i32 ], 0,
  [{
    __mmask16 mask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __mmask16 test = _mm512_mask_cmp_epi32_mask(mask, $Arg0VarName, _mm512_setzero_epi32(), _MM_CMPINT_EQ);
    unsigned short res = _mm512_kxor(mask, test);
    return res != 0;
  }]>;

// VF 16 end
// VF 8 begin

OclBuiltinImpl sub_group_all_avx512fiu32_vf8 = OclBuiltinImpl<sub_group_all_vec, [ v8i32 ], 0,
  [{
    __mmask8 mask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 test = _mm256_mask_cmp_epi32_mask(mask, $Arg0VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    unsigned short res = _kxnor_mask8(mask, test);
    return res == 255;
  }]>;

OclBuiltinImpl sub_group_any_avx512fiu32_vf8 = OclBuiltinImpl<sub_group_any_vec, [ v8i32 ], 0,
  [{
    __mmask8 mask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 test = _mm256_mask_cmp_epi32_mask(mask, $Arg0VarName, _mm256_setzero_ps(), _MM_CMPINT_EQ);
    unsigned short res = _kxor_mask8(mask, test);
    return res != 255;
  }]>;

// VF 8 end
// VF 4 begin

OclBuiltinImpl sub_group_all_avx512fiu32_vf4 = OclBuiltinImpl<sub_group_all_vec, [ v4i32 ], 0,
  [{
    uint8 temp_mask;
    temp_mask.hi = 0;
    temp_mask.lo = $Arg1VarName;
    int8 temp_pred;
    temp_pred.lo = $Arg0VarName;
    return sub_group_all(temp_pred, temp_mask).lo;
  }]>;

OclBuiltinImpl sub_group_any_avx512fiu32_vf4 = OclBuiltinImpl<sub_group_any_vec, [ v4i32 ], 0,
  [{
    uint8 temp_mask;
    temp_mask.hi = 0;
    temp_mask.lo = $Arg1VarName;
    int8 temp_pred;
    temp_pred.lo = $Arg0VarName;
    return sub_group_any(temp_pred, temp_mask).lo;
  }]>;

// VF 4 end
// any/all end

// reduce [add|min|max]
// add begin

// VF 16 begin
OclBuiltinImpl sub_group_reduce_add_avx512fiu32 = OclBuiltinImpl<sub_group_reduce_add_vec, [ v16i32, v16u32 ], 0,
  [{
    __mmask16 mask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_add_epi32(mask, (__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx512ff32 = OclBuiltinImpl<sub_group_reduce_add_vec, [ v16f32 ], 0,
  [{
    __mmask16 mask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_add_ps(mask, (__m512)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx512fv16iu64 = OclBuiltinImpl<sub_group_reduce_add_vec, [ v16i64, v16u64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);
    $Arg0BaseType#8 lo = $Arg0VarName.lo;
    $Arg0BaseType#8 hi = $Arg0VarName.hi;
    $Arg0BaseType lores = _mm512_mask_reduce_add_epi64(mask_lo, *(__m512i*)&lo);
    $Arg0BaseType hires = _mm512_mask_reduce_add_epi64(mask_hi, *(__m512i*)&hi);
    $Arg0BaseType scalar_res = lores + hires;
    $Arg0Type res = ($Arg0Type) scalar_res;
    return res;
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx512fv16f64 = OclBuiltinImpl<sub_group_reduce_add_vec, [ v16f64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);
    $Arg0BaseType#8 lo = $Arg0VarName.lo;
    $Arg0BaseType#8 hi = $Arg0VarName.hi;
    $Arg0BaseType lores = _mm512_mask_reduce_add_pd(mask_lo, *(__m512d*)&lo);
    $Arg0BaseType hires = _mm512_mask_reduce_add_pd(mask_hi, *(__m512d*)&hi);
    $Arg0BaseType scalar_res = lores + hires;
    $Arg0Type res = ($Arg0Type) scalar_res;
    return res;
  }]>;

// VF 16 end
// VF 8 begin
OclBuiltinImpl sub_group_reduce_add_avx512fv8f64 = OclBuiltinImpl<sub_group_reduce_add_vec, [ v8f64 ], 0,
  [{
    __mmask8 mask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_add_pd(mask, (__m512d)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx512fv8f32 = OclBuiltinImpl<sub_group_reduce_add_vec, [ v8f32 ], 0,
  [{
    __mmask16 mask = (__mmask16)_mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    float16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_add_ps(mask, (__m512)num);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx512fv8iu64 = OclBuiltinImpl<sub_group_reduce_add_vec, [ v8i64, v8u64 ], 0,
  [{
    __mmask8 mask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_add_epi64(mask, (__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx512fv8iu32 = OclBuiltinImpl<sub_group_reduce_add_vec, [ v8i32, v8u32 ], 0,
  [{
    __mmask16 mask = (__mmask16)_mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    $Arg0BaseType#16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_add_epi32(mask, (__m512i)num);
  }]>;

// VF 8 end
// VF 4 begin

OclBuiltinImpl sub_group_reduce_add_avx512fv4 = OclBuiltinImpl<sub_group_reduce_add_vec, [v4f64, v4f32, v4i64, v4u64, v4i32, v4u32], 0,
  [{
    uint8 temp_mask;
    temp_mask.hi = 0;
    temp_mask.lo = $Arg1VarName;
    $Arg0BaseType#8 temp_pred;
    temp_pred.lo = $Arg0VarName;
    return sub_group_reduce_add(temp_pred, temp_mask).lo;
  }]>;

//VF 4 end

// add end
// min begin

// VF 16 begin
OclBuiltinImpl sub_group_reduce_min_avx512fi32 = OclBuiltinImpl<sub_group_reduce_min_vec, [ v16i32 ], 0,
  [{
    __mmask16 mask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_min_epi32(mask, (__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fu32 = OclBuiltinImpl<sub_group_reduce_min_vec, [ v16u32 ], 0,
  [{
    __mmask16 mask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_min_epu32(mask, (__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fv16i64 = OclBuiltinImpl<sub_group_reduce_min_vec, [ v16i64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);
    $Arg0BaseType#8 lo = $Arg0VarName.lo;
    $Arg0BaseType#8 hi = $Arg0VarName.hi;
    $Arg0BaseType lores = _mm512_mask_reduce_min_epi64(mask_lo, *((__m512i*)&lo));
    $Arg0BaseType hires = _mm512_mask_reduce_min_epi64(mask_hi, *((__m512i*)&hi));
    $Arg0BaseType scalar_res = lores < hires ? lores : hires;
    $Arg0Type res = ($Arg0Type) scalar_res;
    return res;
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512ff32 = OclBuiltinImpl<sub_group_reduce_min_vec, [ v16f32 ], 0,
  [{
    __mmask16 mask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_min_ps(mask, (__m512)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fv16u64 = OclBuiltinImpl<sub_group_reduce_min_vec, [ v16u64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);
    $Arg0BaseType#8 lo = $Arg0VarName.lo;
    $Arg0BaseType#8 hi = $Arg0VarName.hi;
    $Arg0BaseType lores = _mm512_mask_reduce_min_epu64(mask_lo, *((__m512i*)&lo));
    $Arg0BaseType hires = _mm512_mask_reduce_min_epu64(mask_hi, *((__m512i*)&hi));
    $Arg0BaseType scalar_res = lores < hires ? lores : hires;
    $Arg0Type res = ($Arg0Type) scalar_res;
    return res;
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fv16f64 = OclBuiltinImpl<sub_group_reduce_min_vec, [ v16f64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);
    $Arg0BaseType#8 lo = $Arg0VarName.lo;
    $Arg0BaseType#8 hi = $Arg0VarName.hi;
    $Arg0BaseType lores = _mm512_mask_reduce_min_pd(mask_lo, *((__m512d*)&lo));
    $Arg0BaseType hires = _mm512_mask_reduce_min_pd(mask_hi, *((__m512d*)&hi));
    $Arg0BaseType scalar_res = lores < hires ? lores : hires;
    $Arg0Type res = ($Arg0Type) scalar_res;
    return res;
  }]>;

// VF 16 end
// VF 8 begin

OclBuiltinImpl sub_group_reduce_min_avx512fi32vf8 = OclBuiltinImpl<sub_group_reduce_min_vec, [ v8i32 ], 0,
  [{
    __mmask16 mask = (__mmask16)_mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    int16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_min_epi32(mask, (__m512i)num);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fu32vf8 = OclBuiltinImpl<sub_group_reduce_min_vec, [ v8u32 ], 0,
  [{
    __mmask16 mask = (__mmask16)_mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    uint16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_min_epu32(mask, (__m512i)num);
  }]>;


OclBuiltinImpl sub_group_reduce_min_avx512ff32iv8 = OclBuiltinImpl<sub_group_reduce_min_vec, [ v8f32 ], 0,
  [{
    __mmask16 mask = (__mmask16)_mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    float16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_min_ps(mask, (__m512)num);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fv8i64 = OclBuiltinImpl<sub_group_reduce_min_vec, [ v8i64 ], 0,
  [{
    __mmask8 mask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_min_epi64(mask, (__m512i)$Arg0VarName);
  }]>;


OclBuiltinImpl sub_group_reduce_min_avx512fv8u64 = OclBuiltinImpl<sub_group_reduce_min_vec, [ v8u64 ], 0,
  [{
    __mmask8 mask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_min_epu64(mask, (__m512i)$Arg0VarName);
  }]>;


OclBuiltinImpl sub_group_reduce_min_avx512fv8f64 = OclBuiltinImpl<sub_group_reduce_min_vec, [ v8f64 ], 0,
  [{
    __mmask8 mask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_min_pd(mask, (__m512d)$Arg0VarName);
  }]>;

// VF 8 end
// VF 4 begin

OclBuiltinImpl sub_group_reduce_min_avx512fv4 = OclBuiltinImpl<sub_group_reduce_min_vec, [ v4i32, v4u32, v4f32, v4i64, v4u64, v4f64 ], 0,
  [{
    uint8 temp_mask;
    temp_mask.hi = 0;
    temp_mask.lo = $Arg1VarName;
    $Arg0BaseType#8 temp_pred;
    temp_pred.lo = $Arg0VarName;
    return sub_group_reduce_min(temp_pred, temp_mask).lo;
  }]>;

// VF 4 end

// min end
// max begin

// VF 16 begin

OclBuiltinImpl sub_group_reduce_max_avx512fi32 = OclBuiltinImpl<sub_group_reduce_max_vec, [ v16i32 ], 0,
  [{
    __mmask16 mask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_max_epi32(mask, (__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512fu32 = OclBuiltinImpl<sub_group_reduce_max_vec, [ v16u32 ], 0,
  [{
    __mmask16 mask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_max_epu32(mask, (__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512ff32 = OclBuiltinImpl<sub_group_reduce_max_vec, [ v16f32 ], 0,
  [{
    __mmask16 mask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_max_ps(mask, (__m512)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512fv16i64 = OclBuiltinImpl<sub_group_reduce_max_vec, [ v16i64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);
    $Arg0BaseType#8 lo = $Arg0VarName.lo;
    $Arg0BaseType#8 hi = $Arg0VarName.hi;
    $Arg0BaseType lores = _mm512_mask_reduce_max_epi64(mask_lo, *((__m512i*)&lo));
    $Arg0BaseType hires = _mm512_mask_reduce_max_epi64(mask_hi, *((__m512i*)&hi));
    $Arg0BaseType scalar_res = lores > hires ? lores : hires;
    $Arg0Type res = ($Arg0Type) scalar_res;
    return res;
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512fv16u64 = OclBuiltinImpl<sub_group_reduce_max_vec, [ v16u64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);
    $Arg0BaseType#8 lo = $Arg0VarName.lo;
    $Arg0BaseType#8 hi = $Arg0VarName.hi;
    $Arg0BaseType lores = _mm512_mask_reduce_max_epu64(mask_lo, *((__m512i*)&lo));
    $Arg0BaseType hires = _mm512_mask_reduce_max_epu64(mask_hi, *((__m512i*)&hi));
    $Arg0BaseType scalar_res = lores > hires ? lores : hires;
    $Arg0Type res = ($Arg0Type) scalar_res;
    return res;
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512fv16f64 = OclBuiltinImpl<sub_group_reduce_max_vec, [ v16f64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);
    $Arg0BaseType#8 lo = $Arg0VarName.lo;
    $Arg0BaseType#8 hi = $Arg0VarName.hi;
    $Arg0BaseType lores = _mm512_mask_reduce_max_pd(mask_lo, *((__m512d*)&lo));
    $Arg0BaseType hires = _mm512_mask_reduce_max_pd(mask_hi, *((__m512d*)&hi));
    $Arg0BaseType scalar_res = lores > hires ? lores : hires;
    $Arg0Type res = ($Arg0Type) scalar_res;
    return res;
  }]>;

// VF 16 end
// VF 8 begin

OclBuiltinImpl sub_group_reduce_max_avx512fi32vf8 = OclBuiltinImpl<sub_group_reduce_max_vec, [ v8i32 ], 0,
  [{
    __mmask16 mask = (__mmask16) _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    int16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_max_epi32(mask, (__m512i)num);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512fu32vf8 = OclBuiltinImpl<sub_group_reduce_max_vec, [ v8u32 ], 0,
  [{
    __mmask16 mask = (__mmask16) _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    uint16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_max_epu32(mask, (__m512i)num);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512ff32vf8 = OclBuiltinImpl<sub_group_reduce_max_vec, [ v8f32 ], 0,
  [{
    __mmask16 mask = (__mmask16) _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    float16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_max_ps(mask, (__m512)num);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512fv8i64 = OclBuiltinImpl<sub_group_reduce_max_vec, [ v8i64 ], 0,
  [{
    __mmask8 mask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_max_epi64(mask, (__m512i)$Arg0VarName);
  }]>;


OclBuiltinImpl sub_group_reduce_max_avx512fv8u64 = OclBuiltinImpl<sub_group_reduce_max_vec, [ v8u64 ], 0,
  [{
    __mmask8 mask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_max_epu64(mask, (__m512i)$Arg0VarName);
  }]>;


OclBuiltinImpl sub_group_reduce_max_avx512fv8f64 = OclBuiltinImpl<sub_group_reduce_max_vec, [ v8f64 ], 0,
  [{
    __mmask8 mask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    return _mm512_mask_reduce_max_pd(mask, (__m512d)$Arg0VarName);
  }]>;

// VF 8 end
// VF 4 begin

OclBuiltinImpl sub_group_reduce_max_avx512fv4 = OclBuiltinImpl<sub_group_reduce_max_vec, [ v4i32, v4u32, v4f32, v4i64, v4u64, v4f64 ], 0,
  [{
    uint8 temp_mask;
    temp_mask.hi = 0;
    temp_mask.lo = $Arg1VarName;
    $Arg0BaseType#8 temp_pred;
    temp_pred.lo = $Arg0VarName;
    return sub_group_reduce_max(temp_pred, temp_mask).lo;
  }]>;

// VF 4 end

// max end

// inclusive [add|min|max]
// inclusive scan add begin

// VF 16 begin
OclBuiltinImpl sub_group_scan_inclusive_add_avx512fiu32 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [ v16i32, v16u32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_maskz_mov_epi32(vmask, (__m512i) $Arg0VarName);

    unsigned short mask = 0x01;
    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);

    __v16si rotated = *((__v16si *)&masked_data);
    __v16si res = rotated;

    #pragma clang unroll(full)
    for (int i = 0; i < 15; i++) {
      rotated = (__v16si)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
      mask <<= 1;
      res = (__v16si)_mm512_mask_add_epi32((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx512ff32 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [ v16f32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512 masked_data = _mm512_maskz_mov_ps(vmask, $Arg0VarName);

    unsigned short mask = 0x01;
    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);

    __v16sf rotated = *((__v16sf *)&masked_data);
    __v16sf res = rotated;

    #pragma clang unroll(full)
    for (int i = 0; i < 15; i++) {
      rotated = (__v16sf)_mm512_permutexvar_ps(rotate_mask, (__m512)res);
      mask <<= 1;
      res = (__v16sf)_mm512_mask_add_ps((__m512)res, mask, (__m512)res, (__m512)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx512fv16iu64 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [ v16i64, v16u64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    __m512i lo = _mm512_maskz_mov_epi64(mask_lo, (__m512i) $Arg0VarName.lo);
    __m512i hi = _mm512_maskz_mov_epi64(mask_hi, (__m512i) $Arg0VarName.hi);

    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    ulong16 res;

    __v8du rotated;
    __v8du res_lo = *(__v8du*)(&lo);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_lo);
        mask <<= 1;
        res_lo = (__v8du)_mm512_mask_add_epi64((__m512i)res_lo, mask, (__m512i)res_lo, (__m512i)rotated);
    }

    __v8du res_hi = *(__v8du*)((ulong*)&hi);
    res_hi[0] += res_lo[7];
    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_hi);
        mask <<= 1;
        res_hi = (__v8du)_mm512_mask_add_epi64((__m512i)res_hi, mask, (__m512i)res_hi, (__m512i)rotated);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)((ulong*)&res + 8), (__m512)res_hi);

    return *(($ReturnType *)&res);
  }]>;


OclBuiltinImpl sub_group_scan_inclusive_add_avx512fv16ff64 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [ v16f64 ], 0,
  [{
    $Arg0BaseType#8 lo = $Arg0VarName.lo;
    $Arg0BaseType#8 hi = $Arg0VarName.hi;

    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    lo = _mm512_maskz_mov_pd(mask_lo, lo);
    hi = _mm512_maskz_mov_pd(mask_hi, hi);

    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    double16 res;

    __v8df rotated;
    __v8df res_lo = *(__v8df*)(&lo);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res_lo);
        mask <<= 1;
        res_lo = (__v8df)_mm512_mask_add_pd((__m512d)res_lo, mask, (__m512d)res_lo, (__m512d)rotated);
    }

    __v8df res_hi = *(__v8df*)((double*)&hi);
    res_hi[0] += res_lo[7];
    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res_hi);
        mask <<= 1;
        res_hi = (__v8df)_mm512_mask_add_pd((__m512d)res_hi, mask, (__m512d)res_hi, (__m512d)rotated);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)((double*)&res + 8), (__m512)res_hi);

    return *(($ReturnType *)&res);
  }]>;

// VF 16 end
// VF 8 begin

OclBuiltinImpl sub_group_scan_inclusive_add_avx512fvf8iu32 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [ v8i32, v8u32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256i masked_data = _mm256_maskz_mov_epi32(vmask, (__m256i)$Arg0VarName);

    unsigned short mask = 0x01;
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);

    __v8si rotated = *((__v8si *)&masked_data);
    __v8si res = rotated;

    #pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
      rotated = (__v8si)_mm256_permutexvar_epi32(rotate_mask, (__m256i)res);
      mask <<= 1;
      res = (__v8si)_mm256_mask_add_epi32((__m256i)res, mask, (__m256i)res, (__m256i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx512fvf8f32 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [ v8f32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256 masked_data = _mm256_maskz_mov_ps(vmask, (__m256)$Arg0VarName);

    unsigned short mask = 0x01;
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);

    __v8sf rotated = *((__v8sf *)&masked_data);
    __v8sf res = rotated;

    #pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
      rotated = (__v8sf)_mm256_permutexvar_ps(rotate_mask, (__m256)res);
      mask <<= 1;
      res = (__v8sf)_mm256_mask_add_ps((__m256)res, mask, (__m256)res, (__m256)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx512fvf8iu64 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [ v8i64, v8u64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_maskz_mov_epi64(vmask, (__m512i)$Arg0VarName);

    unsigned char mask = 0x01;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    __v8du rotated = *((__v8du *)&masked_data);
    __v8du res = rotated;

    #pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
      rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res);
      mask <<= 1;
      res = (__v8du)_mm512_mask_add_epi64((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx512fvf8f64 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [ v8f64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512d masked_data = _mm512_maskz_mov_pd(vmask, (__m512d)$Arg0VarName);

    unsigned char mask = 0x01;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    __v8df rotated = *((__v8df *)&masked_data);
    __v8df res = rotated;

    #pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
      rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res);
      mask <<= 1;
      res = (__v8df)_mm512_mask_add_pd((__m512d)res, mask, (__m512d)res, (__m512d)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

// VF 8 end
// VF 4 begin

OclBuiltinImpl sub_group_scan_inclusive_add_avx512fvf4 = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, [ v4i32, v4u32, v4f32, v4i64, v4u64, v4f64 ], 0,
  [{
    uint8 temp_mask;
    temp_mask.hi = 0;
    temp_mask.lo = $Arg1VarName;
    $Arg0BaseType#8 temp_pred;
    temp_pred.lo = $Arg0VarName;
    return sub_group_scan_inclusive_add(temp_pred, temp_mask).lo;
  }]>;

// VF 4 end

// inclusive add end
// inclusive min begin

// VF 16 begin
OclBuiltinImpl sub_group_scan_inclusive_min_avx512fi32 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [ v16i32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_mask_mov_epi32(_mm512_set1_epi32(INT_MAX), vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16si res = *((__v16si *)&masked_data);
    __v16si rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 15; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v16si)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
        res = (__v16si)_mm512_mask_min_epi32((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fu32 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [ v16u32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_mask_mov_epi32(_mm512_set1_epi32(UINT_MAX), vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16su res = *((__v16su *)&masked_data);
    __v16su rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 15; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v16su)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
        res = (__v16su)_mm512_mask_min_epu32((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512ff32vf16 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [ v16f32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512 masked_data = _mm512_mask_mov_ps(_mm512_set1_ps(INFINITY), vmask, (__m512)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16sf res = *((__v16sf *)&masked_data);
    __v16sf rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 15; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v16sf)_mm512_permutexvar_ps(rotate_mask, (__m512)res);
        res = (__v16sf)_mm512_mask_min_ps((__m512)res, mask, (__m512)res, (__m512)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fv16ff64 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [ v16f64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    __m512d lo = _mm512_mask_mov_pd(_mm512_set1_pd(INFINITY), mask_lo, (__m512d)$Arg0VarName.lo);
    __m512d hi = _mm512_mask_mov_pd(_mm512_set1_pd(INFINITY), mask_hi, (__m512d)$Arg0VarName.hi);

    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    double16 res;

    __v8df rotated;
    __v8df res_lo = *(__v8df*)(&lo);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res_lo);
        mask <<= 1;
        res_lo = (__v8df)_mm512_mask_min_pd((__m512d)res_lo, mask, (__m512d)res_lo, (__m512d)rotated);
    }

    __v8df res_hi = *(__v8df*)((double*)&hi);
    res_hi[0] = res_lo[7] < res_hi[0] ? res_lo[7] : res_hi[0];
    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res_hi);
        mask <<= 1;
        res_hi = (__v8df)_mm512_mask_min_pd((__m512d)res_hi, mask, (__m512d)res_hi, (__m512d)rotated);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)((double*)&res + 8), (__m512)res_hi);

    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fv16u64 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [ v16u64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    __m512i lo = _mm512_mask_mov_epi64(_mm512_set1_epi64(ULONG_MAX), mask_lo, (__m512i)$Arg0VarName.lo);
    __m512i hi = _mm512_mask_mov_epi64(_mm512_set1_epi64(ULONG_MAX), mask_hi, (__m512i)$Arg0VarName.hi);

    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    ulong16 res;

    __v8du rotated;
    __v8du res_lo = *(__v8du*)(&lo);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_lo);
        mask <<= 1;
        res_lo = (__v8du)_mm512_mask_min_epu64((__m512i)res_lo, mask, (__m512i)res_lo, (__m512i)rotated);
    }

    __v8du res_hi = *(__v8du*)((long*)&hi);
    res_hi[0] = res_lo[7] < res_hi[0] ? res_lo[7] : res_hi[0];
    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_hi);
        mask <<= 1;
        res_hi = (__v8du)_mm512_mask_min_epu64((__m512i)res_hi, mask, (__m512i)res_hi, (__m512i)rotated);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)((double*)&res + 8), (__m512)res_hi);

    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fv16i64 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [ v16i64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    __m512i lo = _mm512_mask_mov_epi64(_mm512_set1_epi64(LONG_MAX), mask_lo, (__m512i)$Arg0VarName.lo);
    __m512i hi = _mm512_mask_mov_epi64(_mm512_set1_epi64(LONG_MAX), mask_hi, (__m512i)$Arg0VarName.hi);

    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    ulong16 res;

    __v8di rotated;
    __v8di res_lo = *(__v8di*)(&lo);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_lo);
        mask <<= 1;
        res_lo = (__v8di)_mm512_mask_min_epi64((__m512i)res_lo, mask, (__m512i)res_lo, (__m512i)rotated);
    }

    __v8di res_hi = *(__v8di*)((long*)&hi);
    res_hi[0] = res_lo[7] < res_hi[0] ? res_lo[7] : res_hi[0];
    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_hi);
        mask <<= 1;
        res_hi = (__v8di)_mm512_mask_min_epi64((__m512i)res_hi, mask, (__m512i)res_hi, (__m512i)rotated);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)((double*)&res + 8), (__m512)res_hi);

    return *(($ReturnType *)&res);
  }]>;

// VF 16 end
// VF 8 begin

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fvf8i32 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [ v8i32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256i masked_data = _mm256_mask_mov_epi32(_mm256_set1_epi32(INT_MAX), vmask, (__m256i)$Arg0VarName);

    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8si res = *((__v8si *)&masked_data);
    __v8si rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8si)_mm256_permutexvar_epi32(rotate_mask, (__m256i)res);
        res = (__v8si)_mm256_mask_min_epi32((__m256i)res, mask, (__m256i)res, (__m256i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fvf8u32 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [ v8u32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256i masked_data = _mm256_mask_mov_epi32(_mm256_set1_epi32(UINT_MAX), vmask, (__m256i)$Arg0VarName);

    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8su res = *((__v8su *)&masked_data);
    __v8su rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8su)_mm256_permutexvar_epi32(rotate_mask, (__m256i)res);
        res = (__v8su)_mm256_mask_min_epu32((__m256i)res, mask, (__m256i)res, (__m256i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;


OclBuiltinImpl sub_group_scan_inclusive_min_avx512fvf8f32 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [ v8f32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256 masked_data = _mm256_mask_mov_ps(_mm256_set1_ps(INFINITY), vmask, (__m256)$Arg0VarName);

    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8sf res = *((__v8sf *)&masked_data);
    __v8sf rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8sf)_mm256_permutexvar_ps(rotate_mask, (__m256)res);
        res = (__v8sf)_mm256_mask_min_ps((__m256)res, mask, (__m256)res, (__m256)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fv8u64 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [ v8u64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_mask_mov_epi64(_mm512_set1_epi64(ULONG_MAX), vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8du res = *((__v8du *)&masked_data);
    __v8du rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res);
        res = (__v8du)_mm512_mask_min_epu64((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fv8i64 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [ v8i64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_mask_mov_epi64(_mm512_set1_epi64(LONG_MAX), vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8di res = *((__v8di *)&masked_data);
    __v8di rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8di)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
        res = (__v8di)_mm512_mask_min_epu64((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512v8f64 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [ v8f64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512d masked_data = _mm512_mask_mov_pd(_mm512_set1_pd(INFINITY), vmask, (__m512d)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8df res = *((__v8df *)&masked_data);
    __v8df rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res);
        res = (__v8df)_mm512_mask_min_pd((__m512d)res, mask, (__m512d)res, (__m512d)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

// VF 8 end
// VF 4 begin

OclBuiltinImpl sub_group_scan_inclusive_min_avx512v4 = OclBuiltinImpl<sub_group_scan_inclusive_min_vec, [ v4i32, v4u32, v4f32, v4i64, v4u64, v4f64 ], 0,
  [{
    uint8 temp_mask;
    temp_mask.hi = 0;
    temp_mask.lo = $Arg1VarName;
    $Arg0BaseType#8 temp_pred;
    temp_pred.lo = $Arg0VarName;
    return sub_group_scan_inclusive_min(temp_pred, temp_mask).lo;
  }]>;

// VF 4 end

// inclusive min end
// inclusive max begin

// VF 16 begin
OclBuiltinImpl sub_group_scan_inclusive_max_avx512i32 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [ v16i32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_mask_mov_epi32(_mm512_set1_epi32(INT_MIN), vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16si res = *((__v16si *)&masked_data);
    __v16si rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 15; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v16si)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
        res = (__v16si)_mm512_mask_max_epi32((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return res;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512u32 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [ v16u32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_maskz_mov_epi32(vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16su res = *((__v16su *)&masked_data);
    __v16su rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 15; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v16su)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
        res = (__v16su)_mm512_mask_max_epu32((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return res;
  }]>;


OclBuiltinImpl sub_group_scan_inclusive_max_avx512f32 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [ v16f32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512 masked_data = _mm512_mask_mov_ps(_mm512_set1_ps(-INFINITY), vmask, (__m512)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16sf res = *((__v16sf *)&masked_data);
    __v16sf rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 15; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v16sf)_mm512_permutexvar_ps(rotate_mask, (__m512)res);
        res = (__v16sf)_mm512_mask_max_ps((__m512)res, mask, (__m512)res, (__m512)rotated);
    }
    return res;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512fv16f64 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [ v16f64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    __m512d lo = _mm512_mask_mov_pd(_mm512_set1_pd(-INFINITY), mask_lo, (__m512d) $Arg0VarName.lo);
    __m512d hi = _mm512_mask_mov_pd(_mm512_set1_pd(-INFINITY), mask_hi, (__m512d) $Arg0VarName.hi);

    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    double16 res;

    __v8df rotated;
    __v8df res_lo = *(__v8df*)(&lo);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res_lo);
        mask <<= 1;
        res_lo = (__v8df)_mm512_mask_max_pd((__m512d)res_lo, mask, (__m512d)res_lo, (__m512d)rotated);
    }

    __v8df res_hi = *(__v8df*)((double*)&hi);
    res_hi[0] = res_lo[7] > res_hi[0] ? res_lo[7] : res_hi[0];
    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res_hi);
        mask <<= 1;
        res_hi = (__v8df)_mm512_mask_max_pd((__m512d)res_hi, mask, (__m512d)res_hi, (__m512d)rotated);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)((double*)&res + 8), (__m512)res_hi);

    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512fv16u64 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [ v16u64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    __m512i lo = _mm512_maskz_mov_epi64(mask_lo, (__m512i)$Arg0VarName.lo);
    __m512i hi = _mm512_maskz_mov_epi64(mask_hi, (__m512i)$Arg0VarName.hi);

    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    ulong16 res;

    __v8du rotated;
    __v8du res_lo = *(__v8du*)(&lo);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_lo);
        mask <<= 1;
        res_lo = (__v8du)_mm512_mask_max_epu64((__m512i)res_lo, mask, (__m512i)res_lo, (__m512i)rotated);
    }

    __v8du res_hi = *(__v8du*)((long*)&hi);
    res_hi[0] = res_lo[7] > res_hi[0] ? res_lo[7] : res_hi[0];
    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_hi);
        mask <<= 1;
        res_hi = (__v8du)_mm512_mask_max_epu64((__m512i)res_hi, mask, (__m512i)res_hi, (__m512i)rotated);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)((double*)&res + 8), (__m512)res_hi);

    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512fv16i64 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [ v16i64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    __m512i lo = _mm512_mask_mov_epi64(_mm512_set1_epi64(LONG_MIN), mask_lo, (__m512i)$Arg0VarName.lo);
    __m512i hi = _mm512_mask_mov_epi64(_mm512_set1_epi64(LONG_MIN), mask_hi, (__m512i)$Arg0VarName.hi);

    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    ulong16 res;

    __v8di rotated;
    __v8di res_lo = *(__v8di*)(&lo);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_lo);
        mask <<= 1;
        res_lo = (__v8di)_mm512_mask_max_epi64((__m512i)res_lo, mask, (__m512i)res_lo, (__m512i)rotated);
    }

    __v8di res_hi = *(__v8di*)((long*)&hi);
    res_hi[0] = res_lo[7] > res_hi[0] ? res_lo[7] : res_hi[0];
    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_hi);
        mask <<= 1;
        res_hi = (__v8di)_mm512_mask_max_epi64((__m512i)res_hi, mask, (__m512i)res_hi, (__m512i)rotated);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)((double*)&res + 8), (__m512)res_hi);

    return *(($ReturnType *)&res);
  }]>;

// VF 16 end
// VF 8 begin
OclBuiltinImpl sub_group_scan_inclusive_max_avx512fvf8i32 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [ v8i32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256i masked_data = _mm256_mask_mov_epi32(_mm256_set1_epi32(INT_MIN), vmask, (__m256i)$Arg0VarName);

    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8si res = *((__v8si *)&masked_data);
    __v8si rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8si)_mm256_permutexvar_epi32(rotate_mask, (__m256i)res);
        res = (__v8si)_mm256_mask_max_epi32((__m256i)res, mask, (__m256i)res, (__m256i)rotated);
    }
    return res;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512fvf8u32 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [ v8u32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256i masked_data = _mm256_maskz_mov_epi32(vmask, (__m256i)$Arg0VarName);

    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8su res = *((__v8su *)&masked_data);
    __v8su rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8su)_mm256_permutexvar_epi32(rotate_mask, (__m256i)res);
        res = (__v8su)_mm256_mask_max_epu32((__m256i)res, mask, (__m256i)res, (__m256i)rotated);
    }
    return res;
  }]>;


OclBuiltinImpl sub_group_scan_inclusive_max_avx512fvf832 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [ v8f32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256 masked_data = _mm256_mask_mov_ps(_mm256_set1_ps(-INFINITY), vmask, (__m256)$Arg0VarName);

    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8sf res = *((__v8sf *)&masked_data);
    __v8sf rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8sf)_mm256_permutexvar_ps(rotate_mask, (__m256)res);
        res = (__v8sf)_mm256_mask_max_ps((__m256)res, mask, (__m256)res, (__m256)rotated);
    }
    return res;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512fv8u64 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [ v8u64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_maskz_mov_epi64(vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8du res = *((__v8du *)&masked_data);
    __v8du rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res);
        res = (__v8du)_mm512_mask_max_epu64((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return res;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512fvf8i64 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [ v8i64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_mask_mov_epi64(_mm512_set1_epi64(LONG_MIN), vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8di res = *((__v8di *)&masked_data);
    __v8di rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res);
        res = (__v8di)_mm512_mask_max_epi64((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return res;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512fvf8f64 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [ v8f64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512d masked_data = _mm512_mask_mov_pd(_mm512_set1_pd(-INFINITY), vmask, (__m512d)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8df res = *((__v8df *)&masked_data);
    __v8df rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res);
        res = (__v8df)_mm512_mask_max_pd((__m512d)res, mask, (__m512d)res, (__m512d)rotated);
    }
    return res;
  }]>;

// VF 8 end
// VF 4 begin

OclBuiltinImpl sub_group_scan_inclusive_max_avx512fv4 = OclBuiltinImpl<sub_group_scan_inclusive_max_vec, [ v4i32, v4u32, v4f32, v4i64, v4u64, v4f64 ], 0,
  [{
    uint8 temp_mask;
    temp_mask.hi = 0;
    temp_mask.lo = $Arg1VarName;
    $Arg0BaseType#8 temp_pred;
    temp_pred.lo = $Arg0VarName;
    return sub_group_scan_inclusive_max(temp_pred, temp_mask).lo;
  }]>;

// VF 4 end

// inclusive max end
// exclusive [add|min|max]
// exclusive add begin

// VF 16 begin
OclBuiltinImpl sub_group_scan_exclusive_add_avx512fui32 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [ v16i32, v16u32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_maskz_mov_epi32(vmask, (__m512i) $Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16si res = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = masked_data[0];
    __v16si rotated_d = (__v16si)_mm512_permutexvar_epi32(rotate_mask, masked_data);
    __v16si rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 14; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v16si)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
        res = (__v16si)_mm512_mask_add_epi32((__m512i)res, mask, (__m512i)rotated, (__m512i)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx512ff32 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [ v16f32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512 masked_data = _mm512_maskz_mov_ps(vmask, $Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16sf res = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = $Arg0VarName[0];
    __v16sf rotated_d = (__v16sf)_mm512_permutexvar_ps(rotate_mask, (__m512)$Arg0VarName);
    __v16sf rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 14; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v16sf)_mm512_permutexvar_ps(rotate_mask, (__m512)res);
        res = (__v16sf)_mm512_mask_add_ps((__m512)res, mask, (__m512)rotated, (__m512)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx512v16ui64 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [ v16u64, v16i64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    __m512i lo = _mm512_maskz_mov_epi64(mask_lo, (__m512i)$Arg0VarName.lo);
    __m512i hi = _mm512_maskz_mov_epi64(mask_hi, (__m512i)$Arg0VarName.hi);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    $Arg0Type res = ($Arg0Type)0;
    __v8du res_lo = (__v8du) (($Arg0BaseType#8)0);
    __v8du res_hi = (__v8du) (($Arg0BaseType#8)0);
    __v8du d_lo = *(__v8du*)(&lo);
    __v8du d_hi = *(__v8du*)((ulong*)(&hi));
    res_lo[1] = d_lo[0];
    __v8du rotated_d = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)d_lo);
    __v8du rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_lo);
        res_lo = (__v8du)_mm512_mask_add_epi64((__m512i)res_lo, mask, (__m512i)rotated, (__m512i)rotated_d);
    }

    res_hi[0] = res_lo[7] + d_lo[7];

    rotated_d = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)d_hi);

    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_hi);
        res_hi = (__v8du)_mm512_mask_add_epi64((__m512i)res_hi, mask, (__m512i)rotated, (__m512i)rotated_d);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)(((ulong*)&res) + 8), (__m512)res_hi);

    return res;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx512v16f64 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [ v16f64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    __m512d lo = _mm512_maskz_mov_pd(mask_lo, $Arg0VarName.lo);
    __m512d hi = _mm512_maskz_mov_pd(mask_hi, $Arg0VarName.hi);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    $Arg0Type res = ($Arg0Type) 0;
    __v8df res_lo = (__v8df) (($Arg0BaseType#8)0);
    __v8df res_hi = (__v8df) (($Arg0BaseType#8)0);
    __v8df d_lo = *(__v8df*)(&lo);
    __v8df d_hi = *(__v8df*)((double*)(&hi));
    res_lo[1] = d_lo[0];
    __v8df rotated_d = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)d_lo);
    __v8df rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res_lo);
        res_lo = (__v8df)_mm512_mask_add_pd((__m512d)res_lo, mask, (__m512d)rotated, (__m512d)rotated_d);
    }

    res_hi[0] = res_lo[7] + d_lo[7];

    rotated_d = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)d_hi);

    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res_hi);
        res_hi = (__v8df)_mm512_mask_add_pd((__m512d)res_hi, mask, (__m512d)rotated, (__m512d)rotated_d);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)(((double*)&res) + 8), (__m512)res_hi);

    return res;
  }]>;

// VF 16 end
// VF 8 begin

OclBuiltinImpl sub_group_scan_exclusive_add_avx512fvf8ui32 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [ v8i32, v8u32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256i masked_data = _mm256_maskz_mov_epi32(vmask, (__m256i)$Arg0VarName);

    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8si res = {0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = masked_data[0];
    __v8si rotated_d = (__v8si)_mm256_permutexvar_epi32(rotate_mask, masked_data);
    __v8si rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8si)_mm256_permutexvar_epi32(rotate_mask, (__m256i)res);
        res = (__v8si)_mm256_mask_add_epi32((__m256i)res, mask, (__m256i)rotated, (__m256i)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx512fvf8f32 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [ v8f32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256 masked_data = _mm256_maskz_mov_ps(vmask, (__m256)$Arg0VarName);

    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8sf res = {0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = masked_data[0];
    __v8sf rotated_d = (__v8sf)_mm256_permutexvar_ps(rotate_mask, masked_data);
    __v8sf rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8sf)_mm256_permutexvar_ps(rotate_mask, (__m256)res);
        res = (__v8sf)_mm256_mask_add_ps((__m256)res, mask, (__m256)rotated, (__m256)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx512fvf8ui64 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [ v8i64, v8u64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_maskz_mov_epi64(vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8di res = { 0, 0, 0, 0, 0, 0, 0, 0 };
    res[1] = masked_data[0];
    __v8di rotated_d = (__v8di)_mm512_permutexvar_epi64(rotate_mask, masked_data);
    __v8di rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res);
        res = (__v8di)_mm512_mask_add_epi64((__m512i)res, mask, (__m512i)rotated, (__m512i)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx512fv8f64 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [ v8f64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512d masked_data = _mm512_maskz_mov_pd(vmask, (__m512d)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8df res = { 0, 0, 0, 0, 0, 0, 0, 0 };
    res[1] = masked_data[0];
    __v8df rotated_d = (__v8df)_mm512_permutexvar_pd(rotate_mask, masked_data);
    __v8df rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res);
        res = (__v8df)_mm512_mask_add_pd((__m512d)res, mask, (__m512d)rotated, (__m512d)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

// VF 8 end
// VF 4 begin

OclBuiltinImpl sub_group_scan_exclusive_add_avx512fvf4 = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, [ v4i32, v4u32, v4f32, v4i64, v4u64, v4f64 ], 0,
  [{
    uint8 temp_mask;
    temp_mask.hi = 0;
    temp_mask.lo = $Arg1VarName;
    $Arg0BaseType#8 temp_pred;
    temp_pred.lo = $Arg0VarName;
    return sub_group_scan_exclusive_add(temp_pred, temp_mask).lo;
  }]>;

// VF 4 end

// exclusive add end
// exclusive min begin

// VF 16 begin

OclBuiltinImpl sub_group_scan_exclusive_min_avx512fui32 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [ v16u32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_mask_mov_epi32(_mm512_set1_epi32(INT_MAX), vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16su res = ($Arg0Type)(UINT_MAX);
    res[1] = masked_data[0];
    __v16su rotated_d = (__v16su)_mm512_permutexvar_epi32(rotate_mask, masked_data);
    __v16su rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 14; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v16su)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
        res = (__v16su)_mm512_mask_min_epu32((__m512i)res, mask, (__m512i)rotated, (__m512i)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx512fi32 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [ v16i32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_mask_mov_epi32(_mm512_set1_epi32(UINT_MAX), vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16si res = ($Arg0Type)(INT_MAX);
    res[1] = masked_data[0];
    __v16si rotated_d = (__v16si)_mm512_permutexvar_epi32(rotate_mask, masked_data);
    __v16si rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 14; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v16si)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
        res = (__v16si)_mm512_mask_min_epi32((__m512i)res, mask, (__m512i)rotated, (__m512i)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx512ff32 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [ v16f32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512 masked_data = _mm512_mask_mov_ps(_mm512_set1_ps(INFINITY), vmask, (__m512)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16sf res = ($Arg0Type) INFINITY;
    res[1] = masked_data[0];
    __v16sf rotated_d = (__v16sf)_mm512_permutexvar_ps(rotate_mask, masked_data);
    __v16sf rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 14; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v16sf)_mm512_permutexvar_ps(rotate_mask, (__m512)res);
        res = (__v16sf)_mm512_mask_min_ps((__m512)res, mask, (__m512)rotated, (__m512)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx512v16u64 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [ v16u64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    __m512i lo = _mm512_mask_mov_epi64(_mm512_set1_epi64(ULONG_MAX), mask_lo, (__m512i)$Arg0VarName.lo);
    __m512i hi = _mm512_mask_mov_epi64(_mm512_set1_epi64(ULONG_MAX), mask_hi, (__m512i)$Arg0VarName.hi);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    $Arg0Type res = ($Arg0Type) ULONG_MAX;
    __v8du res_lo = (__v8du) (($Arg0BaseType#8)ULONG_MAX);
    __v8du res_hi = (__v8du) (($Arg0BaseType#8)ULONG_MAX);
    __v8du d_lo = *(__v8du*)(&lo);
    __v8du d_hi = *(__v8du*)((ulong*)(&hi));
    res_lo[1] = d_lo[0];
    __v8du rotated_d = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)d_lo);
    __v8du rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_lo);
        res_lo = (__v8du)_mm512_mask_min_epu64((__m512i)res_lo, mask, (__m512i)rotated, (__m512i)rotated_d);
    }

    res_hi[0] = res_lo[7] < d_lo[7] ? res_lo[7] : d_lo[7];

    rotated_d = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)d_hi);

    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_hi);
        res_hi = (__v8du)_mm512_mask_min_epu64((__m512i)res_hi, mask, (__m512i)rotated, (__m512i)rotated_d);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)(((ulong*)&res) + 8), (__m512)res_hi);

    return res;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx512v16i64 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [ v16i64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    __m512i lo = _mm512_mask_mov_epi64(_mm512_set1_epi64(LONG_MAX), mask_lo, (__m512i)$Arg0VarName.lo);
    __m512i hi = _mm512_mask_mov_epi64(_mm512_set1_epi64(LONG_MAX), mask_hi, (__m512i)$Arg0VarName.hi);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    $Arg0Type res = ($Arg0Type) LONG_MAX;
    __v8di res_lo = (__v8di) (($Arg0BaseType#8)LONG_MAX);
    __v8di res_hi = (__v8di) (($Arg0BaseType#8)LONG_MAX);
    __v8di d_lo = *(__v8di*)(&lo);
    __v8di d_hi = *(__v8di*)((ulong*)(&hi));
    res_lo[1] = d_lo[0];
    __v8di rotated_d = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)d_lo);
    __v8di rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_lo);
        res_lo = (__v8di)_mm512_mask_min_epi64((__m512i)res_lo, mask, (__m512i)rotated, (__m512i)rotated_d);
    }

    res_hi[0] = res_lo[7] < d_lo[7] ? res_lo[7] : d_lo[7];

    rotated_d = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)d_hi);

    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_hi);
        res_hi = (__v8di)_mm512_mask_min_epi64((__m512i)res_hi, mask, (__m512i)rotated, (__m512i)rotated_d);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)(((long*)&res) + 8), (__m512)res_hi);

    return res;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx512v16f64 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [ v16f64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    __m512d lo = _mm512_mask_mov_pd(_mm512_set1_pd(INFINITY), mask_lo, (__m512d)$Arg0VarName.lo);
    __m512d hi = _mm512_mask_mov_pd(_mm512_set1_pd(INFINITY), mask_hi, (__m512d)$Arg0VarName.hi);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    $Arg0Type res = ($Arg0Type) INFINITY;
    __v8df res_lo = (__v8df) (($Arg0BaseType#8)INFINITY);
    __v8df res_hi = (__v8df) (($Arg0BaseType#8)INFINITY);
    __v8df d_lo = *(__v8df*)(&lo);
    __v8df d_hi = *(__v8df*)((double*)(&hi));
    res_lo[1] = d_lo[0];
    __v8df rotated_d = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)d_lo);
    __v8df rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res_lo);
        res_lo = (__v8df)_mm512_mask_min_pd((__m512d)res_lo, mask, (__m512d)rotated, (__m512d)rotated_d);
    }

    res_hi[0] = res_lo[7] < d_lo[7] ? res_lo[7] : d_lo[7];

    rotated_d = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)d_hi);

    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res_hi);
        res_hi = (__v8df)_mm512_mask_min_pd((__m512d)res_hi, mask, (__m512d)rotated, (__m512d)rotated_d);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)(((double*)&res) + 8), (__m512)res_hi);

    return res;
  }]>;

// VF 16 end
// VF 8 begin

OclBuiltinImpl sub_group_scan_exclusive_min_avx512fvf8ui32 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [ v8u32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256i masked_data = _mm256_mask_mov_epi32(_mm256_set1_epi32(UINT_MAX), vmask, (__m256i)$Arg0VarName);

    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8su res = ($Arg0Type)(UINT_MAX);
    res[1] = masked_data[0];
    __v8su rotated_d = (__v8su)_mm256_permutexvar_epi32(rotate_mask, masked_data);
    __v8su rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8su)_mm256_permutexvar_epi32(rotate_mask, (__m256i)res);
        res = (__v8su)_mm256_mask_min_epu32((__m256i)res, mask, (__m256i)rotated, (__m256i)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx512fvf8i32 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [ v8i32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256i masked_data = _mm256_mask_mov_epi32(_mm256_set1_epi32(INT_MAX), vmask, (__m256i)$Arg0VarName);

    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8si res = ($Arg0Type)(INT_MAX);
    res[1] = masked_data[0];
    __v8si rotated_d = (__v8si)_mm256_permutexvar_epi32(rotate_mask, masked_data);
    __v8si rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8si)_mm256_permutexvar_epi32(rotate_mask, (__m256i)res);
        res = (__v8si)_mm256_mask_min_epi32((__m256i)res, mask, (__m256i)rotated, (__m256i)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx512fvf8f32 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [ v8f32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256 masked_data = _mm256_mask_mov_ps(_mm256_set1_ps(INFINITY), vmask, (__m256)$Arg0VarName);

    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8sf res = ($Arg0Type) INFINITY;
    res[1] = masked_data[0];
    __v8sf rotated_d = (__v8sf)_mm256_permutexvar_ps(rotate_mask, masked_data);
    __v8sf rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8sf)_mm256_permutexvar_ps(rotate_mask, (__m256)res);
        res = (__v8sf)_mm256_mask_min_ps((__m256)res, mask, (__m256)rotated, (__m256)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx512fvf8u64 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [ v8u64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_mask_mov_epi64(_mm512_set1_epi64(ULONG_MAX), vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8du res = ($Arg0Type) ULONG_MAX;
    res[1] = masked_data[0];
    __v8du rotated_d = (__v8du)_mm512_permutexvar_epi64(rotate_mask, masked_data);
    __v8du rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res);
        res = (__v8du)_mm512_mask_min_epu64((__m512i)res, mask, (__m512i)rotated, (__m512i)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx512fvf8i64 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [ v8i64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_mask_mov_epi64(_mm512_set1_epi64(LONG_MAX), vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8di res = ($Arg0Type) LONG_MAX;
    res[1] = masked_data[0];
    __v8di rotated_d = (__v8di)_mm512_permutexvar_epi64(rotate_mask, masked_data);
    __v8di rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res);
        res = (__v8di)_mm512_mask_min_epi64((__m512i)res, mask, (__m512i)rotated, (__m512i)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx512fvf8f64 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [ v8f64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512d masked_data = _mm512_mask_mov_pd(_mm512_set1_pd(INFINITY), vmask, (__m512d)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8df res = ($Arg0Type) INFINITY;
    res[1] = masked_data[0];
    __v8df rotated_d = (__v8df)_mm512_permutexvar_pd(rotate_mask, masked_data);
    __v8df rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res);
        res = (__v8df)_mm512_mask_min_pd((__m512d)res, mask, (__m512d)rotated, (__m512d)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

// VF 8 end
// VF 4 begin

OclBuiltinImpl sub_group_scan_exclusive_min_avx512ffv4 = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, [ v4i32, v4u32, v4f32, v4i64, v4u64, v4f64 ], 0, 
  [{
    uint8 temp_mask;
    temp_mask.hi = 0;
    temp_mask.lo = $Arg1VarName;
    $Arg0BaseType#8 temp_pred;
    temp_pred.lo = $Arg0VarName;
    return sub_group_scan_exclusive_min(temp_pred, temp_mask).lo;
  }]>;

// VF 4 end

// exclusive min end
// exclusive max begin

// VF 16 begin
OclBuiltinImpl sub_group_scan_exclusive_max_avx512fi32 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [ v16i32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_mask_mov_epi32(_mm512_set1_epi32(INT_MIN), vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16si res = ($Arg0Type)INT_MIN;
    res[1] = masked_data[0];
    __v16si rotated_d = (__v16si)_mm512_permutexvar_epi32(rotate_mask, masked_data);
    __v16si rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 14; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v16si)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
        res = (__v16si)_mm512_mask_max_epi32((__m512i)res, mask, (__m512i)rotated, (__m512i)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx512fu32 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [ v16u32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_maskz_mov_epi32(vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16su res = ($Arg0Type)0;
    res[1] = masked_data[0];
    __v16su rotated_d = (__v16su)_mm512_permutexvar_epi32(rotate_mask, masked_data);
    __v16su rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 14; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v16su)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
        res = (__v16su)_mm512_mask_max_epu32((__m512i)res, mask, (__m512i)rotated, (__m512i)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx512ff32 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [ v16f32 ], 0,
  [{
    __mmask16 vmask = _mm512_cmp_epi32_mask($Arg1VarName, _mm512_setzero_epi32(), _MM_CMPINT_NE);
    __m512 masked_data = _mm512_mask_mov_ps(_mm512_set1_ps(-INFINITY), vmask, (__m512)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16sf res = ($Arg0Type)-INFINITY;
    res[1] = masked_data[0];
    __v16sf rotated_d = (__v16sf)_mm512_permutexvar_ps(rotate_mask, masked_data);
    __v16sf rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 14; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v16sf)_mm512_permutexvar_ps(rotate_mask, (__m512)res);
        res = (__v16sf)_mm512_mask_max_ps((__m512)res, mask, (__m512)rotated, (__m512)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx512v16u64 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [ v16u64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    __m512i lo = _mm512_maskz_mov_epi64(mask_lo, (__m512i)$Arg0VarName.lo);
    __m512i hi = _mm512_maskz_mov_epi64(mask_hi, (__m512i)$Arg0VarName.hi);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    $Arg0Type res = ($Arg0Type)0;
    __v8du res_lo = (__v8du) (($Arg0BaseType#8)0);
    __v8du res_hi = (__v8du) (($Arg0BaseType#8)0);
    __v8du d_lo = *(__v8du*)(&lo);
    __v8du d_hi = *(__v8du*)((ulong*)(&hi));
    res_lo[1] = d_lo[0];
    __v8du rotated_d = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)d_lo);
    __v8du rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_lo);
        res_lo = (__v8du)_mm512_mask_max_epu64((__m512i)res_lo, mask, (__m512i)rotated, (__m512i)rotated_d);
    }

    res_hi[0] = res_lo[7] > d_lo[7] ? res_lo[7] : d_lo[7];

    rotated_d = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)d_hi);

    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_hi);
        res_hi = (__v8du)_mm512_mask_max_epu64((__m512i)res_hi, mask, (__m512i)rotated, (__m512i)rotated_d);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)(((ulong*)&res) + 8), (__m512)res_hi);

    return res;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx512v16i64 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [ v16i64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    __m512i lo = _mm512_mask_mov_epi64(_mm512_set1_epi64(LONG_MIN), mask_lo, (__m512i)$Arg0VarName.lo);
    __m512i hi = _mm512_mask_mov_epi64(_mm512_set1_epi64(LONG_MIN), mask_hi, (__m512i)$Arg0VarName.hi);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    $Arg0Type res = ($Arg0Type)LONG_MIN;
    __v8di res_lo = (__v8di) (($Arg0BaseType#8)LONG_MIN);
    __v8di res_hi = (__v8di) (($Arg0BaseType#8)LONG_MIN);
    __v8di d_lo = *(__v8di*)(&lo);
    __v8di d_hi = *(__v8di*)((long*)(&hi));
    res_lo[1] = d_lo[0];
    __v8di rotated_d = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)d_lo);
    __v8di rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_lo);
        res_lo = (__v8di)_mm512_mask_max_epi64((__m512i)res_lo, mask, (__m512i)rotated, (__m512i)rotated_d);
    }

    res_hi[0] = res_lo[7] > d_lo[7] ? res_lo[7] : d_lo[7];

    rotated_d = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)d_hi);

    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_hi);
        res_hi = (__v8di)_mm512_mask_max_epi64((__m512i)res_hi, mask, (__m512i)rotated, (__m512i)rotated_d);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)(((long*)&res) + 8), (__m512)res_hi);

    return res;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx512v16f64 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [ v16f64 ], 0,
  [{
    __mmask8 mask_lo = _mm256_cmp_epi32_mask($Arg1VarName.lo, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __mmask8 mask_hi = _mm256_cmp_epi32_mask($Arg1VarName.hi, _mm256_setzero_ps(), _MM_CMPINT_NE);

    __m512d lo = _mm512_mask_mov_pd(_mm512_set1_pd(-INFINITY), mask_lo, (__m512d)$Arg0VarName.lo);
    __m512d hi = _mm512_mask_mov_pd(_mm512_set1_pd(-INFINITY), mask_hi, (__m512d)$Arg0VarName.hi);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    $Arg0Type res = ($Arg0Type)-INFINITY;
    __v8df res_lo = (__v8df) (($Arg0BaseType#8)-INFINITY);
    __v8df res_hi = (__v8df) (($Arg0BaseType#8)-INFINITY);
    __v8df d_lo = *(__v8df*)(&lo);
    __v8df d_hi = *(__v8df*)((double*)(&hi));
    res_lo[1] = d_lo[0];
    __v8df rotated_d = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)d_lo);
    __v8df rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res_lo);
        res_lo = (__v8df)_mm512_mask_max_pd((__m512d)res_lo, mask, (__m512d)rotated, (__m512d)rotated_d);
    }

    res_hi[0] = res_lo[7] > d_lo[7] ? res_lo[7] : d_lo[7];

    rotated_d = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)d_hi);

    mask = 1;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res_hi);
        res_hi = (__v8df)_mm512_mask_max_pd((__m512d)res_hi, mask, (__m512d)rotated, (__m512d)rotated_d);
    }

    _mm512_store_ps((void*)&res, (__m512)res_lo);
    _mm512_store_ps((void*)(((double*)&res) + 8), (__m512)res_hi);

    return res;
  }]>;

// VF 16 end
// VF 8 begin

OclBuiltinImpl sub_group_scan_exclusive_max_avx512fvf8i32 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [ v8i32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256i masked_data = _mm256_mask_mov_epi32(_mm256_set1_epi32(INT_MIN), vmask, (__m256i)$Arg0VarName);

    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8si res = ($Arg0Type)INT_MIN;
    res[1] = masked_data[0];
    __v8si rotated_d = (__v8si)_mm256_permutexvar_epi32(rotate_mask, masked_data);
    __v8si rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8si)_mm256_permutexvar_epi32(rotate_mask, (__m256i)res);
        res = (__v8si)_mm256_mask_max_epi32((__m256i)res, mask, (__m256i)rotated, (__m256i)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx512fvf8u32 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [ v8u32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256i masked_data = _mm256_maskz_mov_epi32(vmask, (__m256i)$Arg0VarName);

    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8su res = ($Arg0Type)0;
    res[1] = masked_data[0];
    __v8su rotated_d = (__v8su)_mm256_permutexvar_epi32(rotate_mask, masked_data);
    __v8su rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8su)_mm256_permutexvar_epi32(rotate_mask, (__m256i)res);
        res = (__v8su)_mm256_mask_max_epu32((__m256i)res, mask, (__m256i)rotated, (__m256i)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx512fvf8f32 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [ v8f32 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m256 masked_data = _mm256_mask_mov_ps(_mm256_set1_ps(-INFINITY), vmask, (__m256)$Arg0VarName);

    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8sf res = ($Arg0Type)-INFINITY;
    res[1] = masked_data[0];
    __v8sf rotated_d = (__v8sf)_mm256_permutexvar_ps(rotate_mask, masked_data);
    __v8sf rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8sf)_mm256_permutexvar_ps(rotate_mask, (__m256)res);
        res = (__v8sf)_mm256_mask_max_ps((__m256)res, mask, (__m256)rotated, (__m256)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx512vf8fu64 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [ v8u64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_maskz_mov_epi64(vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8du res = ($Arg0Type)0;
    res[1] = masked_data[0];
    __v8du rotated_d = (__v8du)_mm512_permutexvar_epi64(rotate_mask, masked_data);
    __v8du rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
      mask <<= 1;
      rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res);
      res = (__v8du)_mm512_mask_max_epu64((__m512i)res, mask, (__m512i)rotated, (__m512i)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx512fvf8i64 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [ v8i64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512i masked_data = _mm512_mask_mov_epi64(_mm512_set1_epi64(LONG_MIN), vmask, (__m512i)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8di res = ($Arg0Type)LONG_MIN;
    res[1] = masked_data[0];
    __v8di rotated_d = (__v8di)_mm512_permutexvar_epi64(rotate_mask, masked_data);
    __v8di rotated;
    mask <<= 1;

#pragma clang unroll(full)
    for (int i = 0; i < 6; i++) { // VF - 2 iterations
      mask <<= 1;
      rotated = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res);
      res = (__v8di)_mm512_mask_max_epi64((__m512i)res, mask, (__m512i)rotated, (__m512i)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx512fvf8f64 =  OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [ v8f64 ], 0,
  [{
    __mmask8 vmask = _mm256_cmp_epi32_mask($Arg1VarName, _mm256_setzero_ps(), _MM_CMPINT_NE);
    __m512d masked_data = _mm512_mask_mov_pd(_mm512_set1_pd(-INFINITY), vmask, (__m512d)$Arg0VarName);

    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8df res = ($Arg0Type)-INFINITY;
    res[1] = masked_data[0];
    __v8df rotated_d = (__v8df)_mm512_permutexvar_pd(rotate_mask, masked_data);
    __v8df rotated;
    mask <<= 1;

#pragma clang unroll(full)
for (int i = 0; i < 6; i++) { // VF - 2 iterations
      mask <<= 1;
      rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res);
      res = (__v8df)_mm512_mask_max_pd((__m512d)res, mask, (__m512d)rotated, (__m512d)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

// VF 8 end
// VF 4 begin

OclBuiltinImpl sub_group_scan_exclusive_max_avx512fvf4 = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, [ v4i32, v4u32, v4f32, v4i64, v4u64, v4f64 ], 0,
  [{
    uint8 temp_mask;
    temp_mask.hi = 0;
    temp_mask.lo = $Arg1VarName;
    $Arg0BaseType#8 temp_pred;
    temp_pred.lo = $Arg0VarName;
    return sub_group_scan_exclusive_max(temp_pred, temp_mask).lo;
  }]>;

// VF 4 end

// exclusive max end

//
// Shuffles
//

// int and uint
OclBuiltinImpl sub_group_shuffle_avx512fv16i32 = OclBuiltinImpl<sub_group_shuffle_avx512, [v16i32, v16u32], 0,
  [{
    $Arg1VarName = $Arg1VarName & $Arg2VarName;
    return as_$ReturnType(_mm512_permutexvar_epi32(__builtin_astype($Arg1VarName, __m512i),
                                                   __builtin_astype($Arg0VarName, __m512i)));
  }]>;

// float
OclBuiltinImpl sub_group_shuffle_avx512fv16f32 = OclBuiltinImpl<sub_group_shuffle_avx512, [v16f32], 0,
  [{
    $Arg1VarName = $Arg1VarName & $Arg2VarName;
    return as_$ReturnType(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i),
                                                __builtin_astype($Arg0VarName, __m512)));
  }]>;

// long, ulong, double
OclBuiltinImpl sub_group_shuffle_avx512fv16i64 = OclBuiltinImpl<sub_group_shuffle_avx512, [v16i64, v16u64, v16f64], 0,
 [{
     $Arg1VarName = $Arg1VarName & $Arg2VarName;
     $Arg0Type dest;
     uint32 idx;
     idx.lo = as_uint16(_mm512_cvtepi32_epi64((__m256i)$Arg1VarName.lo));
     idx.hi = as_uint16(_mm512_cvtepi32_epi64((__m256i)$Arg1VarName.hi));
     dest.lo = as_$Arg0BaseType#8(_mm512_permutex2var_epi64((__m512i)$Arg0VarName.lo, (__m512i)idx.lo, (__m512i)$Arg0VarName.hi));
     dest.hi = as_$Arg0BaseType#8(_mm512_permutex2var_epi64((__m512i)$Arg0VarName.lo, (__m512i)idx.hi, (__m512i)$Arg0VarName.hi));
     return dest;
 }]>;

// float2, int2 and uint2 have the same size as long (64 bit).
// So we can shuffle them as double precision numbers
OclBuiltinImpl sub_group_shuffle_avx512fv32f32 = OclBuiltinImpl<sub_group_shuffle_avx512, [v32i32, v32u32, v32f32], 0,
 [{
     $Arg1VarName = $Arg1VarName & $Arg2VarName;
     $Arg0Type dest;
     uint32 idx;
     idx.lo = as_uint16(_mm512_cvtepi32_epi64((__m256i)$Arg1VarName.lo));
     idx.hi = as_uint16(_mm512_cvtepi32_epi64((__m256i)$Arg1VarName.hi));
     dest.lo = as_$Arg0BaseType#16(_mm512_permutex2var_epi64((__m512i)$Arg0VarName.lo, (__m512i)idx.lo, (__m512i)$Arg0VarName.hi));
     dest.hi = as_$Arg0BaseType#16(_mm512_permutex2var_epi64((__m512i)$Arg0VarName.lo, (__m512i)idx.hi, (__m512i)$Arg0VarName.hi));
     return dest;
 }]>;


// float4, int4, uint4
OclBuiltinImpl sub_group_shuffle_avx512f_v64fiu32 = OclBuiltinImpl<sub_group_shuffle_avx512, [v64f32, v64i32, v64u32], 0,
  [{
      $Arg1VarName = $Arg1VarName & $Arg2VarName;
      $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_4x16($Arg0VarName);

      $Arg0BaseType#16 tmp1 = $Arg0VarName.lo.lo;
      $Arg0BaseType#16 tmp2 = $Arg0VarName.lo.hi;
      $Arg0BaseType#16 tmp3 = $Arg0VarName.hi.lo;
      $Arg0BaseType#16 tmp4 = $Arg0VarName.hi.hi;

      $Arg0VarName.lo.lo = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp1, __m512)));
      $Arg0VarName.lo.hi = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp2, __m512)));
      $Arg0VarName.hi.lo = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp3, __m512)));
      $Arg0VarName.hi.hi = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp4, __m512)));

      $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x4($Arg0VarName);
      return $Arg0VarName;
  }]>;


// float8, int8, uint8
OclBuiltinImpl sub_group_shuffle_avx512f_v128fiu32 = OclBuiltinImpl<sub_group_shuffle_avx512, [v128f32, v128i32, v128u32], 0,
  [{
      $Arg1VarName = $Arg1VarName & $Arg2VarName;
      $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x16($Arg0VarName);

      $Arg0BaseType#16 tmp1 = $Arg0VarName.lo.lo.lo;
      $Arg0BaseType#16 tmp2 = $Arg0VarName.lo.lo.hi;
      $Arg0BaseType#16 tmp3 = $Arg0VarName.lo.hi.lo;
      $Arg0BaseType#16 tmp4 = $Arg0VarName.lo.hi.hi;
      $Arg0BaseType#16 tmp5 = $Arg0VarName.hi.lo.lo;
      $Arg0BaseType#16 tmp6 = $Arg0VarName.hi.lo.hi;
      $Arg0BaseType#16 tmp7 = $Arg0VarName.hi.hi.lo;
      $Arg0BaseType#16 tmp8 = $Arg0VarName.hi.hi.hi;

      $Arg0VarName.lo.lo.lo = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp1, __m512)));
      $Arg0VarName.lo.lo.hi = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp2, __m512)));
      $Arg0VarName.lo.hi.lo = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp3, __m512)));
      $Arg0VarName.lo.hi.hi = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp4, __m512)));
      $Arg0VarName.hi.lo.lo = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp5, __m512)));
      $Arg0VarName.hi.lo.hi = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp6, __m512)));
      $Arg0VarName.hi.hi.lo = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp7, __m512)));
      $Arg0VarName.hi.hi.hi = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp8, __m512)));

      $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x8($Arg0VarName);
      return $Arg0VarName;
  }]>;


// float16, int16, uint16
OclBuiltinImpl sub_group_shuffle_avx512f_v256fiu32 = OclBuiltinImpl<sub_group_shuffle_avx512, [v256f32, v256i32, v256u32], 0,
  [{
      $Arg1VarName = $Arg1VarName & $Arg2VarName;
      $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x16($Arg0VarName);

      $Arg0BaseType#16 tmp1  = $Arg0VarName.lo.lo.lo.lo;
      $Arg0BaseType#16 tmp2  = $Arg0VarName.lo.lo.lo.hi;
      $Arg0BaseType#16 tmp3  = $Arg0VarName.lo.lo.hi.lo;
      $Arg0BaseType#16 tmp4  = $Arg0VarName.lo.lo.hi.hi;
      $Arg0BaseType#16 tmp5  = $Arg0VarName.lo.hi.lo.lo;
      $Arg0BaseType#16 tmp6  = $Arg0VarName.lo.hi.lo.hi;
      $Arg0BaseType#16 tmp7  = $Arg0VarName.lo.hi.hi.lo;
      $Arg0BaseType#16 tmp8  = $Arg0VarName.lo.hi.hi.hi;
      $Arg0BaseType#16 tmp9  = $Arg0VarName.hi.lo.lo.lo;
      $Arg0BaseType#16 tmp10 = $Arg0VarName.hi.lo.lo.hi;
      $Arg0BaseType#16 tmp11 = $Arg0VarName.hi.lo.hi.lo;
      $Arg0BaseType#16 tmp12 = $Arg0VarName.hi.lo.hi.hi;
      $Arg0BaseType#16 tmp13 = $Arg0VarName.hi.hi.lo.lo;
      $Arg0BaseType#16 tmp14 = $Arg0VarName.hi.hi.lo.hi;
      $Arg0BaseType#16 tmp15 = $Arg0VarName.hi.hi.hi.lo;
      $Arg0BaseType#16 tmp16 = $Arg0VarName.hi.hi.hi.hi;

      $Arg0VarName.lo.lo.lo.lo = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype( tmp1, __m512)));
      $Arg0VarName.lo.lo.lo.hi = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype( tmp2, __m512)));
      $Arg0VarName.lo.lo.hi.lo = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype( tmp3, __m512)));
      $Arg0VarName.lo.lo.hi.hi = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype( tmp4, __m512)));
      $Arg0VarName.lo.hi.lo.lo = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype( tmp5, __m512)));
      $Arg0VarName.lo.hi.lo.hi = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype( tmp6, __m512)));
      $Arg0VarName.lo.hi.hi.lo = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype( tmp7, __m512)));
      $Arg0VarName.lo.hi.hi.hi = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype( tmp8, __m512)));
      $Arg0VarName.hi.lo.lo.lo = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype( tmp9, __m512)));
      $Arg0VarName.hi.lo.lo.hi = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp10, __m512)));
      $Arg0VarName.hi.lo.hi.lo = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp11, __m512)));
      $Arg0VarName.hi.lo.hi.hi = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp12, __m512)));
      $Arg0VarName.hi.hi.lo.lo = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp13, __m512)));
      $Arg0VarName.hi.hi.lo.hi = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp14, __m512)));
      $Arg0VarName.hi.hi.hi.lo = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp15, __m512)));
      $Arg0VarName.hi.hi.hi.hi = as_$Arg0BaseType#16(_mm512_permutexvar_ps(__builtin_astype($Arg1VarName, __m512i), __builtin_astype(tmp16, __m512)));

      $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x16($Arg0VarName);
      return $Arg0VarName;
  }]>;


//
// Shuffle xor
//
OclBuiltinImpl sub_group_shuffle_xor_avx512_gen = OclBuiltinImpl<sub_group_shuffle_xor_avx512,
               [v16i32, v16u32, v16f32, v16i64, v16u64, v16f64,
                v32i32, v32u32, v32f32, v64i32, v64u32, v64f32,
                v128i32, v128u32, v128f32, v256i32, v256u32, v256f32], 0,
  [{
    uint16 indexes = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    indexes = indexes ^ $Arg1VarName;
    return intel_sub_group_shuffle($Arg0VarName, indexes, $Arg2VarName);
  }]>;

//
// Shuffle down
//
OclBuiltinImpl sub_group_shuffle_down_avx512_gen = OclBuiltinImpl<sub_group_shuffle_down_avx512,
               [v16i32, v16u32, v16f32, v32i32, v32u32, v32f32, v64i32, v64u32, v64f32,
                v128i32, v128u32, v128f32, v256i32, v256u32, v256f32], 0,
  [{
    $Arg0Type res_cur, res_next;
    int$VecLength temp;
    uint16 sg_indexes = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    const uint max_sg_size = 16;

    // Calculate indexes
    sg_indexes = sg_indexes + $Arg2VarName;

    // Apply vec_mask
    sg_indexes &= $Arg3VarName;

    // Get the mask for elements which < VF
    int16 indexes_mask = sg_indexes < max_sg_size;
    int16 inv_indexes_mask = !indexes_mask;

    int$VecLength ext_mask = __ocl_extend_mask_to_$VecLength(indexes_mask);

    // Call masked shuffle 2 times: first for elements from current, second for elements from next
    // Apply extended mask to get rid of unnecessary elements
    res_cur = intel_sub_group_shuffle($Arg0VarName, sg_indexes, *(uint16*)&indexes_mask);
    temp = *((int$VecLength*)&res_cur) & ext_mask;
    res_cur = *($Arg0Type*)&temp;

    res_next = intel_sub_group_shuffle($Arg1VarName, (sg_indexes - max_sg_size), *(uint16*)&inv_indexes_mask);
    temp = *((int$VecLength*)&res_next) & !ext_mask;
    res_next = *($Arg0Type*)&temp;

    temp = (*(int$VecLength*)&res_cur | *(int$VecLength*)&res_next);
    return *($Arg0Type*)&temp;
  }]>;


OclBuiltinImpl sub_group_shuffle_down_avx512_v16d64 = OclBuiltinImpl<sub_group_shuffle_down_avx512,
               [v16i64, v16u64, v16f64], 0,
  [{
    $Arg0Type res_cur, res_next;
    int32 temp;
    uint16 sg_indexes = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    const uint max_sg_size = 16;

    // Calculate indexes
    sg_indexes = sg_indexes + $Arg2VarName;

    // Apply vec_mask
    sg_indexes &= $Arg3VarName;

    // Get the mask for elements which < VF
    int16 indexes_mask = sg_indexes < max_sg_size;
    int16 inv_indexes_mask = !indexes_mask;

    int32 ext_mask = __ocl_extend_mask_to_32(indexes_mask);

    // Call masked shuffle 2 times: first for elements from current, second for elements from next
    // Apply extended mask to get rid of unnecessary elements
    res_cur = intel_sub_group_shuffle($Arg0VarName, sg_indexes, *(uint16*)&indexes_mask);
    temp = *((int32*)&res_cur) & ext_mask;
    res_cur = *($Arg0Type*)&temp;

    res_next = intel_sub_group_shuffle($Arg1VarName, (sg_indexes - max_sg_size), *(uint16*)&inv_indexes_mask);
    temp = *((int32*)&res_next) & !ext_mask;
    res_next = *($Arg0Type*)&temp;

    temp = (*(int32*)&res_cur | *(int32*)&res_next);
    return *($Arg0Type*)&temp;
  }]>;


//
// Shuffle up
//
OclBuiltinImpl sub_group_shuffle_up_avx512_gen = OclBuiltinImpl<sub_group_shuffle_up_avx512,
               [v16i32, v16u32, v16f32, v32i32, v32u32, v32f32, v64i32, v64u32, v64f32,
                v128i32, v128u32, v128f32, v256i32, v256u32, v256f32], 0,
  [{
    $Arg0Type res_cur, res_next;
    int$VecLength temp;
    int16 sg_indexes = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    const int max_sg_size = 16;

    // Calculate indexes
    sg_indexes = sg_indexes - *(int16*)&$Arg2VarName;

    // Apply vec_mask
    sg_indexes &= *(int16*)&$Arg3VarName;

    // Get the mask for elements which >= 0
    int16 indexes_mask = sg_indexes >= 0;
    int16 inv_indexes_mask = !indexes_mask;

    int$VecLength ext_mask = __ocl_extend_mask_to_$VecLength(indexes_mask);

    // Call masked shuffle 2 times: first for elements from current, second for elements from next
    // Apply extended mask to get rid of unnecessary elements
    res_cur = intel_sub_group_shuffle($Arg1VarName, *(uint16*)&sg_indexes, *(uint16*)&indexes_mask);
    temp = *((int$VecLength*)&res_cur) & ext_mask;
    res_cur = *($Arg0Type*)&temp;

    int16 other_ind = sg_indexes + max_sg_size;
    res_next = intel_sub_group_shuffle($Arg0VarName, *(uint16*)&other_ind, *(uint16*)&inv_indexes_mask);
    temp = *((int$VecLength*)&res_next) & !ext_mask;
    res_next = *($Arg0Type*)&temp;

    temp = (*(int$VecLength*)&res_cur | *(int$VecLength*)&res_next);
    return *($Arg0Type*)&temp;
  }]>;


OclBuiltinImpl sub_group_shuffle_up_avx512_v16d64 = OclBuiltinImpl<sub_group_shuffle_up_avx512,
               [v16i64, v16u64, v16f64], 0,
  [{
    $Arg0Type res_cur, res_next;
    int32 temp;
    int16 sg_indexes = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    const int max_sg_size = 16;

    // Calculate indexes
    sg_indexes = sg_indexes - *(int16*)&$Arg2VarName;

    // Apply vec_mask
    sg_indexes &= *(int16*)&$Arg3VarName;

    // Get the mask for elements which >= 0
    int16 indexes_mask = sg_indexes >= 0;
    int16 inv_indexes_mask = !indexes_mask;

    int32 ext_mask = __ocl_extend_mask_to_32(indexes_mask);

    // Call masked shuffle 2 times: first for elements from current, second for elements from next
    // Apply extended mask to get rid of unnecessary elements
    res_cur = intel_sub_group_shuffle($Arg1VarName, *(uint16*)&sg_indexes, *(uint16*)&indexes_mask);
    temp = *((int32*)&res_cur) & ext_mask;
    res_cur = *($Arg0Type*)&temp;

    int16 other_ind = sg_indexes + max_sg_size;
    res_next = intel_sub_group_shuffle($Arg0VarName, *(uint16*)&other_ind, *(uint16*)&inv_indexes_mask);
    temp = *((int32*)&res_next) & !ext_mask;
    res_next = *($Arg0Type*)&temp;

    temp = (*(int32*)&res_cur | *(int32*)&res_next);
    return *($Arg0Type*)&temp;
  }]>;

//
// Block read/write
//

list<OclType> intel_sub_group_block_read_write_us_types = [ v1u64, v1u32, v1u16, v1u8 ];

code BlockRead1_4 =
  [{
    const $ReturnType res = *($ReturnType*)($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 1, 2, 3);
  }];
OclBuiltinImpl intel_sub_group_block_read1_4_avx512f = OclBuiltinImpl<intel_sub_group_block_read1_4, [ v1u32 ], 0, BlockRead1_4>;
OclBuiltinImpl intel_sub_group_block_read_us1_4_avx512f = OclBuiltinImpl<intel_sub_group_block_read_us1_4, intel_sub_group_block_read_write_us_types, 0, BlockRead1_4>;

code BlockRead2_4 =
  [{
    const $ReturnType res = *($ReturnType*)($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 4,
                                             1, 5,
                                             2, 6,
                                             3, 7);
  }];
OclBuiltinImpl intel_sub_group_block_read2_4_avx512f = OclBuiltinImpl<intel_sub_group_block_read2_4, [ v1u32 ], 0, BlockRead2_4>;
OclBuiltinImpl intel_sub_group_block_read_us2_4_avx512f = OclBuiltinImpl<intel_sub_group_block_read_us2_4, intel_sub_group_block_read_write_us_types, 0, BlockRead2_4>;

code BlockRead4_4 =
  [{
    const $ReturnType res = *($ReturnType*)($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 4, 8,  12,
                                             1, 5, 9,  13,
                                             2, 6, 10, 14,
                                             3, 7, 11, 15);
  }];
OclBuiltinImpl intel_sub_group_block_read4_4_avx512f = OclBuiltinImpl<intel_sub_group_block_read4_4, [ v1u32 ], 0, BlockRead4_4>;
OclBuiltinImpl intel_sub_group_block_read_us4_4_avx512f = OclBuiltinImpl<intel_sub_group_block_read_us4_4, intel_sub_group_block_read_write_us_types, 0, BlockRead4_4>;

code BlockRead8_4 =
  [{
    const $ReturnType res = *($ReturnType*)($Arg0VarName);
    return __ocl_shuffle_transpose_$ReturnBaseType#_4x8(res);
  }];
OclBuiltinImpl intel_sub_group_block_read8_4_avx512f = OclBuiltinImpl<intel_sub_group_block_read8_4, [ v1u32 ], 0, BlockRead8_4>;
OclBuiltinImpl intel_sub_group_block_read_us8_4_avx512f = OclBuiltinImpl<intel_sub_group_block_read_us8_4, intel_sub_group_block_read_write_us_types, 0, BlockRead8_4>;

code BlockRead1_8 =
  [{
    const $ReturnType res = *($ReturnType*)($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 1, 2, 3, 4, 5, 6, 7);
  }];
OclBuiltinImpl intel_sub_group_block_read1_8_avx512f = OclBuiltinImpl<intel_sub_group_block_read1_8, [ v1u32 ], 0, BlockRead1_8>;
OclBuiltinImpl intel_sub_group_block_read_us1_8_avx512f = OclBuiltinImpl<intel_sub_group_block_read_us1_8, intel_sub_group_block_read_write_us_types, 0, BlockRead1_8>;

code BlockRead2_8 =
  [{
    const $ReturnType res = *($ReturnType*)($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 8,
                                             1, 9,
                                             2, 10,
                                             3, 11,
                                             4, 12,
                                             5, 13,
                                             6, 14,
                                             7, 15);
  }];
OclBuiltinImpl intel_sub_group_block_read2_8_avx512f = OclBuiltinImpl<intel_sub_group_block_read2_8, [ v1u32 ], 0, BlockRead2_8>;
OclBuiltinImpl intel_sub_group_block_read_us2_8_avx512f = OclBuiltinImpl<intel_sub_group_block_read_us2_8, intel_sub_group_block_read_write_us_types, 0, BlockRead2_8>;

code BlockRead4_8 =
  [{
    const $ReturnType res = *($ReturnType*)($Arg0VarName);
    return __ocl_shuffle_transpose_$ReturnBaseType#_8x4(res);
  }];
OclBuiltinImpl intel_sub_group_block_read4_8_avx512f = OclBuiltinImpl<intel_sub_group_block_read4_8, [ v1u32 ], 0, BlockRead4_8>;
OclBuiltinImpl intel_sub_group_block_read_us4_8_avx512f = OclBuiltinImpl<intel_sub_group_block_read_us4_8, intel_sub_group_block_read_write_us_types, 0, BlockRead4_8>;

code BlockRead8_8 =
  [{
    const $ReturnType res = *($ReturnType*)($Arg0VarName);
    return __ocl_shuffle_transpose_$ReturnBaseType#_8x8(res);
  }];
OclBuiltinImpl intel_sub_group_block_read8_8_avx512f = OclBuiltinImpl<intel_sub_group_block_read8_8, [ v1u32 ], 0, BlockRead8_8>;
OclBuiltinImpl intel_sub_group_block_read_us8_8_avx512f = OclBuiltinImpl<intel_sub_group_block_read_us8_8, intel_sub_group_block_read_write_us_types, 0, BlockRead8_8>;

code BlockRead1_16 =
  [{
    const $ReturnType res = *($ReturnType*)($Arg0VarName);
    return __builtin_shufflevector(res, res, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  }];
OclBuiltinImpl intel_sub_group_block_read1_16_avx512f = OclBuiltinImpl<intel_sub_group_block_read1_16, [ v1u32 ], 0, BlockRead1_16>;
OclBuiltinImpl intel_sub_group_block_read_us1_16_avx512f = OclBuiltinImpl<intel_sub_group_block_read_us1_16, intel_sub_group_block_read_write_us_types, 0, BlockRead1_16>;


code BlockRead2_16 =
  [{
    const $ReturnType res = *($ReturnType*)($Arg0VarName);
    return ($ReturnType)(__builtin_shufflevector(res, res, 0,  16,
                                                           1,  17,
                                                           2,  18,
                                                           3,  19,
                                                           4,  20,
                                                           5,  21,
                                                           6,  22,
                                                           7,  23,
                                                           8,  24,
                                                           9,  25,
                                                           10, 26,
                                                           11, 27,
                                                           12, 28,
                                                           13, 29,
                                                           14, 30,
                                                           15, 31));
  }];
OclBuiltinImpl intel_sub_group_block_read2_16_avx512f = OclBuiltinImpl<intel_sub_group_block_read2_16, [ v1u32 ], 0, BlockRead2_16>;
OclBuiltinImpl intel_sub_group_block_read_us2_16_avx512f = OclBuiltinImpl<intel_sub_group_block_read_us2_16, intel_sub_group_block_read_write_us_types, 0, BlockRead2_16>;

code BlockRead4_16 =
  [{
    const $ReturnType res = *($ReturnType*)($Arg0VarName);
    return __ocl_shuffle_transpose_$ReturnBaseType#_16x4(res);
  }];
OclBuiltinImpl intel_sub_group_block_read4_16_avx512f = OclBuiltinImpl<intel_sub_group_block_read4_16, [ v1u32 ], 0, BlockRead4_16>;
OclBuiltinImpl intel_sub_group_block_read_us4_16_avx512f = OclBuiltinImpl<intel_sub_group_block_read_us4_16, intel_sub_group_block_read_write_us_types, 0, BlockRead4_16>;

code BlockRead8_16 =
  [{
    const $ReturnType res = *($ReturnType*)($Arg0VarName);
    return __ocl_shuffle_transpose_$ReturnBaseType#_16x8(res);
  }];
OclBuiltinImpl intel_sub_group_block_read8_16_avx512f = OclBuiltinImpl<intel_sub_group_block_read8_16, [ v1u32 ], 0, BlockRead8_16>;
OclBuiltinImpl intel_sub_group_block_read_us8_16_avx512f = OclBuiltinImpl<intel_sub_group_block_read_us8_16, intel_sub_group_block_read_write_us_types, 0, BlockRead8_16>;

code BlockWrite1_4 =
  [{
    $Arg1BaseType#4 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 1, 2, 3);
    *($Arg1BaseType#4*)($Arg0VarName) = res;
  }];
OclBuiltinImpl intel_sub_group_block_write1_4_avx512f = OclBuiltinImpl<intel_sub_group_block_write1_4, [ v1u32 ], 0, BlockWrite1_4>;
OclBuiltinImpl intel_sub_group_block_write_us1_4_avx512f = OclBuiltinImpl<intel_sub_group_block_write_us1_4, intel_sub_group_block_read_write_us_types, 0, BlockWrite1_4>;

code BlockWrite2_4 =
  [{
    $Arg1BaseType#8 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 2, 4, 6,
                                                                              1, 3, 5, 7);
    *($Arg1BaseType#8*)($Arg0VarName) = res;
  }];
OclBuiltinImpl intel_sub_group_block_write2_4_avx512f = OclBuiltinImpl<intel_sub_group_block_write2_4, [ v1u32 ], 0, BlockWrite2_4>;
OclBuiltinImpl intel_sub_group_block_write_us2_4_avx512f = OclBuiltinImpl<intel_sub_group_block_write_us2_4, intel_sub_group_block_read_write_us_types, 0, BlockWrite2_4>;

code BlockWrite4_4 =
  [{
    $Arg1BaseType#16 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 4, 8,  12,
                                                                               1, 5, 9,  13,
                                                                               2, 6, 10, 14,
                                                                               3, 7, 11, 15);
    *($Arg1BaseType#16 *)($Arg0VarName) = res;
  }];
OclBuiltinImpl intel_sub_group_block_write4_4_avx512f = OclBuiltinImpl<intel_sub_group_block_write4_4, [ v1u32 ], 0, BlockWrite4_4>;
OclBuiltinImpl intel_sub_group_block_write_us4_4_avx512f = OclBuiltinImpl<intel_sub_group_block_write_us4_4, intel_sub_group_block_read_write_us_types, 0, BlockWrite4_4>;

code BlockWrite8_4 =
  [{
    $Arg1BaseType#32 res = __ocl_shuffle_transpose_$Arg1BaseType#_8x4($Arg1VarName);
    *($Arg1BaseType#32 *)($Arg0VarName) = res;
  }];
OclBuiltinImpl intel_sub_group_block_write8_4_avx512f = OclBuiltinImpl<intel_sub_group_block_write8_4, [ v1u32 ], 0, BlockWrite8_4>;
OclBuiltinImpl intel_sub_group_block_write_us8_4_avx512f = OclBuiltinImpl<intel_sub_group_block_write_us8_4, intel_sub_group_block_read_write_us_types, 0, BlockWrite8_4>;

code BlockWrite1_8 =
  [{
    $Arg1BaseType#8 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 1, 2, 3, 4, 5, 6, 7);
    *($Arg1BaseType#8 *)($Arg0VarName) = res;
  }];
OclBuiltinImpl intel_sub_group_block_write1_8_avx512f = OclBuiltinImpl<intel_sub_group_block_write1_8, [ v1u32 ], 0, BlockWrite1_8>;
OclBuiltinImpl intel_sub_group_block_write_us1_8_avx512f = OclBuiltinImpl<intel_sub_group_block_write_us1_8, intel_sub_group_block_read_write_us_types, 0, BlockWrite1_8>;

code BlockWrite2_8 =
  [{
    $Arg1BaseType#16 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 2, 4, 6, 8, 10, 12, 14,
                                                                               1, 3, 5, 7, 8, 11, 13, 15);
    *($Arg1BaseType#16 *)($Arg0VarName) = res;
  }];
OclBuiltinImpl intel_sub_group_block_write2_8_avx512f = OclBuiltinImpl<intel_sub_group_block_write2_8, [ v1u32 ], 0, BlockWrite2_8>;
OclBuiltinImpl intel_sub_group_block_write_us2_8_avx512f = OclBuiltinImpl<intel_sub_group_block_write_us2_8, intel_sub_group_block_read_write_us_types, 0, BlockWrite2_8>;

code BlockWrite4_8 =
  [{
    $Arg1BaseType#32 res = __ocl_shuffle_transpose_$Arg1BaseType#_4x8($Arg1VarName);
    *($Arg1BaseType#32 *)($Arg0VarName) = res;
  }];
OclBuiltinImpl intel_sub_group_block_write4_8_avx512f = OclBuiltinImpl<intel_sub_group_block_write4_8, [ v1u32 ], 0, BlockWrite4_8>;
OclBuiltinImpl intel_sub_group_block_write_us4_8_avx512f = OclBuiltinImpl<intel_sub_group_block_write_us4_8, intel_sub_group_block_read_write_us_types, 0, BlockWrite4_8>;

code BlockWrite8_8 =
  [{
    $Arg1BaseType#64 res = __ocl_shuffle_transpose_$Arg1BaseType#_8x8($Arg1VarName);
    *($Arg1BaseType#64 *)($Arg0VarName) = res;
  }];
OclBuiltinImpl intel_sub_group_block_write8_8_avx512f = OclBuiltinImpl<intel_sub_group_block_write8_8, [ v1u32 ], 0, BlockWrite8_8>;
OclBuiltinImpl intel_sub_group_block_write_us8_8_avx512f = OclBuiltinImpl<intel_sub_group_block_write_us8_8, intel_sub_group_block_read_write_us_types, 0, BlockWrite8_8>;

code BlockWrite1_16 =
  [{
    $Arg1BaseType#16 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    *($Arg1BaseType#16 *)($Arg0VarName) = res;
  }];
OclBuiltinImpl intel_sub_group_block_write1_16_avx512f = OclBuiltinImpl<intel_sub_group_block_write1_16, [ v1u32 ], 0, BlockWrite1_16>;
OclBuiltinImpl intel_sub_group_block_write_us1_16_avx512f = OclBuiltinImpl<intel_sub_group_block_write_us1_16, intel_sub_group_block_read_write_us_types, 0, BlockWrite1_16>;

code BlockWrite2_16 =
  [{
    $Arg1BaseType#32 res = __builtin_shufflevector($Arg1VarName, $Arg1VarName, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
                                                                               1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31);
    *($Arg1BaseType#32 *)($Arg0VarName) = res;
  }];
OclBuiltinImpl intel_sub_group_block_write2_16_avx512f = OclBuiltinImpl<intel_sub_group_block_write2_16, [ v1u32 ], 0, BlockWrite2_16>;
OclBuiltinImpl intel_sub_group_block_write_us2_16_avx512f = OclBuiltinImpl<intel_sub_group_block_write_us2_16, intel_sub_group_block_read_write_us_types, 0, BlockWrite2_16>;

code BlockWrite4_16 =
  [{
    $Arg1BaseType#64 res = __ocl_shuffle_transpose_$Arg1BaseType#_4x16($Arg1VarName);
    *($Arg1BaseType#64 *)($Arg0VarName) = res;
  }];
OclBuiltinImpl intel_sub_group_block_write4_16_avx512f = OclBuiltinImpl<intel_sub_group_block_write4_16, [ v1u32 ], 0, BlockWrite4_16>;
OclBuiltinImpl intel_sub_group_block_write_us4_16_avx512f = OclBuiltinImpl<intel_sub_group_block_write_us4_16, intel_sub_group_block_read_write_us_types, 0, BlockWrite4_16>;

code BlockWrite8_16 =
  [{
    $Arg1BaseType#128 res = __ocl_shuffle_transpose_$Arg1BaseType#_8x16($Arg1VarName);
    *($Arg1BaseType#128 *)($Arg0VarName) = res;
  }];
OclBuiltinImpl intel_sub_group_block_write8_16_avx512f = OclBuiltinImpl<intel_sub_group_block_write8_16, [ v1u32 ], 0, BlockWrite8_16>;
OclBuiltinImpl intel_sub_group_block_write_us8_16_avx512f = OclBuiltinImpl<intel_sub_group_block_write_us8_16, intel_sub_group_block_read_write_us_types, 0, BlockWrite8_16>;
