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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#include "cl_types.h"
#if defined(_MSC_VER)
#define IMG_FUNC_EXPORT __declspec(dllexport)
#else
#define IMG_FUNC_EXPORT
#endif
#else
#include "cl_types2.h"

#define IMG_FUNC_EXPORT
#define MAX_WORK_DIM 3

// Explicitly define image types
typedef struct _cl_mem_obj_descriptor
{
	uint			dim_count;				// A number of dimensions in the memory object.
	union _dim_t
	{
		unsigned int	dim[MAX_WORK_DIM];		// Multi-dimensional size of the object.
		size_t			buffer_size;
	} dimensions;
	size_t			pitch[MAX_WORK_DIM-1];	// Multi-dimensional pitch of the object, valid only for images (2D/3D).
	cl_image_format_t	format;					// Format of the memory object,valid only for images (2D/3D).
	void*			pData;					// A pointer to the object wherein the object data is stored.
											// Could be a valid memory pointer or a handle to other object.
	unsigned		uiElementSize;			// Size of image pixel element.
} cl_mem_obj_descriptor;
#endif

// Other functions are built into DLL
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)

// Workaround of LLVM bug: vector-2 is passed unpacked, hence clang generates double type instead
#if defined(_M_X64) || defined(__LP64__)
typedef double intVecOf2;
typedef double floatVecOf2;
#else
typedef _2i32  intVecOf2;
typedef float2 floatVecOf2;
#endif

#ifdef _M_X64
#pragma linkage _lnk_read_img_2d_i_ = ( result (xmm0) parameters (rcx edx xmm2) preserved (xmm6 xmm7 xmm8 xmm9 xmm10 xmm11 xmm12 xmm13 xmm14 xmm15))
#pragma linkage _lnk_read_img_2d_f_ = ( result (xmm0) parameters (rcx edx xmm2) preserved (xmm6 xmm7 xmm8 xmm9 xmm10 xmm11 xmm12 xmm13 xmm14 xmm15))
#elif __LP64__
#pragma linkage _lnk_read_img_2d_i_ = ( result (xmm0) parameters (rdi esi xmm0) preserved (xmm6 xmm7 xmm8 xmm9 xmm10 xmm11 xmm12 xmm13 xmm14 xmm15))
#pragma linkage _lnk_read_img_2d_f_ = ( result (xmm0) parameters (rdi esi xmm0) preserved (xmm6 xmm7 xmm8 xmm9 xmm10 xmm11 xmm12 xmm13 xmm14 xmm15))
#else
#pragma linkage _lnk_read_img_2d_i_ = ( result (xmm0) parameters (memory memory xmm0) )
#pragma linkage _lnk_read_img_2d_f_ = ( result (xmm0) parameters (memory memory xmm0) )
#endif
IMG_FUNC_EXPORT float4 _Z11read_imagefP10_image2d_tjDv2_i(image2d_t image, cl_dev_sampler_prop sampler, intVecOf2 coord);
#pragma use_linkage _lnk_read_img_2d_i_ ( _Z11read_imagefP10_image2d_tjDv2_i )
IMG_FUNC_EXPORT float4 _Z11read_imagefP10_image2d_tjDv2_f(image2d_t image, cl_dev_sampler_prop sampler, floatVecOf2 coord);
#pragma use_linkage _lnk_read_img_2d_f_ ( _Z11read_imagefP10_image2d_tjDv2_f )
IMG_FUNC_EXPORT _4i32  _Z11read_imageiP10_image2d_tjDv2_i(image2d_t image, cl_dev_sampler_prop sampler, intVecOf2 coord);
#pragma use_linkage _lnk_read_img_2d_i_ ( _Z11read_imageiP10_image2d_tjDv2_i )
IMG_FUNC_EXPORT _4i32  _Z11read_imageiP10_image2d_tjDv2_f(image2d_t image, cl_dev_sampler_prop sampler, floatVecOf2 coord);
#pragma use_linkage _lnk_read_img_2d_f_ ( _Z11read_imageiP10_image2d_tjDv2_f )
IMG_FUNC_EXPORT _4u32  _Z12read_imageuiP10_image2d_tjDv2_i(image2d_t image, cl_dev_sampler_prop sampler, intVecOf2 coord);
#pragma use_linkage _lnk_read_img_2d_i_ ( _Z12read_imageuiP10_image2d_tjDv2_i )
IMG_FUNC_EXPORT _4u32  _Z12read_imageuiP10_image2d_tjDv2_f(image2d_t image, cl_dev_sampler_prop sampler, floatVecOf2 coord);
#pragma use_linkage _lnk_read_img_2d_f_ ( _Z12read_imageuiP10_image2d_tjDv2_f )

//#pragma linkage _lnk_read_img_3d_ = ( result (xmm0) parameters (memory memory xmm0) )
IMG_FUNC_EXPORT float4 _Z11read_imagefP10_image3d_tjDv4_i(image3d_t image, cl_dev_sampler_prop sampler, _4i32 coord);
//#pragma use_linkage _lnk_read_img_3d_ ( _Z11read_imagefP10_image3d_tjDv4_i )
IMG_FUNC_EXPORT float4 _Z11read_imagefP10_image3d_tjDv4_f(image3d_t image, cl_dev_sampler_prop sampler, float4 coord);
//#pragma use_linkage _lnk_read_img_3d_ ( _Z11read_imagefP10_image3d_tjDv4_f )
IMG_FUNC_EXPORT _4i32  _Z11read_imageiP10_image3d_tjDv4_i(image3d_t image, cl_dev_sampler_prop sampler, _4i32 coord);
//#pragma use_linkage _lnk_read_img_3d_ ( _Z11read_imageiP10_image3d_tjDv4_i )
IMG_FUNC_EXPORT _4i32  _Z11read_imageiP10_image3d_tjDv4_f(image3d_t image, cl_dev_sampler_prop sampler, float4 coord);
//#pragma use_linkage _lnk_read_img_3d_ ( _Z11read_imageiP10_image3d_tjDv4_f )
IMG_FUNC_EXPORT _4u32  _Z12read_imageuiP10_image3d_tjDv4_i(image3d_t image, cl_dev_sampler_prop sampler, _4i32 coord);
//#pragma use_linkage _lnk_read_img_3d_ ( _Z12read_imageuiP10_image3d_tjDv4_i )
IMG_FUNC_EXPORT _4u32  _Z12read_imageuiP10_image3d_tjDv4_f(image3d_t image, cl_dev_sampler_prop sampler, float4 coord);
//#pragma use_linkage _lnk_read_img_3d_ ( _Z12read_imageuiP10_image3d_tjDv4_f )

#ifdef _M_X64
#pragma linkage _lnk_write_img_2d_ = ( parameters (rcx xmm1 r8 ) )
IMG_FUNC_EXPORT void _Z12write_imagefP10_image2d_tDv2_iDv4_f(image2d_t image, intVecOf2 coord, float4 &val);
IMG_FUNC_EXPORT void _Z12write_imageiP10_image2d_tDv2_iDv4_i(image2d_t image, intVecOf2 coord, _4i32 &val);
IMG_FUNC_EXPORT void _Z13write_imageuiP10_image2d_tDv2_iDv4_j(image2d_t image, intVecOf2 coord, _4u32 &val);
#elif __LP64__
#pragma linkage _lnk_write_img_2d_ = ( parameters (rdi xmm0 xmm1 ) ) 
IMG_FUNC_EXPORT void _Z12write_imagefP10_image2d_tDv2_iDv4_f(image2d_t image, intVecOf2 coord, float4 val);
IMG_FUNC_EXPORT void _Z12write_imageiP10_image2d_tDv2_iDv4_i(image2d_t image, intVecOf2 coord, _4i32 val); 
IMG_FUNC_EXPORT void _Z13write_imageuiP10_image2d_tDv2_iDv4_j(image2d_t image, intVecOf2 coord, _4u32 val);
#else
#pragma linkage _lnk_write_img_2d_ = ( parameters (memory xmm0 xmm1 ) )
IMG_FUNC_EXPORT void _Z12write_imagefP10_image2d_tDv2_iDv4_f(image2d_t image, intVecOf2 coord, float4 val);
IMG_FUNC_EXPORT void _Z12write_imageiP10_image2d_tDv2_iDv4_i(image2d_t image, intVecOf2 coord, _4i32 val);
IMG_FUNC_EXPORT void _Z13write_imageuiP10_image2d_tDv2_iDv4_j(image2d_t image, intVecOf2 coord, _4u32 val);
#endif

#pragma use_linkage _lnk_write_img_2d_ ( _Z13write_imageuiP10_image2d_tDv2_iDv4_j )
#pragma use_linkage _lnk_write_img_2d_ ( _Z12write_imageiP10_image2d_tDv2_iDv4_i )
#pragma use_linkage _lnk_write_img_2d_ ( _Z12write_imagefP10_image2d_tDv2_iDv4_f )

// Internal functions
__forceinline __m128 AddressRepeatCorrectNorm(__m128 coor);
__forceinline __m128i AddressRepeatCorrectUnorm(__m128i coor, __m128i dim);
__forceinline __m128 AddressMirrorRepeatCorrectNorm(__m128 coor, __m128 dim);
__forceinline __m128i AddressMirrorRepeatCorrectUnorm(__m128i coor, __m128i dim);

/*
#pragma linkage _lnk_addr_rep_norm_ = ( result(xmm0) parameters (xmm0) )
#pragma use_linkage _lnk_addr_rep_norm_ ( AddressRepeatCorrectNorm )
#pragma linkage _lnk_addr_rep_unorm_ = ( result(xmm0) parameters (xmm0 xmm1) )
#pragma use_linkage _lnk_addr_rep_unorm_ ( AddressRepeatCorrectUnorm )
*/
__forceinline bool ApplyAddressModeI(__m128i& coor, cl_mem_obj_descriptor* image, cl_dev_sampler_prop addrMode);

// Apply linear filter when loading image, alway return float
__forceinline float4 read_2d_linear_f(image2d_t image, cl_dev_sampler_prop sampler, float4 f4coor);
__forceinline _4i32 read_2d_linear_i(image2d_t image, cl_dev_sampler_prop sampler, float4 f4coor);
__forceinline float4 read_3d_linear_f(image3d_t image, cl_dev_sampler_prop sampler, float4 f4coor);
__forceinline _4i32 read_3d_linear_i(image3d_t image, cl_dev_sampler_prop sampler, float4 f4coor);
/*
#pragma linkage _lnk_read_ = ( result(xmm0) parameters (memory memory xmm0) )
#pragma use_linkage _lnk_read_ ( read_2d_linear_f )
#pragma use_linkage _lnk_read_ ( read_2d_linear_i )
#pragma use_linkage _lnk_read_ ( read_3d_linear_f )
#pragma use_linkage _lnk_read_ ( read_3d_linear_i )
*/
// Retrive a pointer to specific pixel
__forceinline void* ExtractPixel2D(__m128i coor, image2d_t image);
__forceinline void* ExtractPixel3D(__m128i coor, image2d_t image);

// Load pixel depend on its format, always as integer
// Convertion to float will be performed when required
__forceinline _4i32 LoadPixel(void* pPxl, cl_image_format fmt);
__forceinline _4i32 LoadSingleChannel(void *pPxl, cl_channel_type type);
__forceinline _4i32 LoadDualChannel(void *pPxl, cl_channel_type type);
__forceinline _4i32 LoadTripleChannel(void *pPxl, cl_channel_type type);
__forceinline _4i32 LoadQuadChannel(void *pPxl, cl_channel_type type);

// Store pixel depend on its format, always as integer
// Convertion to float will be performed when required
__forceinline void StorePixel(void* pPxl, _4i32 i4Val, cl_image_format fmt);
__forceinline void StoreSingleChannel(void *pPxl, _4i32 i4Val, cl_channel_type type);
__forceinline void StoreDualChannel(void *pPxl, _4i32 i4Val, cl_channel_type type);
__forceinline void StoreTripleChannel(void *pPxl, _4i32 i4Val, cl_channel_type type);
__forceinline void StoreQuadChannel(void *pPxl, _4i32 i4Val, cl_channel_type type);

// Retruns border color as integer/float
__forceinline _4i32 BoorderColorI(cl_image_format fmt);
__forceinline float4 BoorderColorF(cl_image_format fmt);

// SVML used functions
#ifndef __SSE4_1__
float4 __svml_floorf4(float4);
float4 __svml_roundf4(float4, int mode);
#endif

#endif // _MSC_DEV

#ifdef __cplusplus
}
#endif
