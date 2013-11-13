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
/**************************************************************************************************
 *  cl_types.h
 *  Created on: 10-Dec-2008 11:42:24 AM
 *  Implementation of the Class OpenCLFramework
 *  Original author: ulevy
 *************************************************************************************************/

#include "cl_device_api.h"
#include <tmmintrin.h>
#include <CL/cl.h>

// ToDo: remove on move to 1.2 spec ---------------- Start

typedef cl_uint cl_kernel_arg_info;
// ToDo: remove on move to 1.2 spec ---------------- End

/**************************************************************************************************
* cl_err_code
* initial data type which represents the return values inside the framework
**************************************************************************************************/
typedef cl_int	cl_err_code;

typedef void (CL_CALLBACK *logging_fn)(const char *, const void *, size_t, void *);
typedef void (CL_CALLBACK *mem_dtor_fn)(cl_mem, void *);
/**************************************************************************************************
* define widen string into multibyte
**************************************************************************************************/
#if defined (_WIN32)
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#else
#define WIDEN(x) x
#endif

#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef INOUT
#define INOUT
#endif

/**************************************************************************************************
* CL_SUCCEEDED
* Checks whether a return code is success
**************************************************************************************************/
#define CL_SUCCEEDED(code)         (CL_SUCCESS == (code))

/**************************************************************************************************
* CL_FAILED
* Checks whether a return code is failure
**************************************************************************************************/
#define CL_FAILED(code)				(CL_SUCCESS > (code))

/**************************************************************************************************
* CL_ERR_OUT
* filter internal error codes
**************************************************************************************************/
#define CL_ERR_OUT(code)			((code) <= CL_ERR_START) ? CL_ERR_FAILURE : (code)

#define CL_INVALID_HANDLE			0

/**************************************************************************************************
* internal error codes
**************************************************************************************************/
#define		CL_ERR_START					-2800	// marker
#define     CL_ERR_FAILURE                  CL_ERR_START
//////////////////////////////////////////////////////////////////////////
#define		CL_ERR_LOGGER_FAILED			-2801
#define		CL_ERR_NOT_IMPLEMENTED			-2802
#define		CL_ERR_NOT_SUPPORTED			-2803
#define		CL_ERR_INITILIZATION_FAILED		-2804
#define		CL_ERR_PLATFORM_FAILED			-2805
#define		CL_ERR_CONTEXT_FAILED			-2806
#define		CL_ERR_EXECUTION_FAILED			-2807
#define		CL_ERR_FILE_NOT_EXISTS			-2808
#define		CL_ERR_KEY_NOT_FOUND			-2809
#define		CL_ERR_KEY_ALLREADY_EXISTS		-2810
#define		CL_ERR_LIST_EMPTY				-2811
#define		CL_ERR_DEVICE_INIT_FAIL			-2850
#define		CL_ERR_FE_COMPILER_INIT_FAIL	-2851
#define		CL_ERR_CPU_NOT_SUPPORTED		-2852
//////////////////////////////////////////////////////////////////////////
#define		CL_ERR_END						-2899	// marker


#define CL_NOT_READY  0x8
#define CL_DONE_ON_RUNTIME  0x9
// cl_command_type (internal use)
#define CL_COMMAND_READ_MEM_OBJECT	 1500
#define	CL_COMMAND_WRITE_MEM_OBJECT  1501
#define	CL_COMMAND_FILL_MEM_OBJECT   1502

///////////////////////////////////////
// Memory runtime declaration
typedef struct _cl_mem_obj_descriptor
{
	cl_uint			dim_count;				// A number of dimensions in the memory object.
	union _dim_t
	{
		unsigned int	dim[MAX_WORK_DIM];		// Multi-dimensional size of the object.
		size_t			buffer_size;
	} dimensions;
	size_t			pitch[MAX_WORK_DIM-1];	// Multi-dimensional pitch of the object, valid only for images (2D/3D).
	cl_image_format	format;					// Format of the memory object,valid only for images (2D/3D).
	void*			pData;					// A pointer to the object wherein the object data is stored.
											// Could be a valid memory pointer or a handle to other object.
	unsigned		uiElementSize;			// Size of image pixel element.
	void*			imageAuxData;			//auxilary data kept for the image purposes
    cl_mem_object_type memObjType;          // type of the memory object
} cl_mem_obj_descriptor;

#ifdef WIN32
#define ALIGN16 __declspec(align(16))
#else
#define ALIGN16 __attribute__((aligned(16)))
#endif
// Explicitly define image types
typedef ALIGN16 struct _image_aux_data
{
    cl_uint			dim_count;				// A number of dimensions in the memory object.
	size_t			pitch[MAX_WORK_DIM-1];	// Multi-dimensional pitch of the object, valid only for images (2D/3D).
	cl_image_format	format;					// Format of the memory object,valid only for images (2D/3D).
	void*			pData;					// A pointer to the object wherein the object data is stored.
											// Could be a valid memory pointer or a handle to other object.
	unsigned		uiElementSize;			// Size of image pixel element.
	
	void*			coord_translate_f_callback[32];    //the list of float coordinate translation callback
	void*			read_img_callback_int[32];   // the list of integer image reader & filter callbacks
	void*			read_img_callback_float[32]; // the list of float   image reader & filter callbacks
	void*			soa4_read_img_callback_int[32]; // the list of soa4 integer image reader & filter callbacks
	void*			soa8_read_img_callback_int[32]; // the list of soa4 integer image reader & filter callbacks
	void*			write_img_callback;    // the write image sampler callback
	void*			soa4_write_img_callback;    // the write image sampler callback
	void*			soa8_write_img_callback;    // the write image sampler callback

	ALIGN16 int dimSub1[MAX_WORK_DIM+1];		// Image size for each dimension subtracted by one
												// Used to optimize coordinates computation not to subtract by one for each read
	ALIGN16 int dim[MAX_WORK_DIM+1];			// Image size for each dimension
	ALIGN16 unsigned int offset[MAX_WORK_DIM+1];// the offset to extract pixels
	ALIGN16 float dimf[MAX_WORK_DIM+1];			// Float image size for each dimension.
												// Used in coordinates computation to avoid
												// int->float type conversion for each read call
    int array_size;     // size of array for 1D and 2d array types, otherwise is set to -1
	int dimmask;		// Mask for dimensions in images
						// Contains ones at dim_count first bytes. Other bytes are zeros.
						// Used for coordinates clamping

} image_aux_data;

typedef image_aux_data* image2d_t;
typedef image_aux_data* image2d_depth_t;
typedef image_aux_data* image3d_t;
// Images 1.2 types
typedef image_aux_data* image1d_t;
typedef image_aux_data* image1d_buffer_t;
typedef image_aux_data* image1d_array_t;
typedef image_aux_data* image2d_array_t;
typedef image_aux_data* image2d_array_depth_t;


typedef struct _cl_llvm_prog_header
{
    // The header contains compiler build options
    bool bDisableOpt;
    bool bDebugInfo;
    bool bProfiling;
    bool bFastRelaxedMath;
    bool bDenormsAreZero;
    bool bEnableLinkOptions;
} cl_llvm_prog_header;

#ifdef _WIN32
#pragma pack(push, 1)
#define PACKED 
#else
#define PACKED __attribute__((packed)) 
#endif

/**
 * This struct hold all the uniform arguments which will be passed to the JIT
 * to start execution of OCL kernel
 */
typedef struct _cl_uniform_kernel_args {
    size_t WorkDim;
    size_t GlobalOffset[MAX_WORK_DIM];
    size_t GlobalSize[MAX_WORK_DIM];
    size_t LocalSize[MAX_WORK_DIM];
    size_t WGCount[MAX_WORK_DIM];
    size_t WGLoopIterCount;
    size_t *pRuntimeContext;
    // This buffer to be removed after localID buffer will be handled by BE
    size_t *pLocalIDIndices;
    // Kernel explicit arguments in same order as in  kernel declaration
    // Alignment of type must be same as sizeof returns on the type
    //gentype arg1;
    //gentype arg2;
    // .  .  .
    //gentype argN;
} PACKED cl_uniform_kernel_args;

#ifdef _WIN32
#pragma pack(pop)
#endif 

/*! \enum cl_dev_sampler_prop
 * Defines possible values of the kernel information that could be retrieved by clDevGetKernelInfo function.
 */
enum cl_dev_sampler_prop
{
    __ADDRESS_BASE								= 0,
    CL_DEV_SAMPLER_ADDRESS_NONE					= 0,
    CL_DEV_SAMPLER_ADDRESS_CLAMP				= 1 << __ADDRESS_BASE ,	//!< Sampler is defined with CLAMP attribute
    CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE		= 2 << __ADDRESS_BASE,	//!< Sampler is defined with CLAMP_TO_EDGE attribute
    CL_DEV_SAMPLER_ADDRESS_REPEAT				= 3 << __ADDRESS_BASE,	//!< Sampler is defined with REPEAT attribute
	CL_DEV_SAMPLER_ADDRESS_MIRRORED_REPEAT		= 4 << __ADDRESS_BASE,	//!< Sampler is defined with MIRRORED_REPEAT attribute
    __ADDRESS_BITS								= 3,					//!< number of bits required to represent address info
    __ADDRESS_MASK								= ( (1<<__ADDRESS_BITS) -1),

    __NORMALIZED_BASE							= __ADDRESS_BITS,
    CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE		= 0,						//!< Sampler is defined with normalized coordinates set to FALSE
    CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE		= 1 << __NORMALIZED_BASE,	//!< Sampler is defined with normalized coordinates set to TRUE
    __NORMALIZED_BITS							=	1,						//!< number of bits required to represent normalize coordinates selection 
    __NORMALIZED_MASK							= ( ((1<<__NORMALIZED_BITS)-1) << __NORMALIZED_BASE ),

    __FILTER_BASE								= __NORMALIZED_BASE + __NORMALIZED_BITS,
    CL_DEV_SAMPLER_FILTER_NEAREST				= 0 << __FILTER_BASE,		//!< Sampler is defined with filtering set to NEAREST
    CL_DEV_SAMPLER_FILTER_LINEAR				= 1 << __FILTER_BASE,		//!< Sampler is defined with filtering set to LINEAR
    __FILTER_BITS					            = 2,						//!< number of bits required to represent filter info
    __FILTER_MASK								= ( ((1<<__FILTER_BITS)-1) << __FILTER_BASE)
};

// Channel order, numbering must be aligned with cl_channel_order in cl.h
enum {
  CLK_R,
  CLK_A,
  CLK_RG,
  CLK_RA,
  CLK_RGB,
  CLK_RGBA,
  CLK_BGRA,
  CLK_ARGB,
  CLK_INTENSITY,
  CLK_LUMINANCE,
  CLK_DEPTH = 0xD,
  CLK_sRGBA = 0x11,
  CLK_sBGRA = 0x12
};

// Channel Type
enum {
  // valid formats for float return types
  CLK_SNORM_INT8,
  CLK_SNORM_INT16,
  CLK_UNORM_INT8,
  CLK_UNORM_INT16,
  CLK_UNORM_SHORT_565,
  CLK_UNORM_SHORT_555,
  CLK_UNORM_INT_101010,

  CLK_SIGNED_INT8,
  CLK_SIGNED_INT16,
  CLK_SIGNED_INT32,
  CLK_UNSIGNED_INT8,
  CLK_UNSIGNED_INT16,
  CLK_UNSIGNED_INT32,

  CLK_HALF_FLOAT,            // four channel RGBA half
  CLK_FLOAT                  // four channel RGBA float
};

#include "cl_types2.h"
