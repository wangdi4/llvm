// Copyright (c) 2017 Intel Corporation
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

EXPORT void __pipe_init_intel(pipe_ptr_t p,
                              int packet_size, int max_packets);
EXPORT void __flush_read_pipe(pipe_ptr_t p);
EXPORT void __flush_write_pipe(pipe_ptr_t p);

EXPORT i32 __read_pipe_2_intel(pipe_ptr_t p, void* dst);
EXPORT i32 __write_pipe_2_intel(pipe_ptr_t p, const void* src);
EXPORT i32 __read_pipe_2_bl_intel(pipe_ptr_t p, void* dst);
EXPORT i32 __write_pipe_2_bl_intel(pipe_ptr_t p, const void* src);

EXPORT i32 __pipe_get_max_packets(i32 depth);
EXPORT i32 __pipe_get_total_size(i32 packet_size, i32 depth);

#endif // __INTEL_PIPES_H__
