#pragma once

#include "icd/icd_dispatch.h"

#ifdef _WIN32
#include <CL/cl_d3d9.h>
#endif

namespace CRT_ICD_DISPATCH
{
#ifdef _WIN32
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
        cl_dx9_device_source_intel  /* d3d_device_source */,
        void*                       /* d3d_object */,
        cl_dx9_device_set_intel     /* d3d_device_set */,
        cl_uint                     /* num_entries */,
        cl_device_id *              /* devices */,
        cl_uint *                   /* num_devices */ );

#else   //Linux/Android
    typedef void *INTELpfn_clCreateFromDX9MediaSurfaceINTEL();
    typedef void *INTELpfn_clEnqueueAcquireDX9ObjectsINTEL();
    typedef void *INTELpfn_clEnqueueReleaseDX9ObjectsINTEL();
    typedef void *INTELpfn_clGetDeviceIDsFromDX9INTEL();
#endif

    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clGetImageParamsINTEL)(
        cl_context                  context,
        const cl_image_format *     image_format,
        const cl_image_desc *       image_desc,        
        size_t*                     image_row_pitch,
        size_t*                     image_slice_pitch);

    /* Performance Counter APIs */
    typedef CL_API_ENTRY cl_command_queue (CL_API_CALL *INTELpfn_clCreatePerfCountersCommandQueueINTEL)(
        cl_context                  context,
        cl_device_id                device,
        cl_command_queue_properties properties,
        cl_uint                     configuration,
        cl_int *                    errcode_ret );

    /* cl_intel_accelerator */
    typedef CL_API_ENTRY cl_accelerator_intel (CL_API_CALL *INTELpfn_clCreateAcceleratorINTEL)(
        cl_context                      context,
        cl_accelerator_type_intel       type, 
        size_t                          desc_size,
        const void *                    desc, 
        cl_int *                        errcode_ret ) CL_API_SUFFIX__VERSION_1_2;

    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clGetAcceleratorInfoINTEL)(
        cl_accelerator_intel         accelerator,
        cl_accelerator_info_intel    param_name,
        size_t                       param_value_size,
        void*                        param_value,
        size_t*                      param_value_size_ret ) CL_API_SUFFIX__VERSION_1_2;

    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clRetainAcceleratorINTEL)(
        cl_accelerator_intel /* accelerator */) CL_API_SUFFIX__VERSION_1_2;

    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clReleaseAcceleratorINTEL)(
        cl_accelerator_intel /* accelerator */) CL_API_SUFFIX__VERSION_1_2;

    /* Kernel Instrumentation Query APIs */
    typedef CL_API_ENTRY cl_program (CL_API_CALL *INTELpfn_clCreateProfiledProgramWithSourceINTEL)(
        cl_context          context,
        cl_uint             count,
        const char**        sources,
        const size_t*       lengths,
        const void*         configurations,
        cl_uint             configurations_count,
        cl_int*             errorCode );

    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clCreateKernelProfilingJournalINTEL)(
        cl_context          context,
        const void*         configuration );

    struct SOCLCRTDispatchTable
    {
        KHRpfn_clGetKernelArgInfo                       clGetKernelArgInfo;
        INTELpfn_clGetDeviceIDsFromDX9INTEL             clGetDeviceIDsFromDX9INTEL;
        INTELpfn_clCreateFromDX9MediaSurfaceINTEL       clCreateFromDX9MediaSurfaceINTEL;
        INTELpfn_clEnqueueAcquireDX9ObjectsINTEL        clEnqueueAcquireDX9ObjectsINTEL;
        INTELpfn_clEnqueueReleaseDX9ObjectsINTEL        clEnqueueReleaseDX9ObjectsINTEL;
        INTELpfn_clGetImageParamsINTEL                  clGetImageParamsINTEL;
        // API to expose the Performance Counters to applications
        INTELpfn_clCreatePerfCountersCommandQueueINTEL  clCreatePerfCountersCommandQueueINTEL;
        INTELpfn_clCreateAcceleratorINTEL               clCreateAcceleratorINTEL;
        INTELpfn_clGetAcceleratorInfoINTEL              clGetAcceleratorInfoINTEL;
        INTELpfn_clRetainAcceleratorINTEL               clRetainAcceleratorINTEL;
        INTELpfn_clReleaseAcceleratorINTEL              clReleaseAcceleratorINTEL;
        // API to expose the Kernel Instrumentation Query to applications
        INTELpfn_clCreateProfiledProgramWithSourceINTEL clCreateProfiledProgramWithSourceINTEL;
        INTELpfn_clCreateKernelProfilingJournalINTEL    clCreateKernelProfilingJournalINTEL;
    };

    struct SOCLEntryPointsTable
    {
        KHRicdVendorDispatch*                           icdDispatch;
        SOCLCRTDispatchTable*                           crtDispatch;
    };  
}