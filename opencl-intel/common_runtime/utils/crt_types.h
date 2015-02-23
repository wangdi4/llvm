// Copyright (c) 2006-2007 Intel Corporation
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
#include <CL/IntelInternal/cl_ext_intel.h>
#include <assert.h>
#ifdef _WIN32
#include <intrin.h>
#include <windows.h>
#endif

typedef void (CL_CALLBACK *user_func)(void *);
typedef void (CL_CALLBACK *ctxt_logging_fn)(const char *, const void *, size_t, void *);
typedef void (CL_CALLBACK *prog_logging_fn)(cl_program, void *);
typedef void (CL_CALLBACK *mem_dtor_fn)(cl_mem, void *);
typedef void (CL_CALLBACK *pfn_notify)(cl_event, cl_int, void *);
typedef void (CL_CALLBACK *pfn_free)(cl_command_queue queue, cl_uint num_svm_pointers, void *svm_pointers[], void *user_data);
typedef int crt_err_code;

#define CRT_FAIL        0x1
#define CRT_SUCCESS     0x0


#define MAX_STRLEN                  (1024)
#define INTEL_PLATFORM_PROFILE      "FULL_PROFILE"
#define INTEL_PLATFORM_NAME         "Intel(R) OpenCL"
#define INTEL_PLATFORM_VENDOR       "Intel(R) Corporation"
#define INTEL_ICD_EXTENSIONS_STRING "INTEL"

#ifdef _WIN32 
    #define CLAPI_EXPORT
#else
    #define CLAPI_EXPORT __attribute__((visibility("default")))
#endif

typedef enum
{    
    OPENCL_INVALID = 0,
    OPENCL_1_1     = 11,
    OPENCL_1_2     = 12,
    OPENCL_2_0     = 20
} OclVersion;

#define INTEL_OPENCL_1_1_PVER_STR "OpenCL 1.1 "
#define INTEL_OPENCL_1_2_PVER_STR "OpenCL 1.2 "
#define INTEL_OPENCL_2_0_PVER_STR "OpenCL 2.0 "


// default device type, which will be picked by the
// CRT in case of two underlying devices
#define CRT_DEFAULT_DEVICE_TYPE  CL_DEVICE_TYPE_GPU

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
#ifdef _WIN32
    return (long)_InterlockedIncrement( (volatile long*)Addend );
#else
    return __sync_add_and_fetch(Addend, 1);
#endif
}

inline long atomic_decrement(long* Addend)
{
#ifdef _WIN32
    return (long)_InterlockedDecrement( (volatile long*)Addend );
#else
    return __sync_sub_and_fetch(Addend, 1);
#endif
}

#ifdef _WIN32
#define SET_ALIAS(func) 
#define GET_ALIAS(func) (func)
#else
#define SET_ALIAS(func) void intel_ocl_crt_##func() __attribute__ ((alias (#func))) 
#define GET_ALIAS(func) intel_ocl_crt_##func 
#endif

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

inline bool IsPowerOf2(unsigned int uiNum)
{
#ifdef _WIN32
    return __popcnt(uiNum) == 1;
#else
    return __builtin_popcount(uiNum) == 1;
#endif
}

// for GetCRTInfo()
#define CRT_NAMED_PIPE 0x1000
typedef unsigned int crt_info;
