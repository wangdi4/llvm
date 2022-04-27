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

#define _tile_tcmmimfp16ps(tdst, tsrc1, tsrc2) \
  __builtin_ia32_tcmmimfp16ps(tdst, tsrc1, tsrc2)
#define _tile_tcmmrlfp16ps(tdst, tsrc1, tsrc2) \
  __builtin_ia32_tcmmrlfp16ps(tdst, tsrc1, tsrc2)
#define _tile_tconjtcmmimfp16ps(tdst, tsrc1, tsrc2) \
  __builtin_ia32_tconjtcmmimfp16ps(tdst, tsrc1, tsrc2)
#define _tile_tconjtfp16(tdst, tsrc1) \
  __builtin_ia32_tconjtfp16(tdst, tsrc1)
#define _tile_ttcmmimfp16ps(tdst, tsrc1, tsrc2) \
  __builtin_ia32_ttcmmimfp16ps(tdst, tsrc1, tsrc2)
#define _tile_ttcmmrlfp16ps(tdst, tsrc1, tsrc2) \
  __builtin_ia32_ttcmmrlfp16ps(tdst, tsrc1, tsrc2)

static __inline__ _tile1024i __DEFAULT_FN_ATTRS_COMPLEX
_tile_tcmmimfp16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_tcmmimfp16ps_internal(m, n, k, dst, src1, src2);
}

static __inline__ _tile1024i __DEFAULT_FN_ATTRS_COMPLEX
_tile_tcmmrlfp16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_tcmmrlfp16ps_internal(m, n, k, dst, src1, src2);
}

static __inline__ _tile1024i __DEFAULT_FN_ATTRS_COMPLEX
_tile_tconjtcmmimfp16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_tconjtcmmimfp16ps_internal(m, n, k, dst, src1, src2);
}

static __inline__ _tile1024i __DEFAULT_FN_ATTRS_COMPLEX
_tile_tconjtfp16_internal(unsigned short m, unsigned short n, _tile1024i src) {
  return __builtin_ia32_tconjtfp16_internal(m, n, src);
}

// dst = m x n (srcdest), src1 = k x m, src2 = k x n
static __inline__ _tile1024i __DEFAULT_FN_ATTRS_COMPLEX
_tile_ttcmmimfp16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_ttcmmimfp16ps_internal(m, n, k, dst, src1, src2);
}

// dst = m x n (srcdest), src1 = k x m, src2 = k x n
static __inline__ _tile1024i __DEFAULT_FN_ATTRS_COMPLEX
_tile_ttcmmrlfp16ps_internal(unsigned short m, unsigned short n, unsigned short k,
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
static void __tile_tcmmimfp16ps(__tile1024i *dst, __tile1024i src0,
                               __tile1024i src1) {
  dst->tile = _tile_tcmmimfp16ps_internal(src0.row, src1.col, src0.col,
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
static void __tile_tcmmrlfp16ps(__tile1024i *dst, __tile1024i src0,
                               __tile1024i src1) {
  dst->tile = _tile_tcmmrlfp16ps_internal(src0.row, src1.col, src0.col,
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
static void __tile_tconjtcmmimfp16ps(__tile1024i *dst, __tile1024i src0,
                                    __tile1024i src1) {
  dst->tile = _tile_tconjtcmmimfp16ps_internal(src0.col / 4, src1.col,
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
static void __tile_tconjtfp16(__tile1024i *dst, __tile1024i src) {
  dst->tile = _tile_tconjtfp16_internal(src.row, src.col, src.tile);
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
static void __tile_ttcmmimfp16ps(__tile1024i *dst, __tile1024i src0,
                               __tile1024i src1) {
  dst->tile = _tile_ttcmmimfp16ps_internal(src0.col / 4, src1.col,
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
static void __tile_ttcmmrlfp16ps(__tile1024i *dst, __tile1024i src0,
                               __tile1024i src1) {
  dst->tile = _tile_ttcmmrlfp16ps_internal(src0.col / 4, src1.col,
                           src0.row * 4, dst->tile, src0.tile, src1.tile);
}

#endif // __x86_64__
#endif // __AMX_COMPLEXINTRIN_H
