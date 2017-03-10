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

// The following struct must be in sync with structs defined in pipes.cl
struct __pipe_t {
  cl_int packet_size;
  cl_int max_packets;

  cl_int read_lock;
  cl_int write_lock;

  cl_int read_begin;
  cl_int read_end;

  cl_int write_begin;
  cl_int write_end;
};

static size_t pipe_get_total_size(cl_uint packet_size, cl_uint max_packets) {
  size_t total = sizeof(__pipe_t)           // header
    + sizeof(cl_int) * (max_packets + 1) // flags
    + packet_size * (max_packets + 1);  // packets (one extra packet
                                        // for a begin/end border)

  printf("OCLRT: pipe (%u x %u)total size = %zu bytes\n", packet_size, max_packets, total);
  return total;
}

static void pipe_init(void* mem, cl_uint packet_size, cl_uint max_packets) {
  __pipe_t* p = (__pipe_t*) mem;
  // zero both header and flags
  memset((char*)p, 0, sizeof(cl_int) * (max_packets + 1));

  p->packet_size = packet_size;
  p->max_packets = max_packets + 1;
}


#endif // __PIPE_COMMON_H__
