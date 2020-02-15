/* INTEL_FEATURE_ISA_AMX_BF16_EVEX */
/*===--- Intel_amxbf16evexintrin.h - AMX_BF16_EVEX intrinsics -*- C++ -*-----===
*
* Copyright (C) 2019 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amxbf16evexintrin.h> directly; use <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMX_BF16EVEXINTRIN_H
#define __AMX_BF16EVEXINTRIN_H
#ifdef __x86_64__

// BF16EVEX
#define _tile_dpbf16pse(tsrc1_dst, tsrc2, tsrc3)                               \
  __builtin_ia32_tdpbf16pse(tsrc1_dst, tsrc2, tsrc3)

#endif /* __x86_64__ */
#endif /* __AMX_BF16EVEXINTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_BF16_EVEX */
