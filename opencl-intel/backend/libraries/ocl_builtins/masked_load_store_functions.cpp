/*=================================================================================
 Copyright (c) 2012, Intel Corporation
 Subject to the terms and conditions of the Master Development License
 Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
 OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
 ==================================================================================*/

///////////////////////////////////////////////////////////
//  masked_load_store_functions.cpp
///////////////////////////////////////////////////////////

#include "masked_load_store_functions.h"

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#define __OPENCL__
#ifdef __APPLE__
  #include <immintrin.h>
#else
  #include <intrin.h>
#endif

// ****************************************************************************
//                                 char4x4
// ****************************************************************************

#if defined(__AVX2__)

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_load_char4x4_AVX2(char4* pLoadAdd, char4* pLoadedValues, int4 mask) {
  char16* tmpLoadedValues = (char16*)pLoadedValues;
  *tmpLoadedValues = as_char16(_mm_maskload_epi32((int*)pLoadAdd, (__m128i)mask));
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_store_char4x4_AVX2(char4* pStoreAdd, char4* pValuesToStore, int4 mask) {
  _mm_maskstore_epi32((int*)pStoreAdd, (__m128i) mask,*((__m128i*) pValuesToStore));
}

#endif // defined(__AVX2__)

#if defined(__AVX__)

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_load_char4x4_AVX(char4* pLoadAdd, char4* pLoadedValues, int4 mask) {
  char16* tmpLoadedValues = (char16*)pLoadedValues;
  *tmpLoadedValues = as_char16(_mm_maskload_ps((float*)pLoadAdd, (__m128)mask));
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_store_char4x4_AVX(char4* pStoreAdd, char4* pValuesToStore, int4 mask) {
  _mm_maskstore_ps((float*)pStoreAdd, (__m128) mask,*((__m128*) pValuesToStore));
}

#endif // defined(__AVX__)


#if defined(__AVX__)

void INLINE_ATTRIBUTE __ocl_masked_load_char4x4(char4* pLoadAdd, char4* pLoadedValues, int4 mask) {
#if defined(__AVX2__)
  __ocl_masked_load_char4x4_AVX2(pLoadAdd, pLoadedValues, mask);
#else // defined(__AVX__)
  __ocl_masked_load_char4x4_AVX(pLoadAdd, pLoadedValues, mask);
#endif
}

void INLINE_ATTRIBUTE __ocl_masked_store_char4x4(char4* pStoreAdd, char4* pValuesToStore, int4 mask) {
#if defined(__AVX2__)
  __ocl_masked_store_char4x4_AVX2(pStoreAdd, pValuesToStore, mask);
#else // defined(__AVX__)
  __ocl_masked_store_char4x4_AVX(pStoreAdd, pValuesToStore, mask);
#endif
}

#endif // defined(__AVX__)

// ****************************************************************************
//                                 char4x8
// ****************************************************************************

#if defined(__AVX2__)

typedef __v32qi my_char32;

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_load_char4x8_AVX2(char4* pLoadAdd, char4* pLoadedValues, int8 mask) {
   my_char32* tmpLoadedValues = (my_char32*)pLoadedValues;
   *tmpLoadedValues = (my_char32)_mm256_maskload_epi32((int*)pLoadAdd, (__m256i)mask);
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_store_char4x8_AVX2(char4* pStoreAdd, char4* pValuesToStore, int8 mask) {
  _mm256_maskstore_epi32((int*)pStoreAdd, (__m256i) mask,*((__m256i*) pValuesToStore));
}

#endif // defined(__AVX2__)


#if defined(__AVX__)

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_load_char4x8_AVX(char4* pLoadAdd, char4* pLoadedValues, int8 mask) {
  float8 res = as_float8(_mm256_maskload_ps((float*)pLoadAdd, (__m256)mask));
  char16* tmpLoadedValues = (char16*)pLoadedValues;
  tmpLoadedValues[0] = as_char16(res.lo);
  tmpLoadedValues[1] = as_char16(res.hi);
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_store_char4x8_AVX(char4* pStoreAdd, char4* pValuesToStore, int8 mask) {
  _mm256_maskstore_ps((float*)pStoreAdd, (__m256) mask,*((__m256*) pValuesToStore));
}

#endif // defined(__AVX__)


#if defined(__AVX__)

void INLINE_ATTRIBUTE __ocl_masked_load_char4x8(char4* pLoadAdd, char4* pLoadedValues, int8 mask) {
#if defined(__AVX2__)
  __ocl_masked_load_char4x8_AVX2(pLoadAdd, pLoadedValues, mask);
#else // defined(__AVX__)
  __ocl_masked_load_char4x8_AVX(pLoadAdd, pLoadedValues, mask);
#endif
}

void INLINE_ATTRIBUTE __ocl_masked_store_char4x8(char4* pStoreAdd, char4* pValuesToStore, int8 mask) {
#if defined(__AVX2__)
  __ocl_masked_store_char4x8_AVX2(pStoreAdd, pValuesToStore, mask);
#else // defined(__AVX__)
  __ocl_masked_store_char4x8_AVX(pStoreAdd, pValuesToStore, mask);
#endif
}

#endif // defined(__AVX__)

// ****************************************************************************
//                                 short4x4
// ****************************************************************************

#if defined(__AVX2__)

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_load_short4x4_AVX2(short4* pLoadAdd, short4* pLoadedValues, int4 mask) {
  int8 extendedMask = mask.s00112233;

  short16* tmpLoadAdd = (short16*)pLoadAdd;
  short16* tmpLoadedValues = (short16*)pLoadedValues;
  *tmpLoadedValues = as_short16( _mm256_maskload_epi32((int*)(tmpLoadAdd), (__m256i)extendedMask) );
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_store_short4x4_AVX2(short4* pStoreAdd, short4* pValuesToStore, int4 mask) {
  int8 extendedMask = mask.s00112233;

  short16* tmpStoreAdd = (short16*)pStoreAdd;
  short16* tmpValuesToStore = (short16*)pValuesToStore;
  _mm256_maskstore_epi32((int*)(tmpStoreAdd), (__m256i)extendedMask, *((__m256i*)tmpValuesToStore));
}

#endif // defined(__AVX2__)


#if defined(__AVX__)

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_load_short4x4_AVX(short4* pLoadAdd, short4* pLoadedValues, int4 mask) {
  int8 extendedMask = mask.s00112233;

  short16* tmpLoadAdd = (short16*)pLoadAdd;
  short16* tmpLoadedValues = (short16*)pLoadedValues;
  *tmpLoadedValues = as_short16( _mm256_maskload_ps((float*)(tmpLoadAdd), (__m256)extendedMask) );
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_store_short4x4_AVX(short4* pStoreAdd, short4* pValuesToStore, int4 mask) {
  int8 extendedMask = mask.s00112233;

  short16* tmpStoreAdd = (short16*)pStoreAdd;
  short16* tmpValuesToStore = (short16*)pValuesToStore;
  _mm256_maskstore_ps((float*)(tmpStoreAdd), (__m256)extendedMask, *((__m256*)tmpValuesToStore));
}

#endif // defined(__AVX__)


#if defined(__AVX__)

void INLINE_ATTRIBUTE __ocl_masked_load_short4x4(short4* pLoadAdd, short4* pLoadedValues, int4 mask) {
#if defined(__AVX2__)
  __ocl_masked_load_short4x4_AVX2(pLoadAdd, pLoadedValues, mask);
#else // defined(__AVX__)
  __ocl_masked_load_short4x4_AVX(pLoadAdd, pLoadedValues, mask);
#endif
}

void INLINE_ATTRIBUTE __ocl_masked_store_short4x4(short4* pStoreAdd, short4* pValuesToStore, int4 mask) {
#if defined(__AVX2__)
  __ocl_masked_store_short4x4_AVX2(pStoreAdd, pValuesToStore, mask);
#else // defined(__AVX__)
  __ocl_masked_store_short4x4_AVX(pStoreAdd, pValuesToStore, mask);
#endif
}

#endif // defined(__AVX__)

// ****************************************************************************
//                                 short4x8
// ****************************************************************************

#if defined(__AVX2__)

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_load_short4x8_AVX2(short4* pLoadAdd, short4* pLoadedValues, int8 mask) {
  int8 maskLow  = mask.s00112233;
  int8 maskHigh = mask.s44556677;

  short16* tmpLoadAdd = (short16*)pLoadAdd;
  short16* tmpLoadedValues = (short16*)pLoadedValues;
  tmpLoadedValues[0] = as_short16( _mm256_maskload_epi32((int*)&(tmpLoadAdd[0]), (__m256i)maskLow) );
  tmpLoadedValues[1] = as_short16( _mm256_maskload_epi32((int*)&(tmpLoadAdd[1]), (__m256i)maskHigh) );
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_store_short4x8_AVX2(short4* pStoreAdd, short4* pValuesToStore, int8 mask) {
  int8 maskLow  = mask.s00112233;
  int8 maskHigh = mask.s44556677;

  short16* tmpStoreAdd = (short16*)pStoreAdd;
  short16* tmpValuesToStore = (short16*)pValuesToStore;
  _mm256_maskstore_epi32((int*)&(tmpStoreAdd[0]), (__m256i)maskLow,  (__m256i)tmpValuesToStore[0]);
  _mm256_maskstore_epi32((int*)&(tmpStoreAdd[1]), (__m256i)maskHigh, (__m256i)tmpValuesToStore[1]);
}

#endif //defined(__AVX2__)


#if defined(__AVX__)

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_load_short4x8_AVX(short4* pLoadAdd, short4* pLoadedValues, int8 mask) {
  int8 maskLow  = mask.s00112233;
  int8 maskHigh = mask.s44556677;

  short16* tmpLoadAdd = (short16*)pLoadAdd;
  short16* tmpLoadedValues = (short16*)pLoadedValues;
  tmpLoadedValues[0] = as_short16( _mm256_maskload_ps((float*)&(tmpLoadAdd[0]), (__m256)maskLow) );
  tmpLoadedValues[1] = as_short16( _mm256_maskload_ps((float*)&(tmpLoadAdd[1]), (__m256)maskHigh) );
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_store_short4x8_AVX(short4* pStoreAdd, short4* pValuesToStore, int8 mask) {
  int8 maskLow  = mask.s00112233;
  int8 maskHigh = mask.s44556677;

  short16* tmpStoreAdd = (short16*)pStoreAdd;
  short16* tmpValuesToStore = (short16*)pValuesToStore;
  _mm256_maskstore_ps((float*)&(tmpStoreAdd[0]), (__m256)maskLow,  (__m256)tmpValuesToStore[0]);
  _mm256_maskstore_ps((float*)&(tmpStoreAdd[1]), (__m256)maskHigh, (__m256)tmpValuesToStore[1]);
}

#endif //defined(__AVX__)


#if defined(__AVX__)

void INLINE_ATTRIBUTE __ocl_masked_load_short4x8(short4* pLoadAdd, short4* pLoadedValues, int8 mask) {
#if defined(__AVX2__)
  __ocl_masked_load_short4x8_AVX2(pLoadAdd, pLoadedValues, mask);
#else // defined(__AVX__)
  __ocl_masked_load_short4x8_AVX(pLoadAdd, pLoadedValues, mask);
#endif
}

void INLINE_ATTRIBUTE __ocl_masked_store_short4x8(short4* pStoreAdd, short4* pValuesToStore, int8 mask) {
#if defined(__AVX2__)
  __ocl_masked_store_short4x8_AVX2(pStoreAdd, pValuesToStore, mask);
#else // defined(__AVX__)
  __ocl_masked_store_short4x8_AVX(pStoreAdd, pValuesToStore, mask);
#endif
}

#endif //defined(__AVX__)

// ****************************************************************************
//                                 int4x4
// ****************************************************************************

#if defined(__AVX2__)

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_load_int4x4_AVX2(int4* pLoadAdd, int4* pLoadedValues, int4 mask) {

  int8 extendedMask = mask.s01230123;
  int8 maskLow = extendedMask.s00001111;
  int8 maskHigh = extendedMask.s22223333;

  int8* tmpLoadAdd = (int8*)pLoadAdd;
  int8* tmpLoadedValues = (int8*)pLoadedValues;
  tmpLoadedValues[0] = as_int8(_mm256_maskload_epi32((int*)&(tmpLoadAdd[0]), (__m256i)maskLow));
  tmpLoadedValues[1] = as_int8(_mm256_maskload_epi32((int*)&(tmpLoadAdd[1]), (__m256i)maskHigh));
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_store_int4x4_AVX2(int4* pStoreAdd, int4* pValuesToStore, int4 mask) {

  int8 extendedMask = mask.s01230123;
  int8 maskLow = extendedMask.s00001111;
  int8 maskHigh = extendedMask.s22223333;

  int8* tmpStoreAdd = (int8*)pStoreAdd;
  int8* tmpValuesToStore = (int8*)pValuesToStore;
  _mm256_maskstore_epi32((int*)&(tmpStoreAdd[0]), (__m256i)maskLow, (__m256i)tmpValuesToStore[0]);
  _mm256_maskstore_epi32((int*)&(tmpStoreAdd[1]), (__m256i)maskHigh, (__m256i)tmpValuesToStore[1]);
}

#endif // defined(__AVX2__)


#if defined(__AVX__)

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_load_float4x4_common(float4* pLoadAdd, float4* pLoadedValues, int4 mask) {

  float8 extendedMask = as_float8(mask.s01230123);
  float8 maskLow = extendedMask.s00001111;
  float8 maskHigh = extendedMask.s22223333;

  float8* tmpLoadAdd = (float8*)pLoadAdd;
  float8* tmpLoadedValues = (float8*)pLoadedValues;
  tmpLoadedValues[0] = as_float8(_mm256_maskload_ps((float*)&(tmpLoadAdd[0]), (__m256)maskLow));
  tmpLoadedValues[1] = as_float8(_mm256_maskload_ps((float*)&(tmpLoadAdd[1]), (__m256)maskHigh));
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_load_int4x4_AVX(int4* pLoadAdd, int4* pLoadedValues, int4 mask) {
  __ocl_masked_load_float4x4_common((float4*)pLoadAdd, (float4*)pLoadedValues, mask);
}

void INTERNAL_INLINE_ATTRIBUTE __ocl_masked_store_int4x4_AVX(int4* pStoreAdd, int4* pValuesToStore, int4 mask) {
  __ocl_masked_store_float4x4((float4*)pStoreAdd, (float4*)pValuesToStore, mask);
}

#endif // defined(__AVX__)


#if defined(__AVX__)

void INLINE_ATTRIBUTE __ocl_masked_load_int4x4(int4* pLoadAdd, int4* pLoadedValues, int4 mask) {
#if defined(__AVX2__)
  __ocl_masked_load_int4x4_AVX2(pLoadAdd, pLoadedValues, mask);
#else // defined(__AVX__)
  __ocl_masked_load_int4x4_AVX(pLoadAdd, pLoadedValues, mask);
#endif
}

void INLINE_ATTRIBUTE __ocl_masked_store_int4x4(int4* pStoreAdd, int4* pValuesToStore, int4 mask) {
#if defined(__AVX2__)
  __ocl_masked_store_int4x4_AVX2(pStoreAdd, pValuesToStore, mask);
#else // defined(__AVX__)
  __ocl_masked_store_int4x4_AVX(pStoreAdd, pValuesToStore, mask);
#endif
}

#endif // defined(__AVX__)

// ****************************************************************************
//                                 int4x8
// ****************************************************************************

#if defined(__AVX__)

void INLINE_ATTRIBUTE __ocl_masked_load_int4x8(int4* pLoadAdd, int4* pLoadedValues, int8 mask) {
  __ocl_masked_load_int4x4(&(pLoadAdd[0]), &(pLoadedValues[0]), mask.lo);
  __ocl_masked_load_int4x4(&(pLoadAdd[4]), &(pLoadedValues[4]), mask.hi);
}

void INLINE_ATTRIBUTE __ocl_masked_store_int4x8(int4* pStoreAdd, int4* pValuesToStore, int8 mask) {
  __ocl_masked_store_int4x4(&(pStoreAdd[0]), &(pValuesToStore[0]), mask.lo);
  __ocl_masked_store_int4x4(&(pStoreAdd[4]), &(pValuesToStore[4]), mask.hi);
}

#endif // defined(__AVX__)


// ****************************************************************************
//                                 float4x4
// ****************************************************************************

#if defined(__AVX__)

void INLINE_ATTRIBUTE __ocl_masked_load_float4x4(float4* pLoadAdd, float4* pLoadedValues, int4 mask) {
  *((float16*)pLoadedValues) = 0;
  __ocl_masked_load_float4x4_common((float4*)pLoadAdd, (float4*)pLoadedValues, mask);
}

void INLINE_ATTRIBUTE __ocl_masked_store_float4x4(float4* pStoreAdd, float4* pValuesToStore, int4 mask) {

  float8 extendedMask = as_float8(mask.s01230123);
  float8 maskLow = extendedMask.s00001111;
  float8 maskHigh = extendedMask.s22223333;

  float8* tmpStoreAdd = (float8*)pStoreAdd;
  float8* tmpValuesToStore = (float8*)pValuesToStore;
  _mm256_maskstore_ps((float*)&(tmpStoreAdd[0]), (__m256)maskLow, (__m256)tmpValuesToStore[0]);
  _mm256_maskstore_ps((float*)&(tmpStoreAdd[1]), (__m256)maskHigh, (__m256)tmpValuesToStore[1]);
}

#endif // defined(__AVX__)

// ****************************************************************************
//                                 float4x8
// ****************************************************************************

#if defined(__AVX__)

void INLINE_ATTRIBUTE __ocl_masked_load_float4x8(float4* pLoadAdd, float4* pLoadedValues, int8 mask) {
  __ocl_masked_load_float4x4(&(pLoadAdd[0]), &(pLoadedValues[0]), mask.lo);
  __ocl_masked_load_float4x4(&(pLoadAdd[4]), &(pLoadedValues[4]), mask.hi);
}

void INLINE_ATTRIBUTE __ocl_masked_store_float4x8(float4* pStoreAdd, float4* pValuesToStore, int8 mask)  {
  __ocl_masked_store_float4x4(&(pStoreAdd[0]), &(pValuesToStore[0]), mask.lo);
  __ocl_masked_store_float4x4(&(pStoreAdd[4]), &(pValuesToStore[4]), mask.hi);
}

#endif // defined(__AVX__)

#if defined(__AVX__)

// ****************************************************************************
//                                 int4
// ****************************************************************************

int4 INLINE_ATTRIBUTE __ocl_masked_load_int4(int4* pLoadAdd, int4 mask) {
#if defined(__AVX2__)
  return as_int4(_mm_maskload_epi32((int const *)pLoadAdd, (__m128i) mask));
#else
  return as_int4(_mm_maskload_ps ((float const *)pLoadAdd, (__m128) mask));
#endif
}

void INLINE_ATTRIBUTE __ocl_masked_store_int4(int4* pStoreAdd, int4 data, int4 mask) {
#if defined(__AVX2__)
  _mm_maskstore_epi32((int *)pStoreAdd, (__m128i) mask, *((__m128i*) &data));
#else
  _mm_maskstore_ps((float *)pStoreAdd, (__m128) mask, *((__m128*) &data));
#endif
}

// ****************************************************************************
//                                 int8
// ****************************************************************************

int8 INLINE_ATTRIBUTE __ocl_masked_load_int8(int8* pLoadAdd, int8 mask) {
#if defined(__AVX2__)
  return as_int8(_mm256_maskload_epi32((int const *)pLoadAdd, (__m256i) mask));
#else
  return as_int8(_mm256_maskload_ps((float const *)pLoadAdd, (__m256) mask));
#endif
}

void INLINE_ATTRIBUTE __ocl_masked_store_int8(int8* pStoreAdd, int8 data, int8 mask) {
#if defined(__AVX2__)
  _mm256_maskstore_epi32((int *)pStoreAdd, (__m256i) mask, *((__m256i*) &data));
#else
  _mm256_maskstore_ps((float *)pStoreAdd, (__m256) mask, *((__m256*) &data));
#endif
}

// ****************************************************************************
//                                 long4
// ****************************************************************************

long4 INLINE_ATTRIBUTE __ocl_masked_load_long4(long4* pLoadAdd, int4 mask) {
  long4 extMask = convert_long4(mask);

#if defined(__AVX2__)
  return as_long4(_mm256_maskload_epi64((const long long *)pLoadAdd, (__m256i) extMask));
#else
  return as_long4(_mm256_maskload_pd((double *)pLoadAdd, (__m256d) extMask));
#endif
}

void INLINE_ATTRIBUTE __ocl_masked_store_long4(long4* pStoreAdd, long4 data, int4 mask) {
  long4 extMask = convert_long4(mask);

#if defined(__AVX2__)
  _mm256_maskstore_epi64((long long *)pStoreAdd, (__m256i) extMask, *((__m256i*) &data));
#else
  _mm256_maskstore_pd((double *)pStoreAdd, (__m256d) extMask, *((__m256d*) &data));
#endif
}

// ****************************************************************************
//                                 long8
// ****************************************************************************

long8 INLINE_ATTRIBUTE __ocl_masked_load_long8(long8* pLoadAdd, int8 mask) {
  long8 res;
  res.lo = __ocl_masked_load_long4((long4*) pLoadAdd, mask.lo);
  res.hi = __ocl_masked_load_long4(((long4*) pLoadAdd)+1, mask.hi);
  return res;
}

void INLINE_ATTRIBUTE __ocl_masked_store_long8(long8* pStoreAdd, long8 data, int8 mask) {
  __ocl_masked_store_long4((long4*) pStoreAdd, data.lo, mask.lo);
  __ocl_masked_store_long4(((long4*) pStoreAdd)+1, data.hi, mask.hi);
}

// ****************************************************************************
//                                 float4
// ****************************************************************************

float4 INLINE_ATTRIBUTE __ocl_masked_load_float4(float4* pLoadAdd, int4 mask) {
  return as_float4(_mm_maskload_ps((float const *)pLoadAdd, (__m128) mask));
}

void INLINE_ATTRIBUTE __ocl_masked_store_float4(float4* pStoreAdd, float4 data, int4 mask) {
  _mm_maskstore_ps((float *)pStoreAdd, (__m128) mask, *((__m128*) &data));
}

// ****************************************************************************
//                                 float8
// ****************************************************************************

float8 INLINE_ATTRIBUTE __ocl_masked_load_float8(float8* pLoadAdd, int8 mask) {
  return as_float8(_mm256_maskload_ps((float const *)pLoadAdd, (__m256) mask));
}

void INLINE_ATTRIBUTE __ocl_masked_store_float8(float8* pStoreAdd, float8 data, int8 mask) {
  _mm256_maskstore_ps((float *)pStoreAdd, (__m256) mask, *((__m256*) &data));
}

// ****************************************************************************
//                                 double4
// ****************************************************************************

double4 INLINE_ATTRIBUTE __ocl_masked_load_double4(double4* pLoadAdd, int4 mask) {
  long4 extMask = convert_long4(mask);
  return as_double4(_mm256_maskload_pd((double *)pLoadAdd, (__m256d) extMask));
}

void INLINE_ATTRIBUTE __ocl_masked_store_double4(double4* pStoreAdd, double4 data, int4 mask) {
  long4 extMask = convert_long4(mask);
  _mm256_maskstore_pd((double *)pStoreAdd, (__m256d) extMask, *((__m256d*) &data));
}

// ****************************************************************************
//                                 double8
// ****************************************************************************

double8 INLINE_ATTRIBUTE __ocl_masked_load_double8(double8* pLoadAdd, int8 mask) {
  double8 res;
  res.lo = __ocl_masked_load_double4((double4*) pLoadAdd, mask.lo);
  res.hi = __ocl_masked_load_double4(((double4*) pLoadAdd)+1, mask.hi);
  return res;
}

void INLINE_ATTRIBUTE __ocl_masked_store_double8(double8* pStoreAdd, double8 data, int8 mask) {
  __ocl_masked_store_double4((double4*) pStoreAdd, data.lo, mask.lo);
  __ocl_masked_store_double4(((double4*) pStoreAdd)+1, data.hi, mask.hi);
}

#endif // defined(__AVX__)
