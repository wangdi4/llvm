// Copyright (c) 2006-2007 Intel Corporation
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
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES{ } LOSS OF USE, DATA, OR
// PROFITS{ } OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directlytypedef uint _PIPE_TYPE;

#ifndef __PIPE_COMMON_H__
#define __PIPE_COMMON_H__

#ifdef BUILD_FPGA_EMULATOR

#include "../backend/libraries/ocl_builtins/pipes.h"
#include <algorithm>

static size_t pipe_get_total_size(cl_uint packet_size, cl_uint depth) {
  size_t total = sizeof(__pipe_t)       // header
    + packet_size * __pipe_get_max_packets(depth);
  return total;
}

static void pipe_init(void* mem, cl_uint packet_size, cl_uint depth) {
  __pipe_t* p = (__pipe_t*) mem;

  memset((char*)p, 0, sizeof(__pipe_t));

  p->packet_size = packet_size;
  p->max_packets = __pipe_get_max_packets(depth);

  p->read_buf.size = -1;
  p->read_buf.limit = PIPE_READ_BUF_PREFERRED_LIMIT;

  p->write_buf.size = -1;
  // Ensure that write buffer limit is a multiple of max supported vector length
  p->write_buf.limit = std::min((int)p->max_packets,
                                PIPE_WRITE_BUF_PREFERRED_LIMIT)
                       & (- MAX_VL_SUPPORTED_BY_PIPES);
}

#else // BUILD_FPGA_EMULATOR

#define CACHE_LINE 64
#define INTEL_PIPE_HEADER_RESERVED_SPACE CACHE_LINE * 2

// The following struct must be in sync with structs defined in CPU/GEN Back-Ends
// Total size:  CACHE_LINE * 2.
//              RT must allocate 128 chars for pipe control at the beginning of
//              contiguous memory. This buffer must be aligned by CACHE_LINE.
typedef struct _tag_pipe_control_intel_t
{
    // Total number of packets in the pipe.  This value must be 
    // set by the host when the pipe is created. Pipe cannot accommodate
    // more than pipe_max_packets – 1 packets. So RT must allocate memory
    // for one more packet.
    cl_uint pipe_max_packets_plus_one;

    // The pipe head and tail must be set by the host when 
    // the pipe is created.  They will probably be set to zero,
    // though as long as head equals tail, it doesn't matter
    // what they are initially set to.
    cl_uint head;  // Head Index, for reading: [0, pipe_max_packets)
    cl_uint tail;  // Tail Index, for writing: [0, pipe_max_packets)
    char pad0[CACHE_LINE - 3 * sizeof(cl_uint)];

    // This controls whether the pipe is unlocked, locked for
    // reading, or locked for writing.  If it is zero, the pipe
    // is unlocked.  If it is positive, it is locked for writing.
    // If it is negative, it is locked for reading. This must
    // be set to zero by the host when the pipe is created.
    cl_int lock;
    char pad1[CACHE_LINE - sizeof(cl_int)];
} pipe_control_intel_t;


static size_t pipe_get_total_size(cl_uint uiPacketSize, cl_uint uiMaxPackets) {
  return INTEL_PIPE_HEADER_RESERVED_SPACE + uiPacketSize * uiMaxPackets;
}

#endif // BUILD_FPGA_EMULATOR

#endif // __PIPE_COMMON_H__
