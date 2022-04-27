/*===---- prfchwintrin.h - PREFETCHW intrinsic -----------------------------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */

#if !defined(__X86INTRIN_H) && !defined(_MM3DNOW_H_INCLUDED)
#error "Never use <prfchwintrin.h> directly; include <x86intrin.h> or <mm3dnow.h> instead."
#endif

#ifndef __PRFCHWINTRIN_H
#define __PRFCHWINTRIN_H

/// Loads a memory sequence containing the specified memory address into
///    all data cache levels. The cache-coherency state is set to exclusive.
///    Data can be read from and written to the cache line without additional
///    delay.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the \c PREFETCHT0 instruction.
///
/// \param __P
///    A pointer specifying the memory address to be prefetched.
static __inline__ void __attribute__((__always_inline__, __nodebug__))
_m_prefetch(void *__P)
{
  __builtin_prefetch (__P, 0, 3 /* _MM_HINT_T0 */);
}

/// Loads a memory sequence containing the specified memory address into
///    the L1 data cache and sets the cache-coherency to modified. This
///    provides a hint to the processor that the cache line will be modified.
///    It is intended for use when the cache line will be written to shortly
///    after the prefetch is performed.
///
///    Note that the effect of this intrinsic is dependent on the processor
///    implementation.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the \c PREFETCHW instruction.
///
/// \param __P
///    A pointer specifying the memory address to be prefetched.
static __inline__ void __attribute__((__always_inline__, __nodebug__))
_m_prefetchw(volatile const void *__P)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
  __builtin_prefetch ((const void*)__P, 1, 3 /* _MM_HINT_T0 */);
#pragma clang diagnostic pop
}

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ISA_PREFETCHST2 */
#if defined(__PREFETCHST2__)
/// Loads a memory sequence containing the specified memory address into
///    the L3 data cache. Data will be shared (read/written) to by requesting
///    core and other cores.
///
///    Note that the effect of this intrinsic is dependent on the processor
///    implementation.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the \c PREFETCHS instruction.
///
/// \param __P
///    A pointer specifying the memory address to be prefetched.
static __inline__ void __attribute__((__always_inline__, __nodebug__))
_m_prefetchs(volatile const void *__P)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
  __builtin_prefetch ((const void*)__P, 2, 1 /* _MM_HINT_T2 */);
#pragma clang diagnostic pop
}
#endif
/* end INTEL_FEATURE_ISA_PREFETCHST2 */

/* INTEL_FEATURE_ISA_PREFETCHI */
#if defined(__PREFETCHI__)
/// Loads an instruction sequence containing the specified memory address into
///    all level cache.
///
///    Note that the effect of this intrinsic is dependent on the processor
///    implementation.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the \c PREFETCHIT0 instruction.
///
/// \param __P
///    A pointer specifying the memory address to be prefetched.
static __inline__ void __attribute__((__always_inline__, __nodebug__))
_m_prefetchit0(volatile const void *__P)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
  __builtin_prefetch ((const void*)__P, 1, 3 /* _MM_HINT_T0 */, 0 /* inst */);
#pragma clang diagnostic pop
}

/// Loads an instruction sequence containing the specified memory address into
///    all but the first-level cache.
///
///    Note that the effect of this intrinsic is dependent on the processor
///    implementation.
///
/// \headerfile <x86intrin.h>
///
/// This intrinsic corresponds to the \c PREFETCHIT1 instruction.
///
/// \param __P
///    A pointer specifying the memory address to be prefetched.
static __inline__ void __attribute__((__always_inline__, __nodebug__))
_m_prefetchit1(volatile const void *__P)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
  __builtin_prefetch ((const void*)__P, 1, 2 /* _MM_HINT_T1 */, 0 /* inst */);
#pragma clang diagnostic pop
}
#endif
/* end INTEL_FEATURE_ISA_PREFETCHI */
/* end INTEL_CUSTOMIZATION */

#endif /* __PRFCHWINTRIN_H */
