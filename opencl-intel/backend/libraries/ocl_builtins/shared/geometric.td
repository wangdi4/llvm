// Copyright (c) 2006 Intel Corporation
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



OclBuiltinImpl cross_v3f64 = OclBuiltinImpl<cross, [v3f64], 0, [{
    double4 tempX = (double4)(0);
    double4 tempY = (double4)(0);
    double4 res;
    tempX.s012 = $Arg0VarName;
    tempY.s012 = $Arg1VarName;
    res = cross(tempX, tempY);
    return res.s012;
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
      sum_multiplier = select( sum_multiplier, ($Arg0VecType)exp600, overflow_mask );
      sum = select( sum, overflow_sum, overflow_mask);
    }
    if ( intel_movemask(underflow_mask) )
    {
      $Arg0VecType underflow_p0_x = $Arg0VarName * exp700;
      $Arg0VecType underflow_p0_y = $Arg1VarName * exp700;
      $Arg0VecType underflow_sum = underflow_p0_x * underflow_p0_x + underflow_p0_y * underflow_p0_y;
      sum_multiplier = select( sum_multiplier, ($Arg0VecType)expMinus700, underflow_mask );
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
      sum_multiplier = select( sum_multiplier, ($Arg0VecType)exp600, overflow_mask );
      sum = select( sum, overflow_sum, overflow_mask);
    }
    if ( intel_movemask(underflow_mask) )
    {
      $Arg0VecType underflow_p0_x = $Arg0VarName * exp700;
      $Arg0VecType underflow_p0_y = $Arg1VarName * exp700;
      $Arg0VecType underflow_p0_z = $Arg2VarName * exp700;
      $Arg0VecType underflow_sum = underflow_p0_x * underflow_p0_x + underflow_p0_y * underflow_p0_y + underflow_p0_z * underflow_p0_z;
      sum_multiplier = select( sum_multiplier, ($Arg0VecType)expMinus700, underflow_mask );
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
      sum_multiplier = select( sum_multiplier, ($Arg0VecType)exp600, overflow_mask );
      sum = select( sum, overflow_sum, overflow_mask);
    }
    if ( intel_movemask(underflow_mask) )
    {
      $Arg0VecType underflow_p0_x = $Arg0VarName * exp700;
      $Arg0VecType underflow_p0_y = $Arg1VarName * exp700;
      $Arg0VecType underflow_p0_z = $Arg2VarName * exp700;
      $Arg0VecType underflow_p0_w = $Arg3VarName * exp700;
      $Arg0VecType underflow_sum = underflow_p0_x * underflow_p0_x + underflow_p0_y * underflow_p0_y + underflow_p0_z * underflow_p0_z + underflow_p0_w * underflow_p0_w;
      sum_multiplier = select( sum_multiplier, ($Arg0VecType)expMinus700, underflow_mask );
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




