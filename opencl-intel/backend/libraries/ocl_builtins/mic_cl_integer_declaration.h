// Copyright (c) 2006-2011 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  mic_cl_integer_declaration.h
///////////////////////////////////////////////////////////
#pragma once

#include "cl_types2.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __x86_64__
# error "MIC only runs with EM64T enabled!"
#endif

#define char1   char
#define short1  short
#define int1    int
#define long1   long
#define uchar1  uchar
#define ushort1 ushort
#define uint1   uint
#define ulong1  ulong

/// Native support
#define OCL_INTR_P1_NATIVE(func, nW, oT0, iT0, mT)                                      \
    oT0##nW __attribute__((overloadable)) func(iT0##nW);                                \
    oT0##nW __attribute__((overloadable)) mask_##func(mT, iT0##nW);                     \

#define OCL_INTR_P2_NATIVE(func, nW, oT0, iT0, iT1, mT)                                 \
    oT0##nW __attribute__((overloadable)) func(iT0##nW, iT1##nW);                       \
    oT0##nW __attribute__((overloadable)) mask_##func(mT, iT0##nW, iT1##nW);            \

#define OCL_INTR_P3_NATIVE(func, nW, oT0, iT0, iT1, iT2, mT)                            \
    oT0##nW __attribute__((overloadable)) func(iT0##nW, iT1##nW, iT2##nW);              \
    oT0##nW __attribute__((overloadable)) mask_##func(mT, iT0##nW, iT1##nW, iT2##nW);   \

/// Promote to wider vector
#define OCL_INTR_P1_PROMOTE(func, nW, oW, oT0, iT0, mT, msk, pat)                       \
    oT0##nW __attribute__((overloadable)) mask_##func(mT, iT0##nW);                     \
    oT0##oW __attribute__((overloadable)) func(iT0##oW x)                               \
    {                                                                                   \
        iT0##nW valx;                                                                   \
        valx.pat = x;                                                                   \
        oT0##nW res = mask_##func(msk, valx);                                           \
        return res.pat;                                                                 \
    }

#define OCL_INTR_P2_PROMOTE(func, nW, oW, oT0, iT0, iT1, mT, msk, pat)                  \
    oT0##nW __attribute__((overloadable)) mask_##func(mT, iT0##nW, iT1##nW);            \
    oT0##oW __attribute__((overloadable)) func(iT0##oW x, iT1##oW y)                    \
    {                                                                                   \
        iT0##nW valx;                                                                   \
        iT1##nW valy;                                                                   \
        valx.pat = x;                                                                   \
        valy.pat = y;                                                                   \
        oT0##nW res = mask_##func(msk, valx, valy);                                     \
        return res.pat;                                                                 \
    }

#define OCL_INTR_P3_PROMOTE(func, nW, oW, oT0, iT0, iT1, iT2, mT, msk, pat)             \
    oT0##nW __attribute__((overloadable)) mask_##func(mT, iT0##nW, iT1##nW, iT2##nW);   \
    oT0##oW __attribute__((overloadable)) func(iT0##oW x, iT1##oW y, iT2##oW z)         \
    {                                                                                   \
        iT0##nW valx;                                                                   \
        iT1##nW valy;                                                                   \
        iT2##nW valz;                                                                   \
        valx.pat = x;                                                                   \
        valy.pat = y;                                                                   \
        valz.pat = z;                                                                   \
        oT0##nW res = mask_##func(msk, valx, valy, valz);                               \
        return res.pat;                                                                 \
    }

/// Expand to narrow vector
#define OCL_INTR_P1_EXPAND(func, nW, oW, oT0, iT0)                                      \
    oT0##nW __attribute__((overloadable)) func(iT0##nW);                                \
    oT0##oW __attribute__((overloadable)) func(iT0##oW x)                               \
    {                                                                                   \
        oT0##oW res;                                                                    \
        _EXPAND1_To##nW##_From##oW(res, func, x);                                       \
        return res;                                                                     \
    }

#define OCL_INTR_P2_EXPAND(func, nW, oW, oT0, iT0, iT1)                                 \
    oT0##nW __attribute__((overloadable)) func(iT0##nW, iT1##nW);                       \
    oT0##oW __attribute__((overloadable)) func(iT0##oW x, iT1##oW y)                    \
    {                                                                                   \
        oT0##oW res;                                                                    \
        _EXPAND2_To##nW##_From##oW(res, func, x, y);                                    \
        return res;                                                                     \
    }

#define OCL_INTR_P3_EXPAND(func, nW, oW, oT0, iT0, iT1, iT2)                            \
    oT0##nW __attribute__((overloadable)) func(iT0##nW, iT1##nW, iT2##nW);              \
    oT0##oW __attribute__((overloadable)) func(iT0##oW x, iT1##oW y, iT2##oW z)         \
    {                                                                                   \
        oT0##oW res;                                                                    \
        _EXPAND3_To##nW##_From##oW(res, func, x, y, z);                                 \
        return res;                                                                     \
    }

/// Vectorize scalar parameter(s)
#define OCL_FUNC_P2_VS(func, oW, oT0, iT0, iT1)                                         \
    oT0##oW __attribute__((overloadable)) func(iT0##oW x, iT1 y)                        \
    {                                                                                   \
        iT1##oW valy;                                                                   \
        valy = (iT1##oW)y;                                                              \
        return func(x, valy);                                                           \
    }

#define OCL_FUNC_P3_VSS(func, oW, oT0, iT0, iT1, iT2)                                   \
    oT0##oW __attribute__((overloadable)) func(iT0##oW x, iT1 y, iT2 z)                 \
    {                                                                                   \
        iT1##oW valy;                                                                   \
        iT2##oW valz;                                                                   \
        valy = (iT1##oW)y;                                                              \
        valz = (iT2##oW)z;                                                              \
        return func(x, valy, valz);                                                     \
    }

/// How expansion is done
// basic step
#define _EXPAND1_1x(res, func, x)               \
    res.lo = func(x.lo);                        \
    res.hi = func(x.hi);                        \

#define _EXPAND2_1x(res, func, x, y)            \
    res.lo = func(x.lo, y.lo);                  \
    res.hi = func(x.hi, y.hi);                  \

#define _EXPAND3_1x(res, func, x, y, z)         \
    res.lo = func(x.lo, y.lo, z.lo);            \
    res.hi = func(x.hi, y.hi, z.lo);            \

// v16 -> v8
#define _EXPAND1_To8_From16(res, func, x)       \
    _EXPAND1_1x(res, func, x)

#define _EXPAND2_To8_From16(res, func, x, y)    \
    _EXPAND2_1x(res, func, x, y)

#define _EXPAND3_To8_From16(res, func, x, y, z) \
    _EXPAND3_1x(res, func, x, y, z)

// v8 -> v4
#define _EXPAND1_To4_From8(res, func, x)        \
    _EXPAND1_1x(res, func, x)

#define _EXPAND2_To4_From8(res, func, x, y)     \
    _EXPAND2_1x(res, func, x, y)

#define _EXPAND3_To4_From8(res, func, x, y, z)  \
    _EXPAND3_1x(res, func, x, z)

// v16 -> v4
#define _EXPAND1_To4_From16(res, func, x)       \
    _EXPAND1_1x(res.lo, func, x.lo)             \
    _EXPAND1_1x(res.hi, func, x.hi)

#define _EXPAND2_To4_From16(res, func, x, y)    \
    _EXPAND2_1x(res.lo, func, x.lo, y.lo)       \
    _EXPAND2_1x(res.hi, func, x.hi, y.hi)

#define _EXPAND3_To4_From16(res, func, x, y, z) \
    _EXPAND3_1x(res.lo, func, x.lo, y.lo, z.lo) \
    _EXPAND3_1x(res.hi, func, x.hi, y.hi, z.hi)

/// oT0 := f(iT0)
///
#define OCL_INTR_P1_iCn_iCn(func)                                               \
    OCL_INTR_P1_PROMOTE(func, 16, 1, char, char, ushort, 0x01, s0)              \
    OCL_INTR_P1_PROMOTE(func, 16, 2, char, char, ushort, 0x03, s01)             \
    OCL_INTR_P1_PROMOTE(func, 16, 3, char, char, ushort, 0x07, s012)            \
    OCL_INTR_P1_PROMOTE(func, 16, 4, char, char, ushort, 0x0F, s0123)           \
    OCL_INTR_P1_PROMOTE(func, 16, 8, char, char, ushort, 0xFF, s01234567)       \
    OCL_INTR_P1_NATIVE(func,    16, char, char, ushort)

#define OCL_INTR_P1_uCn_uCn(func)                                               \
    OCL_INTR_P1_PROMOTE(func, 16, 1, uchar, uchar, ushort, 0x01, s0)            \
    OCL_INTR_P1_PROMOTE(func, 16, 2, uchar, uchar, ushort, 0x03, s01)           \
    OCL_INTR_P1_PROMOTE(func, 16, 3, uchar, uchar, ushort, 0x07, s012)          \
    OCL_INTR_P1_PROMOTE(func, 16, 4, uchar, uchar, ushort, 0x0F, s0123)         \
    OCL_INTR_P1_PROMOTE(func, 16, 8, uchar, uchar, ushort, 0xFF, s01234567)     \
    OCL_INTR_P1_NATIVE(func,    16, uchar, uchar, ushort)

#define OCL_INTR_P1_uCn_iCn(func)                                               \
    OCL_INTR_P1_PROMOTE(func, 16, 1, uchar, char, ushort, 0x01, s0)             \
    OCL_INTR_P1_PROMOTE(func, 16, 2, uchar, char, ushort, 0x03, s01)            \
    OCL_INTR_P1_PROMOTE(func, 16, 3, uchar, char, ushort, 0x07, s012)           \
    OCL_INTR_P1_PROMOTE(func, 16, 4, uchar, char, ushort, 0x0F, s0123)          \
    OCL_INTR_P1_PROMOTE(func, 16, 8, uchar, char, ushort, 0xFF, s01234567)      \
    OCL_INTR_P1_NATIVE(func,    16, uchar, char, ushort)

#define OCL_INTR_P1_iSn_iSn(func)                                               \
    OCL_INTR_P1_PROMOTE(func, 16, 1, short, short, ushort, 0x01, s0)            \
    OCL_INTR_P1_PROMOTE(func, 16, 2, short, short, ushort, 0x03, s01)           \
    OCL_INTR_P1_PROMOTE(func, 16, 3, short, short, ushort, 0x07, s012)          \
    OCL_INTR_P1_PROMOTE(func, 16, 4, short, short, ushort, 0x0F, s0123)         \
    OCL_INTR_P1_PROMOTE(func, 16, 8, short, short, ushort, 0xFF, s01234567)     \
    OCL_INTR_P1_NATIVE(func,    16, short, short, ushort)

#define OCL_INTR_P1_uSn_uSn(func)                                               \
    OCL_INTR_P1_PROMOTE(func, 16, 1, ushort, ushort, ushort, 0x01, s0)          \
    OCL_INTR_P1_PROMOTE(func, 16, 2, ushort, ushort, ushort, 0x03, s01)         \
    OCL_INTR_P1_PROMOTE(func, 16, 3, ushort, ushort, ushort, 0x07, s012)        \
    OCL_INTR_P1_PROMOTE(func, 16, 4, ushort, ushort, ushort, 0x0F, s0123)       \
    OCL_INTR_P1_PROMOTE(func, 16, 8, ushort, ushort, ushort, 0xFF, s01234567)   \
    OCL_INTR_P1_NATIVE(func,    16, ushort, ushort, ushort)

#define OCL_INTR_P1_uSn_iSn(func)                                               \
    OCL_INTR_P1_PROMOTE(func, 16, 1, ushort, short, ushort, 0x01, s0)           \
    OCL_INTR_P1_PROMOTE(func, 16, 2, ushort, short, ushort, 0x03, s01)          \
    OCL_INTR_P1_PROMOTE(func, 16, 3, ushort, short, ushort, 0x07, s012)         \
    OCL_INTR_P1_PROMOTE(func, 16, 4, ushort, short, ushort, 0x0F, s0123)        \
    OCL_INTR_P1_PROMOTE(func, 16, 8, ushort, short, ushort, 0xFF, s01234567)    \
    OCL_INTR_P1_NATIVE(func,    16, ushort, short, ushort)

#define OCL_INTR_P1_iIn_iIn(func)                                               \
    OCL_INTR_P1_PROMOTE(func, 16, 1, int, int, ushort, 0x01, s0)                \
    OCL_INTR_P1_PROMOTE(func, 16, 2, int, int, ushort, 0x03, s01)               \
    OCL_INTR_P1_PROMOTE(func, 16, 3, int, int, ushort, 0x07, s012)              \
    OCL_INTR_P1_PROMOTE(func, 16, 4, int, int, ushort, 0x0F, s0123)             \
    OCL_INTR_P1_PROMOTE(func, 16, 8, int, int, ushort, 0xFF, s01234567)         \
    OCL_INTR_P1_NATIVE(func,    16, int, int, ushort)

#define OCL_INTR_P1_uIn_uIn(func)                                               \
    OCL_INTR_P1_PROMOTE(func, 16, 1, uint, uint, ushort, 0x01, s0)              \
    OCL_INTR_P1_PROMOTE(func, 16, 2, uint, uint, ushort, 0x03, s01)             \
    OCL_INTR_P1_PROMOTE(func, 16, 3, uint, uint, ushort, 0x07, s012)            \
    OCL_INTR_P1_PROMOTE(func, 16, 4, uint, uint, ushort, 0x0F, s0123)           \
    OCL_INTR_P1_PROMOTE(func, 16, 8, uint, uint, ushort, 0xFF, s01234567)       \
    OCL_INTR_P1_NATIVE(func,    16, uint, uint, ushort)

#define OCL_INTR_P1_uIn_iIn(func)                                               \
    OCL_INTR_P1_PROMOTE(func, 16, 1, uint, int, ushort, 0x01, s0)               \
    OCL_INTR_P1_PROMOTE(func, 16, 2, uint, int, ushort, 0x03, s01)              \
    OCL_INTR_P1_PROMOTE(func, 16, 3, uint, int, ushort, 0x07, s012)             \
    OCL_INTR_P1_PROMOTE(func, 16, 4, uint, int, ushort, 0x0F, s0123)            \
    OCL_INTR_P1_PROMOTE(func, 16, 8, uint, int, ushort, 0xFF, s01234567)        \
    OCL_INTR_P1_NATIVE(func,    16, uint, int, ushort)

#define OCL_INTR_P1_iLn_iLn(func)                                               \
    OCL_INTR_P1_PROMOTE(func, 8,  1, long, long, uchar, 0x01, s0)               \
    OCL_INTR_P1_PROMOTE(func, 8,  2, long, long, uchar, 0x03, s01)              \
    OCL_INTR_P1_PROMOTE(func, 8,  3, long, long, uchar, 0x07, s012)             \
    OCL_INTR_P1_PROMOTE(func, 8,  4, long, long, uchar, 0x0F, s0123)            \
    OCL_INTR_P1_NATIVE(func,     8, long, long, uchar)                          \
    OCL_INTR_P1_EXPAND(func, 8, 16, long, long)

#define OCL_INTR_P1_uLn_uLn(func)                                               \
    OCL_INTR_P1_PROMOTE(func, 8,  1, ulong, ulong, uchar, 0x01, s0)             \
    OCL_INTR_P1_PROMOTE(func, 8,  2, ulong, ulong, uchar, 0x03, s01)            \
    OCL_INTR_P1_PROMOTE(func, 8,  3, ulong, ulong, uchar, 0x07, s012)           \
    OCL_INTR_P1_PROMOTE(func, 8,  4, ulong, ulong, uchar, 0x0F, s0123)          \
    OCL_INTR_P1_NATIVE(func,     8, ulong, ulong, uchar)                        \
    OCL_INTR_P1_EXPAND(func, 8, 16, ulong, ulong)

#define OCL_INTR_P1_uLn_iLn(func)                                               \
    OCL_INTR_P1_PROMOTE(func, 8,  1, ulong, long, uchar, 0x01, s0)              \
    OCL_INTR_P1_PROMOTE(func, 8,  2, ulong, long, uchar, 0x03, s01)             \
    OCL_INTR_P1_PROMOTE(func, 8,  3, ulong, long, uchar, 0x07, s012)            \
    OCL_INTR_P1_PROMOTE(func, 8,  4, ulong, long, uchar, 0x0F, s0123)           \
    OCL_INTR_P1_NATIVE(func,     8, ulong, long, uchar)                         \
    OCL_INTR_P1_EXPAND(func, 8, 16, ulong, long)

/// oT0 := f(iT0, iT1)
///
#define OCL_INTR_P2_iCn_iCn_iCn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 16, 1, char, char, char, ushort, 0x01, s0)                \
    OCL_INTR_P2_PROMOTE(func, 16, 2, char, char, char, ushort, 0x03, s01)               \
    OCL_INTR_P2_PROMOTE(func, 16, 3, char, char, char, ushort, 0x07, s012)              \
    OCL_INTR_P2_PROMOTE(func, 16, 4, char, char, char, ushort, 0x0F, s0123)             \
    OCL_INTR_P2_PROMOTE(func, 16, 8, char, char, char, ushort, 0xFF, s01234567)         \
    OCL_INTR_P2_NATIVE(func,    16, char, char, char, ushort)

#define OCL_INTR_P2_uCn_uCn_uCn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 16, 1, uchar, uchar, uchar, ushort, 0x01, s0)             \
    OCL_INTR_P2_PROMOTE(func, 16, 2, uchar, uchar, uchar, ushort, 0x03, s01)            \
    OCL_INTR_P2_PROMOTE(func, 16, 3, uchar, uchar, uchar, ushort, 0x07, s012)           \
    OCL_INTR_P2_PROMOTE(func, 16, 4, uchar, uchar, uchar, ushort, 0x0F, s0123)          \
    OCL_INTR_P2_PROMOTE(func, 16, 8, uchar, uchar, uchar, ushort, 0xFF, s01234567)      \
    OCL_INTR_P2_NATIVE(func,    16, uchar, uchar, uchar, ushort)

#define OCL_INTR_P2_uCn_iCn_iCn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 16, 1, uchar, char, char, ushort, 0x01, s0)               \
    OCL_INTR_P2_PROMOTE(func, 16, 2, uchar, char, char, ushort, 0x03, s01)              \
    OCL_INTR_P2_PROMOTE(func, 16, 3, uchar, char, char, ushort, 0x07, s012)             \
    OCL_INTR_P2_PROMOTE(func, 16, 4, uchar, char, char, ushort, 0x0F, s0123)            \
    OCL_INTR_P2_PROMOTE(func, 16, 8, uchar, char, char, ushort, 0xFF, s01234567)        \
    OCL_INTR_P2_NATIVE(func,    16, uchar, char, char, ushort)

#define OCL_INTR_P2_iSn_iSn_iSn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 16, 1, short, short, short, ushort, 0x01, s0)             \
    OCL_INTR_P2_PROMOTE(func, 16, 2, short, short, short, ushort, 0x03, s01)            \
    OCL_INTR_P2_PROMOTE(func, 16, 3, short, short, short, ushort, 0x07, s012)           \
    OCL_INTR_P2_PROMOTE(func, 16, 4, short, short, short, ushort, 0x0F, s0123)          \
    OCL_INTR_P2_PROMOTE(func, 16, 8, short, short, short, ushort, 0xFF, s01234567)      \
    OCL_INTR_P2_NATIVE(func,    16, short, short, short, ushort)

#define OCL_INTR_P2_iSn_iCn_uCn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 16, 1, short, char, uchar, ushort, 0x01, s0)              \
    OCL_INTR_P2_PROMOTE(func, 16, 2, short, char, uchar, ushort, 0x03, s01)             \
    OCL_INTR_P2_PROMOTE(func, 16, 3, short, char, uchar, ushort, 0x07, s012)            \
    OCL_INTR_P2_PROMOTE(func, 16, 4, short, char, uchar, ushort, 0x0F, s0123)           \
    OCL_INTR_P2_PROMOTE(func, 16, 8, short, char, uchar, ushort, 0xFF, s01234567)       \
    OCL_INTR_P2_NATIVE(func,    16, short, char, uchar, ushort)

#define OCL_INTR_P2_uSn_uSn_uSn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 16, 1, ushort, ushort, ushort, ushort, 0x01, s0)          \
    OCL_INTR_P2_PROMOTE(func, 16, 2, ushort, ushort, ushort, ushort, 0x03, s01)         \
    OCL_INTR_P2_PROMOTE(func, 16, 3, ushort, ushort, ushort, ushort, 0x07, s012)        \
    OCL_INTR_P2_PROMOTE(func, 16, 4, ushort, ushort, ushort, ushort, 0x0F, s0123)       \
    OCL_INTR_P2_PROMOTE(func, 16, 8, ushort, ushort, ushort, ushort, 0xFF, s01234567)   \
    OCL_INTR_P2_NATIVE(func,    16, ushort, ushort, ushort, ushort)

#define OCL_INTR_P2_uSn_iSn_iSn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 16, 1, ushort, short, short, ushort, 0x01, s0)            \
    OCL_INTR_P2_PROMOTE(func, 16, 2, ushort, short, short, ushort, 0x03, s01)           \
    OCL_INTR_P2_PROMOTE(func, 16, 3, ushort, short, short, ushort, 0x07, s012)          \
    OCL_INTR_P2_PROMOTE(func, 16, 4, ushort, short, short, ushort, 0x0F, s0123)         \
    OCL_INTR_P2_PROMOTE(func, 16, 8, ushort, short, short, ushort, 0xFF, s01234567)     \
    OCL_INTR_P2_NATIVE(func,    16, ushort, short, short, ushort)

#define OCL_INTR_P2_uSn_uCn_uCn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 16, 1, ushort, uchar, uchar, ushort, 0x01, s0)            \
    OCL_INTR_P2_PROMOTE(func, 16, 2, ushort, uchar, uchar, ushort, 0x03, s01)           \
    OCL_INTR_P2_PROMOTE(func, 16, 3, ushort, uchar, uchar, ushort, 0x07, s012)          \
    OCL_INTR_P2_PROMOTE(func, 16, 4, ushort, uchar, uchar, ushort, 0x0F, s0123)         \
    OCL_INTR_P2_PROMOTE(func, 16, 8, ushort, uchar, uchar, ushort, 0xFF, s01234567)     \
    OCL_INTR_P2_NATIVE(func,    16, ushort, uchar, uchar, ushort)

#define OCL_INTR_P2_iIn_iIn_iIn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 16, 1, int, int, int, ushort, 0x01, s0)                   \
    OCL_INTR_P2_PROMOTE(func, 16, 2, int, int, int, ushort, 0x03, s01)                  \
    OCL_INTR_P2_PROMOTE(func, 16, 3, int, int, int, ushort, 0x07, s012)                 \
    OCL_INTR_P2_PROMOTE(func, 16, 4, int, int, int, ushort, 0x0F, s0123)                \
    OCL_INTR_P2_PROMOTE(func, 16, 8, int, int, int, ushort, 0xFF, s01234567)            \
    OCL_INTR_P2_NATIVE(func,    16, int, int, int, ushort)

#define OCL_INTR_P2_iIn_iSn_uSn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 16, 1, int, short, ushort, ushort, 0x01, s0)              \
    OCL_INTR_P2_PROMOTE(func, 16, 2, int, short, ushort, ushort, 0x03, s01)             \
    OCL_INTR_P2_PROMOTE(func, 16, 3, int, short, ushort, ushort, 0x07, s012)            \
    OCL_INTR_P2_PROMOTE(func, 16, 4, int, short, ushort, ushort, 0x0F, s0123)           \
    OCL_INTR_P2_PROMOTE(func, 16, 8, int, short, ushort, ushort, 0xFF, s01234567)       \
    OCL_INTR_P2_NATIVE(func,    16, int, short, ushort, ushort)

#define OCL_INTR_P2_uIn_uIn_uIn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 16, 1, uint, uint, uint, ushort, 0x01, s0)                \
    OCL_INTR_P2_PROMOTE(func, 16, 2, uint, uint, uint, ushort, 0x03, s01)               \
    OCL_INTR_P2_PROMOTE(func, 16, 3, uint, uint, uint, ushort, 0x07, s012)              \
    OCL_INTR_P2_PROMOTE(func, 16, 4, uint, uint, uint, ushort, 0x0F, s0123)             \
    OCL_INTR_P2_PROMOTE(func, 16, 8, uint, uint, uint, ushort, 0xFF, s01234567)         \
    OCL_INTR_P2_NATIVE(func,    16, uint, uint, uint, ushort)

#define OCL_INTR_P2_uIn_iIn_iIn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 16, 1, uint, int, int, ushort, 0x01, s0)                  \
    OCL_INTR_P2_PROMOTE(func, 16, 2, uint, int, int, ushort, 0x03, s01)                 \
    OCL_INTR_P2_PROMOTE(func, 16, 3, uint, int, int, ushort, 0x07, s012)                \
    OCL_INTR_P2_PROMOTE(func, 16, 4, uint, int, int, ushort, 0x0F, s0123)               \
    OCL_INTR_P2_PROMOTE(func, 16, 8, uint, int, int, ushort, 0xFF, s01234567)           \
    OCL_INTR_P2_NATIVE(func,    16, uint, int, int, ushort)

#define OCL_INTR_P2_uIn_uSn_uSn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 16, 1, uint, ushort, ushort, ushort, 0x01, s0)            \
    OCL_INTR_P2_PROMOTE(func, 16, 2, uint, ushort, ushort, ushort, 0x03, s01)           \
    OCL_INTR_P2_PROMOTE(func, 16, 3, uint, ushort, ushort, ushort, 0x07, s012)          \
    OCL_INTR_P2_PROMOTE(func, 16, 4, uint, ushort, ushort, ushort, 0x0F, s0123)         \
    OCL_INTR_P2_PROMOTE(func, 16, 8, uint, ushort, ushort, ushort, 0xFF, s01234567)     \
    OCL_INTR_P2_NATIVE(func,    16, uint, ushort, ushort, ushort)

#define OCL_INTR_P2_iLn_iLn_iLn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 8,  1, long, long, long, uchar, 0x01, s0)                 \
    OCL_INTR_P2_PROMOTE(func, 8,  2, long, long, long, uchar, 0x03, s01)                \
    OCL_INTR_P2_PROMOTE(func, 8,  3, long, long, long, uchar, 0x07, s012)               \
    OCL_INTR_P2_PROMOTE(func, 8,  4, long, long, long, uchar, 0x0F, s0123)              \
    OCL_INTR_P2_NATIVE(func,     8, long, long, long, uchar)                            \
    OCL_INTR_P2_EXPAND(func, 8, 16, long, long, long)

#define OCL_INTR_P2_iLn_iIn_uIn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 8,  1, long, int, uint, uchar, 0x01, s0)                  \
    OCL_INTR_P2_PROMOTE(func, 8,  2, long, int, uint, uchar, 0x03, s01)                 \
    OCL_INTR_P2_PROMOTE(func, 8,  3, long, int, uint, uchar, 0x07, s012)                \
    OCL_INTR_P2_PROMOTE(func, 8,  4, long, int, uint, uchar, 0x0F, s0123)               \
    OCL_INTR_P2_NATIVE(func,     8, long, int, uint, uchar)                             \
    OCL_INTR_P2_EXPAND(func, 8, 16, long, int, uint)

#define OCL_INTR_P2_uLn_uLn_uLn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 8,  1, ulong, ulong, ulong, uchar, 0x01, s0)              \
    OCL_INTR_P2_PROMOTE(func, 8,  2, ulong, ulong, ulong, uchar, 0x03, s01)             \
    OCL_INTR_P2_PROMOTE(func, 8,  3, ulong, ulong, ulong, uchar, 0x07, s012)            \
    OCL_INTR_P2_PROMOTE(func, 8,  4, ulong, ulong, ulong, uchar, 0x0F, s0123)           \
    OCL_INTR_P2_NATIVE(func,     8, ulong, ulong, ulong, uchar)                         \
    OCL_INTR_P2_EXPAND(func, 8, 16, ulong, ulong, ulong)

#define OCL_INTR_P2_uLn_iLn_iLn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 8,  1, ulong, long, long, uchar, 0x01, s0)                \
    OCL_INTR_P2_PROMOTE(func, 8,  2, ulong, long, long, uchar, 0x03, s01)               \
    OCL_INTR_P2_PROMOTE(func, 8,  3, ulong, long, long, uchar, 0x07, s012)              \
    OCL_INTR_P2_PROMOTE(func, 8,  4, ulong, long, long, uchar, 0x0F, s0123)             \
    OCL_INTR_P2_NATIVE(func,     8, ulong, long, long, uchar)                           \
    OCL_INTR_P2_EXPAND(func, 8, 16, ulong, long, long)

#define OCL_INTR_P2_uLn_uIn_uIn(func)                                                   \
    OCL_INTR_P2_PROMOTE(func, 8,  1, ulong, uint, uint, uchar, 0x01, s0)                \
    OCL_INTR_P2_PROMOTE(func, 8,  2, ulong, uint, uint, uchar, 0x03, s01)               \
    OCL_INTR_P2_PROMOTE(func, 8,  3, ulong, uint, uint, uchar, 0x07, s012)              \
    OCL_INTR_P2_PROMOTE(func, 8,  4, ulong, uint, uint, uchar, 0x0F, s0123)             \
    OCL_INTR_P2_NATIVE(func,     8, ulong, uint, uint, uchar)                           \
    OCL_INTR_P2_EXPAND(func, 8, 16, ulong, uint, uint)

/// oT0 := f(iT0, iT1, iT2)
///
#define OCL_INTR_P3_iCn_iCn_iCn_iCn(func)                                                       \
    OCL_INTR_P3_PROMOTE(func, 16, 1, char, char, char, char, ushort, 0x01, s0)                  \
    OCL_INTR_P3_PROMOTE(func, 16, 2, char, char, char, char, ushort, 0x03, s01)                 \
    OCL_INTR_P3_PROMOTE(func, 16, 3, char, char, char, char, ushort, 0x07, s012)                \
    OCL_INTR_P3_PROMOTE(func, 16, 4, char, char, char, char, ushort, 0x0F, s0123)               \
    OCL_INTR_P3_PROMOTE(func, 16, 8, char, char, char, char, ushort, 0xFF, s01234567)           \
    OCL_INTR_P3_NATIVE(func,    16, char, char, char, char, ushort)

#define OCL_INTR_P3_uCn_uCn_uCn_uCn(func)                                                       \
    OCL_INTR_P3_PROMOTE(func, 16, 1, uchar, uchar, uchar, uchar, ushort, 0x01, s0)              \
    OCL_INTR_P3_PROMOTE(func, 16, 2, uchar, uchar, uchar, uchar, ushort, 0x03, s01)             \
    OCL_INTR_P3_PROMOTE(func, 16, 3, uchar, uchar, uchar, uchar, ushort, 0x07, s012)            \
    OCL_INTR_P3_PROMOTE(func, 16, 4, uchar, uchar, uchar, uchar, ushort, 0x0F, s0123)           \
    OCL_INTR_P3_PROMOTE(func, 16, 8, uchar, uchar, uchar, uchar, ushort, 0xFF, s01234567)       \
    OCL_INTR_P3_NATIVE(func,    16, uchar, uchar, uchar, uchar, ushort)

#define OCL_INTR_P3_iSn_iSn_iSn_iSn(func)                                                       \
    OCL_INTR_P3_PROMOTE(func, 16, 1, short, short, short, short, ushort, 0x01, s0)              \
    OCL_INTR_P3_PROMOTE(func, 16, 2, short, short, short, short, ushort, 0x03, s01)             \
    OCL_INTR_P3_PROMOTE(func, 16, 3, short, short, short, short, ushort, 0x07, s012)            \
    OCL_INTR_P3_PROMOTE(func, 16, 4, short, short, short, short, ushort, 0x0F, s0123)           \
    OCL_INTR_P3_PROMOTE(func, 16, 8, short, short, short, short, ushort, 0xFF, s01234567)       \
    OCL_INTR_P3_NATIVE(func,    16, short, short, short, short, ushort)

#define OCL_INTR_P3_uSn_uSn_uSn_uSn(func)                                                       \
    OCL_INTR_P3_PROMOTE(func, 16, 1, ushort, ushort, ushort, ushort, ushort, 0x01, s0)          \
    OCL_INTR_P3_PROMOTE(func, 16, 2, ushort, ushort, ushort, ushort, ushort, 0x03, s01)         \
    OCL_INTR_P3_PROMOTE(func, 16, 3, ushort, ushort, ushort, ushort, ushort, 0x07, s012)        \
    OCL_INTR_P3_PROMOTE(func, 16, 4, ushort, ushort, ushort, ushort, ushort, 0x0F, s0123)       \
    OCL_INTR_P3_PROMOTE(func, 16, 8, ushort, ushort, ushort, ushort, ushort, 0xFF, s01234567)   \
    OCL_INTR_P3_NATIVE(func,    16, ushort, ushort, ushort, ushort, ushort)

#define OCL_INTR_P3_iIn_iIn_iIn_iIn(func)                                                       \
    OCL_INTR_P3_PROMOTE(func, 16, 1, int, int, int, int, ushort, 0x01, s0)                      \
    OCL_INTR_P3_PROMOTE(func, 16, 2, int, int, int, int, ushort, 0x03, s01)                     \
    OCL_INTR_P3_PROMOTE(func, 16, 3, int, int, int, int, ushort, 0x07, s012)                    \
    OCL_INTR_P3_PROMOTE(func, 16, 4, int, int, int, int, ushort, 0x0F, s0123)                   \
    OCL_INTR_P3_PROMOTE(func, 16, 8, int, int, int, int, ushort, 0xFF, s01234567)               \
    OCL_INTR_P3_NATIVE(func,    16, int, int, int, int, ushort)

#define OCL_INTR_P3_uIn_uIn_uIn_uIn(func)                                                       \
    OCL_INTR_P3_PROMOTE(func, 16, 1, uint, uint, uint, uint, ushort, 0x01, s0)                  \
    OCL_INTR_P3_PROMOTE(func, 16, 2, uint, uint, uint, uint, ushort, 0x03, s01)                 \
    OCL_INTR_P3_PROMOTE(func, 16, 3, uint, uint, uint, uint, ushort, 0x07, s012)                \
    OCL_INTR_P3_PROMOTE(func, 16, 4, uint, uint, uint, uint, ushort, 0x0F, s0123)               \
    OCL_INTR_P3_PROMOTE(func, 16, 8, uint, uint, uint, uint, ushort, 0xFF, s01234567)           \
    OCL_INTR_P3_NATIVE(func,    16, uint, uint, uint, uint, ushort)

#define OCL_INTR_P3_iLn_iLn_iLn_iLn(func)                                                       \
    OCL_INTR_P3_PROMOTE(func, 8,  1, long, long, long, long, uchar, 0x01, s0)                   \
    OCL_INTR_P3_PROMOTE(func, 8,  2, long, long, long, long, uchar, 0x03, s01)                  \
    OCL_INTR_P3_PROMOTE(func, 8,  3, long, long, long, long, uchar, 0x07, s012)                 \
    OCL_INTR_P3_PROMOTE(func, 8,  4, long, long, long, long, uchar, 0x0F, s0123)                \
    OCL_INTR_P3_NATIVE(func,     8, long, long, long, long, uchar)                              \
    OCL_INTR_P3_EXPAND(func, 8, 16, long, long, long, long)

#define OCL_INTR_P3_uLn_uLn_uLn_uLn(func)                                                       \
    OCL_INTR_P3_PROMOTE(func, 8,  1, ulong, ulong, ulong, ulong, uchar, 0x01, s0)               \
    OCL_INTR_P3_PROMOTE(func, 8,  2, ulong, ulong, ulong, ulong, uchar, 0x03, s01)              \
    OCL_INTR_P3_PROMOTE(func, 8,  3, ulong, ulong, ulong, ulong, uchar, 0x07, s012)             \
    OCL_INTR_P3_PROMOTE(func, 8,  4, ulong, ulong, ulong, ulong, uchar, 0x0F, s0123)            \
    OCL_INTR_P3_NATIVE(func,     8, ulong, ulong, ulong, ulong, uchar)                          \
    OCL_INTR_P3_EXPAND(func, 8, 16, ulong, ulong, ulong, ulong)

/// func
///
#define OCL_FUNC_P2_iCn_iCn_iC1(func)                   \
    OCL_FUNC_P2_VS(func,  2, char, char, char)          \
    OCL_FUNC_P2_VS(func,  3, char, char, char)          \
    OCL_FUNC_P2_VS(func,  4, char, char, char)          \
    OCL_FUNC_P2_VS(func,  8, char, char, char)          \
    OCL_FUNC_P2_VS(func, 16, char, char, char)

#define OCL_FUNC_P2_uCn_uCn_uC1(func)                   \
    OCL_FUNC_P2_VS(func,  2, uchar, uchar, uchar)       \
    OCL_FUNC_P2_VS(func,  3, uchar, uchar, uchar)       \
    OCL_FUNC_P2_VS(func,  4, uchar, uchar, uchar)       \
    OCL_FUNC_P2_VS(func,  8, uchar, uchar, uchar)       \
    OCL_FUNC_P2_VS(func, 16, uchar, uchar, uchar)

#define OCL_FUNC_P2_iSn_iSn_iS1(func)                   \
    OCL_FUNC_P2_VS(func,  2, short, short, short)       \
    OCL_FUNC_P2_VS(func,  3, short, short, short)       \
    OCL_FUNC_P2_VS(func,  4, short, short, short)       \
    OCL_FUNC_P2_VS(func,  8, short, short, short)       \
    OCL_FUNC_P2_VS(func, 16, short, short, short)

#define OCL_FUNC_P2_uSn_uSn_uS1(func)                   \
    OCL_FUNC_P2_VS(func,  2, ushort, ushort, ushort)    \
    OCL_FUNC_P2_VS(func,  3, ushort, ushort, ushort)    \
    OCL_FUNC_P2_VS(func,  4, ushort, ushort, ushort)    \
    OCL_FUNC_P2_VS(func,  8, ushort, ushort, ushort)    \
    OCL_FUNC_P2_VS(func, 16, ushort, ushort, ushort)

#define OCL_FUNC_P2_iIn_iIn_iI1(func)                   \
    OCL_FUNC_P2_VS(func,  2, int, int, int)             \
    OCL_FUNC_P2_VS(func,  3, int, int, int)             \
    OCL_FUNC_P2_VS(func,  4, int, int, int)             \
    OCL_FUNC_P2_VS(func,  8, int, int, int)             \
    OCL_FUNC_P2_VS(func, 16, int, int, int)

#define OCL_FUNC_P2_uIn_uIn_uI1(func)                   \
    OCL_FUNC_P2_VS(func,  2, uint, uint, uint)          \
    OCL_FUNC_P2_VS(func,  3, uint, uint, uint)          \
    OCL_FUNC_P2_VS(func,  4, uint, uint, uint)          \
    OCL_FUNC_P2_VS(func,  8, uint, uint, uint)          \
    OCL_FUNC_P2_VS(func, 16, uint, uint, uint)

#define OCL_FUNC_P2_iLn_iLn_iL1(func)                   \
    OCL_FUNC_P2_VS(func,  2, long, long, long)          \
    OCL_FUNC_P2_VS(func,  3, long, long, long)          \
    OCL_FUNC_P2_VS(func,  4, long, long, long)          \
    OCL_FUNC_P2_VS(func,  8, long, long, long)          \
    OCL_FUNC_P2_VS(func, 16, long, long, long)

#define OCL_FUNC_P2_uLn_uLn_uL1(func)                   \
    OCL_FUNC_P2_VS(func,  2, ulong, ulong, ulong)       \
    OCL_FUNC_P2_VS(func,  3, ulong, ulong, ulong)       \
    OCL_FUNC_P2_VS(func,  4, ulong, ulong, ulong)       \
    OCL_FUNC_P2_VS(func,  8, ulong, ulong, ulong)       \
    OCL_FUNC_P2_VS(func, 16, ulong, ulong, ulong)

#define OCL_FUNC_P3_iCn_iCn_iC1_iC1(func)                       \
    OCL_FUNC_P3_VSS(func,  2, char, char, char, char)           \
    OCL_FUNC_P3_VSS(func,  3, char, char, char, char)           \
    OCL_FUNC_P3_VSS(func,  4, char, char, char, char)           \
    OCL_FUNC_P3_VSS(func,  8, char, char, char, char)           \
    OCL_FUNC_P3_VSS(func, 16, char, char, char, char)

#define OCL_FUNC_P3_uCn_uCn_uC1_uC1(func)                       \
    OCL_FUNC_P3_VSS(func,  2, uchar, uchar, uchar, uchar)       \
    OCL_FUNC_P3_VSS(func,  3, uchar, uchar, uchar, uchar)       \
    OCL_FUNC_P3_VSS(func,  4, uchar, uchar, uchar, uchar)       \
    OCL_FUNC_P3_VSS(func,  8, uchar, uchar, uchar, uchar)       \
    OCL_FUNC_P3_VSS(func, 16, uchar, uchar, uchar, uchar)

#define OCL_FUNC_P3_iSn_iSn_iS1_iS1(func)                       \
    OCL_FUNC_P3_VSS(func,  2, short, short, short, short)       \
    OCL_FUNC_P3_VSS(func,  3, short, short, short, short)       \
    OCL_FUNC_P3_VSS(func,  4, short, short, short, short)       \
    OCL_FUNC_P3_VSS(func,  8, short, short, short, short)       \
    OCL_FUNC_P3_VSS(func, 16, short, short, short, short)

#define OCL_FUNC_P3_uSn_uSn_uS1_uS1(func)                       \
    OCL_FUNC_P3_VSS(func,  2, ushort, ushort, ushort, ushort)   \
    OCL_FUNC_P3_VSS(func,  3, ushort, ushort, ushort, ushort)   \
    OCL_FUNC_P3_VSS(func,  4, ushort, ushort, ushort, ushort)   \
    OCL_FUNC_P3_VSS(func,  8, ushort, ushort, ushort, ushort)   \
    OCL_FUNC_P3_VSS(func, 16, ushort, ushort, ushort, ushort)

#define OCL_FUNC_P3_iIn_iIn_iI1_iI1(func)                       \
    OCL_FUNC_P3_VSS(func,  2, int, int, int, int)               \
    OCL_FUNC_P3_VSS(func,  3, int, int, int, int)               \
    OCL_FUNC_P3_VSS(func,  4, int, int, int, int)               \
    OCL_FUNC_P3_VSS(func,  8, int, int, int, int)               \
    OCL_FUNC_P3_VSS(func, 16, int, int, int, int)

#define OCL_FUNC_P3_uIn_uIn_uI1_uI1(func)                       \
    OCL_FUNC_P3_VSS(func,  2, uint, uint, uint, uint)           \
    OCL_FUNC_P3_VSS(func,  3, uint, uint, uint, uint)           \
    OCL_FUNC_P3_VSS(func,  4, uint, uint, uint, uint)           \
    OCL_FUNC_P3_VSS(func,  8, uint, uint, uint, uint)           \
    OCL_FUNC_P3_VSS(func, 16, uint, uint, uint, uint)

#define OCL_FUNC_P3_iLn_iLn_iL1_iL1(func)                       \
    OCL_FUNC_P3_VSS(func,  2, long, long, long, long)           \
    OCL_FUNC_P3_VSS(func,  3, long, long, long, long)           \
    OCL_FUNC_P3_VSS(func,  4, long, long, long, long)           \
    OCL_FUNC_P3_VSS(func,  8, long, long, long, long)           \
    OCL_FUNC_P3_VSS(func, 16, long, long, long, long)

#define OCL_FUNC_P3_uLn_uLn_uL1_uL1(func)                       \
    OCL_FUNC_P3_VSS(func,  2, ulong, ulong, ulong, ulong)       \
    OCL_FUNC_P3_VSS(func,  3, ulong, ulong, ulong, ulong)       \
    OCL_FUNC_P3_VSS(func,  4, ulong, ulong, ulong, ulong)       \
    OCL_FUNC_P3_VSS(func,  8, ulong, ulong, ulong, ulong)       \
    OCL_FUNC_P3_VSS(func, 16, ulong, ulong, ulong, ulong)
