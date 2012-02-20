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
//  mic_cl_vloadvstore_declaration.h
///////////////////////////////////////////////////////////
#pragma once

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
#define float1  float
#define double1 double

#define vload_half1 vload_half
#define vstore_half1 vstore_half
#define vstore_half1_rte vstore_half_rte
#define vstore_half1_rtz vstore_half_rtz
#define vstore_half1_rtp vstore_half_rtp
#define vstore_half1_rtn vstore_half_rtn

/// Native support
#define OCL_INTR_LD_NATIVE(func, rnd, nW, oT, sT, pT, mT)                                       \
    oT##nW __attribute__((overloadable)) func##nW##rnd(sT, const __private pT *);               \
    oT##nW __attribute__((overloadable)) mask_##func##nW##rnd(mT, sT, const __private pT *);    \
    _LD_FROM(  __global, func, rnd, nW, oT, sT, pT)                                             \
    _LD_FROM(__constant, func, rnd, nW, oT, sT, pT)                                             \
    _LD_FROM(   __local, func, rnd, nW, oT, sT, pT)

#define OCL_INTR_ST_NATIVE(func, rnd, nW, iT, sT, pT, mT)                                       \
    void __attribute__((overloadable)) func##nW##rnd(iT##nW, sT, __private pT *);               \
    void __attribute__((overloadable)) mask_##func##nW##rnd(mT, iT##nW, sT, __private pT *);    \
    _ST_TO(__global, func, rnd, nW, iT, sT, pT)                                                 \
    _ST_TO( __local, func, rnd, nW, iT, sT, pT)

/// Promote to wider vector
#define OCL_INTR_LD_PROMOTE(func, rnd, nW, oW, oT, sT, pT, mT, msk, pat)                        \
    oT##nW __attribute__((overloadable)) mask_##func##nW##rnd(mT, sT, const __private pT *);    \
    oT##oW __attribute__((overloadable)) func##oW##rnd(sT offset, const __private pT *p)        \
    {                                                                                           \
        oT##nW res = mask_##func##nW##rnd(msk, offset, p);                                      \
        return res.pat;                                                                         \
    }                                                                                           \
    _LD_FROM(  __global, func, rnd, oW, oT, sT, pT)                                             \
    _LD_FROM(__constant, func, rnd, oW, oT, sT, pT)                                             \
    _LD_FROM(   __local, func, rnd, oW, oT, sT, pT)

#define OCL_INTR_ST_PROMOTE(func, rnd, nW, oW, iT, sT, pT, mT, msk, pat)                        \
    void __attribute__((overloadable)) mask_##func##nW##rnd(mT, iT##nW, sT, __private pT *);    \
    void __attribute__((overloadable)) func##oW##rnd(iT##oW x, sT offset, __private pT *p)      \
    {                                                                                           \
        iT##nW valx;                                                                            \
        valx.pat = x;                                                                           \
        mask_##func##nW##rnd(msk, valx, offset, p);                                             \
    }                                                                                           \
    _ST_TO(__global, func, rnd, oW, iT, sT, pT)                                                 \
    _ST_TO( __local, func, rnd, oW, iT, sT, pT)

/// Expand to narrow vector
#define OCL_INTR_LD_EXPAND(func, rnd, nW, oW, oT, sT, pT)                                       \
    oT##nW __attribute__((overloadable)) func##nW##rnd(sT, const __private pT *);               \
    oT##oW __attribute__((overloadable)) func##oW##rnd(sT offset, const __private pT *p)        \
    {                                                                                           \
        oT##oW res;                                                                             \
        _EXPANDLD_To##nW##_From##oW(res, func, rnd, offset, p);                                 \
        return res;                                                                             \
    }                                                                                           \
    _LD_FROM(  __global, func, rnd, oW, oT, sT, pT)                                             \
    _LD_FROM(__constant, func, rnd, oW, oT, sT, pT)                                             \
    _LD_FROM(   __local, func, rnd, oW, oT, sT, pT)

#define OCL_INTR_ST_EXPAND(func, rnd, nW, oW, iT, sT, pT)                                       \
    void __attribute__((overloadable)) func##nW##rnd(iT##nW, sT, __private pT *);               \
    void __attribute__((overloadable)) func##oW##rnd(iT##oW x, sT offset, __private pT *p)      \
    {                                                                                           \
        _EXPANDST_To##nW##_From##oW(func, rnd, x, offset, p);                                   \
    }                                                                                           \
    _ST_TO(__global, func, rnd, oW, iT, sT, pT)                                                 \
    _ST_TO( __local, func, rnd, oW, iT, sT, pT)

/// How expansion is done
// basic step
#define _EXPANDLD_1x(n, res, func, offset, p)   \
    res.lo = func(offset + 0, p);               \
    res.hi = func(offset + n, p);

#define _EXPANDST_1x(n, func, x, offset, p)     \
    func(x.lo, offset + 0, p);                  \
    func(x.hi, offset + n, p);

// v16 -> v8
#define _EXPANDLD_To8_From16(res, func, rnd, offset, p)     \
    _EXPANDLD_1x(8, res, func##8##rnd, offset, p)

#define _EXPANDST_To8_From16(func, rnd, x, offset, p)       \
    _EXPANDST_1x(8, func##8##rnd, x, offset, p)

// v8 -> v4
#define _EXPANDLD_To4_From8(res, func, rnd, offset, p)      \
    _EXPANDLD_1x(4, res, func##4##rnd, offset, p)

#define _EXPANDST_To4_From8(func, rnd, x, offset, p)        \
    _EXPANDST_1x(4, func##4##rnd, x, offset, p)

// v16 -> v4
#define _EXPANDLD_To4_From16(res, func, rnd, offset, p)     \
    _EXPANDLD_1x(0, res.lo, func##4##rnd, offset + 0, p)    \
    _EXPANDLD_1x(4, res.hi, func##4##rnd, offset + 8, p)

#define _EXPANDST_To4_From16(func, rnd, x, offset, p)       \
    _EXPANDST_1x(0, func##4##rnd, x.lo, offset + 0, p)      \
    _EXPANDST_1x(4, func##4##rnd, x.hi, offset + 8, p)

// support of address space
#define _LD_FROM(as, func, rnd, oW, oT, sT, pT)                                     \
    oT##oW __attribute__((overloadable)) func##oW##rnd(sT offset, const as pT *p)   \
    {                                                                               \
        return func##oW##rnd(offset, (const pT *)p);                                \
    }

#define _ST_TO(as, func, rnd, oW, iT, sT, pT)                                       \
    void __attribute__((overloadable)) func##oW##rnd(iT##oW x, sT offset, as pT *p) \
    {                                                                               \
        func##oW##rnd(x, offset, (pT *)p);                                          \
    }

/// oT0 := ld(size_t, pT*)
///
#define OCL_INTR_LD_iCn(func, rnd)                                                          \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 1, char, size_t, char, ushort, 0x01, s0)             \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 2, char, size_t, char, ushort, 0x03, s01)            \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 3, char, size_t, char, ushort, 0x07, s012)           \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 4, char, size_t, char, ushort, 0x0F, s0123)          \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 8, char, size_t, char, ushort, 0xFF, s01234567)      \
    OCL_INTR_LD_NATIVE (func, rnd,    16, char, size_t, char, ushort)

#define OCL_INTR_LD_uCn(func, rnd)                                                          \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 1, uchar, size_t, uchar, ushort, 0x01, s0)           \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 2, uchar, size_t, uchar, ushort, 0x03, s01)          \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 3, uchar, size_t, uchar, ushort, 0x07, s012)         \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 4, uchar, size_t, uchar, ushort, 0x0F, s0123)        \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 8, uchar, size_t, uchar, ushort, 0xFF, s01234567)    \
    OCL_INTR_LD_NATIVE (func, rnd,    16, uchar, size_t, uchar, ushort)

#define OCL_INTR_LD_iSn(func, rnd)                                                          \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 1, short, size_t, short, ushort, 0x01, s0)           \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 2, short, size_t, short, ushort, 0x03, s01)          \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 3, short, size_t, short, ushort, 0x07, s012)         \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 4, short, size_t, short, ushort, 0x0F, s0123)        \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 8, short, size_t, short, ushort, 0xFF, s01234567)    \
    OCL_INTR_LD_NATIVE (func, rnd,    16, short, size_t, short, ushort)

#define OCL_INTR_LD_uSn(func, rnd)                                                          \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 1, ushort, size_t, ushort, ushort, 0x01, s0)         \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 2, ushort, size_t, ushort, ushort, 0x03, s01)        \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 3, ushort, size_t, ushort, ushort, 0x07, s012)       \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 4, ushort, size_t, ushort, ushort, 0x0F, s0123)      \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 8, ushort, size_t, ushort, ushort, 0xFF, s01234567)  \
    OCL_INTR_LD_NATIVE (func, rnd,    16, ushort, size_t, ushort, ushort)

#define OCL_INTR_LD_iIn(func, rnd)                                                          \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 1, int, size_t, int, ushort, 0x01, s0)               \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 2, int, size_t, int, ushort, 0x03, s01)              \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 3, int, size_t, int, ushort, 0x07, s012)             \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 4, int, size_t, int, ushort, 0x0F, s0123)            \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 8, int, size_t, int, ushort, 0xFF, s01234567)        \
    OCL_INTR_LD_NATIVE (func, rnd,    16, int, size_t, int, ushort)

#define OCL_INTR_LD_uIn(func, rnd)                                                          \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 1, uint, size_t, uint, ushort, 0x01, s0)             \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 2, uint, size_t, uint, ushort, 0x03, s01)            \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 3, uint, size_t, uint, ushort, 0x07, s012)           \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 4, uint, size_t, uint, ushort, 0x0F, s0123)          \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 8, uint, size_t, uint, ushort, 0xFF, s01234567)      \
    OCL_INTR_LD_NATIVE (func, rnd,    16, uint, size_t, uint, ushort)

#define OCL_INTR_LD_iLn(func, rnd)                                                          \
    OCL_INTR_LD_PROMOTE(func, rnd, 8,  1, long, size_t, long, uchar, 0x01, s0)              \
    OCL_INTR_LD_PROMOTE(func, rnd, 8,  2, long, size_t, long, uchar, 0x03, s01)             \
    OCL_INTR_LD_PROMOTE(func, rnd, 8,  3, long, size_t, long, uchar, 0x07, s012)            \
    OCL_INTR_LD_PROMOTE(func, rnd, 8,  4, long, size_t, long, uchar, 0x0F, s0123)           \
    OCL_INTR_LD_NATIVE (func, rnd,     8, long, size_t, long, uchar) \
    OCL_INTR_LD_EXPAND (func, rnd, 8, 16, long, size_t, long)

#define OCL_INTR_LD_uLn(func, rnd)                                                          \
    OCL_INTR_LD_PROMOTE(func, rnd, 8,  1, ulong, size_t, ulong, uchar, 0x01, s0)            \
    OCL_INTR_LD_PROMOTE(func, rnd, 8,  2, ulong, size_t, ulong, uchar, 0x03, s01)           \
    OCL_INTR_LD_PROMOTE(func, rnd, 8,  3, ulong, size_t, ulong, uchar, 0x07, s012)          \
    OCL_INTR_LD_PROMOTE(func, rnd, 8,  4, ulong, size_t, ulong, uchar, 0x0F, s0123)         \
    OCL_INTR_LD_NATIVE (func, rnd,     8, ulong, size_t, ulong, uchar)                      \
    OCL_INTR_LD_EXPAND (func, rnd, 8, 16, ulong, size_t, ulong)

#define OCL_INTR_LD_Fn(func, rnd)                                                           \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 1, float, size_t, float, ushort, 0x01, s0)           \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 2, float, size_t, float, ushort, 0x03, s01)          \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 3, float, size_t, float, ushort, 0x07, s012)         \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 4, float, size_t, float, ushort, 0x0F, s0123)        \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 8, float, size_t, float, ushort, 0xFF, s01234567)    \
    OCL_INTR_LD_NATIVE (func, rnd,    16, float, size_t, float, ushort)

#define OCL_INTR_LD_Dn(func, rnd)                                                           \
    OCL_INTR_LD_PROMOTE(func, rnd, 8,  1, double, size_t, double, uchar, 0x01, s0)          \
    OCL_INTR_LD_PROMOTE(func, rnd, 8,  2, double, size_t, double, uchar, 0x03, s01)         \
    OCL_INTR_LD_PROMOTE(func, rnd, 8,  3, double, size_t, double, uchar, 0x07, s012)        \
    OCL_INTR_LD_PROMOTE(func, rnd, 8,  4, double, size_t, double, uchar, 0x0F, s0123)       \
    OCL_INTR_LD_NATIVE (func, rnd,     8, double, size_t, double, uchar) \
    OCL_INTR_LD_EXPAND (func, rnd, 8, 16, double, size_t, double)

#define OCL_INTR_LDHALF_Fn(func, rnd)                                                       \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 1, float, size_t, half, ushort, 0x01, s0)            \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 2, float, size_t, half, ushort, 0x03, s01)           \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 3, float, size_t, half, ushort, 0x07, s012)          \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 4, float, size_t, half, ushort, 0x0F, s0123)         \
    OCL_INTR_LD_PROMOTE(func, rnd, 16, 8, float, size_t, half, ushort, 0xFF, s01234567)     \
    OCL_INTR_LD_NATIVE (func, rnd,    16, float, size_t, half, ushort)


/// oT0 := st(size_t, pT*)
///
#define OCL_INTR_ST_iCn(func, rnd)                                                          \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 1, char, size_t, char, ushort, 0x01, s0)             \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 2, char, size_t, char, ushort, 0x03, s01)            \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 3, char, size_t, char, ushort, 0x07, s012)           \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 4, char, size_t, char, ushort, 0x0F, s0123)          \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 8, char, size_t, char, ushort, 0xFF, s01234567)      \
    OCL_INTR_ST_NATIVE (func, rnd,    16, char, size_t, char, ushort)

#define OCL_INTR_ST_uCn(func, rnd)                                                          \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 1, uchar, size_t, uchar, ushort, 0x01, s0)           \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 2, uchar, size_t, uchar, ushort, 0x03, s01)          \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 3, uchar, size_t, uchar, ushort, 0x07, s012)         \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 4, uchar, size_t, uchar, ushort, 0x0F, s0123)        \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 8, uchar, size_t, uchar, ushort, 0xFF, s01234567)    \
    OCL_INTR_ST_NATIVE (func, rnd,    16, uchar, size_t, uchar, ushort)

#define OCL_INTR_ST_iSn(func, rnd)                                                          \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 1, short, size_t, short, ushort, 0x01, s0)           \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 2, short, size_t, short, ushort, 0x03, s01)          \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 3, short, size_t, short, ushort, 0x07, s012)         \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 4, short, size_t, short, ushort, 0x0F, s0123)        \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 8, short, size_t, short, ushort, 0xFF, s01234567)    \
    OCL_INTR_ST_NATIVE (func, rnd,    16, short, size_t, short, ushort)

#define OCL_INTR_ST_uSn(func, rnd)                                                          \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 1, ushort, size_t, ushort, ushort, 0x01, s0)         \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 2, ushort, size_t, ushort, ushort, 0x03, s01)        \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 3, ushort, size_t, ushort, ushort, 0x07, s012)       \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 4, ushort, size_t, ushort, ushort, 0x0F, s0123)      \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 8, ushort, size_t, ushort, ushort, 0xFF, s01234567)  \
    OCL_INTR_ST_NATIVE (func, rnd,    16, ushort, size_t, ushort, ushort)

#define OCL_INTR_ST_iIn(func, rnd)                                                          \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 1, int, size_t, int, ushort, 0x01, s0)               \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 2, int, size_t, int, ushort, 0x03, s01)              \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 3, int, size_t, int, ushort, 0x07, s012)             \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 4, int, size_t, int, ushort, 0x0F, s0123)            \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 8, int, size_t, int, ushort, 0xFF, s01234567)        \
    OCL_INTR_ST_NATIVE (func, rnd,    16, int, size_t, int, ushort)

#define OCL_INTR_ST_uIn(func, rnd)                                                          \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 1, uint, size_t, uint, ushort, 0x01, s0)             \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 2, uint, size_t, uint, ushort, 0x03, s01)            \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 3, uint, size_t, uint, ushort, 0x07, s012)           \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 4, uint, size_t, uint, ushort, 0x0F, s0123)          \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 8, uint, size_t, uint, ushort, 0xFF, s01234567)      \
    OCL_INTR_ST_NATIVE (func, rnd,    16, uint, size_t, uint, ushort)

#define OCL_INTR_ST_iLn(func, rnd)                                                          \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  1, long, size_t, long, ushort, 0x01, s0)             \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  2, long, size_t, long, ushort, 0x03, s01)            \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  3, long, size_t, long, ushort, 0x07, s012)           \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  4, long, size_t, long, ushort, 0x0F, s0123)          \
    OCL_INTR_ST_NATIVE (func, rnd,     8, long, size_t, long, ushort) \
    OCL_INTR_ST_EXPAND (func, rnd, 8, 16, long, size_t, long)

#define OCL_INTR_ST_uLn(func, rnd)                                                          \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  1, ulong, size_t, ulong, ushort, 0x01, s0)           \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  2, ulong, size_t, ulong, ushort, 0x03, s01)          \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  3, ulong, size_t, ulong, ushort, 0x07, s012)         \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  4, ulong, size_t, ulong, ushort, 0x0F, s0123)        \
    OCL_INTR_ST_NATIVE (func, rnd,     8, ulong, size_t, ulong, ushort)                     \
    OCL_INTR_ST_EXPAND (func, rnd, 8, 16, ulong, size_t, ulong)

#define OCL_INTR_ST_Fn(func, rnd)                                                           \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 1, float, size_t, float, ushort, 0x01, s0)           \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 2, float, size_t, float, ushort, 0x03, s01)          \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 3, float, size_t, float, ushort, 0x07, s012)         \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 4, float, size_t, float, ushort, 0x0F, s0123)        \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 8, float, size_t, float, ushort, 0xFF, s01234567)    \
    OCL_INTR_ST_NATIVE (func, rnd,    16, float, size_t, float, ushort)

#define OCL_INTR_ST_Dn(func, rnd)                                                           \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  1, double, size_t, double, uchar, 0x01, s0)          \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  2, double, size_t, double, uchar, 0x03, s01)         \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  3, double, size_t, double, uchar, 0x07, s012)        \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  4, double, size_t, double, uchar, 0x0F, s0123)       \
    OCL_INTR_ST_NATIVE (func, rnd,     8, double, size_t, double, uchar)                    \
    OCL_INTR_ST_EXPAND (func, rnd, 8, 16, double, size_t, double)

#define OCL_INTR_STHALF_Fn(func, rnd)                                                       \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 1, float, size_t, half, ushort, 0x01, s0)            \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 2, float, size_t, half, ushort, 0x03, s01)           \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 3, float, size_t, half, ushort, 0x07, s012)          \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 4, float, size_t, half, ushort, 0x0F, s0123)         \
    OCL_INTR_ST_PROMOTE(func, rnd, 16, 8, float, size_t, half, ushort, 0xFF, s01234567)     \
    OCL_INTR_ST_NATIVE (func, rnd,    16, float, size_t, half, ushort)

#define OCL_INTR_STHALF_Dn(func, rnd)                                                       \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  1, double, size_t, half, uchar, 0x01, s0)            \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  2, double, size_t, half, uchar, 0x03, s01)           \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  3, double, size_t, half, uchar, 0x07, s012)          \
    OCL_INTR_ST_PROMOTE(func, rnd, 8,  4, double, size_t, half, uchar, 0x0F, s0123)         \
    OCL_INTR_ST_NATIVE (func, rnd,     8, double, size_t, half, uchar)                      \
    OCL_INTR_ST_EXPAND (func, rnd, 8, 16, double, size_t, half)
