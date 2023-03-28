/* INTEL_FEATURE_ISA_AMX_TRANSPOSE */
/*===---------- Intel_amxtransposeintrin.h - AMX_TRANSPOSE intrinsics -*- C++ -*---------===
*
* Copyright (C) 2019 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amxtransposeintrin.h> directly; use <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMX_TRANSPOSEINTRIN_H
#define __AMX_TRANSPOSEINTRIN_H
#ifdef __x86_64__

#define __DEFAULT_FN_ATTRS_TRANSPOSE                                           \
  __attribute__((__always_inline__, __nodebug__, __target__("amx-transpose")))

// Transpose
#define _tile_2rpntlvw(tdst, base, stride, src)                                \
  __builtin_ia32_t2rpntlvw(tdst, base, stride, src)
#define _tile_2rpntlvwt1(tdst, base, stride, src)                              \
  __builtin_ia32_t2rpntlvwt1(tdst, base, stride, src)
#define _tile_2rpntlvwz0(tdst, base, stride)                                   \
  __builtin_ia32_t2rpntlvwz0(tdst, base, stride)
#define _tile_2rpntlvwz0t1(tdst, base, stride)                                 \
  __builtin_ia32_t2rpntlvwz0t1(tdst, base, stride)
#define _tile_2rpntlvwz1(tdst, base, stride)                                   \
  __builtin_ia32_t2rpntlvwz1(tdst, base, stride)
#define _tile_2rpntlvwz1t1(tdst, base, stride)                                 \
  __builtin_ia32_t2rpntlvwz1t1(tdst, base, stride)

/// Compute transpose and dot product of BF16 pairs, accumulating to single
/// precision. This function has 3 tile operands, one source/dest accumulator
/// operand \a srcdst and two source operands, \a a and \a b. \a a is
/// transposed and matrix multiplied with \a b. The transpose operation is
/// done in BF16 pair granularity, transforming columns of BF16 pairs into rows.
/// The tile registers specified must be distinct from one another, no repeats.
/// Any attempt to execute an AMX instruction inside a TSX transaction will
/// result in a transaction abort.
///
/// \headerfile <immintrin.h>
///
/// \code
/// void __tile_tdpbf16ps(__tile srcdst, __tile a, __tile b);
/// \endcode
///
/// This intrinsic corresponds to the <c> TTDPBF16PS </c> instruction.
///
/// \param srcdst
/// 	The destination tile. Max size is 1024 Bytes.
/// \param a
/// 	The 1st source tile. Max size is 1024 Bytes.
/// \param b
/// 	The 2nd source tile. Max size is 1024 Bytes.
///
/// \code{.operation}
/// elements_dest:= srcdst.colsb/4
///
/// FOR m := 0 TO (srcdst.rows-1)
/// 	tmp[511:0] := 0
/// 	FOR k := 0 TO (a.rows-1)
/// 		FOR n := 0 TO (elements_dest-1)
/// 			tmp.fp32[2*n+0] += FP32(a.row[k].bfloat16[2*m+0]) * FP32(b.row[k].bfloat16[2*n+0])
/// 			tmp.fp32[2*n+1] += FP32(a.row[k].bfloat16[2*m+1]) * FP32(b.row[k].bfloat16[2*n+1])
/// 		ENDFOR
/// 	ENDFOR
/// 	
/// 	FOR n := 0 TO (elements_dest-1)
/// 		tmp2.f32[n] := tmp.fp32[2*n+0] + tmp.fp32[2*n+1]
/// 		tmp2.f32[n] += srcdst.row[m].fp32[n]
/// 	ENDFOR
/// 	write_row_and_zero(srcdst, m, tmp2, srcdst.colsb)
///
/// ENDFOR
///
/// zero_upper_rows(srcdst, srcdst.rows)
/// zero_tileconfig_start()
/// \endcode
#define _tile_tdpbf16ps(srcdst, a, b)                               \
  __builtin_ia32_ttdpbf16ps(srcdst, a, b)

/// Compute transpose and dot product of FP16 pairs, accumulating to single
/// precision. This function has 3 tile operands, one source/dest accumulator
/// operand srcdst and two source operands, a and b. a is transposed
/// and matrix multiplied with b. The transpose operation is done in FP16
/// pair granularity, transforming columns of FP16 pairs into rows. The tile
/// registers specified must be distinct from one another, no repeats. Any
/// attempt to execute an AMX instruction inside a TSX transaction will result
/// in a transaction abort.
///
/// \headerfile <immintrin.h>
///
/// \code
/// void __tile_tdpfp16ps(__tile srcdst, __tile a, __tile b);
/// \endcode
///
/// This intrinsic corresponds to the <c> TTDPFP16PS </c> instruction.
///
/// \param srcdst
/// 	The destination tile. Max size is 1024 Bytes.
/// \param a
/// 	The 1st source tile. Max size is 1024 Bytes.
/// \param b
/// 	The 2nd source tile. Max size is 1024 Bytes.
///
/// \code{.operation}
/// elements_dest:= srcdst.colsb/4
///
/// FOR m := 0 TO (srcdst.rows-1)
/// 	tmp[511:0] := 0
/// 	FOR k := 0 TO (a.rows-1)
/// 		FOR n := 0 TO (elements_dest-1)
/// 			tmp.fp32[2*n+0] += FP32(a.row[k].fp16[2*m+0]) * FP32(b.row[k].fp16[2*n+0])
/// 			tmp.fp32[2*n+1] += FP32(a.row[k].fp16[2*m+1]) * FP32(b.row[k].fp16[2*n+1])
/// 		ENDFOR
/// 	ENDFOR
/// 	
/// 	FOR n := 0 TO (elements_dest-1)
/// 		tmp2.f32[n] := tmp.fp32[2*n+0] + tmp.fp32[2*n+1]
/// 		tmp2.f32[n] += srcdst.row[m].fp32[n]
/// 	ENDFOR
/// 	write_row_and_zero(srcdst, m, tmp2, srcdst.colsb)
/// 	
/// ENDFOR
///
/// zero_upper_rows(srcdst, srcdst.rows)
/// zero_tileconfig_start()
/// \endcode
#define _tile_tdpfp16ps(srcdst, a, b)                                    \
  __builtin_ia32_ttdpfp16ps(srcdst, a, b)

/// Transpose 32-bit elements from \a src and write the result to \a dst.
///
/// \headerfile <immintrin.h>
///
/// \code
/// void __tile_transposed(__tile dst, __tile src);
/// \endcode
///
/// This intrinsic corresponds to the <c> TTRANSPOSED </c> instruction.
///
/// \param dst
/// 	The destination tile. Max size is 1024 Bytes.
/// \param src
/// 	The 1st source tile. Max size is 1024 Bytes.
///
/// \code{.operation}
///
/// FOR i := 0 TO (dst.rows-1)
/// 	tmp[511:0] := 0
/// 	FOR j := 0 TO (dst.colsb/4-1)
/// 		tmp.dword[j] := src.row[j].dword[i]
/// 	ENDFOR
/// 	dst.row[i] := tmp
/// ENDFOR
///
/// zero_upper_rows(dst, dst.rows)
/// zero_tileconfig_start()
/// \endcode
#define _tile_transposed(dst, src)                                          \
  __builtin_ia32_ttransposed(dst, src)

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

// BF16EVEX
#define _tile_dpbf16pse(tsrc1_dst, tsrc2, tsrc3)                               \
  __builtin_ia32_tdpbf16pse(tsrc1_dst, tsrc2, tsrc3)

// INT8EVEX
#define _tile_dpbssde(tsrc1_dst, tsrc2, tsrc3)                                 \
  __builtin_ia32_tdpbssde(tsrc1_dst, tsrc2, tsrc3)
#define _tile_dpbsude(tsrc1_dst, tsrc2, tsrc3)                                 \
  __builtin_ia32_tdpbsude(tsrc1_dst, tsrc2, tsrc3)
#define _tile_dpbusde(tsrc1_dst, tsrc2, tsrc3)                                 \
  __builtin_ia32_tdpbusde(tsrc1_dst, tsrc2, tsrc3)
#define _tile_dpbuude(tsrc1_dst, tsrc2, tsrc3)                                 \
  __builtin_ia32_tdpbuude(tsrc1_dst, tsrc2, tsrc3)

// TILEEVEX
#define _tile_loadde(dst, base, stride)  __builtin_ia32_tileloadde64((dst),    \
  ((const void *)(base)), (__SIZE_TYPE__)(stride))
#define _tile_stream_loadde(dst, base, stride)                                 \
  __builtin_ia32_tileloaddt1e64((dst), ((const void *)(base)),                 \
  (__SIZE_TYPE__)(stride))
#define _tile_storede(src, base, stride)                                       \
  __builtin_ia32_tilestorede64((src), ((void *)(base)),                        \
  (__SIZE_TYPE__)(stride))
#define _tile_tilemove(tdst, tsrc)  __builtin_ia32_tilemove(tdst, tsrc)
#define _tile_zeroe(tile)       __builtin_ia32_tilezeroe(tile)

//AMX_TRANSPOSE ELEMENTEVEX
#define _tile_cvtd2pse(base, stride, tsrc)                                    \
  __builtin_ia32_tcvtd2pse(base, stride, tsrc)

static __inline__ void __DEFAULT_FN_ATTRS_TRANSPOSE
_tile_t2rpntlvwz0_internal(unsigned short row, unsigned short col0,
                           unsigned short col1, _tile1024i *dst0,
                           _tile1024i *dst1, const void *base,
                           __SIZE_TYPE__ stride) {
  // Use __tile1024i_1024a* to escape the alignment check in
  // clang/test/Headers/x86-intrinsics-headers-clean.cpp
  __builtin_ia32_t2rpntlvwz0_internal(row, col0, col1,
                                      (_tile1024i_1024a*)dst0,
                                      (_tile1024i_1024a*)dst1,
                                      base,
                                      (__SIZE_TYPE__)(stride));
}

static __inline__ void __DEFAULT_FN_ATTRS_TRANSPOSE
_tile_t2rpntlvwz0t1_internal(unsigned short row, unsigned short col0,
                             unsigned short col1, _tile1024i *dst0,
                             _tile1024i *dst1, const void *base,
                             __SIZE_TYPE__ stride) {
  __builtin_ia32_t2rpntlvwz0t1_internal(row, col0, col1,
                                        (_tile1024i_1024a*)dst0,
                                        (_tile1024i_1024a*)dst1,
                                        base,
                                        (__SIZE_TYPE__)(stride));
}

static __inline__ void __DEFAULT_FN_ATTRS_TRANSPOSE
_tile_t2rpntlvwz1_internal(unsigned short row, unsigned short col0,
                           unsigned short col1, _tile1024i *dst0,
                           _tile1024i *dst1, const void *base,
                           __SIZE_TYPE__ stride) {
  __builtin_ia32_t2rpntlvwz1_internal(row, col0, col1,
                                      (_tile1024i_1024a*)dst0,
                                      (_tile1024i_1024a*)dst1,
                                      base,
                                      (__SIZE_TYPE__)(stride));
}

static __inline__ void __DEFAULT_FN_ATTRS_TRANSPOSE
_tile_t2rpntlvwz1t1_internal(unsigned short row, unsigned short col0,
                             unsigned short col1, _tile1024i *dst0,
                             _tile1024i *dst1, const void *base,
                             __SIZE_TYPE__ stride) {
  __builtin_ia32_t2rpntlvwz1t1_internal(row, col0, col1,
                                        (_tile1024i_1024a*)dst0,
                                        (_tile1024i_1024a*)dst1,
                                        base,
                                        (__SIZE_TYPE__)(stride));
}

// This is internal intrinsic. C/C++ user should avoid calling it directly.
static __inline__ _tile1024i __DEFAULT_FN_ATTRS_TRANSPOSE
_tile_transposed_internal(unsigned short m, unsigned short n, _tile1024i src) {
  return __builtin_ia32_ttransposed_internal(m, n, src);
}

// dst = m x n (srcdest), src1 = k x m, src2 = k x n
static __inline__ _tile1024i __DEFAULT_FN_ATTRS_TRANSPOSE
_tile_tdpbf16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                         _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_ttdpbf16ps_internal(m, n, k, dst, src1, src2);
}

// dst = m x n (srcdest), src1 = k x m, src2 = k x n
static __inline__ _tile1024i __DEFAULT_FN_ATTRS_TRANSPOSE
_tile_tdpfp16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                         _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_ttdpfp16ps_internal(m, n, k, dst, src1, src2);
}

static __inline__ _tile1024i __DEFAULT_FN_ATTRS_TRANSPOSE
_tile_conjtcmmimfp16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_tconjtcmmimfp16ps_internal(m, n, k, dst, src1, src2);
}

static __inline__ _tile1024i __DEFAULT_FN_ATTRS_TRANSPOSE
_tile_conjtfp16_internal(unsigned short m, unsigned short n, _tile1024i src) {
  return __builtin_ia32_tconjtfp16_internal(m, n, src);
}

// dst = m x n (srcdest), src1 = k x m, src2 = k x n
static __inline__ _tile1024i __DEFAULT_FN_ATTRS_TRANSPOSE
_tile_tcmmimfp16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_ttcmmimfp16ps_internal(m, n, k, dst, src1, src2);
}

// dst = m x n (srcdest), src1 = k x m, src2 = k x n
static __inline__ _tile1024i __DEFAULT_FN_ATTRS_TRANSPOSE
_tile_tcmmrlfp16ps_internal(unsigned short m, unsigned short n, unsigned short k,
                          _tile1024i dst, _tile1024i src1, _tile1024i src2) {
  return __builtin_ia32_ttcmmrlfp16ps_internal(m, n, k, dst, src1, src2);
}

/// Converts a pair of tiles from memory into VNNI format, and places the
/// results in a pair of destinations specified by dst. The pair of tiles
/// in memory is specified via a tsib; the second tile is after the first
/// one, separated by the same stride that separates each row.
/// The tile configuration for the destination tiles indicates the amount
/// of data to read from memory. The instruction will load a number of rows
/// that is equal to twice the number of rows in tmm1. The size of each row
/// is equal to the average width of the destination tiles. If the second
/// tile is configured with zero rows and columns, only the first tile will
/// be written.
/// Provides a hint to the implementation that the data will likely not be
/// reused in the near future and the data caching can be optimized.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> T2RPNTLVWZ0 </c> instruction.
///
/// \param dst0
///    First tile of destination tile pair. Max size is 1024i*2 Bytes.
/// \param dst1
///    Second tile of destination tile pair. Max size is 1024i*2 Bytes.
/// \param base
///    A pointer to base address.
/// \param stride
///    The stride between the rows' data to be loaded in memory.
__DEFAULT_FN_ATTRS_TRANSPOSE
static void __tile_t2rpntlvwz0(__tile1024i *dst0, __tile1024i *dst1,
                               const void *base,
                               __SIZE_TYPE__ stride) {
  _tile_t2rpntlvwz0_internal(dst0->row, dst0->col, dst1->col,
                             &dst0->tile, &dst1->tile, base, stride);
}

/// Converts a pair of tiles from memory into VNNI format, and places the
/// results in a pair of destinations specified by dst. The pair of tiles
/// in memory is specified via a tsib; the second tile is after the first
/// one, separated by the same stride that separates each row.
/// The tile configuration for the destination tiles indicates the amount
/// of data to read from memory. The instruction will load a number of rows
/// that is equal to twice the number of rows in tmm1. The size of each row
/// is equal to the average width of the destination tiles. If the second
/// tile is configured with zero rows and columns, only the first tile will
/// be written.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> T2RPNTLVWZ0T1 </c> instruction.
///
/// \param dst0
///    First tile of destination tile pair. Max size is 1024i*2 Bytes.
/// \param dst1
///    Second tile of destination tile pair. Max size is 1024i*2 Bytes.
/// \param base
///    A pointer to base address.
/// \param stride
///    The stride between the rows' data to be loaded in memory.
__DEFAULT_FN_ATTRS_TRANSPOSE
static void __tile_t2rpntlvwz0t1(__tile1024i *dst0, __tile1024i *dst1,
                                 const void *base,
                                 __SIZE_TYPE__ stride) {
  _tile_t2rpntlvwz0t1_internal(dst0->row, dst0->col, dst1->col,
                               &dst0->tile, &dst1->tile, base, stride);
}

/// Converts a pair of tiles from memory into VNNI format, and places the
/// results in a pair of destinations specified by dst. The pair of tiles
/// in memory is specified via a tsib; the second tile is after the first
/// one, separated by the same stride that separates each row.
/// The tile configuration for the destination tiles indicates the amount
/// of data to read from memory. The instruction will load a number of rows
/// that is equal to twice the number of rows in tmm1. The size of each row
/// is equal to the average width of the destination tiles. If the second
/// tile is configured with zero rows and columns, only the first tile will
/// be written. The last row will be not be read from memory but instead
/// filled with zeros.
/// Provides a hint to the implementation that the data will likely not be
/// reused in the near future and the data caching can be optimized.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> T2RPNTLVWZ1 </c> instruction.
///
/// \param dst0
///    First tile of destination tile pair. Max size is 1024i*2 Bytes.
/// \param dst1
///    Second tile of destination tile pair. Max size is 1024i*2 Bytes.
/// \param base
///    A pointer to base address.
/// \param stride
///    The stride between the rows' data to be loaded in memory.
__DEFAULT_FN_ATTRS_TRANSPOSE
static void __tile_t2rpntlvwz1(__tile1024i *dst0, __tile1024i *dst1,
                               const void *base,
                               __SIZE_TYPE__ stride) {
  _tile_t2rpntlvwz1_internal(dst0->row, dst0->col, dst1->col,
                             &dst0->tile, &dst1->tile, base, stride);
}

/// Converts a pair of tiles from memory into VNNI format, and places the
/// results in a pair of destinations specified by dst. The pair of tiles
/// in memory is specified via a tsib; the second tile is after the first
/// one, separated by the same stride that separates each row.
/// The tile configuration for the destination tiles indicates the amount
/// of data to read from memory. The instruction will load a number of rows
/// that is equal to twice the number of rows in tmm1. The size of each row
/// is equal to the average width of the destination tiles. If the second
/// tile is configured with zero rows and columns, only the first tile will
/// be written. The last row will be not be read from memory but instead
/// filled with zeros.
/// Provides a hint to the implementation that the data will likely not be
/// reused in the near future and the data caching can be optimized.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> T2RPNTLVWZ1T1 </c> instruction.
///
/// \param dst0
///    First tile of destination tile pair. Max size is 1024i*2 Bytes.
/// \param dst1
///    Second tile of destination tile pair. Max size is 1024i*2 Bytes.
/// \param base
///    A pointer to base address.
/// \param stride
///    The stride between the rows' data to be loaded in memory.
__DEFAULT_FN_ATTRS_TRANSPOSE
static void __tile_t2rpntlvwz1t1(__tile1024i *dst0, __tile1024i *dst1,
                                 const void *base,
                                 __SIZE_TYPE__ stride) {
  _tile_t2rpntlvwz1t1_internal(dst0->row, dst0->col, dst1->col,
                               &dst0->tile, &dst1->tile, base, stride);
}

/// Transpose 32-bit elements from src and write the result to dst.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TTRANSPOSED </c> instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param src
///    The 1st source tile. Max size is 1024 Bytes.
__DEFAULT_FN_ATTRS_TRANSPOSE
static void __tile_transposed(__tile1024i *dst, __tile1024i src) {
  dst->tile = _tile_transposed_internal(src.row, src.col, src.tile);
}

/// Compute transpose and dot product of BF16 pairs, accumulating to single
/// precision. This function has 3 tile operands, one source/dest accumulator
/// operand dst and two source operands, src0 and src1. Src0 is transposed and
/// matrix multiplied with src1. The transpose operation is done in BF16 pair
/// granularity, transforming columns of BF16 pairs into rows. The tile
/// registers specified must be distinct from one another, no repeats. Any
/// attempt to execute an AMX instruction inside a TSX transaction will result
/// in a transaction abort.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TTDPBF16PS </c> instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source tile. Max size is 1024 Bytes.
__DEFAULT_FN_ATTRS_TRANSPOSE
static void __tile_tdpbf16ps(__tile1024i *dst, __tile1024i src0,
                             __tile1024i src1) {
  dst->tile = _tile_tdpbf16ps_internal(src0.col / 4, src1.col,
                           src1.row * 4, dst->tile, src0.tile, src1.tile);
}

/// Compute transpose and dot product of FP16 pairs, accumulating to single
/// precision. This function has 3 tile operands, one source/dest accumulator
/// operand dst and two source operands, src0 and src1. Src0 is transposed and
/// matrix multiplied with src1. The transpose operation is done in FP16 pair
/// granularity, transforming columns of FP16 pairs into rows. The tile
/// registers specified must be distinct from one another, no repeats. Any
/// attempt to execute an AMX instruction inside a TSX transaction will result
/// in a transaction abort.
///
/// \headerfile <immintrin.h>
///
/// This intrinsic corresponds to the <c> TTDPFP16PS </c> instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source tile. Max size is 1024 Bytes.
__DEFAULT_FN_ATTRS_TRANSPOSE
static void __tile_tdpfp16ps(__tile1024i *dst, __tile1024i src0,
                             __tile1024i src1) {
  dst->tile = _tile_tdpfp16ps_internal(src0.col / 4, src1.col,
                           src1.row * 4, dst->tile, src0.tile, src1.tile);
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
__DEFAULT_FN_ATTRS_TRANSPOSE
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
__DEFAULT_FN_ATTRS_TRANSPOSE
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
__DEFAULT_FN_ATTRS_TRANSPOSE
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
__DEFAULT_FN_ATTRS_TRANSPOSE
static void __tile_tcmmrlfp16ps(__tile1024i *dst, __tile1024i src0,
                               __tile1024i src1) {
  dst->tile = _tile_tcmmrlfp16ps_internal(src0.col / 4, src1.col,
                           src0.row * 4, dst->tile, src0.tile, src1.tile);
}

#endif /* __x86_64__ */
#endif /* __AMX_TRANSPOSEINTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_TRANSPOSE */
