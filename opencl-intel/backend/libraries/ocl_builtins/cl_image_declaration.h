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

// !!!IMPORTANT!!! These defines should be the same as in ImageCallbackLibrary.h
#define NONE_FALSE_NEAREST 0x00
#define CLAMP_FALSE_NEAREST 0x01
#define CLAMPTOEDGE_FALSE_NEAREST 0x02
#define REPEAT_FALSE_NEAREST 0x03
#define MIRRORED_FALSE_NEAREST 0x04

#define NONE_TRUE_NEAREST 0x08
#define CLAMP_TRUE_NEAREST 0x09
#define CLAMPTOEDGE_TRUE_NEAREST 0x0a
#define REPEAT_TRUE_NEAREST 0x0b
#define MIRRORED_TRUE_NEAREST 0x0c

#define NONE_FALSE_LINEAR 0x10
#define CLAMP_FALSE_LINEAR 0x11
#define CLAMPTOEDGE_FALSE_LINEAR 0x12
#define REPEAT_FALSE_LINEAR 0x13
#define MIRRORED_FALSE_LINEAR 0x14

#define NONE_TRUE_LINEAR 0x18
#define CLAMP_TRUE_LINEAR 0x19
#define CLAMPTOEDGE_TRUE_LINEAR 0x1a
#define REPEAT_TRUE_LINEAR 0x1b
#define MIRRORED_TRUE_LINEAR 0x1c

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

// calback functions types definitions

// Coordinate callback typedefs
// coordinates callback for integer input
typedef int4 (*Image_I_COORD_CBK) (void*, int4);
// coordinates callback for float input
typedef int4 (*Image_F_COORD_CBK) (void*, float4);
/// coordinates callback should return coordinates of pixel and
/// i0,j0,k0,i1,k1,j1 components for interpolation
typedef float4 (*Image_FF_COORD_CBK) (void*, float4, int4*, int4*);

// Image reading callback typedefs
// Reading from uint32_t image
typedef uint4 (*Image_UI_READ_CBK) (void*, int4);
// Raeding from signed int image
typedef int4 (*Image_I_READ_CBK) (void*, int4);
// read callback for float images and float coordinates takes
// translated coordinates and i0,j0,k0,i1,j1,k1 components for interpolation
typedef float4 (*Image_F_READ_CBK) (void*, int4, int4, float4);
// read callback for float images and integer coordinates
typedef float4 (*Image_FI_READ_CBK) (void*, int4);

// Write image callback typedefs
typedef void (*Image_UI_WRITE_CBK) (void*, uint4);
typedef void (*Image_I_WRITE_CBK) (void*, int4);
typedef void (*Image_F_WRITE_CBK) (void*, float4);

#define ALIGN16 __attribute__ ((aligned(16)))
// Image description. Contains all data about image and required callback functions
// !!!DUPLICATE!!! Has duplicate function in cl_api/cl_types.h
typedef struct _image_aux_data
{
	uint			dim_count;				// A number of dimensions in the memory object.
	size_t			pitch[MAX_WORK_DIM-1];	// Multi-dimensional pitch of the object, valid only for images (2D/3D).
	cl_image_format_t	format;			// Format of the memory object,valid only for images (2D/3D).
										/* cl_image_format fields:
											unsigned int image_channel_order;
											unsigned int image_channel_data_type; */
	void*			pData;					// A pointer to the object wherein the object data is stored.
											// Could be a valid memory pointer or a handle to other object.
	unsigned		uiElementSize;			// Size of image pixel element.
	
	void*			coord_translate_i_callback[32];    //the list of integer coordinate translation callback
	void*			coord_translate_f_callback[32];    //the list of float coordinate translation callback
	void*			read_img_callback[32]; // the list of integer image reader & filter callbacks
	void*			write_img_callback;    // the write image sampler callback

	int dimSub1[MAX_WORK_DIM+1] ALIGN16;	// Image size for each dimension subtracted by one
											// Used to optimize coordinates computation not to subtract by one for each read
	int dim[MAX_WORK_DIM+1] ALIGN16;		// Image size for each dimension
	int offset[MAX_WORK_DIM+1] ALIGN16;		// the offset to extract pixels
	float dimf[MAX_WORK_DIM+1] ALIGN16;		// Float image size for each dimension.
											// Used in coordinates computation to avoid
											// int->float type conversion for each read call
	int dimmask;		// Mask for dimensions in images
						// Contains ones at dim_count first bytes. Other bytes are zeros.
						// Used for coordinates clamping

} image_aux_data;


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

#endif // _MSC_DEV

#ifdef __cplusplus
}
#endif
