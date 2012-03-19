// Copyright (c) 2006-2007 Intel Corporation
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

#if !defined (__MIC__) && !defined(__MIC2__)
#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#if defined(_MSC_VER)
#include "stdafx.h"
#endif
#include <emmintrin.h>
#if !defined(_mm_floor_ps) && defined(__SSE4_1__)
#include <smmintrin.h>
#endif
#else
// Enable double support. It is needed for declarations from intrin.h
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#include <intrin.h>
#endif

#include "cl_image_declaration.h"

#include "ll_intrinsics.h"

#define SHRT16_MIN    (-32768)
#define SHRT16_MAX      32767

// Clamp border color used for CL_A, CL_INTENSITY, CL_Rx, CL_RA, CL_RGx, CL_RGBx, CL_ARGB, CL_BGRA, CL_RGBA
ALIGN16 const float4 BorderColorNoAlphaFloat = {0.0f, 0.0f, 0.0f, 0.0f}; 
ALIGN16 const int4 BorderColorNoAlphaInt = {0, 0, 0, 0}; 
ALIGN16 const uint4 BorderColorNoAlphaUint = {0, 0, 0, 0}; 
ALIGN16 const float4 halfhalfhalfzero = {0.5f, 0.5f, 0.5f, 0.0f};
ALIGN16 const float4 f4half = {0.5f, 0.5f, 0.5f, 0.5f};
ALIGN16 const float4 f4two = {2.f, 2.f, 2.f, 2.f};
/// Minimal representative float. It is represented as zero mantissa 
/// and exponenta with only last bit set to one
ALIGN16 const int4 oneOneOneZero = {1, 1, 1, 0};
ALIGN16 const int4 ZeroInt = {0, 0, 0, 0};
ALIGN16 const float4 float4Allzeros = {0.f ,0.f , 0.f, 0.f};
ALIGN16 const float4 float4Allones = {1.f ,1.f , 1.f, 1.f};
ALIGN16 const float4 f4Unorm8Dim = {(float)(1./255.), (float)(1./255.), (float)(1./255.), (float)(1./255.)};
ALIGN16 const float4 f4Unorm16Dim = {(float)(1./65535.), (float)(1./65535.), (float)(1./65535.), (float)(1./65535.)};
ALIGN16 const float4 f4unorm16mul = {65535.f, 65535.f, 65535.f, 65535.f};
ALIGN16 const float4 f4unorm16lim = {0.f, 0.f, 0.f, 0.f};
ALIGN16 const float4 f4unorm8mul = {255.f, 255.f, 255.f, 255.f};
ALIGN16 const float4 f4unorm8lim = {0.0f, 0.0f, 0.0f, 0.0f};
ALIGN16 const int4 i4int16Min = {SHRT16_MIN, SHRT16_MIN, SHRT16_MIN, SHRT16_MIN};
ALIGN16 const int4 i4int16Max = {SHRT16_MAX, SHRT16_MAX, SHRT16_MAX, SHRT16_MAX};
ALIGN16 const int4 i4uint16Max = {USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX};

// Clamp Border color used for CL_R, CL_RG, CL_RGB, CL_LUMINANCE
ALIGN16 const float4 BorderColorAlphaFloat = {0.0f, 0.0f, 0.0f, 1.0f}; 
ALIGN16 const int4 BorderColorAlphaInt = {0, 0, 0, 1}; 

ALIGN16 const uint4 BorderColorAlphaUint = {0, 0, 0, 1}; 
ALIGN16 const float f4SignMask[] = {-0.f, -0.f, -0.f, -0.f};

ALIGN16 const int4 UndefCoordInt={-1,-1, -1, -1};
ALIGN16 const float4 ZeroFloat={0.0f,0.0f,0.0f,0.0f};

// utility functions declarations
int __attribute__((overloadable)) isOutOfBoundsInt(image2d_t image, int4 coord);
int4 __attribute__((overloadable)) ProjectToEdgeInt(image2d_t image, int4 coord);
float4 __attribute__((overloadable)) Unnormalize(image2d_t image,float4 coord);
int4 __attribute__((overloadable)) ProjectNearest(float4 coord);
float4 __attribute__((overloadable)) frac(float4 coord);

void* __attribute__((overloadable)) extract_pixel_pointer_quad(image2d_t image, int4 coord, void* pData);

uint4 __attribute__((overloadable)) load_pixel_RGBA_UINT8(void* pPixel);
uint4 __attribute__((overloadable)) load_pixel_RGBA_UINT16(void* pPixel);
uint4 __attribute__((overloadable)) load_pixel_RGBA_UINT32(void* pPixel);

int4 __attribute__((overloadable)) load_pixel_RGBA_INT8(void* pPixel);
int4 __attribute__((overloadable)) load_pixel_RGBA_INT16(void* pPixel);
int4 __attribute__((overloadable)) load_pixel_RGBA_INT32(void* pPixel);

float __attribute__((overloadable)) load_value_INTENSITY_FLOAT(void* pPixel);
float __attribute__((overloadable)) load_value_INTENSITY_UNORM_INT8(void* pPixel);
float __attribute__((overloadable)) load_value_INTENSITY_UNORM_INT16(void* pPixel);
float __attribute__((overloadable)) load_value_INTENSITY_HALF_FLOAT(void* pPixel);
float __attribute__((overloadable)) load_value_LUMINANCE_FLOAT(void* pPixel);
float __attribute__((overloadable)) load_value_LUMINANCE_UNORM_INT8(void* pPixel);
float __attribute__((overloadable)) load_value_LUMINANCE_UNORM_INT16(void* pPixel);
float __attribute__((overloadable)) load_value_LUMINANCE_HALF_FLOAT(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_INTENSITY_FLOAT(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_INTENSITY_UNORM_INT8(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_INTENSITY_UNORM_INT16(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_INTENSITY_HALF_FLOAT(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_LUMINANCE_FLOAT(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_LUMINANCE_UNORM_INT8(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_LUMINANCE_UNORM_INT16(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_LUMINANCE_HALF_FLOAT(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_RGBA_HALF_FLOAT(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_RGBA_FLOAT(void* pPixel);

float4 __attribute__((overloadable)) load_pixel_BGRA_UNORM_INT8(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_RGBA_UNORM_INT8(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_RGBA_UNORM_INT16(void* pPixel);

int4 __attribute__((overloadable)) load_pixel_R_INT8(void* pPixel);
int4 __attribute__((overloadable)) load_pixel_R_INT16(void* pPixel);
int4 __attribute__((overloadable)) load_pixel_R_INT32(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_R_FLOAT(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_R_HALF_FLOAT(void* pPixel);
uint4 __attribute__((overloadable)) load_pixel_R_UINT8(void* pPixel);
uint4 __attribute__((overloadable)) load_pixel_R_UINT16(void* pPixel);
uint4 __attribute__((overloadable)) load_pixel_R_UINT32(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_R_UNORM_INT8(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_R_UNORM_INT16(void* pPixel);

float4 __attribute__((overloadable)) load_pixel_A_FLOAT(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_A_UNORM_INT8(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_A_UNORM_INT16(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_A_HALF_FLOAT(void* pPixel);

uint4 __attribute__((overloadable)) load_pixel_RG_UINT8(void* pPixel);
uint4 __attribute__((overloadable)) load_pixel_RG_UINT16(void* pPixel);
uint4 __attribute__((overloadable)) load_pixel_RG_UINT32(void* pPixel);
int4 __attribute__((overloadable)) load_pixel_RG_INT8(void* pPixel);
int4 __attribute__((overloadable)) load_pixel_RG_INT16(void* pPixel);
int4 __attribute__((overloadable)) load_pixel_RG_INT32(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_RG_FLOAT(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_RG_UNORM_INT8(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_RG_UNORM_INT16(void* pPixel);
float4 __attribute__((overloadable)) load_pixel_RG_HALF_FLOAT(void* pPixel);

float4 SampleImage1DFloat(float4 Ti0, float4 Ti1, float4 frac);

float4 SampleImage2DFloat(float4 Ti0j0, float4 Ti1j0, float4 Ti0j1, float4 Ti1j1, float4 frac);
float4 SampleImage2DFloatCh1(float4 components, float4 frac);

float4 SampleImage3DFloat(float4 Ti0j0k0, float4 Ti1j0k0, float4 Ti0j1k0, float4 Ti1j1k0, float4 Ti0j0k1, float4 Ti1j0k1, float4 Ti0j1k1, float4 Ti1j1k1, float4 frac);

#define _mm_abs_ps(X)    _mm_andnot_ps(_mm_load_ps(f4SignMask),X)

ALIGN16 const short  Fvec8Float16ExponentMask[] = {0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00};
ALIGN16 const short  Fvec8Float16MantissaMask[] = {0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF};
ALIGN16 const short  Fvec8Float16SignMask[]     = {0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000};
ALIGN16 const int Fvec4Float32ExponentMask[] = {0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000};
ALIGN16 const int Fvec4Float32NanMask[] = {0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000};
ALIGN16 const int Fvec4Float16NaNExpMask[]   = {0x7C00, 0x7C00, 0x7C00, 0x7C00};
ALIGN16 const int FVec4Float16Implicit1Mask[] = {(1<<10), (1<<10), (1<<10), (1<<10)};
ALIGN16 const int Fvec4Float16ExpMin[] = {(1<<10), (1<<10), (1<<10), (1<<10)};
ALIGN16 const int Fvec4Float16BiasDiffDenorm[] = {((127 - 15 - 10) << 23), ((127 - 15 - 10) << 23), ((127 - 15 - 10) << 23), ((127 - 15 - 10) << 23)};
ALIGN16 const int Fvec4Float16ExpBiasDifference[] = {((127 - 15) << 10), ((127 - 15) << 10), ((127 - 15) << 10), ((127 - 15) << 10)};
ALIGN16 const int f4minNorm[] = {0x00800000, 0x00800000, 0x00800000, 0x00800000};
ALIGN16 const int mth_signMask[] = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};

/// Currently we can't get rid of built-in module loading if built-in functions are used.
/// So built-ins implementation is copy-pasted to images module. In future 
/// This hack should be removed and limited set of built-ins that is required for
/// Images module will be linked.
////////////// built-ins implementation

ALIGN16 const int x8000[] = {0x8000, 0x8000, 0x8000, 0x8000};
ALIGN16 const int x7fff[] = {0x7fff, 0x7fff, 0x7fff, 0x7fff};
ALIGN16 const int x0200[] = {0x0200, 0x0200, 0x0200, 0x0200};
ALIGN16 const int x7c00[] = {0x7c00, 0x7c00, 0x7c00, 0x7c00};
ALIGN16 const int x7fffffff[] = {0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff};
ALIGN16 const int x477ff000[] = {0x477ff000, 0x477ff000, 0x477ff000, 0x477ff000};
ALIGN16 const int x33000000[] = {0x33000000, 0x33000000, 0x33000000, 0x33000000};
ALIGN16 const int x33c00000[] = {0x33c00000, 0x33c00000, 0x33c00000, 0x33c00000};
ALIGN16 const int x01000000[] = {0x01000000, 0x01000000, 0x01000000, 0x01000000};
ALIGN16 const int x46000000[] = {0x46000000, 0x46000000, 0x46000000, 0x46000000};
ALIGN16 const int x07800000[] = {0x07800000, 0x07800000, 0x07800000, 0x07800000};
ALIGN16 const int x7f800000[] = {0x7f800000, 0x7f800000, 0x7f800000, 0x7f800000};
ALIGN16 const int x38800000[] = {0x38800000, 0x38800000, 0x38800000, 0x38800000};
ALIGN16 const int ones[] = {1, 1, 1, 1};
ALIGN16 const char _4x32to4x16[] = {0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1};

float4 float2half_rte(float4 param)
{
    //cl_uint sign = (u.u >> 16) & 0x8000;
    _8i16 temp = (_8i16)_mm_srli_epi32((__m128i)param, 0x10);
    _8i16 signs = (_8i16)_mm_and_si128((__m128i)temp,(__m128i) *((_4i32 *)x8000));
    float4 absParam = (float4)_mm_and_si128((__m128i)param,(__m128i) *((_4i32 *)x7fffffff));

    //Nan
    //if( x != x ) 
    _8i16 eq0 = (_8i16)_mm_cmpneq_ps(absParam, absParam);
    _8i16 eq = (_8i16)_mm_and_si128((__m128i)absParam,(__m128i) eq0);
    //u.u >>= (24-11);
    eq = (_8i16) _mm_srli_epi32((__m128i)eq, 0x0d);
    //u.u &= 0x7fff;
    eq = (_8i16) _mm_and_si128((__m128i)eq,(__m128i) *((_4i32 *)x7fff));
    //u.u |= 0x0200;   -- silence the NaN
    eq = (_8i16) _mm_or_si128((__m128i)eq,(__m128i) *((_4i32 *)x0200));
    //return u.u | sign;
    eq = (_8i16) _mm_or_si128((__m128i)eq,(__m128i) signs);
    eq = (_8i16) _mm_and_si128((__m128i)eq,(__m128i) eq0);
    _8i16 dflt = eq0;

    // overflow
    //if( x >= MAKE_HEX_FLOAT(0x1.ffcp15f, 0x1ffcL, 3) )
    //return 0x7c00 | sign;

    float4 eq1 = _mm_cmpge_ps(absParam, *((float4 *)x477ff000));
    eq0 = (_8i16) _mm_and_si128((__m128i)eq1,(__m128i) *((_4i32 *)x7c00));
    eq0 = (_8i16) _mm_or_si128((__m128i)signs,(__m128i) eq0);
    eq0 = (_8i16) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
    eq = (_8i16) _mm_or_si128((__m128i)eq,(__m128i) eq0);
    dflt = (_8i16) _mm_or_si128((__m128i)eq1,(__m128i) dflt);

    // underflow
    //	if( x <= MAKE_HEX_FLOAT(0x1.0p-25f, 0x1L, -25) )
    // return sign
    eq1 = _mm_cmple_ps(absParam, *((float4 *)x33000000));
    eq0 = (_8i16) _mm_and_si128((__m128i)eq1,(__m128i) signs);
    eq0 = (_8i16) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
    eq = (_8i16) _mm_or_si128((__m128i)eq,(__m128i) eq0);
    dflt = (_8i16) _mm_or_si128((__m128i)eq1,(__m128i) dflt);


    // very small
    //	if( x < MAKE_HEX_FLOAT(0x1.8p-24f, 0x18L, -28) )
    // return sign | 1;
    eq1 = _mm_cmplt_ps(absParam, *((float4 *)x33c00000));
    eq0 = (_8i16) _mm_and_si128((__m128i)eq1, _mm_or_si128((__m128i)signs,(__m128i) *((_4i32 *)ones)));
    eq0 = (_8i16) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
    eq = (_8i16) _mm_or_si128((__m128i)eq,(__m128i) eq0);
    dflt = (_8i16) _mm_or_si128((__m128i)eq1,(__m128i) dflt);

    // half denormal         
    //  if( x < MAKE_HEX_FLOAT(0x1.0p-14f, 0x1L, -14) )
    //	x *= MAKE_HEX_FLOAT(0x1.0p-125f, 0x1L, -125);
    //  return sign | x;
    eq1 = _mm_cmplt_ps(absParam, *((float4 *)x38800000));
    float4 eq2 = _mm_mul_ps(absParam, *((float4 *)x01000000));  //x
    eq0 = (_8i16) _mm_and_si128((__m128i)eq1, _mm_or_si128((__m128i)signs,(__m128i) (_8i16)eq2));
    eq0 = (_8i16) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
    eq = (_8i16) _mm_or_si128((__m128i)eq,(__m128i) eq0);
    dflt = (_8i16) _mm_or_si128((__m128i)eq1,(__m128i) dflt);

    // u.f *= MAKE_HEX_FLOAT(0x1.0p13f, 0x1L, 13);
    // u.u &= 0x7f800000;
    // x += u.f;
    // u.f = x - u.f;
    // u.f *= MAKE_HEX_FLOAT(0x1.0p-112f, 0x1L, -112);
    // return (u.u >> (24-11)) | sign;

    eq1 = _mm_mul_ps(param, *((float4 *)x46000000));  
    eq0 = (_8i16) _mm_and_si128((__m128i)eq1,(__m128i) *((_4i32 *)x7f800000)); //u
    eq1 = _mm_add_ps( (float4)eq0 ,absParam); //x
    eq1 = _mm_sub_ps(eq1, (float4)eq0); //u
    eq1 = _mm_mul_ps(eq1, *((float4 *)x07800000));  
    eq0 = (_8i16) _mm_srli_epi32((__m128i)eq1, 0x0d);
    eq0 = (_8i16) _mm_or_si128((__m128i)eq0,(__m128i) signs);
    eq0 = (_8i16) _mm_andnot_si128((__m128i)dflt,(__m128i) eq0);
    eq = (_8i16) _mm_or_si128((__m128i)eq,(__m128i) eq0);

    eq1 = _mm_castsi128_ps(_mm_shuffle_epi8((__m128i)eq, *((__m128i *)_4x32to4x16)));

    return eq1;
}

float4 float2half(float4 param)
{
    return float2half_rte(param);
}

void __attribute__((overloadable)) vstore_half4(float4 data, size_t offset, half* ptr)
{
    ptr = ptr + (offset*4);
    data = float2half(data);
    *((float2 *)ptr) = data.lo;
}

void __attribute__((overloadable)) vstore_half2(float2 data_val, size_t offset, half* ptr)
{
    float4 data = {0.f, 0.f, 0.f, 0.f};
    data.lo = data_val;
    ptr = ptr + (offset*4);
    data = float2half(data);
    /// Use short as it has the same size as half
    *((short2 *)ptr) = ((short4)data.lo).lo;
}

void __attribute__((overloadable)) vstore_half(float data_val, size_t offset, half* ptr)
{
    float4 data = {0.f, 0.f, 0.f, 0.f};
    data.x = data_val;
    ptr = ptr + (offset*4);
    data = float2half(data);
    /// Use short as it has the same size as half
    *((short *)ptr) = ((short4)data.lo).x;
}

float4  __attribute__((overloadable)) floor(float4 x)
{
    return _mm_floor_ps(x);
}

int4 __attribute__((overloadable)) convert_int4(float4 x)
{
    return (int4)_mm_cvtps_epi32((__m128)x);
}

float4  __attribute__((overloadable)) rint(float4 x)
{
    return _mm_round_ps((__m128)x, 0);
}

float4  __attribute__((overloadable)) fabs(float4 p)
{
    p = _mm_and_ps(p, *(__m128*)mth_signMask);
    return p;
}


#ifdef __SSE4_1__
// Auxiliary routines
__m128i cvt_to_norm(__m128i i4Val, __m128 f4Mul, __m128 lowLimit)
{
    __m128 f4Val = _mm_castsi128_ps(i4Val);
    f4Val = _mm_mul_ps(f4Val, f4Mul);
    // Apply limits, the upper limit is always the devider
    f4Val = _mm_min_ps(f4Val, f4Mul);
    f4Val = _mm_max_ps(f4Val, lowLimit);

    return _mm_cvtps_epi32(f4Val);
}

int4    __attribute__((overloadable)) min(int4 x, int4 y)
{
    return (int4) _mm_min_epi32((__m128i)x ,(__m128i) y);  
}

int4    __attribute__((overloadable)) max(int4 x, int4 y)
{
    return (int4) _mm_max_epi32((__m128i)x ,(__m128i) y);
}

uint4   __attribute__((overloadable)) min(uint4 x, uint4 y)
{
    return (uint4) _mm_min_epu32((__m128i)x, (__m128i) y);
}

uint4   __attribute__((overloadable)) max(uint4 x, uint4 y)
{
    return (uint4) _mm_max_epu32((__m128i)x, (__m128i) y);
}

float4 Half4ToFloat4(_8i16 xmm0)
{
    //_4i32 _mm_setzero_si128() = (_4i32)_mm_setzero_si128();
    _4i32 xmm1 = (_4i32)_mm_and_si128((__m128i)xmm0,(__m128i) *((_4i32*)Fvec8Float16ExponentMask));
    xmm1 = (_4i32) _mm_unpacklo_epi16((__m128i)xmm1, (__m128i)_mm_setzero_si128()); // xmm1 = exponents as DWORDS
    _4i32 xmm2 = (_4i32)_mm_and_si128((__m128i)xmm0,(__m128i) *((_4i32*)Fvec8Float16MantissaMask));
    xmm2 = (_4i32) _mm_unpacklo_epi16((__m128i)xmm2, (__m128i)_mm_setzero_si128()); // xmm2 = mantissas as DWORDS
    xmm0 = (_8i16) _mm_and_si128((__m128i)xmm0,(__m128i) *((_4i32 * )Fvec8Float16SignMask)); 
    _4i32 xmm6 = (_4i32)_mm_unpacklo_epi16((__m128i)_mm_setzero_si128(), (__m128i)xmm0); // xmm6 = sign mask as DWORDS

    // We need to handle the case where the number is NaN or INF
    // If the float16 is one of these, then we create an all '1' exponent for the 32bit float and store it in xmm6 for later use
    xmm0 = (_8i16) _mm_cmpeq_epi32((__m128i)xmm1,(__m128i) *((_4i32 *)Fvec4Float16NaNExpMask)); // xmm0.any dword = 0xFFFFFFFF if exponent is all '1'
    _4i32 xmm4 = (_4i32)_mm_cmpgt_epi32((__m128i)xmm2, (__m128i)_mm_setzero_si128()); // xmm4.any dword = 0xFFFFFFFF if mantissa > 0
    xmm4 = (_4i32) _mm_and_si128((__m128i)xmm4,(__m128i) xmm0); // xmm4.any dword = 0xFFFFFFFF if NAN
    xmm4 = (_4i32) _mm_and_si128((__m128i)xmm4,(__m128i) *((_4i32 * )Fvec4Float32NanMask)); // silence the SNaNs
    xmm0 = (_8i16) _mm_and_si128((__m128i)xmm0, (__m128i)*((_4i32 * )Fvec4Float32ExponentMask)); // // xmm0 = If float16 has all '1' exp, then convert to 32 bit float all '1' exp, otherwise 0
    xmm0 = (_8i16) _mm_or_si128((__m128i)xmm0,(__m128i) xmm4); 

    _4i32 xmm3 = (_4i32)_mm_cmpeq_epi32((__m128i)xmm1, (__m128i)_mm_setzero_si128()); // xmm3.any dword = 0xFFFFFFFF if exp is zero
    int normals = _mm_movemask_epi8((__m128i)xmm3);
    if(normals == 0)  
    {
        xmm1 = (_4i32) _mm_add_epi32((__m128i)xmm1,(__m128i) *((_4i32 * )Fvec4Float16ExpBiasDifference));// xmm1 = exponents converted to float32 bias
        xmm1 = (_4i32) _mm_or_si128((__m128i)xmm1,(__m128i) xmm2); // xmm1 = exponent + mantissa
        xmm1 = (_4i32) _mm_slli_epi32((__m128i)xmm1, 13);
        xmm1 = (_4i32) _mm_or_si128((__m128i)xmm1,(__m128i) xmm6);  // xmm1 = signed number
        float4 res = (float4)_mm_or_si128((__m128i)xmm1,(__m128i) xmm0);
        return res;  // If the original number was NaN or INF, then xmm6 has all '1' for exp. xmm0 will hold a float32 that is NaN or INF 
    }

    xmm3 = (_4i32) _mm_andnot_si128((__m128i)xmm3,(__m128i) *((_4i32 * )FVec4Float16Implicit1Mask));
    xmm2 = (_4i32) _mm_or_si128((__m128i)xmm2,(__m128i) xmm3); // add implicit 1 to the mantissa
    xmm1 = (_4i32) _mm_max_epi16((__m128i)xmm1, (__m128i)*((_4i32 * )Fvec4Float16ExpMin)); // xmm1 = max(exp, 1)

    // we can do the comparison on words since we know that the high word of each dword is 0
    xmm1 = (_4i32) _mm_slli_epi32((__m128i)xmm1, 13);
    xmm1 = (_4i32) _mm_add_epi32((__m128i)xmm1,(__m128i) *((_4i32 * )Fvec4Float16BiasDiffDenorm));  // add the bias difference to xmm1
    xmm1 = (_4i32) _mm_or_si128((__m128i)xmm1,(__m128i) xmm6); // Combine with the sign mask

    float4 xmm5 = _mm_cvtepi32_ps((__m128i)xmm2);  // Convert mantissa to float
    xmm5 = _mm_mul_ps(xmm5, _mm_castsi128_ps((__m128i)xmm1));

    xmm5 = _mm_or_ps((__m128)xmm5, _mm_castsi128_ps((__m128i)xmm0));// If the original number was NaN or INF, then xmm6 has all '1' for exp. xmm0 will hold a float32 that is NaN or INF 
    return xmm5;
}

#endif

/************************Float coordinate translations*************************************/

int4 __attribute__((overloadable)) trans_coord_float_NONE_FALSE_NEAREST(image2d_t image, float4 coord)
{
    //not testing if coords are OOB - this mode doesn't guarantee safeness!
    return ProjectNearest(coord);
}

int4 __attribute__((overloadable)) trans_coord_float_CLAMPTOEDGE_FALSE_NEAREST(image2d_t image, float4 coord)
{
    return ProjectToEdgeInt(image,ProjectNearest(coord));
}

int4 __attribute__((overloadable)) trans_coord_float_UNDEFINED(image2d_t image, float4 coord)
{
    return UndefCoordInt;   //will be background color, but it's a "don't care" situtation
}

int4 __attribute__((overloadable)) trans_coord_float_NONE_TRUE_NEAREST(image2d_t image, float4 coord)
{
    //not testing if coords are OOB - this mode doesn't guarantee safeness!
    int4 result=ProjectNearest(Unnormalize(image, coord));
    return result;
}

int4 __attribute__((overloadable)) trans_coord_float_CLAMPTOEDGE_TRUE_NEAREST(image2d_t image, float4 coord)
{
    int4 result=ProjectToEdgeInt(image, ProjectNearest(Unnormalize(image, coord)));
    return result;
}

int4 __attribute__((overloadable)) trans_coord_float_REPEAT_TRUE_NEAREST(image2d_t image, float4 coord)
{
#ifdef __SSE4_1__
    int4 upper = (int4)_mm_load_si128((__m128i*)(&((image_aux_data*)image)->dimSub1));
#else
    int4 upper=(int4)(0,0,0,0);
#endif
    int4 urcoord = ProjectNearest(Unnormalize(image, coord-floor(coord)));  //unrepeated coords
    
#ifdef __SSE4_1__
    __m128i mask = _mm_cmpgt_epi32((__m128i)urcoord, (__m128i)upper);
    urcoord = (int4)_mm_andnot_si128(mask, (__m128i)urcoord);
#else
    urcoord = urcoord % upper;
#endif
    return urcoord;
}

int4 __attribute__((overloadable)) trans_coord_float_MIRRORED_TRUE_NEAREST(image2d_t image, float4 coord)
{
#ifdef __SSE4_1__
    int4 upper = (int4)_mm_load_si128((__m128i*)(&((image_aux_data*)image)->dimSub1));
    __m128 isZero = _mm_cmpeq_ps((__m128)coord, float4Allzeros);
    __m128 mcoord = (float4)_mm_sub_epi32((__m128i)coord, *((__m128i*)f4minNorm));
    mcoord= _mm_round_ps((__m128)mcoord, _MM_ROUND_NEAREST);
    mcoord = (__m128)_mm_add_epi32((__m128i)mcoord, *((__m128i*)f4minNorm));
    /// Set to zero coordinates that were equal to zero before
    /// multiplications
    mcoord = (__m128)_mm_andnot_si128((__m128i)isZero, (__m128i)mcoord);
    mcoord = (__m128)_mm_sub_ps((__m128)mcoord, (__m128)coord);
#else
    int4 upper = vload4(0,(((image_aux_data*)image)->dimSub1));
    float4 mcoord=2.0f*rint(0.5f*coord);
    mcoord=coord-mcoord;
#endif
    mcoord=fabs(mcoord);
    int4 urcoord = ProjectNearest(Unnormalize(image, (float4)mcoord));  //unrepeated coords
    urcoord=min(urcoord,upper);
    return urcoord;
}


/***********************float to float images translation functions (which accept and return [square0, square1] coordinates******************/

float4 __attribute__((overloadable)) trans_coord_float_float_NONE_FALSE_NEAREST(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    //not testing if coords are OOB - this mode doesn't guarantee safeness!
    *square0=ProjectNearest(coord);
    return ZeroFloat;
}

float4 __attribute__((overloadable)) trans_coord_float_float_CLAMPTOEDGE_FALSE_NEAREST(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    *square0=ProjectToEdgeInt(image,ProjectNearest(coord));
    return ZeroFloat;
}

float4 __attribute__((overloadable)) trans_coord_float_float_UNDEFINED(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    return ZeroFloat;   //will be background color, but it's a "don't care" situtation
}

float4 __attribute__((overloadable)) trans_coord_float_float_NONE_TRUE_NEAREST(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    //not testing if coords are OOB - this mode doesn't guarantee safeness!
    *square0=ProjectNearest(Unnormalize(image, coord));
    return ZeroFloat;
}

float4 __attribute__((overloadable)) trans_coord_float_float_CLAMPTOEDGE_TRUE_NEAREST(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    *square0=ProjectToEdgeInt(image, ProjectNearest(Unnormalize(image, coord)));
    return ZeroFloat;
}

float4 __attribute__((overloadable)) trans_coord_float_float_REPEAT_TRUE_NEAREST(image2d_t image, float4 coord, int4* square0, int4* square1)
{

#ifdef __SSE4_1__
    int4 upper = (int4)_mm_load_si128((__m128i*)(&((image_aux_data*)image)->dimSub1));
#else
    int4 upper=(int4)(0,0,0,0);
#endif
    int4 urcoord = ProjectNearest(Unnormalize(image, coord-floor(coord)));  //unrepeated coords
    
#ifdef __SSE4_1__
    __m128i mask = _mm_cmpgt_epi32((__m128i)urcoord, (__m128i)upper);
    *square0 = (int4)_mm_andnot_si128(mask, (__m128i)urcoord);
#else
    *square0 = urcoord % upper;
#endif
    return ZeroFloat;
}

float4 __attribute__((overloadable)) trans_coord_float_float_MIRRORED_TRUE_NEAREST(image2d_t image, float4 coord, int4* square0, int4* square1)
{
#ifdef __SSE4_1__
    int4 upper = (int4)_mm_load_si128((__m128i*)(&((image_aux_data*)image)->dimSub1));
    __m128 isZero = _mm_cmpeq_ps(coord, float4Allzeros);
    __m128 mcoord = (__m128)_mm_sub_epi32((__m128i)coord, *((__m128i*)f4minNorm));
    mcoord = _mm_round_ps(mcoord, _MM_ROUND_NEAREST);
    mcoord = (__m128)_mm_add_epi32((__m128i)mcoord, *((__m128i*)f4minNorm));
    /// Set to zero coordinates that were equal to zero before
    /// multiplications
    mcoord = (__m128)_mm_andnot_si128((__m128i)isZero, (__m128i)mcoord);
    mcoord = _mm_sub_ps(mcoord, coord);
    mcoord = _mm_abs_ps(mcoord);
#else
    int4 upper = vload4(0,(((image_aux_data*)image)->dimSub1));
    float4 mcoord=2.0f*rint(0.5f*coord);
    mcoord=fabs(coord-mcoord);
#endif
    int4 urcoord = ProjectNearest(Unnormalize(image, (float4)mcoord));  //unrepeated coords
    *square0=min(urcoord,upper);
    return ZeroFloat;
}


float4 __attribute__((overloadable)) trans_coord_float_NONE_FALSE_LINEAR(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    *square0=ProjectNearest(coord - halfhalfhalfzero);
    *square1=ProjectNearest(coord - halfhalfhalfzero) + oneOneOneZero;
    return frac(coord-halfhalfhalfzero);
}


float4 __attribute__ ((overloadable)) trans_coord_float_CLAMPTOEDGE_FALSE_LINEAR(image2d_t image, float4 coord, int4* square0, int4* square1)
{    
    *square0 = ProjectToEdgeInt(image, ProjectNearest(coord - halfhalfhalfzero));
    *square1 = ProjectToEdgeInt(image, ProjectNearest(coord - halfhalfhalfzero) + oneOneOneZero);
    return frac(coord-halfhalfhalfzero);
}


float4 __attribute__((overloadable)) trans_coord_float_NONE_TRUE_LINEAR(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    float4 ucoord = Unnormalize(image, coord);
    *square0=ProjectNearest(ucoord - halfhalfhalfzero);
    *square1=ProjectNearest(ucoord - halfhalfhalfzero) + oneOneOneZero;
    return frac(ucoord-halfhalfhalfzero);
}


float4 __attribute__((overloadable)) trans_coord_float_CLAMPTOEDGE_TRUE_LINEAR(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    float4 ucoord = Unnormalize(image, coord);
    int4 notClampedSquare0 = ProjectNearest(ucoord - halfhalfhalfzero);
    int4 notClampedSquare1 = ProjectNearest(ucoord - halfhalfhalfzero) + oneOneOneZero;

    *square0=ProjectToEdgeInt(image, notClampedSquare0);
    *square1=ProjectToEdgeInt(image, notClampedSquare1);
    return frac(ucoord-halfhalfhalfzero);
}


float4 __attribute__((overloadable)) trans_coord_float_REPEAT_TRUE_LINEAR(image2d_t image, float4 coord, int4* square0, int4* square1)
{
#ifdef __SSE4_1__
    int4 upper = (int4)_mm_load_si128((__m128i*)(&((image_aux_data*)image)->dim));
#else
     int4 upper=(int4)(0,0,0,0);
#endif
    float4 ucoord = Unnormalize(image, coord-floor(coord));  //unrepeated coords
    int4 sq0 = ProjectNearest(ucoord - halfhalfhalfzero);
    int4 sq1 = sq0 + oneOneOneZero;

#ifdef __SSE4_1__
    __m128i mask0 = _mm_cmpgt_epi32((__m128i)ZeroInt, (__m128i)sq0);
    __m128i addedVal = _mm_and_si128(mask0, (__m128i)upper);
    *square0 = sq0 + (int4)addedVal;

    int4 mask1 = (int4)_mm_cmpgt_epi32((__m128i)sq1, (__m128i)(upper-oneOneOneZero));
    *square1 = (int4)_mm_andnot_si128((__m128i)mask1, (__m128i)sq1);

#else
    *square0 = (sq0 +upper) % upper; //sq0 < (int4)0 ? upper + sq0 : sq0;
    *square1 = sq1 % upper; //sq1 > upper - oneOneOneZero ? sq1 - upper : sq1;
#endif
    return frac(ucoord-halfhalfhalfzero);
}

float4 __attribute__((overloadable)) trans_coord_float_MIRRORED_TRUE_LINEAR(image2d_t image, float4 coord, int4* square0, int4* square1)
{
 #ifdef __SSE4_1__
    int4 upper = (int4)_mm_load_si128((__m128i*)(&((image_aux_data*)image)->dimSub1));
#else
    int4 upper=(int4)(0,0,0,0);
 #endif
    float4 mcoord=2.0f*rint(0.5f*coord);
    mcoord=fabs(coord - mcoord);
    float4 urcoord = Unnormalize(image, mcoord);  //unrepeated coords
    int4 sq0 = ProjectNearest(urcoord - halfhalfhalfzero);
    int4 sq1 = sq0 + oneOneOneZero;
    int4 lower = (int4)(0,0,0,0);
    *square0 = max(lower, sq0);
    *square1 = min(sq1, upper);

    return frac(urcoord-halfhalfhalfzero);
}


/*******************************************************************UNSIGNED IMAGE TYPES I/IO*****************************************************************************/


/*****************************RGBA_UINT8 Image type i/o functions****************************************************/

/// Implements nearest callbacks for given image format and border color
#define IMPLEMENT_READ_SAMPLE_NEAREST(FORMAT, RETURN_TYPE, BORDER_COLOR)\
RETURN_TYPE __attribute__((overloadable)) read_sample_NEAREST_NOCLAMP_##FORMAT(image2d_t image, int4 coord, void* pData)\
{\
    void* pixel = extract_pixel_pointer_quad(image, coord, pData);\
    return load_pixel_##FORMAT(pixel);\
}\
\
RETURN_TYPE __attribute__((overloadable)) read_sample_NEAREST_CLAMP_##FORMAT(image2d_t image, int4 coord, void* pData)\
{\
    int isOOB = isOutOfBoundsInt(image, coord);\
    if (isOOB)\
        return BORDER_COLOR;\
    void* pixel = extract_pixel_pointer_quad(image, coord, pData);\
    return load_pixel_##FORMAT(pixel);\
}

#define IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(FORMAT, BORDER_COLOR)\
float4 __attribute__((overloadable)) read_sample_NEAREST_NOCLAMP_##FORMAT(image2d_t image, int4 coord, int4 dummy0, float4 dummy1, void* pData)\
{\
    void* pixel = extract_pixel_pointer_quad(image, coord, pData);\
    return load_pixel_##FORMAT(pixel);\
}\
\
float4 __attribute__((overloadable)) read_sample_NEAREST_CLAMP_##FORMAT(image2d_t image, int4 coord, int4 dummy0, float4 dummy1, void* pData)\
{\
    int isOOB = isOutOfBoundsInt(image, coord);\
    if (isOOB)\
        return BORDER_COLOR;\
    void* pixel = extract_pixel_pointer_quad(image, coord, pData);\
    return load_pixel_##FORMAT(pixel);\
}

IMPLEMENT_READ_SAMPLE_NEAREST(RGBA_UINT8,  uint4, BorderColorNoAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(RGBA_UINT16, uint4, BorderColorNoAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(RGBA_UINT32, uint4, BorderColorNoAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(RGBA_INT8,  int4, BorderColorNoAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST(RGBA_INT16, int4, BorderColorNoAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST(RGBA_INT32, int4, BorderColorNoAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RGBA_UNORM_INT8,  BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RGBA_UNORM_INT16, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RGBA_FLOAT, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RGBA_HALF_FLOAT, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(BGRA_UNORM_INT8, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(INTENSITY_FLOAT, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(INTENSITY_UNORM_INT8, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(INTENSITY_UNORM_INT16, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(INTENSITY_HALF_FLOAT, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(LUMINANCE_FLOAT, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(LUMINANCE_UNORM_INT8, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(LUMINANCE_UNORM_INT16, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(LUMINANCE_HALF_FLOAT, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(R_FLOAT, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(R_UNORM_INT8, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(R_UNORM_INT16, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST(R_INT8, int4, BorderColorAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST(R_INT16, int4, BorderColorAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST(R_INT32, int4, BorderColorAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST(R_UINT8, uint4, BorderColorAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(R_UINT16, uint4, BorderColorAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(R_UINT32, uint4, BorderColorAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(R_HALF_FLOAT, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(A_FLOAT, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(A_UNORM_INT8, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(A_UNORM_INT16, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(A_HALF_FLOAT, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST(RG_UINT8,  uint4, BorderColorAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(RG_UINT16, uint4, BorderColorAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(RG_UINT32, uint4, BorderColorAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(RG_INT8,  int4, BorderColorAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST(RG_INT16, int4, BorderColorAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST(RG_INT32, int4, BorderColorAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RG_UNORM_INT8, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RG_UNORM_INT16, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RG_FLOAT, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RG_HALF_FLOAT, BorderColorAlphaFloat)

void __attribute__((overloadable)) write_sample_RGBA_UINT8(void* pixel, uint4 color)
{
#ifdef __SSE4_1__
    color = min(color, (uint4)(UCHAR_MAX));
    *(char4*)pixel = trunc_v4i32_v4i8(*((int4*)&color));
#else
    // This function should be copy-pasted from BI module
    (*(uchar4*)pixel)=convert_uchar4_sat(color);
#endif
}

void __attribute__((overloadable)) write_sample_RG_UINT8(void* pixel, uint4 color)
{
#ifdef __SSE4_1__
    const __m128i i4uint8Max = _mm_set1_epi32(UCHAR_MAX);
    __m128i i4Val=(__m128i)color;
    i4Val = _mm_min_epi32(i4Val, i4uint8Max);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
#else
    // This function should be copy-pasted from BI module
    (*(uchar2*)pixel)=convert_uchar4_sat(color).lo;
#endif
}

void __attribute__((overloadable)) write_sample_R_UINT8(void* pixel, uint4 color)
{
#ifdef __SSE4_1__
    const __m128i i4uint8Max = _mm_set1_epi32(UCHAR_MAX);
    __m128i i4Val=(__m128i)color;
    i4Val = _mm_min_epi32(i4Val, i4uint8Max);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
#else
    // This function should be copy-pasted from BI module
    (*(uchar*)pixel)=convert_uchar4_sat(color).x;
#endif
}

/*****************************RGBA_UINT16 Image type i/o functions****************************************************/

uint4 __attribute__((overloadable)) load_pixel_RGBA_UINT16(void* pPixel)
{
#ifdef __SSE4_1__
    __m128i i4Val = _mm_loadl_epi64((__m128i*)pPixel);
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    return (uint4)i4Val;
#else
    /// This function should be copy-pasted from BI module
    return convert_uint4(*((ushort4*)pPixel));
#endif
}

void __attribute__((overloadable)) write_sample_RGBA_UINT16(void* pixel, uint4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = _mm_min_epi32((__m128i)color, (__m128i)i4uint16Max);
    /// pack values to pixels
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+2) = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+3) = (unsigned short)_mm_cvtsi128_si32(i4Val);
#else
    (*(ushort4*)pixel)=convert_ushort4_sat(color);
#endif
}

void __attribute__((overloadable)) write_sample_RG_UINT16(void* pixel, uint4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = _mm_min_epi32((__m128i)color, (__m128i)i4uint16Max);
    /// pack values to pixels
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
#else
    (*(ushort2*)pixel)=convert_ushort4_sat(color).xy;
#endif
}

void __attribute__((overloadable)) write_sample_R_UINT16(void* pixel, uint4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = _mm_min_epi32((__m128i)color, (__m128i)i4uint16Max);
    /// pack values to pixels
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
#else
    (*(ushort4*)pixel)=convert_ushort4_sat(color).x;
#endif
}

/*****************************RGBA_UINT32 Image type i/o functions****************************************************/

uint4 __attribute__((overloadable)) load_pixel_RGBA_UINT32(void* pPixel)
{
    return (*((uint4*)pPixel));
}

void __attribute__((overloadable)) write_sample_RGBA_UINT32(void* pixel, uint4 color)
{
    (*(uint4*)pixel)=color;
}

void __attribute__((overloadable)) write_sample_R_UINT32(void* pixel, uint4 color)
{
    (*(uint*)pixel)=color.x;
}

void __attribute__((overloadable)) write_sample_RG_UINT32(void* pixel, uint4 color)
{
    (*(uint2*)pixel)=color.lo;
}

/*******************************************************************SIGNED IMAGE TYPES I/IO*****************************************************************************/


/*****************************RGBA_INT8 Image type i/o functions****************************************************/


int4 __attribute__((overloadable)) load_pixel_RGBA_INT8(void* pPixel)
{
#ifdef __SSE4_1__
    __m128i i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPixel);
    i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    // Extend sign
    i4Val = _mm_slli_si128(i4Val, 3);
    i4Val = _mm_srai_epi32(i4Val, 24);
    return (int4)i4Val;
#else 
    /// This function should be copy-pasted from Built-ins module
    return convert_int4(*((char4*)pPixel));
#endif
}

void __attribute__((overloadable)) write_sample_RGBA_INT8(void* pixel, int4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = _mm_max_epi32((__m128i)color, (__m128i)i4int16Min);
    i4Val = _mm_min_epi32(i4Val, (__m128i)i4int16Max);
    i4Val = _mm_packs_epi32(i4Val, i4Val);
    i4Val = _mm_packs_epi16(i4Val, i4Val);
    *(unsigned int*)pixel = _mm_cvtsi128_si32(i4Val);
#else
    (*(char4*)pixel)=convert_char4_sat(color);
#endif
}

void __attribute__((overloadable)) write_sample_R_INT8(void* pixel, int4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = _mm_max_epi32((__m128i)color, (__m128i)i4int16Min);
    i4Val = _mm_min_epi32(i4Val, (__m128i)i4int16Max);
    i4Val = _mm_packs_epi32(i4Val, i4Val);
    i4Val = _mm_packs_epi16(i4Val, i4Val);
    *(char*)pixel = ((char4)_mm_cvtsi128_si32(i4Val)).x;
#else
    (*(char*)pixel)=convert_char4_sat(color).x;
#endif
}

void __attribute__((overloadable)) write_sample_RG_INT8(void* pixel, int4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = _mm_max_epi32((__m128i)color, (__m128i)i4int16Min);
    i4Val = _mm_min_epi32(i4Val, (__m128i)i4int16Max);
    i4Val = _mm_packs_epi32(i4Val, i4Val);
    i4Val = _mm_packs_epi16(i4Val, i4Val);
    *(unsigned short*)pixel = ((ushort2)_mm_cvtsi128_si32(i4Val)).x;
#else
    (*(unsigned short*)pixel)=((ushort2)convert_char4_sat(color)).x;
#endif
}
/*****************************RGBA_INT16 Image type i/o functions****************************************************/

int4 __attribute__((overloadable)) load_pixel_RGBA_INT16(void* pPixel)
{
#ifdef __SSE4_1__
    __m128i i4Val = _mm_loadl_epi64((__m128i*)pPixel);
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    // Extend sign
    i4Val = _mm_slli_si128(i4Val, 2);
    i4Val = _mm_srai_epi32(i4Val, 16);
    return (int4)i4Val;
#else
    /// This function should be copy-pasted from BI module
    return convert_int4(*((short4*)pPixel));
#endif
}

void __attribute__((overloadable)) write_sample_RGBA_INT16(void* pixel, int4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = (__m128i)color;
    i4Val = _mm_max_epi32(i4Val, (__m128i)i4int16Min);
    i4Val = _mm_min_epi32(i4Val, (__m128i)i4int16Max);
    // Shrink to 8bit
    i4Val = _mm_packs_epi32(i4Val, i4Val);
    _mm_storel_epi64((__m128i*)pixel, i4Val);
#else
    (*(short4*)pixel)=convert_short4_sat(color);
#endif
}

void __attribute__((overloadable)) write_sample_RG_INT16(void* pixel, int4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = (__m128i)color;
    i4Val = _mm_max_epi32(i4Val, (__m128i)i4int16Min);
    i4Val = _mm_min_epi32(i4Val, (__m128i)i4int16Max);
    // i4Val already contains valid short value
    (*(short*)pixel)=((int4)i4Val).x;
    ((short*)pixel)[1]=((int4)i4Val).y;
#else
    (*(short2*)pixel)=convert_short4_sat(color).lo;
#endif
}

void __attribute__((overloadable)) write_sample_R_INT16(void* pixel, int4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = (__m128i)color;
    i4Val = _mm_max_epi32(i4Val, (__m128i)i4int16Min);
    i4Val = _mm_min_epi32(i4Val, (__m128i)i4int16Max);
    // i4Val already contains valid short value
    (*(short*)pixel)=((int4)i4Val).x;
#else
    (*(short*)pixel)=convert_short4_sat(color).x;
#endif
}

/*****************************RGBA_INT32 Image type i/o functions****************************************************/

int4 __attribute__((overloadable)) load_pixel_RGBA_INT32(void* pPixel)
{
    return (*((int4*)pPixel));
}

void __attribute__((overloadable)) write_sample_RGBA_INT32(void* pixel, int4 color)
{
    (*(int4*)pixel)=color;
}

void __attribute__((overloadable)) write_sample_R_INT32(void* pixel, int4 color)
{
    (*(int*)pixel)=color.x;
}

void __attribute__((overloadable)) write_sample_RG_INT32(void* pixel, int4 color)
{
    (*(int2*)pixel)=color.lo;
}

/*****************************************************************UNORM IMAGES TYPES I/O*****************************************************************/


/***************************************RGBA_UNORM8 Image type i/o functions*****************************************************/

float4 __attribute__((overloadable)) load_pixel_RGBA_UNORM_INT8(void* pPixel)
{
#ifdef __SSE4_1__
    __m128i i4Val = (__m128i)_mm_cvtsi32_si128(*(unsigned int*)pPixel);
    i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    float4 converted = _mm_cvtepi32_ps(i4Val);
#else
    uchar4 input = vload4(0, (uchar*)pPixel);
    // This function should be copy-pasted from BI module
    float4 converted = convert_float4(input);
#endif
    converted = converted*(float4)(1.0f/255.0f);
    return converted;
}

void __attribute__((overloadable)) write_sample_RGBA_UNORM_INT8(void* pixel, float4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = cvt_to_norm((__m128i)color, (__m128)f4unorm8mul, (__m128)f4unorm8lim);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+2) = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+3) = (unsigned char)_mm_cvtsi128_si32(i4Val);
#else
    uchar4 convertedColor = convert_uchar4(color * 255.0f);
    (*(uchar4*)pixel) = convertedColor;
#endif
}

void __attribute__((overloadable)) write_sample_RG_UNORM_INT8(void* pixel, float4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm8mul, f4unorm8lim);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
#else
    uchar4 convertedColor = convert_uchar4(color * 255.0f);
    (*(uchar2*)pixel) = convertedColor.lo;
#endif
}

void __attribute__((overloadable)) write_sample_R_UNORM_INT8(void* pixel, float4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm8mul, f4unorm8lim);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
#else
    uchar4 convertedColor = convert_uchar4(color * 255.0f);
    (*(uchar*)pixel) = convertedColor.x;
#endif
}

void __attribute__((overloadable)) write_sample_A_UNORM_INT8(void* pixel, float4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm8mul, f4unorm8lim);
    i4Val = _mm_srli_si128(i4Val, 12);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
#else
    uchar4 convertedColor = convert_uchar4(color * 255.0f);
    (*(uchar*)pixel) = convertedColor.w;
#endif

}

/***************************************RGBA_UNORM16 Image type i/o functions*****************************************************/

float4 __attribute__((overloadable)) load_pixel_RGBA_UNORM_INT16(void* pPixel)
{

#ifdef __SSE4_1__
    __m128i i4Val = _mm_loadl_epi64((__m128i*)pPixel);
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    __m128 f4Val = _mm_cvtepi32_ps(i4Val);
    return _mm_mul_ps(f4Val, f4Unorm16Dim);
#else
    //__m128 i4Val = _mm_loadl_epi64((__m128i*)pPixel);
    ushort4 input = vload4(0, (ushort*)pPixel);
    /// convert_float4 should be copy-pasted from built-ins module
    float4 converted = convert_float4(input)*(1.0f/65535.0f);
    return converted;
#endif
}

void __attribute__((overloadable)) write_sample_RGBA_UNORM_INT16(void* pixel, float4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = cvt_to_norm((__m128i)color, (__m128)f4unorm16mul, (__m128)f4unorm16lim);
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+2) = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+3) = (unsigned short)_mm_cvtsi128_si32(i4Val);
#else
    ushort4 convertedColor = convert_ushort4(color * 65535.0f);
    (*(ushort4*)pixel) = convertedColor;
#endif
}

void __attribute__((overloadable)) write_sample_RG_UNORM_INT16(void* pixel, float4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm16mul, f4unorm16lim);
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
#else
    ushort4 convertedColor = convert_ushort4(color * 65535.0f);
    (*(ushort2*)pixel) = convertedColor.lo;
#endif
}

void __attribute__((overloadable)) write_sample_R_UNORM_INT16(void* pixel, float4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm16mul, f4unorm16lim);
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
#else
    ushort4 convertedColor = convert_ushort4(color * 65535.0f);
    (*(unsigned short*)pixel) = convertedColor.x;
#endif
}

void __attribute__((overloadable)) write_sample_A_UNORM_INT16(void* pixel, float4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm16mul, f4unorm16lim);
    i4Val = _mm_srli_si128(i4Val, 12);
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
#else
    ushort4 convertedColor = convert_ushort4(color * 65535.0f);
    (*(unsigned short*)pixel) = convertedColor.w;
#endif

}



/***************************************BGRA_UNORM8 Image type i/o functions*****************************************************/

float4 __attribute__((overloadable)) load_pixel_BGRA_UNORM_INT8(void* pPixel)
{

#ifdef __SSE4_1__
    __m128i i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPixel);
    i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    i4Val = (__m128i)_mm_cvtepi32_ps(i4Val);
    i4Val = (__m128i)_mm_mul_ps((__m128)i4Val, (__m128)f4Unorm8Dim);
    i4Val = _mm_shuffle_epi32(i4Val, _MM_SHUFFLE(3, 0, 1, 2));
    return (float4)i4Val;
#else
    uchar4 input = vload4(0, (uchar*)pPixel);
    // This function should be copy-pasted from BI module
    float4 res = convert_float4(input)*(float4)(1.f/255.0f);
    // This function should be copy-pasted from BI module
    res = res.zyxw;
    return res;
#endif
}

void __attribute__((overloadable)) write_sample_BGRA_UNORM_INT8(void* pixel, float4 color)
{
#ifdef __SSE4_1__
    float4 convertedColor = color.zyxw;
    __m128i i4Val = cvt_to_norm((__m128i)convertedColor, (__m128)f4unorm8mul, (__m128)f4unorm8lim);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+2) = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+3) = (unsigned char)_mm_cvtsi128_si32(i4Val);
#else
    uchar4 convertedColor = convert_uchar4(color * 255.0f);
    convertedColor = (uchar4)convertedColor.zyxw;
    (*(uchar4*)pixel) = convertedColor;
#endif
}



/****************************************************************FLOAT IMAGE TYPES I/O***************************************************************/


/******************************************RGBA_FLOAT image type i/o functions******************************************************/

float4 __attribute__((overloadable)) load_pixel_RGBA_FLOAT(void* pPixel)
{
    return *((float4*)pPixel);
}

void __attribute__((overloadable)) write_sample_RGBA_FLOAT(void* pixel, float4 color)
{
    (*(float4*)pixel)=color;
}

/******************************************RGBA_HALF_FLOAT image type i/o functions******************************************************/

float4 __attribute__((overloadable)) load_pixel_RGBA_HALF_FLOAT(void* pPixel)
{
#ifdef __SSE4_1__
    return (float4)Half4ToFloat4((short8)_mm_loadl_epi64((__m128i*)pPixel));
#else
    return vloada_half4(0, (half*)pPixel);
#endif
}

void __attribute__((overloadable)) write_sample_RGBA_HALF_FLOAT(void* pixel, float4 color)
{
    vstore_half4(color, 0, (half*)pixel);
}

void __attribute__((overloadable)) write_sample_R_HALF_FLOAT(void* pixel, float4 color)
{
    vstore_half(color.x, 0, (half*)pixel);
}

void __attribute__((overloadable)) write_sample_RG_HALF_FLOAT(void* pixel, float4 color)
{
    vstore_half2(color.lo, 0, (half*)pixel);
}

void __attribute__((overloadable)) write_sample_A_HALF_FLOAT(void* pixel, float4 color)
{
    vstore_half(color.w, 0, (half*)pixel); // store alpha channel from pixel (0,0,0,a)
}

void __attribute__((overloadable)) write_sample_LUMINANCE_HALF_FLOAT(void* pixel, float4 color)
{
    vstore_half(color.x, 0, (half*)pixel);
}

void __attribute__((overloadable)) write_sample_INTENSITY_HALF_FLOAT(void* pixel, float4 color)
{
    vstore_half(color.x, 0, (half*)pixel);
}

/******************************************LUMINANCE image type i/o functions******************************************************/

float __attribute__((overloadable)) load_value_LUMINANCE_FLOAT(void* pPixel)
{
    float luminance = *((float*)pPixel);
    return luminance;
}

float4 __attribute__((overloadable)) load_pixel_LUMINANCE_FLOAT(void* pPixel)
{
    float luminance = load_value_LUMINANCE_FLOAT(pPixel);
    float4 res = (float4)(luminance, luminance, luminance, 1.0f);
    return res;
}

float __attribute__((overloadable)) load_value_LUMINANCE_UNORM_INT8(void* pPixel)
{
    uchar val = *(uchar*)pPixel;
    return val * (1.0f/255.0f);
}

float4 __attribute__((overloadable)) load_pixel_LUMINANCE_UNORM_INT8(void* pPixel)
{
    float luminance = load_value_LUMINANCE_UNORM_INT8(pPixel);
    return (float4)(luminance, luminance, luminance, 1.0f);
}

float __attribute__((overloadable)) load_value_LUMINANCE_UNORM_INT16(void* pPixel)
{
    ushort val = *(ushort*)pPixel;
    return val * (1.0f/65535.0f);
}

float4 __attribute__((overloadable)) load_pixel_LUMINANCE_UNORM_INT16(void* pPixel)
{
    float luminance = load_value_LUMINANCE_UNORM_INT16(pPixel);
    return (float4)(luminance, luminance, luminance, 1.0f);
}

float __attribute__((overloadable)) load_value_LUMINANCE_HALF_FLOAT(void* pPixel)
{
#ifdef __SSE4_1__
    half4 pix;
    pix.x = *(half*)pPixel;
    float4 val = (float4)Half4ToFloat4((short8)_mm_loadl_epi64((__m128i*)(&pix)));
#else
    float4 val = vloada_half4(0, (half*)pPixel);
#endif
    return val.x;
}

float4 __attribute__((overloadable)) load_pixel_LUMINANCE_HALF_FLOAT(void* pPixel)
{
    float val = load_value_LUMINANCE_HALF_FLOAT(pPixel);
    return (float4)(val, val, val, 1.f);
}

float __attribute__((overloadable)) load_value_INTENSITY_UNORM_INT8(void* pPixel)
{
    uchar val = *(uchar*)pPixel;
    return val * (1.0f/255.0f);
}

float4 __attribute__((overloadable)) load_pixel_INTENSITY_UNORM_INT8(void* pPixel)
{
    float intensity = load_value_INTENSITY_UNORM_INT8(pPixel);
    return (float4)(intensity, intensity, intensity, intensity);
}

float __attribute__((overloadable)) load_value_INTENSITY_UNORM_INT16(void* pPixel)
{
    ushort val = *(ushort*)pPixel;
    return val * (1.0f/65535.0f);
}

float4 __attribute__((overloadable)) load_pixel_INTENSITY_UNORM_INT16(void* pPixel)
{
    float intensity = load_value_INTENSITY_UNORM_INT16(pPixel);
    return (float4)(intensity, intensity, intensity, intensity);
}

float __attribute__((overloadable)) load_value_INTENSITY_HALF_FLOAT(void* pPixel)
{
#ifdef __SSE4_1__
    half4 pix;
    pix.x = *(half*)pPixel;
    float4 val = (float4)Half4ToFloat4((short8)_mm_loadl_epi64((__m128i*)(&pix)));
#else
    float4 val = vloada_half4(0, (half*)pPixel);
#endif
    return val.x;
}

float4 __attribute__((overloadable)) load_pixel_INTENSITY_HALF_FLOAT(void* pPixel)
{
    float val = load_value_INTENSITY_HALF_FLOAT(pPixel);
    return (float4)(val, val, val, val);
}

void __attribute__((overloadable)) write_sample_LUMINANCE_FLOAT(void* pixel, float4 color)
{
    (*(float*)pixel)=color.x;
}

/******************************************INTENSITY image type i/o functions******************************************************/

float __attribute__((overloadable)) load_value_INTENSITY_FLOAT(void* pPixel)
{
    float intensity = *((float*)pPixel);
    return intensity;
}

float4 __attribute__((overloadable)) load_pixel_INTENSITY_FLOAT(void* pPixel)
{
    float intensity = load_value_INTENSITY_FLOAT(pPixel);
    return (float4)intensity;
}

void __attribute__((overloadable)) write_sample_INTENSITY_FLOAT(void* pixel, float4 color)
{
    (*(float*)pixel)=color.x;
}

void __attribute__((overloadable)) write_sample_INTENSITY_UNORM_INT8(void* pixel, float4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm8mul, f4unorm8lim);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
#else
    uchar4 convertedColor = convert_uchar4(color * 255.0f);
    (*(uchar*)pixel) = convertedColor.x;
#endif
}

void __attribute__((overloadable)) write_sample_INTENSITY_UNORM_INT16(void* pixel, float4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm16mul, f4unorm16lim);
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
#else
    ushort4 convertedColor = convert_ushort4(color * 65535.0f);
    (*(ushort*)pixel) = convertedColor.x;
#endif
}

void __attribute__((overloadable)) write_sample_LUMINANCE_UNORM_INT8(void* pixel, float4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm8mul, f4unorm8lim);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
#else
    uchar4 convertedColor = convert_uchar4(color * 255.0f);
    (*(uchar*)pixel) = convertedColor.x;
#endif
}

void __attribute__((overloadable)) write_sample_LUMINANCE_UNORM_INT16(void* pixel, float4 color)
{
#ifdef __SSE4_1__
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm16mul, f4unorm16lim);
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
#else
    ushort4 convertedColor = convert_ushort4(color * 65535.0f);
    (*(ushort*)pixel) = convertedColor.x;
#endif
}

void __attribute__((overloadable)) write_sample_R_FLOAT(void* pixel, float4 color)
{
    (*(float*)pixel)=color.x;
}

void __attribute__((overloadable)) write_sample_RG_FLOAT(void* pixel, float4 color)
{
    (*(float2*)pixel)=color.lo;
}

void __attribute__((overloadable)) write_sample_A_FLOAT(void* pixel, float4 color)
{
    (*(float*)pixel)=color.w;
}

/******************LINEAR reading functions******************************************/

float4 luminance_post_process(float4 vec)
{
    vec.w = 1.0f;
    return vec;
}

float4 dummyFnc(float4 input)
{
    return input;
}


#define IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(TYPE, POST_PROCESSING) \
    float4 __attribute__ ((overloadable)) read_sample_LINEAR1D_NOCLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
{\
    int4 point0   = square0;\
    int4 point1   = square1;\
    \
    float4 Ti0 = load_pixel_##TYPE(extract_pixel_pointer_quad(image, point0, pData));\
    float4 Ti1 = load_pixel_##TYPE(extract_pixel_pointer_quad(image, point1, pData));\
    \
    float4 result=SampleImage1DFloat(Ti0, Ti1, fraction);\
    return POST_PROCESSING(result);\
}

IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(RGBA_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(RGBA_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(INTENSITY_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(INTENSITY_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(INTENSITY_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(INTENSITY_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(LUMINANCE_FLOAT, luminance_post_process)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(LUMINANCE_UNORM_INT8, luminance_post_process)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(LUMINANCE_UNORM_INT16, luminance_post_process)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(LUMINANCE_HALF_FLOAT, luminance_post_process)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(RGBA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(BGRA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(RGBA_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(R_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(R_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(R_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(R_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(A_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(A_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(A_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(A_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(RG_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(RG_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(RG_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NOCLAMP(RG_UNORM_INT16, dummyFnc)


// definition for linear read callbacks in case of one channel images
#define IMPLEMENT_read_sample_LINEAR2D_NOCLAMP_CH1(TYPE, POST_PROCESSING) \
    float4 __attribute__ ((overloadable)) read_sample_LINEAR2D_NOCLAMP_CH1_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
{\
    /*First genenrate weights for pixels*/\
    \
    int4 point00   = square0;\
    int4 point10   = (int4)(square1.x, square0.y, 0, 0);\
    int4 point01   = (int4)(square0.x, square1.y, 0, 0);\
    int4 point11   = square1;\
    \
    float4 components;\
    components.x = load_value_##TYPE(extract_pixel_pointer_quad(image, point00, pData));\
    components.y = load_value_##TYPE(extract_pixel_pointer_quad(image, point10, pData));\
    components.z = load_value_##TYPE(extract_pixel_pointer_quad(image, point01, pData));\
    components.w = load_value_##TYPE(extract_pixel_pointer_quad(image, point11, pData));\
    \
    float4 result=SampleImage2DFloatCh1(components, fraction);\
    return POST_PROCESSING(result);\
}

// definition for linear read callbacks in case of 4 channel images
#define IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(TYPE, POST_PROCESSING) \
    float4 __attribute__ ((overloadable)) read_sample_LINEAR2D_NOCLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
{\
    /*First genenrate weights for pixels*/\
    \
    int4 point00   = square0;\
    int4 point10   = (int4)(square1.x, square0.y, 0, 0);\
    int4 point01   = (int4)(square0.x, square1.y, 0, 0);\
    int4 point11   = square1;\
    \
    float4 Ti0j0 = load_pixel_##TYPE(extract_pixel_pointer_quad(image, point00, pData));\
    float4 Ti1j0 = load_pixel_##TYPE(extract_pixel_pointer_quad(image, point10, pData));\
    float4 Ti0j1 = load_pixel_##TYPE(extract_pixel_pointer_quad(image, point01, pData));\
    float4 Ti1j1 = load_pixel_##TYPE(extract_pixel_pointer_quad(image, point11, pData));\
    \
    float4 result=SampleImage2DFloat(Ti0j0, Ti1j0, Ti0j1, Ti1j1, fraction);\
    return POST_PROCESSING(result);\
}

IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(RGBA_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(RGBA_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP_CH1(INTENSITY_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP_CH1(INTENSITY_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP_CH1(INTENSITY_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP_CH1(INTENSITY_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP_CH1(LUMINANCE_FLOAT, luminance_post_process)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP_CH1(LUMINANCE_UNORM_INT8, luminance_post_process)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP_CH1(LUMINANCE_UNORM_INT16, luminance_post_process)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP_CH1(LUMINANCE_HALF_FLOAT, luminance_post_process)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(RGBA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(BGRA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(RGBA_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(R_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(R_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(R_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(R_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(A_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(A_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(A_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(A_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(RG_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(RG_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(RG_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NOCLAMP(RG_UNORM_INT16, dummyFnc)


#define IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(TYPE, POST_PROCESSING) \
float4 __attribute__ ((overloadable)) read_sample_LINEAR3D_NOCLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
{\
    /*First genenerate weights for pixels*/\
    \
    int4 point000   = square0;\
    int4 point100   = (int4)(square1.x, square0.y, square0.z, 0);\
    int4 point010   = (int4)(square0.x, square1.y, square0.z, 0);\
    int4 point110   = (int4)(square1.x, square1.y, square0.z, 0);\
    int4 point001   = (int4)(square0.x, square0.y, square1.z, 0);\
    int4 point101   = (int4)(square1.x, square0.y, square1.z, 0);\
    int4 point011   = (int4)(square0.x, square1.y, square1.z, 0);\
    int4 point111   = square1;\
\
    float4 Ti0j0k0 = load_pixel_##TYPE(extract_pixel_pointer_quad(image, point000, pData));\
    float4 Ti1j0k0 = load_pixel_##TYPE(extract_pixel_pointer_quad(image, point100, pData));\
    float4 Ti0j1k0 = load_pixel_##TYPE(extract_pixel_pointer_quad(image, point010, pData));\
    float4 Ti1j1k0 = load_pixel_##TYPE(extract_pixel_pointer_quad(image, point110, pData));\
    float4 Ti0j0k1 = load_pixel_##TYPE(extract_pixel_pointer_quad(image, point001, pData));\
    float4 Ti1j0k1 = load_pixel_##TYPE(extract_pixel_pointer_quad(image, point101, pData));\
    float4 Ti0j1k1 = load_pixel_##TYPE(extract_pixel_pointer_quad(image, point011, pData));\
    float4 Ti1j1k1 = load_pixel_##TYPE(extract_pixel_pointer_quad(image, point111, pData));\
\
    float4 result=SampleImage3DFloat(Ti0j0k0, Ti1j0k0, Ti0j1k0, Ti1j1k0, Ti0j0k1, Ti1j0k1, Ti0j1k1, Ti1j1k1, fraction);\
    return POST_PROCESSING(result);\
}

IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(RGBA_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(RGBA_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(INTENSITY_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(INTENSITY_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(INTENSITY_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(INTENSITY_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(LUMINANCE_FLOAT, luminance_post_process)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(LUMINANCE_UNORM_INT8, luminance_post_process)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(LUMINANCE_UNORM_INT16, luminance_post_process)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(LUMINANCE_HALF_FLOAT, luminance_post_process)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(RGBA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(BGRA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(RGBA_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(R_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(R_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(R_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(R_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(A_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(A_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(A_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(A_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(RG_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(RG_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(RG_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NOCLAMP(RG_HALF_FLOAT, dummyFnc)


#define IMPLEMENT_read_sample_LINEAR1D_CLAMP(TYPE, BORDER_COLOR, POST_PROCESSING) \
    float4 __attribute__ ((overloadable)) read_sample_LINEAR1D_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
{\
    \
    int4 point0   = square0;\
    int4 point1   = square1;\
    \
    float4 Ti0 = isOutOfBoundsInt(image, point0)   ? BORDER_COLOR : load_pixel_##TYPE(extract_pixel_pointer_quad(image, point0, pData));\
    float4 Ti1 = isOutOfBoundsInt(image, point1)   ? BORDER_COLOR : load_pixel_##TYPE(extract_pixel_pointer_quad(image, point1, pData));\
    \
    float4 result = SampleImage1DFloat(Ti0, Ti1, fraction);\
    return POST_PROCESSING(result);\
}

IMPLEMENT_read_sample_LINEAR1D_CLAMP(RGBA_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RGBA_HALF_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(INTENSITY_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(INTENSITY_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(INTENSITY_UNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(INTENSITY_HALF_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(LUMINANCE_FLOAT, BorderColorAlphaFloat, luminance_post_process)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(LUMINANCE_UNORM_INT8, BorderColorAlphaFloat, luminance_post_process)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(LUMINANCE_UNORM_INT16, BorderColorAlphaFloat, luminance_post_process)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(LUMINANCE_HALF_FLOAT, BorderColorAlphaFloat, luminance_post_process)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RGBA_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(BGRA_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RGBA_UNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(R_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(R_UNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(R_UNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(R_HALF_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(A_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(A_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(A_UNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(A_HALF_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RG_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RG_UNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RG_UNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RG_HALF_FLOAT, BorderColorAlphaFloat, dummyFnc)


#define IMPLEMENT_read_sample_LINEAR2D_CLAMP_CH1(TYPE, POST_PROCESSING) \
    float4 __attribute__ ((overloadable)) read_sample_LINEAR2D_NOCLAMP_CH1_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
{\
    /*First genenrate weights for pixels*/\
    \
    int4 point00   = square0;\
    int4 point10   = (int4)(square1.x, square0.y, 0, 0);\
    int4 point01   = (int4)(square0.x, square1.y, 0, 0);\
    int4 point11   = square1;\
    \
    float4 components;\
    components.x = isOutOfBoundsInt(image, point00)   ? BORDER_COLOR.x : load_value_##TYPE(extract_pixel_pointer_quad(image, point00, pData));\
    components.y = isOutOfBoundsInt(image, point10)   ? BORDER_COLOR.y : load_value_##TYPE(extract_pixel_pointer_quad(image, point10, pData));\
    components.z = isOutOfBoundsInt(image, point01)   ? BORDER_COLOR.z : load_value_##TYPE(extract_pixel_pointer_quad(image, point01, pData));\
    components.w = isOutOfBoundsInt(image, point11)   ? BORDER_COLOR.w : load_value_##TYPE(extract_pixel_pointer_quad(image, point11, pData));\
    \
    float4 result=SampleImage2DFloatCh1(components, fraction);\
    return POST_PROCESSING(result);\
}

#define IMPLEMENT_read_sample_LINEAR2D_CLAMP(TYPE, BORDER_COLOR, POST_PROCESSING) \
    float4 __attribute__ ((overloadable)) read_sample_LINEAR2D_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
{\
    \
    int4 point00   = square0;\
    int4 point10   = (int4)(square1.x, square0.y, 0, 0);\
    int4 point01   = (int4)(square0.x, square1.y, 0, 0);\
    int4 point11   = square1;\
    \
    float4 Ti0j0 = isOutOfBoundsInt(image, point00)   ? BORDER_COLOR : load_pixel_##TYPE(extract_pixel_pointer_quad(image, point00, pData));\
    float4 Ti1j0 = isOutOfBoundsInt(image, point10)   ? BORDER_COLOR : load_pixel_##TYPE(extract_pixel_pointer_quad(image, point10, pData));\
    float4 Ti0j1 = isOutOfBoundsInt(image, point01)   ? BORDER_COLOR : load_pixel_##TYPE(extract_pixel_pointer_quad(image, point01, pData));\
    float4 Ti1j1 = isOutOfBoundsInt(image, point11)   ? BORDER_COLOR : load_pixel_##TYPE(extract_pixel_pointer_quad(image, point11, pData));\
    \
    float4 result = SampleImage2DFloat(Ti0j0, Ti1j0, Ti0j1, Ti1j1, fraction);\
    return POST_PROCESSING(result);\
}

IMPLEMENT_read_sample_LINEAR2D_CLAMP(RGBA_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RGBA_HALF_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(INTENSITY_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(INTENSITY_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(INTENSITY_UNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(INTENSITY_HALF_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(LUMINANCE_FLOAT, BorderColorAlphaFloat, luminance_post_process)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(LUMINANCE_UNORM_INT8, BorderColorAlphaFloat, luminance_post_process)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(LUMINANCE_UNORM_INT16, BorderColorAlphaFloat, luminance_post_process)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(LUMINANCE_HALF_FLOAT, BorderColorAlphaFloat, luminance_post_process)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RGBA_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(BGRA_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RGBA_UNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(R_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(R_UNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(R_UNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(R_HALF_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(A_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(A_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(A_UNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(A_HALF_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RG_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RG_UNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RG_UNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RG_HALF_FLOAT, BorderColorAlphaFloat, dummyFnc)


#define IMPLEMENT_read_sample_LINEAR3D_CLAMP(TYPE, BORDER_COLOR, POST_PROCESSING) \
float4 __attribute__ ((overloadable)) read_sample_LINEAR3D_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
{\
\
    int4 point000   = square0;\
    int4 point100   = (int4)(square1.x, square0.y, square0.z, 0);\
    int4 point010   = (int4)(square0.x, square1.y, square0.z, 0);\
    int4 point110   = (int4)(square1.x, square1.y, square0.z, 0);\
    int4 point001   = (int4)(square0.x, square0.y, square1.z, 0);\
    int4 point101   = (int4)(square1.x, square0.y, square1.z, 0);\
    int4 point011   = (int4)(square0.x, square1.y, square1.z, 0);\
    int4 point111   = square1;\
\
    float4 Ti0j0k0 = isOutOfBoundsInt(image, point000)   ? BORDER_COLOR : load_pixel_##TYPE(extract_pixel_pointer_quad(image, point000, pData));\
    float4 Ti1j0k0 = isOutOfBoundsInt(image, point100)   ? BORDER_COLOR : load_pixel_##TYPE(extract_pixel_pointer_quad(image, point100, pData));\
    float4 Ti0j1k0 = isOutOfBoundsInt(image, point010)   ? BORDER_COLOR : load_pixel_##TYPE(extract_pixel_pointer_quad(image, point010, pData));\
    float4 Ti1j1k0 = isOutOfBoundsInt(image, point110)   ? BORDER_COLOR : load_pixel_##TYPE(extract_pixel_pointer_quad(image, point110, pData));\
    float4 Ti0j0k1 = isOutOfBoundsInt(image, point001)   ? BORDER_COLOR : load_pixel_##TYPE(extract_pixel_pointer_quad(image, point001, pData));\
    float4 Ti1j0k1 = isOutOfBoundsInt(image, point101)   ? BORDER_COLOR : load_pixel_##TYPE(extract_pixel_pointer_quad(image, point101, pData));\
    float4 Ti0j1k1 = isOutOfBoundsInt(image, point011)   ? BORDER_COLOR : load_pixel_##TYPE(extract_pixel_pointer_quad(image, point011, pData));\
    float4 Ti1j1k1 = isOutOfBoundsInt(image, point111)   ? BORDER_COLOR : load_pixel_##TYPE(extract_pixel_pointer_quad(image, point111, pData));\
\
    float4 result = SampleImage3DFloat(Ti0j0k0, Ti1j0k0, Ti0j1k0, Ti1j1k0, Ti0j0k1, Ti1j0k1, Ti0j1k1, Ti1j1k1, fraction);\
    return POST_PROCESSING(result);\
}

IMPLEMENT_read_sample_LINEAR3D_CLAMP(RGBA_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RGBA_HALF_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(INTENSITY_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(INTENSITY_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(INTENSITY_UNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(INTENSITY_HALF_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(LUMINANCE_FLOAT, BorderColorAlphaFloat, luminance_post_process)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(LUMINANCE_UNORM_INT8, BorderColorAlphaFloat, luminance_post_process)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(LUMINANCE_UNORM_INT16, BorderColorAlphaFloat, luminance_post_process)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(LUMINANCE_HALF_FLOAT, BorderColorAlphaFloat, luminance_post_process)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RGBA_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(BGRA_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RGBA_UNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(R_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(R_UNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(R_UNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(R_HALF_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(A_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(A_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(A_UNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(A_HALF_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RG_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RG_UNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RG_UNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RG_HALF_FLOAT, BorderColorAlphaFloat, dummyFnc)


//////////////////////////////////////
//
// Images Load and Convert Functions
//
//////////////////////////////////////


// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_R & CL_Rx
// Channel Data Type: CLK_UNSIGNED_INT32
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a uint4 (r, 0, 0, 1.0)
uint4 __attribute__((overloadable)) load_pixel_R_UINT32(void* pPixel)
{
    uint4 pixel = (uint4)(0, 0, 0, 1);
    pixel.x = *((uint*)pPixel);
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_RG
// Channel Data Type: CLK_UNSIGNED_INT32
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a uint4 (r, g, 0, 1.0)
uint4 __attribute__((overloadable)) load_pixel_RG_UINT32(void* pPixel)
{
    uint4 pixel = (uint4)(0, 0, 0, 1);
    pixel.x = *((uint*)pPixel);
    pixel.y = ((uint*)pPixel)[1];
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_R & CL_Rx
// Channel Data Type: CLK_SIGNED_INT32
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a int4 (r, 0, 0, 1)
int4 __attribute__((overloadable)) load_pixel_R_INT32(void* pPixel)
{
    int4 pixel = (int4)(0, 0, 0, 1);
    pixel.x = *((int*)pPixel);
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_RG
// Channel Data Type: CLK_SIGNED_INT32
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a int4 (r, g, 0, 1)
int4 __attribute__((overloadable)) load_pixel_RG_INT32(void* pPixel)
{
    int4 pixel = (int4)(0, 0, 0, 1);
    pixel.x = *((int*)pPixel);
    pixel.y = ((int*)pPixel)[1];
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_R & CL_Rx
// Channel Data Type: CLK_FLOAT
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a float4 (r, 0, 0, 1.0)
float4 __attribute__((overloadable)) load_pixel_R_FLOAT(void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 1.f);
    pixel.x = *((float*)pPixel);
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_RG
// Channel Data Type: CLK_FLOAT
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a float4 (r, g, 0, 1.0)
float4 __attribute__((overloadable)) load_pixel_RG_FLOAT(void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 1.f);
    pixel.x = *((float*)pPixel);
    pixel.y = ((float*)pPixel)[1];
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_A
// Channel Data Type: CLK_FLOAT
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a float4 (0, 0, 0, a)
float4 __attribute__((overloadable)) load_pixel_A_FLOAT(void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 1.f);
    pixel.w = *((float*)pPixel);
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_A
// Channel Data Type: CLK_HALF_FLOAT
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a uint4 (0, 0, 0, a)
float4 __attribute__((overloadable)) load_pixel_A_HALF_FLOAT(void* pPixel)
{
#ifdef __SSE4_1__
    half4 pix;
    pix.x = *(half*)pPixel;
    float4 val = (float4)Half4ToFloat4((short8)_mm_loadl_epi64((__m128i*)(&pix)));
#else
    float4 val = vloada_half4(0, (half*)pPixel);
#endif

    float4 pixel = (float4)(0.f, 0.f, 0.f, 1.f);
    pixel.w = val.x;
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_R & CL_Rx
// Channel Data Type: CLK_HALF_FLOAT
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a uint4 (r, 0, 0, 1.0)
float4 __attribute__((overloadable)) load_pixel_R_HALF_FLOAT(void* pPixel)
{
#ifdef __SSE4_1__
    half4 pix;
    pix.x = *(half*)pPixel;
    float4 val = (float4)Half4ToFloat4((short8)_mm_loadl_epi64((__m128i*)(&pix)));
#else
    float4 val = vloada_half4(0, (half*)pPixel);
#endif

    float4 pixel = (float4)(0.f, 0.f, 0.f, 1.f);
    pixel.x = val.x;
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_RG
// Channel Data Type: CLK_HALF_FLOAT
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a uint4 (r, g, 0, 1.0)
float4 __attribute__((overloadable)) load_pixel_RG_HALF_FLOAT(void* pPixel)
{
#ifdef __SSE4_1__
    half4 pix;
    pix.lo = *(half2*)pPixel;
    float4 val = (float4)Half4ToFloat4((short8)_mm_loadl_epi64((__m128i*)(&pix)));
#else
    float4 val = vloada_half4(0, (half*)pPixel);
#endif

    float4 pixel = (float4)(0.f, 0.f, 0.f, 1.f);
    pixel.lo = val.lo;
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_RGBA
// Channel Data Type: CLK_UNSIGNED_INT8
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a uint4 (r, g, b, a)
uint4 __attribute__((overloadable)) load_pixel_RGBA_UINT8(void* pPixel)
{
#ifdef __SSE4_1__
    char4 color = *(char4*)pPixel; // nevermind signed/unsigned.
    int4 converted = zext_v4i8_v4i32(color);
    return *(uint4*)&converted;
#else
    return convert_uint4(*((uchar4*)pPixel));
#endif
}


// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_R & CL_Rx
// Channel Data Type: CLK_UNSIGNED_INT8
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a uint4 (r, 0, 0, 1.0)
uint4 __attribute__((overloadable)) load_pixel_R_UINT8(void* pPixel)
{
    uint4 pixel = (uint4)(0, 0, 0, 1);
    pixel.x = (uint)(*((uchar*)pPixel));
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_RG
// Channel Data Type: CLK_UNSIGNED_INT8
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a uint4 (r, g, 0, 1.0)
uint4 __attribute__((overloadable)) load_pixel_RG_UINT8(void* pPixel)
{
    uint4 pixel = (uint4)(0, 0, 0, 1);
    pixel.x = (uint)(*((uchar*)pPixel));
    pixel.y = (uint)(((uchar*)pPixel)[1]);
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_R & CL_Rx
// Channel Data Type: CLK_UNSIGNED_INT16
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a uint4 (r, 0, 0, 1.0)
uint4 __attribute__((overloadable)) load_pixel_R_UINT16(void* pPixel)
{
    uint4 pixel = (uint4)(0, 0, 0, 1);
    pixel.x = (uint)( *((ushort*)pPixel) );
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_RG
// Channel Data Type: CLK_UNSIGNED_INT16
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a uint4 (r, g, 0, 1.0)
uint4 __attribute__((overloadable)) load_pixel_RG_UINT16(void* pPixel)
{
    uint4 pixel = (uint4)(0, 0, 0, 1);
    pixel.x = (uint)( *((ushort*)pPixel) );
    pixel.y = (uint)( (((ushort*)pPixel)[1]) );
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_R & CL_Rx
// Channel Data Type: CLK_SIGNED_INT8
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a int4 (r, 0, 0, 1.0)
int4 __attribute__((overloadable)) load_pixel_R_INT8(void* pPixel)
{
    int4 pixel = (int4)(0, 0, 0, 1);
    pixel.x = (int)(*((char*)pPixel));
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_RG
// Channel Data Type: CLK_SIGNED_INT8
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a int4 (r, g, 0, 1.0)
int4 __attribute__((overloadable)) load_pixel_RG_INT8(void* pPixel)
{
    int4 pixel = (int4)(0, 0, 0, 1);
    pixel.x = (int)(*((char*)pPixel));
    pixel.y = (int)(((char*)pPixel)[1]);
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_A
// Channel Data Type: CLK_UNORM_INT8
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a float4 (0, 0, 0, a)
float4 __attribute__((overloadable)) load_pixel_A_UNORM_INT8(void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 0.f);
    pixel.w = (float)(*((uchar*)pPixel));
    float4 converted = pixel*(float4)(1.0f/255.0f);
    return converted;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_A
// Channel Data Type: CLK_UNORM_INT16
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a float4 (0, 0, 0, a)
float4 __attribute__((overloadable)) load_pixel_A_UNORM_INT16(void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 0.f);
    pixel.w = (float)*((ushort*)pPixel);
    float4 converted = pixel*(float4)(1.0f/65535.0f);
    return converted;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_RG
// Channel Data Type: CLK_UNORM_INT8
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a float4 (r, g, 0, 1)
float4 __attribute__((overloadable)) load_pixel_RG_UNORM_INT8(void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 255.f); // Make the last value 255 to have 1 after conversion
    pixel.x = (float)(*((uchar*)pPixel));
    pixel.y = (float)(((uchar*)pPixel)[1]);
    float4 converted = pixel*(float4)(1.0f/255.0f);
    return converted;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_R & CL_Rx
// Channel Data Type: CLK_UNORM_INT8
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a float4 (r, 0, 0, 1)
float4 __attribute__((overloadable)) load_pixel_R_UNORM_INT8(void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 255.f); // Make the last value 255 to have 1 after conversion
    pixel.x = (float)(*((uchar*)pPixel));
    float4 converted = pixel*(float4)(1.0f/255.0f);
    return converted;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_RG
// Channel Data Type: CLK_UNORM_INT16
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a float4 (r, g, 0, 1)
float4 __attribute__((overloadable)) load_pixel_RG_UNORM_INT16(void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 65535.f); // Make the last value 65535 to have 1 after conversion
    pixel.x = (float)*((ushort*)pPixel);
    pixel.y = (float)(((ushort*)pPixel)[1]);
    float4 converted = pixel*(float4)(1.0f/65535.0f);
    return converted;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_R & CL_Rx
// Channel Data Type: CLK_UNORM_INT16
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a float4 (r, 0, 0, 1)
float4 __attribute__((overloadable)) load_pixel_R_UNORM_INT16(void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 65535.f); // Make the last value 65535 to have 1 after conversion
    pixel.x = (float)*((ushort*)pPixel);
    float4 converted = pixel*(float4)(1.0f/65535.0f);
    return converted;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_R & CL_Rx
// Channel Data Type: CLK_SIGNED_INT16
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a int4 (r, 0, 0, 1.0)
int4 __attribute__((overloadable)) load_pixel_R_INT16(void* pPixel)
{
    int4 pixel = (int4)(0, 0, 0, 1);
    pixel.x = (int)(*((short*)pPixel));
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_RG
// Channel Data Type: CLK_SIGNED_INT16
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a int4 (r, g, 0, 1.0)
int4 __attribute__((overloadable)) load_pixel_RG_INT16(void* pPixel)
{
    int4 pixel = (int4)(0, 0, 0, 1);
    pixel.x = (int)(*((short*)pPixel));
    pixel.y = (int)(((short*)pPixel)[1]);
    return pixel;
}

//
// Utility functions
//

// Extract the pointer to a specific pixel inside the image
//
// @param image: the image object
// @param coord: (x,y) coordinates of the pixel inside the image
//
// return: pointer to the begining of the pixel in memory
void* __attribute__((overloadable)) extract_pixel_pointer_quad(image2d_t image, int4 coord, void* pData)
{
    // Calculate required pixel offset
#ifdef __SSE4_1__
    int4 offset = (int4)_mm_load_si128((__m128i*)(&((image_aux_data*)image)->offset));
#else
    int4 offset=(int4)(0,0,0,0);
#endif
    int4 ocoord=coord*offset;
    void* pixel = pData + ocoord.x + ocoord.y + ocoord.z;
    return pixel;
}


// check if the coordinates are within the image boundaries
//
// @param image: the image object
// @param coord: (x,y) coordinates of the pixel
//
// return: nonzero if the image is out of bounds, otherwise 0
int __attribute__((overloadable)) isOutOfBoundsInt(image2d_t image, int4 coord)
{
#ifdef __SSE4_1__
    __m128i    i4up = _mm_load_si128((__m128i*)(((image_aux_data*)image)->dim));
    // Prepare mask for compare mask extraction
    int iMask=((image_aux_data*)image)->dimmask;
    __m128i iCoord = _mm_max_epi32((__m128i)coord, (__m128i)UndefCoordInt);
    iCoord = _mm_min_epi32(iCoord, i4up);
    __m128i isUp=_mm_cmpeq_epi32(iCoord,(__m128i)i4up);
    __m128i isLo=_mm_cmpeq_epi32(iCoord,(__m128i)UndefCoordInt);
    int iBorder=(_mm_movemask_epi8(isUp) | _mm_movemask_epi8(isLo)) & iMask;
    return iBorder;
#else
    int4 upper = vload4(0,((image_aux_data*)image)->dimSub1);
    int4 lower = (int4)(0,0,0 , 0);
    
    upper = upper < coord;
    lower = lower > coord;

    int4 isOOB = upper || lower;
    int result=isOOB.x || isOOB.y || isOOB.z;
    return result;
#endif
}

int4 __attribute__((overloadable)) ProjectToEdgeInt(image2d_t image, int4 coord)
{
#ifdef __SSE4_1__
    int4 upper = (int4)_mm_load_si128((__m128i*)(&((image_aux_data*)image)->dimSub1));
#else
    int4 upper=(int4)(0,0,0,0);
#endif
    int4 lower = (int4)(0, 0, 0, 0);
    
    int4 correctCoord=min(coord, upper);
    correctCoord=max(correctCoord,lower);
    return correctCoord;

}

float4 __attribute__((overloadable)) Unnormalize(image2d_t image,float4 coord)
{
#ifdef __SSE4_1__
    float4 fupper = _mm_load_ps((float*)(&((image_aux_data*)image)->dimf));
#else
    int4 upper=(int4)(0,0,0,0);
    float4 fupper=convert_float4(upper);
#endif
    return fupper*coord;
}

    
//the coordinate here should be unnormalized already
int4 __attribute__((overloadable)) ProjectNearest(float4 coord)
{
    return convert_int4(floor(coord));
}

float4 __attribute__((overloadable)) frac(float4 coord)
{
    return coord-floor(coord);
}

/// components contain values (00, 10, 01, 11)
float4 SampleImage2DFloatCh1(float4 components, float4 frac)
{
    float a = frac.x;
    float b = frac.y;

    float4 consts1 = (float4)(1.f, 0.0f, 1.f, 0.f);
    float4 consts2 = (float4)(1.f, 1.0f, 0.f, 0.f);
    float4 comp1 = (float4)(-a, a, -a, a);
    float4 comp2 = (float4)(-b, -b, b, b);
#ifdef __SSE4_1__
    __m128 vec1 = _mm_add_ps(consts1, comp1);
    __m128 vec2 = _mm_add_ps(consts2, comp2);

    __m128 res = _mm_mul_ps(vec1, vec2);
    res = _mm_mul_ps(components, res);
    
    /// sum vector elements
    res = _mm_hadd_ps(res, res);
    res = _mm_hadd_ps(res, res);

    return res;
#else
    float4 vec1 = consts1 + comp1;
    float4 vec2 = consts2 + comp2;

    float4 res = vec1 * vec2;
    res = components * res;

/// sum vector elements
    float sum = res.x + res.y + res.z + res.w;

    return (float4)(sum, sum, sum, sum);
#endif

}


float4 SampleImage1DFloat(float4 Ti0, float4 Ti1, float4 frac)
{
    float a = frac.x;

    return (1 - a) * Ti0 + a * Ti1;
}

float4 SampleImage2DFloat(float4 Ti0j0, float4 Ti1j0, float4 Ti0j1, float4 Ti1j1, float4 frac)
{
    float a = frac.x;
    float b = frac.y;

    return (1 - a) * (1 - b) * Ti0j0
        + a * (1 - b) * Ti1j0
        + (1 - a) * b * Ti0j1
        + a * b * Ti1j1;
}

float4 SampleImage3DFloat(float4 Ti0j0k0, float4 Ti1j0k0, float4 Ti0j1k0, float4 Ti1j1k0, float4 Ti0j0k1, float4 Ti1j0k1, float4 Ti0j1k1, float4 Ti1j1k1, float4 frac)
{
    float a = frac.x;
    float b = frac.y;
    float c = frac.z;

    return (1 - a) * (1 - b) * (1 - c) * Ti0j0k0
        + a * (1 - b) * (1 - c) * Ti1j0k0
        + (1 - a) * b * (1 - c) * Ti0j1k0
        + a * b * (1 - c) * Ti1j1k0
        + (1 - a) * (1 - b) * c * Ti0j0k1
        + a * (1 - b) * c * Ti1j0k1
        + (1 - a) * b * c * Ti0j1k1
        + a * b * c * Ti1j1k1;
}


//general functions

uint4 __attribute__((overloadable)) read_sample_UNDEFINED_QUAD(image2d_t image, int4 coord, void* pData)
{
    return BorderColorNoAlphaUint;  //a don't care result
}

#ifdef __cplusplus
}
#endif

#endif // defined (__MIC__) || defined(__MIC2__)

