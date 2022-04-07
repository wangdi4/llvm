// Copyright (c) 2010-2011 Intel Corporation
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

#ifndef __INTRIN_H__
#define __INTRIN_H__

#ifdef __OPENCL__
#define __OCL_CAST_TO_PRIVATE(TY) (__private TY)
#else
#define __OCL_CAST_TO_PRIVATE(TY)
#endif

#ifdef __OPENCL__
#define __OCL_PRIVATE_ADDR_SPACE __private
#else
#define __OCL_PRIVATE_ADDR_SPACE
#endif

#ifdef __OPENCL__
typedef long __i64;
typedef unsigned long __u64;
#else
#ifdef _WIN32
typedef __int64 __i64;
typedef unsigned __int64 __u64;
#else
typedef long long __i64;
typedef unsigned long long __u64;
#endif
#endif // __OPENCL__

#include <immintrin.h>

#endif
