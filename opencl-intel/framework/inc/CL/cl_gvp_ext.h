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

#include <CL/cl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern CL_API_ENTRY cl_int
CL_API_CALL clGetDeviceGlobalVariablePointerINTEL(
    cl_device_id    device,
    cl_program      program,
    const char *    global_variable_name,
    size_t *        global_variable_size_ret,
    void **         global_variable_pointer_ret) CL_API_SUFFIX__VERSION_2_2;

typedef CL_API_ENTRY cl_int
(CL_API_CALL *clGetDeviceGlobalVariablePointerINTEL_fn)(
    cl_device_id    device,
    cl_program      program,
    const char *    global_variable_name,
    size_t *        global_variable_size_ret,
    void **         global_variable_pointer_ret) CL_API_SUFFIX__VERSION_2_2;

#ifdef __cplusplus
}
#endif
