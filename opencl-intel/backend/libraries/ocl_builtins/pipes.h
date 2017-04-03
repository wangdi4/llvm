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

#ifdef __OPENCL_C_VERSION__
  #define i32 int
  #define atomic_i32 volatile atomic_int
#else
  #include <CL/cl.h>
  #define i32 cl_int
  #define atomic_i32 cl_int
#endif

struct __pipe_internal_buf {
  i32 end;   // index for a new element
  i32 size;  // number of elements in the buffer, -1 means a locked buffer
  i32 limit; // max number of elements befor flush
};

struct __pipe_t {
  i32 packet_size;
  i32 max_packets;

  // The fields below are accessible(read, write) from different threads.
  // Alignment needed here to avoid false sharing.
  atomic_i32 head __attribute__((aligned(64)));
  atomic_i32 tail __attribute__((aligned(64)));

#define PIPE_READ_BUF_PREFERRED_LIMIT 256
#define PIPE_WRITE_BUF_PREFERRED_LIMIT 256
  struct __pipe_internal_buf read_buf __attribute__((aligned(64)));
  struct __pipe_internal_buf write_buf __attribute__((aligned(64)));

  // Trailing buffer for packets
  //
  // char buffer[max_packets * packet_size];
} __attribute__((aligned(64)));

#ifdef __OPENCL_C_VERSION__
void __pipe_init_intel(__global struct __pipe_t* p,
                       int packet_size, int max_packets);
void __flush_read_pipe(__global struct __pipe_t* p);
void __flush_write_pipe(__global struct __pipe_t* p);
int __read_pipe_2_intel(__global struct __pipe_t* p, void* dst);
int __write_pipe_2_intel(__global struct __pipe_t* p, void* src);
int __read_pipe_2_bl_intel(__global struct __pipe_t* p, void* dst);
int __write_pipe_2_bl_intel(__global struct __pipe_t* p, void* src);
#endif // __OPENCL_VERSION__

#endif // __INTEL_PIPES_H__
