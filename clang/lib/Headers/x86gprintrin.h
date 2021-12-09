/*===--------------- x86gprintrin.h - X86 GPR intrinsics ------------------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef __X86GPRINTRIN_H
#define __X86GPRINTRIN_H

#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__HRESET__)
#include <hresetintrin.h>
#endif

#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__UINTR__)
#include <uintrintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ISA_GPR_MOVGET */
#if defined(__GPRMOVGET_SUPPORTED__)
#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__GPRMOVGET__) || defined(__M_INTRINSIC_PROMOTE__)
#include <gprmovget/gprmovgetintrin.h>
#endif
#endif
/* end INTEL_FEATURE_ISA_GPR_MOVGET */

/* INTEL_FEATURE_ISA_RAO_INT */
#if defined(__RAOINT_SUPPORTED__)
#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__RAOINT__) || defined(__M_INTRINSIC_PROMOTE__)
#include <raoint/raointintrin.h>
#endif
#endif
/* end INTEL_FEATURE_ISA_RAO_INT */
/* end INTEL_CUSTOMIZATION */
#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__CRC32__)
#include <crc32intrin.h>
#endif

#define __SSC_MARK(Tag)                                                        \
  __asm__ __volatile__("mov {%%ebx, %%eax|eax, ebx}; "                      \
                       "mov {%0, %%ebx|ebx, %0}; "                          \
                       ".byte 0x64, 0x67, 0x90; "                              \
                       "mov {%%eax, %%ebx|ebx, eax};" ::"i"(Tag)            \
                       : "%eax");

#endif /* __X86GPRINTRIN_H */
