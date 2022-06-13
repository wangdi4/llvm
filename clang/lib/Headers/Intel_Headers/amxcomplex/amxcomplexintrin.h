/*===------------- amxcomplexintrin.h - AMXCOMPLEX --------------------------=== */
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
#error "Never use <amxcomplexintrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AMX_COMPLEXINTRIN_H
#define __AMX_COMPLEXINTRIN_H
#ifdef __x86_64__

#define __DEFAULT_FN_ATTRS_COMPLEX                                            \
  __attribute__((__always_inline__, __nodebug__, __target__("amx-complex")))

/// Perform matrix multiplication of two tiles containing complex elements and
///    accumulate the results into a packed single precision tile. Each dword
///    element in input tiles \a a and \a b is interpreted as a complex number
///    with FP16 real part and FP16 imaginary part.
/// Calculates the imaginary part of the result. For each possible combination
///    of (row of \a a, column of \a b), it performs a set of multiplication
///    and accumulations on all corresponding complex numbers (one from \a a
///    and one from \a b). The imaginary part of the \a a element is multiplied
///    with the real part of the corresponding \a b element, and the real part
///    of the \a a element is multiplied with the imaginary part of the
///    corresponding \a b elements. The two accumulated results are added, and
///    then accumulated into the corresponding row and column of \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// void _tile_cmmimfp16ps(__tile dst, __tile a, __tile b);
/// \endcode
///
/// \code{.operation}
/// FOR m := 0 TO dst.rows - 1
///	tmp := dst.row[m]
///	FOR k := 0 TO (a.colsb / 4) - 1
///		FOR n := 0 TO (dst.colsb / 4) - 1
///			tmp.fp32[n] += FP32(a.row[m].fp16[2*k+0]) * FP32(b.row[k].fp16[2*n+1])
///			tmp.fp32[n] += FP32(a.row[m].fp16[2*k+1]) * FP32(b.row[k].fp16[2*n+0])
///		ENDFOR
///	ENDFOR
///	write_row_and_zero(dst, m, tmp, dst.colsb)
/// ENDFOR
/// zero_upper_rows(dst, dst.rows)
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TCMMIMFP16PS instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param a
///    The 1st source tile. Max size is 1024 Bytes.
/// \param b
///    The 2nd source tile. Max size is 1024 Bytes.
#define _tile_cmmimfp16ps(dst, a, b) \
  __builtin_ia32_tcmmimfp16ps(dst, a, b)

/// Perform matrix multiplication of two tiles containing complex elements and
///    accumulate the results into a packed single precision tile. Each dword
///    element in input tiles \a a and \a b is interpreted as a complex number
///    with FP16 real part and FP16 imaginary part.
/// Calculates the real part of the result. For each possible combination
///    of (row of \a a, column of \a b), it performs a set of multiplication
///    and accumulations on all corresponding complex numbers (one from \a a
///    and one from \a b). The real part of the \a a element is multiplied
///    with the real part of the corresponding \a b element, and the negated
///    imaginary part of the \a a element is multiplied with the imaginary
///    part of the corresponding \a b elements. The two accumulated results
///    are added, and then accumulated into the corresponding row and column
///    of \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// void _tile_cmmrlfp16ps(__tile dst, __tile a, __tile b);
/// \endcode
///
/// \code{.operation}
/// FOR m := 0 TO dst.rows - 1
///	tmp := dst.row[m]
///	FOR k := 0 TO (a.colsb / 4) - 1
///		FOR n := 0 TO (dst.colsb / 4) - 1
///			tmp.fp32[n] += FP32(a.row[m].fp16[2*k+0]) * FP32(b.row[k].fp16[2*n+0])
///			tmp.fp32[n] += FP32(-a.row[m].fp16[2*k+1]) * FP32(b.row[k].fp16[2*n+1])
///		ENDFOR
///	ENDFOR
///	write_row_and_zero(dst, m, tmp, dst.colsb)
/// ENDFOR
/// zero_upper_rows(dst, dst.rows)
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TCMMIMFP16PS instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param a
///    The 1st source tile. Max size is 1024 Bytes.
/// \param b
///    The 2nd source tile. Max size is 1024 Bytes.
#define _tile_cmmrlfp16ps(dst, a, b) \
  __builtin_ia32_tcmmrlfp16ps(dst, a, b)

/// Performs a matrix conjugate transpose and multiplication of two tiles
///    containing complex elements and accumulates the results into a packed
///    single precision tile. Each dword element in input tiles \a a and \a b
///    is interpreted as a complex number with FP16 real part and FP16 imaginary
///    part.
/// Calculates the imaginary part of the result. For each possible combination
///    of (transposed column of \a a, column of \a b), the instruction performs
///    a set of multiplication and accumulations on all corresponding complex
///    numbers (one from \a a and one from \a b). The negated imaginary part
///    of the \a a element is multiplied with the real part of the corresponding
///    \a b element, and the real part of the \a a element is multiplied with
///    the imaginary part of the corresponding \a b elements. The two
///    accumulated results are added, and then accumulated into the
///    corresponding row and column of \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// void _tile_conjtcmmimfp16ps(__tile dst, __tile a, __tile b);
/// \endcode
///
/// \code{.operation}
/// FOR m := 0 TO dst.rows - 1
///	tmp := dst.row[m]
///	FOR k := 0 TO (a.colsb / 4) - 1
///		FOR n := 0 TO (dst.colsb / 4) - 1
///			tmp.fp32[n] += FP32(a.row[k].fp16[2*m+0]) * FP32(b.row[k].fp16[2*n+1])
///			tmp.fp32[n] += FP32(-a.row[k].fp16[2*m+1]) * FP32(b.row[k].fp16[2*n+0])
///		ENDFOR
///	ENDFOR
///	write_row_and_zero(dst, m, tmp, dst.colsb)
/// ENDFOR
/// zero_upper_rows(dst, dst.rows)
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TCONJTCMMIMFP16PS instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param a
///    The 1st source tile. Max size is 1024 Bytes.
/// \param b
///    The 2nd source tile. Max size is 1024 Bytes.
#define _tile_conjtcmmimfp16ps(dst, a, b) \
  __builtin_ia32_tconjtcmmimfp16ps(dst, a, b)

/// Conjugate transpose FP16-pair complex elements from \a a and write the
///    result to \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// void _tile_conjtfp16(__tile dst, __tile a);
/// \endcode
///
/// \code{.operation}
/// FOR m := 0 TO dst.rows - 1
///	FOR n := 0 TO (dst.colsb / 4) - 1
///		tmp.dword[n].fp16[0] = a.row[n].dword[m].fp16[0]
///		tmp.dword[n].fp16[1] = -a.row[n].dword[m].fp16[1]
///	ENDFOR
///	write_row_and_zero(dst, m, tmp, dst.colsb)
/// ENDFOR
/// zero_upper_rows(dst, dst.rows)
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TCONJTFP16 instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param a
///    The 1st source tile. Max size is 1024 Bytes.
#define _tile_conjtfp16(dst, a) \
  __builtin_ia32_tconjtfp16(dst, a)

/// Perform matrix transpsoe and multiplication of two tiles containing complex
///    elements and accumulate the results into a packed single precision tile.
///    Each dword element in input tiles \a a and \a b is interpreted as a
///    complex number with FP16 real part and FP16 imaginary part.
/// Calculates the imaginary part of the result. For each possible combination
///    of (transposed column of \a a, column of \a b), the instruction performs
///    a set of multiplication and accumulations on all corresponding complex
///    numbers (one from \a a and one from \a b). The imaginary part of the
///    \a a element is multiplied with the real part of the corresponding \a b
///    element, and the real part of the \a a element is multiplied with the
///    imaginary part of the corresponding \a b elements. The two accumulated
///    results are added, and then accumulated into the corresponding row and
///    column of \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// void _tile_tcmmimfp16ps(__tile dst, __tile a, __tile b);
/// \endcode
///
/// \code{.operation}
/// FOR m := 0 TO dst.rows - 1
///	tmp := dst.row[m]
///	FOR k := 0 TO (a.colsb / 4) - 1
///		FOR n := 0 TO (dst.colsb / 4) - 1
///			tmp.fp32[n] += FP32(a.row[k].fp16[2*m+0]) * FP32(b.row[k].fp16[2*n+1])
///			tmp.fp32[n] += FP32(a.row[k].fp16[2*m+1]) * FP32(b.row[k].fp16[2*n+0])
///		ENDFOR
///	ENDFOR
///	write_row_and_zero(dst, m, tmp, dst.colsb)
/// ENDFOR
/// zero_upper_rows(dst, dst.rows)
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TTCMMIMFP16PS instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param a
///    The 1st source tile. Max size is 1024 Bytes.
/// \param b
///    The 2nd source tile. Max size is 1024 Bytes.
#define _tile_tcmmimfp16ps(dst, a, b) \
  __builtin_ia32_ttcmmimfp16ps(dst, a, b)

/// Perform matrix transpsoe and multiplication of two tiles containing complex
///    elements and accumulate the results into a packed single precision tile.
///    Each dword element in input tiles \a a and \a b is interpreted as a
///    complex number with FP16 real part and FP16 imaginary part.
/// Calculates the real part of the result. For each possible combination of
///    (transposed column of \a a, column of \a b), the instruction performs a
///    set of multiplication and accumulations on all corresponding complex
///    numbers (one from \a a and one from \a b). The real part of the \a a
///    element is multiplied with the real part of the corresponding \a b
///    element, and the negated imaginary part of the \a a element is multiplied
///    with the imaginary part of the corresponding \a b elements. The two
///    accumulated results are added, and then accumulated into the
///    corresponding row and column of \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// void _tile_tcmmrlfp16ps(__tile dst, __tile a, __tile b);
/// \endcode
///
/// \code{.operation}
/// FOR m := 0 TO dst.rows - 1
///	tmp := dst.row[m]
///	FOR k := 0 TO (a.colsb / 4) - 1
///		FOR n := 0 TO (dst.colsb / 4) - 1
///			tmp.fp32[n] += FP32(a.row[k].fp16[2*m+0]) * FP32(b.row[k].fp16[2*n+0])
///			tmp.fp32[n] += FP32(-a.row[k].fp16[2*m+1]) * FP32(b.row[k].fp16[2*n+1])
///		ENDFOR
///	ENDFOR
///	write_row_and_zero(dst, m, tmp, dst.colsb)
/// ENDFOR
/// zero_upper_rows(dst, dst.rows)
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TTCMMIMFP16PS instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param a
///    The 1st source tile. Max size is 1024 Bytes.
/// \param b
///    The 2nd source tile. Max size is 1024 Bytes.
#define _tile_tcmmrlfp16ps(dst, a, b) \
  __builtin_ia32_ttcmmrlfp16ps(dst, a, b)

static __inline__ _tile1024i __DEFAULT_FN_ATTRS_COMPLEX
_tile_cmmimfp16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_tcmmimfp16ps_internal(m, n, k, dst, src1, src2);
}

static __inline__ _tile1024i __DEFAULT_FN_ATTRS_COMPLEX
_tile_cmmrlfp16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_tcmmrlfp16ps_internal(m, n, k, dst, src1, src2);
}

static __inline__ _tile1024i __DEFAULT_FN_ATTRS_COMPLEX
_tile_conjtcmmimfp16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_tconjtcmmimfp16ps_internal(m, n, k, dst, src1, src2);
}

static __inline__ _tile1024i __DEFAULT_FN_ATTRS_COMPLEX
_tile_conjtfp16_internal(unsigned short m, unsigned short n, _tile1024i src) {
  return __builtin_ia32_tconjtfp16_internal(m, n, src);
}

// dst = m x n (srcdest), src1 = k x m, src2 = k x n
static __inline__ _tile1024i __DEFAULT_FN_ATTRS_COMPLEX
_tile_tcmmimfp16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_ttcmmimfp16ps_internal(m, n, k, dst, src1, src2);
}

// dst = m x n (srcdest), src1 = k x m, src2 = k x n
static __inline__ _tile1024i __DEFAULT_FN_ATTRS_COMPLEX
_tile_tcmmrlfp16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_ttcmmrlfp16ps_internal(m, n, k, dst, src1, src2);
}

/// Perform matrix multiplication of two tiles containing complex elements and
/// accumulate the results into a packed single precision tile. Each dword
/// element in input tiles src0 and src1 is interpreted as a complex number with
/// FP16 real part and FP16 imaginary part.
/// This function calculates the imaginary part of the result.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCMMIMFP16PS </c> instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source tile. Max size is 1024 Bytes.
__DEFAULT_FN_ATTRS_COMPLEX
static void __tile_cmmimfp16ps(__tile1024i *dst, __tile1024i src0,
                               __tile1024i src1) {
  dst->tile = _tile_cmmimfp16ps_internal(src0.row, src1.col, src0.col,
                                         dst->tile, src0.tile, src1.tile);
}

/// Perform matrix multiplication of two tiles containing complex elements and
/// accumulate the results into a packed single precision tile. Each dword
/// element in input tiles src0 and src1 is interpreted as a complex number with
/// FP16 real part and FP16 imaginary part.
/// This function calculates the real part of the result.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TCMMRLFP16PS </c> instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source tile. Max size is 1024 Bytes.
__DEFAULT_FN_ATTRS_COMPLEX
static void __tile_cmmrlfp16ps(__tile1024i *dst, __tile1024i src0,
                               __tile1024i src1) {
  dst->tile = _tile_cmmrlfp16ps_internal(src0.row, src1.col, src0.col,
                                         dst->tile, src0.tile, src1.tile);
}

/// Performs a matrix conjugate transpose and multiplication of two tiles
/// containing complex elements and accumulates the results into a packed
/// single precision tile. Each dword element in input tiles src0 and src1
/// is interpreted as a complex number with FP16 real part and FP16 imaginary
/// part. TCONJTCMMIMFP16PS calculates the imaginary part of the result. For
/// each possible combination of (transposed column of src0, column of src1),
/// the instruction performs a set of multiplication and accumulations on all
/// corresponding complex numbers (one from src0 and one from src1). The negated
/// imaginary part of the src0 element is multiplied with the real part of the
/// corresponding src1 element, and the real part of the src0 element is
/// multiplied with the imaginary part of the corresponding src1 elements. The
/// two accumulated results are added, and then accumulated into the
/// corresponding row and column of dst.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> tconjtcmmimfp16ps </c> instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source tile. Max size is 1024 Bytes.
__DEFAULT_FN_ATTRS_COMPLEX
static void __tile_conjtcmmimfp16ps(__tile1024i *dst, __tile1024i src0,
                                    __tile1024i src1) {
  dst->tile = _tile_conjtcmmimfp16ps_internal(src0.col / 4, src1.col,
                           src0.row * 4, dst->tile, src0.tile, src1.tile);
}

/// Conjugate transpose FP16-pair complex elements from src and write
/// the result to dst.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> tconjtfp16 </c> instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param src
///    The 1st source tile. Max size is 1024 Bytes.
__DEFAULT_FN_ATTRS_COMPLEX
static void __tile_conjtfp16(__tile1024i *dst, __tile1024i src) {
  dst->tile = _tile_conjtfp16_internal(src.row, src.col, src.tile);
}

/// Perform matrix transpsoe and multiplication of two tiles containing complex
/// elements and accumulate the results into a packed single precision tile.
/// Each dword element in input tiles src0 and src1 is interpreted as a complex
/// number with FP16 real part and FP16 imaginary part.
/// This function calculates the imaginary part of the result.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TTCMMIMFP16PS </c> instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source tile. Max size is 1024 Bytes.
__DEFAULT_FN_ATTRS_COMPLEX
static void __tile_tcmmimfp16ps(__tile1024i *dst, __tile1024i src0,
                               __tile1024i src1) {
  dst->tile = _tile_tcmmimfp16ps_internal(src0.col / 4, src1.col,
                           src0.row * 4, dst->tile, src0.tile, src1.tile);
}

/// Perform matrix transpsoe and multiplication of two tiles containing complex
/// elements and accumulate the results into a packed single precision tile.
/// Each dword element in input tiles src0 and src1 is interpreted as a complex
/// number with FP16 real part and FP16 imaginary part.
/// This function calculates the real part of the result.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TTCMMRLFP16PS </c> instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source tile. Max size is 1024 Bytes.
__DEFAULT_FN_ATTRS_COMPLEX
static void __tile_tcmmrlfp16ps(__tile1024i *dst, __tile1024i src0,
                               __tile1024i src1) {
  dst->tile = _tile_tcmmrlfp16ps_internal(src0.col / 4, src1.col,
                           src0.row * 4, dst->tile, src0.tile, src1.tile);
}

#endif // __x86_64__
#endif // __AMX_COMPLEXINTRIN_H
