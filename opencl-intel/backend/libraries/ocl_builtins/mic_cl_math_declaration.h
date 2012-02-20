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

#pragma once

#include "cl_types2.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __x86_64__
# error "MIC only runs with EM64T enabled!"
#endif

/*
 * NOTE ON DESIGN: To map OCL builtin types onto SVML functions, it'd better to
 * provide just two interfaces from OCL SVML library, i.e. one non-masked one
 * and the other masked one, on native types, i.e. float16/int16/double8/long8.
 * Such a design provides similar interface to MIC intrinsic design and reduce
 * mandotary interfaces significantly. OCL types not supported natively, like
 * float2, float3, will be mapped onto native one, e.g., float16, and call the
 * masked function with proper mask value, e.g. 0x03 for float2 and 0x07 for
 * float3.
 *
 * However, as the current OCL SVML library is not availble with the new
 * interface design, the code here has both guarded by
 * 'USE_OCL_SVML_MASK_INTERFACE' macro.
 */

// Comment out this when OCL SVML mask interface is available
#undef USE_OCL_SVML_MASK_INTERFACE
// Uncomment out this when OCL SVML mask interface is available
//#define USE_OCL_SVML_MASK_INTERFACE

#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_MASK_FUNCTION(oclfunc,native)  __ocl_svml_b1##oclfunc##native##_mask
#endif
#define OCL_SVML_FUNCTION(oclfunc,native)       __ocl_svml_b1##oclfunc##native


// convention:
//
// - naming convention for macros for OCL builtin collectioins
//     OCL_{IMPL}_P{n}_{SET}
//
// - naming convention for macros for single OCL builtin function
//     OCL_{IMPL}_P{n}_{T0}_{T1}_.._{Tn}
//
// , where
//
// {IMPL} - specify the underlying implementation and possible values are
//          + SVML - OCL builtins are based on SVML functions
//          + INTR - OCL builtins are implemented through intrinsic
//          + FUNC - OCL builtins are based on other builtins, in most cases,
//                   just widening scalar into vector
//
// P{n}   - specify the number of parameters
//
// {SET}  - specify the collection (gentype) and current valid values are
//          + Fn - float/float2/float3/float4/float8/float16
//          + Dn - double/double2/double3/double4/douebl8/double16
//
// {Tn}   - specify the type of the {n}th parameter
//          + F{n} - float{n}
//          + D{n} - double{n}
//          + I{n} - int{n}/uint{n}
//          + L{n} - long{n}/ulong{n}
//        modifier 'p' could be prefixed to specify a pointer type to
//        corresponding vector, e.g. 'pF8' is 'float8*', 'pI2' is 'int2*'.
//
// - SVML function naming convention (current interface):
//
//   __ocl_svml_{arch}_{func}{type}[_{variant}]
//
// , where
// {arch}    - specify the target arch
// {func}    - specify the function name
// {type}    - specify the data type
//             + '1'   double
//             + '2'   double2
//             + '3'   double3
//             + '4'   double4
//             + '8'   double8
//             + '16'  double16
//             + 'f1'  float
//             + 'f2'  float2
//             + 'f3'  float3
//             + 'f4'  float4
//             + 'f8'  float8
//             + 'f16' float16
// {variant} - optional and could be 'native'
//
// - SVML function naming convention (new interface):
//
//   __ocl_svml_{arch}[_{mask}]_{func}{type}[_{variant}]
//
// , where
// {arch}    - specify the target arch
// {func}    - specify the function name
// {type}    - specify the data type, only native types are supported
//             + '8'   double8
//             + 'f16' float16
// {variant} - optional and could be 'native'
//


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_F1_F1(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16); \
    __attribute__((overloadable)) float func(float x)                               \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo.lo.lo = x;                                                       \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x01, valx);  \
        return res.lo.lo.lo.lo;                                                     \
    }
#else
#define OCL_SVML_P1_F1_F1(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) float OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(float); \
    __attribute__((overloadable)) float func(float x)                               \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(x);                        \
    }
#endif

// float2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_F2_F2(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16); \
    __attribute__((overloadable)) float2 func(float2 x)                             \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo.lo = x;                                                          \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x03, valx);  \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P1_F2_F2(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) float2 OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(float2); \
    __attribute__((overloadable)) float2 func(float2 x)                             \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(x);                        \
    }
#endif

// float3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_F3_F3(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16); \
    float3 __attribute__((overloadable)) func(float3 x)                             \
    {                                                                               \
        float16 valx;                                                               \
        valx.s012 = x;                                                              \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x07, valx);  \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P1_F3_F3(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) float3 OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(float3); \
    float3 __attribute__((overloadable)) func(float3 x)                             \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(x);                        \
    }
#endif

// float4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_F4_F4(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16); \
    __attribute__((overloadable)) float4 func(float4 x)                             \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo = x;                                                             \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x0F, valx);  \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P1_F4_F4(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(float4); \
    __attribute__((overloadable)) float4 func(float4 x)                             \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(x);                        \
    }
#endif

// float8
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_F8_F8(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16); \
    __attribute__((overloadable)) float8 func(float8 x)                             \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo = x;                                                                    \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0xFF, valx);  \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P1_F8_F8(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) float8 OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(float8); \
    __attribute__((overloadable)) float8 func(float8 x)                             \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(x);                        \
    }
#endif

// float16
#define OCL_SVML_P1_F16_F16(func, svmlfunc, native)                                 \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(float16); \
    __attribute__((overloadable)) float16 func(float16 x)                           \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(x);                       \
    }

// double
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_D1_D1(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8); \
    __attribute__((overloadable)) double func(double x)                             \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo.lo.lo = x;                                                          \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x01, valx);    \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P1_D1_D1(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) double OCL_SVML_FUNCTION(_##svmlfunc##1,native)(double); \
    __attribute__((overloadable)) double func(double x)                             \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##1,native)(x);                         \
    }
#endif

// double2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_D2_D2(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8); \
    __attribute__((overloadable)) double2 func(double2 x)                           \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo.lo = x;                                                             \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x03, valx);    \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P1_D2_D2(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) double2 OCL_SVML_FUNCTION(_##svmlfunc##2,native)(double2); \
    __attribute__((overloadable)) double2 func(double2 x)                           \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##2,native)(x);                         \
    }
#endif

// double3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_D3_D3(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8); \
    __attribute__((overloadable)) double3 func(double3 x)                           \
    {                                                                               \
        double8 valx;                                                               \
        valx.s012 = x;                                                              \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x07, valx);    \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P1_D3_D3(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) double3 OCL_SVML_FUNCTION(_##svmlfunc##3,native)(double3); \
    __attribute__((overloadable)) double3 func(double3 x)                           \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##3,native)(x);                         \
    }
#endif

// double4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_D4_D4(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8); \
    __attribute__((overloadable)) double4 func(double4 x)                           \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo = x;                                                                \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x0F, valx);    \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P1_D4_D4(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4,native)(double4); \
    __attribute__((overloadable)) double4 func(double4 x)                           \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##4,native)(x);                         \
    }
#endif

// double8
#define OCL_SVML_P1_D8_D8(func, svmlfunc, native)                                   \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8); \
    __attribute__((overloadable)) double8 func(double8 x)                           \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x);                         \
    }

// double16
#define OCL_SVML_P1_D16_D16(func, svmlfunc, native)                                 \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8); \
    __attribute__((overloadable)) double16 func(double16 x)                         \
    {                                                                               \
        double16 res;                                                               \
        res.lo = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.lo);                    \
        res.hi = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.hi);                    \
        return res;                                                                 \
    }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float
#define OCL_INTR_P1_F1_F1(func)                                                     \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16);           \
    __attribute__((overloadable)) float func(float x)                               \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo.lo.lo = x;                                                       \
        float16 res = mask_ ## func(0x01, valx);                                    \
        return res.lo.lo.lo.lo;                                                     \
    }

// float2
#define OCL_INTR_P1_F2_F2(func)                                                     \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16);           \
    __attribute__((overloadable)) float2 func(float2 x)                             \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo.lo = x;                                                          \
        float16 res = mask_ ## func(0x03, valx);                                    \
        return res.lo.lo.lo;                                                        \
    }

// float3
#define OCL_INTR_P1_F3_F3(func)                                                     \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16);           \
    __attribute__((overloadable)) float3 func(float3 x)                             \
    {                                                                               \
        float16 valx;                                                               \
        valx.s012 = x;                                                              \
        float16 res = mask_ ## func(0x07, valx);                                    \
        return res.s012;                                                            \
    }

// float4
#define OCL_INTR_P1_F4_F4(func)                                                     \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16);           \
    __attribute__((overloadable)) float4 func(float4 x)                             \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo = x;                                                             \
        float16 res = mask_ ## func(0x0F, valx);                                    \
        return res.lo.lo;                                                           \
    }

// float8
#define OCL_INTR_P1_F8_F8(func)                                                     \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16);           \
    __attribute__((overloadable)) float8 func(float8 x)                             \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo = x;                                                                \
        float16 res = mask_ ## func(0xFF, valx);                                    \
        return res.lo;                                                              \
    }

// float16
#define OCL_INTR_P1_F16_F16(func)                                                   \
    __attribute__((overloadable)) float16 func(float16);


// double
#define OCL_INTR_P1_D1_D1(func)                                                     \
    __attribute__((overloadable)) double8 mask_ ## func(uchar, double8);            \
    __attribute__((overloadable)) double func(double x)                             \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo.lo.lo = x;                                                          \
        double8 res = mask_ ## func(0x01, valx);                                    \
        return res.lo.lo.lo;                                                        \
    }

// double2
#define OCL_INTR_P1_D2_D2(func)                                                     \
    __attribute__((overloadable)) double8 mask_ ## func(uchar, double8);            \
    __attribute__((overloadable)) double2 func(double2 x)                           \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo.lo = x;                                                             \
        double8 res = mask_ ## func(0x03, valx);                                    \
        return res.lo.lo;                                                           \
    }

// double3
#define OCL_INTR_P1_D3_D3(func)                                                     \
    __attribute__((overloadable)) double8 mask_ ## func(uchar, double8);            \
    __attribute__((overloadable)) double3 func(double3 x)                           \
    {                                                                               \
        double8 valx;                                                               \
        valx.s012 = x;                                                              \
        double8 res = mask_ ## func(0x07, valx);                                    \
        return res.s012;                                                            \
    }

// double4
#define OCL_INTR_P1_D4_D4(func)                                                     \
    __attribute__((overloadable)) double8 mask_ ## func(uchar, double8);            \
    __attribute__((overloadable)) double4 func(double4 x)                           \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo = x;                                                                \
        double8 res = mask_ ## func(0x0F, valx);                                    \
        return res.lo;                                                              \
    }

// double8
#define OCL_INTR_P1_D8_D8(func)                                                     \
    __attribute__((overloadable)) double8 func(double8);

// double16
#define OCL_INTR_P1_D16_D16(func)                                                   \
    __attribute__((overloadable)) double8 func(double8);                            \
    __attribute__((overloadable)) double16 func(double16 x)                         \
    {                                                                               \
        double16 res;                                                               \
        res.lo = func(x.lo);                                                        \
        res.hi = func(x.hi);                                                        \
        return res;                                                                 \
    }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_F1_I1(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, _16##sign##32); \
    __attribute__((overloadable)) float func(_1##sign##32 x)                        \
    {                                                                               \
        _16##sign##32 valx;                                                         \
        valx.lo.lo.lo.lo = x;                                                       \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x01, valx);  \
        return res.lo.lo.lo.lo;                                                     \
    }
#else
#define OCL_SVML_P1_F1_I1(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(_1##sign##32); \
    __attribute__((overloadable)) float func(_1##sign##32 x)                        \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(x);                        \
    }
#endif

// float2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_F2_I2(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, _16##sign##32); \
    __attribute__((overloadable)) float2 func(_2##sign##32 x)                       \
    {                                                                               \
        _16##sign##32 valx;                                                         \
        valx.lo.lo.lo = x;                                                          \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x03, valx);  \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P1_F2_I2(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float2 OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(_2##sign##32); \
    __attribute__((overloadable)) float2 func(_2##sign##32 x)                       \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(x);                        \
    }
#endif

// float3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_F3_I3(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, _16##sign##32); \
    __attribute__((overloadable)) float3 func(_3##sign##32 x)                       \
    {                                                                               \
        _16##sign##32 valx;                                                         \
        valx.s012 = x;                                                              \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x07, valx);  \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P1_F3_I3(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float3 OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(_3##sign##32); \
    __attribute__((overloadable)) float3 func(_3##sign##32 x)                       \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(x);                        \
    }
#endif

// float4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_F4_I4(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, _16##sign##32); \
    __attribute__((overloadable)) float4 func(_4##sign##32 x)                       \
    {                                                                               \
        _16##sign##32 valx;                                                         \
        valx.lo.lo = x;                                                             \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x0F, valx);  \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P1_F4_I4(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(_4##sign##32); \
    __attribute__((overloadable)) float4 func(_4##sign##32 x)                       \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(x);                        \
    }
#endif

// float8
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_F8_I8(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, _16##sign##32); \
    __attribute__((overloadable)) float8 func(_8##sign##32 x)                       \
    {                                                                               \
        _16##sign##32 valx;                                                         \
        valx.lo = x;                                                                \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0xFF, valx);  \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P1_F8_I8(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float8 OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(_8##sign##32); \
    __attribute__((overloadable)) float8 func(_8##sign##32 x)                       \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(x);                        \
    }
#endif

// float16
#define OCL_SVML_P1_F16_I16(func, sign, svmlfunc, native)                           \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(_16##sign##32); \
    __attribute__((overloadable)) float16 func(_16##sign##32 x)                     \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(x);                       \
    }

// double
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_D1_L1(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, _8##sign##64); \
    __attribute__((overloadable)) double func(_1##sign##64 x)                       \
    {                                                                               \
        _8##sign##64 valx;                                                          \
        valx.lo.lo.lo = x;                                                          \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x01, valx);    \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P1_D1_L1(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double OCL_SVML_FUNCTION(_##svmlfunc##1,native)(_1##sign##64); \
    __attribute__((overloadable)) double func(_1##sign##64 x)                       \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##1,native)(x);                         \
    }
#endif

// double2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_D2_L2(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, _8##sign##64); \
    __attribute__((overloadable)) double2 func(_2##sign##64 x)                      \
    {                                                                               \
        _8##sign##64 valx;                                                          \
        valx.lo.lo = x;                                                             \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x03, valx);    \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P1_D2_L2(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double2 OCL_SVML_FUNCTION(_##svmlfunc##2,native)(_2##sign##64); \
    __attribute__((overloadable)) double2 func(_2##sign##64 x)                      \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##2,native)(x);                         \
    }
#endif

// double3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_D3_L3(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, _8##sign##64); \
    __attribute__((overloadable)) double3 func(_3##sign##64 x)                      \
    {                                                                               \
        _8##sign##64 valx;                                                          \
        valx.s012 = x;                                                              \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x07, valx);    \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P1_D3_L3(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double3 OCL_SVML_FUNCTION(_##svmlfunc##3,native)(_3##sign##64); \
    __attribute__((overloadable)) double3 func(_3##sign##64 x)                      \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##3,native)(x);                     \
    }
#endif

// double4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_D4_L4(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, _8##sign##64); \
    __attribute__((overloadable)) double4 func(_4##sign##64 x)                      \
    {                                                                               \
        _8##sign##64 valx;                                                          \
        valx.lo = x;                                                                \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x0F, valx);    \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P1_D4_L4(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4,native)(_4##sign##64); \
    __attribute__((overloadable)) double4 func(_4##sign##64 x)                      \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##4,native)(x);                     \
    }
#endif

// double8
#define OCL_SVML_P1_D8_L8(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(_8##sign##64); \
    __attribute__((overloadable)) double8 func(_8##sign##64 x)                      \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x);                         \
    }

// double16
#define OCL_SVML_P1_D16_L16(func, sign, svmlfunc, native)                           \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(_8##sign##64); \
    __attribute__((overloadable)) double16 func(_16##sign##64 x)                    \
    {                                                                               \
        double16 res;                                                               \
        res.lo = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.lo);                    \
        res.hi = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.hi);                    \
        return res;                                                                 \
    }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float
#define OCL_INTR_P1_F1_I1(func, sign)                                               \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, _16##sign##32);     \
    __attribute__((overloadable)) float func(_1##sign##32 x)                        \
    {                                                                               \
        _16##sign##32 valx;                                                         \
        valx.lo.lo.lo.lo = x;                                                       \
        float16 res = mask_ ## func(0x01, valx);                                    \
        return res.lo.lo.lo.lo;                                                     \
    }

// float2
#define OCL_INTR_P1_F2_I2(func, sign)                                               \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, _16##sign##32);     \
    __attribute__((overloadable)) float2 func(_2##sign##32 x)                       \
    {                                                                               \
        _16##sign##32 valx;                                                         \
        valx.lo.lo.lo = x;                                                          \
        float16 res = mask_ ## func(0x03, valx);                                    \
        return res.lo.lo.lo;                                                        \
    }

// float3
#define OCL_INTR_P1_F3_I3(func, sign)                                               \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, _16##sign##32);     \
    __attribute__((overloadable)) float3 func(_3##sign##32 x)                       \
    {                                                                               \
        _16##sign##32 valx;                                                         \
        valx.s012 = x;                                                              \
        float16 res = mask_ ## func(0x07, valx);                                    \
        return res.s012;                                                            \
    }

// float4
#define OCL_INTR_P1_F4_I4(func, sign)                                               \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, _16##sign##32);     \
    __attribute__((overloadable)) float4 func(_4##sign##32 x)                       \
    {                                                                               \
        _16##sign##32 valx;                                                         \
        valx.lo.lo = x;                                                             \
        float16 res = mask_ ## func(0x0F, valx);                                    \
        return res.lo.lo;                                                           \
    }

// float8
#define OCL_INTR_P1_F8_I8(func, sign)                                               \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, _16##sign##32);     \
    __attribute__((overloadable)) float8 func(_8##sign##32 x)                       \
    {                                                                               \
        _16##sign##32 valx;                                                         \
        valx.lo = x;                                                                \
        float16 res = mask_ ## func(0xFF, valx);                                    \
        return res.lo;                                                              \
    }

// float16
#define OCL_INTR_P1_F16_I16(func, sign)                                             \
    __attribute__((overloadable)) float16 func(_16##sign##32);

// double
#define OCL_INTR_P1_D1_L1(func, sign)                                               \
    __attribute__((overloadable)) double8 mask_ ## func(ushort, _8##sign##64);      \
    __attribute__((overloadable)) double func(_1##sign##64 x)                       \
    {                                                                               \
        _8##sign##64 valx;                                                          \
        valx.lo.lo.lo.lo = x;                                                       \
        double16 res = mask_ ## func(0x01, valx);                                   \
        return res.lo.lo.lo.lo;                                                     \
    }

// double2
#define OCL_INTR_P1_D2_L2(func, sign)                                               \
    __attribute__((overloadable)) double8 mask_ ## func(ushort, _8##sign##64);      \
    __attribute__((overloadable)) double2 func(_2##sign##64 x)                      \
    {                                                                               \
        _8##sign##64 valx;                                                          \
        valx.lo.lo.lo = x;                                                          \
        double16 res = mask_ ## func(0x03, valx);                                   \
        return res.lo.lo.lo;                                                        \
    }

// double3
#define OCL_INTR_P1_D3_L3(func, sign)                                               \
    __attribute__((overloadable)) double8 mask_ ## func(ushort, _8##sign##64);      \
    __attribute__((overloadable)) double3 func(_3##sign##64 x)                      \
    {                                                                               \
        _8##sign##64 valx;                                                          \
        valx.s012 = x;                                                              \
        double16 res = mask_ ## func(0x07, valx);                                   \
        return res.s012;                                                            \
    }

// double4
#define OCL_INTR_P1_D4_L4(func, sign)                                               \
    __attribute__((overloadable)) double8 mask_ ## func(ushort, _8##sign##64);      \
    __attribute__((overloadable)) double4 func(_4##sign##64 x)                      \
    {                                                                               \
        _8##sign##64 valx;                                                          \
        valx.lo.lo = x;                                                             \
        double16 res = mask_ ## func(0x0F, valx);                                   \
        return res.lo.lo;                                                           \
    }

// double8
#define OCL_INTR_P1_D8_L8(func, sign)                                               \
    __attribute__((overloadable)) double8 func(_8##sign##64);

// double16
#define OCL_INTR_P1_D16_L16(func, sign)                                             \
    __attribute__((overloadable)) double8 func(_8##sign##64);                       \
    __attribute__((overloadable)) double16 func(_16##sign##64 x)                    \
    {                                                                               \
        double16 res;                                                               \
        res.lo = func(x.lo);                                                        \
        res.hi = func(x.hi);                                                        \
        return res;                                                                 \
    }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_I1_F1(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _16##sign##32 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16); \
    __attribute__((overloadable)) _1##sign##32 func(float x)                        \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo.lo.lo = x;                                                       \
        _16##sign##32 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x01, valx); \
        return res.lo.lo.lo.lo;                                                     \
    }
#else
#define OCL_SVML_P1_I1_F1(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _1##sign##32 OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(float); \
    __attribute__((overloadable)) _1##sign##32 func(float x)                        \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(x);                        \
    }
#endif

// float2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_I2_F2(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _16##sign##32 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16); \
    __attribute__((overloadable)) _2##sign##32 func(float2 x)                       \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo.lo = x;                                                          \
        _16##sign##32 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x03, valx); \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P1_I2_F2(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _2##sign##32 OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(float2); \
    __attribute__((overloadable)) _2##sign##32 func(float2 x)                       \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(x);                        \
    }
#endif

// float3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_I3_F3(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _16##sign##32 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16); \
    __attribute__((overloadable)) _3##sign##32 func(float3 x)                       \
    {                                                                               \
        float16 valx;                                                               \
        valx.s012 = x;                                                              \
        _16##sign##32 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x07, valx); \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P1_I3_F3(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _3##sign##32 OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(float3); \
    __attribute__((overloadable)) _3##sign##32 func(float3 x)                       \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(x);                        \
    }
#endif

// float4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_I4_F4(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _16##sign##32 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16); \
    __attribute__((overloadable)) _4##sign##32 func(float4 x)                       \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo = x;                                                             \
        _16##sign##32 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x0F, valx); \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P1_I4_F4(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _4##sign##32 OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(float4); \
    __attribute__((overloadable)) _4##sign##32 func(float4 x)                       \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(x);                        \
    }
#endif

// float8
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_I8_F8(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _16##sign##32 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16); \
    __attribute__((overloadable)) _8##sign##32 func(float8 x)                       \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo = x;                                                                \
        _16##sign##32 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0xFF, valx); \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P1_I8_F8(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _8##sign##32 OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(float8); \
    __attribute__((overloadable)) _8##sign##32 func(float8 x)                       \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(x);                        \
    }
#endif

// float16
#define OCL_SVML_P1_I16_F16(func, sign, svmlfunc, native)                           \
    __attribute__((svmlcc)) __attribute__((const)) _16##sign##32 OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(float16); \
    __attribute__((overloadable)) _16##sign##32 func(float16 x)                     \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(x);                       \
    }

// double
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_I1_D1(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _8##sign##32 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8); \
    __attribute__((overloadable)) _1##sign##32 func(double x)                       \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo.lo.lo = x;                                                          \
        _8##sign##32 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x01, valx); \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P1_I1_D1(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _1##sign##32 OCL_SVML_FUNCTION(_##svmlfunc##1,native)(double); \
    __attribute__((overloadable)) _1##sign##32 func(double x)                       \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##1,native)(x);                         \
    }
#endif

// double2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_I2_D2(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _8##sign##32 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8); \
    __attribute__((overloadable)) _2##sign##32 func(double2 x)                      \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo.lo = x;                                                             \
        _8##sign##32 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x03, valx); \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P1_I2_D2(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _2##sign##32 OCL_SVML_FUNCTION(_##svmlfunc##2,native)(double2); \
    __attribute__((overloadable)) _2##sign##32 func(double2 x)                      \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##2,native)(x);                         \
    }
#endif

// double3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_I3_D3(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _8##sign##32 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8); \
    __attribute__((overloadable)) _3##sign##32 func(double3 x)                      \
    {                                                                               \
        double8 valx;                                                               \
        valx.s012 = x;                                                              \
        _8##sign##32 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x07, valx); \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P1_I3_D3(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _3##sign##32 OCL_SVML_FUNCTION(_##svmlfunc##3,native)(double3); \
    __attribute__((overloadable)) _3##sign##32 func(double3 x)                      \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##3,native)(x);                         \
    }
#endif

// double4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P1_I4_D4(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _8##sign##32 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8); \
    __attribute__((overloadable)) _4##sign##32 func(double4 x)                      \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo = x;                                                                \
        _8##sign##32 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x0F, valx); \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P1_I4_D4(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _4##sign##32 OCL_SVML_FUNCTION(_##svmlfunc##4,native)(double4); \
    __attribute__((overloadable)) _4##sign##32 func(double4 x)                      \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##4,native)(x);                         \
    }
#endif

// double8
#define OCL_SVML_P1_I8_D8(func, sign, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) _8##sign##32 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8); \
    __attribute__((overloadable)) _8##sign##32 func(double8 x)                      \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x);                         \
    }

// double16
#define OCL_SVML_P1_I16_D16(func, sign, svmlfunc, native)                           \
    __attribute__((svmlcc)) __attribute__((const)) _8##sign##32 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8); \
    __attribute__((overloadable)) _16##sign##32 func(double16 x)                    \
    {                                                                               \
        _16##sign##32 res;                                                          \
        res.lo = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.lo);                    \
        res.hi = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.hi);                    \
        return res;                                                                 \
    }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float
#define OCL_INTR_P1_I1_F1(func, sign)                                               \
    __attribute__((overloadable)) _16##sign##32 mask_ ## func(ushort, float16);     \
    __attribute__((overloadable)) _1##sign##32 func(float x)                        \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo.lo.lo = x;                                                       \
        _16##sign##32 res = mask_ ## func(0x01, valx);                              \
        return res.lo.lo.lo.lo;                                                     \
    }

// float2
#define OCL_INTR_P1_I2_F2(func, sign)                                               \
    __attribute__((overloadable)) _16##sign##32 mask_ ## func(ushort, float16);     \
    __attribute__((overloadable)) _2##sign##32 func(float2 x)                       \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo.lo = x;                                                          \
        _16##sign##32 res = mask_ ## func(0x03, valx);                              \
        return res.lo.lo.lo;                                                        \
    }

// float3
#define OCL_INTR_P1_I3_F3(func, sign)                                               \
    __attribute__((overloadable)) _16##sign##32 mask_ ## func(ushort, float16);     \
    __attribute__((overloadable)) _3##sign##32 func(float3 x)                       \
    {                                                                               \
        float16 valx;                                                               \
        valx.s012 = x;                                                              \
        _16##sign##32 res = mask_ ## func(0x07, valx);                              \
        return res.s012;                                                            \
    }

// float4
#define OCL_INTR_P1_I4_F4(func, sign)                                               \
    __attribute__((overloadable)) _16##sign##32 mask_ ## func(ushort, float16);     \
    __attribute__((overloadable)) _4##sign##32 func(float4 x)                       \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo = x;                                                             \
        _16##sign##32 res = mask_ ## func(0x0F, valx);                              \
        return res.lo.lo;                                                           \
    }

// float8
#define OCL_INTR_P1_I8_F8(func, sign)                                               \
    __attribute__((overloadable)) _16##sign##32 mask_ ## func(ushort, float16);     \
    __attribute__((overloadable)) _8##sign##32 func(float8 x)                       \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo = x;                                                                \
        _16##sign##32 res = mask_ ## func(0xFF, valx);                              \
        return res.lo;                                                              \
    }

// float16
#define OCL_INTR_P1_I16_F16(func, sign)                                             \
    __attribute__((overloadable)) _16##sign##32 func(float16);

// double
#define OCL_INTR_P1_I1_D1(func, sign)                                               \
    __attribute__((overloadable)) _8##sign##32 mask_ ## func(uchar, double8);       \
    __attribute__((overloadable)) _1##sign##32 func(double x)                       \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo.lo.lo = x;                                                          \
        _8##sign##32 res = mask_ ## func(0x01, valx);                               \
        return res.lo.lo.lo;                                                        \
    }

// double2
#define OCL_INTR_P1_I2_D2(func, sign)                                               \
    __attribute__((overloadable)) _8##sign##32 mask_ ## func(uchar, double8);       \
    __attribute__((overloadable)) _2##sign##32 func(double2 x)                      \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo.lo = x;                                                             \
        _8##sign##32 res = mask_ ## func(0x03, valx);                               \
        return res.lo.lo;                                                           \
    }

// double3
#define OCL_INTR_P1_I3_D3(func, sign)                                               \
    __attribute__((overloadable)) _8##sign##32 mask_ ## func(uchar, double8);       \
    __attribute__((overloadable)) _3##sign##32 func(double3 x)                      \
    {                                                                               \
        double8 valx;                                                               \
        valx.s012 = x;                                                              \
        _8##sign##32 res = mask_ ## func(0x07, valx);                               \
        return res.s012;                                                            \
    }

// double4
#define OCL_INTR_P1_I4_D4(func, sign)                                               \
    __attribute__((overloadable)) _8##sign##32 mask_ ## func(uchar, double8);       \
    __attribute__((overloadable)) _4##sign##32 func(double4 x)                      \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo = x;                                                                \
        _8##sign##32 res = mask_ ## func(0x0F, valx);                               \
        return res.lo;                                                              \
    }

// double8
#define OCL_INTR_P1_I8_D8(func, sign)                                               \
    __attribute__((overloadable)) _8##sign##32 func(double8);

// double16
#define OCL_INTR_P1_I16_D16(func, sign)                                             \
    __attribute__((overloadable)) _8##sign##32 func(double8);                       \
    __attribute__((overloadable)) _16##sign##32 func(double16 x)                    \
    {                                                                               \
        _16##sign##32 res;                                                          \
        res.lo = func(x.lo);                                                        \
        res.hi = func(x.hi);                                                        \
        return res;                                                                 \
    }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F1_F1_F1(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, float16); \
    __attribute__((overloadable)) float func(float x, float y)                      \
    {                                                                               \
        float16 valx, valy;                                                         \
        valx.lo.lo.lo.lo = x;                                                       \
        valy.lo.lo.lo.lo = y;                                                       \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x01, valx, valy); \
        return res.lo.lo.lo.lo;                                                     \
    }
#else
#define OCL_SVML_P2_F1_F1_F1(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) float OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(float, float); \
    __attribute__((overloadable)) float func(float x, float y)                      \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(x, y);                     \
    }
#endif

// float2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F2_F2_F2(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, float16); \
    __attribute__((overloadable)) float2 func(float2 x, float2 y)                   \
    {                                                                               \
        float16 valx, valy;                                                         \
        valx.lo.lo.lo = x;                                                          \
        valy.lo.lo.lo = y;                                                          \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x03, valx, valy); \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P2_F2_F2_F2(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) float2 OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(float2, float2); \
    __attribute__((overloadable)) float2 func(float2 x, float2 y)                   \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(x, y);                     \
    }
#endif

// float3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F3_F3_F3(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, float16); \
    __attribute__((overloadable)) float3 func(float3 x, float3 y)                   \
    {                                                                               \
        float16 valx, valy;                                                         \
        valx.s012 = x;                                                              \
        valy.s012 = y;                                                              \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x07, valx, valy); \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P2_F3_F3_F3(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) float3 OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(float3, float3); \
    __attribute__((overloadable)) float3 func(float3 x, float3 y)                   \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(x, y);                     \
    }
#endif

// float4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F4_F4_F4(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, float16); \
    __attribute__((overloadable)) float4 func(float4 x, float4 y)                   \
    {                                                                               \
        float16 valx, valy;                                                         \
        valx.lo.lo = x;                                                             \
        valy.lo.lo = y;                                                             \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x0F, valx, valy); \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P2_F4_F4_F4(func, svmlfunc, native)                                        \
    __attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(float4, float4); \
    __attribute__((overloadable)) float4 func(float4 x, float4 y)                   \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(x, y);                     \
    }
#endif

// float8
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F8_F8_F8(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, float16); \
    __attribute__((overloadable)) float8 func(float8 x, float8 y)                   \
    {                                                                               \
        float16 valx, valy;                                                         \
        valx.lo = x;                                                                \
        valy.lo = y;                                                                \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0xFF, valx, valy); \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P2_F8_F8_F8(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) float8 OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(float8, float8); \
    __attribute__((overloadable)) float8 func(float8 x, float8 y)                   \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(x, y);                     \
    }
#endif

// float16
#define OCL_SVML_P2_F16_F16_F16(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(float16, float16); \
    __attribute__((overloadable)) float16 func(float16 x, float16 y)                \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(x, y);                    \
    }

// double
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D1_D1_D1(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, double8); \
    __attribute__((overloadable)) double func(double x, double y)                   \
    {                                                                               \
        double8 valx, valy;                                                         \
        valx.lo.lo.lo = x;                                                          \
        valy.lo.lo.lo = y;                                                          \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x01, valx, valy); \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P2_D1_D1_D1(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) double OCL_SVML_FUNCTION(_##svmlfunc##1,native)(double, double); \
    __attribute__((overloadable)) double func(double x, double y)                   \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##1,native)(x, y);                      \
    }
#endif

// double2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D2_D2_D2(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, double8); \
    __attribute__((overloadable)) double2 func(double2 x, double2 y)                \
    {                                                                               \
        double8 valx, valy;                                                         \
        valx.lo.lo = x;                                                             \
        valy.lo.lo = y;                                                             \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x03, valx, valy); \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P2_D2_D2_D2(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) double2 OCL_SVML_FUNCTION(_##svmlfunc##2,native)(double2, double2); \
    __attribute__((overloadable)) double2 func(double2 x, double2 y)                \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##2,native)(x, y);                      \
    }
#endif

// double3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D3_D3_D3(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, double8); \
    __attribute__((overloadable)) double3 func(double3 x, double3 y)                \
    {                                                                               \
        double8 valx, valy;                                                         \
        valx.s012 = x;                                                              \
        valy.s012 = y;                                                              \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x07, valx, valy); \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P2_D3_D3_D3(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) double3 OCL_SVML_FUNCTION(_##svmlfunc##3,native)(double3, double3); \
    __attribute__((overloadable)) double3 func(double3 x, double3 y)                \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##3,native)(x, y);                      \
    }
#endif

// double4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D4_D4_D4(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, double8); \
    __attribute__((overloadable)) double4 func(double4 x, double4 y)                \
    {                                                                               \
        double8 valx, valy;                                                         \
        valx.lo = x;                                                                \
        valy.lo = y;                                                                \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x0F, valx, valy); \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P2_D4_D4_D4(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4,native)(double4, double4); \
    __attribute__((overloadable)) double4 func(double4 x, double4 y)                \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##4,native)(x, y);                      \
    }
#endif

// double8
#define OCL_SVML_P2_D8_D8_D8(func, svmlfunc, native)                                \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8, double8); \
    __attribute__((overloadable)) double8 func(double8 x, double8 y)                \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x, y);                      \
    }

// double16
#define OCL_SVML_P2_D16_D16_D16(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8, double8); \
    __attribute__((overloadable)) double16 func(double16 x, double16 y)             \
    {                                                                               \
        double16 res;                                                               \
        res.lo = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.lo, y.lo);              \
        res.hi = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.hi, y.hi);              \
        return res;                                                                 \
    }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float
#define OCL_INTR_P2_F1_F1_F1(func)                                                  \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16, float16);  \
    __attribute__((overloadable)) float func(float x, float y)                      \
    {                                                                               \
        float16 valx, valy;                                                         \
        valx.lo.lo.lo.lo = x;                                                       \
        valy.lo.lo.lo.lo = y;                                                       \
        float16 res = mask_ ## func(0x01, valx, valy);                              \
        return res.lo.lo.lo.lo;                                                     \
    }

// float2
#define OCL_INTR_P2_F2_F2_F2(func)                                                  \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16, float16);  \
    __attribute__((overloadable)) float2 func(float2 x, float2 y)                   \
    {                                                                               \
        float16 valx, valy;                                                         \
        valx.lo.lo.lo = x;                                                          \
        valy.lo.lo.lo = y;                                                          \
        float16 res = mask_ ## func(0x03, valx, valy);                              \
        return res.lo.lo.lo;                                                        \
    }

// float3
#define OCL_INTR_P2_F3_F3_F3(func)                                                  \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16, float16);  \
    __attribute__((overloadable)) float3 func(float3 x, float3 y)                   \
    {                                                                               \
        float16 valx, valy;                                                         \
        valx.s012 = x;                                                              \
        valy.s012 = y;                                                              \
        float16 res = mask_ ## func(0x07, valx, valy);                              \
        return res.s012;                                                            \
    }

// float4
#define OCL_INTR_P2_F4_F4_F4(func)                                                  \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16, float16);  \
    __attribute__((overloadable)) float4 func(float4 x, float4 y)                   \
    {                                                                               \
        float16 valx, valy;                                                         \
        valx.lo.lo = x;                                                             \
        valy.lo.lo = y;                                                             \
        float16 res = mask_ ## func(0x0F, valx, valy);                              \
        return res.lo.lo;                                                           \
    }

// float8
#define OCL_INTR_P2_F8_F8_F8(func)                                                  \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16, float16);  \
    __attribute__((overloadable)) float8 func(float8 x, float8 y)                   \
    {                                                                               \
        float16 valx, valy;                                                         \
        valx.lo = x;                                                                \
        valy.lo = y;                                                                \
        float16 res = mask_ ## func(0xFF, valx, valy);                              \
        return res.lo;                                                              \
    }

// float16
#define OCL_INTR_P2_F16_F16_F16(func)                                               \
    __attribute__((overloadable)) float16 func(float16, float16);


// double
#define OCL_INTR_P2_D1_D1_D1(func)                                                  \
    __attribute__((overloadable)) double8 mask_ ## func(uchar, double8, double8);   \
    __attribute__((overloadable)) double func(double x, double y)                   \
    {                                                                               \
        double8 valx, valy;                                                         \
        valx.lo.lo.lo = x;                                                          \
        valy.lo.lo.lo = y;                                                          \
        double8 res = mask_ ## func(0x01, valx, valy);                              \
        return res.lo.lo.lo;                                                        \
    }

// double2
#define OCL_INTR_P2_D2_D2_D2(func)                                                  \
    __attribute__((overloadable)) double8 mask_ ## func(uchar, double8, double8);   \
    __attribute__((overloadable)) double2 func(double2 x, double2 y)                \
    {                                                                               \
        double8 valx, valy;                                                         \
        valx.lo.lo = x;                                                             \
        valy.lo.lo = y;                                                             \
        double8 res = mask_ ## func(0x03, valx, valy);                              \
        return res.lo.lo;                                                           \
    }

// double3
#define OCL_INTR_P2_D3_D3_D3(func)                                                  \
    __attribute__((overloadable)) double8 mask_ ## func(uchar, double8, double8);   \
    __attribute__((overloadable)) double3 func(double3 x, double3 y)                \
    {                                                                               \
        double8 valx, valy;                                                         \
        valx.s012 = x;                                                              \
        valy.s012 = y;                                                              \
        double8 res = mask_ ## func(0x07, valx, valy);                              \
        return res.s012;                                                            \
    }

// double4
#define OCL_INTR_P2_D4_D4_D4(func)                                                  \
    __attribute__((overloadable)) double8 mask_ ## func(uchar, double8, double8);   \
    __attribute__((overloadable)) double4 func(double4 x, double4 y)                \
    {                                                                               \
        double8 valx, valy;                                                         \
        valx.lo = x;                                                                \
        valy.lo = y;                                                                \
        double8 res = mask_ ## func(0x0F, valx, valy);                              \
        return res.lo;                                                              \
    }

// double8
#define OCL_INTR_P2_D8_D8_D8(func)                                                  \
    __attribute__((overloadable)) double8 func(double8, double8);

// double16
#define OCL_INTR_P2_D16_D16_D16(func)                                               \
    __attribute__((overloadable)) double8 func(double8, double8);                   \
    __attribute__((overloadable)) double16 func(double16 x, double16 y)             \
    {                                                                               \
        double16 res;                                                               \
        res.lo = func(x.lo, y.lo);                                                  \
        res.hi = func(x.hi, y.hi);                                                  \
        return res;                                                                 \
    }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F1_F1_I1(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, _16##sign##32); \
    __attribute__((overloadable)) float func(float x, _1##sign##32 y)               \
    {                                                                               \
        float16 valx;                                                               \
        _16##sign##32 valy;                                                         \
        valx.lo.lo.lo.lo = x;                                                       \
        valy.lo.lo.lo.lo = y;                                                       \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x01, valx, valy); \
        return res.lo.lo.lo.lo;                                                     \
    }
#else
#define OCL_SVML_P2_F1_F1_I1(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) float OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(float, _1##sign##32); \
    __attribute__((overloadable)) float func(float x, _1##sign##32 y)               \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(x, y);                     \
    }
#endif

// float2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F2_F2_I2(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, _16##sign##32); \
    __attribute__((overloadable)) float2 func(float2 x, _2##sign##32 y)             \
    {                                                                               \
        float16 valx;                                                               \
        _16##sign##32 valy;                                                         \
        valx.lo.lo.lo = x;                                                          \
        valy.lo.lo.lo = y;                                                          \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x03, valx, valy); \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P2_F2_F2_I2(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) float2 OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(float2, _2##sign##32); \
    __attribute__((overloadable)) float2 func(float2 x, _2##sign##32 y)             \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(x, y);                     \
    }
#endif

// float3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F3_F3_I3(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, _16##sign##32); \
    __attribute__((overloadable)) float3 func(float3 x, _3##sign##32 y)             \
    {                                                                               \
        float16 valx;                                                               \
        _16##sign##32 valy;                                                         \
        valx.s012 = x;                                                              \
        valy.s012 = y;                                                              \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x07, valx, valy); \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P2_F3_F3_I3(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) float3 OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(float3, _3##sign##32); \
    __attribute__((overloadable)) float3 func(float3 x, _3##sign##32 y)             \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(x, y);                     \
    }
#endif

// float4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F4_F4_I4(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, _16##sign##32); \
    __attribute__((overloadable)) float4 func(float4 x, _4##sign##32 y)             \
    {                                                                               \
        float16 valx;                                                               \
        _16##sign##32 valy;                                                         \
        valx.lo.lo = x;                                                             \
        valy.lo.lo = y;                                                             \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x0F, valx, valy); \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P2_F4_F4_I4(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(float4, _4##sign##32); \
    __attribute__((overloadable)) float4 func(float4 x, _4##sign##32 y)             \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(x, y);                     \
    }
#endif

// float8
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F8_F8_I8(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, _16##sign##32); \
    __attribute__((overloadable)) float8 func(float8 x, _8##sign##32 y)             \
    {                                                                               \
        float16 valx;                                                               \
        _16##sign##32 valy;                                                         \
        valx.lo = x;                                                                \
        valy.lo = y;                                                                \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0xFF, valx, valy); \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P2_F8_F8_I8(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) float8 OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(float8, _8##sign##32); \
    __attribute__((overloadable)) float8 func(float8 x, _8##sign##32 y)             \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(x, y);                     \
    }
#endif

// float16
#define OCL_SVML_P2_F16_F16_I16(func, sign, svmlfunc, native)                       \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(float16, _16##sign##32); \
    __attribute__((overloadable)) float16 func(float16 x, _16##sign##32 y)          \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(x, y);                    \
    }

// double
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D1_D1_I1(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, _8##sign##32); \
    __attribute__((overloadable)) double func(double x, _1##sign##32 y)             \
    {                                                                               \
        double8 valx;                                                               \
        _8##sign##32 valy;                                                          \
        valx.lo.lo.lo = x;                                                          \
        valy.lo.lo.lo = y;                                                          \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x01, valx, valy); \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P2_D1_D1_I1(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) double OCL_SVML_FUNCTION(_##svmlfunc##1,native)(double, _1##sign##32); \
    __attribute__((overloadable)) double func(double x, _1##sign##32 y)             \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##1,native)(x, y);                      \
    }
#endif

// double2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D2_D2_I2(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, _8##sign##32); \
    __attribute__((overloadable)) double2 func(double2 x, _2##sign##32 y)           \
    {                                                                               \
        double8 valx;                                                               \
        _8##sign##32 valy;                                                          \
        valx.lo.lo = x;                                                             \
        valy.lo.lo = y;                                                             \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x03, valx, valy); \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P2_D2_D2_I2(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) double2 OCL_SVML_FUNCTION(_##svmlfunc##2,native)(double2, _2##sign##32); \
    __attribute__((overloadable)) double2 func(double2 x, _2##sign##32 y)           \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##2,native)(x, y);                      \
    }
#endif

// double3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D3_D3_I3(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, _8##sign##32); \
    __attribute__((overloadable)) double3 func(double3 x, _3##sign##32 y)           \
    {                                                                               \
        double8 valx;                                                               \
        _8##sign##32 valy;                                                          \
        valx.s012 = x;                                                              \
        valy.s012 = y;                                                              \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x07, valx, valy); \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P2_D3_D3_I3(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) double3 OCL_SVML_FUNCTION(_##svmlfunc##3,native)(double3, _3##sign##32); \
    __attribute__((overloadable)) double3 func(double3 x, _3##sign##32 y)           \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##3,native)(x, y);                      \
    }
#endif

// double4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D4_D4_I4(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, _8##sign##32); \
    __attribute__((overloadable)) double4 func(double4 x, _4##sign##32 y)           \
    {                                                                               \
        double8 valx;                                                               \
        _8##sign##32 valy;                                                          \
        valx.lo = x;                                                                \
        valy.lo = y;                                                                \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x0F, valx, valy); \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P2_D4_D4_I4(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4,native)(double4, _4##sign##32); \
    __attribute__((overloadable)) double4 func(double4 x, _4##sign##32 y)           \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##4,native)(x, y);                      \
    }
#endif

// double8
#define OCL_SVML_P2_D8_D8_I8(func, sign, svmlfunc, native)                          \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8, _8##sign##32); \
    __attribute__((overloadable)) double8 func(double8 x, _8##sign##32 y)           \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x, y);                      \
    }

// double16
#define OCL_SVML_P2_D16_D16_I16(func, sign, svmlfunc, native)                       \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8, _8##sign##32); \
    __attribute__((overloadable)) double16 func(double16 x, _16##sign##32 y)        \
    {                                                                               \
        double16 res;                                                               \
        res.lo = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.lo, y.lo);              \
        res.hi = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.hi, y.hi);              \
        return res;                                                                 \
    }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F1_F1_pI1(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, void*); \
    __attribute__((overloadable)) float func(float x, _1##sign##32* y)              \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo.lo.lo = x;                                                       \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x01, valx, y); \
        return res.lo.lo.lo.lo;                                                     \
    }
#else
#define OCL_SVML_P2_F1_F1_pI1(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) float OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(float, _1##sign##32*); \
    __attribute__((overloadable)) float func(float x, _1##sign##32* y)              \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(x, y);                     \
    }
#endif

#define OCL_SVML_P2_F1_F1_pI1_LOCAL(func, sign)                                     \
    __attribute__((overloadable)) float func(float x, __local _1##sign##32* y)      \
    {                                                                               \
        return func(x, (_1##sign##32*)y);                                           \
    }

#define OCL_SVML_P2_F1_F1_pI1_GLOBAL(func, sign)                                    \
    __attribute__((overloadable)) float func(float x, __global _1##sign##32* y)     \
    {                                                                               \
        return func(x, (_1##sign##32*)y);                                           \
    }

// float2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F2_F2_pI2(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, void*); \
    __attribute__((overloadable)) float2 func(float2 x, _2##sign##32* y)            \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo.lo = x;                                                          \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x03, valx, y); \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P2_F2_F2_pI2(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) float2 OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(float2, _2##sign##32*); \
    __attribute__((overloadable)) float2 func(float2 x, _2##sign##32* y)            \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(x, y);                     \
    }
#endif

#define OCL_SVML_P2_F2_F2_pI2_LOCAL(func, sign)                                     \
    __attribute__((overloadable)) float2 func(float2 x, __local _2##sign##32* y)    \
    {                                                                               \
        return func(x, (_2##sign##32*)y);                                           \
    }

#define OCL_SVML_P2_F2_F2_pI2_GLOBAL(func, sign)                                    \
    __attribute__((overloadable)) float2 func(float2 x, __global _2##sign##32* y)   \
    {                                                                               \
        return func(x, (_2##sign##32*)y);                                           \
    }

// float3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F3_F3_pI3(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, void*); \
    __attribute__((overloadable)) float3 func(float3 x, _3##sign##32* y)            \
    {                                                                               \
        float16 valx;                                                               \
        valx.s012 = x;                                                              \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x07, valx, y); \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P2_F3_F3_pI3(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) float3 OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(float3, _3##sign##32*); \
    __attribute__((overloadable)) float3 func(float3 x, _3##sign##32* y)            \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(x, y);                     \
    }
#endif

#define OCL_SVML_P2_F3_F3_pI3_LOCAL(func, sign)                                     \
    __attribute__((overloadable)) float3 func(float3 x, __local _3##sign##32* y)    \
    {                                                                               \
        return func(x, (_3##sign##32*)y);                                           \
    }

#define OCL_SVML_P2_F3_F3_pI3_GLOBAL(func, sign)                                    \
    __attribute__((overloadable)) float3 func(float3 x, __global _3##sign##32* y)   \
    {                                                                               \
        return func(x, (_3##sign##32*)y);                                           \
    }

// float4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F4_F4_pI4(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, void*); \
    __attribute__((overloadable)) float4 func(float4 x, _4##sign##32* y)            \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo = x;                                                             \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x0F, valx, y); \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P2_F4_F4_pI4(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(float4, _4##sign##32*); \
    __attribute__((overloadable)) float4 func(float4 x, _4##sign##32* y)            \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(x, y);                     \
    }
#endif

#define OCL_SVML_P2_F4_F4_pI4_LOCAL(func, sign)                                     \
    __attribute__((overloadable)) float4 func(float4 x, __local _4##sign##32* y)    \
    {                                                                               \
        return func(x, (_4##sign##32*)y);                                           \
    }

#define OCL_SVML_P2_F4_F4_pI4_GLOBAL(func, sign)                            \
    __attribute__((overloadable)) float4 func(float4 x, __global _4##sign##32* y)   \
    {                                                                               \
        return func(x, (_4##sign##32*)y);                                           \
    }

// float8
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F8_F8_pI8(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, void*); \
    __attribute__((overloadable)) float8 func(float8 x, _8##sign##32* y)            \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo = x;                                                                \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0xFF, valx, y); \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P2_F8_F8_pI8(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) float8 OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(float8, _8##sign##32*); \
    __attribute__((overloadable)) float8 func(float8 x, _8##sign##32* y)            \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(x, y);                     \
    }
#endif

#define OCL_SVML_P2_F8_F8_pI8_LOCAL(func, sign)                                     \
    __attribute__((overloadable)) float8 func(float8 x, __local _8##sign##32* y)    \
    {                                                                               \
        return func(x, (_8##sign##32*)y);                                           \
    }

#define OCL_SVML_P2_F8_F8_pI8_GLOBAL(func, sign)                                    \
    __attribute__((overloadable)) float8 func(float8 x, __global _8##sign##32* y)   \
    {                                                                               \
        return func(x, (_8##sign##32*)y);                                           \
    }

// float16
#define OCL_SVML_P2_F16_F16_pI16(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) float16 OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(float16, _16##sign##32*); \
    __attribute__((overloadable)) float16 func(float16 x, _16##sign##32* y)         \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(x, y);                    \
    }

#define OCL_SVML_P2_F16_F16_pI16_LOCAL(func, sign)                                  \
    __attribute__((overloadable)) float16 func(float16 x, __local _16##sign##32* y) \
    {                                                                               \
        return func(x, (_16##sign##32*)y);                                          \
    }

#define OCL_SVML_P2_F16_F16_pI16_GLOBAL(func, sign)                                 \
    __attribute__((overloadable)) float16 func(float16 x, __global _16##sign##32* y) \
    {                                                                               \
        return func(x, (_16##sign##32*)y);                                          \
    }

// double
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D1_D1_pI1(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, void*); \
    __attribute__((overloadable)) double func(double x, _1##sign##32* y)            \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo.lo.lo = x;                                                          \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x01, valx, y); \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P2_D1_D1_pI1(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) double OCL_SVML_FUNCTION(_##svmlfunc##1,native)(double, _1##sign##32*); \
    __attribute__((overloadable)) double func(double x, _1##sign##32* y)            \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##1,native)(x, y);                      \
    }
#endif

#define OCL_SVML_P2_D1_D1_pI1_LOCAL(func, sign)                                     \
    __attribute__((overloadable)) double func(double x, __local _1##sign##32* y)    \
    {                                                                               \
        return func(x, (_1##sign##32*)y);                                           \
    }

#define OCL_SVML_P2_D1_D1_pI1_GLOBAL(func, sign)                                    \
    __attribute__((overloadable)) double func(double x, __global _1##sign##32* y)   \
    {                                                                               \
        return func(x, (_1##sign##32*)y);                                           \
    }

// double2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D2_D2_pI2(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, void*); \
    __attribute__((overloadable)) double2 func(double2 x, _2##sign##32* y)          \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo.lo = x;                                                             \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x03, valx, y); \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P2_D2_D2_pI2(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) double2 OCL_SVML_FUNCTION(_##svmlfunc##2,native)(double2, _2##sign##32*); \
    __attribute__((overloadable)) double2 func(double2 x, _2##sign##32* y)          \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##2,native)(x, y);                      \
    }
#endif

#define OCL_SVML_P2_D2_D2_pI2_LOCAL(func, sign)                                     \
    __attribute__((overloadable)) double2 func(double2 x, __local _2##sign##32* y)  \
    {                                                                               \
        return func(x, (_2##sign##32*)y);                                           \
    }

#define OCL_SVML_P2_D2_D2_pI2_GLOBAL(func, sign)                                    \
    __attribute__((overloadable)) double2 func(double2 x, __global _2##sign##32* y) \
    {                                                                               \
        return func(x, (_2##sign##32*)y);                                           \
    }

// double3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D3_D3_pI3(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, void*); \
    __attribute__((overloadable)) double3 func(double3 x, _3##sign##32* y)          \
    {                                                                               \
        double8 valx;                                                               \
        valx.s012 = x;                                                              \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x07, valx, y); \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P2_D3_D3_pI3(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) double3 OCL_SVML_FUNCTION(_##svmlfunc##3,native)(double3, _3##sign##32*); \
    __attribute__((overloadable)) double3 func(double3 x, _3##sign##32* y)          \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##3,native)(x, y);                      \
    }
#endif

#define OCL_SVML_P2_D3_D3_pI3_LOCAL(func, sign)                                     \
    __attribute__((overloadable)) double3 func(double3 x, __local _3##sign##32* y)  \
    {                                                                               \
        return func(x, (_3##sign##32*)y);                                           \
    }

#define OCL_SVML_P2_D3_D3_pI3_GLOBAL(func, sign)                                    \
    __attribute__((overloadable)) double3 func(double3 x, __global _3##sign##32* y) \
    {                                                                               \
        return func(x, (_3##sign##32*)y);                                           \
    }

// double4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D4_D4_pI4(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, void*); \
    __attribute__((overloadable)) double4 func(double4 x, _4##sign##32* y)          \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo = x;                                                                \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x0F, valx, y); \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P2_D4_D4_pI4(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4,native)(double4, _4##sign##32*); \
    __attribute__((overloadable)) double4 func(double4 x, _4##sign##32* y)          \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##4,native)(x, y);                      \
    }
#endif

#define OCL_SVML_P2_D4_D4_pI4_LOCAL(func, sign)                                     \
    __attribute__((overloadable)) double4 func(double4 x, __local _4##sign##32* y)  \
    {                                                                               \
        return func(x, (_4##sign##32*)y);                                           \
    }

#define OCL_SVML_P2_D4_D4_pI4_GLOBAL(func, sign)                                    \
    __attribute__((overloadable)) double4 func(double4 x, __global _4##sign##32* y) \
    {                                                                               \
        return func(x, (_4##sign##32*)y);                                           \
    }

// double8
#define OCL_SVML_P2_D8_D8_pI8(func, sign, svmlfunc, native)                         \
    __attribute__((svmlcc)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8, _8##sign##32*); \
    __attribute__((overloadable)) double8 func(double8 x, _8##sign##32* y)          \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x, y);                      \
    }

#define OCL_SVML_P2_D8_D8_pI8_LOCAL(func, sign)                                     \
    __attribute__((overloadable)) double8 func(double8 x, __local _8##sign##32* y)  \
    {                                                                               \
        return func(x, (_8##sign##32*)y);                                           \
    }

#define OCL_SVML_P2_D8_D8_pI8_GLOBAL(func, sign)                                    \
    __attribute__((overloadable)) double8 func(double8 x, __global _8##sign##32* y) \
    {                                                                               \
        return func(x, (_8##sign##32*)y);                                           \
    }

// double16
#define OCL_SVML_P2_D16_D16_pI16(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8, _8##sign##32*); \
    __attribute__((overloadable)) double16 func(double16 x, _16##sign##32* y)       \
    {                                                                               \
        double16 res;                                                               \
        _8##sign##32 lo, hi;                                                        \
        res.lo = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.lo, &lo);               \
        res.hi = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.hi, &hi);               \
        y->lo = lo;                                                                 \
        y->hi = hi;                                                                 \
        return res;                                                                 \
    }

#define OCL_SVML_P2_D16_D16_pI16_LOCAL(func, sign)                                  \
    __attribute__((overloadable)) double16 func(double16 x, __local _16##sign##32* y) \
    {                                                                               \
        return func(x, (_16##sign##32*)y);                                          \
    }

#define OCL_SVML_P2_D16_D16_pI16_GLOBAL(func, sign)                                 \
    __attribute__((overloadable)) double16 func(double16 x, __global _16##sign##32* y) \
    {                                                                               \
        return func(x, (_16##sign##32*)y);                                          \
    }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F1_F1_pF1(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, void*); \
    __attribute__((overloadable)) float func(float x, float* y)                     \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo.lo.lo = x;                                                       \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x01, valx, y); \
        return res.lo.lo.lo.lo;                                                     \
    }
#else
#define OCL_SVML_P2_F1_F1_pF1(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) float OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(float, float*); \
    __attribute__((overloadable)) float func(float x, float* y)                     \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(x, y);                     \
    }
#endif

#define OCL_SVML_P2_F1_F1_pF1_LOCAL(func)                                           \
    __attribute__((overloadable)) float func(float x, __local float* y)             \
    {                                                                               \
        return func(x, (float*)y);                                                  \
    }

#define OCL_SVML_P2_F1_F1_pF1_GLOBAL(func)                                          \
    __attribute__((overloadable)) float func(float x, __global float* y)            \
    {                                                                               \
        return func(x, (float*)y);                                                  \
    }

// float2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F2_F2_pF2(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, void*); \
    __attribute__((overloadable)) float2 func(float2 x, float2* y)                  \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo.lo = x;                                                          \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x03, valx, y); \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P2_F2_F2_pF2(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) float2 OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(float2, float2*); \
    __attribute__((overloadable)) float2 func(float2 x, float2* y)                  \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(x, y);                     \
    }
#endif

#define OCL_SVML_P2_F2_F2_pF2_LOCAL(func)                                           \
    __attribute__((overloadable)) float2 func(float2 x, __local float2* y)          \
    {                                                                               \
        return func(x, (float2*)y);                                                 \
    }

#define OCL_SVML_P2_F2_F2_pF2_GLOBAL(func)                                          \
    __attribute__((overloadable)) float2 func(float2 x, __global float2* y)         \
    {                                                                               \
        return func(x, (float2*)y);                                                 \
    }

// float3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F3_F3_pF3(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, void*); \
    __attribute__((overloadable)) float3 func(float3 x, float3* y)                  \
    {                                                                               \
        float16 valx;                                                               \
        valx.s012 = x;                                                              \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x07, valx, y); \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P2_F3_F3_pF3(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) float3 OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(float3, float3*); \
    __attribute__((overloadable)) float3 func(float3 x, float3* y)                  \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(x, y);                     \
    }
#endif

#define OCL_SVML_P2_F3_F3_pF3_LOCAL(func)                                           \
    __attribute__((overloadable)) float3 func(float3 x, __local float3* y)          \
    {                                                                               \
        return func(x, (float3*)y);                                                 \
    }

#define OCL_SVML_P2_F3_F3_pF3_GLOBAL(func)                                          \
    __attribute__((overloadable)) float3 func(float3 x, __global float3* y)         \
    {                                                                               \
        return func(x, (float3*)y);                                                 \
    }

// float4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F4_F4_pF4(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, void*); \
    __attribute__((overloadable)) float4 func(float4 x, float4* y)                  \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo = x;                                                             \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x0F, valx, y); \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P2_F4_F4_pF4(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(float4, float4*); \
    __attribute__((overloadable)) float4 func(float4 x, float4* y)                  \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(x, y);                     \
    }
#endif

#define OCL_SVML_P2_F4_F4_pF4_LOCAL(func)                                           \
    __attribute__((overloadable)) float4 func(float4 x, __local float4* y)          \
    {                                                                               \
        return func(x, (float4*)y);                                                 \
    }

#define OCL_SVML_P2_F4_F4_pF4_GLOBAL(func)                                          \
    __attribute__((overloadable)) float4 func(float4 x, __global float4* y)         \
    {                                                                               \
        return func(x, (float4*)y);                                                 \
    }

// float8
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_F8_F8_pF8(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, void*); \
    __attribute__((overloadable)) float8 func(float8 x, float8* y)                  \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo = x;                                                                \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0xFF, valx, y); \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P2_F8_F8_pF8(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) float8 OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(float8, float8*); \
    __attribute__((overloadable)) float8 func(float8 x, float8* y)                  \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(x, y);                     \
    }
#endif

#define OCL_SVML_P2_F8_F8_pF8_LOCAL(func)                                           \
    __attribute__((overloadable)) float8 func(float8 x, __local float8* y)          \
    {                                                                               \
        return func(x, (float8*)y);                                                 \
    }

#define OCL_SVML_P2_F8_F8_pF8_GLOBAL(func)                                          \
    __attribute__((overloadable)) float8 func(float8 x, __global float8* y)         \
    {                                                                               \
        return func(x, (float8*)y);                                                 \
    }

// float16
#define OCL_SVML_P2_F16_F16_pF16(func, svmlfunc, native)                            \
    __attribute__((svmlcc)) float16 OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(float16, float16*); \
    __attribute__((overloadable)) float16 func(float16 x, float16* y)               \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(x, y);                    \
    }

#define OCL_SVML_P2_F16_F16_pF16_LOCAL(func)                                        \
    __attribute__((overloadable)) float16 func(float16 x, __local float16* y)       \
    {                                                                               \
        return func(x, (float16*)y);                                                \
    }

#define OCL_SVML_P2_F16_F16_pF16_GLOBAL(func)                                       \
    __attribute__((overloadable)) float16 func(float16 x, __global float16* y)      \
    {                                                                               \
        return func(x, (float16*)y);                                                \
    }

// double
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D1_D1_pD1(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, void*); \
    __attribute__((overloadable)) double func(double x, double* y)                  \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo.lo.lo = x;                                                          \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x01, valx, y); \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P2_D1_D1_pD1(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) double OCL_SVML_FUNCTION(_##svmlfunc##1,native)(double, double*); \
    __attribute__((overloadable)) double func(double x, double* y)                  \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##1,native)(x, y);                      \
    }
#endif

#define OCL_SVML_P2_D1_D1_pD1_LOCAL(func)                                           \
    __attribute__((overloadable)) double func(double x, __local double* y)          \
    {                                                                               \
        return func(x, (double*)y);                                                 \
    }

#define OCL_SVML_P2_D1_D1_pD1_GLOBAL(func)                                          \
    __attribute__((overloadable)) double func(double x, __global double* y)         \
    {                                                                               \
        return func(x, (double*)y);                                                 \
    }

// double2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D2_D2_pD2(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, void*); \
    __attribute__((overloadable)) double2 func(double2 x, double2* y)               \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo.lo = x;                                                             \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x03, valx, y); \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P2_D2_D2_pD2(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) double2 OCL_SVML_FUNCTION(_##svmlfunc##2,native)(double2, double2*); \
    __attribute__((overloadable)) double2 func(double2 x, double2* y)               \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##2,native)(x, y);                      \
    }
#endif

#define OCL_SVML_P2_D2_D2_pD2_LOCAL(func)                                           \
    __attribute__((overloadable)) double2 func(double2 x, __local double2* y)       \
    {                                                                               \
        return func(x, (double2*)y);                                                \
    }

#define OCL_SVML_P2_D2_D2_pD2_GLOBAL(func)                                          \
    __attribute__((overloadable)) double2 func(double2 x, __global double2* y)      \
    {                                                                               \
        return func(x, (double2*)y);                                                \
    }

// double3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D3_D3_pD3(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, void*); \
    __attribute__((overloadable)) double3 func(double3 x, double3* y)               \
    {                                                                               \
        double8 valx;                                                               \
        valx.s012 = x;                                                              \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x07, valx, y); \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P2_D3_D3_pD3(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) double3 OCL_SVML_FUNCTION(_##svmlfunc##3,native)(double3, double3*); \
    __attribute__((overloadable)) double3 func(double3 x, double3* y)               \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##3,native)(x, y);                      \
    }
#endif

#define OCL_SVML_P2_D3_D3_pD3_LOCAL(func)                                           \
    __attribute__((overloadable)) double3 func(double3 x, __local double3* y)       \
    {                                                                               \
        return func(x, (double3*)y);                                                \
    }

#define OCL_SVML_P2_D3_D3_pD3_GLOBAL(func)                                          \
    __attribute__((overloadable)) double3 func(double3 x, __global double3* y)      \
    {                                                                               \
        return func(x, (double3*)y);                                                \
    }

// double4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P2_D4_D4_pD4(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, void*); \
    __attribute__((overloadable)) double4 func(double4 x, double4* y)               \
    {                                                                               \
        double8 valx;                                                               \
        valx.lo = x;                                                                \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x0F, valx, y); \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P2_D4_D4_pD4(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4,native)(double4, double4*); \
    __attribute__((overloadable)) double4 func(double4 x, double4* y)               \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##4,native)(x, y);                      \
    }
#endif

#define OCL_SVML_P2_D4_D4_pD4_LOCAL(func)                                           \
    __attribute__((overloadable)) double4 func(double4 x, __local double4* y)       \
    {                                                                               \
        return func(x, (double4*)y);                                                \
    }

#define OCL_SVML_P2_D4_D4_pD4_GLOBAL(func)                                          \
    __attribute__((overloadable)) double4 func(double4 x, __global double4* y)      \
    {                                                                               \
        return func(x, (double4*)y);                                                \
    }

// double8
#define OCL_SVML_P2_D8_D8_pD8(func, svmlfunc, native)                               \
    __attribute__((svmlcc)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8, double8*); \
    __attribute__((overloadable)) double8 func(double8 x, double8* y)               \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x, y);                      \
    }

#define OCL_SVML_P2_D8_D8_pD8_LOCAL(func)                                           \
    __attribute__((overloadable)) double8 func(double8 x, __local double8* y)       \
    {                                                                               \
        return func(x, (double8*)y);                                                \
    }

#define OCL_SVML_P2_D8_D8_pD8_GLOBAL(func)                                          \
    __attribute__((overloadable)) double8 func(double8 x, __global double8* y)      \
    {                                                                               \
        return func(x, (double8*)y);                                                \
    }

// double16
#define OCL_SVML_P2_D16_D16_pD16(func, svmlfunc, native)                            \
    __attribute__((svmlcc)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8, double8*); \
    double16 __attribute__((overloadable)) func(double16 x, double16* y)            \
    {                                                                               \
        double16 res;                                                               \
        double8 lo, hi;                                                             \
        res.lo = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.lo, &lo);               \
        res.hi = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.hi, &hi);               \
        y->lo = lo;                                                                 \
        y->hi = hi;                                                                 \
        return res;                                                                 \
    }                                                                               \

#define OCL_SVML_P2_D16_D16_pD16_GLOBAL(func)                                       \
    double16 __attribute__((overloadable)) func(double16 x, __global double16* y)   \
    {                                                                               \
        return func(x, (double16*)y);                                               \
    }

#define OCL_SVML_P2_D16_D16_pD16_LOCAL(func)                                        \
    double16 __attribute__((overloadable)) func(double16 x, __local double16* y)    \
    {                                                                               \
        return func(x, (double16*)y);                                               \
    }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float
#define OCL_INTR_P2_F1_F1_pF1(func)                                                 \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16, void*);    \
    __attribute__((overloadable)) float func(float x, float* y)                     \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo.lo.lo = x;                                                       \
        float16 res = mask_ ## func(0x01, valx, y);                                 \
        return res.lo.lo.lo.lo;                                                     \
    }

#define OCL_INTR_P2_F1_F1_pF1_GLOBAL(func)                                          \
    __attribute__((overloadable)) float func(float x, __global float* y)            \
    {                                                                               \
        return func(x, (float*)y);                                                  \
    }

#define OCL_INTR_P2_F1_F1_pF1_LOCAL(func)                                           \
    __attribute__((overloadable)) float func(float x, __local float* y)             \
    {                                                                               \
        return func(x, (float*)y);                                                  \
    }

// float2
#define OCL_INTR_P2_F2_F2_pF2(func)                                                 \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16, void*);    \
    __attribute__((overloadable)) float2 func(float2 x, float2* y)                  \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo.lo = x;                                                          \
        float16 res = mask_ ## func(0x02, valx, y);                                 \
        return res.lo.lo.lo;                                                        \
    }

#define OCL_INTR_P2_F2_F2_pF2_LOCAL(func)                                           \
    __attribute__((overloadable)) float2 func(float2 x, __local float2* y)          \
    {                                                                               \
        return func(x, (float2*)y);                                                 \
    }

#define OCL_INTR_P2_F2_F2_pF2_GLOBAL(func)                                          \
    __attribute__((overloadable)) float2 func(float2 x, __global float2* y)         \
    {                                                                               \
        return func(x, (float2*)y);                                                 \
    }

// float3
#define OCL_INTR_P2_F3_F3_pF3(func)                                                 \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16, void*);    \
    __attribute__((overloadable)) float3 func(float3 x, float3* y)                  \
    {                                                                               \
        float16 valx;                                                               \
        valx.s012 = x;                                                              \
        float16 res = mask_ ## func(0x07, valx, y);                                 \
        return res.s012;                                                            \
    }

#define OCL_INTR_P2_F3_F3_pF3_LOCAL(func)                                           \
    __attribute__((overloadable)) float3 func(float3 x, __local float3* y)          \
    {                                                                               \
        return func(x, (float3*)y);                                                 \
    }

#define OCL_INTR_P2_F3_F3_pF3_GLOBAL(func)                                          \
    __attribute__((overloadable)) float3 func(float3 x, __global float3* y)         \
    {                                                                               \
        return func(x, (float3*)y);                                                 \
    }

// float4
#define OCL_INTR_P2_F4_F4_pF4(func)                                                 \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16, void*);    \
    __attribute__((overloadable)) float4 func(float4 x, float4* y)                  \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo.lo = x;                                                             \
        float16 res = mask_ ## func(0x0F, valx, y);                                 \
        return res.lo.lo;                                                           \
    }

#define OCL_INTR_P2_F4_F4_pF4_LOCAL(func)                                           \
    __attribute__((overloadable)) float4 func(float4 x, __local float4* y)          \
    {                                                                               \
        return func(x, (float4*)y);                                                 \
    }

#define OCL_INTR_P2_F4_F4_pF4_GLOBAL(func)                                          \
    __attribute__((overloadable)) float4 func(float4 x, __global float4* y)         \
    {                                                                               \
        return func(x, (float4*)y);                                                 \
    }

// float8
#define OCL_INTR_P2_F8_F8_pF8(func)                                                 \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16, void*);    \
    __attribute__((overloadable)) float8 func(float8 x, float8* y)                  \
    {                                                                               \
        float16 valx;                                                               \
        valx.lo = x;                                                                \
        float16 res = mask_ ## func(0xFF, valx, y);                                 \
        return res.lo;                                                              \
    }

#define OCL_INTR_P2_F8_F8_pF8_LOCAL(func)                                           \
    __attribute__((overloadable)) float8 func(float8 x, __local float8* y)          \
    {                                                                               \
        return func(x, (float8*)y);                                                 \
    }

#define OCL_INTR_P2_F8_F8_pF8_GLOBAL(func)                                          \
    __attribute__((overloadable)) float8 func(float8 x, __global float8* y)         \
    {                                                                               \
        return func(x, (float8*)y);                                                 \
    }

// float16
#define OCL_INTR_P2_F16_F16_pF16(func)                                              \
    __attribute__((overloadable)) float16 func(float16 x, float16* y);

#define OCL_INTR_P2_F16_F16_pF16_LOCAL(func)                                        \
    __attribute__((overloadable)) float16 func(float16 x, __local float16* y)       \
    {                                                                               \
        return func(x, (float16*)y);                                                \
    }

#define OCL_INTR_P2_F16_F16_pF16_GLOBAL(func)                                       \
    __attribute__((overloadable)) float16 func(float16 x, __global float16* y)      \
    {                                                                               \
        return func(x, (float16*)y);                                                \
    }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_F1_F1_F1_F1(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, float16, float16); \
    __attribute__((overloadable)) float func(float x, float y, float z)             \
    {                                                                               \
        float16 valx, valy, valz;                                                   \
        valx.lo.lo.lo.lo = x;                                                       \
        valy.lo.lo.lo.lo = y;                                                       \
        valz.lo.lo.lo.lo = z;                                                       \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x01, valx, valy, valz); \
        return res.lo.lo.lo.lo;                                                     \
    }
#else
#define OCL_SVML_P3_F1_F1_F1_F1(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(float, float, float); \
    __attribute__((overloadable)) float func(float x, float y, float z)             \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(x, y, z);                  \
    }
#endif

// float2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_F2_F2_F2_F2(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, float16, float16); \
    __attribute__((overloadable)) float2 func(float2 x, float2 y, float2 z)         \
    {                                                                               \
        float16 valx, valy, valz;                                                   \
        valx.lo.lo.lo = x;                                                          \
        valy.lo.lo.lo = y;                                                          \
        valz.lo.lo.lo = z;                                                          \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x03, valx, valy, valz); \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P3_F2_F2_F2_F2(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float2 OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(float2, float2, float2); \
    __attribute__((overloadable)) float2 func(float2 x, float2 y, float2 z)         \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(x, y, z);                  \
    }
#endif

// float3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_F3_F3_F3_F3(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, float16, float16); \
    __attribute__((overloadable)) float3 func(float3 x, float3 y, float3 z)         \
    {                                                                               \
        float16 valx, valy, valz;                                                   \
        valx.s012 = x;                                                              \
        valy.s012 = y;                                                              \
        valz.s012 = z;                                                              \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x07, valx, valy, valz); \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P3_F3_F3_F3_F3(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float3 OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(float3, float3, float3); \
    __attribute__((overloadable)) float3 func(float3 x, float3 y, float3 z)         \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(x, y, z);                  \
    }
#endif

// float4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_F4_F4_F4_F4(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, float16, float16); \
    __attribute__((overloadable)) float4 func(float4 x, float4 y, float4 z)         \
    {                                                                               \
        float16 valx, valy, valz;                                                   \
        valx.lo.lo = x;                                                             \
        valy.lo.lo = y;                                                             \
        valz.lo.lo = z;                                                             \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x0F, valx, valy, valz); \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P3_F4_F4_F4_F4(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(float4, float4, float4); \
    __attribute__((overloadable)) float4 func(float4 x, float4 y, float4 z)         \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(x, y, z);                  \
    }
#endif

// float8
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_F8_F8_F8_F8(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, float16, float16); \
    __attribute__((overloadable)) float8 func(float8 x, float8 y, float8 z)         \
    {                                                                               \
        float16 valx, valy, valz;                                                   \
        valx.lo = x;                                                                \
        valy.lo = y;                                                                \
        valz.lo = z;                                                                \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0xFF, valx, valy, valz); \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P3_F8_F8_F8_F8(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) float8 OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(float8, float8, float8); \
    __attribute__((overloadable)) float8 func(float8 x, float8 y, float8 z)         \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(x, y, z);                  \
    }
#endif

// float16
#define OCL_SVML_P3_F16_F16_F16_F16(func, svmlfunc, native)                         \
    __attribute__((svmlcc)) __attribute__((const)) float16 OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(float16, float16, float16); \
    __attribute__((overloadable)) float16 func(float16 x, float16 y, float16 z)     \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(x, y, z);                 \
    }

// double
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_D1_D1_D1_D1(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, double8, double8); \
    __attribute__((overloadable)) double func(double x, double y, double z)         \
    {                                                                               \
        double8 valx, valy, valz;                                                   \
        valx.lo.lo.lo = x;                                                          \
        valy.lo.lo.lo = y;                                                          \
        valz.lo.lo.lo = z;                                                          \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x01, valx, valy, valz); \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P3_D1_D1_D1_D1(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double OCL_SVML_FUNCTION(_##svmlfunc##1,native)(double, double, double); \
    __attribute__((overloadable)) double func(double x, double y, double z)         \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##1,native)(x, y, z);                   \
    }
#endif

// double2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_D2_D2_D2_D2(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, double8, double8); \
    __attribute__((overloadable)) double2 func(double2 x, double2 y, double2 z)     \
    {                                                                               \
        double8 valx, valy, valz;                                                   \
        valx.lo.lo = x;                                                             \
        valy.lo.lo = y;                                                             \
        valz.lo.lo = z;                                                             \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x03, valx, valy, valz); \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P3_D2_D2_D2_D2(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double2 OCL_SVML_FUNCTION(_##svmlfunc##2,native)(double2, double2, double2); \
    __attribute__((overloadable)) double2 func(double2 x, double2 y, double2 z)     \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##2,native)(x, y, z);                   \
    }
#endif

// double3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_D3_D3_D3_D3(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, double8, double8); \
    __attribute__((overloadable)) double3 func(double3 x, double3 y, double3 z)     \
    {                                                                               \
        double8 valx, valy, valz;                                                   \
        valx.s012 = x;                                                              \
        valy.s012 = y;                                                              \
        valz.s012 = z;                                                              \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x07, valx, valy, valz); \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P3_D3_D3_D3_D3(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double3 OCL_SVML_FUNCTION(_##svmlfunc##3,native)(double3, double3, double3); \
    __attribute__((overloadable)) double3 func(double3 x, double3 y, double3 z)     \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##3,native)(x, y, z);                   \
    }
#endif

// double4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_D4_D4_D4_D4(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, double8, double8); \
    __attribute__((overloadable)) double4 func(double4 x, double4 y, double4 z)     \
    {                                                                               \
        double8 valx, valy, valz;                                                   \
        valx.lo = x;                                                                \
        valy.lo = y;                                                                \
        valz.lo = z;                                                                \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x0F, valx, valy, valz); \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P3_D4_D4_D4_D4(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4,native)(double4, double4, double4); \
    __attribute__((overloadable)) double4 func(double4 x, double4 y, double4 z)     \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##4,native)(x, y, z);                   \
    }
#endif

// double8
#define OCL_SVML_P3_D8_D8_D8_D8(func, svmlfunc, native)                             \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8, double8, double8); \
    __attribute__((overloadable)) double8 func(double8 x, double8 y, double8 z)     \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x, y, z);                   \
    }

// double16
#define OCL_SVML_P3_D16_D16_D16_D16(func, svmlfunc, native)                         \
    __attribute__((svmlcc)) __attribute__((const)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8, double8, double8); \
    __attribute__((overloadable)) double16 func(double16 x, double16 y, double16 z) \
    {                                                                               \
        double16 res;                                                               \
        res.lo = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.lo, y.lo, z.lo);        \
        res.hi = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.hi, y.hi, z.hi);        \
        return res;                                                                 \
    }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float
#define OCL_INTR_P3_F1_F1_F1_F1(func)                                               \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16, float16, float16); \
    __attribute__((overloadable)) float func(float x, float y, float z)             \
    {                                                                               \
        float16 valx, valy, valz;                                                   \
        valx.lo.lo.lo.lo = x;                                                       \
        valy.lo.lo.lo.lo = y;                                                       \
        valz.lo.lo.lo.lo = z;                                                       \
        float16 res = mask_ ## func(0x01, valx, valy, valz);                        \
        return res.lo.lo.lo.lo;                                                     \
    }

#define OCL_INTR_P3_F2_F2_F2_F2(func)                                               \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16, float16, float16); \
    __attribute__((overloadable)) float2 func(float2 x, float2 y, float2 z)         \
    {                                                                               \
        float16 valx, valy, valz;                                                   \
        valx.lo.lo.lo = x;                                                          \
        valy.lo.lo.lo = y;                                                          \
        valz.lo.lo.lo = z;                                                          \
        float16 res = mask_ ## func(0x03, valx, valy, valz);                        \
        return res.lo.lo.lo;                                                        \
    }

#define OCL_INTR_P3_F3_F3_F3_F3(func)                                               \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16, float16, float16); \
    __attribute__((overloadable)) float3 func(float3 x, float3 y, float3 z)         \
    {                                                                               \
        float16 valx, valy, valz;                                                   \
        valx.s012 = x;                                                              \
        valy.s012 = y;                                                              \
        valz.s012 = z;                                                              \
        float16 res = mask_ ## func(0x07, valx, valy, valz);                        \
        return res.s012;                                                            \
    }
#define OCL_INTR_P3_F4_F4_F4_F4(func)                                               \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16, float16, float16); \
    __attribute__((overloadable)) float4 func(float4 x, float4 y, float4 z)         \
    {                                                                               \
        float16 valx, valy, valz;                                                   \
        valx.lo.lo = x;                                                             \
        valy.lo.lo = y;                                                             \
        valz.lo.lo = z;                                                             \
        float16 res = mask_ ## func(0x0F, valx, valy, valz);                        \
        return res.lo.lo;                                                           \
    }

#define OCL_INTR_P3_F8_F8_F8_F8(func)                                               \
    __attribute__((overloadable)) float16 mask_ ## func(ushort, float16, float16, float16); \
    __attribute__((overloadable)) float8 func(float8 x, float8 y, float8 z)         \
    {                                                                               \
        float16 valx, valy, valz;                                                   \
        valx.lo = x;                                                                \
        valy.lo = y;                                                                \
        valz.lo = z;                                                                \
        float16 res = mask_ ## func(0xFF, valx, valy, valz);                        \
        return res.lo;                                                              \
    }

#define OCL_INTR_P3_F16_F16_F16_F16(func)                                           \
    __attribute__((overloadable)) float16 func(float16, float16, float16);


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_F1_F1_F1_pI1(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, float16, void*); \
    __attribute__((overloadable)) float func(float x, float y, _1##sign##32* z)     \
    {                                                                               \
        float16 valx, valy;                                                         \
        valx.lo.lo.lo.lo = x;                                                       \
        valy.lo.lo.lo.lo = y;                                                       \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x01, valx, valy, z); \
        return res.lo.lo.lo.lo;                                                     \
    }
#else
#define OCL_SVML_P3_F1_F1_F1_pI1(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) float OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(float, float, _1##sign##32*); \
    __attribute__((overloadable)) float func(float x, float y, _1##sign##32* z)     \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f1,native)(x, y, z);                  \
    }
#endif

#define OCL_SVML_P3_F1_F1_F1_pI1_LOCAL(func, sign)                                  \
    __attribute__((overloadable)) float func(float x, float y, __local _1##sign##32* z) \
    {                                                                               \
        return func(x, y, (int*)z);                                                 \
    }                                                                               \

#define OCL_SVML_P3_F1_F1_F1_pI1_GLOBAL(func, sign)                                 \
    __attribute__((overloadable)) float func(float x, float y, __global _1##sign##32* z) \
    {                                                                               \
        return func(x, y, (int*)z);                                                 \
    }

// float2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_F2_F2_F2_pI2(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, float16, void*); \
    __attribute__((overloadable)) float2 func(float2 x, float2 y, _2##sign##32* z)  \
    {                                                                               \
        float16 valx, valy;                                                         \
        valx.lo.lo.lo = x;                                                          \
        valy.lo.lo.lo = y;                                                          \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x03, valx, valy, z); \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P3_F2_F2_F2_pI2(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) float2 OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(float2, float2, _2##sign##32*); \
    __attribute__((overloadable)) float2 func(float2 x, float2 y, _2##sign##32* z)  \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f2,native)(x, y, z);                  \
    }
#endif

#define OCL_SVML_P3_F2_F2_F2_pI2_LOCAL(func, sign)                                  \
    __attribute__((overloadable)) float2 func(float2 x, float2 y, __local _2##sign##32* z) \
    {                                                                               \
        return func(x, y, (_2##sign##32*)z);                                        \
    }

#define OCL_SVML_P3_F2_F2_F2_pI2_GLOBAL(func, sign)                                 \
    __attribute__((overloadable)) float2 func(float2 x, float2 y, __global _2##sign##32* z) \
    {                                                                               \
        return func(x, y, (_2##sign##32*)z);                                        \
    }

// float3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_F3_F3_F3_pI3(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, float16, void*); \
    __attribute__((overloadable)) float3 func(float3 x, float3 y, _3##sign##32* z)  \
    {                                                                               \
        float16 valx, valy;                                                         \
        valx.s012 = x;                                                              \
        valy.s012 = y;                                                              \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x07, valx, valy, z); \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P3_F3_F3_F3_pI3(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) float3 OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(float3, float3, _3##sign##32*); \
    __attribute__((overloadable)) float3 func(float3 x, float3 y, _3##sign##32* z)  \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f3,native)(x, y, z);                  \
    }
#endif

#define OCL_SVML_P3_F3_F3_F3_pI3_LOCAL(func, sign)                                  \
    __attribute__((overloadable)) float3 func(float3 x, float3 y, __local _3##sign##32* z) \
    {                                                                               \
        return func(x, y, (_3##sign##32*)z);                                        \
    }

#define OCL_SVML_P3_F3_F3_F3_pI3_GLOBAL(func, sign)                                 \
    __attribute__((overloadable)) float3 func(float3 x, float3 y, __global _3##sign##32* z) \
    {                                                                               \
        return func(x, y, (_3##sign##32*)z);                                        \
    }

// float4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_F4_F4_F4_pI4(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, float16, void*); \
    __attribute__((overloadable)) float4 func(float4 x, float4 y, _4##sign##32* z)  \
    {                                                                               \
        float16 valx, valy;                                                         \
        valx.lo.lo = x;                                                             \
        valy.lo.lo = y;                                                             \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0x0F, valx, valy, z); \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P3_F4_F4_F4_pI4(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) float4 OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(float4, float4, _4##sign##32*); \
    __attribute__((overloadable)) float4 func(float4 x, float4 y, _4##sign##32* z)  \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f4,native)(x, y, z);                  \
    }
#endif

#define OCL_SVML_P3_F4_F4_F4_pI4_LOCAL(func, sign)                                  \
    __attribute__((overloadable)) float4 func(float4 x, float4 y, __local _4##sign##32* z) \
    {                                                                               \
        return func(x, y, (_4##sign##32*)z);                                        \
    }

#define OCL_SVML_P3_F4_F4_F4_pI4_GLOBAL(func, sign)                                 \
    __attribute__((overloadable)) float4 func(float4 x, float4 y, __global _4##sign##32* z) \
    {                                                                               \
        return func(x, y, (_4##sign##32*)z);                                        \
    }

// float8
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_F8_F8_F8_pI8(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) float16 OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(ushort, float16, float16, void*); \
    __attribute__((overloadable)) float8 func(float8 x, float8 y, _8##sign##32* z)  \
    {                                                                               \
        float16 valx, valy;                                                         \
        valx.lo = x;                                                                \
        valy.lo = y;                                                                \
        float16 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##f16,native)(0xFF, valx, valy, z); \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P3_F8_F8_F8_pI8(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) float8 OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(float8, float8, _8##sign##32*); \
    __attribute__((overloadable)) float8 func(float8 x, float8 y, _8##sign##32* z)  \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f8,native)(x, y, z);                  \
    }
#endif

#define OCL_SVML_P3_F8_F8_F8_pI8_LOCAL(func, sign)                                  \
    __attribute__((overloadable)) float8 func(float8 x, float8 y, __local _8##sign##32* z) \
    {                                                                               \
        return func(x, y, (_8##sign##32*)z);                                        \
    }

#define OCL_SVML_P3_F8_F8_F8_pI8_GLOBAL(func, sign)                                 \
    __attribute__((overloadable)) float8 func(float8 x, float8 y, __global _8##sign##32* z) \
    {                                                                               \
        return func(x, y, (_8##sign##32*)z);                                        \
    }

// float16
#define OCL_SVML_P3_F16_F16_F16_pI16(func, sign, svmlfunc, native)                  \
    __attribute__((svmlcc)) float16 OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(float16, float16, _16##sign##32*); \
    __attribute__((overloadable)) float16 func(float16 x, float16 y, _16##sign##32* z) \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##f16,native)(x, y, z);                 \
    }

#define OCL_SVML_P3_F16_F16_F16_pI16_LOCAL(func, sign)                              \
    __attribute__((overloadable)) float16 func(float16 x, float16 y, __local _16##sign##32* z) \
    {                                                                               \
        return func(x, y, (_16##sign##32*)z);                                       \
    }

#define OCL_SVML_P3_F16_F16_F16_pI16_GLOBAL(func, sign)                             \
    __attribute__((overloadable)) float16 func(float16 x, float16 y, __global _16##sign##32* z) \
    {                                                                               \
        return func(x, y, (_16##sign##32*)z);                                       \
    }

// double
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_D1_D1_D1_pI1(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, double8, void*); \
    __attribute__((overloadable)) double func(double x, double y, _1##sign##32* z)  \
    {                                                                               \
        double8 valx, valy;                                                         \
        valx.lo.lo.lo = x;                                                          \
        valy.lo.lo.lo = y;                                                          \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x01, valx, valy, z); \
        return res.lo.lo.lo;                                                        \
    }
#else
#define OCL_SVML_P3_D1_D1_D1_pI1(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) double OCL_SVML_FUNCTION(_##svmlfunc##1,native)(double, double, _1##sign##32*); \
    __attribute__((overloadable)) double func(double x, double y, _1##sign##32* z)  \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##1,native)(x, y, z);                   \
    }
#endif

#define OCL_SVML_P3_D1_D1_D1_pI1_LOCAL(func, sign)                                  \
    __attribute__((overloadable)) double func(double x, double y, __local _1##sign##32* z) \
    {                                                                               \
        return func(x, y, (int*)z);                                                 \
    }                                                                               \

#define OCL_SVML_P3_D1_D1_D1_pI1_GLOBAL(func, sign)                                 \
    __attribute__((overloadable)) double func(double x, double y, __global _1##sign##32* z) \
    {                                                                               \
        return func(x, y, (int*)z);                                                 \
    }

// double2
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_D2_D2_D2_pI2(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, double8, void*); \
    __attribute__((overloadable)) double2 func(double2 x, double2 y, _2##sign##32* z) \
    {                                                                               \
        double8 valx, valy;                                                         \
        valx.lo.lo = x;                                                             \
        valy.lo.lo = y;                                                             \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x03, valx, valy, z); \
        return res.lo.lo;                                                           \
    }
#else
#define OCL_SVML_P3_D2_D2_D2_pI2(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) double2 OCL_SVML_FUNCTION(_##svmlfunc##2,native)(double2, double2, _2##sign##32*); \
    __attribute__((overloadable)) double2 func(double2 x, double2 y, _2##sign##32* z) \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##2,native)(x, y, z);                   \
    }
#endif

#define OCL_SVML_P3_D2_D2_D2_pI2_LOCAL(func, sign)                                  \
    __attribute__((overloadable)) double2 func(double2 x, double2 y, __local _2##sign##32* z) \
    {                                                                               \
        return func(x, y, (_2##sign##32*)z);                                        \
    }

#define OCL_SVML_P3_D2_D2_D2_pI2_GLOBAL(func, sign)                                 \
    __attribute__((overloadable)) double2 func(double2 x, double2 y, __global _2##sign##32* z) \
    {                                                                               \
        return func(x, y, (_2##sign##32*)z);                                        \
    }

// double3
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_D3_D3_D3_pI3(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, double8, void*); \
    __attribute__((overloadable)) double3 func(double3 x, double3 y, _3##sign##32* z) \
    {                                                                               \
        double8 valx, valy;                                                         \
        valx.s012 = x;                                                              \
        valy.s012 = y;                                                              \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x07, valx, valy, z); \
        return res.s012;                                                            \
    }
#else
#define OCL_SVML_P3_D3_D3_D3_pI3(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) double3 OCL_SVML_FUNCTION(_##svmlfunc##3,native)(double3, double3, _3##sign##32*); \
    __attribute__((overloadable)) double3 func(double3 x, double3 y, _3##sign##32* z) \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##3,native)(x, y, z);                   \
    }
#endif

#define OCL_SVML_P3_D3_D3_D3_pI3_LOCAL(func, sign)                                  \
    __attribute__((overloadable)) double3 func(double3 x, double3 y, __local _3##sign##32* z) \
    {                                                                               \
        return func(x, y, (_3##sign##32*)z);                                        \
    }

#define OCL_SVML_P3_D3_D3_D3_pI3_GLOBAL(func, sign)                                 \
    __attribute__((overloadable)) double3 func(double3 x, double3 y, __global _3##sign##32* z) \
    {                                                                               \
        return func(x, y, (_3##sign##32*)z);                                        \
    }

// double4
#if defined(USE_OCL_SVML_MASK_INTERFACE)
#define OCL_SVML_P3_D4_D4_D4_pI4(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) double8 OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(uchar, double8, double8, void*); \
    __attribute__((overloadable)) double4 func(double4 x, double4 y, _4##sign##32* z) \
    {                                                                               \
        double8 valx, valy;                                                         \
        valx.lo = x;                                                                \
        valy.lo = y;                                                                \
        double8 res = OCL_SVML_MASK_FUNCTION(_##svmlfunc##8,native)(0x0F, valx, valy, z); \
        return res.lo;                                                              \
    }
#else
#define OCL_SVML_P3_D4_D4_D4_pI4(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) double4 OCL_SVML_FUNCTION(_##svmlfunc##4,native)(double4, double4, _4##sign##32*); \
    __attribute__((overloadable)) double4 func(double4 x, double4 y, _4##sign##32* z) \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##4,native)(x, y, z);                   \
    }
#endif

#define OCL_SVML_P3_D4_D4_D4_pI4_LOCAL(func, sign)                                  \
    __attribute__((overloadable)) double4 func(double4 x, double4 y, __local _4##sign##32* z) \
    {                                                                               \
        return func(x, y, (_4##sign##32*)z);                                        \
    }

#define OCL_SVML_P3_D4_D4_D4_pI4_GLOBAL(func, sign)                                 \
    __attribute__((overloadable)) double4 func(double4 x, double4 y, __global _4##sign##32* z) \
    {                                                                               \
        return func(x, y, (_4##sign##32*)z);                                        \
    }

// double8
#define OCL_SVML_P3_D8_D8_D8_pI8(func, sign, svmlfunc, native)                      \
    __attribute__((svmlcc)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8, double8, _8##sign##32*); \
    __attribute__((overloadable)) double8 func(double8 x, double8 y, _8##sign##32* z)   \
    {                                                                               \
        return OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x, y, z);                   \
    }

#define OCL_SVML_P3_D8_D8_D8_pI8_LOCAL(func, sign)                                  \
    __attribute__((overloadable)) double8 func(double8 x, double8 y, __local _8##sign##32* z) \
    {                                                                               \
        return func(x, y, (_8##sign##32*)z);                                        \
    }

#define OCL_SVML_P3_D8_D8_D8_pI8_GLOBAL(func, sign)                                 \
    __attribute__((overloadable)) double8 func(double8 x, double8 y, __global _8##sign##32* z) \
    {                                                                               \
        return func(x, y, (_8##sign##32*)z);                                        \
    }

// double16
#define OCL_SVML_P3_D16_D16_D16_pI16(func, sign, svmlfunc, native)                  \
    __attribute__((svmlcc)) double8 OCL_SVML_FUNCTION(_##svmlfunc##8,native)(double8, double8, _8##sign##32*); \
    __attribute__((overloadable)) double16 func(double16 x, double16 y, _16##sign##32* z) \
    {                                                                               \
        double16 res;                                                               \
        _8##sign##32 lo, hi;                                                        \
        res.lo = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.lo, y.lo, &lo);         \
        res.hi = OCL_SVML_FUNCTION(_##svmlfunc##8,native)(x.hi, y.hi, &hi);         \
        z->lo = lo;                                                                 \
        z->hi = hi;                                                                 \
        return res;                                                                 \
    }

#define OCL_SVML_P3_D16_D16_D16_pI16_LOCAL(func, sign)                              \
    __attribute__((overloadable)) double16 func(double16 x, double16 y, __local _16##sign##32* z) \
    {                                                                               \
        return func(x, y, (_16##sign##32*)z);                                       \
    }

#define OCL_SVML_P3_D16_D16_D16_pI16_GLOBAL(func, sign)                             \
    __attribute__((overloadable)) double16 func(double16 x, double16 y, __global _16##sign##32* z) \
    {                                                                               \
        return func(x, y, (_16##sign##32*)z);                                       \
    }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float2
#define OCL_FUNC_P2_F2_F2_F1(func)                                                  \
    __attribute__((overloadable)) float2 func(float2 x, float y)                    \
    {                                                                               \
        float2 valy = (float2)y;                                                    \
        return func(x, valy);                                                       \
    }

// float3
#define OCL_FUNC_P2_F3_F3_F1(func)                                                  \
    __attribute__((overloadable)) float3 func(float3 x, float y)                    \
    {                                                                               \
        float3 valy = (float3)y;                                                    \
        return func(x, valy);                                                       \
    }

// float4
#define OCL_FUNC_P2_F4_F4_F1(func)                                                  \
    __attribute__((overloadable)) float4 func(float4 x, float y)                    \
    {                                                                               \
        float4 valy = (float4)y;                                                    \
        return func(x, valy);                                                       \
    }

// float8
#define OCL_FUNC_P2_F8_F8_F1(func)                                                  \
    __attribute__((overloadable)) float8 func(float8 x, float y)                    \
    {                                                                               \
        float8 valy = (float8)y;                                                    \
        return func(x, valy);                                                       \
    }

// float16
#define OCL_FUNC_P2_F16_F16_F1(func)                                                \
    __attribute__((overloadable)) float16 func(float16 x, float y)                  \
    {                                                                               \
        float16 valy = (float16)y;                                                  \
        return func(x, valy);                                                       \
    }

// double2
#define OCL_FUNC_P2_D2_D2_D1(func)                                                  \
    __attribute__((overloadable)) double2 func(double2 x, double y)                 \
    {                                                                               \
        double2 valy = (double2)y;                                                  \
        return func(x, valy);                                                       \
    }

// double3
#define OCL_FUNC_P2_D3_D3_D1(func)                                                  \
    __attribute__((overloadable)) double3 func(double3 x, double y)                 \
    {                                                                               \
        double3 valy = (double3)y;                                                  \
        return func(x, valy);                                                       \
    }

// double4
#define OCL_FUNC_P2_D4_D4_D1(func)                                                  \
    __attribute__((overloadable)) double4 func(double4 x, double y)                 \
    {                                                                               \
        double4 valy = (double4)y;                                                  \
        return func(x, valy);                                                       \
    }

// double8
#define OCL_FUNC_P2_D8_D8_D1(func)                                                  \
    __attribute__((overloadable)) double8 func(double8 x, double y)                 \
    {                                                                               \
        double8 valy = (double8)y;                                                  \
        return func(x, valy);                                                       \
    }

// double16
#define OCL_FUNC_P2_D16_D16_D1(func)                                                \
    __attribute__((overloadable)) double16 func(double16 x, double y)               \
    {                                                                               \
        double16 valy = (double16)y;                                                \
        return func(x, valy);                                                       \
    }


//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
// float2
#define OCL_FUNC_P2_F2_F2_I1(func, sign)                                            \
    __attribute__((overloadable)) float2 func(float2 x, _1##sign##32 y)             \
    {                                                                               \
        _2##sign##32 valy = (_2##sign##32)y;                                        \
        return func(x, valy);                                                       \
    }

// float3
#define OCL_FUNC_P2_F3_F3_I1(func, sign)                                            \
    __attribute__((overloadable)) float3 func(float3 x, _1##sign##32 y)             \
    {                                                                               \
        _3##sign##32 valy = (_3##sign##32)y;                                        \
        return func(x, valy);                                                       \
    }

// float4
#define OCL_FUNC_P2_F4_F4_I1(func, sign)                                            \
    __attribute__((overloadable)) float4 func(float4 x, _1##sign##32 y)             \
    {                                                                               \
        _4##sign##32 valy = (_4##sign##32)y;                                        \
        return func(x, valy);                                                       \
    }

// float8
#define OCL_FUNC_P2_F8_F8_I1(func, sign)                                            \
    __attribute__((overloadable)) float8 func(float8 x, _1##sign##32 y)             \
    {                                                                               \
        _8##sign##32 valy = (_8##sign##32)y;                                        \
        return func(x, valy);                                                       \
    }

// float16
#define OCL_FUNC_P2_F16_F16_I1(func, sign)                                          \
    __attribute__((overloadable)) float16 func(float16 x, _1##sign##32 y)           \
    {                                                                               \
        _16##sign##32 valy = (_16##sign##32)y;                                      \
        return func(x, valy);                                                       \
    }

// double2
#define OCL_FUNC_P2_D2_D2_I1(func, sign)                                            \
    __attribute__((overloadable)) double2 func(double2 x, _1##sign##32 y)           \
    {                                                                               \
        _2##sign##32 valy = (_2##sign##32)y;                                        \
        return func(x, valy);                                                       \
    }

// double3
#define OCL_FUNC_P2_D3_D3_I1(func, sign)                                            \
    __attribute__((overloadable)) double3 func(double3 x, _1##sign##32 y)           \
    {                                                                               \
        _3##sign##32 valy = (_3##sign##32)y;                                        \
        return func(x, valy);                                                       \
    }

// double4
#define OCL_FUNC_P2_D4_D4_I1(func, sign)                                            \
    __attribute__((overloadable)) double4 func(double4 x, _1##sign##32 y)           \
    {                                                                               \
        _4##sign##32 valy = (_4##sign##32)y;                                        \
        return func(x, valy);                                                       \
    }

// double8
#define OCL_FUNC_P2_D8_D8_I1(func, sign)                                            \
    __attribute__((overloadable)) double8 func(double8 x, _1##sign##32 y)           \
    {                                                                               \
        _8##sign##32 valy = (_8##sign##32)y;                                        \
        return func(x, valy);                                                       \
    }

// double16
#define OCL_FUNC_P2_D16_D16_I1(func, sign)                                          \
    __attribute__((overloadable)) double16 func(double16 x, _1##sign##32 y)         \
    {                                                                               \
        _16##sign##32 valy = (_16##sign##32)y;                                      \
        return func(x, valy);                                                       \
    }

#ifdef __cplusplus
}
#endif
