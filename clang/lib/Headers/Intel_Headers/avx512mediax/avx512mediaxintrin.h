/*===--------- avx512mediaxintrin.h - AVX512mediax intrinsics ---------------=== */
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
#error "Never use <avx512mediaxintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __AVX512MEDIAXINTRIN_H
#define __AVX512MEDIAXINTRIN_H

/// Compute the sum of absolute differences (SADs) of quadruplets of unsigned
///    8-bit integers in \a A compared to those in \a B with mask, and store the
///    16-bit results in \a dst using writemask \a U (elements are copied from
///    \a W when the corresponding mask bit is not set). Eight SADs are
///    performed using one quadruplet from \a B and eight quadruplets from \a A.
///    One quadruplet is selected from \a B starting at on the offset specified
///    in \a imm. Eight quadruplets are formed from sequential 8-bit integers
///    selected from \a A starting at the offset specified in \a imm.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m128i _mm_mask_mpsadbw_epu8(__m128i W, __mmask8 U, __m128i A, __m128i B, const int imm);
/// \endcode
///
/// This intrinsic corresponds to the \c VMPSADBW instruction.
///
/// \param W
///    A 128-bit vector of [16 x i8].
/// \param U
///    A 8-bit mask value specifying what is chosen for each element.
/// \param A
///    A 128-bit vector of [16 x i8].
/// \param B
///    A 128-bit vector of [16 x i8].
/// \param imm
///    An 8-bit immediate operand specifying how the absolute differences are to
///    be calculated.
/// \returns A 128-bit integer vector containing the sums of the sets of
///    absolute differences between both operands.
///
/// \code{.operation}
/// DEFINE MPSADBW_MASK(W[127:0], U[7:0], A[127:0], B[127:0], imm[2:0]) {
/// 	a_offset := imm[2]*32
/// 	b_offset := imm[1:0]*32
/// 	FOR j := 0 to 7
/// 		i := j*8
/// 		IF U[j]
/// 			k := a_offset+i
/// 			l := b_offset
/// 			tmp[i*2+15:i*2] := ABS(Signed(A[k+7:k] - B[l+7:l])) + ABS(Signed(A[k+15:k+8] - B[l+15:l+8])) +
/// 			                   ABS(Signed(A[k+23:k+16] - B[l+23:l+16])) + ABS(Signed(A[k+31:k+24] - B[l+31:l+24]))
/// 		ELSE
/// 			tmp[i*2+15:i*2] := W[i*2+15:i*2]
/// 	ENDFOR
/// 	RETURN tmp[127:0]
/// }
/// dst[127:0] := MPSADBW_MASK(W[127:0], U[7:0], A[127:0], B[127:0], imm[2:0])
/// dst[MAX:128] := 0
/// \endcode
#define _mm_mask_mpsadbw_epu8(W, U, A, B, imm) \
  (__m128i)__builtin_ia32_selectw_128((__mmask8)(U), \
                                      (__v8hi)_mm_mpsadbw_epu8((A), (B), (imm)), \
                                      (__v8hi)(__m128i)(W))

/// Compute the sum of absolute differences (SADs) of quadruplets of unsigned
///    8-bit integers in \a A compared to those in \a B with mask, and store the
///    16-bit results in \a dst using zeromask \a U (elements are zeroed out
///    when the corresponding mask bit is not set). Eight SADs are performed
///    using one quadruplet from \a B and eight quadruplets from \a A. One
///    quadruplet is selected from \a B starting at on the offset specified in
///    \a imm. Eight quadruplets are formed from sequential 8-bit integers
///    selected from \a A starting at the offset specified in \a imm.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m128i _mm_maskz_mpsadbw_epu8(__mmask8 U, __m128i A, __m128i B, const int imm);
/// \endcode
///
/// This intrinsic corresponds to the \c VMPSADBW instruction.
///
/// \param U
///    A 8-bit mask value specifying what is chosen for each element.
/// \param A
///    A 128-bit vector of [16 x i8].
/// \param B
///    A 128-bit vector of [16 x i8].
/// \param imm
///    An 8-bit immediate operand specifying how the absolute differences are to
///    be calculated.
/// \returns A 128-bit integer vector containing the sums of the sets of
///    absolute differences between both operands.
///
/// \code{.operation}
/// DEFINE MPSADBW_MASKZ(U[7:0], A[127:0], B[127:0], imm[2:0]) {
/// 	a_offset := imm[2]*32
/// 	b_offset := imm[1:0]*32
/// 	FOR j := 0 to 7
/// 		i := j*8
/// 		IF U[j]
/// 			k := a_offset+i
/// 			l := b_offset
/// 			tmp[i*2+15:i*2] := ABS(Signed(A[k+7:k] - B[l+7:l])) + ABS(Signed(A[k+15:k+8] - B[l+15:l+8])) +
/// 			                   ABS(Signed(A[k+23:k+16] - B[l+23:l+16])) + ABS(Signed(A[k+31:k+24] - B[l+31:l+24]))
/// 		ELSE
/// 			tmp[i*2+15:i*2] := 0
/// 	ENDFOR
/// 	RETURN tmp[127:0]
/// }
/// dst[127:0] := MPSADBW_MASKZ(U[7:0], A[127:0], B[127:0], imm[2:0])
/// dst[MAX:128] := 0
/// \endcode
#define _mm_maskz_mpsadbw_epu8(U, A, B, imm) \
  (__m128i)__builtin_ia32_selectw_128((__mmask8)(U), \
                                      (__v8hi)_mm_mpsadbw_epu8((A), (B), (imm)), \
                                      (__v8hi)_mm_setzero_si128())

/// Compute the sum of absolute differences (SADs) of quadruplets of unsigned
///    8-bit integers in \a A compared to those in \a B with mask, and store the
///    16-bit results in \a dst using writemask \a U (elements are copied from
///    \a W when the corresponding mask bit is not set). Eight SADs are
///    performed for each 128-bit lane using one quadruplet from \a B and eight
///    quadruplets from \a A. One quadruplet is selected from \a B starting at
///    on the offset specified in \a imm. Eight quadruplets are formed from
///    sequential 8-bit integers selected from \a A starting at the offset
///    specified in \a imm.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m256i _mm256_mask_mpsadbw_epu8(__m256i W, __mmask16 U, __m256i A, __m256i B, const int imm);
/// \endcode
///
/// This intrinsic corresponds to the \c VMPSADBW instruction.
///
/// \param W
///    A 256-bit vector of [32 x i8].
/// \param U
///    A 16-bit mask value specifying what is chosen for each element.
/// \param A
///    A 256-bit vector of [32 x i8].
/// \param B
///    A 256-bit vector of [32 x i8].
/// \param imm
///    An 8-bit immediate operand specifying how the absolute differences are to
///    be calculated.
/// \returns A 256-bit integer vector containing the sums of the sets of
///    absolute differences between both operands.
///
/// \code{.operation}
/// DEFINE MPSADBW_MASK(W[127:0], U[7:0], A[127:0], B[127:0], imm[2:0]) {
/// 	a_offset := imm[2]*32
/// 	b_offset := imm[1:0]*32
/// 	FOR j := 0 to 7
/// 		i := j*8
/// 		IF U[j]
/// 			k := a_offset+i
/// 			l := b_offset
/// 			tmp[i*2+15:i*2] := ABS(Signed(A[k+7:k] - B[l+7:l])) + ABS(Signed(A[k+15:k+8] - B[l+15:l+8])) +
/// 			                   ABS(Signed(A[k+23:k+16] - B[l+23:l+16])) + ABS(Signed(A[k+31:k+24] - B[l+31:l+24]))
/// 		ELSE
/// 			tmp[i*2+15:i*2] := W[i*2+15:i*2]
/// 	ENDFOR
/// 	RETURN tmp[127:0]
/// }
/// dst[127:0] := MPSADBW_MASK(W[127:0], U[7:0], A[127:0], B[127:0], imm[2:0])
/// dst[255:128] := MPSADBW_MASK(W[255:128], U[15:8], A[255:128], B[255:128], imm[5:3])
/// dst[MAX:256] := 0
/// \endcode
#define _mm256_mask_mpsadbw_epu8(W, U, A, B, imm) \
  (__m256i)__builtin_ia32_selectw_256((__mmask16)(U), \
                                      (__v16hi)_mm256_mpsadbw_epu8((A), (B), (imm)), \
                                      (__v16hi)(__m256i)(W))

/// Compute the sum of absolute differences (SADs) of quadruplets of unsigned
///    8-bit integers in \a A compared to those in \a B with mask, and store the
///    16-bit results in \a dst using zeromask \a U (elements are zeroed out
///    when the corresponding mask bit is not set). Eight SADs are performed for
///    each 128-bit lane using one quadruplet from \a B and eight quadruplets
///    from \a A. One quadruplet is selected from \a B starting at on the offset
///    specified in \a imm. Eight quadruplets are formed from sequential 8-bit
///    integers selected from \a A starting at the offset specified in \a imm.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m256i _mm256_maskz_mpsadbw_epu8(__mmask16 U, __m256i A, __m256i B, const int imm);
/// \endcode
///
/// This intrinsic corresponds to the \c VMPSADBW instruction.
///
/// \param U
///    A 16-bit mask value specifying what is chosen for each element.
/// \param A
///    A 256-bit vector of [32 x i8].
/// \param B
///    A 256-bit vector of [32 x i8].
/// \param imm
///    An 8-bit immediate operand specifying how the absolute differences are to
///    be calculated.
/// \returns A 256-bit integer vector containing the sums of the sets of
///    absolute differences between both operands.
///
/// \code{.operation}
/// DEFINE MPSADBW_MASKZ(U[7:0], A[127:0], B[127:0], imm[2:0]) {
/// 	a_offset := imm[2]*32
/// 	b_offset := imm[1:0]*32
/// 	FOR j := 0 to 7
/// 		i := j*8
/// 		IF U[j]
/// 			k := a_offset+i
/// 			l := b_offset
/// 			tmp[i*2+15:i*2] := ABS(Signed(A[k+7:k] - B[l+7:l])) + ABS(Signed(A[k+15:k+8] - B[l+15:l+8])) +
/// 			                   ABS(Signed(A[k+23:k+16] - B[l+23:l+16])) + ABS(Signed(A[k+31:k+24] - B[l+31:l+24]))
/// 		ELSE
/// 			tmp[i*2+15:i*2] := 0
/// 	ENDFOR
/// 	RETURN tmp[127:0]
/// }
/// dst[127:0] := MPSADBW_MASKZ(U[7:0], A[127:0], B[127:0], imm[2:0])
/// dst[255:128] := MPSADBW_MASKZ(U[15:8], A[255:128], B[255:128], imm[5:3])
/// dst[MAX:256] := 0
/// \endcode
#define _mm256_maskz_mpsadbw_epu8(U, A, B, imm) \
  (__m256i)__builtin_ia32_selectw_256((__mmask16)(U), \
                                      (__v16hi)_mm256_mpsadbw_epu8((A), (B), (imm)), \
                                      (__v16hi)_mm256_setzero_si256())

/// Compute the sum of absolute differences (SADs) of quadruplets of unsigned
///    8-bit integers in \a A compared to those in \a B, and store the 16-bit
///    results in \a dst. Eight SADs are performed for each 128-bit lane using
///    one quadruplet from \a B and eight quadruplets from \a A. One quadruplet
///    is selected from \a B starting at on the offset specified in \a imm.
///    Eight quadruplets are formed from sequential 8-bit integers selected from
///    \a A starting at the offset specified in \a imm.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m512i _mm512_mpsadbw_epu8(__m512i A, __m512i B, const int imm);
/// \endcode
///
/// This intrinsic corresponds to the \c VMPSADBW instruction.
///
/// \param A
///    A 512-bit vector of [64 x i8].
/// \param B
///    A 512-bit vector of [64 x i8].
/// \param imm
///    An 8-bit immediate operand specifying how the absolute differences are to
///    be calculated.
/// \returns A 512-bit integer vector containing the sums of the sets of
///    absolute differences between both operands.
///
/// \code{.operation}
/// DEFINE MPSADBW(A[127:0], B[127:0], imm[2:0]) {
/// 	a_offset := imm[2]*32
/// 	b_offset := imm[1:0]*32
/// 	FOR j := 0 to 7
/// 		i := j*8
/// 		k := a_offset+i
/// 		l := b_offset
/// 		tmp[i*2+15:i*2] := ABS(Signed(A[k+7:k] - B[l+7:l])) + ABS(Signed(A[k+15:k+8] - B[l+15:l+8])) +
/// 		                   ABS(Signed(A[k+23:k+16] - B[l+23:l+16])) + ABS(Signed(A[k+31:k+24] - B[l+31:l+24]))
/// 	ENDFOR
/// 	RETURN tmp[127:0]
/// }
/// dst[127:0] := MPSADBW(A[127:0], B[127:0], imm[2:0])
/// dst[255:128] := MPSADBW(A[255:128], B[255:128], imm[5:3])
/// dst[383:256] := MPSADBW(A[383:256], B[383:256], imm[2:0])
/// dst[512:384] := MPSADBW(A[512:384], B[512:384], imm[5:3])
/// \endcode
#define _mm512_mpsadbw_epu8(A, B, imm) \
  (__m512i)__builtin_ia32_mpsadbw512((__v64qi)(__m512i)(A), \
                                     (__v64qi)(__m512i)(B), (int)(imm))

/// Compute the sum of absolute differences (SADs) of quadruplets of unsigned
///    8-bit integers in \a A compared to those in \a B with mask, and store the
///    16-bit results in \a dst using writemask \a U (elements are copied from
///    \a W when the corresponding mask bit is not set). Eight SADs are
///    performed for each 128-bit lane using one quadruplet from \a B and eight
///    quadruplets from \a A. One quadruplet is selected from \a B starting at
///    on the offset specified in \a imm. Eight quadruplets are formed from
///    sequential 8-bit integers selected from \a A starting at the offset
///    specified in \a imm.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m512i _mm512_mask_mpsadbw_epu8(__m512i W, __mmask32 U, __m512i A, __m512i B, const int imm);
/// \endcode
///
/// This intrinsic corresponds to the \c VMPSADBW instruction.
///
/// \param W
///    A 512-bit vector of [64 x i8].
/// \param U
///    A 32-bit mask value specifying what is chosen for each element.
/// \param A
///    A 512-bit vector of [64 x i8].
/// \param B
///    A 512-bit vector of [64 x i8].
/// \param imm
///    An 8-bit immediate operand specifying how the absolute differences are to
///    be calculated.
/// \returns A 512-bit integer vector containing the sums of the sets of
///    absolute differences between both operands.
///
/// \code{.operation}
/// DEFINE MPSADBW_MASK(W[127:0], U[7:0], A[127:0], B[127:0], imm[2:0]) {
/// 	a_offset := imm[2]*32
/// 	b_offset := imm[1:0]*32
/// 	FOR j := 0 to 7
/// 		i := j*8
/// 		IF U[j]
/// 			k := a_offset+i
/// 			l := b_offset
/// 			tmp[i*2+15:i*2] := ABS(Signed(A[k+7:k] - B[l+7:l])) + ABS(Signed(A[k+15:k+8] - B[l+15:l+8])) +
/// 			                   ABS(Signed(A[k+23:k+16] - B[l+23:l+16])) + ABS(Signed(A[k+31:k+24] - B[l+31:l+24]))
/// 		ELSE
/// 			tmp[i*2+15:i*2] := W[i*2+15:i*2]
/// 	ENDFOR
/// 	RETURN tmp[127:0]
/// }
/// dst[127:0] := MPSADBW_MASK(W[127:0], U[7:0], A[127:0], B[127:0], imm[2:0])
/// dst[255:128] := MPSADBW_MASK(W[255:128], U[15:8], A[255:128], B[255:128], imm[5:3])
/// dst[383:256] := MPSADBW_MASK(W[383:256], U[23:16], A[383:256], B[383:256], imm[2:0])
/// dst[512:384] := MPSADBW_MASK(W[512:384], U[31:24], A[512:384], B[512:384], imm[5:3])
/// \endcode
#define _mm512_mask_mpsadbw_epu8(W, U, A, B, imm) \
  (__m512i)__builtin_ia32_selectw_512((__mmask32)(U), \
                                      (__v32hi)_mm512_mpsadbw_epu8((A), (B), (imm)), \
                                      (__v32hi)(__m512i)(W))

/// Compute the sum of absolute differences (SADs) of quadruplets of unsigned
///    8-bit integers in \a A compared to those in \a B with mask, and store the
///    16-bit results in \a dst using zeromask \a U (elements are zeroed out
///    when the corresponding mask bit is not set). Eight SADs are performed for
///    each 128-bit lane using one quadruplet from \a B and eight quadruplets
///    from \a A. One quadruplet is selected from \a B starting at on the offset
///    specified in \a imm. Eight quadruplets are formed from sequential 8-bit
///    integers selected from \a A starting at the offset specified in \a imm.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// __m512i _mm512_maskz_mpsadbw_epu8(__mmask32 U, __m512i A, __m512i B, const int imm);
/// \endcode
///
/// This intrinsic corresponds to the \c VMPSADBW instruction.
///
/// \param U
///    A 32-bit mask value specifying what is chosen for each element.
/// \param A
///    A 512-bit vector of [64 x i8].
/// \param B
///    A 512-bit vector of [64 x i8].
/// \param imm
///    An 8-bit immediate operand specifying how the absolute differences are to
///    be calculated.
/// \returns A 512-bit integer vector containing the sums of the sets of
///    absolute differences between both operands.
///
/// \code{.operation}
/// DEFINE MPSADBW_MASKZ(U[7:0], A[127:0], B[127:0], imm[2:0]) {
/// 	a_offset := imm[2]*32
/// 	b_offset := imm[1:0]*32
/// 	FOR j := 0 to 7
/// 		i := j*8
/// 		IF U[j]
/// 			k := a_offset+i
/// 			l := b_offset
/// 			tmp[i*2+15:i*2] := ABS(Signed(A[k+7:k] - B[l+7:l])) + ABS(Signed(A[k+15:k+8] - B[l+15:l+8])) +
/// 			                   ABS(Signed(A[k+23:k+16] - B[l+23:l+16])) + ABS(Signed(A[k+31:k+24] - B[l+31:l+24]))
/// 		ELSE
/// 			tmp[i*2+15:i*2] := 0
/// 	ENDFOR
/// 	RETURN tmp[127:0]
/// }
/// dst[127:0] := MPSADBW_MASKZ(U[7:0], A[127:0], B[127:0], imm[2:0])
/// dst[255:128] := MPSADBW_MASKZ(U[15:8], A[255:128], B[255:128], imm[5:3])
/// dst[383:256] := MPSADBW_MASKZ(U[23:16], A[383:256], B[383:256], imm[2:0])
/// dst[512:384] := MPSADBW_MASKZ(U[31:24], A[512:384], B[512:384], imm[5:3])
/// \endcode
#define _mm512_maskz_mpsadbw_epu8(U, A, B, imm) \
  (__m512i)__builtin_ia32_selectw_512((__mmask32)(U), \
                                      (__v32hi)_mm512_mpsadbw_epu8((A), (B), (imm)), \
                                      (__v32hi)_mm512_setzero_si512())

#endif /* __AVX512MEDIAXINTRIN_H */
