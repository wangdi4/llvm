// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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

#define CBK_ARRAY_SIZE 64

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
	
	void*			coord_translate_f_callback[CBK_ARRAY_SIZE];    //the list of float coordinate translation callback
	void*			read_img_callback_int[CBK_ARRAY_SIZE];   // the list of integer image reader & filter callbacks
	void*			read_img_callback_float[CBK_ARRAY_SIZE]; // the list of float   image reader & filter callbacks
	void*			soa4_read_img_callback_int[CBK_ARRAY_SIZE]; // the list of soa4 integer image reader & filter callbacks
	void*			soa8_read_img_callback_int[CBK_ARRAY_SIZE]; // the list of soa8 integer image reader & filter callbacks
	void*			soa16_read_img_callback_int[CBK_ARRAY_SIZE]; // the list of soa16 integer image reader & filter callbacks
	void*			write_img_callback;    // the write image sampler callback
	void*			soa4_write_img_callback;    // the write image sampler callback
	void*			soa8_write_img_callback;    // the write image sampler callback
	void*			soa16_write_img_callback;   // the write image sampler callback

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
    // ND Range Work Description
    // Kernel explicit arguments in same order as in  kernel declaration
    // Alignment of type must be same as sizeof returns on the type
    //gentype arg1;
    //gentype arg2;
    // .  .  .
    //gentype argN;
    // Kernel implicit arguments continue here
    size_t    WorkDim;                                 // Filled by the runtime
    size_t    GlobalOffset[MAX_WORK_DIM];              // Filled by the runtime
    size_t    GlobalSize[MAX_WORK_DIM];                // Filled by the runtime
    size_t    LocalSize[WG_SIZE_NUM][MAX_WORK_DIM];    // Filled by the runtime, updated by the BE in case of (0,0,0)
                                                       // LocalSize[0] contains unifrom local sizes
                                                       // LocalSize[1] contains non-unifrom local sizes
    size_t    WGCount[MAX_WORK_DIM];                   // Updated by the BE, based on GLOBAL/LOCAL
    // For Opencl2.0: this is a IDeviceCommandManager: the printf interface thing
    void*     RuntimeInterface;                      // Updated by runtime
    /// reference to BlockToKernelMapper object. Class does not own it
    void*     Block2KernelMapper;                      // Updated by the BE
    size_t    minWorkGroupNum;                         // Filled by the runtime, Required by the heuristic
    // Internal for Running the kernel
    const void *pUniformJITEntryPoint;                 // Filled by the BE
    const void *pNonUniformJITEntryPoint;              // Filled by the BE
} PACKED cl_uniform_kernel_args;

#ifdef _WIN32
#pragma pack(pop)
#endif 

/*! \enum cl_dev_sampler_prop
 * Defines possible values of the kernel information that could be retrieved by clDevGetKernelInfo function.
 */
enum cl_dev_sampler_prop
{
    // address
    CLK_ADDRESS_NONE                =0,
    CLK_ADDRESS_CLAMP_TO_EDGE       =2,
    CLK_ADDRESS_CLAMP               =4,
    CLK_ADDRESS_REPEAT              =6,
    CLK_ADDRESS_MIRRORED_REPEAT     =8,

    // normalized
    CLK_NORMALIZED_COORDS_FALSE     =0,
    CLK_NORMALIZED_COORDS_TRUE      =1,

    // filter
    CLK_FILTER_NEAREST              =0x10,
    CLK_FILTER_LINEAR               =0x20
};
// masks that allow to define the type of sampler
// (address, normalized, filter)
#include "cl_sampler_mask.h"

// Channel order, numbering must be aligned with cl_channel_order in cl.h
enum {
    CLK_R                                        =0x10B0,
    CLK_A                                        =0x10B1,
    CLK_RG                                       =0x10B2,
    CLK_RA                                       =0x10B3,
    CLK_RGB                                      =0x10B4,
    CLK_RGBA                                     =0x10B5,
    CLK_BGRA                                     =0x10B6,
    CLK_ARGB                                     =0x10B7,
    CLK_INTENSITY                                =0x10B8,
    CLK_LUMINANCE                                =0x10B9,
    CLK_Rx                                       =0x10BA,
    CLK_RGx                                      =0x10BB,
    CLK_RGBx                                     =0x10BC,
    CLK_DEPTH                                    =0x10BD,
    CLK_DEPTH_STENCIL                            =0x10BE,
    // OpenCL2.0 image formats beyond SPIR 1.2 spec
    CLK_sRGB                                     =0x10BF,
    CLK_sRGBx                                    =0x10C0,
    CLK_sRGBA                                    =0x10C1,
    CLK_sBGRA                                    =0x10C2,
    CLK_ABGR                                     =0x10C3
};

// Channel Type
enum {
    CLK_SNORM_INT8          =0x10D0,
    CLK_SNORM_INT16         =0x10D1,
    CLK_UNORM_INT8          =0x10D2,
    CLK_UNORM_INT16         =0x10D3,
    CLK_UNORM_SHORT_565     =0x10D4,
    CLK_UNORM_SHORT_555     =0x10D5,
    CLK_UNORM_INT_101010    =0x10D6,
    CLK_SIGNED_INT8         =0x10D7,
    CLK_SIGNED_INT16        =0x10D8,
    CLK_SIGNED_INT32        =0x10D9,
    CLK_UNSIGNED_INT8       =0x10DA,
    CLK_UNSIGNED_INT16      =0x10DB,
    CLK_UNSIGNED_INT32      =0x10DC,
    CLK_HALF_FLOAT          =0x10DD,
    CLK_FLOAT               =0x10DE,
    CLK_UNORM_INT24         =0x10DF
};

typedef void*               queue_t;
typedef void*               clk_event_t;

// default is CLK_ENQUEUE_FLAGS_WAIT_KERNEL
typedef int                 kernel_enqueue_flags_t;
#define CLK_ENQUEUE_FLAGS_NO_WAIT                   (cl_uint)0
#define CLK_ENQUEUE_FLAGS_WAIT_KERNEL               (cl_uint)1
#define CLK_ENQUEUE_FLAGS_WAIT_WORK_GROUP           (cl_uint)2

enum clk_profiling_info
{
    CLK_PROFILING_COMMAND_EXEC_TIME = 1
};

#define CL_EVENT_ALLOCATION_FAILURE                 -100
#define CL_ENQUEUE_FAILURE                          -101
#define CL_INVALID_QUEUE                            -102
#define CL_INVALID_PIPE_SIZE                        -69
#define CL_INVALID_DEVICE_QUEUE                     -70

#include "cl_types2.h"
