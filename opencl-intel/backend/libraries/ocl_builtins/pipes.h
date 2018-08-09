// INTEL CONFIDENTIAL
//
// Copyright 2017-2018 Intel Corporation.
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

#ifndef __INTEL_PIPES_H__
#define __INTEL_PIPES_H__

// This header is also used by the RT to allocate OpenCL 2.0 Pipes.
#ifdef __OPENCL_C_VERSION__
typedef int i32;
typedef __global struct __pipe_t* pipe_ptr_t;
typedef __global struct  __pipe_internal_buf* pipe_buf_ptr_t;
typedef __global void* global_ptr_t;
#define EXPORT
#else
#include <CL/cl.h>
typedef cl_int i32;
typedef void* pipe_ptr_t;
typedef void* pipe_buf_ptr_t;
typedef void* global_ptr_t;
#define EXPORT extern "C"
#endif

#include "pipes-defines.h"

EXPORT void
__pipe_init_intel(pipe_ptr_t p, int packet_size, int max_packets, int mode);

EXPORT void __pipe_release_intel(pipe_ptr_t p);
EXPORT void __flush_read_pipe(pipe_ptr_t p);
EXPORT void __flush_write_pipe(pipe_ptr_t p);

EXPORT i32 __read_pipe_2_intel(pipe_ptr_t p, void* dst);
EXPORT i32 __write_pipe_2_intel(pipe_ptr_t p, const void* src);

EXPORT i32 __pipe_get_max_packets(i32 depth, int mode);
EXPORT i32 __pipe_get_total_size(i32 packet_size, i32 depth, int mode);

#endif // __INTEL_PIPES_H__
