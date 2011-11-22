#pragma once

#include <icd_dispatch.h>
#include <d3d9.h>

namespace CRT_ICD_DISPATCH
{

    /// This needs to be integrated into the ICD for the Common Runtime USE
    typedef cl_uint             cl_kernel_arg_info;
    #define CL_API_SUFFIX__VERSION_1_2
    #define CL_EXT_SUFFIX__VERSION_1_2


    typedef CL_API_ENTRY cl_int (CL_API_CALL *KHRpfn_clGetKernelArgInfo)(
        cl_kernel            kernel,
        cl_uint              arg_indx,
        cl_kernel_arg_info   param_name,
        size_t               param_value_size,
        void *               param_value,
        size_t *             param_value_size_ret) CL_API_SUFFIX__VERSION_1_2;

    extern CL_API_ENTRY cl_int CL_API_CALL
    clGetKernelArgInfo(cl_kernel           /* kernel */,
                       cl_uint             /* arg_indx */,
                       cl_kernel_arg_info  /* param_name */,
                       size_t              /* param_value_size */,
                       void *              /* param_value */,
                       size_t *            /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_2;

    typedef cl_uint cl_d3d9_device_source_intel;
    typedef cl_uint cl_d3d9_device_set_intel;
    
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
        cl_d3d9_device_source_intel /* d3d_device_source */,
        void*                       /* d3d_object */,
        cl_d3d9_device_set_intel    /* d3d_device_set */,
        cl_uint                     /* num_entries */, 
        cl_device_id *              /* devices */, 
        cl_uint *                   /* num_devices */ );

    struct CrtKHRicdVendorDispatch: public KHRicdVendorDispatch
    {
       KHRpfn_clGetKernelArgInfo                       clGetKernelArgInfo;
       INTELpfn_clGetDeviceIDsFromDX9INTEL             clGetDeviceIDsFromDX9INTEL;
       INTELpfn_clCreateFromDX9MediaSurfaceINTEL       clCreateFromDX9MediaSurfaceINTEL;
       INTELpfn_clEnqueueAcquireDX9ObjectsINTEL        clEnqueueAcquireDX9ObjectsINTEL;
       INTELpfn_clEnqueueReleaseDX9ObjectsINTEL        clEnqueueReleaseDX9ObjectsINTEL;
    };
};