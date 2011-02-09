/*===---- smmintrin.h - SSE4 intrinsics -----------------------------------===
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
 
#ifndef __SMMINTRIN_H
#define __SMMINTRIN_H

#ifndef __SSE4_1__
#error "SSE4.1 instruction set not enabled"
#else

#include <tmmintrin.h>

/* SSE4.1 */

/* Rounding mode macros. */
#define _MM_FROUND_TO_NEAREST_INT	0x00
#define _MM_FROUND_TO_NEG_INF		0x01
#define _MM_FROUND_TO_POS_INF		0x02
#define _MM_FROUND_TO_ZERO			0x03
#define _MM_FROUND_CUR_DIRECTION	0x04

#define _MM_FROUND_RAISE_EXC		0x00
#define _MM_FROUND_NO_EXC			0x08

#define _MM_FROUND_NINT			(_MM_FROUND_TO_NEAREST_INT | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_FLOOR		(_MM_FROUND_TO_NEG_INF | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_CEIL			(_MM_FROUND_TO_POS_INF | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_TRUNC		(_MM_FROUND_TO_ZERO | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_RINT			(_MM_FROUND_CUR_DIRECTION | _MM_FROUND_RAISE_EXC)
#define _MM_FROUND_NEARBYINT	(_MM_FROUND_CUR_DIRECTION | _MM_FROUND_NO_EXC)


static inline __m128i __attribute__((__always_inline__))
_mm_blend_epi16 (__m128i a, __m128i b, const int mask)
{
  return (__m128i) __builtin_ia32_pblendw128 ((__v8hi)a, (__v8hi)b, mask);
}

static inline __m128i __attribute__((__always_inline__))
_mm_blendv_epi8 (__m128i a, __m128i b, __m128i mask)
{
  return (__m128i) __builtin_ia32_pblendvb128 ((__v16qi)a, (__v16qi)b, (__v16qi)mask);
}

static inline __m128 __attribute__((__always_inline__))
_mm_blend_ps (__m128 a, __m128 b, const int mask)
{
  return (__m128) __builtin_ia32_blendps ((__v4sf)a, (__v4sf)b, mask);
}

static inline __m128 __attribute__((__always_inline__))
_mm_blendv_ps (__m128 a, __m128 b, __m128 mask)
{
  return (__m128) __builtin_ia32_blendvps ((__v4sf)a, (__v4sf)b, (__v4sf)mask);
}

static inline __m128d __attribute__((__always_inline__))
_mm_blend_pd (__m128d a, __m128d b, const int mask)
{
  return (__m128d) __builtin_ia32_blendpd ((__v2df)a, (__v2df)b, mask);
}

static inline __m128d __attribute__((__always_inline__))
_mm_blendv_pd (__m128d a, __m128d b, __m128d mask)
{
  return (__m128d) __builtin_ia32_blendvpd ((__v2df)a, (__v2df)b, (__v2df)mask);
}

static inline __m128 __attribute__((__always_inline__))
_mm_dp_ps (__m128 a, __m128 b, const int mask)
{
  return (__m128) __builtin_ia32_dpps ((__v4sf)a, (__v4sf)b, mask);
}

static inline __m128d __attribute__((__always_inline__))
_mm_dp_pd (__m128d a, __m128d b, const int mask)
{
  return (__m128d) __builtin_ia32_dppd ((__v2df)a, (__v2df)b, mask);
}

static inline __m128i __attribute__((__always_inline__))
_mm_cmpeq_epi64 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_pcmpeqq ((__v2di)a, (__v2di)b);
}

static inline __m128i __attribute__((__always_inline__))
_mm_min_epi8 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_pminsb128 ((__v16qi)a, (__v16qi)b);
}

static inline __m128i __attribute__((__always_inline__))
_mm_max_epi8 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_pmaxsb128 ((__v16qi)a, (__v16qi)b);
}

static inline __m128i __attribute__((__always_inline__))
_mm_min_epu16 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_pminuw128 ((__v8hi)a, (__v8hi)b);
}

static inline __m128i __attribute__((__always_inline__))
_mm_max_epu16 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_pmaxuw128 ((__v8hi)a, (__v8hi)b);
}

static inline __m128i __attribute__((__always_inline__))
_mm_min_epi32 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_pminsd128 ((__v4si)a, (__v4si)b);
}

static inline __m128i __attribute__((__always_inline__))
_mm_max_epi32 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_pmaxsd128 ((__v4si)a, (__v4si)b);
}

static inline __m128i __attribute__((__always_inline__))
_mm_min_epu32 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_pminud128 ((__v4si)a, (__v4si)b);
}

static inline __m128i __attribute__((__always_inline__))
_mm_max_epu32 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_pmaxud128 ((__v4si)a, (__v4si)b);
}

static inline __m128i __attribute__((__always_inline__))
_mm_mullo_epi32 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_pmulld128 ((__v4si)a, (__v4si)b);
}

static inline __m128i __attribute__((__always_inline__))
_mm_mul_epi32 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_pmuldq128 ((__v4si)a, (__v4si)b);
}

static inline int __attribute__((__always_inline__))
_mm_testz_si128 (__m128i mask, __m128i value)
{
  return __builtin_ia32_ptestz128 ((__v4sf)mask, (__v4sf)value);
}

static inline int __attribute__((__always_inline__))
_mm_testc_si128 (__m128i mask, __m128i value)
{
  return __builtin_ia32_ptestc128 ((__v4sf)mask, (__v4sf)value);
}

static inline int __attribute__((__always_inline__))
_mm_testnzc_si128 (__m128i mask, __m128i value)
{
  return __builtin_ia32_ptestnzc128 ((__v4sf)mask, (__v4sf)value);
}

#define _mm_test_all_zeros(mask, val)      _mm_testz_si128((mask), (val))

#define _mm_test_all_ones(val) \
	_mm_testc_si128((val), _mm_cmpeq_epi32((val),(val)))

#define _mm_test_mix_ones_zeros(mask, val) _mm_testnzc_si128((mask), (val))

static inline __m128 __attribute__((__always_inline__))
_mm_insert_ps (__m128 dst, __m128 value, const int n)
{
  return (__m128) __builtin_ia32_insertps128 ((__v4sf)dst,
					      (__v4sf)value,
					      n);
}

/*
 * Helper macro to create ndx-parameter value for _mm_insert_ps
 */
#define _MM_MK_INSERTPS_NDX(srcField, dstField, zeroMask) \
        (((srcField)<<6) | ((dstField)<<4) | (zeroMask))

/* Extract binary representation of single precision float from packed
   single precision array element of X selected by index N.  */

static inline int __attribute__((__always_inline__))
_mm_extract_ps (__m128 a, const int n)
{
  union { int i; float f; } __tmp;
  __tmp.f = __builtin_ia32_vec_ext_v4sf ((__v4sf)a, n);
  return __tmp.i;
}

/*
 * Extract single precision float from packed single precision 
 * array element selected by index into dest
 */
#define _MM_EXTRACT_FLOAT(dest, src, ndx) \
        *((int*)&(dest)) = _mm_extract_ps((src), (ndx))
  
/*
 * Extract specified single precision float element
 * into the lower part of __m128
 */
#define _MM_PICK_OUT_PS(src, num) \
        _mm_insert_ps(_mm_setzero_ps(), (src), \
                      _MM_MK_INSERTPS_NDX((num), 0, 0x0e));

/* Insert integer, S, into packed integer array element of D
   selected by index N.  */

static inline __m128i __attribute__((__always_inline__))
_mm_insert_epi8 (__m128i dst, int value, const int n)
{
  return (__m128i) __builtin_ia32_vec_set_v16qi ((__v16qi)dst,
						 value, n);
}

static inline __m128i __attribute__((__always_inline__))
_mm_insert_epi32 (__m128i dst, int value, const int n)
{
  return (__m128i) __builtin_ia32_vec_set_v4si ((__v4si)dst,
						 value, n);
}

//#ifdef __x86_64__
//static inline __m128i __attribute__((__always_inline__))
//_mm_insert_epi64 (__m128i dst, long value, const int n)
//{
//  return (__m128i) __builtin_ia32_vec_set_v2di ((__v2di)dst,
//						 value, n);
//}
//#endif

static inline int __attribute__((__always_inline__))
_mm_extract_epi8 (__m128i a, const int n)
{
   return __builtin_ia32_vec_ext_v16qi ((__v16qi)a, n);
}

static inline int __attribute__((__always_inline__))
_mm_extract_epi32 (__m128i a, const int n)
{
   return __builtin_ia32_vec_ext_v4si ((__v4si)a, n);
}

//#ifdef __x86_64__
//static inline long  __attribute__((__always_inline__))
//_mm_extract_epi64 (__m128i a, const int n)
//{
//  return __builtin_ia32_vec_ext_v2di ((__v2di)a, n);
//}
//#endif

static inline __m128i __attribute__((__always_inline__))
_mm_minpos_epu16 (__m128i a)
{
  return (__m128i) __builtin_ia32_phminposuw128 ((__v8hi)a);
}

static inline __m128d __attribute__((__always_inline__))
_mm_round_pd (__m128d value, const int mask)
{
  return (__m128d) __builtin_ia32_roundpd ((__v2df)value, mask);
}

static inline __m128d __attribute__((__always_inline__))
_mm_round_sd(__m128d dst, __m128d value, const int mask)
{
  return (__m128d) __builtin_ia32_roundsd ((__v2df)dst,
					   (__v2df)value,
					   mask);
}

static inline __m128 __attribute__((__always_inline__))
_mm_round_ps (__m128 value, const int mask)
{
  return (__m128) __builtin_ia32_roundps ((__v4sf)value, mask);
}

static inline __m128 __attribute__((__always_inline__))
_mm_round_ss (__m128 dst, __m128 value, const int mask)
{
  return (__m128) __builtin_ia32_roundss ((__v4sf)dst,
					  (__v4sf)value,
					  mask);
}

/*
 * MACRO functions for ceil/floor intrinsics
 */

#define _mm_ceil_pd(val)       _mm_round_pd((val), _MM_FROUND_CEIL);      
#define _mm_ceil_sd(dst, val)  _mm_round_sd((dst), (val), _MM_FROUND_CEIL);

#define _mm_floor_pd(val)      _mm_round_pd((val), _MM_FROUND_FLOOR);     
#define _mm_floor_sd(dst, val) _mm_round_sd((dst), (val), _MM_FROUND_FLOOR);

#define _mm_ceil_ps(val)       _mm_round_ps((val), _MM_FROUND_CEIL);     
#define _mm_ceil_ss(dst, val)  _mm_round_ss((dst), (val), _MM_FROUND_CEIL);
                                                                            
#define _mm_floor_ps(val)      _mm_round_ps((val), _MM_FROUND_FLOOR);     
#define _mm_floor_ss(dst, val) _mm_round_ss((dst), (val), _MM_FROUND_FLOOR);

static inline __m128i __attribute__((__always_inline__))
_mm_cvtepi8_epi32 (__m128i a)
{
  return (__m128i) __builtin_ia32_pmovsxbd128 ((__v16qi)a);
}

static inline __m128i __attribute__((__always_inline__))
_mm_cvtepi16_epi32 (__m128i a)
{
  return (__m128i) __builtin_ia32_pmovsxwd128 ((__v8hi)a);
}

static inline __m128i __attribute__((__always_inline__))
_mm_cvtepi8_epi64 (__m128i a)
{
  return (__m128i) __builtin_ia32_pmovsxbq128 ((__v16qi)a);
}

static inline __m128i __attribute__((__always_inline__))
_mm_cvtepi32_epi64 (__m128i a)
{
  return (__m128i) __builtin_ia32_pmovsxdq128 ((__v4si)a);
}

static inline __m128i __attribute__((__always_inline__))
_mm_cvtepi16_epi64 (__m128i a)
{
  return (__m128i) __builtin_ia32_pmovsxwq128 ((__v8hi)a);
}

static inline __m128i __attribute__((__always_inline__))
_mm_cvtepi8_epi16 (__m128i a)
{
  return (__m128i) __builtin_ia32_pmovsxbw128 ((__v16qi)a);
}

/* Packed integer zero-extension. */

static inline __m128i __attribute__((__always_inline__))
_mm_cvtepu8_epi32 (__m128i a)
{
  return (__m128i) __builtin_ia32_pmovzxbd128 ((__v16qi)a);
}

static inline __m128i __attribute__((__always_inline__))
_mm_cvtepu16_epi32 (__m128i a)
{
  return (__m128i) __builtin_ia32_pmovzxwd128 ((__v8hi)a);
}

static inline __m128i __attribute__((__always_inline__))
_mm_cvtepu8_epi64 (__m128i a)
{
  return (__m128i) __builtin_ia32_pmovzxbq128 ((__v16qi)a);
}

static inline __m128i __attribute__((__always_inline__))
_mm_cvtepu32_epi64 (__m128i a)
{
  return (__m128i) __builtin_ia32_pmovzxdq128 ((__v4si)a);
}

static inline __m128i __attribute__((__always_inline__))
_mm_cvtepu16_epi64 (__m128i a)
{
  return (__m128i) __builtin_ia32_pmovzxwq128 ((__v8hi)a);
}

static inline __m128i __attribute__((__always_inline__))
_mm_cvtepu8_epi16 (__m128i a)
{
  return (__m128i) __builtin_ia32_pmovzxbw128 ((__v16qi)a);
}

/* Pack 8 double words from 2 operands into 8 words of result with
   unsigned saturation. */
static inline __m128i __attribute__((__always_inline__))
_mm_packus_epi32 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_packusdw128 ((__v4si)a, (__v4si)b);
}

/* Sum absolute 8-bit integer difference of adjacent groups of 4
   byte integers in the first 2 operands.  Starting offsets within
   operands are determined by the 3rd mask operand.  */

static inline __m128i __attribute__((__always_inline__))
_mm_mpsadbw_epu8 (__m128i a, __m128i b, const int mask)
{
  return (__m128i) __builtin_ia32_mpsadbw128 ((__v16qi)a,
					      (__v16qi)b, mask);
}

/* Load double quadword using non-temporal aligned hint.  */
static inline __m128i __attribute__((__always_inline__))
_mm_stream_load_si128 (__m128i *a)
{
  return (__m128i) __builtin_ia32_movntdqa ((__v2di *) a);
}

#ifdef __SSE4_2__

/* These macros specify the source data format.  */
#define SIDD_UBYTE_OPS				0x00
#define SIDD_UWORD_OPS				0x01
#define SIDD_SBYTE_OPS				0x02
#define SIDD_SWORD_OPS				0x03

/* These macros specify the comparison operation.  */
#define SIDD_CMP_EQUAL_ANY			0x00
#define SIDD_CMP_RANGES				0x04
#define SIDD_CMP_EQUAL_EACH			0x08
#define SIDD_CMP_EQUAL_ORDERED		0x0c

/* These macros specify the the polarity.  */
#define SIDD_POSITIVE_POLARITY		0x00
#define SIDD_NEGATIVE_POLARITY		0x10
#define SIDD_MASKED_POSITIVE_POLARITY	0x20
#define SIDD_MASKED_NEGATIVE_POLARITY	0x30

/* These macros specify the output selection in _mm_cmpXstri ().  */
#define SIDD_LEAST_SIGNIFICANT		0x00
#define SIDD_MOST_SIGNIFICANT		0x40

/* These macros specify the output selection in _mm_cmpXstrm ().  */
#define SIDD_BIT_MASK			0x00
#define SIDD_UNIT_MASK			0x40

static inline __m128i __attribute__((__always_inline__))
_mm_cmpistrm (__m128i a, __m128i b, const int mask)
{
  return (__m128i) __builtin_ia32_pcmpistrm128 ((__v16qi)a, (__v16qi)b, mask);
}

static inline int __attribute__((__always_inline__))
_mm_cmpistri (__m128i a, __m128i b, const int mask)
{
  return __builtin_ia32_pcmpistri128 ((__v16qi)a, (__v16qi)b, mask);
}

static inline __m128i __attribute__((__always_inline__))
_mm_cmpestrm (__m128i a, int x, __m128i b, int y, const int mask)
{
  return (__m128i) __builtin_ia32_pcmpestrm128 ((__v16qi)a, x, (__v16qi)b, y, mask);
}

static inline int __attribute__((__always_inline__))
_mm_cmpestri (__m128i a, int x, __m128i b, int y, const int mask)
{
  return __builtin_ia32_pcmpestri128 ((__v16qi)a, x, (__v16qi)b, y, mask);
}

static inline int __attribute__((__always_inline__))
_mm_cmpistra (__m128i a, __m128i b, const int mask)
{
  return __builtin_ia32_pcmpistria128 ((__v16qi)a, (__v16qi)b, mask);
}

static inline int __attribute__((__always_inline__))
_mm_cmpistrc (__m128i a, __m128i b, const int mask)
{
  return __builtin_ia32_pcmpistric128 ((__v16qi)a, (__v16qi)b, mask);
}

static inline int __attribute__((__always_inline__))
_mm_cmpistro (__m128i a, __m128i b, const int mask)
{
  return __builtin_ia32_pcmpistrio128 ((__v16qi)a, (__v16qi)b, mask);
}

static inline int __attribute__((__always_inline__))
_mm_cmpistrs (__m128i a, __m128i b, const int mask)
{
  return __builtin_ia32_pcmpistris128 ((__v16qi)a, (__v16qi)b, mask);
}

static inline int __attribute__((__always_inline__))
_mm_cmpistrz (__m128i a, __m128i b, const int mask)
{
  return __builtin_ia32_pcmpistriz128 ((__v16qi)a, (__v16qi)b, mask);
}

static inline int __attribute__((__always_inline__))
_mm_cmpestra (__m128i a, int x, __m128i b, int y, const int mask)
{
  return __builtin_ia32_pcmpestria128 ((__v16qi)a, x, (__v16qi)b, y, mask);
}

static inline int __attribute__((__always_inline__))
_mm_cmpestrc (__m128i a, int x, __m128i b, int y, const int mask)
{
  return __builtin_ia32_pcmpestric128 ((__v16qi)a, x, (__v16qi)b, y, mask);
}

static inline int __attribute__((__always_inline__))
_mm_cmpestro (__m128i a, int x, __m128i b, int y, const int mask)
{
  return __builtin_ia32_pcmpestrio128 ((__v16qi)a, x, (__v16qi)b, y, mask);
}

static inline int __attribute__((__always_inline__))
_mm_cmpestrs (__m128i a, int x, __m128i b, int y, const int mask)
{
  return __builtin_ia32_pcmpestris128 ((__v16qi)a, x, (__v16qi)b, y, mask);
}

static inline int __attribute__((__always_inline__))
_mm_cmpestrz (__m128i a, int x, __m128i b, int y, const int mask)
{
  return __builtin_ia32_pcmpestriz128 ((__v16qi)a, x, (__v16qi)b, y, mask);
}
static inline __m128i __attribute__((__always_inline__))
_mm_cmpgt_epi64 (__m128i a, __m128i b)
{
  return (__m128i) __builtin_ia32_pcmpgtq ((__v2di)a, (__v2di)b);
}

static inline int __attribute__((__always_inline__))
_mm_popcnt_u32 (unsigned int a)
{
  return __builtin_popcount (a);
}

#ifdef __x86_64__
static inline long  __attribute__((__always_inline__))
_mm_popcnt_u64 (unsigned long a)
{
  return __builtin_popcountll (a);
}
#endif

static inline unsigned int __attribute__((__always_inline__))
_mm_crc32_u8 (unsigned int c, unsigned char value)
{
  return __builtin_ia32_crc32qi (c, value);
}

static inline unsigned int __attribute__((__always_inline__))
_mm_crc32_u16 (unsigned int c, unsigned short value)
{
  return __builtin_ia32_crc32hi (c, value);
}

static inline unsigned int __attribute__((__always_inline__))
_mm_crc32_u32 (unsigned int c, unsigned int value)
{
  return __builtin_ia32_crc32si (c, value);
}

//#ifdef __x86_64__
//static inline unsigned long __attribute__((__always_inline__))
//_mm_crc32_u64 (unsigned long c, unsigned long value)
//{
//  return __builtin_ia32_crc32di (c, value);
//}
//#endif

#endif /* __SSE4_2__ */

#endif /* __SSE4_1__ */

#endif /* SMMINTRIN_H */
