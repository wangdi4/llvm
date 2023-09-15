/* INTEL_FEATURE_ISA_AMX_FP8_FUTURE */
/*===------- Intel_amxfp8futureintrin.h - AMX intrinsics -*- C++ -*--------===
*
* Copyright (C) 2015 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amxfp8futureintrin.h> directly; include <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMXFP8FUTUREINTRIN_H
#define __AMXFP8FUTUREINTRIN_H
#ifdef __x86_64__

#define _tile_tdpbf8ps __builtin_ia32_ttdpbf8ps
#define _tile_tdpbhf8ps __builtin_ia32_ttdpbhf8ps
#define _tile_tdphbf8ps __builtin_ia32_ttdphbf8ps
#define _tile_tdphf8ps __builtin_ia32_ttdphf8ps

#endif /* __x86_64__ */
#endif /* __AMXFP8FUTUREINTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_FP8_FUTURE */
