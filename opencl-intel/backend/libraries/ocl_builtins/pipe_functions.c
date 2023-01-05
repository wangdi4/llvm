// Copyright (c) 20013 Intel Corporation
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

// This source file contains an implementation of OpenCL 2.0 built-in pipe
// functions

#if defined(_DEBUG)
#define INTEL_PIPE_DPF(format, args...) (void)printf
#else
#define INTEL_PIPE_DPF(format, args...)
#endif

#if __OPENCL_C_VERSION__ >= 200

// TODO : This macro is not defined in spec. It can be removed.
#define __PIPE_RESERVE_ID_VALID_BIT (1U << 30)

#define ALWAYS_INLINE __attribute__((always_inline))
#define OVERLOADABLE __attribute__((overloadable))

// There are no declarations of OpenCL 2.0 builtins in opencl_.h for named
// address space but in the library they has to be called directly because the
// library won't be handled by "Generic Address Resolution" passes. So declare
// them here.
bool OVERLOADABLE atomic_compare_exchange_strong_explicit(
    volatile __global atomic_int *object, __private int *expected, int desired,
    memory_order success, memory_order failure, memory_scope scope);
bool OVERLOADABLE atomic_compare_exchange_strong_explicit(
    volatile __global atomic_uint *object, __private uint *expected,
    uint desired, memory_order success, memory_order failure,
    memory_scope scope);

int OVERLOADABLE atomic_load_explicit(volatile __global atomic_int *object,
                                      memory_order order, memory_scope scope);
uint OVERLOADABLE atomic_load_explicit(volatile __global atomic_uint *object,
                                       memory_order order, memory_scope scope);

int OVERLOADABLE atomic_fetch_add_explicit(volatile __global atomic_int *object,
                                           int operand, memory_order order,
                                           memory_scope scope);
uint OVERLOADABLE
atomic_fetch_add_explicit(volatile __global atomic_uint *object, uint operand,
                          memory_order order, memory_scope scope);

int OVERLOADABLE atomic_fetch_sub_explicit(volatile __global atomic_int *object,
                                           int operand, memory_order order,
                                           memory_scope scope);
uint OVERLOADABLE
atomic_fetch_sub_explicit(volatile __global atomic_uint *object, uint operand,
                          memory_order order, memory_scope scope);

// pipe_control_intel_t structure MUST BE ALIGNED with the one defined in
// src/cl_api/PipeCommon.h
#define INTEL_PIPE_HEADER_RESERVED_SPACE 128
// Total size:  129 ( + 1 because of a variable length array).
//              RT must allocate 128 chars for pipe control at the beginning of
//              contiguous memory. This buffer must be aligned by CACHE_LINE.
#define CACHE_LINE 64
typedef struct _tag_pipe_control_intel_t {
  // The pipe packet size is always passed as an implicit argument (i32
  // immediate)

  // Total number of packets in the pipe.  This value must be
  // set by the host when the pipe is created. Pipe cannot accommodate
  // more than pipe_max_packets_plus_one – 1 packets. So RT must allocate memory
  // for one more packet.
  const uint pipe_max_packets_plus_one;

  // The pipe head and tail must be set by the host when
  // the pipe is created.  They will probably be set to zero,
  // though as long as head equals tail, it doesn't matter
  // what they are initially set to.
  volatile atomic_uint
      head; // Head Index, for reading: [0, pipe_max_packets_plus_one)
  volatile atomic_uint
      tail; // Tail Index, for writing: [0, pipe_max_packets_plus_one)
  char pad0[CACHE_LINE - 2 * sizeof(atomic_uint) - sizeof(uint)];

  // This controls whether the pipe is unlocked, locked for
  // reading, or locked for writing.  If it is zero, the pipe
  // is unlocked.  If it is positive, it is locked for writing.
  // If it is negative, it is locked for reading. This must
  // be set to zero by the host when the pipe is created.
  volatile atomic_int lock;
  char pad1[CACHE_LINE - sizeof(atomic_int)];
  // The end of the control structure as it must be defined in the
  // src/cl_api/PipeCommon.h

  // Packets storage begins right after the pipe control.
  // Compiler will calculate the offset to that storage at places
  // there it will find accesses to the "base" array.
  char base[1];
} pipe_control_intel_t;

#define RTOS(r) ((size_t)(__builtin_astype((r), void *)))
#define STOR(s) (__builtin_astype(((void *)(s)), reserve_id_t))
// Defined in LLVM IR, because OpenCL standard does not allow cast from pipe
// (opaque pointer) to other pointer type.
__global pipe_control_intel_t *__ocl_wpipe2ptr(write_only pipe uchar p);
__global pipe_control_intel_t *__ocl_rpipe2ptr(read_only pipe uchar p);

/////////////////////////////////////////////////////////////////////
// Pipe Helper Functions (static)

ALWAYS_INLINE static uint advance(__global pipe_control_intel_t *p, uint base,
                                  uint stride) {
  return select(base + stride, base + stride - p->pipe_max_packets_plus_one,
                (p->pipe_max_packets_plus_one <= base + stride));
}

ALWAYS_INLINE static reserve_id_t create_reserve_id(uint idx) {
  return STOR((size_t)idx | __PIPE_RESERVE_ID_VALID_BIT);
}

ALWAYS_INLINE static uint extract_index(reserve_id_t rid) {
  return (uint)(RTOS(rid) & ~__PIPE_RESERVE_ID_VALID_BIT);
}

ALWAYS_INLINE static bool
intel_lock_pipe_read(__global pipe_control_intel_t *p) {
  int lock = atomic_load_explicit(&p->lock, memory_order_relaxed,
                                  memory_scope_all_svm_devices);
  while (lock <= 0) {
    int newLock = lock - 1;
    if (atomic_compare_exchange_strong_explicit(
            &p->lock, &lock, newLock, memory_order_relaxed,
            memory_order_relaxed, memory_scope_all_svm_devices)) {
      return true;
    }
  }
  return false;
}

ALWAYS_INLINE static void
intel_unlock_pipe_read(__global pipe_control_intel_t *p) {
  atomic_fetch_add_explicit(&p->lock, 1, memory_order_relaxed,
                            memory_scope_all_svm_devices);
  // OK to inc, since we must have locked.
}

ALWAYS_INLINE static bool
intel_lock_pipe_write(__global pipe_control_intel_t *p) {
  int lock = atomic_load_explicit(&p->lock, memory_order_relaxed,
                                  memory_scope_all_svm_devices);
  while (lock >= 0) {
    int newLock = lock + 1;
    if (atomic_compare_exchange_strong_explicit(
            &p->lock, &lock, newLock, memory_order_relaxed,
            memory_order_relaxed, memory_scope_all_svm_devices)) {
      return true;
    }
  }
  return false;
}

ALWAYS_INLINE static void
intel_unlock_pipe_write(__global pipe_control_intel_t *p) {
  atomic_fetch_sub_explicit(&p->lock, 1, memory_order_relaxed,
                            memory_scope_all_svm_devices);
  // OK to dec, since we must have locked.
}

/////////////////////////////////////////////////////////////////////
// Work Item Reservations
reserve_id_t __reserve_read_pipe(read_only pipe uchar pipe_, uint num_packets,
                                 uint size_of_packet,
                                 uint alignment_of_packet) {
  INTEL_PIPE_DPF("ENTER: reserve_read_pipe( num_packets = %d)\n", num_packets);
  __global pipe_control_intel_t *p = __ocl_rpipe2ptr(pipe_);
  reserve_id_t retVal = CLK_NULL_RESERVE_ID;

  // The maximum possible reservation number is (_pipe_max_packets_plus_one - 1)
  // packets.
  if (num_packets >= p->pipe_max_packets_plus_one || 0 == num_packets) {
    INTEL_PIPE_DPF("\t reserve_read_pipe: Sanity check failed!  num_packets = "
                   "%d, pipe_max_packets_plus_one = %d\n",
                   num_packets, p->pipe_max_packets_plus_one);
  } else if (intel_lock_pipe_read(p)) {
    uint head = atomic_load_explicit(&p->head, memory_order_acquire,
                                     memory_scope_all_svm_devices);
    const uint tail = atomic_load_explicit(&p->tail, memory_order_relaxed,
                                           memory_scope_all_svm_devices);

    while (true) {
      const uint newHead = advance(p, head, num_packets);
      bool wrap = newHead < head;
      INTEL_PIPE_DPF(
          "\t reserve_read_pipe: Initially, head = %d, new head = %d\n", head,
          newHead);

      if (!wrap && (head <= tail && tail < newHead)) // Underflow
      {
        INTEL_PIPE_DPF("\t reserve_read_pipe: Underflow!  num_packets = %d, "
                       "head = %d, tail = %d\n",
                       num_packets, head, tail);
        break;
      } else if (wrap && (head <= tail || tail < newHead)) {
        INTEL_PIPE_DPF("\t reserve_read_pipe: Wrap and Underflow!  num_packets "
                       "= %d, head = %d, tail = %d\n",
                       num_packets, head, tail);
        break;
      }

      if (atomic_compare_exchange_strong_explicit(
              &p->head, &head, newHead, memory_order_release,
              memory_order_relaxed, memory_scope_all_svm_devices)) {
        retVal = create_reserve_id(head);
        // the lock must be unlocked with following commit
        break; // Success.
      } else {
        INTEL_PIPE_DPF("\t read_pipe: Iterate!  old head = %d, new head = %d\n",
                       head, newHead);
      }
    }

    if (RTOS(retVal) == RTOS(CLK_NULL_RESERVE_ID)) {
      intel_unlock_pipe_read(p);
    }
    // Else: note, no unlock!  The pipe will be unlocked as part of committing
    // the reservation.
  }

  INTEL_PIPE_DPF("EXIT: reserve_read_pipe returned %08X\n", retVal);
  return retVal;
}

reserve_id_t __reserve_write_pipe(write_only pipe uchar pipe_, uint num_packets,
                                  uint size_of_packet,
                                  uint alignment_of_packet) {
  INTEL_PIPE_DPF("ENTER: reserve_write_pipe( num_packets = %d)\n", num_packets);
  __global pipe_control_intel_t *p = __ocl_wpipe2ptr(pipe_);
  reserve_id_t retVal = CLK_NULL_RESERVE_ID;

  if (num_packets >= p->pipe_max_packets_plus_one || 0 == num_packets) {
    INTEL_PIPE_DPF("\t reserve_write_pipe: Sanity check failed!  num_packets = "
                   "%d, pipe_max_packets_plus_one = %d\n",
                   num_packets, p->pipe_max_packets_plus_one);
  } else if (intel_lock_pipe_write(p)) {
    uint tail = atomic_load_explicit(&p->tail, memory_order_acquire,
                                     memory_scope_all_svm_devices);
    const uint head = atomic_load_explicit(&p->head, memory_order_relaxed,
                                           memory_scope_all_svm_devices);

    while (true) {
      const uint newTail = advance(p, tail, num_packets);
      INTEL_PIPE_DPF(
          "\t reserve_write_pipe: Initially, tail = %d, new tail = %d\n", tail,
          newTail);
      bool wrap = newTail < tail;

      if (!wrap && (tail < head && head <= newTail)) {
        INTEL_PIPE_DPF("\t reserve_write_pipe: Overflow!  num_packets = %d, "
                       "head = %d, tail = %d\n",
                       num_packets, head, tail);
        break;
      } else if (wrap && (tail < head || head <= newTail)) {
        INTEL_PIPE_DPF("\t reserve_write_pipe: Wrap + Overflow!  num_packets = "
                       "%d, pipe_max_packets_plus_one = %d, head = %d, tail = "
                       "%d\n",
                       num_packets, p->pipe_max_packets_plus_one, head, tail);
        break;
      }

      if (atomic_compare_exchange_strong_explicit(
              &p->tail, &tail, newTail, memory_order_release,
              memory_order_relaxed, memory_scope_all_svm_devices)) {
        retVal = create_reserve_id(tail);
        break; // Success.
        // the lock must be unlocked by the following commit
      } else {
        INTEL_PIPE_DPF(
            "\t reserve_write_pipe: Iterate!  old tail = %d, new tail = %d\n",
            tail, newTail);
      }
    }

    if (RTOS(retVal) == RTOS(CLK_NULL_RESERVE_ID)) {
      intel_unlock_pipe_write(p);
    }
    // Otherwise, note: No unlock!  The pipe will be unlocked as part of
    // committing the reservation.
  }

  INTEL_PIPE_DPF("EXIT: reserve_write_pipe returned %08X\n", retVal);
  return retVal;
}

void __commit_read_pipe(read_only pipe uchar pipe_, reserve_id_t reserve_id,
                        uint size_of_packet, uint alignment_of_packet) {
  INTEL_PIPE_DPF("ENTER: commit_read_pipe( reserve_id = %08X)\n", reserve_id);
  __global pipe_control_intel_t *p = __ocl_rpipe2ptr(pipe_);

  intel_unlock_pipe_read(p);
  INTEL_PIPE_DPF("EXIT: commit_read_pipe\n");
}

void __commit_write_pipe(write_only pipe uchar pipe_, reserve_id_t reserve_id,
                         uint size_of_packet, uint alignment_of_packet) {
  INTEL_PIPE_DPF("ENTER: commit_write_pipe( reserve_id = %08X)\n", reserve_id);
  __global pipe_control_intel_t *p = __ocl_wpipe2ptr(pipe_);

  intel_unlock_pipe_write(p);
  INTEL_PIPE_DPF("EXIT: commit_write_pipe\n");
}

/////////////////////////////////////////////////////////////////////
// Reads and Writes with Reservations
// The reservation functions lock the pipe, so we don't need to
// re-lock here.

int __read_pipe_4(read_only pipe uchar pipe_, reserve_id_t reserve_id,
                  uint index, void *data, uint size_of_packet,
                  uint alignment_of_packet) {
  INTEL_PIPE_DPF("ENTER: read_pipe( reserve_id = %08X, index = %d)\n",
                 reserve_id, index);
  __global pipe_control_intel_t *p = __ocl_rpipe2ptr(pipe_);
  int retVal = -1;

  if (is_valid_reserve_id(reserve_id)) {
    const uint base_idx = extract_index(reserve_id);
    __global char const *src =
        p->base + size_of_packet * advance(p, base_idx, index);
  private
    void *vd = (private void *)data;
  private
    void const *vs = (private void const *)((void const *)src);
    __builtin_memcpy(vd, vs, size_of_packet);
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_acquire,
                           memory_scope_all_svm_devices);
    retVal = 0;
  }

  INTEL_PIPE_DPF("EXIT: read_pipe returned %d\n", retVal);
  return retVal;
}

// write_pipe with 4 explicit arguments
int __write_pipe_4(write_only pipe uchar pipe_, reserve_id_t reserve_id,
                   uint index, void *data, uint size_of_packet,
                   uint alignment_of_packet) {
  INTEL_PIPE_DPF("ENTER: write_pipe( reserve_id = %08X, index = %d)\n",
                 reserve_id, index);
  __global pipe_control_intel_t *p = __ocl_wpipe2ptr(pipe_);
  int retVal = -1;

  if (is_valid_reserve_id(reserve_id)) {
    const uint base_idx = extract_index(reserve_id);
    __global char const *dst =
        p->base + size_of_packet * advance(p, base_idx, index);
  private
    void *vd = (private void *)((void *)dst);
  private
    void const *vs = (private void const *)((void const *)data);
    __builtin_memcpy(vd, vs, size_of_packet);
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_release,
                           memory_scope_all_svm_devices);
    retVal = 0;
  }

  INTEL_PIPE_DPF("EXIT: write_pipe returned %d\n", retVal);
  return retVal;
}

/////////////////////////////////////////////////////////////////////
// Basic Reads and Writes
int __read_pipe_2(read_only pipe uchar pipe_, void *data, uint size_of_packet,
                  uint alignment_of_packet) {
  __global pipe_control_intel_t *p = __ocl_rpipe2ptr(pipe_);
  INTEL_PIPE_DPF("ENTER: read_pipe\n");

  int retVal = -1;

  if (intel_lock_pipe_read(p)) {
    uint head = atomic_load_explicit(&p->head, memory_order_acquire,
                                     memory_scope_all_svm_devices);
    const uint tail = atomic_load_explicit(&p->tail, memory_order_relaxed,
                                           memory_scope_all_svm_devices);
    while (true) {
      INTEL_PIPE_DPF("\t reserve_read_pipe: Initially, head = %d\n", head);
      const uint newHead = advance(p, head, 1);
      bool wrap = newHead < head;

      if (!wrap && (head <= tail && tail < newHead)) /* Underflow*/
      {
        INTEL_PIPE_DPF("\t reserve_read_pipe: Underflow!  num_packets = %d, "
                       "head = %d, tail = %d\n",
                       1, head, tail);
        break;
      } else if (wrap && (head <= tail || tail < newHead)) {
        INTEL_PIPE_DPF("\t reserve_read_pipe: Wrap and Underflow!  num_packets "
                       "= %d, head = %d, tail = %d\n",
                       1, head, tail);
        break;
      }

      if (atomic_compare_exchange_strong_explicit(
              &p->head, &head, newHead, memory_order_release,
              memory_order_relaxed, memory_scope_all_svm_devices)) {
      private
        void *vd = (private void *)data;
      private
        void const *vs = (private void const *)((
            void const *)(p->base + head * size_of_packet));
        __builtin_memcpy(vd, vs, size_of_packet);
        atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_acquire,
                               memory_scope_all_svm_devices);
        intel_unlock_pipe_read(p);
        retVal = 0;
        break; /*Success.*/
      } else {
        INTEL_PIPE_DPF("\t read_pipe: Iterate!  old head = %d, new head = %d\n",
                       head, newHead);
      }
    }
  }

  INTEL_PIPE_DPF("EXIT: read_pipe returned %d\n", retVal);
  return retVal;
}

int __write_pipe_2(write_only pipe uchar pipe_, void *data, uint size_of_packet,
                   uint alignment_of_packet) {
  INTEL_PIPE_DPF("ENTER: write_pipe\n");
  __global pipe_control_intel_t *p = __ocl_wpipe2ptr(pipe_);

  int retVal = -1;
  if (intel_lock_pipe_write(p)) {
    uint tail = atomic_load_explicit(&p->tail, memory_order_acquire,
                                     memory_scope_all_svm_devices);
    const uint head = atomic_load_explicit(&p->head, memory_order_relaxed,
                                           memory_scope_all_svm_devices);
    while (true) {
      INTEL_PIPE_DPF("\t reserve_write_pipe: Initially, tail = %d\n", tail);

      const uint newTail = advance(p, tail, 1);
      bool wrap = newTail < tail;

      if (!wrap && (tail < head && head <= newTail)) {
        INTEL_PIPE_DPF("\t reserve_write_pipe: Overflow!  num_packets = %d, "
                       "head = %d, tail = %d\n",
                       1, head, tail);
        break;
      } else if (wrap && (tail < head || head <= newTail)) {
        INTEL_PIPE_DPF(
            "\t reserve_write_pipe: Wrap + Overflow!  num_packets = %d, "
            "pipe_max_packets_plus_one = %d, head = %d, tail = %d\n",
            1, p->pipe_max_packets_plus_one, head, tail);
        break;
      }

      if (atomic_compare_exchange_strong_explicit(
              &p->tail, &tail, newTail, memory_order_release,
              memory_order_relaxed, memory_scope_all_svm_devices)) {
      private
        void *vd = (private void *)((void *)(p->base + tail * size_of_packet));
      private
        void const *vs = (private void const *)((void const *)data);
        __builtin_memcpy(vd, vs, size_of_packet);
        atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_release,
                               memory_scope_all_svm_devices);
        intel_unlock_pipe_write(p);
        retVal = 0;
        break; /*Success.*/
      } else {
        INTEL_PIPE_DPF(
            "\t reserve_write_pipe: Iterate!  old tail = %d, got %d\n", tail,
            newTail);
      }
    }
  }

  INTEL_PIPE_DPF("EXIT: write_pipe returned %d\n", retVal);
  return retVal;
}

bool OVERLOADABLE is_valid_reserve_id(reserve_id_t reserve_id) {
  // Valid bit is 1U << 30. This means all valid id is less than 5U << 28.
  return RTOS(reserve_id) < (5U << 28);
}

/////////////////////////////////////////////////////////////////////
// Pipe Queries
static uint __get_pipe_num_packets(__global pipe_control_intel_t *p) {
  // load from tail shouldn't be moved before load from head so acquire head
  // first then relaxively load tail
  uint head = atomic_load_explicit(&p->head, memory_order_acquire,
                                   memory_scope_all_svm_devices);
  uint tail = atomic_load_explicit(&p->tail, memory_order_relaxed,
                                   memory_scope_all_svm_devices);

  return select(p->pipe_max_packets_plus_one - head + tail, tail - head,
                (uint)(head <= tail));
}
uint __get_pipe_num_packets_ro(read_only pipe uchar pipe_, uint size_of_packet,
                               uint alignment_of_packet) {
  return __get_pipe_num_packets(__ocl_rpipe2ptr(pipe_));
}
uint __get_pipe_num_packets_wo(write_only pipe uchar pipe_, uint size_of_packet,
                               uint alignment_of_packet) {
  return __get_pipe_num_packets(__ocl_wpipe2ptr(pipe_));
}

uint __get_pipe_max_packets_ro(read_only pipe uchar pipe_, uint size_of_packet,
                               uint alignment_of_packet) {
  __global pipe_control_intel_t *p = __ocl_rpipe2ptr(pipe_);
  return p->pipe_max_packets_plus_one - 1;
}
uint __get_pipe_max_packets_wo(write_only pipe uchar pipe_, uint size_of_packet,
                               uint alignment_of_packet) {
  __global pipe_control_intel_t *p = __ocl_wpipe2ptr(pipe_);
  return p->pipe_max_packets_plus_one - 1;
}

/////////////////////////////////////////////////////////////////////
// WG functions are handled by the barrier pass so that
// they are called once per WG.

reserve_id_t __work_group_reserve_read_pipe(read_only pipe uchar p,
                                            uint num_packets,
                                            uint size_of_packet,
                                            uint alignment_of_packet) {
  return __reserve_read_pipe(p, num_packets, size_of_packet,
                             alignment_of_packet);
}

reserve_id_t __work_group_reserve_write_pipe(write_only pipe uchar p,
                                             uint num_packets,
                                             uint size_of_packet,
                                             uint alignment_of_packet) {
  return __reserve_write_pipe(p, num_packets, size_of_packet,
                              alignment_of_packet);
}

void __work_group_commit_read_pipe(read_only pipe uchar p,
                                   reserve_id_t reserve_id, uint size_of_packet,
                                   uint alignment_of_packet) {
  __commit_read_pipe(p, reserve_id, size_of_packet, alignment_of_packet);
}

void __work_group_commit_write_pipe(write_only pipe uchar p,
                                    reserve_id_t reserve_id,
                                    uint size_of_packet,
                                    uint alignment_of_packet) {
  __commit_write_pipe(p, reserve_id, size_of_packet, alignment_of_packet);
}

#endif // __OPENCL_C_VERSION__ >= 200
