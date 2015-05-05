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
//
//  Original author: rjiossy
///////////////////////////////////////////////////////////
#pragma once

#include "icd/icd_dispatch.h"

#ifdef _WIN32
#include <CL/cl_d3d9.h>
#else
#ifdef LIBVA_SHARING
#include <CL/IntelPublic/va_ext.h>
#endif
#endif

namespace CRT_ICD_DISPATCH
{
#ifdef _WIN32
    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clEnqueueAcquireDX9ObjectsINTEL)(
        cl_command_queue            command_queue,
        cl_uint                     num_objects,
        const cl_mem *              mem_objects,
        cl_uint                     num_events_in_wait_list,
        const cl_event *            event_wait_list,
        cl_event *                  ocl_event );

    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clEnqueueReleaseDX9ObjectsINTEL)(
        cl_command_queue            command_queue,
        cl_uint                     num_objects,
        const cl_mem *              mem_objects,
        cl_uint                     num_events_in_wait_list,
        const cl_event *            event_wait_list,
        cl_event *                  ocl_event );

    typedef CL_API_ENTRY cl_mem (CL_API_CALL *INTELpfn_clCreateFromDX9MediaSurfaceINTEL)(
        cl_context                  context,
        cl_mem_flags                flags,
        IDirect3DSurface9 *         resource,
        HANDLE                      sharedHandle,
        UINT                        plane,
        cl_int *                    errcode_ret );

    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clGetDeviceIDsFromDX9INTEL)(
        cl_platform_id              platform,
        cl_dx9_device_source_intel  d3d_device_source,
        void*                       d3d_object,
        cl_dx9_device_set_intel     d3d_device_set,
        cl_uint                     num_entries,
        cl_device_id *              devices,
        cl_uint *                   num_devices );

#else   //Linux/Android
    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clCreateFromDX9MediaSurfaceINTEL)();
    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clEnqueueAcquireDX9ObjectsINTEL)();
    typedef CL_API_ENTRY cl_mem (CL_API_CALL *INTELpfn_clEnqueueReleaseDX9ObjectsINTEL)();
    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clGetDeviceIDsFromDX9INTEL)();
#endif // Linux and android

    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clGetImageParamsINTEL)(
        cl_context                  context,
        const cl_image_format *     image_format,
        const cl_image_desc *       image_desc,
        size_t *                    image_row_pitch,
        size_t *                    image_slice_pitch);

    /* Performance Counter APIs */
    typedef CL_API_ENTRY cl_command_queue (CL_API_CALL *INTELpfn_clCreatePerfCountersCommandQueueINTEL)(
        cl_context                  context,
        cl_device_id                device,
        cl_command_queue_properties properties,
        cl_uint                     configuration,
        cl_int *                    errcode_ret );

    /* cl_intel_accelerator */
    typedef CL_API_ENTRY cl_accelerator_intel (CL_API_CALL *INTELpfn_clCreateAcceleratorINTEL)(
        cl_context                  context,
        cl_accelerator_type_intel   type,
        size_t                      desc_size,
        const void *                desc,
        cl_int *                    errcode_ret ) CL_API_SUFFIX__VERSION_1_2;

    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clGetAcceleratorInfoINTEL)(
        cl_accelerator_intel        accelerator,
        cl_accelerator_info_intel   param_name,
        size_t                      param_value_size,
        void *                      param_value,
        size_t *                    param_value_size_ret ) CL_API_SUFFIX__VERSION_1_2;

    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clRetainAcceleratorINTEL)(
        cl_accelerator_intel        accelerator ) CL_API_SUFFIX__VERSION_1_2;

    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clReleaseAcceleratorINTEL)(
        cl_accelerator_intel        accelerator ) CL_API_SUFFIX__VERSION_1_2;

    //Kernel Instrumentation Query APIs
    typedef CL_API_ENTRY cl_program (CL_API_CALL *INTELpfn_clCreateProfiledProgramWithSourceINTEL)(
        cl_context                  context,
        cl_uint                     count,
        const char **               sources,
        const size_t *              lengths,
        const void *                configurations,
        cl_uint                     configurations_count,
        cl_int *                    errorCode );

    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clCreateKernelProfilingJournalINTEL)(
        cl_context                  context,
        const void *                configuration );

#ifdef LIBVA_SHARING
    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clEnqueueAcquireVA_APIMediaSurfacesINTEL)(
        cl_command_queue            command_queue,
        cl_uint                     num_objects,
        const cl_mem *              mem_objects,
        cl_uint                     num_events_in_wait_list,
        const cl_event *            event_wait_list,
        cl_event *                  ocl_event );

    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clEnqueueReleaseVA_APIMediaSurfacesINTEL)(
        cl_command_queue            command_queue,
        cl_uint                     num_objects,
        const cl_mem *              mem_objects,
        cl_uint                     num_events_in_wait_list,
        const cl_event *            event_wait_list,
        cl_event *                  ocl_event );

    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clGetDeviceIDsFromVA_APIMediaAdapterINTEL)(
        cl_platform_id                  platform,
        cl_va_api_device_source_intel   media_adapter_type,
        void *                          media_adapter,
        cl_va_api_device_set_intel      media_adapter_set,
        cl_uint                         num_entries,
        cl_device_id *                  devices,
        cl_uint *                       num_devices );

    typedef CL_API_ENTRY cl_mem (CL_API_CALL *INTELpfn_clCreateFromVA_APIMediaSurfaceINTEL)(
        cl_context                  context,
        cl_mem_flags                flags,
        VASurfaceID *               surface,
        cl_uint                     plane,
        cl_int *                    errcode_ret );

#else
    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clCreateFromVA_APIMediaSurfaceINTEL)();
    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clEnqueueReleaseVA_APIMediaSurfacesINTEL)();
    typedef CL_API_ENTRY cl_int (CL_API_CALL *INTELpfn_clGetDeviceIDsFromVA_APIMediaAdapterINTEL)();
    typedef CL_API_ENTRY cl_mem (CL_API_CALL *INTELpfn_clEnqueueAcquireVA_APIMediaSurfacesINTEL)();
#endif //LIBVA_SHARING

    typedef CL_API_ENTRY cl_mem (CL_API_CALL *INTELpfn_clCreatePipeINTEL)(
        cl_context                  context,
        cl_mem_flags                flags,
        cl_uint                     pipe_packet_size,
        cl_uint                     pipe_max_packets,
        const cl_pipe_properties *  properties,
        void *                      host_ptr,
        size_t *                    size_ret,
        cl_int *                    errcode_ret );

    struct SOCLCRTDispatchTable
    {
        KHRpfn_clGetKernelArgInfo                           clGetKernelArgInfo;

        // DX9 interop APIs
        INTELpfn_clGetDeviceIDsFromDX9INTEL                 clGetDeviceIDsFromDX9INTEL;
        INTELpfn_clCreateFromDX9MediaSurfaceINTEL           clCreateFromDX9MediaSurfaceINTEL;
        INTELpfn_clEnqueueAcquireDX9ObjectsINTEL            clEnqueueAcquireDX9ObjectsINTEL;
        INTELpfn_clEnqueueReleaseDX9ObjectsINTEL            clEnqueueReleaseDX9ObjectsINTEL;

        INTELpfn_clGetImageParamsINTEL                      clGetImageParamsINTEL;

        // API to expose the Performance Counters to applications
        INTELpfn_clCreatePerfCountersCommandQueueINTEL      clCreatePerfCountersCommandQueueINTEL;

        // Video Analytics Accelerator
        INTELpfn_clCreateAcceleratorINTEL                   clCreateAcceleratorINTEL;
        INTELpfn_clGetAcceleratorInfoINTEL                  clGetAcceleratorInfoINTEL;
        INTELpfn_clRetainAcceleratorINTEL                   clRetainAcceleratorINTEL;
        INTELpfn_clReleaseAcceleratorINTEL                  clReleaseAcceleratorINTEL;

        // API to expose the Kernel Instrumentation Query to applications
        INTELpfn_clCreateProfiledProgramWithSourceINTEL     clCreateProfiledProgramWithSourceINTEL;
        INTELpfn_clCreateKernelProfilingJournalINTEL        clCreateKernelProfilingJournalINTEL;

        // VAMedia sharing extension
        INTELpfn_clCreateFromVA_APIMediaSurfaceINTEL        clCreateFromVA_APIMediaSurfaceINTEL;
        INTELpfn_clGetDeviceIDsFromVA_APIMediaAdapterINTEL  clGetDeviceIDsFromVA_APIMediaAdapterINTEL;
        INTELpfn_clEnqueueReleaseVA_APIMediaSurfacesINTEL   clEnqueueReleaseVA_APIMediaSurfacesINTEL;
        INTELpfn_clEnqueueAcquireVA_APIMediaSurfacesINTEL   clEnqueueAcquireVA_APIMediaSurfacesINTEL;

        // API to create pipe with host pointer
        INTELpfn_clCreatePipeINTEL                          clCreatePipeINTEL;
    };

    struct SOCLEntryPointsTable
    {
        KHRicdVendorDispatch*                           icdDispatch;
        SOCLCRTDispatchTable*                           crtDispatch;
    };
}
