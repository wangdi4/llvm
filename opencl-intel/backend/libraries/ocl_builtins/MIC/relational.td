// vim:ts=2:sw=2:et:
// Copyright (c) 2006-2012 Intel Corporation
// Relational
// OpenCL 1.2 rev 15, Table 6.14

// isequal
OclBuiltinImpl isequal_scalar = OclBuiltinImpl<isequal, [v1f32, v1f64], 0, [{
    return ($Arg0VarName == $Arg1VarName)?1:0;
  }]>;
OclBuiltinImpl isequal_f32_v16 = OclBuiltinImpl<isequal, [v16f32], 0, [{
    __mmask16 flags = _mm512_cmpeq_ps($Arg0VarName, $Arg1VarName);
    return mask2pi(flags);
  }]>;
OclBuiltinImpl isequal_f32_promote = OclBuiltinImpl<isequal, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote2>;
OclBuiltinImpl isequal_f64_v8 = OclBuiltinImpl<isequal, [v8f64], 0, [{
    __mmask16 flags = _mm512_cmpeq_pd($Arg0VarName, $Arg1VarName);
    return mask2pq(flags);
  }]>;
OclBuiltinImpl isequal_f64_promote = OclBuiltinImpl<isequal, [v2f64, v3f64, v4f64], 0, NonMaskPromote2>;
OclBuiltinImpl isequal_f64_expand = OclBuiltinImpl<isequal, [v16f64], 0, Expand2>;

// isnotequal
OclBuiltinImpl isnotequal_scalar = OclBuiltinImpl<isnotequal, [v1f32, v1f64], 0, [{
    return ($Arg0VarName != $Arg1VarName)?1:0;
  }]>;
OclBuiltinImpl isnotequal_f32_v16 = OclBuiltinImpl<isnotequal, [v16f32], 0, [{
    __mmask16 flags = _mm512_cmpneq_ps($Arg0VarName, $Arg1VarName);
    return mask2pi(flags);
  }]>;
OclBuiltinImpl isnotequal_f32_promote = OclBuiltinImpl<isnotequal, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote2>;
OclBuiltinImpl isnotequal_f64_v8 = OclBuiltinImpl<isnotequal, [v8f64], 0, [{
    __mmask16 flags = _mm512_cmpneq_pd($Arg0VarName, $Arg1VarName);
    return mask2pq(flags);
  }]>;
OclBuiltinImpl isnotequal_f64_promote = OclBuiltinImpl<isnotequal, [v2f64, v3f64, v4f64], 0, NonMaskPromote2>;
OclBuiltinImpl isnotequal_f64_expand = OclBuiltinImpl<isnotequal, [v16f64], 0, Expand2>;

// isgreater
OclBuiltinImpl isgreater_scalar = OclBuiltinImpl<isgreater, [v1f32, v1f64], 0, [{
    return ($Arg0VarName > $Arg1VarName)?1:0;
  }]>;
OclBuiltinImpl isgreater_f32_v16 = OclBuiltinImpl<isgreater, [v16f32], 0, [{
    __mmask16 flags = _mm512_cmpnle_ps($Arg0VarName, $Arg1VarName);
    return mask2pi(flags);
  }]>;
OclBuiltinImpl isgreater_f32_promote = OclBuiltinImpl<isgreater, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote2>;
OclBuiltinImpl isgreater_f64_v8 = OclBuiltinImpl<isgreater, [v8f64], 0, [{
    __mmask16 flags = _mm512_cmpnle_pd($Arg0VarName, $Arg1VarName);
    return mask2pq(flags);
  }]>;
OclBuiltinImpl isgreater_f64_promote = OclBuiltinImpl<isgreater, [v2f64, v3f64, v4f64], 0, NonMaskPromote2>;
OclBuiltinImpl isgreater_f64_expand = OclBuiltinImpl<isgreater, [v16f64], 0, Expand2>;

// isgreaterequal
OclBuiltinImpl isgreaterequal_scalar = OclBuiltinImpl<isgreaterequal, [v1f32, v1f64], 0, [{
    return ($Arg0VarName >= $Arg1VarName)?1:0;
  }]>;
OclBuiltinImpl isgreaterequal_f32_v16 = OclBuiltinImpl<isgreaterequal, [v16f32], 0, [{
    __mmask16 flags = _mm512_cmpnlt_ps($Arg0VarName, $Arg1VarName);
    return mask2pi(flags);
  }]>;
OclBuiltinImpl isgreaterequal_f32_promote = OclBuiltinImpl<isgreaterequal, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote2>;
OclBuiltinImpl isgreaterequal_f64_v8 = OclBuiltinImpl<isgreaterequal, [v8f64], 0, [{
    __mmask16 flags = _mm512_cmpnlt_pd($Arg0VarName, $Arg1VarName);
    return mask2pq(flags);
  }]>;
OclBuiltinImpl isgreaterequal_f64_promote = OclBuiltinImpl<isgreaterequal, [v2f64, v3f64, v4f64], 0, NonMaskPromote2>;
OclBuiltinImpl isgreaterequal_f64_expand = OclBuiltinImpl<isgreaterequal, [v16f64], 0, Expand2>;

// isless
OclBuiltinImpl isless_scalar = OclBuiltinImpl<isless, [v1f32, v1f64], 0, [{
    return ($Arg0VarName < $Arg1VarName)?1:0;
  }]>;
OclBuiltinImpl isless_f32_v16 = OclBuiltinImpl<isless, [v16f32], 0, [{
    __mmask16 flags = _mm512_cmplt_ps($Arg0VarName, $Arg1VarName);
    return mask2pi(flags);
  }]>;
OclBuiltinImpl isless_f32_promote = OclBuiltinImpl<isless, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote2>;
OclBuiltinImpl isless_f64_v8 = OclBuiltinImpl<isless, [v8f64], 0, [{
    __mmask16 flags = _mm512_cmplt_pd($Arg0VarName, $Arg1VarName);
    return mask2pq(flags);
  }]>;
OclBuiltinImpl isless_f64_promote = OclBuiltinImpl<isless, [v2f64, v3f64, v4f64], 0, NonMaskPromote2>;
OclBuiltinImpl isless_f64_expand = OclBuiltinImpl<isless, [v16f64], 0, Expand2>;

// islessequal
OclBuiltinImpl islessequal_scalar = OclBuiltinImpl<islessequal, [v1f32, v1f64], 0, [{
    return ($Arg0VarName <= $Arg1VarName)?1:0;
  }]>;
OclBuiltinImpl islessequal_f32_v16 = OclBuiltinImpl<islessequal, [v16f32], 0, [{
    __mmask16 flags = _mm512_cmple_ps($Arg0VarName, $Arg1VarName);
    return mask2pi(flags);
  }]>;
OclBuiltinImpl islessequal_f32_promote = OclBuiltinImpl<islessequal, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote2>;
OclBuiltinImpl islessequal_f64_v8 = OclBuiltinImpl<islessequal, [v8f64], 0, [{
    __mmask16 flags = _mm512_cmple_pd($Arg0VarName, $Arg1VarName);
    return mask2pq(flags);
  }]>;
OclBuiltinImpl islessequal_f64_promote = OclBuiltinImpl<islessequal, [v2f64, v3f64, v4f64], 0, NonMaskPromote2>;
OclBuiltinImpl islessequal_f64_expand = OclBuiltinImpl<islessequal, [v16f64], 0, Expand2>;

// islessgreater
code islessgreater_scalar_code = [{
    return (($Arg0VarName < $Arg1VarName) || ($Arg0VarName > $Arg1VarName))?1:0;
  }];
OclBuiltinImpl islessgreater_scalar = OclBuiltinImpl<islessgreater, [v1f32, v1f64], 0, islessgreater_scalar_code>;
OclBuiltinImpl islessgreater_f32_v16 = OclBuiltinImpl<islessgreater, [v16f32], 0, [{
    __mmask16 isless = _mm512_cmplt_ps($Arg0VarName, $Arg1VarName);
    __mmask16 isgreater = _mm512_cmpnle_ps($Arg0VarName, $Arg1VarName);
    isless = _mm512_kor(isless, isgreater);
    return mask2pi(isless);
  }]>;
OclBuiltinImpl islessgreater_f32_promote = OclBuiltinImpl<islessgreater, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote2>;
OclBuiltinImpl islessgreater_f64_v8 = OclBuiltinImpl<islessgreater, [v8f64], 0, [{
    __mmask16 isless = _mm512_cmplt_pd($Arg0VarName, $Arg1VarName);
    __mmask16 isgreater = _mm512_cmpnle_pd($Arg0VarName, $Arg1VarName);
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
    __m512i no_sign = _mm512_and_pi((__m512i)$Arg0VarName, const_vector_exp);
    __mmask16 isfinite = _mm512_cmplt_pi(no_sign, const_vector_exp);
    return mask2pi(isfinite);
  }]>;
OclBuiltinImpl isfinite_f32_promote = OclBuiltinImpl<isfinite, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote1>;
OclBuiltinImpl isfinite_f64_v1 = OclBuiltinImpl<isfinite, [v1f64], 0, [{
    return ((as_long($Arg0VarName) & const_dp_exp) < const_dp_exp)?1:0;
  }]>;
OclBuiltinImpl isfinite_f64_v8 = OclBuiltinImpl<isfinite, [v8f64], 0, [{
    $NativeArg0Type upper = ($NativeArg0Type)unpack_hi($Arg0VarName);
    __m512i const_vector_exp = (__m512i)((int16)const_dp_exp);
    __m512i no_sign = _mm512_and_pi((__m512i)upper, const_vector_exp);
    __mmask16 isfinite = _mm512_cmplt_pi(no_sign, const_vector_exp);
    return ($NativeReturnType)mask2pi(isfinite);
  }]>;
OclBuiltinImpl isfinite_f64_promote = OclBuiltinImpl<isfinite, [v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl isfinite_f64_expand = OclBuiltinImpl<isfinite, [v16f64], 0, Expand1>;

// isinf
OclBuiltinImpl isinf_f32_v1 = OclBuiltinImpl<isinf, [v1f32], 0, [{
    return ((as_int($Arg0VarName) & const_fp_no_sign) < const_fp_exp)?1:0;
  }]>;
OclBuiltinImpl isinf_f32_v16 = OclBuiltinImpl<isinf, [v16f32], 0, [{
    __m512i no_sign = _mm512_and_pi((__m512i)($Arg0VarName), (__m512i)((int16)const_fp_no_sign));
    __mmask16 inf = _mm512_cmpeq_pi(no_sign, (__m512i)((int16)const_fp_exp));
    return mask2pi(inf);
  }]>;
OclBuiltinImpl isinf_f32_promote = OclBuiltinImpl<isinf, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote1>;
OclBuiltinImpl isinf_f64_v1 = OclBuiltinImpl<isinf, [v1f64], 0, [{
    return ((as_long($Arg0VarName) & const_dp_no_sign) < const_dp_exp)?1:0;
  }]>;
OclBuiltinImpl isinf_f64_v8 = OclBuiltinImpl<isinf, [v8f64], 0, [{
    $NativeArg0Type upper = ($NativeArg0Type)unpack_hi($Arg0VarName);
    __m512i no_sign = _mm512_and_pi((__m512i)(upper), (__m512i)((int16)const_dp_no_sign));
    __mmask16 inf = _mm512_cmpeq_pi(no_sign, (__m512i)((int16)const_dp_exp));
    return ($NativeReturnType)mask2pi(inf);
  }]>;
OclBuiltinImpl isinf_f64_promote = OclBuiltinImpl<isinf, [v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl isinf_f64_expand = OclBuiltinImpl<isinf, [v16f64], 0, Expand1>;

// isnan
OclBuiltinImpl isnan_f32_v1 = OclBuiltinImpl<isnan, [v1f32], 0, [{
    return ((as_int($Arg0VarName) & const_fp_no_sign) > const_fp_exp)?1:0;
  }]>;
OclBuiltinImpl isnan_f32_v16 = OclBuiltinImpl<isnan, [v16f32], 0, [{
    __m512i no_sign = _mm512_and_pi((__m512i)($Arg0VarName), (__m512i)((int16)const_fp_no_sign));
    __mmask16 nan = _mm512_cmpnle_pi(no_sign, (__m512i)((int16)const_fp_exp));
    return mask2pi(nan);
  }]>;
OclBuiltinImpl isnan_f32_promote = OclBuiltinImpl<isnan, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote1>;
OclBuiltinImpl isnan_f64_v1 = OclBuiltinImpl<isnan, [v1f64], 0, [{
    return ((as_long($Arg0VarName) & const_dp_no_sign) > const_dp_exp)?1:0;
  }]>;
OclBuiltinImpl isnan_f64_v8 = OclBuiltinImpl<isnan, [v8f64], 0, [{
    $NativeArg0Type upper = ($NativeArg0Type)unpack_hi($Arg0VarName);
    $NativeArg0Type lower = ($NativeArg0Type)unpack_lo($Arg0VarName);
    __mmask16 not_zero = _mm512_cmpneq_pi((__m512i)lower, (__m512i)((int16)0));
    __mmask16 nan = _mm512_cmpnle_pi((__m512i)upper, (__m512i)((long8)const_dp_exp));
    nan = _mm512_kor(nan, not_zero);
    return ($NativeReturnType)mask2pi(nan);
  }]>;
OclBuiltinImpl isnan_f64_promote = OclBuiltinImpl<isnan, [v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl isnan_f64_expand = OclBuiltinImpl<isnan, [v16f64], 0, Expand1>;

// isnormal
OclBuiltinImpl isnormal_f32_v1 = OclBuiltinImpl<isnormal, [v1f32], 0, [{
    return ( ((as_int($Arg0VarName) & const_fp_exp)!=0) && ( (as_int($Arg0VarName) & const_fp_exp) != const_fp_exp) )?1:0;
  }]>;
OclBuiltinImpl isnormal_f32_v16 = OclBuiltinImpl<isnormal, [v16f32], 0, [{
    __m512i const_vector_exp = (__m512i)((int16)const_fp_exp);
    __m512i no_sign = _mm512_and_pi((__m512i)($Arg0VarName), const_vector_exp);
    __mmask16 norm = _mm512_cmplt_pi(no_sign, const_vector_exp);
    norm = _mm512_mask_cmpnle_pi(norm, no_sign, (__m512i)((int16)0));
    return mask2pi(norm);
  }]>;
OclBuiltinImpl isnormal_f32_promote = OclBuiltinImpl<isnormal, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote1>;
OclBuiltinImpl isnormal_f64_v1 = OclBuiltinImpl<isnormal, [v1f64], 0, [{
    return ( ((as_long($Arg0VarName) & const_dp_exp)!=0) && ( (as_long($Arg0VarName) & const_dp_exp) != const_dp_exp) )?1:0;
  }]>;
OclBuiltinImpl isnormal_f64_v8 = OclBuiltinImpl<isnormal, [v8f64], 0, [{
    __m512i const_vector_exp = (__m512i)((int16)const_dp_exp_hi);
    $NativeArg0Type upper = ($NativeArg0Type)unpack_hi($Arg0VarName);
    __m512i no_sign = _mm512_and_pi((__m512i)upper, const_vector_exp);
    __mmask16 norm = _mm512_cmplt_pi(no_sign, const_vector_exp);
    norm = _mm512_mask_cmpnle_pi(norm, (__m512i)upper, (__m512i)((int16)0));
    return as_$ReturnType(mask2pi(norm));
  }]>;
OclBuiltinImpl isnormalal_f64_promote = OclBuiltinImpl<isnormal, [v2f64, v3f64, v4f64], 0, NonMaskPromote1>;
OclBuiltinImpl isnormal_f64_expand = OclBuiltinImpl<isnormal, [v16f64], 0, Expand1>;

// isordered
OclBuiltinImpl isordered_f32_v1 = OclBuiltinImpl<isordered, [v1f32], 0, [{
    return (($Arg0VarName==$Arg0VarName) && ($Arg1VarName==$Arg1VarName))?1:0;
  }]>;
OclBuiltinImpl isordered_f32_v16 = OclBuiltinImpl<isordered, [v16f32], 0, [{
    int16 equal1 = isequal($Arg0VarName, $Arg0VarName);
    int16 equal2 = isequal($Arg1VarName, $Arg1VarName);
    __m512i isordered = (__m512i)(equal1);
    isordered = _mm512_and_pi(isordered, (__m512i)equal2);
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
    isordered = _mm512_and_pq(isordered, equal2);
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
    isunordered = _mm512_or_pi(isunordered, (__m512i)nan2);
    return ($NativeReturnType)isunordered;
  }]>;
OclBuiltinImpl isunordered_f32_promote = OclBuiltinImpl<isunordered, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote2>;
OclBuiltinImpl isunordered_f64_v1 = OclBuiltinImpl<isunordered, [v1f64], 0, [{
    return isnan($Arg0VarName) | isnan($Arg1VarName);
  }]>;
OclBuiltinImpl isunordered_f64_v8 = OclBuiltinImpl<isunordered, [v8f64], 0, [{
    long8 nan1 = isnan($Arg0VarName);
    long8 nan2 = isnan($Arg1VarName);
    __m512i isunordered = (__m512i)(nan1);
    isunordered = _mm512_or_pq(isunordered, (__m512i)nan2);
    return ($NativeReturnType)isunordered;
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
    __m512i no_sign = _mm512_and_pi((__m512i)($Arg0VarName), const_vector_msb);
    __mmask16 sign = _mm512_cmpneq_pi(no_sign, zeros);
    return mask2pi(sign);
  }]>;
OclBuiltinImpl signbit_f32_promote = OclBuiltinImpl<signbit, [v2f32, v3f32, v4f32, v8f32], 0, NonMaskPromote1>;
OclBuiltinImpl signbit_f64_v1 = OclBuiltinImpl<signbit, [v1f64], 0, [{
    return (as_long($Arg0VarName) & const_dp_msb)?1:0;
  }]>;
OclBuiltinImpl signbit_f64_v8 = OclBuiltinImpl<signbit, [v8f64], 0, [{
    $NativeArg0Type upper = ($NativeArg0Type)unpack_hi($Arg0VarName);
    __m512i zeros = (__m512i)_mm512_setzero();
    __m512i no_sign = _mm512_and_pi((__m512i)(upper), (__m512i)((int16)const_msb));
    __mmask16 sign = _mm512_cmpneq_pi(no_sign, zeros);
    return ($NativeReturnType) mask2pi(sign);
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
    return ($Arg0VarName&(1 << ($BitWidth-1)))?1:0;
  }]>;
OclBuiltinImpl any_scalar_int64 = OclBuiltinImpl<any, [v1i64, v1u64], 0, [{
    return ($Arg0VarName&(1L << ($BitWidth-1)))?1:0;
  }]>;
OclBuiltinImpl any_scalar_f32 = OclBuiltinImpl<any, [v1f32], 0, [{
    return (as_int($Arg0VarName)&(1 << ($BitWidth-1)))?1:0;
  }]>;
OclBuiltinImpl any_scalar_f64 = OclBuiltinImpl<any, [v1f64], 0, [{
    return (as_long($Arg0VarName)&(1L << ($BitWidth-1)))?1:0;
  }]>;
OclBuiltinImpl any_vector_v16 = OclBuiltinImpl<any, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32], 0, [{
    __m512i const_vector_msb = (__m512i)((int16)(1 << ($BitWidth-1)));
    __m512i zeros = (__m512i)_mm512_setzero();
    int16 x_reg = (int16)sign_ext_from($Arg0VarName);
    __m512i no_sign = _mm512_and_pi((__m512i)(x_reg), const_vector_msb);
    __mmask16 equal = _mm512_cmpeq_pi(no_sign, zeros);
    int all_zero = _mm512_kortestz(equal, equal);
    return all_zero?0:1;
  }]>;
OclBuiltinImpl any_vector_v8 = OclBuiltinImpl<any, [ v8i64, v8u64, v8f64], 0, [{
    int16 x_reg = (int16)unpack_hi($Arg0VarName);
    return any(x_reg);
  }]>;
OclBuiltinImpl any_v16_promote = OclBuiltinImpl<any, [v2i8, v3i8, v4i8, v8i8, v2u8, v3u8, v4u8, v8u8,
                                                      v2i16, v3i16, v4i16, v8i16, v2u16, v3u16, v4u16, v8u16,
                                                      v2i32, v3i32, v4i32, v8i32, v2u32, v3u32, v4u32, v8u32,
                                                      v2i64, v3i64, v4i64, v2u64, v3u64, v4u64,
                                                      v2f32, v3f32, v4f32, v8f32, v2f64, v3f64, v4f64],
                                                      0, ScalarReturnPromote1>;
OclBuiltinImpl any_f64_expand = OclBuiltinImpl<any, [v16f64, v16u64, v16i64], 0, AnyExpand>;

// all
code AllExpand = [{
    $ReturnType $ReturnVarName;
    $ReturnVarName = $ExpandLoFunc($Arg0VarName$ExpandLoPattern);
    $ReturnVarName = $ReturnVarName & $ExpandHiFunc($Arg0VarName$ExpandHiPattern);
    return $ReturnVarName;
  }];
OclBuiltinImpl all_scalar_int = OclBuiltinImpl<all, [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32], 0, [{
    return ($Arg0VarName&(1 << ($BitWidth-1)))?1:0;
  }]>;
OclBuiltinImpl all_scalar_int64 = OclBuiltinImpl<all, [v1i64, v1u64], 0, [{
    return ($Arg0VarName&(1L << ($BitWidth-1)))?1:0;
  }]>;
OclBuiltinImpl all_scalar_f32 = OclBuiltinImpl<all, [v1f32], 0, [{
    return (as_int($Arg0VarName)&(1 << ($BitWidth-1)))?1:0;
  }]>;
OclBuiltinImpl all_scalar_f64 = OclBuiltinImpl<all, [v1f64], 0, [{
    return (as_long($Arg0VarName)&(1L << ($BitWidth-1)))?1:0;
  }]>;
OclBuiltinImpl all_vector_v16 = OclBuiltinImpl<all, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32], 0, [{
    __m512i const_vector_msb = (__m512i)((int16)(1 << ($BitWidth-1)));
    __m512i zeros = (__m512i)_mm512_setzero();
    int16 x_reg = (int16)sign_ext_from($Arg0VarName);
    __m512i no_sign = _mm512_and_pi((__m512i)(x_reg), const_vector_msb);
    __mmask16 equal = _mm512_cmpeq_pi(no_sign, const_vector_msb);
    int all_one = _mm512_kortestc(equal, equal);
    return all_one?0:1;
  }]>;
OclBuiltinImpl all_vector_v8 = OclBuiltinImpl<all, [v8i64, v8u64, v8f64], 0, [{
    int16 x_reg = (int16)unpack_hi($Arg0VarName);
    return all(x_reg);
  }]>;
OclBuiltinImpl all_v16_promote = OclBuiltinImpl<all, [v2i8, v3i8, v4i8, v8i8, v2u8, v3u8, v4u8, v8u8,
                                                      v2i16, v3i16, v4i16, v8i16, v2u16, v3u16, v4u16, v8u16,
                                                      v2i32, v3i32, v4i32, v8i32, v2u32, v3u32, v4u32, v8u32,
                                                      v2i64, v3i64, v4i64, v2u64, v3u64, v4u64,
                                                      v2f32, v3f32, v4f32, v8f32, v2f64, v3f64, v4f64],
                                                      0, ScalarReturnPromote1>;
OclBuiltinImpl all_f64_expand = OclBuiltinImpl<all, [v16f64, v16u64, v16i64], 0, AllExpand>;

// iselect
code select_vector_v16 = [{
    $NativeReturnType result;
    __m512i const_vector_msb = (__m512i)((int16)const_msb);
    int16 c_reg = (int16)sign_ext_from($Arg2VarName);
    int16 a_reg = (int16)ext_from($Arg0VarName);
    int16 b_reg = (int16)ext_from($Arg1VarName);
    __m512i no_sign = _mm512_and_pi((__m512i)(c_reg), const_vector_msb);
    __mmask16 sel = _mm512_cmpeq_pi(no_sign, const_vector_msb);
    __m512i tempi = (__m512i)(b_reg);
    tempi = _mm512_mask_or_pi(tempi, sel, (__m512i)(a_reg), (__m512i)(a_reg));
    result = trunc_to((int16)tempi, &result);
    return result;
  }];
code select_vector_v8 = [{
    int16 x_reg = (int16)unpack_hi($Arg2VarName);
    return ($ReturnType)select((int16)$Arg0VarName, (int16)$Arg1VarName, x_reg);
  }];
code select_scalar = [{
    return ($Arg2VarName&(1 << ($BitWidth-1)))?$Arg1VarName:$Arg0VarName;
  }];
code select_scalar_i64 = [{
    return ($Arg2VarName&(1L << ($BitWidth-1)))?$Arg1VarName:$Arg0VarName;
  }];
code select_scalar_f32 = [{
    return (as_int($Arg2VarName)&(1 << ($BitWidth-1)))?$Arg1VarName:$Arg0VarName;
  }];
code select_scalar_f64 = [{
    return (as_long($Arg2VarName)&(1L << ($BitWidth-1)))?$Arg1VarName:$Arg0VarName;
  }];
OclBuiltinImpl iselect_scalar = OclBuiltinImpl<iselect, [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32],
                                0, select_scalar>;
OclBuiltinImpl iselect_scalar_i64 = OclBuiltinImpl<iselect, [v1i64, v1u64],
                                0, select_scalar_i64>;
OclBuiltinImpl iselect_scalar_f32 = OclBuiltinImpl<iselect, [v1f32], 0, select_scalar_f32>;
OclBuiltinImpl iselect_scalar_f64 = OclBuiltinImpl<iselect, [v1f64], 0, select_scalar_f64>;
OclBuiltinImpl iselect_vector_v16 = OclBuiltinImpl<iselect, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32],
                                    0, select_vector_v16>;
OclBuiltinImpl iselect_vector_v8 = OclBuiltinImpl<iselect, [v8i64, v8u64, v8f64],
                                    0, select_vector_v8>;
OclBuiltinImpl iselect_v16_promote = OclBuiltinImpl<iselect, [v2i8, v3i8, v4i8, v8i8, v2u8, v3u8, v4u8, v8u8,
                                                      v2i16, v3i16, v4i16, v8i16, v2u16, v3u16, v4u16, v8u16,
                                                      v2i32, v3i32, v4i32, v8i32, v2u32, v3u32, v4u32, v8u32,
                                                      v2i64, v3i64, v4i64, v2u64, v3u64, v4u64,
                                                      v2f32, v3f32, v4f32, v8f32, v2f64, v3f64, v4f64],
                                                      0, NonMaskPromote3>;
OclBuiltinImpl iselect_f64_expand = OclBuiltinImpl<iselect, [v16f64, v16u64, v16i64], 0, Expand3>;

// uselect
OclBuiltinImpl uselect_scalar = OclBuiltinImpl<uselect, [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32],
                                              0, select_scalar>;
OclBuiltinImpl uselect_scalar_i64 = OclBuiltinImpl<uselect, [v1i64, v1u64],
                                              0, select_scalar_i64>;
OclBuiltinImpl uselect_vector_v16 = OclBuiltinImpl<uselect, [ v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32],
                                                  0, select_vector_v16>;
OclBuiltinImpl uselect_vector_v8 = OclBuiltinImpl<uselect, [v8i64, v8u64, v8f64],
                                                  0, select_vector_v8>;
OclBuiltinImpl uselect_v16_promote = OclBuiltinImpl<uselect, [v2i8, v3i8, v4i8, v8i8, v2u8, v3u8, v4u8, v8u8,
                                                      v2i16, v3i16, v4i16, v8i16, v2u16, v3u16, v4u16, v8u16,
                                                      v2i32, v3i32, v4i32, v8i32, v2u32, v3u32, v4u32, v8u32,
                                                      v2i64, v3i64, v4i64, v2u64, v3u64, v4u64,
                                                      v2f32, v3f32, v4f32, v8f32, v2f64, v3f64, v4f64],
                                                      0, NonMaskPromote3>;
OclBuiltinImpl uselect_scalar_f32 = OclBuiltinImpl<uselect, [v1f32], 0, select_scalar_f32>;
OclBuiltinImpl uselect_scalar_f64 = OclBuiltinImpl<uselect, [v1f64], 0, select_scalar_f64>;
OclBuiltinImpl uselect_f64_expand = OclBuiltinImpl<uselect, [v16f64, v16u64, v16i64], 0, Expand3>;

// bitselect
OclBuiltinImpl bitselect_scalar = OclBuiltinImpl<bitselect, [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64], 0, [{
    return ($Arg1VarName& $Arg2VarName) | ($Arg0VarName & ~$Arg2VarName);
  }]>;
OclBuiltinImpl bitselect_scalar_f32 = OclBuiltinImpl<bitselect, [v1f32], 0, [{
    return (as_int($Arg1VarName) & as_int($Arg2VarName)) | (as_int($Arg0VarName) & ~as_int($Arg2VarName));
  }]>;
OclBuiltinImpl bitselect_scalar_f64 = OclBuiltinImpl<bitselect, [v1f64], 0, [{
    return (as_long($Arg1VarName) & as_long($Arg2VarName)) | (as_long($Arg0VarName) & ~as_long($Arg2VarName));
  }]>;
OclBuiltinImpl bitselect_vector = OclBuiltinImpl<bitselect, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32,
                                                                      v8i64, v8u64, v16f32, v8f64], 0, [{
    int16 c_reg = (int16)ext_from($Arg2VarName);
    int16 a_reg = (int16)ext_from($Arg0VarName);
    int16 b_reg = (int16)ext_from($Arg1VarName);
    __m512i result_true = _mm512_and_pi((__m512i)b_reg, (__m512i)c_reg);
    __m512i result_false = _mm512_andn_pi((__m512i)a_reg, (__m512i)c_reg);
    result_true = _mm512_or_pi(result_true, result_false);
    $NativeReturnType result; result = trunc_to((int16)result_true, &result);
    return result;
  }]>;
OclBuiltinImpl bitselect_v16_promote = OclBuiltinImpl<bitselect, [v2i8, v3i8, v4i8, v8i8, v2u8, v3u8, v4u8, v8u8,
                                                      v2i16, v3i16, v4i16, v8i16, v2u16, v3u16, v4u16, v8u16,
                                                      v2i32, v3i32, v4i32, v8i32, v2u32, v3u32, v4u32, v8u32,
                                                      v2i64, v3i64, v4i64, v2u64, v3u64, v4u64,
                                                      v2f32, v3f32, v4f32, v8f32, v2f64, v3f64, v4f64, v8f64],
                                                      0, NonMaskPromote3>;
OclBuiltinImpl bitselect_f64_expand = OclBuiltinImpl<bitselect, [v16f64, v16u64, v16i64], 0, Expand3>;
