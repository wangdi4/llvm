/*===---- x86intrin.h - X86 intrinsics -------------------------------------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef __X86INTRIN_H
#define __X86INTRIN_H

/* INTEL_CUSTOMIZATION */
/* Moved to immintrin.h */
/*#include <ia32intrin.h>*/
/* end INTEL_CUSTOMIZATION */

#include <immintrin.h>

<<<<<<< HEAD
/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__3dNOW__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <mm3dnow.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__PRFCHW__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <prfchwintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__SSE4A__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <ammintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__FMA4__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <fma4intrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__XOP__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <xopintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__TBM__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <tbmintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__LWP__) || defined(__M_INTRINSIC_PROMOTE__)
#include <lwpintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__MWAITX__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
#include <mwaitxintrin.h>
#endif

/* INTEL_CUSTOMIZATION */
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__CLZERO__) || defined(__M_INTRINSIC_PROMOTE__)
/* end INTEL_CUSTOMIZATION */
=======
#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__3dNOW__)
#include <mm3dnow.h>
#endif

#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__PRFCHW__)
#include <prfchwintrin.h>
#endif

#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__SSE4A__)
#include <ammintrin.h>
#endif

#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__FMA4__)
#include <fma4intrin.h>
#endif

#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__XOP__)
#include <xopintrin.h>
#endif

#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__TBM__)
#include <tbmintrin.h>
#endif

#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__LWP__)
#include <lwpintrin.h>
#endif

#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__MWAITX__)
#include <mwaitxintrin.h>
#endif

#if !(defined(_MSC_VER) || defined(__SCE__)) || __has_feature(modules) ||      \
    defined(__CLZERO__)
>>>>>>> 046130490f230559e8dae7d7cb4959bd04f89bb8
#include <clzerointrin.h>
#endif


#endif /* __X86INTRIN_H */
