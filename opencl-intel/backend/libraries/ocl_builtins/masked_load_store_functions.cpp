// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  masked_load_store_functions.cpp
///////////////////////////////////////////////////////////

#include "masked_load_store_functions.h"

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#include <intrin.h>


// ****************************************************************************
//                                 char4x4
// ****************************************************************************

#if defined(__AVX2__)

void __inline__ __attribute__((always_inline)) masked_load_char4x4_AVX2(char4* pLoadAdd, char4* pLoadedValues, int4 mask) {
  char16* tmpLoadedValues = (char16*)pLoadedValues;
  *tmpLoadedValues = (char16)_mm_maskload_epi32((int*)pLoadAdd, (__m128i)mask);
}

void __inline__ __attribute__((always_inline)) masked_store_char4x4_AVX2(char4* pStoreAdd, char4* pValuesToStore, int4 mask) {
  _mm_maskstore_epi32((int*)pStoreAdd, (__m128i) mask,*((__m128i*) pValuesToStore)); 
}

#endif // defined(__AVX2__)

#if defined(__AVX__)

void __inline__ __attribute__((always_inline)) masked_load_char4x4_AVX(char4* pLoadAdd, char4* pLoadedValues, int4 mask) {
  char16* tmpLoadedValues = (char16*)pLoadedValues;
  *tmpLoadedValues = (char16)_mm_maskload_ps((float*)pLoadAdd, (__m128)mask);
}

void __inline__ __attribute__((always_inline)) masked_store_char4x4_AVX(char4* pStoreAdd, char4* pValuesToStore, int4 mask) {
  _mm_maskstore_ps((float*)pStoreAdd, (__m128) mask,*((__m128*) pValuesToStore)); 
}

#endif // defined(__AVX__)


#if defined(__AVX__)

void __inline__ __attribute__((always_inline)) masked_load_char4x4(char4* pLoadAdd, char4* pLoadedValues, int4 mask) {
#if defined(__AVX2__)
  masked_load_char4x4_AVX2(pLoadAdd, pLoadedValues, mask);
#else // defined(__AVX__)
  masked_load_char4x4_AVX(pLoadAdd, pLoadedValues, mask);
#endif
}

void __inline__ __attribute__((always_inline)) masked_store_char4x4(char4* pStoreAdd, char4* pValuesToStore, int4 mask) {
#if defined(__AVX2__)
  masked_store_char4x4_AVX2(pStoreAdd, pValuesToStore, mask);
#else // defined(__AVX__)
  masked_store_char4x4_AVX(pStoreAdd, pValuesToStore, mask);
#endif
}

#endif // defined(__AVX__)

// ****************************************************************************
//                                 char4x8
// ****************************************************************************

#if defined(__AVX2__)

typedef __v32qi char32;

void __inline__ __attribute__((always_inline)) masked_load_char4x8_AVX2(char4* pLoadAdd, char4* pLoadedValues, int8 mask) {
   char32* tmpLoadedValues = (char32*)pLoadedValues;
   *tmpLoadedValues = (char32)_mm256_maskload_epi32((int*)pLoadAdd, (__m256i)mask);
}

void __inline__ __attribute__((always_inline)) masked_store_char4x8_AVX2(char4* pStoreAdd, char4* pValuesToStore, int8 mask) {
  _mm256_maskstore_epi32((int*)pStoreAdd, (__m256i) mask,*((__m256i*) pValuesToStore)); 
}

#endif // defined(__AVX2__)


#if defined(__AVX__)

void __inline__ __attribute__((always_inline)) masked_load_char4x8_AVX(char4* pLoadAdd, char4* pLoadedValues, int8 mask) {
  float8 res = (float8)_mm256_maskload_ps((float*)pLoadAdd, (__m256)mask);
  char16* tmpLoadedValues = (char16*)pLoadedValues;
  tmpLoadedValues[0] = (char16) res.lo;
  tmpLoadedValues[1] = (char16) res.hi;
}

void __inline__ __attribute__((always_inline)) masked_store_char4x8_AVX(char4* pStoreAdd, char4* pValuesToStore, int8 mask) {
  _mm256_maskstore_ps((float*)pStoreAdd, (__m256) mask,*((__m256*) pValuesToStore)); 
}

#endif // defined(__AVX__)


#if defined(__AVX__)

void __inline__ __attribute__((always_inline)) masked_load_char4x8(char4* pLoadAdd, char4* pLoadedValues, int8 mask) {
#if defined(__AVX2__)
  masked_load_char4x8_AVX2(pLoadAdd, pLoadedValues, mask);
#else // defined(__AVX__)
  masked_load_char4x8_AVX(pLoadAdd, pLoadedValues, mask);
#endif
}

void __inline__ __attribute__((always_inline)) masked_store_char4x8(char4* pStoreAdd, char4* pValuesToStore, int8 mask) {
#if defined(__AVX2__)
  masked_store_char4x8_AVX2(pStoreAdd, pValuesToStore, mask);
#else // defined(__AVX__)
  masked_store_char4x8_AVX(pStoreAdd, pValuesToStore, mask);
#endif
}

#endif // defined(__AVX__)

// ****************************************************************************
//                                 int4x4
// ****************************************************************************

#if defined(__AVX2__)

void __inline__ __attribute__((always_inline))  masked_load_int4x4_AVX2(int4* pLoadAdd, int4* pLoadedValues, int4 mask) {
  
  int8 extendedMask = mask.s01230123;
  int8 maskLow = extendedMask.s00001111;
  int8 maskHigh = extendedMask.s22223333;
  
  int8* tmpLoadAdd = (int8*)pLoadAdd;
  int8* tmpLoadedValues = (int8*)pLoadedValues;
  tmpLoadedValues[0] = (int8)_mm256_maskload_epi32((int*)&(tmpLoadAdd[0]), (__m256i)maskLow);
  tmpLoadedValues[1] = (int8)_mm256_maskload_epi32((int*)&(tmpLoadAdd[1]), (__m256i)maskHigh);
}

void __inline__ __attribute__((always_inline))  masked_store_int4x4_AVX2(int4* pStoreAdd, int4* pValuesToStore, int4 mask) {
  
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

void __inline__ __attribute__((always_inline)) masked_load_float4x4_common(float4* pLoadAdd, float4* pLoadedValues, int4 mask) {

  float8 extendedMask = (float8) mask.s01230123;
  float8 maskLow = extendedMask.s00001111;
  float8 maskHigh = extendedMask.s22223333;
  
  float8* tmpLoadAdd = (float8*)pLoadAdd;
  float8* tmpLoadedValues = (float8*)pLoadedValues;
  tmpLoadedValues[0] = (float8)_mm256_maskload_ps((float*)&(tmpLoadAdd[0]), (__m256)maskLow);
  tmpLoadedValues[1] = (float8)_mm256_maskload_ps((float*)&(tmpLoadAdd[1]), (__m256)maskHigh);
}

void __inline__ __attribute__((always_inline))  masked_load_int4x4_AVX(int4* pLoadAdd, int4* pLoadedValues, int4 mask) {
  masked_load_float4x4_common((float4*)pLoadAdd, (float4*)pLoadedValues, mask);
}

void __inline__ __attribute__((always_inline))  masked_store_int4x4_AVX(int4* pStoreAdd, int4* pValuesToStore, int4 mask) {
  masked_store_float4x4((float4*)pStoreAdd, (float4*)pValuesToStore, mask);
}

#endif // defined(__AVX__)


#if defined(__AVX__)

void __inline__ __attribute__((always_inline)) masked_load_int4x4(int4* pLoadAdd, int4* pLoadedValues, int4 mask) {
#if defined(__AVX2__)
  masked_load_int4x4_AVX2(pLoadAdd, pLoadedValues, mask);
#else // defined(__AVX__)
  masked_load_int4x4_AVX(pLoadAdd, pLoadedValues, mask);
#endif
}

void __inline__ __attribute__((always_inline)) masked_store_int4x4(int4* pStoreAdd, int4* pValuesToStore, int4 mask) {
#if defined(__AVX2__)
  masked_store_int4x4_AVX2(pStoreAdd, pValuesToStore, mask);
#else // defined(__AVX__)
  masked_store_int4x4_AVX(pStoreAdd, pValuesToStore, mask);
#endif
}

#endif // defined(__AVX__)

// ****************************************************************************
//                                 int4x8
// ****************************************************************************

#if defined(__AVX__)

void __inline__ __attribute__((always_inline)) masked_load_int4x8(int4* pLoadAdd, int4* pLoadedValues, int8 mask) {  
  masked_load_int4x4(&(pLoadAdd[0]), &(pLoadedValues[0]), mask.lo);
  masked_load_int4x4(&(pLoadAdd[4]), &(pLoadedValues[4]), mask.hi);
}

void __inline__ __attribute__((always_inline)) masked_store_int4x8(int4* pStoreAdd, int4* pValuesToStore, int8 mask) {
  masked_store_int4x4(&(pStoreAdd[0]), &(pValuesToStore[0]), mask.lo);
  masked_store_int4x4(&(pStoreAdd[4]), &(pValuesToStore[4]), mask.hi);
}

#endif // defined(__AVX__)


// ****************************************************************************
//                                 float4x4
// ****************************************************************************

#if defined(__AVX__)

void __inline__ __attribute__((always_inline)) masked_load_float4x4(float4* pLoadAdd, float4* pLoadedValues, int4 mask) {
  *((float16*)pLoadedValues) = 0;
  masked_load_float4x4_common((float4*)pLoadAdd, (float4*)pLoadedValues, mask);
}

void __inline__ __attribute__((always_inline)) masked_store_float4x4(float4* pStoreAdd, float4* pValuesToStore, int4 mask) {
  
  float8 extendedMask = (float8) mask.s01230123;
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

void __inline__ __attribute__((always_inline)) masked_load_float4x8(float4* pLoadAdd, float4* pLoadedValues, int8 mask) {
  masked_load_float4x4(&(pLoadAdd[0]), &(pLoadedValues[0]), mask.lo);
  masked_load_float4x4(&(pLoadAdd[4]), &(pLoadedValues[4]), mask.hi);
}

void __inline__ __attribute__((always_inline)) masked_store_float4x8(float4* pStoreAdd, float4* pValuesToStore, int8 mask)  {
  masked_store_float4x4(&(pStoreAdd[0]), &(pValuesToStore[0]), mask.lo);
  masked_store_float4x4(&(pStoreAdd[4]), &(pValuesToStore[4]), mask.hi);
}

#endif // defined(__AVX__)

#if defined(__AVX__)

// ****************************************************************************
//                                 int4
// ****************************************************************************

int4 __inline__ __attribute__((always_inline)) masked_load_int4(int4* pLoadAdd, int4 mask) {
#if defined(__AVX2__)
  return as_int4(_mm_maskload_epi32((int const *)pLoadAdd, (__m128i) mask));
#else
  return as_int4(_mm_maskload_ps ((float const *)pLoadAdd, (__m128) mask));
#endif
}

void __inline__ __attribute__((always_inline)) masked_store_int4(int4* pStoreAdd, int4 data, int4 mask) {
#if defined(__AVX2__)
  _mm_maskstore_epi32((int *)pStoreAdd, (__m128i) mask, *((__m128i*) &data));
#else
  _mm_maskstore_ps((float *)pStoreAdd, (__m128) mask, *((__m128*) &data));
#endif
}

// ****************************************************************************
//                                 int8
// ****************************************************************************

int8 __inline__ __attribute__((always_inline)) masked_load_int8(int8* pLoadAdd, int8 mask) {
#if defined(__AVX2__)
  return as_int8(_mm256_maskload_epi32((int const *)pLoadAdd, (__m256i) mask));
#else
  return as_int8(_mm256_maskload_ps((float const *)pLoadAdd, (__m256) mask)); 
#endif
}

void __inline__ __attribute__((always_inline)) masked_store_int8(int8* pStoreAdd, int8 data, int8 mask) {
#if defined(__AVX2__)
  _mm256_maskstore_epi32((int *)pStoreAdd, (__m256i) mask, *((__m256i*) &data));
#else
  _mm256_maskstore_ps((float *)pStoreAdd, (__m256) mask, *((__m256*) &data));
#endif
}

// ****************************************************************************
//                                 long4
// ****************************************************************************

long4 __inline__ __attribute__((always_inline)) masked_load_long4(long4* pLoadAdd, int4 mask) {
  long4 extMask = convert_long4(mask);

#if defined(__AVX2__)
  return as_long4(_mm256_maskload_epi64((long const *)pLoadAdd, (__m256i) extMask));
#else
  return as_long4(_mm256_maskload_pd((double *)pLoadAdd, (__m256d) extMask));
#endif
}

void __inline__ __attribute__((always_inline)) masked_store_long4(long4* pStoreAdd, long4 data, int4 mask) {
  long4 extMask = convert_long4(mask);

#if defined(__AVX2__)
  _mm256_maskstore_epi64((long *)pStoreAdd, (__m256i) extMask, *((__m256i*) &data));
#else
  _mm256_maskstore_pd((double *)pStoreAdd, (__m256d) extMask, *((__m256d*) &data));
#endif
}

// ****************************************************************************
//                                 long8
// ****************************************************************************

long8 __inline__ __attribute__((always_inline)) masked_load_long8(long8* pLoadAdd, int8 mask) {
  long8 res;
  res.lo = masked_load_long4((long4*) pLoadAdd, mask.lo);
  res.hi = masked_load_long4(((long4*) pLoadAdd)+1, mask.hi);
  return res;
}

void __inline__ __attribute__((always_inline)) masked_store_long8(long8* pStoreAdd, long8 data, int8 mask) {
  masked_store_long4((long4*) pStoreAdd, data.lo, mask.lo);
  masked_store_long4(((long4*) pStoreAdd)+1, data.hi, mask.hi);
}

// ****************************************************************************
//                                 float4
// ****************************************************************************

float4 __inline__ __attribute__((always_inline)) masked_load_float4(float4* pLoadAdd, int4 mask) {
  return as_float4(_mm_maskload_ps((float const *)pLoadAdd, (__m128) mask));
}

void __inline__ __attribute__((always_inline)) masked_store_float4(float4* pStoreAdd, float4 data, int4 mask) {
  _mm_maskstore_ps((float *)pStoreAdd, (__m128) mask, *((__m128*) &data));
}

// ****************************************************************************
//                                 float8
// ****************************************************************************

float8 __inline__ __attribute__((always_inline)) masked_load_float8(float8* pLoadAdd, int8 mask) {
  return as_float8(_mm256_maskload_ps((float const *)pLoadAdd, (__m256) mask)); 
}

void __inline__ __attribute__((always_inline)) masked_store_float8(float8* pStoreAdd, float8 data, int8 mask) {
  _mm256_maskstore_ps((float *)pStoreAdd, (__m256) mask, *((__m256*) &data));
}

// ****************************************************************************
//                                 double4
// ****************************************************************************

double4 __inline__ __attribute__((always_inline)) masked_load_double4(double4* pLoadAdd, int4 mask) {
  long4 extMask = convert_long4(mask);
  return as_double4(_mm256_maskload_pd((double *)pLoadAdd, (__m256d) extMask));
}

void __inline__ __attribute__((always_inline)) masked_store_double4(double4* pStoreAdd, double4 data, int4 mask) {
  long4 extMask = convert_long4(mask);
  _mm256_maskstore_pd((double *)pStoreAdd, (__m256d) extMask, *((__m256d*) &data));
}

// ****************************************************************************
//                                 double8
// ****************************************************************************

double8 __inline__ __attribute__((always_inline)) masked_load_double8(double8* pLoadAdd, int8 mask) {
  double8 res;
  res.lo = masked_load_double4((double4*) pLoadAdd, mask.lo);
  res.hi = masked_load_double4(((double4*) pLoadAdd)+1, mask.hi);
  return res;
}

void __inline__ __attribute__((always_inline)) masked_store_double8(double8* pStoreAdd, double8 data, int8 mask) {
  masked_store_double4((double4*) pStoreAdd, data.lo, mask.lo);
  masked_store_double4(((double4*) pStoreAdd)+1, data.hi, mask.hi);
}

#endif // defined(__AVX__)