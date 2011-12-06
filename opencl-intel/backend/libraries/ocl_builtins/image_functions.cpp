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
#include <limits.h>
#include <math.h>
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#else
#include <intrin.h>
#endif

#include "cl_image_declaration.h"

#if !defined(_MSC_VER) && !defined(__INTEL_COMPILER)

#define ALIGN16 __attribute__ ((aligned(16)))

const int fVec4FloatZeroCoordMask3D[4] ALIGN16 = {0xffffffff, 0xffffffff, 0xffffffff, 0};

int __attribute__((overloadable)) __attribute__((const)) get_image_width(image2d_t img)
{
	return ((image_aux_data*)img)->dim[0];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_width(image3d_t img)
{
	return ((image_aux_data*)img)->dim[0];
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

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_data_type(image2d_t img)
{
	return ((image_aux_data*)img)->format.image_channel_data_type;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_data_type(image3d_t img)
{
	return ((image_aux_data*)img)->format.image_channel_data_type;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_order(image2d_t img)
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

_4i32 __attribute__((overloadable)) __attribute__((const)) get_image_dim(image3d_t img)
{
	__m128i dim = _mm_lddqu_si128((__m128i*)((image_aux_data*)img)->dim);
	// Set to 0 the highest DWORD
	dim = _mm_srli_si128(_mm_slli_si128(dim, 4),4);

	return (_4i32)dim;
}

void* extract_pixel_2D(image2d_t image, int2 coord)
{
    // Calculate required pixel offset
#ifdef __SSE4_1__
    int4 offset = _mm_load_si128((__m128i*)(&((image_aux_data*)image)->offset));
#else
    int4 offset=(int4)(0,0,0,0);
#endif
    void* pixel = (void*)((image_aux_data*)image)->pData+coord.x * offset.x + coord.y * offset.y;
    return pixel;
}

/*********************************UNSIGNED IMAGE I/O FUNCTIONS (read_imageui)************************************************/

uint4  __attribute__((overloadable)) read_imageui(image2d_t image, sampler_t sampler, int2 coord)
{
#ifdef __SSE4_1__
    int4 coord4=_mm_loadl_epi64((__m128i*)&coord);
#else
    int4 coord4=(int4)(coord.x, coord.y, 0,0);
#endif
    Image_I_COORD_CBK coord_cbk=(Image_I_COORD_CBK)((image_aux_data*)image)->coord_translate_i_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position);
}

uint4  __attribute__((overloadable)) read_imageui(image3d_t image, sampler_t sampler, int4 coord)
{
    Image_I_COORD_CBK coord_cbk=(Image_I_COORD_CBK)((image_aux_data*)image)->coord_translate_i_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord);
    return read_cbk((void*)image, trans_position);
}

uint4  __attribute__((overloadable)) read_imageui(image2d_t image, sampler_t sampler, float2 coord)
{
#ifdef __SSE4_1__
    __m128 coord4 = _mm_castsi128_ps(_mm_loadl_epi64((__m128i *)&coord));
#else
    float4 coord4=(float4)(coord.x, coord.y, 0.0,0.0);
#endif
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position);
}

uint4  __attribute__((overloadable)) read_imageui(image3d_t image, sampler_t sampler, float4 coord)
{
    coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_UI_READ_CBK read_cbk = (Image_UI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord);
    return read_cbk((void*)image, trans_position);
}

void  __attribute__((overloadable)) write_imageui(image2d_t image, int2 coord, uint4 color)
{
    void* pixel = extract_pixel_2D(image, coord);
    Image_UI_WRITE_CBK cbk = (Image_UI_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

/******************************************SIGNED INT I/O FUNCTIONS (read_imagei)*************************************************************/

int4  __attribute__((overloadable)) read_imagei(image2d_t image, sampler_t sampler, int2 coord)
{
#ifdef __SSE4_1__
    int4 coord4=_mm_loadl_epi64((__m128i*)&coord);
#else
    int4 coord4=(int4)(coord.x, coord.y, 0,0);
#endif
    Image_I_COORD_CBK coord_cbk=(Image_I_COORD_CBK)((image_aux_data*)image)->coord_translate_i_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position);
}

int4  __attribute__((overloadable)) read_imagei(image3d_t image, sampler_t sampler, int4 coord)
{
    Image_I_COORD_CBK coord_cbk=(Image_I_COORD_CBK)((image_aux_data*)image)->coord_translate_i_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord);
    return read_cbk((void*)image, trans_position);
}

int4  __attribute__((overloadable)) read_imagei(image2d_t image, sampler_t sampler, float2 coord)
{
#ifdef __SSE4_1__
    __m128 coord4 = _mm_castsi128_ps(_mm_loadl_epi64((__m128i *)&coord));
#else
    float4 coord4=(float4)(coord.x, coord.y, 0.0,0.0);
#endif
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position);
}

int4  __attribute__((overloadable)) read_imagei(image3d_t image, sampler_t sampler, float4 coord)
{
    coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    Image_F_COORD_CBK coord_cbk=(Image_F_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_I_READ_CBK read_cbk = (Image_I_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord);
    return read_cbk((void*)image, trans_position);
}

void  __attribute__((overloadable)) write_imagei(image2d_t image, int2 coord, int4 color)
{
    void* pixel = extract_pixel_2D(image, coord);
    Image_I_WRITE_CBK cbk = (Image_I_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}

/***********************************FLOAT IMAGE I/O FUNCTIONS (read_imagef)********************************************************/


float4  __attribute__((overloadable)) read_imagef(image2d_t image, sampler_t sampler, int2 coord)
{
#ifdef __SSE4_1__
    int4 coord4=_mm_loadl_epi64((__m128i*)&coord);
#else
    int4 coord4=(int4)(coord.x, coord.y, 0,0);
#endif
    Image_I_COORD_CBK coord_cbk=(Image_I_COORD_CBK)((image_aux_data*)image)->coord_translate_i_callback[sampler];
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord4);
    return read_cbk((void*)image, trans_position);
}

float4  __attribute__((overloadable)) read_imagef(image3d_t image, sampler_t sampler, int4 coord)
{
    Image_I_COORD_CBK coord_cbk=(Image_I_COORD_CBK)((image_aux_data*)image)->coord_translate_i_callback[sampler];
    Image_FI_READ_CBK read_cbk = (Image_FI_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 trans_position=coord_cbk((void*)image, coord);
    return read_cbk((void*)image, trans_position);
}

float4  __attribute__((overloadable)) read_imagef(image2d_t image, sampler_t sampler, float2 coord)
{
#ifdef __SSE4_1__
    __m128 coord4 = _mm_castsi128_ps(_mm_loadl_epi64((__m128i *)&coord));
#else
    float4 coord4=(float4)(coord.x, coord.y, 0.0,0.0);
#endif
    Image_FF_COORD_CBK coord_cbk=(Image_FF_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    ///// components for interpolation
    int4 square0, square1;
    float4 fraction= coord_cbk((void*)image, coord4, &square0, &square1);
    return read_cbk((void*)image, square0, square1, fraction);
}

float4  __attribute__((overloadable)) read_imagef(image3d_t image, sampler_t sampler, float4 coord)
{
    coord = _mm_and_ps(coord, *(__m128*)fVec4FloatZeroCoordMask3D);
    Image_FF_COORD_CBK coord_cbk=(Image_FF_COORD_CBK)((image_aux_data*)image)->coord_translate_f_callback[sampler];
    Image_F_READ_CBK read_cbk = (Image_F_READ_CBK)((image_aux_data*)image)->read_img_callback[sampler];
    int4 square0, square1;
    float4 fraction=coord_cbk((void*)image, coord, &square0, &square1);
    return read_cbk((void*)image, square0, square1, fraction);
}

void  __attribute__((overloadable)) write_imagef(image2d_t image, int2 coord, float4 color)
{
    void* pixel = extract_pixel_2D(image, coord);
    Image_F_WRITE_CBK cbk = (Image_F_WRITE_CBK)((image_aux_data*)image)->write_img_callback;
    cbk(pixel, color);
}


#endif

#ifdef __cplusplus
}
#endif
