list<OclType> sub_group_types_all = [v1i32, v4i32, v8i32, v16i32,
                                     v1u32, v4u32, v8u32, v16u32,
                                     v1i64, v4i64, v8i64, v16i64,
                                     v1u64, v4u64, v8u64, v16u64,
                                     v1f32, v4f32, v8f32, v16f32,
                                     v1f64, v4f64, v8f64, v16f64];

//
// Reductions
//

// any/all begin
// VF 16 begin

OclBuiltinImpl sub_group_all_avx512fiu32 = OclBuiltinImpl<sub_group_all, [ v16i32 ], 0,
  [{
    __m512i zero = _mm512_set1_epi32(0);
    unsigned short res = _mm512_cmp_epi32_mask($Arg0VarName, zero, _MM_CMPINT_NE);
    return res == 65535;
  }]>;

OclBuiltinImpl sub_group_any_avx512fiu32 = OclBuiltinImpl<sub_group_any, [ v16i32 ], 0,
  [{
      __m512i zero = _mm512_set1_epi32(0);
      unsigned short res = _mm512_cmp_epi32_mask($Arg0VarName, zero, _MM_CMPINT_EQ);
      return res != 65535;
  }]>;

// VF 16 end
// VF 8 begin

OclBuiltinImpl sub_group_all_avx512fiu32_vf8 = OclBuiltinImpl<sub_group_all, [ v8i32 ], 0,
  [{
    __m256i zero = _mm256_set1_epi32(0);
    unsigned short res = _mm256_cmp_epi32_mask($Arg0VarName, zero, _MM_CMPINT_NE);
    return res == 65535;
  }]>;

OclBuiltinImpl sub_group_any_avx512fiu32_vf8 = OclBuiltinImpl<sub_group_any, [ v8i32 ], 0,
  [{
      __m256i zero = _mm256_set1_epi32(0);
      unsigned short res = _mm256_cmp_epi32_mask($Arg0VarName, zero, _MM_CMPINT_EQ);
      return res != 65535;
  }]>;

// VF 8 end
// any/all end

// reduce [add|min|max]
// add begin

OclBuiltinImpl sub_group_reduce_add_avx512fiu32 = OclBuiltinImpl<sub_group_reduce_add, [ v16i32, v16u32 ], 0,
  [{
    return _mm512_reduce_add_epi32((__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx512ff32 = OclBuiltinImpl<sub_group_reduce_add, [ v16f32 ], 0,
  [{
    return _mm512_reduce_add_ps((__m512)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx512fv8f64 = OclBuiltinImpl<sub_group_reduce_add, [ v8f64 ], 0,
  [{
    return _mm512_reduce_add_pd((__m512d)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx512fv8f32 = OclBuiltinImpl<sub_group_reduce_add, [ v8f32 ], 0,
  [{
    __mmask16 mask = 255;
    float16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_add_ps(mask, (__m512)num);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx512fv8iu64 = OclBuiltinImpl<sub_group_reduce_add, [ v8i64, v8u64 ], 0,
  [{
    return _mm512_reduce_add_epi64((__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx512fv8iu32 = OclBuiltinImpl<sub_group_reduce_add, [ v8i32, v8u32 ], 0,
  [{
    __mmask16 mask = 255;
    $Arg0BaseType#16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_add_epi32(mask, (__m512i)num);
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx512fv16iu64 = OclBuiltinImpl<sub_group_reduce_add, [ v16i64, v16u64 ], 0,
  [{
    return _mm512_reduce_add_epi64(*(__m512i*)&$Arg0VarName) + _mm512_reduce_add_epi64(*(__m512i*)(&$Arg0VarName+8));
  }]>;

OclBuiltinImpl sub_group_reduce_add_avx512fv16f64 = OclBuiltinImpl<sub_group_reduce_add, [ v16f64 ], 0,
  [{
    return _mm512_reduce_add_pd(*(__m512d*)&$Arg0VarName) + _mm512_reduce_add_pd(*(__m512d*)(&$Arg0VarName+8));
  }]>;

// add end
// min begin

OclBuiltinImpl sub_group_reduce_min_avx512fi32 = OclBuiltinImpl<sub_group_reduce_min, [ v16i32 ], 0,
  [{
    return _mm512_reduce_min_epi32((__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fu32 = OclBuiltinImpl<sub_group_reduce_min, [ v16u32 ], 0,
  [{
    return _mm512_reduce_min_epu32((__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fi32vf8 = OclBuiltinImpl<sub_group_reduce_min, [ v8i32 ], 0,
  [{
    __mmask16 mask = 255;
    int16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_min_epi32(mask, (__m512i)num);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fu32vf8 = OclBuiltinImpl<sub_group_reduce_min, [ v8u32 ], 0,
  [{
    __mmask16 mask = 255;
    uint16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_min_epu32(mask, (__m512i)num);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512ff32 = OclBuiltinImpl<sub_group_reduce_min, [ v16f32 ], 0,
  [{
    return _mm512_reduce_min_ps((__m512)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512ff32iv8 = OclBuiltinImpl<sub_group_reduce_min, [ v8f32 ], 0,
  [{
    __mmask16 mask = 255;
    float16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_min_ps(mask, (__m512)num);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fv8i64 = OclBuiltinImpl<sub_group_reduce_min, [ v8i64 ], 0,
  [{
    return _mm512_reduce_min_epi64((__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fv16i64 = OclBuiltinImpl<sub_group_reduce_min, [ v16i64 ], 0,
  [{
    long a = _mm512_reduce_min_epi64(*((__m512i*)&$Arg0VarName));
    long b = _mm512_reduce_min_epi64(*((__m512i*)(&$Arg0VarName+8)));
    return a < b ? a : b;
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fv8u64 = OclBuiltinImpl<sub_group_reduce_min, [ v8u64 ], 0,
  [{
    return _mm512_reduce_min_epu64((__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fv16u64 = OclBuiltinImpl<sub_group_reduce_min, [ v16u64 ], 0,
  [{
    unsigned long a = _mm512_reduce_min_epu64(*((__m512i*)&$Arg0VarName));
    unsigned long b = _mm512_reduce_min_epu64(*((__m512i*)(&$Arg0VarName+8)));
    return a < b ? a : b;
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fv8f64 = OclBuiltinImpl<sub_group_reduce_min, [ v8f64 ], 0,
  [{
    return _mm512_reduce_min_pd((__m512d)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fv16f64 = OclBuiltinImpl<sub_group_reduce_min, [ v16f64 ], 0,
  [{
    double a = _mm512_reduce_min_pd(*((__m512d*)&$Arg0VarName));
    double b = _mm512_reduce_min_pd(*((__m512d*)(&$Arg0VarName+8)));
    return a < b ? a : b;
  }]>;

// min end
// max begin

OclBuiltinImpl sub_group_reduce_max_avx512fi32 = OclBuiltinImpl<sub_group_reduce_max, [ v16i32 ], 0,
  [{
    return _mm512_reduce_max_epi32((__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512fu32 = OclBuiltinImpl<sub_group_reduce_max, [ v16u32 ], 0,
  [{
    return _mm512_reduce_max_epu32((__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512fi32vf8 = OclBuiltinImpl<sub_group_reduce_max, [ v8i32 ], 0,
  [{
    __mmask16 mask = 255;
    int16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_max_epi32(mask, (__m512i)num);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512fu32vf8 = OclBuiltinImpl<sub_group_reduce_max, [ v8u32 ], 0,
  [{
    __mmask16 mask = 255;
    uint16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_max_epu32(mask, (__m512i)num);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512ff32 = OclBuiltinImpl<sub_group_reduce_max, [ v16f32 ], 0,
  [{
    return _mm512_reduce_max_ps((__m512)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512ff32vf8 = OclBuiltinImpl<sub_group_reduce_max, [ v8f32 ], 0,
  [{
    __mmask16 mask = 255;
    float16 num;
    num.lo = $Arg0VarName;
    return _mm512_mask_reduce_max_ps(mask, (__m512)num);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512fv8i64 = OclBuiltinImpl<sub_group_reduce_max, [ v8i64 ], 0,
  [{
    return _mm512_reduce_max_epi64((__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512fv16i64 = OclBuiltinImpl<sub_group_reduce_max, [ v16i64 ], 0,
  [{
    long a = _mm512_reduce_max_epi64(*((__m512i*)&$Arg0VarName));
    long b = _mm512_reduce_max_epi64(*((__m512i*)(&$Arg0VarName+8)));
    return a > b ? a : b;
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512fv8u64 = OclBuiltinImpl<sub_group_reduce_max, [ v8u64 ], 0,
  [{
    return _mm512_reduce_max_epu64((__m512i)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fv16u64 = OclBuiltinImpl<sub_group_reduce_min, [ v16u64 ], 0,
  [{
    unsigned long a = _mm512_reduce_max_epu64(*((__m512i*)&$Arg0VarName));
    unsigned long b = _mm512_reduce_max_epu64(*((__m512i*)(&$Arg0VarName+8)));
    return a > b ? a : b;
  }]>;

OclBuiltinImpl sub_group_reduce_max_avx512fv8f64 = OclBuiltinImpl<sub_group_reduce_max, [ v8f64 ], 0,
  [{
    return _mm512_reduce_max_pd((__m512d)$Arg0VarName);
  }]>;

OclBuiltinImpl sub_group_reduce_min_avx512fv16f64 = OclBuiltinImpl<sub_group_reduce_min, [ v16f64 ], 0,
  [{
    double a = _mm512_reduce_min_pd(*((__m512d*)&$Arg0VarName));
    double b = _mm512_reduce_min_pd(*((__m512d*)(&$Arg0VarName+8)));
    return a > b ? a : b;
  }]>;

// max end

// inclusive [add|min|max]
// inclusive scan add begin
OclBuiltinImpl sub_group_scan_inclusive_add_avx512fiu32 = OclBuiltinImpl<sub_group_scan_inclusive_add, [ v16i32, v16u32 ], 0,
  [{
    unsigned short mask = 0x01;
    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);

    __v16si rotated = *((__v16si *)&$Arg0VarName);
    __v16si res = rotated;

    #pragma clang unroll(full)
    for (int i = 0; i < 15; i++) {
      rotated = (__v16si)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
      mask <<= 1;
      res = (__v16si)_mm512_mask_add_epi32((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx512fiu32vf8 = OclBuiltinImpl<sub_group_scan_inclusive_add, [ v8i32, v8u32 ], 0,
  [{
    unsigned short mask = 0x01;
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);

    __v8si rotated = *((__v8si *)&$Arg0VarName);
    __v8si res = rotated;

    #pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
      rotated = (__v8si)_mm256_permutexvar_epi32(rotate_mask, (__m256i)res);
      mask <<= 1;
      res = (__v8si)_mm256_mask_add_epi32((__m256i)res, mask, (__m256i)res, (__m256i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx512ff32 = OclBuiltinImpl<sub_group_scan_inclusive_add, [ v16f32 ], 0,
  [{
    unsigned short mask = 0x01;
    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);

    __v16sf rotated = *((__v16sf *)&$Arg0VarName);
    __v16sf res = rotated;

    #pragma clang unroll(full)
    for (int i = 0; i < 15; i++) {
      rotated = (__v16sf)_mm512_permutexvar_ps(rotate_mask, (__m512)res);
      mask <<= 1;
      res = (__v16sf)_mm512_mask_add_ps((__m512)res, mask, (__m512)res, (__m512)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx512ff32vf8 = OclBuiltinImpl<sub_group_scan_inclusive_add, [ v8f32 ], 0,
  [{
    unsigned short mask = 0x01;
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);

    __v8sf rotated = *((__v8sf *)&$Arg0VarName);
    __v8sf res = rotated;

    #pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
      rotated = (__v8sf)_mm256_permutexvar_ps(rotate_mask, (__m256)res);
      mask <<= 1;
      res = (__v8sf)_mm256_mask_add_ps((__m256)res, mask, (__m256)res, (__m256)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_add_avx512fv16iu64 = OclBuiltinImpl<sub_group_scan_inclusive_add, [ v16i64, v16u64 ], 0,
  [{
    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    ulong16 res;

    __v8du rotated;
    __v8du res_lo = *(__v8du*)(&$Arg0VarName);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_lo);
        mask <<= 1;
        res_lo = (__v8du)_mm512_mask_add_epi64((__m512i)res_lo, mask, (__m512i)res_lo, (__m512i)rotated);
    }

    __v8du res_hi = *(__v8du*)((ulong*)&$Arg0VarName + 8);
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

OclBuiltinImpl sub_group_scan_inclusive_add_avx512fv16ff64 = OclBuiltinImpl<sub_group_scan_inclusive_add, [ v16f64 ], 0,
  [{
    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    double16 res;

    __v8df rotated;
    __v8df res_lo = *(__v8df*)(&$Arg0VarName);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res_lo);
        mask <<= 1;
        res_lo = (__v8df)_mm512_mask_add_pd((__m512d)res_lo, mask, (__m512d)res_lo, (__m512d)rotated);
    }

    __v8df res_hi = *(__v8df*)((double*)&$Arg0VarName + 8);
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
// inclusive add end
// inclusive min begin
OclBuiltinImpl sub_group_scan_inclusive_min_avx512fi32 = OclBuiltinImpl<sub_group_scan_inclusive_min, [ v16i32 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16si res = *((__v16si *)&$Arg0VarName);
    __v16si rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 15; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v16si)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
        res = (__v16si)_mm512_mask_min_epi32((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fu32 = OclBuiltinImpl<sub_group_scan_inclusive_min, [ v16u32 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16su res = *((__v16su *)&$Arg0VarName);
    __v16su rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 15; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v16su)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
        res = (__v16su)_mm512_mask_min_epu32((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fi32vf8 = OclBuiltinImpl<sub_group_scan_inclusive_min, [ v8i32 ], 0,
  [{
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8si res = *((__v8si *)&$Arg0VarName);
    __v8si rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8si)_mm256_permutexvar_epi32(rotate_mask, (__m256i)res);
        res = (__v8si)_mm256_mask_min_epi32((__m256i)res, mask, (__m256i)res, (__m256i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fu32vf8 = OclBuiltinImpl<sub_group_scan_inclusive_min, [ v8u32 ], 0,
  [{
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8su res = *((__v8su *)&$Arg0VarName);
    __v8su rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8su)_mm256_permutexvar_epi32(rotate_mask, (__m256i)res);
        res = (__v8su)_mm256_mask_min_epu32((__m256i)res, mask, (__m256i)res, (__m256i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fu32 = OclBuiltinImpl<sub_group_scan_inclusive_min, [ v16f32 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16sf res = *((__v16sf *)&$Arg0VarName);
    __v16sf rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 15; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v16sf)_mm512_permutexvar_ps(rotate_mask, (__m512)res);
        res = (__v16sf)_mm512_mask_min_ps((__m512)res, mask, (__m512)res, (__m512)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fu32vf8 = OclBuiltinImpl<sub_group_scan_inclusive_min, [ v8f32 ], 0,
  [{
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8sf res = *((__v8sf *)&$Arg0VarName);
    __v8sf rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8sf)_mm256_permutexvar_ps(rotate_mask, (__m256)res);
        res = (__v8sf)_mm256_mask_min_ps((__m256)res, mask, (__m256)res, (__m256)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fv8u64 = OclBuiltinImpl<sub_group_scan_inclusive_min, [ v8u64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8du res = *((__v8du *)&$Arg0VarName);
    __v8du rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res);
        res = (__v8du)_mm512_mask_min_epu64((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fv8i64 = OclBuiltinImpl<sub_group_scan_inclusive_min, [ v8i64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8di res = *((__v8di *)&$Arg0VarName);
    __v8di rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8di)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
        res = (__v8di)_mm512_mask_min_epu64((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512v8f64 = OclBuiltinImpl<sub_group_scan_inclusive_min, [ v8f64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8df res = *((__v8df *)&$Arg0VarName);
    __v8df rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 2 iterations
        mask <<= 1;
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res);
        res = (__v8df)_mm512_mask_min_pd((__m512d)res, mask, (__m512d)res, (__m512d)rotated);
    }
    return *(($ReturnType *)&res);
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fv16ff64 = OclBuiltinImpl<sub_group_scan_inclusive_min, [ v16f64 ], 0,
  [{
    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    double16 res;

    __v8df rotated;
    __v8df res_lo = *(__v8df*)(&$Arg0VarName);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res_lo);
        mask <<= 1;
        res_lo = (__v8df)_mm512_mask_min_pd((__m512d)res_lo, mask, (__m512d)res_lo, (__m512d)rotated);
    }

    __v8df res_hi = *(__v8df*)((double*)&$Arg0VarName + 8);
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

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fv16u64 = OclBuiltinImpl<sub_group_scan_inclusive_min, [ v16u64 ], 0,
  [{
    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    ulong16 res;

    __v8du rotated;
    __v8du res_lo = *(__v8du*)(&$Arg0VarName);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_lo);
        mask <<= 1;
        res_lo = (__v8du)_mm512_mask_min_epu64((__m512i)res_lo, mask, (__m512i)res_lo, (__m512i)rotated);
    }

    __v8du res_hi = *(__v8du*)((long*)&$Arg0VarName + 8);
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

OclBuiltinImpl sub_group_scan_inclusive_min_avx512fv16i64 = OclBuiltinImpl<sub_group_scan_inclusive_min, [ v16i64 ], 0,
  [{
    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    ulong16 res;

    __v8di rotated;
    __v8di res_lo = *(__v8di*)(&$Arg0VarName);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_lo);
        mask <<= 1;
        res_lo = (__v8di)_mm512_mask_min_epi64((__m512i)res_lo, mask, (__m512i)res_lo, (__m512i)rotated);
    }

    __v8di res_hi = *(__v8di*)((long*)&$Arg0VarName + 8);
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
// inclusive min end
// inclusive max begin
OclBuiltinImpl sub_group_scan_inclusive_max_avx512i32 = OclBuiltinImpl<sub_group_scan_inclusive_max, [ v16i32 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16si res = *((__v16si *)&$Arg0VarName);
    __v16si rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 15; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v16si)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
        res = (__v16si)_mm512_mask_max_epi32((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return res;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512u32 = OclBuiltinImpl<sub_group_scan_inclusive_max, [ v16u32 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16su res = *((__v16su *)&$Arg0VarName);
    __v16su rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 15; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v16su)_mm512_permutexvar_epi32(rotate_mask, (__m512i)res);
        res = (__v16su)_mm512_mask_max_epu32((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return res;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512i32vf8 = OclBuiltinImpl<sub_group_scan_inclusive_max, [ v8i32 ], 0,
  [{
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8si res = *((__v8si *)&$Arg0VarName);
    __v8si rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8si)_mm256_permutexvar_epi32(rotate_mask, (__m256i)res);
        res = (__v8si)_mm256_mask_max_epi32((__m256i)res, mask, (__m256i)res, (__m256i)rotated);
    }
    return res;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512u32vf8 = OclBuiltinImpl<sub_group_scan_inclusive_max, [ v8u32 ], 0,
  [{
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8su res = *((__v8su *)&$Arg0VarName);
    __v8su rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8su)_mm256_permutexvar_epi32(rotate_mask, (__m256i)res);
        res = (__v8su)_mm256_mask_max_epu32((__m256i)res, mask, (__m256i)res, (__m256i)rotated);
    }
    return res;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512f32 = OclBuiltinImpl<sub_group_scan_inclusive_max, [ v16f32 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16sf res = *((__v16sf *)&$Arg0VarName);
    __v16sf rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 15; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v16sf)_mm512_permutexvar_ps(rotate_mask, (__m512)res);
        res = (__v16sf)_mm512_mask_max_ps((__m512)res, mask, (__m512)res, (__m512)rotated);
    }
    return res;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512f32vf8 = OclBuiltinImpl<sub_group_scan_inclusive_max, [ v8f32 ], 0,
  [{
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8sf res = *((__v8sf *)&$Arg0VarName);
    __v8sf rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8sf)_mm256_permutexvar_ps(rotate_mask, (__m256)res);
        res = (__v8sf)_mm256_mask_max_ps((__m256)res, mask, (__m256)res, (__m256)rotated);
    }
    return res;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512v8u64 = OclBuiltinImpl<sub_group_scan_inclusive_max, [ v8u64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8du res = *((__v8du *)&$Arg0VarName);
    __v8du rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res);
        res = (__v8du)_mm512_mask_max_epu64((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return res;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512v8u64 = OclBuiltinImpl<sub_group_scan_inclusive_max, [ v8i64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8di res = *((__v8di *)&$Arg0VarName);
    __v8di rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res);
        res = (__v8di)_mm512_mask_max_epi64((__m512i)res, mask, (__m512i)res, (__m512i)rotated);
    }
    return res;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512v8u64 = OclBuiltinImpl<sub_group_scan_inclusive_max, [ v8f64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8df res = *((__v8df *)&$Arg0VarName);
    __v8df rotated = res;

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) { // VF - 1 iterations
        mask <<= 1;
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res);
        res = (__v8df)_mm512_mask_max_pd((__m512d)res, mask, (__m512d)res, (__m512d)rotated);
    }
    return res;
  }]>;

OclBuiltinImpl sub_group_scan_inclusive_max_avx512fv16f64 = OclBuiltinImpl<sub_group_scan_inclusive_max, [ v16f64 ], 0,
  [{
    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    double16 res;

    __v8df rotated;
    __v8df res_lo = *(__v8df*)(&$Arg0VarName);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)res_lo);
        mask <<= 1;
        res_lo = (__v8df)_mm512_mask_max_pd((__m512d)res_lo, mask, (__m512d)res_lo, (__m512d)rotated);
    }

    __v8df res_hi = *(__v8df*)((double*)&$Arg0VarName + 8);
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

OclBuiltinImpl sub_group_scan_inclusive_max_avx512fv16u64 = OclBuiltinImpl<sub_group_scan_inclusive_max, [ v16u64 ], 0,
  [{
    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    ulong16 res;

    __v8du rotated;
    __v8du res_lo = *(__v8du*)(&$Arg0VarName);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_lo);
        mask <<= 1;
        res_lo = (__v8du)_mm512_mask_max_epu64((__m512i)res_lo, mask, (__m512i)res_lo, (__m512i)rotated);
    }

    __v8du res_hi = *(__v8du*)((long*)&$Arg0VarName + 8);
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

OclBuiltinImpl sub_group_scan_inclusive_max_avx512fv16i64 = OclBuiltinImpl<sub_group_scan_inclusive_max, [ v16i64 ], 0,
  [{
    unsigned short mask = 1;
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);

    ulong16 res;

    __v8di rotated;
    __v8di res_lo = *(__v8di*)(&$Arg0VarName);

#pragma clang unroll(full)
    for (int i = 0; i < 7; i++) {
        rotated = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)res_lo);
        mask <<= 1;
        res_lo = (__v8di)_mm512_mask_max_epi64((__m512i)res_lo, mask, (__m512i)res_lo, (__m512i)rotated);
    }

    __v8di res_hi = *(__v8di*)((long*)&$Arg0VarName + 8);
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
// inclusive max end
// exclusive [add|min|max]
// exclusive add begin
OclBuiltinImpl sub_group_scan_exclusive_add_avx512fui32 = OclBuiltinImpl<sub_group_scan_exclusive_add, [ v16i32, v16u32 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16si res = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = $Arg0VarName[0];
    __v16si rotated_d = (__v16si)_mm512_permutexvar_epi32(rotate_mask, (__m512i)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_add_avx512ff32 = OclBuiltinImpl<sub_group_scan_exclusive_add, [ v16f32 ], 0,
  [{
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

OclBuiltinImpl sub_group_scan_exclusive_add_avx512fui32vf8 = OclBuiltinImpl<sub_group_scan_exclusive_add, [ v8i32, v8u32 ], 0,
  [{
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8si res = {0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = $Arg0VarName[0];
    __v8si rotated_d = (__v8si)_mm256_permutexvar_epi32(rotate_mask, (__m256i)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_add_avx512ff32v8 = OclBuiltinImpl<sub_group_scan_exclusive_add, [ v8f32 ], 0,
  [{
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8sf res = {0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = $Arg0VarName[0];
    __v8sf rotated_d = (__v8sf)_mm256_permutexvar_ps(rotate_mask, (__m256)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_add_avx512fv8ui64 = OclBuiltinImpl<sub_group_scan_exclusive_add, [ v8i64, v8u64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8di res;
    res[1] = $Arg0VarName[0];
    __v8di rotated_d = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_add_avx512fv8f64 = OclBuiltinImpl<sub_group_scan_exclusive_add, [ v8f64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8df res;
    res[1] = $Arg0VarName[0];
    __v8df rotated_d = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_add_avx512v16ui64 = OclBuiltinImpl<sub_group_scan_exclusive_add, [ v16u64, v16i64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    ulong16 res;
    __v8du res_lo;
    __v8du res_hi;
    __v8du d_lo = *(__v8du*)(&$Arg0VarName);
    __v8du d_hi = *(__v8du*)((ulong*)(&$Arg0VarName) + 8);
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
    _mm512_store_ps((void*)((ulong*)&res + 8), (__m512)res_hi);

    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_add_avx512v16f64 = OclBuiltinImpl<sub_group_scan_exclusive_add, [ v16f64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    double16 res;
    __v8df res_lo;
    __v8df res_hi;
    __v8df d_lo = *(__v8df*)(&$Arg0VarName);
    __v8df d_hi = *(__v8df*)((double*)(&$Arg0VarName) + 8);
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
    _mm512_store_ps((void*)((ulong*)&res + 8), (__m512)res_hi);

    return as_$ReturnType(res);
  }]>;
// exclusive add end
// exclusive min begin
OclBuiltinImpl sub_group_scan_exclusive_min_avx512fui32 = OclBuiltinImpl<sub_group_scan_exclusive_min, [ v16u32 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16su res = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = $Arg0VarName[0];
    __v16su rotated_d = (__v16su)_mm512_permutexvar_epi32(rotate_mask, (__m512i)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_min_avx512fi32 = OclBuiltinImpl<sub_group_scan_exclusive_min, [ v16i32 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16si res = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = $Arg0VarName[0];
    __v16si rotated_d = (__v16si)_mm512_permutexvar_epi32(rotate_mask, (__m512i)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_min_avx512ff32 = OclBuiltinImpl<sub_group_scan_exclusive_min, [ v16f32 ], 0,
  [{
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
        res = (__v16sf)_mm512_mask_min_ps((__m512)res, mask, (__m512)rotated, (__m512)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx512fui32vf8 = OclBuiltinImpl<sub_group_scan_exclusive_min, [ v8u32 ], 0,
  [{
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8su res = {0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = $Arg0VarName[0];
    __v8su rotated_d = (__v8su)_mm256_permutexvar_epi32(rotate_mask, (__m256i)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_min_avx512fi32vf8 = OclBuiltinImpl<sub_group_scan_exclusive_min, [ v8i32 ], 0,
  [{
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8si res = {0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = $Arg0VarName[0];
    __v8si rotated_d = (__v8si)_mm256_permutexvar_epi32(rotate_mask, (__m256i)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_min_avx512ff32vf8 = OclBuiltinImpl<sub_group_scan_exclusive_min, [ v8f32 ], 0,
  [{
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8sf res = {0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = $Arg0VarName[0];
    __v8sf rotated_d = (__v8sf)_mm256_permutexvar_ps(rotate_mask, (__m256)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_min_avx512fv8u64 = OclBuiltinImpl<sub_group_scan_exclusive_min, [ v8u64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8du res;
    res[1] = $Arg0VarName[0];
    __v8du rotated_d = (__v8du)_mm512_permutexvar_epi64(rotate_mask, (__m512i)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_min_avx512fv8i64 = OclBuiltinImpl<sub_group_scan_exclusive_min, [ v8i64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8di res;
    res[1] = $Arg0VarName[0];
    __v8di rotated_d = (__v8di)_mm512_permutexvar_epi64(rotate_mask, (__m512i)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_min_avx512fv8f64 = OclBuiltinImpl<sub_group_scan_exclusive_min, [ v8f64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8df res;
    res[1] = $Arg0VarName[0];
    __v8df rotated_d = (__v8df)_mm512_permutexvar_pd(rotate_mask, (__m512d)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_min_avx512v16u64 = OclBuiltinImpl<sub_group_scan_exclusive_min, [ v16u64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    ulong16 res;
    __v8du res_lo;
    __v8du res_hi;
    __v8du d_lo = *(__v8du*)(&$Arg0VarName);
    __v8du d_hi = *(__v8du*)((ulong*)(&$Arg0VarName) + 8);
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
    _mm512_store_ps((void*)((ulong*)&res + 8), (__m512)res_hi);

    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx512v16f64 = OclBuiltinImpl<sub_group_scan_exclusive_min, [ v16i64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    ulong16 res;
    __v8di res_lo;
    __v8di res_hi;
    __v8di d_lo = *(__v8di*)(&$Arg0VarName);
    __v8di d_hi = *(__v8di*)((ulong*)(&$Arg0VarName) + 8);
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
    _mm512_store_ps((void*)((ulong*)&res + 8), (__m512)res_hi);

    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_avx512v16f64 = OclBuiltinImpl<sub_group_scan_exclusive_min, [ v16f64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    double16 res;
    __v8df res_lo;
    __v8df res_hi;
    __v8df d_lo = *(__v8df*)(&$Arg0VarName);
    __v8df d_hi = *(__v8df*)((double*)(&$Arg0VarName) + 8);
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
    _mm512_store_ps((void*)((ulong*)&res + 8), (__m512)res_hi);

    return as_$ReturnType(res);
  }]>;
// exclusive min end
// exclusive max begin
OclBuiltinImpl sub_group_scan_exclusive_max_avx512fi32 = OclBuiltinImpl<sub_group_scan_exclusive_max, [ v16i32 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16si res = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = $Arg0VarName[0];
    __v16si rotated_d = (__v16si)_mm512_permutexvar_epi32(rotate_mask, (__m512i)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_max_avx512fu32 = OclBuiltinImpl<sub_group_scan_exclusive_max, [ v16u32 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi32(15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    unsigned short mask = 1;

    __v16su res = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = $Arg0VarName[0];
    __v16su rotated_d = (__v16su)_mm512_permutexvar_epi32(rotate_mask, (__m512i)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_max_avx512ff32 = OclBuiltinImpl<sub_group_scan_exclusive_max, [ v16f32 ], 0,
  [{
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
        res = (__v16sf)_mm512_mask_max_ps((__m512)res, mask, (__m512)rotated, (__m512)rotated_d);
    }
    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx512fi32vf8 = OclBuiltinImpl<sub_group_scan_exclusive_max, [ v8i32 ], 0,
  [{
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8si res = {0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = $Arg0VarName[0];
    __v8si rotated_d = (__v8si)_mm256_permutexvar_epi32(rotate_mask, (__m256i)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_max_avx512fu32vf8 = OclBuiltinImpl<sub_group_scan_exclusive_max, [ v8u32 ], 0,
  [{
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8su res = {0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = $Arg0VarName[0];
    __v8su rotated_d = (__v8su)_mm256_permutexvar_epi32(rotate_mask, (__m256i)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_max_avx512ff32vf8 = OclBuiltinImpl<sub_group_scan_exclusive_max, [ v8f32 ], 0,
  [{
    const __m256i rotate_mask = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    __v8sf res = {0, 0, 0, 0, 0, 0, 0, 0};
    res[1] = $Arg0VarName[0];
    __v8sf rotated_d = (__v8sf)_mm256_permutexvar_ps(rotate_mask, (__m256)$Arg0VarName);
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

OclBuiltinImpl sub_group_scan_exclusive_max_avx512v16u64 = OclBuiltinImpl<sub_group_scan_exclusive_max, [ v16u64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    ulong16 res;
    __v8du res_lo;
    __v8du res_hi;
    __v8du d_lo = *(__v8du*)(&$Arg0VarName);
    __v8du d_hi = *(__v8du*)((ulong*)(&$Arg0VarName) + 8);
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
    _mm512_store_ps((void*)((ulong*)&res + 8), (__m512)res_hi);

    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx512v16f64 = OclBuiltinImpl<sub_group_scan_exclusive_max, [ v16i64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    ulong16 res;
    __v8di res_lo;
    __v8di res_hi;
    __v8di d_lo = *(__v8di*)(&$Arg0VarName);
    __v8di d_hi = *(__v8di*)((ulong*)(&$Arg0VarName) + 8);
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
    _mm512_store_ps((void*)((ulong*)&res + 8), (__m512)res_hi);

    return as_$ReturnType(res);
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_avx512v16f64 = OclBuiltinImpl<sub_group_scan_exclusive_max, [ v16f64 ], 0,
  [{
    const __m512i rotate_mask = _mm512_setr_epi64(7, 0, 1, 2, 3, 4, 5, 6);
    unsigned short mask = 1;

    double16 res;
    __v8df res_lo;
    __v8df res_hi;
    __v8df d_lo = *(__v8df*)(&$Arg0VarName);
    __v8df d_hi = *(__v8df*)((double*)(&$Arg0VarName) + 8);
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
    _mm512_store_ps((void*)((ulong*)&res + 8), (__m512)res_hi);

    return as_$ReturnType(res);
  }]>;
// exclusive max end

