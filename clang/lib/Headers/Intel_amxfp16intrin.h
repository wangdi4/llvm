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

/// Compute dot-product of FP16 (16-bit) floating-point pairs in tiles \a src0
///    and \a src1, accumulating the intermediate single-precision (32-bit)
///    floating-point elements with elements in \a acc, and store the 32-bit
///    result back to tile \a acc.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the \c TDPFP16PS instruction.
///
/// \param acc
///    The destination tile. Max size is 1024 Bytes.
/// \param src0
///    The 1st source tile. Max size is 1024 Bytes.
/// \param src1
///    The 2nd source tile. Max size is 1024 Bytes.
#define _tile_dpfp16ps(acc, src0, src1)                                \
  __builtin_ia32_tdpfp16ps(acc, src0, src1)

#endif /* __x86_64__ */
#endif /* __AMX_FP16INTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_FP16 */
