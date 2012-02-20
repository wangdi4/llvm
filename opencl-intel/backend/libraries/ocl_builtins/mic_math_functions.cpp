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
//  mic_math_functions.cpp
///////////////////////////////////////////////////////////

#if defined (__MIC__) || defined(__MIC2__)

#include <intrin.h>

#include "mic_cl_math_declaration.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// F   := gentypef  := float/float2/float3/float4/float8/float16
// D   := gentyped  := double/double2/double3/double4/double8/double16
// C,i := sgentypec := char/char2/char3/char4/char8/char16
// S,i := sgentypes := short/short2/short3/short4/short8/short16
// I,i := sgentypei := int/int2/int3/int4/int8/int16
// L,i := sgentypel := long/long2/long3/long4/long8/long16
// C,u := ugentypec := uchar/uchar2/uchar3/uchar4/uchar8/uchar16
// S,u := ugentypes := ushort/ushort2/ushort3/ushort4/ushort8/ushort16
// I,u := ugentypei := uint/uint2/uint3/uint4/uint8/uint16
// L,u := ugentypel := ulong/ulong2/ulong3/ulong4/ulong8/ulong16
//
// G  := gentype  := gentypef/gentyped
// sG := sgentype := sgentypec/sgentypes/sgentypei/sgentypel
// uG := ugentype := ugentypec/ugentypeu/ugentypei/ugentypel
//

// gentypef := f(gentypef)
#define OCL_INTR_P1_Fn_Fn(func) \
    OCL_INTR_P1_F1_F1(func)     \
    OCL_INTR_P1_F2_F2(func)     \
    OCL_INTR_P1_F3_F3(func)     \
    OCL_INTR_P1_F4_F4(func)     \
    OCL_INTR_P1_F8_F8(func)     \
    OCL_INTR_P1_F16_F16(func)

// gentyped := f(gentyped)
#define OCL_INTR_P1_Dn_Dn(func) \
    OCL_INTR_P1_D1_D1(func)     \
    OCL_INTR_P1_D2_D2(func)     \
    OCL_INTR_P1_D3_D3(func)     \
    OCL_INTR_P1_D4_D4(func)     \
    OCL_INTR_P1_D8_D8(func)     \
    OCL_INTR_P1_D16_D16(func)

// gentype := f(gentype)
#define OCL_INTR_P1_Gn_Gn(func) \
    OCL_INTR_P1_Fn_Fn(func)     \
    OCL_INTR_P1_Dn_Dn(func)

// gentypef := f(gentypef)
#define OCL_SVML_P1_Fn_Fn(func, svmlfunc, native)                           \
    OCL_SVML_P1_F1_F1                       (func, svmlfunc, native)        \
    OCL_SVML_P1_F2_F2                       (func, svmlfunc, native)        \
    OCL_SVML_P1_F3_F3                       (func, svmlfunc, native)        \
    OCL_SVML_P1_F4_F4                       (func, svmlfunc, native)        \
    OCL_SVML_P1_F8_F8                       (func, svmlfunc, native)        \
    OCL_SVML_P1_F16_F16                     (func, svmlfunc, native)

// gentyped := f(gentyped)
#define OCL_SVML_P1_Dn_Dn(func, svmlfunc, native)                           \
    OCL_SVML_P1_D1_D1                       (func, svmlfunc, native)        \
    OCL_SVML_P1_D2_D2                       (func, svmlfunc, native)        \
    OCL_SVML_P1_D3_D3                       (func, svmlfunc, native)        \
    OCL_SVML_P1_D4_D4                       (func, svmlfunc, native)        \
    OCL_SVML_P1_D8_D8                       (func, svmlfunc, native)        \
    OCL_SVML_P1_D16_D16                     (func, svmlfunc, native)

// gentype := f(gentype)
#define OCL_SVML_P1_Gn_Gn(func, svmlfunc, native)                           \
    OCL_SVML_P1_Fn_Fn                       (func, svmlfunc, native)        \
    OCL_SVML_P1_Dn_Dn                       (func, svmlfunc, native)

// sgentypei/ugentypei := f(gentypef)
#define OCL_SVML_P1_In_Fn(func, sign, svmlfunc, native)                     \
    OCL_SVML_P1_I1_F1                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_I2_F2                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_I3_F3                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_I4_F4                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_I8_F8                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_I16_F16                     (func, sign, svmlfunc, native)

// sgentypei/ugentypei := f(gentyped)
#define OCL_SVML_P1_In_Dn(func, sign, svmlfunc, native)                     \
    OCL_SVML_P1_I1_D1                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_I2_D2                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_I3_D3                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_I4_D4                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_I8_D8                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_I16_D16                     (func, sign, svmlfunc, native)

// gentypef := f(sgentypei/ugentypei)
#define OCL_SVML_P1_Fn_In(func, sign, svmlfunc, native)                     \
    OCL_SVML_P1_F1_I1                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_F2_I2                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_F3_I3                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_F4_I4                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_F8_I8                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_F16_I16                     (func, sign, svmlfunc, native)

// gentyped := f(sgentypel/ugentypel)
#define OCL_SVML_P1_Dn_Ln(func, sign, svmlfunc, native)                     \
    OCL_SVML_P1_D1_L1                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_D2_L2                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_D3_L3                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_D4_L4                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_D8_L8                       (func, sign, svmlfunc, native)  \
    OCL_SVML_P1_D16_L16                     (func, sign, svmlfunc, native)

// gentypef := f(gentypef, gentypef)
#define OCL_SVML_P2_Fn_FnFn(func, svmlfunc, native)                         \
    OCL_SVML_P2_F1_F1_F1                    (func, svmlfunc, native)        \
    OCL_SVML_P2_F2_F2_F2                    (func, svmlfunc, native)        \
    OCL_SVML_P2_F3_F3_F3                    (func, svmlfunc, native)        \
    OCL_SVML_P2_F4_F4_F4                    (func, svmlfunc, native)        \
    OCL_SVML_P2_F8_F8_F8                    (func, svmlfunc, native)        \
    OCL_SVML_P2_F16_F16_F16                 (func, svmlfunc, native)

// gentyped := f(gentyped, gentyped)
#define OCL_SVML_P2_Dn_DnDn(func, svmlfunc, native)                         \
    OCL_SVML_P2_D1_D1_D1                    (func, svmlfunc, native)        \
    OCL_SVML_P2_D2_D2_D2                    (func, svmlfunc, native)        \
    OCL_SVML_P2_D3_D3_D3                    (func, svmlfunc, native)        \
    OCL_SVML_P2_D4_D4_D4                    (func, svmlfunc, native)        \
    OCL_SVML_P2_D8_D8_D8                    (func, svmlfunc, native)        \
    OCL_SVML_P2_D16_D16_D16                 (func, svmlfunc, native)

// gentype := f(gentype, gentype)
#define OCL_SVML_P2_Gn_GnGn(func, svmlfunc, native)                         \
    OCL_SVML_P2_Fn_FnFn                     (func, svmlfunc, native)        \
    OCL_SVML_P2_Dn_DnDn                     (func, svmlfunc, native)

// gentypef := f(gentypef, float)
#define OCL_FUNC_P2_Fn_FnF1(func)                                           \
    OCL_FUNC_P2_F2_F2_F1                    (func)                          \
    OCL_FUNC_P2_F3_F3_F1                    (func)                          \
    OCL_FUNC_P2_F4_F4_F1                    (func)                          \
    OCL_FUNC_P2_F8_F8_F1                    (func)                          \
    OCL_FUNC_P2_F16_F16_F1                  (func)

// gentyped := f(gentyped, double)
#define OCL_FUNC_P2_Dn_DnD1(func)                                           \
    OCL_FUNC_P2_D2_D2_D1                    (func)                          \
    OCL_FUNC_P2_D3_D3_D1                    (func)                          \
    OCL_FUNC_P2_D4_D4_D1                    (func)                          \
    OCL_FUNC_P2_D8_D8_D1                    (func)                          \
    OCL_FUNC_P2_D16_D16_D1                  (func)

// gentypef := f(gentypef, sgentypei)
#define OCL_SVML_P2_Fn_FnIn(func, sign, svmlfunc, native)                   \
    OCL_SVML_P2_F1_F1_I1                    (func, sign, svmlfunc, native)  \
    OCL_SVML_P2_F2_F2_I2                    (func, sign, svmlfunc, native)  \
    OCL_SVML_P2_F3_F3_I3                    (func, sign, svmlfunc, native)  \
    OCL_SVML_P2_F4_F4_I4                    (func, sign, svmlfunc, native)  \
    OCL_SVML_P2_F8_F8_I8                    (func, sign, svmlfunc, native)  \
    OCL_SVML_P2_F16_F16_I16                 (func, sign, svmlfunc, native)

// gentyped := f(gentyped, sgentypei)
#define OCL_SVML_P2_Dn_DnIn(func, sign, svmlfunc, native)                   \
    OCL_SVML_P2_D1_D1_I1                    (func, sign, svmlfunc, native)  \
    OCL_SVML_P2_D2_D2_I2                    (func, sign, svmlfunc, native)  \
    OCL_SVML_P2_D3_D3_I3                    (func, sign, svmlfunc, native)  \
    OCL_SVML_P2_D4_D4_I4                    (func, sign, svmlfunc, native)  \
    OCL_SVML_P2_D8_D8_I8                    (func, sign, svmlfunc, native)  \
    OCL_SVML_P2_D16_D16_I16                 (func, sign, svmlfunc, native)

// gentypef := f(gentypef, int/uint)
#define OCL_FUNC_P2_Fn_FnI1(func, sign)                                     \
    OCL_FUNC_P2_F2_F2_I1                    (func, sign)                    \
    OCL_FUNC_P2_F3_F3_I1                    (func, sign)                    \
    OCL_FUNC_P2_F4_F4_I1                    (func, sign)                    \
    OCL_FUNC_P2_F8_F8_I1                    (func, sign)                    \
    OCL_FUNC_P2_F16_F16_I1                  (func, sign)

// gentyped := f(gentyped, int/uint)
#define OCL_FUNC_P2_Dn_DnI1(func, sign)                                     \
    OCL_FUNC_P2_D2_D2_I1                    (func, sign)                    \
    OCL_FUNC_P2_D3_D3_I1                    (func, sign)                    \
    OCL_FUNC_P2_D4_D4_I1                    (func, sign)                    \
    OCL_FUNC_P2_D8_D8_I1                    (func, sign)                    \
    OCL_FUNC_P2_D16_D16_I1                  (func, sign)

// gentypef := f(gentypef, gentypef*)
#define OCL_SVML_P2_Fn_FnpFn(func, svmlfunc, native)                        \
    OCL_SVML_P2_F1_F1_pF1                   (func, svmlfunc, native)        \
        OCL_SVML_P2_F1_F1_pF1_LOCAL         (func)                          \
        OCL_SVML_P2_F1_F1_pF1_GLOBAL        (func)                          \
    OCL_SVML_P2_F2_F2_pF2                   (func, svmlfunc, native)        \
        OCL_SVML_P2_F2_F2_pF2_LOCAL         (func)                          \
        OCL_SVML_P2_F2_F2_pF2_GLOBAL        (func)                          \
    OCL_SVML_P2_F3_F3_pF3                   (func, svmlfunc, native)        \
        OCL_SVML_P2_F3_F3_pF3_LOCAL         (func)                          \
        OCL_SVML_P2_F3_F3_pF3_GLOBAL        (func)                          \
    OCL_SVML_P2_F4_F4_pF4                   (func, svmlfunc, native)        \
        OCL_SVML_P2_F4_F4_pF4_LOCAL         (func)                          \
        OCL_SVML_P2_F4_F4_pF4_GLOBAL        (func)                          \
    OCL_SVML_P2_F8_F8_pF8                   (func, svmlfunc, native)        \
        OCL_SVML_P2_F8_F8_pF8_LOCAL         (func)                          \
        OCL_SVML_P2_F8_F8_pF8_GLOBAL        (func)                          \
    OCL_SVML_P2_F16_F16_pF16                (func, svmlfunc, native)        \
        OCL_SVML_P2_F16_F16_pF16_LOCAL      (func)                          \
        OCL_SVML_P2_F16_F16_pF16_GLOBAL     (func)

// gentyped := f(gentyped, gentyped*)
#define OCL_SVML_P2_Dn_DnpDn(func, svmlfunc, native)                        \
    OCL_SVML_P2_D1_D1_pD1                   (func, svmlfunc, native)        \
        OCL_SVML_P2_D1_D1_pD1_LOCAL         (func)                          \
        OCL_SVML_P2_D1_D1_pD1_GLOBAL        (func)                          \
    OCL_SVML_P2_D2_D2_pD2                   (func, svmlfunc, native)        \
        OCL_SVML_P2_D2_D2_pD2_LOCAL         (func)                          \
        OCL_SVML_P2_D2_D2_pD2_GLOBAL        (func)                          \
    OCL_SVML_P2_D3_D3_pD3                   (func, svmlfunc, native)        \
        OCL_SVML_P2_D3_D3_pD3_LOCAL         (func)                          \
        OCL_SVML_P2_D3_D3_pD3_GLOBAL        (func)                          \
    OCL_SVML_P2_D4_D4_pD4                   (func, svmlfunc, native)        \
        OCL_SVML_P2_D4_D4_pD4_LOCAL         (func)                          \
        OCL_SVML_P2_D4_D4_pD4_GLOBAL        (func)                          \
    OCL_SVML_P2_D8_D8_pD8                   (func, svmlfunc, native)        \
        OCL_SVML_P2_D8_D8_pD8_LOCAL         (func)                          \
        OCL_SVML_P2_D8_D8_pD8_GLOBAL        (func)                          \
    OCL_SVML_P2_D16_D16_pD16                (func, svmlfunc, native)        \
        OCL_SVML_P2_D16_D16_pD16_LOCAL      (func)                          \
        OCL_SVML_P2_D16_D16_pD16_GLOBAL     (func)

// gentype := f(gentype, gentype*)
#define OCL_SVML_P2_Gn_GnpGn(func, svmlfunc, native)                        \
    OCL_SVML_P2_Fn_FnpFn                    (func, svmlfunc, native)        \
    OCL_SVML_P2_Dn_DnpDn                    (func, svmlfunc, native)        \

// gentypef := f(gentypef, sgentypei/ugentypei*)
#define OCL_SVML_P2_Fn_FnpIn(func, sign, svmlfunc, native)                  \
    OCL_SVML_P2_F1_F1_pI1                   (func, sign, svmlfunc, native)  \
        OCL_SVML_P2_F1_F1_pI1_LOCAL         (func, sign)                    \
        OCL_SVML_P2_F1_F1_pI1_GLOBAL        (func, sign)                    \
    OCL_SVML_P2_F2_F2_pI2                   (func, sign, svmlfunc, native)  \
        OCL_SVML_P2_F2_F2_pI2_LOCAL         (func, sign)                    \
        OCL_SVML_P2_F2_F2_pI2_GLOBAL        (func, sign)                    \
    OCL_SVML_P2_F3_F3_pI3                   (func, sign, svmlfunc, native)  \
        OCL_SVML_P2_F3_F3_pI3_LOCAL         (func, sign)                    \
        OCL_SVML_P2_F3_F3_pI3_GLOBAL        (func, sign)                    \
    OCL_SVML_P2_F4_F4_pI4                   (func, sign, svmlfunc, native)  \
        OCL_SVML_P2_F4_F4_pI4_LOCAL         (func, sign)                    \
        OCL_SVML_P2_F4_F4_pI4_GLOBAL        (func, sign)                    \
    OCL_SVML_P2_F8_F8_pI8                   (func, sign, svmlfunc, native)  \
        OCL_SVML_P2_F8_F8_pI8_LOCAL         (func, sign)                    \
        OCL_SVML_P2_F8_F8_pI8_GLOBAL        (func, sign)                    \
    OCL_SVML_P2_F16_F16_pI16                (func, sign, svmlfunc, native)  \
        OCL_SVML_P2_F16_F16_pI16_LOCAL      (func, sign)                    \
        OCL_SVML_P2_F16_F16_pI16_GLOBAL     (func, sign)

// gentyped := f(gentyped, sgentypei/ugentypei*)
#define OCL_SVML_P2_Dn_DnpIn(func, sign, svmlfunc, native)                  \
    OCL_SVML_P2_D1_D1_pI1                   (func, sign, svmlfunc, native)  \
        OCL_SVML_P2_D1_D1_pI1_LOCAL         (func, sign)                    \
        OCL_SVML_P2_D1_D1_pI1_GLOBAL        (func, sign)                    \
    OCL_SVML_P2_D2_D2_pI2                   (func, sign, svmlfunc, native)  \
        OCL_SVML_P2_D2_D2_pI2_LOCAL         (func, sign)                    \
        OCL_SVML_P2_D2_D2_pI2_GLOBAL        (func, sign)                    \
    OCL_SVML_P2_D3_D3_pI3                   (func, sign, svmlfunc, native)  \
        OCL_SVML_P2_D3_D3_pI3_LOCAL         (func, sign)                    \
        OCL_SVML_P2_D3_D3_pI3_GLOBAL        (func, sign)                    \
    OCL_SVML_P2_D4_D4_pI4                   (func, sign, svmlfunc, native)  \
        OCL_SVML_P2_D4_D4_pI4_LOCAL         (func, sign)                    \
        OCL_SVML_P2_D4_D4_pI4_GLOBAL        (func, sign)                    \
    OCL_SVML_P2_D8_D8_pI8                   (func, sign, svmlfunc, native)  \
        OCL_SVML_P2_D8_D8_pI8_LOCAL         (func, sign)                    \
        OCL_SVML_P2_D8_D8_pI8_GLOBAL        (func, sign)                    \
    OCL_SVML_P2_D16_D16_pI16                (func, sign, svmlfunc, native)  \
        OCL_SVML_P2_D16_D16_pI16_LOCAL      (func, sign)                    \
        OCL_SVML_P2_D16_D16_pI16_GLOBAL     (func, sign)

// gentypef := f(gentypef, gentypef, gentypef)
#define OCL_SVML_P3_Fn_FnFnFn(func, svmlfunc, native)                       \
    OCL_SVML_P3_F1_F1_F1_F1                 (func, svmlfunc, native)        \
    OCL_SVML_P3_F2_F2_F2_F2                 (func, svmlfunc, native)        \
    OCL_SVML_P3_F3_F3_F3_F3                 (func, svmlfunc, native)        \
    OCL_SVML_P3_F4_F4_F4_F4                 (func, svmlfunc, native)        \
    OCL_SVML_P3_F8_F8_F8_F8                 (func, svmlfunc, native)        \
    OCL_SVML_P3_F16_F16_F16_F16             (func, svmlfunc, native)

// gentyped := f(gentyped, gentyped, gentyped)
#define OCL_SVML_P3_Dn_DnDnDn(func, svmlfunc, native)                       \
    OCL_SVML_P3_D1_D1_D1_D1                 (func, svmlfunc, native)        \
    OCL_SVML_P3_D2_D2_D2_D2                 (func, svmlfunc, native)        \
    OCL_SVML_P3_D3_D3_D3_D3                 (func, svmlfunc, native)        \
    OCL_SVML_P3_D4_D4_D4_D4                 (func, svmlfunc, native)        \
    OCL_SVML_P3_D8_D8_D8_D8                 (func, svmlfunc, native)        \
    OCL_SVML_P3_D16_D16_D16_D16             (func, svmlfunc, native)

// gentype := f(gentype, gentype, gentype)
#define OCL_SVML_P3_Gn_GnGnGn(func, svmlfunc, native)                       \
    OCL_SVML_P3_Fn_FnFnFn                   (func, svmlfunc, native)        \
    OCL_SVML_P3_Dn_DnDnDn                   (func, svmlfunc, native)

// gentypef := f(gentypef, gentypef, sgentypei/ugentypei*)
#define OCL_SVML_P3_Fn_FnFnpIn(func, sign, svmlfunc, native)                \
    OCL_SVML_P3_F1_F1_F1_pI1                (func, sign, svmlfunc, native)  \
        OCL_SVML_P3_F1_F1_F1_pI1_LOCAL      (func, sign)                    \
        OCL_SVML_P3_F1_F1_F1_pI1_GLOBAL     (func, sign)                    \
    OCL_SVML_P3_F2_F2_F2_pI2                (func, sign, svmlfunc, native)  \
        OCL_SVML_P3_F2_F2_F2_pI2_LOCAL      (func, sign)                    \
        OCL_SVML_P3_F2_F2_F2_pI2_GLOBAL     (func, sign)                    \
    OCL_SVML_P3_F3_F3_F3_pI3                (func, sign, svmlfunc, native)  \
        OCL_SVML_P3_F3_F3_F3_pI3_LOCAL      (func, sign)                    \
        OCL_SVML_P3_F3_F3_F3_pI3_GLOBAL     (func, sign)                    \
    OCL_SVML_P3_F4_F4_F4_pI4                (func, sign, svmlfunc, native)  \
        OCL_SVML_P3_F4_F4_F4_pI4_LOCAL      (func, sign)                    \
        OCL_SVML_P3_F4_F4_F4_pI4_GLOBAL     (func, sign)                    \
    OCL_SVML_P3_F8_F8_F8_pI8                (func, sign, svmlfunc, native)  \
        OCL_SVML_P3_F8_F8_F8_pI8_LOCAL      (func, sign)                    \
        OCL_SVML_P3_F8_F8_F8_pI8_GLOBAL     (func, sign)                    \
    OCL_SVML_P3_F16_F16_F16_pI16            (func, sign, svmlfunc, native)  \
        OCL_SVML_P3_F16_F16_F16_pI16_LOCAL  (func, sign)                    \
        OCL_SVML_P3_F16_F16_F16_pI16_GLOBAL (func, sign)

// gentyped := f(gentyped, gentyped, sgentypei/ugentypei*)
#define OCL_SVML_P3_Dn_DnDnpIn(func, sign, svmlfunc, native)                \
    OCL_SVML_P3_D1_D1_D1_pI1                (func, sign, svmlfunc, native)  \
        OCL_SVML_P3_D1_D1_D1_pI1_LOCAL      (func, sign)                    \
        OCL_SVML_P3_D1_D1_D1_pI1_GLOBAL     (func, sign)                    \
    OCL_SVML_P3_D2_D2_D2_pI2                (func, sign, svmlfunc, native)  \
        OCL_SVML_P3_D2_D2_D2_pI2_LOCAL      (func, sign)                    \
        OCL_SVML_P3_D2_D2_D2_pI2_GLOBAL     (func, sign)                    \
    OCL_SVML_P3_D3_D3_D3_pI3                (func, sign, svmlfunc, native)  \
        OCL_SVML_P3_D3_D3_D3_pI3_LOCAL      (func, sign)                    \
        OCL_SVML_P3_D3_D3_D3_pI3_GLOBAL     (func, sign)                    \
    OCL_SVML_P3_D4_D4_D4_pI4                (func, sign, svmlfunc, native)  \
        OCL_SVML_P3_D4_D4_D4_pI4_LOCAL      (func, sign)                    \
        OCL_SVML_P3_D4_D4_D4_pI4_GLOBAL     (func, sign)                    \
    OCL_SVML_P3_D8_D8_D8_pI8                (func, sign, svmlfunc, native)  \
        OCL_SVML_P3_D8_D8_D8_pI8_LOCAL      (func, sign)                    \
        OCL_SVML_P3_D8_D8_D8_pI8_GLOBAL     (func, sign)                    \
    OCL_SVML_P3_D16_D16_D16_pI16            (func, sign, svmlfunc, native)  \
        OCL_SVML_P3_D16_D16_D16_pI16_LOCAL  (func, sign)                    \
        OCL_SVML_P3_D16_D16_D16_pI16_GLOBAL (func, sign)

/// OpenCL Spec 1.2 (rev 15), Section 6.12.2, Table 6.8

OCL_SVML_P1_Gn_Gn       (acos,          acos,       )
OCL_SVML_P1_Gn_Gn       (acosh,         acosh,      )
OCL_SVML_P1_Gn_Gn       (acospi,        acospi,     )
OCL_SVML_P1_Gn_Gn       (asin,          asin,       )
OCL_SVML_P1_Gn_Gn       (asinh,         asinh,      )
OCL_SVML_P1_Gn_Gn       (asinpi,        asinpi,     )
OCL_SVML_P1_Gn_Gn       (atan,          atan,       )
OCL_SVML_P2_Gn_GnGn     (atan2,         atan2,      )
OCL_SVML_P1_Gn_Gn       (atanh,         atanh,      )
OCL_SVML_P1_Gn_Gn       (atanpi,        atanpi,     )
OCL_SVML_P2_Gn_GnGn     (atan2pi,       atan2pi,    )
OCL_SVML_P1_Gn_Gn       (cbrt,          cbrt,       )
OCL_SVML_P1_Gn_Gn       (ceil,          ceil,       )
OCL_SVML_P2_Gn_GnGn     (copysign,      copysign,   )
OCL_SVML_P1_Gn_Gn       (cos,           cos,        )
OCL_SVML_P1_Gn_Gn       (cosh,          cosh,       )
OCL_SVML_P1_Gn_Gn       (cospi,         cospi,      )
OCL_SVML_P1_Gn_Gn       (erfc,          erfc,       )
OCL_SVML_P1_Gn_Gn       (erf,           erf,        )
OCL_SVML_P1_Gn_Gn       (exp,           exp,        )
OCL_SVML_P1_Gn_Gn       (exp2,          exp2,       )
OCL_SVML_P1_Gn_Gn       (exp10,         exp10,      )
OCL_SVML_P1_Gn_Gn       (expm1,         expm1,      )
OCL_SVML_P1_Gn_Gn       (fabs,          fabs,       )
OCL_SVML_P1_Gn_Gn       (fdim,          fdim,       )
OCL_SVML_P1_Gn_Gn       (floor,         floor,      )
OCL_SVML_P3_Gn_GnGnGn   (fma,           fma,        )
OCL_SVML_P2_Gn_GnGn     (fmax,          fmax,       )
    OCL_FUNC_P2_Fn_FnF1 (fmax)
    OCL_FUNC_P2_Dn_DnD1 (fmax)
OCL_SVML_P2_Gn_GnGn     (fmin,          fmin,       )
    OCL_FUNC_P2_Fn_FnF1 (fmin)
    OCL_FUNC_P2_Dn_DnD1 (fmin)
OCL_SVML_P2_Gn_GnGn     (fmod,          fmod,       )
OCL_SVML_P2_Gn_GnpGn    (fract,         fract,      )
OCL_SVML_P2_Fn_FnpIn    (frexp, i,      frexp,      )
OCL_SVML_P2_Dn_DnpIn    (frexp, i,      frexp,      )
OCL_SVML_P2_Gn_GnGn     (hypot,         hypot,      )
OCL_SVML_P1_In_Fn       (ilogb, i,      ilogb,      )
OCL_SVML_P1_In_Dn       (ilogb, i,      ilogb,      )
OCL_SVML_P2_Fn_FnIn     (ldexp, i,      ldexp,      )
    OCL_FUNC_P2_Fn_FnI1 (ldexp, i)
OCL_SVML_P2_Dn_DnIn     (ldexp, i,      ldexp,      )
    OCL_FUNC_P2_Dn_DnI1 (ldexp, i)
OCL_SVML_P1_Gn_Gn       (lgamma,        lgamma,     )
OCL_SVML_P2_Fn_FnpIn    (lgamma_r, i,   lgammar,    )
OCL_SVML_P2_Dn_DnpIn    (lgamma_r, i,   lgammar,    )
OCL_SVML_P1_Gn_Gn       (log,           log,        )
OCL_SVML_P1_Gn_Gn       (log2,          log2,       )
OCL_SVML_P1_Gn_Gn       (log10,         log10,      )
OCL_SVML_P1_Gn_Gn       (log1p,         log1p,      )
OCL_SVML_P1_Gn_Gn       (logb,          logb,       )
OCL_SVML_P3_Gn_GnGnGn   (mad,           mad,        )
OCL_SVML_P2_Gn_GnGn     (maxmag,        maxmag,     )
OCL_SVML_P2_Gn_GnGn     (minmag,        minmag,     )
OCL_SVML_P2_Gn_GnpGn    (modf,          modf,       )
OCL_SVML_P1_Fn_In       (nan, u,        nan,        )
OCL_SVML_P1_Dn_Ln       (nan, u,        nan,        )
OCL_SVML_P2_Gn_GnGn     (nextafter,     nextafter,  )
OCL_SVML_P2_Gn_GnGn     (pow,           pow,        )
OCL_SVML_P2_Fn_FnIn     (pown, i,       pown,       )
OCL_SVML_P2_Dn_DnIn     (pown, i,       pown,       )
OCL_SVML_P2_Gn_GnGn     (powr,          powr,       )
OCL_SVML_P2_Gn_GnGn     (remainder,     remainder,  )
OCL_SVML_P3_Fn_FnFnpIn  (remquo, i,     remquo,     )
OCL_SVML_P3_Dn_DnDnpIn  (remquo, i,     remquo,     )
OCL_SVML_P1_Gn_Gn       (rint,          rint,       )
OCL_SVML_P2_Fn_FnIn     (rootn, i,      rootn,      )
OCL_SVML_P2_Dn_DnIn     (rootn, i,      rootn,      )
OCL_SVML_P1_Gn_Gn       (round,         round,      )
OCL_SVML_P1_Gn_Gn       (rsqrt,         rsqrt,      )
OCL_SVML_P1_Gn_Gn       (sin,           sin,        )
OCL_SVML_P2_Gn_GnpGn    (sincos,        sincos,     )
OCL_SVML_P1_Gn_Gn       (sinh,          sinh,       )
OCL_SVML_P1_Gn_Gn       (sinpi,         sinpi,      )
OCL_SVML_P1_Gn_Gn       (sqrt,          sqrt,       )
OCL_SVML_P1_Gn_Gn       (tan,           tan,        )
OCL_SVML_P1_Gn_Gn       (tanh,          tanh,       )
OCL_SVML_P1_Gn_Gn       (tanpi,         tanpi,      )
OCL_SVML_P1_Gn_Gn       (tgamma,        tgamma,     )
OCL_SVML_P1_Gn_Gn       (trunc,         trunc,      )

/// OpenCL Spec 1.2 (rev 15), Section 6.12.2, Table 6.9
/// + double extension on native functions
OCL_SVML_P1_Fn_Fn       (half_cos,      cos,    _native)
OCL_SVML_P2_Fn_FnFn     (half_divide,   div,    _native)
OCL_SVML_P1_Fn_Fn       (half_exp,      exp,    _native)
OCL_SVML_P1_Fn_Fn       (half_exp2,     exp2,   _native)
OCL_SVML_P1_Fn_Fn       (half_exp10,    exp10,  _native)
OCL_SVML_P1_Fn_Fn       (half_log,      log,    _native)
OCL_SVML_P1_Fn_Fn       (half_log2,     log2,   _native)
OCL_SVML_P1_Fn_Fn       (half_log10,    log10,  _native)
OCL_SVML_P1_Fn_Fn       (half_powr,     powr,   _native)
OCL_INTR_P1_Fn_Fn       (half_recip)
OCL_SVML_P1_Fn_Fn       (half_rsqrt,    rsqrt,  _native)
OCL_SVML_P1_Fn_Fn       (half_sin,      sin,    _native)
OCL_SVML_P1_Fn_Fn       (half_sqrt,     sqrt,   _native)
OCL_SVML_P1_Fn_Fn       (half_tan,      tan,    _native)
OCL_SVML_P1_Gn_Gn       (native_cos,    cos,    _native)
OCL_SVML_P2_Gn_GnGn     (native_divide, div,    _native)
OCL_SVML_P1_Gn_Gn       (native_exp,    exp,    _native)
OCL_SVML_P1_Gn_Gn       (native_exp2,   exp2,   _native)
OCL_SVML_P1_Gn_Gn       (native_exp10,  exp10,  _native)
OCL_SVML_P1_Gn_Gn       (native_log,    log,    _native)
OCL_SVML_P1_Gn_Gn       (native_log2,   log2,   _native)
OCL_SVML_P1_Gn_Gn       (native_log10,  log10,  _native)
OCL_SVML_P1_Gn_Gn       (native_powr,   powr,   _native)
OCL_INTR_P1_Gn_Gn       (native_recip)
OCL_SVML_P1_Gn_Gn       (native_rsqrt,  rsqrt,  _native)
OCL_SVML_P1_Gn_Gn       (native_sin,    sin,    _native)
OCL_SVML_P1_Gn_Gn       (native_sqrt,   sqrt,   _native)
OCL_SVML_P1_Gn_Gn       (native_tan,    tan,    _native)

float16 __attribute__((overloadable)) half_recip(float16 x)
{
    return _mm512_recip_ps(x);
}
float16 __attribute__((overloadable)) mask_half_recip(ushort m16, float16 x)
{
    return _mm512_mask_recip_ps(x, m16, x);
}

float16 __attribute__((overloadable)) native_recip(float16 x)
{
    return _mm512_recip_ps(x);
}
float16 __attribute__((overloadable)) mask_native_recip(ushort m16, float16 x)
{
    return _mm512_mask_recip_ps(x, m16, x);
}

double8 __attribute__((overloadable)) native_recip(double8 x)
{
    return _mm512_recip_pd(x);
}
double8 __attribute__((overloadable)) mask_native_recip(uchar m8, double8 x)
{
    return _mm512_mask_recip_pd(x, m8, x);
}

#ifdef __cplusplus
}
#endif

#endif // defined (__MIC__) || defined(__MIC2__)
