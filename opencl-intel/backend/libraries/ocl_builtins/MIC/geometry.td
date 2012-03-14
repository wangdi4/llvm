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
    return((float16)result).s0;
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
    __m512d data = _mm512_cvtl_ps2pd(x_reg); // avoid overflow of (x*x)
    data = _mm512_mul_pd(data, data);
    add_reduce1_pd(data);
    add_reduce2_pd(data);
    __m512 sum = _mm512_undefined_ps();
    sum = _mm512_cvtl_pd2ps(sum, data, RC_RUN_DOWN);
    return sqrt(as_float16(sum).s0);
  }]>;
OclBuiltinImpl length_f64_v3_v4 = OclBuiltinImpl<length, [v3f64, v4f64], 0, [{
    double8 data = (double8)0.0;
    data$Pattern = $Arg0VarName;
    long8 sign_mask   = (long8)(0x7fffffffffffffffL);
    long8 exp_upper   = (long8)(0x5ff0000000000000L); // the upper limit of exponent that might trigger overflow
    long8 exp_zero    = (long8)(0x4000000000000000L); // zero exponent
    long8 exp_lower   = (long8)(0x2010000000000000L); // the lower limit of exponent that might trigger underflow

    double8 norm    = (double8)(1.0);
    double8 denorm  = (double8)(1.0);
    // Get the norm and denorm
    __m512i abs_data = _mm512_and_pq((__m512i)data, (__m512i)sign_mask);
    // The "any" function is implemented through _mm512_kortestz, so there should result no (or almost no)
    // branch prediction penality
    __mmask16 match     = _mm512_cmplt_pi(abs_data, (__m512i)exp_lower);
    __mmask16 zero_exp  = _mm512_cmple_pi(abs_data, (__m512i)exp_zero);
    // Any of the data is smaller than the underflow threshold, pluse none of the data will result overflow after multiplied
    // with norm; !_mm512_kortestz is a "any" operation and _mm512_kortestc is a "all" operation
    if (!_mm512_kortestz(match, match) && _mm512_kortestc(zero_exp, zero_exp)){
      norm    = (double8)exp_upper;
      denorm  = (double8)exp_lower;
    }

    // According to OpenCL manual, overflow needs to be addressed first, then underflow
    match = _mm512_cmpnle_pi(abs_data, (__m512i)exp_upper);
    if (!_mm512_kortestz(match, match)){
      norm    = (double8)exp_lower;
      denorm  = (double8)exp_upper;
    }

    data = _mm512_mul_pd(data, norm);
    data = _mm512_mul_pd(data, data);

    add_reduce1_pd(data);
    add_reduce2_pd(data);

    return sqrt(data.s0)*denorm.s0;
  }]>;

// normalize
OclBuiltinImpl normalize_f32_scalar = OclBuiltinImpl<normalize, [v1f32], 0, [{
    int ifnan = isnan($Arg0VarName);
    int nan = 0xFF800000;
    int result = (as_int($Arg0VarName) & 0x80000000) | 0x3f800000;
    result = (ifnan & nan) | (result & ~nan);
    return ($Arg0VarName==0)?0.0:as_float(result);
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
    __m512i sign_mask = _mm512_and_pi((__m512i)x_reg, (__m512i)((int16)const_vector_msb));
    __m512i one_vector = _mm512_or_pi((__m512i)one, sign_mask); // 1 *sign_bit(x)

    // data = isinf(x_reg)?one:data
    __m512i data = (__m512i)x_reg;
    data = _mm512_andn_pi(data, (__m512i)ifinf);
    one_vector = _mm512_and_pi(one_vector, (__m512i)ifinf);
    data = _mm512_or_pi(data, one_vector);

    // returun 16NAN if any of the element is NAN
    if (any(ifnan))
      return all_nan$Pattern;

    __mmask16 equal = _mm512_cmpeq_ps((__m512)x_reg, (__m512)((float16)0.0));
    int flag;

    // <Caveat> Use a more efficient 4-element length since normalize only support up to 4 elements
    float norm = length(x_reg$Pattern);

    // norm = all_zero?1.0:norm;
    flag = _mm512_kortestc(equal, equal);
    norm = flag?1.0:norm;

    float16 result = x_reg / norm;

    return result$Pattern;
  }]>;
OclBuiltinImpl normalize_f64_scalar = OclBuiltinImpl<normalize, [v1f64], 0, [{
    long ifnan = isnan($Arg0VarName);
    long nan = 0xFFF0000000000000L;
    long result = (as_long($Arg0VarName) & 0x8000000000000000L) | 0x3FF0000000000000L;
    result = (ifnan & nan) | (result & ~nan);
    return ($Arg0VarName==0)?0.0:as_double(result);
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
    __m512i sign_mask = _mm512_and_pq((__m512i)x_reg, const_vector_long_msb);
    __m512i one_vector = _mm512_or_pq((__m512i)one, sign_mask); // 1 *sign_bit(x)

    // data = isinf(x_reg)?one:data
    __m512i data = (__m512i)(x_reg);
    data = _mm512_andn_pq(data, (__m512i)ifinf);
    one_vector = _mm512_and_pq(one_vector, (__m512i)ifinf);
    data = _mm512_or_pq(data, one_vector);

    // returun 16NAN if any of the element is NAN
    if (any(ifnan))
      return all_nan$Pattern;

    __mmask8 equal = _mm512_cmpeq_pd(x_reg, (double8)0.0);
    int flag;

    // <Caveat> Use a more efficient 4-element length since normalize only support up to 4 elements
    double norm = length(x_reg$Pattern);

    // norm = all_zero?1.0:norm;
    flag = _mm512_kortestc(equal, equal);
    norm = flag?1.0:norm;

    double8 result = x_reg / norm;

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

    return ((float16)result)$Pattern;
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
    s201_s012_pd(y_reg, k);

    x_reg = _mm512_mul_pd(x_reg, y_reg);
    result = x_reg;

    f_result.s012 = $Arg0VarName;
    x_reg = f_result;
    s201_s012_pd(x_reg, k);

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

// distance
OclBuiltinImpl distance_scalar = OclBuiltinImpl<distance, [v1f32, v1f64], 0, [{
    return fabs($Arg0VarName - $Arg1VarName);
  }]>;
OclBuiltinImpl distance_vector = OclBuiltinImpl<distance, [v2f32, v3f32, v4f32, v2f64, v3f64, v4f64], 0, [{
    return length($Arg0VarName - $Arg1VarName);
  }]>;

// fast_length
OclBuiltinImpl fast_length_scalar = OclBuiltinImpl<fast_length, [v1f32], 0, [{
    return fabs($Arg0VarName);
  }]>;
OclBuiltinImpl fast_length_vector = OclBuiltinImpl<fast_length, [v2f32, v3f32, v4f32], 0, [{
    return half_sqrt(dot($Arg0VarName,$Arg0VarName));
  }]>;

// fast_distance
OclBuiltinImpl fast_distance_all = OclBuiltinImpl<fast_distance, [v1f32, v2f32, v3f32, v4f32], 0, [{
    return fast_length($Arg0VarName - $Arg1VarName);
  }]>;

// fast_normalize
OclBuiltinImpl fast_normalize_scalar = OclBuiltinImpl<fast_normalize, [v1f32], 0, [{
    return ($Arg0VarName < sqrt(FLT_MIN))?$Arg0VarName:($Arg0VarName/fabs($Arg0VarName));
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
    temp = (__m512)_mm512_mask_and_pi((__m512i)temp, small_val, (__m512i)zeros, (__m512i)zeros);
    temp = _mm512_mul_ps(temp, temp);
    add_reduce1_ps(temp);
    add_reduce2_ps(temp);
    sum = ((float16)temp).s0;

    // Below include checking if all(source == 0) and sum-of-squae is < FLT_MIN
    sum = (sum < FLT_MIN)?1.0:sum;

    sum = half_rsqrt(sum);
    temp = (__m512)x_reg;
    temp = _mm512_mul_ps(temp, (__m512)((float16)sum));
  
    return ((float16)temp)$Pattern;
  }]>;

