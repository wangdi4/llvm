/* INTEL_FEATURE_ISA_AMX_FP8 */
/*===---------- Intel_amxfp8intrin.h - AMX intrinsics -*- C++ -*------------===
*
* Copyright (C) 2015 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amxfp8intrin.h> directly; include <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMXFP8INTRIN_H
#define __AMXFP8INTRIN_H
#ifdef __x86_64__

#define _tile_dpbf8ps __builtin_ia32_tdpbf8ps
#define _tile_dpbhf8ps __builtin_ia32_tdpbhf8ps
#define _tile_dphbf8ps __builtin_ia32_tdphbf8ps
#define _tile_dphf8ps __builtin_ia32_tdphf8ps

#endif /* __x86_64__ */
#endif /* __AMXFP8INTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_FP8 */
