// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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
#ifndef __PIPE_COMMON_H__
#define __PIPE_COMMON_H__

#include "../backend/libraries/ocl_builtins/pipes-native.h"

#define CACHE_LINE 64
#define INTEL_PIPE_HEADER_RESERVED_SPACE CACHE_LINE * 2

// Define the suffix for SYCL pipe backstore name
static constexpr const char* SYCLPIPE_BS = ".syclpipe.bs";

// The following struct must be in sync with structs defined in CPU/GEN
// Back-Ends Total size:  CACHE_LINE * 2.
//              RT must allocate 128 chars for pipe control at the beginning of
//              contiguous memory. This buffer must be aligned by CACHE_LINE.
typedef struct _tag_pipe_control_intel_t {
  // Total number of packets in the pipe.  This value must be
  // set by the host when the pipe is created. Pipe cannot accommodate
  // more than pipe_max_packets – 1 packets. So RT must allocate memory
  // for one more packet.
  cl_uint pipe_max_packets_plus_one;

  // The pipe head and tail must be set by the host when
  // the pipe is created.  They will probably be set to zero,
  // though as long as head equals tail, it doesn't matter
  // what they are initially set to.
  cl_uint head; // Head Index, for reading: [0, pipe_max_packets)
  cl_uint tail; // Tail Index, for writing: [0, pipe_max_packets)
  char pad0[CACHE_LINE - 3 * sizeof(cl_uint)];

  // This controls whether the pipe is unlocked, locked for
  // reading, or locked for writing.  If it is zero, the pipe
  // is unlocked.  If it is positive, it is locked for writing.
  // If it is negative, it is locked for reading. This must
  // be set to zero by the host when the pipe is created.
  cl_int lock;
  char pad1[CACHE_LINE - sizeof(cl_int)];
} pipe_control_intel_t;

#ifndef _WIN32
#define ATTRIBUTE_UNUSED __attribute__((unused))
#else
#define ATTRIBUTE_UNUSED
#endif // _WIN32

// pipe_get_total_size function are not always used when this header is included
// somewhere, let's silence warning
ATTRIBUTE_UNUSED
static size_t pipe_get_total_size(cl_uint uiPacketSize, cl_uint uiMaxPackets) {
  return INTEL_PIPE_HEADER_RESERVED_SPACE + uiPacketSize * uiMaxPackets;
}

#endif // __PIPE_COMMON_H__
