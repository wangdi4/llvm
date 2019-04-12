/* INTEL_FEATURE_ISA_VP2INTERSECT */
/*===------- avx512vpintersectintrin.h - VP2INTERSECT intrinsics ------------===
 *
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
#ifndef __IMMINTRIN_H
#error "Never use <avx512vp2intersect.h> directly; include <immintrin.h> instead."
#endif

#ifndef _AVX512VP2INTERSECT_H
#define _AVX512VP2INTERSECT_H

#define __DEFAULT_FN_ATTRS \
  __attribute__((__always_inline__, __nodebug__,  __target__("avx512vp2intrin"), \
                 __min_vector_width__(512)))

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_2intersect_epi32(__m512i __a, __m512i __b, __mmask16 *__m0, __mmask16 *__m1) {
  __builtin_ia32_vp2intersect_d_512((__v16si)__a, (__v16si)__b, __m0, __m1);
}

static __inline__ void __DEFAULT_FN_ATTRS
_mm512_2intersect_epi64(__m512i __a, __m512i __b, __mmask8 *__m0, __mmask8 *__m1) {
  __builtin_ia32_vp2intersect_q_512((__v8di)__a, (__v8di)__b, __m0, __m1);
}

#undef __DEFAULT_FN_ATTRS

#endif
/* end INTEL_FEATURE_ISA_VP2INTERSECT */
