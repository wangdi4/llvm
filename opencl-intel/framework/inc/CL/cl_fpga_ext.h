// Copyright 2020 Intel Corporation.
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
//
// ===--------------------------------------------------------------------=== //
//
#pragma once

#include "CL/cl.h"
/* cl_mem_properties_intel * is defined in this head file */
#include "CL/cl_ext.h"

#ifdef __cplusplus
extern "C" {
#endif

/* cl_mem_flags - bitfield */
#define CL_CHANNEL_AUTO_INTELFPGA (0 << 16)
#define CL_CHANNEL_1_INTELFPGA (1 << 16)
#define CL_CHANNEL_2_INTELFPGA (2 << 16)
#define CL_CHANNEL_3_INTELFPGA (3 << 16)
#define CL_CHANNEL_4_INTELFPGA (4 << 16)
#define CL_CHANNEL_5_INTELFPGA (5 << 16)
#define CL_CHANNEL_6_INTELFPGA (6 << 16)
#define CL_CHANNEL_7_INTELFPGA (7 << 16)
#define CL_MEM_HETEROGENEOUS_INTELFPGA (1 << 19)

#define CL_KERNEL_ARG_HOST_ACCESSIBLE_PIPE_INTEL 0x4210
#define CL_DEVICE_MAX_HOST_READ_PIPES_INTEL 0x4211
#define CL_DEVICE_MAX_HOST_WRITE_PIPES_INTEL 0x4212

/* cl_mem_properties_intel enum */
#define CL_MEM_CHANNEL_INTEL 0x4213

#define CL_PIPE_FULL -1111
#define CL_PIPE_EMPTY -1112

extern CL_API_ENTRY cl_int CL_API_CALL
clReadPipeIntelFPGA(cl_mem pipe, void *ptr) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int(CL_API_CALL *clReadPipeIntelFPGA_fn)(
    cl_mem pipe, void *ptr) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clWritePipeIntelFPGA(cl_mem pipe, const void *ptr) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int(CL_API_CALL *clWritePipeIntelFPGA_fn)(
    cl_mem pipe, const void *ptr) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY void *CL_API_CALL clMapHostPipeIntelFPGA(
    cl_mem pipe, cl_map_flags map_flags, size_t requested_size,
    size_t *mapped_size, cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY void *(CL_API_CALL *clMapHostPipeIntelFPGA_fn)(
    cl_mem pipe, cl_map_flags map_flags, size_t requested_size,
    size_t *mapped_size, cl_int *errcode_ret)CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clUnmapHostPipeIntelFPGA(cl_mem pipe, void *mapped_ptr, size_t size_to_unmap,
                         size_t *unmapped_size) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int(CL_API_CALL *clUnmapHostPipeIntelFPGA_fn)(
    cl_mem pipe, void *mapped_ptr, size_t size_to_unmap,
    size_t *unmapped_size) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL clGetProfileDataDeviceIntelFPGA(
    cl_device_id device_id, cl_program program, cl_bool read_enqueue_kernels,
    cl_bool read_auto_enqueued, cl_bool clear_counters_after_readback,
    size_t param_value_size, void *param_value, size_t *param_value_size_ret,
    cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int(CL_API_CALL *clGetProfileDataDeviceIntelFPGA_fn)(
    cl_device_id device_id, cl_program program, cl_bool read_enqueue_kernels,
    cl_bool read_auto_enqueued, cl_bool clear_counters_after_readback,
    size_t param_value_size, void *param_value, size_t *param_value_size_ret,
    cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_mem CL_API_CALL clCreateBufferWithPropertiesINTEL(
    cl_context context, const cl_mem_properties_intel *properties,
    cl_mem_flags flags, size_t size, void *host_ptr,
    cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_mem(CL_API_CALL *clCreateBufferWithPropertiesINTEL_fn)(
    cl_context context, const cl_mem_properties_intel *properties,
    cl_mem_flags flags, size_t size, void *host_ptr,
    cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0;

#ifdef __cplusplus
}
#endif
