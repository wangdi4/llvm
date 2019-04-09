/* INTEL_FEATURE_ISA_TSXLDTRK */
/*===------------- tsxldtrkintrin.h - tsxldtrk intrinsics ------------------===
 *
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 *
 * The information and source code contained herein is the exclusive property
 * of Intel Corporation and may not be disclosed, examined or reproduced in
 * whole or in part without explicit written authorization from the company.
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef __IMMINTRIN_H
#error "Never use <tsxldtrkintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __INTEL_TSXLDTRKINTRIN_H
#define __INTEL_TSXLDTRKINTRIN_H

/* Define the default attributes for the functions in this file */
#define _DEFAULT_FN_ATTRS \
  __attribute__((__always_inline__, __nodebug__, __target__("tsxldtrk")))

static __inline__ void _DEFAULT_FN_ATTRS
_xsusldtrk (void)
{
    __builtin_ia32_xsusldtrk();
}

static __inline__ void _DEFAULT_FN_ATTRS
_xresldtrk (void)
{
    __builtin_ia32_xresldtrk();
}

#undef _DEFAULT_FN_ATTRS

#endif /* __INTEL_TSXLDTRKINTRIN_H */
/* end INTEL_FEATURE_ISA_TSXLDTRK */
