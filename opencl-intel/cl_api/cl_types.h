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
#include <CL/cl_ext.h>

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
#define		CL_ERR_START					-800	// marker
#define     CL_ERR_FAILURE                  CL_ERR_START
//////////////////////////////////////////////////////////////////////////
#define		CL_ERR_LOGGER_FAILED			-801
#define		CL_ERR_NOT_IMPLEMENTED			-802
#define		CL_ERR_NOT_SUPPORTED			-803
#define		CL_ERR_INITILIZATION_FAILED		-804
#define		CL_ERR_PLATFORM_FAILED			-805
#define		CL_ERR_CONTEXT_FAILED			-806
#define		CL_ERR_EXECUTION_FAILED			-807
#define		CL_ERR_FILE_NOT_EXISTS			-808
#define		CL_ERR_KEY_NOT_FOUND			-809
#define		CL_ERR_KEY_ALLREADY_EXISTS		-810
#define		CL_ERR_LIST_EMPTY				-811
#define		CL_ERR_DEVICE_INIT_FAIL			-850
#define		CL_ERR_FE_COMPILER_INIT_FAIL	-851
#define		CL_ERR_CPU_NOT_SUPPORTED		-852
//////////////////////////////////////////////////////////////////////////
#define		CL_ERR_END						-899	// marker


#define CL_NOT_READY  0x8
#define CL_DONE_ON_RUNTIME  0x9
// cl_command_type (internal use)
#define CL_COMMAND_READ_MEM_OBJECT	 1500
#define	CL_COMMAND_WRITE_MEM_OBJECT  1501

///////////////////////////////////////
// Memory runtime declaration
typedef struct _cl_mem_obj_descriptor
{
	cl_uint			dim_count;				// A number of dimensions in the memory object.
	unsigned int	dim[MAX_WORK_DIM];		// Multi-dimentional size of the object.
	size_t			pitch[MAX_WORK_DIM-1];	// Multi-dimentional pitch of the object, valid only for images (2D/3D).
	cl_image_format	format;					// Format of the memory object,valid only for images (2D/3D).
	void*			pData;					// A pointer to the object wherein the object data is stored.
											// Could be a valid memory pointer or a handle to other object.
	unsigned		uiElementSize;			// Size of image pixel element.
} cl_mem_obj_descriptor;

typedef _cl_mem_obj_descriptor* image2d_t;
typedef _cl_mem_obj_descriptor* image3d_t;

typedef struct _cl_llvm_prog_header
{
	// The header contains compiler build options
	bool	bDisableOpt;
	bool	bDebugInfo;
	bool	bFastRelaxedMath;
	bool	bDemorsAreZero;
} cl_llvm_prog_header;

// Channel order, must match cl.h
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
  CLK_LUMINANCE
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
