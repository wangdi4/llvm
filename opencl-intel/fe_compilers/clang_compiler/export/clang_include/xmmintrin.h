/*===---- xmmintrin.h - SSE intrinsics -------------------------------------===
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
 
#ifndef __XMMINTRIN_H
#define __XMMINTRIN_H
 
#ifndef __SSE__
#error "SSE instruction set not enabled"
#else

#include <mmintrin.h>

typedef int __v4si __attribute__((__vector_size__(16)));
typedef float __v4sf __attribute__((__vector_size__(16)));
typedef float __m128 __attribute__((__vector_size__(16)));

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_add_ss(__m128 a, __m128 b)
{
  a[0] += b[0];
  return a;
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_add_ps(__m128 a, __m128 b)
{
  return a + b;
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_sub_ss(__m128 a, __m128 b)
{
  a[0] -= b[0];
  return a;
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_sub_ps(__m128 a, __m128 b)
{
  return a - b;
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_mul_ss(__m128 a, __m128 b)
{
  a[0] *= b[0];
  return a;
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_mul_ps(__m128 a, __m128 b)
{
  return a * b;
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_div_ss(__m128 a, __m128 b)
{
  a[0] /= b[0];
  return a;
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_div_ps(__m128 a, __m128 b)
{
  return a / b;
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_sqrt_ss(__m128 a)
{
  return __builtin_ia32_sqrtss(a);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_sqrt_ps(__m128 a)
{
  return __builtin_ia32_sqrtps(a);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rcp_ss(__m128 a)
{
  return __builtin_ia32_rcpss(a);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rcp_ps(__m128 a)
{
  return __builtin_ia32_rcpps(a);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rsqrt_ss(__m128 a)
{
  return __builtin_ia32_rsqrtss(a);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_rsqrt_ps(__m128 a)
{
  return __builtin_ia32_rsqrtps(a);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_min_ss(__m128 a, __m128 b)
{
  return __builtin_ia32_minss(a, b);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_min_ps(__m128 a, __m128 b)
{
  return __builtin_ia32_minps(a, b);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_max_ss(__m128 a, __m128 b)
{
  return __builtin_ia32_maxss(a, b);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_max_ps(__m128 a, __m128 b)
{
  return __builtin_ia32_maxps(a, b);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_and_ps(__m128 a, __m128 b)
{
  return (__m128)((__v4si)a & (__v4si)b);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_andnot_ps(__m128 a, __m128 b)
{
  return (__m128)(~(__v4si)a & (__v4si)b);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_or_ps(__m128 a, __m128 b)
{
  return (__m128)((__v4si)a | (__v4si)b);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_xor_ps(__m128 a, __m128 b)
{
  return (__m128)((__v4si)a ^ (__v4si)b);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_ss(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpss(a, b, 0);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpeq_ps(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpps(a, b, 0);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_ss(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpss(a, b, 1);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmplt_ps(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpps(a, b, 1);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmple_ss(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpss(a, b, 2);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmple_ps(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpps(a, b, 2);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_ss(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpss(b, a, 1);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpgt_ps(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpps(b, a, 1);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpge_ss(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpss(b, a, 2);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpge_ps(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpps(b, a, 2);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpneq_ss(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpss(a, b, 4);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpneq_ps(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpps(a, b, 4);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnlt_ss(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpss(a, b, 5);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnlt_ps(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpps(a, b, 5);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnle_ss(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpss(a, b, 6);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnle_ps(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpps(a, b, 6);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpngt_ss(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpss(b, a, 5);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpngt_ps(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpps(b, a, 5);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnge_ss(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpss(b, a, 6);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpnge_ps(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpps(b, a, 6);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpord_ss(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpss(a, b, 7);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpord_ps(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpps(a, b, 7);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpunord_ss(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpss(a, b, 3);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cmpunord_ps(__m128 a, __m128 b)
{
  return (__m128)__builtin_ia32_cmpps(a, b, 3);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comieq_ss(__m128 a, __m128 b)
{
  return __builtin_ia32_comieq(a, b);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comilt_ss(__m128 a, __m128 b)
{
  return __builtin_ia32_comilt(a, b);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comile_ss(__m128 a, __m128 b)
{
  return __builtin_ia32_comile(a, b);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comigt_ss(__m128 a, __m128 b)
{
  return __builtin_ia32_comigt(a, b);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comige_ss(__m128 a, __m128 b)
{
  return __builtin_ia32_comige(a, b);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_comineq_ss(__m128 a, __m128 b)
{
  return __builtin_ia32_comineq(a, b);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomieq_ss(__m128 a, __m128 b)
{
  return __builtin_ia32_ucomieq(a, b);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomilt_ss(__m128 a, __m128 b)
{
  return __builtin_ia32_ucomilt(a, b);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomile_ss(__m128 a, __m128 b)
{
  return __builtin_ia32_ucomile(a, b);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomigt_ss(__m128 a, __m128 b)
{
  return __builtin_ia32_ucomigt(a, b);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomige_ss(__m128 a, __m128 b)
{
  return __builtin_ia32_ucomige(a, b);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_ucomineq_ss(__m128 a, __m128 b)
{
  return __builtin_ia32_ucomineq(a, b);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvtss_si32(__m128 a)
{
  return __builtin_ia32_cvtss2si(a);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvt_ss2si(__m128 a)
{
  return _mm_cvtss_si32(a);
}

#ifdef __x86_64__

__inline__ long __attribute__((__always_inline__, __nodebug__))
_mm_cvtss_si64(__m128 a)
{
  return __builtin_ia32_cvtss2si64(a);
}

#endif

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvttss_si32(__m128 a)
{
  return a[0];
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_cvtt_ss2si(__m128 a)
{
  return _mm_cvttss_si32(a);
}

__inline__ long __attribute__((__always_inline__, __nodebug__))
_mm_cvttss_si64(__m128 a)
{
  return a[0];
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi32_ss(__m128 a, int b)
{
  a[0] = b;
  return a;
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvt_si2ss(__m128 a, int b)
{
  return _mm_cvtsi32_ss(a, b);
}

#ifdef __x86_64__

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_cvtsi64_ss(__m128 a, long b)
{
  a[0] = b;
  return a;
}

#endif

__inline__ float __attribute__((__always_inline__, __nodebug__))
_mm_cvtss_f32(__m128 a)
{
  return a[0];
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_load_ss(const float *p)
{
  return (__m128){ *p, 0, 0, 0 };
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_load1_ps(const float *p)
{
  return (__m128){ *p, *p, *p, *p };
}

#define        _mm_load_ps1(p) _mm_load1_ps(p)

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_load_ps(const float *p)
{
  return *(__m128*)p;
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_loadu_ps(const float *p)
{
  return __builtin_ia32_loadups(p);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_loadr_ps(const float *p)
{
  __m128 a = _mm_load_ps(p);
  return __builtin_shufflevector(a, a, 3, 2, 1, 0);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_set_ss(float w)
{
  return (__m128){ w, 0, 0, 0 };
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_set1_ps(float w)
{
  return (__m128){ w, w, w, w };
}

// Microsoft specific.
__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_set_ps1(float w)
{
    return _mm_set1_ps(w);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_set_ps(float z, float y, float x, float w)
{
  return (__m128){ w, x, y, z };
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_setr_ps(float z, float y, float x, float w)
{
  return (__m128){ z, y, x, w };
}

__inline__ __m128 __attribute__((__always_inline__))
_mm_setzero_ps(void)
{
  return (__m128){ 0, 0, 0, 0 };
}

__inline__ void __attribute__((__always_inline__))
_mm_store_ss(float *p, __m128 a)
{
  *p = a[0];
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storeu_ps(float *p, __m128 a)
{
  __builtin_ia32_storeups(p, a);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store1_ps(float *p, __m128 a)
{
  a = __builtin_shufflevector(a, a, 0, 0, 0, 0);
  _mm_storeu_ps(p, a);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store_ps1(float *p, __m128 a)
{
    return _mm_store1_ps(p, a);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_store_ps(float *p, __m128 a)
{
  *(__m128 *)p = a;
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_storer_ps(float *p, __m128 a)
{
  a = __builtin_shufflevector(a, a, 3, 2, 1, 0);
  _mm_store_ps(p, a);
}

#define _MM_HINT_T0 3
#define _MM_HINT_T1 2
#define _MM_HINT_T2 1
#define _MM_HINT_NTA 0

/* FIXME: We have to #define this because "sel" must be a constant integer, and
   Sema doesn't do any form of constant propagation yet. */

#define _mm_prefetch(a, sel) (__builtin_prefetch((void *)(a), 0, sel))

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_stream_ps(float *p, __m128 a)
{
  __builtin_ia32_movntps(p, a);
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_sfence(void)
{
  __builtin_ia32_sfence();
}

__inline__ unsigned int __attribute__((__always_inline__, __nodebug__))
_mm_getcsr(void)
{
  return __builtin_ia32_stmxcsr();
}

__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm_setcsr(unsigned int i)
{
  __builtin_ia32_ldmxcsr(i);
}

#define _mm_shuffle_ps(a, b, mask) \
        (__builtin_shufflevector((__v4sf)(a), (__v4sf)(b),                \
                                 (mask) & 0x3, ((mask) & 0xc) >> 2, \
                                 (((mask) & 0x30) >> 4) + 4, \
                                 (((mask) & 0xc0) >> 6) + 4))

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_unpackhi_ps(__m128 a, __m128 b)
{
  return __builtin_shufflevector(a, b, 2, 6, 3, 7);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_unpacklo_ps(__m128 a, __m128 b)
{
  return __builtin_shufflevector(a, b, 0, 4, 1, 5);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_move_ss(__m128 a, __m128 b)
{
  return __builtin_shufflevector(a, b, 4, 1, 2, 3);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_movehl_ps(__m128 a, __m128 b)
{
  return __builtin_shufflevector(a, b, 6, 7, 2, 3);
}

__inline__ __m128 __attribute__((__always_inline__, __nodebug__))
_mm_movelh_ps(__m128 a, __m128 b)
{
  return __builtin_shufflevector(a, b, 0, 1, 4, 5);
}

__inline__ int __attribute__((__always_inline__, __nodebug__))
_mm_movemask_ps(__m128 a)
{
  return __builtin_ia32_movmskps(a);
}

#define _MM_SHUFFLE(z, y, x, w) (((z) << 6) | ((y) << 4) | ((x) << 2) | (w))

#define _MM_EXCEPT_INVALID    (0x0001)
#define _MM_EXCEPT_DENORM     (0x0002)
#define _MM_EXCEPT_DIV_ZERO   (0x0004)
#define _MM_EXCEPT_OVERFLOW   (0x0008)
#define _MM_EXCEPT_UNDERFLOW  (0x0010)
#define _MM_EXCEPT_INEXACT    (0x0020)
#define _MM_EXCEPT_MASK       (0x003f)

#define _MM_MASK_INVALID      (0x0080)
#define _MM_MASK_DENORM       (0x0100)
#define _MM_MASK_DIV_ZERO     (0x0200)
#define _MM_MASK_OVERFLOW     (0x0400)
#define _MM_MASK_UNDERFLOW    (0x0800)
#define _MM_MASK_INEXACT      (0x1000)
#define _MM_MASK_MASK         (0x1f80)

#define _MM_ROUND_NEAREST     (0x0000)
#define _MM_ROUND_DOWN        (0x2000)
#define _MM_ROUND_UP          (0x4000)
#define _MM_ROUND_TOWARD_ZERO (0x6000)
#define _MM_ROUND_MASK        (0x6000)

#define _MM_FLUSH_ZERO_MASK   (0x8000)
#define _MM_FLUSH_ZERO_ON     (0x8000)
#define _MM_FLUSH_ZERO_OFF    (0x8000)

#define _MM_GET_EXCEPTION_MASK() (_mm_getcsr() & _MM_MASK_MASK)
#define _MM_GET_EXCEPTION_STATE() (_mm_getcsr() & _MM_EXCEPT_MASK)
#define _MM_GET_FLUSH_ZERO_MODE() (_mm_getcsr() & _MM_FLUSH_ZERO_MASK)
#define _MM_GET_ROUNDING_MODE() (_mm_getcsr() & _MM_ROUND_MASK)

#define _MM_SET_EXCEPTION_MASK(x) (_mm_setcsr((_mm_getcsr() & ~_MM_MASK_MASK) | (x)))
#define _MM_SET_EXCEPTION_STATE(x) (_mm_setcsr((_mm_getcsr() & ~_MM_EXCEPT_MASK) | (x)))
#define _MM_SET_FLUSH_ZERO_MODE(x) (_mm_setcsr((_mm_getcsr() & ~_MM_FLUSH_ZERO_MASK) | (x)))
#define _MM_SET_ROUNDING_MODE(x) (_mm_setcsr((_mm_getcsr() & ~_MM_ROUND_MASK) | (x)))

#define _MM_TRANSPOSE4_PS(row0, row1, row2, row3) \
do { \
  __m128 tmp3, tmp2, tmp1, tmp0; \
  tmp0 = _mm_unpacklo_ps((row0), (row1)); \
  tmp2 = _mm_unpacklo_ps((row2), (row3)); \
  tmp1 = _mm_unpackhi_ps((row0), (row1)); \
  tmp3 = _mm_unpackhi_ps((row2), (row3)); \
  (row0) = _mm_movelh_ps(tmp0, tmp2); \
  (row1) = _mm_movehl_ps(tmp2, tmp0); \
  (row2) = _mm_movelh_ps(tmp1, tmp3); \
  (row3) = _mm_movehl_ps(tmp3, tmp1); \
} while (0)

/* Aliases for compatibility. */
#define _m_pextrw _mm_extract_pi16
#define _m_pinsrw _mm_insert_pi16
#define _m_pmaxsw _mm_max_pi16
#define _m_pmaxub _mm_max_pu8
#define _m_pminsw _mm_min_pi16
#define _m_pminub _mm_min_pu8
#define _m_pmovmskb _mm_movemask_pi8
#define _m_pmulhuw _mm_mulhi_pu16
#define _m_pshufw _mm_shuffle_pi16
#define _m_maskmovq _mm_maskmove_si64
#define _m_pavgb _mm_avg_pu8
#define _m_pavgw _mm_avg_pu16
#define _m_psadbw _mm_sad_pu8
#define _m_ _mm_
#define _m_ _mm_

/* Ugly hack for backwards-compatibility (compatible with gcc) */
#ifdef __SSE2__
#include <emmintrin.h>
#endif

#endif /* __SSE__ */

#endif /* __XMMINTRIN_H */
