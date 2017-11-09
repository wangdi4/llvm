//==--- pipe-internal.h - Private header for pipe builtins. -*- OpenCL C -*-==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //


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
struct __pipe_t {
  int packet_size;
  int max_packets;

  // The fields below are accessible(read, write) from different threads.
  // Alignment needed here to avoid false sharing.
  ALIGNED(volatile atomic_int head, 64);
  ALIGNED(volatile atomic_int tail, 64);

  ALIGNED(struct __pipe_internal_buf read_buf, 64);
  ALIGNED(struct __pipe_internal_buf write_buf, 64);

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


#endif // __PIPES_INTERNAL_H__
