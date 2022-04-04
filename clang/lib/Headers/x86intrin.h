/*===---- x86intrin.h - X86 intrinsics -------------------------------------=== */
/* INTEL_CUSTOMIZATION */
/*
 * Modifications, Copyright (C) 2021 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */
/*
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef __X86INTRIN_H
#define __X86INTRIN_H

/* INTEL_CUSTOMIZATION */
/* Turn fp precise on for intrinsics, push state to restore at end. */
#pragma float_control(push)
#pragma float_control(precise, on)

/* Moved to immintrin.h */
/*#include <ia32intrin.h>*/
/* end INTEL_CUSTOMIZATION */

#include <immintrin.h>

/* INTEL_CUSTOMIZATION */
#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__3dNOW__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <mm3dnow.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__PRFCHW__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <prfchwintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__SSE4A__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <ammintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__FMA4__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <fma4intrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__XOP__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <xopintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__TBM__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <tbmintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__LWP__) || defined(__M_INTRINSIC_PROMOTE__)
#include <lwpintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__MWAITX__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <mwaitxintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__CLZERO__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <clzerointrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#pragma float_control(pop)
/* end INTEL_CUSTOMIZATION */

#endif /* __X86INTRIN_H */
