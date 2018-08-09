// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

// -----------------------------------------------------------
//             Windows
// -----------------------------------------------------------
#if defined (_WIN32)
#include <basetsd.h>
#include <intrin.h>
#include <io.h>

#include <Windows.h>
#include <string.h>

// Those macros are interfer with other definitions
#undef min
#undef max

#if defined(_M_AMD64)
#define CAS(ptr,old_val,new_val)    _InterlockedCompareExchange64((__int64 volatile*)ptr,(__int64)new_val,(__int64)old_val)
#define TAS(ptr,new_val)            _InterlockedExchange64((__int64 volatile*)ptr,(__int64)new_val)
#else
#define CAS(ptr,old_val,new_val)    _InterlockedCompareExchange((long volatile*)ptr,(LONG_PTR)new_val,(LONG_PTR)old_val)
#define TAS(ptr,new_val)            _InterlockedExchange((long volatile*)ptr,(LONG_PTR)new_val)
#endif

#define INVALID_MUTEX_OWNER            (0)

#define CL_MAX_INT32 MAXINT32
#define CL_MAX_UINT32    MAXUINT32

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

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) > (b)) ? (b) : (a))

#define STRDUP(X) (_strdup(X))
#define CPUID(cpu_info, type) __cpuid((int*)(cpu_info), type)

#define VA_COPY(dst, src) ((dst) = (src))
#define VA_END(va)

#define DUP(oldfd) _dup(oldfd)
#define DUP2(oldfd, newfd) _dup2(oldfd, newfd)

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
#define STRCMPI_S                         _strnicmp
#define SSCANF_S                          sscanf_s

typedef unsigned long long               affinityMask_t;

typedef void*                            EVENT_STRUCTURE;
typedef CRITICAL_SECTION                 MUTEX;
typedef void*                            BINARY_SEMAPHORE;
typedef void*                            READ_WRITE_LOCK;
typedef void*                            CONDITION_VAR;

// aligned malloc
#include <malloc.h>
#define ALIGNED_MALLOC( size, alignment ) _aligned_malloc( (size), (alignment) < sizeof(void*) ? sizeof(void*) : (alignment))
#define ALIGNED_FREE                      _aligned_free

// Windows require more sequre function _malloca. When in certain case may allocate on heap and not on stack.
// For that reason _freea should be called
#define STACK_ALLOC( size )                 _malloca(size)
#define STACK_FREE( ptr )                   _freea(ptr)

#define IsPowerOf2(x)                       (__popcnt((x)) == 1)

#define THREAD_LOCAL            __declspec(thread)

#define OS_DLL_POST(fileName) ((fileName) + ".dll")

// -----------------------------------------------------------
//         Linux
// -----------------------------------------------------------
#else //LINUX

#define CL_MAX_INT32 INT_MAX
#define CL_MAX_UINT32    UINT_MAX
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
#define FALSE    0
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

#define DUP(oldfd) dup(oldfd)
#define DUP2(oldfd, newfd) dup2(oldfd, newfd)

#define GMTIME(tmNow, tNow) (tmNow) = (*(gmtime(&(tNow))))
// aligned malloc


#include <stdlib.h>
#include <sys/mman.h>
#define ALIGNED_FREE                      free

#include <pthread.h>
// OS native event structure
typedef struct event_Structure
{
    bool    bAutoReset;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    volatile bool isFired;
} EVENT_STRUCTURE;

typedef pthread_mutex_t                 MUTEX;
typedef pthread_cond_t                  CONDITION_VAR;

#include <semaphore.h>
// Type declaration for binary semaphore
typedef sem_t                       BINARY_SEMAPHORE;
typedef pthread_rwlock_t            READ_WRITE_LOCK;
// Bug in ICC13.0, that fails to convert pointers
#define CAS(ptr,old_val,new_val)    ((long)__sync_val_compare_and_swap((volatile long*)(ptr),(long)(old_val),(long)new_val))
#define TAS(ptr,new_val)            ((long)__sync_lock_test_and_set((volatile long*)(ptr),(long)(new_val)))
#define INVALID_MUTEX_OWNER (-1)

#include <unistd.h>
#include <sys/syscall.h>

#define THREAD_LOCAL            __thread

#define OS_DLL_POST(fileName) ("lib" + (fileName) + ".so")

inline void* ALIGNED_MALLOC( size_t size, size_t alignment )
{
    void* t = nullptr;
    if (0 != posix_memalign(&t, alignment < sizeof(void*) ? sizeof(void*) : alignment, size))
    {
        t = nullptr;
    }
    return t;
}
#define GET_CURRENT_PROCESS_ID() getpid()
#define GET_CURRENT_THREAD_ID() ((int)syscall(SYS_gettid))
#include <sched.h>
typedef cpu_set_t                      affinityMask_t;

#include "cl_secure_string_linux.h"

#define MEMCPY_S                        Intel::OpenCL::Utils::safeMemCpy
#define STRCPY_S                        Intel::OpenCL::Utils::safeStrCpy
#define STRNCPY_S                       Intel::OpenCL::Utils::safeStrNCpy
#define STRCAT_S                        Intel::OpenCL::Utils::safeStrCat
#define STRTOK_S                        Intel::OpenCL::Utils::safe_strtok
#define SPRINTF_S                       snprintf
#define VSPRINTF_S                      Intel::OpenCL::Utils::safeVStrPrintf
#define STRCMPI_S                       strncasecmp
#define SSCANF_S                        sscanf

#define STACK_ALLOC( size )             alloca(size)
#define STACK_FREE( ptr )

#define IsPowerOf2(x)                   (__builtin_popcount((x)) == 1)

#endif

// Define compiler static assert
//#define STATIC_ASSERT(e) typedef char __STATIC_ASSERT__[(e)?1:-1]
#define STATIC_ASSERT(e) static_assert(e,"")

#define PAGE_4K_SIZE                    4096
#define CPU_CACHE_LINE_SIZE                64

// assumes alignment is a power of 2
#define IS_ALIGNED_ON( what, alignment ) (0 == (((size_t)(what)              &  ((size_t)(alignment) - 1))))
#define ALIGN_DOWN( what, alignment )    ((size_t)(what)                     & ~((size_t)(alignment) - 1))
#define ALIGN_UP( what, alignment )      ((size_t)((what) + (alignment) - 1) & ~((size_t)(alignment) - 1))
