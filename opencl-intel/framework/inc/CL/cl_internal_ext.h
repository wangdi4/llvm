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

#ifdef __cplusplus
extern "C" {
#endif

/* cl_command_type */
#define CL_COMMAND_READ_GLOBAL_VARIABLE_INTEL 0x418E
#define CL_COMMAND_WRITE_GLOBAL_VARIABLE_INTEL 0x418F

/* cl_kernel_exec_info */
#define CL_KERNEL_EXEC_INFO_DISPATCH_TYPE_INTEL 0x4257
#define CL_KERNEL_EXEC_INFO_DISPATCH_TYPE_DEFAULT_INTEL 0
#define CL_KERNEL_EXEC_INFO_DISPATCH_TYPE_CONCURRENT_INTEL 1

typedef cl_uint cl_kernel_exec_info_dispatch_type_intel;

extern CL_API_ENTRY cl_int CL_API_CALL clGetDeviceGlobalVariablePointerINTEL(
    cl_device_id device, cl_program program, const char *global_variable_name,
    size_t *global_variable_size_ret,
    void **global_variable_pointer_ret) CL_API_SUFFIX__VERSION_2_2;

typedef CL_API_ENTRY
cl_int(CL_API_CALL *clGetDeviceGlobalVariablePointerINTEL_fn)(
    cl_device_id device, cl_program program, const char *global_variable_name,
    size_t *global_variable_size_ret,
    void **global_variable_pointer_ret) CL_API_SUFFIX__VERSION_2_2;

extern CL_API_ENTRY cl_int CL_API_CALL clEnqueueReadGlobalVariableINTEL(
    cl_command_queue command_queue, cl_program program, const char *name,
    cl_bool blocking_read, size_t size, size_t offset, void *ptr,
    cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
    cl_event *event) CL_API_SUFFIX__VERSION_3_0;

typedef CL_API_ENTRY cl_int(CL_API_CALL *clEnqueueReadGlobalVariableINTEL_fn)(
    cl_command_queue command_queue, cl_program program, const char *name,
    cl_bool blocking_read, size_t size, size_t offset, void *ptr,
    cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
    cl_event *event) CL_API_SUFFIX__VERSION_3_0;

extern CL_API_ENTRY cl_int CL_API_CALL clEnqueueWriteGlobalVariableINTEL(
    cl_command_queue command_queue, cl_program program, const char *name,
    cl_bool blocking_write, size_t size, size_t offset, const void *ptr,
    cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
    cl_event *event) CL_API_SUFFIX__VERSION_3_0;

typedef CL_API_ENTRY cl_int(CL_API_CALL *clEnqueueWriteGlobalVariableINTEL_fn)(
    cl_command_queue command_queue, cl_program program, const char *name,
    cl_bool blocking_write, size_t size, size_t offset, const void *ptr,
    cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
    cl_event *event) CL_API_SUFFIX__VERSION_3_0;

extern CL_API_ENTRY cl_int CL_API_CALL
clGetKernelMaxConcurrentWorkGroupCountINTEL(
    cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim,
    const size_t *global_work_offset, const size_t *local_work_size,
    size_t *max_work_group_count) CL_API_SUFFIX__VERSION_2_0;

typedef CL_API_ENTRY
cl_int(CL_API_CALL *clGetKernelMaxConcurrentWorkGroupCountINTEL_fn)(
    cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim,
    const size_t *global_work_offset, const size_t *local_work_size,
    size_t *max_work_group_count) CL_API_SUFFIX__VERSION_2_0;

#ifdef __cplusplus
}
#endif
