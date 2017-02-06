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

  cl_int begin;
  cl_int end;
  cl_int hazard_write_begin;
  cl_int hazard_read_end;
};

static size_t pipe_get_total_size(cl_uint packet_size, cl_uint max_packets) {
  return sizeof(__pipe_t)           // header
    + sizeof(cl_int) * max_packets // flags
    + packet_size * max_packets;  // packets
}

static void pipe_init(void* mem, uint packet_size, uint max_packets) {
  __pipe_t* p = (__pipe_t*) mem;
  p->packet_size = packet_size;
  p->max_packets = max_packets;
  p->begin = -1;
  p->end = 0;
  p->hazard_write_begin = 0;
  p->hazard_read_end = 0;

  // zero flags
  memset((char*)p + sizeof(__pipe_t), 0, sizeof(cl_int) * max_packets);
}


#endif // __PIPE_COMMON_H__
