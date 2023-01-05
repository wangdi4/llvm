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

#include "pipes-defines.h"
#include "pipes-internal.h"

int __pipe_get_max_packets_fpga(int depth, int mode) {
  // STRICT mode:
  //   if user specifies the depth -> the exact depth the user asked for is used
  //   if user doesn't specify the depth -> use depth = 1
  // DEFAULT mode:
  //   if user specifies the depth -> the exact depth the user asked for is used
  //   if user doesn't specify the depth -> use whataver we want to achieve max
  //     performance
  // IGNORE_DEPTH mode:
  //    Use whatever we want to achieve max performance, ignore user-provided
  //      values
  //
  //  NOTE: in any mode we need to ensure that at least 'depth' packets can be
  //  written without blocking.
  if (mode == CHANNEL_DEPTH_MODE_STRICT) {
    if (depth == 0)
      depth = 1;
    // reserve one extra element b/w head and tail to distinguish full/empty
    // conditions
    return depth + 1;
  }

  if (mode == CHANNEL_DEPTH_MODE_DEFAULT && depth != 0) {
    // reserve one extra element b/w head and tail to distinguish full/empty
    // conditions
    return depth + 1;
  }

  // if (mode == CHANNEL_DEPTH_MODE_IGNORE_DEPTH ||
  //    (mode == CHANNEL_DEPTH_MODE_DEFAULT && depth == 0))
  // pipe max_packets should be more than maximum of supported VL
  int max_packets =
      depth > MAX_VL_SUPPORTED_BY_PIPES ? depth : MAX_VL_SUPPORTED_BY_PIPES;

  // reserve one extra element b/w head and tail to distinguish full/empty
  // conditions
  max_packets += 1;

  // We must ensure that at least 'depth' packets can be written without
  // blocking. Write cache can block us doing so, because we try to reserve at
  // least 'limit' packets for writing, before performing an actual write.
  //
  // If we have 'depth - 1' packets written, and max_packets == 'depth + 1' (see
  // above), we cannot write last packet, because we wait until 'limit' packets
  // would be available.
  max_packets += PIPE_WRITE_BUF_PREFERRED_LIMIT - 1;

  return max_packets;
}

int __pipe_get_total_size_fpga(int packet_size, int depth, int mode) {
  size_t total = sizeof(struct __pipe_t) // header
                 + packet_size * __pipe_get_max_packets_fpga(depth, mode);
  return total;
}
