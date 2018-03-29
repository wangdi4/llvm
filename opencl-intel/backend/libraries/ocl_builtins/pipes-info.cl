//==--- pipes-info.cl - auxiliary functions for pipe built-ins -*- C++ -*---==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "pipes.h"
#include "pipes-internal.h"

int __pipe_get_max_packets(int depth, int mode) {
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
  int max_packets = depth > MAX_VL_SUPPORTED_BY_PIPES
    ? depth
    : MAX_VL_SUPPORTED_BY_PIPES;

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

int __pipe_get_total_size(int packet_size, int depth, int mode) {
  size_t total = sizeof(struct __pipe_t)       // header
    + packet_size * __pipe_get_max_packets(depth, mode);
  return total;
}
