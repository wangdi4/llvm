// Copyright (c) 2006-2012 Intel Corporation
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
//
//  Original author: rjiossy
///////////////////////////////////////////////////////////
#pragma once
#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <assert.h>
#include <intrin.h>
#include <windows.h>
#pragma intrinsic( _InterlockedCompareExchange )

typedef void (CL_CALLBACK *user_func)(void *);
typedef void (CL_CALLBACK *ctxt_logging_fn)(const char *, const void *, size_t, void *);
typedef void (CL_CALLBACK *prog_logging_fn)(cl_program, void *);
typedef void (CL_CALLBACK *mem_dtor_fn)(cl_mem, void *);
typedef void (CL_CALLBACK *pfn_notify)(cl_event, cl_int, void *);
typedef int crt_err_code;

#define CRT_FAIL        0x1
#define CRT_SUCCESS     0x0


#define MAX_STRLEN                  (1024)
#define INTEL_PLATFORM_PROFILE      "FULL_PROFILE"
#define INTEL_PLATFORM_NAME         "Intel(R) OpenCL"
#define INTEL_PLATFORM_VENDOR       "Intel(R) Corporation"
#define INTEL_ICD_EXTENSIONS_STRING "INTEL"

typedef enum
{    
    OPENCL_INVALID = 0,
    OPENCL_1_1     = 11,
    OPENCL_1_2     = 12
} OclVersion;

#define INTEL_OPENCL_1_1_PVER_STR "OpenCL 1.1 "
#define INTEL_OPENCL_1_2_PVER_STR "OpenCL 1.2 "


// default device type, which will be picked by the
// CRT in case of two underlying devices
#define CRT_DEFAULT_DEVICE_TYPE  CL_DEVICE_TYPE_CPU

// Alignment
#define CRT_PAGE_ALIGNMENT           ( 4096 )
#define CRT_IMAGE_PITCH_ALIGN        ( 64 )

// IMAGE formats
#define IMAGE_FORMATS_UNION
typedef enum
{
    INVALID_MEMOBJ_SIZE    = 0xFFFFFFF1,
    INVALID_IMG_FORMAT     = 0xFFFFFFF2
} IMAGE_FAIL_TYPE;


// Atomic Functions
inline cl_uint atomic_increment(long* Addend)
{
    return (long)_InterlockedIncrement( (volatile long*)Addend );
}

inline long atomic_decrement(long* Addend)
{
    return (long)_InterlockedDecrement( (volatile long*)Addend );
}

inline cl_uint atomic_add_ret_prev(long* Addend, long num)
{
    return (long)InterlockedExchangeAdd( (volatile long*)Addend, num);
}

inline long test_and_set(long* Addend, long comparand, long exchange)
{
    return (long)InterlockedCompareExchange((volatile long*)Addend,exchange,comparand);
}

#define cl_intel_dx9_media_sharing     1

// Extensions support
enum CrtExtension
{
    CRT_CL_GL_EXT               = 1<<0,
    CRT_CL_INTEL_D3D9_EXT       = 1<<1,
    CRT_CL_D3D9_EXT             = 1<<2,
    CRT_CL_D3D10_EXT            = 1<<3,
    CRT_CL_D3D11_EXT            = 1<<4
};

cl_int GetCrtExtension(const char* str_extensions);
cl_uint GetPlatformVersion(const char* platform_version_str);
