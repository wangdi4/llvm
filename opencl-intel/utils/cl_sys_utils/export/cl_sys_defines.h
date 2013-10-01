/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel's prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel's suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////
#pragma once

// -----------------------------------------------------------
// 			Windows
// -----------------------------------------------------------
#if defined (_WIN32)
#include <basetsd.h>
#include <intrin.h>

#if defined(_M_AMD64)
#define CAS(ptr,old_val,new_val)	_InterlockedCompareExchange64((__int64 volatile*)ptr,(__int64)new_val,(__int64)old_val)
#define TAS(ptr,new_val)			_InterlockedExchange64((__int64 volatile*)ptr,(__int64)new_val)
#else
#define CAS(ptr,old_val,new_val)	_InterlockedCompareExchange((long volatile*)ptr,(LONG_PTR)new_val,(LONG_PTR)old_val)
#define TAS(ptr,new_val)			_InterlockedExchange((long volatile*)ptr,(LONG_PTR)new_val)
#endif

#define INVALID_MUTEX_OWNER			(0)

#define CL_MAX_INT32 MAXINT32
#define CL_MAX_UINT32	MAXUINT32

#define API_FUNCTION    __stdcall
#define ASM_FUNCTION    __stdcall
#define STDCALL         __stdcall

#ifndef CDECL
#define CDECL           __cdecl
#endif

#define PACKED
#define PACK_ON  pack(1)
#define PACK_OFF pack()
#define UNUSED(var) var

#ifdef MAX
#undef MAX
#endif // #ifdef MAX

#ifdef MIN
#undef MIN
#endif // #ifdef MIN

#define MAX(a, b) max(a, b)
#define MIN(a, b) min(a, b)

#define STRDUP(X) (_strdup(X))
#define CPUID(cpu_info, type) __cpuid((int*)(cpu_info), type)

#define VA_COPY(dst, src) ((dst) = (src))
#define VA_END(va)

#define GMTIME(tmNow, tNow) (gmtime_s(&(tmNow), &(tNow)))
#define GET_CURRENT_PROCESS_ID() GetCurrentProcessId()
#define GET_CURRENT_THREAD_ID() GetCurrentThreadId()

#define MEMCPY_S                          memcpy_s
#define STRCPY_S                          strcpy_s
#define STRNCPY_S                         strncpy_s
#define STRCAT_S                          strcat_s
#define STRTOK_S                          strtok_s
#define SPRINTF_S                         sprintf_s
#define VSPRINTF_S                        vsprintf_s

typedef unsigned long long               affinityMask_t;

typedef void*							EVENT_STRUCTURE;
typedef void*							BINARY_SEMAPHORE;
typedef void*							READ_WRITE_LOCK;
typedef void*							CONDITION_VAR;

// aligned malloc
#include <malloc.h>
#define ALIGNED_MALLOC( size, alignment ) _aligned_malloc( (size), (alignment) < sizeof(void*) ? sizeof(void*) : (alignment))
#define ALIGNED_FREE                      _aligned_free

// Windows require more sequre function _malloca. When in certain case may allocate on heap and not on stack.
// For that reason _freea should be called
#define STACK_ALLOC( size ) 				_malloca(size)
#define STACK_FREE( ptr )					_freea(ptr)

// -----------------------------------------------------------
// 		Not Windows (Linux / Android )	
// -----------------------------------------------------------
#else //LINUX

#define CL_MAX_INT32 INT_MAX
#define CL_MAX_UINT32	UINT_MAX
#define API_FUNCTION
#define ASM_FUNCTION
#ifndef CDECL
// A bug in 4.0 < GCC < 4.6 treats cdecl attribute ignore (on 64 bit) as error.
#if __x86_64__ && __GNUC__ == 4 &&  __GNUC_MINOR__ < 6
	#define CDECL
#else
	#define CDECL   __attribute__((cdecl))
#endif
#endif
#define STDCALL

#define PACKED  __attribute__((packed))
#define PACK_ON
#define PACK_OFF
#define UNUSED(var) var  __attribute__((unused))

#ifdef UNICODE
#define TEXT(x) L##x
#else
#define TEXT(x) x
#endif

#ifndef errno_t
typedef int errno_t;
#endif

#ifndef MAX_PATH
#include <limits.h>
#define MAX_PATH PATH_MAX
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifdef MAX
#undef MAX
#endif // #ifdef MAX

#ifdef MIN
#undef MIN
#endif // #ifdef MIN

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) > (b)) ? (b) : (a))

#define STRDUP(X) (strdup(X))

#if defined (__INTEL_COMPILER)
	#define CPUID(cpu_info, type) __cpuid(cpu_info, type)
#else
	#define CPUID(cpu_info, type) cpuid(cpu_info, type)
#endif

#define VA_COPY(dst, src) (va_copy((dst), (src)))
#define VA_END(va) (va_end(va))

#define GMTIME(tmNow, tNow) (tmNow) = (*(gmtime(&(tNow))))
// aligned malloc


#include <stdlib.h>
#include <sys/mman.h>
#define ALIGNED_FREE                      free

#include <pthread.h>
// OS native event structure
typedef struct event_Structure
{
	bool	bAutoReset;
	pthread_mutex_t mutex;
	pthread_cond_t condition;
	volatile bool isFired;
} EVENT_STRUCTURE;

typedef pthread_cond_t				  CONDITION_VAR;

#include <semaphore.h>
// Type declaration for binary semaphore
typedef sem_t							      BINARY_SEMAPHORE;
typedef pthread_rwlock_t				READ_WRITE_LOCK;

#define CAS(ptr,old_val,new_val)	__sync_val_compare_and_swap(ptr,old_val,new_val)
#define TAS(ptr,new_val)			__sync_lock_test_and_set(ptr,new_val)
#define INVALID_MUTEX_OWNER (-1)

#include <unistd.h>
#include <sys/syscall.h>

	// -----------------------------
	// Android Sched Utils
	// -----------------------------
#if defined(__ANDROID__)
inline void* ALIGNED_MALLOC( size_t size, size_t alignment )
{
    return memalign( alignment < sizeof(void*) ? sizeof(void*) : alignment, size);
}
typedef unsigned long long        affinityMask_t;
#define GET_CURRENT_PROCESS_ID()        getpid()
#define GET_CURRENT_THREAD_ID()  ((int)gettid())
#define CPU_ZERO(mask)          (*(mask)  =  0)
#define CPU_SET(cpu, mask)      (*(mask) |=  (1 << (cpu)))
#define CPU_CLR(cpu, mask)      (*(mask) &= ~(1 << (cpu)))
#define CPU_ISSET(cpu, mask)    (*(mask) |   (1 << (cpu)))
#define CPU_EQUAL(maskA, maskB) (*(maskA) == *(maskB))

static int sched_setaffinity(pid_t pid, size_t len, affinityMask_t const *cpusetp)
{
	return syscall(__NR_sched_setaffinity, pid, len, cpusetp);
}
static int sched_getaffinity(pid_t pid, size_t len, affinityMask_t const *cpusetp)
{
	return syscall(__NR_sched_getaffinity, pid, len, cpusetp);
}
static int CPU_COUNT(affinityMask_t* set)
{
    // Pretend the data structure is opaque by using other CPU_ macros to implement
    if (NULL == set) return 0;
    int cpu = 0;
    int result = 0;
    affinityMask_t  zero;
    affinityMask_t* zeroSet = &zero;
    CPU_ZERO(zeroSet);
    while (!CPU_EQUAL(set, zeroSet))
    {
        if (CPU_ISSET(cpu, set))
        {
            ++result;
        }
        CPU_CLR(cpu, set);
        ++cpu;
    }
    return result;
}


#define pthread_cancel(...)		assert(0 && "pthread_cancel isn't supported for android")

	// -----------------------------
	// Linux (Not Android) Sched Utils
	// -----------------------------
#else
inline void* ALIGNED_MALLOC( size_t size, size_t alignment )
{
    void* t = NULL;
    if (0 != posix_memalign(&t, alignment < sizeof(void*) ? sizeof(void*) : alignment, size))
    {
        t = NULL;
    }
    return t;
}
#define GET_CURRENT_PROCESS_ID() getpid()
#define GET_CURRENT_THREAD_ID() ((int)syscall(SYS_gettid))
#include <sched.h>
typedef cpu_set_t                      affinityMask_t;
#endif

#include "cl_secure_string_linux.h"

#define MEMCPY_S                        Intel::OpenCL::Utils::safeMemCpy
#define STRCPY_S                        Intel::OpenCL::Utils::safeStrCpy
#define STRNCPY_S                       Intel::OpenCL::Utils::safeStrNCpy
#define STRCAT_S                        Intel::OpenCL::Utils::safeStrCat
#define STRTOK_S                        Intel::OpenCL::Utils::safe_strtok
#define SPRINTF_S                       snprintf
#define VSPRINTF_S                      Intel::OpenCL::Utils::safeVStrPrintf

#define STACK_ALLOC( size ) 				alloca(size)
#define STACK_FREE( ptr )					
#endif

// Define compiler static assert
#define STATIC_ASSERT(e) typedef char __STATIC_ASSERT__[(e)?1:-1]

#define PAGE_4K_SIZE                    4096
#define CPU_CACHE_LINE_SIZE				64

// assumes alignment is a power of 2
#define IS_ALIGNED_ON( what, alignment ) (0 == (((size_t)(what)              &  ((size_t)(alignment) - 1))))
#define ALIGN_DOWN( what, alignment )    ((size_t)(what)                     & ~((size_t)(alignment) - 1))
#define ALIGN_UP( what, alignment )      ((size_t)((what) + (alignment) - 1) & ~((size_t)(alignment) - 1))
