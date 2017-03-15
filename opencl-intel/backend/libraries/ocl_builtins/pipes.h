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

  atomic_i32 head;
  atomic_i32 tail;

#define PIPE_READ_BUF_PREFERRED_LIMIT 64
#define PIPE_WRITE_BUF_PREFERRED_LIMIT 64
  struct __pipe_internal_buf read_buf;
  struct __pipe_internal_buf write_buf;

  // Trailing buffer for packets
  //
  // char buffer[max_packets * packet_size];
};

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
