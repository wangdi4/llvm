/*
    Copyright 2005-2013 Intel Corporation.  All Rights Reserved.

    The source code contained or described herein and all documents related
    to the source code ("Material") are owned by Intel Corporation or its
    suppliers or licensors.  Title to the Material remains with Intel
    Corporation or its suppliers and licensors.  The Material is protected
    by worldwide copyright laws and treaty provisions.  No part of the
    Material may be used, copied, reproduced, modified, published, uploaded,
    posted, transmitted, distributed, or disclosed in any way without
    Intel's prior express written permission.

    No license under any patent, copyright, trade secret or other
    intellectual property right is granted to or conferred upon you by
    disclosure or delivery of the Materials, either expressly, by
    implication, inducement, estoppel or otherwise.  Any license under such
    intellectual property rights must be express and approved by Intel in
    writing.
*/

#if !defined(__TBB_machine_H) || defined(__TBB_msvc_armv7_H)
#error Do not #include this internal file directly; use public TBB headers instead.
#endif

#define __TBB_msvc_armv7_H

#include <intrin.h>
#include <float.h>

#define __TBB_WORDSIZE 4

#define __TBB_BIG_ENDIAN -1 // not currently supported

#define __TBB_compiler_fence() __dmb(_ARM_BARRIER_SY)
#define __TBB_control_consistency_helper() __TBB_compiler_fence()

#define __TBB_armv7_inner_shareable_barrier() __dmb(_ARM_BARRIER_ISH)
#define __TBB_acquire_consistency_helper() __TBB_armv7_inner_shareable_barrier()
#define __TBB_release_consistency_helper() __TBB_armv7_inner_shareable_barrier()
#define __TBB_full_memory_fence() __TBB_armv7_inner_shareable_barrier()

//--------------------------------------------------
// Compare and swap
//--------------------------------------------------

/**
 * Atomic CAS for 32 bit values, if *ptr==comparand, then *ptr=value, returns *ptr
 * @param ptr pointer to value in memory to be swapped with value if *ptr==comparand
 * @param value value to assign *ptr to if *ptr==comparand
 * @param comparand value to compare with *ptr
 * @return value originally in memory at ptr, regardless of success
*/

#define __TBB_MACHINE_DEFINE_ATOMICS_CMPSWP(S,T,F)                                               \
inline T __TBB_machine_cmpswp##S( volatile void *ptr, T value, T comparand ) {                   \
	return _InterlockedCompareExchange##F(reinterpret_cast<volatile T *>(ptr),comparand,value);  \
}                                                                                                \
                                                                                                 
#define __TBB_MACHINE_DEFINE_ATOMICS_FETCHADD(S,T,F)                                             \
inline T __TBB_machine_fetchadd##S( volatile void *ptr, T value ) {                              \
	return _InterlockedAdd##F(reinterpret_cast<volatile T *>(ptr),value);                        \
}                                                                                                \

__TBB_MACHINE_DEFINE_ATOMICS_CMPSWP(1,char,8)
__TBB_MACHINE_DEFINE_ATOMICS_CMPSWP(2,short,16)
__TBB_MACHINE_DEFINE_ATOMICS_CMPSWP(4,long,)
__TBB_MACHINE_DEFINE_ATOMICS_CMPSWP(8,__int64,64)
__TBB_MACHINE_DEFINE_ATOMICS_FETCHADD(4,long,)
__TBB_MACHINE_DEFINE_ATOMICS_FETCHADD(8,__int64,64)


inline void __TBB_machine_pause (int32_t delay )
{
    while(delay>0)
    {
	__TBB_compiler_fence();
        delay--;
    }
}

namespace tbb {
namespace internal {
    template <typename T, size_t S>
    struct machine_load_store_relaxed {
        static inline T load ( const volatile T& location ) {
            const T value = location;

            /*
            * An extra memory barrier is required for errata #761319
            * Please see http://infocenter.arm.com/help/topic/com.arm.doc.uan0004a
            */
            __TBB_armv7_inner_shareable_barrier();
            return value;
        }

        static inline void store ( volatile T& location, T value ) {
            location = value;
        }
    };
}} // namespaces internal, tbb

// Machine specific atomic operations

#define __TBB_CompareAndSwap4(P,V,C) __TBB_machine_cmpswp4(P,V,C)
#define __TBB_CompareAndSwap8(P,V,C) __TBB_machine_cmpswp8(P,V,C)
#define __TBB_Pause(V) __TBB_machine_pause(V)

// Use generics for some things
#define __TBB_USE_GENERIC_PART_WORD_FETCH_ADD                   1
#define __TBB_USE_GENERIC_PART_WORD_FETCH_STORE                 1
#define __TBB_USE_GENERIC_FETCH_STORE                           1
#define __TBB_USE_GENERIC_HALF_FENCED_LOAD_STORE                1
#define __TBB_USE_GENERIC_DWORD_LOAD_STORE                      1
#define __TBB_USE_GENERIC_SEQUENTIAL_CONSISTENCY_LOAD_STORE     1

#define __TBB_Yield() __yield()

// API to retrieve/update FPU control setting not implemented
#define __TBB_CPU_CTL_ENV_PRESENT 1

typedef unsigned int __TBB_cpu_ctl_env_t;

inline void __TBB_get_cpu_ctl_env ( __TBB_cpu_ctl_env_t* ctl ) {
    *ctl = _control87(0, 0);
}
inline void __TBB_set_cpu_ctl_env ( const __TBB_cpu_ctl_env_t* ctl ) {
    _control87( *ctl, ~0U );
}

// Machine specific atomic operations
#define __TBB_AtomicOR(P,V)     __TBB_machine_OR(P,V)
#define __TBB_AtomicAND(P,V)    __TBB_machine_AND(P,V)

template <typename T1,typename T2>
inline void __TBB_machine_OR( T1 *operand, T2 addend ) {
    _InterlockedOr((long volatile *)operand, (long)addend);
}

template <typename T1,typename T2>
inline void __TBB_machine_AND( T1 *operand, T2 addend ) {
    _InterlockedAnd((long volatile *)operand, (long)addend);
}

