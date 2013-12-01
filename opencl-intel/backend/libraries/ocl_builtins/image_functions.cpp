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

#pragma OPENCL EXTENSION cl_khr_fp64         : enable
#pragma OPENCL EXTENSION cl_khr_depth_images : enable

#define __OPENCL__
#include <intrin.h>

#include "cl_image_declaration.h"


#define NORMALIZED_SAMPLER 0x08

#define ALIGN16 __attribute__ ((aligned(16)))

const constant int fVec4FloatZeroCoordMask3D[4] ALIGN16 = {0xffffffff, 0xffffffff, 0xffffffff, 0};
ALIGN16 const constant int4 UndefCoordInt = {0, 0, 0, 0};
ALIGN16 const constant int4 SOA4_UndefCoordIntX = {0, 0, 0, 0};
ALIGN16 const constant int4 SOA4_UndefCoordIntY = {0, 0, 0, 0};
ALIGN16 const constant int8 SOA8_UndefCoordIntX = {0, 0, 0, 0, 0, 0, 0, 0};
ALIGN16 const constant int8 SOA8_UndefCoordIntY = {0, 0, 0, 0, 0, 0, 0, 0};
#define SIMPLE_SAMPLER NONE_FALSE_NEAREST

/// image properties functions
#define IMG_GET_PARAM(FUNC_NAME, IMG_TYPE, PARAM_TYPE, PARAM)\
PARAM_TYPE __attribute__((overloadable)) __attribute__((const)) FUNC_NAME(IMG_TYPE img)\
{\
    __private image_aux_data *pImage = __builtin_astype(img, __private image_aux_data*);\
    return (pImage->PARAM);\
}\
PARAM_TYPE##4 __attribute__((overloadable)) __attribute__((const)) soa4_##FUNC_NAME(IMG_TYPE img)\
{\
    __private image_aux_data *pImage = __builtin_astype(img, __private image_aux_data*);\
    return (PARAM_TYPE##4)(pImage->PARAM);\
}\
PARAM_TYPE##8 __attribute__((overloadable)) __attribute__((const)) soa8_##FUNC_NAME(IMG_TYPE img)\
{\
    __private image_aux_data *pImage = __builtin_astype(img, __private image_aux_data*);\
    return (PARAM_TYPE##8)(pImage->PARAM);\
}

IMG_GET_PARAM(get_image_width, image1d_t, int, dim[0])
IMG_GET_PARAM(get_image_width, image1d_array_t, int, dim[0])
IMG_GET_PARAM(get_image_width, image1d_buffer_t, int, dim[0])
IMG_GET_PARAM(get_image_width, image2d_t, int, dim[0])
IMG_GET_PARAM(get_image_width, image2d_array_t, int, dim[0])
IMG_GET_PARAM(get_image_width, image2d_depth_t, int, dim[0])
IMG_GET_PARAM(get_image_width, image2d_array_depth_t, int, dim[0])
IMG_GET_PARAM(get_image_width, image3d_t, int, dim[0])

IMG_GET_PARAM(get_image_height, image2d_t, int, dim[1])
IMG_GET_PARAM(get_image_height, image2d_array_t, int, dim[1])
IMG_GET_PARAM(get_image_height, image2d_depth_t, int, dim[1])
IMG_GET_PARAM(get_image_height, image2d_array_depth_t, int, dim[1])
IMG_GET_PARAM(get_image_height, image3d_t, int, dim[1])

IMG_GET_PARAM(get_image_depth, image3d_t, int, dim[2])

IMG_GET_PARAM(get_image_channel_data_type, image1d_t, int,        format.image_channel_data_type)
IMG_GET_PARAM(get_image_channel_data_type, image1d_array_t, int,  format.image_channel_data_type)
IMG_GET_PARAM(get_image_channel_data_type, image1d_buffer_t, int, format.image_channel_data_type)
IMG_GET_PARAM(get_image_channel_data_type, image2d_t, int,        format.image_channel_data_type)
IMG_GET_PARAM(get_image_channel_data_type, image2d_array_t, int,  format.image_channel_data_type)
IMG_GET_PARAM(get_image_channel_data_type, image2d_depth_t, int,        format.image_channel_data_type)
IMG_GET_PARAM(get_image_channel_data_type, image2d_array_depth_t, int,  format.image_channel_data_type)
IMG_GET_PARAM(get_image_channel_data_type, image3d_t, int,        format.image_channel_data_type)

IMG_GET_PARAM(get_image_channel_order, image1d_t, int,        format.image_channel_order)
IMG_GET_PARAM(get_image_channel_order, image1d_array_t, int,  format.image_channel_order)
IMG_GET_PARAM(get_image_channel_order, image1d_buffer_t, int, format.image_channel_order)
IMG_GET_PARAM(get_image_channel_order, image2d_t, int,        format.image_channel_order)
IMG_GET_PARAM(get_image_channel_order, image2d_array_t, int,  format.image_channel_order)
IMG_GET_PARAM(get_image_channel_order, image2d_depth_t, int,        format.image_channel_order)
IMG_GET_PARAM(get_image_channel_order, image2d_array_depth_t, int,  format.image_channel_order)
IMG_GET_PARAM(get_image_channel_order, image3d_t, int,        format.image_channel_order)

// this is a common function for image2d_t, image2d_array_t, image2d_depth_t, and image2d_array_depth_t
int2 __attribute__((overloadable)) __attribute__((const)) get_image2d_dim_raw(__private image_aux_data *pImage)
{
    int2 res;
    res.lo = pImage->dim[0];
    res.hi = pImage->dim[1];
    return res;
}

int2 __attribute__((overloadable)) __attribute__((const)) get_image_dim(image2d_depth_t img)
{
    return get_image2d_dim_raw(__builtin_astype(img, __private image_aux_data*));
}

int2 __attribute__((overloadable)) __attribute__((const)) get_image_dim(image2d_array_depth_t img)
{
    return get_image2d_dim_raw(__builtin_astype(img, __private image_aux_data*));
}

int2 __attribute__((overloadable)) __attribute__((const)) get_image_dim(image2d_t img)
{
    return get_image2d_dim_raw(__builtin_astype(img, __private image_aux_data*));
}

int2 __attribute__((overloadable)) __attribute__((const)) get_image_dim(image2d_array_t img)
{
    return get_image2d_dim_raw(__builtin_astype(img, __private image_aux_data*));
}

 int4 __attribute__((overloadable)) __attribute__((const)) get_image_dim(image3d_t img)
{
    __private image_aux_data *pImage = __builtin_astype(img, __private image_aux_data*);
    __m128i dim = _mm_lddqu_si128((__m128i*)pImage->dim);
    // Set to 0 the highest DWORD
    dim = _mm_srli_si128(_mm_slli_si128(dim, 4),4);

    return as_int4(dim);
}

size_t __attribute__((overloadable)) __attribute__((const)) get_image_array_size(image2d_array_t img)
{
        __private image_aux_data *pImage = __builtin_astype(img, __private image_aux_data*);
        return pImage->array_size;
}

size_t __attribute__((overloadable)) __attribute__((const)) get_image_array_size(image1d_array_t img)
{
        __private image_aux_data *pImage = __builtin_astype(img, __private image_aux_data*);
        return pImage->array_size;
}

size_t __attribute__((overloadable)) __attribute__((const)) get_image_array_size(image2d_array_depth_t img)
{
        __private image_aux_data *pImage = __builtin_astype(img, __private image_aux_data*);
        return pImage->array_size;
}

//// Auxiliary built-in functions

int4 __attribute__((overloadable)) ProjectToEdgeInt(image2d_t image, int4 coord)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    int4 upper = *((int4*)(&pImage->dimSub1));
    int4 lower = (int4)(0, 0, 0, 0);

    int4 correctCoord=min(coord, upper);
    correctCoord=max(correctCoord,lower);
    return correctCoord;
}


int4 __attribute__((overloadable)) ProjectToEdgeInt(image2d_depth_t image, int4 coord)
{
    image2d_t proxy = __builtin_astype(image, image2d_t);
    return ProjectToEdgeInt(proxy, coord);
}
// Clamps SOA4 coordinates to be inside image
//
// @param [in] image: the image object
// @param [in] coord_(x,y) coordinates of the pixel 
// @param [out] res_(x,y) output coordinates
void __attribute__((overloadable)) SOA4_ProjectToEdgeInt(image2d_t image, int4 coord_x, int4 coord_y, __private int4* res_x, __private int4* res_y)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    int4 upper_x = (int4)(pImage->dimSub1[0]);
    int4 upper_y = (int4)(pImage->dimSub1[1]);
    int4 lower = (int4)(0, 0, 0, 0);
    coord_x = clamp(coord_x, lower, upper_x);
    coord_y = clamp(coord_y, lower, upper_y);
    *res_x = coord_x;
    *res_y = coord_y;
}

// Clamps SOA8 coordinates to be inside image
//
// @param [in] image: the image object
// @param [in] coord_(x,y) coordinates of the pixel 
// @param [out] res_(x,y) output coordinates
void __attribute__((overloadable)) SOA8_ProjectToEdgeInt(image2d_t image, int8 coord_x, int8 coord_y, __private int8* res_x, __private int8* res_y)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    int8 upper_x = (int8)(pImage->dimSub1[0]);
    int8 upper_y = (int8)(pImage->dimSub1[1]);
    int8 lower = (int8)(0, 0, 0, 0, 0, 0, 0, 0);
    coord_x = clamp(coord_x, lower, upper_x);
    coord_y = clamp(coord_y, lower, upper_y);
    *res_x = coord_x;
    *res_y = coord_y;
}

__private void* __attribute__((overloadable)) __attribute__((const)) extract_pixel(image2d_t image, int2 coord)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    uint4 offset = *(uint4*)(pImage->offset);
    // Use uint for poitner computations to avoid type overrun
    __private void* pixel = (__private void*)pImage->pData+(uint)coord.x * offset.x + (uint)coord.y * offset.y;
    return pixel;
}

__private void* __attribute__((overloadable)) __attribute__((const)) extract_pixel(image2d_depth_t image, int2 coord)
{
    image2d_t proxy = __builtin_astype(image, image2d_t);
    return extract_pixel(proxy, coord);
}

void __attribute__((overloadable)) soa4_extract_pixel(image2d_t image, int4 coord_x, int4 coord_y, __private void** p1, __private void** p2, __private void** p3, __private void** p4)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    uint4 offset_x = (uint4)(pImage->offset[0]);
    uint4 offset_y = (uint4)(pImage->offset[1]);

    // Use uint for poitner computations to avoid type overrun
    uint4 ocoord_x = (as_uint4(coord_x)) * offset_x;
    uint4 ocoord_y = (as_uint4(coord_y)) * offset_y;

    __private char* pData = (__private void*)pImage->pData;

    uint4 ocoord = ocoord_x + ocoord_y;
    *p1 = pData + ocoord.s0;
    *p2 = pData + ocoord.s1;
    *p3 = pData + ocoord.s2;
    *p4 = pData + ocoord.s3;
}

void __attribute__((overloadable)) soa8_extract_pixel(image2d_t image, int8 coord_x, int8 coord_y, __private void** p0, __private void** p1, __private void** p2, __private void** p3, __private void** p4, __private void** p5, __private void** p6, __private void** p7)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    uint8 offset_x = (uint8)(pImage->offset[0]);
    uint8 offset_y = (uint8)(pImage->offset[1]);
    
    uint8 ocoord_x = (as_uint8(coord_x)) * offset_x;
    uint8 ocoord_y = (as_uint8(coord_y)) * offset_y;

    __private char* pData = pImage->pData;

    uint8 ocoord = ocoord_x + ocoord_y;
    *p0 = pData + ocoord.s0;
    *p1 = pData + ocoord.s1;
    *p2 = pData + ocoord.s2;
    *p3 = pData + ocoord.s3;
    *p4 = pData + ocoord.s4;
    *p5 = pData + ocoord.s5;
    *p6 = pData + ocoord.s6;
    *p7 = pData + ocoord.s7;
}

__private void* __attribute__((overloadable)) __attribute__((const)) extract_pixel(image3d_t image, int4 coord)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    uint4 offset = *(uint4*)(pImage->offset);
    __private void* pixel = (__private void*)pImage->pData+(uint)coord.x * offset.x + (uint)coord.y * offset.y 
               + (uint)coord.z * offset.z;
    return pixel;
}

__private void* __attribute__((overloadable)) __attribute__((const)) extract_pixel(image2d_array_t image, int4 coord)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    uint4 offset = *(uint4*)(pImage->offset);
    __private void* pixel = (__private void*)pImage->pData+(uint)coord.x * offset.x + (uint)coord.y * offset.y 
               + (uint)coord.z*pImage->pitch[1];
    return pixel;
}


__private void* __attribute__((overloadable)) __attribute__((const)) extract_pixel(image2d_array_depth_t image, int4 coord)
{
    image2d_array_t proxy = __builtin_astype(image, image2d_array_t);
    return extract_pixel(proxy, coord);
}

__private void* __attribute__((overloadable)) __attribute__((const)) extract_pixel(image1d_t image, int coord)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    uint4 offset = *(uint4*)(pImage->offset);
    __private void* pixel = (__private void*)pImage->pData+(uint)coord * offset.x;
    return pixel;
}

__private void* __attribute__((overloadable)) __attribute__((const)) extract_pixel(image1d_array_t image, int2 coord)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    uint4 offset = *(uint4*)(pImage->offset);
    // Offset for image array correspongs to offset inside image
    // that's why  to compute pixel pointer dimension is used here
    __private void* pixel = (__private void*)pImage->pData + coord.x * offset.x + coord.y * (uint)pImage->pitch[0];
    return pixel;
}

__private void* __attribute__((overloadable)) __attribute__((const))  GetImagePtr(image2d_array_t image, float idxFloat)
{
    // First convert integer image index
    int idx = rint(idxFloat);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    // clamp idx to edge
    if(idx < 0)
    {
        idx = 0;
    } else if(idx >= pImage->array_size)
    {
        idx = pImage->array_size - 1;
    }
    __private void* ptr = (__private void*)pImage->pData + pImage->pitch[1]*idx;
    return ptr;
}

__private void* __attribute__((overloadable)) __attribute__((const))  GetImagePtr(image2d_array_depth_t image, float idxFloat)
{
    image2d_array_t proxy = __builtin_astype(image, image2d_array_t);
    return GetImagePtr(proxy, idxFloat);
}

__private void* __attribute__((overloadable)) __attribute__((const))  GetImagePtr(image1d_array_t image, float idxFloat)
{
    // First convert integer image index
    int idx = rint(idxFloat);
    // clamp idx to edge
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    if(idx < 0)
    {
        idx = 0;
    } else if(idx >= pImage->array_size)
    {
        idx = pImage->array_size - 1;
    }
    __private void* ptr = (__private void*)pImage->pData + pImage->pitch[0]*idx;
    return ptr;
}

__private void* __attribute__((overloadable)) __attribute__((const)) GetImagePtr(image2d_array_t image, int idx)
{
    // clamp idx to edge
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    if(idx < 0)
    {
        idx = 0;
    } else if(idx >= pImage->array_size)
    {
        idx = pImage->array_size - 1;
    }
    __private void* ptr = (__private void*)pImage->pData + pImage->pitch[1]*idx;
    return ptr;
}

__private void* __attribute__((overloadable)) __attribute__((const)) GetImagePtr(image2d_array_depth_t image, int idx)
{
    image2d_array_t proxy = __builtin_astype(image, image2d_array_t);
    return GetImagePtr(proxy, idx);
}

__private void* __attribute__((overloadable)) __attribute__((const)) GetImagePtr(image1d_array_t image, int idx)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    if(idx < 0)
    {
        idx = 0;
    } else if(idx >= pImage->array_size)
    {
        idx = pImage->array_size - 1;
    }
    __private void* ptr = (__private void*)pImage->pData + pImage->pitch[0]*idx;
    return ptr;
}

/// Integer coordinate translation callbacks


int4 __attribute__((overloadable)) trans_coord_int_NONE_FALSE_NEAREST(__private void* image, int4 coord)
{
    //not testing if coords are OOB - this mode doesn't guarantee safeness!
    return coord;
}

void __attribute__((overloadable)) soa4_trans_coord_int_NONE_FALSE_NEAREST(__private void* image, int4 coord_x, int4 coord_y, __private int4* res_coord_x, __private int4* res_coord_y)
{
    //not testing if coords are OOB - this mode doesn't guarantee safeness!
    *res_coord_x = coord_x;
    *res_coord_y = coord_y;
}

void __attribute__((overloadable)) soa8_trans_coord_int_NONE_FALSE_NEAREST(__private void* image, int8 coord_x, int8 coord_y, __private int8* res_coord_x, __private int8* res_coord_y)
{
    //not testing if coords are OOB - this mode doesn't guarantee safeness!
    *res_coord_x = coord_x;
    *res_coord_y = coord_y;
}

int4 __attribute__((overloadable)) trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST(__private void* image, int4 coord)
{
    image2d_t image2d = __builtin_astype(image, image2d_t);
    return ProjectToEdgeInt(image2d, coord);
}

void __attribute__((overloadable)) soa4_trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST(__private void* image, int4 coord_x, int4 coord_y, __private int4* res_coord_x, __private int4* res_coord_y)
{
    image2d_t image2d = __builtin_astype(image, image2d_t);
    return SOA4_ProjectToEdgeInt(image2d, coord_x, coord_y, res_coord_x, res_coord_y);
}

void __attribute__((overloadable)) soa8_trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST(__private void* image, int8 coord_x, int8 coord_y, __private int8* res_coord_x, __private int8* res_coord_y)
{
    image2d_t image2d = __builtin_astype(image, image2d_t);
    return SOA8_ProjectToEdgeInt(image2d, coord_x, coord_y, res_coord_x, res_coord_y);
}

int4 __attribute__((overloadable)) trans_coord_int_UNDEFINED(__private void* image, int4 coord)
{
    return UndefCoordInt;
}

void __attribute__((overloadable)) soa4_trans_coord_int_UNDEFINED(__private void* image, int4 coord_x, int4 coord_y, __private int4* res_coord_x, __private int4* res_coord_y)
{
    *res_coord_x = SOA4_UndefCoordIntX;
    *res_coord_y = SOA4_UndefCoordIntY;
}

void __attribute__((overloadable)) soa8_trans_coord_int_UNDEFINED(__private void* image, int8 coord_x, int8 coord_y, __private int8* res_coord_x, __private int8* res_coord_y)
{
    *res_coord_x = SOA8_UndefCoordIntX;
    *res_coord_y = SOA8_UndefCoordIntY;
}

/// Image reading callbacks

/// read_imageui implementation

uint4  __attribute__((overloadable)) read_imageui(image2d_t image, sampler_t sampler, int2 coord)
{
    int4 coord4 = (int4)(0, 0, 0, 0);
    coord4.lo = coord;
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_I_COORD_CBK coord_cbk = call_coord_translate_i_callback(samplerIndex);
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position=call_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, coord4);
    return call_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
}

void __attribute__((overloadable)) soa4_read_imageui(image2d_t image, sampler_t sampler, int4 coord_x, int4 coord_y,
                                                    __private uint4* res_x, __private uint4* res_y, __private uint4* res_z, __private uint4* res_w)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    SOA4_Image_I_COORD_CBK coord_cbk = call_soa4_coord_translate_i_callback(samplerIndex);
    SOA4_Image_UI_READ_CBK read_cbk = (SOA4_Image_UI_READ_CBK)pImage->soa4_read_img_callback_int[samplerIndex];
    int4 translated_coord_x;
    int4 translated_coord_y;
    call_SOA4_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, coord_x, coord_y, &translated_coord_x, &translated_coord_y );
    call_SOA4_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, translated_coord_x, translated_coord_y, pData, res_x, res_y, res_z, res_w);
}

void __attribute__((overloadable)) soa8_read_imageui(image2d_t image, sampler_t sampler, int8 coord_x, int8 coord_y,
                                                    __private uint8* res_x, __private uint8* res_y, __private uint8* res_z, __private uint8* res_w)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    SOA8_Image_I_COORD_CBK coord_cbk = call_soa8_coord_translate_i_callback(samplerIndex);
    SOA8_Image_UI_READ_CBK read_cbk = (SOA8_Image_UI_READ_CBK)pImage->soa8_read_img_callback_int[samplerIndex];
    int8 translated_coord_x;
    int8 translated_coord_y;
    call_SOA8_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, coord_x, coord_y, &translated_coord_x, &translated_coord_y );
    call_SOA8_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, translated_coord_x, translated_coord_y, pData, res_x, res_y, res_z, res_w);
}

uint4  __attribute__((overloadable)) mask_read_imageui(int mask, image2d_t image, sampler_t sampler, int2 coord)
{
  if (mask) return read_imageui(image, sampler, coord);
  return (uint4)(0, 0, 0, 0);
}

uint4  __attribute__((overloadable)) read_imageui(image3d_t image, sampler_t sampler, int4 coord)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_I_COORD_CBK coord_cbk = call_coord_translate_i_callback(samplerIndex);
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position=call_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, coord);
    return call_Image_UI_READ_CBK(read_cbk,(__private void*)pImage, trans_position, pData);
}

uint4  __attribute__((overloadable)) read_imageui(image2d_t image, sampler_t sampler, float2 coord)
{
    float4 coord4 = (float4)(0.f, 0.f ,0.f ,0.f);
    coord4.lo = coord;
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)pImage->coord_translate_f_callback[samplerIndex];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position=call_Image_F_COORD_CBK(coord_cbk, (__private void*)pImage, coord4);
    return call_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
}

uint4  __attribute__((overloadable)) mask_read_imageui(int mask, image2d_t image, sampler_t sampler, float2 coord)
{
  if (mask) return read_imageui(image, sampler, coord);
  return (uint4)(0,0,0,0);
}

uint4  __attribute__((overloadable)) read_imageui(image3d_t image, sampler_t sampler, float4 coord)
{
    coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData = pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)pImage->coord_translate_f_callback[samplerIndex];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position=call_Image_F_COORD_CBK(coord_cbk, (__private void*)pImage, coord);
    return call_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
}

void  __attribute__((overloadable)) write_imageui(image2d_t image, int2 coord, uint4 color)
{
    __private void* pixel = extract_pixel(image, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_UI_WRITE_CBK cbk = (Image_UI_WRITE_CBK)pImage->write_img_callback;
    call_Image_UI_WRITE_CBK(cbk, pixel, color);
}

void __attribute__((overloadable)) soa4_write_imageui(image2d_t image, int4 coord_x, int4 coord_y, uint4 val_x, uint4 val_y, uint4 val_z, uint4 val_w)
{
    __private uchar4* p1;
    __private uchar4* p2;
    __private uchar4* p3;
    __private uchar4* p4;
    soa4_extract_pixel(image, coord_x, coord_y, (__private void**)&p1, (__private void**)&p2, (__private void**)&p3, (__private void**)&p4);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    SOA4_Image_UI_WRITE_CBK cbk = (SOA4_Image_UI_WRITE_CBK)pImage->soa4_write_img_callback;
    call_SOA4_Image_UI_WRITE_CBK(cbk, p1, p2, p3, p4, val_x, val_y, val_z, val_w);
}

void __attribute__((overloadable)) soa8_write_imageui(image2d_t image, int8 coord_x, int8 coord_y, uint8 val_x, uint8 val_y, uint8 val_z, uint8 val_w)
{
    __private uchar4* p0;
    __private uchar4* p1;
    __private uchar4* p2;
    __private uchar4* p3;
    __private uchar4* p4;
    __private uchar4* p5;
    __private uchar4* p6;
    __private uchar4* p7;
    soa8_extract_pixel(image, coord_x, coord_y, (__private void**)&p0, (__private void**)&p1, (__private void**)&p2, (__private void**)&p3, (__private void**)&p4, (__private void**)&p5, (__private void**)&p6, (__private void**)&p7);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    SOA8_Image_UI_WRITE_CBK cbk = (SOA8_Image_UI_WRITE_CBK)pImage->soa8_write_img_callback;
    call_SOA8_Image_UI_WRITE_CBK(cbk, p0, p1, p2, p3, p4, p5, p6, p7, val_x, val_y, val_z, val_w);
}

void  __attribute__((overloadable)) mask_write_imageui(int mask, image2d_t image, int2 coord, uint4 color)
{
  if (mask) write_imageui(image, coord, color);
}

/******************************************SIGNED INT I/O FUNCTIONS (read_imagei)*************************************************************/

int4  __attribute__((overloadable)) read_imagei(image2d_t image, sampler_t sampler, int2 coord)
{
    int4 coord4 = (int4)(0, 0, 0, 0);
    coord4.lo = coord;
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_I_COORD_CBK coord_cbk = call_coord_translate_i_callback(samplerIndex);
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position=call_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, coord4);
    return call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
}

int4  __attribute__((overloadable)) mask_read_imagei(int mask, image2d_t image, sampler_t sampler, int2 coord)
{
  if (mask) return read_imagei(image, sampler, coord);
  return (int4)(0, 0, 0, 0);
}

int4  __attribute__((overloadable)) read_imagei(image3d_t image, sampler_t sampler, int4 coord)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_I_COORD_CBK coord_cbk = call_coord_translate_i_callback(samplerIndex);
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position=call_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, coord);
    return call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
}

int4  __attribute__((overloadable)) read_imagei(image2d_t image, sampler_t sampler, float2 coord)
{
    float4 coord4 = (float4)(0.f, 0.f, 0.f, 0.f);
    coord4.lo = coord;
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)pImage->coord_translate_f_callback[samplerIndex];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position=call_Image_F_COORD_CBK(coord_cbk, (__private void*)pImage, coord4);
    return call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
}

int4  __attribute__((overloadable)) mask_read_imagei(int mask, image2d_t image, sampler_t sampler, float2 coord)
{
  if (mask) return read_imagei(image, sampler, coord);
  return (int4)(0, 0, 0, 0);
}

int4  __attribute__((overloadable)) read_imagei(image3d_t image, sampler_t sampler, float4 coord)
{
    coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    int samplerIndex = __builtin_astype(sampler, int);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)pImage->coord_translate_f_callback[samplerIndex];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    __private void* pData =pImage->pData;
    int4 trans_position=call_Image_F_COORD_CBK(coord_cbk, (__private void*)pImage, coord);
    return call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
}

void  __attribute__((overloadable)) write_imagei(image2d_t image, int2 coord, int4 color)
{
    __private void* pixel = extract_pixel(image, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_I_WRITE_CBK cbk = (Image_I_WRITE_CBK)pImage->write_img_callback;
    call_Image_I_WRITE_CBK(cbk, pixel, color);
}

void  __attribute__((overloadable)) mask_write_imagei(int mask, image2d_t image, int2 coord, int4 color){
  if (mask) write_imagei(image, coord, color);
}

/***********************************FLOAT IMAGE I/O FUNCTIONS (read_imagef)********************************************************/


float4  __attribute__((overloadable)) read_imagef(image2d_t image, sampler_t sampler, int2 coord)
{
    int4 coord4 = (int4)(0.f, 0.f, 0.f, 0.f);
    coord4.lo = coord;
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_I_COORD_CBK coord_cbk = call_coord_translate_i_callback(samplerIndex);
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)pImage->read_img_callback_float[samplerIndex];
    int4 dummy0;
    float4 dummy1;
    int4 trans_position=call_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, coord4);
    return call_Image_FI_READ_CBK(read_cbk, (__private void*)pImage, trans_position, dummy0, dummy1, pData);
}

float  __attribute__((overloadable)) read_imagef(image2d_depth_t image, sampler_t sampler, int2 coord)
{
    image2d_t proxy = __builtin_astype(image, image2d_t);
    return read_imagef(proxy, sampler, coord).x;
}

float4  __attribute__((overloadable)) mask_read_imagef(int mask, image2d_t image, sampler_t sampler, int2 coord)
{
  if (mask) return read_imagef(image, sampler, coord);
  return (float4)(0.0f, 0.0f, 0.0f, 0.0f);
}

float4  __attribute__((overloadable)) read_imagef(image3d_t image, sampler_t sampler, int4 coord)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_I_COORD_CBK coord_cbk = call_coord_translate_i_callback(samplerIndex);
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)pImage->read_img_callback_float[samplerIndex];
    int4 dummy0;
    float4 dummy1;
    int4 trans_position=call_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, coord);
    return call_Image_FI_READ_CBK(read_cbk, (__private void*)pImage, trans_position, dummy0, dummy1, pData);
}

float4  __attribute__((overloadable)) read_imagef(image2d_t image, sampler_t sampler, float2 coord)
{
    float4 coord4 = (float4)(0.f, 0.f, 0.f, 0.f);
    coord4.lo = coord;
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_FF_COORD_CBK coord_cbk = (Image_FF_COORD_CBK)pImage->coord_translate_f_callback[samplerIndex];
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)pImage->read_img_callback_float[samplerIndex];
    ///// components for interpolation
    int4 square0, square1;
    float4 fraction= call_Image_FF_COORD_CBK(coord_cbk, (__private void*)pImage, coord4, &square0, &square1);
    return call_Image_F_READ_CBK(read_cbk, (__private void*)pImage, square0, square1, fraction, pData);
}

float  __attribute__((overloadable)) read_imagef(image2d_depth_t image, sampler_t sampler, float2 coord)
{
    image2d_t proxy = __builtin_astype(image, image2d_t);
    return read_imagef(proxy, sampler, coord).x;
}

float4  __attribute__((overloadable)) mask_read_imagef(int mask, image2d_t image, sampler_t sampler, float2 coord)
{
    if (mask) return read_imagef(image, sampler, coord);
    return (float4)(0.f, 0.f, 0.f, 0.f);
}

float4  __attribute__((overloadable)) read_imagef(image3d_t image, sampler_t sampler, float4 coord)
{
    coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_FF_COORD_CBK coord_cbk=(Image_FF_COORD_CBK)pImage->coord_translate_f_callback[samplerIndex];
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)pImage->read_img_callback_float[samplerIndex];
    int4 square0, square1;
    float4 fraction=call_Image_FF_COORD_CBK(coord_cbk, (__private void*)pImage, coord, &square0, &square1);
    return call_Image_F_READ_CBK(read_cbk, (__private void*)pImage, square0, square1, fraction, pData);
}

void  __attribute__((overloadable)) write_imagef(image2d_t image, int2 coord, float4 color)
{
    __private void* pixel = extract_pixel(image, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_F_WRITE_CBK cbk = (Image_F_WRITE_CBK)pImage->write_img_callback;
    call_Image_F_WRITE_CBK(cbk, pixel, color);
}

void  __attribute__((overloadable)) write_imagef(image2d_depth_t image, int2 coord, float depth)
{
    image2d_t proxy = __builtin_astype(image, image2d_t);
    write_imagef(proxy, coord, (float4)(depth));
}

void  __attribute__((overloadable)) mask_write_imagef(int mask, image2d_t image, int2 coord, float4 color)
{
  if (mask) write_imagef(image, coord, color);
}

float4  __attribute__((overloadable)) read_imagef(image2d_array_t image, sampler_t sampler, int4 coord)
{
    int4 internal_coord = coord;
    internal_coord.z = 0;
    int arrayIndex = coord.z;
    __private void* pData = GetImagePtr(image, arrayIndex);
    int samplerIndex = __builtin_astype(sampler, int);
    Image_I_COORD_CBK coord_cbk = call_coord_translate_i_callback(samplerIndex);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)pImage->read_img_callback_float[samplerIndex];
    int4 dummy0;
    float4 dummy1;
    int4 trans_position=call_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, internal_coord);
    float4 val = call_Image_FI_READ_CBK(read_cbk, (__private void*)pImage, trans_position, dummy0, dummy1, pData);
    return val;
}

float4  __attribute__((overloadable)) read_imagef(image2d_array_t image, sampler_t sampler, float4 coord)
{
    float4 internal_coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    internal_coord.z = 0;
    __private void* pData = GetImagePtr(image, coord.z);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    int samplerIndex = __builtin_astype(sampler, int);
    Image_FF_COORD_CBK coord_cbk=(Image_FF_COORD_CBK)pImage->coord_translate_f_callback[samplerIndex];
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)pImage->read_img_callback_float[samplerIndex];
    int4 square0, square1;
    float4 fraction=call_Image_FF_COORD_CBK(coord_cbk, (__private void*)pImage, internal_coord, &square0, &square1);
    float4 val = call_Image_F_READ_CBK(read_cbk, (__private void*)pImage, square0, square1, fraction, pData);
    return val;
}

float  __attribute__((overloadable)) read_imagef(image2d_array_depth_t image, sampler_t sampler, int4 coord)
{
    image2d_array_t proxy = __builtin_astype(image, image2d_array_t);
    return read_imagef(proxy, sampler, coord).x;
}

float  __attribute__((overloadable)) read_imagef(image2d_array_depth_t image, sampler_t sampler, float4 coord)
{
    image2d_array_t proxy = __builtin_astype(image, image2d_array_t);
    return read_imagef(proxy, sampler, coord).x;
}

int4  __attribute__((overloadable)) read_imagei(image2d_array_t image, sampler_t sampler, int4 coord)
{
    int4 internal_coord = coord;
    internal_coord.z = 0;
    __private void* pData = GetImagePtr(image, coord.z);
    int samplerIndex = __builtin_astype(sampler, int);
    Image_I_COORD_CBK coord_cbk = call_coord_translate_i_callback(samplerIndex);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position=call_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, internal_coord);
    int4 val = call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
    return val;
}

int4  __attribute__((overloadable)) read_imagei(image2d_array_t image, sampler_t sampler, float4 coord)
{
    float4 internal_coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    internal_coord.z = 0.f;
    __private void* pData = GetImagePtr(image, coord.z);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    int samplerIndex = __builtin_astype(sampler, int);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)pImage->coord_translate_f_callback[samplerIndex];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position = call_Image_F_COORD_CBK(coord_cbk, (__private void*)pImage, internal_coord);
    int4 val = call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
    return val;
}

uint4  __attribute__((overloadable)) read_imageui(image2d_array_t image, sampler_t sampler, int4 coord)
{
    int4 internal_coord = coord;
    internal_coord.z = 0;
    __private void* pData = GetImagePtr(image, coord.z);
    int samplerIndex = __builtin_astype(sampler, int);
    Image_I_COORD_CBK coord_cbk = call_coord_translate_i_callback(samplerIndex);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position=call_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, internal_coord);
    uint4 val = call_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
    return val;
}

uint4  __attribute__((overloadable)) read_imageui(image2d_array_t image, sampler_t sampler, float4 coord)
{
    float4 internal_coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    internal_coord.z = 0.f;
    __private void* pData = GetImagePtr(image, coord.z);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    int samplerIndex = __builtin_astype(sampler, int);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)pImage->coord_translate_f_callback[samplerIndex];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position = call_Image_F_COORD_CBK(coord_cbk, (__private void*)pImage, internal_coord);
    uint4 val = call_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
    return val;
}

float4 __attribute__((overloadable)) read_imagef(image1d_t image, sampler_t sampler, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_I_COORD_CBK coord_cbk = call_coord_translate_i_callback(samplerIndex);
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)pImage->read_img_callback_float[samplerIndex];
    int4 trans_position=call_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, coord4);
    int4 dummy0;
    float4 dummy1;
    return call_Image_FI_READ_CBK(read_cbk, (__private void*)pImage, trans_position, dummy0, dummy1, pData);
}

float4 __attribute__((overloadable)) read_imagef(image1d_t image, sampler_t sampler, float coord)
{
    float4 coord4=(float4)(coord, 0.0,0.0,0.0);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_FF_COORD_CBK coord_cbk=(Image_FF_COORD_CBK)pImage->coord_translate_f_callback[samplerIndex];
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)pImage->read_img_callback_float[samplerIndex];
    int4 square0, square1;
    float4 fraction= call_Image_FF_COORD_CBK(coord_cbk, (__private void*)pImage, coord4, &square0, &square1);
    return call_Image_F_READ_CBK(read_cbk, (__private void*)pImage, square0, square1, fraction, pData);
}

int4 __attribute__((overloadable)) read_imagei(image1d_t image, sampler_t sampler, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_I_COORD_CBK coord_cbk = call_coord_translate_i_callback(samplerIndex);
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position=call_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, coord4);
    return call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
}

int4 __attribute__((overloadable)) read_imagei(image1d_t image, sampler_t sampler, float coord)
{
    float4 coord4=(float4)(coord, 0.0,0.0,0.0);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)pImage->coord_translate_f_callback[samplerIndex];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position=call_Image_F_COORD_CBK(coord_cbk, (__private void*)pImage, coord4);
    return call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
}

uint4 __attribute__((overloadable)) read_imageui(image1d_t image, sampler_t sampler, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_I_COORD_CBK coord_cbk= call_coord_translate_i_callback(samplerIndex);
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position=call_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, coord4);
    return call_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
}

uint4 __attribute__((overloadable)) read_imageui(image1d_t image, sampler_t sampler, float coord)
{
    float4 coord4=(float4)(coord, 0.0,0.0,0.0);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    int samplerIndex = __builtin_astype(sampler, int);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)pImage->coord_translate_f_callback[samplerIndex];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position=call_Image_F_COORD_CBK(coord_cbk, (__private void*)pImage, coord4);
    return call_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
}

float4 __attribute__((overloadable)) read_imagef(image1d_array_t image, sampler_t sampler, int2 coord)
{
    int4 internal_coord=(int4)(coord.x, 0,0,0);
    int arrayIndex = coord.y;
    __private void* pData = GetImagePtr(image, arrayIndex);
    int samplerIndex = __builtin_astype(sampler, int);
    Image_I_COORD_CBK coord_cbk = call_coord_translate_i_callback(samplerIndex);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)pImage->read_img_callback_float[samplerIndex];
    int4 trans_position=call_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, internal_coord);
    int4 dummy0;
    float4 dummy1;
    float4 val = call_Image_FI_READ_CBK(read_cbk, (__private void*)pImage, trans_position, dummy0, dummy1, pData);
    return val;

}

float4 __attribute__((overloadable)) read_imagef(image1d_array_t image, sampler_t sampler, float2 coord)
{
    float4 internal_coord = (float4)(coord.x, 0.0,0.0,0.0);
    __private void* pData = GetImagePtr(image, coord.y);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    int samplerIndex = __builtin_astype(sampler, int);
    Image_FF_COORD_CBK coord_cbk=(Image_FF_COORD_CBK)pImage->coord_translate_f_callback[samplerIndex];
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)pImage->read_img_callback_float[samplerIndex];
    int4 square0, square1;
    float4 fraction=call_Image_FF_COORD_CBK(coord_cbk, (__private void*)pImage, internal_coord, &square0, &square1);
    float4 val = call_Image_F_READ_CBK(read_cbk, (__private void*)pImage, square0, square1, fraction, pData);
    return val;
}

int4 __attribute__((overloadable)) read_imagei(image1d_array_t image, sampler_t sampler, int2 coord)
{
    int4 internal_coord = (int4)(coord.x, 0,0,0);
    __private void* pData = GetImagePtr(image, coord.y);
    int samplerIndex = __builtin_astype(sampler, int);
    Image_I_COORD_CBK coord_cbk = call_coord_translate_i_callback(samplerIndex);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position=call_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, internal_coord);
    int4 val = call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
    return val;
}

int4 __attribute__((overloadable)) read_imagei(image1d_array_t image, sampler_t sampler, float2 coord)
{
    float4 internal_coord = (float4)(coord.x, 0.0,0.0,0.0);
    __private void* pData = GetImagePtr(image, coord.y);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    int samplerIndex = __builtin_astype(sampler, int);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)pImage->coord_translate_f_callback[samplerIndex];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position = call_Image_F_COORD_CBK(coord_cbk, (__private void*)pImage, internal_coord);
    int4 val = call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
    return val;
}

uint4 __attribute__((overloadable)) read_imageui(image1d_array_t image, sampler_t sampler, int2 coord)
{
    int4 internal_coord = (int4)(coord.x, 0,0,0);
    __private void* pData = GetImagePtr(image, coord.y);
    int samplerIndex = __builtin_astype(sampler, int);
    Image_I_COORD_CBK coord_cbk = call_coord_translate_i_callback(samplerIndex);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position=call_Image_I_COORD_CBK(coord_cbk, (__private void*)pImage, internal_coord);
    uint4 val = call_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
    return val;
}

uint4 __attribute__((overloadable)) read_imageui(image1d_array_t image, sampler_t sampler, float2 coord)
{
    float4 internal_coord = (float4)(coord.x, 0.0,0.0,0.0);
    __private void* pData = GetImagePtr(image, coord.y);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    int samplerIndex = __builtin_astype(sampler, int);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)pImage->coord_translate_f_callback[samplerIndex];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[samplerIndex];
    int4 trans_position = call_Image_F_COORD_CBK(coord_cbk, (__private void*)pImage, internal_coord);
    uint4 val = call_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, trans_position, pData);
    return val;
}

// sampler-less calls
float4 __attribute__((overloadable)) read_imagef (image2d_t image, int2 coord)
{
    int4 coord4 = (int4)(0, 0, 0, 0);
    coord4.lo = coord;
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)pImage->read_img_callback_float[SIMPLE_SAMPLER];
    int4 dummy0;
    float dummy1;
    return call_Image_FI_READ_CBK(read_cbk, (__private void*)pImage, coord4, dummy0, dummy1, pData);
}

float __attribute__((overloadable)) read_imagef (image2d_depth_t image, int2 coord)
{
    image2d_t proxy = __builtin_astype(image, image2d_t);
    return read_imagef(proxy, coord).x;
}

float4 __attribute__((overloadable)) mask_read_imagef (int mask, image2d_t image, int2 coord)
{
  if (mask) return read_imagef(image, coord);
  return (float4)(0.f, 0.f, 0.f, 0.f);
}

int4 __attribute__((overloadable)) read_imagei (image2d_t image, int2 coord)
{
    int4 coord4 = (int4)(0, 0, 0, 0);
    coord4.lo = coord;
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[SIMPLE_SAMPLER];
    return call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, coord4, pData);
}

int4 __attribute__((overloadable)) mask_read_imagei (int mask, image2d_t image, int2 coord)
{
  if (mask) return read_imagei(image, coord);
  return (int4)(0, 0, 0, 0);
}

uint4 __attribute__((overloadable)) read_imageui (image2d_t image, int2 coord)
{
    int4 coord4 = (int4)(0, 0, 0, 0);
    coord4.lo = coord;
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[SIMPLE_SAMPLER];
    return call_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, coord4, pData);
}

uint4 __attribute__((overloadable)) mask_read_imageui (int mask, image2d_t image, int2 coord)
{
  if (mask) return read_imageui(image, coord);
  return (uint4)(0, 0, 0, 0);
}

float4 __attribute__((overloadable)) read_imagef (image3d_t image, int4 coord)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)pImage->read_img_callback_float[SIMPLE_SAMPLER];
    int4 dummy0;
    float4 dummy1;
    return call_Image_FI_READ_CBK(read_cbk, (__private void*)pImage, coord, dummy0, dummy1, pData);
}

int4 __attribute__((overloadable)) read_imagei (image3d_t image, int4 coord)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[SIMPLE_SAMPLER];
    return call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, coord, pData);
}

uint4 __attribute__((overloadable)) read_imageui (image3d_t image, int4 coord)
{
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[SIMPLE_SAMPLER];
    return call_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, coord, pData);
}

float4 __attribute__((overloadable)) read_imagef (image2d_array_t image, int4 coord)
{
    // naive read_imagef implementation
    int4 internal_coord = coord;
    internal_coord.z = 0;
    __private void* pData = GetImagePtr(image, coord.z);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)pImage->read_img_callback_float[SIMPLE_SAMPLER];
    int4 dummy0;
    float dummy1;
    return call_Image_FI_READ_CBK(read_cbk, (__private void*)pImage, internal_coord, dummy0, dummy1, pData);
}

float __attribute__((overloadable)) read_imagef (image2d_array_depth_t image, int4 coord)
{
    image2d_array_t proxy = __builtin_astype(image, image2d_array_t);
    return read_imagef(proxy, coord).x;
}

int4 __attribute__((overloadable)) read_imagei (image2d_array_t image, int4 coord)
{
    int4 internal_coord = coord;
    internal_coord.z = 0;
    __private void* pData = GetImagePtr(image, coord.z);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[SIMPLE_SAMPLER];
    return call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, internal_coord, pData);
}

uint4 __attribute__((overloadable)) read_imageui (image2d_array_t image, int4 coord)
{
    int4 internal_coord = coord;
    internal_coord.z = 0;
    __private void* pData = GetImagePtr(image, coord.z);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[SIMPLE_SAMPLER];
    return call_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, internal_coord, pData);
}

float4 __attribute__((overloadable)) read_imagef (image1d_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)pImage->read_img_callback_float[SIMPLE_SAMPLER];
    int4 dummy0;
    float4 dummy1;
    return call_Image_FI_READ_CBK(read_cbk, (__private void*)pImage, coord4, dummy0, dummy1, pData);
}

float4 __attribute__((overloadable)) read_imagef (image1d_buffer_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)pImage->read_img_callback_float[SIMPLE_SAMPLER];
    int4 dummy0;
    float dummy1;
    return call_Image_FI_READ_CBK(read_cbk, (__private void*)pImage, coord4, dummy0, dummy1, pData);
}

int4 __attribute__((overloadable)) read_imagei(image1d_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[SIMPLE_SAMPLER];
    return call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, coord4, pData);
}

uint4 __attribute__((overloadable)) read_imageui(image1d_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[SIMPLE_SAMPLER];
    return call_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, coord4, pData);
}

int4 __attribute__((overloadable)) read_imagei(image1d_buffer_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[SIMPLE_SAMPLER];
    return call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, coord4, pData);
}

uint4 __attribute__((overloadable)) read_imageui(image1d_buffer_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    __private void* pData =pImage->pData;
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[SIMPLE_SAMPLER];
    return call_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, coord4, pData);
}

float4 __attribute__((overloadable)) read_imagef(image1d_array_t image, int2 coord)
{
    int4 internal_coord=(int4)(coord.x, 0,0,0);
    __private void* pData = GetImagePtr(image, coord.y);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)pImage->read_img_callback_float[SIMPLE_SAMPLER];
    int4 dummy0;
    float dummy1;
    return call_Image_FI_READ_CBK(read_cbk, (__private void*)pImage, internal_coord, dummy0, dummy1, pData);
}

int4 __attribute__((overloadable)) read_imagei(image1d_array_t image, int2 coord)
{
    int4 internal_coord=(int4)(coord.x, 0,0,0);
    __private void* pData = GetImagePtr(image, coord.y);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)pImage->read_img_callback_int[SIMPLE_SAMPLER];
    return call_Image_I_READ_CBK(read_cbk, (__private void*)pImage, internal_coord, pData);
}

uint4 __attribute__((overloadable)) read_imageui(image1d_array_t image, int2 coord)
{
    int4 internal_coord=(int4)(coord.x, 0,0,0);
    __private void* pData = GetImagePtr(image, coord.y);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)pImage->read_img_callback_int[SIMPLE_SAMPLER];
    return call_Image_UI_READ_CBK(read_cbk, (__private void*)pImage, internal_coord, pData);
}

// write_image calls
void __attribute__((overloadable)) write_imagef (image2d_array_t image, int4 coord, float4 color)
{
    __private void* pixel = extract_pixel(image, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_F_WRITE_CBK cbk = (Image_F_WRITE_CBK)pImage->write_img_callback;
    call_Image_F_WRITE_CBK(cbk, pixel, color);
}

void __attribute__((overloadable)) write_imagef (image2d_array_depth_t image, int4 coord, float depth)
{
    image2d_array_t proxy = __builtin_astype(image, image2d_array_t);
    write_imagef(proxy, coord, (float4)(depth));
}

void __attribute__((overloadable)) write_imagei (image2d_array_t image, int4 coord, int4 color)
{
    __private void* pixel = extract_pixel(image, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_I_WRITE_CBK cbk = (Image_I_WRITE_CBK)pImage->write_img_callback;
    call_Image_I_WRITE_CBK(cbk, pixel, color);
}

void __attribute__((overloadable)) write_imageui (image2d_array_t image, int4 coord, uint4 color)
{
    __private void* pixel = extract_pixel(image, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_UI_WRITE_CBK cbk = (Image_UI_WRITE_CBK)pImage->write_img_callback;
    call_Image_UI_WRITE_CBK(cbk, pixel, color);
}

void __attribute__((overloadable)) write_imagef (image1d_t image, int coord, float4 color)
{
    __private void* pixel = extract_pixel(image, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_F_WRITE_CBK cbk = (Image_F_WRITE_CBK)pImage->write_img_callback;
    call_Image_F_WRITE_CBK(cbk, pixel, color);
}

void __attribute__((overloadable)) write_imagei (image1d_t image, int coord, int4 color)
{
    __private void* pixel = extract_pixel(image, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_I_WRITE_CBK cbk = (Image_I_WRITE_CBK)pImage->write_img_callback;
    call_Image_I_WRITE_CBK(cbk, pixel, color);
}

void __attribute__((overloadable)) write_imageui (image1d_t image, int coord, uint4 color)
{
    __private void* pixel = extract_pixel(image, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_UI_WRITE_CBK cbk = (Image_UI_WRITE_CBK)pImage->write_img_callback;
    call_Image_UI_WRITE_CBK(cbk, pixel, color);
}

void __attribute__((overloadable)) write_imagef (image1d_buffer_t image, int coord, float4 color)
{
    image1d_t image1d = __builtin_astype(image, image1d_t);
    __private void* pixel = extract_pixel(image1d, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_F_WRITE_CBK cbk = (Image_F_WRITE_CBK)pImage->write_img_callback;
    call_Image_F_WRITE_CBK(cbk, pixel, color);
}

void __attribute__((overloadable)) write_imagei (image1d_buffer_t image, int coord, int4 color)
{
    image1d_t image1d = __builtin_astype(image, image1d_t);
    __private void* pixel = extract_pixel(image1d, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_I_WRITE_CBK cbk = (Image_I_WRITE_CBK)pImage->write_img_callback;
    call_Image_I_WRITE_CBK(cbk, pixel, color);
}

void __attribute__((overloadable)) write_imageui (image1d_buffer_t image, int coord, uint4 color)
{
    image1d_t image1d = __builtin_astype(image, image1d_t);
    __private void* pixel = extract_pixel(image1d, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_UI_WRITE_CBK cbk = (Image_UI_WRITE_CBK)pImage->write_img_callback;
    call_Image_UI_WRITE_CBK(cbk, pixel, color);
}

void __attribute__((overloadable)) write_imagef (image1d_array_t image, int2 coord, float4 color)
{
    __private void* pixel = extract_pixel(image, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_F_WRITE_CBK cbk = (Image_F_WRITE_CBK)pImage->write_img_callback;
    call_Image_F_WRITE_CBK(cbk, pixel, color);
}

void __attribute__((overloadable)) write_imagei (image1d_array_t image, int2 coord, int4 color)
{
    __private void* pixel = extract_pixel(image, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_I_WRITE_CBK cbk = (Image_I_WRITE_CBK)pImage->write_img_callback;
    call_Image_I_WRITE_CBK(cbk, pixel, color);
}

void __attribute__((overloadable)) write_imageui (image1d_array_t image, int2 coord, uint4 color)
{
    __private void* pixel = extract_pixel(image, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_UI_WRITE_CBK cbk = (Image_UI_WRITE_CBK)pImage->write_img_callback;
    call_Image_UI_WRITE_CBK(cbk, pixel, color);
}

void __attribute__((overloadable)) write_imagef(image3d_t image, int4 coord, float4 color)
{
    __private void* pixel = extract_pixel(image, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_F_WRITE_CBK cbk = (Image_F_WRITE_CBK)pImage->write_img_callback;
    call_Image_F_WRITE_CBK(cbk, pixel, color);
}

void __attribute__((overloadable)) write_imagei(image3d_t image, int4 coord, int4 color)
{
    __private void* pixel = extract_pixel(image, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_I_WRITE_CBK cbk = (Image_I_WRITE_CBK)pImage->write_img_callback;
    call_Image_I_WRITE_CBK(cbk, pixel, color);
}

void __attribute__((overloadable)) write_imageui(image3d_t image, int4 coord, uint4 color)
{
    __private void* pixel = extract_pixel(image, coord);
    __private image_aux_data *pImage = __builtin_astype(image, __private image_aux_data*);
    Image_UI_WRITE_CBK cbk = (Image_UI_WRITE_CBK)pImage->write_img_callback;
    call_Image_UI_WRITE_CBK(cbk, pixel, color);
}


// Helper functions to speed up mask analyzing
// linked from tblgen generated file
int __attribute__((const)) __attribute__((overloadable)) intel_movemask(int4);
int __attribute__((const)) __attribute__((overloadable)) intel_movemask(int8);

void __attribute__((overloadable)) mask_soa4_write_imageui(int4 mask, image2d_t image, int4 coord_x, int4 coord_y, uint4 val_x, uint4 val_y, uint4 val_z, uint4 val_w)
{
    const int rescmp = intel_movemask(mask);
    // ALL elements in mask are -1
    if(rescmp == 0xF){
        soa4_write_imageui(image, coord_x, coord_y, val_x, val_y, val_z, val_w);
    }
    // ALL elements in mask are zero
    else if(rescmp == 0){
    // do nothing
    }
    else{
        // serial version
        if (mask.s0 != 0)
            write_imageui(image, (int2)(coord_x.s0, coord_y.s0), (uint4)(val_x.s0, val_y.s0, val_z.s0, val_w.s0));

        if (mask.s1 != 0)
            write_imageui(image, (int2)(coord_x.s1, coord_y.s1), (uint4)(val_x.s1, val_y.s1, val_z.s1, val_w.s1));

        if (mask.s2 != 0)
            write_imageui(image, (int2)(coord_x.s2, coord_y.s2), (uint4)(val_x.s2, val_y.s2, val_z.s2, val_w.s2));

        if (mask.s3 != 0)
            write_imageui(image, (int2)(coord_x.s3, coord_y.s3), (uint4)(val_x.s3, val_y.s3, val_z.s3, val_w.s3));
    }
}

void __attribute__((overloadable)) mask_soa8_write_imageui(int8 mask, image2d_t image, int8 coord_x, int8 coord_y, uint8 val_x, uint8 val_y, uint8 val_z, uint8 val_w)
{
    const int rescmp = intel_movemask(mask);

    // ALL elements in mask are -1
    if(rescmp == 0xFF){
        soa8_write_imageui(image, coord_x, coord_y, val_x, val_y, val_z, val_w);
    }
    // ALL elements in mask are zero
    else if(rescmp == 0){
    // do nothing
    }
    // if low half of mask is set call SOA4
    else if(rescmp == 0xF){
        soa4_write_imageui(image, coord_x.lo, coord_y.lo, val_x.lo, val_y.lo, val_z.lo, val_w.lo);
    }
    // if upper half of mask is set call SOA4
    else if(rescmp == 0xF0){
        soa4_write_imageui(image, coord_x.hi, coord_y.hi, val_x.hi, val_y.hi, val_z.hi, val_w.hi);
    }
    // process mask
    else {
        // process scalar
        if (mask.s0 != 0)
            write_imageui(image, (int2)(coord_x.s0, coord_y.s0), (uint4)(val_x.s0, val_y.s0, val_z.s0, val_w.s0));

        if (mask.s1 != 0)
            write_imageui(image, (int2)(coord_x.s1, coord_y.s1), (uint4)(val_x.s1, val_y.s1, val_z.s1, val_w.s1));

        if (mask.s2 != 0)
            write_imageui(image, (int2)(coord_x.s2, coord_y.s2), (uint4)(val_x.s2, val_y.s2, val_z.s2, val_w.s2));

        if (mask.s3 != 0)
            write_imageui(image, (int2)(coord_x.s3, coord_y.s3), (uint4)(val_x.s3, val_y.s3, val_z.s3, val_w.s3));

        if (mask.s4 != 0)
            write_imageui(image, (int2)(coord_x.s4, coord_y.s4), (uint4)(val_x.s4, val_y.s4, val_z.s4, val_w.s4));

        if (mask.s5 != 0)
            write_imageui(image, (int2)(coord_x.s5, coord_y.s5), (uint4)(val_x.s5, val_y.s5, val_z.s5, val_w.s5));

        if (mask.s6 != 0)
            write_imageui(image, (int2)(coord_x.s6, coord_y.s6), (uint4)(val_x.s6, val_y.s6, val_z.s6, val_w.s6));

        if (mask.s7 != 0)
            write_imageui(image, (int2)(coord_x.s7, coord_y.s7), (uint4)(val_x.s7, val_y.s7, val_z.s7, val_w.s7));
    }
}


#endif // defined (__MIC__) || defined(__MIC2__)
