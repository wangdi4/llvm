/* INTEL_FEATURE_ISA_AMX_CONVERT_EVEX */
/*===- Intel_amxconvertevexintrin.h - AMX_CONVERT_EVEX intrinsics -*- C++ -*-===
*
* Copyright (C) 2019 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amxconvertevexintrin.h> directly; use <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMX_CONVERTEVEXINTRIN_H
#define __AMX_CONVERTEVEXINTRIN_H
#ifdef __x86_64__

//AMX_CONVERT_EVEX
#define _tile_cvtd2pse(base, stride, tsrc)                                    \
  __builtin_ia32_tcvtd2pse(base, stride, tsrc)

#endif /* __x86_64__ */
#endif /* __AMX_CONVERTEVEXINTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_CONVERT_EVEX */
