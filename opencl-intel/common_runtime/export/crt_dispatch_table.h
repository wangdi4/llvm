#pragma once

#include "icd\icd_dispatch.h"
#include <CL/cl_d3d9.h>

namespace CRT_ICD_DISPATCH
{
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

    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clGetImageParamsINTEL)(
        cl_context                  context,
        const cl_image_format *     image_format,
        const cl_mem_object_type    image_type,
        size_t                      image_width,
        size_t                      image_height,
        size_t                      image_depth,
        size_t*                     image_row_pitch,
        size_t*                     image_slice_pitch);

    struct CrtKHRicdVendorDispatch: public KHRicdVendorDispatch
    {
        INTELpfn_clGetDeviceIDsFromDX9INTEL             clGetDeviceIDsFromDX9INTEL;
        INTELpfn_clCreateFromDX9MediaSurfaceINTEL       clCreateFromDX9MediaSurfaceINTEL;
        INTELpfn_clEnqueueAcquireDX9ObjectsINTEL        clEnqueueAcquireDX9ObjectsINTEL;
        INTELpfn_clEnqueueReleaseDX9ObjectsINTEL        clEnqueueReleaseDX9ObjectsINTEL;
        INTELpfn_clGetImageParamsINTEL                  clGetImageParamsINTEL;
    };
};