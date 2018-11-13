/* INTEL_FEATURE_CSA */
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
typedef float __f32x2 __attribute__((__vector_size__(8)));

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
static __inline__ __f32x2 __DEFAULT_FN_ATTRS _mm_packf32x2(float v0, float v1)
{
  return (__f32x2){v0, v1};
}

static __inline__ __f32x2 __DEFAULT_FN_ATTRS _mm_setzerof32x2(void)
{
  return (__f32x2){ 0.0f, 0.0f };
}

static __inline__ float __DEFAULT_FN_ATTRS _mm_extractf32x2(__f32x2 a, int i)
{
  return a[i];
}

#define _mm_addf32x2(A, B, D, M, N) \
  (__f32x2)(__builtin_csa_addf32x2((__f32x2)(A), (__f32x2)(B), \
                                   (D), (M), (N)))

#define _mm_subf32x2(A, B, D, M, N) \
  (__f32x2)(__builtin_csa_subf32x2((__f32x2)(A), (__f32x2)(B), \
                                   (D), (M), (N)))

#define _mm_addsubf32x2(A, B, D, M, N) \
  (__f32x2)(__builtin_csa_addsubf32x2((__f32x2)(A), (__f32x2)(B), \
                                      (D), (M), (N)))

#define _mm_subaddf32x2(A, B, D, M, N) \
  (__f32x2)(__builtin_csa_subaddf32x2((__f32x2)(A), (__f32x2)(B), \
                                      (D), (M), (N)))

#define _mm_mulf32x2(A, B, D, M, N) \
  (__f32x2)(__builtin_csa_mulf32x2((__f32x2)(A), (__f32x2)(B), \
                                   (D), (M), (N)))

#define _mm_fmaf32x2(A, B, C, D, M, N) \
  (__f32x2)(__builtin_csa_fmaf32x2((__f32x2)(A), (__f32x2)(B), (__f32x2)(C), \
                                   (D), (M), (N)))

#define _mm_fmsf32x2(A, B, C, D, M, N) \
  (__f32x2)(__builtin_csa_fmsf32x2((__f32x2)(A), (__f32x2)(B), (__f32x2)(C), \
                                   (D), (M), (N)))

#define _mm_fmrsf32x2(A, B, C, D, M, N) \
  (__f32x2)(__builtin_csa_fmrsf32x2((__f32x2)(A), (__f32x2)(B), (__f32x2)(C), \
                                    (D), (M), (N)))

#define _mm_fmasf32x2(A, B, C, D, M, N) \
  (__f32x2)(__builtin_csa_fmasf32x2((__f32x2)(A), (__f32x2)(B), (__f32x2)(C), \
                                    (D), (M), (N)))

#define _mm_fmsaf32x2(A, B, C, D, M, N) \
  (__f32x2)(__builtin_csa_fmsaf32x2((__f32x2)(A), (__f32x2)(B), (__f32x2)(C), \
                                    (D), (M), (N)))

#define _mm_shuff32x2(A, B, M, N) \
  (__f32x2)(__builtin_shufflevector((A), (B), (M), (N)))

#undef __DEFAULT_FN_ATTRS

#endif /* __CSAINTRIN_H */
/* end INTEL_FEATURE_CSA */
