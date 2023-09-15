// Copyright (c) 2006 Intel Corporation
// Geometric Built-In Functions
// AVX2

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
    float4 res;

    t0 = convert_double4($Arg0VarName);
    t1 = convert_double4($Arg1VarName);

    double4 temp_p0, temp_p1;
    temp_p0 = _mm256_permute4x64_pd(t0, 0xc9);
    temp_p1 = _mm256_permute4x64_pd(t1, 0xc9);

    t0 = t0 * temp_p1;
    t1 = t1 * temp_p0;

    t0 = t0 - t1;

    t0 = _mm256_permute4x64_pd(t0, 0xc9);
    res = convert_float4(t0);
    res.s3 = 0;

    return res;
  }]>;

OclBuiltinImpl cross_v4f64 = OclBuiltinImpl<cross, [v4f64], 0, [{
    double4 res;
    double4 temp_p0, temp_p1;

    temp_p0 = _mm256_permute4x64_pd($Arg0VarName, 0xc9);
    temp_p1 = _mm256_permute4x64_pd($Arg1VarName, 0xc9);

    $Arg0VarName = $Arg0VarName * temp_p1;
    $Arg1VarName = $Arg1VarName * temp_p0;

    $Arg0VarName = $Arg0VarName - $Arg1VarName;

    res = _mm256_permute4x64_pd($Arg0VarName, 0xc9);
    res.s3 = 0;

    return res;
  }]>;
