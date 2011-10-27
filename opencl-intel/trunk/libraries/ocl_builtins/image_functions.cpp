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
int __attribute__((overloadable)) __attribute__((const)) get_image_width(image2d_t img)
{
	return ((cl_mem_obj_descriptor*)img)->dimensions.dim[0];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_width(image3d_t img)
{
	return ((cl_mem_obj_descriptor*)img)->dimensions.dim[0];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_height(image2d_t img)
{
	return ((cl_mem_obj_descriptor*)img)->dimensions.dim[1];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_height(image3d_t img)
{
	return ((cl_mem_obj_descriptor*)img)->dimensions.dim[1];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_depth(image3d_t img)
{
	return ((cl_mem_obj_descriptor*)img)->dimensions.dim[2];
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_data_type(image2d_t img)
{
	return ((cl_mem_obj_descriptor*)img)->format.image_channel_data_type;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_data_type(image3d_t img)
{
	return ((cl_mem_obj_descriptor*)img)->format.image_channel_data_type;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_order(image2d_t img)
{
	return ((cl_mem_obj_descriptor*)img)->format.image_channel_order;
}

int __attribute__((overloadable)) __attribute__((const)) get_image_channel_order(image3d_t img)
{
	return ((cl_mem_obj_descriptor*)img)->format.image_channel_order;
}

_2i32 __attribute__((overloadable)) __attribute__((const)) get_image_dim(image2d_t img)
{
	_2i32 res;

	res.lo = ((cl_mem_obj_descriptor*)img)->dimensions.dim[0];
	res.hi = ((cl_mem_obj_descriptor*)img)->dimensions.dim[1];
	return res;
}

_4i32 __attribute__((overloadable)) __attribute__((const)) get_image_dim(image3d_t img)
{
	__m128i dim = _mm_lddqu_si128((__m128i*)((cl_mem_obj_descriptor*)img)->dimensions.dim);
	// Set to 0 the highest DWORD
	dim = _mm_srli_si128(_mm_slli_si128(dim, 4),4);

	return (_4i32)dim;
}
#endif

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
// convert 4 halfs to 4 floats
//extern __m128 Half4ToFloat4(__m128i);
#define ALIGN16 __declspec(align(16))

ALIGN16 short  Fvec8Float16ExponentMask[] = {0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00, 0x7C00};
ALIGN16 short  Fvec8Float16MantissaMask[] = {0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF, 0x03FF};
ALIGN16 short  Fvec8Float16SignMask[]     = {0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000};
ALIGN16 int Fvec4Float32ExponentMask[] = {0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000};
ALIGN16 int Fvec4Float32NanMask[] = {0x7FC00000, 0x7FC00000, 0x7FC00000, 0x7FC00000};
ALIGN16 int Fvec4Float16NaNExpMask[]   = {0x7C00, 0x7C00, 0x7C00, 0x7C00};
ALIGN16 int FVec4Float16Implicit1Mask[] = {(1<<10), (1<<10), (1<<10), (1<<10)};
ALIGN16 int Fvec4Float16ExpMin[] = {(1<<10), (1<<10), (1<<10), (1<<10)};
ALIGN16 int Fvec4Float16BiasDiffDenorm[] = {((127 - 15 - 10) << 23), ((127 - 15 - 10) << 23), ((127 - 15 - 10) << 23), ((127 - 15 - 10) << 23)};
ALIGN16 int Fvec4Float16ExpBiasDifference[] = {((127 - 15) << 10), ((127 - 15) << 10), ((127 - 15) << 10), ((127 - 15) << 10)};

float4 Half4ToFloat4(_8i16 xmm0)
{

	_4i32 xmm7 = (_4i32)_mm_setzero_si128();
	_4i32 xmm1 = (_4i32)_mm_and_si128((__m128i)xmm0,(__m128i) *((_4i32*)Fvec8Float16ExponentMask));  
	xmm1 = (_4i32) _mm_unpacklo_epi16((__m128i)xmm1, (__m128i)xmm7); // xmm1 = exponents as DWORDS
    _4i32 xmm2 = (_4i32)_mm_and_si128((__m128i)xmm0,(__m128i) *((_4i32*)Fvec8Float16MantissaMask));
	xmm2 = (_4i32) _mm_unpacklo_epi16((__m128i)xmm2, (__m128i)xmm7); // xmm2 = mantissas as DWORDS
	xmm0 = (_8i16) _mm_and_si128((__m128i)xmm0,(__m128i) *((_4i32 * )Fvec8Float16SignMask)); 
	_4i32 xmm6 = (_4i32)_mm_unpacklo_epi16((__m128i)xmm7, (__m128i)xmm0); // xmm6 = sign mask as DWORDS

	// We need to handle the case where the number is NaN or INF
	// If the float16 is one of these, then we create an all '1' exponent for the 32bit float and store it in xmm6 for later use
	xmm0 = (_8i16) _mm_cmpeq_epi32((__m128i)xmm1,(__m128i) *((_4i32 *)Fvec4Float16NaNExpMask)); // xmm0.any dword = 0xFFFFFFFF if exponent is all '1'
	_4i32 xmm4 = (_4i32)_mm_cmpgt_epi32((__m128i)xmm2, (__m128i)xmm7); // xmm4.any dword = 0xFFFFFFFF if mantissa > 0
	xmm4 = (_4i32) _mm_and_si128((__m128i)xmm4,(__m128i) xmm0); // xmm4.any dword = 0xFFFFFFFF if NAN
	xmm4 = (_4i32) _mm_and_si128((__m128i)xmm4,(__m128i) *((_4i32 * )Fvec4Float32NanMask)); // silence the SNaNs
	xmm0 = (_8i16) _mm_and_si128((__m128i)xmm0, (__m128i)*((_4i32 * )Fvec4Float32ExponentMask)); // // xmm0 = If float16 has all '1' exp, then convert to 32 bit float all '1' exp, otherwise 0
	xmm0 = (_8i16) _mm_or_si128((__m128i)xmm0,(__m128i) xmm4); 

	_4i32 xmm3 = (_4i32)_mm_cmpeq_epi32((__m128i)xmm1, (__m128i)xmm7); // xmm3.any dword = 0xFFFFFFFF if exp is zero
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

//////////////////////////////////////////////////////////////////////////////////////////////////
// Constants used for normalized data conversion
__m128 f4snorm8mul = _mm_set1_ps(127.f);
__m128 f4snorm8div = _mm_set1_ps((float)(1./127.));
__m128 f4snorm8lim = _mm_set1_ps(-128.f);
__m128 f4unorm8mul = _mm_set1_ps(255.f);
__m128 f4unorm8div = _mm_set1_ps((float)(1./255.));
__m128 f4unorm8lim = _mm_setzero_ps();
__m128 f4snorm16mul = _mm_set1_ps(32767.f);
__m128 f4snorm16div = _mm_set1_ps((float)(1./32767.));
__m128 f4snorm16lim = _mm_set1_ps(-32768.f);
__m128 f4unorm16mul = _mm_set1_ps(65535.f);
__m128 f4unorm16div = _mm_set1_ps((float)(1./65535.));
__m128 f4unorm16lim = _mm_setzero_ps();
__m128 f4unorm565mul = _mm_set_ps(1.f, 31.f, 63.f, 31.f);
__m128 f4unorm565div = _mm_set_ps(1.f, (float)(1./31.), (float)(1./63.), (float)(1./31.));
__m128 f4unorm565lim = _mm_set_ps(1.f, 0.f, 0.f, 0.f);
__m128 f4unorm555mul = _mm_set_ps(1.f, 31.f, 31.f, 31.f);
__m128 f4unorm555div = _mm_set_ps(1.f, (float)(1./31.), (float)(1./31.), (float)(1./31.));
__m128 f4unorm555lim = _mm_set_ps(1.f, 0.f, 0.f, 0.f);
__m128 f4unorm101010mul = _mm_set_ps(1.f, 1023.f, 1023.f, 1023.f);
__m128 f4unorm101010div = _mm_set_ps(1.f, (float)(1./1023.), (float)(1./1023.), (float)(1./1023.));
__m128 f4unorm101010lim = _mm_set_ps(1.f, 0.f, 0.f, 0.f);

__m128 f4minusone = _mm_set1_ps(-1.f);
__m128 f4half = _mm_set1_ps(0.5);
__m128i i4minusone = _mm_set1_epi32(-1);

// Constant used for half->float conversion
__m128i i4hfSignMask = _mm_set1_epi32(0x8000);
__m128i i4hfExpMask = _mm_set1_epi32(0x7c00);

__m128i i4FloatBorder = _mm_slli_si128(_mm_castps_si128(_mm_set_ss(1.0f)), 12);
__m128i i4IntBorder = _mm_slli_si128(_mm_cvtsi32_si128(1), 12);

// Constant used for REPEAT address mode
__m128 f4one = _mm_set1_ps(1.f);

// Constant used for MIRROR REPEAT address mode
__m128 f4two = _mm_set1_ps(2.f);

// Constant used for UINT16/UINT8 cliping
__m128i    i4uint8Max = _mm_set1_epi32(UCHAR_MAX);
__m128i    i4uint16Max = _mm_set1_epi32(USHRT_MAX);
// Constant used for INT16/INT8 cliping
__m128i    i4int8Max = _mm_set1_epi32(CHAR_MAX);
__m128i    i4int16Max = _mm_set1_epi32(SHRT_MAX);
__m128i    i4int8Min = _mm_set1_epi32(CHAR_MIN);
__m128i    i4int16Min = _mm_set1_epi32(SHRT_MIN);
// Constant used for packed data storage( for pshub)
__m128i    i4i8mask = _mm_setr_epi8(0, 4, 8, 12, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
__m128i    i4i16mask = _mm_setr_epi8(0, 1, 4, 5, 8, 9, 12, 13, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

__m128 f4SignMask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));
#define _mm_abs_ps(X)	_mm_andnot_ps(f4SignMask,X)

float4 _Z11read_imagefP10_image2d_tjDv2_i(image2d_t image, cl_dev_sampler_prop sampler, intVecOf2 coord)     
{
    //TODO check that XMM==>mem==>XMM transition doesn't happen in release mode (i.e. the intrinsic below is dropped)
	_4i32 i4coor = _mm_loadl_epi64((__m128i *)&coord);
	if ( ApplyAddressModeI(i4coor, image, (cl_dev_sampler_prop)(sampler & __ADDRESS_MASK)) )
	{
		return _mm_castsi128_ps(BoorderColorI(image->format));
	}

	// We are not on borders, sample the image
	void* pPxl = ExtractPixel2D(i4coor, image);
	return _mm_castsi128_ps(LoadPixel(pPxl, image->format));
}

float4 _Z11read_imagefP10_image2d_tjDv2_f(image2d_t image, cl_dev_sampler_prop sampler, floatVecOf2 coord)
{
//	if ( fabs(coord.a-1.000000) < 0.001 && fabs(coord.b-0.031469) < 0.001)
//	{
//		int i = 0;
//	}
	int iAddrMode = (sampler & __ADDRESS_MASK);
	__m128 f4coor = _mm_castsi128_ps(_mm_loadl_epi64((__m128i *)&coord));

	__m128i i4dim = _mm_loadl_epi64((__m128i*)image->dimensions.dim);
	__m128	f4dim = _mm_cvtepi32_ps(i4dim);
	// Adjust repeat on normalized coordinates
	if ( iAddrMode == CL_DEV_SAMPLER_ADDRESS_REPEAT)
	{
		f4coor = AddressRepeatCorrectNorm(f4coor);
		// for repeat ALWAYS NORMALIZED
		sampler = (cl_dev_sampler_prop)(sampler | CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE);
	}
	// Adjust repeat on normalized coordinates
	if ( iAddrMode == CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT)
	{
		f4coor = AddressMirrorRepeatCorrectNorm(f4coor, f4dim);
		// for repeat ALWAYS NORMALIZED
		sampler = (cl_dev_sampler_prop)(sampler & (~CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE));
	}
	// Unnormalized coordinates
	if ( (sampler & __NORMALIZED_MASK) ==  CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE)
	{
		// Now are UNORMILIZED, ready to work
		f4coor = _mm_mul_ps(f4coor, f4dim);
	}

	// Apply LINEAR filter
	if ( ( sampler & __FILTER_MASK) == CL_DEV_SAMPLER_FILTER_LINEAR )
	{
		return read_2d_linear_f(image, sampler, f4coor);
	}

	// NEAREST FILTER
#ifdef __SSE4_1__
	__m128  r4coor = _mm_floor_ps(f4coor);
	__m128i i4coor = _mm_cvtps_epi32(r4coor);
#else
	__m128i i4coor = _mm_cvtps_epi32(__svml_floorf4(f4coor));
#endif

	if (CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT == iAddrMode)
	{
		i4coor = _mm_min_epi32(i4coor, _mm_add_epi32(i4dim, i4minusone));
	}
	if ( ((CL_DEV_SAMPLER_ADDRESS_REPEAT != iAddrMode) && (CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT != iAddrMode)) && 
		 ApplyAddressModeI(i4coor, image, (cl_dev_sampler_prop)(sampler & __ADDRESS_MASK)) )
	{
		return _mm_castsi128_ps(BoorderColorI(image->format));
	}

	// We are not on borders, sample the image
	void* pPxl = ExtractPixel2D(i4coor, image);
	return _mm_castsi128_ps(LoadPixel(pPxl, image->format));
}

_4i32   _Z11read_imageiP10_image2d_tjDv2_i(image2d_t image, cl_dev_sampler_prop sampler, intVecOf2 coord)
{
    //TODO check that XMM==>mem==>XMM transition doesn't happen in release mode (i.e. the intrinsic below is dropped)
	_4i32 i4coor = _mm_loadl_epi64((__m128i *)&coord);
	if ( ApplyAddressModeI(i4coor, image, (cl_dev_sampler_prop)(sampler & __ADDRESS_MASK)) )
	{
		return BoorderColorI(image->format);
	}

	// We are not on borders, sample the image
	void* pPxl = ExtractPixel2D(i4coor, image);
	return LoadPixel(pPxl, image->format);
}

_4i32   _Z11read_imageiP10_image2d_tjDv2_f(image2d_t image, cl_dev_sampler_prop sampler, floatVecOf2 coord)
{
	int iAddrMode = (sampler & __ADDRESS_MASK);
	__m128 f4coor = _mm_castsi128_ps(_mm_loadl_epi64((__m128i *)&coord));

	__m128i i4dim = _mm_loadl_epi64((__m128i*)image->dimensions.dim);
	__m128	f4dim = _mm_cvtepi32_ps(i4dim);
	// Adjust repeat on normalized coordinates
	if ( iAddrMode == CL_DEV_SAMPLER_ADDRESS_REPEAT)
	{
		f4coor = AddressRepeatCorrectNorm(f4coor);
		// for repeat ALWAYS NORMALIZED
		sampler = (cl_dev_sampler_prop)(sampler | CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE);
	}

	// Adjust repeat on normalized coordinates
	if ( iAddrMode == CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT)
	{
		f4coor = AddressMirrorRepeatCorrectNorm(f4coor, f4dim);

		// for repeat ALWAYS NORMALIZED
		sampler = (cl_dev_sampler_prop)(sampler & (~CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE));
	}
	// Unnormalized coordinates
	if ( (sampler & __NORMALIZED_MASK) ==  CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE)
	{
		// Now are UNORMILIZED, ready to work
		f4coor = _mm_mul_ps(f4coor, f4dim);
	}

	// Apply LINEAR filter
	if ( ( sampler & __FILTER_MASK) == CL_DEV_SAMPLER_FILTER_LINEAR )
	{
		return read_2d_linear_i(image, sampler, f4coor);
	}

	// NEAREST FILTER
#ifdef __SSE4_1__
	__m128  r4coor = _mm_floor_ps(f4coor);
	__m128i i4coor = _mm_cvtps_epi32(r4coor);
#else
	__m128i i4coor = _mm_cvtps_epi32(__svml_floorf4(f4coor));
#endif

	if (CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT == iAddrMode)
	{
		i4coor = _mm_min_epi32(i4coor, _mm_add_epi32(i4dim, i4minusone));
	}
	if ( ((CL_DEV_SAMPLER_ADDRESS_REPEAT != iAddrMode) && (CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT != iAddrMode)) && 
		ApplyAddressModeI(i4coor, image, (cl_dev_sampler_prop)(sampler & __ADDRESS_MASK)) )
	{
		return BoorderColorI(image->format);
	}

	// We are not on borders, sample the image
	void* pPxl = ExtractPixel2D(i4coor, image);
	return LoadPixel(pPxl, image->format);
}

_4u32  _Z12read_imageuiP10_image2d_tjDv2_i(image2d_t image, cl_dev_sampler_prop sampler, intVecOf2 coord)
{
    //TODO check that XMM==>mem==>XMM transition doesn't happen in release mode (i.e. the intrinsic below is dropped)
	_4i32 i4coor = _mm_loadl_epi64((__m128i *)&coord);
	if ( ApplyAddressModeI(i4coor, image, (cl_dev_sampler_prop)(sampler & __ADDRESS_MASK)) )
	{
		return BoorderColorI(image->format);
	}

	// We are not on borders, sample the image
	void* pPxl = ExtractPixel2D(i4coor, image);
	return LoadPixel(pPxl, image->format);
}

_4u32  _Z12read_imageuiP10_image2d_tjDv2_f(image2d_t image, cl_dev_sampler_prop sampler, floatVecOf2 coord)
{
	// Meanwhile call same function for unsigned values
	// TODO: Probably should be changed to be more specific
	return _Z11read_imageiP10_image2d_tjDv2_f(image, sampler, coord);
}

float4 _Z11read_imagefP10_image3d_tjDv4_i(image3d_t image, cl_dev_sampler_prop sampler, _4i32 coord)
{
	if ( ApplyAddressModeI(coord, image, (cl_dev_sampler_prop)(sampler & __ADDRESS_MASK)) )
	{
		return _mm_castsi128_ps(BoorderColorI(image->format));
	}

	// We are not on borders, sample the image
	void* pPxl = ExtractPixel3D(coord, image);
	return _mm_castsi128_ps(LoadPixel(pPxl, image->format));
}

float4 _Z11read_imagefP10_image3d_tjDv4_f(image3d_t image, cl_dev_sampler_prop sampler, float4 coord)
{
	int iAddrMode = (sampler & __ADDRESS_MASK);
	__m128 f4coor = coord;

	__m128i i4dim = _mm_lddqu_si128((__m128i*)image->dimensions.dim);
	__m128	f4dim = _mm_cvtepi32_ps(i4dim);
	// Adjust repeat on normalized coordinates
	if ( iAddrMode == CL_DEV_SAMPLER_ADDRESS_REPEAT)
	{
		f4coor = AddressRepeatCorrectNorm(f4coor);
		// for repeat ALWAYS NORMALIZED
		sampler = (cl_dev_sampler_prop)(sampler | CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE);
	}

	// Adjust repeat on normalized coordinates
	if ( iAddrMode == CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT)
	{
		f4coor = AddressMirrorRepeatCorrectNorm(f4coor, f4dim);

		// for repeat ALWAYS NORMALIZED
		sampler = (cl_dev_sampler_prop)(sampler & (~CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE));
	}
	// Unnormalized coordinates
	if ( (sampler & __NORMALIZED_MASK) ==  CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE)
	{
		// Now are UNORMILIZED, ready to work
		f4coor = _mm_mul_ps(f4coor, f4dim);
	}

	// Apply LINEAR filter
	if ( ( sampler & __FILTER_MASK) == CL_DEV_SAMPLER_FILTER_LINEAR )
	{
		return read_3d_linear_f(image, sampler, f4coor);
	}

	// NEAREST FILTER
#ifdef __SSE4_1__
	__m128  r4coor = _mm_floor_ps(f4coor);
	__m128i i4coor = _mm_cvtps_epi32(r4coor);
#else
	__m128i i4coor = _mm_cvtps_epi32(__svml_floorf4(f4coor));
#endif

	if (CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT == iAddrMode)
	{
		i4coor = _mm_min_epi32(i4coor, _mm_add_epi32(i4dim, i4minusone));
	}
	if ( ((CL_DEV_SAMPLER_ADDRESS_REPEAT != iAddrMode) && (CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT != iAddrMode)) && 
		ApplyAddressModeI(i4coor, image, (cl_dev_sampler_prop)(sampler & __ADDRESS_MASK)) )
	{
		return _mm_castsi128_ps(BoorderColorI(image->format));
	}

	// We are not on borders, sample the image
	void* pPxl = ExtractPixel3D(i4coor, image);
	return _mm_castsi128_ps(LoadPixel(pPxl, image->format));
}

_4i32   _Z11read_imageiP10_image3d_tjDv4_i(image3d_t image, cl_dev_sampler_prop sampler, _4i32 coord)
{
	if ( ApplyAddressModeI(coord, image, (cl_dev_sampler_prop)(sampler & __ADDRESS_MASK)) )
	{
		return BoorderColorI(image->format);
	}

	// We are not on borders, sample the image
	void* pPxl = ExtractPixel3D(coord, image);
	return LoadPixel(pPxl, image->format);
}

_4i32   _Z11read_imageiP10_image3d_tjDv4_f(image3d_t image, cl_dev_sampler_prop sampler, float4 coord)
{
	int iAddrMode = (sampler & __ADDRESS_MASK);
	__m128 f4coor = coord;

	__m128i i4dim = _mm_lddqu_si128((__m128i*)image->dimensions.dim);
	__m128	f4dim = _mm_cvtepi32_ps(i4dim);
	// Adjust repeat on normalized coordinates
	if ( iAddrMode == CL_DEV_SAMPLER_ADDRESS_REPEAT)
	{
		f4coor = AddressRepeatCorrectNorm(f4coor);
		// for repeat ALWAYS NORMALIZED
		sampler = (cl_dev_sampler_prop)(sampler | CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE);
	}

	// Adjust repeat on normalized coordinates
	if ( iAddrMode == CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT)
	{
		f4coor = AddressMirrorRepeatCorrectNorm(f4coor, f4dim);

		// for repeat ALWAYS NORMALIZED
		sampler = (cl_dev_sampler_prop)(sampler & (~CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE));
	}
	// Unnormalized coordinates
	if ( (sampler & __NORMALIZED_MASK) ==  CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE)
	{
		// Now are UNORMILIZED, ready to work
		f4coor = _mm_mul_ps(f4coor, f4dim);
	}

	// Apply LINEAR filter
	if ( ( sampler & __FILTER_MASK) == CL_DEV_SAMPLER_FILTER_LINEAR )
	{
		return read_3d_linear_i(image, sampler, f4coor);
	}

	// NEAREST FILTER
#ifdef __SSE4_1__
	__m128  r4coor = _mm_floor_ps(f4coor);
	__m128i i4coor = _mm_cvtps_epi32(r4coor);
#else
	__m128i i4coor = _mm_cvtps_epi32(__svml_floorf4(f4coor));
#endif

	if (CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT == iAddrMode)
	{
		i4coor = _mm_min_epi32(i4coor, _mm_add_epi32(i4dim, i4minusone));
	}
	if ( ((CL_DEV_SAMPLER_ADDRESS_REPEAT != iAddrMode) && (CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT != iAddrMode)) && 
		ApplyAddressModeI(i4coor, image, (cl_dev_sampler_prop)(sampler & __ADDRESS_MASK)) )
	{
		return BoorderColorI(image->format);
	}

	// We are not on borders, sample the image
	void* pPxl = ExtractPixel3D(i4coor, image);
	return LoadPixel(pPxl, image->format);
}

_4u32  _Z12read_imageuiP10_image3d_tjDv4_i(image3d_t image, cl_dev_sampler_prop sampler, _4i32 coord)
{
	if ( ApplyAddressModeI(coord, image, (cl_dev_sampler_prop)(sampler & __ADDRESS_MASK)) )
	{
		return BoorderColorI(image->format);
	}

	// We are not on borders, sample the image
	void* pPxl = ExtractPixel3D(coord, image);
	return LoadPixel(pPxl, image->format);
}

_4u32  _Z12read_imageuiP10_image3d_tjDv4_f(image3d_t image, cl_dev_sampler_prop sampler, float4 coord)
{
	return _Z11read_imageiP10_image3d_tjDv4_f(image, sampler, coord);
}

#ifdef _M_X64
void _Z12write_imagefP10_image2d_tDv2_iDv4_f(image2d_t image, intVecOf2 coord, float4 &val)
#else
void _Z12write_imagefP10_image2d_tDv2_iDv4_f(image2d_t image, intVecOf2 coord, float4 val)
#endif
{
    //TODO check that XMM==>mem==>XMM transition doesn't happen in release mode (i.e. the intrinsic below is dropped)
	// TODO: Consider use border detection to prevent writing outside the image
	_4i32 i4coor = _mm_loadl_epi64((__m128i *)&coord);

	void* pPxl = ExtractPixel2D(i4coor, image);
	StorePixel(pPxl, _mm_castps_si128(val), image->format);
}

#ifdef _M_X64
void _Z12write_imageiP10_image2d_tDv2_iDv4_i(image2d_t image, intVecOf2 coord, _4i32 &val)
#else
void _Z12write_imageiP10_image2d_tDv2_iDv4_i(image2d_t image, intVecOf2 coord, _4i32 val)
#endif
{
    //TODO check that XMM==>mem==>XMM transition doesn't happen in release mode (i.e. the intrinsic below is dropped)
	// TODO: Consider use border detection to prevent writing outside the image
	_4i32 i4coor = _mm_loadl_epi64((__m128i *)&coord);

	void* pPxl = ExtractPixel2D(i4coor, image);
	StorePixel(pPxl, val, image->format);
}

#ifdef _M_X64
void _Z13write_imageuiP10_image2d_tDv2_iDv4_j(image2d_t image, intVecOf2 coord, _4u32 &val)
#else
void _Z13write_imageuiP10_image2d_tDv2_iDv4_j(image2d_t image, intVecOf2 coord, _4u32 val)
#endif
{
    //TODO check that XMM==>mem==>XMM transition doesn't happen in release mode (i.e. the intrinsic below is dropped)
	// TODO: Consider use border detection to prevent writing outside the image
	_4i32 i4coor = _mm_loadl_epi64((__m128i *)&coord);

	void* pPxl = ExtractPixel2D(i4coor, image);
	StorePixel(pPxl, val, image->format);
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// Internal functions
//////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////
// Apply addressing mode on provided coordinates
// Operated always on 4 numbers
// integer coordinates
// Returns TRUE if pixel is on borders, else FALSE
bool ApplyAddressModeI(__m128i& coor, cl_mem_obj_descriptor* image, cl_dev_sampler_prop addrMode)
{
	// For integer coordinates it's always UNORMILIZED and NEAREST

	if ( CL_DEV_SAMPLER_ADDRESS_NONE == addrMode )
	{
		// When address mode is not selected, assume pixel inside
		return false;
	}

	// For CLAMP & CLAMP_TO_EDGE we need load image dimentions, as upper bound
	__m128i	i4up = _mm_lddqu_si128((__m128i*)image->dimensions.dim);
	// Set zero a low bound
	__m128i i4low = _mm_setzero_si128();
	// Decrement one for CLAMP address
	if ( CL_DEV_SAMPLER_ADDRESS_CLAMP == addrMode )
	{
		i4low = _mm_add_epi32(i4low, i4minusone);
	}
	else // CL_DEV_SAMPLER_ADDRESS_CLAMP_EDGE
	{
		i4up = _mm_add_epi32(i4up, i4minusone);
	}

	// Prepare mask for compare mask extraction
	int iMask = (1<<(image->dim_count*4))-1;

	// now perform clamp, no MIN/MAX for 32bit below SSE4.1
#ifdef __SSE4_1__
	coor = _mm_max_epi32(coor, i4low);
	coor = _mm_min_epi32(coor, i4up);
	// Check against lower bound
	__m128i i4mask = _mm_cmpgt_epi32(coor, i4low);
	i4mask = _mm_xor_si128(i4mask, i4minusone);                 // Get inverted mask
	int iBorder = _mm_movemask_epi8(i4mask) & iMask;            // Mark lower borders
	i4mask = _mm_cmplt_epi32(coor, i4up);
	i4mask = _mm_xor_si128(i4mask, i4minusone);                       // Get inverted mask
	iBorder |= _mm_movemask_epi8(i4mask) & iMask;               // Mark upper borders
#else
	// Check against lower bound
	__m128i i4mask = _mm_cmpgt_epi32(coor, i4low);
	i4mask = _mm_xor_si128(i4mask, i4minusone);			// Get inverted mask
	int iBorder = _mm_movemask_epi8(i4mask)  & iMask;	// Mark lower borders
	__m128i i4temp = _mm_and_si128(i4mask, i4low);
	i4mask = _mm_andnot_si128(i4mask, coor);
	i4temp = _mm_or_si128(i4temp, i4mask);				// contains latest data
	// Check against upper bound
	i4mask = _mm_cmplt_epi32(i4temp, i4up);
	i4mask = _mm_xor_si128(i4mask, i4minusone);			// Get inverted mask
	iBorder |= _mm_movemask_epi8(i4mask) & iMask;		// Mark upper borders
	coor = _mm_and_si128(i4mask, i4up);
	i4mask = _mm_andnot_si128(i4mask, i4temp);
	coor = _mm_or_si128(coor, i4mask);
#endif

	// Border is signaled only in CLAMP mode
	return iBorder && (CL_DEV_SAMPLER_ADDRESS_CLAMP == addrMode);
}

__forceinline __m128 AddressRepeatCorrectNorm(__m128 f4coor)
{
#ifdef __SSE4_1__
		__m128 f4tmp = _mm_floor_ps(f4coor);
#else
		__m128 f4tmp = __svml_floorf4(f4coor);
#endif
		f4coor = _mm_sub_ps(f4coor, f4tmp);

		// Correct values, spec: for values >= 1.0 substitute 1.0
		// Actually 1.0 is maximum execpeted value
		// Fast: Zero all values >= 1.0
		f4tmp = _mm_cmpge_ps(f4coor, f4one);
		f4tmp = _mm_andnot_ps(f4tmp, f4coor);

		return f4tmp;
}

__forceinline __m128 AddressMirrorRepeatCorrectNorm(__m128 f4coor, __m128 f4dim)
{
		__m128 f4S_ = _mm_mul_ps(f4coor, f4half);
#ifdef __SSE4_1__
		f4S_= _mm_round_ps(f4S_, _MM_ROUND_NEAREST);
#else
		f4S_= __svml_roundf4(f4S_, _MM_ROUND_NEAREST); 
#endif
		f4S_ = _mm_mul_ps(f4S_, f4two);
		f4S_ = _mm_sub_ps(f4S_, f4coor);
		f4S_ = _mm_abs_ps(f4S_);
		__m128 f4U = _mm_mul_ps(f4S_, f4dim);
		return f4U;
}

__forceinline __m128i AddressRepeatCorrectUnorm(__m128i coor, __m128i dim)
{
	// Set zero a low bound
	__m128i i4low = _mm_setzero_si128();

	dim = _mm_add_epi32(dim, i4minusone);
	// Check against lower bound
	__m128i i4mask = _mm_cmpgt_epi32(i4low, coor);
	// Use mask to blend with upper bound
#ifdef __SSE4_1__
	coor = _mm_blendv_epi8(coor, dim, i4mask);
#else
	_4i32 i4tmp = _mm_and_si128(dim, i4mask);
	i4mask = _mm_andnot_si128(i4mask, coor);
	coor = _mm_or_si128(i4tmp, i4mask);
#endif
	// Check with upper bound
	// If x > dim-1, it should be 0
	i4mask = _mm_cmpgt_epi32(coor, dim);
	i4mask = _mm_andnot_si128(i4mask, coor);
	return i4mask;
}

__forceinline __m128i AddressMirroredRepeatCorrectUnorm(__m128i coor, __m128i dim)
{
	coor = _mm_max_epi32(coor, _mm_setzero_si128());
	dim = _mm_add_epi32(dim, i4minusone);
	coor = _mm_min_epi32(coor, dim);
	return coor;
}

// Extract pointer to a specific pixel inside the image
// Alway receives integer coordinates
__forceinline void* ExtractPixel2D(__m128i coor, image2d_t image)
{
	// Retrieve X, Y coordinates
	int x = _mm_cvtsi128_si32(coor);
	coor = _mm_srli_si128(coor, 4);
	int y = _mm_cvtsi128_si32(coor);

	// Calculate required pixel offset
	void* pPxl = (char*)image->pData+y*image->pitch[0];
	pPxl = (char*)pPxl + x*image->uiElementSize;

	return pPxl;
}

__forceinline void* ExtractPixel3D(__m128i coor, image3d_t image)
{
	// Retrive X, Y coordinates
	int x = _mm_cvtsi128_si32(coor);
	coor = _mm_srli_si128(coor, 4);
	int y = _mm_cvtsi128_si32(coor);
	coor = _mm_srli_si128(coor, 4);
	int z = _mm_cvtsi128_si32(coor);

	// Calculate required pixel offset
	void* pPxl = (char*)image->pData+z*image->pitch[1];
	pPxl = (char*)pPxl + y*image->pitch[0];
	pPxl = (char*)pPxl + x*image->uiElementSize;
	
	return pPxl;
}

__forceinline _4i32 getBorder(cl_channel_type type)
{
	switch (type)
	{
	case CLK_SNORM_INT8: case CLK_SNORM_INT16: case CLK_UNORM_INT8: case CLK_UNORM_INT16:
	case CLK_FLOAT: case CLK_HALF_FLOAT: case CLK_UNORM_INT_101010: case CLK_UNORM_SHORT_555:
	case CLK_UNORM_SHORT_565:
		return i4FloatBorder;
	case CLK_UNSIGNED_INT8:
	case CLK_SIGNED_INT8:
	case CLK_UNSIGNED_INT16:
	case CLK_SIGNED_INT16:
	case CLK_UNSIGNED_INT32:
	case CLK_SIGNED_INT32:
		return i4IntBorder;
	}
	return _mm_setzero_si128();
}

__forceinline _4i32 get_pixel_val_2D(image2d_t image, _4i32 coor, int iAddrMode)
{
	_4i32 pixel;
	if ( ((CL_DEV_SAMPLER_ADDRESS_REPEAT != iAddrMode) && (CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT != iAddrMode)) && 
		ApplyAddressModeI(coor, image, (cl_dev_sampler_prop)iAddrMode) )
	{
		pixel = BoorderColorI(image->format);
	} else
	{
		if ((CL_DEV_SAMPLER_ADDRESS_REPEAT == iAddrMode))
		{
			coor = AddressRepeatCorrectUnorm(coor, _mm_loadl_epi64((__m128i*)image->dimensions.dim));
		}
		if ((CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT == iAddrMode))
		{
			coor = AddressMirroredRepeatCorrectUnorm(coor, _mm_loadl_epi64((__m128i*)image->dimensions.dim));
		}
		// We are not on borders, sample the image
		void* pPxl = ExtractPixel2D(coor, image);
		pixel = LoadPixel(pPxl, image->format);
	}
	return pixel;
}

__forceinline _4i32 get_pixel_val_3D(image2d_t image, _4i32 coor, int iAddrMode)
{
	_4i32 pixel;

	if ( ((CL_DEV_SAMPLER_ADDRESS_REPEAT != iAddrMode) && (CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT != iAddrMode)) && 
		ApplyAddressModeI(coor, image, (cl_dev_sampler_prop)iAddrMode) )
	{
		pixel = BoorderColorI(image->format);
	} else
	{
		if ((CL_DEV_SAMPLER_ADDRESS_REPEAT == iAddrMode))
		{
			coor = AddressRepeatCorrectUnorm(coor, _mm_lddqu_si128((__m128i*)image->dimensions.dim));
		}
		if ((CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT == iAddrMode))
		{
			coor = AddressMirroredRepeatCorrectUnorm(coor, _mm_lddqu_si128((__m128i*)image->dimensions.dim));
		}
		// We are not on borders, sample the image
		void* pPxl = ExtractPixel3D(coor, image);
		pixel = LoadPixel(pPxl, image->format);
	}

	return pixel;
}

__forceinline float4 read_2d_linear_f(image2d_t image, cl_dev_sampler_prop sampler, float4 f4coor)
{
	f4coor = _mm_sub_ps(f4coor, f4half);
#ifdef __SSE4_1__
	float4	f4floor = _mm_floor_ps(f4coor);
#else
	float4	f4floor = __svml_floorf4(f4coor);
#endif

	// Calculate blending values
	f4coor = _mm_sub_ps(f4coor, f4floor);
	float4 f4BlendX = _mm_shuffle_ps(f4coor, f4coor, _MM_SHUFFLE(0, 0, 0, 0));
	float4 f4BlendY = _mm_shuffle_ps(f4coor, f4coor, _MM_SHUFFLE(1, 1, 1, 1));

	// We have two sets of coordinates
	_4i32 i4Y0X0 = _mm_cvtps_epi32(f4floor);							// Y0|X0
	_4i32 i4Y1X1 = _mm_sub_epi32(i4Y0X0, i4minusone);				// Y1|X1

	// Now we need 2 additional permutations
	_4i32 i4Y0X1 = _mm_unpacklo_epi32(i4Y0X0, i4Y1X1);				// Y1|X1|Y0|X0
	// Permute
	i4Y0X1 = _mm_shuffle_epi32(i4Y0X1, _MM_SHUFFLE(3, 0, 2, 1));	// Y1|X0|Y0|X1
	_4i32 i4Y1X0 = _mm_srli_si128(i4Y0X1, 8);

	int iAddrMode = (sampler & __ADDRESS_MASK);
	// Load Y0X0
	float4 f4Y0X0 = _mm_castsi128_ps(get_pixel_val_2D(image, i4Y0X0, iAddrMode));
	// Load Y1X0
	float4 f4Y1X0 = _mm_castsi128_ps(get_pixel_val_2D(image, i4Y1X0, iAddrMode));

	// Blend along Y axis
	// (1-a)*A+a*B == A+(B-A)*a
	f4Y1X0 = _mm_sub_ps(f4Y1X0, f4Y0X0);
	f4Y1X0 = _mm_mul_ps(f4Y1X0, f4BlendY);
	f4Y0X0 = _mm_add_ps(f4Y0X0, f4Y1X0);			//	f4Y0X0 contains first interpolated value

	// Load Y0X1
	float4 f4Y0X1 = _mm_castsi128_ps(get_pixel_val_2D(image, i4Y0X1, iAddrMode));
	// Load Y1X1
	float4 f4Y1X1 = _mm_castsi128_ps(get_pixel_val_2D(image, i4Y1X1, iAddrMode));

	// Blend along Y axis
	// (1-a)*A+a*B == A+(B-A)*a
	f4Y1X1 = _mm_sub_ps(f4Y1X1, f4Y0X1);
	f4Y1X1 = _mm_mul_ps(f4Y1X1, f4BlendY);
	f4Y0X1 = _mm_add_ps(f4Y0X1, f4Y1X1);			//	f4Y0X0 contains first interpolated value

	// Now blend along X axis
	f4Y0X1 = _mm_sub_ps(f4Y0X1, f4Y0X0);
	f4Y0X1 = _mm_mul_ps(f4Y0X1, f4BlendX);
	f4Y0X0 = _mm_add_ps(f4Y0X0, f4Y0X1);			//	f4Y0X0 contains final interpolated value

	return f4Y0X0;	
}

__forceinline float4 read_3d_linear_f(image3d_t image, cl_dev_sampler_prop sampler, float4 f4coor)
{
	f4coor = _mm_sub_ps(f4coor, f4half);
#ifdef __SSE4_1__
	float4	f4floor = _mm_floor_ps(f4coor);
#else
	float4	f4floor = __svml_floorf4(f4coor);
#endif

	// Calculate blending values
	f4coor = _mm_sub_ps(f4coor, f4floor);
	float4 f4BlendX = _mm_shuffle_ps(f4coor, f4coor, _MM_SHUFFLE(0, 0, 0, 0));
	float4 f4BlendY = _mm_shuffle_ps(f4coor, f4coor, _MM_SHUFFLE(1, 1, 1, 1));
	float4 f4BlendZ = _mm_shuffle_ps(f4coor, f4coor, _MM_SHUFFLE(2, 2, 2, 2));

	// We have two sets of coordinates
	_4i32 i4Z0Y0X0 = _mm_cvtps_epi32(f4floor);							// Z0|Y0|X0
	_4i32 i4Z1Y1X1 = _mm_sub_epi32(i4Z0Y0X0, i4minusone);				// Z1|Y1|X1

	// Now we need 2 additional permutations
	_4i32 i4Y0X1 = _mm_unpacklo_epi32(i4Z0Y0X0, i4Z1Y1X1);			// Y1|X1|Y0|X0
	// Permute
	i4Y0X1 = _mm_shuffle_epi32(i4Y0X1, _MM_SHUFFLE(3, 0, 2, 1));	// Y1|X0|Y0|X1
	_4i32 i4Y1X0 = _mm_srli_si128(i4Y0X1, 8);

	int iAddrMode = (sampler & __ADDRESS_MASK);

	/////////////////////////////////////////////////////////////////////////////////////////
	// Blend first plane
	// Load Z0Y0X0
	float4 f4Y0X0 = _mm_castsi128_ps(get_pixel_val_3D(image, i4Z0Y0X0, iAddrMode));

	_4i32 i4Z0 = _mm_srli_si128(i4Z0Y0X0, 8);
	// Load Y1X0
	// Merge Z value
	i4Y1X0 = _mm_unpacklo_epi64(i4Y1X0, i4Z0);
	float4 f4Y1X0 = _mm_castsi128_ps(get_pixel_val_3D(image, i4Y1X0, iAddrMode));

	// Blend along Y axis
	// (1-a)*A+a*B == A+(B-A)*a
	f4Y1X0 = _mm_sub_ps(f4Y1X0, f4Y0X0);
	f4Y1X0 = _mm_mul_ps(f4Y1X0, f4BlendY);
	f4Y0X0 = _mm_add_ps(f4Y0X0, f4Y1X0);			//	f4Y0X0 contains first interpolated value

	// Load Y0X1
	// Merge Z value
	i4Y0X1 = _mm_unpacklo_epi64(i4Y0X1, i4Z0);
	float4 f4Y0X1 = _mm_castsi128_ps(get_pixel_val_3D(image, i4Y0X1, iAddrMode));
	// Load Y1X1
	// Merge Z value
	_4i32 i4Y1X1 = _mm_unpacklo_epi64(i4Z1Y1X1, i4Z0);
	float4 f4Y1X1 = _mm_castsi128_ps(get_pixel_val_3D(image, i4Y1X1, iAddrMode));
	// Blend along Y axis
	// (1-a)*A+a*B == A+(B-A)*a
	f4Y1X1 = _mm_sub_ps(f4Y1X1, f4Y0X1);
	f4Y1X1 = _mm_mul_ps(f4Y1X1, f4BlendY);
	f4Y0X1 = _mm_add_ps(f4Y0X1, f4Y1X1);			//	f4Y0X0 contains first interpolated value

	// Now blend along X axis
	f4Y0X1 = _mm_sub_ps(f4Y0X1, f4Y0X0);
	f4Y0X1 = _mm_mul_ps(f4Y0X1, f4BlendX);
	f4Y0X0 = _mm_add_ps(f4Y0X0, f4Y0X1);			//	f4Y0X0 contains first plane interpolated value

	/////////////////////////////////////////////////////////////////////////////////////////
	// Blend second plane
	// Load Z1Y1X1
	f4Y1X1 = _mm_castsi128_ps(get_pixel_val_3D(image, i4Z1Y1X1, iAddrMode));
	_4i32 i4Z1 = _mm_srli_si128(i4Z1Y1X1, 8);
	// Load Y0X1
	// Merge Z value
	i4Y0X1 = _mm_unpacklo_epi64(i4Y0X1, i4Z1);
	f4Y0X1 = _mm_castsi128_ps(get_pixel_val_3D(image, i4Y0X1, iAddrMode));

	// Blend along Y axis
	// (1-a)*A+a*B == A+(B-A)*a
	f4Y1X1 = _mm_sub_ps(f4Y1X1, f4Y0X1);
	f4Y1X1 = _mm_mul_ps(f4Y1X1, f4BlendY);
	f4Y0X1 = _mm_add_ps(f4Y0X1, f4Y1X1);			//	f4Y0X1 contains first interpolated value

	// Load Y0X0 to Y1X1
	// Merge Z value
	_4i32 i4Y0X0 = _mm_unpacklo_epi64(i4Z0Y0X0, i4Z1);
	f4Y1X1 = _mm_castsi128_ps(get_pixel_val_3D(image, i4Y0X0, iAddrMode));
	// Load Y1X0
	// Merge Z value
	i4Y1X0 = _mm_unpacklo_epi64(i4Y1X0, i4Z1);
	f4Y1X0 = _mm_castsi128_ps(get_pixel_val_3D(image, i4Y1X0, iAddrMode));

	// Blend along Y axis
	// (1-a)*A+a*B == A+(B-A)*a
	f4Y1X0 = _mm_sub_ps(f4Y1X0, f4Y1X1);
	f4Y1X0 = _mm_mul_ps(f4Y1X0, f4BlendY);
	f4Y1X1 = _mm_add_ps(f4Y1X1, f4Y1X0);			//	f4Y1X1 contains first interpolated value

	// Now blend along X axis
	f4Y0X1 = _mm_sub_ps(f4Y0X1, f4Y1X1);
	f4Y0X1 = _mm_mul_ps(f4Y0X1, f4BlendX);
	f4Y1X1 = _mm_add_ps(f4Y1X1, f4Y0X1);			//	f4Y0X0 contains second plane interpolated value

	// Interpolate between two planes
	f4Y1X1 = _mm_sub_ps(f4Y1X1, f4Y0X0);
	f4Y1X1 = _mm_mul_ps(f4Y1X1, f4BlendZ);
	f4Y0X0 = _mm_add_ps(f4Y0X0, f4Y1X1);

	return f4Y0X0;	
}

__forceinline _4i32 read_2d_linear_i(image2d_t image, cl_dev_sampler_prop sampler, float4 f4coor)
{
	f4coor = _mm_sub_ps(f4coor, f4half);
#ifdef __SSE4_1__
	float4	f4floor = _mm_floor_ps(f4coor);
#else
	float4	f4floor = __svml_floorf4(f4coor);
#endif

	// Calculate blending values
	f4coor = _mm_sub_ps(f4coor, f4floor);
	float4 f4BlendX = _mm_shuffle_ps(f4coor, f4coor, _MM_SHUFFLE(0, 0, 0, 0));
	float4 f4BlendY = _mm_shuffle_ps(f4coor, f4coor, _MM_SHUFFLE(1, 1, 1, 1));

	// We have two sets of coordinates
	_4i32 i4Y0X0 = _mm_cvtps_epi32(f4floor);							// Y0|X0
	_4i32 i4Y1X1 = _mm_sub_epi32(i4Y0X0, i4minusone);				// Y1|X1

	// Now we need 2 additional permutations
	_4i32 i4Y0X1 = _mm_unpacklo_epi32(i4Y0X0, i4Y1X1);				// Y1|X1|Y0|X0
	// Permute
	i4Y0X1 = _mm_shuffle_epi32(i4Y0X1, _MM_SHUFFLE(3, 0, 2, 1));	// Y1|X0|Y0|X1
	_4i32 i4Y1X0 = _mm_srli_si128(i4Y0X1, 8);

	int iAddrMode = (sampler & __ADDRESS_MASK);
	// Load Y0X0
	float4 f4Y0X0 = _mm_cvtepi32_ps(get_pixel_val_2D(image, i4Y0X0, iAddrMode));
	// Load Y1X0
	float4 f4Y1X0 = _mm_cvtepi32_ps(get_pixel_val_2D(image, i4Y0X0, iAddrMode));

	// Blend along Y axis
	// (1-a)*A+a*B == A+(B-A)*a
	f4Y1X0 = _mm_sub_ps(f4Y1X0, f4Y0X0);
	f4Y1X0 = _mm_mul_ps(f4Y1X0, f4BlendY);
	f4Y0X0 = _mm_add_ps(f4Y0X0, f4Y1X0);			//	f4Y0X0 contains first interpolated value

	// Load Y0X1
	float4 f4Y0X1 = _mm_cvtepi32_ps(get_pixel_val_2D(image, i4Y0X1, iAddrMode));
	// Load Y1X1
	float4 f4Y1X1 = _mm_cvtepi32_ps(get_pixel_val_2D(image, i4Y1X1, iAddrMode));

	// Blend along Y axis
	// (1-a)*A+a*B == A+(B-A)*a
	f4Y1X1 = _mm_sub_ps(f4Y1X1, f4Y0X1);
	f4Y1X1 = _mm_mul_ps(f4Y1X1, f4BlendY);
	f4Y0X1 = _mm_add_ps(f4Y0X1, f4Y1X1);			//	f4Y0X0 contains first interpolated value

	// Now blend along X axis
	f4Y0X1 = _mm_sub_ps(f4Y0X1, f4Y0X0);
	f4Y0X1 = _mm_mul_ps(f4Y0X1, f4BlendX);
	f4Y0X0 = _mm_add_ps(f4Y0X0, f4Y0X1);			//	f4Y0X0 contains final interpolated value

	// Convert back to integer
	return _mm_cvtps_epi32(f4Y0X0);
}

__forceinline _4i32 read_3d_linear_i(image3d_t image, cl_dev_sampler_prop sampler, float4 f4coor)
{
	f4coor = _mm_sub_ps(f4coor, f4half);
#ifdef __SSE4_1__
	float4	f4floor = _mm_floor_ps(f4coor);
#else
	float4	f4floor = __svml_floorf4(f4coor);
#endif

	// Calculate blending values
	f4coor = _mm_sub_ps(f4coor, f4floor);
	float4 f4BlendX = _mm_shuffle_ps(f4coor, f4coor, _MM_SHUFFLE(0, 0, 0, 0));
	float4 f4BlendY = _mm_shuffle_ps(f4coor, f4coor, _MM_SHUFFLE(1, 1, 1, 1));
	float4 f4BlendZ = _mm_shuffle_ps(f4coor, f4coor, _MM_SHUFFLE(2, 2, 2, 2));

	// We have two sets of coordinates
	_4i32 i4Z0Y0X0 = _mm_cvtps_epi32(f4floor);							// Z0|Y0|X0
	_4i32 i4Z1Y1X1 = _mm_sub_epi32(i4Z0Y0X0, i4minusone);				// Z1|Y1|X1

	// Now we need 2 additional permutations
	_4i32 i4Y0X1 = _mm_unpacklo_epi32(i4Z0Y0X0, i4Z1Y1X1);			// Y1|X1|Y0|X0
	// Permute
	i4Y0X1 = _mm_shuffle_epi32(i4Y0X1, _MM_SHUFFLE(3, 0, 2, 1));	// Y1|X0|Y0|X1
	_4i32 i4Y1X0 = _mm_srli_si128(i4Y0X1, 8);

	int iAddrMode = (sampler & __ADDRESS_MASK);

	/////////////////////////////////////////////////////////////////////////////////////////
	// Blend first plane
	// Load Z0Y0X0
	float4 f4Y0X0 = _mm_cvtepi32_ps(get_pixel_val_2D(image, i4Z0Y0X0, iAddrMode));

	_4i32 i4Z0 = _mm_srli_si128(i4Z0Y0X0, 8);
	// Load Y1X0
	// Merge Z value
	i4Y1X0 = _mm_unpacklo_epi64(i4Y1X0, i4Z0);
	float4 f4Y1X0 = _mm_cvtepi32_ps(get_pixel_val_2D(image, i4Y1X0, iAddrMode));

	// Blend along Y axis
	// (1-a)*A+a*B == A+(B-A)*a
	f4Y1X0 = _mm_sub_ps(f4Y1X0, f4Y0X0);
	f4Y1X0 = _mm_mul_ps(f4Y1X0, f4BlendY);
	f4Y0X0 = _mm_add_ps(f4Y0X0, f4Y1X0);			//	f4Y0X0 contains first interpolated value

	// Load Y0X1
	// Merge Z value
	i4Y0X1 = _mm_unpacklo_epi64(i4Y0X1, i4Z0);
	float4 f4Y0X1 = _mm_cvtepi32_ps(get_pixel_val_2D(image, i4Y0X1, iAddrMode));
	// Load Y1X1
	// Merge Z value
	_4i32 i4Y1X1 = _mm_unpacklo_epi64(i4Z1Y1X1, i4Z0);
	float4 f4Y1X1 = _mm_cvtepi32_ps(get_pixel_val_2D(image, i4Y1X1, iAddrMode));

	// Blend along Y axis
	// (1-a)*A+a*B == A+(B-A)*a
	f4Y1X1 = _mm_sub_ps(f4Y1X1, f4Y0X1);
	f4Y1X1 = _mm_mul_ps(f4Y1X1, f4BlendY);
	f4Y0X1 = _mm_add_ps(f4Y0X1, f4Y1X1);			//	f4Y0X0 contains first interpolated value

	// Now blend along X axis
	f4Y0X1 = _mm_sub_ps(f4Y0X1, f4Y0X0);
	f4Y0X1 = _mm_mul_ps(f4Y0X1, f4BlendX);
	f4Y0X0 = _mm_add_ps(f4Y0X0, f4Y0X1);			//	f4Y0X0 contains first plane interpolated value

	/////////////////////////////////////////////////////////////////////////////////////////
	// Blend second plane
	// Load Z1Y1X1
	f4Y1X1 = _mm_cvtepi32_ps(get_pixel_val_2D(image, i4Z1Y1X1, iAddrMode));

	_4i32 i4Z1 = _mm_srli_si128(i4Z1Y1X1, 8);
	// Load Y0X1
	// Merge Z value
	i4Y0X1 = _mm_unpacklo_epi64(i4Y0X1, i4Z1);
	f4Y0X1 = _mm_cvtepi32_ps(get_pixel_val_2D(image, i4Y0X1, iAddrMode));

	// Blend along Y axis
	// (1-a)*A+a*B == A+(B-A)*a
	f4Y1X1 = _mm_sub_ps(f4Y1X1, f4Y0X1);
	f4Y1X1 = _mm_mul_ps(f4Y1X1, f4BlendY);
	f4Y0X1 = _mm_add_ps(f4Y0X1, f4Y1X1);			//	f4Y0X1 contains first interpolated value

	// Load Y0X0 to Y1X1
	// Merge Z value
	_4i32 i4Y0X0 = _mm_unpacklo_epi64(i4Z0Y0X0, i4Z1);
	f4Y1X1 = _mm_cvtepi32_ps(get_pixel_val_2D(image, i4Y0X0, iAddrMode));

	// Load Y1X0
	// Merge Z value
	i4Y1X0 = _mm_unpacklo_epi64(i4Y1X0, i4Z1);
	f4Y1X0 = _mm_cvtepi32_ps(get_pixel_val_2D(image, i4Y1X0, iAddrMode));

	// Blend along Y axis
	// (1-a)*A+a*B == A+(B-A)*a
	f4Y1X0 = _mm_sub_ps(f4Y1X0, f4Y1X1);
	f4Y1X0 = _mm_mul_ps(f4Y1X0, f4BlendY);
	f4Y1X1 = _mm_add_ps(f4Y1X1, f4Y1X0);			//	f4Y1X1 contains first interpolated value

	// Now blend along X axis
	f4Y0X1 = _mm_sub_ps(f4Y0X1, f4Y1X1);
	f4Y0X1 = _mm_mul_ps(f4Y0X1, f4BlendX);
	f4Y1X1 = _mm_add_ps(f4Y1X1, f4Y0X1);			//	f4Y0X0 contains second plane interpolated value

	// Interpolate between two planes
	f4Y1X1 = _mm_sub_ps(f4Y1X1, f4Y0X0);
	f4Y1X1 = _mm_mul_ps(f4Y1X1, f4BlendZ);
	f4Y0X0 = _mm_add_ps(f4Y0X0, f4Y1X1);

	return _mm_cvtps_epi32(f4Y0X0);
}

//------------------------------------------------------------------------
// Load function
_4i32 LoadPixel(void* pPxl, cl_image_format fmt)
{
	cl_uint type = fmt.image_channel_data_type;
	_4i32	i4Val;
	switch (fmt.image_channel_order)
	{
	case CLK_R:
		i4Val = LoadSingleChannel(pPxl, type);
		// Set Alpha channel to 1
		i4Val = _mm_or_si128(i4Val, getBorder(type));
		break;
	case CLK_A:
		i4Val = LoadSingleChannel(pPxl, type);
		// Move Alpha channel to highest DWORD
		i4Val = _mm_slli_si128(i4Val, 12);
		break;
	case CLK_LUMINANCE:
		i4Val = LoadSingleChannel(pPxl, type);
		i4Val = _mm_or_si128(i4Val, getBorder(type));
		// Take all values from lower DWORD, execpt the highest one
		i4Val = _mm_shuffle_epi32(i4Val, _MM_SHUFFLE(3, 0, 0, 0));
		break;
	case CLK_INTENSITY:
		i4Val = LoadSingleChannel(pPxl, type);
		// Take all values from lower DWORD, execpt the highest one
		i4Val = _mm_shuffle_epi32(i4Val, _MM_SHUFFLE(0, 0, 0, 0));
		break;
	case CLK_RG:
		i4Val = LoadDualChannel(pPxl, type);
		// Set Alpha channel to 1
		i4Val = _mm_or_si128(i4Val, getBorder(type));
		break;
	case CLK_RA:
		i4Val = LoadDualChannel(pPxl, type);
		// Move Alpha channel to highest DWORD, leave R and fill with 0 others
		i4Val = _mm_shuffle_epi32(i4Val, _MM_SHUFFLE(1, 3, 3, 0));
		break;
	case CLK_RGB:
		i4Val = LoadTripleChannel(pPxl, type);
//		i4Val = _mm_shuffle_epi32(i4Val, _MM_SHUFFLE(3, 0, 1, 2));
		i4Val = _mm_or_si128(i4Val, getBorder(type));
		break;
	case CLK_RGBA:
		i4Val = LoadQuadChannel(pPxl, type);
		//i4Val = _mm_shuffle_epi32(i4Val, _MM_SHUFFLE(3, 2, 1, 0));
		break;
	case CLK_BGRA:
		i4Val = LoadQuadChannel(pPxl, type);
		// Exchange bewteen R & B channels
		i4Val = _mm_shuffle_epi32(i4Val, _MM_SHUFFLE(3, 0, 1, 2));
		break;
	case CLK_ARGB:
		i4Val = LoadQuadChannel(pPxl, type);
		// Exchange bewteen A,R,G & B channels
		i4Val = _mm_shuffle_epi32(i4Val, _MM_SHUFFLE(0, 3, 2, 1));
		break;

	default:
		return _mm_setzero_si128();
	}

	return i4Val;
}
// Returs XMM register contains channels extened to 32bit
// Float values are passed by integer representation

// Functions for converting NORM integers to float
__forceinline __m128i cvt_from_snorm(__m128i i4Val, __m128& Div)
{
	__m128 f4Val = _mm_cvtepi32_ps(i4Val);
	f4Val = _mm_mul_ps(f4Val, Div);
	f4Val = _mm_max_ps(f4Val, f4minusone);
	return _mm_castps_si128(f4Val);
}

__forceinline __m128i cvt_from_unorm(__m128i i4Val, __m128& Div)
{
	__m128 f4Val = _mm_cvtepi32_ps(i4Val);
	// Multiply by 1/x
	f4Val = _mm_mul_ps(f4Val, Div);
	return _mm_castps_si128(f4Val);
}

// Functions for converting float integers to NORM
__forceinline __m128i cvt_to_norm(__m128i i4Val, __m128& f4Mul, __m128& lowLimit)
{
	__m128 f4Val = _mm_castsi128_ps(i4Val);
	f4Val = _mm_mul_ps(f4Val, f4Mul);
	// Apply limits, the upper limit is always the devider
	f4Val = _mm_min_ps(f4Val, f4Mul);
	f4Val = _mm_max_ps(f4Val, lowLimit);

	return _mm_cvtps_epi32(f4Val);
}

// Load pixels that are constructed from single channel
_4i32 LoadSingleChannel(void *pPxl, cl_channel_type type)
{
	__m128i	i4Val = _mm_setzero_si128();

	switch (type)
	{
	case CLK_SNORM_INT8:
		i4Val = _mm_cvtsi32_si128(*(char*)pPxl);
		i4Val = cvt_from_snorm(i4Val, f4snorm8div);
		break;
	case CLK_SIGNED_INT8:
		i4Val = _mm_cvtsi32_si128(*(char*)pPxl);
		break;
	case CLK_UNORM_INT8:
		i4Val = _mm_cvtsi32_si128(*((unsigned char*)pPxl));
		i4Val = cvt_from_unorm(i4Val, f4unorm8div);
		break;
	case CLK_UNSIGNED_INT8:
		i4Val = _mm_cvtsi32_si128(*((unsigned char*)pPxl));
		break;
	case CLK_SNORM_INT16:
		i4Val = _mm_cvtsi32_si128(*((short*)pPxl));
		i4Val = cvt_from_snorm(i4Val, f4snorm16div);
		break;
	case CLK_SIGNED_INT16:
		i4Val = _mm_cvtsi32_si128(*((short*)pPxl));
		break;
	case CLK_UNORM_INT16:
		i4Val = _mm_cvtsi32_si128(*((unsigned short*)pPxl));
		i4Val = cvt_from_unorm(i4Val, f4unorm16div);
		break;
	case CLK_UNSIGNED_INT16:
		i4Val = _mm_cvtsi32_si128(*((unsigned short*)pPxl));
		break;
	case CLK_HALF_FLOAT:
		i4Val = (__m128i)Half4ToFloat4(_mm_loadl_epi64((__m128i*)pPxl));
		break;
	case CLK_SIGNED_INT32:
	case CLK_UNSIGNED_INT32:
	case CLK_FLOAT:
		i4Val = _mm_cvtsi32_si128(*((int*)pPxl));
		break;
	}

	return i4Val;
}

// Load pixels that are constructed from two channel
_4i32 LoadDualChannel(void *pPxl, cl_channel_type type)
{
	__m128i	i4Val;
	switch (type)
	{
	case CLK_SNORM_INT8:
		i4Val = _mm_cvtsi32_si128(*(unsigned short*)pPxl);
		i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// Extend sign
		i4Val = _mm_slli_si128(i4Val, 3);
		i4Val = _mm_srai_epi32(i4Val, 24);
		i4Val = cvt_from_snorm(i4Val, f4snorm8div);
		break;

	case CLK_SIGNED_INT8:
		i4Val = _mm_cvtsi32_si128(*(unsigned short*)pPxl);
		i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// Extend sign
		i4Val = _mm_slli_si128(i4Val, 3);
		i4Val = _mm_srai_epi32(i4Val, 24);
		break;

	case CLK_UNORM_INT8:
		i4Val = _mm_cvtsi32_si128(*(unsigned short*)pPxl);
		i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// No sign extend
		i4Val = cvt_from_unorm(i4Val, f4unorm8div);
		break;
	case CLK_UNSIGNED_INT8:
		i4Val = _mm_cvtsi32_si128(*(unsigned short*)pPxl);
		i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// No sign extend
		break;

	case CLK_SNORM_INT16:
		i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPxl);
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// Extend sign
		i4Val = _mm_slli_si128(i4Val, 2);
		i4Val = _mm_srai_epi32(i4Val, 16);
		i4Val = cvt_from_snorm(i4Val, f4snorm16div);
		break;

	case CLK_SIGNED_INT16:
		i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPxl);
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// Extend sign
		i4Val = _mm_slli_si128(i4Val, 2);
		i4Val = _mm_srai_epi32(i4Val, 16);
		break;

	case CLK_UNORM_INT16:
		i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPxl);
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// no extend sign
		i4Val = cvt_from_unorm(i4Val, f4unorm16div);
		break;

	case CLK_UNSIGNED_INT16:
		i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPxl);
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// no extend sign
		break;

	case CLK_HALF_FLOAT:
		i4Val = (__m128i)Half4ToFloat4(_mm_loadl_epi64((__m128i*)pPxl));
		break;

	case CLK_SIGNED_INT32:
	case CLK_UNSIGNED_INT32:
	case CLK_FLOAT:
		i4Val = _mm_loadl_epi64((__m128i*)pPxl);
		break;
	}

	return i4Val;
}

// Load pixels that are constructed from three channels
_4i32 LoadTripleChannel(void *pPxl, cl_channel_type type)
{
	__m128i	i4Val;
	int		iVal;
	switch (type)
	{
	case CLK_SNORM_INT8:
		iVal = *((unsigned short*)pPxl)<<16 + *((unsigned char*)pPxl+2);
		i4Val = _mm_cvtsi32_si128(iVal);
		i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// Extend sign
		i4Val = _mm_slli_si128(i4Val, 3);
		i4Val = _mm_srai_epi32(i4Val, 24);
		i4Val = cvt_from_snorm(i4Val, f4snorm8div);
		break;
	case CLK_SIGNED_INT8:
		iVal = *((unsigned short*)pPxl)<<16 + *((unsigned char*)pPxl+2);
		i4Val = _mm_cvtsi32_si128(iVal);
		i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// Extend sign
		i4Val = _mm_slli_si128(i4Val, 3);
		i4Val = _mm_srai_epi32(i4Val, 24);
		break;
	case CLK_UNORM_INT8:
		iVal = *((unsigned short*)pPxl)<<16 + *((unsigned char*)pPxl+2);
		i4Val = _mm_cvtsi32_si128(iVal);
		i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// No sign extend
		i4Val = cvt_from_unorm(i4Val, f4unorm8div);
		break;
	case CLK_UNSIGNED_INT8:
		iVal = *((unsigned short*)pPxl)<<16 + *((unsigned char*)pPxl+2);
		i4Val = _mm_cvtsi32_si128(iVal);
		i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// No sign extend
		break;
	case CLK_SNORM_INT16:
		i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPxl);
		i4Val = _mm_slli_si128(i4Val, 2);
		i4Val = _mm_or_si128(i4Val, _mm_cvtsi32_si128(*((unsigned short*)pPxl+2)));
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// Extend sign
		i4Val = _mm_slli_si128(i4Val, 2);
		i4Val = _mm_srai_epi32(i4Val, 16);
		i4Val = cvt_from_snorm(i4Val, f4snorm16div);
		break;
	case CLK_SIGNED_INT16:
		i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPxl);
		i4Val = _mm_slli_si128(i4Val, 2);
		i4Val = _mm_or_si128(i4Val, _mm_cvtsi32_si128(*((unsigned short*)pPxl+2)));
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// Extend sign
		i4Val = _mm_slli_si128(i4Val, 2);
		i4Val = _mm_srai_epi32(i4Val, 16);
		break;
	case CLK_UNORM_INT16:
		i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPxl);
		i4Val = _mm_slli_si128(i4Val, 2);
		i4Val = _mm_or_si128(i4Val, _mm_cvtsi32_si128(*((unsigned short*)pPxl+2)));
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// no extend sign
		i4Val = cvt_from_unorm(i4Val, f4unorm16div);
		break;
	case CLK_UNORM_SHORT_565:
		iVal = *(unsigned short*)pPxl;
		i4Val = _mm_cvtsi32_si128(iVal & 0x1F);
		iVal >>=5;
		i4Val = _mm_slli_si128(i4Val, 4);
		i4Val = _mm_or_si128(i4Val, _mm_cvtsi32_si128(iVal & 0x3F));
		iVal >>=6;
		i4Val = _mm_slli_si128(i4Val, 4);
		i4Val = _mm_or_si128(i4Val, _mm_cvtsi32_si128(iVal & 0x1F));
		i4Val = cvt_from_unorm(i4Val, f4unorm565div);
		break;
	case CLK_UNORM_SHORT_555:
		iVal = *(unsigned short*)pPxl;
		i4Val = _mm_cvtsi32_si128(iVal & 0x1F);
		iVal >>=5;
		i4Val = _mm_slli_si128(i4Val, 4);
		i4Val = _mm_or_si128(i4Val, _mm_cvtsi32_si128(iVal & 0x1F));
		iVal >>=5;
		i4Val = _mm_slli_si128(i4Val, 4);
		i4Val = _mm_or_si128(i4Val, _mm_cvtsi32_si128(iVal & 0x1F));
		i4Val = cvt_from_unorm(i4Val, f4unorm555div);
		break;
	case CLK_UNORM_INT_101010:
		iVal = *(unsigned int*)pPxl;
		i4Val = _mm_cvtsi32_si128(iVal & 0x3FF);
		iVal >>=10;
		i4Val = _mm_slli_si128(i4Val, 4);
		i4Val = _mm_or_si128(i4Val, _mm_cvtsi32_si128(iVal & 0x3FF));
		iVal >>=10;
		i4Val = _mm_slli_si128(i4Val, 4);
		i4Val = _mm_or_si128(i4Val, _mm_cvtsi32_si128(iVal & 0x3FF));
		i4Val = cvt_from_unorm(i4Val, f4unorm101010div);
		break;
	case CLK_UNSIGNED_INT16:
		i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPxl);
		i4Val = _mm_slli_si128(i4Val, 2);
		i4Val = _mm_or_si128(i4Val, _mm_cvtsi32_si128(*((unsigned short*)pPxl+2)));
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// no extend sign
		break;
	case CLK_HALF_FLOAT:
		i4Val = (__m128i)Half4ToFloat4(_mm_loadl_epi64((__m128i*)pPxl));
		break;
	case CLK_SIGNED_INT32:
	case CLK_UNSIGNED_INT32:
	case CLK_FLOAT:
		i4Val = _mm_loadl_epi64((__m128i*)pPxl);
		i4Val = _mm_slli_si128(i4Val, 4);
		i4Val = _mm_or_si128(i4Val, _mm_cvtsi32_si128(*((unsigned int*)pPxl+2)));
		break;
	}

	return i4Val;
}

// Load pixels that are constructed from two channel
_4i32 LoadQuadChannel(void *pPxl, cl_channel_type type)
{
	__m128i	i4Val;
	switch (type)
	{
	case CLK_SNORM_INT8:
		i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPxl);
		i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// Extend sign
		i4Val = _mm_slli_si128(i4Val, 3);
		i4Val = _mm_srai_epi32(i4Val, 24);
		i4Val = cvt_from_snorm(i4Val, f4snorm8div);
		break;
	case CLK_SIGNED_INT8:
		i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPxl);
		i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// Extend sign
		i4Val = _mm_slli_si128(i4Val, 3);
		i4Val = _mm_srai_epi32(i4Val, 24);
		break;
	case CLK_UNORM_INT8:
		i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPxl);
		i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// No sign extend
		i4Val = cvt_from_unorm(i4Val, f4unorm8div);
		break;
	case CLK_UNSIGNED_INT8:
		i4Val = _mm_cvtsi32_si128(*(unsigned int*)pPxl);
		i4Val = _mm_unpacklo_epi8(i4Val, _mm_setzero_si128());
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// No sign extend
		break;
	case CLK_SNORM_INT16:
		i4Val = _mm_loadl_epi64((__m128i*)pPxl);
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// Extend sign
		i4Val = _mm_slli_si128(i4Val, 2);
		i4Val = _mm_srai_epi32(i4Val, 16);
		i4Val = cvt_from_snorm(i4Val, f4snorm16div);
		break;
	case CLK_SIGNED_INT16:
		i4Val = _mm_loadl_epi64((__m128i*)pPxl);
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// Extend sign
		i4Val = _mm_slli_si128(i4Val, 2);
		i4Val = _mm_srai_epi32(i4Val, 16);
		break;
	case CLK_UNORM_INT16:
		i4Val = _mm_loadl_epi64((__m128i*)pPxl);
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// no extend sign
		i4Val = cvt_from_unorm(i4Val, f4unorm16div);
		break;
	case CLK_UNSIGNED_INT16:
		i4Val = _mm_loadl_epi64((__m128i*)pPxl);
		i4Val = _mm_unpacklo_epi16(i4Val, _mm_setzero_si128());
		// no extend sign
		break;
	case CLK_HALF_FLOAT:
		i4Val = (__m128i)Half4ToFloat4(_mm_loadl_epi64((__m128i*)pPxl));
		break;

	case CLK_SIGNED_INT32:
	case CLK_UNSIGNED_INT32:
	case CLK_FLOAT:
		// Assume 16 byte aligned
		i4Val = _mm_load_si128((__m128i*)pPxl);
		break;
	}

	return i4Val;
}

//------------------------------------------------------------------------
// Return border color
__forceinline _4i32 BoorderColorI(cl_image_format fmt)
{
	_4i32 i4Val;

	switch ( fmt.image_channel_order )
	{
	case CLK_A:
	case CLK_INTENSITY:
	case CLK_RA:
	case CLK_ARGB:
	case CLK_BGRA:
	case CLK_RGBA:
		return _mm_setzero_si128();
	default:
		return getBorder(fmt.image_channel_data_type);
	}
}


//------------------------------------------------------------------------
// Store functions
void StorePixel(void* pPxl, _4i32 i4Val, cl_image_format fmt)
{
	cl_uint type = fmt.image_channel_data_type;
	switch (fmt.image_channel_order)
	{
	case CLK_R:case CLK_LUMINANCE:case CLK_INTENSITY:
		StoreSingleChannel(pPxl, i4Val, type);
		break;
	case CLK_A:
		// Move Alpha channel to lowest DWORD
		i4Val = _mm_srli_si128(i4Val, 12);
		StoreSingleChannel(pPxl, i4Val, type);
		break;
	case CLK_RG:
		StoreDualChannel(pPxl, i4Val, type);
		break;
	case CLK_RA:
		// Move Alpha channel to be ajucent to R
		i4Val = _mm_shuffle_epi32(i4Val, _MM_SHUFFLE(0, 0, 3, 0));
		StoreDualChannel(pPxl, i4Val, type);
		break;
	case CLK_RGB:
		StoreTripleChannel(pPxl, i4Val, type);
		break;
	case CLK_RGBA:
		StoreQuadChannel(pPxl, i4Val, type);
		break;
	case CLK_BGRA:
		i4Val = _mm_shuffle_epi32(i4Val, _MM_SHUFFLE(3, 0, 1, 2));
		StoreQuadChannel(pPxl, i4Val, type);
		break;
	case CLK_ARGB:
		// Exchange bewteen A,R,G & B channels
		i4Val = _mm_shuffle_epi32(i4Val, _MM_SHUFFLE(2, 1, 0, 3));
		StoreQuadChannel(pPxl, i4Val, type);
		break;
	}
}

// Sore pixels that are constructed from single channel
void StoreSingleChannel(void *pPxl, _4i32 i4Val, cl_channel_type type)
{
	switch (type)
	{
	case CLK_SNORM_INT8:
		i4Val = cvt_to_norm(i4Val, f4snorm8mul, f4snorm8lim);
		*(char*)pPxl = (char)_mm_cvtsi128_si32(i4Val);
		break;

	case CLK_SIGNED_INT8:
#ifdef __SSE4_1__
		i4Val = _mm_max_epi32(i4Val, i4int8Min);
		i4Val = _mm_min_epi32(i4Val, i4int8Max);
		*(char*)pPxl = (char)_mm_cvtsi128_si32(i4Val);
#else
		*((char*)pPxl) = (char)(
			min(max(_mm_cvtsi128_si32(i4Val), CHAR_MIN), CHAR_MAX));
#endif
		break;

	case CLK_UNORM_INT8:
		i4Val = cvt_to_norm(i4Val, f4unorm8mul, f4unorm8lim);
		*(unsigned char*)pPxl = (unsigned char)_mm_cvtsi128_si32(i4Val);
		break;

	case CLK_UNSIGNED_INT8:
		i4Val = _mm_min_epu32(i4Val, i4uint8Max);
		*(unsigned char*)pPxl = (unsigned char)_mm_cvtsi128_si32(i4Val);
		break;

	case CLK_SNORM_INT16:
		i4Val = cvt_to_norm(i4Val, f4snorm16mul,f4snorm16lim);
		*((short*)pPxl) = (short)_mm_cvtsi128_si32(i4Val);
		break;

	case CLK_SIGNED_INT16:
#ifdef __SSE4_1__
		i4Val = _mm_max_epi32(i4Val, i4int16Min);
		i4Val = _mm_min_epi32(i4Val, i4int16Max);
		*((short*)pPxl) = (short)_mm_cvtsi128_si32(i4Val);
#else
		*((short*)pPxl) = (short)(
			min(max(_mm_cvtsi128_si32(i4Val), SHRT_MIN), SHRT_MAX));
#endif
		break;

	case CLK_UNORM_INT16:
		i4Val = cvt_to_norm(i4Val, f4unorm16mul, f4unorm16lim);
		*((short*)pPxl) = (short)_mm_cvtsi128_si32(i4Val);
		break;

	case CLK_UNSIGNED_INT16:
#ifdef __SSE4_1__
		i4Val = _mm_min_epu32(i4Val, i4uint16Max);
		*((short*)pPxl) = (short)_mm_cvtsi128_si32(i4Val);
#else
		*((short*)pPxl) = (short)(min(_mm_cvtsi128_si32(i4Val), USHRT_MAX));
#endif
		break;

	case CLK_HALF_FLOAT:
		// Conversion to half 
		i4Val = _mm_cvtps_ph((__m128)i4Val, 0);
		*((short*)pPxl) = (short)_mm_cvtsi128_si32(i4Val);
		break;

	case CLK_SIGNED_INT32:
	case CLK_UNSIGNED_INT32:
	case CLK_FLOAT:
		*((int*)pPxl) = _mm_cvtsi128_si32(i4Val);
		break;
	}

}

// Store pixels that are constructed from two channel
void StoreDualChannel(void *pPxl, _4i32 i4Val, cl_channel_type type)
{
#ifndef __SSE4_1__
	__m128      f4Val;
#endif
	switch (type)
	{
	case CLK_SNORM_INT8:
		i4Val = cvt_to_norm(i4Val, f4snorm8mul, f4snorm8lim);
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i8mask);
#else
		i4Val = _mm_packs_epi32(i4Val, i4Val);
		i4Val = _mm_packs_epi16(i4Val, i4Val);
#endif
		*(unsigned short*)pPxl = (unsigned short)_mm_cvtsi128_si32(i4Val);
		break;

	case CLK_SIGNED_INT8:
#ifdef __SSE4_1__
		i4Val = _mm_max_epi32(i4Val, i4int8Min);
		i4Val = _mm_min_epi32(i4Val, i4int8Max);
#else
		f4Val = _mm_cvtepi32_ps(i4Val);
		f4Val = _mm_max_ps(f4Val, f4snorm8lim);
		f4Val = _mm_min_ps(f4Val, f4snorm8mul);
		i4Val = _mm_cvtps_epi32(f4Val);
#endif
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i8mask);
#else
		i4Val = _mm_packs_epi32(i4Val, i4Val);
		i4Val = _mm_packs_epi16(i4Val, i4Val);
#endif
		*(unsigned short*)pPxl = (unsigned short)_mm_cvtsi128_si32(i4Val);
		break;

	case CLK_UNORM_INT8:
		i4Val = cvt_to_norm(i4Val, f4unorm8mul, f4unorm8lim);
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i8mask);
		*(unsigned short*)pPxl = (unsigned short)_mm_cvtsi128_si32(i4Val);
#else
		*(unsigned char*)pPxl = (unsigned char)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned char*)pPxl+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
#endif
		break;

	case CLK_UNSIGNED_INT8:
#ifdef __SSE4_1__
		i4Val = _mm_min_epu32(i4Val, i4uint8Max);
#else
		f4Val = _mm_cvtepi32_ps(i4Val);
		f4Val = _mm_min_ps(f4Val, f4unorm8mul);
		i4Val = _mm_cvtps_epi32(f4Val);
#endif
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i8mask);
		*(unsigned short*)pPxl = (unsigned short)_mm_cvtsi128_si32(i4Val);
#else
		*(unsigned char*)pPxl = (unsigned char)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned char*)pPxl+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
#endif
		break;

	case CLK_SNORM_INT16:
		i4Val = cvt_to_norm(i4Val, f4snorm16mul,f4snorm16lim);
		i4Val = _mm_packs_epi32(i4Val, i4Val);
		*(unsigned int*)pPxl = _mm_cvtsi128_si32(i4Val);
		break;

	case CLK_SIGNED_INT16:
#ifdef __SSE4_1__
		i4Val = _mm_max_epi32(i4Val, i4int16Min);
		i4Val = _mm_min_epi32(i4Val, i4int16Max);
#else
		f4Val = _mm_cvtepi32_ps(i4Val);
		f4Val = _mm_max_ps(f4Val, f4snorm16lim);
		f4Val = _mm_min_ps(f4Val, f4snorm16mul);
		i4Val = _mm_cvtps_epi32(f4Val);
#endif
		i4Val = _mm_packs_epi32(i4Val, i4Val);
		*(unsigned int*)pPxl = _mm_cvtsi128_si32(i4Val);
		break;


	case CLK_UNORM_INT16:
		i4Val = cvt_to_norm(i4Val, f4unorm16mul, f4unorm16lim);
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i16mask);
		*(unsigned int*)pPxl = _mm_cvtsi128_si32(i4Val);
#else
		*(unsigned short*)pPxl = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
#endif
		break;

	case CLK_UNSIGNED_INT16:
#ifdef __SSE4_1__
		i4Val = _mm_min_epu32(i4Val, i4uint16Max);
		i4Val = _mm_shuffle_epi8(i4Val, i4i16mask);
		*(unsigned int*)pPxl = _mm_cvtsi128_si32(i4Val);
#else
		*((unsigned short*)pPxl) = (unsigned short)(min(_mm_cvtsi128_si32(i4Val), USHRT_MAX));
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+1) = (unsigned short)(min(_mm_cvtsi128_si32(i4Val), USHRT_MAX));
#endif
		break;

	case CLK_HALF_FLOAT:
		// convertion to half
		i4Val = _mm_cvtps_ph((__m128)i4Val, 0);
#ifdef __SSSE3__
		*(unsigned int*)pPxl = _mm_cvtsi128_si32(i4Val);
#else
		*(unsigned short*)pPxl = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
#endif
		break;

	case CLK_SIGNED_INT32:
	case CLK_UNSIGNED_INT32:
	case CLK_FLOAT:
		_mm_storel_epi64((__m128i*)pPxl, i4Val);
		break;
	}
}

// Store pixels that are constructed from three channels
void StoreTripleChannel(void *pPxl, _4i32 i4Val, cl_channel_type type)
{
	int         iVal;
#ifndef __SSE4_1__
	__m128      f4Val;
#endif
	switch (type)
	{
	case CLK_SNORM_INT8:
		i4Val = cvt_to_norm(i4Val, f4snorm8mul, f4snorm8lim);
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i8mask);
#else
		i4Val = _mm_packs_epi32(i4Val, i4Val);
		i4Val = _mm_packs_epi16(i4Val, i4Val);
#endif
		iVal = _mm_cvtsi128_si32(i4Val);
		*(unsigned short*)pPxl = (unsigned short)iVal;
		*((unsigned char*)pPxl+2) = (unsigned char)(iVal >> 16);
		break;

	case CLK_SIGNED_INT8:
#ifdef __SSE4_1__
		i4Val = _mm_max_epi32(i4Val, i4int8Min);
		i4Val = _mm_min_epi32(i4Val, i4int8Max);
#else
		f4Val = _mm_cvtepi32_ps(i4Val);
		f4Val = _mm_max_ps(f4Val, f4snorm8lim);
		f4Val = _mm_min_ps(f4Val, f4snorm8mul);
		i4Val = _mm_cvtps_epi32(f4Val);
#endif
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i8mask);
#else
		i4Val = _mm_packs_epi32(i4Val, i4Val);
		i4Val = _mm_packs_epi16(i4Val, i4Val);
#endif
		iVal = _mm_cvtsi128_si32(i4Val);
		*(unsigned short*)pPxl = (unsigned short)iVal;
		*((unsigned char*)pPxl+2) = (unsigned char)(iVal >> 16);
		break;

	case CLK_UNORM_INT8:
		i4Val = cvt_to_norm(i4Val, f4unorm8mul, f4unorm8lim);
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i8mask);
		iVal = _mm_cvtsi128_si32(i4Val);
		*(unsigned short*)pPxl = (unsigned short)iVal;
		*((unsigned char*)pPxl+2) = (unsigned char)(iVal >> 16);
#else
		*(unsigned char*)pPxl = (unsigned char)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned char*)pPxl+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned char*)pPxl+2) = (unsigned char)_mm_cvtsi128_si32(i4Val);
#endif
		break;

	case CLK_UNSIGNED_INT8:
#ifdef __SSE4_1__
		i4Val = _mm_min_epu32(i4Val, i4uint8Max);
#else
		f4Val = _mm_cvtepi32_ps(i4Val);
		f4Val = _mm_min_ps(f4Val, f4unorm8mul);
		i4Val = _mm_cvtps_epi32(f4Val);
#endif
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i8mask);
		iVal = _mm_cvtsi128_si32(i4Val);
		*(unsigned short*)pPxl = (unsigned short)iVal;
		*((unsigned char*)pPxl+2) = (unsigned char)(iVal >> 16);
#else
		*(unsigned char*)pPxl = (unsigned char)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned char*)pPxl+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned char*)pPxl+2) = (unsigned char)_mm_cvtsi128_si32(i4Val);
#endif
		break;

	case CLK_SNORM_INT16:
		i4Val = cvt_to_norm(i4Val, f4snorm16mul, f4snorm16lim);
		// Shrink to 16bit
		i4Val = _mm_packs_epi32(i4Val, i4Val);
		*(unsigned int*)pPxl = _mm_cvtsi128_si32(i4Val);
		*((short*)pPxl+2) = _mm_extract_epi16(i4Val, 3);
		break;

	case CLK_SIGNED_INT16:
		// Shrink to 16bit
#ifdef __SSE4_1__
		i4Val = _mm_max_epi32(i4Val, i4int16Min);
		i4Val = _mm_min_epi32(i4Val, i4int16Max);
#else
		f4Val = _mm_cvtepi32_ps(i4Val);
		f4Val = _mm_max_ps(f4Val, f4snorm16lim);
		f4Val = _mm_min_ps(f4Val, f4snorm16mul);
		i4Val = _mm_cvtps_epi32(f4Val);
#endif
		i4Val = _mm_packs_epi32(i4Val, i4Val);
		*(unsigned int*)pPxl = _mm_cvtsi128_si32(i4Val);
		*((short*)pPxl+2) = _mm_extract_epi16(i4Val, 3);
		break;

	case CLK_UNORM_INT16:
		i4Val = cvt_to_norm(i4Val, f4unorm16mul, f4unorm16lim);
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i16mask);
		*(unsigned int*)pPxl = _mm_cvtsi128_si32(i4Val);
		*((unsigned short*)pPxl+2) = _mm_extract_epi16(i4Val, 3);
#else
		*(unsigned short*)pPxl = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+2) = (unsigned short)_mm_cvtsi128_si32(i4Val);
#endif
		break;
	case CLK_UNSIGNED_INT16:
#ifdef __SSE4_1__
		i4Val = _mm_min_epu32(i4Val, i4uint16Max);
#else
		f4Val = _mm_cvtepi32_ps(i4Val);
		f4Val = _mm_min_ps(f4Val, f4snorm16mul);
		i4Val = _mm_cvtps_epi32(f4Val);
#endif
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i16mask);
		*(unsigned int*)pPxl = _mm_cvtsi128_si32(i4Val);
		*((unsigned short*)pPxl+2) = _mm_extract_epi16(i4Val, 3);
#else
		*(unsigned short*)pPxl = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+2) = (unsigned short)_mm_cvtsi128_si32(i4Val);
#endif
		break;

	case CLK_UNORM_SHORT_565:
		i4Val = cvt_to_norm(i4Val, f4unorm565mul, f4unorm565lim);
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i8mask);
		iVal = _mm_cvtsi128_si32(i4Val);
		iVal = (iVal & 0x1F) | ((iVal & 0x3F00)>>3) | ((iVal & 0x1F0000)>>5);
#else
		iVal = ((unsigned char)_mm_cvtsi128_si32(i4Val) & 0x1F) << 11;
		i4Val = _mm_srli_si128(i4Val, 4);
		iVal |= ((unsigned char)_mm_cvtsi128_si32(i4Val) & 0x3F) << 5;
		i4Val = _mm_srli_si128(i4Val, 4);
		iVal |= ((unsigned char)_mm_cvtsi128_si32(i4Val) & 0x1F);
#endif
		*(unsigned short*)pPxl = iVal;
		break;

	case CLK_UNORM_SHORT_555:
		i4Val = cvt_to_norm(i4Val, f4unorm555mul, f4unorm555lim);
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i8mask);
		iVal = _mm_cvtsi128_si32(i4Val);
		iVal = (iVal & 0x1F) | ((iVal & 0x1F00)>>3) | ((iVal & 0x1F0000)>>6);
#else
		iVal = ((unsigned char)_mm_cvtsi128_si32(i4Val) & 0x1F) << 10;
		i4Val = _mm_srli_si128(i4Val, 4);
		iVal |= ((unsigned char)_mm_cvtsi128_si32(i4Val) & 0x1F) << 5;
		i4Val = _mm_srli_si128(i4Val, 4);
		iVal |= ((unsigned char)_mm_cvtsi128_si32(i4Val) & 0x1F);
#endif
		*(unsigned short*)pPxl = iVal;
		break;

	case CLK_UNORM_INT_101010:
		i4Val = cvt_to_norm(i4Val, f4unorm101010mul, f4unorm101010lim);
		iVal = (_mm_cvtsi128_si32(i4Val) & 0x3FF)<<20;
		i4Val = _mm_srli_si128(i4Val, 4);
		iVal |= (_mm_cvtsi128_si32(i4Val) & 0x3FF)<<10;
		i4Val = _mm_srli_si128(i4Val, 4);
		iVal |= (_mm_cvtsi128_si32(i4Val) & 0x3FF);
		*(unsigned int*)pPxl = iVal;
		break;

	case CLK_HALF_FLOAT:
		// convertion to half
		i4Val = _mm_cvtps_ph((__m128)i4Val, 0);
#ifdef __SSSE3__
		*(unsigned int*)pPxl = _mm_cvtsi128_si32(i4Val);
		*((unsigned short*)pPxl+2) = _mm_extract_epi16(i4Val, 3);
#else
		*(unsigned short*)pPxl = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+2) = (unsigned short)_mm_cvtsi128_si32(i4Val);
#endif

		break;

	case CLK_SIGNED_INT32:
	case CLK_UNSIGNED_INT32:
	case CLK_FLOAT:
		*((unsigned int*)pPxl+2) = _mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		_mm_storel_epi64((__m128i*)pPxl, i4Val);
		break;
	}
}

// Load pixels that are constructed from two channel
void StoreQuadChannel(void *pPxl, _4i32 i4Val, cl_channel_type type)
{
#ifndef __SSE4_1__
	__m128      f4Val;
#endif

	switch (type)
	{
	case CLK_SNORM_INT8:
		i4Val = cvt_to_norm(i4Val, f4snorm8mul, f4snorm8lim);
		i4Val = _mm_packs_epi32(i4Val, i4Val);
		i4Val = _mm_packs_epi16(i4Val, i4Val);
		*(unsigned int*)pPxl = _mm_cvtsi128_si32(i4Val);
		break;

	case CLK_SIGNED_INT8:
#ifdef __SSE4_1__
		i4Val = _mm_max_epi32(i4Val, i4int16Min);
		i4Val = _mm_min_epi32(i4Val, i4int16Max);
#else
		f4Val = _mm_cvtepi32_ps(i4Val);
		f4Val = _mm_max_ps(f4Val, f4snorm8lim);
		f4Val = _mm_min_ps(f4Val, f4snorm8mul);
		i4Val = _mm_cvtps_epi32(f4Val);
#endif
		i4Val = _mm_packs_epi32(i4Val, i4Val);
		i4Val = _mm_packs_epi16(i4Val, i4Val);
		*(unsigned int*)pPxl = _mm_cvtsi128_si32(i4Val);
		break;

	case CLK_UNORM_INT8:
		i4Val = cvt_to_norm(i4Val, f4unorm8mul, f4unorm8lim);
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i8mask);
		*(unsigned int*)pPxl = _mm_cvtsi128_si32(i4Val);
#else
		*(unsigned char*)pPxl = (unsigned char)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned char*)pPxl+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned char*)pPxl+2) = (unsigned char)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned char*)pPxl+3) = (unsigned char)_mm_cvtsi128_si32(i4Val);
#endif
		break;
	case CLK_UNSIGNED_INT8:
#ifdef __SSE4_1__
		i4Val = _mm_min_epi32(i4Val, i4uint8Max);
#else
		f4Val = _mm_cvtepi32_ps(i4Val);
		f4Val = _mm_min_ps(f4Val, f4unorm8mul);
		i4Val = _mm_cvtps_epi32(f4Val);
#endif
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i8mask);
		*(unsigned int*)pPxl = _mm_cvtsi128_si32(i4Val);
#else
		*(unsigned char*)pPxl = (unsigned char)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned char*)pPxl+1) = (unsigned char)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned char*)pPxl+2) = (unsigned char)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned char*)pPxl+3) = (unsigned char)_mm_cvtsi128_si32(i4Val);
#endif
		break;

	case CLK_SNORM_INT16:
		i4Val = cvt_to_norm(i4Val, f4snorm16mul, f4snorm16lim);
		// Shrink to 8bit
		i4Val = _mm_packs_epi32(i4Val, i4Val);
		_mm_storel_epi64((__m128i*)pPxl, i4Val);
		break;

	case CLK_SIGNED_INT16:
#ifdef __SSE4_1__
		i4Val = _mm_max_epi32(i4Val, i4int16Min);
		i4Val = _mm_min_epi32(i4Val, i4int16Max);
#else
		f4Val = _mm_cvtepi32_ps(i4Val);
		f4Val = _mm_max_ps(f4Val, f4snorm16lim);
		f4Val = _mm_min_ps(f4Val, f4snorm16mul);
		i4Val = _mm_cvtps_epi32(f4Val);
#endif
		// Shrink to 8bit
		i4Val = _mm_packs_epi32(i4Val, i4Val);
		_mm_storel_epi64((__m128i*)pPxl, i4Val);
		break;

	case CLK_UNORM_INT16:
		i4Val = cvt_to_norm(i4Val, f4unorm16mul, f4unorm16lim);
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i16mask);
		_mm_storel_epi64((__m128i*)pPxl, i4Val);
#else
		// TODO: Use pshub
		*(unsigned short*)pPxl = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+2) = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+3) = (unsigned short)_mm_cvtsi128_si32(i4Val);
#endif
		break;
	case CLK_UNSIGNED_INT16:
#ifdef __SSE4_1__
		i4Val = _mm_min_epi32(i4Val, i4uint16Max);
#else
		f4Val = _mm_cvtepi32_ps(i4Val);
		f4Val = _mm_max_ps(f4Val, f4unorm16lim);
		f4Val = _mm_min_ps(f4Val, f4unorm16mul);
		i4Val = _mm_cvtps_epi32(f4Val);
#endif
#ifdef __SSSE3__
		i4Val = _mm_shuffle_epi8(i4Val, i4i16mask);
		_mm_storel_epi64((__m128i*)pPxl, i4Val);
#else
		*(unsigned short*)pPxl = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+2) = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+3) = (unsigned short)_mm_cvtsi128_si32(i4Val);
#endif
		break;

	case CLK_HALF_FLOAT:
		// convertion to half
		i4Val = _mm_cvtps_ph((__m128)i4Val, 0);
#ifdef __SSSE3__
		_mm_storel_epi64((__m128i*)pPxl, i4Val);
#else
		*(unsigned short*)pPxl = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+1) = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+2) = (unsigned short)_mm_cvtsi128_si32(i4Val);
		i4Val = _mm_srli_si128(i4Val, 4);
		*((unsigned short*)pPxl+3) = (unsigned short)_mm_cvtsi128_si32(i4Val);
#endif
		break;

	case CLK_SIGNED_INT32:
	case CLK_UNSIGNED_INT32:
	case CLK_FLOAT:
		// Assume 16 byte aligned
		_mm_store_si128((__m128i*)pPxl, i4Val);
		break;
	}
}

#else // _MSC_VER


#endif
#ifdef __cplusplus
}
#endif
