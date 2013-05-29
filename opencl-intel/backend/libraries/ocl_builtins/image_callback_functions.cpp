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

// overload transpose with naive implementation until optimized one is ready
#define OVERLOAD_TRANSPOSES

// Enable double support. It is needed for declarations from intrin.h
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#define __OPENCL__
#include <intrin.h>

#include "cl_image_declaration.h"
#include "GENERIC/ll_intrinsics.h"
#include "transpose_functions.h"

#define SHRT16_MIN    (-32768)
#define SHRT16_MAX      32767

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

void* extract_pixel_pointer_quad(image2d_t image, int4 coord, void* pData);

uint4 load_pixel_RGBA_UNSIGNED_INT8(void* pPixel);
uint4 load_pixel_RGBA_UNSIGNED_INT16(void* pPixel);
uint4 load_pixel_RGBA_UNSIGNED_INT32(void* pPixel);

int4 load_pixel_RGBA_SIGNED_INT8(void* pPixel);
int4 load_pixel_RGBA_SIGNED_INT16(void* pPixel);
int4 load_pixel_RGBA_SIGNED_INT32(void* pPixel);

float load_value_INTENSITY_FLOAT(void* pPixel);
float load_value_INTENSITY_UNORM_INT8(void* pPixel);
float load_value_INTENSITY_UNORM_INT16(void* pPixel);
float load_value_INTENSITY_HALF_FLOAT(void* pPixel);
float load_value_LUMINANCE_FLOAT(void* pPixel);
float load_value_LUMINANCE_UNORM_INT8(void* pPixel);
float load_value_LUMINANCE_UNORM_INT16(void* pPixel);
float load_value_LUMINANCE_HALF_FLOAT(void* pPixel);
float4 load_pixel_INTENSITY_FLOAT(void* pPixel);
float4 load_pixel_INTENSITY_UNORM_INT8(void* pPixel);
float4 load_pixel_INTENSITY_UNORM_INT16(void* pPixel);
float4 load_pixel_INTENSITY_HALF_FLOAT(void* pPixel);
float4 load_pixel_LUMINANCE_FLOAT(void* pPixel);
float4 load_pixel_LUMINANCE_UNORM_INT8(void* pPixel);
float4 load_pixel_LUMINANCE_UNORM_INT16(void* pPixel);
float4 load_pixel_LUMINANCE_HALF_FLOAT(void* pPixel);
float4 load_pixel_RGBA_HALF_FLOAT(void* pPixel);
float4 load_pixel_RGBA_FLOAT(void* pPixel);

float4 load_pixel_BGRA_UNORM_INT8(void* pPixel);
float4 load_pixel_RGBA_UNORM_INT8(void* pPixel);
float4 load_pixel_RGBA_UNORM_INT16(void* pPixel);

int4 load_pixel_R_SIGNED_INT8(void* pPixel);
int4 load_pixel_R_SIGNED_INT16(void* pPixel);
int4 load_pixel_R_SIGNED_INT32(void* pPixel);
float4 load_pixel_R_FLOAT(void* pPixel);
float4 load_pixel_R_HALF_FLOAT(void* pPixel);
uint4 load_pixel_R_UNSIGNED_INT8(void* pPixel);
uint4 load_pixel_R_UNSIGNED_INT16(void* pPixel);
uint4 load_pixel_R_UNSIGNED_INT32(void* pPixel);
float4 load_pixel_R_UNORM_INT8(void* pPixel);
float4 load_pixel_R_UNORM_INT16(void* pPixel);

float4 load_pixel_A_FLOAT(void* pPixel);
float4 load_pixel_A_UNORM_INT8(void* pPixel);
float4 load_pixel_A_UNORM_INT16(void* pPixel);
float4 load_pixel_A_HALF_FLOAT(void* pPixel);

uint4 load_pixel_RG_UNSIGNED_INT8(void* pPixel);
uint4 load_pixel_RG_UNSIGNED_INT16(void* pPixel);
uint4 load_pixel_RG_UNSIGNED_INT32(void* pPixel);
int4 load_pixel_RG_SIGNED_INT8(void* pPixel);
int4 load_pixel_RG_SIGNED_INT16(void* pPixel);
int4 load_pixel_RG_SIGNED_INT32(void* pPixel);
float4 load_pixel_RG_FLOAT(void* pPixel);
float4 load_pixel_RG_UNORM_INT8(void* pPixel);
float4 load_pixel_RG_UNORM_INT16(void* pPixel);
float4 load_pixel_RG_HALF_FLOAT(void* pPixel);

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

// Helper functions to speed up mask analyzing
// tblgen generated
int __attribute__((const)) __attribute__((overloadable)) intel_movemask(int4);
int __attribute__((const)) __attribute__((overloadable)) intel_movemask(int8);

// Implement naive version of transpose built-ins until we have optimized in built-ins library
#ifdef OVERLOAD_TRANSPOSES

// if AVX is not defined then simulate missing transposes
// TODO : move this to transpose_functions.cpp
#if !defined(__AVX__)

// TODO : move this to transpose_functions.cpp
// CSSD100015383
#if !defined(__SSE4_2__) 
void __ocl_transpose_char4x4(char4 xyzw0, char4 xyzw1, char4 xyzw2, char4 xyzw3,
                              char4* xOut, char4* yOut, char4* zOut, char4* wOut) {
 (*xOut).s0 = xyzw0.s0;
 (*xOut).s1 = xyzw1.s0;
 (*xOut).s2 = xyzw2.s0;
 (*xOut).s3 = xyzw3.s0;

 (*yOut).s0 = xyzw0.s1;
 (*yOut).s1 = xyzw1.s1;
 (*yOut).s2 = xyzw2.s1;
 (*yOut).s3 = xyzw3.s1;

 (*zOut).s0 = xyzw0.s2;
 (*zOut).s1 = xyzw1.s2;
 (*zOut).s2 = xyzw2.s2;
 (*zOut).s3 = xyzw3.s2;

 (*wOut).s0 = xyzw0.s3;
 (*wOut).s1 = xyzw1.s3;
 (*wOut).s2 = xyzw2.s3;
 (*wOut).s3 = xyzw3.s3;
}

void __inline__ __attribute__((always_inline)) __ocl_transpose_char4x8(char4 xyzw0, char4 xyzw1, char4 xyzw2, char4 xyzw3,
                              char4 xyzw4, char4 xyzw5, char4 xyzw6, char4 xyzw7,
                              char8* xOut, char8* yOut, char8* zOut, char8* wOut) {
 char4 xLow;
 char4 yLow;
 char4 zLow;
 char4 wLow;

 __ocl_transpose_char4x4(xyzw0, xyzw1, xyzw2, xyzw3,
                              &xLow, &yLow, &zLow, &wLow);

 char4 xHigh;
 char4 yHigh;
 char4 zHigh;
 char4 wHigh;

 __ocl_transpose_char4x4(xyzw0, xyzw1, xyzw2, xyzw3,
                              &xHigh, &yHigh, &zHigh, &wHigh);

 (*xOut).lo = xLow;
 (*xOut).hi = xHigh;
 (*yOut).lo = yLow;
 (*yOut).hi = yHigh;
 (*zOut).lo = zLow;
 (*zOut).hi = zHigh;
 (*wOut).lo = wLow;
 (*wOut).hi = wHigh;
}

void __inline__ __attribute__((always_inline)) __ocl_transpose_char8x4( char8 xIn, char8 yIn, char8 zIn, char8 wIn,
                              char4* xyzw0, char4* xyzw1, char4* xyzw2, char4* xyzw3,
                              char4* xyzw4, char4* xyzw5, char4* xyzw6, char4* xyzw7) {
 char4 xLow = xIn.lo;
 char4 yLow = yIn.lo;
 char4 zLow = zIn.lo;
 char4 wLow = wIn.lo;

 __ocl_transpose_char4x4(xLow, yLow, zLow, wLow,
                            xyzw0, xyzw1, xyzw2, xyzw3);

 char4 xHigh = xIn.hi;
 char4 yHigh = yIn.hi;
 char4 zHigh = zIn.hi;
 char4 wHigh = wIn.hi;

 __ocl_transpose_char4x4(xHigh, yHigh, zHigh, wHigh,
                            xyzw4, xyzw5, xyzw6, xyzw7);
}


void __inline__ __attribute__((always_inline)) __ocl_gather_transpose_char4x4(char4* pLoadAdd0, char4* pLoadAdd1, char4* pLoadAdd2, char4* pLoadAdd3,
                              char4* xOut, char4* yOut, char4* zOut, char4* wOut) {
 char4 xyzw0 = *pLoadAdd0;
 char4 xyzw1 = *pLoadAdd1;
 char4 xyzw2 = *pLoadAdd2;
 char4 xyzw3 = *pLoadAdd3;

 __ocl_transpose_char4x4(xyzw0, xyzw1, xyzw2, xyzw3,
                              xOut, yOut, zOut, wOut);
}

void __inline__ __attribute__((always_inline)) __ocl_transpose_scatter_char4x4(char4* pStoreAdd0, char4* pStoreAdd1, char4* pStoreAdd2, char4* pStoreAdd3,
                               char4 xIn, char4 yIn, char4 zIn, char4 wIn) {
  __ocl_transpose_char4x4(xIn, yIn, zIn, wIn,
                              pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3);
}

void __inline__ __attribute__((always_inline)) __ocl_gather_transpose_char4x8(char4* pLoadAdd0, char4* pLoadAdd1, char4* pLoadAdd2, char4* pLoadAdd3,
                              char4* pLoadAdd4, char4* pLoadAdd5, char4* pLoadAdd6, char4* pLoadAdd7,
                              char8* xOut, char8* yOut, char8* zOut, char8* wOut) {
 char4 xyzw0 = *pLoadAdd0;
 char4 xyzw1 = *pLoadAdd1;
 char4 xyzw2 = *pLoadAdd2;
 char4 xyzw3 = *pLoadAdd3;
 char4 xyzw4 = *pLoadAdd4;
 char4 xyzw5 = *pLoadAdd5;
 char4 xyzw6 = *pLoadAdd6;
 char4 xyzw7 = *pLoadAdd7;

 __ocl_transpose_char4x8( xyzw0, xyzw1, xyzw2, xyzw3,
                    xyzw4, xyzw5, xyzw6, xyzw7,
                    xOut, yOut, zOut, wOut);
}

void __ocl_transpose_scatter_char4x8(char4* pStoreAdd0, char4* pStoreAdd1, char4* pStoreAdd2, char4* pStoreAdd3,
                               char4* pStoreAdd4, char4* pStoreAdd5, char4* pStoreAdd6, char4* pStoreAdd7,
                               char8 xIn, char8 yIn, char8 zIn, char8 wIn) {
  __ocl_transpose_char8x4(xIn, yIn, zIn, wIn,
                              pStoreAdd0, pStoreAdd1, pStoreAdd2, pStoreAdd3,
                              pStoreAdd4, pStoreAdd5, pStoreAdd6, pStoreAdd7);
}

#endif

// simulate masked transposes. they are not implemented in transpose_functions.cpp
void __ocl_masked_gather_transpose_char4x4(char4* pLoadAdd0, char4* pLoadAdd1, char4* pLoadAdd2, char4* pLoadAdd3,
                              char4* xOut, char4* yOut, char4* zOut, char4* wOut, int4 mask)
{
  // get mask as bits in int
  const int rescmp = intel_movemask(mask);
  // ALL 4 elements in mask are -1
  if(rescmp == 0xF){
    __ocl_gather_transpose_char4x4(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                              xOut, yOut, zOut, wOut);
    return;
  }
  // ALL elements in mask are zero
  if(rescmp == 0){
      return;
  }
  // mask addresses to stub variable
  char4 stub;
  pLoadAdd0 = mask.s0 ? pLoadAdd0 : &stub;
  pLoadAdd1 = mask.s1 ? pLoadAdd1 : &stub;
  pLoadAdd2 = mask.s2 ? pLoadAdd2 : &stub;
  pLoadAdd3 = mask.s3 ? pLoadAdd3 : &stub;

  __ocl_gather_transpose_char4x4(pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
                              xOut, yOut, zOut, wOut);
}

void __ocl_masked_gather_transpose_char4x8(char4* pLoadAdd0, char4* pLoadAdd1, char4* pLoadAdd2, char4* pLoadAdd3,
                              char4* pLoadAdd4, char4* pLoadAdd5, char4* pLoadAdd6, char4* pLoadAdd7,
                              char8* xOut, char8* yOut, char8* zOut, char8* wOut, int8 mask)
{
  // get mask as bits in int
  const int rescmp = intel_movemask(mask);
  
  // ALL 8 elements in mask are -1
  if(rescmp == 0xFF){
       __ocl_gather_transpose_char4x8(
           pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
           pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
           xOut, yOut, zOut, wOut);	
       return;
  }
  // ALL elements in mask are zero
  if(rescmp == 0){
      return;
  }
  // mask addresses to stub variable
  char4 stub;
  pLoadAdd0 = mask.s0 ? pLoadAdd0 : &stub;
  pLoadAdd1 = mask.s1 ? pLoadAdd1 : &stub;
  pLoadAdd2 = mask.s2 ? pLoadAdd2 : &stub;
  pLoadAdd3 = mask.s3 ? pLoadAdd3 : &stub;
  pLoadAdd4 = mask.s4 ? pLoadAdd0 : &stub;
  pLoadAdd5 = mask.s5 ? pLoadAdd1 : &stub;
  pLoadAdd6 = mask.s6 ? pLoadAdd2 : &stub;
  pLoadAdd7 = mask.s7 ? pLoadAdd3 : &stub;

  __ocl_gather_transpose_char4x8(
       pLoadAdd0, pLoadAdd1, pLoadAdd2, pLoadAdd3,
       pLoadAdd4, pLoadAdd5, pLoadAdd6, pLoadAdd7,
       xOut, yOut, zOut, wOut);
}

#endif // __AVX__

#endif // OVERLOAD_TRANSPOSES

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
void __attribute__((overloadable)) soa8_extract_pixel_pointer_quad(image2d_t image, int8 coord_x, int8 coord_y, void* pData,
                                   void** p0, void** p1, void** p2, void** p3, void** p4, void** p5, void** p6, void** p7)
{
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*); 
    uint8 offset_x = (uint8)(pImage->offset[0]);
    uint8 offset_y = (uint8)(pImage->offset[1]);
    
    uint8 ocoord_x = (as_uint8(coord_x)) * offset_x;
    uint8 ocoord_y = (as_uint8(coord_y)) * offset_y;

    uint8 ocoord = ocoord_x + ocoord_y;
    *p0 = (char*)pData + ocoord.s0;
    *p1 = (char*)pData + ocoord.s1;
    *p2 = (char*)pData + ocoord.s2;
    *p3 = (char*)pData + ocoord.s3;
    *p4 = (char*)pData + ocoord.s4;
    *p5 = (char*)pData + ocoord.s5;
    *p6 = (char*)pData + ocoord.s6;
    *p7 = (char*)pData + ocoord.s7;
    return;
}

void __attribute__((overloadable)) soa8_load_pixel_RGBA_UNSIGNED_INT8(void* p0,void* p1, void* p2, void* p3, void* p4,void* p5, void* p6, void* p7, 
                                                              uint8* res_x, uint8* res_y, uint8* res_z, uint8* res_w)
{
    uchar8 color_x, color_y, color_z, color_w; // nevermind signed/unsigned.
    __ocl_gather_transpose_char4x8(p0, p1, p2, p3, p4, p5, p6, p7, 
        (char8*)&color_x, (char8*)&color_y, (char8*)&color_z, (char8*)&color_w);
    *res_x = convert_uint8(color_x);
    *res_y = convert_uint8(color_y);
    *res_z = convert_uint8(color_z);
    *res_w = convert_uint8(color_w);
}

void __attribute__((overloadable)) soa8_load_pixel_RGBA_UNSIGNED_INT8_oob(int8 isNotOOB, void* p0,void* p1, void* p2, void* p3, void* p4,void* p5, void* p6, void* p7, 
                                                              uint8* res_x, uint8* res_y, uint8* res_z, uint8* res_w)
{
    uchar8 color_x, color_y, color_z, color_w; // nevermind signed/unsigned.
    __ocl_masked_gather_transpose_char4x8(p0, p1, p2, p3, p4, p5, p6, p7, 
        (char8*)&color_x, (char8*)&color_y, (char8*)&color_z, (char8*)&color_w, isNotOOB);

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
void __attribute__((overloadable)) soa4_extract_pixel_pointer_quad(image2d_t image, int4 coord_x, int4 coord_y, void* pData, void** p1, void** p2, void** p3, void** p4)
{
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*); 
    uint4 offset_x = (uint4)(pImage->offset[0]);
    uint4 offset_y = (uint4)(pImage->offset[1]);
    
    uint4 ocoord_x = (as_uint4(coord_x)) * offset_x;
    uint4 ocoord_y = (as_uint4(coord_y)) * offset_y;

    uint4 ocoord = ocoord_x + ocoord_y;
    *p1 = (char*)pData + ocoord.s0;
    *p2 = (char*)pData + ocoord.s1;
    *p3 = (char*)pData + ocoord.s2;
    *p4 = (char*)pData + ocoord.s3;
    return;
}

void __attribute__((overloadable)) soa4_load_pixel_RGBA_UNSIGNED_INT8(void* pPixel_0,void* pPixel_1, void* pPixel_2, void* pPixel_3, 
                                                              uint4* res_x, uint4* res_y, uint4* res_z, uint4* res_w)
{
    uchar4 color_x, color_y, color_z, color_w; // nevermind signed/unsigned.
    __ocl_gather_transpose_char4x4(pPixel_0, pPixel_1, pPixel_2, pPixel_3, 
        (char4*)&color_x, (char4*)&color_y, (char4*)&color_z, (char4*)&color_w);
    *res_x = convert_uint4(color_x);
    *res_y = convert_uint4(color_y);
    *res_z = convert_uint4(color_z);
    *res_w = convert_uint4(color_w);
}

void __attribute__((overloadable)) soa4_load_pixel_RGBA_UNSIGNED_INT8_oob(int4 isNotOOB, void* pPixel_0,void* pPixel_1, void* pPixel_2, void* pPixel_3, 
                                                              uint4* res_x, uint4* res_y, uint4* res_z, uint4* res_w)
{
    uchar4 color_x, color_y, color_z, color_w; // nevermind signed/unsigned.
    __ocl_masked_gather_transpose_char4x4(pPixel_0, pPixel_1, pPixel_2, pPixel_3, 
        (char4*)&color_x, (char4*)&color_y, (char4*)&color_z, (char4*)&color_w, isNotOOB);
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
    PIX_TYPE##4 read_sample_##FILTER_TYPE##_##CLAMP_FLAG##_##FORMAT(image2d_t, COORD_TYPE##4, void*);\
    void soa##NSOA##_read_sample_##FILTER_TYPE##_##CLAMP_FLAG##_##FORMAT( image2d_t image, COORD_TYPE##NSOA coord_x, COORD_TYPE##NSOA coord_y, void* pData,\
                  PIX_TYPE##NSOA* res_x, PIX_TYPE##NSOA* res_y, PIX_TYPE##NSOA* res_z, PIX_TYPE##NSOA* res_w )\
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
    void soa8_read_sample_NEAREST_NO_CLAMP_##FORMAT( image2d_t image, int8 coord_x, int8 coord_y, void* pData,\
                                                                RETURN_TYPE* res_x, RETURN_TYPE* res_y, RETURN_TYPE* res_z, RETURN_TYPE* res_w )\
{\
    void *p0, *p1, *p2, *p3, *p4, *p5, *p6, *p7;\
    soa8_extract_pixel_pointer_quad(image, coord_x, coord_y, pData, &p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7);\
    soa8_load_pixel_##FORMAT(p0, p1, p2, p3, p4, p5, p6, p7, res_x, res_y, res_z, res_w);\
}

#define IMPLEMENT_SOA8_CBK_NEAREST_CLAMP(FORMAT, RETURN_TYPE)\
    void soa8_read_sample_NEAREST_CLAMP_##FORMAT( image2d_t image, int8 coord_x, int8 coord_y, void* pData,\
                                                                RETURN_TYPE* res_x, RETURN_TYPE* res_y, RETURN_TYPE* res_z, RETURN_TYPE* res_w )\
{\
    int8 isNotOOB = soa8_isInsideBoundsInt(image, coord_x, coord_y);\
    void *p0, *p1, *p2, *p3, *p4, *p5, *p6, *p7;\
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
    void soa4_read_sample_NEAREST_NO_CLAMP_##FORMAT( image2d_t image, int4 coord_x, int4 coord_y, void* pData,\
                                                                RETURN_TYPE* res_x, RETURN_TYPE* res_y, RETURN_TYPE* res_z, RETURN_TYPE* res_w )\
{\
    void *p0, *p1, *p2, *p3;\
    soa4_extract_pixel_pointer_quad(image, coord_x, coord_y, pData, &p0, &p1, &p2, &p3);\
    soa4_load_pixel_##FORMAT(p0, p1, p2, p3, res_x, res_y, res_z, res_w);\
}

#define IMPLEMENT_SOA4_CBK_NEAREST_CLAMP(FORMAT, RETURN_TYPE)\
    void soa4_read_sample_NEAREST_CLAMP_##FORMAT( image2d_t image, int4 coord_x, int4 coord_y, void* pData,\
                                                                RETURN_TYPE* res_x, RETURN_TYPE* res_y, RETURN_TYPE* res_z, RETURN_TYPE* res_w )\
{\
    int4 isNotOOB = soa4_isInsideBoundsInt(image, coord_x, coord_y);\
    void *p0, *p1, *p2, *p3;\
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
RETURN_TYPE read_sample_NEAREST_NO_CLAMP_##FORMAT(image2d_t image, int4 coord, void* pData)\
{\
    void* pixel = extract_pixel_pointer_quad(image, coord, pData);\
    return load_pixel_##FORMAT(pixel);\
}\
\
RETURN_TYPE read_sample_NEAREST_CLAMP_##FORMAT(image2d_t image, int4 coord, void* pData)\
{\
    int isOOB = isOutOfBoundsInt(image, coord);\
    if (isOOB)\
        return BORDER_COLOR;\
    void* pixel = extract_pixel_pointer_quad(image, coord, pData);\
    return load_pixel_##FORMAT(pixel);\
}

#define IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(FORMAT, BORDER_COLOR)\
float4 read_sample_NEAREST_NO_CLAMP_##FORMAT(image2d_t image, int4 coord, int4 dummy0, float4 dummy1, void* pData)\
{\
    void* pixel = extract_pixel_pointer_quad(image, coord, pData);\
    return load_pixel_##FORMAT(pixel);\
}\
\
float4 read_sample_NEAREST_CLAMP_##FORMAT(image2d_t image, int4 coord, int4 dummy0, float4 dummy1, void* pData)\
{\
    int isOOB = isOutOfBoundsInt(image, coord);\
    if (isOOB)\
        return BORDER_COLOR;\
    void* pixel = extract_pixel_pointer_quad(image, coord, pData);\
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
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RG_FLOAT, BorderColorAlphaFloat)
IMPLEMENT_READ_SAMPLE_NEAREST_FLOAT(RG_HALF_FLOAT, BorderColorAlphaFloat)

void write_sample_RGBA_UNSIGNED_INT8(void* pixel, uint4 color)
{
    color = min(color, (uint4)(UCHAR_MAX));
    *(char4*)pixel = __ocl_trunc_v4i32_v4i8(*((int4*)&color));
}

void write_sample_RG_UNSIGNED_INT8(void* pixel, uint4 color)
{
    const __m128i i4uint8Max = _mm_set1_epi32(UCHAR_MAX);
    __m128i i4Val=(__m128i)color;
    i4Val = (__m128i)min(as_int4(i4Val), as_int4(i4uint8Max));
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
}

void write_sample_R_UNSIGNED_INT8(void* pixel, uint4 color)
{
    const __m128i i4uint8Max = _mm_set1_epi32(UCHAR_MAX);
    __m128i i4Val=(__m128i)color;
    i4Val = (__m128i)min(as_int4(i4Val), as_int4(i4uint8Max));
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
}

/*****************************RGBA_UNSIGNED_INT16 Image type i/o functions****************************************************/

uint4 load_pixel_RGBA_UNSIGNED_INT16(void* pPixel)
{
    __m128i i4Val = _mm_loadl_epi64((__m128i*)pPixel);
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    return as_uint4(i4Val);
}

void write_sample_RGBA_UNSIGNED_INT16(void* pixel, uint4 color)
{
    __m128i i4Val = (__m128i)min(color, i4uint16Max);
    /// pack values to pixels
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+2) = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+3) = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

void write_sample_RG_UNSIGNED_INT16(void* pixel, uint4 color)
{
    __m128i i4Val = (__m128i)min(color, i4uint16Max);
    /// pack values to pixels
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

void write_sample_R_UNSIGNED_INT16(void* pixel, uint4 color)
{
    __m128i i4Val = (__m128i)min(color, i4uint16Max);
    /// pack values to pixels
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

/*****************************RGBA_UNSIGNED_INT32 Image type i/o functions****************************************************/

uint4 load_pixel_RGBA_UNSIGNED_INT32(void* pPixel)
{
    return (*((uint4*)pPixel));
}

void write_sample_RGBA_UNSIGNED_INT32(void* pixel, uint4 color)
{
    (*(uint4*)pixel)=color;
}

void write_sample_R_UNSIGNED_INT32(void* pixel, uint4 color)
{
    (*(uint*)pixel)=color.x;
}

void write_sample_RG_UNSIGNED_INT32(void* pixel, uint4 color)
{
    (*(uint2*)pixel)=color.lo;
}

/*******************************************************************SIGNED IMAGE TYPES I/IO*****************************************************************************/


/*****************************RGBA_SIGNED_INT8 Image type i/o functions****************************************************/


int4 load_pixel_RGBA_SIGNED_INT8(void* pPixel)
{
    __m128i i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPixel);
    i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    // Extend sign
    i4Val = _mm_slli_si128(i4Val, 3);
    i4Val = _mm_srai_epi32(i4Val, 24);
    return as_int4(i4Val);
}

void write_sample_RGBA_SIGNED_INT8(void* pixel, int4 color)
{
    __m128i i4Val = (__m128i)max(color, i4int16Min);
    i4Val = (__m128i)min(as_int4(i4Val), i4int16Max);
    i4Val = _mm_packs_epi32(i4Val, i4Val);
    i4Val = _mm_packs_epi16(i4Val, i4Val);
    *(unsigned int*)pixel = _mm_cvtsi128_si32(i4Val);
}

void write_sample_R_SIGNED_INT8(void* pixel, int4 color)
{
    __m128i i4Val = (__m128i)max(color, i4int16Min);
    i4Val = (__m128i)min(as_int4(i4Val), i4int16Max);
    i4Val = _mm_packs_epi32(i4Val, i4Val);
    i4Val = _mm_packs_epi16(i4Val, i4Val);
    *(char*)pixel = ((char4)_mm_cvtsi128_si32(i4Val)).x;
}

void write_sample_RG_SIGNED_INT8(void* pixel, int4 color)
{
    __m128i i4Val = (__m128i)max(color, i4int16Min);
    i4Val = (__m128i)min(as_int4(i4Val), i4int16Max);
    i4Val = _mm_packs_epi32(i4Val, i4Val);
    i4Val = _mm_packs_epi16(i4Val, i4Val);
    *(unsigned short*)pixel = ((ushort2)_mm_cvtsi128_si32(i4Val)).x;
}
/*****************************RGBA_SIGNED_INT16 Image type i/o functions****************************************************/

int4 load_pixel_RGBA_SIGNED_INT16(void* pPixel)
{
    __m128i i4Val = _mm_loadl_epi64((__m128i*)pPixel);
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    // Extend sign
    i4Val = _mm_slli_si128(i4Val, 2);
    i4Val = _mm_srai_epi32(i4Val, 16);
    return as_int4(i4Val);
}

void write_sample_RGBA_SIGNED_INT16(void* pixel, int4 color)
{
    __m128i i4Val = (__m128i)color;
    i4Val = (__m128i)max(as_int4(i4Val), i4int16Min);
    i4Val = (__m128i)min(as_int4(i4Val), i4int16Max);
    // Shrink to 8bit
    i4Val = _mm_packs_epi32(i4Val, i4Val);
    _mm_storel_epi64((__m128i*)pixel, i4Val);
}

void write_sample_RG_SIGNED_INT16(void* pixel, int4 color)
{
    __m128i i4Val = (__m128i)color;
    i4Val = (__m128i)max(as_int4(i4Val), i4int16Min);
    i4Val = (__m128i)min(as_int4(i4Val), i4int16Max);
    // i4Val already contains valid short value
    (*(short*)pixel)=(as_int4(i4Val)).x;
    ((short*)pixel)[1]=(as_int4(i4Val)).y;
}

void write_sample_R_SIGNED_INT16(void* pixel, int4 color)
{
    __m128i i4Val = (__m128i)color;
    i4Val = (__m128i)max(as_int4(i4Val), i4int16Min);
    i4Val = (__m128i)min(as_int4(i4Val), i4int16Max);
    // i4Val already contains valid short value
    (*(short*)pixel)=(as_int4(i4Val)).x;
}

/*****************************RGBA_SIGNED_INT32 Image type i/o functions****************************************************/

int4 load_pixel_RGBA_SIGNED_INT32(void* pPixel)
{
    return (*((int4*)pPixel));
}

void write_sample_RGBA_SIGNED_INT32(void* pixel, int4 color)
{
    (*(int4*)pixel)=color;
}

void write_sample_R_SIGNED_INT32(void* pixel, int4 color)
{
    (*(int*)pixel)=color.x;
}

void write_sample_RG_SIGNED_INT32(void* pixel, int4 color)
{
    (*(int2*)pixel)=color.lo;
}

/*****************************************************************UNORM IMAGES TYPES I/O*****************************************************************/


/***************************************RGBA_UNORM8 Image type i/o functions*****************************************************/

float4 load_pixel_RGBA_UNORM_INT8(void* pPixel)
{
    __m128i i4Val = (__m128i)_mm_cvtsi32_si128(*(unsigned int*)pPixel);
    i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    float4 converted = _mm_cvtepi32_ps(i4Val);
    converted = converted*(float4)(1.0f/255.0f);
    return converted;
}

void write_sample_RGBA_UNORM_INT8(void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, (__m128)f4unorm8mul, (__m128)f4unorm8lim);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+2) = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+3) = (unsigned char)_mm_cvtsi128_si32(i4Val);
}

void write_sample_RG_UNORM_INT8(void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm8mul, f4unorm8lim);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
}

void write_sample_R_UNORM_INT8(void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm8mul, f4unorm8lim);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
}

void write_sample_A_UNORM_INT8(void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm8mul, f4unorm8lim);
    i4Val = _mm_srli_si128(i4Val, 12);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
}

/***************************************RGBA_UNORM16 Image type i/o functions*****************************************************/

float4 load_pixel_RGBA_UNORM_INT16(void* pPixel)
{

    __m128i i4Val = _mm_loadl_epi64((__m128i*)pPixel);
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    __m128 f4Val = _mm_cvtepi32_ps(i4Val);
    return _mm_mul_ps(f4Val, f4Unorm16Dim);
}

void write_sample_RGBA_UNORM_INT16(void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, (__m128)f4unorm16mul, (__m128)f4unorm16lim);
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+2) = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+3) = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

void write_sample_RG_UNORM_INT16(void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm16mul, f4unorm16lim);
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned short*)pixel+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

void write_sample_R_UNORM_INT16(void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm16mul, f4unorm16lim);
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

void write_sample_A_UNORM_INT16(void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm16mul, f4unorm16lim);
    i4Val = _mm_srli_si128(i4Val, 12);
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
}



/***************************************BGRA_UNORM8 Image type i/o functions*****************************************************/

float4 load_pixel_BGRA_UNORM_INT8(void* pPixel)
{

    __m128i i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPixel);
    i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
    i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
    i4Val = (__m128i)_mm_cvtepi32_ps(i4Val);
    i4Val = (__m128i)_mm_mul_ps((__m128)i4Val, (__m128)f4Unorm8Dim);
    i4Val = _mm_shuffle_epi32(i4Val, _MM_SHUFFLE(3, 0, 1, 2));
    return as_float4(i4Val);
}

void write_sample_BGRA_UNORM_INT8(void* pixel, float4 color)
{
    float4 convertedColor = color.zyxw;
    __m128i i4Val = cvt_to_norm((__m128i)convertedColor, (__m128)f4unorm8mul, (__m128)f4unorm8lim);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+2) = (unsigned char)_mm_cvtsi128_si32(i4Val);
    i4Val = _mm_srli_si128(i4Val, 4);
    *((unsigned char*)pixel+3) = (unsigned char)_mm_cvtsi128_si32(i4Val);
}



/****************************************************************FLOAT IMAGE TYPES I/O***************************************************************/


/******************************************RGBA_FLOAT image type i/o functions******************************************************/

float4 load_pixel_RGBA_FLOAT(void* pPixel)
{
    return *((float4*)pPixel);
}

void write_sample_RGBA_FLOAT(void* pixel, float4 color)
{
    (*(float4*)pixel)=color;
}

/******************************************RGBA_HALF_FLOAT image type i/o functions******************************************************/

float4 load_pixel_RGBA_HALF_FLOAT(void* pPixel)
{
    return (float4)vloada_half4(0, (half*)pPixel);
}

void write_sample_RGBA_HALF_FLOAT(void* pixel, float4 color)
{
    vstore_half4(color, 0, (half*)pixel);
}

void write_sample_R_HALF_FLOAT(void* pixel, float4 color)
{
    vstore_half(color.x, 0, (half*)pixel);
}

void write_sample_RG_HALF_FLOAT(void* pixel, float4 color)
{
    vstore_half2(color.lo, 0, (half*)pixel);
}

void write_sample_A_HALF_FLOAT(void* pixel, float4 color)
{
    vstore_half(color.w, 0, (half*)pixel); // store alpha channel from pixel (0,0,0,a)
}

void write_sample_LUMINANCE_HALF_FLOAT(void* pixel, float4 color)
{
    vstore_half(color.x, 0, (half*)pixel);
}

void write_sample_INTENSITY_HALF_FLOAT(void* pixel, float4 color)
{
    vstore_half(color.x, 0, (half*)pixel);
}

/******************************************LUMINANCE image type i/o functions******************************************************/

float load_value_LUMINANCE_FLOAT(void* pPixel)
{
    float luminance = *((float*)pPixel);
    return luminance;
}

float4 load_pixel_LUMINANCE_FLOAT(void* pPixel)
{
    float luminance = load_value_LUMINANCE_FLOAT(pPixel);
    float4 res = (float4)(luminance, luminance, luminance, 1.0f);
    return res;
}

float load_value_LUMINANCE_UNORM_INT8(void* pPixel)
{
    uchar val = *(uchar*)pPixel;
    return val * (1.0f/255.0f);
}

float4 load_pixel_LUMINANCE_UNORM_INT8(void* pPixel)
{
    float luminance = load_value_LUMINANCE_UNORM_INT8(pPixel);
    return (float4)(luminance, luminance, luminance, 1.0f);
}

float load_value_LUMINANCE_UNORM_INT16(void* pPixel)
{
    ushort val = *(ushort*)pPixel;
    return val * (1.0f/65535.0f);
}

float4 load_pixel_LUMINANCE_UNORM_INT16(void* pPixel)
{
    float luminance = load_value_LUMINANCE_UNORM_INT16(pPixel);
    return (float4)(luminance, luminance, luminance, 1.0f);
}

float load_value_LUMINANCE_HALF_FLOAT(void* pPixel)
{
    float val = vloada_half(0, (half*)pPixel);
    return val;
}

float4 load_pixel_LUMINANCE_HALF_FLOAT(void* pPixel)
{
    float val = load_value_LUMINANCE_HALF_FLOAT(pPixel);
    return (float4)(val, val, val, 1.f);
}

float load_value_INTENSITY_UNORM_INT8(void* pPixel)
{
    uchar val = *(uchar*)pPixel;
    return val * (1.0f/255.0f);
}

float4 load_pixel_INTENSITY_UNORM_INT8(void* pPixel)
{
    float intensity = load_value_INTENSITY_UNORM_INT8(pPixel);
    return (float4)(intensity, intensity, intensity, intensity);
}

float load_value_INTENSITY_UNORM_INT16(void* pPixel)
{
    ushort val = *(ushort*)pPixel;
    return val * (1.0f/65535.0f);
}

float4 load_pixel_INTENSITY_UNORM_INT16(void* pPixel)
{
    float intensity = load_value_INTENSITY_UNORM_INT16(pPixel);
    return (float4)(intensity, intensity, intensity, intensity);
}

float load_value_INTENSITY_HALF_FLOAT(void* pPixel)
{
    float val = vloada_half(0, (half*)pPixel);
    return val;
}

float4 load_pixel_INTENSITY_HALF_FLOAT(void* pPixel)
{
    float val = load_value_INTENSITY_HALF_FLOAT(pPixel);
    return (float4)(val, val, val, val);
}

void write_sample_LUMINANCE_FLOAT(void* pixel, float4 color)
{
    (*(float*)pixel)=color.x;
}

/******************************************INTENSITY image type i/o functions******************************************************/

float load_value_INTENSITY_FLOAT(void* pPixel)
{
    float intensity = *((float*)pPixel);
    return intensity;
}

float4 load_pixel_INTENSITY_FLOAT(void* pPixel)
{
    float intensity = load_value_INTENSITY_FLOAT(pPixel);
    return (float4)intensity;
}

void write_sample_INTENSITY_FLOAT(void* pixel, float4 color)
{
    (*(float*)pixel)=color.x;
}

void write_sample_INTENSITY_UNORM_INT8(void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm8mul, f4unorm8lim);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
}

void write_sample_INTENSITY_UNORM_INT16(void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm16mul, f4unorm16lim);
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

void write_sample_LUMINANCE_UNORM_INT8(void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm8mul, f4unorm8lim);
    *(unsigned char*)pixel = (unsigned char)_mm_cvtsi128_si32(i4Val);
}

void write_sample_LUMINANCE_UNORM_INT16(void* pixel, float4 color)
{
    __m128i i4Val = cvt_to_norm((__m128i)color, f4unorm16mul, f4unorm16lim);
    *(unsigned short*)pixel = (unsigned short)_mm_cvtsi128_si32(i4Val);
}

void write_sample_R_FLOAT(void* pixel, float4 color)
{
    (*(float*)pixel)=color.x;
}

void write_sample_RG_FLOAT(void* pixel, float4 color)
{
    (*(float2*)pixel)=color.lo;
}

void write_sample_A_FLOAT(void* pixel, float4 color)
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


#define IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(TYPE, POST_PROCESSING) \
    float4 read_sample_LINEAR1D_NO_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
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
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(R_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(R_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(R_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(R_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(A_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(A_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(A_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(A_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RG_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RG_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RG_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR1D_NO_CLAMP(RG_UNORM_INT16, dummyFnc)


// definition for linear read callbacks in case of one channel images
#define IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP_CH1(TYPE, POST_PROCESSING) \
    float4 read_sample_LINEAR2D_NO_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
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
    float4 read_sample_LINEAR2D_NO_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
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
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(R_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(R_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(R_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(R_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(A_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(A_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(A_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(A_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RG_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RG_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RG_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR2D_NO_CLAMP(RG_UNORM_INT16, dummyFnc)


#define IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(TYPE, POST_PROCESSING) \
float4 read_sample_LINEAR3D_NO_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
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
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(BGRA_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RGBA_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(R_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(R_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(R_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(R_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(A_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(A_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(A_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(A_HALF_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RG_FLOAT, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RG_UNORM_INT8, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RG_UNORM_INT16, dummyFnc)
IMPLEMENT_read_sample_LINEAR3D_NO_CLAMP(RG_HALF_FLOAT, dummyFnc)


#define IMPLEMENT_read_sample_LINEAR1D_CLAMP(TYPE, BORDER_COLOR, POST_PROCESSING) \
    float4 read_sample_LINEAR1D_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
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
    float4 read_sample_LINEAR2D_NO_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
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
    float4 read_sample_LINEAR2D_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
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
float4 read_sample_LINEAR3D_CLAMP_##TYPE(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
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
uint4 load_pixel_R_UNSIGNED_INT32(void* pPixel)
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
uint4 load_pixel_RG_UNSIGNED_INT32(void* pPixel)
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
int4 load_pixel_R_SIGNED_INT32(void* pPixel)
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
int4 load_pixel_RG_SIGNED_INT32(void* pPixel)
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
float4 load_pixel_R_FLOAT(void* pPixel)
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
float4 load_pixel_RG_FLOAT(void* pPixel)
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
float4 load_pixel_A_FLOAT(void* pPixel)
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
float4 load_pixel_A_HALF_FLOAT(void* pPixel)
{
    float val = vloada_half(0, (half*)pPixel);

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
float4 load_pixel_R_HALF_FLOAT(void* pPixel)
{
    float val = vloada_half(0, (half*)pPixel);

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
float4 load_pixel_RG_HALF_FLOAT(void* pPixel)
{
    float2 val = vloada_half2(0, (half*)pPixel);

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
uint4 load_pixel_RGBA_UNSIGNED_INT8(void* pPixel)
{
    char4 color = *(char4*)pPixel; // nevermind signed/unsigned.
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
uint4 load_pixel_R_UNSIGNED_INT8(void* pPixel)
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
uint4 load_pixel_RG_UNSIGNED_INT8(void* pPixel)
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
uint4 load_pixel_R_UNSIGNED_INT16(void* pPixel)
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
uint4 load_pixel_RG_UNSIGNED_INT16(void* pPixel)
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
int4 load_pixel_R_SIGNED_INT8(void* pPixel)
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
int4 load_pixel_RG_SIGNED_INT8(void* pPixel)
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
float4 load_pixel_A_UNORM_INT8(void* pPixel)
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
float4 load_pixel_A_UNORM_INT16(void* pPixel)
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
float4 load_pixel_RG_UNORM_INT8(void* pPixel)
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
float4 load_pixel_R_UNORM_INT8(void* pPixel)
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
float4 load_pixel_RG_UNORM_INT16(void* pPixel)
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
float4 load_pixel_R_UNORM_INT16(void* pPixel)
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
int4 load_pixel_R_SIGNED_INT16(void* pPixel)
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
int4 load_pixel_RG_SIGNED_INT16(void* pPixel)
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
void* extract_pixel_pointer_quad(image2d_t image, int4 coord, void* pData)
{
    // Calculate required pixel offset
    image_aux_data *pImage = __builtin_astype(image, image_aux_data*);
    uint4 offset = *((uint4*)(&pImage->offset));
    uint4 ocoord = (as_uint4(coord)) * offset;
    void* pixel = pData + ocoord.x + ocoord.y + ocoord.z;
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
void soa8_write_sample_RGBA_UNSIGNED_INT8(void* p0, void* p1, void* p2, void* p3, 
                                                                        void* p4, void* p5, void* p6, void* p7, 
                                                                        uint8 val_x, uint8 val_y, uint8 val_z, uint8 val_w)
{

    uchar8 clr_x = convert_uchar8_sat(val_x);
    uchar8 clr_y = convert_uchar8_sat(val_y);
    uchar8 clr_z = convert_uchar8_sat(val_z);
    uchar8 clr_w = convert_uchar8_sat(val_w);

    __ocl_transpose_scatter_char4x8((char4*)p0, (char4*)p1, (char4*)p2, (char4*)p3,
                              (char4*)p4, (char4*)p5, (char4*)p6, (char4*)p7,
                              as_char8(clr_x), as_char8(clr_y), as_char8(clr_z), as_char8(clr_w));
}

/// SOA4 write RGBA_UNSIGNED_INT8
void soa4_write_sample_RGBA_UNSIGNED_INT8(void* p0, void* p1, void* p2, void* p3, uint4 val_x, uint4 val_y, uint4 val_z, uint4 val_w)
{

    uchar4 clr_x = convert_uchar4_sat(val_x);
    uchar4 clr_y = convert_uchar4_sat(val_y);
    uchar4 clr_z = convert_uchar4_sat(val_z);
    uchar4 clr_w = convert_uchar4_sat(val_w);

    __ocl_transpose_scatter_char4x4((char4*)p0, (char4*)p1, (char4*)p2, (char4*)p3,
                              as_char4(clr_x), as_char4(clr_y), as_char4(clr_z), as_char4(clr_w));
}


#define SCALARIZE_SOA4_WRITE_SAMPLE(FORMAT, PIX_TYPE)\
    void soa4_write_sample_##FORMAT(\
           void* p0, void* p1, void* p2, void* p3,\
           PIX_TYPE##4 val_x, PIX_TYPE##4 val_y, PIX_TYPE##4 val_z, PIX_TYPE##4 val_w){\
      write_sample_##FORMAT(p0, (PIX_TYPE##4)(val_x.s0, val_y.s0, val_z.s0, val_w.s0));\
      write_sample_##FORMAT(p1, (PIX_TYPE##4)(val_x.s1, val_y.s1, val_z.s1, val_w.s1));\
      write_sample_##FORMAT(p2, (PIX_TYPE##4)(val_x.s2, val_y.s2, val_z.s2, val_w.s2));\
      write_sample_##FORMAT(p3, (PIX_TYPE##4)(val_x.s3, val_y.s3, val_z.s3, val_w.s3));\
}

#define SCALARIZE_SOA8_WRITE_SAMPLE(FORMAT, PIX_TYPE)\
    void soa8_write_sample_##FORMAT(\
           void* p0, void* p1, void* p2, void* p3,\
           void* p4, void* p5, void* p6, void* p7,\
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
void soa4_read_sample_UNDEFINED_QUAD_INT( image2d_t image, int4 coord_x, int4 coord_y, void* pData, 
                                                                               uint4* res_x, uint4* res_y, uint4* res_z, uint4* res_w )
{
    *res_x = BorderColorNoAlphaUint.x;
    *res_y = BorderColorNoAlphaUint.y;
    *res_z = BorderColorNoAlphaUint.z;
    *res_w = BorderColorNoAlphaUint.w;
}

void soa8_read_sample_UNDEFINED_QUAD_INT( image2d_t image, int8 coord_x, int8 coord_y, void* pData, 
                                                                               uint8* res_x, uint8* res_y, uint8* res_z, uint8* res_w )
{
    *res_x = BorderColorNoAlphaUint.x;
    *res_y = BorderColorNoAlphaUint.y;
    *res_z = BorderColorNoAlphaUint.z;
    *res_w = BorderColorNoAlphaUint.w;
}

uint4 read_sample_UNDEFINED_QUAD_INT(image2d_t image, int4 coord, void* pData)
{
    return BorderColorNoAlphaUint;  //return all zeros vector
}

float4 read_sample_UNDEFINED_QUAD_FLOAT(image2d_t image, int4 square0, int4 square1, float4 fraction, void* pData)  \
{
    return BorderColorNoAlphaFloat;  //return all zeros vector
}

void trap_function()
{
    printf ("***Runtime error: reached an uninitialized image function***\n");
    __builtin_debugtrap();
}

#endif // defined (__MIC__) || defined(__MIC2__)

