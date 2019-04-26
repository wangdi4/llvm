/* INTEL_FEATURE_ISA_ULI */
/*===--------------------- uliintrin.h - ULI intrinsics --------------------===
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
#error "Never use <uliintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __ULIINTRIN_H
#define __ULIINTRIN_H

/* Define the default attributes for the functions in this file */
#define __DEFAULT_FN_ATTRS \
  __attribute__((__always_inline__, __nodebug__, __target__("uli")))

#ifdef __x86_64__

static __inline__ void __DEFAULT_FN_ATTRS
_uiret (void)
{
  __builtin_ia32_uiret();
}

static __inline__ void __DEFAULT_FN_ATTRS
_clui (void)
{
  __builtin_ia32_clui();
}

static __inline__ void __DEFAULT_FN_ATTRS
_stui (void)
{
  __builtin_ia32_stui();
}

static __inline__ unsigned char __DEFAULT_FN_ATTRS
_testui (void)
{
  return __builtin_ia32_testui();
}

static __inline__ void __DEFAULT_FN_ATTRS
_senduipi (unsigned long long __a)
{
  __builtin_ia32_senduipi(__a);
}

#endif /* __x86_64__ */

#undef __DEFAULT_FN_ATTRS

#endif /* __ULIINTRIN_H */
/* end INTEL_FEATURE_ISA_ULI */
