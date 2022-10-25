/*===-------- avxvnniint8intrin.h - AVXVNNIINT8 intrinsics -----------=== */
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
#error "Never use <avxvnniint8intrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVXVNNIINT8INTRIN_H
#define __AVXVNNIINT8INTRIN_H

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS256 \
  __attribute__((__always_inline__, __nodebug__, __target__("avxvnniint8"), __min_vector_width__(256)))
#define __DEFAULT_FN_ATTRS128 \
  __attribute__((__always_inline__, __nodebug__, __target__("avxvnniint8"), __min_vector_width__(128)))

/// Multiply groups of 4 adjacent pairs of signed 8-bit integers in \a __A with
///    corresponding signed 8-bit integers in \a __B, producing 4 intermediate
///    signed 16-bit results. Sum these 4 results with the corresponding
///    32-bit integer in \a __W, and store the packed 32-bit results in \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// _mm_vpdpbssd_epi32(__m128i __W, __m128i __A, __m128i __B);
/// \endcode
///
/// This intrinsic corresponds to the \c VPDPBSSD instruction.
///
/// \param __A
///    A 128-bit vector of [16 x char].
/// \param __B
///    A 128-bit vector of [16 x char].
/// \returns
///    A 128-bit vector of [4 x int].
///
/// \code{.operation}
/// FOR j := 0 to 3
/// 	tmp1.word := SignExtend16(__A.byte[4*j]) * SignExtend16(__B.byte[4*j])
/// 	tmp2.word := SignExtend16(__A.byte[4*j+1]) * SignExtend16(__B.byte[4*j+1])
/// 	tmp3.word := SignExtend16(__A.byte[4*j+2]) * SignExtend16(__B.byte[4*j+2])
/// 	tmp4.word := SignExtend16(__A.byte[4*j+3]) * SignExtend16(__B.byte[4*j+3])
/// 	dst.dword[j] := __W.dword[j] + tmp1 + tmp2 + tmp3 + tmp4
/// ENDFOR
/// dst[MAX:128] := 0
/// \endcode
static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_vpdpbssd_epi32(__m128i __W, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vpdpbssd128((__v4si)__W, (__v4si)__A, (__v4si)__B);
}

/// Multiply groups of 4 adjacent pairs of signed 8-bit integers in \a __A with
///    corresponding signed 8-bit integers in \a __B, producing 4 intermediate
///    signed 16-bit results. Sum these 4 results with the corresponding
///    32-bit integer in \a __W, and store the packed 32-bit results in \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// _mm256_vpdpbssd_epi32(__m256i __W, __m256i __A, __m256i __B);
/// \endcode
///
/// This intrinsic corresponds to the \c VPDPBSSD instruction.
///
/// \param __A
///    A 256-bit vector of [32 x char].
/// \param __B
///    A 256-bit vector of [32 x char].
/// \returns
///    A 256-bit vector of [8 x int].
///
/// \code{.operation}
/// FOR j := 0 to 7
/// 	tmp1.word := SignExtend16(__A.byte[4*j]) * SignExtend16(__B.byte[4*j])
/// 	tmp2.word := SignExtend16(__A.byte[4*j+1]) * SignExtend16(__B.byte[4*j+1])
/// 	tmp3.word := SignExtend16(__A.byte[4*j+2]) * SignExtend16(__B.byte[4*j+2])
/// 	tmp4.word := SignExtend16(__A.byte[4*j+3]) * SignExtend16(__B.byte[4*j+3])
/// 	dst.dword[j] := __W.dword[j] + tmp1 + tmp2 + tmp3 + tmp4
/// ENDFOR
/// dst[MAX:256] := 0
/// \endcode
static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_vpdpbssd_epi32(__m256i __W, __m256i __A, __m256i __B) {
  return (__m256i)__builtin_ia32_vpdpbssd256((__v8si)__W, (__v8si)__A, (__v8si)__B);
}

/// Multiply groups of 4 adjacent pairs of signed 8-bit integers in \a __A with
///    corresponding signed 8-bit integers in \a __B, producing 4 intermediate
///    signed 16-bit results. Sum these 4 results with the corresponding
///    32-bit integer in \a __W with signed saturation, and store the packed
///    32-bit results in \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// _mm_vpdpbssds_epi32( __m128i __W, __m128i __A, __m128i __B);
/// \endcode
///
/// This intrinsic corresponds to the \c VPDPBSSD instruction.
///
/// \param __A
///    A 128-bit vector of [16 x char].
/// \param __B
///    A 128-bit vector of [16 x char].
/// \returns
///    A 128-bit vector of [4 x int].
///
/// \code{.operation}
/// FOR j := 0 to 3
/// 	tmp1.word := SignExtend16(__A.byte[4*j]) * SignExtend16(__B.byte[4*j])
/// 	tmp2.word := SignExtend16(__A.byte[4*j+1]) * SignExtend16(__B.byte[4*j+1])
/// 	tmp3.word := SignExtend16(__A.byte[4*j+2]) * SignExtend16(__B.byte[4*j+2])
/// 	tmp4.word := SignExtend16(__A.byte[4*j+3]) * SignExtend16(__B.byte[4*j+3])
/// 	dst.dword[j] := SIGNED_DWORD_SATURATE(__W.dword[j] + tmp1 + tmp2 + tmp3 + tmp4)
/// ENDFOR
/// dst[MAX:128] := 0
/// \endcode
static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_vpdpbssds_epi32( __m128i __W, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vpdpbssds128((__v4si)__W, (__v4si)__A,(__v4si)__B);
}

/// Multiply groups of 4 adjacent pairs of signed 8-bit integers in \a __A with
///    corresponding signed 8-bit integers in \a __B, producing 4 intermediate
///    signed 16-bit results. Sum these 4 results with the corresponding
///    32-bit integer in \a __W with signed saturation, and store the packed
///    32-bit results in \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// _mm256_vpdpbssds_epi32(__m256i __W, __m256i __A, __m256i __B);
/// \endcode
///
/// This intrinsic corresponds to the \c VPDPBSSD instruction.
///
/// \param __A
///    A 256-bit vector of [32 x char].
/// \param __B
///    A 256-bit vector of [32 x char].
/// \returns
///    A 256-bit vector of [8 x int].
///
/// \code{.operation}
/// FOR j := 0 to 7
/// 	tmp1.word := SignExtend16(__A.byte[4*j]) * SignExtend16(__B.byte[4*j])
/// 	tmp2.word := SignExtend16(__A.byte[4*j+1]) * SignExtend16(__B.byte[4*j+1])
/// 	tmp3.word := SignExtend16(__A.byte[4*j+2]) * SignExtend16(__B.byte[4*j+2])
/// 	tmp4.word := SignExtend16(__A.byte[4*j+3]) * SignExtend16(__B.byte[4*j+3])
/// 	dst.dword[j] := SIGNED_DWORD_SATURATE(__W.dword[j] + tmp1 + tmp2 + tmp3 + tmp4)
/// ENDFOR
/// dst[MAX:256] := 0
/// \endcode
static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_vpdpbssds_epi32(__m256i __W, __m256i __A, __m256i __B) {
  return (__m256i)__builtin_ia32_vpdpbssds256((__v8si)__W, (__v8si)__A, (__v8si)__B);
}

/// Multiply groups of 4 adjacent pairs of signed 8-bit integers in \a __A with
///    corresponding unsigned 8-bit integers in \a __B, producing 4 intermediate
///    signed 16-bit results. Sum these 4 results with the corresponding
///    32-bit integer in \a __W, and store the packed 32-bit results in \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// _mm_vpdpbsud_epi32(__m128i __W, __m128i __A, __m128i __B);
/// \endcode
///
/// This intrinsic corresponds to the \c VPDPBSSD instruction.
///
/// \param __A
///    A 128-bit vector of [16 x char].
/// \param __B
///    A 128-bit vector of [16 x unsigned char].
/// \returns
///    A 128-bit vector of [4 x int].
///
/// \code{.operation}
/// FOR j := 0 to 3
/// 	tmp1.word := Signed(SignExtend16(__A.byte[4*j]) * ZeroExtend16(__B.byte[4*j]))
/// 	tmp2.word := Signed(SignExtend16(__A.byte[4*j+1]) * ZeroExtend16(__B.byte[4*j+1]))
/// 	tmp3.word := Signed(SignExtend16(__A.byte[4*j+2]) * ZeroExtend16(__B.byte[4*j+2]))
/// 	tmp4.word := Signed(SignExtend16(__A.byte[4*j+3]) * ZeroExtend16(__B.byte[4*j+3]))
/// 	dst.dword[j] := __W.dword[j] + tmp1 + tmp2 + tmp3 + tmp4
/// ENDFOR
/// dst[MAX:128] := 0
/// \endcode
static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_vpdpbsud_epi32(__m128i __W, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vpdpbsud128((__v4si)__W, (__v4si)__A,(__v4si)__B);
}

/// Multiply groups of 4 adjacent pairs of signed 8-bit integers in \a __A with
///    corresponding unsigned 8-bit integers in \a __B, producing 4 intermediate
///    signed 16-bit results. Sum these 4 results with the corresponding
///    32-bit integer in \a __W, and store the packed 32-bit results in \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// _mm256_vpdpbsud_epi32(__m256i __W, __m256i __A, __m256i __B);
/// \endcode
///
/// This intrinsic corresponds to the \c VPDPBSSD instruction.
///
/// \param __A
///    A 256-bit vector of [32 x char].
/// \param __B
///    A 256-bit vector of [32 x unsigned char].
/// \returns
///    A 256-bit vector of [8 x int].
///
/// \code{.operation}
/// FOR j := 0 to 7
/// 	tmp1.word := Signed(SignExtend16(__A.byte[4*j]) * ZeroExtend16(__B.byte[4*j]))
/// 	tmp2.word := Signed(SignExtend16(__A.byte[4*j+1]) * ZeroExtend16(__B.byte[4*j+1]))
/// 	tmp3.word := Signed(SignExtend16(__A.byte[4*j+2]) * ZeroExtend16(__B.byte[4*j+2]))
/// 	tmp4.word := Signed(SignExtend16(__A.byte[4*j+3]) * ZeroExtend16(__B.byte[4*j+3]))
/// 	dst.dword[j] := __W.dword[j] + tmp1 + tmp2 + tmp3 + tmp4
/// ENDFOR
/// dst[MAX:256] := 0
/// \endcode
static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_vpdpbsud_epi32(__m256i __W, __m256i __A, __m256i __B) {
  return (__m256i)__builtin_ia32_vpdpbsud256 ((__v8si)__W, (__v8si)__A, (__v8si)__B);
}

/// Multiply groups of 4 adjacent pairs of signed 8-bit integers in \a __A with
///    corresponding unsigned 8-bit integers in \a __B, producing 4 intermediate
///    signed 16-bit results. Sum these 4 results with the corresponding
///    32-bit integer in \a __W with signed saturation, and store the packed
///    32-bit results in \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// _mm_vpdpbsuds_epi32( __m128i __W, __m128i __A, __m128i __B);
/// \endcode
///
/// This intrinsic corresponds to the \c VPDPBSSD instruction.
///
/// \param __A
///    A 128-bit vector of [16 x char].
/// \param __B
///    A 128-bit vector of [16 x unsigned char].
/// \returns
///    A 128-bit vector of [4 x int].
///
/// \code{.operation}
/// FOR j := 0 to 3
/// 	tmp1.word := Signed(SignExtend16(__A.byte[4*j]) * ZeroExtend16(__B.byte[4*j]))
/// 	tmp2.word := Signed(SignExtend16(__A.byte[4*j+1]) * ZeroExtend16(__B.byte[4*j+1]))
/// 	tmp3.word := Signed(SignExtend16(__A.byte[4*j+2]) * ZeroExtend16(__B.byte[4*j+2]))
/// 	tmp4.word := Signed(SignExtend16(__A.byte[4*j+3]) * ZeroExtend16(__B.byte[4*j+3]))
/// 	dst.dword[j] := SIGNED_DWORD_SATURATE(__W.dword[j] + tmp1 + tmp2 + tmp3 + tmp4)
/// ENDFOR
/// dst[MAX:128] := 0
/// \endcode
static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_vpdpbsuds_epi32(__m128i __W, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vpdpbsuds128((__v4si)__W, (__v4si)__A,(__v4si)__B);
}

/// Multiply groups of 4 adjacent pairs of signed 8-bit integers in \a __A with
///    corresponding unsigned 8-bit integers in \a __B, producing 4 intermediate
///    signed 16-bit results. Sum these 4 results with the corresponding
///    32-bit integer in \a __W with signed saturation, and store the packed
///    32-bit results in \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// _mm256_vpdpbsuds_epi32(__m256i __W, __m256i __A, __m256i __B);
/// \endcode
///
/// This intrinsic corresponds to the \c VPDPBSSD instruction.
///
/// \param __A
///    A 256-bit vector of [32 x char].
/// \param __B
///    A 256-bit vector of [32 x unsigned char].
/// \returns
///    A 256-bit vector of [8 x int].
///
/// \code{.operation}
/// FOR j := 0 to 7
/// 	tmp1.word := Signed(SignExtend16(__A.byte[4*j]) * ZeroExtend16(__B.byte[4*j]))
/// 	tmp2.word := Signed(SignExtend16(__A.byte[4*j+1]) * ZeroExtend16(__B.byte[4*j+1]))
/// 	tmp3.word := Signed(SignExtend16(__A.byte[4*j+2]) * ZeroExtend16(__B.byte[4*j+2]))
/// 	tmp4.word := Signed(SignExtend16(__A.byte[4*j+3]) * ZeroExtend16(__B.byte[4*j+3]))
/// 	dst.dword[j] := SIGNED_DWORD_SATURATE(__W.dword[j] + tmp1 + tmp2 + tmp3 + tmp4)
/// ENDFOR
/// dst[MAX:256] := 0
/// \endcode
static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_vpdpbsuds_epi32(__m256i __W, __m256i __A, __m256i __B) {
  return (__m256i)__builtin_ia32_vpdpbsuds256((__v8si)__W, (__v8si)__A, (__v8si)__B);
}

/// Multiply groups of 4 adjacent pairs of unsigned 8-bit integers in \a __A with
///    corresponding unsigned 8-bit integers in \a __B, producing 4 intermediate
///    signed 16-bit results. Sum these 4 results with the corresponding
///    32-bit integer in \a __W, and store the packed 32-bit results in \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// _mm_vpdpbuud_epi32(__m128i __W, __m128i __A, __m128i __B);
/// \endcode
///
/// This intrinsic corresponds to the \c VPDPBSSD instruction.
///
/// \param __A
///    A 128-bit vector of [16 x unsigned char].
/// \param __B
///    A 128-bit vector of [16 x unsigned char].
/// \returns
///    A 128-bit vector of [4 x int].
///
/// \code{.operation}
/// FOR j := 0 to 3
/// 	tmp1.word := ZeroExtend16(__A.byte[4*j]) * ZeroExtend16(__B.byte[4*j])
/// 	tmp2.word := ZeroExtend16(__A.byte[4*j+1]) * ZeroExtend16(__B.byte[4*j+1])
/// 	tmp3.word := ZeroExtend16(__A.byte[4*j+2]) * ZeroExtend16(__B.byte[4*j+2])
/// 	tmp4.word := ZeroExtend16(__A.byte[4*j+3]) * ZeroExtend16(__B.byte[4*j+3])
/// 	dst.dword[j] := __W.dword[j] + tmp1 + tmp2 + tmp3 + tmp4
/// ENDFOR
/// dst[MAX:128] := 0
/// \endcode
static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_vpdpbuud_epi32(__m128i __W, __m128i __A, __m128i __B) {
  return (__m128i)__builtin_ia32_vpdpbuud128((__v4si)__W, (__v4si)__A,(__v4si)__B);
}

/// Multiply groups of 4 adjacent pairs of unsigned 8-bit integers in \a __A with
///    corresponding unsigned 8-bit integers in \a __B, producing 4 intermediate
///    signed 16-bit results. Sum these 4 results with the corresponding
///    32-bit integer in \a __W, and store the packed 32-bit results in \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// _mm256_vpdpbuud_epi32(__m256i __W, __m256i __A, __m256i __B);
/// \endcode
///
/// This intrinsic corresponds to the \c VPDPBSSD instruction.
///
/// \param __A
///    A 256-bit vector of [32 x unsigned char].
/// \param __B
///    A 256-bit vector of [32 x unsigned char].
/// \returns
///    A 256-bit vector of [8 x int].
///
/// \code{.operation}
/// FOR j := 0 to 7
/// 	tmp1.word := ZeroExtend16(__A.byte[4*j]) * ZeroExtend16(__B.byte[4*j])
/// 	tmp2.word := ZeroExtend16(__A.byte[4*j+1]) * ZeroExtend16(__B.byte[4*j+1])
/// 	tmp3.word := ZeroExtend16(__A.byte[4*j+2]) * ZeroExtend16(__B.byte[4*j+2])
/// 	tmp4.word := ZeroExtend16(__A.byte[4*j+3]) * ZeroExtend16(__B.byte[4*j+3])
/// 	dst.dword[j] := __W.dword[j] + tmp1 + tmp2 + tmp3 + tmp4
/// ENDFOR
/// dst[MAX:256] := 0
/// \endcode
static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_vpdpbuud_epi32(__m256i __W, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_vpdpbuud256 ((__v8si)__W, (__v8si)__A, (__v8si)__B);
}

/// Multiply groups of 4 adjacent pairs of unsigned 8-bit integers in \a __A with
///    corresponding unsigned 8-bit integers in \a __B, producing 4 intermediate
///    signed 16-bit results. Sum these 4 results with the corresponding
///    32-bit integer in \a __W with signed saturation, and store the packed
///    32-bit results in \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// _mm_vpdpbuuds_epi32( __m128i __W, __m128i __A, __m128i __B);
/// \endcode
///
/// This intrinsic corresponds to the \c VPDPBUUDS instruction.
///
/// \param __A
///    A 128-bit vector of [16 x unsigned char].
/// \param __B
///    A 128-bit vector of [16 x unsigned char].
/// \returns
///    A 128-bit vector of [4 x int].
///
/// \code{.operation}
/// FOR j := 0 to 3
/// 	tmp1.word := ZeroExtend16(__A.byte[4*j]) * ZeroExtend16(__B.byte[4*j])
/// 	tmp2.word := ZeroExtend16(__A.byte[4*j+1]) * ZeroExtend16(__B.byte[4*j+1])
/// 	tmp3.word := ZeroExtend16(__A.byte[4*j+2]) * ZeroExtend16(__B.byte[4*j+2])
/// 	tmp4.word := ZeroExtend16(__A.byte[4*j+3]) * ZeroExtend16(__B.byte[4*j+3])
/// 	dst.dword[j] := UNSIGNED_DWORD_SATURATE(__W.dword[j] + tmp1 + tmp2 + tmp3 + tmp4)
/// ENDFOR
/// dst[MAX:128] := 0
/// \endcode
static __inline__ __m128i __DEFAULT_FN_ATTRS128
_mm_vpdpbuuds_epi32(__m128i __W, __m128i __A, __m128i __B)
{
  return (__m128i)__builtin_ia32_vpdpbuuds128((__v4si)__W, (__v4si)__A, (__v4si)__B);
}

/// Multiply groups of 4 adjacent pairs of signed 8-bit integers in \a __A with
///    corresponding unsigned 8-bit integers in \a __B, producing 4 intermediate
///    signed 16-bit results. Sum these 4 results with the corresponding
///    32-bit integer in \a __W with signed saturation, and store the packed
///    32-bit results in \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// _mm256_vpdpbuuds_epi32(__m256i __W, __m256i __A, __m256i __B);
/// \endcode
///
/// This intrinsic corresponds to the \c VPDPBUUDS instruction.
///
/// \param __A
///    A 256-bit vector of [32 x unsigned char].
/// \param __B
///    A 256-bit vector of [32 x unsigned char].
/// \returns
///    A 256-bit vector of [8 x int].
///
/// \code{.operation}
/// FOR j := 0 to 7
/// 	tmp1.word := ZeroExtend16(__A.byte[4*j]) * ZeroExtend16(__B.byte[4*j])
/// 	tmp2.word := ZeroExtend16(__A.byte[4*j+1]) * ZeroExtend16(__B.byte[4*j+1])
/// 	tmp3.word := ZeroExtend16(__A.byte[4*j+2]) * ZeroExtend16(__B.byte[4*j+2])
/// 	tmp4.word := ZeroExtend16(__A.byte[4*j+3]) * ZeroExtend16(__B.byte[4*j+3])
/// 	dst.dword[j] := UNSIGNED_DWORD_SATURATE(__W.dword[j] + tmp1 + tmp2 + tmp3 + tmp4)
/// ENDFOR
/// dst[MAX:256] := 0
/// \endcode
static __inline__ __m256i __DEFAULT_FN_ATTRS256
_mm256_vpdpbuuds_epi32(__m256i __W, __m256i __A, __m256i __B)
{
  return (__m256i)__builtin_ia32_vpdpbuuds256 ((__v8si)__W, (__v8si)__A, (__v8si)__B);
}
#undef __DEFAULT_FN_ATTRS128
#undef __DEFAULT_FN_ATTRS256

#endif // __AVXVNNIINT8INTRIN_H
