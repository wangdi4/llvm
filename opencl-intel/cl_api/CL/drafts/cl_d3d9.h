/**********************************************************************************
 * Copyright (c) 2008-2009 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 **********************************************************************************/

/* $Revision$ on $Date$ */

#ifndef __OPENCL_CL_D3D9_H
#define __OPENCL_CL_D3D9_H

#include <CL/cl_platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/* cl_khr_d3d9_sharing extension    */
#define cl_khr_d3d9_sharing 1

/* cl_context_properties            */
#define CL_CONTEXT_D3D9_DEVICE 0x1085

extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromD3D9BufferKHR(
    cl_context           /* context */,
    cl_mem_flags         /* flags */,
    IDirect3DResource9 * /* resource */,
    HANDLE               /* shared_handle */,
    cl_int *             /* errcode_ret */);


extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromD3D9TextureKHR(
    cl_context          /* context */,
    cl_mem_flags        /* flags */,
    IDirect3DTexture9 * /* texture */,
    HANDLE              /* shared_handle */,
    UINT                /* miplevel */,
    cl_int *            /* errcode_ret */);

extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromD3D9VolumeTextureKHR(
    cl_context                /* context */,
    cl_mem_flags              /* flags */,
    IDirect3DVolumeTexture9 * /* resource */,
    HANDLE                    /* shared_handle */,
    UINT                      /* miplevel */,
    cl_int *                  /* errcode_ret */);

extern CL_API_ENTRY cl_mem CL_API_CALL
clCreateFromD3D9CubeTextureKHR(
    cl_context                /* context */,
    cl_mem_flags              /* flags */,
    IDirect3DCubeTexture9 *   /* resource */,                            
    HANDLE                    /* shared_handle */,
    D3DCUBEMAP_FACES FaceType /* face */,
    UINT                      /* miplevel */,
    cl_int *                  /* errcode_ret */);

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueAcquireD3D9ObjectsKHR(
    cl_command_queue /* command_queue */,
    cl_uint          /* num_objects */,
    const cl_mem *   /* mem_objects */,
    cl_uint          /* num_events_in_wait_list */,
    const cl_event * /* event_wait_list */,
    cl_event *       /* event */);

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReleaseD3D9ObjectsKHR(
    cl_command_queue /* command_queue */,
    cl_uint          /* num_objects */,
    const cl_mem *   /* mem_objects */,
    cl_uint          /* num_events_in_wait_list */,
    const cl_event * /* event_wait_list */,
    cl_event *       /* event */);

#ifdef __cplusplus
}
#endif

#endif  /* __OPENCL_CL_D3D9_H   */

