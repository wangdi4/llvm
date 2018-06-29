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

typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clEnqueueAcquireDX9ObjectsINTEL)(
    cl_command_queue            /* command_queue */,
    cl_uint                     /* num_objects */,
    const cl_mem *              /* mem_objects */,
    cl_uint                     /* num_events_in_wait_list */,
    const cl_event *            /* event_wait_list */,
    cl_event *                  /* ocl_event */ );

typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clEnqueueReleaseDX9ObjectsINTEL)(
    cl_command_queue            /* command_queue */,
    cl_uint                     /* num_objects */,
    const cl_mem *              /* mem_objects */,
    cl_uint                     /* num_events_in_wait_list */,
    const cl_event *            /* event_wait_list */,
    cl_event *                  /* ocl_event */ );

typedef CL_API_ENTRY cl_mem (CL_API_CALL *INTELpfn_clCreateFromDX9MediaSurfaceINTEL)(
    cl_context                  /* context */,
    cl_mem_flags                /* flags */,
    IDirect3DSurface9 *         /* resource */,
    HANDLE                      /* sharedHandle */,
    UINT                        /* plane */,
    cl_int *                    /* errcode_ret */ );

typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clGetDeviceIDsFromDX9INTEL)(
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

#else
typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clEnqueueAcquireDX9ObjectsINTEL)();
typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clEnqueueReleaseDX9ObjectsINTEL)();
typedef CL_API_ENTRY cl_mem (CL_API_CALL *INTELpfn_clCreateFromDX9MediaSurfaceINTEL)();
typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clGetDeviceIDsFromDX9INTEL)();
#endif // defined DX_MEDIA_SHARING

// Those are dummy entries for GPU in CRT dispatch table
typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clGetImageParamsINTEL)();
typedef CL_API_ENTRY cl_command_queue (CL_API_CALL *INTELpfn_clCreatePerfCountersCommandQueueINTEL)();
typedef CL_API_ENTRY cl_accelerator_intel (CL_API_CALL *INTELpfn_clCreateAcceleratorINTEL)();
typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clGetAcceleratorInfoINTEL)();
typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clRetainAcceleratorINTEL)();
typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clReleaseAcceleratorINTEL)();
typedef CL_API_ENTRY cl_program (CL_API_CALL *INTELpfn_clCreateProfiledProgramWithSourceINTEL)();
typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clCreateKernelProfilingJournalINTEL)();
typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clEnqueueAcquireVAMediaSurfacesINTEL)();
typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clEnqueueReleaseVAMediaSurfacesINTEL)();
typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clGetDeviceIDsFromVAMediaAdapterINTEL)();
typedef CL_API_ENTRY cl_mem (CL_API_CALL *INTELpfn_clCreateFromVAMediaSurfaceINTEL)();
typedef CL_API_ENTRY cl_int( CL_API_CALL *INTELpfn_clSetDebugVariableINTEL)();
typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clSetAcceleratorInfoINTEL)();

typedef CL_API_ENTRY cl_mem (CL_API_CALL *INTELpfn_clCreatePipeINTEL)(
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
// CRT dispatch table: this should be exactly same as in crt_dispatch_table.h
struct SOCLCRTDispatchTable
{
    KHRpfn_clGetKernelArgInfo                       clGetKernelArgInfo;

    INTELpfn_clGetDeviceIDsFromDX9INTEL             clGetDeviceIDsFromDX9INTEL;
    INTELpfn_clCreateFromDX9MediaSurfaceINTEL       clCreateFromDX9MediaSurfaceINTEL;
    INTELpfn_clEnqueueAcquireDX9ObjectsINTEL        clEnqueueAcquireDX9ObjectsINTEL;
    INTELpfn_clEnqueueReleaseDX9ObjectsINTEL        clEnqueueReleaseDX9ObjectsINTEL;

    // GPU entries
    INTELpfn_clGetImageParamsINTEL                  clGetImageParamsINTEL;
    INTELpfn_clCreatePerfCountersCommandQueueINTEL  clCreatePerfCountersCommandQueueINTEL;
    INTELpfn_clCreateAcceleratorINTEL               clCreateAcceleratorINTEL;
    INTELpfn_clGetAcceleratorInfoINTEL              clGetAcceleratorInfoINTEL;
    INTELpfn_clRetainAcceleratorINTEL               clRetainAcceleratorINTEL;
    INTELpfn_clReleaseAcceleratorINTEL              clReleaseAcceleratorINTEL;
    INTELpfn_clCreateProfiledProgramWithSourceINTEL clCreateProfiledProgramWithSourceINTEL;
    INTELpfn_clCreateKernelProfilingJournalINTEL    clCreateKernelProfilingJournalINTEL;
    INTELpfn_clCreateFromVAMediaSurfaceINTEL        clCreateFromVAMediaSurfaceINTEL;
    INTELpfn_clGetDeviceIDsFromVAMediaAdapterINTEL  clGetDeviceIDsFromVAMediaAdapterINTEL;
    INTELpfn_clEnqueueReleaseVAMediaSurfacesINTEL   clEnqueueReleaseVAMediaSurfacesINTEL;
    INTELpfn_clEnqueueAcquireVAMediaSurfacesINTEL   clEnqueueAcquireVAMediaSurfacesINTEL;

    // API to create pipe with host pointer
    INTELpfn_clCreatePipeINTEL                      clCreatePipeINTEL;

    INTELpfn_clSetDebugVariableINTEL                clSetDebugVariableINTEL;

        // Video Analytics Accelerator
    INTELpfn_clSetAcceleratorInfoINTEL              clSetAcceleratorInfoINTEL;
};

struct ocl_entry_points
{
    KHRicdVendorDispatch*                           icdDispatch;
    SOCLCRTDispatchTable*                           crtDispatch;
};

struct _crt_dispatch
{	
    SOCLCRTDispatchTable*       crtDispatch;	
};

struct _cl_object : public _crt_dispatch
{    
	void * object;
	KHRicdVendorDispatch *dispatch;
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
