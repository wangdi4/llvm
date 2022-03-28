/*===---- csaintrin.h - CSA intrinsics -------------------------------------=== */
/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2021 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */
/*
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef __CSAINTRIN_H
#define __CSAINTRIN_H

typedef float           __m64f __attribute__((__vector_size__(8))); /* Used for fp32x2 */

#ifdef __CSA__
/* Define the types for vector types on CSA
   q    : denotes  8 bits size
   h    : denotes 16 bits size
   s, u : denote  32 bits size */

typedef _Float16        __m64h __attribute__((__vector_size__(8))); /* Used for fp16x4           */
typedef signed char     __v8qi __attribute__((__vector_size__(8))); /* Used for signed int8x8    */
typedef unsigned char   __v8qu __attribute__((__vector_size__(8))); /* Used for unsigned int8x8  */
typedef short           __v4hi __attribute__((__vector_size__(8))); /* Used for signed int16x4   */
typedef unsigned short  __v4hu __attribute__((__vector_size__(8))); /* Used for unsigned int16x4 */
typedef int             __v2si __attribute__((__vector_size__(8))); /* Used for signed int32x2   */
typedef unsigned int    __v2ui __attribute__((__vector_size__(8))); /* Used for unsigned int32x2 */
typedef long long       __m64i __attribute__((__vector_size__(8))); /* Used for signatures (int8x8, int16x4, int32x2, etc.) */
#endif /* __CSA__ */

/* Disable and swizzle enums */
typedef enum {
  _MM_SWIZZLE_NONE = 0,
  _MM_SWIZZLE_BCAST_LOW = 1,
  _MM_SWIZZLE_BCAST_HIGH = 2,
  _MM_SWIZZLE_INTERCHANGE = 3
} _MM_SWIZZLE_ENUM;

typedef enum {
  _MM_DISABLE_NONE = 0,
  _MM_DISABLE_LOW = 1,
  _MM_DISABLE_HIGH = 2,
  _MM_DISABLE_BOTH = 3
} _MM_DISABLE_ENUM;

/* Comparison enum. These values should match the FP values of ISD::CondCode
 * where possible. This matches up as follows (in terms of bits): S N U L G E
 * The S bit is the signaling parameter.
 * The N bit is don't-care-about-NaN, or signed integer comparison.
 * The other bits are true if the result of the FP op is unordered, less,
 * greater, or equal, respectively. */
#define __CMP_S_BIT 0x80
#define __CMP_N_BIT 0x10
#define __CMP_U_BIT 0x08
typedef enum {
  _CMP_FALSE_OQ,
  _CMP_EQ_OQ,
  _CMP_GT_OQ,
  _CMP_GE_OQ,
  _CMP_LT_OQ,
  _CMP_LE_OQ,
  _CMP_NEQ_OQ,
  _CMP_ORD_Q,
  _CMP_UNORD_Q,
  _CMP_EQ_UQ,
  _CMP_NLE_UQ,
  _CMP_NLT_UQ,
  _CMP_NGE_UQ,
  _CMP_NGT_UQ,
  _CMP_NEQ_UQ,
  _CMP_TRUE_UQ,

  _CMP_FALSE_OS = _CMP_FALSE_OQ | __CMP_S_BIT,
  _CMP_EQ_OS,
  _CMP_GT_OS,
  _CMP_GE_OS,
  _CMP_LT_OS,
  _CMP_LE_OS,
  _CMP_NEQ_OS,
  _CMP_ORD_S,
  _CMP_UNORD_S,
  _CMP_EQ_US,
  _CMP_NLE_US,
  _CMP_NLT_US,
  _CMP_NGE_US,
  _CMP_NGT_US,
  _CMP_NEQ_US,
  _CMP_TRUE_US,
} _MM_CMP_ENUM;

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__))

/* Functions for building and unpacking vectors */
static __inline__ __m64f __DEFAULT_FN_ATTRS _mm64_pack_ps(float v0, float v1)
{
  return (__m64f){v0, v1};
}

static __inline__ __m64f __DEFAULT_FN_ATTRS _mm64_setzero_ps(void)
{
  return (__m64f){ 0.0f, 0.0f };
}

static __inline__ float __DEFAULT_FN_ATTRS _mm64_extract_ps(__m64f a, int i)
{
  return a[i];
}

#ifdef __CSA__
#define _mm64_add_ps(A, B, D, M, N) \
  ((__m64f)(__builtin_csa_addf32x2((__m64f)(A), (__m64f)(B), \
                                   (D), (M), (N))))

#define _mm64_sub_ps(A, B, D, M, N) \
  ((__m64f)(__builtin_csa_subf32x2((__m64f)(A), (__m64f)(B), \
                                   (D), (M), (N))))

#define _mm64_addsub_ps(A, B, D, M, N) \
  ((__m64f)(__builtin_csa_addsubf32x2((__m64f)(A), (__m64f)(B), \
                                      (D), (M), (N))))

#define _mm64_subadd_ps(A, B, D, M, N) \
  ((__m64f)(__builtin_csa_subaddf32x2((__m64f)(A), (__m64f)(B), \
                                      (D), (M), (N))))

#define _mm64_mul_ps(A, B, D, M, N) \
  ((__m64f)(__builtin_csa_mulf32x2((__m64f)(A), (__m64f)(B), \
                                   (D), (M), (N))))

#define _mm64_fma_ps(A, B, C, D, M, N) \
  ((__m64f)(__builtin_csa_fmaf32x2((__m64f)(A), (__m64f)(B), (__m64f)(C), \
                                   (D), (M), (N))))

#define _mm64_fms_ps(A, B, C, D, M, N) \
  ((__m64f)(__builtin_csa_fmsf32x2((__m64f)(A), (__m64f)(B), (__m64f)(C), \
                                   (D), (M), (N))))

#define _mm64_fmrs_ps(A, B, C, D, M, N) \
  ((__m64f)(__builtin_csa_fmrsf32x2((__m64f)(A), (__m64f)(B), (__m64f)(C), \
                                    (D), (M), (N))))

#define _mm64_fmas_ps(A, B, C, D, M, N) \
  ((__m64f)(__builtin_csa_fmasf32x2((__m64f)(A), (__m64f)(B), (__m64f)(C), \
                                    (D), (M), (N))))

#define _mm64_fmsa_ps(A, B, C, D, M, N) \
  ((__m64f)(__builtin_csa_fmsaf32x2((__m64f)(A), (__m64f)(B), (__m64f)(C), \
                                    (D), (M), (N))))
#else
#include <cmath>

static float fms(float a, float b, float c) {
#pragma clang fp contract(fast)
  return a * b - c;
}

static float fmrs(float a, float b, float c) {
#pragma clang fp contract(fast)
  return c - a * b;
}


static __m64f _mm64_add_ps(__m64f a, __m64f b, _MM_DISABLE_ENUM disable, _MM_SWIZZLE_ENUM swizzle1, _MM_SWIZZLE_ENUM swizzle2) {
  switch (swizzle1) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: a[1] = a[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: a[0] = a[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = a[0];
    a[0] = a[1];
    a[1] = tmp;
    break;
  }
  }
  switch (swizzle2) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: b[1] = b[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: b[0] = b[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = b[0];
    b[0] = b[1];
    b[1] = tmp;
    break;
  }
  }
  float a_lo = a[0], a_hi = a[1];
  float b_lo = b[0], b_hi = b[1];
  float res_lo = (disable & 1) ? a_lo : a_lo + b_lo;
  float res_hi = (disable & 2) ? a_hi : a_hi + b_hi;
  return (__m64f){res_lo, res_hi};
}

static __m64f _mm64_sub_ps(__m64f a, __m64f b, _MM_DISABLE_ENUM disable, _MM_SWIZZLE_ENUM swizzle1, _MM_SWIZZLE_ENUM swizzle2) { 
  switch (swizzle1) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: a[1] = a[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: a[0] = a[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = a[0];
    a[0] = a[1];
    a[1] = tmp;
    break;
  }
  }
  switch (swizzle2) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: b[1] = b[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: b[0] = b[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = b[0];
    b[0] = b[1];
    b[1] = tmp;
    break;
  }
  }
  float a_lo = a[0], a_hi = a[1];
  float b_lo = b[0], b_hi = b[1];
  float res_lo = (disable & 1) ? a_lo : a_lo - b_lo;
  float res_hi = (disable & 2) ? a_hi : a_hi - b_hi;
  return (__m64f){res_lo, res_hi};
}

static __m64f _mm64_mul_ps(__m64f a, __m64f b, _MM_DISABLE_ENUM disable, _MM_SWIZZLE_ENUM swizzle1, _MM_SWIZZLE_ENUM swizzle2) { 
  switch (swizzle1) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: a[1] = a[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: a[0] = a[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = a[0];
    a[0] = a[1];
    a[1] = tmp;
    break;
  }
  }
  switch (swizzle2) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: b[1] = b[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: b[0] = b[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = b[0];
    b[0] = b[1];
    b[1] = tmp;
    break;
  }
  }
  float a_lo = a[0], a_hi = a[1];
  float b_lo = b[0], b_hi = b[1];
  float res_lo = (disable & 1) ? a_lo : a_lo * b_lo;
  float res_hi = (disable & 2) ? a_hi : a_hi * b_hi;
  return (__m64f){res_lo, res_hi};
}

static __m64f _mm64_addsub_ps(__m64f a, __m64f b, _MM_DISABLE_ENUM disable, _MM_SWIZZLE_ENUM swizzle1, _MM_SWIZZLE_ENUM swizzle2)  { 
  switch (swizzle1) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: a[1] = a[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: a[0] = a[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = a[0];
    a[0] = a[1];
    a[1] = tmp;
    break;
  }
  }
  switch (swizzle2) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: b[1] = b[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: b[0] = b[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = b[0];
    b[0] = b[1];
    b[1] = tmp;
    break;
  }
  }
  float a_lo = a[0], a_hi = a[1];
  float b_lo = b[0], b_hi = b[1];
  float res_lo = (disable & 1) ? a_lo : a_lo - b_lo;
  float res_hi = (disable & 2) ? a_hi : a_hi + b_hi;
  return (__m64f){res_lo, res_hi};
}

static __m64f _mm64_subadd_ps(__m64f a, __m64f b, _MM_DISABLE_ENUM disable, _MM_SWIZZLE_ENUM swizzle1, _MM_SWIZZLE_ENUM swizzle2)  { 
  switch (swizzle1) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: a[1] = a[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: a[0] = a[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = a[0];
    a[0] = a[1];
    a[1] = tmp;
    break;
  }
  }
  switch (swizzle2) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: b[1] = b[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: b[0] = b[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = b[0];
    b[0] = b[1];
    b[1] = tmp;
    break;
  }
  }
  float a_lo = a[0], a_hi = a[1];
  float b_lo = b[0], b_hi = b[1];
  float res_lo = (disable & 1) ? a_lo : a_lo + b_lo;
  float res_hi = (disable & 2) ? a_hi : a_hi - b_hi;
  return (__m64f){res_lo, res_hi};
}

static __m64f _mm64_fma_ps(__m64f a, __m64f b, __m64f c, _MM_DISABLE_ENUM disable, _MM_SWIZZLE_ENUM swizzle1, _MM_SWIZZLE_ENUM swizzle2)  {
  switch (swizzle1) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: a[1] = a[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: a[0] = a[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = a[0];
    a[0] = a[1];
    a[1] = tmp;
    break;
  }
  }
  switch (swizzle2) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: b[1] = b[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: b[0] = b[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = b[0];
    b[0] = b[1];
    b[1] = tmp;
    break;
  }
  }
  float a_lo = a[0], a_hi = a[1];
  float b_lo = b[0], b_hi = b[1];
  float c_lo = c[0], c_hi = c[1];
  float res_lo = (disable & 1) ? a_lo : fma(a_lo, b_lo, c_lo);
  float res_hi = (disable & 2) ? a_hi : fma(a_hi, b_hi, c_hi);
  return (__m64f){res_lo, res_hi};
}


static __m64f _mm64_fms_ps(__m64f a, __m64f b, __m64f c, _MM_DISABLE_ENUM disable, _MM_SWIZZLE_ENUM swizzle1, _MM_SWIZZLE_ENUM swizzle2)  {
  switch (swizzle1) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: a[1] = a[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: a[0] = a[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = a[0];
    a[0] = a[1];
    a[1] = tmp;
    break;
  }
  }
  switch (swizzle2) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: b[1] = b[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: b[0] = b[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = b[0];
    b[0] = b[1];
    b[1] = tmp;
    break;
  }
  }
  float a_lo = a[0], a_hi = a[1];
  float b_lo = b[0], b_hi = b[1];
  float c_lo = c[0], c_hi = c[1];
  float res_lo = (disable & 1) ? a_lo : fms(a_lo, b_lo, c_lo);
  float res_hi = (disable & 2) ? a_hi : fms(a_hi, b_hi, c_hi);
  return (__m64f){res_lo, res_hi};
}

static __m64f _mm64_fmrs_ps(__m64f a, __m64f b, __m64f c, _MM_DISABLE_ENUM disable, _MM_SWIZZLE_ENUM swizzle1, _MM_SWIZZLE_ENUM swizzle2)  {
  switch (swizzle1) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: a[1] = a[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: a[0] = a[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = a[0];
    a[0] = a[1];
    a[1] = tmp;
    break;
  }
  }
  switch (swizzle2) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: b[1] = b[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: b[0] = b[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = b[0];
    b[0] = b[1];
    b[1] = tmp;
    break;
  }
  }
  float a_lo = a[0], a_hi = a[1];
  float b_lo = b[0], b_hi = b[1];
  float c_lo = c[0], c_hi = c[1];
  float res_lo = (disable & 1) ? a_lo : fmrs(a_lo, b_lo, c_lo);
  float res_hi = (disable & 2) ? a_hi : fmrs(a_hi, b_hi, c_hi);
  return (__m64f){res_lo, res_hi};
}

static __m64f _mm64_fmas_ps(__m64f a, __m64f b, __m64f c, _MM_DISABLE_ENUM disable, _MM_SWIZZLE_ENUM swizzle1, _MM_SWIZZLE_ENUM swizzle2)  {
  switch (swizzle1) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: a[1] = a[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: a[0] = a[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = a[0];
    a[0] = a[1];
    a[1] = tmp;
    break;
  }
  }
  switch (swizzle2) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: b[1] = b[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: b[0] = b[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = b[0];
    b[0] = b[1];
    b[1] = tmp;
    break;
  }
  }
  float a_lo = a[0], a_hi = a[1];
  float b_lo = b[0], b_hi = b[1];
  float c_lo = c[0], c_hi = c[1];
  float res_lo = (disable & 1) ? a_lo : fms(a_lo, b_lo, c_lo);
  float res_hi = (disable & 2) ? a_hi : fma(a_hi, b_hi, c_hi);
  return (__m64f){res_lo, res_hi};
}

static __m64f _mm64_fmsa_ps(__m64f a, __m64f b, __m64f c, _MM_DISABLE_ENUM disable, _MM_SWIZZLE_ENUM swizzle1, _MM_SWIZZLE_ENUM swizzle2)  {
  switch (swizzle1) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: a[1] = a[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: a[0] = a[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = a[0];
    a[0] = a[1];
    a[1] = tmp;
    break;
  }
  }
  switch (swizzle2) {
  case _MM_SWIZZLE_NONE: break;
  case _MM_SWIZZLE_BCAST_LOW: b[1] = b[0]; break;
  case _MM_SWIZZLE_BCAST_HIGH: b[0] = b[1]; break;
  case _MM_SWIZZLE_INTERCHANGE: {
    float tmp = b[0];
    b[0] = b[1];
    b[1] = tmp;
    break;
  }
  }
  float a_lo = a[0], a_hi = a[1];
  float b_lo = b[0], b_hi = b[1];
  float c_lo = c[0], c_hi = c[1];
  float res_lo = (disable & 1) ? a_lo : fma(a_lo, b_lo, c_lo);
  float res_hi = (disable & 2) ? a_hi : fms(a_hi, b_hi, c_hi);
  return (__m64f){res_lo, res_hi};
}
#endif /* __CSA__ */

#define _mm64_shuf_ps(A, B, M, N) \
  ((__m64f)(__builtin_shufflevector((__m64f)(A), (__m64f)(B), (M), (N))))


#ifdef __CSA__
/*
 * Integer Vector Intrinsics
 */

/* Add packed 8-bit integers in a and b, and return the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_add_epi8 (__m64i a, __m64i b)
{
  return (__m64i)((__v8qi)a + (__v8qi)b);
}

/* Add packed 16-bit integers in a and b, and return the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_add_epi16(__m64i a, __m64i b)
{
  return (__m64i)((__v4hi)a + (__v4hi)b);
}

/* Subtract packed 8-bit integers in b from packed 8-bit integers in a, and return the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_sub_epi8(__m64i a, __m64i b)
{
  return (__m64i) ((__v8qi)a - (__v8qi)b);
}

/* Subtract packed 16-bit integers in b from packed 16-bit integers in a, and return the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_sub_epi16(__m64i a, __m64i b)
{
  return (__m64i) ((__v4hi)a - (__v4hi)b);
}

/* Add packed 8-bit integers in a and b using saturation, and return the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_adds_epi8(__m64i a, __m64i b)
{
  return (__m64i) __builtin_csa_addss8x8((__v8qi)a, (__v8qi)b);
}

/* Add packed 16-bit integers in a and b using saturation, and return the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_adds_epi16(__m64i a, __m64i b)
{
  return (__m64i) __builtin_csa_addss16x4((__v4hi)a, (__v4hi)b);
}

/* Add packed unsigned 8-bit integers in a and b using saturation, and return the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_adds_epu8(__m64i a, __m64i b)
{
  return (__m64i) __builtin_csa_addsu8x8((__v8qu)a, (__v8qu)b);
}

/* Add packed unsigned 16-bit integers in a and b using saturation, and return the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_adds_epu16(__m64i a, __m64i b)
{
  return (__m64i) __builtin_csa_addsu16x4((__v4hu)a, (__v4hu)b);
}

/* Subtract packed 8-bit integers in b from packed 8-bit integers in a using saturation,
   and return the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_subs_epi8(__m64i a, __m64i b)
{
  return (__m64i) __builtin_csa_subss8x8((__v8qi)a, (__v8qi)b);
}

/* Subtract packed 16-bit integers in b from packed 16-bit integers in a using saturation,
   and return the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_subs_epi16(__m64i a, __m64i b)
{
  return (__m64i) __builtin_csa_subss16x4((__v4hi)a, (__v4hi)b);
}

/* Subtract packed unsigned 8-bit integers in b from packed unsigned 8-bit integers in a
   using saturation, and return the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_subs_epu8(__m64i a, __m64i b)
{
  return (__m64i) __builtin_csa_subsu8x8((__v8qu)a, (__v8qu)b);
}

/* Subtract packed unsigned 16-bit integers in b from packed unsigned 16-bit integers in a
   using saturation, and return the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_subs_epu16(__m64i a, __m64i b)
{
  return (__m64i) __builtin_csa_subsu16x4((__v4hu)a, (__v4hu)b);
}

/* Compare packed 8-bit integers in a and b, and return the packed minimum values. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_min_epi8(__m64i a, __m64i b)
{
  return (__m64i) __builtin_csa_mins8x8((__v8qi)a, (__v8qi)b);
}

/* Compare packed 16-bit integers in a and b, and return the packed minimum values. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_min_epi16(__m64i a, __m64i b)
{
  return (__m64i) __builtin_csa_mins16x4((__v4hi)a, (__v4hi)b);
}

/* Compare packed unsigned 8-bit integers in a and b, and return the packed minimum values. */

static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_min_epu8(__m64i a, __m64i b)
{
  return (__m64i) __builtin_csa_minu8x8((__v8qu)a, (__v8qu)b);
}

/* Compare packed unsigned 16-bit integers in a and b, and return the packed minimum values. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_min_epu16(__m64i a, __m64i b)
{
  return (__m64i) __builtin_csa_minu16x4((__v4hu)a, (__v4hu)b);
}

/* Compare packed 8-bit integers in a and b, and return the packed maximum values. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_max_epi8(__m64i a, __m64i b)
{
  return (__m64i)__builtin_csa_maxs8x8((__v8qi)a, (__v8qi)b);
}

/* Compare packed 16-bit integers in a and b, and return the packed maximum values. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_max_epi16(__m64i a, __m64i b)
{
  return (__m64i) __builtin_csa_maxs16x4((__v4hi)a, (__v4hi)b);
}

/* Compare packed unsigned 8-bit integers in a and b, and return the packed maximum values. */

static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_max_epu8(__m64i a, __m64i b)
{
  return (__m64i) __builtin_csa_maxu8x8((__v8qu)a, (__v8qu)b);
}

/* Compare packed unsigned 16-bit integers in a and b, and return the packed maximum values. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_max_epu16(__m64i a, __m64i b)
{
  return (__m64i) __builtin_csa_maxu16x4((__v4hu)a, (__v4hu)b);
}

/* Compute the bitwise AND of corresponding 8-bit integers in a and b, and set the
   corresponding bit in the result if the result is non-zero. */
static __inline__ int __DEFAULT_FN_ATTRS _mm64_test_epi8(__m64i a, __m64i b)
{
  return (int)__builtin_csa_test8x8((__v8qu)a, (__v8qu)b);
}

/* Compute the bitwise AND of corresponding 16-bit integers in a and b, and set the
   corresponding bit in the result if the result is non-zero. */
static __inline__ int __DEFAULT_FN_ATTRS _mm64_test_epi16(__m64i a, __m64i b)
{
  return (int)__builtin_csa_test16x4((__v4hu)a, (__v4hu)b);
}

/* Compute the bitwise AND of corresponding 32-bit integers in a and b, and set the
   corresponding bit in the result if the result is non-zero. */
static __inline__ int __DEFAULT_FN_ATTRS _mm64_test_epi32(__m64i a, __m64i b)
{
  return (int)__builtin_csa_test32x2((__m64f)a, (__m64f)b);
}

/* Compute the absolute value of packed 8-bit integers in a, and return the unsigned results. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_abs_epi8(__m64i a)
{
  return (__m64i)__builtin_csa_abs8x8((__v8qu)a);
}

/* Compute the absolute value of packed 16-bit integers in a, and return the unsigned results. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_abs_epi16(__m64i a)
{
  return (__m64i)__builtin_csa_abs16x4((__v4hu)a);
}

/* Compute the average of packed 8-bit integers in a and b, and return the results. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_avg_epi8(__m64i a, __m64i b)
{
  return (__m64i)__builtin_csa_avgs8x8((__v8qi)a, (__v8qi)b);
}

/* Compute the average of packed unsigned 8-bit integers in a and b, and return the results. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_avg_epu8(__m64i a, __m64i b)
{
  return (__m64i)__builtin_csa_avgu8x8((__v8qu)a, (__v8qu)b);
}

/* Compute the average of packed 16-bit integers in a and b, and return the results. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_avg_epi16(__m64i a, __m64i b)
{
  return (__m64i)__builtin_csa_avgs16x4((__v4hi)a, (__v4hi)b);
}

/* Compute the average of packed unsigned 16-bit integers in a and b, and return the results. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_avg_epu16(__m64i a, __m64i b)
{
  return (__m64i)__builtin_csa_avgu16x4((__v4hu)a, (__v4hu)b);
}

/* Create mask from the most significant bit of each 8-bit element in a, and return the result. */
static __inline__ int __DEFAULT_FN_ATTRS _mm64_movemask_epi8(__m64i a)
{
  return (int)__builtin_csa_movmsk8x8((__v8qu)a);
}

/* Create mask from the most significant bit of each 16-bit element in a, and return the result. */
static __inline__ int __DEFAULT_FN_ATTRS _mm64_movemask_epi16(__m64i a)
{
  return (int)__builtin_csa_movmsk16x4((__v4hu)a);
}

/* Compute the bitwise AND of 64 bits (representing integer data) in a and b, and return the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_and_si64(__m64i a, __m64i b)
{
  return (__m64i)((__m64i)a & (__m64i)b);
}

/* Compute the bitwise OR of 64 bits (representing integer data) in a and b, and return the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_or_si64(__m64i a, __m64i b)
{
  return (__m64i)((__m64i)a | (__m64i)b);
}

/* Compute the bitwise XOR of 64 bits (representing integer data) in a and b, and return the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_xor_si64(__m64i a, __m64i b)
{
  return (__m64i) (a ^ b);
}

/* Copy 64-bit integer a to the result. */
static __inline__ __int64 __DEFAULT_FN_ATTRS _mm64_cvtm64_si64(__m64i a)
{
  return (__int64) a;
}

/* Copy 32-bit integer a to the lower elements of the result, and zero the upper element. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_cvtsi32_si64(int a)
{
  return (__m64i) ((__v2si){a, 0});
}

/* Copy 64-bit integer a to the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_cvtsi64_m64(__int64 a)
{
  return (__m64i) a;
}

/* Copy the lower 32-bit integer in a to the result. */
static __inline__ int __DEFAULT_FN_ATTRS _mm64_cvtsi64_si32(__m64i a)
{
  return (int) (((__v2si)a)[0]);
}

/* Set packed 8-bit integers in the result with the supplied values. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_set_epi8(char e7, char e6, char e5, char e4, char e3, char e2, char e1, char e0)
{
  return (__m64i) ((__v8qi){e0, e1, e2, e3, e4, e5, e6, e7});
}

/* Set packed 16-bit integers in the result with the supplied values. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_set_epi16(short e3, short e2, short e1, short e0)
{
  return (__m64i) ((__v4hi){e0, e1, e2, e3});
}

/* Set packed 32-bit integers in the result with the supplied values. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_set_epi32(int e1, int e0)
{
  return (__m64i) ((__v2si){e0, e1});
}

/* Broadcast 8-bit integer a to all elements of the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_set1_epi8(char a)
{
  return (__m64i) ((__v8qi){a, a, a, a, a, a, a, a});
}

/* Broadcast 16-bit integer a to all elements of the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS  _mm64_set1_epi16(short a)
{
  return (__m64i) ((__v4hi){a, a, a, a});
}

/* Broadcast 32-bit integer a to all elements of the result. */
static __inline__ __m64i __DEFAULT_FN_ATTRS  _mm64_set1_epi32(int a)
{
  return (__m64i) ((__v2si){a, a});
}

/* Set packed 8-bit integers in the result with the supplied values in reverse order. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_setr_epi8(char e0, char e1, char e2, char e3, char e4, char e5, char e6, char e7)
{
  return (__m64i) ((__v8qi){e0, e1, e2, e3, e4, e5, e6, e7});
}

/* Set packed 16-bit integers in the result with the supplied values in reverse order. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_setr_epi16(short e0, short e1, short e2, short e3)
{
  return (__m64i) ((__v4hi){e0, e1, e2, e3});
}

/* Set packed 32-bit integers in the result with the supplied values in reverse order. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_setr_epi32(int e0, int e1)
{
  return (__m64i) ((__v2si){e0, e1});
}

/* Return vector of type __m64i with all elements set to zero. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_setzero_si64()
{
  return (__m64i) ((__v4hi){0, 0, 0, 0});
}

/* Extract an 8-bit integer from a, selected with i, and return the result. */
static __inline__ char __DEFAULT_FN_ATTRS _mm64_extract_epi8(__m64i a, int i)
{
  return (char) (((__v8qi)a)[i]);
}

/* Extract a 16-bit integer from a, selected with i, and return the result. */
static __inline__ short __DEFAULT_FN_ATTRS _mm64_extract_epi16(__m64i a, int i)
{
  return (short) (((__v4hi)a)[i]);
}

/* Extract a 32-bit integer from a, selected with i, and return the result. */
static __inline__ int __DEFAULT_FN_ATTRS _mm64_extract_epi32(__m64i a, int i)
{
  return (int) (((__v2si)a)[i]);
}

/* Return a new vector which is composed of 16-bit slices from the two input vectors. Each of
   the selectors determine the corresponding lane on in the input, with sel0 determining the
   low 16 bits and sel3 the highest 16 bits. */
#define _mm64_shuf_si64(A, B, M, N, O, P) \
  ((__m64i)(__builtin_shufflevector((__v4hi)(A), (__v4hi)(B), (M), (N), (O), (P))))

/* Shuffle 16-bit integers in A using the control in IMM8, and return the
   shuffled vector. Every 2 bits of IMM8 are used to control the output. */
#define _mm64_shuffle_epi16(A, IMM8) \
  ((__m64i) (__builtin_shufflevector((__v4hi)(A), (__v4hi) _mm64_set1_epi16(0), \
    (IMM8) & 0x3,          \
    ((IMM8) & 0xc) >> 2,   \
    ((IMM8) & 0x30) >> 4,  \
    ((IMM8) & 0xc0) >> 6)))

/* Return a new vector which is composed of 8-bit slices from the two input vectors.
   Each of the selectors determine the corresponding lane on in the input,
   with sel0 determining the low 8 bits and sel7 the highest 8 bits. */
#define _mm64_shufi8x8(A, B, M, N, O, P, Q, R, S, T)                     \
  ((__m64i)(__builtin_shufflevector((__v8qi)(A), (__v8qi)(B), (M), (N), (O), (P), \
    (Q), (R), (S), (T))))

/* Shuffle 8-bit integers in A using the control in IMM32, and return the
   shuffled vector. Every 3 bits of IMM32 are used to control the output. */
#define _mm64_shuffle_epi8(A, IMM32) \
  ((__m64i) (__builtin_shufflevector((__v8qi)(A), (__v8qi) _mm64_set1_epi8(0), \
    (IMM32)         & 0x7,  \
    ((IMM32) >> 3)  & 0x7,  \
    ((IMM32) >> 6)  & 0x7,  \
    ((IMM32) >> 9)  & 0x7,  \
    ((IMM32) >> 12) & 0x7,  \
    ((IMM32) >> 15) & 0x7,  \
    ((IMM32) >> 18) & 0x7,  \
    ((IMM32) >> 21) & 0x7)))

/* Blend packed 8-bit integers from a and b using the bits in mask. Each element is picked
   from a if its corresponding bit in the mask is 0, and from b if it is 1. */
static __inline__ __m64i  __DEFAULT_FN_ATTRS _mm64_blend_epi8(unsigned char mask, __m64i a, __m64i b) {
  return (__m64i)__builtin_csa_blend8x8(mask, (__v8qu)a, (__v8qu)b);
}

/* Blend packed 16-bit integers from a and b using the bits in mask. Each element is picked
   from a if its corresponding bit in the mask is 0, and from b if it is 1. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_blend_epi16(unsigned char mask, __m64i a, __m64i b)
{
  return (__m64i)__builtin_csa_blend16x4(mask, (__v4hi)a, (__v4hi)b);
}

/* Blend packed 32-bit floating point numbers from a and b using the bits in mask. Each element
   is picked from a if its corresponding bit in the mask is 0, and from b if it is 1. */
static __inline__ __m64f __DEFAULT_FN_ATTRS _mm64_blend_ps(unsigned char mask, __m64f a, __m64f b)
{
  return (__m64f)__builtin_csa_blend32x2(mask, a, b);
}

/*
 * Half-Precision Vector Intrinsics
 */

/* Return a vector where all floating-point values are set to a value of 0. */
static __inline__ __m64h __DEFAULT_FN_ATTRS _mm64_setzero_ph(void)
{
  return (__m64h){ 0.0f16, 0.0f16, 0.0f16, 0.0f16 };
}

/* Set packed 16-bit floating-point values in the result with the supplied values. */
static __inline__ __m64h __DEFAULT_FN_ATTRS _mm64_set_ph(_Float16 v3,
                                                         _Float16 v2,
                                                         _Float16 v1,
                                                         _Float16 v0)
{
  return (__m64h){v0, v1, v2, v3};
}

/* Broadcast 16-bit floating-point value a to all elements of the result. */
static __inline__ __m64h __DEFAULT_FN_ATTRS _mm64_set1_ph(_Float16 v)
{
  return (__m64h){v, v, v, v};
}

/* Set packed 16-bit floating-point values in the result with the supplied values in reverse order. */
static __inline__ __m64h __DEFAULT_FN_ATTRS _mm64_setr_ph(_Float16 v0,
                                                          _Float16 v1,
                                                          _Float16 v2,
                                                          _Float16 v3)
{
  return (__m64h){v0, v1, v2, v3};
}

/* Return the floating-point value in the corresponding lane of the vector. */
static __inline__ _Float16 __DEFAULT_FN_ATTRS _mm64_extract_ph(__m64h a, int i)
{
  return a[i];
}

/* Return a new vector which is composed of 16-bit slices from the two input vectors.
   Each of the selectors determines the corresponding lane on in the input. */
#define _mm64_shuf_ph(A, B, M, N, O, P)                                   \
  ((__m64h)(__builtin_shufflevector((__m64h)(A), (__m64h)(B), (M), (N), (O), (P))))

/* Shuffle 16-bit floating-point values in a using the control in imm8, and return the shuffled vector. */
#define _mm64_shuffle_ph(A, M) \
  ((__m64h)(__builtin_shufflevector((__m64h)(A), (__m64h)(A), (M) & 0x3, ((M) >> 2) & 0x3, ((M) >> 4) & 0x3, ((M) >> 6) & 0x3)))

/* Cast vector of type __m64f to type __m64h. */
static __inline__ __m64h __DEFAULT_FN_ATTRS _mm64_castps_ph(__m64f a)
{
  return (__m64h)(a);
}

/* Cast vector of type __m64h to type __m64f. */
static __inline__ __m64f __DEFAULT_FN_ATTRS _mm64_castph_ps(__m64h a)
{
  return (__m64f)(a);
}

/* Cast vector of type __m64i to type __m64h. */
static __inline__ __m64h __DEFAULT_FN_ATTRS _mm64_casti64_ph(__m64i a)
{
  return (__m64h)(a);
}

/* Cast vector of type __m64h to type __m64i. */
static __inline__ __m64i __DEFAULT_FN_ATTRS _mm64_castph_i64(__m64h a)
{
  return (__m64i)(a);
}

/* Perform a floating-point addition on the corresponding half-precision floating-point
   values in the vector. The swizzle and disable inputs modify the operation. */
#define _mm64_add_ph(OP1, OP2, D, SW1, SW2)                             \
  ((__m64h)(__builtin_csa_addf16x4 ((__m64h)(OP1), (__m64h)(OP2),       \
                                    (D), (SW1), (SW2))))

/* Perform a floating-point subtraction on the corresponding half-precision floating-point
   values in the vector. The swizzle and disable inputs modify the operation. */
#define _mm64_sub_ph(OP1, OP2, D, SW1, SW2)                             \
  ((__m64h)(__builtin_csa_subf16x4 ((__m64h)(OP1), (__m64h)(OP2),       \
                                    (D), (SW1), (SW2))))

/* Perform a floating-point multiplication on the corresponding half-precision floating-point
   values in the vector. The swizzle and disable inputs modify the operation. */
#define _mm64_mul_ph(OP1, OP2, D, SW1, SW2)                             \
  ((__m64h)(__builtin_csa_mulf16x4 ((__m64h)(OP1), (__m64h)(OP2),       \
                                    (D), (SW1), (SW2))))

/* Perform a floating-point addition on the odd lanes of the input vectors, and a floating-point
   subtraction of the even lanes of the input vectors. The swizzle and disable inputs modify the
   operation. */
#define _mm64_addsub_ph(OP1, OP2, D, SW1, SW2)                          \
  ((__m64h)(__builtin_csa_addsubf16x4 ((__m64h)(OP1), (__m64h)(OP2),    \
                                       (D), (SW1), (SW2))))

/* Perform a floating-point subtraction on the odd lanes of the input vectors, and a floating-point
   addition of the even lanes of the input vectors. The swizzle and disable inputs modify the
   operation as described above. */
#define _mm64_subadd_ph(OP1, OP2, D, SW1, SW2)                          \
  ((__m64h)(__builtin_csa_subaddf16x4 ((__m64h)(OP1), (__m64h)(OP2),    \
                                       (D), (SW1), (SW2))))

/* Perform the op1 * op2 + op3 floating-point operation (with no intermediate rounding) on the
   corresponding lanes of the input vectors. The swizzle and disable inputs modify the operation.
   There is no swizzle parameter available for op3. */
#define _mm64_fma_ph(OP1, OP2, OP3, D, SW1, SW2)	                         \
  ((__m64h)(__builtin_csa_fmaf16x4 ((__m64h)(OP1), (__m64h)(OP2), (__m64h)(OP3), \
                                    (D), (SW1), (SW2))))

/* Perform the op1 * op2 - op3 floating-point operation (with no intermediate rounding) on the
   corresponding lanes of the input vectors. The swizzle and disable inputs modify the operation.
   There is no swizzle parameter available for op3. */
#define _mm64_fms_ph(OP1, OP2, OP3, D, SW1, SW2)	                         \
  ((__m64h)(__builtin_csa_fmsf16x4 ((__m64h)(OP1), (__m64h)(OP2), (__m64h)(OP3), \
                                    (D), (SW1), (SW2))))

/* Perform the op3 - op1 * op2 floating-point operation (with no intermediate rounding) on the
   corresponding lanes of the input vectors. The swizzle and disable inputs modify the operation.
   There is no swizzle parameter available for op3. */
#define _mm64_fmrs_ph(OP1, OP2, OP3, D, SW1, SW2)	                          \
  ((__m64h)(__builtin_csa_fmrsf16x4 ((__m64h)(OP1), (__m64h)(OP2), (__m64h)(OP3), \
                                     (D), (SW1), (SW2))))

/* Perform the op1 * op2 + op3 floating-point operation (with no intermediate rounding) on the odd
   lanes and the op1 * op2 - op3 operation on the even lanes of the input vectors. The swizzle and
   disable inputs modify the operation. There is no swizzle parameter available for op3. */
#define _mm64_fmas_ph(OP1, OP2, OP3, D, SW1, SW2)	                          \
  ((__m64h)(__builtin_csa_fmasf16x4 ((__m64h)(OP1), (__m64h)(OP2), (__m64h)(OP3), \
                                     (D), (SW1), (SW2))))

/* Perform the op1 * op2 - op3 floating-point operation (with no intermediate rounding) on the odd
   lanes and the op1 * op2 + op3 operation on the even lanes of the input vectors. The swizzle and
   disable inputs modify the operation. There is no swizzle parameter available for op3. */
#define _mm64_fmsa_ph(OP1, OP2, OP3, D, SW1, SW2)	                          \
  ((__m64h)(__builtin_csa_fmsaf16x4 ((__m64h)(OP1), (__m64h)(OP2), (__m64h)(OP3), \
                                     (D), (SW1), (SW2))))

/*
 * New Half-Precision Vector Intrinsics
 */

/* Perform the -(op1 * op2) - op3 floating-point operation (with no intermediate rounding)
   on the corresponding lanes of the input vectors. The swizzle and disable inputs modify
   the operation. There is no swizzle parameter available for op3. */
#define _mm64_fnms_ph(OP1, OP2, OP3, D, SW1, SW2)	                          \
  ((__m64h)(__builtin_csa_fnmsf16x4 ((__m64h)(OP1), (__m64h)(OP2), (__m64h)(OP3), \
                                     (D), (SW1), (SW2))))

/* Compare the corresponding half-precision floating-point values and store the minimum,
   for each lane in the vector. The swizzle and disable inputs modify the operation. */
#define _mm64_min_ph(OP1, OP2, D, SW1, SW2)                             \
  ((__m64h)(__builtin_csa_min16x4 ((__m64h)(OP1), (__m64h)(OP2),        \
                                   (D), (SW1), (SW2))))

/* Compare the corresponding half-precision floating-point values and store the maximum,
   for each lane in the vector. The swizzle and disable inputs modify the operation. */
#define _mm64_max_ph(OP1, OP2, D, SW1, SW2)                             \
  ((__m64h)(__builtin_csa_max16x4 ((__m64h)(OP1), (__m64h)(OP2),        \
                                   (D), (SW1), (SW2))))

/*
 * New Single-Precision Vector Intrinsics
 */

/* Perform the -(op1 * op2) - op3 floating-point operation (with no intermediate rounding)
   on the corresponding lanes of the input vectors. The swizzle and disable inputs modify
   the operation. There is no swizzle parameter available for op3. */
#define _mm64_fnms_ps(OP1, OP2, OP3, D, SW1, SW2)                                 \
  ((__m64f)(__builtin_csa_fnmsf32x2 ((__m64f)(OP1), (__m64f)(OP2), (__m64f)(OP3), \
                                     (D), (SW1), (SW2))))

/* Compare the corresponding single-precision floating-point values and store the minimum,
   for each lane in the vector. The swizzle and disable inputs modify the operation. */
#define _mm64_min_ps(OP1, OP2, D, SW1, SW2)                             \
  ((__m64f)(__builtin_csa_min32x2 ((__m64f)(OP1), (__m64f)(OP2),        \
                                   (D), (SW1), (SW2))))

/* Compares the corresponding single-precision floating-point values and store the maximum,
   for each lane in the vector. The swizzle and disable inputs modify the operation. */
#define _mm64_max_ps(OP1, OP2, D, SW1, SW2)                             \
  ((__m64f)(__builtin_csa_max32x2 ((__m64f)(OP1), (__m64f)(OP2),        \
                                   (D), (SW1), (SW2))))

/* SIMD integer: we use a single intrinsic with comparison codes here. The
 * mapping is based on ISD::CondCode, see the comment above _MM_CMP_ENUM. */
static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmpeq_epi8(__m64i a, __m64i b)
{
  return __builtin_csa_cmp8x8((__v8qi)a, (__v8qi)b, _CMP_EQ_OQ | __CMP_N_BIT);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmpne_epi8(__m64i a, __m64i b)
{
  return __builtin_csa_cmp8x8((__v8qi)a, (__v8qi)b, _CMP_NEQ_OQ | __CMP_N_BIT);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmpgt_epi8(__m64i a, __m64i b)
{
  return __builtin_csa_cmp8x8((__v8qi)a, (__v8qi)b, _CMP_GT_OQ | __CMP_N_BIT);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmpge_epi8(__m64i a, __m64i b)
{
  return __builtin_csa_cmp8x8((__v8qi)a, (__v8qi)b, _CMP_GE_OQ | __CMP_N_BIT);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmplt_epi8(__m64i a, __m64i b)
{
  return __builtin_csa_cmp8x8((__v8qi)a, (__v8qi)b, _CMP_LT_OQ | __CMP_N_BIT);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmple_epi8(__m64i a, __m64i b)
{
  return __builtin_csa_cmp8x8((__v8qi)a, (__v8qi)b, _CMP_LE_OQ | __CMP_N_BIT);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmpgt_epu8(__m64i a, __m64i b)
{
  return __builtin_csa_cmp8x8((__v8qi)a, (__v8qi)b, _CMP_NLE_UQ);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmpge_epu8(__m64i a, __m64i b)
{
  return __builtin_csa_cmp8x8((__v8qi)a, (__v8qi)b, _CMP_NLT_UQ);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmplt_epu8(__m64i a, __m64i b)
{
  return __builtin_csa_cmp8x8((__v8qi)a, (__v8qi)b, _CMP_NGE_UQ);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmple_epu8(__m64i a, __m64i b)
{
  return __builtin_csa_cmp8x8((__v8qi)a, (__v8qi)b, _CMP_NGT_UQ);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmpeq_epi16(__m64i a, __m64i b)
{
  return __builtin_csa_cmp16x4((__v4hi)a, (__v4hi)b, _CMP_EQ_OQ | __CMP_N_BIT);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmpne_epi16(__m64i a, __m64i b)
{
  return __builtin_csa_cmp16x4((__v4hi)a, (__v4hi)b, _CMP_NEQ_OQ | __CMP_N_BIT);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmpgt_epi16(__m64i a, __m64i b)
{
  return __builtin_csa_cmp16x4((__v4hi)a, (__v4hi)b, _CMP_GT_OQ | __CMP_N_BIT);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmpge_epi16(__m64i a, __m64i b)
{
  return __builtin_csa_cmp16x4((__v4hi)a, (__v4hi)b, _CMP_GE_OQ | __CMP_N_BIT);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmplt_epi16(__m64i a, __m64i b)
{
  return __builtin_csa_cmp16x4((__v4hi)a, (__v4hi)b, _CMP_LT_OQ | __CMP_N_BIT);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmple_epi16(__m64i a, __m64i b)
{
  return __builtin_csa_cmp16x4((__v4hi)a, (__v4hi)b, _CMP_LE_OQ | __CMP_N_BIT);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmpgt_epu16(__m64i a, __m64i b)
{
  return __builtin_csa_cmp16x4((__v4hi)a, (__v4hi)b, _CMP_NLE_UQ);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmpge_epu16(__m64i a, __m64i b)
{
  return __builtin_csa_cmp16x4((__v4hi)a, (__v4hi)b, _CMP_NLT_UQ);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmplt_epu16(__m64i a, __m64i b)
{
  return __builtin_csa_cmp16x4((__v4hi)a, (__v4hi)b, _CMP_NGE_UQ);
}

static __inline__ int __DEFAULT_FN_ATTRS _mm64_cmple_epu16(__m64i a, __m64i b)
{
  return __builtin_csa_cmp16x4((__v4hi)a, (__v4hi)b, _CMP_NGT_UQ);
}

/* Floating point comparisons: the LLVM intrinsic breaks the comparison
 * parameter into three different parameters.
 */
#define _mm64_cmp_ph(A, B, CMP, D, S1, S2) \
  (__builtin_csa_cmpf16x4((__m64h)(A), (__m64h)(B), (CMP) & 0x7,  \
                          (D), (S1), (S2),                        \
                          ((CMP) & __CMP_U_BIT) == 0,             \
                          ((CMP) & __CMP_S_BIT) != 0))

#define _mm64_cmp_ps(A, B, CMP, D, S1, S2) \
  (__builtin_csa_cmpf32x2((__m64f)(A), (__m64f)(B), (CMP) & 0x7, \
                          (D), (S1), (S2),                       \
                          ((CMP) & __CMP_U_BIT) == 0,            \
                          ((CMP) & __CMP_S_BIT) != 0))

#define _mm64_min_ph(A, B, D, S1, S2) \
  ((__m64h)(__builtin_csa_minf16x4((__m64h)(A), (__m64h)(B), (D), (S1), (S2), 0, 0)))

#define _mm64_max_ph(A, B, D, S1, S2) \
  ((__m64h)(__builtin_csa_maxf16x4((__m64h)(A), (__m64h)(B), (D), (S1), (S2), 0, 0)))

#define _mm64_min_ps(A, B, D, S1, S2) \
  ((__m64f)(__builtin_csa_minf32x2((__m64f)(A), (__m64f)(B), (D), (S1), (S2), 0, 0)))

#define _mm64_max_ps(A, B, D, S1, S2) \
  ((__m64f)(__builtin_csa_maxf32x2((__m64f)(A), (__m64f)(B), (D), (S1), (S2), 0, 0)))

/*
 * New Scalar Intrinsics
 */

/* Perform a carry-less multiplication of two 8-bit integers a and b, and return the 16-bit result. */
static __inline__ unsigned short _clxmul8_16(unsigned char a, unsigned char b)
{
  return (unsigned short) __builtin_csa_clxmul8(a, b);
}

/* Perform a carry-less multiplication of two 16-bit integers a and b, and return the 32-bit result. */
static __inline__ unsigned int _clxmul16_32(unsigned short a, unsigned short b)
{
  return (unsigned int) __builtin_csa_clxmul16(a, b);
}

/* Perform a carry-less multiplication of two 32-bit integers a and b, and return the 64-bit result. */
static __inline__ unsigned __int64 _clxmul32_64(unsigned int a, unsigned int b)
{
  return (unsigned __int64) __builtin_csa_clxmul32(a, b);
}

/* Compute the approximate reciprocal of the single-precision floating-point value. The maximum
   relative error for this approximation is less than 2^-14. */
static __inline__ float _rcp14_ss(float a)
{
  return (float) __builtin_csa_rcp14f32(a);
}

/* Compute the approximate reciprocal of the double-precision floating-point value. The maximum
   relative error for this approximation is less than 2^-14. */
static __inline__ double _rcp14_sd(double a)
{
  return (double) __builtin_csa_rcp14f64(a);
}

/* Compute the approximate reciprocal square root of the single-precision floating-point value.
   The maximum relative error for this approximation is less than 2^-14. */
static __inline__ float _rsqrt14_ss(float a)
{
  return (float) __builtin_csa_rsqrt14f32(a);
}

/* Compute the approximate reciprocal square root of the double-precision floating-point value.
   The maximum relative error for this approximation is less than 2^-14. */
static __inline__ double _rsqrt14_sd(double a)
{
  return (double) __builtin_csa_rsqrt14f64(a);
}

/* Convert the half-precision floating-point value to a single-precision floating point value. */
static __inline__ float _cvtsh_ss(unsigned short a)
{
  return (float)a;
}

/* Convert the single-precision (32-bit) floating-point value a to a half-precision (16-bit)
   floating-point value, and store the result in dst. Rounding is done according to the rounding
   parameter. However, rounding is currently ignored. */
static __inline__ unsigned short _cvtss_sh(float a, const int rounding)
{
  return (unsigned short)a;
}

/* Bitwise ternary logic that provides the capability to implement any three-operand binary function;
   the specific binary function is specified by the value in imm8. For each bit, the corresponding bits
   from a, b, and c are used to form a 3-bit index into imm8, and the value at that bit is written
   to the corresponding bit in the output. */
#define _ternlog_u1(A, B, C, IMM8) \
  ((unsigned char)(__builtin_csa_ternlog_u1((unsigned char)(A), \
                                            (unsigned char)(B), \
                                            (unsigned char)(C), \
                                            (IMM8))))

/* Bitwise ternary logic that provides the capability to implement any three-operand binary function;
   the specific binary function is specified by the value in imm8. For each bit, the corresponding bits
   from a, b, and c are used to form a 3-bit index into imm8, and the value at that bit is written to
   the corresponding bit in the output. */
#define _ternlog_u8(A, B, C, IMM8) \
  ((unsigned char)(__builtin_csa_ternlog_u8((unsigned char)(A), \
                                            (unsigned char)(B), \
                                            (unsigned char)(C), \
                                            (IMM8))))

/* Bitwise ternary logic that provides the capability to implement any three-operand binary function;
   the specific binary function is specified by the value in imm8. For each bit, the corresponding bits
   from a, b, and c are used to form a 3-bit index into imm8, and the value at that bit is written to
   the corresponding bit in the output. */
#define _ternlog_u16(A, B, C, IMM8) \
  ((unsigned short)(__builtin_csa_ternlog_u16((unsigned short)(A), \
                                              (unsigned short)(B), \
                                              (unsigned short)(C), \
                                              (IMM8))))

/* Bitwise ternary logic that provides the capability to implement any three-operand binary function;
   the specific binary function is specified by the value in imm8. For each bit, the corresponding bits
   from a, b, and c are used to form a 3-bit index into imm8, and the value at that bit is written to
   the corresponding bit in the output. */
#define _ternlog_u32(A, B, C, IMM8) \
  ((unsigned int)(__builtin_csa_ternlog_u32((unsigned int)(A), \
                                            (unsigned int)(B), \
                                            (unsigned int)(C), \
                                            (IMM8))))

/* Bitwise ternary logic that provides the capability to implement any three-operand binary function;
   the specific binary function is specified by the value in imm8. For each bit, the corresponding bits
   from a, b, and c are used to form a 3-bit index into imm8, and the value at that bit is written to
   the corresponding bit in the output. */
#define _ternlog_u64(A, B, C, IMM8) \
  ((unsigned __int64)(__builtin_csa_ternlog_u64((unsigned __int64)(A), \
                                                (unsigned __int64)(B), \
                                                (unsigned __int64)(C), \
                                                (IMM8))))

#endif /* __CSA__ */

#undef __DEFAULT_FN_ATTRS

#endif /* __CSAINTRIN_H */

