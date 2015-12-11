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

// This source file contains an implementation of OpenCL 2.0 built-in pipe functions

#if defined(_DEBUG)
#define INTEL_PIPE_DPF(format, args...) (void)printf
#else
#define INTEL_PIPE_DPF(format, args...)
#endif

#if __OPENCL_C_VERSION__ >= 200

#if !defined (__MIC__) && !defined(__MIC2__)

#define ALWAYS_INLINE __attribute__((always_inline))
#define OVERLOADABLE __attribute__((overloadable))

// CSSD100017148 workaround:
//    Enable build of pipe_functions.c for OpenCL 1.2 by replacing "pipe int" with "__global struct pipe_t *".
//    The workaround relies on BuiltInFuncImport pass which maps different structure types of the same arguments.
//    Look for CSSD100017148 in src/backend/passes/BuiltInFuncImport/BuiltInFuncImport.cpp
typedef __global struct pipe_t* PIPE_T;

// There are no declarations of OpenCL 2.0 builtins in opencl_.h for named address space
// but in the library they has to be called directly because the library
// won't be handled by "Generic Address Resolution" passes. So declare them here.
bool OVERLOADABLE atomic_compare_exchange_strong_explicit(volatile __global atomic_int *object, __private int *expected, int desired,
                                                                           memory_order success, memory_order failure, memory_scope scope);
bool OVERLOADABLE atomic_compare_exchange_strong_explicit(volatile __global atomic_uint *object, __private uint *expected, uint desired,
                                                                           memory_order success, memory_order failure, memory_scope scope);

int OVERLOADABLE atomic_load_explicit(volatile __global atomic_int *object, memory_order order, memory_scope scope);
uint OVERLOADABLE atomic_load_explicit(volatile __global atomic_uint *object, memory_order order, memory_scope scope);

int OVERLOADABLE atomic_fetch_add_explicit(volatile __global atomic_int *object, int operand, memory_order order, memory_scope scope);
uint OVERLOADABLE atomic_fetch_add_explicit(volatile __global atomic_uint *object, uint operand, memory_order order, memory_scope scope);

int OVERLOADABLE atomic_fetch_sub_explicit(volatile __global atomic_int *object, int operand, memory_order order, memory_scope scope);
uint OVERLOADABLE atomic_fetch_sub_explicit(volatile __global atomic_uint *object, uint operand, memory_order order, memory_scope scope);

// Auxiliary macroses to build mangled names of read/write pipe built-ins
// for different address spaces.
#define PRIVATE P
#define LOCAL   PU3AS3
#define GLOBAL  PU3AS1
#define CONCATMACROSES( BEGIN, ADDRSPACE, END )      BEGIN ## ADDRSPACE ## END
#define ASSEMBLEMANGLEDNAME( BEGIN, ADDRSPACE, END ) CONCATMACROSES( BEGIN, ADDRSPACE, END )

// The following functions are threated by clang as builtins.
// To be SPIR conformant we have to use correctly mangled names in OCL code
#define WRITE_PIPE_2(ADDRSPACE)            ASSEMBLEMANGLEDNAME( _Z10write_pipePU3AS110ocl_pipe_t, ADDRSPACE, vi )
#define WRITE_PIPE_4(ADDRSPACE)            ASSEMBLEMANGLEDNAME( _Z10write_pipePU3AS110ocl_pipe_t16ocl_reserve_id_tj, ADDRSPACE, vi )
#define READ_PIPE_2(ADDRSPACE)             ASSEMBLEMANGLEDNAME( _Z9read_pipePU3AS110ocl_pipe_t, ADDRSPACE, vi )
#define READ_PIPE_4(ADDRSPACE)             ASSEMBLEMANGLEDNAME( _Z9read_pipePU3AS110ocl_pipe_t16ocl_reserve_id_tj, ADDRSPACE, vi )
#define RESERVE_READ_PIPE                  _Z17reserve_read_pipePU3AS110ocl_pipe_tji
#define RESERVE_WRITE_PIPE                 _Z18reserve_write_pipePU3AS110ocl_pipe_tji
#define COMMIT_READ_PIPE                   _Z16commit_read_pipePU3AS110ocl_pipe_t16ocl_reserve_id_ti
#define COMMIT_WRITE_PIPE                  _Z17commit_write_pipePU3AS110ocl_pipe_t16ocl_reserve_id_ti
#define WORK_GROUP_RESERVE_READ_PIPE       _Z28work_group_reserve_read_pipePU3AS110ocl_pipe_tji
#define WORK_GROUP_RESERVE_WRITE_PIPE      _Z29work_group_reserve_write_pipePU3AS110ocl_pipe_tji
#define WORK_GROUP_COMMIT_READ_PIPE        _Z27work_group_commit_read_pipePU3AS110ocl_pipe_t16ocl_reserve_id_ti
#define WORK_GROUP_COMMIT_WRITE_PIPE       _Z28work_group_commit_write_pipePU3AS110ocl_pipe_t16ocl_reserve_id_ti
#define GET_PIPE_NUM_PACKETS               _Z20get_pipe_num_packetsPU3AS110ocl_pipe_ti
#define GET_PIPE_MAX_PACKETS               _Z20get_pipe_max_packetsPU3AS110ocl_pipe_ti

// pipe_control_intel_t structure MUST BE ALIGNED with the one defined in src/cl_api/PipeCommon.h
#define INTEL_PIPE_HEADER_RESERVED_SPACE    128
// Total size:  129 ( + 1 because of a variable length array).
//              RT must allocate 128 chars for pipe control at the beginning of
//              contiguous memory. This buffer must be aligned by CACHE_LINE.
#define CACHE_LINE 64
typedef struct _tag_pipe_control_intel_t
{
    // The pipe packet size is always passed as an implicit argument (i32 immediate)

    // Total number of packets in the pipe.  This value must be
    // set by the host when the pipe is created. Pipe cannot accommodate
    // more than pipe_max_packets_plus_one – 1 packets. So RT must allocate memory
    // for one more packet.
    const uint pipe_max_packets_plus_one;

    // The pipe head and tail must be set by the host when
    // the pipe is created.  They will probably be set to zero,
    // though as long as head equals tail, it doesn't matter
    // what they are initially set to.
    volatile atomic_uint head;  // Head Index, for reading: [0, pipe_max_packets_plus_one)
    volatile atomic_uint tail;  // Tail Index, for writing: [0, pipe_max_packets_plus_one)
    char pad0[CACHE_LINE - 2 * sizeof(atomic_uint) - sizeof(uint)];

    // This controls whether the pipe is unlocked, locked for
    // reading, or locked for writing.  If it is zero, the pipe
    // is unlocked.  If it is positive, it is locked for writing.
    // If it is negative, it is locked for reading. This must
    // be set to zero by the host when the pipe is created.
    volatile atomic_int lock;
    char pad1[CACHE_LINE - sizeof(atomic_int)];
    // The end of the control structure as it must be defined in the src/cl_api/PipeCommon.h

    // Packets storage begins right after the pipe control.
    // Compiler will calculate the offset to that storage at places
    // there it will find accesses to the "base" array.
    char base[1];
} pipe_control_intel_t;

#define INTEL_PIPE_RESERVE_ID_VALID_BIT ((size_t)(1UL << (sizeof(size_t) * 8 - 1)))

#define RTOS(r) ((size_t)(__builtin_astype((r), void*)))
#define STOR(s) (__builtin_astype(((void*)(s)), reserve_id_t))

// WORKAROUND: __builtin_astype isn't working with pipes for the moment
//#define PTOC(p)(__builtin_astype((p), (__global pipe_control_intel_t*)))
typedef __global pipe_control_intel_t* gp_pipe_control_intel_t;
#define PTOC(p) (*(gp_pipe_control_intel_t*)(&(p)))

/////////////////////////////////////////////////////////////////////
// Pipe Helper Functions (static)

ALWAYS_INLINE static uint advance( __global pipe_control_intel_t* p, uint base, uint stride )
{
    return select( base + stride,
                   base + stride - p->pipe_max_packets_plus_one,
                   (p->pipe_max_packets_plus_one <= base + stride) );
}

ALWAYS_INLINE static reserve_id_t create_reserve_id( uint idx )
{
    return STOR((size_t)idx | INTEL_PIPE_RESERVE_ID_VALID_BIT);
}


ALWAYS_INLINE static uint extract_index( reserve_id_t rid )
{
    return (uint)(RTOS(rid) & ~INTEL_PIPE_RESERVE_ID_VALID_BIT);
}

ALWAYS_INLINE static bool intel_lock_pipe_read( __global pipe_control_intel_t* p )
{
    int lock = atomic_load_explicit( &p->lock, memory_order_relaxed, memory_scope_all_svm_devices );
    while( lock <= 0 )
    {
        int newLock = lock - 1;
        if(atomic_compare_exchange_strong_explicit( &p->lock, &lock, newLock,
                                                    memory_order_relaxed,
                                                    memory_order_relaxed,
                                                    memory_scope_all_svm_devices ))
        {
          return true;
        }
    }
    return false;
}

ALWAYS_INLINE static void intel_unlock_pipe_read( __global pipe_control_intel_t* p )
{
    atomic_fetch_add_explicit( &p->lock, 1,
                               memory_order_relaxed,
                               memory_scope_all_svm_devices );
    // OK to inc, since we must have locked.
}

ALWAYS_INLINE static bool intel_lock_pipe_write( __global  pipe_control_intel_t* p )
{
    int lock = atomic_load_explicit( &p->lock, memory_order_relaxed, memory_scope_all_svm_devices );
    while( lock >= 0 )
    {
        int newLock = lock + 1;
        if(atomic_compare_exchange_strong_explicit( &p->lock, &lock, newLock,
                                                    memory_order_relaxed,
                                                    memory_order_relaxed,
                                                    memory_scope_all_svm_devices ))
        {
          return true;
        }
    }
    return false;
}

ALWAYS_INLINE static void intel_unlock_pipe_write( __global pipe_control_intel_t* p )
{
    atomic_fetch_sub_explicit( &p->lock, 1,
                               memory_order_relaxed,
                               memory_scope_all_svm_devices );
    // OK to dec, since we must have locked.
}


/////////////////////////////////////////////////////////////////////
// Work Item Reservations
reserve_id_t RESERVE_READ_PIPE( PIPE_T pipe_, uint num_packets, uint size_of_packet )
{
  INTEL_PIPE_DPF( "ENTER: reserve_read_pipe( num_packets = %d)\n", num_packets );
  __global pipe_control_intel_t* p = PTOC(pipe_);
  reserve_id_t retVal = CLK_NULL_RESERVE_ID;

  // The maximum possible reservation number is (_pipe_max_packets_plus_one - 1) packets.
  if( num_packets >= p->pipe_max_packets_plus_one ||
      0 == num_packets )
  {
    INTEL_PIPE_DPF( "\t reserve_read_pipe: Sanity check failed!  num_packets = %d, pipe_max_packets_plus_one = %d\n",
                    num_packets, p->pipe_max_packets_plus_one );
  }
  else if( intel_lock_pipe_read( p ) )
  {
    uint head = atomic_load_explicit( &p->head,
                                      memory_order_acquire,
                                      memory_scope_all_svm_devices );
    const uint tail = atomic_load_explicit( &p->tail,
                                            memory_order_relaxed,
                                            memory_scope_all_svm_devices );

    while( true )
    {
      const uint newHead = advance(p, head, num_packets);
      bool wrap = newHead < head;
      INTEL_PIPE_DPF( "\t reserve_read_pipe: Initially, head = %d, new head = %d\n", head, newHead );

      if( !wrap && ( head <= tail && tail < newHead ) )    // Underflow
      {
        INTEL_PIPE_DPF( "\t reserve_read_pipe: Underflow!  num_packets = %d, head = %d, tail = %d\n",
                        num_packets, head, tail );
        break;
      }
      else if ( wrap && ( head <= tail || tail < newHead ) )
      {
        INTEL_PIPE_DPF( "\t reserve_read_pipe: Wrap and Underflow!  num_packets = %d, head = %d, tail = %d\n",
                        num_packets, head, tail );
        break;
      }

      if(atomic_compare_exchange_strong_explicit( &p->head, &head, newHead,
                                                  memory_order_release,
                                                  memory_order_relaxed,
                                                  memory_scope_all_svm_devices ))
      {
        retVal = create_reserve_id(head);
        // the lock must be unlocked with following commit
        break;  // Success.
      }
      else
      {
        INTEL_PIPE_DPF( "\t read_pipe: Iterate!  old head = %d, new head = %d\n", head, newHead );
      }
    }

    if( RTOS(retVal) == 0 )
    {
      intel_unlock_pipe_read( p );
    }
    // Else: note, no unlock!  The pipe will be unlocked as part of committing
    // the reservation.
  }

  INTEL_PIPE_DPF( "EXIT: reserve_read_pipe returned %08X\n", retVal );
  return retVal;
}

reserve_id_t RESERVE_WRITE_PIPE( PIPE_T pipe_, uint num_packets, uint size_of_packet )
{
  INTEL_PIPE_DPF( "ENTER: reserve_write_pipe( num_packets = %d)\n", num_packets );
  __global pipe_control_intel_t* p = PTOC(pipe_);
  reserve_id_t    retVal = CLK_NULL_RESERVE_ID;

  if( num_packets >= p->pipe_max_packets_plus_one ||
      0 == num_packets )
  {
    INTEL_PIPE_DPF( "\t reserve_write_pipe: Sanity check failed!  num_packets = %d, pipe_max_packets_plus_one = %d\n",
                    num_packets, p->pipe_max_packets_plus_one );
  }
  else if( intel_lock_pipe_write( p ) )
  {
    uint tail = atomic_load_explicit( &p->tail,
                                      memory_order_acquire,
                                      memory_scope_all_svm_devices );
    const uint head = atomic_load_explicit( &p->head,
                                            memory_order_relaxed,
                                            memory_scope_all_svm_devices );

    while( true )
    {
      const uint newTail = advance(p, tail, num_packets);
      INTEL_PIPE_DPF( "\t reserve_write_pipe: Initially, tail = %d, new tail = %d\n", tail, newTail );
      bool wrap = newTail < tail;

      if(!wrap && ( tail < head && head <= newTail ) ) {
        INTEL_PIPE_DPF( "\t reserve_write_pipe: Overflow!  num_packets = %d, head = %d, tail = %d\n",
                        num_packets, head, tail );
        break;
      }
      else if(wrap && ( tail < head || head <= newTail ) )
      {
        INTEL_PIPE_DPF( "\t reserve_write_pipe: Wrap + Overflow!  num_packets = %d, pipe_max_packets_plus_one = %d, head = %d, tail = %d\n",
                        num_packets, p->pipe_max_packets_plus_one, head, tail );
        break;
      }

      if(atomic_compare_exchange_strong_explicit( &p->tail, &tail, newTail,
                                                  memory_order_release,
                                                  memory_order_relaxed,
                                                  memory_scope_all_svm_devices ))
      {
        retVal = create_reserve_id(tail);
        break;  // Success.
        // the lock must be unlocked by the following commit
      }
      else
      {
        INTEL_PIPE_DPF( "\t reserve_write_pipe: Iterate!  old tail = %d, new tail = %d\n", tail, newTail );
      }
    }

    if( RTOS(retVal) == 0 )
    {
      intel_unlock_pipe_write( p );
    }
    // Otherwise, note: No unlock!  The pipe will be unlocked as part of committing
    // the reservation.
  }

  INTEL_PIPE_DPF( "EXIT: reserve_write_pipe returned %08X\n", retVal );
  return retVal;
}

void COMMIT_READ_PIPE( PIPE_T pipe_, reserve_id_t reserve_id, uint size_of_packet )
{
  INTEL_PIPE_DPF( "ENTER: commit_read_pipe( reserve_id = %08X)\n", reserve_id );
  __global pipe_control_intel_t* p = PTOC(pipe_);

  intel_unlock_pipe_read( p );
  INTEL_PIPE_DPF( "EXIT: commit_read_pipe\n" );
}

void COMMIT_WRITE_PIPE( PIPE_T pipe_, reserve_id_t reserve_id, uint size_of_packet )
{
  INTEL_PIPE_DPF( "ENTER: commit_write_pipe( reserve_id = %08X)\n", reserve_id );
  __global pipe_control_intel_t* p = PTOC(pipe_);

  intel_unlock_pipe_write( p );
  INTEL_PIPE_DPF( "EXIT: commit_write_pipe\n" );
}


/////////////////////////////////////////////////////////////////////
// Reads and Writes with Reservations
// The reservation functions lock the pipe, so we don't need to
// re-lock here.

int READ_PIPE_4(GLOBAL)( PIPE_T pipe_, reserve_id_t reserve_id, uint index, __global const void* data, uint size_of_packet)
{
  INTEL_PIPE_DPF( "ENTER: read_pipe( reserve_id = %08X, index = %d)\n", reserve_id, index );
  __global pipe_control_intel_t* p = PTOC(pipe_);
  int retVal = -1;

  if( is_valid_reserve_id( reserve_id ) )
  {
    const uint base_idx = extract_index(reserve_id);
    __global char const * src = p->base + size_of_packet * advance(p, base_idx, index);
    private void *vd = (private void *)((void*)data);
    private void const *vs = (private void const *)((void const *)src);
    __builtin_memcpy(vd, vs, size_of_packet);
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_acquire, memory_scope_all_svm_devices);
    retVal = 0;
  }

  INTEL_PIPE_DPF( "EXIT: read_pipe returned %d\n", retVal );
  return retVal;
}

// write_pipe with 4 explicit arguments
int WRITE_PIPE_4(GLOBAL)( PIPE_T pipe_, reserve_id_t reserve_id, uint index, __global const void* data, uint size_of_packet)
{
  INTEL_PIPE_DPF( "ENTER: write_pipe( reserve_id = %08X, index = %d)\n", reserve_id, index );
  __global pipe_control_intel_t* p = PTOC(pipe_);
  int retVal = -1;

  if( is_valid_reserve_id( reserve_id ) )
  {
    const uint base_idx = extract_index(reserve_id);
    __global char const * dst = p->base + size_of_packet * advance(p, base_idx, index);
    private void *vd = (private void *)((void*)dst);
    private void const *vs = (private void const *)((void const *)data);
    __builtin_memcpy(vd, vs, size_of_packet);
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_release, memory_scope_all_svm_devices);
    retVal = 0;
  }

  INTEL_PIPE_DPF( "EXIT: write_pipe returned %d\n", retVal );
  return retVal;
}

/////////////////////////////////////////////////////////////////////
// Basic Reads and Writes

int READ_PIPE_2(GLOBAL)( PIPE_T pipe_, __global const void* data, uint size_of_packet)
{
  __global pipe_control_intel_t* p = PTOC(pipe_);
  INTEL_PIPE_DPF( "ENTER: read_pipe\n" );

  int retVal = -1;

  if( intel_lock_pipe_read( p ) )
  {
    uint head = atomic_load_explicit( &p->head,
                                      memory_order_acquire,
                                      memory_scope_all_svm_devices );
    const uint tail = atomic_load_explicit( &p->tail,
                                            memory_order_relaxed,
                                            memory_scope_all_svm_devices );
    while( true )
    {
      INTEL_PIPE_DPF( "\t reserve_read_pipe: Initially, head = %d\n", head );
      const uint newHead = advance(p, head, 1);
      bool wrap = newHead < head;

      if( !wrap && ( head <= tail && tail < newHead ) )    // Underflow
      {
        INTEL_PIPE_DPF( "\t reserve_read_pipe: Underflow!  num_packets = %d, head = %d, tail = %d\n",
                        1, head, tail );
        break;
      }
      else if ( wrap && ( head <= tail || tail < newHead ) )
      {
        INTEL_PIPE_DPF( "\t reserve_read_pipe: Wrap and Underflow!  num_packets = %d, head = %d, tail = %d\n",
                        1, head, tail );
        break;
      }

      if(atomic_compare_exchange_strong_explicit( &p->head, &head, newHead,
                                                  memory_order_release,
                                                  memory_order_relaxed,
                                                  memory_scope_all_svm_devices ))
      {
        private void *vd = (private void *)((void*)data);
        private void const *vs = (private void const *)((void const *)(p->base + head * size_of_packet));
        __builtin_memcpy(vd, vs, size_of_packet);
        atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_acquire, memory_scope_all_svm_devices);
        intel_unlock_pipe_read( p );
        retVal = 0;
        break;  // Success.
      }
      else
      {
        INTEL_PIPE_DPF( "\t read_pipe: Iterate!  old head = %d, new head = %d\n", head, newHead );
      }
    }

  }

  INTEL_PIPE_DPF( "EXIT: read_pipe returned %d\n", retVal );
  return retVal;
}

int WRITE_PIPE_2(GLOBAL)( PIPE_T pipe_, __global const void* data, uint size_of_packet )
{
  INTEL_PIPE_DPF( "ENTER: write_pipe\n" );
  __global pipe_control_intel_t* p = PTOC(pipe_);

  int retVal = -1;
  if( intel_lock_pipe_write( p ) )
  {
    uint tail = atomic_load_explicit( &p->tail,
                                      memory_order_acquire,
                                      memory_scope_all_svm_devices );
    const uint head = atomic_load_explicit( &p->head,
                                            memory_order_relaxed,
                                            memory_scope_all_svm_devices );
    while( true )
    {
      INTEL_PIPE_DPF( "\t reserve_write_pipe: Initially, tail = %d\n", tail );

      const uint newTail = advance(p, tail, 1);
      bool wrap = newTail < tail;

      if(!wrap && ( tail < head && head <= newTail ) ) {
        INTEL_PIPE_DPF( "\t reserve_write_pipe: Overflow!  num_packets = %d, head = %d, tail = %d\n",
                        1, head, tail );
        break;
      }
      else if(wrap && ( tail < head || head <= newTail ) )
      {
        INTEL_PIPE_DPF( "\t reserve_write_pipe: Wrap + Overflow!  num_packets = %d, pipe_max_packets_plus_one = %d, head = %d, tail = %d\n",
                        1, p->pipe_max_packets_plus_one, head, tail );
        break;
      }

      if(atomic_compare_exchange_strong_explicit( &p->tail, &tail, newTail,
                                                  memory_order_release,
                                                  memory_order_relaxed,
                                                  memory_scope_all_svm_devices ))
      {
        private void *vd = (private void *)((void*)(p->base + tail * size_of_packet));
        private void const *vs = (private void const *)((void const *)data);
        __builtin_memcpy(vd, vs, size_of_packet);
        atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_release, memory_scope_all_svm_devices);
        intel_unlock_pipe_write( p );
        retVal = 0;
        break;  // Success.
      }
      else
      {
        INTEL_PIPE_DPF( "\t reserve_write_pipe: Iterate!  old tail = %d, got %d\n", tail, newTail );
      }
    }
  }

  INTEL_PIPE_DPF( "EXIT: write_pipe returned %d\n", retVal );
  return retVal;
}

// NOTE: Let the clang to mangle is_valid_reserve_id.
bool OVERLOADABLE is_valid_reserve_id( reserve_id_t reserve_id )
{
  return ( RTOS(reserve_id) & INTEL_PIPE_RESERVE_ID_VALID_BIT ) != 0;
}

/////////////////////////////////////////////////////////////////////
// Pipe Queries
uint GET_PIPE_NUM_PACKETS( PIPE_T pipe_, uint size_of_packet )
{
  (void)size_of_packet; // Avoid warning about unused variable.
  __global pipe_control_intel_t* p = PTOC(pipe_);

  // load from tail shouldn't be moved before load from head so acquire head first then relaxively load tail
  uint head = atomic_load_explicit( &p->head, memory_order_acquire, memory_scope_all_svm_devices );
  uint tail = atomic_load_explicit( &p->tail, memory_order_relaxed, memory_scope_all_svm_devices );

  return select( p->pipe_max_packets_plus_one - head + tail,
                 tail - head,
                 (uint)(head <= tail) );
}

uint GET_PIPE_MAX_PACKETS( PIPE_T pipe_, uint size_of_packet )
{
  (void)size_of_packet; // Avoid warning about unused variable.
  __global pipe_control_intel_t* p = PTOC(pipe_);
  return p->pipe_max_packets_plus_one - 1;
}

/////////////////////////////////////////////////////////////////////
// WG functions are handled by the barrier pass so that
// they are called once per WG.

reserve_id_t WORK_GROUP_RESERVE_READ_PIPE( PIPE_T p, uint num_packets, uint size_of_packet )
{
  return RESERVE_READ_PIPE( p, num_packets, size_of_packet );
}

reserve_id_t WORK_GROUP_RESERVE_WRITE_PIPE( PIPE_T p, uint num_packets, uint size_of_packet )
{
  return RESERVE_WRITE_PIPE( p, num_packets, size_of_packet );
}

void WORK_GROUP_COMMIT_READ_PIPE( PIPE_T p, reserve_id_t reserve_id, uint size_of_packet )
{
  COMMIT_READ_PIPE( p, reserve_id, size_of_packet );
}

void WORK_GROUP_COMMIT_WRITE_PIPE( PIPE_T p, reserve_id_t reserve_id, uint size_of_packet )
{
  COMMIT_WRITE_PIPE( p, reserve_id, size_of_packet );
}

/////////////////////////////////////////////////////////////////////
// Any of CPU specific proxy functions always call its __global version.

// read_pipe(pipe gentype p, gentype *data);
// private
int READ_PIPE_2(PRIVATE)( PIPE_T p, __private const void* data, uint size_of_packet)
{
  return READ_PIPE_2(GLOBAL)(p, (__global const void*)(const void*)data, size_of_packet);
}
// local
int READ_PIPE_2(LOCAL)( PIPE_T p, __local const void* data, uint size_of_packet)
{
  return READ_PIPE_2(GLOBAL)(p, (__global const void*)(const void*)data, size_of_packet);
}

// read_pipe(pipe gentype p, reserve_id_t reserve_id, uint index, gentype *data);
// private
int READ_PIPE_4(PRIVATE)( PIPE_T p, reserve_id_t reserve_id, uint index, __private void* data, uint size_of_packet)
{
  return READ_PIPE_4(GLOBAL)(p, reserve_id, index, (__global void*)(void*)data, size_of_packet);
}
// local
int READ_PIPE_4(LOCAL)( PIPE_T p, reserve_id_t reserve_id, uint index, __local void* data, uint size_of_packet)
{
  return READ_PIPE_4(GLOBAL)(p, reserve_id, index, (__global void*)(void*)data, size_of_packet);
}

// write_pipe(pipe gentype p, gentype *data);
// private
int WRITE_PIPE_2(PRIVATE)( PIPE_T p, __private const void* data, uint size_of_packet)
{
  return WRITE_PIPE_2(GLOBAL)(p, (__global const void*)(const void*)data, size_of_packet);
}
// local
int WRITE_PIPE_2(LOCAL)( PIPE_T p, __local const void* data, uint size_of_packet)
{
  return WRITE_PIPE_2(GLOBAL)(p, (__global const void*)(const void*)data, size_of_packet);
}

// write_pipe(pipe gentype p, reserve_id_t reserve_id, uint index, gentype *data);
// private
int WRITE_PIPE_4(PRIVATE)( PIPE_T p, reserve_id_t reserve_id, uint index, __private const void* data, uint size_of_packet)
{
  return WRITE_PIPE_4(GLOBAL)(p, reserve_id, index, (__global const void*)(const void*)data, size_of_packet);
}
// local
int WRITE_PIPE_4(LOCAL)( PIPE_T p, reserve_id_t reserve_id, uint index, __local const void* data, uint size_of_packet)
{
  return WRITE_PIPE_4(GLOBAL)(p, reserve_id, index, (__global const void*)(const void*)data, size_of_packet);
}
#endif // defined (__MIC__) || defined(__MIC2__)

#endif // __OPENCL_C_VERSION__ >= 200
