// Copyright (c) 2006 Intel Corporation
// Geometric Built-In Functions
// SSE42

/*****************************************************************\
 * dot:                                                          *
 *   float dot (floatn p0, floatn p1)                            *
 *   double dot (doublen p0, doublen p1)                         *
 *   Compute dot product.                                        *
\*****************************************************************/

OclBuiltinImpl dot_v4f32 = OclBuiltinImpl<dot, [v4f32], 0, [{
    $Arg0VarName = _mm_dp_ps($Arg0VarName, $Arg1VarName, 0b11110001);
    return $Arg0VarName.s0;
  }]>;

OclBuiltinImpl dot_v2f64 = OclBuiltinImpl<dot, [v2f64], 0, [{
    $Arg0VarName = _mm_dp_pd($Arg0VarName, $Arg1VarName, 0b110001);
    return $Arg0VarName$ExpandLoPattern;
  }]>;

OclBuiltinImpl dot_v3f64 = OclBuiltinImpl<dot, [v3f64], 0, [{
    double2 resLo = _mm_dp_pd($Arg0VarName$ExpandLoPattern, $Arg1VarName$ExpandLoPattern, 0b110001);
    double2 resHi = (double2)0.0;
    resHi.lo = $Arg0VarName$ExpandHiPattern * $Arg1VarName$ExpandHiPattern;
    resHi = resLo + resHi;
    return resHi.lo;
  }]>;


/*****************************************************************\
 * fast_normalize:                                               *
 *   float fast_normalize (floatn p0)                            *
 *   Returns p * half_rsqrt (p.x2 + p.y2 + ... )                 *
\*****************************************************************/
OclBuiltinImpl fast_normalize_v4f32 = OclBuiltinImpl<fast_normalize, [v4f32], 0, [{
    // Calculate vector length
    float4 tmp = _mm_dp_ps($Arg0VarName, $Arg0VarName, 0b11111111);
    if( tmp.s0 == 0.f )
    {
      // Special edge case: copy vector over without change
      return $Arg0VarName;
    }

    tmp = half_rsqrt(tmp);
    $Arg0VarName = $Arg0VarName * tmp;

    return $Arg0VarName;
  }]>;
