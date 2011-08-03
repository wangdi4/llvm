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
#include <assert.h>
#include <intrin.h>
#include <windows.h>
#pragma intrinsic( _InterlockedCompareExchange )

typedef void (*user_func)(void *);
typedef void (CL_CALLBACK *logging_fn)(const char *, const void *, size_t, void *);
typedef void (CL_CALLBACK *prog_logging_fn)(cl_program, void *) ;
typedef void (CL_CALLBACK *mem_dtor_fn)(cl_mem, void *);
typedef void (CL_CALLBACK *pfn_notify)(cl_event, cl_int, void *);
typedef int crt_err_code ;

#define CRT_FAIL		0x1
#define CRT_SUCCESS		0x0


#define MAX_STRLEN                  (1024)
#define INTEL_PLATFORM_NAME         "Intel(R) OpenCL HD Graphics"
#define INTEL_ICD_EXTENSIONS_STRING "INTC"
#define PLATFORM_EXT_STRING         ""

	/// default device type, which is gonna be picked by the
	/// CRT in case of two underlying devices
#define CRT_DEFAULT_DEVICE_TYPE  CL_DEVICE_TYPE_CPU

	/// Alignment
#define CRT_PAGE_ALIGNMENT           ( 4096 )

	/// Atomic Functions
inline cl_uint atomic_increment(cl_uint* Addend)
{
	return (cl_uint)_InterlockedIncrement( (volatile long*)Addend );
}

inline cl_uint atomic_decrement(cl_uint* Addend)
{
	return (cl_uint)_InterlockedDecrement( (volatile long*)Addend );
}

inline cl_uint atomic_increment_ret_prev(cl_uint* Addend)
{
	return (cl_uint)InterlockedExchangeAdd( (volatile long*)Addend, 1 );
}


inline cl_uint atomic_decrement_ret_prev(cl_uint* Addend)
{
	return (cl_uint)InterlockedExchangeAdd( (volatile long*)Addend, -1);
}

