/* INTEL_FEATURE_ISA_AMX_FP16 */
/*===---------- Intel_amxfp16intrin.h - AMX_FP16 intrinsics -*- C++ -*---------===
*
* Copyright (C) 2020 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amxfp16intrin.h> directly; use <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMX_FP16INTRIN_H
#define __AMX_FP16INTRIN_H
#ifdef __x86_64__

/// Compute dot-product of FP16 (16-bit) floating-point pairs in tiles \a a
///    and \a b, accumulating the intermediate single-precision (32-bit)
///    floating-point elements with elements in \a dst, and store the 32-bit
///    result back to tile \a dst.
///
/// \headerfile <x86intrin.h>
///
/// \code
/// void _tile_dpfp16ps (__tile dst, __tile a, __tile b)
/// \endcode
///
/// \code{.operation}
/// FOR m := 0 TO dst.rows - 1
///	tmp := dst.row[m]
///	FOR k := 0 TO (a.colsb / 4) - 1
///		FOR n := 0 TO (dst.colsb / 4) - 1
///			tmp.fp32[n] += FP32(a.row[m].fp16[2*k+0]) *
///					FP32(b.row[k].fp16[2*n+0])
///			tmp.fp32[n] += FP32(a.row[m].fp16[2*k+1]) *
///					FP32(b.row[k].fp16[2*n+1])
///		ENDFOR
///	ENDFOR
///	write_row_and_zero(dst, m, tmp, dst.colsb)
/// ENDFOR
/// zero_upper_rows(dst, dst.rows)
/// zero_tileconfig_start()
/// \endcode
///
/// This intrinsic corresponds to the \c TDPFP16PS instruction.
///
/// \param dst
///    The destination tile. Max size is 1024 Bytes.
/// \param a
///    The 1st source tile. Max size is 1024 Bytes.
/// \param b
///    The 2nd source tile. Max size is 1024 Bytes.
#define _tile_dpfp16ps(dst, a, b)                                \
  __builtin_ia32_tdpfp16ps(dst, a, b)

#endif /* __x86_64__ */
#endif /* __AMX_FP16INTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_FP16 */
