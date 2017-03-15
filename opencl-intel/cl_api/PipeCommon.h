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

#include "../backend/libraries/ocl_builtins/pipes.h"
#include <algorithm>

static size_t pipe_get_total_size(cl_uint packet_size, cl_uint max_packets) {
  size_t total = sizeof(__pipe_t)       // header
    + packet_size * (max_packets + 1);  // packets (one extra packet
                                        // for a begin/end border)
  return total;
}

static void pipe_init(void* mem, cl_uint packet_size, cl_uint max_packets) {
  if (max_packets == 1) {
    max_packets++;
  }

  __pipe_t* p = (__pipe_t*) mem;

  memset((char*)p, 0, sizeof(__pipe_t));

  p->packet_size = packet_size;
  p->max_packets = max_packets;

  p->read_buf.size = -1;
  p->read_buf.limit = PIPE_READ_BUF_PREFERRED_LIMIT;

  p->write_buf.size = -1;
  p->write_buf.limit = std::min((int)max_packets - 1,
                                PIPE_WRITE_BUF_PREFERRED_LIMIT);
}


#endif // __PIPE_COMMON_H__
