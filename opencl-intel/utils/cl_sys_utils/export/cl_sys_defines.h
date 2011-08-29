/////////////////////////////////////////////////////////////////////////
// cl_sys_defines.h
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
// or disclosed in any way without Intel’s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////
#pragma once

#if defined (_WIN32)
#include <basetsd.h>

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

#else //LINUX

#define CL_MAX_INT32 INT_MAX
#define CL_MAX_UINT32	UINT_MAX
#define API_FUNCTION
#define ASM_FUNCTION
#define CDECL   __attribute((cdecl))
#define STDCALL __attribute((stdcall))

#define PACKED  __attribute ((packed))
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

#define STDCALL __attribute((stdcall))

#define FALSE	0

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

#endif

#if defined (_WIN32)

#define MEMCPY_S                          memcpy_s
#define MULTIBYTE_TO_WIDE_CHARACTER_S	::mbstowcs_s
#define STRCPY_S                          strcpy_s
#define STRCAT_S                          strcat_s
#define STRTOK_S                          strtok_s
#define SPRINTF_S                         sprintf_s
#define WCSCAT_S                          wcscat_s
#define SWPRINTF_S                        swprintf_s
#define WCSCPY_S                          wcscpy_s
#define VSPRINTF_S                        vsprintf_s

typedef unsigned long long               affinityMask_t;

// aligned malloc
#include <malloc.h>
#define ALIGNED_MALLOC( size, alignment ) _aligned_malloc( size, (alignment) < sizeof(void*) ? sizeof(void*) : (alignment))
#define ALIGNED_FREE                      _aligned_free

#else

#include "cl_secure_string_linux.h"

#define MEMCPY_S                        Intel::OpenCL::Utils::safeMemCpy
#define MULTIBYTE_TO_WIDE_CHARACTER_S   Intel::OpenCL::Utils::safeMbToWc
#define STRCPY_S                        Intel::OpenCL::Utils::safeStrCpy
#define STRCAT_S                        Intel::OpenCL::Utils::safeStrCat
#define STRTOK_S                        Intel::OpenCL::Utils::safe_strtok
#define SPRINTF_S                       Intel::OpenCL::Utils::safeStrPrintf
#define WCSCAT_S                        Intel::OpenCL::Utils::safeWStrCat
#define SWPRINTF_S                      Intel::OpenCL::Utils::safeWStrPrintf
#define WCSCPY_S                        Intel::OpenCL::Utils::safeWStrCpy
#define VSPRINTF_S                      Intel::OpenCL::Utils::safeVStrPrintf

#include <sched.h>
typedef cpu_set_t                      affinityMask_t;

// aligned malloc
#include <stdlib.h>

inline void* ALIGNED_MALLOC( size_t size, size_t alignment )
{
    void* t = NULL;
    posix_memalign(&t, alignment < sizeof(void*) ? sizeof(void*) : alignment, size);
    return t;
}

#define ALIGNED_FREE                      free

#endif

// Define compiler static assert
#define STATIC_ASSERT(e) typedef char __STATIC_ASSERT__[(e)?1:-1]

#define PAGE_4K_SIZE                    4096

