/* INTEL_FEATURE_ISA_AMX_BF8 */
/*===---------- Intel_amxbf8intrin.h - AMX intrinsics -*- C++ -*------------===
*
* Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amxbf8intrin.h> directly; include <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMXBF8INTRIN_H
#define __AMXBF8INTRIN_H
#ifdef __x86_64__

#define _tile_dpbf8ps __builtin_ia32_tdpbf8ps

#endif /* __x86_64__ */
#endif /* __AMXBF8INTRIN_H */
/* end INTEL_FEATURE_ISA_AMX_BF8 */
