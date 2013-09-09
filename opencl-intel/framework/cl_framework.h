// Copyright (c) 2006-2012 Intel Corporation
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

#define PLATFORM_MODULE		FrameworkProxy::Instance()->GetPlatformModule()
#define CONTEXT_MODULE		FrameworkProxy::Instance()->GetContextModule()
#define EXECUTION_MODULE	FrameworkProxy::Instance()->GetExecutionModule()
#define API_IS_DISABLED	    FrameworkProxy::Instance()->API_Disabled()

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

#if defined DX_MEDIA_SHARING

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueAcquireDX9ObjectsINTEL)(
    cl_command_queue            /* command_queue */,
    cl_uint                     /* num_objects */,
    const cl_mem *              /* mem_objects */,
    cl_uint                     /* num_events_in_wait_list */,
    const cl_event *            /* event_wait_list */,
    cl_event *                  /* ocl_event */ );

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clEnqueueReleaseDX9ObjectsINTEL)(
    cl_command_queue            /* command_queue */,
    cl_uint                     /* num_objects */,
    const cl_mem *              /* mem_objects */,
    cl_uint                     /* num_events_in_wait_list */,
    const cl_event *            /* event_wait_list */,
    cl_event *                  /* ocl_event */ );

typedef CL_API_ENTRY cl_mem (CL_API_CALL *KHRpfn_clCreateFromDX9MediaSurfaceINTEL)(
    cl_context                  /* context */,
    cl_mem_flags                /* flags */,
    IDirect3DSurface9 *         /* resource */,
    HANDLE                      /* sharedHandle */,
    UINT                        /* plane */,
    cl_int *                    /* errcode_ret */ );

typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetDeviceIDsFromDX9INTEL)(
    cl_platform_id              /* platform */,
    cl_dx9_device_source_intel /* d3d_device_source */,
    void*                       /* d3d_object */,
    cl_dx9_device_set_intel    /* d3d_device_set */,
    cl_uint                     /* num_entries */, 
    cl_device_id *              /* devices */, 
    cl_uint *                   /* num_devices */ );

extern CL_API_ENTRY cl_mem CL_API_CALL clCreateFromDX9MediaSurfaceINTEL(
    cl_context /*context*/,
    cl_mem_flags /*flags*/,
    IDirect3DSurface9* /*resource*/,
    HANDLE /*sharehandle*/,
    UINT /*plane*/,
    cl_int* /*errcode_ret*/);

extern CL_API_ENTRY cl_int CL_API_CALL clEnqueueAcquireDX9ObjectsINTEL(
    cl_command_queue /*command_queue*/,
    cl_uint /*num_objects*/,
    const cl_mem * /*mem_objects*/,
    cl_uint /*num_events_in_wait_list*/,
    const cl_event * /*event_wait_list*/,
    cl_event * /*event*/);

extern CL_API_ENTRY cl_int CL_API_CALL clEnqueueReleaseDX9ObjectsINTEL(
    cl_command_queue /*command_queue*/,
    cl_uint /*num_objects*/,
    cl_mem * /*mem_objects*/,
    cl_uint /*num_events_in_wait_list*/,
    const cl_event * /*event_wait_list*/,
    cl_event * /*event*/);

extern CL_API_ENTRY cl_int CL_API_CALL clGetDeviceIDsFromDX9INTEL(
    cl_platform_id /*platform*/,
    cl_dx9_device_source_intel /*d3d_device_source*/,
    void* /*d3d_object*/,
    cl_dx9_device_set_intel /*d3d_device_set*/,
    cl_uint /*num_entries*/, 
    cl_device_id* /*devices*/, 
    cl_uint* /*num_devices*/);

#endif

typedef CL_API_ENTRY cl_mem (CL_API_CALL *KHRpfn_clCreatePipeINTEL)(
    cl_context                  /*context*/,
    cl_mem_flags                /*flags*/,
    cl_uint                     /*pipe_packet_size*/,
    cl_uint                     /*pipe_max_packets*/,
    const cl_pipe_properties *  /*attributes*/,
    void *                      /*host_ptr*/,
    size_t *                    /*size_ret*/,
    cl_int *                    /*errcode_ret*/ );

extern CL_API_ENTRY cl_mem CL_API_CALL clCreatePipeINTEL(
    cl_context                  /*context*/,
    cl_mem_flags                /*flags*/,
    cl_uint                     /*pipe_packet_size*/,
    cl_uint                     /*pipe_max_packets*/,
    const cl_pipe_properties*   /*attributes*/,
    void *                      /*host_ptr*/,
    size_t *                    /*size_ret*/,
    cl_int *                    /*errcode_ret*/ );

#ifdef __cplusplus
}
#endif




//// ------------------------------------
//// vendor dispatch table structure
//
struct COCLCRTDispatchTable
{
    KHRpfn_clGetKernelArgInfo                     clGetKernelArgInfo;
    KHRpfn_clCreatePipeINTEL                      clCreatePipeINTEL;
#ifdef DX_MEDIA_SHARING
    KHRpfn_clGetDeviceIDsFromDX9INTEL             clGetDeviceIDsFromDX9INTEL;
    KHRpfn_clCreateFromDX9MediaSurfaceINTEL       clCreateFromDX9MediaSurfaceINTEL;
    KHRpfn_clEnqueueAcquireDX9ObjectsINTEL        clEnqueueAcquireDX9ObjectsINTEL;
    KHRpfn_clEnqueueReleaseDX9ObjectsINTEL        clEnqueueReleaseDX9ObjectsINTEL;
#endif    
};

struct ocl_entry_points
{
    KHRicdVendorDispatch*                           icdDispatch;
    COCLCRTDispatchTable*                           crtDispatch;
};

struct _cl_object 
{    
	void * object;
};

struct _crt_dispatch
{	
    COCLCRTDispatchTable*       crtDispatch;	
};

struct _cl_platform_id_int : public _cl_platform_id, public _crt_dispatch
{    
	void *  object;
};

struct _cl_device_id_int : public _cl_device_id, public _crt_dispatch
{       
	void * object;
};

struct _cl_context_int : public _cl_context, public _crt_dispatch
{        
	void * object;
};

struct _cl_command_queue_int : public _cl_command_queue, public _crt_dispatch
{        
	void * object;
};

struct _cl_mem_int : public _cl_mem, public _crt_dispatch
{       
	void * object;
};

struct _cl_program_int : public _cl_program, public _crt_dispatch
{      
	void * object;
};

struct _cl_kernel_int : public _cl_kernel, public _crt_dispatch
{       
	void * object;
};

struct _cl_event_int : public _cl_event, public _crt_dispatch
{     
	void * object;
};

struct _cl_sampler_int : public _cl_sampler, public _crt_dispatch
{     
	void * object;
};
