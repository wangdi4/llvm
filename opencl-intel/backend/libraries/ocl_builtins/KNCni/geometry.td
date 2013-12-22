// Copyright (c) 2006-2012 Intel Corporation
// Geometry
// OpenCL 1.2 rev 15, Table 6.13

// dot
OclBuiltinImpl dot_scalar = OclBuiltinImpl<dot, [v1f32, v1f64], 0, [{
    return $Arg0VarName * $Arg1VarName;
  }]>;
OclBuiltinImpl dot_f32_v2 = OclBuiltinImpl<dot, [v2f32], 0, [{
    float16 x_reg = (float16)0;
    float16 y_reg = (float16)0;
    x_reg.s01 = $Arg0VarName;
    y_reg.s01 = $Arg1VarName;
    __m512 result = (__m512)x_reg;
    result = _mm512_mul_ps(result, (__m512)(y_reg));
    add_reduce1_ps(result);
    return as_float16(result).s0;
  }]>;
OclBuiltinImpl dot_f32_v3_v4 = OclBuiltinImpl<dot, [v3f32, v4f32], 0, [{
    float16 x_reg = (float16)0;
    float16 y_reg = (float16)0;
    x_reg$Pattern = $Arg0VarName;
    y_reg$Pattern = $Arg1VarName;
    __m512 result = x_reg;
    result = _mm512_mul_ps(result, y_reg);
    add_reduce1_ps(result);
    add_reduce2_ps(result);
    return as_float16(result).s0;
  }]>;
OclBuiltinImpl dot_f64_v2 = OclBuiltinImpl<dot, [v2f64], 0, [{
    double8 x_reg = (double8)0;
    double8 y_reg = (double8)0;
    x_reg.s01 = $Arg0VarName;
    y_reg.s01 = $Arg1VarName;
    __m512d result = x_reg;
    result = _mm512_mul_pd(result, y_reg);
    add_reduce1_pd(result);
    return as_double8(result).s0;
  }]>;
OclBuiltinImpl dot_f64_v3_v4 = OclBuiltinImpl<dot, [v3f64, v4f64], 0, [{
    double8 x_reg = (double8)0;
    double8 y_reg = (double8)0;
    x_reg$Pattern = $Arg0VarName;
    y_reg$Pattern = $Arg1VarName;
    __m512d result = x_reg;
    result = _mm512_mul_pd(result, y_reg);
    add_reduce1_pd(result);
    add_reduce2_pd(result);
    return as_double8(result).s0;
  }]>;

// length
OclBuiltinImpl length_scalar = OclBuiltinImpl<length, [v1f32, v1f64], 0, [{
    return fabs($Arg0VarName);
  }]>;
OclBuiltinImpl length_f32_v2 = OclBuiltinImpl<length, [v2f32, v2f64], 0, [{
    return hypot($Arg0VarName.s0, $Arg0VarName.s1);
  }]>;
OclBuiltinImpl length_f32_v3_v4 = OclBuiltinImpl<length, [v3f32, v4f32], 0, [{
    float16 x_reg = (float16)0.0;
    x_reg$Pattern = $Arg0VarName;
    __m512d data = _mm512_cvtpslo_pd(x_reg); // avoid overflow of (x*x)
    data = _mm512_mul_pd(data, data);
    add_reduce1_pd(data);
    add_reduce2_pd(data);
    data = sqrt(as_double8(data));
    __m512 sum = _mm512_cvt_roundpd_pslo(data, _MM_FROUND_CUR_DIRECTION);
    return as_float16(sum).s0;
  }]>;
OclBuiltinImpl length_f64_v3_v4 = OclBuiltinImpl<length, [v3f64, v4f64], 0, [{
    double8 data = (double8)0.0;
    data$Pattern = $Arg0VarName;
    long8 sign_mask   = (long8)(0x7fffffffffffffffL);
    double8 exp_upper = (double8)(0x1p-510);            // the upper limit of exponent that might trigger overflow
    double8 exp_zero  = (double8)(0x1p-1);              // zero exponent
    double8 exp_lower = (double8)(0x1p+510);            // the lower limit of exponent that might trigger underflow
    double8 norm_up   = (double8)(0x1p-512);            // the upper limit of exponent that might trigger overflow
    double8 norm_down = (double8)(0x1p+512);            // the lower limit of exponent that might trigger underflow

    double8 norm    = (double8)(1.0);
    double8 denorm  = (double8)(1.0);
    // Get the norm and denorm
    __m512i abs_data = _mm512_and_epi64((__m512i)data, (__m512i)sign_mask);
    // The "any" function is implemented through _mm512_kortestz, so there should result no (or almost no)
    // branch prediction penality
    __mmask8 match     = _mm512_cmplt_epi64((__m512i)abs_data, (__m512i)exp_upper);
    __mmask8 zero_exp  = _mm512_cmplt_epi64(abs_data, (__m512i)exp_zero);
    // Any of the data is smaller than the underflow threshold, pluse none of the data will result overflow after multiplied
    // with norm; !_mm512_kortestz is a "any" operation and _mm512_kortestc is a "all" operation
    if (!_mm512_kortestz(match, match) &&
        ((zero_exp & 0xFF) == 0xFF)){
        //_mm512_kortestc(zero_exp, zero_exp)){
      norm    = norm_down;
      denorm  = norm_up;
    }

    // According to OpenCL manual, overflow needs to be addressed first, then underflow
    match = _mm512_cmplt_epi64((__m512i)exp_lower, (__m512i)abs_data);
    if (match){
      norm    = norm_up;
      denorm  = norm_down;
    }

    data = _mm512_mul_pd(data, norm);
    data = _mm512_mul_pd(data, data);

    add_reduce1_pd(data);
    add_reduce2_pd(data);

    return sqrt(data.s0)*denorm.s0;
    return as_double8(abs_data).s0;
  }]>;

// normalize
OclBuiltinImpl normalize_f32_scalar = OclBuiltinImpl<normalize, [v1f32, v1f64], 0, [{
    return ($Arg0VarName==0) ? 0.0f :
           isnan($Arg0VarName) ? $Arg0VarName :
           ($Arg0VarName < 0) ? -1.0f : 1.0f;
  }]>;
OclBuiltinImpl normalize_f32_vector = OclBuiltinImpl<normalize, [v2f32, v3f32, v4f32], 0, [{
    float16 x_reg = (float16)0.0;
    x_reg$Pattern = $Arg0VarName;
    int16 const_vector_msb = (int16)0x80000000;
    float16 one = (float16)1.0f;
    int16 ifnan = isnan(x_reg);
    int16 ifinf = isinf(x_reg);
    float16 all_nan = (float16)as_float(0xFF800000); // float NAN

    // one_vector=( x_reg >0)?1.0:-1.0
    __m512i sign_mask = _mm512_and_epi32((__m512i)x_reg, (__m512i)((int16)const_vector_msb));
    __m512i one_vector = _mm512_or_epi32((__m512i)one, sign_mask); // 1 *sign_bit(x)

    // data = isinf(x_reg)?one:data
    __m512i data = (__m512i)x_reg;
    data = _mm512_andnot_epi32((__m512i)ifinf, data);
    one_vector = _mm512_and_epi32((__m512i)ifinf, one_vector);
    data = _mm512_or_epi32(data, one_vector);

    // returun 16NAN if any of the element is NAN
    if (any(ifnan))
      return all_nan$Pattern;

    __mmask16 equal = _mm512_cmpeq_ps((__m512)x_reg, (__m512)((float16)0.0));
    int flag;

    // <Caveat> Use a more efficient 4-element length since normalize only support up to 4 elements
    double8 x_dp = _mm512_cvtpslo_pd(x_reg);
    double norm = length(x_dp$Pattern);

    // norm = all_zero?1.0:norm;
    flag = _mm512_kortestc(equal, equal);
    norm = flag?1.0:norm;

    double8 result = x_dp/(double8)norm;
    float16 spResult = _mm512_cvt_roundpd_pslo(result, _MM_FROUND_CUR_DIRECTION);

    return spResult$Pattern;
  }]>;
OclBuiltinImpl normalize_f64_vector = OclBuiltinImpl<normalize, [v2f64, v3f64, v4f64], 0, [{
    double8 x_reg = (double8)0.0;
    x_reg$Pattern = $Arg0VarName;
    __m512i const_vector_long_msb= (__m512i)((long8)0x8000000000000000L);
    double8 one = (double8)1.0;
    long8 ifnan = isnan(x_reg);
    long8 ifinf = isinf(x_reg);
    double8 all_nan = (double8)as_double(0xFFF0000000000000L); // DP NAN

    // one_vector=( x_reg >0)?1.0:-1.0
    __m512i sign_mask = _mm512_and_epi64((__m512i)x_reg, const_vector_long_msb);
    __m512i one_vector = _mm512_or_epi64((__m512i)one, sign_mask); // 1 *sign_bit(x)

    // data = isinf(x_reg)?one:data
    __m512i data = (__m512i)(x_reg);
    data = _mm512_andnot_epi64(data, (__m512i)ifinf);
    one_vector = _mm512_and_epi64(one_vector, (__m512i)ifinf);
    data = _mm512_or_epi64(data, one_vector);

    // returun 16NAN if any of the element is NAN
    if (any(ifnan))
      return all_nan$Pattern;

    __mmask8 equal = _mm512_cmpeq_pd(x_reg, (double8)0.0);
    int flag;

    double sum = x_reg.s0 * x_reg.s0 + x_reg.s1 * x_reg.s1 + x_reg.s2 * x_reg.s2 + x_reg.s3 * x_reg.s3;
    double8 x_norm = x_reg;
    double8 norm_o = (double8)1.0;
    double8 norm_of = (double8)0x1.0p-513;
    double8 norm_uf = (double8)0x1.0p700;

    norm_o = isinf(sum) ? norm_of : norm_o;
    norm_o = isless(sum, (2 * DBL_MIN / DBL_EPSILON)) ? norm_uf : norm_o;

    // <Caveat> Use a more efficient 4-element length since normalize only support up to 4 elements
    double norm = length((x_norm*norm_o)$Pattern);

    // norm = all_zero?1.0:norm;
    flag = _mm512_kortestc(equal, equal);
    norm = flag?1.0:norm;

    double8 result = (x_norm * norm_o)/norm;

    return result$Pattern;
  }]>;

// cross
OclBuiltinImpl cross_f32_v3 = OclBuiltinImpl<cross, [v3f32], 0, [{
    __m512 zeros = (__m512)((float16)0.0);
    __m512 result;
    float16 f_result = (float16)0.0;
    __m512 x_reg, y_reg;
    __mmask16 k;

    f_result.s012 = $Arg0VarName;
    x_reg = (__m512)f_result;
    s120_s012_ps(x_reg, zeros, k);

    f_result.s012 = $Arg1VarName;
    y_reg = (__m512)f_result;
    y_reg = s201_s012_ps(y_reg, k);

    x_reg = _mm512_mul_ps(x_reg, y_reg);
    result = x_reg;

    f_result.s012 = $Arg0VarName;
    x_reg = (__m512)f_result;
    x_reg = s201_s012_ps(x_reg, k);

    f_result.s012 = $Arg1VarName;
    y_reg = (__m512)f_result;
    s120_s012_ps(y_reg, zeros, k); // imply k <--0x7

    y_reg = _mm512_mul_ps(y_reg, x_reg);
    result = _mm512_sub_ps(result, y_reg);

    return (as_float16(result))$Pattern;
  }]>;
OclBuiltinImpl cross_f32_v4 = OclBuiltinImpl<cross, [v4f32], 0, [{
    float3 temp = cross ($Arg0VarName.s012, $Arg1VarName.s012);
    float4 result;
    result.s012 = temp;
    result.s3 =0;
    return result;
  }]>;
OclBuiltinImpl cross_f64_v3 = OclBuiltinImpl<cross, [v3f64], 0, [{
    __m512d zeros = (double8)0.0;
    __m512d result;
    double8 f_result = (double8)0.0;
    __m512d x_reg, y_reg;
    __mmask8 k;

    f_result.s012 = $Arg0VarName;
    x_reg = f_result;
    s120_s012_pd(x_reg, zeros, k);

    f_result.s012 = $Arg1VarName;
    y_reg = f_result;
    y_reg = s201_s012_pd(y_reg, k);

    x_reg = _mm512_mul_pd(x_reg, y_reg);
    result = x_reg;

    f_result.s012 = $Arg0VarName;
    x_reg = f_result;
    x_reg = s201_s012_pd(x_reg, k);

    f_result.s012 = $Arg1VarName;
    y_reg = f_result;
    s120_s012_pd(y_reg, zeros, k); // imply k <--0x7

    y_reg = _mm512_mul_pd(y_reg, x_reg);
    result = _mm512_sub_pd(result, y_reg);

    return as_double8(result).s012;
  }]>;
OclBuiltinImpl cross_f64_v4 = OclBuiltinImpl<cross, [v4f64], 0, [{
    double3 temp = cross ($Arg0VarName.s012, $Arg1VarName.s012);
    double4 result;
    result.s012 = temp;
    result.s3 =0;
    return result;
  }]>;

// fast_normalize
OclBuiltinImpl fast_normalize_scalar = OclBuiltinImpl<fast_normalize, [v1f32], 0, [{
    return (fabs($Arg0VarName) < sqrt(FLT_MIN))?$Arg0VarName:($Arg0VarName/fabs($Arg0VarName));
  }]>;
OclBuiltinImpl fast_normalize_f32_vector = OclBuiltinImpl<fast_normalize, [v2f32, v3f32, v4f32], 0, [{
    float16 x_reg = (float16)0.0;
    x_reg$Pattern = $Arg0VarName;
    // Below instruction is added to force FTZ happens when program runs
    // on FTZ mode; no mode checking is necessary
    float16 ref = sqrt(x_reg);

    __m512 temp;
    float sum;
    __m512 zeros = _mm512_setzero();
    __mmask16 small_val = _mm512_cmpeq_ps(ref, zeros);
    temp = (__m512)x_reg;
    temp = (__m512)_mm512_mask_and_epi32((__m512i)temp, small_val, (__m512i)zeros, (__m512i)zeros);
    temp = _mm512_mul_ps(temp, temp);
    add_reduce1_ps(temp);
    add_reduce2_ps(temp);
    sum = as_float16(temp).s0;

    // Below include checking if all(source == 0) and sum-of-squae is < FLT_MIN
    sum = (sum < FLT_MIN)?1.0:sum;

    sum = half_rsqrt(sum);
    temp = (__m512)x_reg;
    temp = _mm512_mul_ps(temp, (__m512)((float16)sum));
  
    return (as_float16(temp))$Pattern;
  }]>;

// Extend SOA Geometry built-in functions
// 
code ZeroPromote2 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName = ($NativeArg0Type)0; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName = ($NativeArg1Type)0; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName);
    return $ReturnVarName$Pattern;
  }];
  
code ZeroPromote3 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName = ($NativeArg0Type)0; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName = ($NativeArg1Type)0; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg2VarName; n$Arg2VarName = ($NativeArg2Type)0; n$Arg2VarName$Pattern = $Arg2VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName, n$Arg2VarName);
    return $ReturnVarName$Pattern;
  }];
  
code NonMaskPromote4 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg2VarName; n$Arg2VarName$Pattern = $Arg2VarName;
    $NativeArg3Type n$Arg3VarName; n$Arg3VarName$Pattern = $Arg3VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName, n$Arg2VarName, n$Arg3VarName);
    return $ReturnVarName$Pattern;
  }];
  
code ZeroPromote4 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName = ($NativeArg0Type)0; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName = ($NativeArg1Type)0; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg2VarName; n$Arg2VarName = ($NativeArg2Type)0; n$Arg2VarName$Pattern = $Arg2VarName;
    $NativeArg3Type n$Arg3VarName; n$Arg3VarName = ($NativeArg3Type)0; n$Arg3VarName$Pattern = $Arg3VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc($Mask, n$Arg0VarName, n$Arg1VarName, n$Arg2VarName, n$Arg3VarName);
    return $ReturnVarName$Pattern;
  }];

code Expand4 = [{
    $ReturnType $ReturnVarName;
    $ReturnVarName$ExpandLoPattern = $ExpandLoFunc($Arg0VarName$ExpandLoPattern, $Arg1VarName$ExpandLoPattern, $Arg2VarName$ExpandLoPattern, $Arg3VarName$ExpandLoPattern);
    $ReturnVarName$ExpandHiPattern = $ExpandHiFunc($Arg0VarName$ExpandHiPattern, $Arg1VarName$ExpandHiPattern, $Arg2VarName$ExpandHiPattern, $Arg3VarName$ExpandHiPattern);
    return $ReturnVarName;
  }];
  
code NonMaskPromote6 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg2VarName; n$Arg2VarName$Pattern = $Arg2VarName;
    $NativeArg3Type n$Arg3VarName; n$Arg3VarName$Pattern = $Arg3VarName;
    $NativeArg4Type n$Arg4VarName; n$Arg4VarName$Pattern = $Arg4VarName;
    $NativeArg5Type n$Arg5VarName; n$Arg5VarName$Pattern = $Arg5VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName, n$Arg2VarName, n$Arg3VarName, n$Arg4VarName, n$Arg5VarName);
    return $ReturnVarName$Pattern;
  }];
  
code ZeroPromote6 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName = ($NativeArg0Type)0; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName = ($NativeArg1Type)0; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg2VarName; n$Arg2VarName = ($NativeArg2Type)0; n$Arg2VarName$Pattern = $Arg2VarName;
    $NativeArg3Type n$Arg3VarName; n$Arg3VarName = ($NativeArg3Type)0; n$Arg3VarName$Pattern = $Arg3VarName;
    $NativeArg4Type n$Arg2VarName; n$Arg4VarName = ($NativeArg4Type)0; n$Arg4VarName$Pattern = $Arg4VarName;
    $NativeArg5Type n$Arg3VarName; n$Arg5VarName = ($NativeArg5Type)0; n$Arg5VarName$Pattern = $Arg5VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName, n$Arg2VarName, n$Arg3VarName, n$Arg4VarName, n$Arg5VarName);
    return $ReturnVarName$Pattern;
  }];

code Expand6 = [{
    $ReturnType $ReturnVarName;
    $ReturnVarName$ExpandLoPattern = $ExpandLoFunc($Arg0VarName$ExpandLoPattern, $Arg1VarName$ExpandLoPattern, $Arg2VarName$ExpandLoPattern, $Arg3VarName$ExpandLoPattern, $Arg4VarName$ExpandLoPattern, $Arg5VarName$ExpandLoPattern);
    $ReturnVarName$ExpandHiPattern = $ExpandHiFunc($Arg0VarName$ExpandHiPattern, $Arg1VarName$ExpandHiPattern, $Arg2VarName$ExpandHiPattern, $Arg3VarName$ExpandHiPattern, $Arg4VarName$ExpandHiPattern, $Arg5VarName$ExpandHiPattern);
    return $ReturnVarName;
  }];
  
code NonMaskPromote8 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg2VarName; n$Arg2VarName$Pattern = $Arg2VarName;
    $NativeArg3Type n$Arg3VarName; n$Arg3VarName$Pattern = $Arg3VarName;
    $NativeArg4Type n$Arg4VarName; n$Arg4VarName$Pattern = $Arg4VarName;
    $NativeArg5Type n$Arg5VarName; n$Arg5VarName$Pattern = $Arg5VarName;
    $NativeArg6Type n$Arg6VarName; n$Arg6VarName$Pattern = $Arg6VarName;
    $NativeArg7Type n$Arg7VarName; n$Arg7VarName$Pattern = $Arg7VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName, n$Arg2VarName, n$Arg3VarName, n$Arg4VarName, n$Arg5VarName, n$Arg6VarName, n$Arg7VarName);
    return $ReturnVarName$Pattern;
  }];
  
code ZeroPromote8 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName = ($NativeArg0Type)0; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName = ($NativeArg1Type)0; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg2VarName; n$Arg2VarName = ($NativeArg2Type)0; n$Arg2VarName$Pattern = $Arg2VarName;
    $NativeArg3Type n$Arg3VarName; n$Arg3VarName = ($NativeArg3Type)0; n$Arg3VarName$Pattern = $Arg3VarName;
    $NativeArg4Type n$Arg4VarName; n$Arg4VarName = ($NativeArg4Type)0; n$Arg4VarName$Pattern = $Arg4VarName;
    $NativeArg5Type n$Arg5VarName; n$Arg5VarName = ($NativeArg5Type)0; n$Arg5VarName$Pattern = $Arg5VarName;
    $NativeArg6Type n$Arg6VarName; n$Arg6VarName = ($NativeArg6Type)0; n$Arg6VarName$Pattern = $Arg6VarName;
    $NativeArg7Type n$Arg7VarName; n$Arg7VarName = ($NativeArg7Type)0; n$Arg7VarName$Pattern = $Arg7VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName, n$Arg2VarName, n$Arg3VarName, n$Arg4VarName, n$Arg5VarName, n$Arg6VarName, n$Arg7VarName);
    return $ReturnVarName$Pattern;
  }];

code Expand8 = [{
    $ReturnType $ReturnVarName;
    $ReturnVarName$ExpandLoPattern = $ExpandLoFunc($Arg0VarName$ExpandLoPattern, $Arg1VarName$ExpandLoPattern, $Arg2VarName$ExpandLoPattern, $Arg3VarName$ExpandLoPattern, $Arg4VarName$ExpandLoPattern, $Arg5VarName$ExpandLoPattern, $Arg6VarName$ExpandLoPattern, $Arg7VarName$ExpandLoPattern);
    $ReturnVarName$ExpandHiPattern = $ExpandHiFunc($Arg0VarName$ExpandHiPattern, $Arg1VarName$ExpandHiPattern, $Arg2VarName$ExpandHiPattern, $Arg3VarName$ExpandHiPattern, $Arg4VarName$ExpandHiPattern, $Arg5VarName$ExpandHiPattern, $Arg6VarName$ExpandHiPattern, $Arg7VarName$ExpandHiPattern);
    return $ReturnVarName;
  }];

list<OclType> fp_native_types  = [v16f32, v8f64];
list<OclType> fp_promote_types = [v4f32, v8f32, v4f64];
list<OclType> fp_expand_types  = [v16f64];
list<OclType> fp_soa_types     = [v4f32, v8f32, v16f32, v4f64, v8f64, v16f64];
list<OclType> sp_soa_types     = [v4f32, v8f32, v16f32];
list<OclType> dp_soa_types     = [v4f64, v8f64, v16f64];
list<OclType> dp_native_types  = [v8f64];
list<OclType> dp_promote_types = [v4f64];
list<OclType> dp_expand_types  = [v16f64];
list<OclType> sp_native_types  = [v16f32];
list<OclType> sp_promote_types = [v4f32, v8f32];

// soa_dot
OclBuiltinImpl soa_dot1_native = OclBuiltinImpl<soa_dot1, fp_native_types, 0, [{
    return ($Arg0VarName * $Arg1VarName);
  }]>;
OclBuiltinImpl soa_dot1_promote = OclBuiltinImpl<soa_dot1, fp_promote_types, 0, NonMaskPromote2>;
OclBuiltinImpl soa_dot1_expand  = OclBuiltinImpl<soa_dot1, fp_expand_types, 0, Expand2>;

OclBuiltinImpl soa_dot2_native = OclBuiltinImpl<soa_dot2, fp_native_types, 0, [{
    return ($Arg0VarName * $Arg2VarName)+ ($Arg1VarName * $Arg3VarName);
  }]>; 
OclBuiltinImpl soa_dot2_promote = OclBuiltinImpl<soa_dot2, fp_promote_types, 0, NonMaskPromote4>;
OclBuiltinImpl soa_dot2_expand  = OclBuiltinImpl<soa_dot2, fp_expand_types, 0, Expand4>;

OclBuiltinImpl soa_dot3_native = OclBuiltinImpl<soa_dot3, fp_native_types, 0, [{
    return ($Arg0VarName * $Arg3VarName) + ($Arg1VarName * $Arg4VarName) + ($Arg2VarName * $Arg5VarName);
  }]>;
OclBuiltinImpl soa_dot3_promote = OclBuiltinImpl<soa_dot3, fp_promote_types, 0, NonMaskPromote6>;
OclBuiltinImpl soa_dot3_expand  = OclBuiltinImpl<soa_dot3, fp_expand_types, 0, Expand6>;
  
OclBuiltinImpl soa_dot4_native = OclBuiltinImpl<soa_dot4, fp_native_types, 0, [{
    return ($Arg0VarName * $Arg4VarName) + ($Arg1VarName * $Arg5VarName) + ($Arg2VarName * $Arg6VarName) + ($Arg3VarName * $Arg7VarName);
  }]>;
OclBuiltinImpl soa_dot4_promote = OclBuiltinImpl<soa_dot4, fp_promote_types, 0, NonMaskPromote8>;
OclBuiltinImpl soa_dot4_expand  = OclBuiltinImpl<soa_dot4, fp_expand_types, 0, Expand8>;

// soa_cross
OclBuiltinImpl soa_cross3_native = OclBuiltinImpl<soa_cross3, fp_native_types, 0, [{
    *$Arg6VarName = ($Arg1VarName * $Arg5VarName) - ($Arg2VarName * $Arg4VarName);
    *$Arg7VarName = ($Arg2VarName * $Arg3VarName) - ($Arg0VarName * $Arg5VarName);
    *$Arg8VarName = ($Arg0VarName * $Arg4VarName) - ($Arg1VarName * $Arg3VarName);
  }]>;
OclBuiltinImpl soa_cross3_promote = OclBuiltinImpl<soa_cross3, fp_promote_types, 0, [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg2VarName; n$Arg2VarName$Pattern = $Arg2VarName;
    $NativeArg3Type n$Arg3VarName; n$Arg3VarName$Pattern = $Arg3VarName;
    $NativeArg4Type n$Arg4VarName; n$Arg4VarName$Pattern = $Arg4VarName;
    $NativeArg5Type n$Arg5VarName; n$Arg5VarName$Pattern = $Arg5VarName;
    $NativeArg6Type n$Arg6VarName;
    $NativeArg7Type n$Arg7VarName;
    $NativeArg8Type n$Arg8VarName;
    soa_cross3(n$Arg0VarName, n$Arg1VarName, n$Arg2VarName, n$Arg3VarName, n$Arg4VarName, n$Arg5VarName, &n$Arg6VarName, &n$Arg7VarName, &n$Arg8VarName);
    *$Arg6VarName = n$Arg6VarName$Pattern;
    *$Arg7VarName = n$Arg7VarName$Pattern;
    *$Arg8VarName = n$Arg8VarName$Pattern;
  }]>;    
OclBuiltinImpl soa_cross3_expand = OclBuiltinImpl<soa_cross3, fp_expand_types, 0, [{ 
    double8 temp_x, temp_y, temp_z;
    soa_cross3($Arg0VarName$ExpandLoPattern, $Arg1VarName$ExpandLoPattern, $Arg2VarName$ExpandLoPattern, $Arg3VarName$ExpandLoPattern, $Arg4VarName$ExpandLoPattern, $Arg5VarName$ExpandLoPattern, &temp_x, &temp_y, &temp_z);   
    (*$Arg6VarName)$ExpandLoPattern = temp_x;
    (*$Arg7VarName)$ExpandLoPattern = temp_y;
    (*$Arg8VarName)$ExpandLoPattern = temp_z;
    soa_cross3($Arg0VarName$ExpandHiPattern, $Arg1VarName$ExpandHiPattern, $Arg2VarName$ExpandHiPattern, $Arg3VarName$ExpandHiPattern, $Arg4VarName$ExpandHiPattern, $Arg5VarName$ExpandHiPattern, &temp_x, &temp_y, &temp_z);
    (*$Arg6VarName)$ExpandHiPattern = temp_x;
    (*$Arg7VarName)$ExpandHiPattern = temp_y;
    (*$Arg8VarName)$ExpandHiPattern = temp_z;
  }]>;
   
OclBuiltinImpl fp_soa_cross4 = OclBuiltinImpl<soa_cross4, fp_soa_types, 0, [{
    soa_cross3($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg8VarName, $Arg9VarName, $Arg10VarName);
    *$Arg11VarName = ($Arg0Type)0.0;
  }]>;

// soa_length
code soa_length2_cond_sp = [{
    $Arg0Type sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName;
    if (any(isinf(sum)) || any(isless(sum, ($Arg0Type)(2*FLT_MIN/FLT_EPSILON) ))){
      return length2_up_convert($Arg0VarName, $Arg1VarName);
    }
    return sqrt(sum);
  }];
code soa_length2_always_sp = [{
    return length2_up_convert($Arg0VarName, $Arg1VarName);
  }];

code soa_length3_cond_sp = [{
    $Arg0Type sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName;
    if (any(isinf(sum)) || any(isless(sum, ($Arg0Type)(2*FLT_MIN/FLT_EPSILON) ))){
      return length3_up_convert($Arg0VarName, $Arg1VarName, $Arg2VarName);
    }
    return sqrt(sum);
  }];
code soa_length3_always_sp = [{
    return length3_up_convert($Arg0VarName, $Arg1VarName, $Arg2VarName);
  }];
  
code soa_length4_cond_sp = [{
    $Arg0Type sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName + $Arg3VarName * $Arg3VarName;
    if (any(isinf(sum)) || any(isless(sum, ($Arg0Type)(2*FLT_MIN/FLT_EPSILON) ))){
      return length4_up_convert($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);
    }
    return sqrt(sum);
  }];
code soa_length4_always_sp = [{
    return length4_up_convert($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);
  }];
  
// soa_length1
OclBuiltinImpl sp_soa_length1 = OclBuiltinImpl<soa_length1, fp_soa_types, 0, [{
    return fabs($Arg0VarName);
  }]>;
  
// soa_length2
OclBuiltinImpl sp_soa_length2_native = OclBuiltinImpl<soa_length2, sp_native_types, 0, soa_length2_always_sp>;
OclBuiltinImpl sp_soa_length2_promote = OclBuiltinImpl<soa_length2, sp_promote_types, 0, NonMaskPromote2>;

OclBuiltinImpl dp_soa_length2_native = OclBuiltinImpl<soa_length2, dp_native_types, 0, [{
    $Arg0Type sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName;
    $Arg0Type norm = ($Arg0Type)1.0;
    $Arg0Type denorm = ($Arg0Type)1.0;
    $Arg0Type norm_of = ($Arg0Type)0x1.0p-600;
    $Arg0Type norm_uf = ($Arg0Type)0x1.0p700;
    $Arg0Type denorm_of = ($Arg0Type)0x1.0p600;
    $Arg0Type denorm_uf = ($Arg0Type)0x1.0p-700;
  
    norm    = select(norm, norm_of, isinf(sum));
    denorm  = select(denorm, denorm_of, isinf(sum));
    norm    = select(norm, norm_uf, isless(sum, ($Arg0Type)(2 * DBL_MIN / DBL_EPSILON)));
    denorm  = select(denorm, denorm_uf, isless(sum, ($Arg0Type)(2 * DBL_MIN / DBL_EPSILON)));
  
    $Arg0Type new_$Arg0VarName = $Arg0VarName * norm;
    $Arg1Type new_$Arg1VarName = $Arg1VarName * norm;
    return sqrt( new_$Arg0VarName * new_$Arg0VarName + new_$Arg1VarName * new_$Arg1VarName)* denorm;  
  }]>; 
OclBuiltinImpl dp_soa_length2_promote = OclBuiltinImpl<soa_length2, dp_promote_types, 0, NonMaskPromote2>;
OclBuiltinImpl dp_soa_length2_expand = OclBuiltinImpl<soa_length2, dp_expand_types, 0, Expand2>;
// soa_length3
OclBuiltinImpl sp_soa_length3_native = OclBuiltinImpl<soa_length3, sp_native_types, 0, soa_length3_always_sp>;
OclBuiltinImpl sp_soa_length3_promote = OclBuiltinImpl<soa_length3, sp_promote_types, 0, NonMaskPromote3>;

OclBuiltinImpl dp_soa_length3_native = OclBuiltinImpl<soa_length3, dp_native_types, 0, [{
    $Arg0Type sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName;
    $Arg0Type norm = ($Arg0Type)1.0;
    $Arg0Type denorm = ($Arg0Type)1.0;
    $Arg0Type norm_of = ($Arg0Type)0x1.0p-600;
    $Arg0Type norm_uf = ($Arg0Type)0x1.0p700;
    $Arg0Type denorm_of = ($Arg0Type)0x1.0p600;
    $Arg0Type denorm_uf = ($Arg0Type)0x1.0p-700;
  
    norm    = select(norm, norm_of, isinf(sum));
    denorm  = select(denorm, denorm_of, isinf(sum));
    norm    = select(norm, norm_uf, isless(sum, ($Arg0Type)(3 * DBL_MIN / DBL_EPSILON)));
    denorm  = select(denorm, denorm_uf, isless(sum, ($Arg0Type)(3 * DBL_MIN / DBL_EPSILON)));
  
    $Arg0Type new_$Arg0VarName = $Arg0VarName * norm;
    $Arg1Type new_$Arg1VarName = $Arg1VarName * norm;
    $Arg2Type new_$Arg2VarName = $Arg2VarName * norm;
    return sqrt( new_$Arg0VarName * new_$Arg0VarName + new_$Arg1VarName * new_$Arg1VarName + new_$Arg2VarName * new_$Arg2VarName)* denorm;  
  }]>; 
OclBuiltinImpl dp_soa_length3_promote = OclBuiltinImpl<soa_length3, dp_promote_types, 0, NonMaskPromote3>;
OclBuiltinImpl dp_soa_length3_expand = OclBuiltinImpl<soa_length3, dp_expand_types, 0, Expand3>;
// soa_length4
OclBuiltinImpl sp_soa_length4_native = OclBuiltinImpl<soa_length4, sp_native_types, 0, soa_length4_always_sp>;
OclBuiltinImpl sp_soa_length4_promote = OclBuiltinImpl<soa_length4, sp_promote_types, 0, NonMaskPromote4>;

OclBuiltinImpl dp_soa_length4_native = OclBuiltinImpl<soa_length4, dp_native_types, 0, [{
    $Arg0Type sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName + $Arg3VarName * $Arg3VarName;
    $Arg0Type norm = ($Arg0Type)1.0;
    $Arg0Type denorm = ($Arg0Type)1.0;
    $Arg0Type norm_of = ($Arg0Type)0x1.0p-600;
    $Arg0Type norm_uf = ($Arg0Type)0x1.0p700;
    $Arg0Type denorm_of = ($Arg0Type)0x1.0p600;
    $Arg0Type denorm_uf = ($Arg0Type)0x1.0p-700;
  
    norm    = select(norm, norm_of, isinf(sum));
    denorm  = select(denorm, denorm_of, isinf(sum));
    norm    = select(norm, norm_uf, isless(sum, ($Arg0Type)(4 * DBL_MIN / DBL_EPSILON)));
    denorm  = select(denorm, denorm_uf, isless(sum, ($Arg0Type)(4 * DBL_MIN / DBL_EPSILON)));
  
    $Arg0Type new_$Arg0VarName = $Arg0VarName * norm;
    $Arg1Type new_$Arg1VarName = $Arg1VarName * norm;
    $Arg2Type new_$Arg2VarName = $Arg2VarName * norm;
    $Arg3Type new_$Arg3VarName = $Arg3VarName * norm;
    return sqrt( new_$Arg0VarName * new_$Arg0VarName + new_$Arg1VarName * new_$Arg1VarName + new_$Arg2VarName * new_$Arg2VarName + new_$Arg3VarName * new_$Arg3VarName)* denorm;  
  }]>; 
OclBuiltinImpl dp_soa_length4_promote = OclBuiltinImpl<soa_length4, dp_promote_types, 0, NonMaskPromote4>;
OclBuiltinImpl dp_soa_length4_expand = OclBuiltinImpl<soa_length4, dp_expand_types, 0, Expand4>;

// soa_fast_length
// soa_fast_length1
OclBuiltinImpl sp_soa_fast_length1 = OclBuiltinImpl<soa_fast_length1, [v4f32, v8f32, v16f32], 0, [{
    return fabs($Arg0VarName);
  }]>;
// soa_fast_length2
OclBuiltinImpl sp_soa_fast_length2_native = OclBuiltinImpl<soa_fast_length2, sp_native_types, 0, [{
    $Arg0Type sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName;
    return half_sqrt(sum);
  }]>;
OclBuiltinImpl sp_soa_fast_length2_promote = OclBuiltinImpl<soa_fast_length2, sp_promote_types, 0, NonMaskPromote2>;
// soa_fast_length3
OclBuiltinImpl sp_soa_fast_length3_native = OclBuiltinImpl<soa_fast_length3, sp_native_types, 0, [{
    $Arg0Type sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName;
    return half_sqrt(sum);
  }]>;
OclBuiltinImpl sp_soa_fast_length3_promote = OclBuiltinImpl<soa_fast_length3, sp_promote_types, 0, NonMaskPromote3>;
// soa_fast_length4
OclBuiltinImpl sp_soa_fast_length4_native = OclBuiltinImpl<soa_fast_length4, sp_native_types, 0, [{
    $Arg0Type sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName + $Arg3VarName * $Arg3VarName;
    return half_sqrt(sum);
  }]>;
OclBuiltinImpl soa_fast_length4_promote = OclBuiltinImpl<soa_fast_length4, sp_promote_types, 0, NonMaskPromote4>;

// soa_normalize
// soa_normalize1
OclBuiltinImpl fp_soa_normalize1_native = OclBuiltinImpl<soa_normalize1, fp_native_types, 0, [{
    $NativeReturnType res = select(($NativeArg0Type)-1.0, ($NativeArg0Type)1.f, isgreater($Arg0VarName, ($NativeArg0Type)0.0) );
    res = select(res, $Arg0VarName, isnan($Arg0VarName));
    res = select(res, $Arg0VarName, isequal($Arg0VarName, ($NativeArg0Type)0.0) );
    return res;
  }]>;
OclBuiltinImpl fp_soa_normalize1_promote = OclBuiltinImpl<soa_normalize1, fp_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl fp_soa_normalize1_expand = OclBuiltinImpl<soa_normalize1, fp_expand_types, 0, Expand1>;
// soa_normalize2
OclBuiltinImpl sp_soa_normalize2_promote = OclBuiltinImpl<soa_normalize2, sp_promote_types, 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeArg1Type y_reg; y_reg$Pattern = $Arg1VarName;
    $NativeArg2Type temp_x;
    $NativeArg3Type temp_y;

    double8 x_dp = as_double8(_mm512_cvtpslo_pd( x_reg));
    double8 y_dp = as_double8(_mm512_cvtpslo_pd( y_reg));
    double8 sum = x_dp * x_dp + y_dp *y_dp;
    double8 length = sqrt(sum);
    double8 temp_x_dp = x_dp/length;
    double8 temp_y_dp = y_dp/length;
    long8 mask; mask = isequal(length, (double8)0.0);
    temp_x_dp = select(temp_x_dp, x_dp, mask);
    temp_y_dp = select(temp_y_dp, y_dp, mask);
    temp_x = _mm512_cvt_roundpd_pslo(temp_x_dp, _MM_FROUND_CUR_DIRECTION);
    temp_y = _mm512_cvt_roundpd_pslo(temp_y_dp, _MM_FROUND_CUR_DIRECTION);

    *$Arg2VarName = temp_x$Pattern;
    *$Arg3VarName = temp_y$Pattern;
  }]>;
OclBuiltinImpl sp_soa_normalize2_native = OclBuiltinImpl<soa_normalize2, sp_native_types, 0, [{
    float8 temp_x;
    float8 temp_y;
    soa_normalize2($Arg0VarName$ExpandLoPattern, $Arg1VarName$ExpandLoPattern, &temp_x, &temp_y);
    (*$Arg2VarName)$ExpandLoPattern = temp_x;
    (*$Arg3VarName)$ExpandLoPattern = temp_y;
    soa_normalize2($Arg0VarName$ExpandHiPattern, $Arg1VarName$ExpandHiPattern, &temp_x, &temp_y);
    (*$Arg2VarName)$ExpandHiPattern = temp_x;
    (*$Arg3VarName)$ExpandHiPattern = temp_y;
  }]>;

OclBuiltinImpl dp_soa_normalize2_native = OclBuiltinImpl<soa_normalize2, dp_native_types, 0, [{
    $Arg0Type sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName;
    $Arg0Type norm = ($Arg0Type)1.0;
    $Arg0Type norm_of = ($Arg0Type)0x1.0p-513;
    $Arg0Type norm_uf = ($Arg0Type)0x1.0p700;

    norm    = select(norm, norm_of, isinf(sum));
    norm    = select(norm, norm_uf, isless(sum, ($Arg0Type)(2 * DBL_MIN / DBL_EPSILON)));

    $Arg0Type new_$Arg0VarName = $Arg0VarName * norm;
    $Arg1Type new_$Arg1VarName = $Arg1VarName * norm;
    sum = new_$Arg0VarName * new_$Arg0VarName + new_$Arg1VarName * new_$Arg1VarName;
    $Arg0Type length = sqrt(sum);

    *$Arg2VarName = select(new_$Arg0VarName/length, $Arg0VarName, isequal(sum, ($NativeArg0Type)0.0) );
    *$Arg3VarName = select(new_$Arg1VarName/length, $Arg1VarName, isequal(sum, ($NativeArg0Type)0.0) );
  }]>;
OclBuiltinImpl dp_soa_normalize2_promote = OclBuiltinImpl<soa_normalize2, dp_promote_types, 0, [{
    $NativeArg2Type temp_x;
    $NativeArg3Type temp_y;
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    soa_normalize2(n$Arg0VarName, n$Arg1VarName, &temp_x, &temp_y);
    *$Arg2VarName = temp_x$Pattern;
    *$Arg3VarName = temp_y$Pattern;
  }]>;
OclBuiltinImpl dp_soa_normalize2_expand = OclBuiltinImpl<soa_normalize2, dp_expand_types, 0, [{
    double8 temp_x;
    double8 temp_y;
    soa_normalize2($Arg0VarName$ExpandLoPattern, $Arg1VarName$ExpandLoPattern, &temp_x, &temp_y);
    (*$Arg2VarName)$ExpandLoPattern = temp_x;
    (*$Arg3VarName)$ExpandLoPattern = temp_y;
    soa_normalize2($Arg0VarName$ExpandHiPattern, $Arg1VarName$ExpandHiPattern, &temp_x, &temp_y);
    (*$Arg2VarName)$ExpandHiPattern = temp_x;
    (*$Arg3VarName)$ExpandHiPattern = temp_y;
  }]>;

// soa_normalize3
OclBuiltinImpl sp_soa_normalize3_promote = OclBuiltinImpl<soa_normalize3, sp_promote_types, 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeArg1Type y_reg; y_reg$Pattern = $Arg1VarName;
    $NativeArg2Type z_reg; z_reg$Pattern = $Arg2VarName;
    $NativeArg3Type temp_x;
    $NativeArg4Type temp_y;
    $NativeArg5Type temp_z;

    double8 x_dp = as_double8(_mm512_cvtpslo_pd(x_reg));
    double8 y_dp = as_double8(_mm512_cvtpslo_pd(y_reg));
    double8 z_dp = as_double8(_mm512_cvtpslo_pd(z_reg));
    double8 sum = x_dp * x_dp + y_dp * y_dp + z_dp * z_dp;
    double8 length = sqrt(sum);
    double8 temp_x_dp = x_dp/length;
    double8 temp_y_dp = y_dp/length;
    double8 temp_z_dp = z_dp/length;
    long8 mask; mask = isequal(length, (double8)0.0);
    temp_x_dp = select(temp_x_dp, x_dp, mask);
    temp_y_dp = select(temp_y_dp, y_dp, mask);
    temp_z_dp = select(temp_z_dp, z_dp, mask);
    temp_x = _mm512_cvt_roundpd_pslo(temp_x_dp, _MM_FROUND_CUR_DIRECTION);
    temp_y = _mm512_cvt_roundpd_pslo(temp_y_dp, _MM_FROUND_CUR_DIRECTION);
    temp_z = _mm512_cvt_roundpd_pslo(temp_z_dp, _MM_FROUND_CUR_DIRECTION);

    *$Arg3VarName = temp_x$Pattern;
    *$Arg4VarName = temp_y$Pattern;
    *$Arg5VarName = temp_z$Pattern;
  }]>;
OclBuiltinImpl sp_soa_normalize3_native = OclBuiltinImpl<soa_normalize3, sp_native_types, 0, [{
    float8 temp_x;
    float8 temp_y;
    float8 temp_z;
    soa_normalize3($Arg0VarName$ExpandLoPattern, $Arg1VarName$ExpandLoPattern, $Arg2VarName$ExpandLoPattern, &temp_x, &temp_y, &temp_z);
    (*$Arg3VarName)$ExpandLoPattern = temp_x;
    (*$Arg4VarName)$ExpandLoPattern = temp_y;
    (*$Arg5VarName)$ExpandLoPattern = temp_z;
    soa_normalize3($Arg0VarName$ExpandHiPattern, $Arg1VarName$ExpandHiPattern, $Arg2VarName$ExpandHiPattern, &temp_x, &temp_y, &temp_z);
    (*$Arg3VarName)$ExpandHiPattern = temp_x;
    (*$Arg4VarName)$ExpandHiPattern = temp_y;
    (*$Arg5VarName)$ExpandHiPattern = temp_z;
  }]>;

OclBuiltinImpl dp_soa_normalize3_native = OclBuiltinImpl<soa_normalize3, dp_native_types, 0, [{
    $Arg0Type sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName;
    $Arg0Type norm = ($Arg0Type)1.0;
    $Arg0Type norm_of = ($Arg0Type)0x1.0p-513;
    $Arg0Type norm_uf = ($Arg0Type)0x1.0p700;

    norm    = select(norm, norm_of, isinf(sum));
    norm    = select(norm, norm_uf, isless(sum, ($Arg0Type)(2 * DBL_MIN / DBL_EPSILON)));

    $Arg0Type new_$Arg0VarName = $Arg0VarName * norm;
    $Arg1Type new_$Arg1VarName = $Arg1VarName * norm;
    $Arg2Type new_$Arg2VarName = $Arg2VarName * norm;
    sum = new_$Arg0VarName * new_$Arg0VarName + new_$Arg1VarName * new_$Arg1VarName + new_$Arg2VarName * new_$Arg2VarName;
    $Arg0Type length = sqrt(sum);

    *$Arg3VarName = select(new_$Arg0VarName/length, $Arg0VarName, isequal(sum, ($NativeArg0Type)0.0) );
    *$Arg4VarName = select(new_$Arg1VarName/length, $Arg1VarName, isequal(sum, ($NativeArg1Type)0.0) );
    *$Arg5VarName = select(new_$Arg2VarName/length, $Arg2VarName, isequal(sum, ($NativeArg2Type)0.0) );
  }]>;

OclBuiltinImpl dp_soa_normalize3_promote = OclBuiltinImpl<soa_normalize3, dp_promote_types, 0, [{
    $NativeArg3Type temp_x;
    $NativeArg4Type temp_y;
    $NativeArg5Type temp_z;
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg2VarName; n$Arg2VarName$Pattern = $Arg2VarName;
    soa_normalize3(n$Arg0VarName, n$Arg1VarName, n$Arg2VarName, &temp_x, &temp_y, &temp_z);
    *$Arg3VarName = temp_x$Pattern;
    *$Arg4VarName = temp_y$Pattern;
    *$Arg5VarName = temp_z$Pattern;
  }]>;
OclBuiltinImpl dp_soa_normalize3_expand = OclBuiltinImpl<soa_normalize3, dp_expand_types, 0, [{
    double8 temp_x;
    double8 temp_y;
    double8 temp_z;
    soa_normalize3($Arg0VarName$ExpandLoPattern, $Arg1VarName$ExpandLoPattern, $Arg2VarName$ExpandLoPattern, &temp_x, &temp_y, &temp_z);
    (*$Arg3VarName)$ExpandLoPattern = temp_x;
    (*$Arg4VarName)$ExpandLoPattern = temp_y;
    (*$Arg5VarName)$ExpandLoPattern = temp_z;
    soa_normalize3($Arg0VarName$ExpandHiPattern, $Arg1VarName$ExpandHiPattern, $Arg2VarName$ExpandHiPattern, &temp_x, &temp_y, &temp_z);
    (*$Arg3VarName)$ExpandHiPattern = temp_x;
    (*$Arg4VarName)$ExpandHiPattern = temp_y;
    (*$Arg5VarName)$ExpandHiPattern = temp_z;
  }]>;
// soa_normalize4
OclBuiltinImpl sp_soa_normalize4_promote = OclBuiltinImpl<soa_normalize4, sp_promote_types, 0, [{
    $NativeArg0Type x_reg; x_reg$Pattern = $Arg0VarName;
    $NativeArg1Type y_reg; y_reg$Pattern = $Arg1VarName;
    $NativeArg2Type z_reg; z_reg$Pattern = $Arg2VarName;
    $NativeArg3Type w_reg; w_reg$Pattern = $Arg3VarName;
    $NativeArg4Type temp_x;
    $NativeArg5Type temp_y;
    $NativeArg6Type temp_z;
    $NativeArg7Type temp_w;

    double8 x_dp = as_double8(_mm512_cvtpslo_pd(x_reg));
    double8 y_dp = as_double8(_mm512_cvtpslo_pd(y_reg));
    double8 z_dp = as_double8(_mm512_cvtpslo_pd(z_reg));
    double8 w_dp = as_double8(_mm512_cvtpslo_pd(w_reg));
    double8 sum = x_dp * x_dp + y_dp * y_dp + z_dp * z_dp + w_dp * w_dp;
    double8 length = sqrt(sum);
    double8 temp_x_dp = x_dp/length;
    double8 temp_y_dp = y_dp/length;
    double8 temp_z_dp = z_dp/length;
    double8 temp_w_dp = w_dp/length;
    long8 mask; mask = isequal(length, (double8)0.0);
    temp_x_dp = select(temp_x_dp, x_dp, mask);
    temp_y_dp = select(temp_y_dp, y_dp, mask);
    temp_z_dp = select(temp_z_dp, z_dp, mask);
    temp_w_dp = select(temp_w_dp, w_dp, mask);
    temp_x = _mm512_cvt_roundpd_pslo(temp_x_dp, _MM_FROUND_CUR_DIRECTION);
    temp_y = _mm512_cvt_roundpd_pslo(temp_y_dp, _MM_FROUND_CUR_DIRECTION);
    temp_z = _mm512_cvt_roundpd_pslo(temp_z_dp, _MM_FROUND_CUR_DIRECTION);
    temp_w = _mm512_cvt_roundpd_pslo(temp_w_dp, _MM_FROUND_CUR_DIRECTION);

    *$Arg4VarName = temp_x$Pattern;
    *$Arg5VarName = temp_y$Pattern;
    *$Arg6VarName = temp_z$Pattern;
    *$Arg7VarName = temp_w$Pattern;
  }]>;
OclBuiltinImpl sp_soa_normalize4_native = OclBuiltinImpl<soa_normalize4, sp_native_types, 0, [{
    float8 temp_x;
    float8 temp_y;
    float8 temp_z;
    float8 temp_w;
    soa_normalize4($Arg0VarName$ExpandLoPattern, $Arg1VarName$ExpandLoPattern, $Arg2VarName$ExpandLoPattern, $Arg3VarName$ExpandLoPattern, &temp_x, &temp_y, &temp_z, &temp_w);
    (*$Arg4VarName)$ExpandLoPattern = temp_x;
    (*$Arg5VarName)$ExpandLoPattern = temp_y;
    (*$Arg6VarName)$ExpandLoPattern = temp_z;
    (*$Arg7VarName)$ExpandLoPattern = temp_w;
    soa_normalize4($Arg0VarName$ExpandHiPattern, $Arg1VarName$ExpandHiPattern, $Arg2VarName$ExpandHiPattern, $Arg3VarName$ExpandHiPattern, &temp_x, &temp_y, &temp_z, &temp_w);
    (*$Arg4VarName)$ExpandHiPattern = temp_x;
    (*$Arg5VarName)$ExpandHiPattern = temp_y;
    (*$Arg6VarName)$ExpandHiPattern = temp_z;
    (*$Arg7VarName)$ExpandHiPattern = temp_w;
  }]>;

OclBuiltinImpl dp_soa_normalize4_native = OclBuiltinImpl<soa_normalize4, dp_native_types, 0, [{
    $Arg0Type sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName + $Arg3VarName * $Arg3VarName;
    $Arg0Type norm = ($Arg0Type)1.0;
    $Arg0Type norm_of = ($Arg0Type)0x1.0p-513;
    $Arg0Type norm_uf = ($Arg0Type)0x1.0p700;

    norm    = select(norm, norm_of, isinf(sum));
    norm    = select(norm, norm_uf, isless(sum, ($Arg0Type)(2 * DBL_MIN / DBL_EPSILON)));

    $Arg0Type new_$Arg0VarName = $Arg0VarName * norm;
    $Arg1Type new_$Arg1VarName = $Arg1VarName * norm;
    $Arg2Type new_$Arg2VarName = $Arg2VarName * norm;
    $Arg3Type new_$Arg3VarName = $Arg3VarName * norm;
    sum = new_$Arg0VarName * new_$Arg0VarName + new_$Arg1VarName * new_$Arg1VarName + new_$Arg2VarName * new_$Arg2VarName + new_$Arg3VarName * new_$Arg3VarName;
    $Arg0Type length = sqrt(sum);

    *$Arg4VarName = select(new_$Arg0VarName/length, $Arg0VarName, isequal(sum, ($NativeArg0Type)0.0) );
    *$Arg5VarName = select(new_$Arg1VarName/length, $Arg1VarName, isequal(sum, ($NativeArg1Type)0.0) );
    *$Arg6VarName = select(new_$Arg2VarName/length, $Arg2VarName, isequal(sum, ($NativeArg2Type)0.0) );
    *$Arg7VarName = select(new_$Arg3VarName/length, $Arg3VarName, isequal(sum, ($NativeArg3Type)0.0) );
  }]>;

OclBuiltinImpl dp_soa_normalize4_promote = OclBuiltinImpl<soa_normalize4, dp_promote_types, 0, [{
    $NativeArg4Type temp_x;
    $NativeArg5Type temp_y;
    $NativeArg6Type temp_z;
    $NativeArg7Type temp_w;
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg2VarName; n$Arg2VarName$Pattern = $Arg2VarName;
    $NativeArg3Type n$Arg3VarName; n$Arg3VarName$Pattern = $Arg3VarName;
    soa_normalize4(n$Arg0VarName, n$Arg1VarName, n$Arg2VarName, n$Arg3VarName, &temp_x, &temp_y, &temp_z, &temp_w);
    *$Arg4VarName = temp_x$Pattern;
    *$Arg5VarName = temp_y$Pattern;
    *$Arg6VarName = temp_z$Pattern;
    *$Arg7VarName = temp_w$Pattern;
  }]>;
OclBuiltinImpl dp_soa_normalize4_expand = OclBuiltinImpl<soa_normalize4, dp_expand_types, 0, [{
    double8 temp_x;
    double8 temp_y;
    double8 temp_z;
    double8 temp_w;
    soa_normalize4($Arg0VarName$ExpandLoPattern, $Arg1VarName$ExpandLoPattern, $Arg2VarName$ExpandLoPattern, $Arg3VarName$ExpandLoPattern, &temp_x, &temp_y, &temp_z, &temp_w);
    (*$Arg4VarName)$ExpandLoPattern = temp_x;
    (*$Arg5VarName)$ExpandLoPattern = temp_y;
    (*$Arg6VarName)$ExpandLoPattern = temp_z;
    (*$Arg7VarName)$ExpandLoPattern = temp_w;
    soa_normalize4($Arg0VarName$ExpandHiPattern, $Arg1VarName$ExpandHiPattern, $Arg2VarName$ExpandHiPattern, $Arg3VarName$ExpandHiPattern, &temp_x, &temp_y, &temp_z, &temp_w);
    (*$Arg4VarName)$ExpandHiPattern = temp_x;
    (*$Arg5VarName)$ExpandHiPattern = temp_y;
    (*$Arg6VarName)$ExpandHiPattern = temp_z;
    (*$Arg7VarName)$ExpandHiPattern = temp_w;
  }]>;

// soa_fast_normalize (sp only)
// soa_fast_normalize1
OclBuiltinImpl sp_soa_fast_normalize1_native = OclBuiltinImpl<soa_fast_normalize1, sp_native_types, 0, [{
    return select(($NativeReturnType)-1.0, ($NativeReturnType)1.0, isgreater($Arg0VarName, ($NativeReturnType)0.0));
  }]>;
OclBuiltinImpl sp_soa_fast_normalize1_promote = OclBuiltinImpl<soa_fast_normalize1, sp_promote_types, 0, NonMaskPromote1>;
// soa_fast_normalize2
OclBuiltinImpl sp_soa_fast_normalize2_native = OclBuiltinImpl<soa_fast_normalize2, sp_native_types, 0, [{
    $Arg0Type rnorm = half_rsqrt( $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName );
    *$Arg2VarName = $Arg0VarName * rnorm;
    *$Arg3VarName = $Arg1VarName * rnorm;
  }]>;
OclBuiltinImpl sp_soa_fast_normalize2_promote = OclBuiltinImpl<soa_fast_normalize2, sp_promote_types, 0, [{
    $NativeArg2Type temp_x;
    $NativeArg3Type temp_y;
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    soa_fast_normalize2(n$Arg0VarName, n$Arg1VarName, &temp_x, &temp_y);
    *$Arg2VarName = temp_x$Pattern;
    *$Arg3VarName = temp_y$Pattern;
  }]>;
// soa_fast_normalize3
OclBuiltinImpl sp_soa_fast_normalize3_native = OclBuiltinImpl<soa_fast_normalize3, sp_native_types, 0, [{
    $Arg0Type rnorm = half_rsqrt( $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName );
    *$Arg3VarName = $Arg0VarName * rnorm;
    *$Arg4VarName = $Arg1VarName * rnorm;
    *$Arg5VarName = $Arg2VarName * rnorm;
  }]>;
OclBuiltinImpl sp_soa_fast_normalize3_promote = OclBuiltinImpl<soa_fast_normalize3, sp_promote_types, 0, [{
    $NativeArg3Type temp_x;
    $NativeArg4Type temp_y;
    $NativeArg5Type temp_z;
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg2VarName; n$Arg2VarName$Pattern = $Arg2VarName;
    soa_fast_normalize3(n$Arg0VarName, n$Arg1VarName, n$Arg2VarName, &temp_x, &temp_y, &temp_z);
    *$Arg3VarName = temp_x$Pattern;
    *$Arg4VarName = temp_y$Pattern;  
    *$Arg5VarName = temp_z$Pattern;  
  }]>;
// soa_fast_normalize4
OclBuiltinImpl sp_soa_fast_normalize4_native = OclBuiltinImpl<soa_fast_normalize4, sp_native_types, 0, [{
    $Arg0Type rnorm = half_rsqrt( $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName + $Arg3VarName * $Arg3VarName );
    *$Arg4VarName = $Arg0VarName * rnorm;
    *$Arg5VarName = $Arg1VarName * rnorm;
    *$Arg6VarName = $Arg2VarName * rnorm;
    *$Arg7VarName = $Arg3VarName * rnorm;
  }]>;
OclBuiltinImpl sp_soa_fast_normalize4_promote = OclBuiltinImpl<soa_fast_normalize4, sp_promote_types, 0, [{
    $NativeArg4Type temp_x;
    $NativeArg5Type temp_y;
    $NativeArg6Type temp_z;
    $NativeArg7Type temp_w;
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg2VarName; n$Arg2VarName$Pattern = $Arg2VarName;
    $NativeArg3Type n$Arg3VarName; n$Arg3VarName$Pattern = $Arg3VarName;
    soa_fast_normalize4(n$Arg0VarName, n$Arg1VarName, n$Arg2VarName, n$Arg3VarName, &temp_x, &temp_y, &temp_z, &temp_w);
    *$Arg4VarName = temp_x$Pattern;
    *$Arg5VarName = temp_y$Pattern;
    *$Arg6VarName = temp_z$Pattern;
    *$Arg7VarName = temp_w$Pattern;
  }]>;

  
