/* INTEL_FEATURE_ISA_HRESET */
// ==------------- hresetintrin.h - serialzie intrinsic -*- C -*------------==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef __IMMINTRIN_H
#error "Never use <hresetintrin.h> directly; include <immintrin.h> instead."
#endif

#ifndef __INTEL_HRESETINTRIN_H
#define __INTEL_HRESETINTRIN_H

#if __has_extension(gnu_asm)

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS \
  __attribute__((__always_inline__, __nodebug__,  __target__("hreset")))

static __inline void __DEFAULT_FN_ATTRS
_hreset(int __eax)
{
  __asm__ ("hreset $0" :: "a"(__eax));
}

#undef __DEFAULT_FN_ATTRS

#endif /* __has_extension(gnu_asm) */

#endif /* __INTEL_HRESETINTRIN_H */
/* end INTEL_FEATURE_ISA_HRESET */
