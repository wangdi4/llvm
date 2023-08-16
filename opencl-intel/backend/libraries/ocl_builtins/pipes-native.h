// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
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

#ifndef __INTEL_PIPES_NATIVE_H__
#define __INTEL_PIPES_NATIVE_H__

// This header should be used in the RT to allocate OpenCL 2.0 Pipes.
#include "CL/cl.h"
#include "pipes-defines.h"

extern "C" {

//
// Global constructor and descructor
//
void __pipe_init_fpga(void *p, cl_int packet_size, cl_int max_packets,
                      cl_int mode);
void __pipe_release_fpga(void *p);

//
// Flush mechanism support
//
void __flush_read_pipe(void *p);
void __flush_write_pipe(void *p);

//

// Main read/write built-ins
//
cl_int __read_pipe_2_fpga(void *p, void *dst, cl_int size, cl_int align);
cl_int __write_pipe_2_fpga(void *p, const void *src, cl_int size, cl_int align);

//
// Info queries
//
cl_int __pipe_get_max_packets_fpga(cl_int depth, cl_int mode);
cl_int __pipe_get_total_size_fpga(cl_int packet_size, cl_int depth,
                                  cl_int mode);

} // extern "C"

#endif // __INTEL_PIPES_NATIVE_H__
