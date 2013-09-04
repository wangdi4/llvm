// Copyright (c) 2006-2012 Intel Corporation
// Geometric Built-In Functions
// SSSE3

/*****************************************************************\
 * dot:                                                          *
 *   float dot (floatn p0, floatn p1)                            *
 *   double dot (doublen p0, doublen p1)                         *
 *   Compute dot product.                                        *
\*****************************************************************/

OclBuiltinImpl dot_scalar = OclBuiltinImpl<dot, [v1f32, v1f64], 0, [{
    return $Arg0VarName * $Arg1VarName;
  }]>;

OclBuiltinImpl dot_v2f32 = OclBuiltinImpl<dot, [v2f32], 0, [{
    float4 n$Arg0VarName = (float4)0;
    n$Arg0VarName.lo = $Arg0VarName;
    float4 n$Arg1VarName = (float4)0;
    n$Arg1VarName.lo = $Arg1VarName;
    n$Arg0VarName = n$Arg0VarName * n$Arg1VarName;
    n$Arg0VarName = _mm_hadd_ps(n$Arg0VarName, n$Arg0VarName);
    return n$Arg0VarName.s0;
  }]>;

OclBuiltinImpl dot_promote = OclBuiltinImpl<dot, [v3f32], 0, Promote2_scalar>;

OclBuiltinImpl dot_v4f32 = OclBuiltinImpl<dot, [v4f32], 0, [{
    return dot($Arg0VarName.lo, $Arg1VarName.lo) + dot($Arg0VarName.hi, $Arg1VarName.hi);
  }]>;

OclBuiltinImpl dot_v2f64 = OclBuiltinImpl<dot, [v2f64], 0, [{
    return $Arg0VarName.lo * $Arg1VarName.lo + $Arg0VarName.hi * $Arg1VarName.hi;
  }]>;

OclBuiltinImpl dot_v3f64 = OclBuiltinImpl<dot, [v3f64], 0, [{
    return dot($Arg0VarName.xy, $Arg1VarName.xy) + dot($Arg0VarName.z, $Arg1VarName.z);
  }]>;
  
OclBuiltinImpl dot_v4f64 = OclBuiltinImpl<dot, [v4f64], 0, [{
    double2 resLo = $Arg0VarName.lo * $Arg1VarName.lo;
    double2 resHi = $Arg0VarName.hi * $Arg1VarName.hi;
    resLo = resLo + resHi;
    resLo = _mm_hadd_pd(resLo, resLo);
    return resLo.s0;
  }]>;


/*****************************************************************\
 * cross:                                                        *
 *   float4 cross (float4 p0, float4 p1)                         *
 *   float3 cross (float3 p0, float3 p1)                         *
 *   double4 cross (double4 p0, double4 p1)                      *
 *   double3 cross (double3 p0, double3 p1)                      *
 *   Returns the cross product of p0.xyz and p1.xyz. The         *
 *   w component of float4 result returned will be 0.0.          *
\*****************************************************************/

OclBuiltinImpl cross_v3f32 = OclBuiltinImpl<cross, [v3f32], 0, [{
    float4  tempX = (float4)(0);
    float4  tempY = (float4)(0);
    float4  res;
    tempX.s012 = $Arg0VarName;
    tempY.s012 = $Arg1VarName;
    res = cross(tempX, tempY);
    return res.s012;
  }]>;

OclBuiltinImpl cross_v4f32 = OclBuiltinImpl<cross, [v4f32], 0, [{
    double4 t0, t1;
    double4 res = (double4)0;

    t0 = convert_double4($Arg0VarName);
    t1 = convert_double4($Arg1VarName);

    double2 p0Lo = _mm_shuffle_pd(t0.lo, t0.hi, 0b01);
    double2 p0Hi = _mm_shuffle_pd(t0.lo, t0.hi, 0b10);
    double2 p1Lo = _mm_shuffle_pd(t1.lo, t1.hi, 0b01);
    double2 p1Hi = _mm_shuffle_pd(t1.lo, t1.hi, 0b10);

    t0.lo = t0.lo * p1Lo;
    t0.hi = t0.hi * p1Hi;
    t1.lo = t1.lo * p0Lo;
    t1.hi = t1.hi * p0Hi;

    t0.lo = t0.lo - t1.lo;
    t0.hi = t0.hi - t1.hi;

    res.lo = _mm_shuffle_pd(t0.lo, t0.hi, 0b01);
    res.s2 = t0.s0;

    return convert_float4(res);
  }]>;

OclBuiltinImpl cross_v3f64 = OclBuiltinImpl<cross, [v3f64], 0, [{
    double4 tempX = (double4)(0);
    double4 tempY = (double4)(0);
    double4 res;
    tempX.s012 = $Arg0VarName;
    tempY.s012 = $Arg1VarName;
    res = cross(tempX, tempY);
    return res.s012;
  }]>;

OclBuiltinImpl cross_v4f64 = OclBuiltinImpl<cross, [v4f64], 0, [{
    double4 res = (double4)0;

    double2 p0Lo = _mm_shuffle_pd($Arg0VarName.lo, $Arg0VarName.hi, 0b01);
    double2 p0Hi = _mm_shuffle_pd($Arg0VarName.lo, $Arg0VarName.hi, 0b10);
    double2 p1Lo = _mm_shuffle_pd($Arg1VarName.lo, $Arg1VarName.hi, 0b01);
    double2 p1Hi = _mm_shuffle_pd($Arg1VarName.lo, $Arg1VarName.hi, 0b10);

    $Arg0VarName.lo = $Arg0VarName.lo * p1Lo;
    $Arg0VarName.hi = $Arg0VarName.hi * p1Hi;
    $Arg1VarName.lo = $Arg1VarName.lo * p0Lo;
    $Arg1VarName.hi = $Arg1VarName.hi * p0Hi;

    $Arg0VarName.lo = $Arg0VarName.lo - $Arg1VarName.lo;
    $Arg0VarName.hi = $Arg0VarName.hi - $Arg1VarName.hi;

    res.lo = _mm_shuffle_pd($Arg0VarName.lo, $Arg0VarName.hi, 0b01);
    res.s2 = $Arg0VarName.s0;

    return res;
  }]>;


/*****************************************************************\
 * fast_normalize:                                               *
 *   float fast_normalize (floatn p0)                            *
 *   Returns p * half_rsqrt (p.x2 + p.y2 + ... )                 *
\*****************************************************************/
OclBuiltinImpl fast_normalize_scalar = OclBuiltinImpl<fast_normalize, [v1f32], 0, [{
   if( $Arg0VarName == 0.f )
    {
      // Special edge case: copy vector over without change
      return $Arg0VarName;
    }
    return ($Arg0VarName > 0.f) ? 1.f : -1.f;
  }]>;

OclBuiltinImpl fast_normalize_v2v3f32 = OclBuiltinImpl<fast_normalize, [v2f32, v3f32], 0, Promote1>;

OclBuiltinImpl fast_normalize_v4f32 = OclBuiltinImpl<fast_normalize, [v4f32], 0, [{
    // Calculate vector length
    float4 tmp;
    tmp.s0 = dot($Arg0VarName, $Arg0VarName);
    if( tmp.s0 == 0.f )
    {
      // Special edge case: copy vector over without change
      return $Arg0VarName;
    }

    tmp = as_float4(_mm_shuffle_ps((__m128)tmp, (__m128)tmp, 0x0));
    tmp = half_rsqrt(tmp);
    $Arg0VarName = $Arg0VarName * tmp;

    return $Arg0VarName;
  }]>;

/*****************************************************************\
 * Extend SOA Geometry built-in functions                        *
\*****************************************************************/
/*****************************************************************\
 * soa_dot:                                                      *
\*****************************************************************/
// soa_dot1
OclBuiltinImpl soa_dot1_all = OclBuiltinImpl<soa_dot1, fp_soa_types, 0, [{
    return ($Arg0VarName * $Arg1VarName);
  }]>;
// soa_dot2
OclBuiltinImpl soa_dot2_all = OclBuiltinImpl<soa_dot2, fp_soa_types, 0, [{
    return ($Arg0VarName * $Arg2VarName) + ($Arg1VarName * $Arg3VarName);
  }]>;
// soa_dot3
OclBuiltinImpl soa_dot3_all = OclBuiltinImpl<soa_dot3, fp_soa_types, 0, [{
    return ($Arg0VarName * $Arg3VarName) + ($Arg1VarName * $Arg4VarName) + ($Arg2VarName * $Arg5VarName);
  }]>;
// soa_dot4
OclBuiltinImpl soa_dot4_all = OclBuiltinImpl<soa_dot4, fp_soa_types, 0, [{
    return ($Arg0VarName * $Arg4VarName) + ($Arg1VarName * $Arg5VarName) + ($Arg2VarName * $Arg6VarName) + ($Arg3VarName * $Arg7VarName);
  }]>;


/*****************************************************************\
 * soa_cross:                                                    *
\*****************************************************************/
// soa_cross3
OclBuiltinImpl soa_cross3_all = OclBuiltinImpl<soa_cross3, fp_soa_types, 0, [{
    *$Arg6VarName = ($Arg1VarName * $Arg5VarName) - ($Arg2VarName * $Arg4VarName);
    *$Arg7VarName = ($Arg2VarName * $Arg3VarName) - ($Arg0VarName * $Arg5VarName);
    *$Arg8VarName = ($Arg0VarName * $Arg4VarName) - ($Arg1VarName * $Arg3VarName);
  }]>;
// soa_cross4
OclBuiltinImpl soa_cross4_all = OclBuiltinImpl<soa_cross4, fp_soa_types, 0, [{
    soa_cross3($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg8VarName, $Arg9VarName, $Arg10VarName);
    *$Arg11VarName = ($Arg0Type)0.0;
  }]>;


/*****************************************************************\
 * soa_length:                                                   *
\*****************************************************************/
// soa_length1
OclBuiltinImpl soa_length1_all = OclBuiltinImpl<soa_length1 , fp_soa_types, 0, [{
    return fabs($Arg0VarName);
  }]>;
// soa_length2_f32
OclBuiltinImpl soa_length2_f32 = OclBuiltinImpl<soa_length2 , sp_fp_soa_types, 0, [{
    $Arg0VecType sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName;
    int$VecLength overflow_mask = sum == INFINITY;
    int$VecLength underflow_mask = sum < (2 * FLT_MIN / FLT_EPSILON);
    if( intel_movemask(overflow_mask) || intel_movemask(underflow_mask) )
    {
      double$VecLength dp_p0_x = convert_double$VecLength($Arg0VarName);
      double$VecLength dp_p0_y = convert_double$VecLength($Arg1VarName);
      double$VecLength dp_sum = (dp_p0_x * dp_p0_x) + (dp_p0_y * dp_p0_y);
      dp_sum = sqrt( dp_sum );
      return convert_float$VecLength( dp_sum );
    }
    return sqrt( sum );
  }]>;
// soa_length2_f64
OclBuiltinImpl soa_length2_f64 = OclBuiltinImpl<soa_length2 , dp_fp_soa_types, 0, [{
    $Arg0VecType sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName;
    $Arg0VecType sum_multiplier = 1.0;
    long$VecLength overflow_mask = (sum == INFINITY);
    long$VecLength underflow_mask = (sum  < (2 * DBL_MIN / DBL_EPSILON));
    if ( intel_movemask(overflow_mask)  )
    {
      $Arg0VecType overflow_p0_x = $Arg0VarName * expMinus600;
      $Arg0VecType overflow_p0_y = $Arg1VarName * expMinus600;
      $Arg0VecType overflow_sum = overflow_p0_x * overflow_p0_x + overflow_p0_y * overflow_p0_y;
      sum_multiplier = select( sum_multiplier, exp600, overflow_mask );
      sum = select( sum, overflow_sum, overflow_mask);
    }
    if ( intel_movemask(underflow_mask) )
    {
      $Arg0VecType underflow_p0_x = $Arg0VarName * exp700;
      $Arg0VecType underflow_p0_y = $Arg1VarName * exp700;
      $Arg0VecType underflow_sum = underflow_p0_x * underflow_p0_x + underflow_p0_y * underflow_p0_y;
      sum_multiplier = select( sum_multiplier, expMinus700, underflow_mask );
      sum = select( sum, underflow_sum, underflow_mask);
    }
    return sqrt( sum ) * sum_multiplier;
  }]>;
// soa_length3_f32
OclBuiltinImpl soa_length3_f32 = OclBuiltinImpl<soa_length3 , sp_fp_soa_types, 0, [{
    $Arg0VecType sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName;
    int$VecLength overflow_mask = sum == INFINITY;
    int$VecLength underflow_mask = sum < (2 * FLT_MIN / FLT_EPSILON);
    if( intel_movemask(overflow_mask) || intel_movemask(underflow_mask) )
    {
      double$VecLength dp_p0_x = convert_double$VecLength($Arg0VarName);
      double$VecLength dp_p0_y = convert_double$VecLength($Arg1VarName);
      double$VecLength dp_p0_z = convert_double$VecLength($Arg2VarName);
      double$VecLength dp_sum = (dp_p0_x * dp_p0_x) + (dp_p0_y * dp_p0_y) + (dp_p0_z * dp_p0_z);
      dp_sum = sqrt( dp_sum );
      return convert_float$VecLength( dp_sum );
    }
    return sqrt( sum );
  }]>;
// soa_length3_f64
OclBuiltinImpl soa_length3_f64 = OclBuiltinImpl<soa_length3 , dp_fp_soa_types, 0, [{
    $Arg0VecType sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName;
    $Arg0VecType sum_multiplier = 1.0;
    long$VecLength overflow_mask = (sum == INFINITY);
    long$VecLength underflow_mask = (sum  < (2 * DBL_MIN / DBL_EPSILON));
    if ( intel_movemask(overflow_mask)  )
    {
      $Arg0VecType overflow_p0_x = $Arg0VarName * expMinus600;
      $Arg0VecType overflow_p0_y = $Arg1VarName * expMinus600;
      $Arg0VecType overflow_p0_z = $Arg2VarName * expMinus600;
      $Arg0VecType overflow_sum = overflow_p0_x * overflow_p0_x + overflow_p0_y * overflow_p0_y + overflow_p0_z * overflow_p0_z;
      sum_multiplier = select( sum_multiplier, exp600, overflow_mask );
      sum = select( sum, overflow_sum, overflow_mask);
    }
    if ( intel_movemask(underflow_mask) )
    {
      $Arg0VecType underflow_p0_x = $Arg0VarName * exp700;
      $Arg0VecType underflow_p0_y = $Arg1VarName * exp700;
      $Arg0VecType underflow_p0_z = $Arg2VarName * exp700;
      $Arg0VecType underflow_sum = underflow_p0_x * underflow_p0_x + underflow_p0_y * underflow_p0_y + underflow_p0_z * underflow_p0_z;
      sum_multiplier = select( sum_multiplier, expMinus700, underflow_mask );
      sum = select( sum, underflow_sum, underflow_mask);
    }
    return sqrt( sum ) * sum_multiplier;
  }]>;
// soa_length4_f32
OclBuiltinImpl soa_length4_f32 = OclBuiltinImpl<soa_length4 , sp_fp_soa_types, 0, [{
    $Arg0VecType sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName + $Arg3VarName * $Arg3VarName;
    int$VecLength overflow_mask = sum == INFINITY;
    int$VecLength underflow_mask = sum < (2 * FLT_MIN / FLT_EPSILON);
    if( intel_movemask(overflow_mask) || intel_movemask(underflow_mask) )
    {
      double$VecLength dp_p0_x = convert_double$VecLength($Arg0VarName);
      double$VecLength dp_p0_y = convert_double$VecLength($Arg1VarName);
      double$VecLength dp_p0_z = convert_double$VecLength($Arg2VarName);
      double$VecLength dp_p0_w = convert_double$VecLength($Arg3VarName);
      double$VecLength dp_sum = (dp_p0_x * dp_p0_x) + (dp_p0_y * dp_p0_y) + (dp_p0_z * dp_p0_z) + (dp_p0_w * dp_p0_w);
      dp_sum = sqrt( dp_sum );
      return convert_float$VecLength( dp_sum );
    }
    return sqrt( sum );
  }]>;
// soa_length4_f64
OclBuiltinImpl soa_length4_f64 = OclBuiltinImpl<soa_length4 , dp_fp_soa_types, 0, [{
    $Arg0VecType sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName + $Arg3VarName * $Arg3VarName;
    $Arg0VecType sum_multiplier = 1.0;
    long$VecLength overflow_mask = (sum == INFINITY);
    long$VecLength underflow_mask = (sum  < (2 * DBL_MIN / DBL_EPSILON));
    if ( intel_movemask(overflow_mask)  )
    {
      $Arg0VecType overflow_p0_x = $Arg0VarName * expMinus600;
      $Arg0VecType overflow_p0_y = $Arg1VarName * expMinus600;
      $Arg0VecType overflow_p0_z = $Arg2VarName * expMinus600;
      $Arg0VecType overflow_p0_w = $Arg3VarName * expMinus600;
      $Arg0VecType overflow_sum = overflow_p0_x * overflow_p0_x + overflow_p0_y * overflow_p0_y + overflow_p0_z * overflow_p0_z + overflow_p0_w * overflow_p0_w;
      sum_multiplier = select( sum_multiplier, exp600, overflow_mask );
      sum = select( sum, overflow_sum, overflow_mask);
    }
    if ( intel_movemask(underflow_mask) )
    {
      $Arg0VecType underflow_p0_x = $Arg0VarName * exp700;
      $Arg0VecType underflow_p0_y = $Arg1VarName * exp700;
      $Arg0VecType underflow_p0_z = $Arg2VarName * exp700;
      $Arg0VecType underflow_p0_w = $Arg3VarName * exp700;
      $Arg0VecType underflow_sum = underflow_p0_x * underflow_p0_x + underflow_p0_y * underflow_p0_y + underflow_p0_z * underflow_p0_z + underflow_p0_w * underflow_p0_w;
      sum_multiplier = select( sum_multiplier, expMinus700, underflow_mask );
      sum = select( sum, underflow_sum, underflow_mask);
    }
    return sqrt( sum ) * sum_multiplier;
  }]>;


/*****************************************************************\
 * soa_fast_length:                                              *
\*****************************************************************/
// soa_fast_length1
OclBuiltinImpl soa_fast_length1_f32 = OclBuiltinImpl<soa_fast_length1 , sp_fp_soa_types, 0, [{
    return fabs($Arg0VarName);
  }]>;
// soa_fast_length2
OclBuiltinImpl soa_fast_length2_f32 = OclBuiltinImpl<soa_fast_length2 , sp_fp_soa_types, 0, [{
    $Arg0VecType sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName;
    return sqrt( sum );
  }]>;
// soa_fast_length3
OclBuiltinImpl soa_fast_length3_f32 = OclBuiltinImpl<soa_fast_length3 , sp_fp_soa_types, 0, [{
    $Arg0VecType sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName;
    return sqrt( sum );
  }]>;
// soa_fast_length4
OclBuiltinImpl soa_fast_length4_f32 = OclBuiltinImpl<soa_fast_length4 , sp_fp_soa_types, 0, [{
    $Arg0VecType sum = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName + $Arg3VarName * $Arg3VarName;
    return sqrt( sum );
  }]>;


/*****************************************************************\
 * soa_normalize:                                                *
\*****************************************************************/
// soa_normalize1_f32
OclBuiltinImpl soa_normalize1_f32 = OclBuiltinImpl<soa_normalize1 , sp_fp_soa_types, 0, [{
    int$VecLength zero_mask = ($Arg0VarName == 0.f);
    int$VecLength NAN_mask = ($Arg0VarName != $Arg0VarName);
    int$VecLength gtz_mask = ($Arg0VarName > 0.f);
    $Arg0VecType res = select(($Arg0VecType)-1.f, ($Arg0VecType)1.f, gtz_mask);
    res = select(res, $Arg0VarName, NAN_mask);
    res = select(res, $Arg0VarName, zero_mask);
    return res;
  }]>;
// soa_normalize1_f64
OclBuiltinImpl soa_normalize1_f64 = OclBuiltinImpl<soa_normalize1 , dp_fp_soa_types, 0, [{
    long$VecLength zero_mask = ($Arg0VarName == 0.f);
    long$VecLength NAN_mask = ($Arg0VarName != $Arg0VarName);
    long$VecLength gtz_mask = ($Arg0VarName > 0.f);
    $Arg0VecType res = select(($Arg0VecType)-1.f, ($Arg0VecType)1.f, gtz_mask);
    res = select(res, $Arg0VarName, NAN_mask);
    res = select(res, $Arg0VarName, zero_mask);
    return res;
  }]>;
// soa_normalize2_f32
OclBuiltinImpl soa_normalize2_f32 = OclBuiltinImpl<soa_normalize2 , sp_fp_soa_types, 0, [{
    double$VecLength p0_x_d = convert_double$VecLength($Arg0VarName);
    double$VecLength p0_y_d = convert_double$VecLength($Arg1VarName);
    double$VecLength length = p0_x_d * p0_x_d + p0_y_d * p0_y_d;
    double$VecLength length_inf;
    long$VecLength zero_mask = (length == 0.f);
    long$VecLength inf_mask = (length == INFINITY);

    if( intel_movemask( inf_mask ) )
    {
      float$VecLength temp_x = (fabs($Arg0VarName) == INFINITY) ? copysign( (float$VecLength)1.0f, $Arg0VarName ) : copysign( (float$VecLength)0.0f, $Arg0VarName );
      float$VecLength temp_y = (fabs($Arg1VarName) == INFINITY) ? copysign( (float$VecLength)1.0f, $Arg1VarName ) : copysign( (float$VecLength)0.0f, $Arg1VarName );
      double$VecLength temp_x_d = convert_double$VecLength(temp_x);
      double$VecLength temp_y_d = convert_double$VecLength(temp_y);
      length_inf = temp_x_d * temp_x_d + temp_y_d * temp_y_d;
      length_inf = sqrt(length_inf);
    }

    length = sqrt(length);
    length = select( length, length_inf, inf_mask);
    double$VecLength norm_x = p0_x_d / length;
    double$VecLength norm_y = p0_y_d / length;
    *$Arg2VarName = convert_float$VecLength( norm_x );
    *$Arg3VarName = convert_float$VecLength( norm_y );
    *$Arg2VarName = convert_float$VecLength( select( norm_x, p0_x_d, zero_mask) );
    *$Arg3VarName = convert_float$VecLength( select( norm_y, p0_y_d, zero_mask) );
  }]>;
// soa_normalize2_f64
OclBuiltinImpl soa_normalize2_f64 = OclBuiltinImpl<soa_normalize2 , dp_fp_soa_types, 0, [{
    double$VecLength total = 0.0, value;
    total = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName;
    long$VecLength underflow_mask = (total < (2 * DBL_MIN / DBL_EPSILON));
    long$VecLength underflow_zero_mask = 0;
    long$VecLength overflow_mask = (total == INFINITY);

    if( intel_movemask(underflow_mask) )
    {
      double$VecLength underflow_total = 0.0;
      double$VecLength t_x, t_y;
      t_x = $Arg0VarName * exp700;
      t_y = $Arg1VarName * exp700;
      underflow_total = t_x * t_x + t_y * t_y;
      underflow_zero_mask = (underflow_total == 0.0);
      underflow_zero_mask = underflow_zero_mask;
      $Arg0VarName = select($Arg0VarName, t_x, underflow_mask);
      $Arg1VarName = select($Arg1VarName, t_y, underflow_mask);
      total = select( total, underflow_total, underflow_mask);
    }

    if( intel_movemask(overflow_mask) )
    {
      double$VecLength scale = expMinus512_2;
      double$VecLength overflow_total = 0;
      double$VecLength t_x, t_y;
      t_x = $Arg0VarName * scale;
      t_y = $Arg1VarName * scale;
      overflow_total = t_x * t_x + t_y * t_y;
      long$VecLength still_overflow_mask = (overflow_total == INFINITY);

      if( intel_movemask(still_overflow_mask) )
      {
        double$VecLength stil_overflow_total = 0.0;
        double$VecLength t_of_x, t_of_y;

        long$VecLength is_inf_mask = isinf( t_x );
        t_of_x = select( copysign(0.0, $Arg0VarName), copysign(1.0, $Arg0VarName), is_inf_mask);
        stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask);

        is_inf_mask = isinf( t_y );
        t_of_y = select( copysign(0.0, $Arg1VarName), copysign(1.0, $Arg1VarName), is_inf_mask);
        stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask);

        t_x = select(t_x, t_of_x, still_overflow_mask);
        t_y = select(t_y, t_of_y, still_overflow_mask);
        overflow_total = select(overflow_total, stil_overflow_total, still_overflow_mask);
      }

      $Arg0VarName = select($Arg0VarName, t_x, overflow_mask);
      $Arg1VarName = select($Arg1VarName, t_y, overflow_mask);
      total = select(total, overflow_total, overflow_mask);
    }

    value = sqrt( total );
    *$Arg2VarName = select($Arg0VarName / value, $Arg0VarName, underflow_zero_mask);
    *$Arg3VarName = select($Arg1VarName / value, $Arg1VarName, underflow_zero_mask);
  }]>;
// soa_normalize3_f32
OclBuiltinImpl soa_normalize3_f32 = OclBuiltinImpl<soa_normalize3 , sp_fp_soa_types, 0, [{
    double$VecLength p0_x_d = convert_double$VecLength($Arg0VarName);
    double$VecLength p0_y_d = convert_double$VecLength($Arg1VarName);
    double$VecLength p0_z_d = convert_double$VecLength($Arg2VarName);
    double$VecLength length = p0_x_d * p0_x_d + p0_y_d * p0_y_d + p0_z_d * p0_z_d;
    double$VecLength length_inf;
    long$VecLength zero_mask = (length == 0.f);
    long$VecLength inf_mask = (length == INFINITY);

    if( intel_movemask( inf_mask ) )
    {
      float$VecLength temp_x = (fabs($Arg0VarName) == INFINITY) ? copysign( (float$VecLength)1.0f, $Arg0VarName ) : copysign( (float$VecLength)0.0f, $Arg0VarName );
      float$VecLength temp_y = (fabs($Arg1VarName) == INFINITY) ? copysign( (float$VecLength)1.0f, $Arg1VarName ) : copysign( (float$VecLength)0.0f, $Arg1VarName );
      float$VecLength temp_z = (fabs($Arg2VarName) == INFINITY) ? copysign( (float$VecLength)1.0f, $Arg2VarName ) : copysign( (float$VecLength)0.0f, $Arg2VarName );
      double$VecLength temp_x_d = convert_double$VecLength(temp_x);
      double$VecLength temp_y_d = convert_double$VecLength(temp_y);
      double$VecLength temp_z_d = convert_double$VecLength(temp_z);
      length_inf = temp_x_d * temp_x_d + temp_y_d * temp_y_d + temp_z_d * temp_z_d;
      length_inf = sqrt(length_inf);
    }

    length = sqrt(length);
    length = select( length, length_inf, inf_mask);
    double$VecLength norm_x = p0_x_d / length;
    double$VecLength norm_y = p0_y_d / length;
    double$VecLength norm_z = p0_z_d / length;
    *$Arg3VarName = convert_float$VecLength( norm_x );
    *$Arg4VarName = convert_float$VecLength( norm_y );
    *$Arg5VarName = convert_float$VecLength( norm_z );
    *$Arg3VarName = convert_float$VecLength( select( norm_x, p0_x_d, zero_mask) );
    *$Arg4VarName = convert_float$VecLength( select( norm_y, p0_y_d, zero_mask) );
    *$Arg5VarName = convert_float$VecLength( select( norm_z, p0_z_d, zero_mask) );
  }]>;
// soa_normalize3_f64
OclBuiltinImpl soa_normalize3_f64 = OclBuiltinImpl<soa_normalize3 , dp_fp_soa_types, 0, [{
    double$VecLength total = 0.0, value;
    total = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName;
    long$VecLength underflow_mask = (total < (2 * DBL_MIN / DBL_EPSILON));
    long$VecLength underflow_zero_mask = 0;
    long$VecLength overflow_mask = (total == INFINITY);

    if( intel_movemask(underflow_mask) )
    {
      double$VecLength underflow_total = 0.0;
      double$VecLength t_x, t_y, t_z;
      t_x = $Arg0VarName * exp700;
      t_y = $Arg1VarName * exp700;
      t_z = $Arg2VarName * exp700;
      underflow_total = t_x * t_x + t_y * t_y + t_z * t_z;
      underflow_zero_mask = (underflow_total == 0.0);
      underflow_zero_mask = underflow_zero_mask;
      $Arg0VarName = select($Arg0VarName, t_x, underflow_mask);
      $Arg1VarName = select($Arg1VarName, t_y, underflow_mask);
      $Arg2VarName = select($Arg2VarName, t_z, underflow_mask);
      total = select( total, underflow_total, underflow_mask);
    }

    if( intel_movemask(overflow_mask) )
    {
      double$VecLength scale = expMinus512_2;
      double$VecLength overflow_total = 0;
      double$VecLength t_x, t_y, t_z;
      t_x = $Arg0VarName * scale;
      t_y = $Arg1VarName * scale;
      t_z = $Arg2VarName * scale;
      overflow_total = t_x * t_x + t_y * t_y + t_z * t_z;
      long$VecLength still_overflow_mask = (overflow_total == INFINITY);

      if( intel_movemask(still_overflow_mask) )
      {
        double$VecLength stil_overflow_total = 0.0;
        double$VecLength t_of_x, t_of_y, t_of_z;

        long$VecLength is_inf_mask = isinf( t_x );
        t_of_x = select( copysign(0.0, $Arg0VarName), copysign(1.0, $Arg0VarName), is_inf_mask);
        stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask);

        is_inf_mask = isinf( t_y );
        t_of_y = select( copysign(0.0, $Arg1VarName), copysign(1.0, $Arg1VarName), is_inf_mask);
        stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask);

        is_inf_mask = isinf( t_z );
        t_of_z = select( copysign(0.0, $Arg2VarName), copysign(1.0, $Arg2VarName), is_inf_mask);
        stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask);

        t_x = select(t_x, t_of_x, still_overflow_mask);
        t_y = select(t_y, t_of_y, still_overflow_mask);
        t_z = select(t_z, t_of_z, still_overflow_mask);
        overflow_total = select(overflow_total, stil_overflow_total, still_overflow_mask);
      }

      $Arg0VarName = select($Arg0VarName, t_x, overflow_mask);
      $Arg1VarName = select($Arg1VarName, t_y, overflow_mask);
      $Arg2VarName = select($Arg2VarName, t_z, overflow_mask);
      total = select(total, overflow_total, overflow_mask);
    }

    value = sqrt( total );
    *$Arg3VarName = select($Arg0VarName / value, $Arg0VarName, underflow_zero_mask);
    *$Arg4VarName = select($Arg1VarName / value, $Arg1VarName, underflow_zero_mask);
    *$Arg5VarName = select($Arg2VarName / value, $Arg2VarName, underflow_zero_mask);
  }]>;
// soa_normalize4_f32
OclBuiltinImpl soa_normalize4_f32 = OclBuiltinImpl<soa_normalize4 , sp_fp_soa_types, 0, [{
    double$VecLength p0_x_d = convert_double$VecLength($Arg0VarName);
    double$VecLength p0_y_d = convert_double$VecLength($Arg1VarName);
    double$VecLength p0_z_d = convert_double$VecLength($Arg2VarName);
    double$VecLength p0_w_d = convert_double$VecLength($Arg3VarName);
    double$VecLength length = p0_x_d * p0_x_d + p0_y_d * p0_y_d + p0_z_d * p0_z_d + p0_w_d * p0_w_d;
    double$VecLength length_inf;
    long$VecLength zero_mask = (length == 0.f);
    long$VecLength inf_mask = (length == INFINITY);

    if( intel_movemask( inf_mask ) )
    {
      float$VecLength temp_x = (fabs($Arg0VarName) == INFINITY) ? copysign( (float$VecLength)1.0f, $Arg0VarName ) : copysign( (float$VecLength)0.0f, $Arg0VarName );
      float$VecLength temp_y = (fabs($Arg1VarName) == INFINITY) ? copysign( (float$VecLength)1.0f, $Arg1VarName ) : copysign( (float$VecLength)0.0f, $Arg1VarName );
      float$VecLength temp_z = (fabs($Arg2VarName) == INFINITY) ? copysign( (float$VecLength)1.0f, $Arg2VarName ) : copysign( (float$VecLength)0.0f, $Arg2VarName );
      float$VecLength temp_w = (fabs($Arg3VarName) == INFINITY) ? copysign( (float$VecLength)1.0f, $Arg3VarName ) : copysign( (float$VecLength)0.0f, $Arg3VarName );
      double$VecLength temp_x_d = convert_double$VecLength(temp_x);
      double$VecLength temp_y_d = convert_double$VecLength(temp_y);
      double$VecLength temp_z_d = convert_double$VecLength(temp_z);
      double$VecLength temp_w_d = convert_double$VecLength(temp_w);
      length_inf = temp_x_d * temp_x_d + temp_y_d * temp_y_d + temp_z_d * temp_z_d + temp_w_d * temp_w_d;
      length_inf = sqrt(length_inf);
    }

    length = sqrt(length);
    length = select( length, length_inf, inf_mask);
    double$VecLength norm_x = p0_x_d / length;
    double$VecLength norm_y = p0_y_d / length;
    double$VecLength norm_z = p0_z_d / length;
    double$VecLength norm_w = p0_w_d / length;
    *$Arg4VarName = convert_float$VecLength( norm_x );
    *$Arg5VarName = convert_float$VecLength( norm_y );
    *$Arg6VarName = convert_float$VecLength( norm_z );
    *$Arg7VarName = convert_float$VecLength( norm_w );
    *$Arg4VarName = convert_float$VecLength( select( norm_x, p0_x_d, zero_mask) );
    *$Arg5VarName = convert_float$VecLength( select( norm_y, p0_y_d, zero_mask) );
    *$Arg6VarName = convert_float$VecLength( select( norm_z, p0_z_d, zero_mask) );
    *$Arg7VarName = convert_float$VecLength( select( norm_w, p0_w_d, zero_mask) );
  }]>;
// soa_normalize4_f64
OclBuiltinImpl soa_normalize4_f64 = OclBuiltinImpl<soa_normalize4 , dp_fp_soa_types, 0, [{
    double$VecLength total = 0.0, value;
    total = $Arg0VarName * $Arg0VarName + $Arg1VarName * $Arg1VarName + $Arg2VarName * $Arg2VarName + $Arg3VarName * $Arg3VarName;
    long$VecLength underflow_mask = (total < (2 * DBL_MIN / DBL_EPSILON));
    long$VecLength underflow_zero_mask = 0;
    long$VecLength overflow_mask = (total == INFINITY);

    if( intel_movemask(underflow_mask) )
    {
      double$VecLength underflow_total = 0.0;
      double$VecLength t_x, t_y, t_z, t_w;
      t_x = $Arg0VarName * exp700;
      t_y = $Arg1VarName * exp700;
      t_z = $Arg2VarName * exp700;
      t_w = $Arg3VarName * exp700;
      underflow_total = t_x * t_x + t_y * t_y + t_z * t_z + t_w * t_w;
      underflow_zero_mask = (underflow_total == 0.0);
      underflow_zero_mask = underflow_zero_mask;
      $Arg0VarName = select($Arg0VarName, t_x, underflow_mask);
      $Arg1VarName = select($Arg1VarName, t_y, underflow_mask);
      $Arg2VarName = select($Arg2VarName, t_z, underflow_mask);
      $Arg3VarName = select($Arg3VarName, t_w, underflow_mask);
      total = select( total, underflow_total, underflow_mask);
    }

    if( intel_movemask(overflow_mask) )
    {
      double$VecLength scale = expMinus512_2;
      double$VecLength overflow_total = 0;
      double$VecLength t_x, t_y, t_z, t_w;
      t_x = $Arg0VarName * scale;
      t_y = $Arg1VarName * scale;
      t_z = $Arg2VarName * scale;
      t_w = $Arg3VarName * scale;
      overflow_total = t_x * t_x + t_y * t_y + t_z * t_z + t_w * t_w;
      long$VecLength still_overflow_mask = (overflow_total == INFINITY);

      if( intel_movemask(still_overflow_mask) )
      {
        double$VecLength stil_overflow_total = 0.0;
        double$VecLength t_of_x, t_of_y, t_of_z, t_of_w;

        long$VecLength is_inf_mask = isinf( t_x );
        t_of_x = select( copysign(0.0, $Arg0VarName), copysign(1.0, $Arg0VarName), is_inf_mask);
        stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask);

        is_inf_mask = isinf( t_y );
        t_of_y = select( copysign(0.0, $Arg1VarName), copysign(1.0, $Arg1VarName), is_inf_mask);
        stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask);

        is_inf_mask = isinf( t_z );
        t_of_z = select( copysign(0.0, $Arg2VarName), copysign(1.0, $Arg2VarName), is_inf_mask);
        stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask);

        is_inf_mask = isinf( t_w );
        t_of_w = select( copysign(0.0, $Arg3VarName), copysign(1.0, $Arg3VarName), is_inf_mask);
        stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask);

        t_x = select(t_x, t_of_x, still_overflow_mask);
        t_y = select(t_y, t_of_y, still_overflow_mask);
        t_z = select(t_z, t_of_z, still_overflow_mask);
        t_w = select(t_w, t_of_w, still_overflow_mask);
        overflow_total = select(overflow_total, stil_overflow_total, still_overflow_mask);
      }

      $Arg0VarName = select($Arg0VarName, t_x, overflow_mask);
      $Arg1VarName = select($Arg1VarName, t_y, overflow_mask);
      $Arg2VarName = select($Arg2VarName, t_z, overflow_mask);
      $Arg3VarName = select($Arg3VarName, t_w, overflow_mask);
      total = select(total, overflow_total, overflow_mask);
    }

    value = sqrt( total );
    *$Arg4VarName = select($Arg0VarName / value, $Arg0VarName, underflow_zero_mask);
    *$Arg5VarName = select($Arg1VarName / value, $Arg1VarName, underflow_zero_mask);
    *$Arg6VarName = select($Arg2VarName / value, $Arg2VarName, underflow_zero_mask);
    *$Arg7VarName = select($Arg3VarName / value, $Arg3VarName, underflow_zero_mask);
  }]>;


/*****************************************************************\
 * soa_fast_normalize:                                           *
\*****************************************************************/
// soa_fast_normalize1_f32
OclBuiltinImpl soa_fast_normalize1_f32 = OclBuiltinImpl<soa_fast_normalize1 , sp_fp_soa_types, 0, [{
    int$VecLength gtz_mask = ($Arg0VarName > 0.f);\
    int$VecLength zero_mask = ($Arg0VarName == 0.f);\
    $Arg0VecType res = select(($Arg0VecType)-1.f, ($Arg0VecType)1.f, gtz_mask);\
    return select(res, $Arg0VarName, zero_mask);\
  }]>;
// soa_fast_normalize2_f32
OclBuiltinImpl soa_fast_normalize2_f32 = OclBuiltinImpl<soa_fast_normalize2 , sp_fp_soa_types, 0, [{
    $Arg0VecType length = $Arg0VarName  * $Arg0VarName  + $Arg1VarName  * $Arg1VarName;
    $Arg0VecType rnorm = half_rsqrt( length );
    rnorm = (length == ($Arg0VecType)0.0f) ? ($Arg0VecType)1.0f : rnorm;
    *$Arg2VarName = $Arg0VarName * rnorm;
    *$Arg3VarName = $Arg1VarName * rnorm;
  }]>;
// soa_fast_normalize3_f32
OclBuiltinImpl soa_fast_normalize3_f32 = OclBuiltinImpl<soa_fast_normalize3 , sp_fp_soa_types, 0, [{
    $Arg0VecType length = $Arg0VarName  * $Arg0VarName  + $Arg1VarName  * $Arg1VarName + $Arg2VarName  * $Arg2VarName;
    $Arg0VecType rnorm = half_rsqrt( length );
    rnorm = (length == ($Arg0VecType)0.0f) ? ($Arg0VecType)1.0f : rnorm;
    *$Arg3VarName = $Arg0VarName * rnorm;
    *$Arg4VarName = $Arg1VarName * rnorm;
    *$Arg5VarName = $Arg2VarName * rnorm;
  }]>;
// soa_fast_normalize4_f32
OclBuiltinImpl soa_fast_normalize4_f32 = OclBuiltinImpl<soa_fast_normalize4 , sp_fp_soa_types, 0, [{
    $Arg0VecType length = $Arg0VarName  * $Arg0VarName  + $Arg1VarName  * $Arg1VarName + $Arg2VarName  * $Arg2VarName + $Arg3VarName  * $Arg3VarName;
    $Arg0VecType rnorm = half_rsqrt( length );
    rnorm = (length == ($Arg0VecType)0.0f) ? ($Arg0VecType)1.0f : rnorm;
    *$Arg4VarName = $Arg0VarName * rnorm;
    *$Arg5VarName = $Arg1VarName * rnorm;
    *$Arg6VarName = $Arg2VarName * rnorm;
    *$Arg7VarName = $Arg3VarName * rnorm;
  }]>;
