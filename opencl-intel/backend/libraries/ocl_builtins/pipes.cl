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
//     full, i.e. one element is reserved to distinguish between full
//     and empty conditions
//
//    tail    head
//    ~~~v   v~~~~
// |---+---+---+---+---+---+---|
// | x |   | x | x | x | x | x |  // pipe is full - 1 element reserved
// |---+---+---+---+---+---+---|  // b/w head and tail
//
//
// Buffering
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

#include "pipes-defines.h"
#include "pipes-internal.h"

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
#define NOINLINE_SWITCH 0

#if NOINLINE_SWITCH
#define DEBUG_NOINLINE __attribute__((noinline))
#else
#define DEBUG_NOINLINE
#endif

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#if DEBUG_ASSERTS
#define ASSERT(cond)                                                           \
  {                                                                            \
    if (!(cond)) {                                                             \
      printf(">> ASSERT at " STRINGIFY(__FILE__) ":" STRINGIFY(                \
          __LINE__) ": " #cond "\n");                                          \
      *((volatile int *)NULL) = 0xbeef;                                        \
    }                                                                          \
  }
#else
#define ASSERT(cond)                                                           \
  do {                                                                         \
  } while (0);
#endif

static bool is_buffer_full(__global const struct __pipe_internal_buf *b) {
  return b->size >= b->limit;
}

int get_buffer_capacity(const __global struct __pipe_internal_buf *b) {
  return b->limit - b->size;
}

__global void *get_packet_ptr(__global struct __pipe_t *p, int index) {
  // memory for packets is co-allocated *after* the __pipe_t struct
  __global char *packets_begin = (__global char *)(p + 1);
  return packets_begin + index * p->packet_size;
}

bool reserve_write_buffer(__global struct __pipe_internal_buf *b,
                          int capacity) {
  if (!(capacity >= b->limit))
    return false; // pipe is full

  b->size = 0;
  return true;
}

bool reserve_read_buffer(__global struct __pipe_internal_buf *b, int capacity) {
  b->limit = min(capacity, PIPE_READ_BUF_PREFERRED_LIMIT);
  if (!(b->limit))
    return false; // pipe doesn't contain enough elements to read

  b->size = 0;
  return true;
}

/// Return next nth item from circular buffer
int advance(const __global struct __pipe_t *p, int index, int offset) {
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
static int dist(__global const struct __pipe_t *p, int index_from,
                int index_to) {
  int res = index_from <= index_to ? index_to - index_from
                                   : p->max_packets - index_from + index_to;
  ASSERT(res >= 0);
  return res;
}
int get_read_capacity(__global struct __pipe_t *p) {
  int head = atomic_load_explicit(&p->head, memory_order_relaxed);
  int tail = atomic_load_explicit(&p->tail, memory_order_acquire);

  int result = dist(p, head, tail);
  return result;
}

int get_write_capacity(__global struct __pipe_t *p) {
  int head = atomic_load_explicit(&p->head, memory_order_acquire);
  int tail = atomic_load_explicit(&p->tail, memory_order_relaxed);

  int result = (tail == head) ? p->max_packets - 1 // pipe is empty
                              : dist(p, tail, head) -
                                    1; // reserve one element b/w head and tail
  return result;
}

void __pipe_init_fpga(__global void *pp, int packet_size, int depth, int mode) {
  __pipe_init_ext_fpga(pp, packet_size, depth, mode, 0);
}

void __pipe_init_ext_fpga(__global void *pp, int packet_size, int depth,
                          int mode, int protocol) {
  __global struct __pipe_t *p = (__global struct __pipe_t *)pp;
  p->packet_size = packet_size;
  p->max_packets =
      depth == -1 ? depth : __pipe_get_max_packets_fpga(depth, mode);

  p->io = NULL;
  p->protocol = protocol;

  if (depth == -1) {
    // preserve at list 2 empty elements in the circular linked list at the
    // beginning.
    p->hp_write_ptr = (__hostpipe_packet *)malloc(sizeof(__hostpipe_packet));
    p->hp_write_ptr->data = malloc(packet_size);
    p->hp_write_ptr->next =
        (__hostpipe_packet *)malloc(sizeof(__hostpipe_packet));
    p->hp_write_ptr->next->data = malloc(packet_size);
    p->hp_write_ptr->next->next = p->hp_write_ptr;
    // At the beginging read_buf and write_buf points to the same element.
    p->hp_read_ptr = p->hp_write_ptr;
  } else {
    atomic_init(&p->head, 0);
    atomic_init(&p->tail, 0);

    p->read_buf.end = 0;
    p->read_buf.size = -1;
    p->read_buf.limit = PIPE_READ_BUF_PREFERRED_LIMIT;

    p->write_buf.end = 0;
    p->write_buf.size = -1;

    if (mode == CHANNEL_DEPTH_MODE_STRICT ||
        (mode == CHANNEL_DEPTH_MODE_DEFAULT && depth != 0)) {
      // See notes in __pipe_get_max_packets function: "We must ensure that at
      // least 'depth' packets can be written without blocking..."
      p->write_buf.limit = 1;
    } else {
      // Limit pipe write buffer by pipe write capacity, which is a maximum
      // capacity, since the pipe is empty.
      int write_buf_limit =
          min(get_write_capacity(p), PIPE_WRITE_BUF_PREFERRED_LIMIT);
      // Ensure that write buffer limit is a multiple of max supported vector
      // length
      p->write_buf.limit =
          write_buf_limit - (write_buf_limit % MAX_VL_SUPPORTED_BY_PIPES);
    }
  }
}

void __pipe_release_fpga(__global void *pp) {
  __global struct __pipe_t *p = (__global struct __pipe_t *)pp;
  if (p->io != NULL)
    fclose(p->io);

  if (p->max_packets == -1) { // host pipe
    __hostpipe_packet *indicator = p->hp_write_ptr->next;
    p->hp_write_ptr->next = NULL; // cut the link

    while (indicator != NULL) {
      __hostpipe_packet *release = indicator;
      indicator = indicator->next;
      // Release data.
      if (release->data)
        free(release->data);
      // Release packet itself.
      free(release);
    }
  }
}

void __pipe_init_array_fpga(__global void *__global *p, int array_size,
                            int packet_size, int depth, int mode) {
  __pipe_init_array_ext_fpga(p, array_size, packet_size, depth, mode, 0);
}

void __pipe_init_array_ext_fpga(__global void *__global *p, int array_size,
                                int packet_size, int depth, int mode,
                                int protocol) {
  for (int i = 0; i < array_size; ++i) {
    __pipe_init_ext_fpga(p[i], packet_size, depth, mode, protocol);
  }
}

void __flush_read_pipe(__global void *pp) {
  __global struct __pipe_t *p = (__global struct __pipe_t *)pp;
  p->read_buf.size = -1;
  atomic_store_explicit(&p->head, p->read_buf.end, memory_order_release);
}

void __flush_write_pipe(__global void *pp) {
  __global struct __pipe_t *p = (__global struct __pipe_t *)pp;
  p->write_buf.size = -1;
  atomic_store_explicit(&p->tail, p->write_buf.end, memory_order_release);
}

DEBUG_NOINLINE
int __read_pipe_2_fpga(read_only pipe uchar pp, void *dst, uint size,
                       uint align) {
  __global struct __pipe_t *p = __ocl_rpipe2ptr(pp);
  __global struct __pipe_internal_buf *buf = &p->read_buf;

  if (p->max_packets == -1) {
    __hostpipe_packet *hp_read_ptr = p->hp_read_ptr;
    __hostpipe_packet *hp_write_ptr = p->hp_write_ptr;

    if (hp_read_ptr == NULL || hp_read_ptr->data == NULL ||
        hp_read_ptr == hp_write_ptr)
      return -1; // Initialized failed or empty;

    __builtin_memcpy(dst, hp_read_ptr->data, size);
    p->hp_read_ptr = hp_read_ptr->next;
    return 0;
  }

  if (buf->size < 0) {
    // Try to reserve a buffer
    if (!reserve_read_buffer(buf, get_read_capacity(p)))
      return -1;
  }

  ASSERT(size == p->packet_size && "Runtime and compiler sizes are different.");
  __builtin_memcpy(dst, get_packet_ptr(p, buf->end), size);

  buf->end = advance(p, buf->end, 1);
  buf->size++;

  if (is_buffer_full(buf)) {
    __flush_read_pipe(p);
  }

  return 0;
}

// FIXME: pipe protocol is defined in a header provided by fpga team which is
// not included in sycl
#define AVALON_MM 2

DEBUG_NOINLINE
int __write_pipe_2_fpga(write_only pipe uchar pp, const void *src, uint size,
                        uint align) {
  __global struct __pipe_t *p = __ocl_wpipe2ptr(pp);
  __global struct __pipe_internal_buf *buf = &p->write_buf;

  if (p->max_packets == -1) {
    __hostpipe_packet *hp_read_ptr = p->hp_read_ptr;
    __hostpipe_packet *hp_write_ptr = p->hp_write_ptr;

    if (hp_write_ptr == NULL || hp_write_ptr->data == NULL) {
      return -1; // pipe initialization failed.
    }
    if (hp_write_ptr->next == hp_read_ptr) { // full
                                             // allocat one packet
      __hostpipe_packet *tmppacket =
          (__hostpipe_packet *)malloc(sizeof(__hostpipe_packet));
      if (tmppacket == NULL)
        return -1;

      tmppacket->data = malloc(p->packet_size);

      if (tmppacket->data == NULL)
        return -1;

      tmppacket->next = hp_write_ptr->next;
      hp_write_ptr->next = tmppacket;
    }
    __builtin_memcpy(hp_write_ptr->data, src, size);
    p->hp_write_ptr = hp_write_ptr->next;
    return 0;
  }

  if (buf->size < 0) {
    // Try to reserve a buffer
    if (!reserve_write_buffer(buf, get_write_capacity(p))) {
      int protocol = p->protocol;
      // AVALON_MM is a non-blocking pipe
      if (protocol == AVALON_MM) {
        // When pipe is full, overwrite the latest packet
        int end = buf->end - 1;
        if (end < 0) {
          end += p->max_packets;
        }
        __builtin_memcpy(get_packet_ptr(p, end), src, size);
        return 0;
      }
      return -1;
    }
  }

  ASSERT(size == p->packet_size && "Runtime and compiler sizes are different.");
  __builtin_memcpy(get_packet_ptr(p, buf->end), src, size);

  buf->end = advance(p, buf->end, 1);
  buf->size++;

  if (is_buffer_full(buf)) {
    __flush_write_pipe(p);
  }

  return 0;
}

DEBUG_NOINLINE
int __read_pipe_2_io_fpga(read_only pipe uchar pp, void *dst,
                          const char *dstName, uint size, uint align) {
  __global struct __pipe_t *p = __ocl_rpipe2ptr(pp);
  if (p->io == NULL)
    p->io = fopen(dstName, "rb");
  if (p->io == NULL)
    return -2;

  ASSERT(size == p->packet_size && "Runtime and compiler sizes are different.");
  if (fread(dst, size, 1, p->io) == 0)
    return -1;

  return 0;
}

DEBUG_NOINLINE
int __write_pipe_2_io_fpga(write_only pipe uchar pp, const void *src,
                           const char *srcName, uint size, uint align) {
  __global struct __pipe_t *p = __ocl_wpipe2ptr(pp);
  if (p->io == NULL)
    p->io = fopen(srcName, "wb");
  if (p->io == NULL)
    return -2;

  ASSERT(size == p->packet_size && "Runtime and compiler sizes are different.");
  if (fwrite(src, size, 1, p->io) == 0)
    return -1;

  fflush(p->io);

  return 0;
}

DEBUG_NOINLINE
void __store_write_pipe_use(__global void *__private *__private arr,
                            __private int *size, write_only pipe uchar pp) {
  __global struct __pipe_t *p = __ocl_wpipe2ptr(pp);
  for (int i = 0; i < *size; i++) {
    if (arr[i] == p)
      return;
  }
  arr[*size] = p;
  ++(*size);
}

DEBUG_NOINLINE
void __store_read_pipe_use(__global void *__private *__private arr,
                           __private int *size, read_only pipe uchar pp) {
  __global struct __pipe_t *p = __ocl_rpipe2ptr(pp);
  for (int i = 0; i < *size; i++) {
    if (arr[i] == p)
      return;
  }
  arr[*size] = p;
  ++(*size);
}

DEBUG_NOINLINE
void __flush_pipe_read_array(__global void *__private *arr,
                             __private int *size) {
  for (int i = 0; i < *size; ++i)
    __flush_read_pipe(arr[i]);
}

DEBUG_NOINLINE
void __flush_pipe_write_array(__global void *__private *arr,
                              __private int *size) {
  for (int i = 0; i < *size; ++i)
    __flush_write_pipe(arr[i]);
}
