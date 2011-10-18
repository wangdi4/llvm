/*
    Copyright(c) 2005-2011 Intel Corporation.  All Rights Reserved.

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
#ifndef _ITTX_UTILS_H_
#define _ITTX_UTILS_H_

/************************************************************************/
/* Types                                                                */
/************************************************************************/
#if ITT_PLATFORM == ITT_PLATFORM_WIN
typedef __int32 __ittx_int32;
typedef __int64 __ittx_int64;
typedef unsigned __int32 __ittx_uint32;
typedef unsigned __int64 __ittx_uint64;
#elif ITT_PLATFORM == ITT_PLATFORM_POSIX
typedef int32_t __ittx_int32;
typedef int64_t __ittx_int64;
typedef uint32_t __ittx_uint32;
typedef uint64_t __ittx_uint64;
#else
#error Unsupported platform
#endif


/************************************************************************/
/* Atomic increment                                                     */
/************************************************************************/
#ifndef INTEL_NO_ITTNOTIFY_API
#if ITT_PLATFORM == ITT_PLATFORM_WIN
#include <math.h> // Magical fix for x64 debug compile issue -- warning C4985: 'ceil': attributes not present on previous declaration.
#include <intrin.h>
#endif // ITT_PLATFORM

#if ITT_PLATFORM == ITT_PLATFORM_WIN
#if !defined(_M_X64) && !defined(_MANAGED) /* The asm really pisses off the clr compiler */ 

// (Vista32)
INLINE __ittx_int32 __ittx_atomic_increment(__ittx_int32 volatile* in_pAddend)
{
    __ittx_int32 result;
    __asm {
        mov         ecx, dword ptr [in_pAddend]
        mov         eax, 1
        lock xadd   dword ptr [ecx], eax 
        inc         eax
        mov         result, eax
    }
    return result;
}

#else // defined(_M_X64) || defined(_MANAGED)

// (Vista64)
INLINE __ittx_int32 __ittx_atomic_increment(__ittx_int32 volatile* in_pAddend)
{
    return _InterlockedIncrement((long*)in_pAddend);
}

#endif // _M_X64
#elif ITT_PLATFORM == ITT_PLATFORM_POSIX
INLINE __ittx_int32 __ittx_atomic_increment(__ittx_int32 volatile* in_pAddend)
{
    int32_t result;
    __asm__("                \
            movl       $1, %0;   \
            lock xaddl  %0, (%1); \
            incl       %0;       \
            "
            : "=&r" (result)
            : "r" (in_pAddend)
            );
    return result;
}
#else 
#error "Not defined for current platform"
#endif // ITT_PLATFORM

#else // INTEL_NO_ITTNOTIFY_API
#define __ittx_atomic_increment(x) 0
#endif // INTEL_NO_ITTNOTIFY_API


/************************************************************************/
/* RDTSC                                                                */
/************************************************************************/
#ifndef INTEL_NO_ITTNOTIFY_API

#if ITT_PLATFORM == ITT_PLATFORM_WIN
#ifdef _M_X64
    __ittx_uint64 __rdtsc(void);
#pragma intrinsic(__rdtsc)
#define __ITTX_RDTSC __rdtsc
#else
#define __ITTX_RDTSC_STACK(ts) \
    __asm rdtsc \
    __asm mov DWORD PTR [ts], eax \
    __asm mov DWORD PTR [ts+4], edx

    __inline __ittx_uint64 __itt_rdtsc() 
    {
        __ittx_uint64 t;
        __ITTX_RDTSC_STACK(t);
        return t;
    }
#define __ITTX_RDTSC __itt_rdtsc
#endif
#elif ITT_PLATFORM == ITT_PLATFORM_POSIX
    INLINE __ittx_uint64 __itt_rdtsc(void) 
    {
        __ittx_uint64 result;
#ifdef defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
        unsigned int __a,__d;
        __asm volatile("rdtsc" : "=a" (__a), "=d" (__d));
        result = ((unsigned long long)__a) | (((unsigned long long)__d)<<32);
#else
        __asm__ __volatile__("rdtsc" : "=A" (result));
#endif
        return result;
    }
#define __ITTX_RDTSC __itt_rdtsc
#else
#error Unsupported platform
#endif /*ITT_PLATFORM*/

INLINE __ittx_uint64 __ittx_get_current_time()
{
    return __ITTX_RDTSC();
}
#else /*INTEL_NO_ITTNOTIFY_API*/
#define __ittx_get_current_time() 0
#endif /*INTEL_NO_ITTNOTIFY_API*/

#endif /*_ITTX_UTILS_H_*/