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

// FP16
#define _tile_dpfp16ps(tsrc1_dst, tsrc2, tsrc3)                                \
  __builtin_ia32_tdpfp16ps(tsrc1_dst, tsrc2, tsrc3)

#endif /* __x86_64__ */
#endif /* __AMX_FP16INTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_FP16 */
