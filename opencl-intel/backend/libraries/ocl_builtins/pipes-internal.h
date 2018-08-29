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

#ifndef __PIPES_INTERNAL_H__
#define __PIPES_INTERNAL_H__

#define MAX_VL_SUPPORTED_BY_PIPES 16
#define PIPE_READ_BUF_PREFERRED_LIMIT 256
#define PIPE_WRITE_BUF_PREFERRED_LIMIT 256

#define ATTR_ALIGN(alignment)  __attribute__((aligned(alignment)))
#define ALIGNED(decl, alignment) decl ATTR_ALIGN(alignment)

// Utility struct for maintaining a pipe internal cache.
struct __pipe_internal_buf {
  int end;   // index for a new element
  int size;  // number of elements in the buffer, -1 means a locked buffer
  int limit; // max number of elements before flush
};

// Pipe object is a buffer which consist of:
//   - a header (__pipe_t struct)
//   - a variable-length array for packets
//
// Memory layout is:
//
// +---------------+-------------------------------------------+
// |   __pipe_t    |   (max_packets + K + 1) * packet_size     |
// +---------------+-------------------------------------------+
// ^~~ aligned for a cache line size
//
//   where K is a constant number of packets, depends on a cache
//   settings
typedef struct FILE FILE;

FILE *fopen(__generic const char *filename, __constant char *mode);
int fclose(FILE *fp);
int fflush(FILE *fp);
size_t fread(void *ptr, size_t size_of_elements,
             size_t number_of_elements, FILE *a_file);
size_t fwrite(const void *ptr, size_t size_of_elements,
              size_t number_of_elements, FILE *a_file);

struct __pipe_t {
  int packet_size;
  int max_packets;

  // The fields below are accessible(read, write) from different threads.
  // Alignment needed here to avoid false sharing.
  ALIGNED(volatile atomic_int head, 64);
  ALIGNED(volatile atomic_int tail, 64);

  ALIGNED(struct __pipe_internal_buf read_buf, 64);
  ALIGNED(struct __pipe_internal_buf write_buf, 64);

  FILE *io;

  // Pipe object also has a trailing buffer for packets:
  // char buffer[max_packets * packet_size];
} ATTR_ALIGN(64);

int get_buffer_capacity(__global const struct __pipe_internal_buf* b);
__global void* get_packet_ptr(__global struct __pipe_t* p, int index);
bool reserve_write_buffer(__global struct __pipe_internal_buf* b, int capacity);
bool reserve_read_buffer(__global struct __pipe_internal_buf* b, int capacity);
int advance(__global const struct __pipe_t* p, int index, int offset);
int get_read_capacity(__global struct __pipe_t* p);
int get_write_capacity(__global struct __pipe_t* p);

void __store_pipe_use(__global struct __pipe_t* __private* c,
                      __private int* size, __global struct __pipe_t* p);
void __flush_pipe_read_array(__global struct __pipe_t* __private* arr,
                             __private int* size);
void __flush_pipe_write_array(__global struct __pipe_t* __private* arr,
                              __private int* size);

#endif // __PIPES_INTERNAL_H__
