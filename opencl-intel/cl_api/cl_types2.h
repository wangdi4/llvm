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

#pragma once

#if defined(_MSC_VER)
// ------------------------------------
// Define OCL types
// Single precision floating point
// Used by built-in functions
struct float2 {float a; float b;};
typedef __m128 float4;
struct float8
{
	__m128	a; __m128	b;
};
struct float16
{
	__m128	a; __m128	b; __m128	c;	__m128	d;
};

// ------------------------------------
// Define OCL integer types
// char{2|4|8|16}, uchar, uchar{2|4|8|16}, short, short{2|4|8|16},
// ushort, ushort{2|4|8|16}, int, int{2|4|8|16}, uint,
// uint{2|4|8|16}, long, long{2|4|8|16} ulong, or ulong{2|4|8|16}

// TODO: The short vector parameters ( < 64bit) are passed, one by one in stack
// So each element should be defined as 'int'
// Change LLVM backend (JIT) to perform more efficient call to short vectors

//char - 8bit
typedef char			_1i8;
struct _2i8 { int a; int b;};
typedef __m128i			_2i8p;
struct _4i8 { int a; int b; int c; int d;};
typedef __m128i			_4i8p;
struct _8i8 {_4i8 a; _4i8 b;};
typedef __m128i			_8i8p;
typedef __m128i			_16i8;

//uchar - 8bit
typedef unsigned char	_1u8;
struct _2u8 { unsigned int a; unsigned int b;};
typedef __m128i			_2u8p;
struct _4u8 { int a; int b; int c; int d;};
typedef __m128i			_4u8p;
struct _8u8 {_4u8 a; _4u8 b;};
typedef __m128i			_8u8p;
typedef __m128i			_16u8;

//short - 16 bit
typedef short			_1i16;
struct _2i16 { int a; int b;};
typedef __m128i			_2i16p;
struct _4i16 { _2i16 a; _2i16 b;};
typedef __m128i			_4i16p;
typedef __m128i			_8i16;
struct _16i16 { __m128i a; __m128i b;};

//ushort - 16 bit
typedef unsigned short	_1u16;
struct _2u16 { unsigned int a; unsigned int b;};
typedef __m128i			_2u16p;
struct _4u16 { _2u16 a; _2u16 b;};
typedef __m128i			_4u16p;
typedef __m128i			_8u16;
struct _16u16 { __m128i a; __m128i b;};

//int - 32 bit
typedef int				_1i32;
struct _2i32 { int a; int b;};
typedef __m128i			_4i32;
struct _8i32 { __m128i a; __m128i b;};
struct _16i32 { __m128i a; __m128i b;__m128i c; __m128i d;};

//unit - 32 bit
typedef unsigned int	_1u32;
struct _2u32 { unsigned int a; unsigned int b;};
typedef __m128i			_4u32;
struct _8u32 { __m128i a; __m128i b;};
struct _16u32 { __m128i a; __m128i b;__m128i c; __m128i d;};

//long - 64 bit
typedef __int64		_1i64;
struct	_1i64ret { int lo; int hi;};
typedef __m128i		_2i64;
struct _4i64 { __m128i a; __m128i b;};
struct _8i64 { __m128i a; __m128i b;__m128i c; __m128i d;};
struct _16i64 { __m128i a; __m128i b;__m128i c; __m128i d;
__m128i e; __m128i f;__m128i g; __m128i h;};

//ulong 64 bit
typedef unsigned __int64	_1u64;
struct	_1u64ret { unsigned int lo; unsigned int hi;};
typedef __m128i				_2u64;
struct _4u64 { __m128i a; __m128i b;};
struct _8u64 { __m128i a; __m128i b;__m128i c; __m128i d;};
struct _16u64 { __m128i a; __m128i b;__m128i c; __m128i d;
__m128i e; __m128i f;__m128i g; __m128i h;};

#define lo a
#define hi b

typedef short half;

//Double definition should be moved to cl_types when implemented
typedef __m128d double2;


struct double4
{
	__m128d	a; __m128d	b;
};
struct double8
{
	__m128d	a; __m128d	b; __m128d	c;	__m128d	d;
};
struct double16
{
	__m128d	a; __m128d	b; __m128d	c;	__m128d	d;
	__m128d	e; __m128d	f; __m128d	g;	__m128d	h;
};




struct float2_svml	{	__m128  a;	} ;
struct float4_svml	{	__m128	a;	} ;
struct float8_svml	{	__m128	a;	} ;
// Make these types compatible with float8 in LLVM
struct float1x2_svml {   float s0; float s4;  };
struct float2x2_svml {   __m128 a; __m128 b;  } ;
struct float4x2_svml {   float4 a; float4 b;  } ;
struct float8x2_svml {   float8 r1; float8 r2;  } ;
struct float16x2_svml {   float16 r1; float16 r2;  } ;

struct double1x2_svml {   double s0; double s4;  };
struct double2x2_svml {   double2 a; double2 b;  } ;
struct double4x2_svml {   double4 a; double4 b;  } ;
struct double8x2_svml {   double8 r1; double8 r2;  } ;
struct double16x2_svml {   double16 r1; double16 r2;  } ;

#else

//char - 8bit
#define _1i8			char
#define _2i8			char2
#define _4i8			char4
#define	_8i8			char8
#define	_16i8			char16

//uchar - 8bit
#define _1u8			uchar
#define _2u8			uchar2
#define _4u8			uchar4
#define	_8u8			uchar8
#define	_16u8			uchar16

//short - 16 bit

#define _1i16			short
#define _2i16			short2
#define _4i16			short4
#define	_8i16			short8
#define	_16i16			short16

//ushort - 16 bit
#define _1u16			ushort
#define _2u16			ushort2
#define _4u16			ushort4
#define	_8u16			ushort8
#define	_16u16			ushort16

//int - 32 bit
#define _1i32			int
#define _2i32			int2
#define _4i32			int4
#define _8i32			int8
#define _16i32			int16

//unit - 32 bit
#define _1u32			uint
#define _2u32			uint2
#define _4u32			uint4
#define _8u32			uint8
#define _16u32			uint16

//long - 64 bit
#define _1i64			long
#define _2i64			long2
#define _4i64			long4
#define _8i64			long8
#define _16i64			long16

//ulong 64 bit
#define _1u64			ulong
#define _2u64			ulong2
#define _4u64			ulong4
#define _8u64			ulong8
#define _16u64			ulong16

typedef float8		float1x2_svml;
typedef float8		float2x2_svml;
typedef float8		float4x2_svml;






#endif