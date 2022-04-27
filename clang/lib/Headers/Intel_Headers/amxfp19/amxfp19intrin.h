/*===------------------ amxfp19intrin.h - AMXFP19 ---------------------------=== */
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
#error "Never use <amxfp19intrin.h> directly; include <immintrin.h> instead."
#endif // __IMMINTRIN_H

#ifndef __AMX_FP19INTRIN_H
#define __AMX_FP19INTRIN_H
#ifdef __x86_64__

#define __DEFAULT_FN_ATTRS_FP19                                                \
  __attribute__((__always_inline__, __nodebug__, __target__("amx-fp19")))

#define _tile_tmmulfp19ps(tdst, tsrc1, tsrc2)                                  \
  __builtin_ia32_tmmulfp19ps(tdst, tsrc1, tsrc2)
#define _tile_ttmmulfp19ps(tdst, tsrc1, tsrc2)                                 \
  __builtin_ia32_ttmmulfp19ps(tdst, tsrc1, tsrc2)

static __inline__ _tile1024i __DEFAULT_FN_ATTRS_FP19
_tile_tmmulfp19ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_tmmulfp19ps_internal(m, n, k, dst, src1, src2);
}

/// Do Matrix Multiplication of src0 and src1, and then do Matrix Plus with dst.
/// All the calculation is base on float32 but with the lower 13-bit set to 0.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TMMULFP19PS </c> instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source tile. Max size is 1024 Bytes.
__DEFAULT_FN_ATTRS_FP19
static void __tile_tmmulfp19ps(__tile1024i *dst, __tile1024i src0,
                               __tile1024i src1) {
  dst->tile = _tile_tmmulfp19ps_internal(src0.row, src1.col, src0.col,
                                         dst->tile, src0.tile, src1.tile);
}

// dst = m x n (srcdest), src1 = k x m, src2 = k x n
static __inline__ _tile1024i __DEFAULT_FN_ATTRS_FP19
_tile_ttmmulfp19ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_ttmmulfp19ps_internal(m, n, k, dst, src1, src2);
}

/// Compute transpose and do Matrix Multiplication of src0 and src1, and then do
/// Matrix Plus with dst. All the calculation is base on float32 but with the
/// lower 13-bit set to 0.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TTMMULFP19PS </c> instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source tile. Max size is 1024 Bytes.
__DEFAULT_FN_ATTRS_FP19
static void __tile_ttmmulfp19ps(__tile1024i *dst, __tile1024i src0,
                               __tile1024i src1) {
  dst->tile = _tile_ttmmulfp19ps_internal(src0.row, src1.col, src0.col,
                                         dst->tile, src0.tile, src1.tile);
}

#endif // __x86_64__
#endif // __AMX_FP19INTRIN_H
