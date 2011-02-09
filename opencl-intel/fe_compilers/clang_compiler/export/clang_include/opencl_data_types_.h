// Copyright (c) 1997-2010 Intel Corporation
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

#pragma once

// address space qualifiers

#define __private   __attribute__((address_space(0)))
#define __global    __attribute__((address_space(1)))
#define __constant  __attribute__((address_space(2)))
#define __local     __attribute__((address_space(3)))

#define private     __attribute__((address_space(0)))
#define global      __attribute__((address_space(1)))
#define constant    __attribute__((address_space(2)))
#define local       __attribute__((address_space(3)))

// built-in scalar data types:

/**
 * An unsigned 8-bit integer.
 */
typedef unsigned char uchar;

/**
 * An unsigned 16-bit integer.
 */
typedef unsigned short ushort;

/**
 * An unsigned 32-bit integer.
 */
typedef unsigned int uint;

/**
 * An unsigned 64-bit integer.
 */
typedef unsigned long ulong;

/**
 * A 16-bit float. The half data type must conform to the IEEE 754-
 * 2008 half precision storage format.
 */
typedef short half;

/**
 * The unsigned integer type of the result of the sizeof operator. This
 * is a 32-bit unsigned integer if CL_DEVICE_ADDRESS_BITS
 * defined in table 4.3 is 32-bits and is a 64-bit unsigned integer if
 * CL_DEVICE_ADDRESS_BITS is 64-bits.
 */
typedef __typeof__(sizeof(int)) size_t;

/**
 * A signed integer type that is the result of subtracting two pointers.
 * This is a 32-bit signed integer if CL_DEVICE_ADDRESS_BITS
 * defined in table 4.3 is 32-bits and is a 64-bit signed integer if
 * CL_DEVICE_ADDRESS_BITS is 64-bits.
 */
typedef __typeof__(((int*)0)-((int*)0)) ptrdiff_t;

/**
 * A signed integer type with the property that any valid pointer to
 * void can be converted to this type, then converted back to pointer
 * to void, and the result will compare equal to the original pointer.
 */

// the following code is copied from stdint.h

#define __stdint_join3(a,b,c) a ## b ## c

// we use _t1, so the generated type won't be called int64_t/int32_t, since these types are already defined in stdint.h
#define  __intn_t(n) __stdint_join3( int, n, _t1)
#define __uintn_t(n) __stdint_join3(uint, n, _t1)

# ifndef __int8_t_defined /* glibc sys/types.h also defines int64_t*/
#define int64_t1 long
#define int32_t1 int
# endif /* __int8_t_defined */
#define uint64_t1 unsigned long
#define uint32_t1 unsigned int

#ifndef __intptr_t_defined
typedef  __intn_t(__INTPTR_WIDTH__)  intptr_t;
#define __intptr_t_defined
#endif

/**
 * An unsigned integer type with the property that any valid pointer to
 * void can be converted to this type, then converted back to pointer
 * to void, and the result will compare equal to the original pointer.
 */
typedef __uintn_t(__INTPTR_WIDTH__) uintptr_t;

#undef int64_t1
#undef uint64_t1
#undef int32_t1
#undef uint32_t1

struct _image2d_t;
struct _image3d_t;
typedef struct _image2d_t* image2d_t;
typedef struct _image3d_t* image3d_t;
typedef uint sampler_t;

// built-in vector data types:

typedef char char2 __attribute__((ext_vector_type(2)));
typedef char char3 __attribute__((ext_vector_type(3)));
typedef char char4 __attribute__((ext_vector_type(4)));
typedef char char8 __attribute__((ext_vector_type(8)));
typedef char char16 __attribute__((ext_vector_type(16)));
typedef uchar uchar2 __attribute__((ext_vector_type(2)));
typedef uchar uchar3 __attribute__((ext_vector_type(3)));
typedef uchar uchar4 __attribute__((ext_vector_type(4)));
typedef uchar uchar8 __attribute__((ext_vector_type(8)));
typedef uchar uchar16 __attribute__((ext_vector_type(16)));
typedef short short2 __attribute__((ext_vector_type(2)));
typedef short short3 __attribute__((ext_vector_type(3)));
typedef short short4 __attribute__((ext_vector_type(4)));
typedef short short8 __attribute__((ext_vector_type(8)));
typedef short short16 __attribute__((ext_vector_type(16)));
typedef ushort ushort2 __attribute__((ext_vector_type(2)));
typedef ushort ushort3 __attribute__((ext_vector_type(3)));
typedef ushort ushort4 __attribute__((ext_vector_type(4)));
typedef ushort ushort8 __attribute__((ext_vector_type(8)));
typedef ushort ushort16 __attribute__((ext_vector_type(16)));
typedef int int2 __attribute__((ext_vector_type(2)));
typedef int int3 __attribute__((ext_vector_type(3)));
typedef int int4 __attribute__((ext_vector_type(4)));
typedef int int8 __attribute__((ext_vector_type(8)));
typedef int int16 __attribute__((ext_vector_type(16)));
typedef uint uint2 __attribute__((ext_vector_type(2)));
typedef uint uint3 __attribute__((ext_vector_type(3)));
typedef uint uint4 __attribute__((ext_vector_type(4)));
typedef uint uint8 __attribute__((ext_vector_type(8)));
typedef uint uint16 __attribute__((ext_vector_type(16)));
typedef long long2 __attribute__((ext_vector_type(2)));
typedef long long3 __attribute__((ext_vector_type(3)));
typedef long long4 __attribute__((ext_vector_type(4)));
typedef long long8 __attribute__((ext_vector_type(8)));
typedef long long16 __attribute__((ext_vector_type(16)));
typedef ulong ulong2 __attribute__((ext_vector_type(2)));
typedef ulong ulong3 __attribute__((ext_vector_type(3)));
typedef ulong ulong4 __attribute__((ext_vector_type(4)));
typedef ulong ulong8 __attribute__((ext_vector_type(8)));
typedef ulong ulong16 __attribute__((ext_vector_type(16)));
typedef float float2 __attribute__((ext_vector_type(2)));
typedef float float3 __attribute__((ext_vector_type(3)));
typedef float float4 __attribute__((ext_vector_type(4)));
typedef float float8 __attribute__((ext_vector_type(8)));
typedef float float16 __attribute__((ext_vector_type(16)));
typedef half half2 __attribute__((ext_vector_type(2)));
typedef half half3 __attribute__((ext_vector_type(3)));
typedef half half4 __attribute__((ext_vector_type(4)));
typedef half half8 __attribute__((ext_vector_type(8)));
typedef half half16 __attribute__((ext_vector_type(16)));
typedef double double2 __attribute__((ext_vector_type(2)));
typedef double double3 __attribute__((ext_vector_type(3)));
typedef double double4 __attribute__((ext_vector_type(4)));
typedef double double8 __attribute__((ext_vector_type(8)));
typedef double double16 __attribute__((ext_vector_type(16)));
