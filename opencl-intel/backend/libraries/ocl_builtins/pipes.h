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

#ifdef _WIN32
  #define DECLSPEC_ALIGN(alignment) __declspec(align(alignment))
  #define ATTR_ALIGN(alignment)
  #define ALIGNED(decl, alignment) DECLSPEC_ALIGN(alignment) decl
#else
  #define DECLSPEC_ALIGN(alignment)
  #define ATTR_ALIGN(alignment)  __attribute__((aligned(alignment)))
  #define ALIGNED(decl, alignment) decl ATTR_ALIGN(alignment)
#endif

#define MAX_VL_SUPPORTED_BY_PIPES 16

struct __pipe_internal_buf {
  i32 end;   // index for a new element
  i32 size;  // number of elements in the buffer, -1 means a locked buffer
  i32 limit; // max number of elements before flush
};

DECLSPEC_ALIGN(64) struct __pipe_t {
  i32 packet_size;
  i32 max_packets;

  // The fields below are accessible(read, write) from different threads.
  // Alignment needed here to avoid false sharing.
  ALIGNED(atomic_i32 head, 64);
  ALIGNED(atomic_i32 tail, 64);

#define PIPE_READ_BUF_PREFERRED_LIMIT 256
#define PIPE_WRITE_BUF_PREFERRED_LIMIT 256
  ALIGNED(struct __pipe_internal_buf read_buf, 64);
  ALIGNED(struct __pipe_internal_buf write_buf, 64);

  // Trailing buffer for packets
  //
  // char buffer[max_packets * packet_size];
} ATTR_ALIGN(64);

#ifdef __OPENCL_C_VERSION__
void __pipe_init_intel(__global struct __pipe_t* p,
                       int packet_size, int max_packets);
void __flush_read_pipe(__global struct __pipe_t* p);
void __flush_write_pipe(__global struct __pipe_t* p);

int __read_pipe_2_intel(__global struct __pipe_t* p, void* dst);
int __write_pipe_2_intel(__global struct __pipe_t* p, void* src);
int __read_pipe_2_bl_intel(__global struct __pipe_t* p, void* dst);
int __write_pipe_2_bl_intel(__global struct __pipe_t* p, void* src);

int get_buffer_capacity(const struct __pipe_internal_buf* b);
__global void* get_packet_ptr(__global struct __pipe_t* p, int index);
bool reserve_write_buffer(struct __pipe_internal_buf* b, int capacity);
bool reserve_read_buffer(struct __pipe_internal_buf* b, int capacity);
int advance(__global const struct __pipe_t* p, int index, int offset);
int get_read_capacity(__global struct __pipe_t* p);
int get_write_capacity(__global struct __pipe_t* p);

#endif // __OPENCL_VERSION__

#endif // __INTEL_PIPES_H__
