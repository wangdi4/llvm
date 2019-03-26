/* INTEL_FEATURE_ISA_ENQCMD */
/*===------------------ enqcmdintrin.h - enqcmd intrinsics -----------------===
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
#error "Never use <enqcmdintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __ENQCMDINTRIN_H
#define __ENQCMDINTRIN_H

/* Define the default attributes for the functions in this file */
#define _DEFAULT_FN_ATTRS \
  __attribute__((__always_inline__, __nodebug__, __target__("enqcmd")))

static __inline__ int _DEFAULT_FN_ATTRS
_enqcmd (void *__dst, const void *__src)
{
  return __builtin_ia32_enqcmd(__dst, __src);
}

static __inline__ int _DEFAULT_FN_ATTRS
_enqcmds (void *__dst, const void *__src)
{
  return __builtin_ia32_enqcmds(__dst, __src);
}

#undef _DEFAULT_FN_ATTRS

#endif /* __ENQCMDINTRIN_H */
/* end INTEL_FEATURE_ISA_ENQCMD */
