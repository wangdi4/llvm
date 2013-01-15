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

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#include <intrin.h>

#include "cl_image_declaration.h"
#include "ll_intrinsics.h"

#define NORMALIZED_SAMPLER 0x08

#define ALIGN16 __attribute__ ((aligned(16)))

const int fVec4FloatZeroCoordMask3D[4] ALIGN16 = {0xffffffff, 0xffffffff, 0xffffffff, 0};
ALIGN16 const int4 UndefCoordInt = {0, 0, 0, 0};
ALIGN16 const int4 SOA4_UndefCoordIntX = {0, 0, 0, 0};
ALIGN16 const int4 SOA4_UndefCoordIntY = {0, 0, 0, 0};
ALIGN16 const int8 SOA8_UndefCoordIntX = {0, 0, 0, 0, 0, 0, 0, 0};
ALIGN16 const int8 SOA8_UndefCoordIntY = {0, 0, 0, 0, 0, 0, 0, 0};
#define SIMPLE_SAMPLER NONE_FALSE_NEAREST

/// image properties functions
#define IMG_GET_PARAM(FUNC_NAME, IMG_TYPE, PARAM_TYPE, PARAM)\
PARAM_TYPE __attribute__((overloadable)) __attribute__((const)) FUNC_NAME(IMG_TYPE img)\
{\
    return ((image_aux_data*)img)->PARAM;\
}\
PARAM_TYPE##4 __attribute__((overloadable)) __attribute__((const)) soa4_##FUNC_NAME(IMG_TYPE img)\
{\
    return (PARAM_TYPE##4)(((image_aux_data*)img)->PARAM);\
}\
PARAM_TYPE##8 __attribute__((overloadable)) __attribute__((const)) soa8_##FUNC_NAME(IMG_TYPE img)\
{\
    return (PARAM_TYPE##8)(((image_aux_data*)img)->PARAM);\
}

IMG_GET_PARAM(get_image_width, image1d_t, int, dim[0])
IMG_GET_PARAM(get_image_width, image1d_array_t, int, dim[0])
IMG_GET_PARAM(get_image_width, image1d_buffer_t, int, dim[0])
IMG_GET_PARAM(get_image_width, image2d_t, int, dim[0])
IMG_GET_PARAM(get_image_width, image2d_array_t, int, dim[0])
IMG_GET_PARAM(get_image_width, image3d_t, int, dim[0])

IMG_GET_PARAM(get_image_height, image3d_t, int, dim[1])
IMG_GET_PARAM(get_image_height, image2d_t, int, dim[1])
IMG_GET_PARAM(get_image_height, image2d_array_t, int, dim[1])

IMG_GET_PARAM(get_image_depth, image3d_t, int, dim[2])

IMG_GET_PARAM(get_image_channel_data_type, image1d_t, int,        format.image_channel_data_type)
IMG_GET_PARAM(get_image_channel_data_type, image1d_array_t, int,  format.image_channel_data_type)
IMG_GET_PARAM(get_image_channel_data_type, image1d_buffer_t, int, format.image_channel_data_type)
IMG_GET_PARAM(get_image_channel_data_type, image2d_t, int,        format.image_channel_data_type)
IMG_GET_PARAM(get_image_channel_data_type, image2d_array_t, int,  format.image_channel_data_type)
IMG_GET_PARAM(get_image_channel_data_type, image3d_t, int,        format.image_channel_data_type)

IMG_GET_PARAM(get_image_channel_order, image1d_t, int,        format.image_channel_order)
IMG_GET_PARAM(get_image_channel_order, image1d_array_t, int,  format.image_channel_order)
IMG_GET_PARAM(get_image_channel_order, image1d_buffer_t, int, format.image_channel_order)
IMG_GET_PARAM(get_image_channel_order, image2d_t, int,        format.image_channel_order)
IMG_GET_PARAM(get_image_channel_order, image2d_array_t, int,  format.image_channel_order)
IMG_GET_PARAM(get_image_channel_order, image3d_t, int,        format.image_channel_order)

int2 __attribute__((overloadable)) __attribute__((const)) get_image_dim(image2d_t img)
{
    int2 res;

    res.lo = ((image_aux_data*)img)->dim[0];
    res.hi = ((image_aux_data*)img)->dim[1];
    return res;
}

int2 __attribute__((overloadable)) __attribute__((const)) get_image_dim(image2d_array_t img)
{
    int2 res;

    res.lo = ((image_aux_data*)img)->dim[0];
    res.hi = ((image_aux_data*)img)->dim[1];
    return res;
}
 int4 __attribute__((overloadable)) __attribute__((const)) get_image_dim(image3d_t img)
{
    __m128i dim = _mm_lddqu_si128((__m128i*)((image_aux_data*)img)->dim);
    // Set to 0 the highest DWORD
    dim = _mm_srli_si128(_mm_slli_si128(dim, 4),4);

    return (_4i32)dim;
}

size_t __attribute__((overloadable)) __attribute__((const)) get_image_array_size(image2d_array_t img)
{
        return ((image_aux_data*)img)->array_size;
}

size_t __attribute__((overloadable)) __attribute__((const)) get_image_array_size(image1d_array_t img)
{
        return ((image_aux_data*)img)->array_size;
}


//// Auxiliary built-in functions

int4 __attribute__((overloadable)) ProjectToEdgeInt(image2d_t image, int4 coord)
{
    int4 upper = *((int4*)(&((image_aux_data*)image)->dimSub1));
    int4 lower = (int4)(0, 0, 0, 0);

    int4 correctCoord=min(coord, upper);
    correctCoord=max(correctCoord,lower);
    return correctCoord;
}

// Clamps SOA4 coordinates to be inside image
//
// @param [in] image: the image object
// @param [in] coord_(x,y) coordinates of the pixel 
// @param [out] res_(x,y) output coordinates
void __attribute__((overloadable)) SOA4_ProjectToEdgeInt(image2d_t image, int4 coord_x, int4 coord_y, int4* res_x, int4* res_y)
{
    int4 upper_x = (int4)(((image_aux_data*)image)->dimSub1[0]);
    int4 upper_y = (int4)(((image_aux_data*)image)->dimSub1[1]);
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
void __attribute__((overloadable)) SOA8_ProjectToEdgeInt(image2d_t image, int8 coord_x, int8 coord_y, int8* res_x, int8* res_y)
{
    int8 upper_x = (int8)(((image_aux_data*)image)->dimSub1[0]);
    int8 upper_y = (int8)(((image_aux_data*)image)->dimSub1[1]);
    int8 lower = (int8)(0, 0, 0, 0, 0, 0, 0, 0);
    coord_x = clamp(coord_x, lower, upper_x);
    coord_y = clamp(coord_y, lower, upper_y);
    *res_x = coord_x;
    *res_y = coord_y;
}

void* __attribute__((overloadable)) __attribute__((const)) extract_pixel(image2d_t image, int2 coord)
{
    uint4 offset = *(uint4*)(((image_aux_data*)image)->offset);
    // Use uint for poitner computations to avoid type overrun
    void* pixel = (void*)((image_aux_data*)image)->pData+(uint)coord.x * offset.x + (uint)coord.y * offset.y;
    return pixel;
}

void __attribute__((overloadable)) soa4_extract_pixel(image2d_t image, int4 coord_x, int4 coord_y, void** p1, void** p2, void** p3, void** p4)
{
    uint4 offset_x = (uint4)(((image_aux_data*)image)->offset[0]);
    uint4 offset_y = (uint4)(((image_aux_data*)image)->offset[1]);

    // Use uint for poitner computations to avoid type overrun
    uint4 ocoord_x = ((uint4)coord_x) * offset_x;
    uint4 ocoord_y = ((uint4)coord_y) * offset_y;

    char* pData = (void*)((image_aux_data*)image)->pData;

    uint4 ocoord = ocoord_x + ocoord_y;
    *p1 = pData + ocoord.s0;
    *p2 = pData + ocoord.s1;
    *p3 = pData + ocoord.s2;
    *p4 = pData + ocoord.s3;
}

void __attribute__((overloadable)) soa8_extract_pixel(image2d_t image, int8 coord_x, int8 coord_y, void** p0, void** p1, void** p2, void** p3, void** p4, void** p5, void** p6, void** p7)
{
    uint8 offset_x = (uint8)(((image_aux_data*)image)->offset[0]);
    uint8 offset_y = (uint8)(((image_aux_data*)image)->offset[1]);
    
    uint8 ocoord_x = ((uint8)coord_x) * offset_x;
    uint8 ocoord_y = ((uint8)coord_y) * offset_y;

    char* pData = (void*)((image_aux_data*)image)->pData;

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

void* __attribute__((overloadable)) __attribute__((const)) extract_pixel(image2d_array_t image, int4 coord)
{
    uint4 offset = *(uint4*)(((image_aux_data*)image)->offset);
    void* pixel = (void*)((image_aux_data*)image)->pData+(uint)coord.x * offset.x + (uint)coord.y * offset.y 
               + (uint)coord.z*((image_aux_data*)image)->pitch[1];
    return pixel;
}

void* __attribute__((overloadable)) __attribute__((const)) extract_pixel(image1d_t image, int coord)
{
    uint4 offset = *(uint4*)(((image_aux_data*)image)->offset);
    void* pixel = (void*)((image_aux_data*)image)->pData+(uint)coord * offset.x;
    return pixel;
}

void* __attribute__((overloadable)) __attribute__((const)) extract_pixel(image1d_array_t image, int2 coord)
{
    uint4 offset = *(uint4*)(((image_aux_data*)image)->offset);
    // Offset for image array correspongs to offset inside image
    // that's why  to compute pixel pointer dimension is used here
    void* pixel = (void*)((image_aux_data*)image)->pData + coord.x * offset.x + coord.y * (uint)((image_aux_data*)image)->pitch[0];
    return pixel;
}

void* __attribute__((overloadable)) __attribute__((const))  GetImagePtr(image2d_array_t image, float idxFloat)
{
    // First convert integer image index
    int idx = rint(idxFloat);
    // clamp idx to edge
    if(idx < 0)
    {
        idx = 0;
    } else if(idx >= ((image_aux_data*)image)->array_size)
    {
        idx = ((image_aux_data*)image)->array_size - 1;
    }
    void* ptr = (void*)((image_aux_data*)image)->pData + ((image_aux_data*)image)->pitch[1]*idx;
    return ptr;
}

void* __attribute__((overloadable)) __attribute__((const))  GetImagePtr(image1d_array_t image, float idxFloat)
{
    // First convert integer image index
    int idx = rint(idxFloat);
    // clamp idx to edge
    if(idx < 0)
    {
        idx = 0;
    } else if(idx >= ((image_aux_data*)image)->array_size)
    {
        idx = ((image_aux_data*)image)->array_size - 1;
    }
    void* ptr = (void*)((image_aux_data*)image)->pData + ((image_aux_data*)image)->pitch[0]*idx;
    return ptr;
}

void* __attribute__((overloadable)) __attribute__((const)) GetImagePtr(image2d_array_t image, int idx)
{
    // clamp idx to edge
    if(idx < 0)
    {
        idx = 0;
    } else if(idx >= ((image_aux_data*)image)->array_size)
    {
        idx = ((image_aux_data*)image)->array_size - 1;
    }
    void* ptr = (void*)((image_aux_data*)image)->pData + ((image_aux_data*)image)->pitch[1]*idx;
    return ptr;
}

void* __attribute__((overloadable)) __attribute__((const)) GetImagePtr(image1d_array_t image, int idx)
{
    if(idx < 0)
    {
        idx = 0;
    } else if(idx >= ((image_aux_data*)image)->array_size)
    {
        idx = ((image_aux_data*)image)->array_size - 1;
    }
    void* ptr = (void*)((image_aux_data*)image)->pData + ((image_aux_data*)image)->pitch[0]*idx;
    return ptr;
}

/// Integer coordinate translation callbacks

// Coordinates callback for integer input
typedef int4 (*Image_I_COORD_CBK) (void*, int4);
// Coordinates callback for SOA4 integer input
typedef void (*SOA4_Image_I_COORD_CBK) (void*, int4, int4, int4*, int4*);
// Coordinates callback for SOA8 integer input
typedef void (*SOA8_Image_I_COORD_CBK) (void*, int8, int8, int8*, int8*);

int4 __attribute__((overloadable)) trans_coord_int_NONE_FALSE_NEAREST(void* image, int4 coord)
{
    //not testing if coords are OOB - this mode doesn't guarantee safeness!
    return coord;
}

void __attribute__((overloadable)) soa4_trans_coord_int_NONE_FALSE_NEAREST(void* image, int4 coord_x, int4 coord_y, int4* res_coord_x, int4* res_coord_y)
{
    //not testing if coords are OOB - this mode doesn't guarantee safeness!
    *res_coord_x = coord_x;
    *res_coord_y = coord_y;
}

void __attribute__((overloadable)) soa8_trans_coord_int_NONE_FALSE_NEAREST(void* image, int8 coord_x, int8 coord_y, int8* res_coord_x, int8* res_coord_y)
{
    //not testing if coords are OOB - this mode doesn't guarantee safeness!
    *res_coord_x = coord_x;
    *res_coord_y = coord_y;
}

int4 __attribute__((overloadable)) trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST(void* image, int4 coord)
{
    return ProjectToEdgeInt((image2d_t)image, coord);
}

void __attribute__((overloadable)) soa4_trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST(void* image, int4 coord_x, int4 coord_y, int4* res_coord_x, int4* res_coord_y)
{
    return SOA4_ProjectToEdgeInt((image2d_t)image, coord_x, coord_y, res_coord_x, res_coord_y);
}

void __attribute__((overloadable)) soa8_trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST(void* image, int8 coord_x, int8 coord_y, int8* res_coord_x, int8* res_coord_y)
{
    return SOA8_ProjectToEdgeInt((image2d_t)image, coord_x, coord_y, res_coord_x, res_coord_y);
}

int4 __attribute__((overloadable)) trans_coord_int_UNDEFINED(void* image, int4 coord)
{
    return UndefCoordInt;
}

void __attribute__((overloadable)) soa4_trans_coord_int_UNDEFINED(void* image, int4 coord_x, int4 coord_y, int4* res_coord_x, int4* res_coord_y)
{
    *res_coord_x = SOA4_UndefCoordIntX;
    *res_coord_y = SOA4_UndefCoordIntY;
}

void __attribute__((overloadable)) soa8_trans_coord_int_UNDEFINED(void* image, int8 coord_x, int8 coord_y, int8* res_coord_x, int8* res_coord_y)
{
    *res_coord_x = SOA8_UndefCoordIntX;
    *res_coord_y = SOA8_UndefCoordIntY;
}

// Following table used to belong to image_aux_data similar to coord_translate_f_callback,
// but was moved here statically to facilitate inlining. It relies on the enumeration of samplers:
//
// !!!IMPORTANT!!! These defines should be the same as in ImageCallbackLibrary.h and cl_image_declaration.h
//#define NONE_FALSE_NEAREST 0x00
//#define CLAMP_FALSE_NEAREST 0x01
//#define CLAMPTOEDGE_FALSE_NEAREST 0x02
//    pImageAuxData->coord_translate_i_callback[NONE_FALSE_NEAREST] = pImageCallbackFuncs->m_fpINoneFalseNearest;
//    pImageAuxData->coord_translate_i_callback[CLAMP_FALSE_NEAREST] = pImageCallbackFuncs->m_fpINoneFalseNearest;
//    pImageAuxData->coord_translate_i_callback[CLAMPTOEDGE_FALSE_NEAREST] = pImageCallbackFuncs->m_fpIClampToEdgeFalseNearest;
//    pImageAuxData->coord_translate_i_callback[REPEAT_FALSE_NEAREST] = pImageCallbackFuncs->m_fpIUndefTrans;    //REPEAT+UI COORDINATES MODE IS NOT DEFINED
//    pImageAuxData->coord_translate_i_callback[MIRRORED_FALSE_NEAREST] = pImageCallbackFuncs->m_fpIUndefTrans;    //REPEAT+UI COORDINATES MODE IS NOT DEFINED
//    Normalized and bilinear modes are not defined with integer coordinates
// and all remaining entries are undefined.

Image_I_COORD_CBK const            coord_translate_i_callback[32] = {
    trans_coord_int_NONE_FALSE_NEAREST,
    trans_coord_int_NONE_FALSE_NEAREST,
    trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED,
    trans_coord_int_UNDEFINED
};    //the list of integer coordinate translation callback

SOA4_Image_I_COORD_CBK const    soa4_coord_translate_i_callback[32] = {
    soa4_trans_coord_int_NONE_FALSE_NEAREST,
    soa4_trans_coord_int_NONE_FALSE_NEAREST,
    soa4_trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED,
    soa4_trans_coord_int_UNDEFINED
};    //the list of soa4 integer coordinate translation callback

SOA8_Image_I_COORD_CBK const    soa8_coord_translate_i_callback[32] = {
    soa8_trans_coord_int_NONE_FALSE_NEAREST,
    soa8_trans_coord_int_NONE_FALSE_NEAREST,
    soa8_trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED,
    soa8_trans_coord_int_UNDEFINED
};    //the list of soa8 integer coordinate translation callback


/// Image reading callbacks

/// read_imageui implementation

uint4  __attribute__((overloadable)) read_imageui(image2d_t image, sampler_t sampler, int2 coord)
{
    int4 coord4 = (int4)(0, 0, 0, 0);
    coord4.lo = coord;
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

void __attribute__((overloadable)) soa4_read_imageui(image2d_t image, sampler_t sampler, int4 coord_x, int4 coord_y, 
                                                    uint4* res_x, uint4* res_y, uint4* res_z, uint4* res_w)
{
    void* pData =((image_aux_data*)image)->pData;
    SOA4_Image_I_COORD_CBK coord_cbk = soa4_coord_translate_i_callback[sampler];
    SOA4_Image_UI_READ_CBK read_cbk = (SOA4_Image_UI_READ_CBK)((image_aux_data*)image)->soa4_read_img_callback_int[sampler];
    int4 translated_coord_x;
    int4 translated_coord_y;
    coord_cbk((void*)image, coord_x, coord_y, &translated_coord_x, &translated_coord_y );
    read_cbk((void*)image, translated_coord_x, translated_coord_y, pData, res_x, res_y, res_z, res_w);
}

void __attribute__((overloadable)) soa8_read_imageui(image2d_t image, sampler_t sampler, int8 coord_x, int8 coord_y,
                                                    uint8* res_x, uint8* res_y, uint8* res_z, uint8* res_w)
{
    void* pData =((image_aux_data*)image)->pData;
    SOA8_Image_I_COORD_CBK coord_cbk = soa8_coord_translate_i_callback[sampler];
    SOA8_Image_UI_READ_CBK read_cbk = (SOA8_Image_UI_READ_CBK)((image_aux_data*)image)->soa8_read_img_callback_int[sampler];
    int8 translated_coord_x;
    int8 translated_coord_y;
    coord_cbk((void*)image, coord_x, coord_y, &translated_coord_x, &translated_coord_y );
    read_cbk((void*)image, translated_coord_x, translated_coord_y, pData, res_x, res_y, res_z, res_w);
}

uint4  __attribute__((overloadable)) mask_read_imageui(int mask, image2d_t image, sampler_t sampler, int2 coord)
{
  if (mask) return read_imageui(image, sampler, coord);
  return (uint4)(0, 0, 0, 0);
}

uint4  __attribute__((overloadable)) read_imageui(image3d_t image, sampler_t sampler, int4 coord)
{
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position=coord_cbk((void*)image, coord);
    return read_cbk((void*)image, trans_position, pData);
}

uint4  __attribute__((overloadable)) read_imageui(image2d_t image, sampler_t sampler, float2 coord)
{
    float4 coord4 = (float4)(0.f, 0.f ,0.f ,0.f);
    coord4.lo = coord;
    void* pData =((image_aux_data*)image)->pData;
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

uint4  __attribute__((overloadable)) mask_read_imageui(int mask, image2d_t image, sampler_t sampler, float2 coord)
{
  if (mask) return read_imageui(image, sampler, coord);
  return (uint4)(0,0,0,0);
}

uint4  __attribute__((overloadable)) read_imageui(image3d_t image, sampler_t sampler, float4 coord)
{
    coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    void* pData =((image_aux_data*)image)->pData;
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position=coord_cbk((void*)image, coord);
    return read_cbk((void*)image, trans_position, pData);
}

void  __attribute__((overloadable)) write_imageui(image2d_t image, int2 coord, uint4 color)
{
    void* pixel = extract_pixel(image, coord);
    Image_UI_WRITE_CBK cbk = (Image_UI_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

void __attribute__((overloadable)) soa4_write_imageui(image2d_t image, int4 coord_x, int4 coord_y, uint4 val_x, uint4 val_y, uint4 val_z, uint4 val_w)
{
    uchar4* p1;
    uchar4* p2;
    uchar4* p3;
    uchar4* p4;
    soa4_extract_pixel(image, coord_x, coord_y, (void**)&p1, (void**)&p2, (void**)&p3, (void**)&p4);
    SOA4_Image_UI_WRITE_CBK cbk = (SOA4_Image_UI_WRITE_CBK)((image_aux_data*)image)->soa4_write_img_callback;
    cbk(p1, p2, p3, p4, val_x, val_y, val_z, val_w);
}

void __attribute__((overloadable)) soa8_write_imageui(image2d_t image, int8 coord_x, int8 coord_y, uint8 val_x, uint8 val_y, uint8 val_z, uint8 val_w)
{
    uchar4* p0;
    uchar4* p1;
    uchar4* p2;
    uchar4* p3;
    uchar4* p4;
    uchar4* p5;
    uchar4* p6;
    uchar4* p7;
    soa8_extract_pixel(image, coord_x, coord_y, (void**)&p0, (void**)&p1, (void**)&p2, (void**)&p3, (void**)&p4, (void**)&p5, (void**)&p6, (void**)&p7);
    SOA8_Image_UI_WRITE_CBK cbk = (SOA8_Image_UI_WRITE_CBK)((image_aux_data*)image)->soa8_write_img_callback;
    cbk(p0, p1, p2, p3, p4, p5, p6, p7, val_x, val_y, val_z, val_w);
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
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

int4  __attribute__((overloadable)) mask_read_imagei(int mask, image2d_t image, sampler_t sampler, int2 coord)
{
  if (mask) return read_imagei(image, sampler, coord);
  return (int4)(0, 0, 0, 0);
}

int4  __attribute__((overloadable)) read_imagei(image3d_t image, sampler_t sampler, int4 coord)
{
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position=coord_cbk((void*)image, coord);
    return read_cbk((void*)image, trans_position, pData);
}

int4  __attribute__((overloadable)) read_imagei(image2d_t image, sampler_t sampler, float2 coord)
{
    float4 coord4 = (float4)(0.f, 0.f, 0.f, 0.f);
    coord4.lo = coord;
    void* pData =((image_aux_data*)image)->pData;
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

int4  __attribute__((overloadable)) mask_read_imagei(int mask, image2d_t image, sampler_t sampler, float2 coord)
{
  if (mask) return read_imagei(image, sampler, coord);
  return (int4)(0, 0, 0, 0);
}

int4  __attribute__((overloadable)) read_imagei(image3d_t image, sampler_t sampler, float4 coord)
{
    coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    void* pData =((image_aux_data*)image)->pData;
    int4 trans_position=coord_cbk((void*)image, coord);
    return read_cbk((void*)image, trans_position, pData);
}

void  __attribute__((overloadable)) write_imagei(image2d_t image, int2 coord, int4 color)
{
    void* pixel = extract_pixel(image, coord);
    Image_I_WRITE_CBK cbk = (Image_I_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

void  __attribute__((overloadable)) mask_write_imagei(int mask, image2d_t image, int2 coord, int4 color){
  if (mask) write_imagei(image, coord, color);
}

/***********************************FLOAT IMAGE I/O FUNCTIONS (read_imagef)********************************************************/


float4  __attribute__((overloadable)) read_imagef(image2d_t image, sampler_t sampler, int2 coord)
{
    int4 coord4 = (int4)(0.f, 0.f, 0.f, 0.f);
    coord4.lo = coord;
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback_float[sampler];
    int4 dummy0;
    float4 dummy1;
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, dummy0, dummy1, pData);
}

float4  __attribute__((overloadable)) mask_read_imagef(int mask, image2d_t image, sampler_t sampler, int2 coord)
{
  if (mask) return read_imagef(image, sampler, coord);
  return (float4)(0.0f, 0.0f, 0.0f, 0.0f);
}

float4  __attribute__((overloadable)) read_imagef(image3d_t image, sampler_t sampler, int4 coord)
{
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback_float[sampler];
    int4 dummy0;
    float4 dummy1;
    int4 trans_position=coord_cbk((void*)image, coord);
    return read_cbk((void*)image, trans_position, dummy0, dummy1, pData);
}

float4  __attribute__((overloadable)) read_imagef(image2d_t image, sampler_t sampler, float2 coord)
{
    float4 coord4 = (float4)(0.f, 0.f, 0.f, 0.f);
    coord4.lo = coord;
    void* pData =((image_aux_data*)image)->pData;
    Image_FF_COORD_CBK coord_cbk=(Image_FF_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)((image_aux_data*)image)->read_img_callback_float[sampler];
    ///// components for interpolation
    int4 square0, square1;
    float4 fraction= coord_cbk((void*)image, coord4, &square0, &square1);
    return read_cbk((void*)image, square0, square1, fraction, pData);
}

float4  __attribute__((overloadable)) mask_read_imagef(int mask, image2d_t image, sampler_t sampler, float2 coord)
{
    if (mask) return read_imagef(image, sampler, coord);
    return (float4)(0.f, 0.f, 0.f, 0.f);
}

float4  __attribute__((overloadable)) read_imagef(image3d_t image, sampler_t sampler, float4 coord)
{
    coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    void* pData =((image_aux_data*)image)->pData;
    Image_FF_COORD_CBK coord_cbk=(Image_FF_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)((image_aux_data*)image)->read_img_callback_float[sampler];
    int4 square0, square1;
    float4 fraction=coord_cbk((void*)image, coord, &square0, &square1);
    return read_cbk((void*)image, square0, square1, fraction, pData);
}

void  __attribute__((overloadable)) write_imagef(image2d_t image, int2 coord, float4 color)
{
    void* pixel = extract_pixel(image, coord);
    Image_F_WRITE_CBK cbk = (Image_F_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

void  __attribute__((overloadable)) mask_write_imagef(int mask, image2d_t image, int2 coord, float4 color)
{
  if (mask) write_imagef(image, coord, color);
}

/// 1_2 image functions
/// They are left empty to be able to add 1.2 images to recorder and BIMeter before implementation

float4  __attribute__((overloadable)) read_imagef(image2d_array_t image, sampler_t sampler, int4 coord)
{
    int4 internal_coord = coord;
    internal_coord.z = 0;
    int arrayIndex = coord.z;
    void* pData = GetImagePtr(image, arrayIndex);
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback_float[sampler];
    int4 dummy0;
    float4 dummy1;
    int4 trans_position=coord_cbk((void*)image, internal_coord);
    float4 val = read_cbk((void*)image, trans_position, dummy0, dummy1, pData);
    return val;
}

float4  __attribute__((overloadable)) read_imagef(image2d_array_t image, sampler_t sampler, float4 coord)
{
    float4 internal_coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    internal_coord.z = 0;
    void* pData = GetImagePtr(image, coord.z);
    Image_FF_COORD_CBK coord_cbk=(Image_FF_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)((image_aux_data*)image)->read_img_callback_float[sampler];
    int4 square0, square1;
    float4 fraction=coord_cbk((void*)image, internal_coord, &square0, &square1);
    float4 val = read_cbk((void*)image, square0, square1, fraction, pData);
    return val;
}

int4  __attribute__((overloadable)) read_imagei(image2d_array_t image, sampler_t sampler, int4 coord)
{
    int4 internal_coord = coord;
    internal_coord.z = 0;
    void* pData = GetImagePtr(image, coord.z);
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position=coord_cbk((void*)image, internal_coord);
    int4 val = read_cbk((void*)image, trans_position, pData);
    return val;
}

int4  __attribute__((overloadable)) read_imagei(image2d_array_t image, sampler_t sampler, float4 coord)
{
    float4 internal_coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    internal_coord.z = 0.f;
    void* pData = GetImagePtr(image, coord.z);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position = coord_cbk((void*)image, internal_coord);
    int4 val = read_cbk((void*)image, trans_position, pData);
    return val;
}

uint4  __attribute__((overloadable)) read_imageui(image2d_array_t image, sampler_t sampler, int4 coord)
{
    int4 internal_coord = coord;
    internal_coord.z = 0;
    void* pData = GetImagePtr(image, coord.z);
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position=coord_cbk((void*)image, internal_coord);
    uint4 val = read_cbk((void*)image, trans_position, pData);
    return val;
}

uint4  __attribute__((overloadable)) read_imageui(image2d_array_t image, sampler_t sampler, float4 coord)
{
    float4 internal_coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    internal_coord.z = 0.f;
    void* pData = GetImagePtr(image, coord.z);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position = coord_cbk((void*)image, internal_coord);
    uint4 val = read_cbk((void*)image, trans_position, pData);
    return val;
}

float4 __attribute__((overloadable)) read_imagef(image1d_t image, sampler_t sampler, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback_float[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);    
    int4 dummy0;
    float4 dummy1;
	return read_cbk((void*)image, trans_position, dummy0, dummy1, pData);
}

float4 __attribute__((overloadable)) read_imagef(image1d_t image, sampler_t sampler, float coord)
{
    float4 coord4=(float4)(coord, 0.0,0.0,0.0);
    void* pData =((image_aux_data*)image)->pData;
    Image_FF_COORD_CBK coord_cbk=(Image_FF_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)((image_aux_data*)image)->read_img_callback_float[sampler];
    int4 square0, square1;
    float4 fraction= coord_cbk((void*)image, coord4, &square0, &square1);
    return read_cbk((void*)image, square0, square1, fraction, pData);
}

int4 __attribute__((overloadable)) read_imagei(image1d_t image, sampler_t sampler, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

int4 __attribute__((overloadable)) read_imagei(image1d_t image, sampler_t sampler, float coord)
{
    float4 coord4=(float4)(coord, 0.0,0.0,0.0);
    void* pData =((image_aux_data*)image)->pData;
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

uint4 __attribute__((overloadable)) read_imageui(image1d_t image, sampler_t sampler, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk= coord_translate_i_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

uint4 __attribute__((overloadable)) read_imageui(image1d_t image, sampler_t sampler, float coord)
{
    float4 coord4=(float4)(coord, 0.0,0.0,0.0);
    void* pData =((image_aux_data*)image)->pData;
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

float4 __attribute__((overloadable)) read_imagef(image1d_array_t image, sampler_t sampler, int2 coord)
{   
    int4 internal_coord=(int4)(coord.x, 0,0,0);
    int arrayIndex = coord.y;
    void* pData = GetImagePtr(image, arrayIndex);
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback_float[sampler];
    int4 trans_position=coord_cbk((void*)image, internal_coord);
    int4 dummy0;
    float4 dummy1;
    float4 val = read_cbk((void*)image, trans_position, dummy0, dummy1, pData);
    return val;

}

float4 __attribute__((overloadable)) read_imagef(image1d_array_t image, sampler_t sampler, float2 coord)
{
    float4 internal_coord = (float4)(coord.x, 0.0,0.0,0.0);
    void* pData = GetImagePtr(image, coord.y);
    Image_FF_COORD_CBK coord_cbk=(Image_FF_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)((image_aux_data*)image)->read_img_callback_float[sampler];
    int4 square0, square1;
    float4 fraction=coord_cbk((void*)image, internal_coord, &square0, &square1);
    float4 val = read_cbk((void*)image, square0, square1, fraction, pData);
    return val;
}

int4 __attribute__((overloadable)) read_imagei(image1d_array_t image, sampler_t sampler, int2 coord)
{
    int4 internal_coord = (int4)(coord.x, 0,0,0);
    void* pData = GetImagePtr(image, coord.y);
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position=coord_cbk((void*)image, internal_coord);
    int4 val = read_cbk((void*)image, trans_position, pData);
    return val;
}

int4 __attribute__((overloadable)) read_imagei(image1d_array_t image, sampler_t sampler, float2 coord)
{
    float4 internal_coord = (float4)(coord.x, 0.0,0.0,0.0);
    void* pData = GetImagePtr(image, coord.y);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position = coord_cbk((void*)image, internal_coord);
    int4 val = read_cbk((void*)image, trans_position, pData);
    return val;
}

uint4 __attribute__((overloadable)) read_imageui(image1d_array_t image, sampler_t sampler, int2 coord)
{
    int4 internal_coord = (int4)(coord.x, 0,0,0);
    void* pData = GetImagePtr(image, coord.y);
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position=coord_cbk((void*)image, internal_coord);
    uint4 val = read_cbk((void*)image, trans_position, pData);
    return val;
}

uint4 __attribute__((overloadable)) read_imageui(image1d_array_t image, sampler_t sampler, float2 coord)
{
    float4 internal_coord = (float4)(coord.x, 0.0,0.0,0.0);
    void* pData = GetImagePtr(image, coord.y);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[sampler];
    int4 trans_position = coord_cbk((void*)image, internal_coord);
    uint4 val = read_cbk((void*)image, trans_position, pData);
    return val;
}

// sampler-less calls
float4 __attribute__((overloadable)) read_imagef (image2d_t image, int2 coord)
{
    int4 coord4 = (int4)(0, 0, 0, 0);
    coord4.lo = coord;
    void* pData =((image_aux_data*)image)->pData;
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback_float[SIMPLE_SAMPLER];
    int4 dummy0;
    float dummy1;
    return read_cbk((void*)image, coord4, dummy0, dummy1, pData);
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
    void* pData =((image_aux_data*)image)->pData;
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord4, pData);
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
    void* pData =((image_aux_data*)image)->pData;
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord4, pData);
}

uint4 __attribute__((overloadable)) mask_read_imageui (int mask, image2d_t image, int2 coord)
{
  if (mask) return read_imageui(image, coord);
  return (uint4)(0, 0, 0, 0);
}

float4 __attribute__((overloadable)) read_imagef (image3d_t image, int4 coord)
{
    void* pData =((image_aux_data*)image)->pData;
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback_float[SIMPLE_SAMPLER];
    int4 dummy0;
    float4 dummy1;
    return read_cbk((void*)image, coord, dummy0, dummy1, pData);
}

int4 __attribute__((overloadable)) read_imagei (image3d_t image, int4 coord)
{
    void* pData =((image_aux_data*)image)->pData;
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord, pData);
}

uint4 __attribute__((overloadable)) read_imageui (image3d_t image, int4 coord)
{
    void* pData =((image_aux_data*)image)->pData;
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord, pData);
}

float4 __attribute__((overloadable)) read_imagef (image2d_array_t image, int4 coord)
{
    // naive read_imagef implementation
    int4 internal_coord = coord;
    internal_coord.z = 0;
    void* pData = GetImagePtr(image, coord.z);
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback_float[SIMPLE_SAMPLER];
    int4 dummy0; 
    float dummy1;
    return read_cbk((void*)image, internal_coord, dummy0, dummy1, pData);
}

int4 __attribute__((overloadable)) read_imagei (image2d_array_t image, int4 coord)
{
    int4 internal_coord = coord;
    internal_coord.z = 0;
    void* pData = GetImagePtr(image, coord.z);
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[SIMPLE_SAMPLER];
    return read_cbk((void*)image, internal_coord, pData);
}

uint4 __attribute__((overloadable)) read_imageui (image2d_array_t image, int4 coord)
{
    int4 internal_coord = coord;
    internal_coord.z = 0;
    void* pData = GetImagePtr(image, coord.z);
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[SIMPLE_SAMPLER];
    return read_cbk((void*)image, internal_coord, pData);
}

float4 __attribute__((overloadable)) read_imagef (image1d_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback_float[SIMPLE_SAMPLER];
    int4 dummy0;
    float4 dummy1;
    return read_cbk((void*)image, coord4, dummy0, dummy1, pData);
}

float4 __attribute__((overloadable)) read_imagef (image1d_buffer_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback_float[SIMPLE_SAMPLER];
    int4 dummy0;
    float dummy1;
    return read_cbk((void*)image, coord4, dummy0, dummy1, pData);
}

int4 __attribute__((overloadable)) read_imagei(image1d_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord4, pData);
}

uint4 __attribute__((overloadable)) read_imageui(image1d_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord4, pData);
}

int4 __attribute__((overloadable)) read_imagei(image1d_buffer_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord4, pData);
}

uint4 __attribute__((overloadable)) read_imageui(image1d_buffer_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord4, pData);
}

float4 __attribute__((overloadable)) read_imagef(image1d_array_t image, int2 coord)
{
    int4 internal_coord=(int4)(coord.x, 0,0,0);
    void* pData = GetImagePtr(image, coord.y);
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback_float[SIMPLE_SAMPLER];
    int4 dummy0;
    float dummy1;
    return read_cbk((void*)image, internal_coord, dummy0, dummy1, pData);
}

int4 __attribute__((overloadable)) read_imagei(image1d_array_t image, int2 coord)
{
    int4 internal_coord=(int4)(coord.x, 0,0,0);
    void* pData = GetImagePtr(image, coord.y);
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback_int[SIMPLE_SAMPLER];
    return read_cbk((void*)image, internal_coord, pData);
}

uint4 __attribute__((overloadable)) read_imageui(image1d_array_t image, int2 coord)
{
	int4 internal_coord=(int4)(coord.x, 0,0,0);
    void* pData = GetImagePtr(image, coord.y);
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback_int[SIMPLE_SAMPLER];
    return read_cbk((void*)image, internal_coord, pData);
}

// write_image calls
void __attribute__((overloadable)) write_imagef (image2d_array_t image, int4 coord, float4 color)
{
    void* pixel = extract_pixel(image, coord);
    Image_F_WRITE_CBK cbk = (Image_F_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

void __attribute__((overloadable)) write_imagei (image2d_array_t image, int4 coord, int4 color)
{
    void* pixel = extract_pixel(image, coord);
    Image_I_WRITE_CBK cbk = (Image_I_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

void __attribute__((overloadable)) write_imageui (image2d_array_t image, int4 coord, uint4 color)
{
    void* pixel = extract_pixel(image, coord);
    Image_UI_WRITE_CBK cbk = (Image_UI_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

void __attribute__((overloadable)) write_imagef (image1d_t image, int coord, float4 color)
{
    void* pixel = extract_pixel(image, coord);
    Image_F_WRITE_CBK cbk = (Image_F_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

void __attribute__((overloadable)) write_imagei (image1d_t image, int coord, int4 color)
{
    void* pixel = extract_pixel(image, coord);
    Image_I_WRITE_CBK cbk = (Image_I_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

void __attribute__((overloadable)) write_imageui (image1d_t image, int coord, uint4 color)
{
    void* pixel = extract_pixel(image, coord);
    Image_UI_WRITE_CBK cbk = (Image_UI_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

void __attribute__((overloadable)) write_imagef (image1d_buffer_t image, int coord, float4 color)
{
    void* pixel = extract_pixel((image1d_t)image, coord);
    Image_F_WRITE_CBK cbk = (Image_F_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

void __attribute__((overloadable)) write_imagei (image1d_buffer_t image, int coord, int4 color)
{
    void* pixel = extract_pixel((image1d_t)image, coord);
    Image_I_WRITE_CBK cbk = (Image_I_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

void __attribute__((overloadable)) write_imageui (image1d_buffer_t image, int coord, uint4 color)
{
    void* pixel = extract_pixel((image1d_t)image, coord);
    Image_UI_WRITE_CBK cbk = (Image_UI_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

void __attribute__((overloadable)) write_imagef (image1d_array_t image, int2 coord, float4 color)
{
    void* pixel = extract_pixel(image, coord);
    Image_F_WRITE_CBK cbk = (Image_F_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

void __attribute__((overloadable)) write_imagei (image1d_array_t image, int2 coord, int4 color)
{
    void* pixel = extract_pixel(image, coord);
    Image_I_WRITE_CBK cbk = (Image_I_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

void __attribute__((overloadable)) write_imageui (image1d_array_t image, int2 coord, uint4 color)
{
    void* pixel = extract_pixel(image, coord);
    Image_UI_WRITE_CBK cbk = (Image_UI_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
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


#ifdef __cplusplus
}
#endif

#endif // defined (__MIC__) || defined(__MIC2__)
