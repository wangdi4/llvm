/* INTEL_FEATURE_ISA_AMX_CONVERT */
/*===---- Intel_amxconvertintrin.h - AMX_CONVERT intrinsics -*- C++ -*---------===
*
* Copyright (C) 2020 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amxconvertintrin.h> directly; use <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMX_CONVERTINTRIN_H
#define __AMX_CONVERTINTRIN_H
#ifdef __x86_64__

// CONVERT
#define _tile_cvt2ps2bf16(base, stride, tsrc1, tsrc2)                         \
  __builtin_ia32_tcvt2ps2bf16(base, stride, tsrc1, tsrc2)
#define _tile_cvt2ps2ph(base, stride, tsrc1, tsrc2)                           \
  __builtin_ia32_tcvt2ps2ph(base, stride, tsrc1, tsrc2)
#define _tile_amxconvert_cvtd2ps(base, stride, tsrc)                          \
  __builtin_ia32_amxconvert_tcvtd2ps(base, stride, tsrc)
#define _tile_amxconvert_cvtps2bf16(base, stride, tsrc)                       \
  __builtin_ia32_amxconvert_tcvtps2bf16(base, stride, tsrc)
#define _tile_cvtps2ph(base, stride, tsrc)                                    \
  __builtin_ia32_tcvtps2ph(base, stride, tsrc)

#endif /* __x86_64__ */
#endif /* __AMX_CONVERTINTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_CONVERT */
