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
///////////////////////////////////////////////////////////////////////////////////////////////////
//  FrameworkProxy.h
//  Implementation of the Class FrameworkProxy
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "cl_sys_defines.h"
#include "cl_linux_utils.h"
#include "cl_types.h"
#include <icd_dispatch.h>
#include <string>
#include <map>
#if defined (DX9_SHARING)
#include "CL/cl_d3d9.h"
#endif

#define PLATFORM_MODULE		FrameworkProxy::Instance()->GetPlatformModule()
#define CONTEXT_MODULE		FrameworkProxy::Instance()->GetContextModule()
#define EXECUTION_MODULE	FrameworkProxy::Instance()->GetExecutionModule()


struct _cl_object
{
	void * object;
};

struct _cl_platform_id_int : public _cl_platform_id
{
	void * object;
};

struct _cl_device_id_int : public _cl_device_id
{
	void * object;
};

struct _cl_context_int : public _cl_context
{
	void * object;
};

struct _cl_command_queue_int : public _cl_command_queue
{
	void * object;
};

struct _cl_mem_int : public _cl_mem
{
	void * object;
};

struct _cl_program_int : public _cl_program
{
	void * object;
};

struct _cl_kernel_int : public _cl_kernel
{
	void * object;
};

struct _cl_event_int : public _cl_event
{
	void * object;
};

struct _cl_sampler_int : public _cl_sampler
{
	void * object;
};

extern "C" int IsCPUSupported(void);

// SSE2   = 0x0001 (1)
// SSE3   = 0x0002 (2)
// SSSE3  = 0x0004 (4)
// SSE4.1 = 0x0008 (8)
// SSE4.2 = 0x0010 (16)
// AVX1.0 = 0x0020 (32)
extern "C" int IsFeatureSupported(int iCPUFeature);

// Define a static map for extension functions support
typedef std::map<std::string, void*> ExtensionFunctionAddressResolveMap;
extern ExtensionFunctionAddressResolveMap g_extFuncResolveMap;
extern void* RegisterExtensionFunctionAddress(const char* pFuncName, void* pFuncPtr);
#define REGISTER_EXTENSION_FUNCTION(__NAME__,__ADDRESS__) \
        static void* UNUSED(func##__ADDRESS__) = RegisterExtensionFunctionAddress(#__NAME__, (void*)(ptrdiff_t)GET_ALIAS(__ADDRESS__))


//----------------------------------------------------------------
/// ToDo: Remove on move to 1.2
//----------------------------------------------------------------

// We need C linkage since we are defining alias in linux.
#ifdef __cplusplus
extern "C" {
#endif

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetKernelArgInfo)(
        cl_kernel            kernel,
        cl_uint              arg_indx,
        cl_kernel_arg_info   param_name,
        size_t               param_value_size,
        void *               param_value,
        size_t *             param_value_size_ret);


extern cl_int CL_API_CALL clGetKernelArgInfo(
                                cl_kernel		kernel,
								cl_uint				arg_indx,
								cl_kernel_arg_info	param_name,
								size_t				param_value_size,
								void *				param_value,
								size_t *			param_value_size_ret);
#ifdef __cplusplus
}
#endif




//// ------------------------------------
//// vendor dispatch table structure
//
typedef struct: public KHRicdVendorDispatch
{             
       KHRpfn_clGetKernelArgInfo                    clGetKernelArgInfo;
        
#ifdef DX9_SHARING
       clGetDeviceIDsFromD3D9Intel_fn           clGetDeviceIDsFromD3D9INTEL;
       clCreateFromD3D9SurfaceIntel_fn          clCreateFromD3D9SurfaceINTEL;
       clEnqueueAcquireD3D9ObjectsIntel_fn      clEnqueueAcquireD3D9ObjectsINTEL;
       clEnqueueReleaseD3D9ObjectsIntel_fn      clEnqueueReleaseD3D9ObjectsINTEL;
#endif
    /// Add CPU specific Extra functions here
} ocl_entry_points;
