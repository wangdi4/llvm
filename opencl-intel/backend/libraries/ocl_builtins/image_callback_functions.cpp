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


// Enable double support. It is needed for declarations from intrin.h
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#define __OPENCL__
#include <intrin.h>

#include "cl_image_declaration.h"
#include "GENERIC/ll_intrinsics.h"
#include "transpose_functions.h"

#define SHRT16_MIN    (-32768)
#define SHRT16_MAX      32767
#define SNORM_INT16_FACTOR 32767.f
#define SNORM_INT8_FACTOR  127.f
#define UNORM_INT16_FACTOR 65535.f
#define UNORM_INT8_FACTOR  255.f

// Clamp border color used for CL_A, CL_INTENSITY, CL_Rx, CL_RA, CL_RGx, CL_RGBx, CL_ARGB, CL_BGRA, CL_RGBA
ALIGN16 const constant float4 BorderColorNoAlphaFloat = {0.0f, 0.0f, 0.0f, 0.0f};
ALIGN16 const constant int4 BorderColorNoAlphaInt = {0, 0, 0, 0};
ALIGN16 const constant uint4 BorderColorNoAlphaUint = {0, 0, 0, 0};
ALIGN16 const constant float4 halfhalfhalfzero = {0.5f, 0.5f, 0.5f, 0.0f};
ALIGN16 const constant float4 f4half = {0.5f, 0.5f, 0.5f, 0.5f};
ALIGN16 const constant float4 f4two = {2.f, 2.f, 2.f, 2.f};
/// Minimal representative float. It is represented as zero mantissa
/// and exponenta with only last bit set to one
ALIGN16 const constant int4 oneOneOneZero = {1, 1, 1, 0};
ALIGN16 const constant int4   int4AllZeros = {0, 0, 0, 0};
ALIGN16 const constant int4   int4MinusOnes= {-1,-1,-1,-1};
ALIGN16 const constant float4 float4AllZeros = {0.f ,0.f , 0.f, 0.f};
ALIGN16 const constant float4 f4Unorm8Dim = {(float)(1./255.), (float)(1./255.), (float)(1./255.), (float)(1./255.)};
ALIGN16 const constant float4 f4Unorm16Dim = {(float)(1./65535.), (float)(1./65535.), (float)(1./65535.), (float)(1./65535.)};
ALIGN16 const constant float4 f4unorm16mul = {65535.f, 65535.f, 65535.f, 65535.f};
ALIGN16 const constant float4 f4unorm16lim = {0.f, 0.f, 0.f, 0.f};
ALIGN16 const constant float4 f4unorm8mul = {255.f, 255.f, 255.f, 255.f};
ALIGN16 const constant float4 f4unorm8lim = {0.0f, 0.0f, 0.0f, 0.0f};
ALIGN16 const constant int4 i4int16Min = {SHRT16_MIN, SHRT16_MIN, SHRT16_MIN, SHRT16_MIN};
ALIGN16 const constant int4 i4int16Max = {SHRT16_MAX, SHRT16_MAX, SHRT16_MAX, SHRT16_MAX};
ALIGN16 const constant uint4 i4uint16Max = {USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX};

// Clamp Border color used for CL_R, CL_RG, CL_RGB, CL_LUMINANCE
ALIGN16 const constant float4 BorderColorAlphaFloat = {0.0f, 0.0f, 0.0f, 1.0f};
ALIGN16 const constant int4 BorderColorAlphaInt = {0, 0, 0, 1};

ALIGN16 const constant uint4 BorderColorAlphaUint = {0, 0, 0, 1};
ALIGN16 const constant float f4SignMask[] = {-0.f, -0.f, -0.f, -0.f};

// utility functions declarations
int isOutOfBoundsInt(image2d_t image, int4 coord);
int4 ProjectToEdgeInt(image2d_t image, int4 coord);
float4 Unnormalize(image2d_t image,float4 coord);
int4 ProjectNearest(float4 coord);
float4 frac(float4 coord);

__private void* extract_pixel_pointer_quad(image2d_t image, int4 coord, __private void* pData);

uint4 load_pixel_RGBA_UNSIGNED_INT8(__private void* pPixel);
uint4 load_pixel_RGBA_UNSIGNED_INT16(__private void* pPixel);
uint4 load_pixel_RGBA_UNSIGNED_INT32(__private void* pPixel);

int4 load_pixel_RGBA_SIGNED_INT8(__private void* pPixel);
int4 load_pixel_RGBA_SIGNED_INT16(__private void* pPixel);
int4 load_pixel_RGBA_SIGNED_INT32(__private void* pPixel);

float load_value_INTENSITY_FLOAT(__private void* pPixel);
float load_value_INTENSITY_UNORM_INT8(__private void* pPixel);
float load_value_INTENSITY_UNORM_INT16(__private void* pPixel);
float load_value_INTENSITY_HALF_FLOAT(__private void* pPixel);
float load_value_LUMINANCE_FLOAT(__private void* pPixel);
float load_value_LUMINANCE_UNORM_INT8(__private void* pPixel);
float load_value_LUMINANCE_UNORM_INT16(__private void* pPixel);
float load_value_LUMINANCE_HALF_FLOAT(__private void* pPixel);
float4 load_pixel_INTENSITY_FLOAT(__private void* pPixel);
float4 load_pixel_INTENSITY_UNORM_INT8(__private void* pPixel);
float4 load_pixel_INTENSITY_UNORM_INT16(__private void* pPixel);
float4 load_pixel_INTENSITY_HALF_FLOAT(__private void* pPixel);
float4 load_pixel_LUMINANCE_FLOAT(__private void* pPixel);
float4 load_pixel_LUMINANCE_UNORM_INT8(__private void* pPixel);
float4 load_pixel_LUMINANCE_UNORM_INT16(__private void* pPixel);
float4 load_pixel_LUMINANCE_HALF_FLOAT(__private void* pPixel);
float4 load_pixel_RGBA_HALF_FLOAT(__private void* pPixel);
float4 load_pixel_RGBA_FLOAT(__private void* pPixel);

float4 load_pixel_BGRA_UNORM_INT8(__private void* pPixel);
float4 load_pixel_RGBA_UNORM_INT8(__private void* pPixel);
float4 load_pixel_RGBA_UNORM_INT16(__private void* pPixel);
float4 load_pixel_RGBA_SNORM_INT8(__private void* pPixel);
float4 load_pixel_RGBA_SNORM_INT16(__private void* pPixel);

float4 load_pixel_sRGBA_UNORM_INT8(__private void* pPixel);
float4 load_pixel_sBGRA_UNORM_INT8(__private void* pPixel);

int4 load_pixel_R_SIGNED_INT8(__private void* pPixel);
int4 load_pixel_R_SIGNED_INT16(__private void* pPixel);
int4 load_pixel_R_SIGNED_INT32(__private void* pPixel);
float4 load_pixel_R_FLOAT(__private void* pPixel);
float4 load_pixel_R_HALF_FLOAT(__private void* pPixel);
uint4 load_pixel_R_UNSIGNED_INT8(__private void* pPixel);
uint4 load_pixel_R_UNSIGNED_INT16(__private void* pPixel);
uint4 load_pixel_R_UNSIGNED_INT32(__private void* pPixel);
float4 load_pixel_R_UNORM_INT8(__private void* pPixel);
float4 load_pixel_R_UNORM_INT16(__private void* pPixel);
float4 load_pixel_R_SNORM_INT8(__private void* pPixel);
float4 load_pixel_R_SNORM_INT16(__private void* pPixel);

float4 load_pixel_DEPTH_FLOAT(__private void* pPixel);
float4 load_pixel_DEPTH_UNORM_INT16(__private void* pPixel);

float4 load_pixel_A_FLOAT(__private void* pPixel);
float4 load_pixel_A_UNORM_INT8(__private void* pPixel);
float4 load_pixel_A_UNORM_INT16(__private void* pPixel);
float4 load_pixel_A_HALF_FLOAT(__private void* pPixel);

uint4 load_pixel_RG_UNSIGNED_INT8(__private void* pPixel);
uint4 load_pixel_RG_UNSIGNED_INT16(__private void* pPixel);
uint4 load_pixel_RG_UNSIGNED_INT32(__private void* pPixel);
int4 load_pixel_RG_SIGNED_INT8(__private void* pPixel);
int4 load_pixel_RG_SIGNED_INT16(__private void* pPixel);
int4 load_pixel_RG_SIGNED_INT32(__private void* pPixel);
float4 load_pixel_RG_FLOAT(__private void* pPixel);
float4 load_pixel_RG_UNORM_INT8(__private void* pPixel);
float4 load_pixel_RG_UNORM_INT16(__private void* pPixel);
float4 load_pixel_RG_SNORM_INT8(__private void* pPixel);
float4 load_pixel_RG_SNORM_INT16(__private void* pPixel);
float4 load_pixel_RG_HALF_FLOAT(__private void* pPixel);

float4 SampleImage1DFloat(float4 Ti0, float4 Ti1, float4 frac);

float4 SampleImage2DFloat(float4 Ti0j0, float4 Ti1j0, float4 Ti0j1, float4 Ti1j1, float4 frac);
float4 SampleImage2DFloatCh1(float4 components, float4 frac);

float4 SampleImage3DFloat(float4 Ti0j0k0, float4 Ti1j0k0, float4 Ti0j1k0, float4 Ti1j1k0, float4 Ti0j0k1, float4 Ti1j0k1, float4 Ti0j1k1, float4 Ti1j1k1, float4 frac);

#define _mm_abs_ps(X)    _mm_andnot_ps(_mm_load_ps((const float*)f4SignMask),X)

ALIGN16 const constant short  Fvec8Float16ExponentMask[] = {0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00};
ALIGN16 const constant short  Fvec8Float16MantissaMask[] = {0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF};
ALIGN16 const constant short  Fvec8Float16SignMask[]     = {0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000};
ALIGN16 const constant int Fvec4Float32ExponentMask[] = {0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000};
ALIGN16 const constant int Fvec4Float32NanMask[] = {0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000};
ALIGN16 const constant int Fvec4Float16NaNExpMask[]   = {0x7C00, 0x7C00, 0x7C00, 0x7C00};
ALIGN16 const constant int FVec4Float16Implicit1Mask[] = {(1<<10), (1<<10), (1<<10), (1<<10)};
ALIGN16 const constant int Fvec4Float16ExpMin[] = {(1<<10), (1<<10), (1<<10), (1<<10)};
ALIGN16 const constant int Fvec4Float16BiasDiffDenorm[] = {((127 - 15 - 10) << 23), ((127 - 15 - 10) << 23), ((127 - 15 - 10) << 23), ((127 - 15 - 10) << 23)};
ALIGN16 const constant int Fvec4Float16ExpBiasDifference[] = {((127 - 15) << 10), ((127 - 15) << 10), ((127 - 15) << 10), ((127 - 15) << 10)};
ALIGN16 const constant int f4minNorm[] = {0x00800000, 0x00800000, 0x00800000, 0x00800000};
ALIGN16 const constant int mth_signMask[] = {0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF};

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


///////// SOA8 auxiliary image functions ///////////////

// check if the coordinates are within the image boundaries
//
// @param image: the image object
// @param coord: (x,y) coordinates of pixels
//
// return: vector, that contains all zeros if corresponding pixel is outside the boundaries
//         or all ones otherwise
int8 __attribute__((overloadable)) soa8_isInsideBoundsInt(image2d_t image, int8 coord_x, int8 coord_y)
{
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*);
    int8 upper_x = (int8)(pImage->dimSub1[0]);
    int8 upper_y = (int8)(pImage->dimSub1[1]);
    int8 lower_x = (int8)(0,0,0,0,0,0,0,0);
    int8 lower_y = (int8)(0,0,0,0,0,0,0,0);

    upper_x = upper_x >= coord_x;
    lower_x = lower_x <= coord_x;
    upper_y = upper_y >= coord_y;
    lower_y = lower_y <= coord_y;

    int8 res = upper_x & upper_y & lower_x & lower_y;

    return res;
}

// Extract the pointer to a specific pixels inside the image
//
// @param image: the image object
// @param coord: (x,y) coordinates of the pixel inside the image
//
// return: pointer to the begining of the pixel in memory
void __attribute__((overloadable)) soa8_extract_pixel_pointer_quad(image2d_t image, int8 coord_x, int8 coord_y, __private void* pData,
                                   __private void** p0, __private void** p1, __private void** p2, __private void** p3, __private void** p4, __private void** p5, __private void** p6, __private void** p7)
{
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*);
    uint8 offset_x = (uint8)(pImage->offset[0]);
    uint8 offset_y = (uint8)(pImage->offset[1]);

    uint8 ocoord_x = (as_uint8(coord_x)) * offset_x;
    uint8 ocoord_y = (as_uint8(coord_y)) * offset_y;

    uint8 ocoord = ocoord_x + ocoord_y;
    *p0 = (__private char*)pData + ocoord.s0;
    *p1 = (__private char*)pData + ocoord.s1;
    *p2 = (__private char*)pData + ocoord.s2;
    *p3 = (__private char*)pData + ocoord.s3;
    *p4 = (__private char*)pData + ocoord.s4;
    *p5 = (__private char*)pData + ocoord.s5;
    *p6 = (__private char*)pData + ocoord.s6;
    *p7 = (__private char*)pData + ocoord.s7;
    return;
}

void __attribute__((overloadable)) soa8_load_pixel_RGBA_UNSIGNED_INT8(__private void* p0, __private void* p1, __private void* p2, __private void* p3, __private void* p4, __private void* p5, __private void* p6, __private void* p7, 
                                                              __private uint8* res_x, __private uint8* res_y, __private uint8* res_z, __private uint8* res_w)
{
    uchar8 color_x, color_y, color_z, color_w; // nevermind signed/unsigned.
    __ocl_gather_transpose_char_4x8( p0, p1, p2, p3, p4, p5, p6, p7,
        (__private char8*)&color_x, (__private char8*)&color_y, (__private char8*)&color_z, (__private char8*)&color_w);
    *res_x = convert_uint8(color_x);
    *res_y = convert_uint8(color_y);
    *res_z = convert_uint8(color_z);
    *res_w = convert_uint8(color_w);
}

void __attribute__((overloadable)) soa8_load_pixel_RGBA_UNSIGNED_INT8_oob(int8 isNotOOB, __private void* p0, __private void* p1, __private void* p2, __private void* p3, __private void* p4, __private void* p5, __private void* p6, __private void* p7, 
                                                              __private uint8* res_x, __private uint8* res_y, __private uint8* res_z, __private uint8* res_w)
{
    uchar8 color_x, color_y, color_z, color_w; // nevermind signed/unsigned.
    __ocl_masked_gather_transpose_char_4x8( p0, p1, p2, p3, p4, p5, p6, p7,
        (__private char8*)&color_x, (__private char8*)&color_y, (__private char8*)&color_z, (__private char8*)&color_w, isNotOOB);

    *res_x = isNotOOB ? convert_uint8(color_x) : (uint8)BorderColorNoAlphaUint.x ;
    *res_y = isNotOOB ? convert_uint8(color_y) : (uint8)BorderColorNoAlphaUint.y ;
    *res_z = isNotOOB ? convert_uint8(color_z) : (uint8)BorderColorNoAlphaUint.z ;
    *res_w = isNotOOB ? convert_uint8(color_w) : (uint8)BorderColorNoAlphaUint.w ;
}

///////// SOA4 auxiliary image functions ///////////////
// check if the coordinates are within the image boundaries
//
// @param image: the image object
// @param coord: (x,y) coordinates of pixels
//
// return: vector, that contains all zeros if corresponding pixel is outside the boundaries
//         or all ones otherwise
int4 __attribute__((overloadable)) soa4_isInsideBoundsInt(image2d_t image, int4 coord_x, int4 coord_y)
{
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*);
    int4 upper_x = (int4)(pImage->dimSub1[0]);
    int4 upper_y = (int4)(pImage->dimSub1[1]);
    int4 lower_x = (int4)(0,0,0,0);
    int4 lower_y = (int4)(0,0,0,0);

    upper_x = upper_x >= coord_x;
    lower_x = lower_x <= coord_x;
    upper_y = upper_y >= coord_y;
    lower_y = lower_y <= coord_y;

    int4 res = upper_x & upper_y & lower_x & lower_y;

    return res;
}

// Extract the pointer to a specific pixel inside the image
//
// @param image: the image object
// @param coord: (x,y) coordinates of the pixel inside the image
//
// return: pointer to the begining of the pixel in memory
void __attribute__((overloadable)) soa4_extract_pixel_pointer_quad(image2d_t image, int4 coord_x, int4 coord_y, __private void* pData, __private void** p1, __private void** p2, __private void** p3, __private void** p4)
{
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*);
    uint4 offset_x = (uint4)(pImage->offset[0]);
    uint4 offset_y = (uint4)(pImage->offset[1]);

    uint4 ocoord_x = (as_uint4(coord_x)) * offset_x;
    uint4 ocoord_y = (as_uint4(coord_y)) * offset_y;

    uint4 ocoord = ocoord_x + ocoord_y;
    *p1 = (__private char*)pData + ocoord.s0;
    *p2 = (__private char*)pData + ocoord.s1;
    *p3 = (__private char*)pData + ocoord.s2;
    *p4 = (__private char*)pData + ocoord.s3;
    return;
}

void __attribute__((overloadable)) soa4_load_pixel_RGBA_UNSIGNED_INT8(__private void* pPixel_0, __private void* pPixel_1, __private void* pPixel_2, __private void* pPixel_3, 
                                                              __private uint4* res_x, __private uint4* res_y, __private uint4* res_z, __private uint4* res_w)
{
    uchar4 color_x, color_y, color_z, color_w; // nevermind signed/unsigned.
    __ocl_gather_transpose_char_4x4( pPixel_0, pPixel_1, pPixel_2, pPixel_3,
        (__private char4*)&color_x, (__private char4*)&color_y,
        (__private char4*)&color_z, (__private char4*)&color_w);
    *res_x = convert_uint4(color_x);
    *res_y = convert_uint4(color_y);
    *res_z = convert_uint4(color_z);
    *res_w = convert_uint4(color_w);
}

void __attribute__((overloadable)) soa4_load_pixel_RGBA_UNSIGNED_INT8_oob(int4 isNotOOB, __private void* pPixel_0, __private void* pPixel_1, __private void* pPixel_2, __private void* pPixel_3, 
                                                              __private uint4* res_x, __private uint4* res_y, __private uint4* res_z, __private uint4* res_w)
{
    uchar4 color_x, color_y, color_z, color_w; // nevermind signed/unsigned.
    __ocl_masked_gather_transpose_char_4x4( pPixel_0, pPixel_1, pPixel_2, pPixel_3,
        (__private char4*)&color_x, (__private char4*)&color_y,
        (__private char4*)&color_z, (__private char4*)&color_w, isNotOOB);
    *res_x = isNotOOB ? convert_uint4(color_x) : (uint4)BorderColorNoAlphaUint.x ;
    *res_y = isNotOOB ? convert_uint4(color_y) : (uint4)BorderColorNoAlphaUint.y ;
    *res_z = isNotOOB ? convert_uint4(color_z) : (uint4)BorderColorNoAlphaUint.z ;
    *res_w = isNotOOB ? convert_uint4(color_w) : (uint4)BorderColorNoAlphaUint.w ;
}

// macro to unroll scalarized SOA calls
#define IMAGE_SOA_MAKE_SEQ1(FUNC)\
  FUNC; cnt++;
#define IMAGE_SOA_MAKE_SEQ2(FUNC)\
  IMAGE_SOA_MAKE_SEQ1(FUNC)\
  FUNC; cnt++;
#define IMAGE_SOA_MAKE_SEQ3(FUNC)\
  IMAGE_SOA_MAKE_SEQ2(FUNC)\
  FUNC; cnt++;
#define IMAGE_SOA_MAKE_SEQ4(FUNC)\
  IMAGE_SOA_MAKE_SEQ3(FUNC)\
  FUNC; cnt++;
#define IMAGE_SOA_MAKE_SEQ5(FUNC)\
  IMAGE_SOA_MAKE_SEQ4(FUNC)\
  FUNC; cnt++;
#define IMAGE_SOA_MAKE_SEQ6(FUNC)\
  IMAGE_SOA_MAKE_SEQ5(FUNC)\
  FUNC; cnt++;
#define IMAGE_SOA_MAKE_SEQ7(FUNC)\
  IMAGE_SOA_MAKE_SEQ6(FUNC)\
  FUNC; cnt++;
#define IMAGE_SOA_MAKE_SEQ8(FUNC)\
  IMAGE_SOA_MAKE_SEQ7(FUNC)\
  FUNC; cnt++;

// macro to scalarize SOA calls
#define SCALARIZE_SOA_CBK(FORMAT, FILTER_TYPE, CLAMP_FLAG, PIX_TYPE, NSOA, COORD_TYPE)\
    PIX_TYPE##4 read_sample_##FILTER_TYPE##_##CLAMP_FLAG##_##FORMAT(image2d_t, COORD_TYPE##4, __private void*);\
    void soa##NSOA##_read_sample_##FILTER_TYPE##_##CLAMP_FLAG##_##FORMAT( image2d_t image, COORD_TYPE##NSOA coord_x, COORD_TYPE##NSOA coord_y, __private void* pData,\
                  __private PIX_TYPE##NSOA* res_x, __private PIX_TYPE##NSOA* res_y, __private PIX_TYPE##NSOA* res_z, __private PIX_TYPE##NSOA* res_w )\
{\
    COORD_TYPE *lcoord_x = (COORD_TYPE *)&coord_x, *lcoord_y = (COORD_TYPE *)&coord_y;\
    PIX_TYPE *lpix_x=(PIX_TYPE *)res_x, *lpix_y=(PIX_TYPE *)res_y,\
             *lpix_z=(PIX_TYPE *)res_z, *lpix_w=(PIX_TYPE *)res_w;\
    int cnt = 0;\
    IMAGE_SOA_MAKE_SEQ##NSOA({\
         COORD_TYPE##4 vcoord = 0;\
         vcoord.x = lcoord_x[cnt];\
         vcoord.y = lcoord_y[cnt];\
         image_aux_data *pImage = __builtin_astype(image, image_aux_data*);\
         PIX_TYPE##4 pixval = \
           read_sample_##FILTER_TYPE##_##CLAMP_FLAG##_##FORMAT(\
           image, vcoord, pImage->pData);\
         lpix_x[cnt] = pixval.x;\
         lpix_y[cnt] = pixval.y;\
         lpix_z[cnt] = pixval.z;\
         lpix_w[cnt] = pixval.w;\
    })\
}

/// SOA8 reading functions
#define IMPLEMENT_SOA8_CBK_NEAREST_NO_CLAMP(FORMAT, RETURN_TYPE)\
    void soa8_read_sample_NEAREST_NO_CLAMP_##FORMAT( image2d_t image, int8 coord_x, int8 coord_y, __private void* pData,\
                                                     __private RETURN_TYPE* res_x, __private RETURN_TYPE* res_y, __private RETURN_TYPE* res_z, __private RETURN_TYPE* res_w )\
{\
    __private void *p0, *p1, *p2, *p3, *p4, *p5, *p6, *p7;\
    soa8_extract_pixel_pointer_quad(image, coord_x, coord_y, pData, &p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7);\
    soa8_load_pixel_##FORMAT(p0, p1, p2, p3, p4, p5, p6, p7, res_x, res_y, res_z, res_w);\
}

#define IMPLEMENT_SOA8_CBK_NEAREST_CLAMP(FORMAT, RETURN_TYPE)\
    void soa8_read_sample_NEAREST_CLAMP_##FORMAT( image2d_t image, int8 coord_x, int8 coord_y, __private void* pData,\
                                                  __private RETURN_TYPE* res_x, __private RETURN_TYPE* res_y, __private RETURN_TYPE* res_z, __private RETURN_TYPE* res_w )\
{\
    int8 isNotOOB = soa8_isInsideBoundsInt(image, coord_x, coord_y);\
    __private void *p0, *p1, *p2, *p3, *p4, *p5, *p6, *p7;\
    soa8_extract_pixel_pointer_quad(image, coord_x & isNotOOB, coord_y & isNotOOB, pData, &p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7);\
    soa8_load_pixel_##FORMAT##_oob(isNotOOB, p0, p1, p2, p3, p4, p5, p6, p7, res_x, res_y, res_z, res_w);\
}

IMPLEMENT_SOA8_CBK_NEAREST_CLAMP(RGBA_UNSIGNED_INT8, uint8)
IMPLEMENT_SOA8_CBK_NEAREST_NO_CLAMP(RGBA_UNSIGNED_INT8, uint8)
SCALARIZE_SOA_CBK(RGBA_UNSIGNED_INT16, NEAREST, CLAMP, uint, 8, int)
SCALARIZE_SOA_CBK(RGBA_UNSIGNED_INT16, NEAREST, NO_CLAMP, uint, 8, int)
SCALARIZE_SOA_CBK(RGBA_UNSIGNED_INT32, NEAREST, CLAMP, uint, 8, int)
SCALARIZE_SOA_CBK(RGBA_UNSIGNED_INT32, NEAREST, NO_CLAMP, uint, 8, int)
SCALARIZE_SOA_CBK(R_UNSIGNED_INT8, NEAREST, CLAMP, uint, 8, int)
SCALARIZE_SOA_CBK(R_UNSIGNED_INT8, NEAREST, NO_CLAMP, uint, 8, int)
SCALARIZE_SOA_CBK(R_UNSIGNED_INT16, NEAREST, CLAMP, uint, 8, int)
SCALARIZE_SOA_CBK(R_UNSIGNED_INT16, NEAREST, NO_CLAMP, uint, 8, int)
SCALARIZE_SOA_CBK(R_UNSIGNED_INT32, NEAREST, NO_CLAMP, uint, 8, int)
SCALARIZE_SOA_CBK(R_UNSIGNED_INT32, NEAREST, CLAMP, uint, 8, int)
SCALARIZE_SOA_CBK(RG_UNSIGNED_INT8, NEAREST, CLAMP, uint, 8, int)
SCALARIZE_SOA_CBK(RG_UNSIGNED_INT8, NEAREST, NO_CLAMP, uint, 8, int)
SCALARIZE_SOA_CBK(RG_UNSIGNED_INT16, NEAREST, CLAMP, uint, 8, int)
SCALARIZE_SOA_CBK(RG_UNSIGNED_INT16, NEAREST, NO_CLAMP, uint, 8, int)
SCALARIZE_SOA_CBK(RG_UNSIGNED_INT32, NEAREST, CLAMP, uint, 8, int)
SCALARIZE_SOA_CBK(RG_UNSIGNED_INT32, NEAREST, NO_CLAMP, uint, 8, int)


/// SOA4 reading functions
#define IMPLEMENT_SOA4_CBK_NEAREST_NO_CLAMP(FORMAT, RETURN_TYPE)\
    void soa4_read_sample_NEAREST_NO_CLAMP_##FORMAT( image2d_t image, int4 coord_x, int4 coord_y, __private void* pData,\
                                                     __private RETURN_TYPE* res_x, __private RETURN_TYPE* res_y, __private RETURN_TYPE* res_z, __private RETURN_TYPE* res_w )\
{\
    __private void *p0, *p1, *p2, *p3;\
    soa4_extract_pixel_pointer_quad(image, coord_x, coord_y, pData, &p0, &p1, &p2, &p3);\
    soa4_load_pixel_##FORMAT(p0, p1, p2, p3, res_x, res_y, res_z, res_w);\
}

#define IMPLEMENT_SOA4_CBK_NEAREST_CLAMP(FORMAT, RETURN_TYPE)\
    void soa4_read_sample_NEAREST_CLAMP_##FORMAT( image2d_t image, int4 coord_x, int4 coord_y, __private void* pData,\
                                                  __private RETURN_TYPE* res_x, __private RETURN_TYPE* res_y, __private RETURN_TYPE* res_z, __private RETURN_TYPE* res_w )\
{\
    int4 isNotOOB = soa4_isInsideBoundsInt(image, coord_x, coord_y);\
    __private void *p0, *p1, *p2, *p3;\
    soa4_extract_pixel_pointer_quad(image, coord_x & isNotOOB, coord_y & isNotOOB, pData, &p0, &p1, &p2, &p3);\
    soa4_load_pixel_##FORMAT##_oob(isNotOOB, p0, p1, p2, p3, res_x, res_y, res_z, res_w);\
}

IMPLEMENT_SOA4_CBK_NEAREST_CLAMP(RGBA_UNSIGNED_INT8, uint4)
IMPLEMENT_SOA4_CBK_NEAREST_NO_CLAMP(RGBA_UNSIGNED_INT8, uint4)
SCALARIZE_SOA_CBK(RGBA_UNSIGNED_INT16, NEAREST, CLAMP, uint, 4, int)
SCALARIZE_SOA_CBK(RGBA_UNSIGNED_INT16, NEAREST, NO_CLAMP, uint, 4, int)
SCALARIZE_SOA_CBK(RGBA_UNSIGNED_INT32, NEAREST, CLAMP, uint, 4, int)
SCALARIZE_SOA_CBK(RGBA_UNSIGNED_INT32, NEAREST, NO_CLAMP, uint, 4, int)
SCALARIZE_SOA_CBK(R_UNSIGNED_INT8, NEAREST, CLAMP, uint, 4, int)
SCALARIZE_SOA_CBK(R_UNSIGNED_INT8, NEAREST, NO_CLAMP, uint, 4, int)
SCALARIZE_SOA_CBK(R_UNSIGNED_INT16, NEAREST, CLAMP, uint, 4, int)
SCALARIZE_SOA_CBK(R_UNSIGNED_INT16, NEAREST, NO_CLAMP, uint, 4, int)
SCALARIZE_SOA_CBK(R_UNSIGNED_INT32, NEAREST, CLAMP, uint, 4, int)
SCALARIZE_SOA_CBK(R_UNSIGNED_INT32, NEAREST, NO_CLAMP, uint, 4, int)
SCALARIZE_SOA_CBK(RG_UNSIGNED_INT8, NEAREST, CLAMP, uint, 4, int)
SCALARIZE_SOA_CBK(RG_UNSIGNED_INT8, NEAREST, NO_CLAMP, uint, 4, int)
SCALARIZE_SOA_CBK(RG_UNSIGNED_INT16, NEAREST, CLAMP, uint, 4, int)
SCALARIZE_SOA_CBK(RG_UNSIGNED_INT16, NEAREST, NO_CLAMP, uint, 4, int)
SCALARIZE_SOA_CBK(RG_UNSIGNED_INT32, NEAREST, CLAMP, uint, 4, int)
SCALARIZE_SOA_CBK(RG_UNSIGNED_INT32, NEAREST, NO_CLAMP, uint, 4, int)


/************************Float coordinate translations*************************************/

int4 trans_coord_float_NONE_FALSE_NEAREST(image2d_t image, float4 coord)
{
    //not testing if coords are OOB - this mode doesn't guarantee safeness!
    return ProjectNearest(coord);
}

int4 trans_coord_float_CLAMP_TO_EDGE_FALSE_NEAREST(image2d_t image, float4 coord)
{
    return ProjectToEdgeInt(image,ProjectNearest(coord));
}

int4 trans_coord_float_UNDEFINED(image2d_t image, float4 coord)
{
    return int4AllZeros;   //will be background color, but it's a "don't care" situtation
}

int4 trans_coord_float_NONE_TRUE_NEAREST(image2d_t image, float4 coord)
{
    //not testing if coords are OOB - this mode doesn't guarantee safeness!
    int4 result=ProjectNearest(Unnormalize(image, coord));
    return result;
}

int4 trans_coord_float_CLAMP_TO_EDGE_TRUE_NEAREST(image2d_t image, float4 coord)
{
    int4 result=ProjectToEdgeInt(image, ProjectNearest(Unnormalize(image, coord)));
    return result;
}

int4 trans_coord_float_REPEAT_TRUE_NEAREST(image2d_t image, float4 coord)
{
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*);
    int4 upper = as_int4(_mm_load_si128((__m128i*)(&pImage->dimSub1)));
    int4 urcoord = ProjectNearest(Unnormalize(image, coord-floor(coord)));  //unrepeated coords

    urcoord = urcoord <= upper ? urcoord : 0;

    return urcoord;
}

int4 trans_coord_float_MIRRORED_REPEAT_TRUE_NEAREST(image2d_t image, float4 coord)
{
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*);
    int4 upper = as_int4(_mm_load_si128((__m128i*)(&pImage->dimSub1)));
    __m128 isZero = _mm_cmpeq_ps((__m128)coord, float4AllZeros);
    __m128 mcoord = as_float4(_mm_sub_epi32((__m128i)coord, *((__m128i*)f4minNorm)));
    mcoord= rint((__m128)mcoord);
    mcoord = (__m128)_mm_add_epi32((__m128i)mcoord, *((__m128i*)f4minNorm));
    /// Set to zero coordinates that were equal to zero before
    /// multiplications
    mcoord = (__m128)_mm_andnot_si128((__m128i)isZero, (__m128i)mcoord);
    mcoord = (__m128)_mm_sub_ps((__m128)mcoord, (__m128)coord);
    mcoord=fabs(mcoord);
    int4 urcoord = ProjectNearest(Unnormalize(image, as_float4(mcoord)));  //unrepeated coords
    urcoord=min(urcoord,upper);
    return urcoord;
}


/***********************float to float images translation functions (which accept and return [square0, square1] coordinates******************/

float4 trans_coord_float_float_NONE_FALSE_NEAREST(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    //not testing if coords are OOB - this mode doesn't guarantee safeness!
    *square0=ProjectNearest(coord);
    return float4AllZeros;
}

float4 trans_coord_float_float_CLAMP_TO_EDGE_FALSE_NEAREST(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    *square0=ProjectToEdgeInt(image,ProjectNearest(coord));
    return float4AllZeros;
}

float4 trans_coord_float_float_UNDEFINED(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    return float4AllZeros;   //will be background color, but it's a "don't care" situtation
}

float4 trans_coord_float_float_NONE_TRUE_NEAREST(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    //not testing if coords are OOB - this mode doesn't guarantee safeness!
    *square0=ProjectNearest(Unnormalize(image, coord));
    return float4AllZeros;
}

float4 trans_coord_float_float_CLAMP_TO_EDGE_TRUE_NEAREST(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    *square0=ProjectToEdgeInt(image, ProjectNearest(Unnormalize(image, coord)));
    return float4AllZeros;
}

float4 trans_coord_float_float_REPEAT_TRUE_NEAREST(image2d_t image, float4 coord, int4* square0, int4* square1)
{

    image_aux_data *pImage = __builtin_astype(image, image_aux_data*);
    int4 upper = as_int4(_mm_load_si128((__m128i*)(&pImage->dimSub1)));
    int4 urcoord = ProjectNearest(Unnormalize(image, coord-floor(coord)));  //unrepeated coords

    *square0 = urcoord <= upper ? urcoord : 0;

    return float4AllZeros;
}

float4 trans_coord_float_float_MIRRORED_REPEAT_TRUE_NEAREST(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*);
    int4 upper = as_int4(_mm_load_si128((__m128i*)(&pImage->dimSub1)));
    __m128 isZero = _mm_cmpeq_ps(coord, float4AllZeros);
    __m128 mcoord = (__m128)_mm_sub_epi32((__m128i)coord, *((__m128i*)f4minNorm));
    mcoord = rint(mcoord);
    mcoord = (__m128)_mm_add_epi32((__m128i)mcoord, *((__m128i*)f4minNorm));
    /// Set to zero coordinates that were equal to zero before
    /// multiplications
    mcoord = (__m128)_mm_andnot_si128((__m128i)isZero, (__m128i)mcoord);
    mcoord = _mm_sub_ps(mcoord, coord);
    mcoord = _mm_abs_ps(mcoord);
    int4 urcoord = ProjectNearest(Unnormalize(image, as_float4(mcoord)));  //unrepeated coords
    *square0=min(urcoord,upper);
    return float4AllZeros;
}


float4 trans_coord_float_float_NONE_FALSE_LINEAR(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    *square0=ProjectNearest(coord - halfhalfhalfzero);
    *square1=ProjectNearest(coord - halfhalfhalfzero) + oneOneOneZero;
    return frac(coord-halfhalfhalfzero);
}


float4 trans_coord_float_float_CLAMP_TO_EDGE_FALSE_LINEAR(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    *square0 = ProjectToEdgeInt(image, ProjectNearest(coord - halfhalfhalfzero));
    *square1 = ProjectToEdgeInt(image, ProjectNearest(coord - halfhalfhalfzero) + oneOneOneZero);
    return frac(coord-halfhalfhalfzero);
}


float4 trans_coord_float_float_NONE_TRUE_LINEAR(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    float4 ucoord = Unnormalize(image, coord);
    *square0=ProjectNearest(ucoord - halfhalfhalfzero);
    *square1=ProjectNearest(ucoord - halfhalfhalfzero) + oneOneOneZero;
    return frac(ucoord-halfhalfhalfzero);
}


float4 trans_coord_float_float_CLAMP_TO_EDGE_TRUE_LINEAR(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    float4 ucoord = Unnormalize(image, coord);
    int4 notClampedSquare0 = ProjectNearest(ucoord - halfhalfhalfzero);
    int4 notClampedSquare1 = ProjectNearest(ucoord - halfhalfhalfzero) + oneOneOneZero;

    *square0=ProjectToEdgeInt(image, notClampedSquare0);
    *square1=ProjectToEdgeInt(image, notClampedSquare1);
    return frac(ucoord-halfhalfhalfzero);
}


float4 trans_coord_float_float_REPEAT_TRUE_LINEAR(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*);
    int4 upper = as_int4(_mm_load_si128((__m128i*)(&pImage->dim)));
    float4 ucoord = Unnormalize(image, coord-floor(coord));  //unrepeated coords
    int4 sq0 = ProjectNearest(ucoord - halfhalfhalfzero);
    int4 sq1 = sq0 + oneOneOneZero;

    *square0 = sq0 < (int4)0 ? upper + sq0 : sq0;
    *square1 = sq1 > upper - oneOneOneZero ? sq1 - upper : sq1;

    return frac(ucoord-halfhalfhalfzero);
}

float4 trans_coord_float_float_MIRRORED_REPEAT_TRUE_LINEAR(image2d_t image, float4 coord, int4* square0, int4* square1)
{
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*);
    int4 upper = as_int4(_mm_load_si128((__m128i*)(&pImage->dimSub1)));
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


/*****************************RGBA_UNSIGNED_INT8 Image type i/o functions****************************************************/

/// Implements nearest callbacks for given image format and border color
#define IMPLEMENT_READ_SAMPLE_NEAREST(FORMAT, RETURN_TYPE, BORDER_COLOR)\
RETURN_TYPE read_sample_NEAREST_NO_CLAMP_##FORMAT(image2d_t image, int4 coord, __private void* pData)\
{\
    __private void* pixel = extract_pixel_pointer_quad(image, coord, pData);\
    return load_pixel_##FORMAT(pixel);\
}\
\
RETURN_TYPE read_sample_NEAREST_CLAMP_##FORMAT(image2d_t image, int4 coord, __private void* pData)\
{\
    int isOOB = isOutOfBoundsInt(image, coord);\
    if (isOOB)\
        return BORDER_COLOR;\
    __private void* pixel = extract_pixel_pointer_quad(image, coord, pData);\
    return load_pixel_##FORMAT(pixel);\
}

#define IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(FORMAT, BORDER_COLOR)\
float4 read_sample_NEAREST_NO_CLAMP_##FORMAT(image2d_t image, int4 coord, int4 dummy0, float4 dummy1, __private void* pData)\
{\
    __private void* pixel = extract_pixel_pointer_quad(image, coord, pData);\
    return load_pixel_##FORMAT(pixel);\
}\
\
float4 read_sample_NEAREST_CLAMP_##FORMAT(image2d_t image, int4 coord, int4 dummy0, float4 dummy1, __private void* pData)\
{\
    int isOOB = isOutOfBoundsInt(image, coord);\
    if (isOOB)\
        return BORDER_COLOR;\
    __private void* pixel = extract_pixel_pointer_quad(image, coord, pData);\
    return load_pixel_##FORMAT(pixel);\
}

IMPLEMENT_READ_SAMPLE_NEAREST(RGBA_UNSIGNED_INT8,  uint4, BorderColorNoAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(RGBA_UNSIGNED_INT16, uint4, BorderColorNoAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(RGBA_UNSIGNED_INT32, uint4, BorderColorNoAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(RGBA_SIGNED_INT8,  int4, BorderColorNoAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST(RGBA_SIGNED_INT16, int4, BorderColorNoAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST(RGBA_SIGNED_INT32, int4, BorderColorNoAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RGBA_UNORM_INT8,  BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RGBA_UNORM_INT16, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RGBA_SNORM_INT8,  BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RGBA_SNORM_INT16, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RGBA_FLOAT, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RGBA_HALF_FLOAT, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(sRGBA_UNORM_INT8,  BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(sBGRA_UNORM_INT8,  BorderColorNoAlphaFloat)
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
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(R_SNORM_INT8, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(R_SNORM_INT16, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST(R_SIGNED_INT8, int4, BorderColorAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST(R_SIGNED_INT16, int4, BorderColorAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST(R_SIGNED_INT32, int4, BorderColorAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST(R_UNSIGNED_INT8, uint4, BorderColorAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(R_UNSIGNED_INT16, uint4, BorderColorAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(R_UNSIGNED_INT32, uint4, BorderColorAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(R_HALF_FLOAT, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(A_FLOAT, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(A_UNORM_INT8, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(A_UNORM_INT16, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(A_HALF_FLOAT, BorderColorNoAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST(RG_UNSIGNED_INT8,  uint4, BorderColorAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(RG_UNSIGNED_INT16, uint4, BorderColorAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(RG_UNSIGNED_INT32, uint4, BorderColorAlphaUint)
IMPLEMENT_READ_SAMPLE_NEAREST(RG_SIGNED_INT8,  int4, BorderColorAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST(RG_SIGNED_INT16, int4, BorderColorAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST(RG_SIGNED_INT32, int4, BorderColorAlphaInt)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RG_UNORM_INT8, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RG_UNORM_INT16, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RG_SNORM_INT8, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RG_SNORM_INT16, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RG_FLOAT, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RG_HALF_FLOAT, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(DEPTH_FLOAT, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(DEPTH_UNORM_INT16, BorderColorAlphaFloat)

void write_sample_RGBA_UNSIGNED_INT8(__private void* pixel, uint4 color)
{
    color = min(color, (uint4)(UCHAR_MAX));
    *(__private char4*)pixel = __ocl_trunc_v4i32_v4i8(*((int4*)&color));
}

void write_sample_RG_UNSIGNED_INT8(__private void* pixel, uint4 color)
{
    const __m128i i4uint8Max = _mm_set1_epi32(UCHAR_MAX);
    __m128i i4Val=(__m128i)color;
    i4Val = (__m128i)min(as_int4(i4Val), as_int4(i4uint8Max));
    *(__private unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned char*)pixel+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
}

void write_sample_R_UNSIGNED_INT8(__private void* pixel, uint4 color)
{
    const __m128i i4uint8Max = _mm_set1_epi32(UCHAR_MAX);
    __m128i i4Val=(__m128i)color;
    i4Val = (__m128i)min(as_int4(i4Val), as_int4(i4uint8Max));
    *(__private unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
}

/*****************************RGBA_UNSIGNED_INT16 Image type i/o functions****************************************************/

uint4 load_pixel_RGBA_UNSIGNED_INT16(__private void* pPixel)
{
    __m128i i4Val = _mm_loadl_epi64((__m128i*)pPixel);
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    return as_uint4(i4Val);
}

void write_sample_RGBA_UNSIGNED_INT16(__private void* pixel, uint4 color)
{
    __m128i i4Val = (__m128i)min(color, i4uint16Max);
    /// pack values to pixels
    *(__private unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned short*)pixel+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned short*)pixel+2) = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned short*)pixel+3) = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

void write_sample_RG_UNSIGNED_INT16(__private void* pixel, uint4 color)
{
    __m128i i4Val = (__m128i)min(color, i4uint16Max);
    /// pack values to pixels
    *(__private unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned short*)pixel+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

void write_sample_R_UNSIGNED_INT16(__private void* pixel, uint4 color)
{
    __m128i i4Val = (__m128i)min(color, i4uint16Max);
    /// pack values to pixels
    *(__private unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

/*****************************RGBA_UNSIGNED_INT32 Image type i/o functions****************************************************/

uint4 load_pixel_RGBA_UNSIGNED_INT32(__private void* pPixel)
{
    return (*((uint4*)pPixel));
}

void write_sample_RGBA_UNSIGNED_INT32(__private void* pixel, uint4 color)
{
    (*(__private uint4*)pixel)=color;
}

void write_sample_R_UNSIGNED_INT32(__private void* pixel, uint4 color)
{
    (*(__private uint*)pixel)=color.x;
}

void write_sample_RG_UNSIGNED_INT32(__private void* pixel, uint4 color)
{
    (*(__private uint2*)pixel)=color.lo;
}

/*******************************************************************SIGNED IMAGE TYPES I/IO*****************************************************************************/


/*****************************RGBA_SIGNED_INT8 Image type i/o functions****************************************************/


int4 load_pixel_RGBA_SIGNED_INT8(__private void* pPixel)
{
    __m128i i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPixel);
    i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    // Extend sign
    i4Val = _mm_slli_si128(i4Val, 3);
    i4Val = _mm_srai_epi32(i4Val, 24);
    return as_int4(i4Val);
}

void write_sample_RGBA_SIGNED_INT8(__private void* pixel, int4 color)
{
    __m128i i4Val = (__m128i)max(color, i4int16Min);
    i4Val = (__m128i)min(as_int4(i4Val), i4int16Max);
    i4Val = _mm_packs_epi32(i4Val, i4Val);
    i4Val = _mm_packs_epi16(i4Val, i4Val);
    *(__private unsigned int*)pixel = _mm_cvtsi128_si32(i4Val);
}

void write_sample_R_SIGNED_INT8(__private void* pixel, int4 color)
{
    __m128i i4Val = (__m128i)max(color, i4int16Min);
    i4Val = (__m128i)min(as_int4(i4Val), i4int16Max);
    i4Val = _mm_packs_epi32(i4Val, i4Val);
    i4Val = _mm_packs_epi16(i4Val, i4Val);
    *(__private char*)pixel = ((char4)_mm_cvtsi128_si32(i4Val)).x;
}

void write_sample_RG_SIGNED_INT8(__private void* pixel, int4 color)
{
    __m128i i4Val = (__m128i)max(color, i4int16Min);
    i4Val = (__m128i)min(as_int4(i4Val), i4int16Max);
    i4Val = _mm_packs_epi32(i4Val, i4Val);
    i4Val = _mm_packs_epi16(i4Val, i4Val);
    *(__private unsigned short*)pixel = ((ushort2)_mm_cvtsi128_si32(i4Val)).x;
}
/*****************************RGBA_SIGNED_INT16 Image type i/o functions****************************************************/

int4 load_pixel_RGBA_SIGNED_INT16(__private void* pPixel)
{
    __m128i i4Val = _mm_loadl_epi64((__m128i*)pPixel);
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    // Extend sign
    i4Val = _mm_slli_si128(i4Val, 2);
    i4Val = _mm_srai_epi32(i4Val, 16);
    return as_int4(i4Val);
}

void write_sample_RGBA_SIGNED_INT16(__private void* pixel, int4 color)
{
    __m128i i4Val = (__m128i)color;
    i4Val = (__m128i)max(as_int4(i4Val), i4int16Min);
    i4Val = (__m128i)min(as_int4(i4Val), i4int16Max);
    // Shrink to 8bit
    i4Val = _mm_packs_epi32(i4Val, i4Val);
    _mm_storel_epi64((__m128i*)pixel, i4Val);
}

void write_sample_RG_SIGNED_INT16(__private void* pixel, int4 color)
{
    __m128i i4Val = (__m128i)color;
    i4Val = (__m128i)max(as_int4(i4Val), i4int16Min);
    i4Val = (__m128i)min(as_int4(i4Val), i4int16Max);
    // i4Val already contains valid short value
    (*(short*)pixel)=(as_int4(i4Val)).x;
    ((short*)pixel)[1]=(as_int4(i4Val)).y;
}

void write_sample_R_SIGNED_INT16(__private void* pixel, int4 color)
{
    __m128i i4Val = (__m128i)color;
    i4Val = (__m128i)max(as_int4(i4Val), i4int16Min);
    i4Val = (__m128i)min(as_int4(i4Val), i4int16Max);
    // i4Val already contains valid short value
    (*(__private short*)pixel)=(as_int4(i4Val)).x;
}

/*****************************RGBA_SIGNED_INT32 Image type i/o functions****************************************************/

int4 load_pixel_RGBA_SIGNED_INT32(__private void* pPixel)
{
    return (*(__private int4*)pPixel);
}

void write_sample_RGBA_SIGNED_INT32(__private void* pixel, int4 color)
{
    (*(__private int4*)pixel)=color;
}

void write_sample_R_SIGNED_INT32(__private void* pixel, int4 color)
{
    (*(__private int*)pixel)=color.x;
}

void write_sample_RG_SIGNED_INT32(__private void* pixel, int4 color)
{
    (*(__private int2*)pixel)=color.lo;
}

/*****************************************************************UNORM IMAGES TYPES I/O*****************************************************************/


/***************************************RGBA_UNORM8 Image type i/o functions*****************************************************/

float4 load_pixel_RGBA_UNORM_INT8(__private void* pPixel)
{
    __m128i i4Val = (__m128i)_mm_cvtsi32_si128(*(unsigned int*)pPixel);
    i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    float4 converted = _mm_cvtepi32_ps(i4Val);
    converted = converted*(float4)(1.0f/255.0f);
    return converted;
}

void write_sample_RGBA_UNORM_INT8(__private void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, (__m128)f4unorm8mul, (__m128)f4unorm8lim);
    *(__private unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned char*)pixel+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned char*)pixel+2) = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned char*)pixel+3) = (unsigned char)_mm_cvtsi128_si32(i4Val);
}

void write_sample_RG_UNORM_INT8(__private void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm8mul, f4unorm8lim);
    *(__private unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned char*)pixel+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
}

void write_sample_R_UNORM_INT8(__private void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm8mul, f4unorm8lim);
    *(__private unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
}

void write_sample_A_UNORM_INT8(__private void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm8mul, f4unorm8lim);
    i4Val = _mm_srli_si128(i4Val, 12);
    *(__private unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
}


/***************************************RGBA_UNORM16 Image type i/o functions*****************************************************/

float4 load_pixel_RGBA_UNORM_INT16(__private void* pPixel)
{

    __m128i i4Val = _mm_loadl_epi64((__m128i*)pPixel);
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    __m128 f4Val = _mm_cvtepi32_ps(i4Val);
    return _mm_mul_ps(f4Val, f4Unorm16Dim);
}

void write_sample_RGBA_UNORM_INT16(__private void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, (__m128)f4unorm16mul, (__m128)f4unorm16lim);
    *(__private unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned short*)pixel+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned short*)pixel+2) = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned short*)pixel+3) = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

void write_sample_RG_UNORM_INT16(__private void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm16mul, f4unorm16lim);
    *(__private unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned short*)pixel+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

void write_sample_R_UNORM_INT16(__private void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm16mul, f4unorm16lim);
    *(__private unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

void write_sample_DEPTH_UNORM_INT16(__private void* pixel, float4 color)
{
    ushort converted = convert_ushort_sat_rte(color.x * UNORM_INT16_FACTOR);
    *(__private ushort*)pixel = converted;
}

void write_sample_A_UNORM_INT16(__private void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm16mul, f4unorm16lim);
    i4Val = _mm_srli_si128(i4Val, 12);
    *(__private unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
}



/*************************************** R, RG, and RGBA with SNORM8 Image type i/o functions*****************************************************/

float4 load_pixel_RGBA_SNORM_INT8(__private void* pPixel)
{
    char4 pixel = vload4(0, (__private char*)pPixel);
    float4 converted = convert_float4(pixel) / SNORM_INT8_FACTOR;
    return max(-1.f, converted);
}

float4 load_pixel_RG_SNORM_INT8(__private void* pPixel)
{
    char2 pixel = vload2(0, (__private char*)pPixel);
    float4 converted = (float4)(convert_float2(pixel) / SNORM_INT8_FACTOR, 0.f, 1.f);
    return max((float4)(-1.f), converted);
}

float4 load_pixel_R_SNORM_INT8(__private void* pPixel)
{
    char pixel = *(__private char*)pPixel;
    float4 converted = (float4)(convert_float(pixel) / SNORM_INT8_FACTOR, 0.f, 0.f, 1.f);
    return max((float4)(-1.f), converted);
}

void write_sample_RGBA_SNORM_INT8(__private void* pixel, float4 color)
{
    char4 converted = convert_char4_sat_rte(color * SNORM_INT8_FACTOR);
    vstore4(converted, 0, (__private char*)pixel);
}

void write_sample_RG_SNORM_INT8(__private void* pixel, float4 color)
{
    char2 converted = convert_char2_sat_rte(color.lo * SNORM_INT8_FACTOR);
    vstore2(converted, 0, (__private char*)pixel);
}

void write_sample_R_SNORM_INT8(__private void* pixel, float4 color)
{
    *(__private char*)pixel = convert_char_sat_rte(color.x * SNORM_INT8_FACTOR);
}

/*************************************** R, RG, and RGBA with SNORM16 Image type i/o functions*****************************************************/

float4 load_pixel_RGBA_SNORM_INT16(__private void* pPixel)
{
    short4 pixel = vload4(0, (__private short*)pPixel);
    float4 converted = convert_float4(pixel) / SNORM_INT16_FACTOR;
    return max(-1.f, converted);
}

float4 load_pixel_RG_SNORM_INT16(__private void* pPixel)
{
    short2 pixel = vload2(0, (__private short*)pPixel);
    float4 converted = (float4)(convert_float2(pixel) / SNORM_INT16_FACTOR, 0.f, 1.f);
    return max((float4)(-1.f), converted);
}

float4 load_pixel_R_SNORM_INT16(__private void* pPixel)
{
    short pixel = *(__private short*)pPixel;
    float4 converted = (float4)(convert_float(pixel) / SNORM_INT16_FACTOR, 0.f, 0.f, 1.f);
    return max((float4)(-1.f), converted);
}

void write_sample_RGBA_SNORM_INT16(__private void* pixel, float4 color)
{
    short4 converted = convert_short4_sat_rte(color * SNORM_INT16_FACTOR);
    vstore4(converted, 0, (__private short*)pixel);
}

void write_sample_RG_SNORM_INT16(__private void* pixel, float4 color)
{
    short2 converted = convert_short2_sat_rte(color.lo * SNORM_INT16_FACTOR);
    vstore2(converted, 0, (__private short*)pixel);
}

void write_sample_R_SNORM_INT16(__private void* pixel, float4 color)
{
    *(__private short*)pixel = convert_short_sat_rte(color.x * SNORM_INT16_FACTOR);
}


/***************************************BGRA_UNORM8 Image type i/o functions*****************************************************/

float4 load_pixel_BGRA_UNORM_INT8(__private void* pPixel)
{

    __m128i i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPixel);
    i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    i4Val = (__m128i)_mm_cvtepi32_ps(i4Val);
    i4Val = (__m128i)_mm_mul_ps((__m128)i4Val, (__m128)f4Unorm8Dim);
    i4Val = _mm_shuffle_epi32(i4Val, _MM_SHUFFLE(3, 0, 1, 2));
    return as_float4(i4Val);
}

void write_sample_BGRA_UNORM_INT8(__private void* pixel, float4 color)
{
    float4 convertedColor = color.zyxw;
    __m128i i4Val = cvt_to_norm((__m128i)convertedColor, (__m128)f4unorm8mul, (__m128)f4unorm8lim);
    *(__private unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned char*)pixel+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned char*)pixel+2) = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((__private unsigned char*)pixel+3) = (unsigned char)_mm_cvtsi128_si32(i4Val);
}


/****************************************************************FLOAT IMAGE TYPES I/O***************************************************************/


/******************************************RGBA_FLOAT image type i/o functions******************************************************/

float4 load_pixel_RGBA_FLOAT(__private void* pPixel)
{
    return *((__private float4*)pPixel);
}

void write_sample_RGBA_FLOAT(__private void* pixel, float4 color)
{
    (*(__private float4*)pixel)=color;
}

/******************************************RGBA_HALF_FLOAT image type i/o functions******************************************************/

float4 load_pixel_RGBA_HALF_FLOAT(__private void* pPixel)
{
    return (float4)vloada_half4(0, (__private half*)pPixel);
}

void write_sample_RGBA_HALF_FLOAT(__private void* pixel, float4 color)
{
    vstore_half4(color, 0, (__private half*)pixel);
}

void write_sample_R_HALF_FLOAT(__private void* pixel, float4 color)
{
    vstore_half(color.x, 0, (__private half*)pixel);
}

void write_sample_RG_HALF_FLOAT(__private void* pixel, float4 color)
{
    vstore_half2(color.lo, 0, (__private half*)pixel);
}

void write_sample_A_HALF_FLOAT(__private void* pixel, float4 color)
{
    vstore_half(color.w, 0, (__private half*)pixel); // store alpha channel from pixel (0,0,0,a)
}

void write_sample_LUMINANCE_HALF_FLOAT(__private void* pixel, float4 color)
{
    vstore_half(color.x, 0, (__private half*)pixel);
}

void write_sample_INTENSITY_HALF_FLOAT(__private void* pixel, float4 color)
{
    vstore_half(color.x, 0, (__private half*)pixel);
}

/******************************************LUMINANCE image type i/o functions******************************************************/

float load_value_LUMINANCE_FLOAT(__private void* pPixel)
{
    float luminance = *((float*)pPixel);
    return luminance;
}

float4 load_pixel_LUMINANCE_FLOAT(__private void* pPixel)
{
    float luminance = load_value_LUMINANCE_FLOAT(pPixel);
    float4 res = (float4)(luminance, luminance, luminance, 1.0f);
    return res;
}

float load_value_LUMINANCE_UNORM_INT8(__private void* pPixel)
{
    uchar val = *(uchar*)pPixel;
    return val * (1.0f/255.0f);
}

float4 load_pixel_LUMINANCE_UNORM_INT8(__private void* pPixel)
{
    float luminance = load_value_LUMINANCE_UNORM_INT8(pPixel);
    return (float4)(luminance, luminance, luminance, 1.0f);
}

float load_value_LUMINANCE_UNORM_INT16(__private void* pPixel)
{
    ushort val = *(ushort*)pPixel;
    return val * (1.0f/65535.0f);
}

float4 load_pixel_LUMINANCE_UNORM_INT16(__private void* pPixel)
{
    float luminance = load_value_LUMINANCE_UNORM_INT16(pPixel);
    return (float4)(luminance, luminance, luminance, 1.0f);
}

float load_value_LUMINANCE_HALF_FLOAT(__private void* pPixel)
{
    float val = vloada_half(0, (__private half*)pPixel);
    return val;
}

float4 load_pixel_LUMINANCE_HALF_FLOAT(__private void* pPixel)
{
    float val = load_value_LUMINANCE_HALF_FLOAT(pPixel);
    return (float4)(val, val, val, 1.f);
}

float load_value_INTENSITY_UNORM_INT8(__private void* pPixel)
{
    uchar val = *(uchar*)pPixel;
    return val * (1.0f/255.0f);
}

float4 load_pixel_INTENSITY_UNORM_INT8(__private void* pPixel)
{
    float intensity = load_value_INTENSITY_UNORM_INT8(pPixel);
    return (float4)(intensity, intensity, intensity, intensity);
}

float load_value_INTENSITY_UNORM_INT16(__private void* pPixel)
{
    ushort val = *(__private ushort*)pPixel;
    return val * (1.0f/65535.0f);
}

float4 load_pixel_INTENSITY_UNORM_INT16(__private void* pPixel)
{
    float intensity = load_value_INTENSITY_UNORM_INT16(pPixel);
    return (float4)(intensity, intensity, intensity, intensity);
}

float load_value_INTENSITY_HALF_FLOAT(__private void* pPixel)
{
    float val = vloada_half(0, (__private half*)pPixel);
    return val;
}

float4 load_pixel_INTENSITY_HALF_FLOAT(__private void* pPixel)
{
    float val = load_value_INTENSITY_HALF_FLOAT(pPixel);
    return (float4)(val, val, val, val);
}

void write_sample_LUMINANCE_FLOAT(__private void* pixel, float4 color)
{
    (*(__private float*)pixel)=color.x;
}

/******************************************INTENSITY image type i/o functions******************************************************/

float load_value_INTENSITY_FLOAT(__private void* pPixel)
{
    float intensity = *((float*)pPixel);
    return intensity;
}

float4 load_pixel_INTENSITY_FLOAT(__private void* pPixel)
{
    float intensity = load_value_INTENSITY_FLOAT(pPixel);
    return (float4)intensity;
}

void write_sample_INTENSITY_FLOAT(__private void* pixel, float4 color)
{
    (*(__private float*)pixel)=color.x;
}

void write_sample_INTENSITY_UNORM_INT8(__private void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm8mul, f4unorm8lim);
    *(__private unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
}

void write_sample_INTENSITY_UNORM_INT16(__private void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm16mul, f4unorm16lim);
    *(__private unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

void write_sample_LUMINANCE_UNORM_INT8(__private void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm8mul, f4unorm8lim);
    *(__private unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
}

void write_sample_LUMINANCE_UNORM_INT16(__private void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm16mul, f4unorm16lim);
    *(__private unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

void write_sample_R_FLOAT(__private void* pixel, float4 color)
{
    (*(__private float*)pixel)=color.x;
}

void write_sample_DEPTH_FLOAT(__private void* pixel, float4 color)
{
    (*(__private float*)pixel)=color.x;
}

void write_sample_RG_FLOAT(__private void* pixel, float4 color)
{
    (*(__private float2*)pixel)=color.lo;
}

void write_sample_A_FLOAT(__private void* pixel, float4 color)
{
    (*(__private float*)pixel)=color.w;
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


#define IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(TYPE, POST_PROCESSING) \
    float4 read_sample_LINEAR1D_NO_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, __private void* pData)  \
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

IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RGBA_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RGBA_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(INTENSITY_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(INTENSITY_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(INTENSITY_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(INTENSITY_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(LUMINANCE_FLOAT, luminance_post_process)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(LUMINANCE_UNORM_INT8, luminance_post_process)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(LUMINANCE_UNORM_INT16, luminance_post_process)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(LUMINANCE_HALF_FLOAT, luminance_post_process)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RGBA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(BGRA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RGBA_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RGBA_SNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RGBA_SNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(sRGBA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(sBGRA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(R_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(R_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(R_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(R_SNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(R_SNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(R_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(A_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(A_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(A_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(A_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RG_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RG_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RG_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RG_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RG_SNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RG_SNORM_INT16, dummyFnc)
// the following implementations are workaround to avoid changes
// in the image callback library architecture
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(DEPTH_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(DEPTH_UNORM_INT16, dummyFnc)

// definition for linear read callbacks in case of one channel images
#define IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP_CH1(TYPE, POST_PROCESSING) \
    float4 read_sample_LINEAR2D_NO_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, __private void* pData)  \
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
#define IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(TYPE, POST_PROCESSING) \
    float4 read_sample_LINEAR2D_NO_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, __private void* pData)  \
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

IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RGBA_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RGBA_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP_CH1(INTENSITY_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP_CH1(INTENSITY_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP_CH1(INTENSITY_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP_CH1(INTENSITY_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP_CH1(LUMINANCE_FLOAT, luminance_post_process)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP_CH1(LUMINANCE_UNORM_INT8, luminance_post_process)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP_CH1(LUMINANCE_UNORM_INT16, luminance_post_process)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP_CH1(LUMINANCE_HALF_FLOAT, luminance_post_process)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RGBA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(BGRA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RGBA_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RGBA_SNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RGBA_SNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(sRGBA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(sBGRA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(R_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(R_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(R_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(R_SNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(R_SNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(R_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(A_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(A_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(A_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(A_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RG_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RG_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RG_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RG_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RG_SNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RG_SNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(DEPTH_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(DEPTH_UNORM_INT16, dummyFnc)


#define IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(TYPE, POST_PROCESSING) \
float4 read_sample_LINEAR3D_NO_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, __private void* pData)  \
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

IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RGBA_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RGBA_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(INTENSITY_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(INTENSITY_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(INTENSITY_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(INTENSITY_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(LUMINANCE_FLOAT, luminance_post_process)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(LUMINANCE_UNORM_INT8, luminance_post_process)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(LUMINANCE_UNORM_INT16, luminance_post_process)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(LUMINANCE_HALF_FLOAT, luminance_post_process)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RGBA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(sRGBA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(sBGRA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(BGRA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RGBA_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RGBA_SNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RGBA_SNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(R_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(R_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(R_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(R_SNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(R_SNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(R_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(A_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(A_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(A_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(A_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RG_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RG_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RG_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RG_SNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RG_SNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RG_HALF_FLOAT, dummyFnc)
// the following implementations are workaround to avoid changes
// in the image callback library architecture
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(DEPTH_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(DEPTH_UNORM_INT16, dummyFnc)


#define IMPLEMENT_read_sample_LINEAR1D_CLAMP(TYPE, BORDER_COLOR, POST_PROCESSING) \
    float4 read_sample_LINEAR1D_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, __private void* pData)  \
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
IMPLEMENT_read_sample_LINEAR1D_CLAMP(sRGBA_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(sBGRA_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(BGRA_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RGBA_UNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RGBA_SNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RGBA_SNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(R_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(R_UNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(R_UNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(R_SNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(R_SNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(R_HALF_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(A_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(A_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(A_UNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(A_HALF_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RG_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RG_UNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RG_UNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RG_SNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RG_SNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(RG_HALF_FLOAT, BorderColorAlphaFloat, dummyFnc)
// the following implementations are workaround to avoid changes
// in the image callback library architecture
IMPLEMENT_read_sample_LINEAR1D_CLAMP(DEPTH_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_CLAMP(DEPTH_UNORM_INT16, BorderColorAlphaFloat, dummyFnc)


#define IMPLEMENT_read_sample_LINEAR2D_CLAMP_CH1(TYPE, POST_PROCESSING) \
    float4 read_sample_LINEAR2D_NO_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, __private void* pData)  \
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
    float4 read_sample_LINEAR2D_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, __private void* pData)  \
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
IMPLEMENT_read_sample_LINEAR2D_CLAMP(sRGBA_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(sBGRA_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(BGRA_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RGBA_UNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RGBA_SNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RGBA_SNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(R_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(R_UNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(R_UNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(R_SNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(R_SNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(R_HALF_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(A_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(A_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(A_UNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(A_HALF_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RG_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RG_UNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RG_UNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RG_SNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RG_SNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(RG_HALF_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(DEPTH_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_CLAMP(DEPTH_UNORM_INT16, BorderColorAlphaFloat, dummyFnc)


#define IMPLEMENT_read_sample_LINEAR3D_CLAMP(TYPE, BORDER_COLOR, POST_PROCESSING) \
float4 read_sample_LINEAR3D_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, __private void* pData)  \
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
IMPLEMENT_read_sample_LINEAR3D_CLAMP(sRGBA_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(sBGRA_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(BGRA_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RGBA_UNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RGBA_SNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RGBA_SNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(R_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(R_UNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(R_UNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(R_SNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(R_SNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(R_HALF_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(A_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(A_UNORM_INT8, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(A_UNORM_INT16, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(A_HALF_FLOAT, BorderColorNoAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RG_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RG_UNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RG_UNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RG_SNORM_INT8, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RG_SNORM_INT16, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(RG_HALF_FLOAT, BorderColorAlphaFloat, dummyFnc)
// the following implementations are workaround to avoid changes
// in the image callback library architecture
IMPLEMENT_read_sample_LINEAR3D_CLAMP(DEPTH_FLOAT, BorderColorAlphaFloat, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_CLAMP(DEPTH_UNORM_INT16, BorderColorAlphaFloat, dummyFnc)


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
uint4 load_pixel_R_UNSIGNED_INT32(__private void* pPixel)
{
    uint4 pixel = (uint4)(0, 0, 0, 1);
    pixel.x = *((__private uint*)pPixel);
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
uint4 load_pixel_RG_UNSIGNED_INT32(__private void* pPixel)
{
    uint4 pixel = (uint4)(0, 0, 0, 1);
    pixel.x = *((__private uint*)pPixel);
    pixel.y = ((__private uint*)pPixel)[1];
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
int4 load_pixel_R_SIGNED_INT32(__private void* pPixel)
{
    int4 pixel = (int4)(0, 0, 0, 1);
    pixel.x = *((__private int*)pPixel);
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
int4 load_pixel_RG_SIGNED_INT32(__private void* pPixel)
{
    int4 pixel = (int4)(0, 0, 0, 1);
    pixel.x = *((__private int*)pPixel);
    pixel.y = ((__private int*)pPixel)[1];
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
float4 load_pixel_R_FLOAT(__private void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 1.f);
    pixel.x = *((__private float*)pPixel);
    return pixel;
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CLK_DEPTH
// Channel Data Type: CLK_FLOAT
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a float4 (d, 0, 0, 1.0)
float4 load_pixel_DEPTH_FLOAT(__private void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 1.f);
    pixel.x = *((__private float*)pPixel);
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
float4 load_pixel_RG_FLOAT(__private void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 1.f);
    pixel.x = *((__private float*)pPixel);
    pixel.y = ((__private float*)pPixel)[1];
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
float4 load_pixel_A_FLOAT(__private void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 1.f);
    pixel.w = *((__private float*)pPixel);
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
float4 load_pixel_A_HALF_FLOAT(__private void* pPixel)
{
    float val = vloada_half(0, (__private half*)pPixel);

    float4 pixel = (float4)(0.f, 0.f, 0.f, 1.f);
    pixel.w = val;
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
float4 load_pixel_R_HALF_FLOAT(__private void* pPixel)
{
    float val = vloada_half(0, (__private half*)pPixel);

    float4 pixel = (float4)(0.f, 0.f, 0.f, 1.f);
    pixel.x = val;
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
float4 load_pixel_RG_HALF_FLOAT(__private void* pPixel)
{
    float2 val = vloada_half2(0, (__private half*)pPixel);

    float4 pixel = (float4)(0.f, 0.f, 0.f, 1.f);
    pixel.lo = val;
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
uint4 load_pixel_RGBA_UNSIGNED_INT8(__private void* pPixel)
{
    char4 color = *(__private char4*)pPixel; // nevermind signed/unsigned.
    int4 converted = __ocl_zext_v4i8_v4i32(color);
    return *(uint4*)&converted;
}


// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_R & CL_Rx
// Channel Data Type: CLK_UNSIGNED_INT8
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a uint4 (r, 0, 0, 1.0)
uint4 load_pixel_R_UNSIGNED_INT8(__private void* pPixel)
{
    uint4 pixel = (uint4)(0, 0, 0, 1);
    pixel.x = (uint)(*((__private uchar*)pPixel));
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
uint4 load_pixel_RG_UNSIGNED_INT8(__private void* pPixel)
{
    uint4 pixel = (uint4)(0, 0, 0, 1);
    pixel.x = (uint)(*((__private uchar*)pPixel));
    pixel.y = (uint)(((__private uchar*)pPixel)[1]);
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
uint4 load_pixel_R_UNSIGNED_INT16(__private void* pPixel)
{
    uint4 pixel = (uint4)(0, 0, 0, 1);
    pixel.x = (uint)( *((__private ushort*)pPixel) );
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
uint4 load_pixel_RG_UNSIGNED_INT16(__private void* pPixel)
{
    uint4 pixel = (uint4)(0, 0, 0, 1);
    pixel.x = (uint)( *((__private ushort*)pPixel) );
    pixel.y = (uint)( (((__private ushort*)pPixel)[1]) );
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
int4 load_pixel_R_SIGNED_INT8(__private void* pPixel)
{
    int4 pixel = (int4)(0, 0, 0, 1);
    pixel.x = (int)(*((__private char*)pPixel));
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
int4 load_pixel_RG_SIGNED_INT8(__private void* pPixel)
{
    int4 pixel = (int4)(0, 0, 0, 1);
    pixel.x = (int)(*((__private char*)pPixel));
    pixel.y = (int)(((__private char*)pPixel)[1]);
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
float4 load_pixel_A_UNORM_INT8(__private void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 0.f);
    pixel.w = (float)(*((__private uchar*)pPixel));
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
float4 load_pixel_A_UNORM_INT16(__private void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 0.f);
    pixel.w = (float)*((__private ushort*)pPixel);
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
float4 load_pixel_RG_UNORM_INT8(__private void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 255.f); // Make the last value 255 to have 1 after conversion
    pixel.x = (float)(*((__private uchar*)pPixel));
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
float4 load_pixel_R_UNORM_INT8(__private void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 255.f); // Make the last value 255 to have 1 after conversion
    pixel.x = (float)(*((__private uchar*)pPixel));
    float4 converted = pixel*(float4)(1.0f/255.0f);
    return converted;
}

/***************************************sRGB Image type i/o functions*****************************************************/

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_sRGBA and CL_sBGRA
// Channel Data Type: CLK_UNORM_INT8
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a float4 (r, g, b, a)
//
// The following is lookup table with precomputed sRGB values for each of possible UNORM_INT8
// it is aligned by cacheline length
//
// The table was computed with this kernel:
//  __kernel void calclulate_sRGBA_read_imagef_lut(__private float *out) {
//      int index = get_private_id(0);
//      float c = convert_float(index) * 1.f/255.f;
//      if (c <= 0.04045f)
//        out[index] = c / 12.92f;
//      else
//        out[index] = powr((c + 0.055f) / 1.055f, 2.4f);
//  }
 __attribute__ ((aligned(64))) const constant float read_imagef_sRGBA_UNORM_INT8_LUT[256] = {
0, 0.00030352699104696512, 0.00060705398209393024, 0.00091058103134855628,
0.0012141079641878605, 0.0015176349552348256, 0.0018211620626971126, 0.0021246890537440777,
0.002428215928375721, 0.0027317430358380079, 0.0030352699104696512, 0.0033465356100350618,
0.0036765069235116243, 0.0040247170254588127, 0.0043914420530200005, 0.0047769532538950443,
0.0051815169863402843, 0.0056053916923701763, 0.0060488325543701649, 0.0065120910294353962,
0.0069954101927578449, 0.0074990317225456238, 0.0080231921747326851, 0.00856812484562397,
0.0091340569779276848, 0.0097212176769971848, 0.010329823009669781, 0.010960093699395657,
0.011612244881689548, 0.01228648703545332, 0.012983030639588833, 0.013702080585062504,
0.014443843625485897, 0.015208514407277107, 0.015996292233467102, 0.016807375475764275,
0.017641952261328697, 0.018500218167901039, 0.019382361322641373, 0.020288562402129173,
0.021219009533524513, 0.022173883393406868, 0.023153364658355713, 0.024157630279660225,
0.025186857208609581, 0.026241222396492958, 0.027320891618728638, 0.028426038101315498,
0.029556842520833015, 0.030713450163602829, 0.031896039843559265, 0.033104773610830307,
0.034339811652898788, 0.035601325333118439, 0.036889452487230301, 0.038204375654459,
0.039546247571706772, 0.040915209800004959, 0.042311422526836395, 0.043735042214393616,
0.04518621414899826, 0.046665094792842865, 0.048171833157539368, 0.049706574529409409,
0.051269467920064926, 0.052860654890537262, 0.054480280727148056, 0.056128494441509247,
0.057805433869361877, 0.059511240571737289, 0.061246071010828018, 0.063010029494762421,
0.064803279936313629, 0.06662595272064209, 0.068478181958198547, 0.070360109210014343,
0.072271861135959625, 0.074213579297065735, 0.07618539035320282, 0.078187428414821625,
0.080219827592372894, 0.082282714545726776, 0.084376215934753418, 0.086500465869903564,
0.088655605912208557, 0.090841732919216156, 0.093058981001377106, 0.095307484269142151,
0.097587361931800842, 0.099898740649223328, 0.10224174708127975, 0.10461649298667908,
0.10702311247587204, 0.1094617173075676, 0.11193243414163589, 0.11443538218736649,
0.11697069555521011, 0.11953844875097275, 0.12213881313800812, 0.12477186322212219,
0.12743772566318512, 0.13013651967048645, 0.13286836445331573, 0.13563336431980133,
0.13843165338039398, 0.14126332104206085, 0.14412850141525269, 0.14702729880809784,
0.14995981752872467, 0.15292617678642273, 0.15592649579048157, 0.15896086394786835,
0.16202943027019501, 0.16513223946094513, 0.16826945543289185, 0.17144115269184113,
0.17464745044708252, 0.17788846790790558, 0.18116429448127747, 0.18447503447532654,
0.18782080709934235, 0.19120171666145325, 0.1946178674697876, 0.19806934893131256,
0.20155629515647888, 0.20507876574993134, 0.20863689482212067, 0.21223078668117523,
0.21586053073406219, 0.21952623128890991, 0.22322797775268555, 0.22696588933467865,
0.23074007034301758, 0.23455065488815308, 0.23839765787124634, 0.24228119850158691,
0.24620139598846436, 0.25015836954116821, 0.25415217876434326, 0.25818291306495667,
0.26225072145462036, 0.26635566353797913, 0.27049785852432251, 0.27467736601829529,
0.2788943350315094, 0.28314879536628723, 0.28744089603424072, 0.29177069664001465,
0.29613831639289856, 0.30054384469985962, 0.30498737096786499, 0.30946895480155945,
0.31398874521255493, 0.3185468316078186, 0.32314324378967285, 0.32777813076972961,
0.33245158195495605, 0.33716365694999695, 0.34191444516181946, 0.34670409560203552,
0.3515326976776123, 0.3564002513885498, 0.36130687594413757, 0.36625269055366516,
0.37123778462409973, 0.37626221776008606, 0.3813261091709137, 0.38642951846122742,
0.39157256484031677, 0.39675530791282654, 0.40197786688804626, 0.40724030137062073,
0.41254270076751709, 0.41788515448570251, 0.42326775193214417, 0.42869055271148682,
0.43415370583534241, 0.43965724110603333, 0.44520124793052673, 0.45078584551811218,
0.45641106367111206, 0.46207705140113831, 0.46778383851051331, 0.47353154420852661,
0.479320228099823, 0.48514997959136963, 0.49102088809013367, 0.49693304300308228,
0.50288659334182739, 0.50888144969940186, 0.51491779088973999, 0.52099567651748657,
0.5271153450012207, 0.53327661752700806, 0.53947967290878296, 0.54572468996047974,
0.55201160907745361, 0.55834060907363892, 0.56471168994903564, 0.57112503051757812,
0.57758063077926636, 0.58407860994338989, 0.59061902761459351, 0.59720200300216675,
0.60382753610610962, 0.61049574613571167, 0.61720675230026245, 0.62396055459976196,
0.63075733184814453, 0.63759702444076538, 0.64447987079620361, 0.65140581130981445,
0.65837496519088745, 0.66538745164871216, 0.67244333028793335, 0.67954260110855103,
0.68668544292449951, 0.69387203454971313, 0.70110213756561279, 0.70837604999542236,
0.71569377183914185, 0.72305536270141602, 0.7304610013961792, 0.7379106879234314,
0.74540448188781738, 0.75294244289398193, 0.76052474975585938, 0.76815140247344971,
0.77582246065139771, 0.78353804349899292, 0.79129815101623535, 0.79910296201705933,
0.80695247650146484, 0.81484681367874146, 0.82278597354888916, 0.83077007532119751,
0.83879923820495605, 0.84687346220016479, 0.85499280691146851, 0.86315739154815674,
0.87136733531951904, 0.87962257862091064, 0.88792318105697632, 0.89626955986022949,
0.90466135740280151, 0.91309899091720581, 0.92158204317092896, 0.93011116981506348,
0.93868589401245117, 0.94730687141418457, 0.95597350597381592, 0.96468657255172729,
0.97344547510147095, 0.9822508692741394, 0.99110221862792969, 1
};

float4 load_pixel_sRGBA_UNORM_INT8(__private void* pPixel)
{
    __private uchar * data = (__private uchar*)pPixel;
    float4 pixel = (float4)(read_imagef_sRGBA_UNORM_INT8_LUT[data[0]],
                            read_imagef_sRGBA_UNORM_INT8_LUT[data[1]],
                            read_imagef_sRGBA_UNORM_INT8_LUT[data[2]],
                            data[3] * 1.f/255.f);
    return pixel;
}

float4 load_pixel_sBGRA_UNORM_INT8(__private void* pPixel)
{
    __private uchar * data = (__private uchar*)pPixel;
    float4 pixel = (float4)(read_imagef_sRGBA_UNORM_INT8_LUT[data[2]],
                            read_imagef_sRGBA_UNORM_INT8_LUT[data[1]],
                            read_imagef_sRGBA_UNORM_INT8_LUT[data[0]],
                            data[3] * 1.f/255.f);
    return pixel;
}

void write_sample_sRGBA_UNORM_INT8(__private void* pixel, float4 color)
{
  // stubbed: write to sRGBA images is optional and not supported yet
}

void write_sample_sBGRA_UNORM_INT8(__private void* pixel, float4 color)
{
  // stubbed: write to sBGRA images is optional and not supported yet
}

/*************************************************************************************************************************/


// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_RG
// Channel Data Type: CLK_UNORM_INT16
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a float4 (r, g, 0, 1)
float4 load_pixel_RG_UNORM_INT16(__private void* pPixel)
{
    float4 pixel = (float4)(0.f, 0.f, 0.f, 65535.f); // Make the last value 65535 to have 1 after conversion
    pixel.x = (float)*((__private ushort*)pPixel);
    pixel.y = (float)(((__private ushort*)pPixel)[1]);
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
float4 load_pixel_R_UNORM_INT16(__private void* pPixel)
{
    float r = convert_float(*(__private ushort*)pPixel) / 65535.f;
    return (float4)(r, 0.f, 0.f, 1.f);
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CLK_DEPTH
// Channel Data Type: CLK_UNORM_INT16
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a float4 (d, 0, 0, 1)
float4 load_pixel_DEPTH_UNORM_INT16(__private void* pPixel)
{
    float d = convert_float(*(__private ushort*)pPixel) / 65535.f;
    return (float4)(d, 0.f, 0.f, 1.f);
}

// loads and converts pixel data from a given pixel pointer when the image has the following properties:
// Channel Order: CL_R & CL_Rx
// Channel Data Type: CLK_SIGNED_INT16
//
// @param image: the image object
// @param pPixel: the pointer to the pixel
//
// returns a int4 (r, 0, 0, 1.0)
int4 load_pixel_R_SIGNED_INT16(__private void* pPixel)
{
    int4 pixel = (int4)(0, 0, 0, 1);
    pixel.x = (int)(*((__private short*)pPixel));
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
int4 load_pixel_RG_SIGNED_INT16(__private void* pPixel)
{
    int4 pixel = (int4)(0, 0, 0, 1);
    pixel.x = (int)(*((__private short*)pPixel));
    pixel.y = (int)(((__private short*)pPixel)[1]);
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
__private void* extract_pixel_pointer_quad(image2d_t image, int4 coord, __private void* pData)
{
    // Calculate required pixel offset
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*);
    uint4 offset = *((__private uint4*)(&pImage->offset));
    uint4 ocoord = (as_uint4(coord)) * offset;
    __private void* pixel = pData + ocoord.x + ocoord.y + ocoord.z;
    return pixel;
}


// check if the coordinates are within the image boundaries
//
// @param image: the image object
// @param coord: (x,y) coordinates of the pixel
//
// return: nonzero if the image is out of bounds, otherwise 0
int isOutOfBoundsInt(image2d_t image, int4 coord)
{
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*);
    __m128i    i4up = _mm_load_si128((__m128i*)(pImage->dim));
    // Prepare mask for compare mask extraction
    int iMask=pImage->dimmask;
    int4 iCoord = max(coord, int4MinusOnes);
    iCoord = min(iCoord, as_int4(i4up));
    __m128i isUp=_mm_cmpeq_epi32((__m128i)iCoord,(__m128i)i4up);
    __m128i isLo=_mm_cmpeq_epi32((__m128i)iCoord,(__m128i)int4MinusOnes);
    int iBorder=(_mm_movemask_epi8(isUp) | _mm_movemask_epi8(isLo)) & iMask;
    return iBorder;
}

int4 ProjectToEdgeInt(image2d_t image, int4 coord)
{
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*);
    int4 upper = as_int4(_mm_load_si128((__m128i*)(&pImage->dimSub1)));
    int4 lower = (int4)(0, 0, 0, 0);

    int4 correctCoord=min(coord, upper);
    correctCoord=max(correctCoord,lower);
    return correctCoord;

}

float4 Unnormalize(image2d_t image,float4 coord)
{
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*);
    float4 fupper = as_float4(_mm_load_ps((float*)(&pImage->dimf)));
    return fupper*coord;
}


//the coordinate here should be unnormalized already
int4 ProjectNearest(float4 coord)
{
    return as_int4(_mm_cvtps_epi32(floor(coord)));
}

float4 frac(float4 coord)
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

    float4 vec1 = consts1 + comp1;
    float4 vec2 = consts2 + comp2;

    float4 res = vec1 * vec2;
    res = components * res;

/// sum vector elements
    float sum = res.x + res.y + res.z + res.w;

    return (float4)(sum, sum, sum, sum);
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

/****************************************** SOA image write functions******************************************************/

/// SOA8 write RGBA_UNSIGNED_INT8
void soa8_write_sample_RGBA_UNSIGNED_INT8(__private void* p0, __private void* p1, __private void* p2, __private void* p3, 
                                                                        __private void* p4, __private void* p5, __private void* p6, __private void* p7, 
                                                                        uint8 val_x, uint8 val_y, uint8 val_z, uint8 val_w)
{

    uchar8 clr_x = convert_uchar8_sat(val_x);
    uchar8 clr_y = convert_uchar8_sat(val_y);
    uchar8 clr_z = convert_uchar8_sat(val_z);
    uchar8 clr_w = convert_uchar8_sat(val_w);

    // there is no transpose functions with __private address space
    // convert __private to __private as a workaround
    __ocl_transpose_scatter_char_4x8((__private char4*)p0, (__private char4*)p1, (__private char4*)p2, (__private char4*)p3,
                              (__private char4*)p4, (__private char4*)p5, (__private char4*)p6, (__private char4*)p7,
                              as_char8(clr_x), as_char8(clr_y), as_char8(clr_z), as_char8(clr_w));
}

/// SOA4 write RGBA_UNSIGNED_INT8
void soa4_write_sample_RGBA_UNSIGNED_INT8(__private void* p0, __private void* p1, __private void* p2, __private void* p3, uint4 val_x, uint4 val_y, uint4 val_z, uint4 val_w)
{

    uchar4 clr_x = convert_uchar4_sat(val_x);
    uchar4 clr_y = convert_uchar4_sat(val_y);
    uchar4 clr_z = convert_uchar4_sat(val_z);
    uchar4 clr_w = convert_uchar4_sat(val_w);

    // there is no transpose functions with __private address space
    // convert __private to __private as a workaround
    __ocl_transpose_scatter_char_4x4((__private char4*)p0, (__private char4*)p1, (__private char4*)p2, (__private char4*)p3,
                              as_char4(clr_x), as_char4(clr_y), as_char4(clr_z), as_char4(clr_w));
}


#define SCALARIZE_SOA4_WRITE_SAMPLE(FORMAT, PIX_TYPE)\
    void soa4_write_sample_##FORMAT(\
           __private void* p0, __private void* p1, __private void* p2, __private void* p3,\
           PIX_TYPE##4 val_x, PIX_TYPE##4 val_y, PIX_TYPE##4 val_z, PIX_TYPE##4 val_w){\
      write_sample_##FORMAT(p0, (PIX_TYPE##4)(val_x.s0, val_y.s0, val_z.s0, val_w.s0));\
      write_sample_##FORMAT(p1, (PIX_TYPE##4)(val_x.s1, val_y.s1, val_z.s1, val_w.s1));\
      write_sample_##FORMAT(p2, (PIX_TYPE##4)(val_x.s2, val_y.s2, val_z.s2, val_w.s2));\
      write_sample_##FORMAT(p3, (PIX_TYPE##4)(val_x.s3, val_y.s3, val_z.s3, val_w.s3));\
}

#define SCALARIZE_SOA8_WRITE_SAMPLE(FORMAT, PIX_TYPE)\
    void soa8_write_sample_##FORMAT(\
           __private void* p0, __private void* p1, __private void* p2, __private void* p3,\
           __private void* p4, __private void* p5, __private void* p6, __private void* p7,\
           PIX_TYPE##8 val_x, PIX_TYPE##8 val_y, PIX_TYPE##8 val_z, PIX_TYPE##8 val_w){\
      write_sample_##FORMAT(p0, (PIX_TYPE##4)(val_x.s0, val_y.s0, val_z.s0, val_w.s0));\
      write_sample_##FORMAT(p1, (PIX_TYPE##4)(val_x.s1, val_y.s1, val_z.s1, val_w.s1));\
      write_sample_##FORMAT(p2, (PIX_TYPE##4)(val_x.s2, val_y.s2, val_z.s2, val_w.s2));\
      write_sample_##FORMAT(p3, (PIX_TYPE##4)(val_x.s3, val_y.s3, val_z.s3, val_w.s3));\
      write_sample_##FORMAT(p4, (PIX_TYPE##4)(val_x.s4, val_y.s4, val_z.s4, val_w.s4));\
      write_sample_##FORMAT(p5, (PIX_TYPE##4)(val_x.s5, val_y.s5, val_z.s5, val_w.s5));\
      write_sample_##FORMAT(p6, (PIX_TYPE##4)(val_x.s6, val_y.s6, val_z.s6, val_w.s6));\
      write_sample_##FORMAT(p7, (PIX_TYPE##4)(val_x.s7, val_y.s7, val_z.s7, val_w.s7));\
}

#define SCALARIZE_WRITE_SAMPLE(FORMAT, PIX_TYPE)\
    SCALARIZE_SOA4_WRITE_SAMPLE(FORMAT, PIX_TYPE)\
    SCALARIZE_SOA8_WRITE_SAMPLE(FORMAT, PIX_TYPE)

SCALARIZE_WRITE_SAMPLE(RG_UNSIGNED_INT8, uint)
SCALARIZE_WRITE_SAMPLE(R_UNSIGNED_INT8, uint)
SCALARIZE_WRITE_SAMPLE(RGBA_UNSIGNED_INT16, uint)
SCALARIZE_WRITE_SAMPLE(RG_UNSIGNED_INT16, uint)
SCALARIZE_WRITE_SAMPLE(R_UNSIGNED_INT16, uint)
SCALARIZE_WRITE_SAMPLE(RGBA_UNSIGNED_INT32, uint)
SCALARIZE_WRITE_SAMPLE(RG_UNSIGNED_INT32, uint)
SCALARIZE_WRITE_SAMPLE(R_UNSIGNED_INT32, uint)

// undefined callbacks implementation
void soa4_read_sample_UNDEFINED_QUAD_INT( image2d_t image, int4 coord_x, int4 coord_y, __private void* pData,
                                          __private uint4* res_x, __private uint4* res_y, __private uint4* res_z, __private uint4* res_w )
{
    *res_x = BorderColorNoAlphaUint.x;
    *res_y = BorderColorNoAlphaUint.y;
    *res_z = BorderColorNoAlphaUint.z;
    *res_w = BorderColorNoAlphaUint.w;
}

void soa8_read_sample_UNDEFINED_QUAD_INT( image2d_t image, int8 coord_x, int8 coord_y, __private void* pData,
                                          __private uint8* res_x, __private uint8* res_y, __private uint8* res_z, __private uint8* res_w )
{
    *res_x = BorderColorNoAlphaUint.x;
    *res_y = BorderColorNoAlphaUint.y;
    *res_z = BorderColorNoAlphaUint.z;
    *res_w = BorderColorNoAlphaUint.w;
}

uint4 read_sample_UNDEFINED_QUAD_INT(image2d_t image, int4 coord, __private void* pData)
{
    return BorderColorNoAlphaUint;  //return all zeros vector
}

float4 read_sample_UNDEFINED_QUAD_FLOAT(image2d_t image, int4 square0, int4 square1, float4 fraction, __private void* pData)  \
{
    return BorderColorNoAlphaFloat;  //return all zeros vector
}

void trap_function()
{
    printf ("***Runtime error: reached an uninitialized image function***\n");
    __builtin_debugtrap();
}

#endif // defined (__MIC__) || defined(__MIC2__)

