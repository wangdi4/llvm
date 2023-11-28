// Copyright (c) 2006 Intel Corporation
// Geometric Built-In Functions
// SSSE3

/*****************************************************************\
 * dot:                                                          *
 *   float dot (floatn p0, floatn p1)                            *
 *   double dot (doublen p0, doublen p1)                         *
 *   Compute dot product.                                        *
\*****************************************************************/

OclBuiltinImpl dot_v4f32 = OclBuiltinImpl<dot, [v4f32], 0, [{
    return dot($Arg0VarName.lo, $Arg1VarName.lo) + dot($Arg0VarName.hi, $Arg1VarName.hi);
  }]>;


OclBuiltinImpl dot_v3f64 = OclBuiltinImpl<dot, [v3f64], 0, [{
    return dot($Arg0VarName.xy, $Arg1VarName.xy) + dot($Arg0VarName.z, $Arg1VarName.z);
  }]>;
  
OclBuiltinImpl dot_v2f64 = OclBuiltinImpl<dot, [v2f64], 0, [{
    return $Arg0VarName.lo * $Arg1VarName.lo + $Arg0VarName.hi * $Arg1VarName.hi;
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
list<OclType> fp_soa_types    = [v4f32, v8f32, v16f32, v4f64, v8f64, v16f64];
list<OclType> sp_fp_soa_types = [v4f32, v8f32, v16f32];
list<OclType> dp_fp_soa_types = [v4f64, v8f64, v16f64];
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
 * soa_normalize:                                                *
\*****************************************************************/
// soa_normalize2_f32
OclBuiltinImpl soa_normalize2_f32 = OclBuiltinImpl<soa_normalize2 , sp_fp_soa_types, 0, [{
    double$VecLength p0_x_d = convert_double$VecLength($Arg0VarName);
    double$VecLength p0_y_d = convert_double$VecLength($Arg1VarName);
    double$VecLength length = p0_x_d * p0_x_d + p0_y_d * p0_y_d;
    double$VecLength length_inf = (double$VecLength)(0.0f);
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
        t_of_x = select( copysign(($Arg0VecType)0.0, $Arg0VarName), copysign(($Arg0VecType)1.0, $Arg0VarName), is_inf_mask);
        stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask);

        is_inf_mask = isinf( t_y );
        t_of_y = select( copysign(($Arg1VecType)0.0, $Arg1VarName), copysign(($Arg1VecType)1.0, $Arg1VarName), is_inf_mask);
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
    double$VecLength length_inf = (double$VecLength)(0.0f);
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
        t_of_x = select( copysign(($Arg0VecType)0.0, $Arg0VarName), copysign(($Arg0VecType)1.0, $Arg0VarName), is_inf_mask);
        stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask);

        is_inf_mask = isinf( t_y );
        t_of_y = select( copysign(($Arg1VecType)0.0, $Arg1VarName), copysign(($Arg1VecType)1.0, $Arg1VarName), is_inf_mask);
        stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask);

        is_inf_mask = isinf( t_z );
        t_of_z = select( copysign(($Arg2VecType)0.0, $Arg2VarName), copysign(($Arg2VecType)1.0, $Arg2VarName), is_inf_mask);
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
    double$VecLength length_inf = (double$VecLength)(0.0f);
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
        t_of_x = select( copysign(($Arg0VecType)0.0, $Arg0VarName), copysign(($Arg0VecType)1.0, $Arg0VarName), is_inf_mask);
        stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask);

        is_inf_mask = isinf( t_y );
        t_of_y = select( copysign(($Arg1VecType)0.0, $Arg1VarName), copysign(($Arg1VecType)1.0, $Arg1VarName), is_inf_mask);
        stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask);

        is_inf_mask = isinf( t_z );
        t_of_z = select( copysign(($Arg2VecType)0.0, $Arg2VarName), copysign(($Arg2VecType)1.0, $Arg2VarName), is_inf_mask);
        stil_overflow_total = select( stil_overflow_total, stil_overflow_total + 1.0, is_inf_mask);

        is_inf_mask = isinf( t_w );
        t_of_w = select( copysign(($Arg3VecType)0.0, $Arg3VarName), copysign(($Arg3VecType)1.0, $Arg3VarName), is_inf_mask);
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
