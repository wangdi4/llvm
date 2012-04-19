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

#define NORMALIZED_SAMPLER 0x08


#define ALIGN16 __attribute__ ((aligned(16)))

const int fVec4FloatZeroCoordMask3D[4] ALIGN16 = {0xffffffff, 0xffffffff, 0xffffffff, 0};

ALIGN16 const int4 UndefCoordInt={-1,-1, -1, -1};

int4 __attribute__((overloadable)) ProjectToEdgeInt(image2d_t image, int4 coord);

/************************Integer coordinate translations*************************************/

int4 __attribute__((overloadable)) trans_coord_int_NONE_FALSE_NEAREST(void* image, int4 coord)
{
	//not testing if coords are OOB - this mode doesn't guarantee safeness!
	return coord;
}

int4 __attribute__((overloadable)) trans_coord_int_CLAMPTOEDGE_FALSE_NEAREST(void* image, int4 coord)
{
	return ProjectToEdgeInt((image2d_t)image, coord);
}

int4 __attribute__((overloadable)) trans_coord_int_UNDEFINED(void* image, int4 coord)
{
	return UndefCoordInt;   //will be background color, but it's a "don't care" situtation
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

// coordinates callback for integer input
typedef int4 (*Image_I_COORD_CBK) (void*, int4);

Image_I_COORD_CBK const			coord_translate_i_callback[32] = {
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

int __attribute__((overloadable)) __attribute__((const)) get_image_width(image1d_t img)
{
	return ((image_aux_data*)img)->dim[0];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_width(image1d_array_t img)
{
	return ((image_aux_data*)img)->dim[0];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_width(image1d_buffer_t img)
{
	return ((image_aux_data*)img)->dim[0];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_width(image2d_t img)
{
	return ((image_aux_data*)img)->dim[0];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_width(image2d_array_t img)
{
	return ((image_aux_data*)img)->dim[0];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_width(image3d_t img)
{
	return ((image_aux_data*)img)->dim[0];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_height(image2d_array_t img)
{
	return ((image_aux_data*)img)->dim[1];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_height(image2d_t img)
{
	return ((image_aux_data*)img)->dim[1];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_height(image3d_t img)
{
	return ((image_aux_data*)img)->dim[1];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_depth(image3d_t img)
{
	return ((image_aux_data*)img)->dim[2];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_data_type(image1d_t img)
{
	return ((image_aux_data*)img)->format.image_channel_data_type;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_data_type(image1d_array_t img)
{
	return ((image_aux_data*)img)->format.image_channel_data_type;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_data_type(image1d_buffer_t img)
{
	return ((image_aux_data*)img)->format.image_channel_data_type;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_data_type(image2d_t img)
{
	return ((image_aux_data*)img)->format.image_channel_data_type;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_data_type(image2d_array_t img)
{
	return ((image_aux_data*)img)->format.image_channel_data_type;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_data_type(image3d_t img)
{
	return ((image_aux_data*)img)->format.image_channel_data_type;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_order(image1d_t img)
{
	return ((image_aux_data*)img)->format.image_channel_order;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_order(image1d_array_t img)
{
	return ((image_aux_data*)img)->format.image_channel_order;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_order(image1d_buffer_t img)
{
	return ((image_aux_data*)img)->format.image_channel_order;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_order(image2d_t img)
{
	return ((image_aux_data*)img)->format.image_channel_order;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_order(image2d_array_t img)
{
	return ((image_aux_data*)img)->format.image_channel_order;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_order(image3d_t img)
{
	return ((image_aux_data*)img)->format.image_channel_order;
}

_2i32 __attribute__((overloadable)) __attribute__((const)) get_image_dim(image2d_t img)
{
	_2i32 res;

	res.lo = ((image_aux_data*)img)->dim[0];
	res.hi = ((image_aux_data*)img)->dim[1];
	return res;
}

_2i32 __attribute__((overloadable)) __attribute__((const)) get_image_dim(image2d_array_t img)
{
	_2i32 res;

	res.lo = ((image_aux_data*)img)->dim[0];
	res.hi = ((image_aux_data*)img)->dim[1];
	return res;
}

_4i32 __attribute__((overloadable)) __attribute__((const)) get_image_dim(image3d_t img)
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

void* __attribute__((overloadable)) __attribute__((const)) extract_pixel(image2d_t image, int2 coord)
{
    uint4 offset = *(uint4*)(((image_aux_data*)image)->offset);
    // Use uint for poitner computations to avoid type overrun
    void* pixel = (void*)((image_aux_data*)image)->pData+(uint)coord.x * offset.x + (uint)coord.y * offset.y;
    return pixel;
}

void* __attribute__((overloadable)) __attribute__((const)) extract_pixel(image2d_array_t image, int4 coord)
{
    uint4 offset = *(uint4*)(((image_aux_data*)image)->offset);
    void* pixel = (void*)((image_aux_data*)image)->pData+(uint)coord.x * offset.x + (uint)coord.y * offset.y 
               + (uint)coord.z*((image_aux_data*)image)->dim[0]*((image_aux_data*)image)->dim[1]*((image_aux_data*)image)->offset[0];
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
    void* pixel = (void*)((image_aux_data*)image)->pData+ offset.x * ((uint)coord.x + 
        (uint)((image_aux_data*)image)->dim[0]*(uint)coord.y);
    return pixel;
}

void* __attribute__((overloadable)) __attribute__((const))  GetImagePtr(image2d_array_t image, sampler_t sampler,float idxFloat)
{
    int isNormalized = sampler & NORMALIZED_SAMPLER;
    if(isNormalized)
    {
        idxFloat = idxFloat * ((image_aux_data*)image)->array_size;
    }
    // First convert integer image index
    int idx = floor(idxFloat + 0.5f);
    // clamp idx to edge
    if(idx < 0)
    {
        idx = 0;
    } else if(idx >= ((image_aux_data*)image)->array_size)
    {
        idx = ((image_aux_data*)image)->array_size - 1;
    }
    void* ptr = (void*)((image_aux_data*)image)->pData + ((image_aux_data*)image)->dim[0]*((image_aux_data*)image)->dim[1]*((image_aux_data*)image)->offset[0]*idx;
    return ptr;
}

void* __attribute__((overloadable)) __attribute__((const))  GetImagePtr(image1d_array_t image, sampler_t sampler,float idxFloat)
{
    int isNormalized = sampler & NORMALIZED_SAMPLER;
    if(isNormalized)
    {
        idxFloat = idxFloat * ((image_aux_data*)image)->array_size;
    }
    // First convert integer image index
    int idx = floor(idxFloat + 0.5f);
    // clamp idx to edge
    if(idx < 0)
    {
        idx = 0;
    } else if(idx >= ((image_aux_data*)image)->array_size)
    {
        idx = ((image_aux_data*)image)->array_size - 1;
    }
    void* ptr = (void*)((image_aux_data*)image)->pData + ((image_aux_data*)image)->dim[0]*((image_aux_data*)image)->offset[0]*idx;
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
    void* ptr = (void*)((image_aux_data*)image)->pData + ((image_aux_data*)image)->dim[0]*((image_aux_data*)image)->dim[1]*((image_aux_data*)image)->offset[0]*idx;
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
    void* ptr = (void*)((image_aux_data*)image)->pData + ((image_aux_data*)image)->dim[0]*((image_aux_data*)image)->offset[0]*idx;
    return ptr;
}

#define SIMPLE_SAMPLER NONE_FALSE_NEAREST

/*********************************UNSIGNED IMAGE I/O FUNCTIONS (read_imageui)************************************************/

uint4  __attribute__((overloadable)) read_imageui(image2d_t image, sampler_t sampler, int2 coord)
{
    int4 coord4 = (int4)(0, 0, 0, 0);
    coord4.lo = coord;
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

uint4  __attribute__((overloadable)) read_imageui(image3d_t image, sampler_t sampler, int4 coord)
{
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord);
    return read_cbk((void*)image, trans_position, pData);
}

uint4  __attribute__((overloadable)) read_imageui(image2d_t image, sampler_t sampler, float2 coord)
{
    float4 coord4 = (float4)(0.f, 0.f ,0.f ,0.f);
    coord4.lo = coord;
    void* pData =((image_aux_data*)image)->pData;
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

uint4  __attribute__((overloadable)) read_imageui(image3d_t image, sampler_t sampler, float4 coord)
{
    coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    void* pData =((image_aux_data*)image)->pData;
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord);
    return read_cbk((void*)image, trans_position, pData);
}

void  __attribute__((overloadable)) write_imageui(image2d_t image, int2 coord, uint4 color)
{
    void* pixel = extract_pixel(image, coord);
    Image_UI_WRITE_CBK cbk = (Image_UI_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

/******************************************SIGNED INT I/O FUNCTIONS (read_imagei)*************************************************************/

int4  __attribute__((overloadable)) read_imagei(image2d_t image, sampler_t sampler, int2 coord)
{
    int4 coord4 = (int4)(0, 0, 0, 0);
    coord4.lo = coord;
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

int4  __attribute__((overloadable)) read_imagei(image3d_t image, sampler_t sampler, int4 coord)
{
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord);
    return read_cbk((void*)image, trans_position, pData);
}

int4  __attribute__((overloadable)) read_imagei(image2d_t image, sampler_t sampler, float2 coord)
{
    float4 coord4 = (float4)(0.f, 0.f, 0.f, 0.f);
    coord4.lo = coord;
    void* pData =((image_aux_data*)image)->pData;
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

int4  __attribute__((overloadable)) read_imagei(image3d_t image, sampler_t sampler, float4 coord)
{
    coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
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

/***********************************FLOAT IMAGE I/O FUNCTIONS (read_imagef)********************************************************/


float4  __attribute__((overloadable)) read_imagef(image2d_t image, sampler_t sampler, int2 coord)
{
    int4 coord4 = (int4)(0.f, 0.f, 0.f, 0.f);
    coord4.lo = coord;
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 dummy0;
    float4 dummy1;
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, dummy0, dummy1, pData);
}

float4  __attribute__((overloadable)) read_imagef(image3d_t image, sampler_t sampler, int4 coord)
{
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
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
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    ///// components for interpolation
    int4 square0, square1;
    float4 fraction= coord_cbk((void*)image, coord4, &square0, &square1);
    return read_cbk((void*)image, square0, square1, fraction, pData);
}

float4  __attribute__((overloadable)) read_imagef(image3d_t image, sampler_t sampler, float4 coord)
{
    coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    void* pData =((image_aux_data*)image)->pData;
    Image_FF_COORD_CBK coord_cbk=(Image_FF_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
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

/// 1_2 image functions
/// They are left empty to be able to add 1.2 images to recorder and BIMeter before implementation

float4  __attribute__((overloadable)) read_imagef(image2d_array_t image, sampler_t sampler, int4 coord)
{
    int4 internal_coord = coord;
    internal_coord.z = 0;
    int arrayIndex = coord.z;
    void* pData = GetImagePtr(image, arrayIndex);
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
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
    void* pData = GetImagePtr(image, sampler, coord.z);
    Image_FF_COORD_CBK coord_cbk=(Image_FF_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
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
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, internal_coord);
    int4 val = read_cbk((void*)image, trans_position, pData);
    return val;
}

int4  __attribute__((overloadable)) read_imagei(image2d_array_t image, sampler_t sampler, float4 coord)
{
    float4 internal_coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    internal_coord.z = 0.f;
    void* pData = GetImagePtr(image, sampler, coord.z);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
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
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, internal_coord);
    uint4 val = read_cbk((void*)image, trans_position, pData);
    return val;
}

uint4  __attribute__((overloadable)) read_imageui(image2d_array_t image, sampler_t sampler, float4 coord)
{
    float4 internal_coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    internal_coord.z = 0.f;
    void* pData = GetImagePtr(image, sampler, coord.z);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position = coord_cbk((void*)image, internal_coord);
    uint4 val = read_cbk((void*)image, trans_position, pData);
    return val;
}

float4 __attribute__((overloadable)) read_imagef(image1d_t image, sampler_t sampler, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
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
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 square0, square1;
    float4 fraction= coord_cbk((void*)image, coord4, &square0, &square1);
    return read_cbk((void*)image, square0, square1, fraction, pData);
}

int4 __attribute__((overloadable)) read_imagei(image1d_t image, sampler_t sampler, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

int4 __attribute__((overloadable)) read_imagei(image1d_t image, sampler_t sampler, float coord)
{
    float4 coord4=(float4)(coord, 0.0,0.0,0.0);
    void* pData =((image_aux_data*)image)->pData;
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

uint4 __attribute__((overloadable)) read_imageui(image1d_t image, sampler_t sampler, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_I_COORD_CBK coord_cbk= coord_translate_i_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

uint4 __attribute__((overloadable)) read_imageui(image1d_t image, sampler_t sampler, float coord)
{
    float4 coord4=(float4)(coord, 0.0,0.0,0.0);
    void* pData =((image_aux_data*)image)->pData;
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position, pData);
}

float4 __attribute__((overloadable)) read_imagef(image1d_array_t image, sampler_t sampler, int2 coord)
{   
    int4 internal_coord=(int4)(coord.x, 0,0,0);
    int arrayIndex = coord.y;
    void* pData = GetImagePtr(image, arrayIndex);
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, internal_coord);
    int4 dummy0;
    float4 dummy1;
    float4 val = read_cbk((void*)image, trans_position, dummy0, dummy1, pData);
    return val;

}

float4 __attribute__((overloadable)) read_imagef(image1d_array_t image, sampler_t sampler, float2 coord)
{
    float4 internal_coord = (float4)(coord.x, 0.0,0.0,0.0);
    void* pData = GetImagePtr(image, sampler, coord.y);
    Image_FF_COORD_CBK coord_cbk=(Image_FF_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
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
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, internal_coord);
    int4 val = read_cbk((void*)image, trans_position, pData);
    return val;
}

int4 __attribute__((overloadable)) read_imagei(image1d_array_t image, sampler_t sampler, float2 coord)
{
    float4 internal_coord = (float4)(coord.x, 0.0,0.0,0.0);
    void* pData = GetImagePtr(image, sampler, coord.y);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position = coord_cbk((void*)image, internal_coord);
    int4 val = read_cbk((void*)image, trans_position, pData);
    return val;
}

uint4 __attribute__((overloadable)) read_imageui(image1d_array_t image, sampler_t sampler, int2 coord)
{
    int4 internal_coord = (int4)(coord.x, 0,0,0);
    void* pData = GetImagePtr(image, coord.y);
    Image_I_COORD_CBK coord_cbk = coord_translate_i_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, internal_coord);
    uint4 val = read_cbk((void*)image, trans_position, pData);
    return val;
}

uint4 __attribute__((overloadable)) read_imageui(image1d_array_t image, sampler_t sampler, float2 coord)
{
    float4 internal_coord = (float4)(coord.x, 0.0,0.0,0.0);
    void* pData = GetImagePtr(image, sampler, coord.y);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
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
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    int4 dummy0;
    float dummy1;
    return read_cbk((void*)image, coord4, dummy0, dummy1, pData);
}

int4 __attribute__((overloadable)) read_imagei (image2d_t image, int2 coord)
{
    int4 coord4 = (int4)(0, 0, 0, 0);
    coord4.lo = coord;
    void* pData =((image_aux_data*)image)->pData;
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord4, pData);
}

uint4 __attribute__((overloadable)) read_imageui (image2d_t image, int2 coord)
{
    int4 coord4 = (int4)(0, 0, 0, 0);
    coord4.lo = coord;    
    void* pData =((image_aux_data*)image)->pData;
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord4, pData);
}

float4 __attribute__((overloadable)) read_imagef (image3d_t image, int4 coord)
{
    void* pData =((image_aux_data*)image)->pData;
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    int4 dummy0;
    float4 dummy1;
    return read_cbk((void*)image, coord, dummy0, dummy1, pData);
}

int4 __attribute__((overloadable)) read_imagei (image3d_t image, int4 coord)
{
    void* pData =((image_aux_data*)image)->pData;
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord, pData);

}

uint4 __attribute__((overloadable)) read_imageui (image3d_t image, int4 coord)
{
    void* pData =((image_aux_data*)image)->pData;
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord, pData);
}

float4 __attribute__((overloadable)) read_imagef (image2d_array_t image, int4 coord)
{
    // naive read_imagef implementation
    int4 internal_coord = coord;
    internal_coord.z = 0;
    void* pData = GetImagePtr(image, coord.z);
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    int4 dummy0; 
    float dummy1;
    return read_cbk((void*)image, internal_coord, dummy0, dummy1, pData);
}

int4 __attribute__((overloadable)) read_imagei (image2d_array_t image, int4 coord)
{
    int4 internal_coord = coord;
    internal_coord.z = 0;
    void* pData = GetImagePtr(image, coord.z);
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    return read_cbk((void*)image, internal_coord, pData);
}

uint4 __attribute__((overloadable)) read_imageui (image2d_array_t image, int4 coord)
{
    int4 internal_coord = coord;
    internal_coord.z = 0;
    void* pData = GetImagePtr(image, coord.z);
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    return read_cbk((void*)image, internal_coord, pData);
}

float4 __attribute__((overloadable)) read_imagef (image1d_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    int4 dummy0;
    float4 dummy1;
    return read_cbk((void*)image, coord4, dummy0, dummy1, pData);
}

float4 __attribute__((overloadable)) read_imagef (image1d_buffer_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    int4 dummy0;
    float dummy1;
    return read_cbk((void*)image, coord4, dummy0, dummy1, pData);
}

int4 __attribute__((overloadable)) read_imagei(image1d_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord4, pData);
}

uint4 __attribute__((overloadable)) read_imageui(image1d_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord4, pData);
}

int4 __attribute__((overloadable)) read_imagei(image1d_buffer_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord4, pData);
}

uint4 __attribute__((overloadable)) read_imageui(image1d_buffer_t image, int coord)
{
    int4 coord4=(int4)(coord, 0,0,0);
    void* pData =((image_aux_data*)image)->pData;
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    return read_cbk((void*)image, coord4, pData);
}

float4 __attribute__((overloadable)) read_imagef(image1d_array_t image, int2 coord)
{
    int4 internal_coord=(int4)(coord.x, 0,0,0);
    void* pData = GetImagePtr(image, coord.y);
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    int4 dummy0;
    float dummy1;
    return read_cbk((void*)image, internal_coord, dummy0, dummy1, pData);
}

int4 __attribute__((overloadable)) read_imagei(image1d_array_t image, int2 coord)
{
    int4 internal_coord=(int4)(coord.x, 0,0,0);
    void* pData = GetImagePtr(image, coord.y);
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
    return read_cbk((void*)image, internal_coord, pData);
}

uint4 __attribute__((overloadable)) read_imageui(image1d_array_t image, int2 coord)
{
	int4 internal_coord=(int4)(coord.x, 0,0,0);
    void* pData = GetImagePtr(image, coord.y);
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[SIMPLE_SAMPLER];
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

#ifdef __cplusplus
}
#endif

#endif // defined (__MIC__) || defined(__MIC2__)
