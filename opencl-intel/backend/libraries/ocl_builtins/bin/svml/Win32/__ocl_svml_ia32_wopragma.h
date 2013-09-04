/*
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//    Copyright (C) 1996-2010 Intel Corporation. All Rights Reserved.
//
*/

#ifndef __OCL_SVML_H__
#define __OCL_SVML_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <ia32intrin.h>

  typedef int int1_sse;
  typedef struct
  {
    __m128i v1;
  } int2_sse;
  typedef struct
  {
    __m128i v1;
  } int3_sse;
  typedef struct
  {
    __m128i v1;
  } int4_sse;
  typedef struct
  {
    __m128i v1;
    __m128i v2;
  } int8_sse;
  typedef struct
  {
    __m128i v1;
    __m128i v2;
    __m128i v3;
    __m128i v4;
  } int16_sse;
  typedef __int64 long1_sse;

  typedef struct
  {
    __m128i v1;
  } long2_sse;
  typedef struct
  {
    __m128i v1;
    __m128i v2;
  } long3_sse;
  typedef struct
  {
    __m128i v1;
    __m128i v2;
  } long4_sse;
  typedef struct
  {
    __m128i v1;
    __m128i v2;
    __m128i v3;
    __m128i v4;
  } long8_sse;
  typedef struct
  {
    __m128i v1;
    __m128i v2;
    __m128i v3;
    __m128i v4;
    __m128i v5;
    __m128i v6;
    __m128i v7;
    __m128i v8;
  } long16_sse;

  typedef float float1_sse;
  typedef struct
  {
    __m128 v1;
  } float2_sse;
  typedef struct
  {
    __m128 v1;
  } float3_sse;
  typedef struct
  {
    __m128 v1;
  } float4_sse;
  typedef struct
  {
    __m128 v1;
    __m128 v2;
  } float8_sse;
  typedef struct
  {
    __m128 v1;
    __m128 v2;
    __m128 v3;
    __m128 v4;
  } float16_sse;

  typedef double double1_sse;
  typedef struct
  {
    __m128d v1;
  } double2_sse;
  typedef struct
  {
    __m128d v1;
    __m128d v2;
  } double3_sse;
  typedef struct
  {
    __m128d v1;
    __m128d v2;
  } double4_sse;
  typedef struct
  {
    __m128d v1;
    __m128d v2;
    __m128d v3;
    __m128d v4;
  } double8_sse;
  typedef struct
  {
    __m128d v1;
    __m128d v2;
    __m128d v3;
    __m128d v4;
    __m128d v5;
    __m128d v6;
    __m128d v7;
    __m128d v8;
  } double16_sse;

  typedef int int1_avx;
  typedef struct
  {
    __m128i v1;
  } int2_avx;
  typedef struct
  {
    __m128i v1;
  } int3_avx;
  typedef struct
  {
    __m128i v1;
  } int4_avx;
  typedef struct
  {
    __m256i v1;
  } int8_avx;
  typedef struct
  {
    __m256i v1;
    __m256i v2;
  } int16_avx;

  typedef __int64 long1_avx;
  typedef struct
  {
    __m128i v1;
  } long2_avx;
  typedef struct
  {
    __m256i v1;
  } long3_avx;
  typedef struct
  {
    __m256i v1;
  } long4_avx;
  typedef struct
  {
    __m256i v1;
    __m256i v2;
  } long8_avx;
  typedef struct
  {
    __m256i v1;
    __m256i v2;
    __m256i v3;
    __m256i v4;
  } long16_avx;

  typedef float float1_avx;
  typedef struct
  {
    __m128 v1;
  } float2_avx;
  typedef struct
  {
    __m128 v1;
  } float3_avx;
  typedef struct
  {
    __m128 v1;
  } float4_avx;
  typedef struct
  {
    __m256 v1;
  } float8_avx;
  typedef struct
  {
    __m256 v1;
    __m256 v2;
  } float16_avx;

  typedef double double1_avx;
  typedef struct
  {
    __m128d v1;
  } double2_avx;
  typedef struct
  {
    __m256d v1;
  } double3_avx;
  typedef struct
  {
    __m256d v1;
  } double4_avx;
  typedef struct
  {
    __m256d v1;
    __m256d v2;
  } double8_avx;
  typedef struct
  {
    __m256d v1;
    __m256d v2;
    __m256d v3;
    __m256d v4;
  } double16_avx;

  typedef struct
  {
    int1_sse r1;
    int1_sse r2;
  } int1x2_sse;
  typedef struct
  {
    int2_sse r1;
    int2_sse r2;
  } int2x2_sse;
  typedef struct
  {
    int3_sse r1;
    int3_sse r2;
  } int3x2_sse;
  typedef struct
  {
    int4_sse r1;
    int4_sse r2;
  } int4x2_sse;
  typedef struct
  {
    int8_sse r1;
    int8_sse r2;
  } int8x2_sse;
  typedef struct
  {
    int16_sse r1;
    int16_sse r2;
  } int16x2_sse;

  typedef struct
  {
    long1_sse r1;
    long1_sse r2;
  } long1x2_sse;
  typedef struct
  {
    long2_sse r1;
    long2_sse r2;
  } long2x2_sse;
  typedef struct
  {
    long3_sse r1;
    long3_sse r2;
  } long3x2_sse;
  typedef struct
  {
    long4_sse r1;
    long4_sse r2;
  } long4x2_sse;
  typedef struct
  {
    long8_sse r1;
    long8_sse r2;
  } long8x2_sse;
  typedef struct
  {
    long16_sse r1;
    long16_sse r2;
  } long16x2_sse;

  typedef struct
  {
    float1_sse r1;
    float1_sse r2;
  } float1x2_sse;
  typedef struct
  {
    float2_sse r1;
    float2_sse r2;
  } float2x2_sse;
  typedef struct
  {
    float3_sse r1;
    float3_sse r2;
  } float3x2_sse;
  typedef struct
  {
    float4_sse r1;
    float4_sse r2;
  } float4x2_sse;
  typedef struct
  {
    float8_sse r1;
    float8_sse r2;
  } float8x2_sse;
  typedef struct
  {
    float16_sse r1;
    float16_sse r2;
  } float16x2_sse;

  typedef struct
  {
    double1_sse r1;
    double1_sse r2;
  } double1x2_sse;
  typedef struct
  {
    double2_sse r1;
    double2_sse r2;
  } double2x2_sse;
  typedef struct
  {
    double3_sse r1;
    double3_sse r2;
  } double3x2_sse;
  typedef struct
  {
    double4_sse r1;
    double4_sse r2;
  } double4x2_sse;
  typedef struct
  {
    double8_sse r1;
    double8_sse r2;
  } double8x2_sse;
  typedef struct
  {
    double16_sse r1;
    double16_sse r2;
  } double16x2_sse;

  typedef struct
  {
    int1_avx r1;
    int1_avx r2;
  } int1x2_avx;
  typedef struct
  {
    int2_avx r1;
    int2_avx r2;
  } int2x2_avx;
  typedef struct
  {
    int3_avx r1;
    int3_avx r2;
  } int3x2_avx;
  typedef struct
  {
    int4_avx r1;
    int4_avx r2;
  } int4x2_avx;
  typedef struct
  {
    int8_avx r1;
    int8_avx r2;
  } int8x2_avx;
  typedef struct
  {
    int16_avx r1;
    int16_avx r2;
  } int16x2_avx;

  typedef struct
  {
    long1_avx r1;
    long1_avx r2;
  } long1x2_avx;
  typedef struct
  {
    long2_avx r1;
    long2_avx r2;
  } long2x2_avx;
  typedef struct
  {
    long3_avx r1;
    long3_avx r2;
  } long3x2_avx;
  typedef struct
  {
    long4_avx r1;
    long4_avx r2;
  } long4x2_avx;
  typedef struct
  {
    long8_avx r1;
    long8_avx r2;
  } long8x2_avx;
  typedef struct
  {
    long16_avx r1;
    long16_avx r2;
  } long16x2_avx;

  typedef struct
  {
    float1_avx r1;
    float1_avx r2;
  } float1x2_avx;
  typedef struct
  {
    float2_avx r1;
    float2_avx r2;
  } float2x2_avx;
  typedef struct
  {
    float3_avx r1;
    float3_avx r2;
  } float3x2_avx;
  typedef struct
  {
    float4_avx r1;
    float4_avx r2;
  } float4x2_avx;
  typedef struct
  {
    float8_avx r1;
    float8_avx r2;
  } float8x2_avx;
  typedef struct
  {
    float16_avx r1;
    float16_avx r2;
  } float16x2_avx;

  typedef struct
  {
    double1_avx r1;
    double1_avx r2;
  } double1x2_avx;
  typedef struct
  {
    double2_avx r1;
    double2_avx r2;
  } double2x2_avx;
  typedef struct
  {
    double3_avx r1;
    double3_avx r2;
  } double3x2_avx;
  typedef struct
  {
    double4_avx r1;
    double4_avx r2;
  } double4x2_avx;
  typedef struct
  {
    double8_avx r1;
    double8_avx r2;
  } double8x2_avx;
  typedef struct
  {
    double16_avx r1;
    double16_avx r2;
  } double16x2_avx;

#if (defined __OCL_SVML_AVX)

  typedef int1_avx int1;
  typedef int2_avx int2;
  typedef int3_avx int3;
  typedef int4_avx int4;
  typedef int8_avx int8;
  typedef int16_avx int16;

  typedef long1_avx long1;
  typedef long2_avx long2;
  typedef long3_avx long3;
  typedef long4_avx long4;
  typedef long8_avx long8;
  typedef long16_avx long16;

  typedef float1_avx float1;
  typedef float2_avx float2;
  typedef float3_avx float3;
  typedef float4_avx float4;
  typedef float8_avx float8;
  typedef float16_avx float16;

  typedef double1_avx double1;
  typedef double2_avx double2;
  typedef double3_avx double3;
  typedef double4_avx double4;
  typedef double8_avx double8;
  typedef double16_avx double16;

  typedef int1x2_avx int1x2;
  typedef int2x2_avx int2x2;
  typedef int3x2_avx int3x2;
  typedef int4x2_avx int4x2;
  typedef int8x2_avx int8x2;
  typedef int16x2_avx int16x2;

  typedef long1x2_avx long1x2;
  typedef long2x2_avx long2x2;
  typedef long3x2_avx long3x2;
  typedef long4x2_avx long4x2;
  typedef long8x2_avx long8x2;
  typedef long16x2_avx long16x2;

  typedef float1x2_avx float1x2;
  typedef float2x2_avx float2x2;
  typedef float3x2_avx float3x2;
  typedef float4x2_avx float4x2;
  typedef float8x2_avx float8x2;
  typedef float16x2_avx float16x2;

  typedef double1x2_avx double1x2;
  typedef double2x2_avx double2x2;
  typedef double3x2_avx double3x2;
  typedef double4x2_avx double4x2;
  typedef double8x2_avx double8x2;
  typedef double16x2_avx double16x2;

#else

  typedef int1_sse int1;
  typedef int2_sse int2;
  typedef int3_sse int3;
  typedef int4_sse int4;
  typedef int8_sse int8;
  typedef int16_sse int16;

  typedef long1_sse long1;
  typedef long2_sse long2;
  typedef long3_sse long3;
  typedef long4_sse long4;
  typedef long8_sse long8;
  typedef long16_sse long16;

  typedef float1_sse float1;
  typedef float2_sse float2;
  typedef float3_sse float3;
  typedef float4_sse float4;
  typedef float8_sse float8;
  typedef float16_sse float16;

  typedef double1_sse double1;
  typedef double2_sse double2;
  typedef double3_sse double3;
  typedef double4_sse double4;
  typedef double8_sse double8;
  typedef double16_sse double16;

  typedef int1x2_sse int1x2;
  typedef int2x2_sse int2x2;
  typedef int3x2_sse int3x2;
  typedef int4x2_sse int4x2;
  typedef int8x2_sse int8x2;
  typedef int16x2_sse int16x2;

  typedef long1x2_sse long1x2;
  typedef long2x2_sse long2x2;
  typedef long3x2_sse long3x2;
  typedef long4x2_sse long4x2;
  typedef long8x2_sse long8x2;
  typedef long16x2_sse long16x2;

  typedef float1x2_sse float1x2;
  typedef float2x2_sse float2x2;
  typedef float3x2_sse float3x2;
  typedef float4x2_sse float4x2;
  typedef float8x2_sse float8x2;
  typedef float16x2_sse float16x2;

  typedef double1x2_sse double1x2;
  typedef double2x2_sse double2x2;
  typedef double3x2_sse double3x2;
  typedef double4x2_sse double4x2;
  typedef double8x2_sse double8x2;
  typedef double16x2_sse double16x2;

#endif

  float1_sse __ocl_svml_n8_invf1 (float1_sse a);

  float1_sse __ocl_svml_n8_divf1 (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_sqrtf1 (float1_sse a);

  float1_sse __ocl_svml_n8_rsqrtf1 (float1_sse a);

  float1_sse __ocl_svml_n8_cbrtf1 (float1_sse a);

  float1_sse __ocl_svml_n8_rcbrtf1 (float1_sse a);

  float1_sse __ocl_svml_n8_hypotf1 (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_powf1 (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_powrf1 (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_pownf1 (float1_sse a, int1_sse b);

  float1_sse __ocl_svml_n8_rootnf1 (float1_sse a, int1_sse b);

  float1_sse __ocl_svml_n8_expf1 (float1_sse a);

  float1_sse __ocl_svml_n8_exp2f1 (float1_sse a);

  float1_sse __ocl_svml_n8_exp10f1 (float1_sse a);

  float1_sse __ocl_svml_n8_expm1f1 (float1_sse a);

  float1_sse __ocl_svml_n8_logf1 (float1_sse a);

  float1_sse __ocl_svml_n8_log10f1 (float1_sse a);

  float1_sse __ocl_svml_n8_log2f1 (float1_sse a);

  float1_sse __ocl_svml_n8_log1pf1 (float1_sse a);

  float1_sse __ocl_svml_n8_sinf1 (float1_sse a);

  float1_sse __ocl_svml_n8_cosf1 (float1_sse a);

  float1_sse __ocl_svml_n8_sincosf1 (float1_sse a, float1_sse * c);

  float1x2_sse __ocl_svml_n8_sincosregf1 (float1_sse a);

  float1_sse __ocl_svml_n8_tanf1 (float1_sse a);

  float1_sse __ocl_svml_n8_sinpif1 (float1_sse a);

  float1_sse __ocl_svml_n8_cospif1 (float1_sse a);

  float1_sse __ocl_svml_n8_tanpif1 (float1_sse a);

  float1_sse __ocl_svml_n8_acosf1 (float1_sse a);

  float1_sse __ocl_svml_n8_asinf1 (float1_sse a);

  float1_sse __ocl_svml_n8_atanf1 (float1_sse a);

  float1_sse __ocl_svml_n8_atan2f1 (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_asinpif1 (float1_sse a);

  float1_sse __ocl_svml_n8_acospif1 (float1_sse a);

  float1_sse __ocl_svml_n8_atanpif1 (float1_sse a);

  float1_sse __ocl_svml_n8_atan2pif1 (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_sinhf1 (float1_sse a);

  float1_sse __ocl_svml_n8_coshf1 (float1_sse a);

  float1_sse __ocl_svml_n8_tanhf1 (float1_sse a);

  float1_sse __ocl_svml_n8_asinhf1 (float1_sse a);

  float1_sse __ocl_svml_n8_acoshf1 (float1_sse a);

  float1_sse __ocl_svml_n8_atanhf1 (float1_sse a);

  float1_sse __ocl_svml_n8_erff1 (float1_sse a);

  float1_sse __ocl_svml_n8_erfcf1 (float1_sse a);

  float1_avx __ocl_svml_g9_invf1 (float1_avx a);

  float1_avx __ocl_svml_g9_divf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_sqrtf1 (float1_avx a);

  float1_avx __ocl_svml_g9_rsqrtf1 (float1_avx a);

  float1_avx __ocl_svml_g9_cbrtf1 (float1_avx a);

  float1_avx __ocl_svml_g9_rcbrtf1 (float1_avx a);

  float1_avx __ocl_svml_g9_hypotf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_powf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_powrf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_pownf1 (float1_avx a, int1_avx b);

  float1_avx __ocl_svml_g9_rootnf1 (float1_avx a, int1_avx b);

  float1_avx __ocl_svml_g9_expf1 (float1_avx a);

  float1_avx __ocl_svml_g9_exp2f1 (float1_avx a);

  float1_avx __ocl_svml_g9_exp10f1 (float1_avx a);

  float1_avx __ocl_svml_g9_expm1f1 (float1_avx a);

  float1_avx __ocl_svml_g9_logf1 (float1_avx a);

  float1_avx __ocl_svml_g9_log10f1 (float1_avx a);

  float1_avx __ocl_svml_g9_log2f1 (float1_avx a);

  float1_avx __ocl_svml_g9_log1pf1 (float1_avx a);

  float1_avx __ocl_svml_g9_sinf1 (float1_avx a);

  float1_avx __ocl_svml_g9_cosf1 (float1_avx a);

  float1_avx __ocl_svml_g9_sincosf1 (float1_avx a, float1_avx * c);

  float1x2_avx __ocl_svml_g9_sincosregf1 (float1_avx a);

  float1_avx __ocl_svml_g9_tanf1 (float1_avx a);

  float1_avx __ocl_svml_g9_sinpif1 (float1_avx a);

  float1_avx __ocl_svml_g9_cospif1 (float1_avx a);

  float1_avx __ocl_svml_g9_tanpif1 (float1_avx a);

  float1_avx __ocl_svml_g9_acosf1 (float1_avx a);

  float1_avx __ocl_svml_g9_asinf1 (float1_avx a);

  float1_avx __ocl_svml_g9_atanf1 (float1_avx a);

  float1_avx __ocl_svml_g9_atan2f1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_asinpif1 (float1_avx a);

  float1_avx __ocl_svml_g9_acospif1 (float1_avx a);

  float1_avx __ocl_svml_g9_atanpif1 (float1_avx a);

  float1_avx __ocl_svml_g9_atan2pif1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_sinhf1 (float1_avx a);

  float1_avx __ocl_svml_g9_coshf1 (float1_avx a);

  float1_avx __ocl_svml_g9_tanhf1 (float1_avx a);

  float1_avx __ocl_svml_g9_asinhf1 (float1_avx a);

  float1_avx __ocl_svml_g9_acoshf1 (float1_avx a);

  float1_avx __ocl_svml_g9_atanhf1 (float1_avx a);

  float1_avx __ocl_svml_g9_erff1 (float1_avx a);

  float1_avx __ocl_svml_g9_erfcf1 (float1_avx a);

  float1_avx __ocl_svml_s9_invf1 (float1_avx a);

  float1_avx __ocl_svml_s9_divf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_sqrtf1 (float1_avx a);

  float1_avx __ocl_svml_s9_rsqrtf1 (float1_avx a);

  float1_avx __ocl_svml_s9_cbrtf1 (float1_avx a);

  float1_avx __ocl_svml_s9_rcbrtf1 (float1_avx a);

  float1_avx __ocl_svml_s9_hypotf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_powf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_powrf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_pownf1 (float1_avx a, int1_avx b);

  float1_avx __ocl_svml_s9_rootnf1 (float1_avx a, int1_avx b);

  float1_avx __ocl_svml_s9_expf1 (float1_avx a);

  float1_avx __ocl_svml_s9_exp2f1 (float1_avx a);

  float1_avx __ocl_svml_s9_exp10f1 (float1_avx a);

  float1_avx __ocl_svml_s9_expm1f1 (float1_avx a);

  float1_avx __ocl_svml_s9_logf1 (float1_avx a);

  float1_avx __ocl_svml_s9_log10f1 (float1_avx a);

  float1_avx __ocl_svml_s9_log2f1 (float1_avx a);

  float1_avx __ocl_svml_s9_log1pf1 (float1_avx a);

  float1_avx __ocl_svml_s9_sinf1 (float1_avx a);

  float1_avx __ocl_svml_s9_cosf1 (float1_avx a);

  float1_avx __ocl_svml_s9_sincosf1 (float1_avx a, float1_avx * c);

  float1x2_avx __ocl_svml_s9_sincosregf1 (float1_avx a);

  float1_avx __ocl_svml_s9_tanf1 (float1_avx a);

  float1_avx __ocl_svml_s9_sinpif1 (float1_avx a);

  float1_avx __ocl_svml_s9_cospif1 (float1_avx a);

  float1_avx __ocl_svml_s9_tanpif1 (float1_avx a);

  float1_avx __ocl_svml_s9_acosf1 (float1_avx a);

  float1_avx __ocl_svml_s9_asinf1 (float1_avx a);

  float1_avx __ocl_svml_s9_atanf1 (float1_avx a);

  float1_avx __ocl_svml_s9_atan2f1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_asinpif1 (float1_avx a);

  float1_avx __ocl_svml_s9_acospif1 (float1_avx a);

  float1_avx __ocl_svml_s9_atanpif1 (float1_avx a);

  float1_avx __ocl_svml_s9_atan2pif1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_sinhf1 (float1_avx a);

  float1_avx __ocl_svml_s9_coshf1 (float1_avx a);

  float1_avx __ocl_svml_s9_tanhf1 (float1_avx a);

  float1_avx __ocl_svml_s9_asinhf1 (float1_avx a);

  float1_avx __ocl_svml_s9_acoshf1 (float1_avx a);

  float1_avx __ocl_svml_s9_atanhf1 (float1_avx a);

  float1_avx __ocl_svml_s9_erff1 (float1_avx a);

  float1_avx __ocl_svml_s9_erfcf1 (float1_avx a);

  float1_sse __ocl_svml_n8_fmaf1 (float1_sse a, float1_sse b, float1_sse c);

  float1_sse __ocl_svml_n8_fabsf1 (float1_sse a);

  float1_sse __ocl_svml_n8_fminf1 (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_fmaxf1 (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_maxmagf1 (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_minmagf1 (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_nanf1 (int1_sse a);

  float1_sse __ocl_svml_n8_ceilf1 (float1_sse a);

  float1_sse __ocl_svml_n8_floorf1 (float1_sse a);

  float1_sse __ocl_svml_n8_roundf1 (float1_sse a);

  float1_sse __ocl_svml_n8_truncf1 (float1_sse a);

  float1_sse __ocl_svml_n8_rintf1 (float1_sse a);

  float1_sse __ocl_svml_n8_nearbyintf1 (float1_sse a);

  float1_sse __ocl_svml_n8_fmodf1 (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_modff1 (float1_sse a, float1_sse * c);

  float1_sse __ocl_svml_n8_remainderf1 (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_remquof1 (float1_sse a, float1_sse b, int1_sse * c);

  float1_sse __ocl_svml_n8_copysignf1 (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_frexpf1 (float1_sse a, int1_sse * c);

  float1_sse __ocl_svml_n8_fdimf1 (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_logbf1 (float1_sse a);

  int1_sse __ocl_svml_n8_ilogbf1 (float1_sse a);

  float1_sse __ocl_svml_n8_nextafterf1 (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_ldexpf1 (float1_sse a, int1_sse b);

  float1_sse __ocl_svml_n8_fractf1 (float1_sse a, float1_sse * c);

  float1_sse __ocl_svml_n8_tgammaf1 (float1_sse a);

  float1_sse __ocl_svml_n8_lgammaf1 (float1_sse a);

  float1_sse __ocl_svml_n8_lgammarf1 (float1_sse a, int1_sse * c);

  long1_sse __ocl_svml_n8_cvtfptou64rtenosatf1 (float1_sse a);

  long1_sse __ocl_svml_n8_cvtfptou64rtesatf1 (float1_sse a);

  long1_sse __ocl_svml_n8_cvtfptou64rtnnosatf1 (float1_sse a);

  long1_sse __ocl_svml_n8_cvtfptou64rtnsatf1 (float1_sse a);

  long1_sse __ocl_svml_n8_cvtfptou64rtpnosatf1 (float1_sse a);

  long1_sse __ocl_svml_n8_cvtfptou64rtpsatf1 (float1_sse a);

  long1_sse __ocl_svml_n8_cvtfptou64rtznosatf1 (float1_sse a);

  long1_sse __ocl_svml_n8_cvtfptou64rtzsatf1 (float1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtenosatf1 (float1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtesatf1 (float1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtnnosatf1 (float1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtnsatf1 (float1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtpnosatf1 (float1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtpsatf1 (float1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtznosatf1 (float1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtzsatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtenosatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtesatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtnnosatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtnsatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtpnosatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtpsatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtznosatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtzsatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtenosatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtesatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtnnosatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtnsatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtpnosatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtpsatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtznosatf1 (float1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtzsatf1 (float1_sse a);

  float1_sse __ocl_svml_n8_cvtu64tofprtef1 (long1_sse a);

  float1_sse __ocl_svml_n8_cvtu64tofprtnf1 (long1_sse a);

  float1_sse __ocl_svml_n8_cvtu64tofprtpf1 (long1_sse a);

  float1_sse __ocl_svml_n8_cvtu64tofprtzf1 (long1_sse a);

  float1_sse __ocl_svml_n8_cvti64tofprtef1 (long1_sse a);

  float1_sse __ocl_svml_n8_cvti64tofprtnf1 (long1_sse a);

  float1_sse __ocl_svml_n8_cvti64tofprtpf1 (long1_sse a);

  float1_sse __ocl_svml_n8_cvti64tofprtzf1 (long1_sse a);

  float1_sse __ocl_svml_n8_cvtu32tofprtef1 (int1_sse a);

  float1_sse __ocl_svml_n8_cvtu32tofprtnf1 (int1_sse a);

  float1_sse __ocl_svml_n8_cvtu32tofprtpf1 (int1_sse a);

  float1_sse __ocl_svml_n8_cvtu32tofprtzf1 (int1_sse a);

  float1_sse __ocl_svml_n8_cvti32tofprtef1 (int1_sse a);

  float1_sse __ocl_svml_n8_cvti32tofprtnf1 (int1_sse a);

  float1_sse __ocl_svml_n8_cvti32tofprtpf1 (int1_sse a);

  float1_sse __ocl_svml_n8_cvti32tofprtzf1 (int1_sse a);

  float1_avx __ocl_svml_g9_fmaf1 (float1_avx a, float1_avx b, float1_avx c);

  float1_avx __ocl_svml_g9_fabsf1 (float1_avx a);

  float1_avx __ocl_svml_g9_fminf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_fmaxf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_maxmagf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_minmagf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_nanf1 (int1_avx a);

  float1_avx __ocl_svml_g9_ceilf1 (float1_avx a);

  float1_avx __ocl_svml_g9_floorf1 (float1_avx a);

  float1_avx __ocl_svml_g9_roundf1 (float1_avx a);

  float1_avx __ocl_svml_g9_truncf1 (float1_avx a);

  float1_avx __ocl_svml_g9_rintf1 (float1_avx a);

  float1_avx __ocl_svml_g9_nearbyintf1 (float1_avx a);

  float1_avx __ocl_svml_g9_fmodf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_modff1 (float1_avx a, float1_avx * c);

  float1_avx __ocl_svml_g9_remainderf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_remquof1 (float1_avx a, float1_avx b, int1_avx * c);

  float1_avx __ocl_svml_g9_copysignf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_frexpf1 (float1_avx a, int1_avx * c);

  float1_avx __ocl_svml_g9_fdimf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_logbf1 (float1_avx a);

  int1_avx __ocl_svml_g9_ilogbf1 (float1_avx a);

  float1_avx __ocl_svml_g9_nextafterf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_ldexpf1 (float1_avx a, int1_avx b);

  float1_avx __ocl_svml_g9_fractf1 (float1_avx a, float1_avx * c);

  float1_avx __ocl_svml_g9_tgammaf1 (float1_avx a);

  float1_avx __ocl_svml_g9_lgammaf1 (float1_avx a);

  float1_avx __ocl_svml_g9_lgammarf1 (float1_avx a, int1_avx * c);

  long1_avx __ocl_svml_g9_cvtfptou64rtenosatf1 (float1_avx a);

  long1_avx __ocl_svml_g9_cvtfptou64rtesatf1 (float1_avx a);

  long1_avx __ocl_svml_g9_cvtfptou64rtnnosatf1 (float1_avx a);

  long1_avx __ocl_svml_g9_cvtfptou64rtnsatf1 (float1_avx a);

  long1_avx __ocl_svml_g9_cvtfptou64rtpnosatf1 (float1_avx a);

  long1_avx __ocl_svml_g9_cvtfptou64rtpsatf1 (float1_avx a);

  long1_avx __ocl_svml_g9_cvtfptou64rtznosatf1 (float1_avx a);

  long1_avx __ocl_svml_g9_cvtfptou64rtzsatf1 (float1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtenosatf1 (float1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtesatf1 (float1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtnnosatf1 (float1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtnsatf1 (float1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtpnosatf1 (float1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtpsatf1 (float1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtznosatf1 (float1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtzsatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtenosatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtesatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtnnosatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtnsatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtpnosatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtpsatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtznosatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtzsatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtenosatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtesatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtnnosatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtnsatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtpnosatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtpsatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtznosatf1 (float1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtzsatf1 (float1_avx a);

  float1_avx __ocl_svml_g9_cvtu64tofprtef1 (long1_avx a);

  float1_avx __ocl_svml_g9_cvtu64tofprtnf1 (long1_avx a);

  float1_avx __ocl_svml_g9_cvtu64tofprtpf1 (long1_avx a);

  float1_avx __ocl_svml_g9_cvtu64tofprtzf1 (long1_avx a);

  float1_avx __ocl_svml_g9_cvti64tofprtef1 (long1_avx a);

  float1_avx __ocl_svml_g9_cvti64tofprtnf1 (long1_avx a);

  float1_avx __ocl_svml_g9_cvti64tofprtpf1 (long1_avx a);

  float1_avx __ocl_svml_g9_cvti64tofprtzf1 (long1_avx a);

  float1_avx __ocl_svml_g9_cvtu32tofprtef1 (int1_avx a);

  float1_avx __ocl_svml_g9_cvtu32tofprtnf1 (int1_avx a);

  float1_avx __ocl_svml_g9_cvtu32tofprtpf1 (int1_avx a);

  float1_avx __ocl_svml_g9_cvtu32tofprtzf1 (int1_avx a);

  float1_avx __ocl_svml_g9_cvti32tofprtef1 (int1_avx a);

  float1_avx __ocl_svml_g9_cvti32tofprtnf1 (int1_avx a);

  float1_avx __ocl_svml_g9_cvti32tofprtpf1 (int1_avx a);

  float1_avx __ocl_svml_g9_cvti32tofprtzf1 (int1_avx a);

  float1_avx __ocl_svml_s9_fmaf1 (float1_avx a, float1_avx b, float1_avx c);

  float1_avx __ocl_svml_s9_fabsf1 (float1_avx a);

  float1_avx __ocl_svml_s9_fminf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_fmaxf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_maxmagf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_minmagf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_nanf1 (int1_avx a);

  float1_avx __ocl_svml_s9_ceilf1 (float1_avx a);

  float1_avx __ocl_svml_s9_floorf1 (float1_avx a);

  float1_avx __ocl_svml_s9_roundf1 (float1_avx a);

  float1_avx __ocl_svml_s9_truncf1 (float1_avx a);

  float1_avx __ocl_svml_s9_rintf1 (float1_avx a);

  float1_avx __ocl_svml_s9_nearbyintf1 (float1_avx a);

  float1_avx __ocl_svml_s9_fmodf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_modff1 (float1_avx a, float1_avx * c);

  float1_avx __ocl_svml_s9_remainderf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_remquof1 (float1_avx a, float1_avx b, int1_avx * c);

  float1_avx __ocl_svml_s9_copysignf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_frexpf1 (float1_avx a, int1_avx * c);

  float1_avx __ocl_svml_s9_fdimf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_logbf1 (float1_avx a);

  int1_avx __ocl_svml_s9_ilogbf1 (float1_avx a);

  float1_avx __ocl_svml_s9_nextafterf1 (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_ldexpf1 (float1_avx a, int1_avx b);

  float1_avx __ocl_svml_s9_fractf1 (float1_avx a, float1_avx * c);

  float1_avx __ocl_svml_s9_tgammaf1 (float1_avx a);

  float1_avx __ocl_svml_s9_lgammaf1 (float1_avx a);

  float1_avx __ocl_svml_s9_lgammarf1 (float1_avx a, int1_avx * c);

  long1_avx __ocl_svml_s9_cvtfptou64rtenosatf1 (float1_avx a);

  long1_avx __ocl_svml_s9_cvtfptou64rtesatf1 (float1_avx a);

  long1_avx __ocl_svml_s9_cvtfptou64rtnnosatf1 (float1_avx a);

  long1_avx __ocl_svml_s9_cvtfptou64rtnsatf1 (float1_avx a);

  long1_avx __ocl_svml_s9_cvtfptou64rtpnosatf1 (float1_avx a);

  long1_avx __ocl_svml_s9_cvtfptou64rtpsatf1 (float1_avx a);

  long1_avx __ocl_svml_s9_cvtfptou64rtznosatf1 (float1_avx a);

  long1_avx __ocl_svml_s9_cvtfptou64rtzsatf1 (float1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtenosatf1 (float1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtesatf1 (float1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtnnosatf1 (float1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtnsatf1 (float1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtpnosatf1 (float1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtpsatf1 (float1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtznosatf1 (float1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtzsatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtenosatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtesatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtnnosatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtnsatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtpnosatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtpsatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtznosatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtzsatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtenosatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtesatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtnnosatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtnsatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtpnosatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtpsatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtznosatf1 (float1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtzsatf1 (float1_avx a);

  float1_avx __ocl_svml_s9_cvtu64tofprtef1 (long1_avx a);

  float1_avx __ocl_svml_s9_cvtu64tofprtnf1 (long1_avx a);

  float1_avx __ocl_svml_s9_cvtu64tofprtpf1 (long1_avx a);

  float1_avx __ocl_svml_s9_cvtu64tofprtzf1 (long1_avx a);

  float1_avx __ocl_svml_s9_cvti64tofprtef1 (long1_avx a);

  float1_avx __ocl_svml_s9_cvti64tofprtnf1 (long1_avx a);

  float1_avx __ocl_svml_s9_cvti64tofprtpf1 (long1_avx a);

  float1_avx __ocl_svml_s9_cvti64tofprtzf1 (long1_avx a);

  float1_avx __ocl_svml_s9_cvtu32tofprtef1 (int1_avx a);

  float1_avx __ocl_svml_s9_cvtu32tofprtnf1 (int1_avx a);

  float1_avx __ocl_svml_s9_cvtu32tofprtpf1 (int1_avx a);

  float1_avx __ocl_svml_s9_cvtu32tofprtzf1 (int1_avx a);

  float1_avx __ocl_svml_s9_cvti32tofprtef1 (int1_avx a);

  float1_avx __ocl_svml_s9_cvti32tofprtnf1 (int1_avx a);

  float1_avx __ocl_svml_s9_cvti32tofprtpf1 (int1_avx a);

  float1_avx __ocl_svml_s9_cvti32tofprtzf1 (int1_avx a);

  double1_sse __ocl_svml_n8_inv1 (double1_sse a);

  double1_sse __ocl_svml_n8_div1 (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_sqrt1 (double1_sse a);

  double1_sse __ocl_svml_n8_rsqrt1 (double1_sse a);

  double1_sse __ocl_svml_n8_cbrt1 (double1_sse a);

  double1_sse __ocl_svml_n8_rcbrt1 (double1_sse a);

  double1_sse __ocl_svml_n8_hypot1 (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_pow1 (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_powr1 (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_pown1 (double1_sse a, int1_sse b);

  double1_sse __ocl_svml_n8_rootn1 (double1_sse a, int1_sse b);

  double1_sse __ocl_svml_n8_exp1 (double1_sse a);

  double1_sse __ocl_svml_n8_exp21 (double1_sse a);

  double1_sse __ocl_svml_n8_exp101 (double1_sse a);

  double1_sse __ocl_svml_n8_expm11 (double1_sse a);

  double1_sse __ocl_svml_n8_log1 (double1_sse a);

  double1_sse __ocl_svml_n8_log101 (double1_sse a);

  double1_sse __ocl_svml_n8_log21 (double1_sse a);

  double1_sse __ocl_svml_n8_log1p1 (double1_sse a);

  double1_sse __ocl_svml_n8_sin1 (double1_sse a);

  double1_sse __ocl_svml_n8_cos1 (double1_sse a);

  double1_sse __ocl_svml_n8_sincos1 (double1_sse a, double1_sse * c);

  double1x2_sse __ocl_svml_n8_sincosreg1 (double1_sse a);

  double1_sse __ocl_svml_n8_tan1 (double1_sse a);

  double1_sse __ocl_svml_n8_sinpi1 (double1_sse a);

  double1_sse __ocl_svml_n8_cospi1 (double1_sse a);

  double1_sse __ocl_svml_n8_tanpi1 (double1_sse a);

  double1_sse __ocl_svml_n8_acos1 (double1_sse a);

  double1_sse __ocl_svml_n8_asin1 (double1_sse a);

  double1_sse __ocl_svml_n8_atan1 (double1_sse a);

  double1_sse __ocl_svml_n8_atan21 (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_asinpi1 (double1_sse a);

  double1_sse __ocl_svml_n8_acospi1 (double1_sse a);

  double1_sse __ocl_svml_n8_atanpi1 (double1_sse a);

  double1_sse __ocl_svml_n8_atan2pi1 (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_sinh1 (double1_sse a);

  double1_sse __ocl_svml_n8_cosh1 (double1_sse a);

  double1_sse __ocl_svml_n8_tanh1 (double1_sse a);

  double1_sse __ocl_svml_n8_asinh1 (double1_sse a);

  double1_sse __ocl_svml_n8_acosh1 (double1_sse a);

  double1_sse __ocl_svml_n8_atanh1 (double1_sse a);

  double1_sse __ocl_svml_n8_erf1 (double1_sse a);

  double1_sse __ocl_svml_n8_erfc1 (double1_sse a);

  double1_avx __ocl_svml_g9_inv1 (double1_avx a);

  double1_avx __ocl_svml_g9_div1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_sqrt1 (double1_avx a);

  double1_avx __ocl_svml_g9_rsqrt1 (double1_avx a);

  double1_avx __ocl_svml_g9_cbrt1 (double1_avx a);

  double1_avx __ocl_svml_g9_rcbrt1 (double1_avx a);

  double1_avx __ocl_svml_g9_hypot1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_pow1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_powr1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_pown1 (double1_avx a, int1_avx b);

  double1_avx __ocl_svml_g9_rootn1 (double1_avx a, int1_avx b);

  double1_avx __ocl_svml_g9_exp1 (double1_avx a);

  double1_avx __ocl_svml_g9_exp21 (double1_avx a);

  double1_avx __ocl_svml_g9_exp101 (double1_avx a);

  double1_avx __ocl_svml_g9_expm11 (double1_avx a);

  double1_avx __ocl_svml_g9_log1 (double1_avx a);

  double1_avx __ocl_svml_g9_log101 (double1_avx a);

  double1_avx __ocl_svml_g9_log21 (double1_avx a);

  double1_avx __ocl_svml_g9_log1p1 (double1_avx a);

  double1_avx __ocl_svml_g9_sin1 (double1_avx a);

  double1_avx __ocl_svml_g9_cos1 (double1_avx a);

  double1_avx __ocl_svml_g9_sincos1 (double1_avx a, double1_avx * c);

  double1x2_avx __ocl_svml_g9_sincosreg1 (double1_avx a);

  double1_avx __ocl_svml_g9_tan1 (double1_avx a);

  double1_avx __ocl_svml_g9_sinpi1 (double1_avx a);

  double1_avx __ocl_svml_g9_cospi1 (double1_avx a);

  double1_avx __ocl_svml_g9_tanpi1 (double1_avx a);

  double1_avx __ocl_svml_g9_acos1 (double1_avx a);

  double1_avx __ocl_svml_g9_asin1 (double1_avx a);

  double1_avx __ocl_svml_g9_atan1 (double1_avx a);

  double1_avx __ocl_svml_g9_atan21 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_asinpi1 (double1_avx a);

  double1_avx __ocl_svml_g9_acospi1 (double1_avx a);

  double1_avx __ocl_svml_g9_atanpi1 (double1_avx a);

  double1_avx __ocl_svml_g9_atan2pi1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_sinh1 (double1_avx a);

  double1_avx __ocl_svml_g9_cosh1 (double1_avx a);

  double1_avx __ocl_svml_g9_tanh1 (double1_avx a);

  double1_avx __ocl_svml_g9_asinh1 (double1_avx a);

  double1_avx __ocl_svml_g9_acosh1 (double1_avx a);

  double1_avx __ocl_svml_g9_atanh1 (double1_avx a);

  double1_avx __ocl_svml_g9_erf1 (double1_avx a);

  double1_avx __ocl_svml_g9_erfc1 (double1_avx a);

  double1_avx __ocl_svml_s9_inv1 (double1_avx a);

  double1_avx __ocl_svml_s9_div1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_sqrt1 (double1_avx a);

  double1_avx __ocl_svml_s9_rsqrt1 (double1_avx a);

  double1_avx __ocl_svml_s9_cbrt1 (double1_avx a);

  double1_avx __ocl_svml_s9_rcbrt1 (double1_avx a);

  double1_avx __ocl_svml_s9_hypot1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_pow1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_powr1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_pown1 (double1_avx a, int1_avx b);

  double1_avx __ocl_svml_s9_rootn1 (double1_avx a, int1_avx b);

  double1_avx __ocl_svml_s9_exp1 (double1_avx a);

  double1_avx __ocl_svml_s9_exp21 (double1_avx a);

  double1_avx __ocl_svml_s9_exp101 (double1_avx a);

  double1_avx __ocl_svml_s9_expm11 (double1_avx a);

  double1_avx __ocl_svml_s9_log1 (double1_avx a);

  double1_avx __ocl_svml_s9_log101 (double1_avx a);

  double1_avx __ocl_svml_s9_log21 (double1_avx a);

  double1_avx __ocl_svml_s9_log1p1 (double1_avx a);

  double1_avx __ocl_svml_s9_sin1 (double1_avx a);

  double1_avx __ocl_svml_s9_cos1 (double1_avx a);

  double1_avx __ocl_svml_s9_sincos1 (double1_avx a, double1_avx * c);

  double1x2_avx __ocl_svml_s9_sincosreg1 (double1_avx a);

  double1_avx __ocl_svml_s9_tan1 (double1_avx a);

  double1_avx __ocl_svml_s9_sinpi1 (double1_avx a);

  double1_avx __ocl_svml_s9_cospi1 (double1_avx a);

  double1_avx __ocl_svml_s9_tanpi1 (double1_avx a);

  double1_avx __ocl_svml_s9_acos1 (double1_avx a);

  double1_avx __ocl_svml_s9_asin1 (double1_avx a);

  double1_avx __ocl_svml_s9_atan1 (double1_avx a);

  double1_avx __ocl_svml_s9_atan21 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_asinpi1 (double1_avx a);

  double1_avx __ocl_svml_s9_acospi1 (double1_avx a);

  double1_avx __ocl_svml_s9_atanpi1 (double1_avx a);

  double1_avx __ocl_svml_s9_atan2pi1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_sinh1 (double1_avx a);

  double1_avx __ocl_svml_s9_cosh1 (double1_avx a);

  double1_avx __ocl_svml_s9_tanh1 (double1_avx a);

  double1_avx __ocl_svml_s9_asinh1 (double1_avx a);

  double1_avx __ocl_svml_s9_acosh1 (double1_avx a);

  double1_avx __ocl_svml_s9_atanh1 (double1_avx a);

  double1_avx __ocl_svml_s9_erf1 (double1_avx a);

  double1_avx __ocl_svml_s9_erfc1 (double1_avx a);

  double1_sse __ocl_svml_n8_fma1 (double1_sse a, double1_sse b, double1_sse c);

  double1_sse __ocl_svml_n8_fabs1 (double1_sse a);

  double1_sse __ocl_svml_n8_fmin1 (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_fmax1 (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_maxmag1 (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_minmag1 (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_nan1 (long1_sse a);

  double1_sse __ocl_svml_n8_ceil1 (double1_sse a);

  double1_sse __ocl_svml_n8_floor1 (double1_sse a);

  double1_sse __ocl_svml_n8_round1 (double1_sse a);

  double1_sse __ocl_svml_n8_trunc1 (double1_sse a);

  double1_sse __ocl_svml_n8_rint1 (double1_sse a);

  double1_sse __ocl_svml_n8_nearbyint1 (double1_sse a);

  double1_sse __ocl_svml_n8_fmod1 (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_modf1 (double1_sse a, double1_sse * c);

  double1_sse __ocl_svml_n8_remainder1 (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_remquo1 (double1_sse a, double1_sse b, int1_sse * c);

  double1_sse __ocl_svml_n8_copysign1 (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_frexp1 (double1_sse a, int1_sse * c);

  double1_sse __ocl_svml_n8_fdim1 (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_logb1 (double1_sse a);

  int1_sse __ocl_svml_n8_ilogb1 (double1_sse a);

  double1_sse __ocl_svml_n8_nextafter1 (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_ldexp1 (double1_sse a, int1_sse b);

  double1_sse __ocl_svml_n8_fract1 (double1_sse a, double1_sse * c);

  double1_sse __ocl_svml_n8_tgamma1 (double1_sse a);

  double1_sse __ocl_svml_n8_lgamma1 (double1_sse a);

  double1_sse __ocl_svml_n8_lgammar1 (double1_sse a, int1_sse * c);

  long1_sse __ocl_svml_n8_cvtfptou64rtenosat1 (double1_sse a);

  long1_sse __ocl_svml_n8_cvtfptou64rtesat1 (double1_sse a);

  long1_sse __ocl_svml_n8_cvtfptou64rtnnosat1 (double1_sse a);

  long1_sse __ocl_svml_n8_cvtfptou64rtnsat1 (double1_sse a);

  long1_sse __ocl_svml_n8_cvtfptou64rtpnosat1 (double1_sse a);

  long1_sse __ocl_svml_n8_cvtfptou64rtpsat1 (double1_sse a);

  long1_sse __ocl_svml_n8_cvtfptou64rtznosat1 (double1_sse a);

  long1_sse __ocl_svml_n8_cvtfptou64rtzsat1 (double1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtenosat1 (double1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtesat1 (double1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtnnosat1 (double1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtnsat1 (double1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtpnosat1 (double1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtpsat1 (double1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtznosat1 (double1_sse a);

  long1_sse __ocl_svml_n8_cvtfptoi64rtzsat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtenosat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtesat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtnnosat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtnsat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtpnosat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtpsat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtznosat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptou32rtzsat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtenosat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtesat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtnnosat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtnsat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtpnosat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtpsat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtznosat1 (double1_sse a);

  int1_sse __ocl_svml_n8_cvtfptoi32rtzsat1 (double1_sse a);

  double1_sse __ocl_svml_n8_cvtu64tofprte1 (long1_sse a);

  double1_sse __ocl_svml_n8_cvtu64tofprtn1 (long1_sse a);

  double1_sse __ocl_svml_n8_cvtu64tofprtp1 (long1_sse a);

  double1_sse __ocl_svml_n8_cvtu64tofprtz1 (long1_sse a);

  double1_sse __ocl_svml_n8_cvti64tofprte1 (long1_sse a);

  double1_sse __ocl_svml_n8_cvti64tofprtn1 (long1_sse a);

  double1_sse __ocl_svml_n8_cvti64tofprtp1 (long1_sse a);

  double1_sse __ocl_svml_n8_cvti64tofprtz1 (long1_sse a);

  double1_sse __ocl_svml_n8_cvtu32tofprte1 (int1_sse a);

  double1_sse __ocl_svml_n8_cvtu32tofprtn1 (int1_sse a);

  double1_sse __ocl_svml_n8_cvtu32tofprtp1 (int1_sse a);

  double1_sse __ocl_svml_n8_cvtu32tofprtz1 (int1_sse a);

  double1_sse __ocl_svml_n8_cvti32tofprte1 (int1_sse a);

  double1_sse __ocl_svml_n8_cvti32tofprtn1 (int1_sse a);

  double1_sse __ocl_svml_n8_cvti32tofprtp1 (int1_sse a);

  double1_sse __ocl_svml_n8_cvti32tofprtz1 (int1_sse a);

  float1_sse __ocl_svml_n8_cvtfp64tofp32rte1 (double1_sse a);

  float1_sse __ocl_svml_n8_cvtfp64tofp32rtn1 (double1_sse a);

  float1_sse __ocl_svml_n8_cvtfp64tofp32rtp1 (double1_sse a);

  float1_sse __ocl_svml_n8_cvtfp64tofp32rtz1 (double1_sse a);

  double1_avx __ocl_svml_g9_fma1 (double1_avx a, double1_avx b, double1_avx c);

  double1_avx __ocl_svml_g9_fabs1 (double1_avx a);

  double1_avx __ocl_svml_g9_fmin1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_fmax1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_maxmag1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_minmag1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_nan1 (long1_avx a);

  double1_avx __ocl_svml_g9_ceil1 (double1_avx a);

  double1_avx __ocl_svml_g9_floor1 (double1_avx a);

  double1_avx __ocl_svml_g9_round1 (double1_avx a);

  double1_avx __ocl_svml_g9_trunc1 (double1_avx a);

  double1_avx __ocl_svml_g9_rint1 (double1_avx a);

  double1_avx __ocl_svml_g9_nearbyint1 (double1_avx a);

  double1_avx __ocl_svml_g9_fmod1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_modf1 (double1_avx a, double1_avx * c);

  double1_avx __ocl_svml_g9_remainder1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_remquo1 (double1_avx a, double1_avx b, int1_avx * c);

  double1_avx __ocl_svml_g9_copysign1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_frexp1 (double1_avx a, int1_avx * c);

  double1_avx __ocl_svml_g9_fdim1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_logb1 (double1_avx a);

  int1_avx __ocl_svml_g9_ilogb1 (double1_avx a);

  double1_avx __ocl_svml_g9_nextafter1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_ldexp1 (double1_avx a, int1_avx b);

  double1_avx __ocl_svml_g9_fract1 (double1_avx a, double1_avx * c);

  double1_avx __ocl_svml_g9_tgamma1 (double1_avx a);

  double1_avx __ocl_svml_g9_lgamma1 (double1_avx a);

  double1_avx __ocl_svml_g9_lgammar1 (double1_avx a, int1_avx * c);

  long1_avx __ocl_svml_g9_cvtfptou64rtenosat1 (double1_avx a);

  long1_avx __ocl_svml_g9_cvtfptou64rtesat1 (double1_avx a);

  long1_avx __ocl_svml_g9_cvtfptou64rtnnosat1 (double1_avx a);

  long1_avx __ocl_svml_g9_cvtfptou64rtnsat1 (double1_avx a);

  long1_avx __ocl_svml_g9_cvtfptou64rtpnosat1 (double1_avx a);

  long1_avx __ocl_svml_g9_cvtfptou64rtpsat1 (double1_avx a);

  long1_avx __ocl_svml_g9_cvtfptou64rtznosat1 (double1_avx a);

  long1_avx __ocl_svml_g9_cvtfptou64rtzsat1 (double1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtenosat1 (double1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtesat1 (double1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtnnosat1 (double1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtnsat1 (double1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtpnosat1 (double1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtpsat1 (double1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtznosat1 (double1_avx a);

  long1_avx __ocl_svml_g9_cvtfptoi64rtzsat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtenosat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtesat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtnnosat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtnsat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtpnosat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtpsat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtznosat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptou32rtzsat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtenosat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtesat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtnnosat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtnsat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtpnosat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtpsat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtznosat1 (double1_avx a);

  int1_avx __ocl_svml_g9_cvtfptoi32rtzsat1 (double1_avx a);

  double1_avx __ocl_svml_g9_cvtu64tofprte1 (long1_avx a);

  double1_avx __ocl_svml_g9_cvtu64tofprtn1 (long1_avx a);

  double1_avx __ocl_svml_g9_cvtu64tofprtp1 (long1_avx a);

  double1_avx __ocl_svml_g9_cvtu64tofprtz1 (long1_avx a);

  double1_avx __ocl_svml_g9_cvti64tofprte1 (long1_avx a);

  double1_avx __ocl_svml_g9_cvti64tofprtn1 (long1_avx a);

  double1_avx __ocl_svml_g9_cvti64tofprtp1 (long1_avx a);

  double1_avx __ocl_svml_g9_cvti64tofprtz1 (long1_avx a);

  double1_avx __ocl_svml_g9_cvtu32tofprte1 (int1_avx a);

  double1_avx __ocl_svml_g9_cvtu32tofprtn1 (int1_avx a);

  double1_avx __ocl_svml_g9_cvtu32tofprtp1 (int1_avx a);

  double1_avx __ocl_svml_g9_cvtu32tofprtz1 (int1_avx a);

  double1_avx __ocl_svml_g9_cvti32tofprte1 (int1_avx a);

  double1_avx __ocl_svml_g9_cvti32tofprtn1 (int1_avx a);

  double1_avx __ocl_svml_g9_cvti32tofprtp1 (int1_avx a);

  double1_avx __ocl_svml_g9_cvti32tofprtz1 (int1_avx a);

  float1_avx __ocl_svml_g9_cvtfp64tofp32rte1 (double1_avx a);

  float1_avx __ocl_svml_g9_cvtfp64tofp32rtn1 (double1_avx a);

  float1_avx __ocl_svml_g9_cvtfp64tofp32rtp1 (double1_avx a);

  float1_avx __ocl_svml_g9_cvtfp64tofp32rtz1 (double1_avx a);

  double1_avx __ocl_svml_s9_fma1 (double1_avx a, double1_avx b, double1_avx c);

  double1_avx __ocl_svml_s9_fabs1 (double1_avx a);

  double1_avx __ocl_svml_s9_fmin1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_fmax1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_maxmag1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_minmag1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_nan1 (long1_avx a);

  double1_avx __ocl_svml_s9_ceil1 (double1_avx a);

  double1_avx __ocl_svml_s9_floor1 (double1_avx a);

  double1_avx __ocl_svml_s9_round1 (double1_avx a);

  double1_avx __ocl_svml_s9_trunc1 (double1_avx a);

  double1_avx __ocl_svml_s9_rint1 (double1_avx a);

  double1_avx __ocl_svml_s9_nearbyint1 (double1_avx a);

  double1_avx __ocl_svml_s9_fmod1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_modf1 (double1_avx a, double1_avx * c);

  double1_avx __ocl_svml_s9_remainder1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_remquo1 (double1_avx a, double1_avx b, int1_avx * c);

  double1_avx __ocl_svml_s9_copysign1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_frexp1 (double1_avx a, int1_avx * c);

  double1_avx __ocl_svml_s9_fdim1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_logb1 (double1_avx a);

  int1_avx __ocl_svml_s9_ilogb1 (double1_avx a);

  double1_avx __ocl_svml_s9_nextafter1 (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_ldexp1 (double1_avx a, int1_avx b);

  double1_avx __ocl_svml_s9_fract1 (double1_avx a, double1_avx * c);

  double1_avx __ocl_svml_s9_tgamma1 (double1_avx a);

  double1_avx __ocl_svml_s9_lgamma1 (double1_avx a);

  double1_avx __ocl_svml_s9_lgammar1 (double1_avx a, int1_avx * c);

  long1_avx __ocl_svml_s9_cvtfptou64rtenosat1 (double1_avx a);

  long1_avx __ocl_svml_s9_cvtfptou64rtesat1 (double1_avx a);

  long1_avx __ocl_svml_s9_cvtfptou64rtnnosat1 (double1_avx a);

  long1_avx __ocl_svml_s9_cvtfptou64rtnsat1 (double1_avx a);

  long1_avx __ocl_svml_s9_cvtfptou64rtpnosat1 (double1_avx a);

  long1_avx __ocl_svml_s9_cvtfptou64rtpsat1 (double1_avx a);

  long1_avx __ocl_svml_s9_cvtfptou64rtznosat1 (double1_avx a);

  long1_avx __ocl_svml_s9_cvtfptou64rtzsat1 (double1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtenosat1 (double1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtesat1 (double1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtnnosat1 (double1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtnsat1 (double1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtpnosat1 (double1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtpsat1 (double1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtznosat1 (double1_avx a);

  long1_avx __ocl_svml_s9_cvtfptoi64rtzsat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtenosat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtesat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtnnosat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtnsat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtpnosat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtpsat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtznosat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptou32rtzsat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtenosat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtesat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtnnosat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtnsat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtpnosat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtpsat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtznosat1 (double1_avx a);

  int1_avx __ocl_svml_s9_cvtfptoi32rtzsat1 (double1_avx a);

  double1_avx __ocl_svml_s9_cvtu64tofprte1 (long1_avx a);

  double1_avx __ocl_svml_s9_cvtu64tofprtn1 (long1_avx a);

  double1_avx __ocl_svml_s9_cvtu64tofprtp1 (long1_avx a);

  double1_avx __ocl_svml_s9_cvtu64tofprtz1 (long1_avx a);

  double1_avx __ocl_svml_s9_cvti64tofprte1 (long1_avx a);

  double1_avx __ocl_svml_s9_cvti64tofprtn1 (long1_avx a);

  double1_avx __ocl_svml_s9_cvti64tofprtp1 (long1_avx a);

  double1_avx __ocl_svml_s9_cvti64tofprtz1 (long1_avx a);

  double1_avx __ocl_svml_s9_cvtu32tofprte1 (int1_avx a);

  double1_avx __ocl_svml_s9_cvtu32tofprtn1 (int1_avx a);

  double1_avx __ocl_svml_s9_cvtu32tofprtp1 (int1_avx a);

  double1_avx __ocl_svml_s9_cvtu32tofprtz1 (int1_avx a);

  double1_avx __ocl_svml_s9_cvti32tofprte1 (int1_avx a);

  double1_avx __ocl_svml_s9_cvti32tofprtn1 (int1_avx a);

  double1_avx __ocl_svml_s9_cvti32tofprtp1 (int1_avx a);

  double1_avx __ocl_svml_s9_cvti32tofprtz1 (int1_avx a);

  float1_avx __ocl_svml_s9_cvtfp64tofp32rte1 (double1_avx a);

  float1_avx __ocl_svml_s9_cvtfp64tofp32rtn1 (double1_avx a);

  float1_avx __ocl_svml_s9_cvtfp64tofp32rtp1 (double1_avx a);

  float1_avx __ocl_svml_s9_cvtfp64tofp32rtz1 (double1_avx a);

  int1_sse __ocl_svml_n8_idiv1 (int1_sse a, int1_sse b);

  int1_sse __ocl_svml_n8_irem1 (int1_sse a, int1_sse b);

  int1x2_sse __ocl_svml_n8_idivrem1 (int1_sse a, int1_sse b);

  int1_avx __ocl_svml_g9_idiv1 (int1_avx a, int1_avx b);

  int1_avx __ocl_svml_g9_irem1 (int1_avx a, int1_avx b);

  int1x2_avx __ocl_svml_g9_idivrem1 (int1_avx a, int1_avx b);

  int1_avx __ocl_svml_s9_idiv1 (int1_avx a, int1_avx b);

  int1_avx __ocl_svml_s9_irem1 (int1_avx a, int1_avx b);

  int1x2_avx __ocl_svml_s9_idivrem1 (int1_avx a, int1_avx b);

  int1_sse __ocl_svml_n8_udiv1 (int1_sse a, int1_sse b);

  int1_sse __ocl_svml_n8_urem1 (int1_sse a, int1_sse b);

  int1x2_sse __ocl_svml_n8_udivrem1 (int1_sse a, int1_sse b);

  int1_avx __ocl_svml_g9_udiv1 (int1_avx a, int1_avx b);

  int1_avx __ocl_svml_g9_urem1 (int1_avx a, int1_avx b);

  int1x2_avx __ocl_svml_g9_udivrem1 (int1_avx a, int1_avx b);

  int1_avx __ocl_svml_s9_udiv1 (int1_avx a, int1_avx b);

  int1_avx __ocl_svml_s9_urem1 (int1_avx a, int1_avx b);

  int1x2_avx __ocl_svml_s9_udivrem1 (int1_avx a, int1_avx b);

  float2_sse __ocl_svml_n8_invf2 (float2_sse a);

  float2_sse __ocl_svml_n8_divf2 (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_sqrtf2 (float2_sse a);

  float2_sse __ocl_svml_n8_rsqrtf2 (float2_sse a);

  float2_sse __ocl_svml_n8_cbrtf2 (float2_sse a);

  float2_sse __ocl_svml_n8_rcbrtf2 (float2_sse a);

  float2_sse __ocl_svml_n8_hypotf2 (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_powf2 (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_powrf2 (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_pownf2 (float2_sse a, int2_sse b);

  float2_sse __ocl_svml_n8_rootnf2 (float2_sse a, int2_sse b);

  float2_sse __ocl_svml_n8_expf2 (float2_sse a);

  float2_sse __ocl_svml_n8_exp2f2 (float2_sse a);

  float2_sse __ocl_svml_n8_exp10f2 (float2_sse a);

  float2_sse __ocl_svml_n8_expm1f2 (float2_sse a);

  float2_sse __ocl_svml_n8_logf2 (float2_sse a);

  float2_sse __ocl_svml_n8_log10f2 (float2_sse a);

  float2_sse __ocl_svml_n8_log2f2 (float2_sse a);

  float2_sse __ocl_svml_n8_log1pf2 (float2_sse a);

  float2_sse __ocl_svml_n8_sinf2 (float2_sse a);

  float2_sse __ocl_svml_n8_cosf2 (float2_sse a);

  float2_sse __ocl_svml_n8_sincosf2 (float2_sse a, float2_sse * c);

  float2x2_sse __ocl_svml_n8_sincosregf2 (float2_sse a);

  float2_sse __ocl_svml_n8_tanf2 (float2_sse a);

  float2_sse __ocl_svml_n8_sinpif2 (float2_sse a);

  float2_sse __ocl_svml_n8_cospif2 (float2_sse a);

  float2_sse __ocl_svml_n8_tanpif2 (float2_sse a);

  float2_sse __ocl_svml_n8_acosf2 (float2_sse a);

  float2_sse __ocl_svml_n8_asinf2 (float2_sse a);

  float2_sse __ocl_svml_n8_atanf2 (float2_sse a);

  float2_sse __ocl_svml_n8_atan2f2 (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_asinpif2 (float2_sse a);

  float2_sse __ocl_svml_n8_acospif2 (float2_sse a);

  float2_sse __ocl_svml_n8_atanpif2 (float2_sse a);

  float2_sse __ocl_svml_n8_atan2pif2 (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_sinhf2 (float2_sse a);

  float2_sse __ocl_svml_n8_coshf2 (float2_sse a);

  float2_sse __ocl_svml_n8_tanhf2 (float2_sse a);

  float2_sse __ocl_svml_n8_asinhf2 (float2_sse a);

  float2_sse __ocl_svml_n8_acoshf2 (float2_sse a);

  float2_sse __ocl_svml_n8_atanhf2 (float2_sse a);

  float2_sse __ocl_svml_n8_erff2 (float2_sse a);

  float2_sse __ocl_svml_n8_erfcf2 (float2_sse a);

  float2_avx __ocl_svml_g9_invf2 (float2_avx a);

  float2_avx __ocl_svml_g9_divf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_sqrtf2 (float2_avx a);

  float2_avx __ocl_svml_g9_rsqrtf2 (float2_avx a);

  float2_avx __ocl_svml_g9_cbrtf2 (float2_avx a);

  float2_avx __ocl_svml_g9_rcbrtf2 (float2_avx a);

  float2_avx __ocl_svml_g9_hypotf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_powf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_powrf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_pownf2 (float2_avx a, int2_avx b);

  float2_avx __ocl_svml_g9_rootnf2 (float2_avx a, int2_avx b);

  float2_avx __ocl_svml_g9_expf2 (float2_avx a);

  float2_avx __ocl_svml_g9_exp2f2 (float2_avx a);

  float2_avx __ocl_svml_g9_exp10f2 (float2_avx a);

  float2_avx __ocl_svml_g9_expm1f2 (float2_avx a);

  float2_avx __ocl_svml_g9_logf2 (float2_avx a);

  float2_avx __ocl_svml_g9_log10f2 (float2_avx a);

  float2_avx __ocl_svml_g9_log2f2 (float2_avx a);

  float2_avx __ocl_svml_g9_log1pf2 (float2_avx a);

  float2_avx __ocl_svml_g9_sinf2 (float2_avx a);

  float2_avx __ocl_svml_g9_cosf2 (float2_avx a);

  float2_avx __ocl_svml_g9_sincosf2 (float2_avx a, float2_avx * c);

  float2x2_avx __ocl_svml_g9_sincosregf2 (float2_avx a);

  float2_avx __ocl_svml_g9_tanf2 (float2_avx a);

  float2_avx __ocl_svml_g9_sinpif2 (float2_avx a);

  float2_avx __ocl_svml_g9_cospif2 (float2_avx a);

  float2_avx __ocl_svml_g9_tanpif2 (float2_avx a);

  float2_avx __ocl_svml_g9_acosf2 (float2_avx a);

  float2_avx __ocl_svml_g9_asinf2 (float2_avx a);

  float2_avx __ocl_svml_g9_atanf2 (float2_avx a);

  float2_avx __ocl_svml_g9_atan2f2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_asinpif2 (float2_avx a);

  float2_avx __ocl_svml_g9_acospif2 (float2_avx a);

  float2_avx __ocl_svml_g9_atanpif2 (float2_avx a);

  float2_avx __ocl_svml_g9_atan2pif2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_sinhf2 (float2_avx a);

  float2_avx __ocl_svml_g9_coshf2 (float2_avx a);

  float2_avx __ocl_svml_g9_tanhf2 (float2_avx a);

  float2_avx __ocl_svml_g9_asinhf2 (float2_avx a);

  float2_avx __ocl_svml_g9_acoshf2 (float2_avx a);

  float2_avx __ocl_svml_g9_atanhf2 (float2_avx a);

  float2_avx __ocl_svml_g9_erff2 (float2_avx a);

  float2_avx __ocl_svml_g9_erfcf2 (float2_avx a);

  float2_avx __ocl_svml_s9_invf2 (float2_avx a);

  float2_avx __ocl_svml_s9_divf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_sqrtf2 (float2_avx a);

  float2_avx __ocl_svml_s9_rsqrtf2 (float2_avx a);

  float2_avx __ocl_svml_s9_cbrtf2 (float2_avx a);

  float2_avx __ocl_svml_s9_rcbrtf2 (float2_avx a);

  float2_avx __ocl_svml_s9_hypotf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_powf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_powrf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_pownf2 (float2_avx a, int2_avx b);

  float2_avx __ocl_svml_s9_rootnf2 (float2_avx a, int2_avx b);

  float2_avx __ocl_svml_s9_expf2 (float2_avx a);

  float2_avx __ocl_svml_s9_exp2f2 (float2_avx a);

  float2_avx __ocl_svml_s9_exp10f2 (float2_avx a);

  float2_avx __ocl_svml_s9_expm1f2 (float2_avx a);

  float2_avx __ocl_svml_s9_logf2 (float2_avx a);

  float2_avx __ocl_svml_s9_log10f2 (float2_avx a);

  float2_avx __ocl_svml_s9_log2f2 (float2_avx a);

  float2_avx __ocl_svml_s9_log1pf2 (float2_avx a);

  float2_avx __ocl_svml_s9_sinf2 (float2_avx a);

  float2_avx __ocl_svml_s9_cosf2 (float2_avx a);

  float2_avx __ocl_svml_s9_sincosf2 (float2_avx a, float2_avx * c);

  float2x2_avx __ocl_svml_s9_sincosregf2 (float2_avx a);

  float2_avx __ocl_svml_s9_tanf2 (float2_avx a);

  float2_avx __ocl_svml_s9_sinpif2 (float2_avx a);

  float2_avx __ocl_svml_s9_cospif2 (float2_avx a);

  float2_avx __ocl_svml_s9_tanpif2 (float2_avx a);

  float2_avx __ocl_svml_s9_acosf2 (float2_avx a);

  float2_avx __ocl_svml_s9_asinf2 (float2_avx a);

  float2_avx __ocl_svml_s9_atanf2 (float2_avx a);

  float2_avx __ocl_svml_s9_atan2f2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_asinpif2 (float2_avx a);

  float2_avx __ocl_svml_s9_acospif2 (float2_avx a);

  float2_avx __ocl_svml_s9_atanpif2 (float2_avx a);

  float2_avx __ocl_svml_s9_atan2pif2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_sinhf2 (float2_avx a);

  float2_avx __ocl_svml_s9_coshf2 (float2_avx a);

  float2_avx __ocl_svml_s9_tanhf2 (float2_avx a);

  float2_avx __ocl_svml_s9_asinhf2 (float2_avx a);

  float2_avx __ocl_svml_s9_acoshf2 (float2_avx a);

  float2_avx __ocl_svml_s9_atanhf2 (float2_avx a);

  float2_avx __ocl_svml_s9_erff2 (float2_avx a);

  float2_avx __ocl_svml_s9_erfcf2 (float2_avx a);

  float2_sse __ocl_svml_n8_fmaf2 (float2_sse a, float2_sse b, float2_sse c);

  float2_sse __ocl_svml_n8_fabsf2 (float2_sse a);

  float2_sse __ocl_svml_n8_fminf2 (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_fmaxf2 (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_maxmagf2 (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_minmagf2 (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_nanf2 (int2_sse a);

  float2_sse __ocl_svml_n8_ceilf2 (float2_sse a);

  float2_sse __ocl_svml_n8_floorf2 (float2_sse a);

  float2_sse __ocl_svml_n8_roundf2 (float2_sse a);

  float2_sse __ocl_svml_n8_truncf2 (float2_sse a);

  float2_sse __ocl_svml_n8_rintf2 (float2_sse a);

  float2_sse __ocl_svml_n8_nearbyintf2 (float2_sse a);

  float2_sse __ocl_svml_n8_fmodf2 (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_modff2 (float2_sse a, float2_sse * c);

  float2_sse __ocl_svml_n8_remainderf2 (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_remquof2 (float2_sse a, float2_sse b, int2_sse * c);

  float2_sse __ocl_svml_n8_copysignf2 (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_frexpf2 (float2_sse a, int2_sse * c);

  float2_sse __ocl_svml_n8_fdimf2 (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_logbf2 (float2_sse a);

  int2_sse __ocl_svml_n8_ilogbf2 (float2_sse a);

  float2_sse __ocl_svml_n8_nextafterf2 (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_ldexpf2 (float2_sse a, int2_sse b);

  float2_sse __ocl_svml_n8_fractf2 (float2_sse a, float2_sse * c);

  float2_sse __ocl_svml_n8_tgammaf2 (float2_sse a);

  float2_sse __ocl_svml_n8_lgammaf2 (float2_sse a);

  float2_sse __ocl_svml_n8_lgammarf2 (float2_sse a, int2_sse * c);

  long2_sse __ocl_svml_n8_cvtfptou64rtenosatf2 (float2_sse a);

  long2_sse __ocl_svml_n8_cvtfptou64rtesatf2 (float2_sse a);

  long2_sse __ocl_svml_n8_cvtfptou64rtnnosatf2 (float2_sse a);

  long2_sse __ocl_svml_n8_cvtfptou64rtnsatf2 (float2_sse a);

  long2_sse __ocl_svml_n8_cvtfptou64rtpnosatf2 (float2_sse a);

  long2_sse __ocl_svml_n8_cvtfptou64rtpsatf2 (float2_sse a);

  long2_sse __ocl_svml_n8_cvtfptou64rtznosatf2 (float2_sse a);

  long2_sse __ocl_svml_n8_cvtfptou64rtzsatf2 (float2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtenosatf2 (float2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtesatf2 (float2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtnnosatf2 (float2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtnsatf2 (float2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtpnosatf2 (float2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtpsatf2 (float2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtznosatf2 (float2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtzsatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtenosatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtesatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtnnosatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtnsatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtpnosatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtpsatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtznosatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtzsatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtenosatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtesatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtnnosatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtnsatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtpnosatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtpsatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtznosatf2 (float2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtzsatf2 (float2_sse a);

  float2_sse __ocl_svml_n8_cvtu64tofprtef2 (long2_sse a);

  float2_sse __ocl_svml_n8_cvtu64tofprtnf2 (long2_sse a);

  float2_sse __ocl_svml_n8_cvtu64tofprtpf2 (long2_sse a);

  float2_sse __ocl_svml_n8_cvtu64tofprtzf2 (long2_sse a);

  float2_sse __ocl_svml_n8_cvti64tofprtef2 (long2_sse a);

  float2_sse __ocl_svml_n8_cvti64tofprtnf2 (long2_sse a);

  float2_sse __ocl_svml_n8_cvti64tofprtpf2 (long2_sse a);

  float2_sse __ocl_svml_n8_cvti64tofprtzf2 (long2_sse a);

  float2_sse __ocl_svml_n8_cvtu32tofprtef2 (int2_sse a);

  float2_sse __ocl_svml_n8_cvtu32tofprtnf2 (int2_sse a);

  float2_sse __ocl_svml_n8_cvtu32tofprtpf2 (int2_sse a);

  float2_sse __ocl_svml_n8_cvtu32tofprtzf2 (int2_sse a);

  float2_sse __ocl_svml_n8_cvti32tofprtef2 (int2_sse a);

  float2_sse __ocl_svml_n8_cvti32tofprtnf2 (int2_sse a);

  float2_sse __ocl_svml_n8_cvti32tofprtpf2 (int2_sse a);

  float2_sse __ocl_svml_n8_cvti32tofprtzf2 (int2_sse a);

  float2_avx __ocl_svml_g9_fmaf2 (float2_avx a, float2_avx b, float2_avx c);

  float2_avx __ocl_svml_g9_fabsf2 (float2_avx a);

  float2_avx __ocl_svml_g9_fminf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_fmaxf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_maxmagf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_minmagf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_nanf2 (int2_avx a);

  float2_avx __ocl_svml_g9_ceilf2 (float2_avx a);

  float2_avx __ocl_svml_g9_floorf2 (float2_avx a);

  float2_avx __ocl_svml_g9_roundf2 (float2_avx a);

  float2_avx __ocl_svml_g9_truncf2 (float2_avx a);

  float2_avx __ocl_svml_g9_rintf2 (float2_avx a);

  float2_avx __ocl_svml_g9_nearbyintf2 (float2_avx a);

  float2_avx __ocl_svml_g9_fmodf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_modff2 (float2_avx a, float2_avx * c);

  float2_avx __ocl_svml_g9_remainderf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_remquof2 (float2_avx a, float2_avx b, int2_avx * c);

  float2_avx __ocl_svml_g9_copysignf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_frexpf2 (float2_avx a, int2_avx * c);

  float2_avx __ocl_svml_g9_fdimf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_logbf2 (float2_avx a);

  int2_avx __ocl_svml_g9_ilogbf2 (float2_avx a);

  float2_avx __ocl_svml_g9_nextafterf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_ldexpf2 (float2_avx a, int2_avx b);

  float2_avx __ocl_svml_g9_fractf2 (float2_avx a, float2_avx * c);

  float2_avx __ocl_svml_g9_tgammaf2 (float2_avx a);

  float2_avx __ocl_svml_g9_lgammaf2 (float2_avx a);

  float2_avx __ocl_svml_g9_lgammarf2 (float2_avx a, int2_avx * c);

  long2_avx __ocl_svml_g9_cvtfptou64rtenosatf2 (float2_avx a);

  long2_avx __ocl_svml_g9_cvtfptou64rtesatf2 (float2_avx a);

  long2_avx __ocl_svml_g9_cvtfptou64rtnnosatf2 (float2_avx a);

  long2_avx __ocl_svml_g9_cvtfptou64rtnsatf2 (float2_avx a);

  long2_avx __ocl_svml_g9_cvtfptou64rtpnosatf2 (float2_avx a);

  long2_avx __ocl_svml_g9_cvtfptou64rtpsatf2 (float2_avx a);

  long2_avx __ocl_svml_g9_cvtfptou64rtznosatf2 (float2_avx a);

  long2_avx __ocl_svml_g9_cvtfptou64rtzsatf2 (float2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtenosatf2 (float2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtesatf2 (float2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtnnosatf2 (float2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtnsatf2 (float2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtpnosatf2 (float2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtpsatf2 (float2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtznosatf2 (float2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtzsatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtenosatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtesatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtnnosatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtnsatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtpnosatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtpsatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtznosatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtzsatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtenosatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtesatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtnnosatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtnsatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtpnosatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtpsatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtznosatf2 (float2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtzsatf2 (float2_avx a);

  float2_avx __ocl_svml_g9_cvtu64tofprtef2 (long2_avx a);

  float2_avx __ocl_svml_g9_cvtu64tofprtnf2 (long2_avx a);

  float2_avx __ocl_svml_g9_cvtu64tofprtpf2 (long2_avx a);

  float2_avx __ocl_svml_g9_cvtu64tofprtzf2 (long2_avx a);

  float2_avx __ocl_svml_g9_cvti64tofprtef2 (long2_avx a);

  float2_avx __ocl_svml_g9_cvti64tofprtnf2 (long2_avx a);

  float2_avx __ocl_svml_g9_cvti64tofprtpf2 (long2_avx a);

  float2_avx __ocl_svml_g9_cvti64tofprtzf2 (long2_avx a);

  float2_avx __ocl_svml_g9_cvtu32tofprtef2 (int2_avx a);

  float2_avx __ocl_svml_g9_cvtu32tofprtnf2 (int2_avx a);

  float2_avx __ocl_svml_g9_cvtu32tofprtpf2 (int2_avx a);

  float2_avx __ocl_svml_g9_cvtu32tofprtzf2 (int2_avx a);

  float2_avx __ocl_svml_g9_cvti32tofprtef2 (int2_avx a);

  float2_avx __ocl_svml_g9_cvti32tofprtnf2 (int2_avx a);

  float2_avx __ocl_svml_g9_cvti32tofprtpf2 (int2_avx a);

  float2_avx __ocl_svml_g9_cvti32tofprtzf2 (int2_avx a);

  float2_avx __ocl_svml_s9_fmaf2 (float2_avx a, float2_avx b, float2_avx c);

  float2_avx __ocl_svml_s9_fabsf2 (float2_avx a);

  float2_avx __ocl_svml_s9_fminf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_fmaxf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_maxmagf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_minmagf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_nanf2 (int2_avx a);

  float2_avx __ocl_svml_s9_ceilf2 (float2_avx a);

  float2_avx __ocl_svml_s9_floorf2 (float2_avx a);

  float2_avx __ocl_svml_s9_roundf2 (float2_avx a);

  float2_avx __ocl_svml_s9_truncf2 (float2_avx a);

  float2_avx __ocl_svml_s9_rintf2 (float2_avx a);

  float2_avx __ocl_svml_s9_nearbyintf2 (float2_avx a);

  float2_avx __ocl_svml_s9_fmodf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_modff2 (float2_avx a, float2_avx * c);

  float2_avx __ocl_svml_s9_remainderf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_remquof2 (float2_avx a, float2_avx b, int2_avx * c);

  float2_avx __ocl_svml_s9_copysignf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_frexpf2 (float2_avx a, int2_avx * c);

  float2_avx __ocl_svml_s9_fdimf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_logbf2 (float2_avx a);

  int2_avx __ocl_svml_s9_ilogbf2 (float2_avx a);

  float2_avx __ocl_svml_s9_nextafterf2 (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_ldexpf2 (float2_avx a, int2_avx b);

  float2_avx __ocl_svml_s9_fractf2 (float2_avx a, float2_avx * c);

  float2_avx __ocl_svml_s9_tgammaf2 (float2_avx a);

  float2_avx __ocl_svml_s9_lgammaf2 (float2_avx a);

  float2_avx __ocl_svml_s9_lgammarf2 (float2_avx a, int2_avx * c);

  long2_avx __ocl_svml_s9_cvtfptou64rtenosatf2 (float2_avx a);

  long2_avx __ocl_svml_s9_cvtfptou64rtesatf2 (float2_avx a);

  long2_avx __ocl_svml_s9_cvtfptou64rtnnosatf2 (float2_avx a);

  long2_avx __ocl_svml_s9_cvtfptou64rtnsatf2 (float2_avx a);

  long2_avx __ocl_svml_s9_cvtfptou64rtpnosatf2 (float2_avx a);

  long2_avx __ocl_svml_s9_cvtfptou64rtpsatf2 (float2_avx a);

  long2_avx __ocl_svml_s9_cvtfptou64rtznosatf2 (float2_avx a);

  long2_avx __ocl_svml_s9_cvtfptou64rtzsatf2 (float2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtenosatf2 (float2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtesatf2 (float2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtnnosatf2 (float2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtnsatf2 (float2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtpnosatf2 (float2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtpsatf2 (float2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtznosatf2 (float2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtzsatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtenosatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtesatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtnnosatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtnsatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtpnosatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtpsatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtznosatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtzsatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtenosatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtesatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtnnosatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtnsatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtpnosatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtpsatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtznosatf2 (float2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtzsatf2 (float2_avx a);

  float2_avx __ocl_svml_s9_cvtu64tofprtef2 (long2_avx a);

  float2_avx __ocl_svml_s9_cvtu64tofprtnf2 (long2_avx a);

  float2_avx __ocl_svml_s9_cvtu64tofprtpf2 (long2_avx a);

  float2_avx __ocl_svml_s9_cvtu64tofprtzf2 (long2_avx a);

  float2_avx __ocl_svml_s9_cvti64tofprtef2 (long2_avx a);

  float2_avx __ocl_svml_s9_cvti64tofprtnf2 (long2_avx a);

  float2_avx __ocl_svml_s9_cvti64tofprtpf2 (long2_avx a);

  float2_avx __ocl_svml_s9_cvti64tofprtzf2 (long2_avx a);

  float2_avx __ocl_svml_s9_cvtu32tofprtef2 (int2_avx a);

  float2_avx __ocl_svml_s9_cvtu32tofprtnf2 (int2_avx a);

  float2_avx __ocl_svml_s9_cvtu32tofprtpf2 (int2_avx a);

  float2_avx __ocl_svml_s9_cvtu32tofprtzf2 (int2_avx a);

  float2_avx __ocl_svml_s9_cvti32tofprtef2 (int2_avx a);

  float2_avx __ocl_svml_s9_cvti32tofprtnf2 (int2_avx a);

  float2_avx __ocl_svml_s9_cvti32tofprtpf2 (int2_avx a);

  float2_avx __ocl_svml_s9_cvti32tofprtzf2 (int2_avx a);

  double2_sse __ocl_svml_n8_inv2 (double2_sse a);

  double2_sse __ocl_svml_n8_div2 (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_sqrt2 (double2_sse a);

  double2_sse __ocl_svml_n8_rsqrt2 (double2_sse a);

  double2_sse __ocl_svml_n8_cbrt2 (double2_sse a);

  double2_sse __ocl_svml_n8_rcbrt2 (double2_sse a);

  double2_sse __ocl_svml_n8_hypot2 (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_pow2 (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_powr2 (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_pown2 (double2_sse a, int2_sse b);

  double2_sse __ocl_svml_n8_rootn2 (double2_sse a, int2_sse b);

  double2_sse __ocl_svml_n8_exp2 (double2_sse a);

  double2_sse __ocl_svml_n8_exp22 (double2_sse a);

  double2_sse __ocl_svml_n8_exp102 (double2_sse a);

  double2_sse __ocl_svml_n8_expm12 (double2_sse a);

  double2_sse __ocl_svml_n8_log2 (double2_sse a);

  double2_sse __ocl_svml_n8_log102 (double2_sse a);

  double2_sse __ocl_svml_n8_log22 (double2_sse a);

  double2_sse __ocl_svml_n8_log1p2 (double2_sse a);

  double2_sse __ocl_svml_n8_sin2 (double2_sse a);

  double2_sse __ocl_svml_n8_cos2 (double2_sse a);

  double2_sse __ocl_svml_n8_sincos2 (double2_sse a, double2_sse * c);

  double2x2_sse __ocl_svml_n8_sincosreg2 (double2_sse a);

  double2_sse __ocl_svml_n8_tan2 (double2_sse a);

  double2_sse __ocl_svml_n8_sinpi2 (double2_sse a);

  double2_sse __ocl_svml_n8_cospi2 (double2_sse a);

  double2_sse __ocl_svml_n8_tanpi2 (double2_sse a);

  double2_sse __ocl_svml_n8_acos2 (double2_sse a);

  double2_sse __ocl_svml_n8_asin2 (double2_sse a);

  double2_sse __ocl_svml_n8_atan2 (double2_sse a);

  double2_sse __ocl_svml_n8_atan22 (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_asinpi2 (double2_sse a);

  double2_sse __ocl_svml_n8_acospi2 (double2_sse a);

  double2_sse __ocl_svml_n8_atanpi2 (double2_sse a);

  double2_sse __ocl_svml_n8_atan2pi2 (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_sinh2 (double2_sse a);

  double2_sse __ocl_svml_n8_cosh2 (double2_sse a);

  double2_sse __ocl_svml_n8_tanh2 (double2_sse a);

  double2_sse __ocl_svml_n8_asinh2 (double2_sse a);

  double2_sse __ocl_svml_n8_acosh2 (double2_sse a);

  double2_sse __ocl_svml_n8_atanh2 (double2_sse a);

  double2_sse __ocl_svml_n8_erf2 (double2_sse a);

  double2_sse __ocl_svml_n8_erfc2 (double2_sse a);

  double2_avx __ocl_svml_g9_inv2 (double2_avx a);

  double2_avx __ocl_svml_g9_div2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_sqrt2 (double2_avx a);

  double2_avx __ocl_svml_g9_rsqrt2 (double2_avx a);

  double2_avx __ocl_svml_g9_cbrt2 (double2_avx a);

  double2_avx __ocl_svml_g9_rcbrt2 (double2_avx a);

  double2_avx __ocl_svml_g9_hypot2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_pow2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_powr2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_pown2 (double2_avx a, int2_avx b);

  double2_avx __ocl_svml_g9_rootn2 (double2_avx a, int2_avx b);

  double2_avx __ocl_svml_g9_exp2 (double2_avx a);

  double2_avx __ocl_svml_g9_exp22 (double2_avx a);

  double2_avx __ocl_svml_g9_exp102 (double2_avx a);

  double2_avx __ocl_svml_g9_expm12 (double2_avx a);

  double2_avx __ocl_svml_g9_log2 (double2_avx a);

  double2_avx __ocl_svml_g9_log102 (double2_avx a);

  double2_avx __ocl_svml_g9_log22 (double2_avx a);

  double2_avx __ocl_svml_g9_log1p2 (double2_avx a);

  double2_avx __ocl_svml_g9_sin2 (double2_avx a);

  double2_avx __ocl_svml_g9_cos2 (double2_avx a);

  double2_avx __ocl_svml_g9_sincos2 (double2_avx a, double2_avx * c);

  double2x2_avx __ocl_svml_g9_sincosreg2 (double2_avx a);

  double2_avx __ocl_svml_g9_tan2 (double2_avx a);

  double2_avx __ocl_svml_g9_sinpi2 (double2_avx a);

  double2_avx __ocl_svml_g9_cospi2 (double2_avx a);

  double2_avx __ocl_svml_g9_tanpi2 (double2_avx a);

  double2_avx __ocl_svml_g9_acos2 (double2_avx a);

  double2_avx __ocl_svml_g9_asin2 (double2_avx a);

  double2_avx __ocl_svml_g9_atan2 (double2_avx a);

  double2_avx __ocl_svml_g9_atan22 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_asinpi2 (double2_avx a);

  double2_avx __ocl_svml_g9_acospi2 (double2_avx a);

  double2_avx __ocl_svml_g9_atanpi2 (double2_avx a);

  double2_avx __ocl_svml_g9_atan2pi2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_sinh2 (double2_avx a);

  double2_avx __ocl_svml_g9_cosh2 (double2_avx a);

  double2_avx __ocl_svml_g9_tanh2 (double2_avx a);

  double2_avx __ocl_svml_g9_asinh2 (double2_avx a);

  double2_avx __ocl_svml_g9_acosh2 (double2_avx a);

  double2_avx __ocl_svml_g9_atanh2 (double2_avx a);

  double2_avx __ocl_svml_g9_erf2 (double2_avx a);

  double2_avx __ocl_svml_g9_erfc2 (double2_avx a);

  double2_avx __ocl_svml_s9_inv2 (double2_avx a);

  double2_avx __ocl_svml_s9_div2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_sqrt2 (double2_avx a);

  double2_avx __ocl_svml_s9_rsqrt2 (double2_avx a);

  double2_avx __ocl_svml_s9_cbrt2 (double2_avx a);

  double2_avx __ocl_svml_s9_rcbrt2 (double2_avx a);

  double2_avx __ocl_svml_s9_hypot2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_pow2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_powr2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_pown2 (double2_avx a, int2_avx b);

  double2_avx __ocl_svml_s9_rootn2 (double2_avx a, int2_avx b);

  double2_avx __ocl_svml_s9_exp2 (double2_avx a);

  double2_avx __ocl_svml_s9_exp22 (double2_avx a);

  double2_avx __ocl_svml_s9_exp102 (double2_avx a);

  double2_avx __ocl_svml_s9_expm12 (double2_avx a);

  double2_avx __ocl_svml_s9_log2 (double2_avx a);

  double2_avx __ocl_svml_s9_log102 (double2_avx a);

  double2_avx __ocl_svml_s9_log22 (double2_avx a);

  double2_avx __ocl_svml_s9_log1p2 (double2_avx a);

  double2_avx __ocl_svml_s9_sin2 (double2_avx a);

  double2_avx __ocl_svml_s9_cos2 (double2_avx a);

  double2_avx __ocl_svml_s9_sincos2 (double2_avx a, double2_avx * c);

  double2x2_avx __ocl_svml_s9_sincosreg2 (double2_avx a);

  double2_avx __ocl_svml_s9_tan2 (double2_avx a);

  double2_avx __ocl_svml_s9_sinpi2 (double2_avx a);

  double2_avx __ocl_svml_s9_cospi2 (double2_avx a);

  double2_avx __ocl_svml_s9_tanpi2 (double2_avx a);

  double2_avx __ocl_svml_s9_acos2 (double2_avx a);

  double2_avx __ocl_svml_s9_asin2 (double2_avx a);

  double2_avx __ocl_svml_s9_atan2 (double2_avx a);

  double2_avx __ocl_svml_s9_atan22 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_asinpi2 (double2_avx a);

  double2_avx __ocl_svml_s9_acospi2 (double2_avx a);

  double2_avx __ocl_svml_s9_atanpi2 (double2_avx a);

  double2_avx __ocl_svml_s9_atan2pi2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_sinh2 (double2_avx a);

  double2_avx __ocl_svml_s9_cosh2 (double2_avx a);

  double2_avx __ocl_svml_s9_tanh2 (double2_avx a);

  double2_avx __ocl_svml_s9_asinh2 (double2_avx a);

  double2_avx __ocl_svml_s9_acosh2 (double2_avx a);

  double2_avx __ocl_svml_s9_atanh2 (double2_avx a);

  double2_avx __ocl_svml_s9_erf2 (double2_avx a);

  double2_avx __ocl_svml_s9_erfc2 (double2_avx a);

  double2_sse __ocl_svml_n8_fma2 (double2_sse a, double2_sse b, double2_sse c);

  double2_sse __ocl_svml_n8_fabs2 (double2_sse a);

  double2_sse __ocl_svml_n8_fmin2 (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_fmax2 (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_maxmag2 (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_minmag2 (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_nan2 (long2_sse a);

  double2_sse __ocl_svml_n8_ceil2 (double2_sse a);

  double2_sse __ocl_svml_n8_floor2 (double2_sse a);

  double2_sse __ocl_svml_n8_round2 (double2_sse a);

  double2_sse __ocl_svml_n8_trunc2 (double2_sse a);

  double2_sse __ocl_svml_n8_rint2 (double2_sse a);

  double2_sse __ocl_svml_n8_nearbyint2 (double2_sse a);

  double2_sse __ocl_svml_n8_fmod2 (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_modf2 (double2_sse a, double2_sse * c);

  double2_sse __ocl_svml_n8_remainder2 (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_remquo2 (double2_sse a, double2_sse b, int2_sse * c);

  double2_sse __ocl_svml_n8_copysign2 (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_frexp2 (double2_sse a, int2_sse * c);

  double2_sse __ocl_svml_n8_fdim2 (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_logb2 (double2_sse a);

  int2_sse __ocl_svml_n8_ilogb2 (double2_sse a);

  double2_sse __ocl_svml_n8_nextafter2 (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_ldexp2 (double2_sse a, int2_sse b);

  double2_sse __ocl_svml_n8_fract2 (double2_sse a, double2_sse * c);

  double2_sse __ocl_svml_n8_tgamma2 (double2_sse a);

  double2_sse __ocl_svml_n8_lgamma2 (double2_sse a);

  double2_sse __ocl_svml_n8_lgammar2 (double2_sse a, int2_sse * c);

  long2_sse __ocl_svml_n8_cvtfptou64rtenosat2 (double2_sse a);

  long2_sse __ocl_svml_n8_cvtfptou64rtesat2 (double2_sse a);

  long2_sse __ocl_svml_n8_cvtfptou64rtnnosat2 (double2_sse a);

  long2_sse __ocl_svml_n8_cvtfptou64rtnsat2 (double2_sse a);

  long2_sse __ocl_svml_n8_cvtfptou64rtpnosat2 (double2_sse a);

  long2_sse __ocl_svml_n8_cvtfptou64rtpsat2 (double2_sse a);

  long2_sse __ocl_svml_n8_cvtfptou64rtznosat2 (double2_sse a);

  long2_sse __ocl_svml_n8_cvtfptou64rtzsat2 (double2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtenosat2 (double2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtesat2 (double2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtnnosat2 (double2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtnsat2 (double2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtpnosat2 (double2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtpsat2 (double2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtznosat2 (double2_sse a);

  long2_sse __ocl_svml_n8_cvtfptoi64rtzsat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtenosat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtesat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtnnosat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtnsat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtpnosat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtpsat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtznosat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptou32rtzsat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtenosat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtesat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtnnosat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtnsat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtpnosat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtpsat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtznosat2 (double2_sse a);

  int2_sse __ocl_svml_n8_cvtfptoi32rtzsat2 (double2_sse a);

  double2_sse __ocl_svml_n8_cvtu64tofprte2 (long2_sse a);

  double2_sse __ocl_svml_n8_cvtu64tofprtn2 (long2_sse a);

  double2_sse __ocl_svml_n8_cvtu64tofprtp2 (long2_sse a);

  double2_sse __ocl_svml_n8_cvtu64tofprtz2 (long2_sse a);

  double2_sse __ocl_svml_n8_cvti64tofprte2 (long2_sse a);

  double2_sse __ocl_svml_n8_cvti64tofprtn2 (long2_sse a);

  double2_sse __ocl_svml_n8_cvti64tofprtp2 (long2_sse a);

  double2_sse __ocl_svml_n8_cvti64tofprtz2 (long2_sse a);

  double2_sse __ocl_svml_n8_cvtu32tofprte2 (int2_sse a);

  double2_sse __ocl_svml_n8_cvtu32tofprtn2 (int2_sse a);

  double2_sse __ocl_svml_n8_cvtu32tofprtp2 (int2_sse a);

  double2_sse __ocl_svml_n8_cvtu32tofprtz2 (int2_sse a);

  double2_sse __ocl_svml_n8_cvti32tofprte2 (int2_sse a);

  double2_sse __ocl_svml_n8_cvti32tofprtn2 (int2_sse a);

  double2_sse __ocl_svml_n8_cvti32tofprtp2 (int2_sse a);

  double2_sse __ocl_svml_n8_cvti32tofprtz2 (int2_sse a);

  float2_sse __ocl_svml_n8_cvtfp64tofp32rte2 (double2_sse a);

  float2_sse __ocl_svml_n8_cvtfp64tofp32rtn2 (double2_sse a);

  float2_sse __ocl_svml_n8_cvtfp64tofp32rtp2 (double2_sse a);

  float2_sse __ocl_svml_n8_cvtfp64tofp32rtz2 (double2_sse a);

  double2_avx __ocl_svml_g9_fma2 (double2_avx a, double2_avx b, double2_avx c);

  double2_avx __ocl_svml_g9_fabs2 (double2_avx a);

  double2_avx __ocl_svml_g9_fmin2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_fmax2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_maxmag2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_minmag2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_nan2 (long2_avx a);

  double2_avx __ocl_svml_g9_ceil2 (double2_avx a);

  double2_avx __ocl_svml_g9_floor2 (double2_avx a);

  double2_avx __ocl_svml_g9_round2 (double2_avx a);

  double2_avx __ocl_svml_g9_trunc2 (double2_avx a);

  double2_avx __ocl_svml_g9_rint2 (double2_avx a);

  double2_avx __ocl_svml_g9_nearbyint2 (double2_avx a);

  double2_avx __ocl_svml_g9_fmod2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_modf2 (double2_avx a, double2_avx * c);

  double2_avx __ocl_svml_g9_remainder2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_remquo2 (double2_avx a, double2_avx b, int2_avx * c);

  double2_avx __ocl_svml_g9_copysign2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_frexp2 (double2_avx a, int2_avx * c);

  double2_avx __ocl_svml_g9_fdim2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_logb2 (double2_avx a);

  int2_avx __ocl_svml_g9_ilogb2 (double2_avx a);

  double2_avx __ocl_svml_g9_nextafter2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_ldexp2 (double2_avx a, int2_avx b);

  double2_avx __ocl_svml_g9_fract2 (double2_avx a, double2_avx * c);

  double2_avx __ocl_svml_g9_tgamma2 (double2_avx a);

  double2_avx __ocl_svml_g9_lgamma2 (double2_avx a);

  double2_avx __ocl_svml_g9_lgammar2 (double2_avx a, int2_avx * c);

  long2_avx __ocl_svml_g9_cvtfptou64rtenosat2 (double2_avx a);

  long2_avx __ocl_svml_g9_cvtfptou64rtesat2 (double2_avx a);

  long2_avx __ocl_svml_g9_cvtfptou64rtnnosat2 (double2_avx a);

  long2_avx __ocl_svml_g9_cvtfptou64rtnsat2 (double2_avx a);

  long2_avx __ocl_svml_g9_cvtfptou64rtpnosat2 (double2_avx a);

  long2_avx __ocl_svml_g9_cvtfptou64rtpsat2 (double2_avx a);

  long2_avx __ocl_svml_g9_cvtfptou64rtznosat2 (double2_avx a);

  long2_avx __ocl_svml_g9_cvtfptou64rtzsat2 (double2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtenosat2 (double2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtesat2 (double2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtnnosat2 (double2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtnsat2 (double2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtpnosat2 (double2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtpsat2 (double2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtznosat2 (double2_avx a);

  long2_avx __ocl_svml_g9_cvtfptoi64rtzsat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtenosat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtesat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtnnosat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtnsat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtpnosat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtpsat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtznosat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptou32rtzsat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtenosat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtesat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtnnosat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtnsat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtpnosat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtpsat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtznosat2 (double2_avx a);

  int2_avx __ocl_svml_g9_cvtfptoi32rtzsat2 (double2_avx a);

  double2_avx __ocl_svml_g9_cvtu64tofprte2 (long2_avx a);

  double2_avx __ocl_svml_g9_cvtu64tofprtn2 (long2_avx a);

  double2_avx __ocl_svml_g9_cvtu64tofprtp2 (long2_avx a);

  double2_avx __ocl_svml_g9_cvtu64tofprtz2 (long2_avx a);

  double2_avx __ocl_svml_g9_cvti64tofprte2 (long2_avx a);

  double2_avx __ocl_svml_g9_cvti64tofprtn2 (long2_avx a);

  double2_avx __ocl_svml_g9_cvti64tofprtp2 (long2_avx a);

  double2_avx __ocl_svml_g9_cvti64tofprtz2 (long2_avx a);

  double2_avx __ocl_svml_g9_cvtu32tofprte2 (int2_avx a);

  double2_avx __ocl_svml_g9_cvtu32tofprtn2 (int2_avx a);

  double2_avx __ocl_svml_g9_cvtu32tofprtp2 (int2_avx a);

  double2_avx __ocl_svml_g9_cvtu32tofprtz2 (int2_avx a);

  double2_avx __ocl_svml_g9_cvti32tofprte2 (int2_avx a);

  double2_avx __ocl_svml_g9_cvti32tofprtn2 (int2_avx a);

  double2_avx __ocl_svml_g9_cvti32tofprtp2 (int2_avx a);

  double2_avx __ocl_svml_g9_cvti32tofprtz2 (int2_avx a);

  float2_avx __ocl_svml_g9_cvtfp64tofp32rte2 (double2_avx a);

  float2_avx __ocl_svml_g9_cvtfp64tofp32rtn2 (double2_avx a);

  float2_avx __ocl_svml_g9_cvtfp64tofp32rtp2 (double2_avx a);

  float2_avx __ocl_svml_g9_cvtfp64tofp32rtz2 (double2_avx a);

  double2_avx __ocl_svml_s9_fma2 (double2_avx a, double2_avx b, double2_avx c);

  double2_avx __ocl_svml_s9_fabs2 (double2_avx a);

  double2_avx __ocl_svml_s9_fmin2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_fmax2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_maxmag2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_minmag2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_nan2 (long2_avx a);

  double2_avx __ocl_svml_s9_ceil2 (double2_avx a);

  double2_avx __ocl_svml_s9_floor2 (double2_avx a);

  double2_avx __ocl_svml_s9_round2 (double2_avx a);

  double2_avx __ocl_svml_s9_trunc2 (double2_avx a);

  double2_avx __ocl_svml_s9_rint2 (double2_avx a);

  double2_avx __ocl_svml_s9_nearbyint2 (double2_avx a);

  double2_avx __ocl_svml_s9_fmod2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_modf2 (double2_avx a, double2_avx * c);

  double2_avx __ocl_svml_s9_remainder2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_remquo2 (double2_avx a, double2_avx b, int2_avx * c);

  double2_avx __ocl_svml_s9_copysign2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_frexp2 (double2_avx a, int2_avx * c);

  double2_avx __ocl_svml_s9_fdim2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_logb2 (double2_avx a);

  int2_avx __ocl_svml_s9_ilogb2 (double2_avx a);

  double2_avx __ocl_svml_s9_nextafter2 (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_ldexp2 (double2_avx a, int2_avx b);

  double2_avx __ocl_svml_s9_fract2 (double2_avx a, double2_avx * c);

  double2_avx __ocl_svml_s9_tgamma2 (double2_avx a);

  double2_avx __ocl_svml_s9_lgamma2 (double2_avx a);

  double2_avx __ocl_svml_s9_lgammar2 (double2_avx a, int2_avx * c);

  long2_avx __ocl_svml_s9_cvtfptou64rtenosat2 (double2_avx a);

  long2_avx __ocl_svml_s9_cvtfptou64rtesat2 (double2_avx a);

  long2_avx __ocl_svml_s9_cvtfptou64rtnnosat2 (double2_avx a);

  long2_avx __ocl_svml_s9_cvtfptou64rtnsat2 (double2_avx a);

  long2_avx __ocl_svml_s9_cvtfptou64rtpnosat2 (double2_avx a);

  long2_avx __ocl_svml_s9_cvtfptou64rtpsat2 (double2_avx a);

  long2_avx __ocl_svml_s9_cvtfptou64rtznosat2 (double2_avx a);

  long2_avx __ocl_svml_s9_cvtfptou64rtzsat2 (double2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtenosat2 (double2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtesat2 (double2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtnnosat2 (double2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtnsat2 (double2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtpnosat2 (double2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtpsat2 (double2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtznosat2 (double2_avx a);

  long2_avx __ocl_svml_s9_cvtfptoi64rtzsat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtenosat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtesat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtnnosat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtnsat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtpnosat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtpsat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtznosat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptou32rtzsat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtenosat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtesat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtnnosat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtnsat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtpnosat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtpsat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtznosat2 (double2_avx a);

  int2_avx __ocl_svml_s9_cvtfptoi32rtzsat2 (double2_avx a);

  double2_avx __ocl_svml_s9_cvtu64tofprte2 (long2_avx a);

  double2_avx __ocl_svml_s9_cvtu64tofprtn2 (long2_avx a);

  double2_avx __ocl_svml_s9_cvtu64tofprtp2 (long2_avx a);

  double2_avx __ocl_svml_s9_cvtu64tofprtz2 (long2_avx a);

  double2_avx __ocl_svml_s9_cvti64tofprte2 (long2_avx a);

  double2_avx __ocl_svml_s9_cvti64tofprtn2 (long2_avx a);

  double2_avx __ocl_svml_s9_cvti64tofprtp2 (long2_avx a);

  double2_avx __ocl_svml_s9_cvti64tofprtz2 (long2_avx a);

  double2_avx __ocl_svml_s9_cvtu32tofprte2 (int2_avx a);

  double2_avx __ocl_svml_s9_cvtu32tofprtn2 (int2_avx a);

  double2_avx __ocl_svml_s9_cvtu32tofprtp2 (int2_avx a);

  double2_avx __ocl_svml_s9_cvtu32tofprtz2 (int2_avx a);

  double2_avx __ocl_svml_s9_cvti32tofprte2 (int2_avx a);

  double2_avx __ocl_svml_s9_cvti32tofprtn2 (int2_avx a);

  double2_avx __ocl_svml_s9_cvti32tofprtp2 (int2_avx a);

  double2_avx __ocl_svml_s9_cvti32tofprtz2 (int2_avx a);

  float2_avx __ocl_svml_s9_cvtfp64tofp32rte2 (double2_avx a);

  float2_avx __ocl_svml_s9_cvtfp64tofp32rtn2 (double2_avx a);

  float2_avx __ocl_svml_s9_cvtfp64tofp32rtp2 (double2_avx a);

  float2_avx __ocl_svml_s9_cvtfp64tofp32rtz2 (double2_avx a);

  int2_sse __ocl_svml_n8_idiv2 (int2_sse a, int2_sse b);

  int2_sse __ocl_svml_n8_irem2 (int2_sse a, int2_sse b);

  int2x2_sse __ocl_svml_n8_idivrem2 (int2_sse a, int2_sse b);

  int2_avx __ocl_svml_g9_idiv2 (int2_avx a, int2_avx b);

  int2_avx __ocl_svml_g9_irem2 (int2_avx a, int2_avx b);

  int2x2_avx __ocl_svml_g9_idivrem2 (int2_avx a, int2_avx b);

  int2_avx __ocl_svml_s9_idiv2 (int2_avx a, int2_avx b);

  int2_avx __ocl_svml_s9_irem2 (int2_avx a, int2_avx b);

  int2x2_avx __ocl_svml_s9_idivrem2 (int2_avx a, int2_avx b);

  int2_sse __ocl_svml_n8_udiv2 (int2_sse a, int2_sse b);

  int2_sse __ocl_svml_n8_urem2 (int2_sse a, int2_sse b);

  int2x2_sse __ocl_svml_n8_udivrem2 (int2_sse a, int2_sse b);

  int2_avx __ocl_svml_g9_udiv2 (int2_avx a, int2_avx b);

  int2_avx __ocl_svml_g9_urem2 (int2_avx a, int2_avx b);

  int2x2_avx __ocl_svml_g9_udivrem2 (int2_avx a, int2_avx b);

  int2_avx __ocl_svml_s9_udiv2 (int2_avx a, int2_avx b);

  int2_avx __ocl_svml_s9_urem2 (int2_avx a, int2_avx b);

  int2x2_avx __ocl_svml_s9_udivrem2 (int2_avx a, int2_avx b);

  float3_sse __ocl_svml_n8_invf3 (float3_sse a);

  float3_sse __ocl_svml_n8_divf3 (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_sqrtf3 (float3_sse a);

  float3_sse __ocl_svml_n8_rsqrtf3 (float3_sse a);

  float3_sse __ocl_svml_n8_cbrtf3 (float3_sse a);

  float3_sse __ocl_svml_n8_rcbrtf3 (float3_sse a);

  float3_sse __ocl_svml_n8_hypotf3 (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_powf3 (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_powrf3 (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_pownf3 (float3_sse a, int3_sse b);

  float3_sse __ocl_svml_n8_rootnf3 (float3_sse a, int3_sse b);

  float3_sse __ocl_svml_n8_expf3 (float3_sse a);

  float3_sse __ocl_svml_n8_exp2f3 (float3_sse a);

  float3_sse __ocl_svml_n8_exp10f3 (float3_sse a);

  float3_sse __ocl_svml_n8_expm1f3 (float3_sse a);

  float3_sse __ocl_svml_n8_logf3 (float3_sse a);

  float3_sse __ocl_svml_n8_log10f3 (float3_sse a);

  float3_sse __ocl_svml_n8_log2f3 (float3_sse a);

  float3_sse __ocl_svml_n8_log1pf3 (float3_sse a);

  float3_sse __ocl_svml_n8_sinf3 (float3_sse a);

  float3_sse __ocl_svml_n8_cosf3 (float3_sse a);

  float3_sse __ocl_svml_n8_sincosf3 (float3_sse a, float3_sse * c);

  float3x2_sse __ocl_svml_n8_sincosregf3 (float3_sse a);

  float3_sse __ocl_svml_n8_tanf3 (float3_sse a);

  float3_sse __ocl_svml_n8_sinpif3 (float3_sse a);

  float3_sse __ocl_svml_n8_cospif3 (float3_sse a);

  float3_sse __ocl_svml_n8_tanpif3 (float3_sse a);

  float3_sse __ocl_svml_n8_acosf3 (float3_sse a);

  float3_sse __ocl_svml_n8_asinf3 (float3_sse a);

  float3_sse __ocl_svml_n8_atanf3 (float3_sse a);

  float3_sse __ocl_svml_n8_atan2f3 (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_asinpif3 (float3_sse a);

  float3_sse __ocl_svml_n8_acospif3 (float3_sse a);

  float3_sse __ocl_svml_n8_atanpif3 (float3_sse a);

  float3_sse __ocl_svml_n8_atan2pif3 (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_sinhf3 (float3_sse a);

  float3_sse __ocl_svml_n8_coshf3 (float3_sse a);

  float3_sse __ocl_svml_n8_tanhf3 (float3_sse a);

  float3_sse __ocl_svml_n8_asinhf3 (float3_sse a);

  float3_sse __ocl_svml_n8_acoshf3 (float3_sse a);

  float3_sse __ocl_svml_n8_atanhf3 (float3_sse a);

  float3_sse __ocl_svml_n8_erff3 (float3_sse a);

  float3_sse __ocl_svml_n8_erfcf3 (float3_sse a);

  float3_avx __ocl_svml_g9_invf3 (float3_avx a);

  float3_avx __ocl_svml_g9_divf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_sqrtf3 (float3_avx a);

  float3_avx __ocl_svml_g9_rsqrtf3 (float3_avx a);

  float3_avx __ocl_svml_g9_cbrtf3 (float3_avx a);

  float3_avx __ocl_svml_g9_rcbrtf3 (float3_avx a);

  float3_avx __ocl_svml_g9_hypotf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_powf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_powrf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_pownf3 (float3_avx a, int3_avx b);

  float3_avx __ocl_svml_g9_rootnf3 (float3_avx a, int3_avx b);

  float3_avx __ocl_svml_g9_expf3 (float3_avx a);

  float3_avx __ocl_svml_g9_exp2f3 (float3_avx a);

  float3_avx __ocl_svml_g9_exp10f3 (float3_avx a);

  float3_avx __ocl_svml_g9_expm1f3 (float3_avx a);

  float3_avx __ocl_svml_g9_logf3 (float3_avx a);

  float3_avx __ocl_svml_g9_log10f3 (float3_avx a);

  float3_avx __ocl_svml_g9_log2f3 (float3_avx a);

  float3_avx __ocl_svml_g9_log1pf3 (float3_avx a);

  float3_avx __ocl_svml_g9_sinf3 (float3_avx a);

  float3_avx __ocl_svml_g9_cosf3 (float3_avx a);

  float3_avx __ocl_svml_g9_sincosf3 (float3_avx a, float3_avx * c);

  float3x2_avx __ocl_svml_g9_sincosregf3 (float3_avx a);

  float3_avx __ocl_svml_g9_tanf3 (float3_avx a);

  float3_avx __ocl_svml_g9_sinpif3 (float3_avx a);

  float3_avx __ocl_svml_g9_cospif3 (float3_avx a);

  float3_avx __ocl_svml_g9_tanpif3 (float3_avx a);

  float3_avx __ocl_svml_g9_acosf3 (float3_avx a);

  float3_avx __ocl_svml_g9_asinf3 (float3_avx a);

  float3_avx __ocl_svml_g9_atanf3 (float3_avx a);

  float3_avx __ocl_svml_g9_atan2f3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_asinpif3 (float3_avx a);

  float3_avx __ocl_svml_g9_acospif3 (float3_avx a);

  float3_avx __ocl_svml_g9_atanpif3 (float3_avx a);

  float3_avx __ocl_svml_g9_atan2pif3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_sinhf3 (float3_avx a);

  float3_avx __ocl_svml_g9_coshf3 (float3_avx a);

  float3_avx __ocl_svml_g9_tanhf3 (float3_avx a);

  float3_avx __ocl_svml_g9_asinhf3 (float3_avx a);

  float3_avx __ocl_svml_g9_acoshf3 (float3_avx a);

  float3_avx __ocl_svml_g9_atanhf3 (float3_avx a);

  float3_avx __ocl_svml_g9_erff3 (float3_avx a);

  float3_avx __ocl_svml_g9_erfcf3 (float3_avx a);

  float3_avx __ocl_svml_s9_invf3 (float3_avx a);

  float3_avx __ocl_svml_s9_divf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_sqrtf3 (float3_avx a);

  float3_avx __ocl_svml_s9_rsqrtf3 (float3_avx a);

  float3_avx __ocl_svml_s9_cbrtf3 (float3_avx a);

  float3_avx __ocl_svml_s9_rcbrtf3 (float3_avx a);

  float3_avx __ocl_svml_s9_hypotf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_powf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_powrf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_pownf3 (float3_avx a, int3_avx b);

  float3_avx __ocl_svml_s9_rootnf3 (float3_avx a, int3_avx b);

  float3_avx __ocl_svml_s9_expf3 (float3_avx a);

  float3_avx __ocl_svml_s9_exp2f3 (float3_avx a);

  float3_avx __ocl_svml_s9_exp10f3 (float3_avx a);

  float3_avx __ocl_svml_s9_expm1f3 (float3_avx a);

  float3_avx __ocl_svml_s9_logf3 (float3_avx a);

  float3_avx __ocl_svml_s9_log10f3 (float3_avx a);

  float3_avx __ocl_svml_s9_log2f3 (float3_avx a);

  float3_avx __ocl_svml_s9_log1pf3 (float3_avx a);

  float3_avx __ocl_svml_s9_sinf3 (float3_avx a);

  float3_avx __ocl_svml_s9_cosf3 (float3_avx a);

  float3_avx __ocl_svml_s9_sincosf3 (float3_avx a, float3_avx * c);

  float3x2_avx __ocl_svml_s9_sincosregf3 (float3_avx a);

  float3_avx __ocl_svml_s9_tanf3 (float3_avx a);

  float3_avx __ocl_svml_s9_sinpif3 (float3_avx a);

  float3_avx __ocl_svml_s9_cospif3 (float3_avx a);

  float3_avx __ocl_svml_s9_tanpif3 (float3_avx a);

  float3_avx __ocl_svml_s9_acosf3 (float3_avx a);

  float3_avx __ocl_svml_s9_asinf3 (float3_avx a);

  float3_avx __ocl_svml_s9_atanf3 (float3_avx a);

  float3_avx __ocl_svml_s9_atan2f3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_asinpif3 (float3_avx a);

  float3_avx __ocl_svml_s9_acospif3 (float3_avx a);

  float3_avx __ocl_svml_s9_atanpif3 (float3_avx a);

  float3_avx __ocl_svml_s9_atan2pif3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_sinhf3 (float3_avx a);

  float3_avx __ocl_svml_s9_coshf3 (float3_avx a);

  float3_avx __ocl_svml_s9_tanhf3 (float3_avx a);

  float3_avx __ocl_svml_s9_asinhf3 (float3_avx a);

  float3_avx __ocl_svml_s9_acoshf3 (float3_avx a);

  float3_avx __ocl_svml_s9_atanhf3 (float3_avx a);

  float3_avx __ocl_svml_s9_erff3 (float3_avx a);

  float3_avx __ocl_svml_s9_erfcf3 (float3_avx a);

  float3_sse __ocl_svml_n8_fmaf3 (float3_sse a, float3_sse b, float3_sse c);

  float3_sse __ocl_svml_n8_fabsf3 (float3_sse a);

  float3_sse __ocl_svml_n8_fminf3 (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_fmaxf3 (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_maxmagf3 (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_minmagf3 (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_nanf3 (int3_sse a);

  float3_sse __ocl_svml_n8_ceilf3 (float3_sse a);

  float3_sse __ocl_svml_n8_floorf3 (float3_sse a);

  float3_sse __ocl_svml_n8_roundf3 (float3_sse a);

  float3_sse __ocl_svml_n8_truncf3 (float3_sse a);

  float3_sse __ocl_svml_n8_rintf3 (float3_sse a);

  float3_sse __ocl_svml_n8_nearbyintf3 (float3_sse a);

  float3_sse __ocl_svml_n8_fmodf3 (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_modff3 (float3_sse a, float3_sse * c);

  float3_sse __ocl_svml_n8_remainderf3 (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_remquof3 (float3_sse a, float3_sse b, int3_sse * c);

  float3_sse __ocl_svml_n8_copysignf3 (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_frexpf3 (float3_sse a, int3_sse * c);

  float3_sse __ocl_svml_n8_fdimf3 (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_logbf3 (float3_sse a);

  int3_sse __ocl_svml_n8_ilogbf3 (float3_sse a);

  float3_sse __ocl_svml_n8_nextafterf3 (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_ldexpf3 (float3_sse a, int3_sse b);

  float3_sse __ocl_svml_n8_fractf3 (float3_sse a, float3_sse * c);

  float3_sse __ocl_svml_n8_tgammaf3 (float3_sse a);

  float3_sse __ocl_svml_n8_lgammaf3 (float3_sse a);

  float3_sse __ocl_svml_n8_lgammarf3 (float3_sse a, int3_sse * c);

  long3_sse __ocl_svml_n8_cvtfptou64rtenosatf3 (float3_sse a);

  long3_sse __ocl_svml_n8_cvtfptou64rtesatf3 (float3_sse a);

  long3_sse __ocl_svml_n8_cvtfptou64rtnnosatf3 (float3_sse a);

  long3_sse __ocl_svml_n8_cvtfptou64rtnsatf3 (float3_sse a);

  long3_sse __ocl_svml_n8_cvtfptou64rtpnosatf3 (float3_sse a);

  long3_sse __ocl_svml_n8_cvtfptou64rtpsatf3 (float3_sse a);

  long3_sse __ocl_svml_n8_cvtfptou64rtznosatf3 (float3_sse a);

  long3_sse __ocl_svml_n8_cvtfptou64rtzsatf3 (float3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtenosatf3 (float3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtesatf3 (float3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtnnosatf3 (float3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtnsatf3 (float3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtpnosatf3 (float3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtpsatf3 (float3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtznosatf3 (float3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtzsatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtenosatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtesatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtnnosatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtnsatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtpnosatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtpsatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtznosatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtzsatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtenosatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtesatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtnnosatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtnsatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtpnosatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtpsatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtznosatf3 (float3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtzsatf3 (float3_sse a);

  float3_sse __ocl_svml_n8_cvtu64tofprtef3 (long3_sse a);

  float3_sse __ocl_svml_n8_cvtu64tofprtnf3 (long3_sse a);

  float3_sse __ocl_svml_n8_cvtu64tofprtpf3 (long3_sse a);

  float3_sse __ocl_svml_n8_cvtu64tofprtzf3 (long3_sse a);

  float3_sse __ocl_svml_n8_cvti64tofprtef3 (long3_sse a);

  float3_sse __ocl_svml_n8_cvti64tofprtnf3 (long3_sse a);

  float3_sse __ocl_svml_n8_cvti64tofprtpf3 (long3_sse a);

  float3_sse __ocl_svml_n8_cvti64tofprtzf3 (long3_sse a);

  float3_sse __ocl_svml_n8_cvtu32tofprtef3 (int3_sse a);

  float3_sse __ocl_svml_n8_cvtu32tofprtnf3 (int3_sse a);

  float3_sse __ocl_svml_n8_cvtu32tofprtpf3 (int3_sse a);

  float3_sse __ocl_svml_n8_cvtu32tofprtzf3 (int3_sse a);

  float3_sse __ocl_svml_n8_cvti32tofprtef3 (int3_sse a);

  float3_sse __ocl_svml_n8_cvti32tofprtnf3 (int3_sse a);

  float3_sse __ocl_svml_n8_cvti32tofprtpf3 (int3_sse a);

  float3_sse __ocl_svml_n8_cvti32tofprtzf3 (int3_sse a);

  float3_avx __ocl_svml_g9_fmaf3 (float3_avx a, float3_avx b, float3_avx c);

  float3_avx __ocl_svml_g9_fabsf3 (float3_avx a);

  float3_avx __ocl_svml_g9_fminf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_fmaxf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_maxmagf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_minmagf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_nanf3 (int3_avx a);

  float3_avx __ocl_svml_g9_ceilf3 (float3_avx a);

  float3_avx __ocl_svml_g9_floorf3 (float3_avx a);

  float3_avx __ocl_svml_g9_roundf3 (float3_avx a);

  float3_avx __ocl_svml_g9_truncf3 (float3_avx a);

  float3_avx __ocl_svml_g9_rintf3 (float3_avx a);

  float3_avx __ocl_svml_g9_nearbyintf3 (float3_avx a);

  float3_avx __ocl_svml_g9_fmodf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_modff3 (float3_avx a, float3_avx * c);

  float3_avx __ocl_svml_g9_remainderf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_remquof3 (float3_avx a, float3_avx b, int3_avx * c);

  float3_avx __ocl_svml_g9_copysignf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_frexpf3 (float3_avx a, int3_avx * c);

  float3_avx __ocl_svml_g9_fdimf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_logbf3 (float3_avx a);

  int3_avx __ocl_svml_g9_ilogbf3 (float3_avx a);

  float3_avx __ocl_svml_g9_nextafterf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_ldexpf3 (float3_avx a, int3_avx b);

  float3_avx __ocl_svml_g9_fractf3 (float3_avx a, float3_avx * c);

  float3_avx __ocl_svml_g9_tgammaf3 (float3_avx a);

  float3_avx __ocl_svml_g9_lgammaf3 (float3_avx a);

  float3_avx __ocl_svml_g9_lgammarf3 (float3_avx a, int3_avx * c);

  long3_avx __ocl_svml_g9_cvtfptou64rtenosatf3 (float3_avx a);

  long3_avx __ocl_svml_g9_cvtfptou64rtesatf3 (float3_avx a);

  long3_avx __ocl_svml_g9_cvtfptou64rtnnosatf3 (float3_avx a);

  long3_avx __ocl_svml_g9_cvtfptou64rtnsatf3 (float3_avx a);

  long3_avx __ocl_svml_g9_cvtfptou64rtpnosatf3 (float3_avx a);

  long3_avx __ocl_svml_g9_cvtfptou64rtpsatf3 (float3_avx a);

  long3_avx __ocl_svml_g9_cvtfptou64rtznosatf3 (float3_avx a);

  long3_avx __ocl_svml_g9_cvtfptou64rtzsatf3 (float3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtenosatf3 (float3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtesatf3 (float3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtnnosatf3 (float3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtnsatf3 (float3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtpnosatf3 (float3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtpsatf3 (float3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtznosatf3 (float3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtzsatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtenosatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtesatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtnnosatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtnsatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtpnosatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtpsatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtznosatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtzsatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtenosatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtesatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtnnosatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtnsatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtpnosatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtpsatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtznosatf3 (float3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtzsatf3 (float3_avx a);

  float3_avx __ocl_svml_g9_cvtu64tofprtef3 (long3_avx a);

  float3_avx __ocl_svml_g9_cvtu64tofprtnf3 (long3_avx a);

  float3_avx __ocl_svml_g9_cvtu64tofprtpf3 (long3_avx a);

  float3_avx __ocl_svml_g9_cvtu64tofprtzf3 (long3_avx a);

  float3_avx __ocl_svml_g9_cvti64tofprtef3 (long3_avx a);

  float3_avx __ocl_svml_g9_cvti64tofprtnf3 (long3_avx a);

  float3_avx __ocl_svml_g9_cvti64tofprtpf3 (long3_avx a);

  float3_avx __ocl_svml_g9_cvti64tofprtzf3 (long3_avx a);

  float3_avx __ocl_svml_g9_cvtu32tofprtef3 (int3_avx a);

  float3_avx __ocl_svml_g9_cvtu32tofprtnf3 (int3_avx a);

  float3_avx __ocl_svml_g9_cvtu32tofprtpf3 (int3_avx a);

  float3_avx __ocl_svml_g9_cvtu32tofprtzf3 (int3_avx a);

  float3_avx __ocl_svml_g9_cvti32tofprtef3 (int3_avx a);

  float3_avx __ocl_svml_g9_cvti32tofprtnf3 (int3_avx a);

  float3_avx __ocl_svml_g9_cvti32tofprtpf3 (int3_avx a);

  float3_avx __ocl_svml_g9_cvti32tofprtzf3 (int3_avx a);

  float3_avx __ocl_svml_s9_fmaf3 (float3_avx a, float3_avx b, float3_avx c);

  float3_avx __ocl_svml_s9_fabsf3 (float3_avx a);

  float3_avx __ocl_svml_s9_fminf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_fmaxf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_maxmagf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_minmagf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_nanf3 (int3_avx a);

  float3_avx __ocl_svml_s9_ceilf3 (float3_avx a);

  float3_avx __ocl_svml_s9_floorf3 (float3_avx a);

  float3_avx __ocl_svml_s9_roundf3 (float3_avx a);

  float3_avx __ocl_svml_s9_truncf3 (float3_avx a);

  float3_avx __ocl_svml_s9_rintf3 (float3_avx a);

  float3_avx __ocl_svml_s9_nearbyintf3 (float3_avx a);

  float3_avx __ocl_svml_s9_fmodf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_modff3 (float3_avx a, float3_avx * c);

  float3_avx __ocl_svml_s9_remainderf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_remquof3 (float3_avx a, float3_avx b, int3_avx * c);

  float3_avx __ocl_svml_s9_copysignf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_frexpf3 (float3_avx a, int3_avx * c);

  float3_avx __ocl_svml_s9_fdimf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_logbf3 (float3_avx a);

  int3_avx __ocl_svml_s9_ilogbf3 (float3_avx a);

  float3_avx __ocl_svml_s9_nextafterf3 (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_ldexpf3 (float3_avx a, int3_avx b);

  float3_avx __ocl_svml_s9_fractf3 (float3_avx a, float3_avx * c);

  float3_avx __ocl_svml_s9_tgammaf3 (float3_avx a);

  float3_avx __ocl_svml_s9_lgammaf3 (float3_avx a);

  float3_avx __ocl_svml_s9_lgammarf3 (float3_avx a, int3_avx * c);

  long3_avx __ocl_svml_s9_cvtfptou64rtenosatf3 (float3_avx a);

  long3_avx __ocl_svml_s9_cvtfptou64rtesatf3 (float3_avx a);

  long3_avx __ocl_svml_s9_cvtfptou64rtnnosatf3 (float3_avx a);

  long3_avx __ocl_svml_s9_cvtfptou64rtnsatf3 (float3_avx a);

  long3_avx __ocl_svml_s9_cvtfptou64rtpnosatf3 (float3_avx a);

  long3_avx __ocl_svml_s9_cvtfptou64rtpsatf3 (float3_avx a);

  long3_avx __ocl_svml_s9_cvtfptou64rtznosatf3 (float3_avx a);

  long3_avx __ocl_svml_s9_cvtfptou64rtzsatf3 (float3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtenosatf3 (float3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtesatf3 (float3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtnnosatf3 (float3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtnsatf3 (float3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtpnosatf3 (float3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtpsatf3 (float3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtznosatf3 (float3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtzsatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtenosatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtesatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtnnosatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtnsatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtpnosatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtpsatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtznosatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtzsatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtenosatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtesatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtnnosatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtnsatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtpnosatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtpsatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtznosatf3 (float3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtzsatf3 (float3_avx a);

  float3_avx __ocl_svml_s9_cvtu64tofprtef3 (long3_avx a);

  float3_avx __ocl_svml_s9_cvtu64tofprtnf3 (long3_avx a);

  float3_avx __ocl_svml_s9_cvtu64tofprtpf3 (long3_avx a);

  float3_avx __ocl_svml_s9_cvtu64tofprtzf3 (long3_avx a);

  float3_avx __ocl_svml_s9_cvti64tofprtef3 (long3_avx a);

  float3_avx __ocl_svml_s9_cvti64tofprtnf3 (long3_avx a);

  float3_avx __ocl_svml_s9_cvti64tofprtpf3 (long3_avx a);

  float3_avx __ocl_svml_s9_cvti64tofprtzf3 (long3_avx a);

  float3_avx __ocl_svml_s9_cvtu32tofprtef3 (int3_avx a);

  float3_avx __ocl_svml_s9_cvtu32tofprtnf3 (int3_avx a);

  float3_avx __ocl_svml_s9_cvtu32tofprtpf3 (int3_avx a);

  float3_avx __ocl_svml_s9_cvtu32tofprtzf3 (int3_avx a);

  float3_avx __ocl_svml_s9_cvti32tofprtef3 (int3_avx a);

  float3_avx __ocl_svml_s9_cvti32tofprtnf3 (int3_avx a);

  float3_avx __ocl_svml_s9_cvti32tofprtpf3 (int3_avx a);

  float3_avx __ocl_svml_s9_cvti32tofprtzf3 (int3_avx a);

  double3_sse __ocl_svml_n8_inv3 (double3_sse a);

  double3_sse __ocl_svml_n8_div3 (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_sqrt3 (double3_sse a);

  double3_sse __ocl_svml_n8_rsqrt3 (double3_sse a);

  double3_sse __ocl_svml_n8_cbrt3 (double3_sse a);

  double3_sse __ocl_svml_n8_rcbrt3 (double3_sse a);

  double3_sse __ocl_svml_n8_hypot3 (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_pow3 (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_powr3 (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_pown3 (double3_sse a, int3_sse b);

  double3_sse __ocl_svml_n8_rootn3 (double3_sse a, int3_sse b);

  double3_sse __ocl_svml_n8_exp3 (double3_sse a);

  double3_sse __ocl_svml_n8_exp23 (double3_sse a);

  double3_sse __ocl_svml_n8_exp103 (double3_sse a);

  double3_sse __ocl_svml_n8_expm13 (double3_sse a);

  double3_sse __ocl_svml_n8_log3 (double3_sse a);

  double3_sse __ocl_svml_n8_log103 (double3_sse a);

  double3_sse __ocl_svml_n8_log23 (double3_sse a);

  double3_sse __ocl_svml_n8_log1p3 (double3_sse a);

  double3_sse __ocl_svml_n8_sin3 (double3_sse a);

  double3_sse __ocl_svml_n8_cos3 (double3_sse a);

  double3_sse __ocl_svml_n8_sincos3 (double3_sse a, double3_sse * c);

  double3x2_sse __ocl_svml_n8_sincosreg3 (double3_sse a);

  double3_sse __ocl_svml_n8_tan3 (double3_sse a);

  double3_sse __ocl_svml_n8_sinpi3 (double3_sse a);

  double3_sse __ocl_svml_n8_cospi3 (double3_sse a);

  double3_sse __ocl_svml_n8_tanpi3 (double3_sse a);

  double3_sse __ocl_svml_n8_acos3 (double3_sse a);

  double3_sse __ocl_svml_n8_asin3 (double3_sse a);

  double3_sse __ocl_svml_n8_atan3 (double3_sse a);

  double3_sse __ocl_svml_n8_atan23 (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_asinpi3 (double3_sse a);

  double3_sse __ocl_svml_n8_acospi3 (double3_sse a);

  double3_sse __ocl_svml_n8_atanpi3 (double3_sse a);

  double3_sse __ocl_svml_n8_atan2pi3 (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_sinh3 (double3_sse a);

  double3_sse __ocl_svml_n8_cosh3 (double3_sse a);

  double3_sse __ocl_svml_n8_tanh3 (double3_sse a);

  double3_sse __ocl_svml_n8_asinh3 (double3_sse a);

  double3_sse __ocl_svml_n8_acosh3 (double3_sse a);

  double3_sse __ocl_svml_n8_atanh3 (double3_sse a);

  double3_sse __ocl_svml_n8_erf3 (double3_sse a);

  double3_sse __ocl_svml_n8_erfc3 (double3_sse a);

  double3_avx __ocl_svml_g9_inv3 (double3_avx a);

  double3_avx __ocl_svml_g9_div3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_sqrt3 (double3_avx a);

  double3_avx __ocl_svml_g9_rsqrt3 (double3_avx a);

  double3_avx __ocl_svml_g9_cbrt3 (double3_avx a);

  double3_avx __ocl_svml_g9_rcbrt3 (double3_avx a);

  double3_avx __ocl_svml_g9_hypot3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_pow3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_powr3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_pown3 (double3_avx a, int3_avx b);

  double3_avx __ocl_svml_g9_rootn3 (double3_avx a, int3_avx b);

  double3_avx __ocl_svml_g9_exp3 (double3_avx a);

  double3_avx __ocl_svml_g9_exp23 (double3_avx a);

  double3_avx __ocl_svml_g9_exp103 (double3_avx a);

  double3_avx __ocl_svml_g9_expm13 (double3_avx a);

  double3_avx __ocl_svml_g9_log3 (double3_avx a);

  double3_avx __ocl_svml_g9_log103 (double3_avx a);

  double3_avx __ocl_svml_g9_log23 (double3_avx a);

  double3_avx __ocl_svml_g9_log1p3 (double3_avx a);

  double3_avx __ocl_svml_g9_sin3 (double3_avx a);

  double3_avx __ocl_svml_g9_cos3 (double3_avx a);

  double3_avx __ocl_svml_g9_sincos3 (double3_avx a, double3_avx * c);

  double3x2_avx __ocl_svml_g9_sincosreg3 (double3_avx a);

  double3_avx __ocl_svml_g9_tan3 (double3_avx a);

  double3_avx __ocl_svml_g9_sinpi3 (double3_avx a);

  double3_avx __ocl_svml_g9_cospi3 (double3_avx a);

  double3_avx __ocl_svml_g9_tanpi3 (double3_avx a);

  double3_avx __ocl_svml_g9_acos3 (double3_avx a);

  double3_avx __ocl_svml_g9_asin3 (double3_avx a);

  double3_avx __ocl_svml_g9_atan3 (double3_avx a);

  double3_avx __ocl_svml_g9_atan23 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_asinpi3 (double3_avx a);

  double3_avx __ocl_svml_g9_acospi3 (double3_avx a);

  double3_avx __ocl_svml_g9_atanpi3 (double3_avx a);

  double3_avx __ocl_svml_g9_atan2pi3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_sinh3 (double3_avx a);

  double3_avx __ocl_svml_g9_cosh3 (double3_avx a);

  double3_avx __ocl_svml_g9_tanh3 (double3_avx a);

  double3_avx __ocl_svml_g9_asinh3 (double3_avx a);

  double3_avx __ocl_svml_g9_acosh3 (double3_avx a);

  double3_avx __ocl_svml_g9_atanh3 (double3_avx a);

  double3_avx __ocl_svml_g9_erf3 (double3_avx a);

  double3_avx __ocl_svml_g9_erfc3 (double3_avx a);

  double3_avx __ocl_svml_s9_inv3 (double3_avx a);

  double3_avx __ocl_svml_s9_div3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_sqrt3 (double3_avx a);

  double3_avx __ocl_svml_s9_rsqrt3 (double3_avx a);

  double3_avx __ocl_svml_s9_cbrt3 (double3_avx a);

  double3_avx __ocl_svml_s9_rcbrt3 (double3_avx a);

  double3_avx __ocl_svml_s9_hypot3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_pow3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_powr3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_pown3 (double3_avx a, int3_avx b);

  double3_avx __ocl_svml_s9_rootn3 (double3_avx a, int3_avx b);

  double3_avx __ocl_svml_s9_exp3 (double3_avx a);

  double3_avx __ocl_svml_s9_exp23 (double3_avx a);

  double3_avx __ocl_svml_s9_exp103 (double3_avx a);

  double3_avx __ocl_svml_s9_expm13 (double3_avx a);

  double3_avx __ocl_svml_s9_log3 (double3_avx a);

  double3_avx __ocl_svml_s9_log103 (double3_avx a);

  double3_avx __ocl_svml_s9_log23 (double3_avx a);

  double3_avx __ocl_svml_s9_log1p3 (double3_avx a);

  double3_avx __ocl_svml_s9_sin3 (double3_avx a);

  double3_avx __ocl_svml_s9_cos3 (double3_avx a);

  double3_avx __ocl_svml_s9_sincos3 (double3_avx a, double3_avx * c);

  double3x2_avx __ocl_svml_s9_sincosreg3 (double3_avx a);

  double3_avx __ocl_svml_s9_tan3 (double3_avx a);

  double3_avx __ocl_svml_s9_sinpi3 (double3_avx a);

  double3_avx __ocl_svml_s9_cospi3 (double3_avx a);

  double3_avx __ocl_svml_s9_tanpi3 (double3_avx a);

  double3_avx __ocl_svml_s9_acos3 (double3_avx a);

  double3_avx __ocl_svml_s9_asin3 (double3_avx a);

  double3_avx __ocl_svml_s9_atan3 (double3_avx a);

  double3_avx __ocl_svml_s9_atan23 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_asinpi3 (double3_avx a);

  double3_avx __ocl_svml_s9_acospi3 (double3_avx a);

  double3_avx __ocl_svml_s9_atanpi3 (double3_avx a);

  double3_avx __ocl_svml_s9_atan2pi3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_sinh3 (double3_avx a);

  double3_avx __ocl_svml_s9_cosh3 (double3_avx a);

  double3_avx __ocl_svml_s9_tanh3 (double3_avx a);

  double3_avx __ocl_svml_s9_asinh3 (double3_avx a);

  double3_avx __ocl_svml_s9_acosh3 (double3_avx a);

  double3_avx __ocl_svml_s9_atanh3 (double3_avx a);

  double3_avx __ocl_svml_s9_erf3 (double3_avx a);

  double3_avx __ocl_svml_s9_erfc3 (double3_avx a);

  double3_sse __ocl_svml_n8_fabs3 (double3_sse a);

  double3_sse __ocl_svml_n8_fmin3 (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_fmax3 (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_maxmag3 (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_minmag3 (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_nan3 (long3_sse a);

  double3_sse __ocl_svml_n8_ceil3 (double3_sse a);

  double3_sse __ocl_svml_n8_floor3 (double3_sse a);

  double3_sse __ocl_svml_n8_round3 (double3_sse a);

  double3_sse __ocl_svml_n8_trunc3 (double3_sse a);

  double3_sse __ocl_svml_n8_rint3 (double3_sse a);

  double3_sse __ocl_svml_n8_nearbyint3 (double3_sse a);

  double3_sse __ocl_svml_n8_fmod3 (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_modf3 (double3_sse a, double3_sse * c);

  double3_sse __ocl_svml_n8_remainder3 (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_remquo3 (double3_sse a, double3_sse b, int3_sse * c);

  double3_sse __ocl_svml_n8_copysign3 (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_frexp3 (double3_sse a, int3_sse * c);

  double3_sse __ocl_svml_n8_fdim3 (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_logb3 (double3_sse a);

  int3_sse __ocl_svml_n8_ilogb3 (double3_sse a);

  double3_sse __ocl_svml_n8_nextafter3 (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_ldexp3 (double3_sse a, int3_sse b);

  double3_sse __ocl_svml_n8_fract3 (double3_sse a, double3_sse * c);

  double3_sse __ocl_svml_n8_tgamma3 (double3_sse a);

  double3_sse __ocl_svml_n8_lgamma3 (double3_sse a);

  double3_sse __ocl_svml_n8_lgammar3 (double3_sse a, int3_sse * c);

  long3_sse __ocl_svml_n8_cvtfptou64rtenosat3 (double3_sse a);

  long3_sse __ocl_svml_n8_cvtfptou64rtesat3 (double3_sse a);

  long3_sse __ocl_svml_n8_cvtfptou64rtnnosat3 (double3_sse a);

  long3_sse __ocl_svml_n8_cvtfptou64rtnsat3 (double3_sse a);

  long3_sse __ocl_svml_n8_cvtfptou64rtpnosat3 (double3_sse a);

  long3_sse __ocl_svml_n8_cvtfptou64rtpsat3 (double3_sse a);

  long3_sse __ocl_svml_n8_cvtfptou64rtznosat3 (double3_sse a);

  long3_sse __ocl_svml_n8_cvtfptou64rtzsat3 (double3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtenosat3 (double3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtesat3 (double3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtnnosat3 (double3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtnsat3 (double3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtpnosat3 (double3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtpsat3 (double3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtznosat3 (double3_sse a);

  long3_sse __ocl_svml_n8_cvtfptoi64rtzsat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtenosat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtesat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtnnosat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtnsat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtpnosat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtpsat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtznosat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptou32rtzsat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtenosat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtesat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtnnosat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtnsat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtpnosat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtpsat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtznosat3 (double3_sse a);

  int3_sse __ocl_svml_n8_cvtfptoi32rtzsat3 (double3_sse a);

  double3_sse __ocl_svml_n8_cvtu64tofprte3 (long3_sse a);

  double3_sse __ocl_svml_n8_cvtu64tofprtn3 (long3_sse a);

  double3_sse __ocl_svml_n8_cvtu64tofprtp3 (long3_sse a);

  double3_sse __ocl_svml_n8_cvtu64tofprtz3 (long3_sse a);

  double3_sse __ocl_svml_n8_cvti64tofprte3 (long3_sse a);

  double3_sse __ocl_svml_n8_cvti64tofprtn3 (long3_sse a);

  double3_sse __ocl_svml_n8_cvti64tofprtp3 (long3_sse a);

  double3_sse __ocl_svml_n8_cvti64tofprtz3 (long3_sse a);

  double3_sse __ocl_svml_n8_cvtu32tofprte3 (int3_sse a);

  double3_sse __ocl_svml_n8_cvtu32tofprtn3 (int3_sse a);

  double3_sse __ocl_svml_n8_cvtu32tofprtp3 (int3_sse a);

  double3_sse __ocl_svml_n8_cvtu32tofprtz3 (int3_sse a);

  double3_sse __ocl_svml_n8_cvti32tofprte3 (int3_sse a);

  double3_sse __ocl_svml_n8_cvti32tofprtn3 (int3_sse a);

  double3_sse __ocl_svml_n8_cvti32tofprtp3 (int3_sse a);

  double3_sse __ocl_svml_n8_cvti32tofprtz3 (int3_sse a);

  float3_sse __ocl_svml_n8_cvtfp64tofp32rte3 (double3_sse a);

  float3_sse __ocl_svml_n8_cvtfp64tofp32rtn3 (double3_sse a);

  float3_sse __ocl_svml_n8_cvtfp64tofp32rtp3 (double3_sse a);

  float3_sse __ocl_svml_n8_cvtfp64tofp32rtz3 (double3_sse a);

  double3_avx __ocl_svml_g9_fma3 (double3_avx a, double3_avx b, double3_avx c);

  double3_avx __ocl_svml_g9_fabs3 (double3_avx a);

  double3_avx __ocl_svml_g9_fmin3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_fmax3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_maxmag3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_minmag3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_nan3 (long3_avx a);

  double3_avx __ocl_svml_g9_ceil3 (double3_avx a);

  double3_avx __ocl_svml_g9_floor3 (double3_avx a);

  double3_avx __ocl_svml_g9_round3 (double3_avx a);

  double3_avx __ocl_svml_g9_trunc3 (double3_avx a);

  double3_avx __ocl_svml_g9_rint3 (double3_avx a);

  double3_avx __ocl_svml_g9_nearbyint3 (double3_avx a);

  double3_avx __ocl_svml_g9_fmod3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_modf3 (double3_avx a, double3_avx * c);

  double3_avx __ocl_svml_g9_remainder3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_remquo3 (double3_avx a, double3_avx b, int3_avx * c);

  double3_avx __ocl_svml_g9_copysign3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_frexp3 (double3_avx a, int3_avx * c);

  double3_avx __ocl_svml_g9_fdim3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_logb3 (double3_avx a);

  int3_avx __ocl_svml_g9_ilogb3 (double3_avx a);

  double3_avx __ocl_svml_g9_nextafter3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_ldexp3 (double3_avx a, int3_avx b);

  double3_avx __ocl_svml_g9_fract3 (double3_avx a, double3_avx * c);

  double3_avx __ocl_svml_g9_tgamma3 (double3_avx a);

  double3_avx __ocl_svml_g9_lgamma3 (double3_avx a);

  double3_avx __ocl_svml_g9_lgammar3 (double3_avx a, int3_avx * c);

  long3_avx __ocl_svml_g9_cvtfptou64rtenosat3 (double3_avx a);

  long3_avx __ocl_svml_g9_cvtfptou64rtesat3 (double3_avx a);

  long3_avx __ocl_svml_g9_cvtfptou64rtnnosat3 (double3_avx a);

  long3_avx __ocl_svml_g9_cvtfptou64rtnsat3 (double3_avx a);

  long3_avx __ocl_svml_g9_cvtfptou64rtpnosat3 (double3_avx a);

  long3_avx __ocl_svml_g9_cvtfptou64rtpsat3 (double3_avx a);

  long3_avx __ocl_svml_g9_cvtfptou64rtznosat3 (double3_avx a);

  long3_avx __ocl_svml_g9_cvtfptou64rtzsat3 (double3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtenosat3 (double3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtesat3 (double3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtnnosat3 (double3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtnsat3 (double3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtpnosat3 (double3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtpsat3 (double3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtznosat3 (double3_avx a);

  long3_avx __ocl_svml_g9_cvtfptoi64rtzsat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtenosat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtesat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtnnosat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtnsat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtpnosat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtpsat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtznosat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptou32rtzsat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtenosat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtesat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtnnosat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtnsat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtpnosat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtpsat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtznosat3 (double3_avx a);

  int3_avx __ocl_svml_g9_cvtfptoi32rtzsat3 (double3_avx a);

  double3_avx __ocl_svml_g9_cvtu64tofprte3 (long3_avx a);

  double3_avx __ocl_svml_g9_cvtu64tofprtn3 (long3_avx a);

  double3_avx __ocl_svml_g9_cvtu64tofprtp3 (long3_avx a);

  double3_avx __ocl_svml_g9_cvtu64tofprtz3 (long3_avx a);

  double3_avx __ocl_svml_g9_cvti64tofprte3 (long3_avx a);

  double3_avx __ocl_svml_g9_cvti64tofprtn3 (long3_avx a);

  double3_avx __ocl_svml_g9_cvti64tofprtp3 (long3_avx a);

  double3_avx __ocl_svml_g9_cvti64tofprtz3 (long3_avx a);

  double3_avx __ocl_svml_g9_cvtu32tofprte3 (int3_avx a);

  double3_avx __ocl_svml_g9_cvtu32tofprtn3 (int3_avx a);

  double3_avx __ocl_svml_g9_cvtu32tofprtp3 (int3_avx a);

  double3_avx __ocl_svml_g9_cvtu32tofprtz3 (int3_avx a);

  double3_avx __ocl_svml_g9_cvti32tofprte3 (int3_avx a);

  double3_avx __ocl_svml_g9_cvti32tofprtn3 (int3_avx a);

  double3_avx __ocl_svml_g9_cvti32tofprtp3 (int3_avx a);

  double3_avx __ocl_svml_g9_cvti32tofprtz3 (int3_avx a);

  float3_avx __ocl_svml_g9_cvtfp64tofp32rte3 (double3_avx a);

  float3_avx __ocl_svml_g9_cvtfp64tofp32rtn3 (double3_avx a);

  float3_avx __ocl_svml_g9_cvtfp64tofp32rtp3 (double3_avx a);

  float3_avx __ocl_svml_g9_cvtfp64tofp32rtz3 (double3_avx a);

  double3_avx __ocl_svml_s9_fma3 (double3_avx a, double3_avx b, double3_avx c);

  double3_avx __ocl_svml_s9_fabs3 (double3_avx a);

  double3_avx __ocl_svml_s9_fmin3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_fmax3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_maxmag3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_minmag3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_nan3 (long3_avx a);

  double3_avx __ocl_svml_s9_ceil3 (double3_avx a);

  double3_avx __ocl_svml_s9_floor3 (double3_avx a);

  double3_avx __ocl_svml_s9_round3 (double3_avx a);

  double3_avx __ocl_svml_s9_trunc3 (double3_avx a);

  double3_avx __ocl_svml_s9_rint3 (double3_avx a);

  double3_avx __ocl_svml_s9_nearbyint3 (double3_avx a);

  double3_avx __ocl_svml_s9_fmod3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_modf3 (double3_avx a, double3_avx * c);

  double3_avx __ocl_svml_s9_remainder3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_remquo3 (double3_avx a, double3_avx b, int3_avx * c);

  double3_avx __ocl_svml_s9_copysign3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_frexp3 (double3_avx a, int3_avx * c);

  double3_avx __ocl_svml_s9_fdim3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_logb3 (double3_avx a);

  int3_avx __ocl_svml_s9_ilogb3 (double3_avx a);

  double3_avx __ocl_svml_s9_nextafter3 (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_ldexp3 (double3_avx a, int3_avx b);

  double3_avx __ocl_svml_s9_fract3 (double3_avx a, double3_avx * c);

  double3_avx __ocl_svml_s9_tgamma3 (double3_avx a);

  double3_avx __ocl_svml_s9_lgamma3 (double3_avx a);

  double3_avx __ocl_svml_s9_lgammar3 (double3_avx a, int3_avx * c);

  long3_avx __ocl_svml_s9_cvtfptou64rtenosat3 (double3_avx a);

  long3_avx __ocl_svml_s9_cvtfptou64rtesat3 (double3_avx a);

  long3_avx __ocl_svml_s9_cvtfptou64rtnnosat3 (double3_avx a);

  long3_avx __ocl_svml_s9_cvtfptou64rtnsat3 (double3_avx a);

  long3_avx __ocl_svml_s9_cvtfptou64rtpnosat3 (double3_avx a);

  long3_avx __ocl_svml_s9_cvtfptou64rtpsat3 (double3_avx a);

  long3_avx __ocl_svml_s9_cvtfptou64rtznosat3 (double3_avx a);

  long3_avx __ocl_svml_s9_cvtfptou64rtzsat3 (double3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtenosat3 (double3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtesat3 (double3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtnnosat3 (double3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtnsat3 (double3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtpnosat3 (double3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtpsat3 (double3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtznosat3 (double3_avx a);

  long3_avx __ocl_svml_s9_cvtfptoi64rtzsat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtenosat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtesat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtnnosat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtnsat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtpnosat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtpsat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtznosat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptou32rtzsat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtenosat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtesat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtnnosat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtnsat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtpnosat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtpsat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtznosat3 (double3_avx a);

  int3_avx __ocl_svml_s9_cvtfptoi32rtzsat3 (double3_avx a);

  double3_avx __ocl_svml_s9_cvtu64tofprte3 (long3_avx a);

  double3_avx __ocl_svml_s9_cvtu64tofprtn3 (long3_avx a);

  double3_avx __ocl_svml_s9_cvtu64tofprtp3 (long3_avx a);

  double3_avx __ocl_svml_s9_cvtu64tofprtz3 (long3_avx a);

  double3_avx __ocl_svml_s9_cvti64tofprte3 (long3_avx a);

  double3_avx __ocl_svml_s9_cvti64tofprtn3 (long3_avx a);

  double3_avx __ocl_svml_s9_cvti64tofprtp3 (long3_avx a);

  double3_avx __ocl_svml_s9_cvti64tofprtz3 (long3_avx a);

  double3_avx __ocl_svml_s9_cvtu32tofprte3 (int3_avx a);

  double3_avx __ocl_svml_s9_cvtu32tofprtn3 (int3_avx a);

  double3_avx __ocl_svml_s9_cvtu32tofprtp3 (int3_avx a);

  double3_avx __ocl_svml_s9_cvtu32tofprtz3 (int3_avx a);

  double3_avx __ocl_svml_s9_cvti32tofprte3 (int3_avx a);

  double3_avx __ocl_svml_s9_cvti32tofprtn3 (int3_avx a);

  double3_avx __ocl_svml_s9_cvti32tofprtp3 (int3_avx a);

  double3_avx __ocl_svml_s9_cvti32tofprtz3 (int3_avx a);

  float3_avx __ocl_svml_s9_cvtfp64tofp32rte3 (double3_avx a);

  float3_avx __ocl_svml_s9_cvtfp64tofp32rtn3 (double3_avx a);

  float3_avx __ocl_svml_s9_cvtfp64tofp32rtp3 (double3_avx a);

  float3_avx __ocl_svml_s9_cvtfp64tofp32rtz3 (double3_avx a);

  int3_sse __ocl_svml_n8_idiv3 (int3_sse a, int3_sse b);

  int3_sse __ocl_svml_n8_irem3 (int3_sse a, int3_sse b);

  int3x2_sse __ocl_svml_n8_idivrem3 (int3_sse a, int3_sse b);

  int3_avx __ocl_svml_g9_idiv3 (int3_avx a, int3_avx b);

  int3_avx __ocl_svml_g9_irem3 (int3_avx a, int3_avx b);

  int3x2_avx __ocl_svml_g9_idivrem3 (int3_avx a, int3_avx b);

  int3_avx __ocl_svml_s9_idiv3 (int3_avx a, int3_avx b);

  int3_avx __ocl_svml_s9_irem3 (int3_avx a, int3_avx b);

  int3x2_avx __ocl_svml_s9_idivrem3 (int3_avx a, int3_avx b);

  int3_sse __ocl_svml_n8_udiv3 (int3_sse a, int3_sse b);

  int3_sse __ocl_svml_n8_urem3 (int3_sse a, int3_sse b);

  int3x2_sse __ocl_svml_n8_udivrem3 (int3_sse a, int3_sse b);

  int3_avx __ocl_svml_g9_udiv3 (int3_avx a, int3_avx b);

  int3_avx __ocl_svml_g9_urem3 (int3_avx a, int3_avx b);

  int3x2_avx __ocl_svml_g9_udivrem3 (int3_avx a, int3_avx b);

  int3_avx __ocl_svml_s9_udiv3 (int3_avx a, int3_avx b);

  int3_avx __ocl_svml_s9_urem3 (int3_avx a, int3_avx b);

  int3x2_avx __ocl_svml_s9_udivrem3 (int3_avx a, int3_avx b);

  float4_sse __ocl_svml_n8_invf4 (float4_sse a);

  float4_sse __ocl_svml_n8_divf4 (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_sqrtf4 (float4_sse a);

  float4_sse __ocl_svml_n8_rsqrtf4 (float4_sse a);

  float4_sse __ocl_svml_n8_cbrtf4 (float4_sse a);

  float4_sse __ocl_svml_n8_rcbrtf4 (float4_sse a);

  float4_sse __ocl_svml_n8_hypotf4 (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_powf4 (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_powrf4 (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_pownf4 (float4_sse a, int4_sse b);

  float4_sse __ocl_svml_n8_rootnf4 (float4_sse a, int4_sse b);

  float4_sse __ocl_svml_n8_expf4 (float4_sse a);

  float4_sse __ocl_svml_n8_exp2f4 (float4_sse a);

  float4_sse __ocl_svml_n8_exp10f4 (float4_sse a);

  float4_sse __ocl_svml_n8_expm1f4 (float4_sse a);

  float4_sse __ocl_svml_n8_logf4 (float4_sse a);

  float4_sse __ocl_svml_n8_log10f4 (float4_sse a);

  float4_sse __ocl_svml_n8_log2f4 (float4_sse a);

  float4_sse __ocl_svml_n8_log1pf4 (float4_sse a);

  float4_sse __ocl_svml_n8_sinf4 (float4_sse a);

  float4_sse __ocl_svml_n8_cosf4 (float4_sse a);

  float4_sse __ocl_svml_n8_sincosf4 (float4_sse a, float4_sse * c);

  float4x2_sse __ocl_svml_n8_sincosregf4 (float4_sse a);

  float4_sse __ocl_svml_n8_tanf4 (float4_sse a);

  float4_sse __ocl_svml_n8_sinpif4 (float4_sse a);

  float4_sse __ocl_svml_n8_cospif4 (float4_sse a);

  float4_sse __ocl_svml_n8_tanpif4 (float4_sse a);

  float4_sse __ocl_svml_n8_acosf4 (float4_sse a);

  float4_sse __ocl_svml_n8_asinf4 (float4_sse a);

  float4_sse __ocl_svml_n8_atanf4 (float4_sse a);

  float4_sse __ocl_svml_n8_atan2f4 (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_asinpif4 (float4_sse a);

  float4_sse __ocl_svml_n8_acospif4 (float4_sse a);

  float4_sse __ocl_svml_n8_atanpif4 (float4_sse a);

  float4_sse __ocl_svml_n8_atan2pif4 (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_sinhf4 (float4_sse a);

  float4_sse __ocl_svml_n8_coshf4 (float4_sse a);

  float4_sse __ocl_svml_n8_tanhf4 (float4_sse a);

  float4_sse __ocl_svml_n8_asinhf4 (float4_sse a);

  float4_sse __ocl_svml_n8_acoshf4 (float4_sse a);

  float4_sse __ocl_svml_n8_atanhf4 (float4_sse a);

  float4_sse __ocl_svml_n8_erff4 (float4_sse a);

  float4_sse __ocl_svml_n8_erfcf4 (float4_sse a);

  float4_avx __ocl_svml_g9_invf4 (float4_avx a);

  float4_avx __ocl_svml_g9_divf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_sqrtf4 (float4_avx a);

  float4_avx __ocl_svml_g9_rsqrtf4 (float4_avx a);

  float4_avx __ocl_svml_g9_cbrtf4 (float4_avx a);

  float4_avx __ocl_svml_g9_rcbrtf4 (float4_avx a);

  float4_avx __ocl_svml_g9_hypotf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_powf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_powrf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_pownf4 (float4_avx a, int4_avx b);

  float4_avx __ocl_svml_g9_rootnf4 (float4_avx a, int4_avx b);

  float4_avx __ocl_svml_g9_expf4 (float4_avx a);

  float4_avx __ocl_svml_g9_exp2f4 (float4_avx a);

  float4_avx __ocl_svml_g9_exp10f4 (float4_avx a);

  float4_avx __ocl_svml_g9_expm1f4 (float4_avx a);

  float4_avx __ocl_svml_g9_logf4 (float4_avx a);

  float4_avx __ocl_svml_g9_log10f4 (float4_avx a);

  float4_avx __ocl_svml_g9_log2f4 (float4_avx a);

  float4_avx __ocl_svml_g9_log1pf4 (float4_avx a);

  float4_avx __ocl_svml_g9_sinf4 (float4_avx a);

  float4_avx __ocl_svml_g9_cosf4 (float4_avx a);

  float4_avx __ocl_svml_g9_sincosf4 (float4_avx a, float4_avx * c);

  float4x2_avx __ocl_svml_g9_sincosregf4 (float4_avx a);

  float4_avx __ocl_svml_g9_tanf4 (float4_avx a);

  float4_avx __ocl_svml_g9_sinpif4 (float4_avx a);

  float4_avx __ocl_svml_g9_cospif4 (float4_avx a);

  float4_avx __ocl_svml_g9_tanpif4 (float4_avx a);

  float4_avx __ocl_svml_g9_acosf4 (float4_avx a);

  float4_avx __ocl_svml_g9_asinf4 (float4_avx a);

  float4_avx __ocl_svml_g9_atanf4 (float4_avx a);

  float4_avx __ocl_svml_g9_atan2f4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_asinpif4 (float4_avx a);

  float4_avx __ocl_svml_g9_acospif4 (float4_avx a);

  float4_avx __ocl_svml_g9_atanpif4 (float4_avx a);

  float4_avx __ocl_svml_g9_atan2pif4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_sinhf4 (float4_avx a);

  float4_avx __ocl_svml_g9_coshf4 (float4_avx a);

  float4_avx __ocl_svml_g9_tanhf4 (float4_avx a);

  float4_avx __ocl_svml_g9_asinhf4 (float4_avx a);

  float4_avx __ocl_svml_g9_acoshf4 (float4_avx a);

  float4_avx __ocl_svml_g9_atanhf4 (float4_avx a);

  float4_avx __ocl_svml_g9_erff4 (float4_avx a);

  float4_avx __ocl_svml_g9_erfcf4 (float4_avx a);

  float4_avx __ocl_svml_s9_invf4 (float4_avx a);

  float4_avx __ocl_svml_s9_divf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_sqrtf4 (float4_avx a);

  float4_avx __ocl_svml_s9_rsqrtf4 (float4_avx a);

  float4_avx __ocl_svml_s9_cbrtf4 (float4_avx a);

  float4_avx __ocl_svml_s9_rcbrtf4 (float4_avx a);

  float4_avx __ocl_svml_s9_hypotf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_powf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_powrf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_pownf4 (float4_avx a, int4_avx b);

  float4_avx __ocl_svml_s9_rootnf4 (float4_avx a, int4_avx b);

  float4_avx __ocl_svml_s9_expf4 (float4_avx a);

  float4_avx __ocl_svml_s9_exp2f4 (float4_avx a);

  float4_avx __ocl_svml_s9_exp10f4 (float4_avx a);

  float4_avx __ocl_svml_s9_expm1f4 (float4_avx a);

  float4_avx __ocl_svml_s9_logf4 (float4_avx a);

  float4_avx __ocl_svml_s9_log10f4 (float4_avx a);

  float4_avx __ocl_svml_s9_log2f4 (float4_avx a);

  float4_avx __ocl_svml_s9_log1pf4 (float4_avx a);

  float4_avx __ocl_svml_s9_sinf4 (float4_avx a);

  float4_avx __ocl_svml_s9_cosf4 (float4_avx a);

  float4_avx __ocl_svml_s9_sincosf4 (float4_avx a, float4_avx * c);

  float4x2_avx __ocl_svml_s9_sincosregf4 (float4_avx a);

  float4_avx __ocl_svml_s9_tanf4 (float4_avx a);

  float4_avx __ocl_svml_s9_sinpif4 (float4_avx a);

  float4_avx __ocl_svml_s9_cospif4 (float4_avx a);

  float4_avx __ocl_svml_s9_tanpif4 (float4_avx a);

  float4_avx __ocl_svml_s9_acosf4 (float4_avx a);

  float4_avx __ocl_svml_s9_asinf4 (float4_avx a);

  float4_avx __ocl_svml_s9_atanf4 (float4_avx a);

  float4_avx __ocl_svml_s9_atan2f4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_asinpif4 (float4_avx a);

  float4_avx __ocl_svml_s9_acospif4 (float4_avx a);

  float4_avx __ocl_svml_s9_atanpif4 (float4_avx a);

  float4_avx __ocl_svml_s9_atan2pif4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_sinhf4 (float4_avx a);

  float4_avx __ocl_svml_s9_coshf4 (float4_avx a);

  float4_avx __ocl_svml_s9_tanhf4 (float4_avx a);

  float4_avx __ocl_svml_s9_asinhf4 (float4_avx a);

  float4_avx __ocl_svml_s9_acoshf4 (float4_avx a);

  float4_avx __ocl_svml_s9_atanhf4 (float4_avx a);

  float4_avx __ocl_svml_s9_erff4 (float4_avx a);

  float4_avx __ocl_svml_s9_erfcf4 (float4_avx a);

  float4_sse __ocl_svml_n8_fmaf4 (float4_sse a, float4_sse b, float4_sse c);

  float4_sse __ocl_svml_n8_fabsf4 (float4_sse a);

  float4_sse __ocl_svml_n8_fminf4 (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_fmaxf4 (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_maxmagf4 (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_minmagf4 (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_nanf4 (int4_sse a);

  float4_sse __ocl_svml_n8_ceilf4 (float4_sse a);

  float4_sse __ocl_svml_n8_floorf4 (float4_sse a);

  float4_sse __ocl_svml_n8_roundf4 (float4_sse a);

  float4_sse __ocl_svml_n8_truncf4 (float4_sse a);

  float4_sse __ocl_svml_n8_rintf4 (float4_sse a);

  float4_sse __ocl_svml_n8_nearbyintf4 (float4_sse a);

  float4_sse __ocl_svml_n8_fmodf4 (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_modff4 (float4_sse a, float4_sse * c);

  float4_sse __ocl_svml_n8_remainderf4 (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_remquof4 (float4_sse a, float4_sse b, int4_sse * c);

  float4_sse __ocl_svml_n8_copysignf4 (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_frexpf4 (float4_sse a, int4_sse * c);

  float4_sse __ocl_svml_n8_fdimf4 (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_logbf4 (float4_sse a);

  int4_sse __ocl_svml_n8_ilogbf4 (float4_sse a);

  float4_sse __ocl_svml_n8_nextafterf4 (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_ldexpf4 (float4_sse a, int4_sse b);

  float4_sse __ocl_svml_n8_fractf4 (float4_sse a, float4_sse * c);

  float4_sse __ocl_svml_n8_tgammaf4 (float4_sse a);

  float4_sse __ocl_svml_n8_lgammaf4 (float4_sse a);

  float4_sse __ocl_svml_n8_lgammarf4 (float4_sse a, int4_sse * c);

  long4_sse __ocl_svml_n8_cvtfptou64rtenosatf4 (float4_sse a);

  long4_sse __ocl_svml_n8_cvtfptou64rtesatf4 (float4_sse a);

  long4_sse __ocl_svml_n8_cvtfptou64rtnnosatf4 (float4_sse a);

  long4_sse __ocl_svml_n8_cvtfptou64rtnsatf4 (float4_sse a);

  long4_sse __ocl_svml_n8_cvtfptou64rtpnosatf4 (float4_sse a);

  long4_sse __ocl_svml_n8_cvtfptou64rtpsatf4 (float4_sse a);

  long4_sse __ocl_svml_n8_cvtfptou64rtznosatf4 (float4_sse a);

  long4_sse __ocl_svml_n8_cvtfptou64rtzsatf4 (float4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtenosatf4 (float4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtesatf4 (float4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtnnosatf4 (float4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtnsatf4 (float4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtpnosatf4 (float4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtpsatf4 (float4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtznosatf4 (float4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtzsatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtenosatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtesatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtnnosatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtnsatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtpnosatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtpsatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtznosatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtzsatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtenosatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtesatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtnnosatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtnsatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtpnosatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtpsatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtznosatf4 (float4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtzsatf4 (float4_sse a);

  float4_sse __ocl_svml_n8_cvtu64tofprtef4 (long4_sse a);

  float4_sse __ocl_svml_n8_cvtu64tofprtnf4 (long4_sse a);

  float4_sse __ocl_svml_n8_cvtu64tofprtpf4 (long4_sse a);

  float4_sse __ocl_svml_n8_cvtu64tofprtzf4 (long4_sse a);

  float4_sse __ocl_svml_n8_cvti64tofprtef4 (long4_sse a);

  float4_sse __ocl_svml_n8_cvti64tofprtnf4 (long4_sse a);

  float4_sse __ocl_svml_n8_cvti64tofprtpf4 (long4_sse a);

  float4_sse __ocl_svml_n8_cvti64tofprtzf4 (long4_sse a);

  float4_sse __ocl_svml_n8_cvtu32tofprtef4 (int4_sse a);

  float4_sse __ocl_svml_n8_cvtu32tofprtnf4 (int4_sse a);

  float4_sse __ocl_svml_n8_cvtu32tofprtpf4 (int4_sse a);

  float4_sse __ocl_svml_n8_cvtu32tofprtzf4 (int4_sse a);

  float4_sse __ocl_svml_n8_cvti32tofprtef4 (int4_sse a);

  float4_sse __ocl_svml_n8_cvti32tofprtnf4 (int4_sse a);

  float4_sse __ocl_svml_n8_cvti32tofprtpf4 (int4_sse a);

  float4_sse __ocl_svml_n8_cvti32tofprtzf4 (int4_sse a);

  float4_avx __ocl_svml_g9_fmaf4 (float4_avx a, float4_avx b, float4_avx c);

  float4_avx __ocl_svml_g9_fabsf4 (float4_avx a);

  float4_avx __ocl_svml_g9_fminf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_fmaxf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_maxmagf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_minmagf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_nanf4 (int4_avx a);

  float4_avx __ocl_svml_g9_ceilf4 (float4_avx a);

  float4_avx __ocl_svml_g9_floorf4 (float4_avx a);

  float4_avx __ocl_svml_g9_roundf4 (float4_avx a);

  float4_avx __ocl_svml_g9_truncf4 (float4_avx a);

  float4_avx __ocl_svml_g9_rintf4 (float4_avx a);

  float4_avx __ocl_svml_g9_nearbyintf4 (float4_avx a);

  float4_avx __ocl_svml_g9_fmodf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_modff4 (float4_avx a, float4_avx * c);

  float4_avx __ocl_svml_g9_remainderf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_remquof4 (float4_avx a, float4_avx b, int4_avx * c);

  float4_avx __ocl_svml_g9_copysignf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_frexpf4 (float4_avx a, int4_avx * c);

  float4_avx __ocl_svml_g9_fdimf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_logbf4 (float4_avx a);

  int4_avx __ocl_svml_g9_ilogbf4 (float4_avx a);

  float4_avx __ocl_svml_g9_nextafterf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_ldexpf4 (float4_avx a, int4_avx b);

  float4_avx __ocl_svml_g9_fractf4 (float4_avx a, float4_avx * c);

  float4_avx __ocl_svml_g9_tgammaf4 (float4_avx a);

  float4_avx __ocl_svml_g9_lgammaf4 (float4_avx a);

  float4_avx __ocl_svml_g9_lgammarf4 (float4_avx a, int4_avx * c);

  long4_avx __ocl_svml_g9_cvtfptou64rtenosatf4 (float4_avx a);

  long4_avx __ocl_svml_g9_cvtfptou64rtesatf4 (float4_avx a);

  long4_avx __ocl_svml_g9_cvtfptou64rtnnosatf4 (float4_avx a);

  long4_avx __ocl_svml_g9_cvtfptou64rtnsatf4 (float4_avx a);

  long4_avx __ocl_svml_g9_cvtfptou64rtpnosatf4 (float4_avx a);

  long4_avx __ocl_svml_g9_cvtfptou64rtpsatf4 (float4_avx a);

  long4_avx __ocl_svml_g9_cvtfptou64rtznosatf4 (float4_avx a);

  long4_avx __ocl_svml_g9_cvtfptou64rtzsatf4 (float4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtenosatf4 (float4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtesatf4 (float4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtnnosatf4 (float4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtnsatf4 (float4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtpnosatf4 (float4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtpsatf4 (float4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtznosatf4 (float4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtzsatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtenosatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtesatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtnnosatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtnsatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtpnosatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtpsatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtznosatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtzsatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtenosatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtesatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtnnosatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtnsatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtpnosatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtpsatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtznosatf4 (float4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtzsatf4 (float4_avx a);

  float4_avx __ocl_svml_g9_cvtu64tofprtef4 (long4_avx a);

  float4_avx __ocl_svml_g9_cvtu64tofprtnf4 (long4_avx a);

  float4_avx __ocl_svml_g9_cvtu64tofprtpf4 (long4_avx a);

  float4_avx __ocl_svml_g9_cvtu64tofprtzf4 (long4_avx a);

  float4_avx __ocl_svml_g9_cvti64tofprtef4 (long4_avx a);

  float4_avx __ocl_svml_g9_cvti64tofprtnf4 (long4_avx a);

  float4_avx __ocl_svml_g9_cvti64tofprtpf4 (long4_avx a);

  float4_avx __ocl_svml_g9_cvti64tofprtzf4 (long4_avx a);

  float4_avx __ocl_svml_g9_cvtu32tofprtef4 (int4_avx a);

  float4_avx __ocl_svml_g9_cvtu32tofprtnf4 (int4_avx a);

  float4_avx __ocl_svml_g9_cvtu32tofprtpf4 (int4_avx a);

  float4_avx __ocl_svml_g9_cvtu32tofprtzf4 (int4_avx a);

  float4_avx __ocl_svml_g9_cvti32tofprtef4 (int4_avx a);

  float4_avx __ocl_svml_g9_cvti32tofprtnf4 (int4_avx a);

  float4_avx __ocl_svml_g9_cvti32tofprtpf4 (int4_avx a);

  float4_avx __ocl_svml_g9_cvti32tofprtzf4 (int4_avx a);

  float4_avx __ocl_svml_s9_fmaf4 (float4_avx a, float4_avx b, float4_avx c);

  float4_avx __ocl_svml_s9_fabsf4 (float4_avx a);

  float4_avx __ocl_svml_s9_fminf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_fmaxf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_maxmagf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_minmagf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_nanf4 (int4_avx a);

  float4_avx __ocl_svml_s9_ceilf4 (float4_avx a);

  float4_avx __ocl_svml_s9_floorf4 (float4_avx a);

  float4_avx __ocl_svml_s9_roundf4 (float4_avx a);

  float4_avx __ocl_svml_s9_truncf4 (float4_avx a);

  float4_avx __ocl_svml_s9_rintf4 (float4_avx a);

  float4_avx __ocl_svml_s9_nearbyintf4 (float4_avx a);

  float4_avx __ocl_svml_s9_fmodf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_modff4 (float4_avx a, float4_avx * c);

  float4_avx __ocl_svml_s9_remainderf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_remquof4 (float4_avx a, float4_avx b, int4_avx * c);

  float4_avx __ocl_svml_s9_copysignf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_frexpf4 (float4_avx a, int4_avx * c);

  float4_avx __ocl_svml_s9_fdimf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_logbf4 (float4_avx a);

  int4_avx __ocl_svml_s9_ilogbf4 (float4_avx a);

  float4_avx __ocl_svml_s9_nextafterf4 (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_ldexpf4 (float4_avx a, int4_avx b);

  float4_avx __ocl_svml_s9_fractf4 (float4_avx a, float4_avx * c);

  float4_avx __ocl_svml_s9_tgammaf4 (float4_avx a);

  float4_avx __ocl_svml_s9_lgammaf4 (float4_avx a);

  float4_avx __ocl_svml_s9_lgammarf4 (float4_avx a, int4_avx * c);

  long4_avx __ocl_svml_s9_cvtfptou64rtenosatf4 (float4_avx a);

  long4_avx __ocl_svml_s9_cvtfptou64rtesatf4 (float4_avx a);

  long4_avx __ocl_svml_s9_cvtfptou64rtnnosatf4 (float4_avx a);

  long4_avx __ocl_svml_s9_cvtfptou64rtnsatf4 (float4_avx a);

  long4_avx __ocl_svml_s9_cvtfptou64rtpnosatf4 (float4_avx a);

  long4_avx __ocl_svml_s9_cvtfptou64rtpsatf4 (float4_avx a);

  long4_avx __ocl_svml_s9_cvtfptou64rtznosatf4 (float4_avx a);

  long4_avx __ocl_svml_s9_cvtfptou64rtzsatf4 (float4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtenosatf4 (float4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtesatf4 (float4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtnnosatf4 (float4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtnsatf4 (float4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtpnosatf4 (float4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtpsatf4 (float4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtznosatf4 (float4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtzsatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtenosatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtesatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtnnosatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtnsatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtpnosatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtpsatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtznosatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtzsatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtenosatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtesatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtnnosatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtnsatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtpnosatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtpsatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtznosatf4 (float4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtzsatf4 (float4_avx a);

  float4_avx __ocl_svml_s9_cvtu64tofprtef4 (long4_avx a);

  float4_avx __ocl_svml_s9_cvtu64tofprtnf4 (long4_avx a);

  float4_avx __ocl_svml_s9_cvtu64tofprtpf4 (long4_avx a);

  float4_avx __ocl_svml_s9_cvtu64tofprtzf4 (long4_avx a);

  float4_avx __ocl_svml_s9_cvti64tofprtef4 (long4_avx a);

  float4_avx __ocl_svml_s9_cvti64tofprtnf4 (long4_avx a);

  float4_avx __ocl_svml_s9_cvti64tofprtpf4 (long4_avx a);

  float4_avx __ocl_svml_s9_cvti64tofprtzf4 (long4_avx a);

  float4_avx __ocl_svml_s9_cvtu32tofprtef4 (int4_avx a);

  float4_avx __ocl_svml_s9_cvtu32tofprtnf4 (int4_avx a);

  float4_avx __ocl_svml_s9_cvtu32tofprtpf4 (int4_avx a);

  float4_avx __ocl_svml_s9_cvtu32tofprtzf4 (int4_avx a);

  float4_avx __ocl_svml_s9_cvti32tofprtef4 (int4_avx a);

  float4_avx __ocl_svml_s9_cvti32tofprtnf4 (int4_avx a);

  float4_avx __ocl_svml_s9_cvti32tofprtpf4 (int4_avx a);

  float4_avx __ocl_svml_s9_cvti32tofprtzf4 (int4_avx a);

  double4_sse __ocl_svml_n8_inv4 (double4_sse a);

  double4_sse __ocl_svml_n8_div4 (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_sqrt4 (double4_sse a);

  double4_sse __ocl_svml_n8_rsqrt4 (double4_sse a);

  double4_sse __ocl_svml_n8_cbrt4 (double4_sse a);

  double4_sse __ocl_svml_n8_rcbrt4 (double4_sse a);

  double4_sse __ocl_svml_n8_hypot4 (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_pow4 (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_powr4 (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_pown4 (double4_sse a, int4_sse b);

  double4_sse __ocl_svml_n8_rootn4 (double4_sse a, int4_sse b);

  double4_sse __ocl_svml_n8_exp4 (double4_sse a);

  double4_sse __ocl_svml_n8_exp24 (double4_sse a);

  double4_sse __ocl_svml_n8_exp104 (double4_sse a);

  double4_sse __ocl_svml_n8_expm14 (double4_sse a);

  double4_sse __ocl_svml_n8_log4 (double4_sse a);

  double4_sse __ocl_svml_n8_log104 (double4_sse a);

  double4_sse __ocl_svml_n8_log24 (double4_sse a);

  double4_sse __ocl_svml_n8_log1p4 (double4_sse a);

  double4_sse __ocl_svml_n8_sin4 (double4_sse a);

  double4_sse __ocl_svml_n8_cos4 (double4_sse a);

  double4_sse __ocl_svml_n8_sincos4 (double4_sse a, double4_sse * c);

  double4x2_sse __ocl_svml_n8_sincosreg4 (double4_sse a);

  double4_sse __ocl_svml_n8_tan4 (double4_sse a);

  double4_sse __ocl_svml_n8_sinpi4 (double4_sse a);

  double4_sse __ocl_svml_n8_cospi4 (double4_sse a);

  double4_sse __ocl_svml_n8_tanpi4 (double4_sse a);

  double4_sse __ocl_svml_n8_acos4 (double4_sse a);

  double4_sse __ocl_svml_n8_asin4 (double4_sse a);

  double4_sse __ocl_svml_n8_atan4 (double4_sse a);

  double4_sse __ocl_svml_n8_atan24 (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_asinpi4 (double4_sse a);

  double4_sse __ocl_svml_n8_acospi4 (double4_sse a);

  double4_sse __ocl_svml_n8_atanpi4 (double4_sse a);

  double4_sse __ocl_svml_n8_atan2pi4 (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_sinh4 (double4_sse a);

  double4_sse __ocl_svml_n8_cosh4 (double4_sse a);

  double4_sse __ocl_svml_n8_tanh4 (double4_sse a);

  double4_sse __ocl_svml_n8_asinh4 (double4_sse a);

  double4_sse __ocl_svml_n8_acosh4 (double4_sse a);

  double4_sse __ocl_svml_n8_atanh4 (double4_sse a);

  double4_sse __ocl_svml_n8_erf4 (double4_sse a);

  double4_sse __ocl_svml_n8_erfc4 (double4_sse a);

  double4_avx __ocl_svml_g9_inv4 (double4_avx a);

  double4_avx __ocl_svml_g9_div4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_sqrt4 (double4_avx a);

  double4_avx __ocl_svml_g9_rsqrt4 (double4_avx a);

  double4_avx __ocl_svml_g9_cbrt4 (double4_avx a);

  double4_avx __ocl_svml_g9_rcbrt4 (double4_avx a);

  double4_avx __ocl_svml_g9_hypot4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_pow4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_powr4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_pown4 (double4_avx a, int4_avx b);

  double4_avx __ocl_svml_g9_rootn4 (double4_avx a, int4_avx b);

  double4_avx __ocl_svml_g9_exp4 (double4_avx a);

  double4_avx __ocl_svml_g9_exp24 (double4_avx a);

  double4_avx __ocl_svml_g9_exp104 (double4_avx a);

  double4_avx __ocl_svml_g9_expm14 (double4_avx a);

  double4_avx __ocl_svml_g9_log4 (double4_avx a);

  double4_avx __ocl_svml_g9_log104 (double4_avx a);

  double4_avx __ocl_svml_g9_log24 (double4_avx a);

  double4_avx __ocl_svml_g9_log1p4 (double4_avx a);

  double4_avx __ocl_svml_g9_sin4 (double4_avx a);

  double4_avx __ocl_svml_g9_cos4 (double4_avx a);

  double4_avx __ocl_svml_g9_sincos4 (double4_avx a, double4_avx * c);

  double4x2_avx __ocl_svml_g9_sincosreg4 (double4_avx a);

  double4_avx __ocl_svml_g9_tan4 (double4_avx a);

  double4_avx __ocl_svml_g9_sinpi4 (double4_avx a);

  double4_avx __ocl_svml_g9_cospi4 (double4_avx a);

  double4_avx __ocl_svml_g9_tanpi4 (double4_avx a);

  double4_avx __ocl_svml_g9_acos4 (double4_avx a);

  double4_avx __ocl_svml_g9_asin4 (double4_avx a);

  double4_avx __ocl_svml_g9_atan4 (double4_avx a);

  double4_avx __ocl_svml_g9_atan24 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_asinpi4 (double4_avx a);

  double4_avx __ocl_svml_g9_acospi4 (double4_avx a);

  double4_avx __ocl_svml_g9_atanpi4 (double4_avx a);

  double4_avx __ocl_svml_g9_atan2pi4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_sinh4 (double4_avx a);

  double4_avx __ocl_svml_g9_cosh4 (double4_avx a);

  double4_avx __ocl_svml_g9_tanh4 (double4_avx a);

  double4_avx __ocl_svml_g9_asinh4 (double4_avx a);

  double4_avx __ocl_svml_g9_acosh4 (double4_avx a);

  double4_avx __ocl_svml_g9_atanh4 (double4_avx a);

  double4_avx __ocl_svml_g9_erf4 (double4_avx a);

  double4_avx __ocl_svml_g9_erfc4 (double4_avx a);

  double4_avx __ocl_svml_s9_inv4 (double4_avx a);

  double4_avx __ocl_svml_s9_div4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_sqrt4 (double4_avx a);

  double4_avx __ocl_svml_s9_rsqrt4 (double4_avx a);

  double4_avx __ocl_svml_s9_cbrt4 (double4_avx a);

  double4_avx __ocl_svml_s9_rcbrt4 (double4_avx a);

  double4_avx __ocl_svml_s9_hypot4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_pow4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_powr4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_pown4 (double4_avx a, int4_avx b);

  double4_avx __ocl_svml_s9_rootn4 (double4_avx a, int4_avx b);

  double4_avx __ocl_svml_s9_exp4 (double4_avx a);

  double4_avx __ocl_svml_s9_exp24 (double4_avx a);

  double4_avx __ocl_svml_s9_exp104 (double4_avx a);

  double4_avx __ocl_svml_s9_expm14 (double4_avx a);

  double4_avx __ocl_svml_s9_log4 (double4_avx a);

  double4_avx __ocl_svml_s9_log104 (double4_avx a);

  double4_avx __ocl_svml_s9_log24 (double4_avx a);

  double4_avx __ocl_svml_s9_log1p4 (double4_avx a);

  double4_avx __ocl_svml_s9_sin4 (double4_avx a);

  double4_avx __ocl_svml_s9_cos4 (double4_avx a);

  double4_avx __ocl_svml_s9_sincos4 (double4_avx a, double4_avx * c);

  double4x2_avx __ocl_svml_s9_sincosreg4 (double4_avx a);

  double4_avx __ocl_svml_s9_tan4 (double4_avx a);

  double4_avx __ocl_svml_s9_sinpi4 (double4_avx a);

  double4_avx __ocl_svml_s9_cospi4 (double4_avx a);

  double4_avx __ocl_svml_s9_tanpi4 (double4_avx a);

  double4_avx __ocl_svml_s9_acos4 (double4_avx a);

  double4_avx __ocl_svml_s9_asin4 (double4_avx a);

  double4_avx __ocl_svml_s9_atan4 (double4_avx a);

  double4_avx __ocl_svml_s9_atan24 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_asinpi4 (double4_avx a);

  double4_avx __ocl_svml_s9_acospi4 (double4_avx a);

  double4_avx __ocl_svml_s9_atanpi4 (double4_avx a);

  double4_avx __ocl_svml_s9_atan2pi4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_sinh4 (double4_avx a);

  double4_avx __ocl_svml_s9_cosh4 (double4_avx a);

  double4_avx __ocl_svml_s9_tanh4 (double4_avx a);

  double4_avx __ocl_svml_s9_asinh4 (double4_avx a);

  double4_avx __ocl_svml_s9_acosh4 (double4_avx a);

  double4_avx __ocl_svml_s9_atanh4 (double4_avx a);

  double4_avx __ocl_svml_s9_erf4 (double4_avx a);

  double4_avx __ocl_svml_s9_erfc4 (double4_avx a);

  double4_sse __ocl_svml_n8_fabs4 (double4_sse a);

  double4_sse __ocl_svml_n8_fmin4 (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_fmax4 (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_maxmag4 (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_minmag4 (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_nan4 (long4_sse a);

  double4_sse __ocl_svml_n8_ceil4 (double4_sse a);

  double4_sse __ocl_svml_n8_floor4 (double4_sse a);

  double4_sse __ocl_svml_n8_round4 (double4_sse a);

  double4_sse __ocl_svml_n8_trunc4 (double4_sse a);

  double4_sse __ocl_svml_n8_rint4 (double4_sse a);

  double4_sse __ocl_svml_n8_nearbyint4 (double4_sse a);

  double4_sse __ocl_svml_n8_fmod4 (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_modf4 (double4_sse a, double4_sse * c);

  double4_sse __ocl_svml_n8_remainder4 (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_remquo4 (double4_sse a, double4_sse b, int4_sse * c);

  double4_sse __ocl_svml_n8_copysign4 (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_frexp4 (double4_sse a, int4_sse * c);

  double4_sse __ocl_svml_n8_fdim4 (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_logb4 (double4_sse a);

  int4_sse __ocl_svml_n8_ilogb4 (double4_sse a);

  double4_sse __ocl_svml_n8_nextafter4 (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_ldexp4 (double4_sse a, int4_sse b);

  double4_sse __ocl_svml_n8_fract4 (double4_sse a, double4_sse * c);

  double4_sse __ocl_svml_n8_tgamma4 (double4_sse a);

  double4_sse __ocl_svml_n8_lgamma4 (double4_sse a);

  double4_sse __ocl_svml_n8_lgammar4 (double4_sse a, int4_sse * c);

  long4_sse __ocl_svml_n8_cvtfptou64rtenosat4 (double4_sse a);

  long4_sse __ocl_svml_n8_cvtfptou64rtesat4 (double4_sse a);

  long4_sse __ocl_svml_n8_cvtfptou64rtnnosat4 (double4_sse a);

  long4_sse __ocl_svml_n8_cvtfptou64rtnsat4 (double4_sse a);

  long4_sse __ocl_svml_n8_cvtfptou64rtpnosat4 (double4_sse a);

  long4_sse __ocl_svml_n8_cvtfptou64rtpsat4 (double4_sse a);

  long4_sse __ocl_svml_n8_cvtfptou64rtznosat4 (double4_sse a);

  long4_sse __ocl_svml_n8_cvtfptou64rtzsat4 (double4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtenosat4 (double4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtesat4 (double4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtnnosat4 (double4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtnsat4 (double4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtpnosat4 (double4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtpsat4 (double4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtznosat4 (double4_sse a);

  long4_sse __ocl_svml_n8_cvtfptoi64rtzsat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtenosat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtesat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtnnosat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtnsat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtpnosat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtpsat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtznosat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptou32rtzsat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtenosat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtesat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtnnosat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtnsat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtpnosat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtpsat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtznosat4 (double4_sse a);

  int4_sse __ocl_svml_n8_cvtfptoi32rtzsat4 (double4_sse a);

  double4_sse __ocl_svml_n8_cvtu64tofprte4 (long4_sse a);

  double4_sse __ocl_svml_n8_cvtu64tofprtn4 (long4_sse a);

  double4_sse __ocl_svml_n8_cvtu64tofprtp4 (long4_sse a);

  double4_sse __ocl_svml_n8_cvtu64tofprtz4 (long4_sse a);

  double4_sse __ocl_svml_n8_cvti64tofprte4 (long4_sse a);

  double4_sse __ocl_svml_n8_cvti64tofprtn4 (long4_sse a);

  double4_sse __ocl_svml_n8_cvti64tofprtp4 (long4_sse a);

  double4_sse __ocl_svml_n8_cvti64tofprtz4 (long4_sse a);

  double4_sse __ocl_svml_n8_cvtu32tofprte4 (int4_sse a);

  double4_sse __ocl_svml_n8_cvtu32tofprtn4 (int4_sse a);

  double4_sse __ocl_svml_n8_cvtu32tofprtp4 (int4_sse a);

  double4_sse __ocl_svml_n8_cvtu32tofprtz4 (int4_sse a);

  double4_sse __ocl_svml_n8_cvti32tofprte4 (int4_sse a);

  double4_sse __ocl_svml_n8_cvti32tofprtn4 (int4_sse a);

  double4_sse __ocl_svml_n8_cvti32tofprtp4 (int4_sse a);

  double4_sse __ocl_svml_n8_cvti32tofprtz4 (int4_sse a);

  float4_sse __ocl_svml_n8_cvtfp64tofp32rte4 (double4_sse a);

  float4_sse __ocl_svml_n8_cvtfp64tofp32rtn4 (double4_sse a);

  float4_sse __ocl_svml_n8_cvtfp64tofp32rtp4 (double4_sse a);

  float4_sse __ocl_svml_n8_cvtfp64tofp32rtz4 (double4_sse a);

  double4_avx __ocl_svml_g9_fma4 (double4_avx a, double4_avx b, double4_avx c);

  double4_avx __ocl_svml_g9_fabs4 (double4_avx a);

  double4_avx __ocl_svml_g9_fmin4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_fmax4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_maxmag4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_minmag4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_nan4 (long4_avx a);

  double4_avx __ocl_svml_g9_ceil4 (double4_avx a);

  double4_avx __ocl_svml_g9_floor4 (double4_avx a);

  double4_avx __ocl_svml_g9_round4 (double4_avx a);

  double4_avx __ocl_svml_g9_trunc4 (double4_avx a);

  double4_avx __ocl_svml_g9_rint4 (double4_avx a);

  double4_avx __ocl_svml_g9_nearbyint4 (double4_avx a);

  double4_avx __ocl_svml_g9_fmod4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_modf4 (double4_avx a, double4_avx * c);

  double4_avx __ocl_svml_g9_remainder4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_remquo4 (double4_avx a, double4_avx b, int4_avx * c);

  double4_avx __ocl_svml_g9_copysign4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_frexp4 (double4_avx a, int4_avx * c);

  double4_avx __ocl_svml_g9_fdim4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_logb4 (double4_avx a);

  int4_avx __ocl_svml_g9_ilogb4 (double4_avx a);

  double4_avx __ocl_svml_g9_nextafter4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_ldexp4 (double4_avx a, int4_avx b);

  double4_avx __ocl_svml_g9_fract4 (double4_avx a, double4_avx * c);

  double4_avx __ocl_svml_g9_tgamma4 (double4_avx a);

  double4_avx __ocl_svml_g9_lgamma4 (double4_avx a);

  double4_avx __ocl_svml_g9_lgammar4 (double4_avx a, int4_avx * c);

  long4_avx __ocl_svml_g9_cvtfptou64rtenosat4 (double4_avx a);

  long4_avx __ocl_svml_g9_cvtfptou64rtesat4 (double4_avx a);

  long4_avx __ocl_svml_g9_cvtfptou64rtnnosat4 (double4_avx a);

  long4_avx __ocl_svml_g9_cvtfptou64rtnsat4 (double4_avx a);

  long4_avx __ocl_svml_g9_cvtfptou64rtpnosat4 (double4_avx a);

  long4_avx __ocl_svml_g9_cvtfptou64rtpsat4 (double4_avx a);

  long4_avx __ocl_svml_g9_cvtfptou64rtznosat4 (double4_avx a);

  long4_avx __ocl_svml_g9_cvtfptou64rtzsat4 (double4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtenosat4 (double4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtesat4 (double4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtnnosat4 (double4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtnsat4 (double4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtpnosat4 (double4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtpsat4 (double4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtznosat4 (double4_avx a);

  long4_avx __ocl_svml_g9_cvtfptoi64rtzsat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtenosat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtesat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtnnosat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtnsat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtpnosat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtpsat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtznosat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptou32rtzsat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtenosat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtesat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtnnosat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtnsat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtpnosat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtpsat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtznosat4 (double4_avx a);

  int4_avx __ocl_svml_g9_cvtfptoi32rtzsat4 (double4_avx a);

  double4_avx __ocl_svml_g9_cvtu64tofprte4 (long4_avx a);

  double4_avx __ocl_svml_g9_cvtu64tofprtn4 (long4_avx a);

  double4_avx __ocl_svml_g9_cvtu64tofprtp4 (long4_avx a);

  double4_avx __ocl_svml_g9_cvtu64tofprtz4 (long4_avx a);

  double4_avx __ocl_svml_g9_cvti64tofprte4 (long4_avx a);

  double4_avx __ocl_svml_g9_cvti64tofprtn4 (long4_avx a);

  double4_avx __ocl_svml_g9_cvti64tofprtp4 (long4_avx a);

  double4_avx __ocl_svml_g9_cvti64tofprtz4 (long4_avx a);

  double4_avx __ocl_svml_g9_cvtu32tofprte4 (int4_avx a);

  double4_avx __ocl_svml_g9_cvtu32tofprtn4 (int4_avx a);

  double4_avx __ocl_svml_g9_cvtu32tofprtp4 (int4_avx a);

  double4_avx __ocl_svml_g9_cvtu32tofprtz4 (int4_avx a);

  double4_avx __ocl_svml_g9_cvti32tofprte4 (int4_avx a);

  double4_avx __ocl_svml_g9_cvti32tofprtn4 (int4_avx a);

  double4_avx __ocl_svml_g9_cvti32tofprtp4 (int4_avx a);

  double4_avx __ocl_svml_g9_cvti32tofprtz4 (int4_avx a);

  float4_avx __ocl_svml_g9_cvtfp64tofp32rte4 (double4_avx a);

  float4_avx __ocl_svml_g9_cvtfp64tofp32rtn4 (double4_avx a);

  float4_avx __ocl_svml_g9_cvtfp64tofp32rtp4 (double4_avx a);

  float4_avx __ocl_svml_g9_cvtfp64tofp32rtz4 (double4_avx a);

  double4_avx __ocl_svml_s9_fma4 (double4_avx a, double4_avx b, double4_avx c);

  double4_avx __ocl_svml_s9_fabs4 (double4_avx a);

  double4_avx __ocl_svml_s9_fmin4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_fmax4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_maxmag4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_minmag4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_nan4 (long4_avx a);

  double4_avx __ocl_svml_s9_ceil4 (double4_avx a);

  double4_avx __ocl_svml_s9_floor4 (double4_avx a);

  double4_avx __ocl_svml_s9_round4 (double4_avx a);

  double4_avx __ocl_svml_s9_trunc4 (double4_avx a);

  double4_avx __ocl_svml_s9_rint4 (double4_avx a);

  double4_avx __ocl_svml_s9_nearbyint4 (double4_avx a);

  double4_avx __ocl_svml_s9_fmod4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_modf4 (double4_avx a, double4_avx * c);

  double4_avx __ocl_svml_s9_remainder4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_remquo4 (double4_avx a, double4_avx b, int4_avx * c);

  double4_avx __ocl_svml_s9_copysign4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_frexp4 (double4_avx a, int4_avx * c);

  double4_avx __ocl_svml_s9_fdim4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_logb4 (double4_avx a);

  int4_avx __ocl_svml_s9_ilogb4 (double4_avx a);

  double4_avx __ocl_svml_s9_nextafter4 (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_ldexp4 (double4_avx a, int4_avx b);

  double4_avx __ocl_svml_s9_fract4 (double4_avx a, double4_avx * c);

  double4_avx __ocl_svml_s9_tgamma4 (double4_avx a);

  double4_avx __ocl_svml_s9_lgamma4 (double4_avx a);

  double4_avx __ocl_svml_s9_lgammar4 (double4_avx a, int4_avx * c);

  long4_avx __ocl_svml_s9_cvtfptou64rtenosat4 (double4_avx a);

  long4_avx __ocl_svml_s9_cvtfptou64rtesat4 (double4_avx a);

  long4_avx __ocl_svml_s9_cvtfptou64rtnnosat4 (double4_avx a);

  long4_avx __ocl_svml_s9_cvtfptou64rtnsat4 (double4_avx a);

  long4_avx __ocl_svml_s9_cvtfptou64rtpnosat4 (double4_avx a);

  long4_avx __ocl_svml_s9_cvtfptou64rtpsat4 (double4_avx a);

  long4_avx __ocl_svml_s9_cvtfptou64rtznosat4 (double4_avx a);

  long4_avx __ocl_svml_s9_cvtfptou64rtzsat4 (double4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtenosat4 (double4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtesat4 (double4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtnnosat4 (double4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtnsat4 (double4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtpnosat4 (double4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtpsat4 (double4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtznosat4 (double4_avx a);

  long4_avx __ocl_svml_s9_cvtfptoi64rtzsat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtenosat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtesat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtnnosat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtnsat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtpnosat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtpsat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtznosat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptou32rtzsat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtenosat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtesat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtnnosat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtnsat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtpnosat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtpsat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtznosat4 (double4_avx a);

  int4_avx __ocl_svml_s9_cvtfptoi32rtzsat4 (double4_avx a);

  double4_avx __ocl_svml_s9_cvtu64tofprte4 (long4_avx a);

  double4_avx __ocl_svml_s9_cvtu64tofprtn4 (long4_avx a);

  double4_avx __ocl_svml_s9_cvtu64tofprtp4 (long4_avx a);

  double4_avx __ocl_svml_s9_cvtu64tofprtz4 (long4_avx a);

  double4_avx __ocl_svml_s9_cvti64tofprte4 (long4_avx a);

  double4_avx __ocl_svml_s9_cvti64tofprtn4 (long4_avx a);

  double4_avx __ocl_svml_s9_cvti64tofprtp4 (long4_avx a);

  double4_avx __ocl_svml_s9_cvti64tofprtz4 (long4_avx a);

  double4_avx __ocl_svml_s9_cvtu32tofprte4 (int4_avx a);

  double4_avx __ocl_svml_s9_cvtu32tofprtn4 (int4_avx a);

  double4_avx __ocl_svml_s9_cvtu32tofprtp4 (int4_avx a);

  double4_avx __ocl_svml_s9_cvtu32tofprtz4 (int4_avx a);

  double4_avx __ocl_svml_s9_cvti32tofprte4 (int4_avx a);

  double4_avx __ocl_svml_s9_cvti32tofprtn4 (int4_avx a);

  double4_avx __ocl_svml_s9_cvti32tofprtp4 (int4_avx a);

  double4_avx __ocl_svml_s9_cvti32tofprtz4 (int4_avx a);

  float4_avx __ocl_svml_s9_cvtfp64tofp32rte4 (double4_avx a);

  float4_avx __ocl_svml_s9_cvtfp64tofp32rtn4 (double4_avx a);

  float4_avx __ocl_svml_s9_cvtfp64tofp32rtp4 (double4_avx a);

  float4_avx __ocl_svml_s9_cvtfp64tofp32rtz4 (double4_avx a);

  int4_sse __ocl_svml_n8_idiv4 (int4_sse a, int4_sse b);

  int4_sse __ocl_svml_n8_irem4 (int4_sse a, int4_sse b);

  int4x2_sse __ocl_svml_n8_idivrem4 (int4_sse a, int4_sse b);

  int4_avx __ocl_svml_g9_idiv4 (int4_avx a, int4_avx b);

  int4_avx __ocl_svml_g9_irem4 (int4_avx a, int4_avx b);

  int4x2_avx __ocl_svml_g9_idivrem4 (int4_avx a, int4_avx b);

  int4_avx __ocl_svml_s9_idiv4 (int4_avx a, int4_avx b);

  int4_avx __ocl_svml_s9_irem4 (int4_avx a, int4_avx b);

  int4x2_avx __ocl_svml_s9_idivrem4 (int4_avx a, int4_avx b);

  int4_sse __ocl_svml_n8_udiv4 (int4_sse a, int4_sse b);

  int4_sse __ocl_svml_n8_urem4 (int4_sse a, int4_sse b);

  int4x2_sse __ocl_svml_n8_udivrem4 (int4_sse a, int4_sse b);

  int4_avx __ocl_svml_g9_udiv4 (int4_avx a, int4_avx b);

  int4_avx __ocl_svml_g9_urem4 (int4_avx a, int4_avx b);

  int4x2_avx __ocl_svml_g9_udivrem4 (int4_avx a, int4_avx b);

  int4_avx __ocl_svml_s9_udiv4 (int4_avx a, int4_avx b);

  int4_avx __ocl_svml_s9_urem4 (int4_avx a, int4_avx b);

  int4x2_avx __ocl_svml_s9_udivrem4 (int4_avx a, int4_avx b);

  float8_sse __ocl_svml_n8_invf8 (float8_sse a);

  float8_sse __ocl_svml_n8_divf8 (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_sqrtf8 (float8_sse a);

  float8_sse __ocl_svml_n8_rsqrtf8 (float8_sse a);

  float8_sse __ocl_svml_n8_cbrtf8 (float8_sse a);

  float8_sse __ocl_svml_n8_rcbrtf8 (float8_sse a);

  float8_sse __ocl_svml_n8_hypotf8 (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_powf8 (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_powrf8 (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_pownf8 (float8_sse a, int8_sse b);

  float8_sse __ocl_svml_n8_rootnf8 (float8_sse a, int8_sse b);

  float8_sse __ocl_svml_n8_expf8 (float8_sse a);

  float8_sse __ocl_svml_n8_exp2f8 (float8_sse a);

  float8_sse __ocl_svml_n8_exp10f8 (float8_sse a);

  float8_sse __ocl_svml_n8_expm1f8 (float8_sse a);

  float8_sse __ocl_svml_n8_logf8 (float8_sse a);

  float8_sse __ocl_svml_n8_log10f8 (float8_sse a);

  float8_sse __ocl_svml_n8_log2f8 (float8_sse a);

  float8_sse __ocl_svml_n8_log1pf8 (float8_sse a);

  float8_sse __ocl_svml_n8_sinf8 (float8_sse a);

  float8_sse __ocl_svml_n8_cosf8 (float8_sse a);

  float8_sse __ocl_svml_n8_sincosf8 (float8_sse a, float8_sse * c);

  float8x2_sse __ocl_svml_n8_sincosregf8 (float8_sse a);

  float8_sse __ocl_svml_n8_tanf8 (float8_sse a);

  float8_sse __ocl_svml_n8_sinpif8 (float8_sse a);

  float8_sse __ocl_svml_n8_cospif8 (float8_sse a);

  float8_sse __ocl_svml_n8_tanpif8 (float8_sse a);

  float8_sse __ocl_svml_n8_acosf8 (float8_sse a);

  float8_sse __ocl_svml_n8_asinf8 (float8_sse a);

  float8_sse __ocl_svml_n8_atanf8 (float8_sse a);

  float8_sse __ocl_svml_n8_atan2f8 (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_asinpif8 (float8_sse a);

  float8_sse __ocl_svml_n8_acospif8 (float8_sse a);

  float8_sse __ocl_svml_n8_atanpif8 (float8_sse a);

  float8_sse __ocl_svml_n8_atan2pif8 (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_sinhf8 (float8_sse a);

  float8_sse __ocl_svml_n8_coshf8 (float8_sse a);

  float8_sse __ocl_svml_n8_tanhf8 (float8_sse a);

  float8_sse __ocl_svml_n8_asinhf8 (float8_sse a);

  float8_sse __ocl_svml_n8_acoshf8 (float8_sse a);

  float8_sse __ocl_svml_n8_atanhf8 (float8_sse a);

  float8_sse __ocl_svml_n8_erff8 (float8_sse a);

  float8_sse __ocl_svml_n8_erfcf8 (float8_sse a);

  float8_avx __ocl_svml_g9_invf8 (float8_avx a);

  float8_avx __ocl_svml_g9_divf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_sqrtf8 (float8_avx a);

  float8_avx __ocl_svml_g9_rsqrtf8 (float8_avx a);

  float8_avx __ocl_svml_g9_cbrtf8 (float8_avx a);

  float8_avx __ocl_svml_g9_rcbrtf8 (float8_avx a);

  float8_avx __ocl_svml_g9_hypotf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_powf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_powrf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_pownf8 (float8_avx a, int8_avx b);

  float8_avx __ocl_svml_g9_rootnf8 (float8_avx a, int8_avx b);

  float8_avx __ocl_svml_g9_expf8 (float8_avx a);

  float8_avx __ocl_svml_g9_exp2f8 (float8_avx a);

  float8_avx __ocl_svml_g9_exp10f8 (float8_avx a);

  float8_avx __ocl_svml_g9_expm1f8 (float8_avx a);

  float8_avx __ocl_svml_g9_logf8 (float8_avx a);

  float8_avx __ocl_svml_g9_log10f8 (float8_avx a);

  float8_avx __ocl_svml_g9_log2f8 (float8_avx a);

  float8_avx __ocl_svml_g9_log1pf8 (float8_avx a);

  float8_avx __ocl_svml_g9_sinf8 (float8_avx a);

  float8_avx __ocl_svml_g9_cosf8 (float8_avx a);

  float8_avx __ocl_svml_g9_sincosf8 (float8_avx a, float8_avx * c);

  float8x2_avx __ocl_svml_g9_sincosregf8 (float8_avx a);

  float8_avx __ocl_svml_g9_tanf8 (float8_avx a);

  float8_avx __ocl_svml_g9_sinpif8 (float8_avx a);

  float8_avx __ocl_svml_g9_cospif8 (float8_avx a);

  float8_avx __ocl_svml_g9_tanpif8 (float8_avx a);

  float8_avx __ocl_svml_g9_acosf8 (float8_avx a);

  float8_avx __ocl_svml_g9_asinf8 (float8_avx a);

  float8_avx __ocl_svml_g9_atanf8 (float8_avx a);

  float8_avx __ocl_svml_g9_atan2f8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_asinpif8 (float8_avx a);

  float8_avx __ocl_svml_g9_acospif8 (float8_avx a);

  float8_avx __ocl_svml_g9_atanpif8 (float8_avx a);

  float8_avx __ocl_svml_g9_atan2pif8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_sinhf8 (float8_avx a);

  float8_avx __ocl_svml_g9_coshf8 (float8_avx a);

  float8_avx __ocl_svml_g9_tanhf8 (float8_avx a);

  float8_avx __ocl_svml_g9_asinhf8 (float8_avx a);

  float8_avx __ocl_svml_g9_acoshf8 (float8_avx a);

  float8_avx __ocl_svml_g9_atanhf8 (float8_avx a);

  float8_avx __ocl_svml_g9_erff8 (float8_avx a);

  float8_avx __ocl_svml_g9_erfcf8 (float8_avx a);

  float8_avx __ocl_svml_s9_invf8 (float8_avx a);

  float8_avx __ocl_svml_s9_divf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_sqrtf8 (float8_avx a);

  float8_avx __ocl_svml_s9_rsqrtf8 (float8_avx a);

  float8_avx __ocl_svml_s9_cbrtf8 (float8_avx a);

  float8_avx __ocl_svml_s9_rcbrtf8 (float8_avx a);

  float8_avx __ocl_svml_s9_hypotf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_powf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_powrf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_pownf8 (float8_avx a, int8_avx b);

  float8_avx __ocl_svml_s9_rootnf8 (float8_avx a, int8_avx b);

  float8_avx __ocl_svml_s9_expf8 (float8_avx a);

  float8_avx __ocl_svml_s9_exp2f8 (float8_avx a);

  float8_avx __ocl_svml_s9_exp10f8 (float8_avx a);

  float8_avx __ocl_svml_s9_expm1f8 (float8_avx a);

  float8_avx __ocl_svml_s9_logf8 (float8_avx a);

  float8_avx __ocl_svml_s9_log10f8 (float8_avx a);

  float8_avx __ocl_svml_s9_log2f8 (float8_avx a);

  float8_avx __ocl_svml_s9_log1pf8 (float8_avx a);

  float8_avx __ocl_svml_s9_sinf8 (float8_avx a);

  float8_avx __ocl_svml_s9_cosf8 (float8_avx a);

  float8_avx __ocl_svml_s9_sincosf8 (float8_avx a, float8_avx * c);

  float8x2_avx __ocl_svml_s9_sincosregf8 (float8_avx a);

  float8_avx __ocl_svml_s9_tanf8 (float8_avx a);

  float8_avx __ocl_svml_s9_sinpif8 (float8_avx a);

  float8_avx __ocl_svml_s9_cospif8 (float8_avx a);

  float8_avx __ocl_svml_s9_tanpif8 (float8_avx a);

  float8_avx __ocl_svml_s9_acosf8 (float8_avx a);

  float8_avx __ocl_svml_s9_asinf8 (float8_avx a);

  float8_avx __ocl_svml_s9_atanf8 (float8_avx a);

  float8_avx __ocl_svml_s9_atan2f8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_asinpif8 (float8_avx a);

  float8_avx __ocl_svml_s9_acospif8 (float8_avx a);

  float8_avx __ocl_svml_s9_atanpif8 (float8_avx a);

  float8_avx __ocl_svml_s9_atan2pif8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_sinhf8 (float8_avx a);

  float8_avx __ocl_svml_s9_coshf8 (float8_avx a);

  float8_avx __ocl_svml_s9_tanhf8 (float8_avx a);

  float8_avx __ocl_svml_s9_asinhf8 (float8_avx a);

  float8_avx __ocl_svml_s9_acoshf8 (float8_avx a);

  float8_avx __ocl_svml_s9_atanhf8 (float8_avx a);

  float8_avx __ocl_svml_s9_erff8 (float8_avx a);

  float8_avx __ocl_svml_s9_erfcf8 (float8_avx a);

  float8_sse __ocl_svml_n8_fabsf8 (float8_sse a);

  float8_sse __ocl_svml_n8_fminf8 (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_fmaxf8 (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_maxmagf8 (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_minmagf8 (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_nanf8 (int8_sse a);

  float8_sse __ocl_svml_n8_ceilf8 (float8_sse a);

  float8_sse __ocl_svml_n8_floorf8 (float8_sse a);

  float8_sse __ocl_svml_n8_roundf8 (float8_sse a);

  float8_sse __ocl_svml_n8_truncf8 (float8_sse a);

  float8_sse __ocl_svml_n8_rintf8 (float8_sse a);

  float8_sse __ocl_svml_n8_nearbyintf8 (float8_sse a);

  float8_sse __ocl_svml_n8_fmodf8 (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_modff8 (float8_sse a, float8_sse * c);

  float8_sse __ocl_svml_n8_remainderf8 (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_remquof8 (float8_sse a, float8_sse b, int8_sse * c);

  float8_sse __ocl_svml_n8_copysignf8 (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_frexpf8 (float8_sse a, int8_sse * c);

  float8_sse __ocl_svml_n8_fdimf8 (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_logbf8 (float8_sse a);

  int8_sse __ocl_svml_n8_ilogbf8 (float8_sse a);

  float8_sse __ocl_svml_n8_nextafterf8 (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_ldexpf8 (float8_sse a, int8_sse b);

  float8_sse __ocl_svml_n8_fractf8 (float8_sse a, float8_sse * c);

  float8_sse __ocl_svml_n8_tgammaf8 (float8_sse a);

  float8_sse __ocl_svml_n8_lgammaf8 (float8_sse a);

  float8_sse __ocl_svml_n8_lgammarf8 (float8_sse a, int8_sse * c);

  long8_sse __ocl_svml_n8_cvtfptou64rtenosatf8 (float8_sse a);

  long8_sse __ocl_svml_n8_cvtfptou64rtesatf8 (float8_sse a);

  long8_sse __ocl_svml_n8_cvtfptou64rtnnosatf8 (float8_sse a);

  long8_sse __ocl_svml_n8_cvtfptou64rtnsatf8 (float8_sse a);

  long8_sse __ocl_svml_n8_cvtfptou64rtpnosatf8 (float8_sse a);

  long8_sse __ocl_svml_n8_cvtfptou64rtpsatf8 (float8_sse a);

  long8_sse __ocl_svml_n8_cvtfptou64rtznosatf8 (float8_sse a);

  long8_sse __ocl_svml_n8_cvtfptou64rtzsatf8 (float8_sse a);

  long8_sse __ocl_svml_n8_cvtfptoi64rtenosatf8 (float8_sse a);

  long8_sse __ocl_svml_n8_cvtfptoi64rtesatf8 (float8_sse a);

  long8_sse __ocl_svml_n8_cvtfptoi64rtnnosatf8 (float8_sse a);

  long8_sse __ocl_svml_n8_cvtfptoi64rtnsatf8 (float8_sse a);

  long8_sse __ocl_svml_n8_cvtfptoi64rtpnosatf8 (float8_sse a);

  long8_sse __ocl_svml_n8_cvtfptoi64rtpsatf8 (float8_sse a);

  long8_sse __ocl_svml_n8_cvtfptoi64rtznosatf8 (float8_sse a);

  long8_sse __ocl_svml_n8_cvtfptoi64rtzsatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptou32rtenosatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptou32rtesatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptou32rtnnosatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptou32rtnsatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptou32rtpnosatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptou32rtpsatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptou32rtznosatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptou32rtzsatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptoi32rtenosatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptoi32rtesatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptoi32rtnnosatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptoi32rtnsatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptoi32rtpnosatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptoi32rtpsatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptoi32rtznosatf8 (float8_sse a);

  int8_sse __ocl_svml_n8_cvtfptoi32rtzsatf8 (float8_sse a);

  float8_sse __ocl_svml_n8_cvtu64tofprtef8 (long8_sse a);

  float8_sse __ocl_svml_n8_cvtu64tofprtnf8 (long8_sse a);

  float8_sse __ocl_svml_n8_cvtu64tofprtpf8 (long8_sse a);

  float8_sse __ocl_svml_n8_cvtu64tofprtzf8 (long8_sse a);

  float8_sse __ocl_svml_n8_cvti64tofprtef8 (long8_sse a);

  float8_sse __ocl_svml_n8_cvti64tofprtnf8 (long8_sse a);

  float8_sse __ocl_svml_n8_cvti64tofprtpf8 (long8_sse a);

  float8_sse __ocl_svml_n8_cvti64tofprtzf8 (long8_sse a);

  float8_sse __ocl_svml_n8_cvtu32tofprtef8 (int8_sse a);

  float8_sse __ocl_svml_n8_cvtu32tofprtnf8 (int8_sse a);

  float8_sse __ocl_svml_n8_cvtu32tofprtpf8 (int8_sse a);

  float8_sse __ocl_svml_n8_cvtu32tofprtzf8 (int8_sse a);

  float8_sse __ocl_svml_n8_cvti32tofprtef8 (int8_sse a);

  float8_sse __ocl_svml_n8_cvti32tofprtnf8 (int8_sse a);

  float8_sse __ocl_svml_n8_cvti32tofprtpf8 (int8_sse a);

  float8_sse __ocl_svml_n8_cvti32tofprtzf8 (int8_sse a);

  float8_avx __ocl_svml_g9_fmaf8 (float8_avx a, float8_avx b, float8_avx c);

  float8_avx __ocl_svml_g9_fabsf8 (float8_avx a);

  float8_avx __ocl_svml_g9_fminf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_fmaxf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_maxmagf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_minmagf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_nanf8 (int8_avx a);

  float8_avx __ocl_svml_g9_ceilf8 (float8_avx a);

  float8_avx __ocl_svml_g9_floorf8 (float8_avx a);

  float8_avx __ocl_svml_g9_roundf8 (float8_avx a);

  float8_avx __ocl_svml_g9_truncf8 (float8_avx a);

  float8_avx __ocl_svml_g9_rintf8 (float8_avx a);

  float8_avx __ocl_svml_g9_nearbyintf8 (float8_avx a);

  float8_avx __ocl_svml_g9_fmodf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_modff8 (float8_avx a, float8_avx * c);

  float8_avx __ocl_svml_g9_remainderf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_remquof8 (float8_avx a, float8_avx b, int8_avx * c);

  float8_avx __ocl_svml_g9_copysignf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_frexpf8 (float8_avx a, int8_avx * c);

  float8_avx __ocl_svml_g9_fdimf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_logbf8 (float8_avx a);

  int8_avx __ocl_svml_g9_ilogbf8 (float8_avx a);

  float8_avx __ocl_svml_g9_nextafterf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_ldexpf8 (float8_avx a, int8_avx b);

  float8_avx __ocl_svml_g9_fractf8 (float8_avx a, float8_avx * c);

  float8_avx __ocl_svml_g9_tgammaf8 (float8_avx a);

  float8_avx __ocl_svml_g9_lgammaf8 (float8_avx a);

  float8_avx __ocl_svml_g9_lgammarf8 (float8_avx a, int8_avx * c);

  long8_avx __ocl_svml_g9_cvtfptou64rtenosatf8 (float8_avx a);

  long8_avx __ocl_svml_g9_cvtfptou64rtesatf8 (float8_avx a);

  long8_avx __ocl_svml_g9_cvtfptou64rtnnosatf8 (float8_avx a);

  long8_avx __ocl_svml_g9_cvtfptou64rtnsatf8 (float8_avx a);

  long8_avx __ocl_svml_g9_cvtfptou64rtpnosatf8 (float8_avx a);

  long8_avx __ocl_svml_g9_cvtfptou64rtpsatf8 (float8_avx a);

  long8_avx __ocl_svml_g9_cvtfptou64rtznosatf8 (float8_avx a);

  long8_avx __ocl_svml_g9_cvtfptou64rtzsatf8 (float8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtenosatf8 (float8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtesatf8 (float8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtnnosatf8 (float8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtnsatf8 (float8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtpnosatf8 (float8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtpsatf8 (float8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtznosatf8 (float8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtzsatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtenosatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtesatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtnnosatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtnsatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtpnosatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtpsatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtznosatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtzsatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtenosatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtesatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtnnosatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtnsatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtpnosatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtpsatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtznosatf8 (float8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtzsatf8 (float8_avx a);

  float8_avx __ocl_svml_g9_cvtu64tofprtef8 (long8_avx a);

  float8_avx __ocl_svml_g9_cvtu64tofprtnf8 (long8_avx a);

  float8_avx __ocl_svml_g9_cvtu64tofprtpf8 (long8_avx a);

  float8_avx __ocl_svml_g9_cvtu64tofprtzf8 (long8_avx a);

  float8_avx __ocl_svml_g9_cvti64tofprtef8 (long8_avx a);

  float8_avx __ocl_svml_g9_cvti64tofprtnf8 (long8_avx a);

  float8_avx __ocl_svml_g9_cvti64tofprtpf8 (long8_avx a);

  float8_avx __ocl_svml_g9_cvti64tofprtzf8 (long8_avx a);

  float8_avx __ocl_svml_g9_cvtu32tofprtef8 (int8_avx a);

  float8_avx __ocl_svml_g9_cvtu32tofprtnf8 (int8_avx a);

  float8_avx __ocl_svml_g9_cvtu32tofprtpf8 (int8_avx a);

  float8_avx __ocl_svml_g9_cvtu32tofprtzf8 (int8_avx a);

  float8_avx __ocl_svml_g9_cvti32tofprtef8 (int8_avx a);

  float8_avx __ocl_svml_g9_cvti32tofprtnf8 (int8_avx a);

  float8_avx __ocl_svml_g9_cvti32tofprtpf8 (int8_avx a);

  float8_avx __ocl_svml_g9_cvti32tofprtzf8 (int8_avx a);

  float8_avx __ocl_svml_s9_fmaf8 (float8_avx a, float8_avx b, float8_avx c);

  float8_avx __ocl_svml_s9_fabsf8 (float8_avx a);

  float8_avx __ocl_svml_s9_fminf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_fmaxf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_maxmagf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_minmagf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_nanf8 (int8_avx a);

  float8_avx __ocl_svml_s9_ceilf8 (float8_avx a);

  float8_avx __ocl_svml_s9_floorf8 (float8_avx a);

  float8_avx __ocl_svml_s9_roundf8 (float8_avx a);

  float8_avx __ocl_svml_s9_truncf8 (float8_avx a);

  float8_avx __ocl_svml_s9_rintf8 (float8_avx a);

  float8_avx __ocl_svml_s9_nearbyintf8 (float8_avx a);

  float8_avx __ocl_svml_s9_fmodf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_modff8 (float8_avx a, float8_avx * c);

  float8_avx __ocl_svml_s9_remainderf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_remquof8 (float8_avx a, float8_avx b, int8_avx * c);

  float8_avx __ocl_svml_s9_copysignf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_frexpf8 (float8_avx a, int8_avx * c);

  float8_avx __ocl_svml_s9_fdimf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_logbf8 (float8_avx a);

  int8_avx __ocl_svml_s9_ilogbf8 (float8_avx a);

  float8_avx __ocl_svml_s9_nextafterf8 (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_ldexpf8 (float8_avx a, int8_avx b);

  float8_avx __ocl_svml_s9_fractf8 (float8_avx a, float8_avx * c);

  float8_avx __ocl_svml_s9_tgammaf8 (float8_avx a);

  float8_avx __ocl_svml_s9_lgammaf8 (float8_avx a);

  float8_avx __ocl_svml_s9_lgammarf8 (float8_avx a, int8_avx * c);

  long8_avx __ocl_svml_s9_cvtfptou64rtenosatf8 (float8_avx a);

  long8_avx __ocl_svml_s9_cvtfptou64rtesatf8 (float8_avx a);

  long8_avx __ocl_svml_s9_cvtfptou64rtnnosatf8 (float8_avx a);

  long8_avx __ocl_svml_s9_cvtfptou64rtnsatf8 (float8_avx a);

  long8_avx __ocl_svml_s9_cvtfptou64rtpnosatf8 (float8_avx a);

  long8_avx __ocl_svml_s9_cvtfptou64rtpsatf8 (float8_avx a);

  long8_avx __ocl_svml_s9_cvtfptou64rtznosatf8 (float8_avx a);

  long8_avx __ocl_svml_s9_cvtfptou64rtzsatf8 (float8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtenosatf8 (float8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtesatf8 (float8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtnnosatf8 (float8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtnsatf8 (float8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtpnosatf8 (float8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtpsatf8 (float8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtznosatf8 (float8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtzsatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtenosatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtesatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtnnosatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtnsatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtpnosatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtpsatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtznosatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtzsatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtenosatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtesatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtnnosatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtnsatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtpnosatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtpsatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtznosatf8 (float8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtzsatf8 (float8_avx a);

  float8_avx __ocl_svml_s9_cvtu64tofprtef8 (long8_avx a);

  float8_avx __ocl_svml_s9_cvtu64tofprtnf8 (long8_avx a);

  float8_avx __ocl_svml_s9_cvtu64tofprtpf8 (long8_avx a);

  float8_avx __ocl_svml_s9_cvtu64tofprtzf8 (long8_avx a);

  float8_avx __ocl_svml_s9_cvti64tofprtef8 (long8_avx a);

  float8_avx __ocl_svml_s9_cvti64tofprtnf8 (long8_avx a);

  float8_avx __ocl_svml_s9_cvti64tofprtpf8 (long8_avx a);

  float8_avx __ocl_svml_s9_cvti64tofprtzf8 (long8_avx a);

  float8_avx __ocl_svml_s9_cvtu32tofprtef8 (int8_avx a);

  float8_avx __ocl_svml_s9_cvtu32tofprtnf8 (int8_avx a);

  float8_avx __ocl_svml_s9_cvtu32tofprtpf8 (int8_avx a);

  float8_avx __ocl_svml_s9_cvtu32tofprtzf8 (int8_avx a);

  float8_avx __ocl_svml_s9_cvti32tofprtef8 (int8_avx a);

  float8_avx __ocl_svml_s9_cvti32tofprtnf8 (int8_avx a);

  float8_avx __ocl_svml_s9_cvti32tofprtpf8 (int8_avx a);

  float8_avx __ocl_svml_s9_cvti32tofprtzf8 (int8_avx a);

  double8_avx __ocl_svml_g9_inv8 (double8_avx a);

  double8_avx __ocl_svml_g9_div8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_sqrt8 (double8_avx a);

  double8_avx __ocl_svml_g9_rsqrt8 (double8_avx a);

  double8_avx __ocl_svml_g9_cbrt8 (double8_avx a);

  double8_avx __ocl_svml_g9_rcbrt8 (double8_avx a);

  double8_avx __ocl_svml_g9_hypot8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_pow8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_powr8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_pown8 (double8_avx a, int8_avx b);

  double8_avx __ocl_svml_g9_rootn8 (double8_avx a, int8_avx b);

  double8_avx __ocl_svml_g9_exp8 (double8_avx a);

  double8_avx __ocl_svml_g9_exp28 (double8_avx a);

  double8_avx __ocl_svml_g9_exp108 (double8_avx a);

  double8_avx __ocl_svml_g9_expm18 (double8_avx a);

  double8_avx __ocl_svml_g9_log8 (double8_avx a);

  double8_avx __ocl_svml_g9_log108 (double8_avx a);

  double8_avx __ocl_svml_g9_log28 (double8_avx a);

  double8_avx __ocl_svml_g9_log1p8 (double8_avx a);

  double8_avx __ocl_svml_g9_sin8 (double8_avx a);

  double8_avx __ocl_svml_g9_cos8 (double8_avx a);

  double8_avx __ocl_svml_g9_sincos8 (double8_avx a, double8_avx * c);

  double8x2_avx __ocl_svml_g9_sincosreg8 (double8_avx a);

  double8_avx __ocl_svml_g9_tan8 (double8_avx a);

  double8_avx __ocl_svml_g9_sinpi8 (double8_avx a);

  double8_avx __ocl_svml_g9_cospi8 (double8_avx a);

  double8_avx __ocl_svml_g9_tanpi8 (double8_avx a);

  double8_avx __ocl_svml_g9_acos8 (double8_avx a);

  double8_avx __ocl_svml_g9_asin8 (double8_avx a);

  double8_avx __ocl_svml_g9_atan8 (double8_avx a);

  double8_avx __ocl_svml_g9_atan28 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_asinpi8 (double8_avx a);

  double8_avx __ocl_svml_g9_acospi8 (double8_avx a);

  double8_avx __ocl_svml_g9_atanpi8 (double8_avx a);

  double8_avx __ocl_svml_g9_atan2pi8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_sinh8 (double8_avx a);

  double8_avx __ocl_svml_g9_cosh8 (double8_avx a);

  double8_avx __ocl_svml_g9_tanh8 (double8_avx a);

  double8_avx __ocl_svml_g9_asinh8 (double8_avx a);

  double8_avx __ocl_svml_g9_acosh8 (double8_avx a);

  double8_avx __ocl_svml_g9_atanh8 (double8_avx a);

  double8_avx __ocl_svml_g9_erf8 (double8_avx a);

  double8_avx __ocl_svml_g9_erfc8 (double8_avx a);

  double8_avx __ocl_svml_s9_inv8 (double8_avx a);

  double8_avx __ocl_svml_s9_div8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_sqrt8 (double8_avx a);

  double8_avx __ocl_svml_s9_rsqrt8 (double8_avx a);

  double8_avx __ocl_svml_s9_cbrt8 (double8_avx a);

  double8_avx __ocl_svml_s9_rcbrt8 (double8_avx a);

  double8_avx __ocl_svml_s9_hypot8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_pow8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_powr8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_pown8 (double8_avx a, int8_avx b);

  double8_avx __ocl_svml_s9_rootn8 (double8_avx a, int8_avx b);

  double8_avx __ocl_svml_s9_exp8 (double8_avx a);

  double8_avx __ocl_svml_s9_exp28 (double8_avx a);

  double8_avx __ocl_svml_s9_exp108 (double8_avx a);

  double8_avx __ocl_svml_s9_expm18 (double8_avx a);

  double8_avx __ocl_svml_s9_log8 (double8_avx a);

  double8_avx __ocl_svml_s9_log108 (double8_avx a);

  double8_avx __ocl_svml_s9_log28 (double8_avx a);

  double8_avx __ocl_svml_s9_log1p8 (double8_avx a);

  double8_avx __ocl_svml_s9_sin8 (double8_avx a);

  double8_avx __ocl_svml_s9_cos8 (double8_avx a);

  double8_avx __ocl_svml_s9_sincos8 (double8_avx a, double8_avx * c);

  double8x2_avx __ocl_svml_s9_sincosreg8 (double8_avx a);

  double8_avx __ocl_svml_s9_tan8 (double8_avx a);

  double8_avx __ocl_svml_s9_sinpi8 (double8_avx a);

  double8_avx __ocl_svml_s9_cospi8 (double8_avx a);

  double8_avx __ocl_svml_s9_tanpi8 (double8_avx a);

  double8_avx __ocl_svml_s9_acos8 (double8_avx a);

  double8_avx __ocl_svml_s9_asin8 (double8_avx a);

  double8_avx __ocl_svml_s9_atan8 (double8_avx a);

  double8_avx __ocl_svml_s9_atan28 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_asinpi8 (double8_avx a);

  double8_avx __ocl_svml_s9_acospi8 (double8_avx a);

  double8_avx __ocl_svml_s9_atanpi8 (double8_avx a);

  double8_avx __ocl_svml_s9_atan2pi8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_sinh8 (double8_avx a);

  double8_avx __ocl_svml_s9_cosh8 (double8_avx a);

  double8_avx __ocl_svml_s9_tanh8 (double8_avx a);

  double8_avx __ocl_svml_s9_asinh8 (double8_avx a);

  double8_avx __ocl_svml_s9_acosh8 (double8_avx a);

  double8_avx __ocl_svml_s9_atanh8 (double8_avx a);

  double8_avx __ocl_svml_s9_erf8 (double8_avx a);

  double8_avx __ocl_svml_s9_erfc8 (double8_avx a);

  double8_avx __ocl_svml_g9_fabs8 (double8_avx a);

  double8_avx __ocl_svml_g9_fmin8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_fmax8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_maxmag8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_minmag8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_nan8 (long8_avx a);

  double8_avx __ocl_svml_g9_ceil8 (double8_avx a);

  double8_avx __ocl_svml_g9_floor8 (double8_avx a);

  double8_avx __ocl_svml_g9_round8 (double8_avx a);

  double8_avx __ocl_svml_g9_trunc8 (double8_avx a);

  double8_avx __ocl_svml_g9_rint8 (double8_avx a);

  double8_avx __ocl_svml_g9_nearbyint8 (double8_avx a);

  double8_avx __ocl_svml_g9_fmod8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_modf8 (double8_avx a, double8_avx * c);

  double8_avx __ocl_svml_g9_remainder8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_remquo8 (double8_avx a, double8_avx b, int8_avx * c);

  double8_avx __ocl_svml_g9_copysign8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_frexp8 (double8_avx a, int8_avx * c);

  double8_avx __ocl_svml_g9_fdim8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_logb8 (double8_avx a);

  int8_avx __ocl_svml_g9_ilogb8 (double8_avx a);

  double8_avx __ocl_svml_g9_nextafter8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_ldexp8 (double8_avx a, int8_avx b);

  double8_avx __ocl_svml_g9_fract8 (double8_avx a, double8_avx * c);

  double8_avx __ocl_svml_g9_tgamma8 (double8_avx a);

  double8_avx __ocl_svml_g9_lgamma8 (double8_avx a);

  double8_avx __ocl_svml_g9_lgammar8 (double8_avx a, int8_avx * c);

  long8_avx __ocl_svml_g9_cvtfptou64rtenosat8 (double8_avx a);

  long8_avx __ocl_svml_g9_cvtfptou64rtesat8 (double8_avx a);

  long8_avx __ocl_svml_g9_cvtfptou64rtnnosat8 (double8_avx a);

  long8_avx __ocl_svml_g9_cvtfptou64rtnsat8 (double8_avx a);

  long8_avx __ocl_svml_g9_cvtfptou64rtpnosat8 (double8_avx a);

  long8_avx __ocl_svml_g9_cvtfptou64rtpsat8 (double8_avx a);

  long8_avx __ocl_svml_g9_cvtfptou64rtznosat8 (double8_avx a);

  long8_avx __ocl_svml_g9_cvtfptou64rtzsat8 (double8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtenosat8 (double8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtesat8 (double8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtnnosat8 (double8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtnsat8 (double8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtpnosat8 (double8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtpsat8 (double8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtznosat8 (double8_avx a);

  long8_avx __ocl_svml_g9_cvtfptoi64rtzsat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtenosat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtesat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtnnosat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtnsat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtpnosat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtpsat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtznosat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptou32rtzsat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtenosat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtesat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtnnosat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtnsat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtpnosat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtpsat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtznosat8 (double8_avx a);

  int8_avx __ocl_svml_g9_cvtfptoi32rtzsat8 (double8_avx a);

  double8_avx __ocl_svml_g9_cvtu64tofprte8 (long8_avx a);

  double8_avx __ocl_svml_g9_cvtu64tofprtn8 (long8_avx a);

  double8_avx __ocl_svml_g9_cvtu64tofprtp8 (long8_avx a);

  double8_avx __ocl_svml_g9_cvtu64tofprtz8 (long8_avx a);

  double8_avx __ocl_svml_g9_cvti64tofprte8 (long8_avx a);

  double8_avx __ocl_svml_g9_cvti64tofprtn8 (long8_avx a);

  double8_avx __ocl_svml_g9_cvti64tofprtp8 (long8_avx a);

  double8_avx __ocl_svml_g9_cvti64tofprtz8 (long8_avx a);

  double8_avx __ocl_svml_g9_cvtu32tofprte8 (int8_avx a);

  double8_avx __ocl_svml_g9_cvtu32tofprtn8 (int8_avx a);

  double8_avx __ocl_svml_g9_cvtu32tofprtp8 (int8_avx a);

  double8_avx __ocl_svml_g9_cvtu32tofprtz8 (int8_avx a);

  double8_avx __ocl_svml_g9_cvti32tofprte8 (int8_avx a);

  double8_avx __ocl_svml_g9_cvti32tofprtn8 (int8_avx a);

  double8_avx __ocl_svml_g9_cvti32tofprtp8 (int8_avx a);

  double8_avx __ocl_svml_g9_cvti32tofprtz8 (int8_avx a);

  float8_avx __ocl_svml_g9_cvtfp64tofp32rte8 (double8_avx a);

  float8_avx __ocl_svml_g9_cvtfp64tofp32rtn8 (double8_avx a);

  float8_avx __ocl_svml_g9_cvtfp64tofp32rtp8 (double8_avx a);

  float8_avx __ocl_svml_g9_cvtfp64tofp32rtz8 (double8_avx a);

  double8_avx __ocl_svml_s9_fabs8 (double8_avx a);

  double8_avx __ocl_svml_s9_fmin8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_fmax8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_maxmag8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_minmag8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_nan8 (long8_avx a);

  double8_avx __ocl_svml_s9_ceil8 (double8_avx a);

  double8_avx __ocl_svml_s9_floor8 (double8_avx a);

  double8_avx __ocl_svml_s9_round8 (double8_avx a);

  double8_avx __ocl_svml_s9_trunc8 (double8_avx a);

  double8_avx __ocl_svml_s9_rint8 (double8_avx a);

  double8_avx __ocl_svml_s9_nearbyint8 (double8_avx a);

  double8_avx __ocl_svml_s9_fmod8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_modf8 (double8_avx a, double8_avx * c);

  double8_avx __ocl_svml_s9_remainder8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_remquo8 (double8_avx a, double8_avx b, int8_avx * c);

  double8_avx __ocl_svml_s9_copysign8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_frexp8 (double8_avx a, int8_avx * c);

  double8_avx __ocl_svml_s9_fdim8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_logb8 (double8_avx a);

  int8_avx __ocl_svml_s9_ilogb8 (double8_avx a);

  double8_avx __ocl_svml_s9_nextafter8 (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_ldexp8 (double8_avx a, int8_avx b);

  double8_avx __ocl_svml_s9_fract8 (double8_avx a, double8_avx * c);

  double8_avx __ocl_svml_s9_tgamma8 (double8_avx a);

  double8_avx __ocl_svml_s9_lgamma8 (double8_avx a);

  double8_avx __ocl_svml_s9_lgammar8 (double8_avx a, int8_avx * c);

  long8_avx __ocl_svml_s9_cvtfptou64rtenosat8 (double8_avx a);

  long8_avx __ocl_svml_s9_cvtfptou64rtesat8 (double8_avx a);

  long8_avx __ocl_svml_s9_cvtfptou64rtnnosat8 (double8_avx a);

  long8_avx __ocl_svml_s9_cvtfptou64rtnsat8 (double8_avx a);

  long8_avx __ocl_svml_s9_cvtfptou64rtpnosat8 (double8_avx a);

  long8_avx __ocl_svml_s9_cvtfptou64rtpsat8 (double8_avx a);

  long8_avx __ocl_svml_s9_cvtfptou64rtznosat8 (double8_avx a);

  long8_avx __ocl_svml_s9_cvtfptou64rtzsat8 (double8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtenosat8 (double8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtesat8 (double8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtnnosat8 (double8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtnsat8 (double8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtpnosat8 (double8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtpsat8 (double8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtznosat8 (double8_avx a);

  long8_avx __ocl_svml_s9_cvtfptoi64rtzsat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtenosat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtesat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtnnosat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtnsat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtpnosat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtpsat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtznosat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptou32rtzsat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtenosat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtesat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtnnosat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtnsat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtpnosat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtpsat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtznosat8 (double8_avx a);

  int8_avx __ocl_svml_s9_cvtfptoi32rtzsat8 (double8_avx a);

  double8_avx __ocl_svml_s9_cvtu64tofprte8 (long8_avx a);

  double8_avx __ocl_svml_s9_cvtu64tofprtn8 (long8_avx a);

  double8_avx __ocl_svml_s9_cvtu64tofprtp8 (long8_avx a);

  double8_avx __ocl_svml_s9_cvtu64tofprtz8 (long8_avx a);

  double8_avx __ocl_svml_s9_cvti64tofprte8 (long8_avx a);

  double8_avx __ocl_svml_s9_cvti64tofprtn8 (long8_avx a);

  double8_avx __ocl_svml_s9_cvti64tofprtp8 (long8_avx a);

  double8_avx __ocl_svml_s9_cvti64tofprtz8 (long8_avx a);

  double8_avx __ocl_svml_s9_cvtu32tofprte8 (int8_avx a);

  double8_avx __ocl_svml_s9_cvtu32tofprtn8 (int8_avx a);

  double8_avx __ocl_svml_s9_cvtu32tofprtp8 (int8_avx a);

  double8_avx __ocl_svml_s9_cvtu32tofprtz8 (int8_avx a);

  double8_avx __ocl_svml_s9_cvti32tofprte8 (int8_avx a);

  double8_avx __ocl_svml_s9_cvti32tofprtn8 (int8_avx a);

  double8_avx __ocl_svml_s9_cvti32tofprtp8 (int8_avx a);

  double8_avx __ocl_svml_s9_cvti32tofprtz8 (int8_avx a);

  float8_avx __ocl_svml_s9_cvtfp64tofp32rte8 (double8_avx a);

  float8_avx __ocl_svml_s9_cvtfp64tofp32rtn8 (double8_avx a);

  float8_avx __ocl_svml_s9_cvtfp64tofp32rtp8 (double8_avx a);

  float8_avx __ocl_svml_s9_cvtfp64tofp32rtz8 (double8_avx a);

  int8_sse __ocl_svml_n8_idiv8 (int8_sse a, int8_sse b);

  int8_sse __ocl_svml_n8_irem8 (int8_sse a, int8_sse b);

  int8x2_sse __ocl_svml_n8_idivrem8 (int8_sse a, int8_sse b);

  int8_avx __ocl_svml_g9_idiv8 (int8_avx a, int8_avx b);

  int8_avx __ocl_svml_g9_irem8 (int8_avx a, int8_avx b);

  int8x2_avx __ocl_svml_g9_idivrem8 (int8_avx a, int8_avx b);

  int8_avx __ocl_svml_s9_idiv8 (int8_avx a, int8_avx b);

  int8_avx __ocl_svml_s9_irem8 (int8_avx a, int8_avx b);

  int8x2_avx __ocl_svml_s9_idivrem8 (int8_avx a, int8_avx b);

  int8_sse __ocl_svml_n8_udiv8 (int8_sse a, int8_sse b);

  int8_sse __ocl_svml_n8_urem8 (int8_sse a, int8_sse b);

  int8x2_sse __ocl_svml_n8_udivrem8 (int8_sse a, int8_sse b);

  int8_avx __ocl_svml_g9_udiv8 (int8_avx a, int8_avx b);

  int8_avx __ocl_svml_g9_urem8 (int8_avx a, int8_avx b);

  int8x2_avx __ocl_svml_g9_udivrem8 (int8_avx a, int8_avx b);

  int8_avx __ocl_svml_s9_udiv8 (int8_avx a, int8_avx b);

  int8_avx __ocl_svml_s9_urem8 (int8_avx a, int8_avx b);

  int8x2_avx __ocl_svml_s9_udivrem8 (int8_avx a, int8_avx b);

  float16_avx __ocl_svml_g9_invf16 (float16_avx a);

  float16_avx __ocl_svml_g9_divf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_sqrtf16 (float16_avx a);

  float16_avx __ocl_svml_g9_rsqrtf16 (float16_avx a);

  float16_avx __ocl_svml_g9_cbrtf16 (float16_avx a);

  float16_avx __ocl_svml_g9_rcbrtf16 (float16_avx a);

  float16_avx __ocl_svml_g9_hypotf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_powf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_powrf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_pownf16 (float16_avx a, int16_avx b);

  float16_avx __ocl_svml_g9_rootnf16 (float16_avx a, int16_avx b);

  float16_avx __ocl_svml_g9_expf16 (float16_avx a);

  float16_avx __ocl_svml_g9_exp2f16 (float16_avx a);

  float16_avx __ocl_svml_g9_exp10f16 (float16_avx a);

  float16_avx __ocl_svml_g9_expm1f16 (float16_avx a);

  float16_avx __ocl_svml_g9_logf16 (float16_avx a);

  float16_avx __ocl_svml_g9_log10f16 (float16_avx a);

  float16_avx __ocl_svml_g9_log2f16 (float16_avx a);

  float16_avx __ocl_svml_g9_log1pf16 (float16_avx a);

  float16_avx __ocl_svml_g9_sinf16 (float16_avx a);

  float16_avx __ocl_svml_g9_cosf16 (float16_avx a);

  float16_avx __ocl_svml_g9_sincosf16 (float16_avx a, float16_avx * c);

  float16x2_avx __ocl_svml_g9_sincosregf16 (float16_avx a);

  float16_avx __ocl_svml_g9_tanf16 (float16_avx a);

  float16_avx __ocl_svml_g9_sinpif16 (float16_avx a);

  float16_avx __ocl_svml_g9_cospif16 (float16_avx a);

  float16_avx __ocl_svml_g9_tanpif16 (float16_avx a);

  float16_avx __ocl_svml_g9_acosf16 (float16_avx a);

  float16_avx __ocl_svml_g9_asinf16 (float16_avx a);

  float16_avx __ocl_svml_g9_atanf16 (float16_avx a);

  float16_avx __ocl_svml_g9_atan2f16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_asinpif16 (float16_avx a);

  float16_avx __ocl_svml_g9_acospif16 (float16_avx a);

  float16_avx __ocl_svml_g9_atanpif16 (float16_avx a);

  float16_avx __ocl_svml_g9_atan2pif16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_sinhf16 (float16_avx a);

  float16_avx __ocl_svml_g9_coshf16 (float16_avx a);

  float16_avx __ocl_svml_g9_tanhf16 (float16_avx a);

  float16_avx __ocl_svml_g9_asinhf16 (float16_avx a);

  float16_avx __ocl_svml_g9_acoshf16 (float16_avx a);

  float16_avx __ocl_svml_g9_atanhf16 (float16_avx a);

  float16_avx __ocl_svml_g9_erff16 (float16_avx a);

  float16_avx __ocl_svml_g9_erfcf16 (float16_avx a);

  float16_avx __ocl_svml_s9_invf16 (float16_avx a);

  float16_avx __ocl_svml_s9_divf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_sqrtf16 (float16_avx a);

  float16_avx __ocl_svml_s9_rsqrtf16 (float16_avx a);

  float16_avx __ocl_svml_s9_cbrtf16 (float16_avx a);

  float16_avx __ocl_svml_s9_rcbrtf16 (float16_avx a);

  float16_avx __ocl_svml_s9_hypotf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_powf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_powrf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_pownf16 (float16_avx a, int16_avx b);

  float16_avx __ocl_svml_s9_rootnf16 (float16_avx a, int16_avx b);

  float16_avx __ocl_svml_s9_expf16 (float16_avx a);

  float16_avx __ocl_svml_s9_exp2f16 (float16_avx a);

  float16_avx __ocl_svml_s9_exp10f16 (float16_avx a);

  float16_avx __ocl_svml_s9_expm1f16 (float16_avx a);

  float16_avx __ocl_svml_s9_logf16 (float16_avx a);

  float16_avx __ocl_svml_s9_log10f16 (float16_avx a);

  float16_avx __ocl_svml_s9_log2f16 (float16_avx a);

  float16_avx __ocl_svml_s9_log1pf16 (float16_avx a);

  float16_avx __ocl_svml_s9_sinf16 (float16_avx a);

  float16_avx __ocl_svml_s9_cosf16 (float16_avx a);

  float16_avx __ocl_svml_s9_sincosf16 (float16_avx a, float16_avx * c);

  float16x2_avx __ocl_svml_s9_sincosregf16 (float16_avx a);

  float16_avx __ocl_svml_s9_tanf16 (float16_avx a);

  float16_avx __ocl_svml_s9_sinpif16 (float16_avx a);

  float16_avx __ocl_svml_s9_cospif16 (float16_avx a);

  float16_avx __ocl_svml_s9_tanpif16 (float16_avx a);

  float16_avx __ocl_svml_s9_acosf16 (float16_avx a);

  float16_avx __ocl_svml_s9_asinf16 (float16_avx a);

  float16_avx __ocl_svml_s9_atanf16 (float16_avx a);

  float16_avx __ocl_svml_s9_atan2f16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_asinpif16 (float16_avx a);

  float16_avx __ocl_svml_s9_acospif16 (float16_avx a);

  float16_avx __ocl_svml_s9_atanpif16 (float16_avx a);

  float16_avx __ocl_svml_s9_atan2pif16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_sinhf16 (float16_avx a);

  float16_avx __ocl_svml_s9_coshf16 (float16_avx a);

  float16_avx __ocl_svml_s9_tanhf16 (float16_avx a);

  float16_avx __ocl_svml_s9_asinhf16 (float16_avx a);

  float16_avx __ocl_svml_s9_acoshf16 (float16_avx a);

  float16_avx __ocl_svml_s9_atanhf16 (float16_avx a);

  float16_avx __ocl_svml_s9_erff16 (float16_avx a);

  float16_avx __ocl_svml_s9_erfcf16 (float16_avx a);

  float16_avx __ocl_svml_g9_fabsf16 (float16_avx a);

  float16_avx __ocl_svml_g9_fminf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_fmaxf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_maxmagf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_minmagf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_nanf16 (int16_avx a);

  float16_avx __ocl_svml_g9_ceilf16 (float16_avx a);

  float16_avx __ocl_svml_g9_floorf16 (float16_avx a);

  float16_avx __ocl_svml_g9_roundf16 (float16_avx a);

  float16_avx __ocl_svml_g9_truncf16 (float16_avx a);

  float16_avx __ocl_svml_g9_rintf16 (float16_avx a);

  float16_avx __ocl_svml_g9_nearbyintf16 (float16_avx a);

  float16_avx __ocl_svml_g9_fmodf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_modff16 (float16_avx a, float16_avx * c);

  float16_avx __ocl_svml_g9_remainderf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_remquof16 (float16_avx a, float16_avx b, int16_avx * c);

  float16_avx __ocl_svml_g9_copysignf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_frexpf16 (float16_avx a, int16_avx * c);

  float16_avx __ocl_svml_g9_fdimf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_logbf16 (float16_avx a);

  int16_avx __ocl_svml_g9_ilogbf16 (float16_avx a);

  float16_avx __ocl_svml_g9_nextafterf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_ldexpf16 (float16_avx a, int16_avx b);

  float16_avx __ocl_svml_g9_fractf16 (float16_avx a, float16_avx * c);

  float16_avx __ocl_svml_g9_tgammaf16 (float16_avx a);

  float16_avx __ocl_svml_g9_lgammaf16 (float16_avx a);

  float16_avx __ocl_svml_g9_lgammarf16 (float16_avx a, int16_avx * c);

  long16_avx __ocl_svml_g9_cvtfptou64rtenosatf16 (float16_avx a);

  long16_avx __ocl_svml_g9_cvtfptou64rtesatf16 (float16_avx a);

  long16_avx __ocl_svml_g9_cvtfptou64rtnnosatf16 (float16_avx a);

  long16_avx __ocl_svml_g9_cvtfptou64rtnsatf16 (float16_avx a);

  long16_avx __ocl_svml_g9_cvtfptou64rtpnosatf16 (float16_avx a);

  long16_avx __ocl_svml_g9_cvtfptou64rtpsatf16 (float16_avx a);

  long16_avx __ocl_svml_g9_cvtfptou64rtznosatf16 (float16_avx a);

  long16_avx __ocl_svml_g9_cvtfptou64rtzsatf16 (float16_avx a);

  long16_avx __ocl_svml_g9_cvtfptoi64rtenosatf16 (float16_avx a);

  long16_avx __ocl_svml_g9_cvtfptoi64rtesatf16 (float16_avx a);

  long16_avx __ocl_svml_g9_cvtfptoi64rtnnosatf16 (float16_avx a);

  long16_avx __ocl_svml_g9_cvtfptoi64rtnsatf16 (float16_avx a);

  long16_avx __ocl_svml_g9_cvtfptoi64rtpnosatf16 (float16_avx a);

  long16_avx __ocl_svml_g9_cvtfptoi64rtpsatf16 (float16_avx a);

  long16_avx __ocl_svml_g9_cvtfptoi64rtznosatf16 (float16_avx a);

  long16_avx __ocl_svml_g9_cvtfptoi64rtzsatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptou32rtenosatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptou32rtesatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptou32rtnnosatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptou32rtnsatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptou32rtpnosatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptou32rtpsatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptou32rtznosatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptou32rtzsatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptoi32rtenosatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptoi32rtesatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptoi32rtnnosatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptoi32rtnsatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptoi32rtpnosatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptoi32rtpsatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptoi32rtznosatf16 (float16_avx a);

  int16_avx __ocl_svml_g9_cvtfptoi32rtzsatf16 (float16_avx a);

  float16_avx __ocl_svml_g9_cvtu64tofprtef16 (long16_avx a);

  float16_avx __ocl_svml_g9_cvtu64tofprtnf16 (long16_avx a);

  float16_avx __ocl_svml_g9_cvtu64tofprtpf16 (long16_avx a);

  float16_avx __ocl_svml_g9_cvtu64tofprtzf16 (long16_avx a);

  float16_avx __ocl_svml_g9_cvti64tofprtef16 (long16_avx a);

  float16_avx __ocl_svml_g9_cvti64tofprtnf16 (long16_avx a);

  float16_avx __ocl_svml_g9_cvti64tofprtpf16 (long16_avx a);

  float16_avx __ocl_svml_g9_cvti64tofprtzf16 (long16_avx a);

  float16_avx __ocl_svml_g9_cvtu32tofprtef16 (int16_avx a);

  float16_avx __ocl_svml_g9_cvtu32tofprtnf16 (int16_avx a);

  float16_avx __ocl_svml_g9_cvtu32tofprtpf16 (int16_avx a);

  float16_avx __ocl_svml_g9_cvtu32tofprtzf16 (int16_avx a);

  float16_avx __ocl_svml_g9_cvti32tofprtef16 (int16_avx a);

  float16_avx __ocl_svml_g9_cvti32tofprtnf16 (int16_avx a);

  float16_avx __ocl_svml_g9_cvti32tofprtpf16 (int16_avx a);

  float16_avx __ocl_svml_g9_cvti32tofprtzf16 (int16_avx a);

  float16_avx __ocl_svml_s9_fabsf16 (float16_avx a);

  float16_avx __ocl_svml_s9_fminf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_fmaxf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_maxmagf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_minmagf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_nanf16 (int16_avx a);

  float16_avx __ocl_svml_s9_ceilf16 (float16_avx a);

  float16_avx __ocl_svml_s9_floorf16 (float16_avx a);

  float16_avx __ocl_svml_s9_roundf16 (float16_avx a);

  float16_avx __ocl_svml_s9_truncf16 (float16_avx a);

  float16_avx __ocl_svml_s9_rintf16 (float16_avx a);

  float16_avx __ocl_svml_s9_nearbyintf16 (float16_avx a);

  float16_avx __ocl_svml_s9_fmodf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_modff16 (float16_avx a, float16_avx * c);

  float16_avx __ocl_svml_s9_remainderf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_remquof16 (float16_avx a, float16_avx b, int16_avx * c);

  float16_avx __ocl_svml_s9_copysignf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_frexpf16 (float16_avx a, int16_avx * c);

  float16_avx __ocl_svml_s9_fdimf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_logbf16 (float16_avx a);

  int16_avx __ocl_svml_s9_ilogbf16 (float16_avx a);

  float16_avx __ocl_svml_s9_nextafterf16 (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_ldexpf16 (float16_avx a, int16_avx b);

  float16_avx __ocl_svml_s9_fractf16 (float16_avx a, float16_avx * c);

  float16_avx __ocl_svml_s9_tgammaf16 (float16_avx a);

  float16_avx __ocl_svml_s9_lgammaf16 (float16_avx a);

  float16_avx __ocl_svml_s9_lgammarf16 (float16_avx a, int16_avx * c);

  long16_avx __ocl_svml_s9_cvtfptou64rtenosatf16 (float16_avx a);

  long16_avx __ocl_svml_s9_cvtfptou64rtesatf16 (float16_avx a);

  long16_avx __ocl_svml_s9_cvtfptou64rtnnosatf16 (float16_avx a);

  long16_avx __ocl_svml_s9_cvtfptou64rtnsatf16 (float16_avx a);

  long16_avx __ocl_svml_s9_cvtfptou64rtpnosatf16 (float16_avx a);

  long16_avx __ocl_svml_s9_cvtfptou64rtpsatf16 (float16_avx a);

  long16_avx __ocl_svml_s9_cvtfptou64rtznosatf16 (float16_avx a);

  long16_avx __ocl_svml_s9_cvtfptou64rtzsatf16 (float16_avx a);

  long16_avx __ocl_svml_s9_cvtfptoi64rtenosatf16 (float16_avx a);

  long16_avx __ocl_svml_s9_cvtfptoi64rtesatf16 (float16_avx a);

  long16_avx __ocl_svml_s9_cvtfptoi64rtnnosatf16 (float16_avx a);

  long16_avx __ocl_svml_s9_cvtfptoi64rtnsatf16 (float16_avx a);

  long16_avx __ocl_svml_s9_cvtfptoi64rtpnosatf16 (float16_avx a);

  long16_avx __ocl_svml_s9_cvtfptoi64rtpsatf16 (float16_avx a);

  long16_avx __ocl_svml_s9_cvtfptoi64rtznosatf16 (float16_avx a);

  long16_avx __ocl_svml_s9_cvtfptoi64rtzsatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptou32rtenosatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptou32rtesatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptou32rtnnosatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptou32rtnsatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptou32rtpnosatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptou32rtpsatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptou32rtznosatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptou32rtzsatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptoi32rtenosatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptoi32rtesatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptoi32rtnnosatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptoi32rtnsatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptoi32rtpnosatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptoi32rtpsatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptoi32rtznosatf16 (float16_avx a);

  int16_avx __ocl_svml_s9_cvtfptoi32rtzsatf16 (float16_avx a);

  float16_avx __ocl_svml_s9_cvtu64tofprtef16 (long16_avx a);

  float16_avx __ocl_svml_s9_cvtu64tofprtnf16 (long16_avx a);

  float16_avx __ocl_svml_s9_cvtu64tofprtpf16 (long16_avx a);

  float16_avx __ocl_svml_s9_cvtu64tofprtzf16 (long16_avx a);

  float16_avx __ocl_svml_s9_cvti64tofprtef16 (long16_avx a);

  float16_avx __ocl_svml_s9_cvti64tofprtnf16 (long16_avx a);

  float16_avx __ocl_svml_s9_cvti64tofprtpf16 (long16_avx a);

  float16_avx __ocl_svml_s9_cvti64tofprtzf16 (long16_avx a);

  float16_avx __ocl_svml_s9_cvtu32tofprtef16 (int16_avx a);

  float16_avx __ocl_svml_s9_cvtu32tofprtnf16 (int16_avx a);

  float16_avx __ocl_svml_s9_cvtu32tofprtpf16 (int16_avx a);

  float16_avx __ocl_svml_s9_cvtu32tofprtzf16 (int16_avx a);

  float16_avx __ocl_svml_s9_cvti32tofprtef16 (int16_avx a);

  float16_avx __ocl_svml_s9_cvti32tofprtnf16 (int16_avx a);

  float16_avx __ocl_svml_s9_cvti32tofprtpf16 (int16_avx a);

  float16_avx __ocl_svml_s9_cvti32tofprtzf16 (int16_avx a);

  int16_avx __ocl_svml_g9_idiv16 (int16_avx a, int16_avx b);

  int16_avx __ocl_svml_g9_irem16 (int16_avx a, int16_avx b);

  int16x2_avx __ocl_svml_g9_idivrem16 (int16_avx a, int16_avx b);

  int16_avx __ocl_svml_s9_idiv16 (int16_avx a, int16_avx b);

  int16_avx __ocl_svml_s9_irem16 (int16_avx a, int16_avx b);

  int16x2_avx __ocl_svml_s9_idivrem16 (int16_avx a, int16_avx b);

  int16_avx __ocl_svml_g9_udiv16 (int16_avx a, int16_avx b);

  int16_avx __ocl_svml_g9_urem16 (int16_avx a, int16_avx b);

  int16x2_avx __ocl_svml_g9_udivrem16 (int16_avx a, int16_avx b);

  int16_avx __ocl_svml_s9_udiv16 (int16_avx a, int16_avx b);

  int16_avx __ocl_svml_s9_urem16 (int16_avx a, int16_avx b);

  int16x2_avx __ocl_svml_s9_udivrem16 (int16_avx a, int16_avx b);

  float1_sse __ocl_svml_n8_cosf1_half (float1_sse a);

  float1_sse __ocl_svml_n8_divf1_half (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_expf1_half (float1_sse a);

  float1_sse __ocl_svml_n8_exp2f1_half (float1_sse a);

  float1_sse __ocl_svml_n8_exp10f1_half (float1_sse a);

  float1_sse __ocl_svml_n8_logf1_half (float1_sse a);

  float1_sse __ocl_svml_n8_log2f1_half (float1_sse a);

  float1_sse __ocl_svml_n8_log10f1_half (float1_sse a);

  float1_sse __ocl_svml_n8_powrf1_half (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_invf1_half (float1_sse a);

  float1_sse __ocl_svml_n8_rsqrtf1_half (float1_sse a);

  float1_sse __ocl_svml_n8_sinf1_half (float1_sse a);

  float1_sse __ocl_svml_n8_sqrtf1_half (float1_sse a);

  float1_sse __ocl_svml_n8_tanf1_half (float1_sse a);

  float1_avx __ocl_svml_g9_cosf1_half (float1_avx a);

  float1_avx __ocl_svml_g9_divf1_half (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_expf1_half (float1_avx a);

  float1_avx __ocl_svml_g9_exp2f1_half (float1_avx a);

  float1_avx __ocl_svml_g9_exp10f1_half (float1_avx a);

  float1_avx __ocl_svml_g9_logf1_half (float1_avx a);

  float1_avx __ocl_svml_g9_log2f1_half (float1_avx a);

  float1_avx __ocl_svml_g9_log10f1_half (float1_avx a);

  float1_avx __ocl_svml_g9_powrf1_half (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_invf1_half (float1_avx a);

  float1_avx __ocl_svml_g9_rsqrtf1_half (float1_avx a);

  float1_avx __ocl_svml_g9_sinf1_half (float1_avx a);

  float1_avx __ocl_svml_g9_sqrtf1_half (float1_avx a);

  float1_avx __ocl_svml_g9_tanf1_half (float1_avx a);

  float1_avx __ocl_svml_s9_cosf1_half (float1_avx a);

  float1_avx __ocl_svml_s9_divf1_half (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_expf1_half (float1_avx a);

  float1_avx __ocl_svml_s9_exp2f1_half (float1_avx a);

  float1_avx __ocl_svml_s9_exp10f1_half (float1_avx a);

  float1_avx __ocl_svml_s9_logf1_half (float1_avx a);

  float1_avx __ocl_svml_s9_log2f1_half (float1_avx a);

  float1_avx __ocl_svml_s9_log10f1_half (float1_avx a);

  float1_avx __ocl_svml_s9_powrf1_half (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_invf1_half (float1_avx a);

  float1_avx __ocl_svml_s9_rsqrtf1_half (float1_avx a);

  float1_avx __ocl_svml_s9_sinf1_half (float1_avx a);

  float1_avx __ocl_svml_s9_sqrtf1_half (float1_avx a);

  float1_avx __ocl_svml_s9_tanf1_half (float1_avx a);

  float2_sse __ocl_svml_n8_cosf2_half (float2_sse a);

  float2_sse __ocl_svml_n8_divf2_half (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_expf2_half (float2_sse a);

  float2_sse __ocl_svml_n8_exp2f2_half (float2_sse a);

  float2_sse __ocl_svml_n8_exp10f2_half (float2_sse a);

  float2_sse __ocl_svml_n8_logf2_half (float2_sse a);

  float2_sse __ocl_svml_n8_log2f2_half (float2_sse a);

  float2_sse __ocl_svml_n8_log10f2_half (float2_sse a);

  float2_sse __ocl_svml_n8_powrf2_half (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_invf2_half (float2_sse a);

  float2_sse __ocl_svml_n8_rsqrtf2_half (float2_sse a);

  float2_sse __ocl_svml_n8_sinf2_half (float2_sse a);

  float2_sse __ocl_svml_n8_sqrtf2_half (float2_sse a);

  float2_sse __ocl_svml_n8_tanf2_half (float2_sse a);

  float2_avx __ocl_svml_g9_cosf2_half (float2_avx a);

  float2_avx __ocl_svml_g9_divf2_half (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_expf2_half (float2_avx a);

  float2_avx __ocl_svml_g9_exp2f2_half (float2_avx a);

  float2_avx __ocl_svml_g9_exp10f2_half (float2_avx a);

  float2_avx __ocl_svml_g9_logf2_half (float2_avx a);

  float2_avx __ocl_svml_g9_log2f2_half (float2_avx a);

  float2_avx __ocl_svml_g9_log10f2_half (float2_avx a);

  float2_avx __ocl_svml_g9_powrf2_half (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_invf2_half (float2_avx a);

  float2_avx __ocl_svml_g9_rsqrtf2_half (float2_avx a);

  float2_avx __ocl_svml_g9_sinf2_half (float2_avx a);

  float2_avx __ocl_svml_g9_sqrtf2_half (float2_avx a);

  float2_avx __ocl_svml_g9_tanf2_half (float2_avx a);

  float2_avx __ocl_svml_s9_cosf2_half (float2_avx a);

  float2_avx __ocl_svml_s9_divf2_half (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_expf2_half (float2_avx a);

  float2_avx __ocl_svml_s9_exp2f2_half (float2_avx a);

  float2_avx __ocl_svml_s9_exp10f2_half (float2_avx a);

  float2_avx __ocl_svml_s9_logf2_half (float2_avx a);

  float2_avx __ocl_svml_s9_log2f2_half (float2_avx a);

  float2_avx __ocl_svml_s9_log10f2_half (float2_avx a);

  float2_avx __ocl_svml_s9_powrf2_half (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_invf2_half (float2_avx a);

  float2_avx __ocl_svml_s9_rsqrtf2_half (float2_avx a);

  float2_avx __ocl_svml_s9_sinf2_half (float2_avx a);

  float2_avx __ocl_svml_s9_sqrtf2_half (float2_avx a);

  float2_avx __ocl_svml_s9_tanf2_half (float2_avx a);

  float3_sse __ocl_svml_n8_cosf3_half (float3_sse a);

  float3_sse __ocl_svml_n8_divf3_half (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_expf3_half (float3_sse a);

  float3_sse __ocl_svml_n8_exp2f3_half (float3_sse a);

  float3_sse __ocl_svml_n8_exp10f3_half (float3_sse a);

  float3_sse __ocl_svml_n8_logf3_half (float3_sse a);

  float3_sse __ocl_svml_n8_log2f3_half (float3_sse a);

  float3_sse __ocl_svml_n8_log10f3_half (float3_sse a);

  float3_sse __ocl_svml_n8_powrf3_half (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_invf3_half (float3_sse a);

  float3_sse __ocl_svml_n8_rsqrtf3_half (float3_sse a);

  float3_sse __ocl_svml_n8_sinf3_half (float3_sse a);

  float3_sse __ocl_svml_n8_sqrtf3_half (float3_sse a);

  float3_sse __ocl_svml_n8_tanf3_half (float3_sse a);

  float3_avx __ocl_svml_g9_cosf3_half (float3_avx a);

  float3_avx __ocl_svml_g9_divf3_half (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_expf3_half (float3_avx a);

  float3_avx __ocl_svml_g9_exp2f3_half (float3_avx a);

  float3_avx __ocl_svml_g9_exp10f3_half (float3_avx a);

  float3_avx __ocl_svml_g9_logf3_half (float3_avx a);

  float3_avx __ocl_svml_g9_log2f3_half (float3_avx a);

  float3_avx __ocl_svml_g9_log10f3_half (float3_avx a);

  float3_avx __ocl_svml_g9_powrf3_half (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_invf3_half (float3_avx a);

  float3_avx __ocl_svml_g9_rsqrtf3_half (float3_avx a);

  float3_avx __ocl_svml_g9_sinf3_half (float3_avx a);

  float3_avx __ocl_svml_g9_sqrtf3_half (float3_avx a);

  float3_avx __ocl_svml_g9_tanf3_half (float3_avx a);

  float3_avx __ocl_svml_s9_cosf3_half (float3_avx a);

  float3_avx __ocl_svml_s9_divf3_half (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_expf3_half (float3_avx a);

  float3_avx __ocl_svml_s9_exp2f3_half (float3_avx a);

  float3_avx __ocl_svml_s9_exp10f3_half (float3_avx a);

  float3_avx __ocl_svml_s9_logf3_half (float3_avx a);

  float3_avx __ocl_svml_s9_log2f3_half (float3_avx a);

  float3_avx __ocl_svml_s9_log10f3_half (float3_avx a);

  float3_avx __ocl_svml_s9_powrf3_half (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_invf3_half (float3_avx a);

  float3_avx __ocl_svml_s9_rsqrtf3_half (float3_avx a);

  float3_avx __ocl_svml_s9_sinf3_half (float3_avx a);

  float3_avx __ocl_svml_s9_sqrtf3_half (float3_avx a);

  float3_avx __ocl_svml_s9_tanf3_half (float3_avx a);

  float4_sse __ocl_svml_n8_cosf4_half (float4_sse a);

  float4_sse __ocl_svml_n8_divf4_half (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_expf4_half (float4_sse a);

  float4_sse __ocl_svml_n8_exp2f4_half (float4_sse a);

  float4_sse __ocl_svml_n8_exp10f4_half (float4_sse a);

  float4_sse __ocl_svml_n8_logf4_half (float4_sse a);

  float4_sse __ocl_svml_n8_log2f4_half (float4_sse a);

  float4_sse __ocl_svml_n8_log10f4_half (float4_sse a);

  float4_sse __ocl_svml_n8_powrf4_half (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_invf4_half (float4_sse a);

  float4_sse __ocl_svml_n8_rsqrtf4_half (float4_sse a);

  float4_sse __ocl_svml_n8_sinf4_half (float4_sse a);

  float4_sse __ocl_svml_n8_sqrtf4_half (float4_sse a);

  float4_sse __ocl_svml_n8_tanf4_half (float4_sse a);

  float4_avx __ocl_svml_g9_cosf4_half (float4_avx a);

  float4_avx __ocl_svml_g9_divf4_half (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_expf4_half (float4_avx a);

  float4_avx __ocl_svml_g9_exp2f4_half (float4_avx a);

  float4_avx __ocl_svml_g9_exp10f4_half (float4_avx a);

  float4_avx __ocl_svml_g9_logf4_half (float4_avx a);

  float4_avx __ocl_svml_g9_log2f4_half (float4_avx a);

  float4_avx __ocl_svml_g9_log10f4_half (float4_avx a);

  float4_avx __ocl_svml_g9_powrf4_half (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_invf4_half (float4_avx a);

  float4_avx __ocl_svml_g9_rsqrtf4_half (float4_avx a);

  float4_avx __ocl_svml_g9_sinf4_half (float4_avx a);

  float4_avx __ocl_svml_g9_sqrtf4_half (float4_avx a);

  float4_avx __ocl_svml_g9_tanf4_half (float4_avx a);

  float4_avx __ocl_svml_s9_cosf4_half (float4_avx a);

  float4_avx __ocl_svml_s9_divf4_half (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_expf4_half (float4_avx a);

  float4_avx __ocl_svml_s9_exp2f4_half (float4_avx a);

  float4_avx __ocl_svml_s9_exp10f4_half (float4_avx a);

  float4_avx __ocl_svml_s9_logf4_half (float4_avx a);

  float4_avx __ocl_svml_s9_log2f4_half (float4_avx a);

  float4_avx __ocl_svml_s9_log10f4_half (float4_avx a);

  float4_avx __ocl_svml_s9_powrf4_half (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_invf4_half (float4_avx a);

  float4_avx __ocl_svml_s9_rsqrtf4_half (float4_avx a);

  float4_avx __ocl_svml_s9_sinf4_half (float4_avx a);

  float4_avx __ocl_svml_s9_sqrtf4_half (float4_avx a);

  float4_avx __ocl_svml_s9_tanf4_half (float4_avx a);

  float8_sse __ocl_svml_n8_cosf8_half (float8_sse a);

  float8_sse __ocl_svml_n8_divf8_half (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_expf8_half (float8_sse a);

  float8_sse __ocl_svml_n8_exp2f8_half (float8_sse a);

  float8_sse __ocl_svml_n8_exp10f8_half (float8_sse a);

  float8_sse __ocl_svml_n8_logf8_half (float8_sse a);

  float8_sse __ocl_svml_n8_log2f8_half (float8_sse a);

  float8_sse __ocl_svml_n8_log10f8_half (float8_sse a);

  float8_sse __ocl_svml_n8_powrf8_half (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_invf8_half (float8_sse a);

  float8_sse __ocl_svml_n8_rsqrtf8_half (float8_sse a);

  float8_sse __ocl_svml_n8_sinf8_half (float8_sse a);

  float8_sse __ocl_svml_n8_sqrtf8_half (float8_sse a);

  float8_sse __ocl_svml_n8_tanf8_half (float8_sse a);

  float8_avx __ocl_svml_g9_cosf8_half (float8_avx a);

  float8_avx __ocl_svml_g9_divf8_half (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_expf8_half (float8_avx a);

  float8_avx __ocl_svml_g9_exp2f8_half (float8_avx a);

  float8_avx __ocl_svml_g9_exp10f8_half (float8_avx a);

  float8_avx __ocl_svml_g9_logf8_half (float8_avx a);

  float8_avx __ocl_svml_g9_log2f8_half (float8_avx a);

  float8_avx __ocl_svml_g9_log10f8_half (float8_avx a);

  float8_avx __ocl_svml_g9_powrf8_half (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_invf8_half (float8_avx a);

  float8_avx __ocl_svml_g9_rsqrtf8_half (float8_avx a);

  float8_avx __ocl_svml_g9_sinf8_half (float8_avx a);

  float8_avx __ocl_svml_g9_sqrtf8_half (float8_avx a);

  float8_avx __ocl_svml_g9_tanf8_half (float8_avx a);

  float8_avx __ocl_svml_s9_cosf8_half (float8_avx a);

  float8_avx __ocl_svml_s9_divf8_half (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_expf8_half (float8_avx a);

  float8_avx __ocl_svml_s9_exp2f8_half (float8_avx a);

  float8_avx __ocl_svml_s9_exp10f8_half (float8_avx a);

  float8_avx __ocl_svml_s9_logf8_half (float8_avx a);

  float8_avx __ocl_svml_s9_log2f8_half (float8_avx a);

  float8_avx __ocl_svml_s9_log10f8_half (float8_avx a);

  float8_avx __ocl_svml_s9_powrf8_half (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_invf8_half (float8_avx a);

  float8_avx __ocl_svml_s9_rsqrtf8_half (float8_avx a);

  float8_avx __ocl_svml_s9_sinf8_half (float8_avx a);

  float8_avx __ocl_svml_s9_sqrtf8_half (float8_avx a);

  float8_avx __ocl_svml_s9_tanf8_half (float8_avx a);

  float16_avx __ocl_svml_g9_cosf16_half (float16_avx a);

  float16_avx __ocl_svml_g9_divf16_half (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_expf16_half (float16_avx a);

  float16_avx __ocl_svml_g9_exp2f16_half (float16_avx a);

  float16_avx __ocl_svml_g9_exp10f16_half (float16_avx a);

  float16_avx __ocl_svml_g9_logf16_half (float16_avx a);

  float16_avx __ocl_svml_g9_log2f16_half (float16_avx a);

  float16_avx __ocl_svml_g9_log10f16_half (float16_avx a);

  float16_avx __ocl_svml_g9_powrf16_half (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_invf16_half (float16_avx a);

  float16_avx __ocl_svml_g9_rsqrtf16_half (float16_avx a);

  float16_avx __ocl_svml_g9_sinf16_half (float16_avx a);

  float16_avx __ocl_svml_g9_sqrtf16_half (float16_avx a);

  float16_avx __ocl_svml_g9_tanf16_half (float16_avx a);

  float16_avx __ocl_svml_s9_cosf16_half (float16_avx a);

  float16_avx __ocl_svml_s9_divf16_half (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_expf16_half (float16_avx a);

  float16_avx __ocl_svml_s9_exp2f16_half (float16_avx a);

  float16_avx __ocl_svml_s9_exp10f16_half (float16_avx a);

  float16_avx __ocl_svml_s9_logf16_half (float16_avx a);

  float16_avx __ocl_svml_s9_log2f16_half (float16_avx a);

  float16_avx __ocl_svml_s9_log10f16_half (float16_avx a);

  float16_avx __ocl_svml_s9_powrf16_half (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_invf16_half (float16_avx a);

  float16_avx __ocl_svml_s9_rsqrtf16_half (float16_avx a);

  float16_avx __ocl_svml_s9_sinf16_half (float16_avx a);

  float16_avx __ocl_svml_s9_sqrtf16_half (float16_avx a);

  float16_avx __ocl_svml_s9_tanf16_half (float16_avx a);

  float1_sse __ocl_svml_n8_invf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_divf1_native (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_sqrtf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_rsqrtf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_cbrtf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_rcbrtf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_hypotf1_native (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_powf1_native (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_powrf1_native (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_pownf1_native (float1_sse a, int1_sse b);

  float1_sse __ocl_svml_n8_rootnf1_native (float1_sse a, int1_sse b);

  float1_sse __ocl_svml_n8_expf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_exp2f1_native (float1_sse a);

  float1_sse __ocl_svml_n8_exp10f1_native (float1_sse a);

  float1_sse __ocl_svml_n8_expm1f1_native (float1_sse a);

  float1_sse __ocl_svml_n8_logf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_log10f1_native (float1_sse a);

  float1_sse __ocl_svml_n8_log2f1_native (float1_sse a);

  float1_sse __ocl_svml_n8_log1pf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_sinf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_cosf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_sincosf1_native (float1_sse a, float1_sse * c);

  float1x2_sse __ocl_svml_n8_sincosregf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_tanf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_sinpif1_native (float1_sse a);

  float1_sse __ocl_svml_n8_cospif1_native (float1_sse a);

  float1_sse __ocl_svml_n8_tanpif1_native (float1_sse a);

  float1_sse __ocl_svml_n8_acosf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_asinf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_atanf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_atan2f1_native (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_asinpif1_native (float1_sse a);

  float1_sse __ocl_svml_n8_acospif1_native (float1_sse a);

  float1_sse __ocl_svml_n8_atanpif1_native (float1_sse a);

  float1_sse __ocl_svml_n8_atan2pif1_native (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_sinhf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_coshf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_tanhf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_asinhf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_acoshf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_atanhf1_native (float1_sse a);

  float1_sse __ocl_svml_n8_erff1_native (float1_sse a);

  float1_sse __ocl_svml_n8_erfcf1_native (float1_sse a);

  float1_avx __ocl_svml_g9_invf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_divf1_native (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_sqrtf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_rsqrtf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_cbrtf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_rcbrtf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_hypotf1_native (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_powf1_native (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_powrf1_native (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_pownf1_native (float1_avx a, int1_avx b);

  float1_avx __ocl_svml_g9_rootnf1_native (float1_avx a, int1_avx b);

  float1_avx __ocl_svml_g9_expf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_exp2f1_native (float1_avx a);

  float1_avx __ocl_svml_g9_exp10f1_native (float1_avx a);

  float1_avx __ocl_svml_g9_expm1f1_native (float1_avx a);

  float1_avx __ocl_svml_g9_logf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_log10f1_native (float1_avx a);

  float1_avx __ocl_svml_g9_log2f1_native (float1_avx a);

  float1_avx __ocl_svml_g9_log1pf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_sinf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_cosf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_sincosf1_native (float1_avx a, float1_avx * c);

  float1x2_avx __ocl_svml_g9_sincosregf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_tanf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_sinpif1_native (float1_avx a);

  float1_avx __ocl_svml_g9_cospif1_native (float1_avx a);

  float1_avx __ocl_svml_g9_tanpif1_native (float1_avx a);

  float1_avx __ocl_svml_g9_acosf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_asinf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_atanf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_atan2f1_native (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_asinpif1_native (float1_avx a);

  float1_avx __ocl_svml_g9_acospif1_native (float1_avx a);

  float1_avx __ocl_svml_g9_atanpif1_native (float1_avx a);

  float1_avx __ocl_svml_g9_atan2pif1_native (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_sinhf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_coshf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_tanhf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_asinhf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_acoshf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_atanhf1_native (float1_avx a);

  float1_avx __ocl_svml_g9_erff1_native (float1_avx a);

  float1_avx __ocl_svml_g9_erfcf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_invf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_divf1_native (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_sqrtf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_rsqrtf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_cbrtf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_rcbrtf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_hypotf1_native (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_powf1_native (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_powrf1_native (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_pownf1_native (float1_avx a, int1_avx b);

  float1_avx __ocl_svml_s9_rootnf1_native (float1_avx a, int1_avx b);

  float1_avx __ocl_svml_s9_expf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_exp2f1_native (float1_avx a);

  float1_avx __ocl_svml_s9_exp10f1_native (float1_avx a);

  float1_avx __ocl_svml_s9_expm1f1_native (float1_avx a);

  float1_avx __ocl_svml_s9_logf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_log10f1_native (float1_avx a);

  float1_avx __ocl_svml_s9_log2f1_native (float1_avx a);

  float1_avx __ocl_svml_s9_log1pf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_sinf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_cosf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_sincosf1_native (float1_avx a, float1_avx * c);

  float1x2_avx __ocl_svml_s9_sincosregf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_tanf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_sinpif1_native (float1_avx a);

  float1_avx __ocl_svml_s9_cospif1_native (float1_avx a);

  float1_avx __ocl_svml_s9_tanpif1_native (float1_avx a);

  float1_avx __ocl_svml_s9_acosf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_asinf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_atanf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_atan2f1_native (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_asinpif1_native (float1_avx a);

  float1_avx __ocl_svml_s9_acospif1_native (float1_avx a);

  float1_avx __ocl_svml_s9_atanpif1_native (float1_avx a);

  float1_avx __ocl_svml_s9_atan2pif1_native (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_sinhf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_coshf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_tanhf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_asinhf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_acoshf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_atanhf1_native (float1_avx a);

  float1_avx __ocl_svml_s9_erff1_native (float1_avx a);

  float1_avx __ocl_svml_s9_erfcf1_native (float1_avx a);

  double1_sse __ocl_svml_n8_inv1_native (double1_sse a);

  double1_sse __ocl_svml_n8_div1_native (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_sqrt1_native (double1_sse a);

  double1_sse __ocl_svml_n8_rsqrt1_native (double1_sse a);

  double1_sse __ocl_svml_n8_cbrt1_native (double1_sse a);

  double1_sse __ocl_svml_n8_rcbrt1_native (double1_sse a);

  double1_sse __ocl_svml_n8_hypot1_native (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_pow1_native (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_powr1_native (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_pown1_native (double1_sse a, int1_sse b);

  double1_sse __ocl_svml_n8_rootn1_native (double1_sse a, int1_sse b);

  double1_sse __ocl_svml_n8_exp1_native (double1_sse a);

  double1_sse __ocl_svml_n8_exp21_native (double1_sse a);

  double1_sse __ocl_svml_n8_exp101_native (double1_sse a);

  double1_sse __ocl_svml_n8_expm11_native (double1_sse a);

  double1_sse __ocl_svml_n8_log1_native (double1_sse a);

  double1_sse __ocl_svml_n8_log101_native (double1_sse a);

  double1_sse __ocl_svml_n8_log21_native (double1_sse a);

  double1_sse __ocl_svml_n8_log1p1_native (double1_sse a);

  double1_sse __ocl_svml_n8_sin1_native (double1_sse a);

  double1_sse __ocl_svml_n8_cos1_native (double1_sse a);

  double1_sse __ocl_svml_n8_sincos1_native (double1_sse a, double1_sse * c);

  double1x2_sse __ocl_svml_n8_sincosreg1_native (double1_sse a);

  double1_sse __ocl_svml_n8_tan1_native (double1_sse a);

  double1_sse __ocl_svml_n8_sinpi1_native (double1_sse a);

  double1_sse __ocl_svml_n8_cospi1_native (double1_sse a);

  double1_sse __ocl_svml_n8_tanpi1_native (double1_sse a);

  double1_sse __ocl_svml_n8_acos1_native (double1_sse a);

  double1_sse __ocl_svml_n8_asin1_native (double1_sse a);

  double1_sse __ocl_svml_n8_atan1_native (double1_sse a);

  double1_sse __ocl_svml_n8_atan21_native (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_asinpi1_native (double1_sse a);

  double1_sse __ocl_svml_n8_acospi1_native (double1_sse a);

  double1_sse __ocl_svml_n8_atanpi1_native (double1_sse a);

  double1_sse __ocl_svml_n8_atan2pi1_native (double1_sse a, double1_sse b);

  double1_sse __ocl_svml_n8_sinh1_native (double1_sse a);

  double1_sse __ocl_svml_n8_cosh1_native (double1_sse a);

  double1_sse __ocl_svml_n8_tanh1_native (double1_sse a);

  double1_sse __ocl_svml_n8_asinh1_native (double1_sse a);

  double1_sse __ocl_svml_n8_acosh1_native (double1_sse a);

  double1_sse __ocl_svml_n8_atanh1_native (double1_sse a);

  double1_sse __ocl_svml_n8_erf1_native (double1_sse a);

  double1_sse __ocl_svml_n8_erfc1_native (double1_sse a);

  double1_avx __ocl_svml_g9_inv1_native (double1_avx a);

  double1_avx __ocl_svml_g9_div1_native (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_sqrt1_native (double1_avx a);

  double1_avx __ocl_svml_g9_rsqrt1_native (double1_avx a);

  double1_avx __ocl_svml_g9_cbrt1_native (double1_avx a);

  double1_avx __ocl_svml_g9_rcbrt1_native (double1_avx a);

  double1_avx __ocl_svml_g9_hypot1_native (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_pow1_native (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_powr1_native (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_pown1_native (double1_avx a, int1_avx b);

  double1_avx __ocl_svml_g9_rootn1_native (double1_avx a, int1_avx b);

  double1_avx __ocl_svml_g9_exp1_native (double1_avx a);

  double1_avx __ocl_svml_g9_exp21_native (double1_avx a);

  double1_avx __ocl_svml_g9_exp101_native (double1_avx a);

  double1_avx __ocl_svml_g9_expm11_native (double1_avx a);

  double1_avx __ocl_svml_g9_log1_native (double1_avx a);

  double1_avx __ocl_svml_g9_log101_native (double1_avx a);

  double1_avx __ocl_svml_g9_log21_native (double1_avx a);

  double1_avx __ocl_svml_g9_log1p1_native (double1_avx a);

  double1_avx __ocl_svml_g9_sin1_native (double1_avx a);

  double1_avx __ocl_svml_g9_cos1_native (double1_avx a);

  double1_avx __ocl_svml_g9_sincos1_native (double1_avx a, double1_avx * c);

  double1x2_avx __ocl_svml_g9_sincosreg1_native (double1_avx a);

  double1_avx __ocl_svml_g9_tan1_native (double1_avx a);

  double1_avx __ocl_svml_g9_sinpi1_native (double1_avx a);

  double1_avx __ocl_svml_g9_cospi1_native (double1_avx a);

  double1_avx __ocl_svml_g9_tanpi1_native (double1_avx a);

  double1_avx __ocl_svml_g9_acos1_native (double1_avx a);

  double1_avx __ocl_svml_g9_asin1_native (double1_avx a);

  double1_avx __ocl_svml_g9_atan1_native (double1_avx a);

  double1_avx __ocl_svml_g9_atan21_native (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_asinpi1_native (double1_avx a);

  double1_avx __ocl_svml_g9_acospi1_native (double1_avx a);

  double1_avx __ocl_svml_g9_atanpi1_native (double1_avx a);

  double1_avx __ocl_svml_g9_atan2pi1_native (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_g9_sinh1_native (double1_avx a);

  double1_avx __ocl_svml_g9_cosh1_native (double1_avx a);

  double1_avx __ocl_svml_g9_tanh1_native (double1_avx a);

  double1_avx __ocl_svml_g9_asinh1_native (double1_avx a);

  double1_avx __ocl_svml_g9_acosh1_native (double1_avx a);

  double1_avx __ocl_svml_g9_atanh1_native (double1_avx a);

  double1_avx __ocl_svml_g9_erf1_native (double1_avx a);

  double1_avx __ocl_svml_g9_erfc1_native (double1_avx a);

  double1_avx __ocl_svml_s9_inv1_native (double1_avx a);

  double1_avx __ocl_svml_s9_div1_native (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_sqrt1_native (double1_avx a);

  double1_avx __ocl_svml_s9_rsqrt1_native (double1_avx a);

  double1_avx __ocl_svml_s9_cbrt1_native (double1_avx a);

  double1_avx __ocl_svml_s9_rcbrt1_native (double1_avx a);

  double1_avx __ocl_svml_s9_hypot1_native (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_pow1_native (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_powr1_native (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_pown1_native (double1_avx a, int1_avx b);

  double1_avx __ocl_svml_s9_rootn1_native (double1_avx a, int1_avx b);

  double1_avx __ocl_svml_s9_exp1_native (double1_avx a);

  double1_avx __ocl_svml_s9_exp21_native (double1_avx a);

  double1_avx __ocl_svml_s9_exp101_native (double1_avx a);

  double1_avx __ocl_svml_s9_expm11_native (double1_avx a);

  double1_avx __ocl_svml_s9_log1_native (double1_avx a);

  double1_avx __ocl_svml_s9_log101_native (double1_avx a);

  double1_avx __ocl_svml_s9_log21_native (double1_avx a);

  double1_avx __ocl_svml_s9_log1p1_native (double1_avx a);

  double1_avx __ocl_svml_s9_sin1_native (double1_avx a);

  double1_avx __ocl_svml_s9_cos1_native (double1_avx a);

  double1_avx __ocl_svml_s9_sincos1_native (double1_avx a, double1_avx * c);

  double1x2_avx __ocl_svml_s9_sincosreg1_native (double1_avx a);

  double1_avx __ocl_svml_s9_tan1_native (double1_avx a);

  double1_avx __ocl_svml_s9_sinpi1_native (double1_avx a);

  double1_avx __ocl_svml_s9_cospi1_native (double1_avx a);

  double1_avx __ocl_svml_s9_tanpi1_native (double1_avx a);

  double1_avx __ocl_svml_s9_acos1_native (double1_avx a);

  double1_avx __ocl_svml_s9_asin1_native (double1_avx a);

  double1_avx __ocl_svml_s9_atan1_native (double1_avx a);

  double1_avx __ocl_svml_s9_atan21_native (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_asinpi1_native (double1_avx a);

  double1_avx __ocl_svml_s9_acospi1_native (double1_avx a);

  double1_avx __ocl_svml_s9_atanpi1_native (double1_avx a);

  double1_avx __ocl_svml_s9_atan2pi1_native (double1_avx a, double1_avx b);

  double1_avx __ocl_svml_s9_sinh1_native (double1_avx a);

  double1_avx __ocl_svml_s9_cosh1_native (double1_avx a);

  double1_avx __ocl_svml_s9_tanh1_native (double1_avx a);

  double1_avx __ocl_svml_s9_asinh1_native (double1_avx a);

  double1_avx __ocl_svml_s9_acosh1_native (double1_avx a);

  double1_avx __ocl_svml_s9_atanh1_native (double1_avx a);

  double1_avx __ocl_svml_s9_erf1_native (double1_avx a);

  double1_avx __ocl_svml_s9_erfc1_native (double1_avx a);

  float2_sse __ocl_svml_n8_invf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_divf2_native (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_sqrtf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_rsqrtf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_cbrtf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_rcbrtf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_hypotf2_native (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_powf2_native (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_powrf2_native (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_pownf2_native (float2_sse a, int2_sse b);

  float2_sse __ocl_svml_n8_rootnf2_native (float2_sse a, int2_sse b);

  float2_sse __ocl_svml_n8_expf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_exp2f2_native (float2_sse a);

  float2_sse __ocl_svml_n8_exp10f2_native (float2_sse a);

  float2_sse __ocl_svml_n8_expm1f2_native (float2_sse a);

  float2_sse __ocl_svml_n8_logf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_log10f2_native (float2_sse a);

  float2_sse __ocl_svml_n8_log2f2_native (float2_sse a);

  float2_sse __ocl_svml_n8_log1pf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_sinf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_cosf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_sincosf2_native (float2_sse a, float2_sse * c);

  float2x2_sse __ocl_svml_n8_sincosregf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_tanf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_sinpif2_native (float2_sse a);

  float2_sse __ocl_svml_n8_cospif2_native (float2_sse a);

  float2_sse __ocl_svml_n8_tanpif2_native (float2_sse a);

  float2_sse __ocl_svml_n8_acosf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_asinf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_atanf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_atan2f2_native (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_asinpif2_native (float2_sse a);

  float2_sse __ocl_svml_n8_acospif2_native (float2_sse a);

  float2_sse __ocl_svml_n8_atanpif2_native (float2_sse a);

  float2_sse __ocl_svml_n8_atan2pif2_native (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_sinhf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_coshf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_tanhf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_asinhf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_acoshf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_atanhf2_native (float2_sse a);

  float2_sse __ocl_svml_n8_erff2_native (float2_sse a);

  float2_sse __ocl_svml_n8_erfcf2_native (float2_sse a);

  float2_avx __ocl_svml_g9_invf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_divf2_native (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_sqrtf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_rsqrtf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_cbrtf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_rcbrtf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_hypotf2_native (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_powf2_native (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_powrf2_native (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_pownf2_native (float2_avx a, int2_avx b);

  float2_avx __ocl_svml_g9_rootnf2_native (float2_avx a, int2_avx b);

  float2_avx __ocl_svml_g9_expf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_exp2f2_native (float2_avx a);

  float2_avx __ocl_svml_g9_exp10f2_native (float2_avx a);

  float2_avx __ocl_svml_g9_expm1f2_native (float2_avx a);

  float2_avx __ocl_svml_g9_logf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_log10f2_native (float2_avx a);

  float2_avx __ocl_svml_g9_log2f2_native (float2_avx a);

  float2_avx __ocl_svml_g9_log1pf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_sinf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_cosf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_sincosf2_native (float2_avx a, float2_avx * c);

  float2x2_avx __ocl_svml_g9_sincosregf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_tanf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_sinpif2_native (float2_avx a);

  float2_avx __ocl_svml_g9_cospif2_native (float2_avx a);

  float2_avx __ocl_svml_g9_tanpif2_native (float2_avx a);

  float2_avx __ocl_svml_g9_acosf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_asinf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_atanf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_atan2f2_native (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_asinpif2_native (float2_avx a);

  float2_avx __ocl_svml_g9_acospif2_native (float2_avx a);

  float2_avx __ocl_svml_g9_atanpif2_native (float2_avx a);

  float2_avx __ocl_svml_g9_atan2pif2_native (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_sinhf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_coshf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_tanhf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_asinhf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_acoshf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_atanhf2_native (float2_avx a);

  float2_avx __ocl_svml_g9_erff2_native (float2_avx a);

  float2_avx __ocl_svml_g9_erfcf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_invf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_divf2_native (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_sqrtf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_rsqrtf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_cbrtf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_rcbrtf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_hypotf2_native (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_powf2_native (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_powrf2_native (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_pownf2_native (float2_avx a, int2_avx b);

  float2_avx __ocl_svml_s9_rootnf2_native (float2_avx a, int2_avx b);

  float2_avx __ocl_svml_s9_expf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_exp2f2_native (float2_avx a);

  float2_avx __ocl_svml_s9_exp10f2_native (float2_avx a);

  float2_avx __ocl_svml_s9_expm1f2_native (float2_avx a);

  float2_avx __ocl_svml_s9_logf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_log10f2_native (float2_avx a);

  float2_avx __ocl_svml_s9_log2f2_native (float2_avx a);

  float2_avx __ocl_svml_s9_log1pf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_sinf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_cosf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_sincosf2_native (float2_avx a, float2_avx * c);

  float2x2_avx __ocl_svml_s9_sincosregf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_tanf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_sinpif2_native (float2_avx a);

  float2_avx __ocl_svml_s9_cospif2_native (float2_avx a);

  float2_avx __ocl_svml_s9_tanpif2_native (float2_avx a);

  float2_avx __ocl_svml_s9_acosf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_asinf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_atanf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_atan2f2_native (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_asinpif2_native (float2_avx a);

  float2_avx __ocl_svml_s9_acospif2_native (float2_avx a);

  float2_avx __ocl_svml_s9_atanpif2_native (float2_avx a);

  float2_avx __ocl_svml_s9_atan2pif2_native (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_sinhf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_coshf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_tanhf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_asinhf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_acoshf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_atanhf2_native (float2_avx a);

  float2_avx __ocl_svml_s9_erff2_native (float2_avx a);

  float2_avx __ocl_svml_s9_erfcf2_native (float2_avx a);

  double2_sse __ocl_svml_n8_inv2_native (double2_sse a);

  double2_sse __ocl_svml_n8_div2_native (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_sqrt2_native (double2_sse a);

  double2_sse __ocl_svml_n8_rsqrt2_native (double2_sse a);

  double2_sse __ocl_svml_n8_cbrt2_native (double2_sse a);

  double2_sse __ocl_svml_n8_rcbrt2_native (double2_sse a);

  double2_sse __ocl_svml_n8_hypot2_native (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_pow2_native (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_powr2_native (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_pown2_native (double2_sse a, int2_sse b);

  double2_sse __ocl_svml_n8_rootn2_native (double2_sse a, int2_sse b);

  double2_sse __ocl_svml_n8_exp2_native (double2_sse a);

  double2_sse __ocl_svml_n8_exp22_native (double2_sse a);

  double2_sse __ocl_svml_n8_exp102_native (double2_sse a);

  double2_sse __ocl_svml_n8_expm12_native (double2_sse a);

  double2_sse __ocl_svml_n8_log2_native (double2_sse a);

  double2_sse __ocl_svml_n8_log102_native (double2_sse a);

  double2_sse __ocl_svml_n8_log22_native (double2_sse a);

  double2_sse __ocl_svml_n8_log1p2_native (double2_sse a);

  double2_sse __ocl_svml_n8_sin2_native (double2_sse a);

  double2_sse __ocl_svml_n8_cos2_native (double2_sse a);

  double2_sse __ocl_svml_n8_sincos2_native (double2_sse a, double2_sse * c);

  double2x2_sse __ocl_svml_n8_sincosreg2_native (double2_sse a);

  double2_sse __ocl_svml_n8_tan2_native (double2_sse a);

  double2_sse __ocl_svml_n8_sinpi2_native (double2_sse a);

  double2_sse __ocl_svml_n8_cospi2_native (double2_sse a);

  double2_sse __ocl_svml_n8_tanpi2_native (double2_sse a);

  double2_sse __ocl_svml_n8_acos2_native (double2_sse a);

  double2_sse __ocl_svml_n8_asin2_native (double2_sse a);

  double2_sse __ocl_svml_n8_atan2_native (double2_sse a);

  double2_sse __ocl_svml_n8_atan22_native (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_asinpi2_native (double2_sse a);

  double2_sse __ocl_svml_n8_acospi2_native (double2_sse a);

  double2_sse __ocl_svml_n8_atanpi2_native (double2_sse a);

  double2_sse __ocl_svml_n8_atan2pi2_native (double2_sse a, double2_sse b);

  double2_sse __ocl_svml_n8_sinh2_native (double2_sse a);

  double2_sse __ocl_svml_n8_cosh2_native (double2_sse a);

  double2_sse __ocl_svml_n8_tanh2_native (double2_sse a);

  double2_sse __ocl_svml_n8_asinh2_native (double2_sse a);

  double2_sse __ocl_svml_n8_acosh2_native (double2_sse a);

  double2_sse __ocl_svml_n8_atanh2_native (double2_sse a);

  double2_sse __ocl_svml_n8_erf2_native (double2_sse a);

  double2_sse __ocl_svml_n8_erfc2_native (double2_sse a);

  double2_avx __ocl_svml_g9_inv2_native (double2_avx a);

  double2_avx __ocl_svml_g9_div2_native (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_sqrt2_native (double2_avx a);

  double2_avx __ocl_svml_g9_rsqrt2_native (double2_avx a);

  double2_avx __ocl_svml_g9_cbrt2_native (double2_avx a);

  double2_avx __ocl_svml_g9_rcbrt2_native (double2_avx a);

  double2_avx __ocl_svml_g9_hypot2_native (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_pow2_native (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_powr2_native (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_pown2_native (double2_avx a, int2_avx b);

  double2_avx __ocl_svml_g9_rootn2_native (double2_avx a, int2_avx b);

  double2_avx __ocl_svml_g9_exp2_native (double2_avx a);

  double2_avx __ocl_svml_g9_exp22_native (double2_avx a);

  double2_avx __ocl_svml_g9_exp102_native (double2_avx a);

  double2_avx __ocl_svml_g9_expm12_native (double2_avx a);

  double2_avx __ocl_svml_g9_log2_native (double2_avx a);

  double2_avx __ocl_svml_g9_log102_native (double2_avx a);

  double2_avx __ocl_svml_g9_log22_native (double2_avx a);

  double2_avx __ocl_svml_g9_log1p2_native (double2_avx a);

  double2_avx __ocl_svml_g9_sin2_native (double2_avx a);

  double2_avx __ocl_svml_g9_cos2_native (double2_avx a);

  double2_avx __ocl_svml_g9_sincos2_native (double2_avx a, double2_avx * c);

  double2x2_avx __ocl_svml_g9_sincosreg2_native (double2_avx a);

  double2_avx __ocl_svml_g9_tan2_native (double2_avx a);

  double2_avx __ocl_svml_g9_sinpi2_native (double2_avx a);

  double2_avx __ocl_svml_g9_cospi2_native (double2_avx a);

  double2_avx __ocl_svml_g9_tanpi2_native (double2_avx a);

  double2_avx __ocl_svml_g9_acos2_native (double2_avx a);

  double2_avx __ocl_svml_g9_asin2_native (double2_avx a);

  double2_avx __ocl_svml_g9_atan2_native (double2_avx a);

  double2_avx __ocl_svml_g9_atan22_native (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_asinpi2_native (double2_avx a);

  double2_avx __ocl_svml_g9_acospi2_native (double2_avx a);

  double2_avx __ocl_svml_g9_atanpi2_native (double2_avx a);

  double2_avx __ocl_svml_g9_atan2pi2_native (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_g9_sinh2_native (double2_avx a);

  double2_avx __ocl_svml_g9_cosh2_native (double2_avx a);

  double2_avx __ocl_svml_g9_tanh2_native (double2_avx a);

  double2_avx __ocl_svml_g9_asinh2_native (double2_avx a);

  double2_avx __ocl_svml_g9_acosh2_native (double2_avx a);

  double2_avx __ocl_svml_g9_atanh2_native (double2_avx a);

  double2_avx __ocl_svml_g9_erf2_native (double2_avx a);

  double2_avx __ocl_svml_g9_erfc2_native (double2_avx a);

  double2_avx __ocl_svml_s9_inv2_native (double2_avx a);

  double2_avx __ocl_svml_s9_div2_native (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_sqrt2_native (double2_avx a);

  double2_avx __ocl_svml_s9_rsqrt2_native (double2_avx a);

  double2_avx __ocl_svml_s9_cbrt2_native (double2_avx a);

  double2_avx __ocl_svml_s9_rcbrt2_native (double2_avx a);

  double2_avx __ocl_svml_s9_hypot2_native (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_pow2_native (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_powr2_native (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_pown2_native (double2_avx a, int2_avx b);

  double2_avx __ocl_svml_s9_rootn2_native (double2_avx a, int2_avx b);

  double2_avx __ocl_svml_s9_exp2_native (double2_avx a);

  double2_avx __ocl_svml_s9_exp22_native (double2_avx a);

  double2_avx __ocl_svml_s9_exp102_native (double2_avx a);

  double2_avx __ocl_svml_s9_expm12_native (double2_avx a);

  double2_avx __ocl_svml_s9_log2_native (double2_avx a);

  double2_avx __ocl_svml_s9_log102_native (double2_avx a);

  double2_avx __ocl_svml_s9_log22_native (double2_avx a);

  double2_avx __ocl_svml_s9_log1p2_native (double2_avx a);

  double2_avx __ocl_svml_s9_sin2_native (double2_avx a);

  double2_avx __ocl_svml_s9_cos2_native (double2_avx a);

  double2_avx __ocl_svml_s9_sincos2_native (double2_avx a, double2_avx * c);

  double2x2_avx __ocl_svml_s9_sincosreg2_native (double2_avx a);

  double2_avx __ocl_svml_s9_tan2_native (double2_avx a);

  double2_avx __ocl_svml_s9_sinpi2_native (double2_avx a);

  double2_avx __ocl_svml_s9_cospi2_native (double2_avx a);

  double2_avx __ocl_svml_s9_tanpi2_native (double2_avx a);

  double2_avx __ocl_svml_s9_acos2_native (double2_avx a);

  double2_avx __ocl_svml_s9_asin2_native (double2_avx a);

  double2_avx __ocl_svml_s9_atan2_native (double2_avx a);

  double2_avx __ocl_svml_s9_atan22_native (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_asinpi2_native (double2_avx a);

  double2_avx __ocl_svml_s9_acospi2_native (double2_avx a);

  double2_avx __ocl_svml_s9_atanpi2_native (double2_avx a);

  double2_avx __ocl_svml_s9_atan2pi2_native (double2_avx a, double2_avx b);

  double2_avx __ocl_svml_s9_sinh2_native (double2_avx a);

  double2_avx __ocl_svml_s9_cosh2_native (double2_avx a);

  double2_avx __ocl_svml_s9_tanh2_native (double2_avx a);

  double2_avx __ocl_svml_s9_asinh2_native (double2_avx a);

  double2_avx __ocl_svml_s9_acosh2_native (double2_avx a);

  double2_avx __ocl_svml_s9_atanh2_native (double2_avx a);

  double2_avx __ocl_svml_s9_erf2_native (double2_avx a);

  double2_avx __ocl_svml_s9_erfc2_native (double2_avx a);

  float3_sse __ocl_svml_n8_invf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_divf3_native (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_sqrtf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_rsqrtf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_cbrtf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_rcbrtf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_hypotf3_native (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_powf3_native (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_powrf3_native (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_pownf3_native (float3_sse a, int3_sse b);

  float3_sse __ocl_svml_n8_rootnf3_native (float3_sse a, int3_sse b);

  float3_sse __ocl_svml_n8_expf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_exp2f3_native (float3_sse a);

  float3_sse __ocl_svml_n8_exp10f3_native (float3_sse a);

  float3_sse __ocl_svml_n8_expm1f3_native (float3_sse a);

  float3_sse __ocl_svml_n8_logf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_log10f3_native (float3_sse a);

  float3_sse __ocl_svml_n8_log2f3_native (float3_sse a);

  float3_sse __ocl_svml_n8_log1pf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_sinf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_cosf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_sincosf3_native (float3_sse a, float3_sse * c);

  float3x2_sse __ocl_svml_n8_sincosregf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_tanf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_sinpif3_native (float3_sse a);

  float3_sse __ocl_svml_n8_cospif3_native (float3_sse a);

  float3_sse __ocl_svml_n8_tanpif3_native (float3_sse a);

  float3_sse __ocl_svml_n8_acosf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_asinf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_atanf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_atan2f3_native (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_asinpif3_native (float3_sse a);

  float3_sse __ocl_svml_n8_acospif3_native (float3_sse a);

  float3_sse __ocl_svml_n8_atanpif3_native (float3_sse a);

  float3_sse __ocl_svml_n8_atan2pif3_native (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_sinhf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_coshf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_tanhf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_asinhf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_acoshf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_atanhf3_native (float3_sse a);

  float3_sse __ocl_svml_n8_erff3_native (float3_sse a);

  float3_sse __ocl_svml_n8_erfcf3_native (float3_sse a);

  float3_avx __ocl_svml_g9_invf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_divf3_native (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_sqrtf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_rsqrtf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_cbrtf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_rcbrtf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_hypotf3_native (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_powf3_native (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_powrf3_native (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_pownf3_native (float3_avx a, int3_avx b);

  float3_avx __ocl_svml_g9_rootnf3_native (float3_avx a, int3_avx b);

  float3_avx __ocl_svml_g9_expf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_exp2f3_native (float3_avx a);

  float3_avx __ocl_svml_g9_exp10f3_native (float3_avx a);

  float3_avx __ocl_svml_g9_expm1f3_native (float3_avx a);

  float3_avx __ocl_svml_g9_logf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_log10f3_native (float3_avx a);

  float3_avx __ocl_svml_g9_log2f3_native (float3_avx a);

  float3_avx __ocl_svml_g9_log1pf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_sinf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_cosf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_sincosf3_native (float3_avx a, float3_avx * c);

  float3x2_avx __ocl_svml_g9_sincosregf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_tanf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_sinpif3_native (float3_avx a);

  float3_avx __ocl_svml_g9_cospif3_native (float3_avx a);

  float3_avx __ocl_svml_g9_tanpif3_native (float3_avx a);

  float3_avx __ocl_svml_g9_acosf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_asinf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_atanf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_atan2f3_native (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_asinpif3_native (float3_avx a);

  float3_avx __ocl_svml_g9_acospif3_native (float3_avx a);

  float3_avx __ocl_svml_g9_atanpif3_native (float3_avx a);

  float3_avx __ocl_svml_g9_atan2pif3_native (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_sinhf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_coshf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_tanhf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_asinhf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_acoshf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_atanhf3_native (float3_avx a);

  float3_avx __ocl_svml_g9_erff3_native (float3_avx a);

  float3_avx __ocl_svml_g9_erfcf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_invf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_divf3_native (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_sqrtf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_rsqrtf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_cbrtf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_rcbrtf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_hypotf3_native (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_powf3_native (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_powrf3_native (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_pownf3_native (float3_avx a, int3_avx b);

  float3_avx __ocl_svml_s9_rootnf3_native (float3_avx a, int3_avx b);

  float3_avx __ocl_svml_s9_expf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_exp2f3_native (float3_avx a);

  float3_avx __ocl_svml_s9_exp10f3_native (float3_avx a);

  float3_avx __ocl_svml_s9_expm1f3_native (float3_avx a);

  float3_avx __ocl_svml_s9_logf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_log10f3_native (float3_avx a);

  float3_avx __ocl_svml_s9_log2f3_native (float3_avx a);

  float3_avx __ocl_svml_s9_log1pf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_sinf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_cosf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_sincosf3_native (float3_avx a, float3_avx * c);

  float3x2_avx __ocl_svml_s9_sincosregf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_tanf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_sinpif3_native (float3_avx a);

  float3_avx __ocl_svml_s9_cospif3_native (float3_avx a);

  float3_avx __ocl_svml_s9_tanpif3_native (float3_avx a);

  float3_avx __ocl_svml_s9_acosf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_asinf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_atanf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_atan2f3_native (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_asinpif3_native (float3_avx a);

  float3_avx __ocl_svml_s9_acospif3_native (float3_avx a);

  float3_avx __ocl_svml_s9_atanpif3_native (float3_avx a);

  float3_avx __ocl_svml_s9_atan2pif3_native (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_sinhf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_coshf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_tanhf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_asinhf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_acoshf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_atanhf3_native (float3_avx a);

  float3_avx __ocl_svml_s9_erff3_native (float3_avx a);

  float3_avx __ocl_svml_s9_erfcf3_native (float3_avx a);

  double3_sse __ocl_svml_n8_inv3_native (double3_sse a);

  double3_sse __ocl_svml_n8_div3_native (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_sqrt3_native (double3_sse a);

  double3_sse __ocl_svml_n8_rsqrt3_native (double3_sse a);

  double3_sse __ocl_svml_n8_cbrt3_native (double3_sse a);

  double3_sse __ocl_svml_n8_rcbrt3_native (double3_sse a);

  double3_sse __ocl_svml_n8_hypot3_native (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_pow3_native (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_powr3_native (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_pown3_native (double3_sse a, int3_sse b);

  double3_sse __ocl_svml_n8_rootn3_native (double3_sse a, int3_sse b);

  double3_sse __ocl_svml_n8_exp3_native (double3_sse a);

  double3_sse __ocl_svml_n8_exp23_native (double3_sse a);

  double3_sse __ocl_svml_n8_exp103_native (double3_sse a);

  double3_sse __ocl_svml_n8_expm13_native (double3_sse a);

  double3_sse __ocl_svml_n8_log3_native (double3_sse a);

  double3_sse __ocl_svml_n8_log103_native (double3_sse a);

  double3_sse __ocl_svml_n8_log23_native (double3_sse a);

  double3_sse __ocl_svml_n8_log1p3_native (double3_sse a);

  double3_sse __ocl_svml_n8_sin3_native (double3_sse a);

  double3_sse __ocl_svml_n8_cos3_native (double3_sse a);

  double3_sse __ocl_svml_n8_sincos3_native (double3_sse a, double3_sse * c);

  double3x2_sse __ocl_svml_n8_sincosreg3_native (double3_sse a);

  double3_sse __ocl_svml_n8_tan3_native (double3_sse a);

  double3_sse __ocl_svml_n8_sinpi3_native (double3_sse a);

  double3_sse __ocl_svml_n8_cospi3_native (double3_sse a);

  double3_sse __ocl_svml_n8_tanpi3_native (double3_sse a);

  double3_sse __ocl_svml_n8_acos3_native (double3_sse a);

  double3_sse __ocl_svml_n8_asin3_native (double3_sse a);

  double3_sse __ocl_svml_n8_atan3_native (double3_sse a);

  double3_sse __ocl_svml_n8_atan23_native (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_asinpi3_native (double3_sse a);

  double3_sse __ocl_svml_n8_acospi3_native (double3_sse a);

  double3_sse __ocl_svml_n8_atanpi3_native (double3_sse a);

  double3_sse __ocl_svml_n8_atan2pi3_native (double3_sse a, double3_sse b);

  double3_sse __ocl_svml_n8_sinh3_native (double3_sse a);

  double3_sse __ocl_svml_n8_cosh3_native (double3_sse a);

  double3_sse __ocl_svml_n8_tanh3_native (double3_sse a);

  double3_sse __ocl_svml_n8_asinh3_native (double3_sse a);

  double3_sse __ocl_svml_n8_acosh3_native (double3_sse a);

  double3_sse __ocl_svml_n8_atanh3_native (double3_sse a);

  double3_sse __ocl_svml_n8_erf3_native (double3_sse a);

  double3_sse __ocl_svml_n8_erfc3_native (double3_sse a);

  double3_avx __ocl_svml_g9_inv3_native (double3_avx a);

  double3_avx __ocl_svml_g9_div3_native (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_sqrt3_native (double3_avx a);

  double3_avx __ocl_svml_g9_rsqrt3_native (double3_avx a);

  double3_avx __ocl_svml_g9_cbrt3_native (double3_avx a);

  double3_avx __ocl_svml_g9_rcbrt3_native (double3_avx a);

  double3_avx __ocl_svml_g9_hypot3_native (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_pow3_native (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_powr3_native (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_pown3_native (double3_avx a, int3_avx b);

  double3_avx __ocl_svml_g9_rootn3_native (double3_avx a, int3_avx b);

  double3_avx __ocl_svml_g9_exp3_native (double3_avx a);

  double3_avx __ocl_svml_g9_exp23_native (double3_avx a);

  double3_avx __ocl_svml_g9_exp103_native (double3_avx a);

  double3_avx __ocl_svml_g9_expm13_native (double3_avx a);

  double3_avx __ocl_svml_g9_log3_native (double3_avx a);

  double3_avx __ocl_svml_g9_log103_native (double3_avx a);

  double3_avx __ocl_svml_g9_log23_native (double3_avx a);

  double3_avx __ocl_svml_g9_log1p3_native (double3_avx a);

  double3_avx __ocl_svml_g9_sin3_native (double3_avx a);

  double3_avx __ocl_svml_g9_cos3_native (double3_avx a);

  double3_avx __ocl_svml_g9_sincos3_native (double3_avx a, double3_avx * c);

  double3x2_avx __ocl_svml_g9_sincosreg3_native (double3_avx a);

  double3_avx __ocl_svml_g9_tan3_native (double3_avx a);

  double3_avx __ocl_svml_g9_sinpi3_native (double3_avx a);

  double3_avx __ocl_svml_g9_cospi3_native (double3_avx a);

  double3_avx __ocl_svml_g9_tanpi3_native (double3_avx a);

  double3_avx __ocl_svml_g9_acos3_native (double3_avx a);

  double3_avx __ocl_svml_g9_asin3_native (double3_avx a);

  double3_avx __ocl_svml_g9_atan3_native (double3_avx a);

  double3_avx __ocl_svml_g9_atan23_native (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_asinpi3_native (double3_avx a);

  double3_avx __ocl_svml_g9_acospi3_native (double3_avx a);

  double3_avx __ocl_svml_g9_atanpi3_native (double3_avx a);

  double3_avx __ocl_svml_g9_atan2pi3_native (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_g9_sinh3_native (double3_avx a);

  double3_avx __ocl_svml_g9_cosh3_native (double3_avx a);

  double3_avx __ocl_svml_g9_tanh3_native (double3_avx a);

  double3_avx __ocl_svml_g9_asinh3_native (double3_avx a);

  double3_avx __ocl_svml_g9_acosh3_native (double3_avx a);

  double3_avx __ocl_svml_g9_atanh3_native (double3_avx a);

  double3_avx __ocl_svml_g9_erf3_native (double3_avx a);

  double3_avx __ocl_svml_g9_erfc3_native (double3_avx a);

  double3_avx __ocl_svml_s9_inv3_native (double3_avx a);

  double3_avx __ocl_svml_s9_div3_native (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_sqrt3_native (double3_avx a);

  double3_avx __ocl_svml_s9_rsqrt3_native (double3_avx a);

  double3_avx __ocl_svml_s9_cbrt3_native (double3_avx a);

  double3_avx __ocl_svml_s9_rcbrt3_native (double3_avx a);

  double3_avx __ocl_svml_s9_hypot3_native (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_pow3_native (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_powr3_native (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_pown3_native (double3_avx a, int3_avx b);

  double3_avx __ocl_svml_s9_rootn3_native (double3_avx a, int3_avx b);

  double3_avx __ocl_svml_s9_exp3_native (double3_avx a);

  double3_avx __ocl_svml_s9_exp23_native (double3_avx a);

  double3_avx __ocl_svml_s9_exp103_native (double3_avx a);

  double3_avx __ocl_svml_s9_expm13_native (double3_avx a);

  double3_avx __ocl_svml_s9_log3_native (double3_avx a);

  double3_avx __ocl_svml_s9_log103_native (double3_avx a);

  double3_avx __ocl_svml_s9_log23_native (double3_avx a);

  double3_avx __ocl_svml_s9_log1p3_native (double3_avx a);

  double3_avx __ocl_svml_s9_sin3_native (double3_avx a);

  double3_avx __ocl_svml_s9_cos3_native (double3_avx a);

  double3_avx __ocl_svml_s9_sincos3_native (double3_avx a, double3_avx * c);

  double3x2_avx __ocl_svml_s9_sincosreg3_native (double3_avx a);

  double3_avx __ocl_svml_s9_tan3_native (double3_avx a);

  double3_avx __ocl_svml_s9_sinpi3_native (double3_avx a);

  double3_avx __ocl_svml_s9_cospi3_native (double3_avx a);

  double3_avx __ocl_svml_s9_tanpi3_native (double3_avx a);

  double3_avx __ocl_svml_s9_acos3_native (double3_avx a);

  double3_avx __ocl_svml_s9_asin3_native (double3_avx a);

  double3_avx __ocl_svml_s9_atan3_native (double3_avx a);

  double3_avx __ocl_svml_s9_atan23_native (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_asinpi3_native (double3_avx a);

  double3_avx __ocl_svml_s9_acospi3_native (double3_avx a);

  double3_avx __ocl_svml_s9_atanpi3_native (double3_avx a);

  double3_avx __ocl_svml_s9_atan2pi3_native (double3_avx a, double3_avx b);

  double3_avx __ocl_svml_s9_sinh3_native (double3_avx a);

  double3_avx __ocl_svml_s9_cosh3_native (double3_avx a);

  double3_avx __ocl_svml_s9_tanh3_native (double3_avx a);

  double3_avx __ocl_svml_s9_asinh3_native (double3_avx a);

  double3_avx __ocl_svml_s9_acosh3_native (double3_avx a);

  double3_avx __ocl_svml_s9_atanh3_native (double3_avx a);

  double3_avx __ocl_svml_s9_erf3_native (double3_avx a);

  double3_avx __ocl_svml_s9_erfc3_native (double3_avx a);

  float4_sse __ocl_svml_n8_invf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_divf4_native (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_sqrtf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_rsqrtf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_cbrtf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_rcbrtf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_hypotf4_native (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_powf4_native (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_powrf4_native (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_pownf4_native (float4_sse a, int4_sse b);

  float4_sse __ocl_svml_n8_rootnf4_native (float4_sse a, int4_sse b);

  float4_sse __ocl_svml_n8_expf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_exp2f4_native (float4_sse a);

  float4_sse __ocl_svml_n8_exp10f4_native (float4_sse a);

  float4_sse __ocl_svml_n8_expm1f4_native (float4_sse a);

  float4_sse __ocl_svml_n8_logf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_log10f4_native (float4_sse a);

  float4_sse __ocl_svml_n8_log2f4_native (float4_sse a);

  float4_sse __ocl_svml_n8_log1pf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_sinf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_cosf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_sincosf4_native (float4_sse a, float4_sse * c);

  float4x2_sse __ocl_svml_n8_sincosregf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_tanf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_sinpif4_native (float4_sse a);

  float4_sse __ocl_svml_n8_cospif4_native (float4_sse a);

  float4_sse __ocl_svml_n8_tanpif4_native (float4_sse a);

  float4_sse __ocl_svml_n8_acosf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_asinf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_atanf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_atan2f4_native (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_asinpif4_native (float4_sse a);

  float4_sse __ocl_svml_n8_acospif4_native (float4_sse a);

  float4_sse __ocl_svml_n8_atanpif4_native (float4_sse a);

  float4_sse __ocl_svml_n8_atan2pif4_native (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_sinhf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_coshf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_tanhf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_asinhf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_acoshf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_atanhf4_native (float4_sse a);

  float4_sse __ocl_svml_n8_erff4_native (float4_sse a);

  float4_sse __ocl_svml_n8_erfcf4_native (float4_sse a);

  float4_avx __ocl_svml_g9_invf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_divf4_native (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_sqrtf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_rsqrtf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_cbrtf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_rcbrtf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_hypotf4_native (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_powf4_native (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_powrf4_native (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_pownf4_native (float4_avx a, int4_avx b);

  float4_avx __ocl_svml_g9_rootnf4_native (float4_avx a, int4_avx b);

  float4_avx __ocl_svml_g9_expf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_exp2f4_native (float4_avx a);

  float4_avx __ocl_svml_g9_exp10f4_native (float4_avx a);

  float4_avx __ocl_svml_g9_expm1f4_native (float4_avx a);

  float4_avx __ocl_svml_g9_logf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_log10f4_native (float4_avx a);

  float4_avx __ocl_svml_g9_log2f4_native (float4_avx a);

  float4_avx __ocl_svml_g9_log1pf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_sinf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_cosf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_sincosf4_native (float4_avx a, float4_avx * c);

  float4x2_avx __ocl_svml_g9_sincosregf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_tanf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_sinpif4_native (float4_avx a);

  float4_avx __ocl_svml_g9_cospif4_native (float4_avx a);

  float4_avx __ocl_svml_g9_tanpif4_native (float4_avx a);

  float4_avx __ocl_svml_g9_acosf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_asinf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_atanf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_atan2f4_native (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_asinpif4_native (float4_avx a);

  float4_avx __ocl_svml_g9_acospif4_native (float4_avx a);

  float4_avx __ocl_svml_g9_atanpif4_native (float4_avx a);

  float4_avx __ocl_svml_g9_atan2pif4_native (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_sinhf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_coshf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_tanhf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_asinhf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_acoshf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_atanhf4_native (float4_avx a);

  float4_avx __ocl_svml_g9_erff4_native (float4_avx a);

  float4_avx __ocl_svml_g9_erfcf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_invf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_divf4_native (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_sqrtf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_rsqrtf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_cbrtf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_rcbrtf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_hypotf4_native (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_powf4_native (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_powrf4_native (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_pownf4_native (float4_avx a, int4_avx b);

  float4_avx __ocl_svml_s9_rootnf4_native (float4_avx a, int4_avx b);

  float4_avx __ocl_svml_s9_expf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_exp2f4_native (float4_avx a);

  float4_avx __ocl_svml_s9_exp10f4_native (float4_avx a);

  float4_avx __ocl_svml_s9_expm1f4_native (float4_avx a);

  float4_avx __ocl_svml_s9_logf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_log10f4_native (float4_avx a);

  float4_avx __ocl_svml_s9_log2f4_native (float4_avx a);

  float4_avx __ocl_svml_s9_log1pf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_sinf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_cosf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_sincosf4_native (float4_avx a, float4_avx * c);

  float4x2_avx __ocl_svml_s9_sincosregf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_tanf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_sinpif4_native (float4_avx a);

  float4_avx __ocl_svml_s9_cospif4_native (float4_avx a);

  float4_avx __ocl_svml_s9_tanpif4_native (float4_avx a);

  float4_avx __ocl_svml_s9_acosf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_asinf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_atanf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_atan2f4_native (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_asinpif4_native (float4_avx a);

  float4_avx __ocl_svml_s9_acospif4_native (float4_avx a);

  float4_avx __ocl_svml_s9_atanpif4_native (float4_avx a);

  float4_avx __ocl_svml_s9_atan2pif4_native (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_sinhf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_coshf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_tanhf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_asinhf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_acoshf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_atanhf4_native (float4_avx a);

  float4_avx __ocl_svml_s9_erff4_native (float4_avx a);

  float4_avx __ocl_svml_s9_erfcf4_native (float4_avx a);

  double4_sse __ocl_svml_n8_inv4_native (double4_sse a);

  double4_sse __ocl_svml_n8_div4_native (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_sqrt4_native (double4_sse a);

  double4_sse __ocl_svml_n8_rsqrt4_native (double4_sse a);

  double4_sse __ocl_svml_n8_cbrt4_native (double4_sse a);

  double4_sse __ocl_svml_n8_rcbrt4_native (double4_sse a);

  double4_sse __ocl_svml_n8_hypot4_native (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_pow4_native (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_powr4_native (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_pown4_native (double4_sse a, int4_sse b);

  double4_sse __ocl_svml_n8_rootn4_native (double4_sse a, int4_sse b);

  double4_sse __ocl_svml_n8_exp4_native (double4_sse a);

  double4_sse __ocl_svml_n8_exp24_native (double4_sse a);

  double4_sse __ocl_svml_n8_exp104_native (double4_sse a);

  double4_sse __ocl_svml_n8_expm14_native (double4_sse a);

  double4_sse __ocl_svml_n8_log4_native (double4_sse a);

  double4_sse __ocl_svml_n8_log104_native (double4_sse a);

  double4_sse __ocl_svml_n8_log24_native (double4_sse a);

  double4_sse __ocl_svml_n8_log1p4_native (double4_sse a);

  double4_sse __ocl_svml_n8_sin4_native (double4_sse a);

  double4_sse __ocl_svml_n8_cos4_native (double4_sse a);

  double4_sse __ocl_svml_n8_sincos4_native (double4_sse a, double4_sse * c);

  double4x2_sse __ocl_svml_n8_sincosreg4_native (double4_sse a);

  double4_sse __ocl_svml_n8_tan4_native (double4_sse a);

  double4_sse __ocl_svml_n8_sinpi4_native (double4_sse a);

  double4_sse __ocl_svml_n8_cospi4_native (double4_sse a);

  double4_sse __ocl_svml_n8_tanpi4_native (double4_sse a);

  double4_sse __ocl_svml_n8_acos4_native (double4_sse a);

  double4_sse __ocl_svml_n8_asin4_native (double4_sse a);

  double4_sse __ocl_svml_n8_atan4_native (double4_sse a);

  double4_sse __ocl_svml_n8_atan24_native (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_asinpi4_native (double4_sse a);

  double4_sse __ocl_svml_n8_acospi4_native (double4_sse a);

  double4_sse __ocl_svml_n8_atanpi4_native (double4_sse a);

  double4_sse __ocl_svml_n8_atan2pi4_native (double4_sse a, double4_sse b);

  double4_sse __ocl_svml_n8_sinh4_native (double4_sse a);

  double4_sse __ocl_svml_n8_cosh4_native (double4_sse a);

  double4_sse __ocl_svml_n8_tanh4_native (double4_sse a);

  double4_sse __ocl_svml_n8_asinh4_native (double4_sse a);

  double4_sse __ocl_svml_n8_acosh4_native (double4_sse a);

  double4_sse __ocl_svml_n8_atanh4_native (double4_sse a);

  double4_sse __ocl_svml_n8_erf4_native (double4_sse a);

  double4_sse __ocl_svml_n8_erfc4_native (double4_sse a);

  double4_avx __ocl_svml_g9_inv4_native (double4_avx a);

  double4_avx __ocl_svml_g9_div4_native (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_sqrt4_native (double4_avx a);

  double4_avx __ocl_svml_g9_rsqrt4_native (double4_avx a);

  double4_avx __ocl_svml_g9_cbrt4_native (double4_avx a);

  double4_avx __ocl_svml_g9_rcbrt4_native (double4_avx a);

  double4_avx __ocl_svml_g9_hypot4_native (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_pow4_native (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_powr4_native (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_pown4_native (double4_avx a, int4_avx b);

  double4_avx __ocl_svml_g9_rootn4_native (double4_avx a, int4_avx b);

  double4_avx __ocl_svml_g9_exp4_native (double4_avx a);

  double4_avx __ocl_svml_g9_exp24_native (double4_avx a);

  double4_avx __ocl_svml_g9_exp104_native (double4_avx a);

  double4_avx __ocl_svml_g9_expm14_native (double4_avx a);

  double4_avx __ocl_svml_g9_log4_native (double4_avx a);

  double4_avx __ocl_svml_g9_log104_native (double4_avx a);

  double4_avx __ocl_svml_g9_log24_native (double4_avx a);

  double4_avx __ocl_svml_g9_log1p4_native (double4_avx a);

  double4_avx __ocl_svml_g9_sin4_native (double4_avx a);

  double4_avx __ocl_svml_g9_cos4_native (double4_avx a);

  double4_avx __ocl_svml_g9_sincos4_native (double4_avx a, double4_avx * c);

  double4x2_avx __ocl_svml_g9_sincosreg4_native (double4_avx a);

  double4_avx __ocl_svml_g9_tan4_native (double4_avx a);

  double4_avx __ocl_svml_g9_sinpi4_native (double4_avx a);

  double4_avx __ocl_svml_g9_cospi4_native (double4_avx a);

  double4_avx __ocl_svml_g9_tanpi4_native (double4_avx a);

  double4_avx __ocl_svml_g9_acos4_native (double4_avx a);

  double4_avx __ocl_svml_g9_asin4_native (double4_avx a);

  double4_avx __ocl_svml_g9_atan4_native (double4_avx a);

  double4_avx __ocl_svml_g9_atan24_native (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_asinpi4_native (double4_avx a);

  double4_avx __ocl_svml_g9_acospi4_native (double4_avx a);

  double4_avx __ocl_svml_g9_atanpi4_native (double4_avx a);

  double4_avx __ocl_svml_g9_atan2pi4_native (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_g9_sinh4_native (double4_avx a);

  double4_avx __ocl_svml_g9_cosh4_native (double4_avx a);

  double4_avx __ocl_svml_g9_tanh4_native (double4_avx a);

  double4_avx __ocl_svml_g9_asinh4_native (double4_avx a);

  double4_avx __ocl_svml_g9_acosh4_native (double4_avx a);

  double4_avx __ocl_svml_g9_atanh4_native (double4_avx a);

  double4_avx __ocl_svml_g9_erf4_native (double4_avx a);

  double4_avx __ocl_svml_g9_erfc4_native (double4_avx a);

  double4_avx __ocl_svml_s9_inv4_native (double4_avx a);

  double4_avx __ocl_svml_s9_div4_native (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_sqrt4_native (double4_avx a);

  double4_avx __ocl_svml_s9_rsqrt4_native (double4_avx a);

  double4_avx __ocl_svml_s9_cbrt4_native (double4_avx a);

  double4_avx __ocl_svml_s9_rcbrt4_native (double4_avx a);

  double4_avx __ocl_svml_s9_hypot4_native (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_pow4_native (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_powr4_native (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_pown4_native (double4_avx a, int4_avx b);

  double4_avx __ocl_svml_s9_rootn4_native (double4_avx a, int4_avx b);

  double4_avx __ocl_svml_s9_exp4_native (double4_avx a);

  double4_avx __ocl_svml_s9_exp24_native (double4_avx a);

  double4_avx __ocl_svml_s9_exp104_native (double4_avx a);

  double4_avx __ocl_svml_s9_expm14_native (double4_avx a);

  double4_avx __ocl_svml_s9_log4_native (double4_avx a);

  double4_avx __ocl_svml_s9_log104_native (double4_avx a);

  double4_avx __ocl_svml_s9_log24_native (double4_avx a);

  double4_avx __ocl_svml_s9_log1p4_native (double4_avx a);

  double4_avx __ocl_svml_s9_sin4_native (double4_avx a);

  double4_avx __ocl_svml_s9_cos4_native (double4_avx a);

  double4_avx __ocl_svml_s9_sincos4_native (double4_avx a, double4_avx * c);

  double4x2_avx __ocl_svml_s9_sincosreg4_native (double4_avx a);

  double4_avx __ocl_svml_s9_tan4_native (double4_avx a);

  double4_avx __ocl_svml_s9_sinpi4_native (double4_avx a);

  double4_avx __ocl_svml_s9_cospi4_native (double4_avx a);

  double4_avx __ocl_svml_s9_tanpi4_native (double4_avx a);

  double4_avx __ocl_svml_s9_acos4_native (double4_avx a);

  double4_avx __ocl_svml_s9_asin4_native (double4_avx a);

  double4_avx __ocl_svml_s9_atan4_native (double4_avx a);

  double4_avx __ocl_svml_s9_atan24_native (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_asinpi4_native (double4_avx a);

  double4_avx __ocl_svml_s9_acospi4_native (double4_avx a);

  double4_avx __ocl_svml_s9_atanpi4_native (double4_avx a);

  double4_avx __ocl_svml_s9_atan2pi4_native (double4_avx a, double4_avx b);

  double4_avx __ocl_svml_s9_sinh4_native (double4_avx a);

  double4_avx __ocl_svml_s9_cosh4_native (double4_avx a);

  double4_avx __ocl_svml_s9_tanh4_native (double4_avx a);

  double4_avx __ocl_svml_s9_asinh4_native (double4_avx a);

  double4_avx __ocl_svml_s9_acosh4_native (double4_avx a);

  double4_avx __ocl_svml_s9_atanh4_native (double4_avx a);

  double4_avx __ocl_svml_s9_erf4_native (double4_avx a);

  double4_avx __ocl_svml_s9_erfc4_native (double4_avx a);

  float8_sse __ocl_svml_n8_invf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_divf8_native (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_sqrtf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_rsqrtf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_cbrtf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_rcbrtf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_hypotf8_native (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_powf8_native (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_powrf8_native (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_pownf8_native (float8_sse a, int8_sse b);

  float8_sse __ocl_svml_n8_rootnf8_native (float8_sse a, int8_sse b);

  float8_sse __ocl_svml_n8_expf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_exp2f8_native (float8_sse a);

  float8_sse __ocl_svml_n8_exp10f8_native (float8_sse a);

  float8_sse __ocl_svml_n8_expm1f8_native (float8_sse a);

  float8_sse __ocl_svml_n8_logf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_log10f8_native (float8_sse a);

  float8_sse __ocl_svml_n8_log2f8_native (float8_sse a);

  float8_sse __ocl_svml_n8_log1pf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_sinf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_cosf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_sincosf8_native (float8_sse a, float8_sse * c);

  float8x2_sse __ocl_svml_n8_sincosregf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_tanf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_sinpif8_native (float8_sse a);

  float8_sse __ocl_svml_n8_cospif8_native (float8_sse a);

  float8_sse __ocl_svml_n8_tanpif8_native (float8_sse a);

  float8_sse __ocl_svml_n8_acosf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_asinf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_atanf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_atan2f8_native (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_asinpif8_native (float8_sse a);

  float8_sse __ocl_svml_n8_acospif8_native (float8_sse a);

  float8_sse __ocl_svml_n8_atanpif8_native (float8_sse a);

  float8_sse __ocl_svml_n8_atan2pif8_native (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_sinhf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_coshf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_tanhf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_asinhf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_acoshf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_atanhf8_native (float8_sse a);

  float8_sse __ocl_svml_n8_erff8_native (float8_sse a);

  float8_sse __ocl_svml_n8_erfcf8_native (float8_sse a);

  float8_avx __ocl_svml_g9_invf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_divf8_native (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_sqrtf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_rsqrtf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_cbrtf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_rcbrtf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_hypotf8_native (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_powf8_native (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_powrf8_native (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_pownf8_native (float8_avx a, int8_avx b);

  float8_avx __ocl_svml_g9_rootnf8_native (float8_avx a, int8_avx b);

  float8_avx __ocl_svml_g9_expf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_exp2f8_native (float8_avx a);

  float8_avx __ocl_svml_g9_exp10f8_native (float8_avx a);

  float8_avx __ocl_svml_g9_expm1f8_native (float8_avx a);

  float8_avx __ocl_svml_g9_logf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_log10f8_native (float8_avx a);

  float8_avx __ocl_svml_g9_log2f8_native (float8_avx a);

  float8_avx __ocl_svml_g9_log1pf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_sinf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_cosf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_sincosf8_native (float8_avx a, float8_avx * c);

  float8x2_avx __ocl_svml_g9_sincosregf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_tanf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_sinpif8_native (float8_avx a);

  float8_avx __ocl_svml_g9_cospif8_native (float8_avx a);

  float8_avx __ocl_svml_g9_tanpif8_native (float8_avx a);

  float8_avx __ocl_svml_g9_acosf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_asinf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_atanf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_atan2f8_native (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_asinpif8_native (float8_avx a);

  float8_avx __ocl_svml_g9_acospif8_native (float8_avx a);

  float8_avx __ocl_svml_g9_atanpif8_native (float8_avx a);

  float8_avx __ocl_svml_g9_atan2pif8_native (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_sinhf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_coshf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_tanhf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_asinhf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_acoshf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_atanhf8_native (float8_avx a);

  float8_avx __ocl_svml_g9_erff8_native (float8_avx a);

  float8_avx __ocl_svml_g9_erfcf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_invf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_divf8_native (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_sqrtf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_rsqrtf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_cbrtf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_rcbrtf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_hypotf8_native (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_powf8_native (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_powrf8_native (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_pownf8_native (float8_avx a, int8_avx b);

  float8_avx __ocl_svml_s9_rootnf8_native (float8_avx a, int8_avx b);

  float8_avx __ocl_svml_s9_expf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_exp2f8_native (float8_avx a);

  float8_avx __ocl_svml_s9_exp10f8_native (float8_avx a);

  float8_avx __ocl_svml_s9_expm1f8_native (float8_avx a);

  float8_avx __ocl_svml_s9_logf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_log10f8_native (float8_avx a);

  float8_avx __ocl_svml_s9_log2f8_native (float8_avx a);

  float8_avx __ocl_svml_s9_log1pf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_sinf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_cosf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_sincosf8_native (float8_avx a, float8_avx * c);

  float8x2_avx __ocl_svml_s9_sincosregf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_tanf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_sinpif8_native (float8_avx a);

  float8_avx __ocl_svml_s9_cospif8_native (float8_avx a);

  float8_avx __ocl_svml_s9_tanpif8_native (float8_avx a);

  float8_avx __ocl_svml_s9_acosf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_asinf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_atanf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_atan2f8_native (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_asinpif8_native (float8_avx a);

  float8_avx __ocl_svml_s9_acospif8_native (float8_avx a);

  float8_avx __ocl_svml_s9_atanpif8_native (float8_avx a);

  float8_avx __ocl_svml_s9_atan2pif8_native (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_sinhf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_coshf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_tanhf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_asinhf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_acoshf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_atanhf8_native (float8_avx a);

  float8_avx __ocl_svml_s9_erff8_native (float8_avx a);

  float8_avx __ocl_svml_s9_erfcf8_native (float8_avx a);

  double8_avx __ocl_svml_g9_inv8_native (double8_avx a);

  double8_avx __ocl_svml_g9_div8_native (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_sqrt8_native (double8_avx a);

  double8_avx __ocl_svml_g9_rsqrt8_native (double8_avx a);

  double8_avx __ocl_svml_g9_cbrt8_native (double8_avx a);

  double8_avx __ocl_svml_g9_rcbrt8_native (double8_avx a);

  double8_avx __ocl_svml_g9_hypot8_native (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_pow8_native (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_powr8_native (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_pown8_native (double8_avx a, int8_avx b);

  double8_avx __ocl_svml_g9_rootn8_native (double8_avx a, int8_avx b);

  double8_avx __ocl_svml_g9_exp8_native (double8_avx a);

  double8_avx __ocl_svml_g9_exp28_native (double8_avx a);

  double8_avx __ocl_svml_g9_exp108_native (double8_avx a);

  double8_avx __ocl_svml_g9_expm18_native (double8_avx a);

  double8_avx __ocl_svml_g9_log8_native (double8_avx a);

  double8_avx __ocl_svml_g9_log108_native (double8_avx a);

  double8_avx __ocl_svml_g9_log28_native (double8_avx a);

  double8_avx __ocl_svml_g9_log1p8_native (double8_avx a);

  double8_avx __ocl_svml_g9_sin8_native (double8_avx a);

  double8_avx __ocl_svml_g9_cos8_native (double8_avx a);

  double8_avx __ocl_svml_g9_sincos8_native (double8_avx a, double8_avx * c);

  double8x2_avx __ocl_svml_g9_sincosreg8_native (double8_avx a);

  double8_avx __ocl_svml_g9_tan8_native (double8_avx a);

  double8_avx __ocl_svml_g9_sinpi8_native (double8_avx a);

  double8_avx __ocl_svml_g9_cospi8_native (double8_avx a);

  double8_avx __ocl_svml_g9_tanpi8_native (double8_avx a);

  double8_avx __ocl_svml_g9_acos8_native (double8_avx a);

  double8_avx __ocl_svml_g9_asin8_native (double8_avx a);

  double8_avx __ocl_svml_g9_atan8_native (double8_avx a);

  double8_avx __ocl_svml_g9_atan28_native (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_asinpi8_native (double8_avx a);

  double8_avx __ocl_svml_g9_acospi8_native (double8_avx a);

  double8_avx __ocl_svml_g9_atanpi8_native (double8_avx a);

  double8_avx __ocl_svml_g9_atan2pi8_native (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_g9_sinh8_native (double8_avx a);

  double8_avx __ocl_svml_g9_cosh8_native (double8_avx a);

  double8_avx __ocl_svml_g9_tanh8_native (double8_avx a);

  double8_avx __ocl_svml_g9_asinh8_native (double8_avx a);

  double8_avx __ocl_svml_g9_acosh8_native (double8_avx a);

  double8_avx __ocl_svml_g9_atanh8_native (double8_avx a);

  double8_avx __ocl_svml_g9_erf8_native (double8_avx a);

  double8_avx __ocl_svml_g9_erfc8_native (double8_avx a);

  double8_avx __ocl_svml_s9_inv8_native (double8_avx a);

  double8_avx __ocl_svml_s9_div8_native (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_sqrt8_native (double8_avx a);

  double8_avx __ocl_svml_s9_rsqrt8_native (double8_avx a);

  double8_avx __ocl_svml_s9_cbrt8_native (double8_avx a);

  double8_avx __ocl_svml_s9_rcbrt8_native (double8_avx a);

  double8_avx __ocl_svml_s9_hypot8_native (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_pow8_native (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_powr8_native (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_pown8_native (double8_avx a, int8_avx b);

  double8_avx __ocl_svml_s9_rootn8_native (double8_avx a, int8_avx b);

  double8_avx __ocl_svml_s9_exp8_native (double8_avx a);

  double8_avx __ocl_svml_s9_exp28_native (double8_avx a);

  double8_avx __ocl_svml_s9_exp108_native (double8_avx a);

  double8_avx __ocl_svml_s9_expm18_native (double8_avx a);

  double8_avx __ocl_svml_s9_log8_native (double8_avx a);

  double8_avx __ocl_svml_s9_log108_native (double8_avx a);

  double8_avx __ocl_svml_s9_log28_native (double8_avx a);

  double8_avx __ocl_svml_s9_log1p8_native (double8_avx a);

  double8_avx __ocl_svml_s9_sin8_native (double8_avx a);

  double8_avx __ocl_svml_s9_cos8_native (double8_avx a);

  double8_avx __ocl_svml_s9_sincos8_native (double8_avx a, double8_avx * c);

  double8x2_avx __ocl_svml_s9_sincosreg8_native (double8_avx a);

  double8_avx __ocl_svml_s9_tan8_native (double8_avx a);

  double8_avx __ocl_svml_s9_sinpi8_native (double8_avx a);

  double8_avx __ocl_svml_s9_cospi8_native (double8_avx a);

  double8_avx __ocl_svml_s9_tanpi8_native (double8_avx a);

  double8_avx __ocl_svml_s9_acos8_native (double8_avx a);

  double8_avx __ocl_svml_s9_asin8_native (double8_avx a);

  double8_avx __ocl_svml_s9_atan8_native (double8_avx a);

  double8_avx __ocl_svml_s9_atan28_native (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_asinpi8_native (double8_avx a);

  double8_avx __ocl_svml_s9_acospi8_native (double8_avx a);

  double8_avx __ocl_svml_s9_atanpi8_native (double8_avx a);

  double8_avx __ocl_svml_s9_atan2pi8_native (double8_avx a, double8_avx b);

  double8_avx __ocl_svml_s9_sinh8_native (double8_avx a);

  double8_avx __ocl_svml_s9_cosh8_native (double8_avx a);

  double8_avx __ocl_svml_s9_tanh8_native (double8_avx a);

  double8_avx __ocl_svml_s9_asinh8_native (double8_avx a);

  double8_avx __ocl_svml_s9_acosh8_native (double8_avx a);

  double8_avx __ocl_svml_s9_atanh8_native (double8_avx a);

  double8_avx __ocl_svml_s9_erf8_native (double8_avx a);

  double8_avx __ocl_svml_s9_erfc8_native (double8_avx a);

  float16_avx __ocl_svml_g9_invf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_divf16_native (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_sqrtf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_rsqrtf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_cbrtf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_rcbrtf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_hypotf16_native (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_powf16_native (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_powrf16_native (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_pownf16_native (float16_avx a, int16_avx b);

  float16_avx __ocl_svml_g9_rootnf16_native (float16_avx a, int16_avx b);

  float16_avx __ocl_svml_g9_expf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_exp2f16_native (float16_avx a);

  float16_avx __ocl_svml_g9_exp10f16_native (float16_avx a);

  float16_avx __ocl_svml_g9_expm1f16_native (float16_avx a);

  float16_avx __ocl_svml_g9_logf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_log10f16_native (float16_avx a);

  float16_avx __ocl_svml_g9_log2f16_native (float16_avx a);

  float16_avx __ocl_svml_g9_log1pf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_sinf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_cosf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_sincosf16_native (float16_avx a, float16_avx * c);

  float16x2_avx __ocl_svml_g9_sincosregf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_tanf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_sinpif16_native (float16_avx a);

  float16_avx __ocl_svml_g9_cospif16_native (float16_avx a);

  float16_avx __ocl_svml_g9_tanpif16_native (float16_avx a);

  float16_avx __ocl_svml_g9_acosf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_asinf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_atanf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_atan2f16_native (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_asinpif16_native (float16_avx a);

  float16_avx __ocl_svml_g9_acospif16_native (float16_avx a);

  float16_avx __ocl_svml_g9_atanpif16_native (float16_avx a);

  float16_avx __ocl_svml_g9_atan2pif16_native (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_sinhf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_coshf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_tanhf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_asinhf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_acoshf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_atanhf16_native (float16_avx a);

  float16_avx __ocl_svml_g9_erff16_native (float16_avx a);

  float16_avx __ocl_svml_g9_erfcf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_invf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_divf16_native (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_sqrtf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_rsqrtf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_cbrtf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_rcbrtf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_hypotf16_native (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_powf16_native (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_powrf16_native (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_pownf16_native (float16_avx a, int16_avx b);

  float16_avx __ocl_svml_s9_rootnf16_native (float16_avx a, int16_avx b);

  float16_avx __ocl_svml_s9_expf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_exp2f16_native (float16_avx a);

  float16_avx __ocl_svml_s9_exp10f16_native (float16_avx a);

  float16_avx __ocl_svml_s9_expm1f16_native (float16_avx a);

  float16_avx __ocl_svml_s9_logf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_log10f16_native (float16_avx a);

  float16_avx __ocl_svml_s9_log2f16_native (float16_avx a);

  float16_avx __ocl_svml_s9_log1pf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_sinf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_cosf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_sincosf16_native (float16_avx a, float16_avx * c);

  float16x2_avx __ocl_svml_s9_sincosregf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_tanf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_sinpif16_native (float16_avx a);

  float16_avx __ocl_svml_s9_cospif16_native (float16_avx a);

  float16_avx __ocl_svml_s9_tanpif16_native (float16_avx a);

  float16_avx __ocl_svml_s9_acosf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_asinf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_atanf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_atan2f16_native (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_asinpif16_native (float16_avx a);

  float16_avx __ocl_svml_s9_acospif16_native (float16_avx a);

  float16_avx __ocl_svml_s9_atanpif16_native (float16_avx a);

  float16_avx __ocl_svml_s9_atan2pif16_native (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_sinhf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_coshf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_tanhf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_asinhf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_acoshf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_atanhf16_native (float16_avx a);

  float16_avx __ocl_svml_s9_erff16_native (float16_avx a);

  float16_avx __ocl_svml_s9_erfcf16_native (float16_avx a);

  float1_sse __ocl_svml_n8_cosf1_rm (float1_sse a);

  float1_sse __ocl_svml_n8_divf1_rm (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_expf1_rm (float1_sse a);

  float1_sse __ocl_svml_n8_exp2f1_rm (float1_sse a);

  float1_sse __ocl_svml_n8_exp10f1_rm (float1_sse a);

  float1_sse __ocl_svml_n8_logf1_rm (float1_sse a);

  float1_sse __ocl_svml_n8_log2f1_rm (float1_sse a);

  float1_sse __ocl_svml_n8_powf1_rm (float1_sse a, float1_sse b);

  float1_sse __ocl_svml_n8_invf1_rm (float1_sse a);

  float1_sse __ocl_svml_n8_sinf1_rm (float1_sse a);

  float1_sse __ocl_svml_n8_sincosf1_rm (float1_sse a, float1_sse* c);

  float1_sse __ocl_svml_n8_tanf1_rm (float1_sse a);

  float1_sse __ocl_svml_n8_fmaf1_rm (float1_sse a, float1_sse b, float1_sse c);

  float1_avx __ocl_svml_g9_cosf1_rm (float1_avx a);

  float1_avx __ocl_svml_g9_divf1_rm (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_expf1_rm (float1_avx a);

  float1_avx __ocl_svml_g9_exp2f1_rm (float1_avx a);

  float1_avx __ocl_svml_g9_exp10f1_rm (float1_avx a);

  float1_avx __ocl_svml_g9_logf1_rm (float1_avx a);

  float1_avx __ocl_svml_g9_log2f1_rm (float1_avx a);

  float1_avx __ocl_svml_g9_powf1_rm (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_g9_invf1_rm (float1_avx a);

  float1_avx __ocl_svml_g9_sinf1_rm (float1_avx a);

  float1_avx __ocl_svml_g9_sincosf1_rm (float1_avx a, float1_avx* c);

  float1_avx __ocl_svml_g9_tanf1_rm (float1_avx a);

  float1_avx __ocl_svml_g9_fmaf1_rm (float1_avx a, float1_avx b, float1_avx c);

  float1_avx __ocl_svml_s9_cosf1_rm (float1_avx a);

  float1_avx __ocl_svml_s9_divf1_rm (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_expf1_rm (float1_avx a);

  float1_avx __ocl_svml_s9_exp2f1_rm (float1_avx a);

  float1_avx __ocl_svml_s9_exp10f1_rm (float1_avx a);

  float1_avx __ocl_svml_s9_logf1_rm (float1_avx a);

  float1_avx __ocl_svml_s9_log2f1_rm (float1_avx a);

  float1_avx __ocl_svml_s9_powf1_rm (float1_avx a, float1_avx b);

  float1_avx __ocl_svml_s9_invf1_rm (float1_avx a);

  float1_avx __ocl_svml_s9_sinf1_rm (float1_avx a);

  float1_avx __ocl_svml_s9_sincosf1_rm (float1_avx a, float1_avx* c);

  float1_avx __ocl_svml_s9_tanf1_rm (float1_avx a);

  float1_avx __ocl_svml_s9_fmaf1_rm (float1_avx a, float1_avx b, float1_avx c);

  float2_sse __ocl_svml_n8_cosf2_rm (float2_sse a);

  float2_sse __ocl_svml_n8_divf2_rm (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_expf2_rm (float2_sse a);

  float2_sse __ocl_svml_n8_exp2f2_rm (float2_sse a);

  float2_sse __ocl_svml_n8_exp10f2_rm (float2_sse a);

  float2_sse __ocl_svml_n8_logf2_rm (float2_sse a);

  float2_sse __ocl_svml_n8_log2f2_rm (float2_sse a);

  float2_sse __ocl_svml_n8_powf2_rm (float2_sse a, float2_sse b);

  float2_sse __ocl_svml_n8_invf2_rm (float2_sse a);

  float2_sse __ocl_svml_n8_sinf2_rm (float2_sse a);

  float2_sse __ocl_svml_n8_sincosf2_rm (float2_sse a, float2_sse* c);

  float2_sse __ocl_svml_n8_tanf2_rm (float2_sse a);

  float2_sse __ocl_svml_n8_fmaf2_rm (float2_sse a, float2_sse b, float2_sse c);

  float2_avx __ocl_svml_g9_cosf2_rm (float2_avx a);

  float2_avx __ocl_svml_g9_divf2_rm (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_expf2_rm (float2_avx a);

  float2_avx __ocl_svml_g9_exp2f2_rm (float2_avx a);

  float2_avx __ocl_svml_g9_exp10f2_rm (float2_avx a);

  float2_avx __ocl_svml_g9_logf2_rm (float2_avx a);

  float2_avx __ocl_svml_g9_log2f2_rm (float2_avx a);

  float2_avx __ocl_svml_g9_powf2_rm (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_g9_invf2_rm (float2_avx a);

  float2_avx __ocl_svml_g9_sinf2_rm (float2_avx a);

  float2_avx __ocl_svml_g9_sincosf2_rm (float2_avx a, float2_avx* c);

  float2_avx __ocl_svml_g9_tanf2_rm (float2_avx a);

  float2_avx __ocl_svml_g9_fmaf2_rm (float2_avx a, float2_avx b, float2_avx c);

  float2_avx __ocl_svml_s9_cosf2_rm (float2_avx a);

  float2_avx __ocl_svml_s9_divf2_rm (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_expf2_rm (float2_avx a);

  float2_avx __ocl_svml_s9_exp2f2_rm (float2_avx a);

  float2_avx __ocl_svml_s9_exp10f2_rm (float2_avx a);

  float2_avx __ocl_svml_s9_logf2_rm (float2_avx a);

  float2_avx __ocl_svml_s9_log2f2_rm (float2_avx a);

  float2_avx __ocl_svml_s9_powf2_rm (float2_avx a, float2_avx b);

  float2_avx __ocl_svml_s9_invf2_rm (float2_avx a);

  float2_avx __ocl_svml_s9_sinf2_rm (float2_avx a);

  float2_avx __ocl_svml_s9_sincosf2_rm (float2_avx a, float2_avx* c);

  float2_avx __ocl_svml_s9_tanf2_rm (float2_avx a);

  float2_avx __ocl_svml_s9_fmaf2_rm (float2_avx a, float2_avx b, float2_avx c);

  float3_sse __ocl_svml_n8_cosf3_rm (float3_sse a);

  float3_sse __ocl_svml_n8_divf3_rm (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_expf3_rm (float3_sse a);

  float3_sse __ocl_svml_n8_exp2f3_rm (float3_sse a);

  float3_sse __ocl_svml_n8_exp10f3_rm (float3_sse a);

  float3_sse __ocl_svml_n8_logf3_rm (float3_sse a);

  float3_sse __ocl_svml_n8_log2f3_rm (float3_sse a);

  float3_sse __ocl_svml_n8_powf3_rm (float3_sse a, float3_sse b);

  float3_sse __ocl_svml_n8_invf3_rm (float3_sse a);

  float3_sse __ocl_svml_n8_sinf3_rm (float3_sse a);

  float3_sse __ocl_svml_n8_sincosf3_rm (float3_sse a, float3_sse* c);

  float3_sse __ocl_svml_n8_tanf3_rm (float3_sse a);

  float3_sse __ocl_svml_n8_fmaf3_rm (float3_sse a, float3_sse b, float3_sse c);

  float3_avx __ocl_svml_g9_cosf3_rm (float3_avx a);

  float3_avx __ocl_svml_g9_divf3_rm (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_expf3_rm (float3_avx a);

  float3_avx __ocl_svml_g9_exp2f3_rm (float3_avx a);

  float3_avx __ocl_svml_g9_exp10f3_rm (float3_avx a);

  float3_avx __ocl_svml_g9_logf3_rm (float3_avx a);

  float3_avx __ocl_svml_g9_log2f3_rm (float3_avx a);

  float3_avx __ocl_svml_g9_powf3_rm (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_g9_invf3_rm (float3_avx a);

  float3_avx __ocl_svml_g9_sinf3_rm (float3_avx a);

  float3_avx __ocl_svml_g9_sincosf3_rm (float3_avx a, float3_avx* c);

  float3_avx __ocl_svml_g9_tanf3_rm (float3_avx a);

  float3_avx __ocl_svml_g9_fmaf3_rm (float3_avx a, float3_avx b, float3_avx c);

  float3_avx __ocl_svml_s9_cosf3_rm (float3_avx a);

  float3_avx __ocl_svml_s9_divf3_rm (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_expf3_rm (float3_avx a);

  float3_avx __ocl_svml_s9_exp2f3_rm (float3_avx a);

  float3_avx __ocl_svml_s9_exp10f3_rm (float3_avx a);

  float3_avx __ocl_svml_s9_logf3_rm (float3_avx a);

  float3_avx __ocl_svml_s9_log2f3_rm (float3_avx a);

  float3_avx __ocl_svml_s9_powf3_rm (float3_avx a, float3_avx b);

  float3_avx __ocl_svml_s9_invf3_rm (float3_avx a);

  float3_avx __ocl_svml_s9_sinf3_rm (float3_avx a);

  float3_avx __ocl_svml_s9_sincosf3_rm (float3_avx a, float3_avx* c);

  float3_avx __ocl_svml_s9_tanf3_rm (float3_avx a);

  float3_avx __ocl_svml_s9_fmaf3_rm (float3_avx a, float3_avx b, float3_avx c);

  float4_sse __ocl_svml_n8_cosf4_rm (float4_sse a);

  float4_sse __ocl_svml_n8_divf4_rm (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_expf4_rm (float4_sse a);

  float4_sse __ocl_svml_n8_exp2f4_rm (float4_sse a);

  float4_sse __ocl_svml_n8_exp10f4_rm (float4_sse a);

  float4_sse __ocl_svml_n8_logf4_rm (float4_sse a);

  float4_sse __ocl_svml_n8_log2f4_rm (float4_sse a);

  float4_sse __ocl_svml_n8_powf4_rm (float4_sse a, float4_sse b);

  float4_sse __ocl_svml_n8_invf4_rm (float4_sse a);

  float4_sse __ocl_svml_n8_sinf4_rm (float4_sse a);

  float4_sse __ocl_svml_n8_sincosf4_rm (float4_sse a, float4_sse* c);

  float4_sse __ocl_svml_n8_tanf4_rm (float4_sse a);

  float4_sse __ocl_svml_n8_fmaf4_rm (float4_sse a, float4_sse b, float4_sse c);

  float4_avx __ocl_svml_g9_cosf4_rm (float4_avx a);

  float4_avx __ocl_svml_g9_divf4_rm (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_expf4_rm (float4_avx a);

  float4_avx __ocl_svml_g9_exp2f4_rm (float4_avx a);

  float4_avx __ocl_svml_g9_exp10f4_rm (float4_avx a);

  float4_avx __ocl_svml_g9_logf4_rm (float4_avx a);

  float4_avx __ocl_svml_g9_log2f4_rm (float4_avx a);

  float4_avx __ocl_svml_g9_powf4_rm (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_g9_invf4_rm (float4_avx a);

  float4_avx __ocl_svml_g9_sinf4_rm (float4_avx a);

  float4_avx __ocl_svml_g9_sincosf4_rm (float4_avx a, float4_avx* c);

  float4_avx __ocl_svml_g9_tanf4_rm (float4_avx a);

  float4_avx __ocl_svml_g9_fmaf4_rm (float4_avx a, float4_avx b, float4_avx c);

  float4_avx __ocl_svml_s9_cosf4_rm (float4_avx a);

  float4_avx __ocl_svml_s9_divf4_rm (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_expf4_rm (float4_avx a);

  float4_avx __ocl_svml_s9_exp2f4_rm (float4_avx a);

  float4_avx __ocl_svml_s9_exp10f4_rm (float4_avx a);

  float4_avx __ocl_svml_s9_logf4_rm (float4_avx a);

  float4_avx __ocl_svml_s9_log2f4_rm (float4_avx a);

  float4_avx __ocl_svml_s9_powf4_rm (float4_avx a, float4_avx b);

  float4_avx __ocl_svml_s9_invf4_rm (float4_avx a);

  float4_avx __ocl_svml_s9_sinf4_rm (float4_avx a);

  float4_avx __ocl_svml_s9_sincosf4_rm (float4_avx a, float4_avx* c);

  float4_avx __ocl_svml_s9_tanf4_rm (float4_avx a);

  float4_avx __ocl_svml_s9_fmaf4_rm (float4_avx a, float4_avx b, float4_avx c);

  float8_sse __ocl_svml_n8_cosf8_rm (float8_sse a);

  float8_sse __ocl_svml_n8_divf8_rm (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_expf8_rm (float8_sse a);

  float8_sse __ocl_svml_n8_exp2f8_rm (float8_sse a);

  float8_sse __ocl_svml_n8_exp10f8_rm (float8_sse a);

  float8_sse __ocl_svml_n8_logf8_rm (float8_sse a);

  float8_sse __ocl_svml_n8_log2f8_rm (float8_sse a);

  float8_sse __ocl_svml_n8_powf8_rm (float8_sse a, float8_sse b);

  float8_sse __ocl_svml_n8_invf8_rm (float8_sse a);

  float8_sse __ocl_svml_n8_sinf8_rm (float8_sse a);

  float8_sse __ocl_svml_n8_sincosf8_rm (float8_sse a, float8_sse* c);

  float8_sse __ocl_svml_n8_tanf8_rm (float8_sse a);

  float8_sse __ocl_svml_n8_fmaf8_rm (float8_sse a, float8_sse b, float8_sse c); 

  float8_avx __ocl_svml_g9_cosf8_rm (float8_avx a);

  float8_avx __ocl_svml_g9_divf8_rm (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_expf8_rm (float8_avx a);

  float8_avx __ocl_svml_g9_exp2f8_rm (float8_avx a);

  float8_avx __ocl_svml_g9_exp10f8_rm (float8_avx a);

  float8_avx __ocl_svml_g9_logf8_rm (float8_avx a);

  float8_avx __ocl_svml_g9_log2f8_rm (float8_avx a);

  float8_avx __ocl_svml_g9_powf8_rm (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_g9_invf8_rm (float8_avx a);

  float8_avx __ocl_svml_g9_sinf8_rm (float8_avx a);

  float8_avx __ocl_svml_g9_sincosf8_rm (float8_avx a, float8_avx* c);

  float8_avx __ocl_svml_g9_tanf8_rm (float8_avx a);

  float8_avx __ocl_svml_g9_fmaf8_rm (float8_avx a, float8_avx b, float8_avx c);

  float8_avx __ocl_svml_s9_cosf8_rm (float8_avx a);

  float8_avx __ocl_svml_s9_divf8_rm (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_expf8_rm (float8_avx a);

  float8_avx __ocl_svml_s9_exp2f8_rm (float8_avx a);

  float8_avx __ocl_svml_s9_exp10f8_rm (float8_avx a);

  float8_avx __ocl_svml_s9_logf8_rm (float8_avx a);

  float8_avx __ocl_svml_s9_log2f8_rm (float8_avx a);

  float8_avx __ocl_svml_s9_powf8_rm (float8_avx a, float8_avx b);

  float8_avx __ocl_svml_s9_invf8_rm (float8_avx a);

  float8_avx __ocl_svml_s9_sinf8_rm (float8_avx a);

  float8_avx __ocl_svml_s9_sincosf8_rm (float8_avx a, float8_avx* c);

  float8_avx __ocl_svml_s9_tanf8_rm (float8_avx a);

  float8_avx __ocl_svml_s9_fmaf8_rm (float8_avx a, float8_avx b, float8_avx c);

  float16_avx __ocl_svml_g9_cosf16_rm (float16_avx a);

  float16_avx __ocl_svml_g9_divf16_rm (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_expf16_rm (float16_avx a);

  float16_avx __ocl_svml_g9_exp2f16_rm (float16_avx a);

  float16_avx __ocl_svml_g9_exp10f16_rm (float16_avx a);

  float16_avx __ocl_svml_g9_logf16_rm (float16_avx a);

  float16_avx __ocl_svml_g9_log2f16_rm (float16_avx a);

  float16_avx __ocl_svml_g9_powf16_rm (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_g9_invf16_rm (float16_avx a);

  float16_avx __ocl_svml_g9_sinf16_rm (float16_avx a);

  float16_avx __ocl_svml_g9_sincosf16_rm (float16_avx a, float16_avx* c);

  float16_avx __ocl_svml_g9_tanf16_rm (float16_avx a);

  float16_avx __ocl_svml_g9_fmaf16_rm (float16_avx a, float16_avx b, float16_avx c);

  float16_avx __ocl_svml_s9_cosf16_rm (float16_avx a);

  float16_avx __ocl_svml_s9_divf16_rm (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_expf16_rm (float16_avx a);

  float16_avx __ocl_svml_s9_exp2f16_rm (float16_avx a);

  float16_avx __ocl_svml_s9_exp10f16_rm (float16_avx a);

  float16_avx __ocl_svml_s9_logf16_rm (float16_avx a);

  float16_avx __ocl_svml_s9_log2f16_rm (float16_avx a);

  float16_avx __ocl_svml_s9_powf16_rm (float16_avx a, float16_avx b);

  float16_avx __ocl_svml_s9_invf16_rm (float16_avx a);

  float16_avx __ocl_svml_s9_sinf16_rm (float16_avx a);

  float16_avx __ocl_svml_s9_sincosf16_rm (float16_avx a, float16_avx* c);

  float16_avx __ocl_svml_s9_tanf16_rm (float16_avx a);

  float16_avx __ocl_svml_s9_fmaf16_rm (float16_avx a, float16_avx b, float16_avx c);

#ifdef __cplusplus
}
#endif

#endif /* __OCL_SVML_H__ */
