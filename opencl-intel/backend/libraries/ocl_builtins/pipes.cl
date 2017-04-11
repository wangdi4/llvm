// Copyright (c) 2017 Intel Corporation
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
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly


// Summary
// ------------
//
// This is the implementation of Single Producer Single Consumer (SPSC)
// queue used for FPGA emulation of pipes/channels.
//
// In ChannelPipeTransformation pass we replace all channel usage with
// corresponding calls to pipes, so we don't need both channel BIs and
// pipe BIs: we can use only pipe ones.
//
// FPGA restrictions guarantee that only one kernel would read (write)
// from (to) a pipe, so we have SPSC pattern here.
//
// Pipe object is a buffer which consist of:
//   - header (__pipe_t struct)
//   - variable-length array for packets
//
// Memory layout is:
// +---+---+---+---+---+---+---+---+---+---+---+---+
// |   __pipe_t    |   max_packets * packet_size   |
// +---+---+---+---+---+---+---+---+---+---+---+---+
// ^~~ aligned for packet_align
//
//
// Pipe object
// ------------
// Pipe header has 2 indexes to maintain a circular buffer:
//   - head points to the beginning of elements stored in a pipe
//   - tail points to the end, where new elements should go
//
//   head         tail
//   ~~~~v        ~~~v
// |---+---+---+---+---+---+---|
// |   | x | x | x |   |   |   |
// |---+---+---+---+---+---+---|
//
//   - when head == tail we assume that pipe has no elements
//   - when a distance between tail and head is 1, we assume that pipe is
//     full, i.e. one element is reserved to distinguish betweeen full
//     and empty conditions
//
//   tail    head
//   v~~~    v~~~
// |---+---+---+---+---+---+---|
// | x |   | x | x | x | x | x |  // pipe is full - 1 element reserved
// |---+---+---+---+---+---+---|  // b/w head and tail
//
//
// Bufferinng
// ------------
// To avoid updating atomics head and tail, we buffer write and read
// operation, updating only private struct variables (__pipe_internal_buf
// struct).
//
//   head         tail
//   ~~~~v        ~~~v
// |---+---+---+---+---+---+---|
// |   | x | x | x | b | b |   | // 2 elements are buffered
// |---+---+---+---+---+---+---|
//
// When buffer is full - update head or tail.
//
//   head                 tail
//   ~~~~v                ~~~v
// |---+---+---+---+---+---+---|
// |   | x | x | x | x | x |   |
// |---+---+---+---+---+---+---|
//
// We lock the buffer by setting size to -1, to indicate that
// writes or reads are not allowed, because we do not have free memory
// to guarantee non-blocking flush.


#include "pipes.h"

// There are no declarations of OpenCL 2.0 builtins in opencl-c.h for named
// address space but in the library they has to be called directly because the
// library won't be handled by "Generic Address Resolution" passes. So declare
// them here.
#define __ovld __attribute__((overloadable))
void __ovld atomic_init(__global volatile atomic_int *object, int value);
int __ovld atomic_load_explicit(__global volatile atomic_int *object,
                                memory_order order);
void __ovld atomic_store_explicit(__global volatile atomic_int *object,
                                  int desired, memory_order order);
// Debug switches
#define DEBUG_ASSERTS 0

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#if DEBUG_ASSERTS
#define ASSERT(cond) { if (!(cond)) {printf(">> ASSERT at "              \
                                          STRINGIFY(__FILE__) ":"        \
                                          STRINGIFY(__LINE__) ": " #cond \
                                          "\n");                         \
                                   *((volatile int*)NULL) = 0xbeef;}}
#else
#define ASSERT(cond) do {} while(0);
#endif

static bool is_buffer_full(const struct __pipe_internal_buf* b) {
  return b->size >= b->limit;
}

int get_buffer_capacity(const struct __pipe_internal_buf* b) {
    return b->limit - b->size;
}

__global void* get_packet_ptr(__global struct __pipe_t* p, int index) {
  // memory for packets is co-allocated *after* the __pipe_t struct
  __global char* packets_begin = (__global char*)(p + 1);
  return packets_begin + index * p->packet_size;
}

bool reserve_write_buffer(struct __pipe_internal_buf* b, int capacity) {
  if (!(capacity >= b->limit))
    return false; // pipe is full

  b->size = 0;
  return true;
}

bool reserve_read_buffer(struct __pipe_internal_buf* b, int capacity) {
  b->limit = min(capacity, PIPE_READ_BUF_PREFERRED_LIMIT);
  if (!(b->limit))
    return false; // pipe doesn't contain enough elements to read

  b->size = 0;
  return true;
}

/// Return next nth item from circular buffer
int advance(__global const struct __pipe_t* p, int index, int offset) {
  ASSERT(offset < p->max_packets);
  ASSERT(offset >= 0);
  int new = index + offset;
  if (new >= p->max_packets) {
    new -= p->max_packets;
  }
  return new;
}

/// For given \p index_from and \p index_to compute the number of elements
/// between them. The function behaves exactly as std::distance.
static int dist(__global const struct __pipe_t* p,
                    int index_from, int index_to) {
  int res = index_from <= index_to ? index_to - index_from
    : p->max_packets - index_from + index_to;
  ASSERT(res >= 0);
  return res;
}
int get_read_capacity(__global struct __pipe_t* p) {
  int head = atomic_load_explicit(&p->head, memory_order_relaxed);
  int tail = atomic_load_explicit(&p->tail, memory_order_acquire);

  int result = dist(p, head, tail);
  return result;
}

int get_write_capacity(__global struct __pipe_t* p) {
  int head = atomic_load_explicit(&p->head, memory_order_acquire);
  int tail = atomic_load_explicit(&p->tail, memory_order_relaxed);

  int result = (tail == head)
    ? p->max_packets - 1 // pipe is empty
    : dist(p, tail, head) - 1; // reserve one element b/w head and tail
  return result;
}

void __pipe_init_intel(__global struct __pipe_t* p,
                       int packet_size, int max_packets) {
  // p->max_packets should be more then maximum of supported VL
  if (max_packets <= MAX_VL_SUPPORTED_BY_PIPES)
    max_packets = MAX_VL_SUPPORTED_BY_PIPES;

  p->packet_size = packet_size;
  p->max_packets = max_packets + 1; // reserve one element b/w head and tail
  atomic_init(&p->head, 0);
  atomic_init(&p->tail, 0);

  p->read_buf.end = 0;
  p->read_buf.size = -1;
  p->read_buf.limit = PIPE_READ_BUF_PREFERRED_LIMIT;

  p->write_buf.end = 0;
  p->write_buf.size = -1;

  // Ensure that write buffer limit is a multiple of max supported vector length
  p->write_buf.limit = min(max_packets, PIPE_WRITE_BUF_PREFERRED_LIMIT) &
                       (- MAX_VL_SUPPORTED_BY_PIPES);
}

void __pipe_init_array_intel(__global struct __pipe_t* __global* p,
                             int array_size, int packet_size, int max_packets) {
  for (int i = 0; i < array_size; ++i) {
    __pipe_init_intel(p[i], packet_size, max_packets);
  }
}

void __flush_read_pipe(__global struct __pipe_t* p) {
  p->read_buf.size = -1;
  atomic_store_explicit(&p->head, p->read_buf.end, memory_order_release);
}

void __flush_read_pipe_array(__global struct __pipe_t* __global* p,
                             int array_size) {
  for (int i = 0; i < array_size; ++i) {
    __flush_read_pipe(p[i]);
  }
}

void __flush_write_pipe(__global struct __pipe_t* p) {
  p->write_buf.size = -1;
  atomic_store_explicit(&p->tail, p->write_buf.end, memory_order_release);
}

void __flush_write_pipe_array(__global struct __pipe_t* __global* p,
                             int array_size) {
  for (int i = 0; i < array_size; ++i) {
    __flush_write_pipe(p[i]);
  }
}

int __read_pipe_2_intel(__global struct __pipe_t* p, void* dst) {
  __global struct __pipe_internal_buf* buf = &p->read_buf;

  if (buf->size < 0) {
    // Try to reserve a buffer
    if (!reserve_read_buffer(buf, get_read_capacity(p)))
      return -1;
  }

  __builtin_memcpy(dst, get_packet_ptr(p, buf->end), p->packet_size);
  buf->end = advance(p, buf->end, 1);
  buf->size++;

  if (is_buffer_full(buf)) {
    __flush_read_pipe(p);
  }

  return 0;
}

int __write_pipe_2_intel(__global struct __pipe_t* p, void* src) {
  __global struct __pipe_internal_buf* buf = &p->write_buf;

  if (buf->size < 0) {
    // Try to reserve a buffer
    if (!reserve_write_buffer(buf, get_write_capacity(p)))
      return -1;
  }

  __builtin_memcpy(get_packet_ptr(p, buf->end), src, p->packet_size);
  buf->end = advance(p, buf->end, 1);
  buf->size++;

  if (is_buffer_full(buf)) {
    __flush_write_pipe(p);
  }

  return 0;
}

int __read_pipe_2_bl_intel(__global struct __pipe_t* p, void* dst) {
  while(__read_pipe_2_intel(p, dst)) {
  }
  return 0;
}

int __write_pipe_2_bl_intel(__global struct __pipe_t* p, void* src) {
  while(__write_pipe_2_intel(p, src)) {
  }
  return 0;
}
