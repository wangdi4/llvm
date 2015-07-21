//
// Copyright (C) 2015 Intel Corporation.  All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
/*===---- __x86intrin_features.h -------------------------------------------===
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef __X86INTRIN_FEATURES_H
#define __X86INTRIN_FEATURES_H
 
#define _FEATURE_GENERIC_IA32        0x00000001ULL
#define _FEATURE_FPU                 0x00000002ULL
#define _FEATURE_CMOV                0x00000004ULL
#define _FEATURE_MMX                 0x00000008ULL
#define _FEATURE_FXSAVE              0x00000010ULL
#define _FEATURE_SSE                 0x00000020ULL
#define _FEATURE_SSE2                0x00000040ULL
#define _FEATURE_SSE3                0x00000080ULL
#define _FEATURE_SSSE3               0x00000100ULL
#define _FEATURE_SSE4_1              0x00000200ULL
#define _FEATURE_SSE4_2              0x00000400ULL
#define _FEATURE_MOVBE               0x00000800ULL
#define _FEATURE_POPCNT              0x00001000ULL
#define _FEATURE_PCLMULQDQ           0x00002000ULL
#define _FEATURE_AES                 0x00004000ULL
#define _FEATURE_F16C                0x00008000ULL
#define _FEATURE_AVX                 0x00010000ULL
#define _FEATURE_RDRND               0x00020000ULL
#define _FEATURE_FMA                 0x00040000ULL
#define _FEATURE_BMI                 0x00080000ULL
#define _FEATURE_LZCNT               0x00100000ULL
#define _FEATURE_HLE                 0x00200000ULL
#define _FEATURE_RTM                 0x00400000ULL
#define _FEATURE_AVX2                0x00800000ULL
#define _FEATURE_AVX512DQ            0x01000000ULL
#define _FEATURE_KNCNI               0x04000000ULL
#define _FEATURE_AVX512F             0x08000000ULL
#define _FEATURE_ADX                 0x10000000ULL
#define _FEATURE_RDSEED              0x20000000ULL
#define _FEATURE_AVX512IFMA52        0x40000000ULL
#define _FEATURE_AVX512ER            0x100000000ULL
#define _FEATURE_AVX512PF            0x200000000ULL
#define _FEATURE_AVX512CD            0x400000000ULL
#define _FEATURE_SHA                 0x800000000ULL
#define _FEATURE_MPX                 0x1000000000ULL
#define _FEATURE_AVX512BW            0x2000000000ULL
#define _FEATURE_AVX512VL            0x4000000000ULL
#define _FEATURE_AVX512VBMI          0x8000000000ULL

#endif // __X86INTRIN_FEATURES_H
