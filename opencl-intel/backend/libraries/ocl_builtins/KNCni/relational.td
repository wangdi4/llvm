// vim:ts=2:sw=2:et:
// Copyright (c) 2006-2012 Intel Corporation
// Relational
// OpenCL 1.2 rev 15, Table 6.14

// islessgreater
code islessgreater_scalar_code = [{
    return (($Arg0VarName < $Arg1VarName) || ($Arg0VarName > $Arg1VarName))?1:0;
  }];
OclBuiltinImpl islessgreater_scalar = OclBuiltinImpl<islessgreater, [v1f32, v1f64], 0, islessgreater_scalar_code>;
OclBuiltinImpl islessgreater_f32_v16 = OclBuiltinImpl<islessgreater, [v16f32], 0, [{
    __mmask16 isless = _mm512_cmplt_ps($Arg0VarName, $Arg1VarName);
    __mmask16 isgreater = _mm512_cmplt_ps($Arg1VarName, $Arg0VarName);
    isless = _mm512_kor(isless, isgreater);
    return mask2pi(isless);
  }]>;
OclBuiltinImpl islessgreater_f32_promote = OclBuiltinImpl<islessgreater, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote2>;
OclBuiltinImpl islessgreater_f64_v8 = OclBuiltinImpl<islessgreater, [v8f64], 0, [{
    __mmask8 isless = _mm512_cmplt_pd($Arg0VarName, $Arg1VarName);
    __mmask8 isgreater = _mm512_cmplt_pd($Arg1VarName, $Arg0VarName);
    isless = _mm512_kor(isless, isgreater);
    return mask2pq(isless);
  }]>;
OclBuiltinImpl islessgreater_f64_promote = OclBuiltinImpl<islessgreater, [v2f64, v3f64, v4f64], 0, NonMaskPromote2>;
OclBuiltinImpl islessgreater_f64_expand = OclBuiltinImpl<islessgreater, [v16f64], 0, Expand2>;

// isfinite
OclBuiltinImpl isfinite_f32_v1 = OclBuiltinImpl<isfinite, [v1f32], 0, [{
    return ((as_int($Arg0VarName) & const_fp_exp) < const_fp_exp)?1:0;
  }]>;
OclBuiltinImpl isfinite_f32_v16 = OclBuiltinImpl<isfinite, [v16f32], 0, [{
    __m512i const_vector_exp = (__m512i)((int16)const_fp_exp);
    __m512i no_sign = _mm512_and_epi32((__m512i)$Arg0VarName, const_vector_exp);
    __mmask16 isfinite = _mm512_cmplt_epi32(no_sign, const_vector_exp);
    return mask2pi(isfinite);
  }]>;
OclBuiltinImpl isfinite_f32_promote = OclBuiltinImpl<isfinite, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote1>;
OclBuiltinImpl isfinite_f64_v1 = OclBuiltinImpl<isfinite, [v1f64], 0, [{
    return ((as_long($Arg0VarName) & const_dp_exp) < const_dp_exp)?1:0;
  }]>;

OclBuiltinImpl isfinite_f64_v8 = OclBuiltinImpl<isfinite, [v8f64], 0, [{
    __m512i const_vector_exp = (__m512i)((long8)const_dp_exp);
    __m512i no_sign = _mm512_and_epi64((__m512i)$Arg0VarName, const_vector_exp);
    __mmask8 isfinite = _mm512_cmplt_epi64(no_sign, const_vector_exp);
    return ($NativeReturnType)mask2pq(isfinite);
  }]>;
OclBuiltinImpl isfinite_f64_promote = OclBuiltinImpl<isfinite, [v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl isfinite_f64_expand = OclBuiltinImpl<isfinite, [v16f64], 0, Expand1>;

// isinf
OclBuiltinImpl isinf_f32_v1 = OclBuiltinImpl<isinf, [v1f32], 0, [{
    return ((as_int($Arg0VarName) & const_fp_no_sign) == const_fp_exp)?1:0;
  }]>;
OclBuiltinImpl isinf_f32_v16 = OclBuiltinImpl<isinf, [v16f32], 0, [{
    __m512i no_sign = _mm512_and_epi32((__m512i)($Arg0VarName), (__m512i)((int16)const_fp_no_sign));
    __mmask16 inf = _mm512_cmpeq_epi32(no_sign, (__m512i)((int16)const_fp_exp));
    return mask2pi(inf);
  }]>;
OclBuiltinImpl isinf_f32_promote = OclBuiltinImpl<isinf, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote1>;
OclBuiltinImpl isinf_f64_v1 = OclBuiltinImpl<isinf, [v1f64], 0, [{
    return ((as_long($Arg0VarName) & const_dp_no_sign) == const_dp_exp)?1:0;
  }]>;
OclBuiltinImpl isinf_f64_v8 = OclBuiltinImpl<isinf, [v8f64], 0, [{
    __m512i no_sign = _mm512_and_epi64((__m512i)($Arg0VarName), (__m512i)((long8)const_dp_no_sign));
    __mmask8 inf = _mm512_cmpeq_epi64(no_sign, (__m512i)((long8)const_dp_exp));
    return mask2pq(inf);
  }]>;
OclBuiltinImpl isinf_f64_promote = OclBuiltinImpl<isinf, [v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl isinf_f64_expand = OclBuiltinImpl<isinf, [v16f64], 0, Expand1>;

// isnan
OclBuiltinImpl isnan_f32_v1 = OclBuiltinImpl<isnan, [v1f32], 0, [{
    return ((as_int($Arg0VarName) & const_fp_no_sign) > const_fp_exp)?1:0;
  }]>;
OclBuiltinImpl isnan_f32_v16 = OclBuiltinImpl<isnan, [v16f32], 0, [{
    __m512i no_sign = _mm512_and_epi32((__m512i)($Arg0VarName), (__m512i)((int16)const_fp_no_sign));
    __mmask16 nan = _mm512_cmpnle_epi32(no_sign, (__m512i)((int16)const_fp_exp));
    return mask2pi(nan);
  }]>;
OclBuiltinImpl isnan_f32_promote = OclBuiltinImpl<isnan, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote1>;

// isnormal
OclBuiltinImpl isnormal_f32_v1 = OclBuiltinImpl<isnormal, [v1f32], 0, [{
    return ( ((as_int($Arg0VarName) & const_fp_exp)!=0) && ( (as_int($Arg0VarName) & const_fp_exp) != const_fp_exp) )?1:0;
  }]>;
OclBuiltinImpl isnormal_f32_v16 = OclBuiltinImpl<isnormal, [v16f32], 0, [{
    __m512i const_vector_exp = (__m512i)((int16)const_fp_exp);
    __m512i no_sign = _mm512_and_epi32((__m512i)($Arg0VarName), const_vector_exp);
    __mmask16 norm = _mm512_cmplt_epi32(no_sign, const_vector_exp);
    norm = _mm512_mask_cmpnle_epi32(norm, no_sign, (__m512i)((int16)0));
    return mask2pi(norm);
  }]>;
OclBuiltinImpl isnormal_f32_promote = OclBuiltinImpl<isnormal, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote1>;

// isordered
OclBuiltinImpl isordered_f32_v1 = OclBuiltinImpl<isordered, [v1f32], 0, [{
    return (($Arg0VarName==$Arg0VarName) && ($Arg1VarName==$Arg1VarName))?1:0;
  }]>;
OclBuiltinImpl isordered_f32_v16 = OclBuiltinImpl<isordered, [v16f32], 0, [{
    int16 equal1 = isequal($Arg0VarName, $Arg0VarName);
    int16 equal2 = isequal($Arg1VarName, $Arg1VarName);
    __m512i isordered = (__m512i)(equal1);
    isordered = _mm512_and_epi32(isordered, (__m512i)equal2);
    return as_$ReturnType(isordered);
  }]>;
OclBuiltinImpl isordered_f32_promote = OclBuiltinImpl<isordered, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote2>;
OclBuiltinImpl isordered_f64_v1 = OclBuiltinImpl<isordered, [v1f64], 0, [{
    return (($Arg0VarName==$Arg0VarName) && ($Arg1VarName==$Arg1VarName))?1:0;
  }]>;
OclBuiltinImpl isordered_f64_v8 = OclBuiltinImpl<isordered, [v8f64], 0, [{
    long8 equal1 = isequal($Arg0VarName, $Arg0VarName);
    long8 equal2 = isequal($Arg1VarName, $Arg1VarName);
    __m512i isordered = (__m512i)(equal1);
    isordered = _mm512_and_epi64(isordered, equal2);
    return as_$ReturnType(isordered);
  }]>;
OclBuiltinImpl isordered_f64_promote = OclBuiltinImpl<isordered, [v2f64, v3f64, v4f64], 0, NonMaskPromote2>;
OclBuiltinImpl isordered_f64_expand = OclBuiltinImpl<isordered, [v16f64], 0, Expand2>;

// isunordered
OclBuiltinImpl isunordered_f32_v1 = OclBuiltinImpl<isunordered, [v1f32], 0, [{
    return isnan($Arg0VarName) | isnan($Arg1VarName);
  }]>;
OclBuiltinImpl isunordered_f32_v16 = OclBuiltinImpl<isunordered, [v16f32], 0, [{
    int16 nan1 = isnan($Arg0VarName);
    int16 nan2 = isnan($Arg1VarName);
    __m512i isunordered = (__m512i)(nan1);
    isunordered = _mm512_or_epi32(isunordered, (__m512i)nan2);
    return as_$NativeReturnType(isunordered);
  }]>;
OclBuiltinImpl isunordered_f32_promote = OclBuiltinImpl<isunordered, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote2>;
OclBuiltinImpl isunordered_f64_v1 = OclBuiltinImpl<isunordered, [v1f64], 0, [{
    return isnan($Arg0VarName) | isnan($Arg1VarName);
  }]>;
OclBuiltinImpl isunordered_f64_v8 = OclBuiltinImpl<isunordered, [v8f64], 0, [{
    long8 nan1 = isnan($Arg0VarName);
    long8 nan2 = isnan($Arg1VarName);
    __m512i isunordered = (__m512i)(nan1);
    isunordered = _mm512_or_epi64(isunordered, (__m512i)nan2);
    return as_$NativeReturnType(isunordered);
  }]>;
OclBuiltinImpl isunordered_f64_promote = OclBuiltinImpl<isunordered, [v2f64, v3f64, v4f64], 0, NonMaskPromote2>;
OclBuiltinImpl isunordered_f64_expand = OclBuiltinImpl<isunordered, [v16f64], 0, Expand2>;

// signbit
OclBuiltinImpl signbit_f32_v1 = OclBuiltinImpl<signbit, [v1f32], 0, [{
    return (as_int($Arg0VarName) & const_msb)?1:0;
  }]>;
OclBuiltinImpl signbit_f32_v16 = OclBuiltinImpl<signbit, [v16f32], 0, [{
    __m512i const_vector_msb = (__m512i)((int16)const_msb);
    __m512i zeros = (__m512i)_mm512_setzero();
    __m512i no_sign = _mm512_and_epi32((__m512i)($Arg0VarName), const_vector_msb);
    __mmask16 sign = _mm512_cmpneq_epi32(no_sign, zeros);
    return mask2pi(sign);
  }]>;
OclBuiltinImpl signbit_f32_promote = OclBuiltinImpl<signbit, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote1>;
OclBuiltinImpl signbit_f64_v1 = OclBuiltinImpl<signbit, [v1f64], 0, [{
    return (as_long($Arg0VarName) & const_dp_msb)?1:0;
  }]>;
OclBuiltinImpl signbit_f64_v8 = OclBuiltinImpl<signbit, [v8f64], 0, [{
    $NativeArg0Type upper = as_$NativeArg0Type(unpack_hi($Arg0VarName));
    __m512i zeros = (__m512i)_mm512_setzero();
    __m512i no_sign = _mm512_and_epi32((__m512i)(upper), (__m512i)((int16)const_msb));
    __mmask16 sign = _mm512_cmpneq_epi32(no_sign, zeros);
    return as_$NativeReturnType(mask2pi(sign));
  }]>;
OclBuiltinImpl signbit_f64_promote = OclBuiltinImpl<signbit, [v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl signbit_f64_expand = OclBuiltinImpl<signbit, [v16f64], 0, Expand1>;

// any
code AnyExpand = [{
    $ReturnType $ReturnVarName;
    $ReturnVarName = $ExpandLoFunc($Arg0VarName$ExpandLoPattern);
    $ReturnVarName = $ReturnVarName | $ExpandHiFunc($Arg0VarName$ExpandHiPattern);
    return $ReturnVarName;
  }];
OclBuiltinImpl any_scalar_int = OclBuiltinImpl<any, [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32], 0, [{
    return ($Arg0VarName >> ($BitWidth-1))&1;
  }]>;
OclBuiltinImpl any_scalar_int64 = OclBuiltinImpl<any, [v1i64, v1u64], 0, [{
    return ($Arg0VarName >> ($BitWidth-1))&1;
  }]>;
OclBuiltinImpl any_scalar_f32 = OclBuiltinImpl<any, [v1f32], 0, [{
    return (as_int($Arg0VarName) >> ($BitWidth-1))&1;
  }]>;
OclBuiltinImpl any_scalar_f64 = OclBuiltinImpl<any, [v1f64], 0, [{
    return (as_long($Arg0VarName) >> ($BitWidth-1))&1;
  }]>;
OclBuiltinImpl mask_any_vector_v16 = OclBuiltinImpl<mask_any, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32], 0, [{
    __m512i const_vector_msb = (__m512i)((int16)(1 << ($BitWidth-1)));
    __m512i zeros = (__m512i)_mm512_setzero();
    __m512i sign_bits = (__m512i)_mm512_setzero();
    int16 x_reg = as_int16(sign_ext_from($Arg1VarName));
    sign_bits = _mm512_mask_and_epi32(sign_bits, $Arg0VarName, (__m512i)(x_reg), const_vector_msb);
    __mmask16 sign_mask = _mm512_cmpneq_epi32(sign_bits, zeros);
    int all_zero = _mm512_kortestz(sign_mask, sign_mask);
    return all_zero?0:1;
  }]>;
OclBuiltinImpl any_vector_v16 = OclBuiltinImpl<any, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32], 0, [{
    __m512i const_vector_msb = (__m512i)((int16)(1 << ($BitWidth-1)));
    __m512i zeros = (__m512i)_mm512_setzero();
    int16 x_reg = as_int16(sign_ext_from($Arg0VarName));
    __m512i sign_bits = _mm512_and_epi32((__m512i)(x_reg), const_vector_msb);
    __mmask16 sign_mask = _mm512_cmpneq_epi32(sign_bits, zeros);
    int all_zero = _mm512_kortestz(sign_mask, sign_mask);
    return all_zero?0:1;
  }]>;
OclBuiltinImpl any_vector_v8 = OclBuiltinImpl<any, [ v8i64, v8u64, v8f64], 0, [{
    int16 x_reg = as_int16(unpack_hi($Arg0VarName));
    return any(x_reg);
  }]>;
OclBuiltinImpl any_short_promote = OclBuiltinImpl<any, [v2i8, v3i8, v4i8, v8i8, v2u8, v3u8, v4u8, v8u8,
                                                      v2i16, v3i16, v4i16, v8i16, v2u16, v3u16, v4u16, v8u16,
                                                      v2i32, v3i32, v4i32, v8i32, v2u32, v3u32, v4u32, v8u32,
                                                      v2f32, v3f32, v4f32, v8f32],
                                                      0, [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnType $ReturnVarName = mask_any($Mask, n$Arg0VarName);
    return $ReturnVarName;
  }]>;
OclBuiltinImpl any_long_promote = OclBuiltinImpl<any, [v2i64, v3i64, v4i64, v2u64, v3u64, v4u64, v2f64, v3f64, v4f64],
                                                      0, [{
    $NativeArg0Type n$Arg0VarName = ($NativeArg0Type)(0); n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnType $ReturnVarName = any(n$Arg0VarName);
    return $ReturnVarName;
  }]>;
OclBuiltinImpl any_f64_expand = OclBuiltinImpl<any, [v16f64, v16u64, v16i64], 0, AnyExpand>;

// all
code AllExpand = [{
    $ReturnType $ReturnVarName;
    $ReturnVarName = $ExpandLoFunc($Arg0VarName$ExpandLoPattern);
    $ReturnVarName = $ReturnVarName & $ExpandHiFunc($Arg0VarName$ExpandHiPattern);
    return $ReturnVarName;
  }];
OclBuiltinImpl all_scalar_int = OclBuiltinImpl<all, [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32], 0, [{
    return ($Arg0VarName >> ($BitWidth-1)) & 1;
  }]>;
OclBuiltinImpl all_scalar_int64 = OclBuiltinImpl<all, [v1i64, v1u64], 0, [{
    return ($Arg0VarName >> ($BitWidth-1)) & 1;
  }]>;
OclBuiltinImpl all_scalar_f32 = OclBuiltinImpl<all, [v1f32], 0, [{
    return (as_int($Arg0VarName) >> ($BitWidth-1)) & 1;
  }]>;
OclBuiltinImpl all_scalar_f64 = OclBuiltinImpl<all, [v1f64], 0, [{
    return (as_long($Arg0VarName) >> ($BitWidth-1)) & 1;
  }]>;
OclBuiltinImpl mask_all_vector_v16 = OclBuiltinImpl<mask_all, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32], 0, [{
    __m512i const_vector_msb = (__m512i)((int16)(1 << ($BitWidth-1)));
    __m512i no_sign = const_vector_msb;
    __m512i zeros = (__m512i)_mm512_setzero();
    int16 x_reg = as_int16(sign_ext_from($Arg1VarName));
    no_sign = _mm512_mask_and_epi32(no_sign, $Arg0VarName, (__m512i)(x_reg), const_vector_msb);
    __mmask16 equal = _mm512_cmpeq_epi32(no_sign, const_vector_msb);
    int all_one = _mm512_kortestc(equal, equal);
    return all_one?1:0;
  }]>;
OclBuiltinImpl all_vector_v16 = OclBuiltinImpl<all, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32], 0, [{
    __m512i const_vector_msb = (__m512i)((int16)(1 << ($BitWidth-1)));
    __m512i zeros = (__m512i)_mm512_setzero();
    int16 x_reg = as_int16(sign_ext_from($Arg0VarName));
    __m512i no_sign = _mm512_and_epi32((__m512i)(x_reg), const_vector_msb);
    __mmask16 equal = _mm512_cmpeq_epi32(no_sign, const_vector_msb);
    int all_one = _mm512_kortestc(equal, equal);
    return all_one?1:0;
  }]>;
OclBuiltinImpl all_vector_v8 = OclBuiltinImpl<all, [v8i64, v8u64, v8f64], 0, [{
    int16 x_reg = as_int16(unpack_hi($Arg0VarName));
    return all(x_reg);
  }]>;
OclBuiltinImpl all_short_promote = OclBuiltinImpl<all, [v2i8, v3i8, v4i8, v8i8, v2u8, v3u8, v4u8, v8u8,
                                                      v2i16, v3i16, v4i16, v8i16, v2u16, v3u16, v4u16, v8u16,
                                                      v2i32, v3i32, v4i32, v8i32, v2u32, v3u32, v4u32, v8u32,
                                                      v2f32, v3f32, v4f32, v8f32],
                                                      0, [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnType $ReturnVarName = mask_all($Mask, n$Arg0VarName);
    return $ReturnVarName;
  }]>;
OclBuiltinImpl all_i64_promote = OclBuiltinImpl<all, [v2i64, v3i64, v4i64, v2u64, v3u64, v4u64],
                                                      0, [{
    $NativeArg0Type n$Arg0VarName = ($NativeArg0Type)(1L << ($BitWidth-1)); n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnType $ReturnVarName = all(n$Arg0VarName);
    return $ReturnVarName;
  }]>;
OclBuiltinImpl all_f64_promote = OclBuiltinImpl<all, [v2f64, v3f64, v4f64],
                                                      0, [{
    $NativeArg0Type n$Arg0VarName = ($NativeArg0Type)(as_double(1L << ($BitWidth-1)); n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnType $ReturnVarName = all(n$Arg0VarName);
    return $ReturnVarName;
  }]>;
OclBuiltinImpl all_f64_expand = OclBuiltinImpl<all, [v16f64, v16u64, v16i64], 0, AllExpand>;

code select_native_vector = [{
    $Arg0igentype ret =
      __ocl_select_$ReturnSymigentype(
                    as_$Arg0igentype($Arg0VarName),
                    as_$Arg1igentype($Arg1VarName),
                    as_$Arg2igentype($Arg2VarName));
    return as_$ReturnType(ret);
  }];

code select_narrow_types = [{
    $Arg0i32gentype arg0 = convert_$Arg0i32gentype($Arg0VarName);
    $Arg1i32gentype arg1 = convert_$Arg1i32gentype($Arg1VarName);
    $Arg2i32gentype arg2 = __ocl_sext_$Arg2Symigentype_$Arg2Symi32gentype(
      as_$Arg2igentype($Arg2VarName));

    $Returni32gentype ret = select(arg0, arg1, arg2);
    return convert_$ReturnType(ret);
  }];

code select_scalar = [{
    return $Arg2VarName?$Arg1VarName:$Arg0VarName;
  }];

// iselect
OclBuiltinImpl iselect_scalar = OclBuiltinImpl<iselect, [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64],
                                0, select_scalar>;
OclBuiltinImpl iselect_native_vector = OclBuiltinImpl<iselect, [v16i32, v16u32, v16f32, v8i64, v8u64, v8f64],
                                    0, select_native_vector>;

OclBuiltinImpl iselect_expand_wide = OclBuiltinImpl<iselect, [v16f64, v16u64, v16i64], 0, Expand3>;

OclBuiltinImpl iselect_promote = OclBuiltinImpl<iselect,
                                               [v2i32, v3i32, v4i32, v8i32,
                                                v2u32, v3u32, v4u32, v8u32,
                                                v2f32, v3f32, v4f32, v8f32,
                                                v2i64, v3i64, v4i64,
                                                v2u64, v3u64, v4u64,
                                                v2f64, v3f64, v4f64],
                                                0, NonMaskPromote3>;

OclBuiltinImpl iselect_narrow_types = OclBuiltinImpl<iselect,
                                                    [v2i8, v3i8, v4i8, v8i8, v16i8,
                                                     v2u8, v3u8, v4u8, v8u8, v16u8,
                                                     v2i16, v3i16, v4i16, v8i16, v16i16,
                                                     v2u16, v3u16, v4u16, v8u16, v16u16],
                                                     0, select_narrow_types>;

// uselect
OclBuiltinImpl uselect_scalar = OclBuiltinImpl<uselect, [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64],
                                0, select_scalar>;
OclBuiltinImpl uselect_native_vector = OclBuiltinImpl<uselect, [v16i32, v16u32, v16f32, v8i64, v8u64, v8f64],
                                    0, select_native_vector>;

OclBuiltinImpl uselect_expand_wide = OclBuiltinImpl<uselect, [v16f64, v16u64, v16i64], 0, Expand3>;

OclBuiltinImpl uselect_promote = OclBuiltinImpl<uselect,
                                               [v2i32, v3i32, v4i32, v8i32,
                                                v2u32, v3u32, v4u32, v8u32,
                                                v2f32, v3f32, v4f32, v8f32,
                                                v2i64, v3i64, v4i64,
                                                v2u64, v3u64, v4u64,
                                                v2f64, v3f64, v4f64],
                                                0, NonMaskPromote3>;

OclBuiltinImpl uselect_narrow_types = OclBuiltinImpl<uselect,
                                                    [v2i8, v3i8, v4i8, v8i8, v16i8,
                                                     v2u8, v3u8, v4u8, v8u8, v16u8,
                                                     v2i16, v3i16, v4i16, v8i16, v16i16,
                                                     v2u16, v3u16, v4u16, v8u16, v16u16],
                                                     0, select_narrow_types>;


// bitselect
OclBuiltinImpl bitselect_scalar = OclBuiltinImpl<bitselect, [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64], 0, [{
    return ($Arg1VarName& $Arg2VarName) | ($Arg0VarName & ~$Arg2VarName);
  }]>;
OclBuiltinImpl bitselect_scalar_f32 = OclBuiltinImpl<bitselect, [v1f32], 0, [{
    return as_float((as_int($Arg1VarName) & as_int($Arg2VarName)) | (as_int($Arg0VarName) & ~as_int($Arg2VarName)));
  }]>;
OclBuiltinImpl bitselect_scalar_f64 = OclBuiltinImpl<bitselect, [v1f64], 0, [{
    return as_double((as_long($Arg1VarName) & as_long($Arg2VarName)) | (as_long($Arg0VarName) & ~as_long($Arg2VarName)));
  }]>;
OclBuiltinImpl bitselect_vector = OclBuiltinImpl<bitselect, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32,
                                                                      v8i64, v8u64, v16f32, v8f64], 0, [{
    int16 c_reg = as_int16(ext_from($Arg2VarName));
    int16 a_reg = as_int16(ext_from($Arg0VarName));
    int16 b_reg = as_int16(ext_from($Arg1VarName));
    __m512i result_true = _mm512_and_epi32((__m512i)b_reg, (__m512i)c_reg);
    __m512i result_false = _mm512_andnot_epi32((__m512i)c_reg, (__m512i)a_reg);
    result_true = _mm512_or_epi32(result_true, result_false);
    return trunc_to_$NativeReturnType(as_int16(result_true));
  }]>;
OclBuiltinImpl bitselect_v16_promote = OclBuiltinImpl<bitselect, [v2i8, v3i8, v4i8, v8i8, v2u8, v3u8, v4u8, v8u8,
                                                      v2i16, v3i16, v4i16, v8i16, v2u16, v3u16, v4u16, v8u16,
                                                      v2i32, v3i32, v4i32, v8i32, v2u32, v3u32, v4u32, v8u32,
                                                      v2i64, v3i64, v4i64, v2u64, v3u64, v4u64,
                                                      v2f32, v3f32, v4f32, v8f32, v2f64, v3f64, v4f64, v8f64],
                                                      0, NonMaskPromote3>;
OclBuiltinImpl bitselect_f64_expand = OclBuiltinImpl<bitselect, [v16f64, v16u64, v16i64], 0, Expand3>;

