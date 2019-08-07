/*===---- csaintrin.h - CSA intrinsics -------------------------------------===
 *
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

/* Define the types for vector types on CSA */
typedef float __m64f __attribute__((__vector_size__(8)));

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
  (__m64f)(__builtin_csa_addf32x2((__m64f)(A), (__m64f)(B), \
                                   (D), (M), (N)))

#define _mm64_sub_ps(A, B, D, M, N) \
  (__m64f)(__builtin_csa_subf32x2((__m64f)(A), (__m64f)(B), \
                                   (D), (M), (N)))

#define _mm64_addsub_ps(A, B, D, M, N) \
  (__m64f)(__builtin_csa_addsubf32x2((__m64f)(A), (__m64f)(B), \
                                      (D), (M), (N)))

#define _mm64_subadd_ps(A, B, D, M, N) \
  (__m64f)(__builtin_csa_subaddf32x2((__m64f)(A), (__m64f)(B), \
                                      (D), (M), (N)))

#define _mm64_mul_ps(A, B, D, M, N) \
  (__m64f)(__builtin_csa_mulf32x2((__m64f)(A), (__m64f)(B), \
                                   (D), (M), (N)))

#define _mm64_fma_ps(A, B, C, D, M, N) \
  (__m64f)(__builtin_csa_fmaf32x2((__m64f)(A), (__m64f)(B), (__m64f)(C), \
                                   (D), (M), (N)))

#define _mm64_fms_ps(A, B, C, D, M, N) \
  (__m64f)(__builtin_csa_fmsf32x2((__m64f)(A), (__m64f)(B), (__m64f)(C), \
                                   (D), (M), (N)))

#define _mm64_fmrs_ps(A, B, C, D, M, N) \
  (__m64f)(__builtin_csa_fmrsf32x2((__m64f)(A), (__m64f)(B), (__m64f)(C), \
                                    (D), (M), (N)))

#define _mm64_fmas_ps(A, B, C, D, M, N) \
  (__m64f)(__builtin_csa_fmasf32x2((__m64f)(A), (__m64f)(B), (__m64f)(C), \
                                    (D), (M), (N)))

#define _mm64_fmsa_ps(A, B, C, D, M, N) \
  (__m64f)(__builtin_csa_fmsaf32x2((__m64f)(A), (__m64f)(B), (__m64f)(C), \
                                    (D), (M), (N)))
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
#endif

#define _mm64_shuf_ps(A, B, M, N) \
  (__m64f)(__builtin_shufflevector((__m64f)(A), (__m64f)(B), (M), (N)))

#undef __DEFAULT_FN_ATTRS

#endif /* __CSAINTRIN_H */
