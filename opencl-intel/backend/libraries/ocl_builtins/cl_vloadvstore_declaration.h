// Copyright (c) 2006-2009 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#pragma once

#include "cl_types.h"

#ifdef CL_BUILTIN_FUNCTIONS_EXPORTS
#define VLOADVSTORE_FUNC_DECL __declspec(dllexport)
#else
#define VLOADVSTORE_FUNC_DECL __declspec(dllimport)
#endif

/*

// two input in memory, vector output
// 8 bit
#pragma linkage _lnk_in_mem_mem_res_2x8_ = ( result (xmm0) parameters (memory, memory) )
#pragma linkage _lnk_in_mem_mem_res_4x8_ = ( result (xmm0) parameters (memory, memory) )
#pragma linkage _lnk_in_mem_mem_res_8x8_ = ( result (xmm0) parameters (memory, memory) )
#pragma linkage _lnk_in_mem_mem_res_16x8_ = ( result (xmm0) parameters (memory, memory) )

// 16 bit
#pragma linkage _lnk_in_mem_mem_res_2x16_ = ( result (xmm0) parameters (memory, memory) )
#pragma linkage _lnk_in_mem_mem_res_4x16_ = ( result (xmm0) parameters (memory, memory) )
#pragma linkage _lnk_in_mem_mem_res_8x16_ = ( result (xmm0) parameters (memory, memory) )
#pragma linkage _lnk_in_mem_mem_res_16x16_ = ( result ((xmm0 xmm1)) parameters (memory, memory) )

// 32 bit
#pragma linkage _lnk_in_mem_mem_res_2x32_ = ( result (mm0) parameters (memory, memory) )
#pragma linkage _lnk_in_mem_mem_res_4x32_ = ( result (xmm0) parameters (memory, memory) )
#pragma linkage _lnk_in_mem_mem_res_8x32_ = ( result ((xmm0 xmm1)) parameters (memory, memory) )
#pragma linkage _lnk_in_mem_mem_res_16x32_ = ( result ((xmm0 xmm1 xmm2 xmm3)) parameters (memory, memory) )

// 64 bit
#pragma linkage _lnk_in_mem_mem_res_2x64_ = ( result (xmm0) parameters (memory, memory) )
#pragma linkage _lnk_in_mem_mem_res_4x64_ = ( result ((xmm0 xmm1)) parameters (memory, memory) )
#pragma linkage _lnk_in_mem_mem_res_8x64_ = ( result ((xmm0 xmm1 xmm2 xmm3)) parameters (memory, memory) )

// three input one vector two memory
// 8 bit

#pragma linkage _lnk_in_2x8_mem_mem_ = ( parameters (memory, memory, memory) )
#pragma linkage _lnk_in_4x8_mem_mem_ = ( parameters (memory, memory, memory) )
#pragma linkage _lnk_in_8x8_mem_mem_ = ( parameters (mm0, memory, memory) )
#pragma linkage _lnk_in_16x8_mem_mem_ = (parameters (xmm0, memory, memory) )

// 16 bit
#pragma linkage _lnk_in_2x16_mem_mem_ = ( parameters (memory, memory, memory) )
#pragma linkage _lnk_in_4x16_mem_mem_ = ( parameters (mm0, memory, memory) )
#pragma linkage _lnk_in_8x16_mem_mem_ = ( parameters (xmm0, memory, memory) )
#pragma linkage _lnk_in_16x16_mem_mem_ = (parameters ((xmm0 xmm1) , memory, memory) )

// 32 bit
#pragma linkage _lnk_in_2x32_mem_mem_ = ( parameters (mm0, memory, memory) )
#pragma linkage _lnk_in_4x32_mem_mem_ = ( parameters (xmm0, memory, memory) )
#pragma linkage _lnk_in_8x32_mem_mem_ = ( parameters ( (xmm0 xmm1), memory, memory) )
#pragma linkage _lnk_in_16x32_mem_mem_ = (parameters ((xmm0 xmm1 xmm2 xmm3) , memory, memory) )

// 64 bit
#pragma linkage _lnk_in_2x64_mem_mem_ = ( parameters (xmm0, memory, memory) )
#pragma linkage _lnk_in_4x64_mem_mem_ = ( parameters ( (xmm0 xmm1), memory, memory) )
#pragma linkage _lnk_in_8x64_mem_mem_ = ( parameters ( (xmm0 xmm1 xmm2 xmm3), memory, memory) )
#pragma linkage _lnk_in_16x64_mem_mem_ = (parameters (memory , memory, memory) )

#pragma linkage _lnk_in_mem_mem_res_f = ( result (xmm0) parameters (memory, memory) )

*/

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)

//8 bit load
extern "C" VLOADVSTORE_FUNC_DECL _2i8p __vload2_2i8( size_t offset, const char* ptr);
//_//_pragma (use_linkage _lnk_in_mem_mem_res_2x8_ ( __vload2_2i8 ))

extern "C" VLOADVSTORE_FUNC_DECL _4i8p __vload4_4i8( size_t offset, const char* ptr);
//_//_pragma (use_linkage _lnk_in_mem_mem_res_4x8_(__vload4_4i8));

extern "C" VLOADVSTORE_FUNC_DECL _8i8p __vload8_8i8( size_t offset, const char* ptr );
//_//_pragma (use_linkage  _lnk_in_mem_mem_res_8x8_(__vload8_8i8));

extern "C" VLOADVSTORE_FUNC_DECL _16i8 __vload16_16i8( size_t offset, const char* ptr );
//_//_pragma (use_linkage _lnk_in_mem_mem_res_16x8_(__vload16_16i8));

extern "C" VLOADVSTORE_FUNC_DECL _2u8p __vload2_2u8( size_t offset, const unsigned char* ptr);
//_//_pragma (use_linkage _lnk_in_mem_mem_res_2x8_(__vload2_2u8));

extern "C" VLOADVSTORE_FUNC_DECL _4u8p __vload4_4u8( size_t offset, const unsigned char* ptr);
//_//_pragma (use_linkage _lnk_in_mem_mem_res_4x8_(__vload4_4u8));

extern "C" VLOADVSTORE_FUNC_DECL _8u8p __vload8_8u8( size_t offset, const unsigned char* ptr);
//_//_pragma (use_linkage _lnk_in_mem_mem_res_8x8_(__vload8_8u8));

extern "C" VLOADVSTORE_FUNC_DECL _16u8 __vload16_16u8( size_t offset, const unsigned char* ptr);
//_//_pragma (use_linkage _lnk_in_mem_mem_res_16x8_(__vload16_16u8));

//16 bit load
extern "C" VLOADVSTORE_FUNC_DECL _2i16p __vload2_2i16( size_t, const short* );  
//_//_pragma (use_linkage _lnk_in_mem_mem_res_2x16_(__vload2_2i16));

extern "C" VLOADVSTORE_FUNC_DECL _4i16p __vload4_4i16( size_t, const short* );
//_//_pragma (use_linkage _lnk_in_mem_mem_res_4x16_(__vload4_4i16));

extern "C" VLOADVSTORE_FUNC_DECL _8i16 __vload8_8i16( size_t, const short* );
//_//_pragma (use_linkage _lnk_in_mem_mem_res_8x16_(__vload8_8i16));

extern "C" VLOADVSTORE_FUNC_DECL _16i16 __vload16_16i16( size_t, const short* );
//_//_pragma (use_linkage _lnk_in_mem_mem_res_16x16_(__vload16_16i16));

extern "C" VLOADVSTORE_FUNC_DECL _2u16p __vload2_2u16( size_t, const unsigned short* );  
//_//_pragma (use_linkage _lnk_in_mem_mem_res_2x16_(__vload2_2u16));

extern "C" VLOADVSTORE_FUNC_DECL _4u16p __vload4_4u16( size_t, const unsigned short* );
//_//_pragma (use_linkage _lnk_in_mem_mem_res_4x16_(__vload4_4u16));

extern "C" VLOADVSTORE_FUNC_DECL _8u16 __vload8_8u16( size_t, const unsigned short* );
//_//_pragma (use_linkage _lnk_in_mem_mem_res_8x16_(__vload8_8u16));

extern "C" VLOADVSTORE_FUNC_DECL _16u16 __vload16_16u16( size_t, const unsigned short* );
//_//_pragma (use_linkage _lnk_in_mem_mem_res_16x16_(__vload16_16u16));

//32 bit load
extern "C" VLOADVSTORE_FUNC_DECL _2i32 __vload2_2i32( size_t, const int* );
//_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload2_2i32));

extern "C" VLOADVSTORE_FUNC_DECL _4i32 __vload4_4i32( size_t, const int* );  
//_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload4_4i32));

extern "C" VLOADVSTORE_FUNC_DECL _8i32 __vload8_8i32( size_t, const int* );  
//_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload8_8i32));

extern "C" VLOADVSTORE_FUNC_DECL _16i32 __vload16_16i32( size_t, const int* );
//_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload16_16i32));

extern "C" VLOADVSTORE_FUNC_DECL _2u32 __vload2_2u32( size_t, const unsigned int* );
//_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload2_2u32));

extern "C" VLOADVSTORE_FUNC_DECL _4u32 __vload4_4u32( size_t, const unsigned int* );  
//_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload4_4u32));

extern "C" VLOADVSTORE_FUNC_DECL _8u32 __vload8_8u32( size_t, const unsigned int* );  
//_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload8_8u32));

extern "C" VLOADVSTORE_FUNC_DECL _16u32 __vload16_16u32( size_t, const unsigned int* );
//_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload16_16u32));

//64 bit
extern "C" VLOADVSTORE_FUNC_DECL _2i64 __vload2_2i64( size_t, const __int64* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x64_(__vload2_2i64));

extern "C" VLOADVSTORE_FUNC_DECL _4i64 __vload4_4i64( size_t, const __int64* ); 
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x64_(__vload4_4i64));

extern "C" VLOADVSTORE_FUNC_DECL _8i64 __vload8_8i64( size_t, const __int64* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x64_(__vload8_8i64));

extern "C" VLOADVSTORE_FUNC_DECL void __vload16_16i64(_16i64 *, size_t, const __int64* );

extern "C" VLOADVSTORE_FUNC_DECL _2u64 __vload2_2u64( size_t, const __int64* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x64_(__vload2_2u64));

extern "C" VLOADVSTORE_FUNC_DECL _4u64 __vload4_4u64( size_t, const __int64* ); 
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x64_(__vload4_4u64));

extern "C" VLOADVSTORE_FUNC_DECL _8u64 __vload8_8u64( size_t, const __int64* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x64_(__vload8_8u64));

extern "C" VLOADVSTORE_FUNC_DECL void __vload16_16u64(_16u64 *, size_t, const __int64* );


extern "C" VLOADVSTORE_FUNC_DECL float2 __vload2_f2( size_t, const float* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload2_f2));

extern "C" VLOADVSTORE_FUNC_DECL float4 __vload4_f4( size_t, const float* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload4_f4));

extern "C" VLOADVSTORE_FUNC_DECL float8 __vload8_f8( size_t, const float* ); 
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload8_f8));

extern "C" VLOADVSTORE_FUNC_DECL float16 __vload16_f16( size_t, const float* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload16_f16));


//8 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i8( _2i8 data, size_t offset, char* ptr );
_//_pragma (use_linkage _lnk_in_2x8_mem_mem_(__vstore2_i8));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i8( _4i8 data, size_t offset , char* ptr);
_//_pragma (use_linkage _lnk_in_4x8_mem_mem_(__vstore4_i8));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i8( _8i8 data, size_t offset, char* ptr);
_//_pragma (use_linkage _lnk_in_8x8_mem_mem_(__vstore8_i8));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i8( _16i8 data, size_t offset, char* ptr);
_//_pragma (use_linkage _lnk_in_16x8_mem_mem_(__vstore16_i8));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u8( _2u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_2x8_mem_mem_(__vstore2_u8));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u8( _4u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_4x8_mem_mem_(__vstore4_u8));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u8( _8u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_8x8_mem_mem_(__vstore8_u8));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u8( _16u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_16x8_mem_mem_(__vstore16_u8));

//16 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i16( _2i16 data, size_t offset, short* ptr);
_//_pragma (use_linkage _lnk_in_2x16_mem_mem_(__vstore2_i16));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i16( _4i16 data, size_t offset, short* ptr);  
_//_pragma (use_linkage _lnk_in_4x16_mem_mem_(__vstore4_i16));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i16( _8i16 data, size_t offset, short* ptr);  
_//_pragma (use_linkage _lnk_in_8x16_mem_mem_(__vstore8_i16));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i16( _16i16 data, size_t offset, short* ptr);  
_//_pragma (use_linkage _lnk_in_16x16_mem_mem_(__vstore16_i16));


extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u16( _2u16 data, size_t offset, unsigned short* ptr);
_//_pragma (use_linkage _lnk_in_2x16_mem_mem_(__vstore2_u16));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u16( _4u16 data, size_t offset, unsigned short* ptr);  
_//_pragma (use_linkage _lnk_in_4x16_mem_mem_(__vstore4_u16));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u16( _8u16 data, size_t offset, unsigned short* ptr);  
_//_pragma (use_linkage _lnk_in_8x16_mem_mem_(__vstore8_u16));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u16( _16u16 data, size_t offset, unsigned short* ptr);  
_//_pragma (use_linkage _lnk_in_16x16_mem_mem_(__vstore16_u16));

//32 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i32( _2i32, size_t, int* ); 
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore2_i32));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i32( _4i32, size_t, int* );  
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore4_i32));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i32( _8i32, size_t, int* );  
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore8_i32));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i32( _16i32, size_t, int* );  
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore16_i32));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u32( _2u32, size_t, unsigned int* ); 
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore2_u32));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u32( _4u32, size_t, unsigned int* );  
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore4_u32));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u32( _8u32, size_t, unsigned int* );  
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore8_u32));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u32( _16u32, size_t, unsigned int* );  
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore16_u32));

//64 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i64( _2i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_2x64_mem_mem_(__vstore2_i64));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i64( _4i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_4x64_mem_mem_(__vstore4_i64));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i64( _8i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_8x64_mem_mem_(__vstore8_i64));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i64( _16i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_16x64_mem_mem_(__vstore16_i64));


extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u64( _2u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_2x64_mem_mem_(__vstore2_u64));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u64( _4u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_4x64_mem_mem_(__vstore4_u64));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u64( _8u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_8x64_mem_mem_(__vstore8_u64));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u64( _16u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_16x64_mem_mem_(__vstore16_u64));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_f( float2, size_t, float* );  
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore2_f));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_f( float4, size_t, float* ); 
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore4_f));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_f( float8, size_t, float* );  
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore8_f));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_f( float16, size_t, float* );  
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore16_f));


//Local
extern "C" VLOADVSTORE_FUNC_DECL _2i8p __vload2_l2i8( size_t offset, const char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x8_ ( __vload2_l2i8 ))

extern "C" VLOADVSTORE_FUNC_DECL _4i8p __vload4_l4i8( size_t offset, const char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x8_(__vload4_l4i8));

extern "C" VLOADVSTORE_FUNC_DECL _8i8p __vload8_l8i8( size_t offset, const char* ptr );
_//_pragma (use_linkage  _lnk_in_mem_mem_res_8x8_(__vload8_l8i8));

extern "C" VLOADVSTORE_FUNC_DECL _16i8 __vload16_l16i8( size_t offset, const char* ptr );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x8_(__vload16_l16i8));

extern "C" VLOADVSTORE_FUNC_DECL _2u8p __vload2_l2u8( size_t offset, const unsigned char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x8_(__vload2_l2u8));

extern "C" VLOADVSTORE_FUNC_DECL _4u8p __vload4_l4u8( size_t offset, const unsigned char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x8_(__vload4_l4u8));

extern "C" VLOADVSTORE_FUNC_DECL _8u8p __vload8_l8u8( size_t offset, const unsigned char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x8_(__vload8_l8u8));

extern "C" VLOADVSTORE_FUNC_DECL _16u8 __vload16_l16u8( size_t offset, const unsigned char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x8_(__vload16_l16u8));

//16 bit load
extern "C" VLOADVSTORE_FUNC_DECL _2i16p __vload2_l2i16( size_t, const short* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x16_(__vload2_l2i16));

extern "C" VLOADVSTORE_FUNC_DECL _4i16p __vload4_l4i16( size_t, const short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x16_(__vload4_l4i16));

extern "C" VLOADVSTORE_FUNC_DECL _8i16 __vload8_l8i16( size_t, const short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x16_(__vload8_l8i16));

extern "C" VLOADVSTORE_FUNC_DECL _16i16 __vload16_l16i16( size_t, const short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x16_(__vload16_l16i16));

extern "C" VLOADVSTORE_FUNC_DECL _2u16p __vload2_l2u16( size_t, const unsigned short* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x16_(__vload2_l2u16));

extern "C" VLOADVSTORE_FUNC_DECL _4u16p __vload4_l4u16( size_t, const unsigned short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x16_(__vload4_l4u16));

extern "C" VLOADVSTORE_FUNC_DECL _8u16 __vload8_l8u16( size_t, const unsigned short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x16_(__vload8_l8u16));

extern "C" VLOADVSTORE_FUNC_DECL _16u16 __vload16_l16u16( size_t, const unsigned short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x16_(__vload16_l16u16));

//32 bit load
extern "C" VLOADVSTORE_FUNC_DECL _2i32 __vload2_l2i32( size_t, const int* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload2_l2i32));

extern "C" VLOADVSTORE_FUNC_DECL _4i32 __vload4_l4i32( size_t, const int* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload4_l4i32));

extern "C" VLOADVSTORE_FUNC_DECL _8i32 __vload8_l8i32( size_t, const int* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload8_l8i32));

extern "C" VLOADVSTORE_FUNC_DECL _16i32 __vload16_l16i32( size_t, const int* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload16_l16i32));

extern "C" VLOADVSTORE_FUNC_DECL _2u32 __vload2_l2u32( size_t, const unsigned int* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload2_l2u32));

extern "C" VLOADVSTORE_FUNC_DECL _4u32 __vload4_l4u32( size_t, const unsigned int* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload4_l4u32));

extern "C" VLOADVSTORE_FUNC_DECL _8u32 __vload8_l8u32( size_t, const unsigned int* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload8_l8u32));

extern "C" VLOADVSTORE_FUNC_DECL _16u32 __vload16_l16u32( size_t, const  unsigned int* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload16_l16u32));

//64 bit
extern "C" VLOADVSTORE_FUNC_DECL _2i64 __vload2_l2i64( size_t, const __int64* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x64_(__vload2_l2i64));

extern "C" VLOADVSTORE_FUNC_DECL _4i64 __vload4_l4i64( size_t, const __int64* ); 
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x64_(__vload4_l4i64));

extern "C" VLOADVSTORE_FUNC_DECL _8i64 __vload8_l8i64( size_t, const __int64* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x64_(__vload8_l8i64));

extern "C" VLOADVSTORE_FUNC_DECL void __vload16_l16i64(_16i64 *res, size_t, const __int64* );

extern "C" VLOADVSTORE_FUNC_DECL _2u64 __vload2_l2u64( size_t, const unsigned __int64* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x64_(__vload2_l2u64));

extern "C" VLOADVSTORE_FUNC_DECL _4u64 __vload4_l4u64( size_t, const unsigned __int64* ); 
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x64_(__vload4_l4u64));

extern "C" VLOADVSTORE_FUNC_DECL _8u64 __vload8_l8u64( size_t, const unsigned __int64* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x64_(__vload8_l8u64));

extern "C" VLOADVSTORE_FUNC_DECL void __vload16_l16u64(_16u64 *, size_t, const unsigned __int64* );


extern "C" VLOADVSTORE_FUNC_DECL float2 __vload2_lf2( size_t, const float* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload2_lf2));

extern "C" VLOADVSTORE_FUNC_DECL float4 __vload4_lf4( size_t, const float* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload4_lf4));

extern "C" VLOADVSTORE_FUNC_DECL float8 __vload8_lf8( size_t, const float* ); 
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload8_lf8));

extern "C" VLOADVSTORE_FUNC_DECL float16 __vload16_lf16( size_t, const float* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload16_lf16));


//8 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i8l( _2i8 data, size_t offset, char* ptr );
_//_pragma (use_linkage _lnk_in_2x8_mem_mem_(__vstore2_i8l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i8l( _4i8 data, size_t offset , char* ptr);
_//_pragma (use_linkage _lnk_in_4x8_mem_mem_(__vstore4_i8l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i8l( _8i8 data, size_t offset, char* ptr);
_//_pragma (use_linkage _lnk_in_8x8_mem_mem_(__vstore8_i8l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i8l( _16i8 data, size_t offset, char* ptr);
_//_pragma (use_linkage _lnk_in_16x8_mem_mem_(__vstore16_i8l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u8l( _2u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_2x8_mem_mem_(__vstore2_u8l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u8l( _4u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_4x8_mem_mem_(__vstore4_u8l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u8l( _8u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_8x8_mem_mem_(__vstore8_u8l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u8l( _16u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_16x8_mem_mem_(__vstore16_u8l));

//16 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i16l( _2i16 data, size_t offset, short* ptr);
_//_pragma (use_linkage _lnk_in_2x16_mem_mem_(__vstore2_i16l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i16l( _4i16 data, size_t offset, short* ptr);  
_//_pragma (use_linkage _lnk_in_4x16_mem_mem_(__vstore4_i16l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i16l( _8i16 data, size_t offset, short* ptr);  
_//_pragma (use_linkage _lnk_in_8x16_mem_mem_(__vstore8_i16l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i16l( _16i16 data, size_t offset, short* ptr);  
_//_pragma (use_linkage _lnk_in_16x16_mem_mem_(__vstore16_i16l));


extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u16l( _2u16 data, size_t offset, unsigned short* ptr);
_//_pragma (use_linkage _lnk_in_2x16_mem_mem_(__vstore2_u16l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u16l( _4u16 data, size_t offset, unsigned short* ptr);  
_//_pragma (use_linkage _lnk_in_4x16_mem_mem_(__vstore4_u16l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u16l( _8u16 data, size_t offset, unsigned short* ptr);  
_//_pragma (use_linkage _lnk_in_8x16_mem_mem_(__vstore8_u16l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u16l( _16u16 data, size_t offset, unsigned short* ptr);  
_//_pragma (use_linkage _lnk_in_16x16_mem_mem_(__vstore16_u16l));

//32 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i32l( _2i32, size_t, int* ); 
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore2_i32l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i32l( _4i32, size_t, int* );  
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore4_i32l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i32l( _8i32, size_t, int* );  
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore8_i32l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i32l( _16i32, size_t, int* );  
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore16_i32l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u32l( _2u32, size_t, unsigned int* ); 
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore2_u32l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u32l( _4u32, size_t, unsigned int* );  
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore4_u32l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u32l( _8u32, size_t, unsigned int* );  
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore8_u32l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u32l( _16u32, size_t, unsigned int* );  
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore16_u32l));

//64 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i64l( _2i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_2x64_mem_mem_(__vstore2_i64l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i64l( _4i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_4x64_mem_mem_(__vstore4_i64l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i64l( _8i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_8x64_mem_mem_(__vstore8_i64l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i64l( _16i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_16x64_mem_mem_(__vstore16_i64l));


extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u64l( _2u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_2x64_mem_mem_(__vstore2_u64l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u64l( _4u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_4x64_mem_mem_(__vstore4_u64l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u64l( _8u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_8x64_mem_mem_(__vstore8_u64l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u64l( _16u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_16x64_mem_mem_(__vstore16_u64l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_fl( float2, size_t, float* );  
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore2_fl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_fl( float4, size_t, float* ); 
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore4_fl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_fl( float8, size_t, float* );  
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore8_fl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_fl( float16, size_t, float* );  
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore16_fl));


//Global
//8 bit load
extern "C" VLOADVSTORE_FUNC_DECL _2i8p __vload2_g2i8( size_t offset, const char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x8_ ( __vload2_g2i8 ))

extern "C" VLOADVSTORE_FUNC_DECL _4i8p __vload4_g4i8( size_t offset, const char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x8_(__vload4_g4i8));

extern "C" VLOADVSTORE_FUNC_DECL _8i8p __vload8_g8i8( size_t offset, const char* ptr );
_//_pragma (use_linkage  _lnk_in_mem_mem_res_8x8_(__vload8_g8i8));

extern "C" VLOADVSTORE_FUNC_DECL _16i8 __vload16_g16i8( size_t offset, const char* ptr );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x8_(__vload16_g16i8));

extern "C" VLOADVSTORE_FUNC_DECL _2u8p __vload2_g2u8( size_t offset, const unsigned char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x8_(__vload2_g2u8));

extern "C" VLOADVSTORE_FUNC_DECL _4u8p __vload4_g4u8( size_t offset, const unsigned char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x8_(__vload4_g4u8));

extern "C" VLOADVSTORE_FUNC_DECL _8u8p __vload8_g8u8( size_t offset, const unsigned char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x8_(__vload8_g8u8));

extern "C" VLOADVSTORE_FUNC_DECL _16u8 __vload16_g16u8( size_t offset, const unsigned char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x8_(__vload16_g16u8));

//16 bit load
extern "C" VLOADVSTORE_FUNC_DECL _2i16p __vload2_g2i16( size_t, const short* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x16_(__vload2_g2i16));

extern "C" VLOADVSTORE_FUNC_DECL _4i16p __vload4_g4i16( size_t, const short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x16_(__vload4_g4i16));

extern "C" VLOADVSTORE_FUNC_DECL _8i16 __vload8_g8i16( size_t, const short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x16_(__vload8_g8i16));

extern "C" VLOADVSTORE_FUNC_DECL _16i16 __vload16_g16i16( size_t, const short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x16_(__vload16_g16i16));

extern "C" VLOADVSTORE_FUNC_DECL _2u16p __vload2_g2u16( size_t, const unsigned short* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x16_(__vload2_g2u16));

extern "C" VLOADVSTORE_FUNC_DECL _4u16p __vload4_g4u16( size_t, const unsigned short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x16_(__vload4_g4u16));

extern "C" VLOADVSTORE_FUNC_DECL _8u16 __vload8_g8u16( size_t, const unsigned short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x16_(__vload8_g8u16));

extern "C" VLOADVSTORE_FUNC_DECL _16u16 __vload16_g16u16( size_t, const unsigned short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x16_(__vload16_g16u16));

//32 bit load
extern "C" VLOADVSTORE_FUNC_DECL _2i32 __vload2_g2i32( size_t, const int* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload2_g2i32));

extern "C" VLOADVSTORE_FUNC_DECL _4i32 __vload4_g4i32( size_t, const int* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload4_g4i32));

extern "C" VLOADVSTORE_FUNC_DECL _8i32 __vload8_g8i32( size_t, const int* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload8_g8i32));

extern "C" VLOADVSTORE_FUNC_DECL _16i32 __vload16_g16i32( size_t, const int* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload16_g16i32));

extern "C" VLOADVSTORE_FUNC_DECL _2u32 __vload2_g2u32( size_t, const unsigned int* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload2_g2u32));

extern "C" VLOADVSTORE_FUNC_DECL _4u32 __vload4_g4u32( size_t, const unsigned int* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload4_g4u32));

extern "C" VLOADVSTORE_FUNC_DECL _8u32 __vload8_g8u32( size_t, const unsigned int* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload8_g8u32));

extern "C" VLOADVSTORE_FUNC_DECL _16u32 __vload16_g16u32( size_t, const unsigned int* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload16_g16u32));

//64 bit
extern "C" VLOADVSTORE_FUNC_DECL _2i64 __vload2_g2i64( size_t, const __int64* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x64_(__vload2_g2i64));

extern "C" VLOADVSTORE_FUNC_DECL _4i64 __vload4_g4i64( size_t, const __int64* ); 
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x64_(__vload4_g4i64));

extern "C" VLOADVSTORE_FUNC_DECL _8i64 __vload8_g8i64( size_t, const __int64* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x64_(__vload8_g8i64));

extern "C" VLOADVSTORE_FUNC_DECL  void __vload16_g16i64(_16i64 *res, size_t offset, const __int64* ptr);

extern "C" VLOADVSTORE_FUNC_DECL _2u64 __vload2_g2u64( size_t, const unsigned __int64* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x64_(__vload2_g2u64));

extern "C" VLOADVSTORE_FUNC_DECL _4u64 __vload4_g4u64( size_t, const unsigned __int64* ); 
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x64_(__vload4_g4u64));

extern "C" VLOADVSTORE_FUNC_DECL _8u64 __vload8_g8u64( size_t, const unsigned __int64* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x64_(__vload8_g8u64));

extern "C" VLOADVSTORE_FUNC_DECL void __vload16_g16u64(_16u64 *, size_t, const unsigned __int64* );


extern "C" VLOADVSTORE_FUNC_DECL float2 __vload2_gf2( size_t, const float* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload2_gf2));

extern "C" VLOADVSTORE_FUNC_DECL float4 __vload4_gf4( size_t, const float* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload4_gf4));

extern "C" VLOADVSTORE_FUNC_DECL float8 __vload8_gf8( size_t, const float* ); 
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload8_gf8));

extern "C" VLOADVSTORE_FUNC_DECL float16 __vload16_gf16( size_t, const float* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload16_gf16));


//8 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i8g( _2i8 data, size_t offset, char* ptr );
_//_pragma (use_linkage _lnk_in_2x8_mem_mem_(__vstore2_i8g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i8g( _4i8 data, size_t offset , char* ptr);
_//_pragma (use_linkage _lnk_in_4x8_mem_mem_(__vstore4_i8g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i8g( _8i8 data, size_t offset, char* ptr);
_//_pragma (use_linkage _lnk_in_8x8_mem_mem_(__vstore8_i8g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i8g( _16i8 data, size_t offset, char* ptr);
_//_pragma (use_linkage _lnk_in_16x8_mem_mem_(__vstore16_i8g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u8g( _2u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_2x8_mem_mem_(__vstore2_u8g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u8g( _4u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_4x8_mem_mem_(__vstore4_u8g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u8g( _8u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_8x8_mem_mem_(__vstore8_u8g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u8g( _16u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_16x8_mem_mem_(__vstore16_u8g));

//16 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i16g( _2i16 data, size_t offset, short* ptr);
_//_pragma (use_linkage _lnk_in_2x16_mem_mem_(__vstore2_i16g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i16g( _4i16 data, size_t offset, short* ptr);  
_//_pragma (use_linkage _lnk_in_4x16_mem_mem_(__vstore4_i16g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i16g( _8i16 data, size_t offset, short* ptr);  
_//_pragma (use_linkage _lnk_in_8x16_mem_mem_(__vstore8_i16g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i16g( _16i16 data, size_t offset, short* ptr);  
_//_pragma (use_linkage _lnk_in_16x16_mem_mem_(__vstore16_i16g));


extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u16g( _2u16 data, size_t offset, unsigned short* ptr);
_//_pragma (use_linkage _lnk_in_2x16_mem_mem_(__vstore2_u16g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u16g( _4u16 data, size_t offset, unsigned short* ptr);  
_//_pragma (use_linkage _lnk_in_4x16_mem_mem_(__vstore4_u16g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u16g( _8u16 data, size_t offset, unsigned short* ptr);  
_//_pragma (use_linkage _lnk_in_8x16_mem_mem_(__vstore8_u16g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u16g( _16u16 data, size_t offset, unsigned short* ptr);  
_//_pragma (use_linkage _lnk_in_16x16_mem_mem_(__vstore16_u16g));

//32 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i32g( _2i32, size_t, int* ); 
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore2_i32g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i32g( _4i32, size_t, int* );  
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore4_i32g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i32g( _8i32, size_t, int* );  
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore8_i32g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i32g( _16i32, size_t, int* );  
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore16_i32g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u32g( _2u32, size_t, unsigned int* ); 
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore2_u32g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u32g( _4u32, size_t, unsigned int* );  
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore4_u32g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u32g( _8u32, size_t, unsigned int* );  
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore8_u32g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u32g( _16u32, size_t, unsigned int* );  
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore16_u32g));

//64 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i64g( _2i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_2x64_mem_mem_(__vstore2_i64g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i64g( _4i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_4x64_mem_mem_(__vstore4_i64g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i64g( _8i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_8x64_mem_mem_(__vstore8_i64g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i64g( _16i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_16x64_mem_mem_(__vstore16_i64g));


extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u64g( _2u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_2x64_mem_mem_(__vstore2_u64g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u64g( _4u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_4x64_mem_mem_(__vstore4_u64g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u64g( _8u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_8x64_mem_mem_(__vstore8_u64g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u64g( _16u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_16x64_mem_mem_(__vstore16_u64g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_fg( float2, size_t, float* );  
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore2_fg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_fg( float4, size_t, float* ); 
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore4_fg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_fg( float8, size_t, float* );  
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore8_fg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_fg( float16, size_t, float* );  
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore16_fg));

//Constant
//8 bit load
extern "C" VLOADVSTORE_FUNC_DECL _2i8p __vload2_c2i8( size_t offset, const char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x8_ ( __vload2_c2i8 ))

extern "C" VLOADVSTORE_FUNC_DECL _4i8p __vload4_c4i8( size_t offset, const char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x8_(__vload4_c4i8));

extern "C" VLOADVSTORE_FUNC_DECL _8i8p __vload8_c8i8( size_t offset, const char* ptr );
_//_pragma (use_linkage  _lnk_in_mem_mem_res_8x8_(__vload8_c8i8));

extern "C" VLOADVSTORE_FUNC_DECL _16i8 __vload16_c16i8( size_t offset, const char* ptr );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x8_(__vload16_c16i8));

extern "C" VLOADVSTORE_FUNC_DECL _2u8p __vload2_c2u8( size_t offset, const unsigned char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x8_(__vload2_c2u8));

extern "C" VLOADVSTORE_FUNC_DECL _4u8p __vload4_c4u8( size_t offset, const unsigned char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x8_(__vload4_c4u8));

extern "C" VLOADVSTORE_FUNC_DECL _8u8p __vload8_c8u8( size_t offset, const unsigned char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x8_(__vload8_c8u8));

extern "C" VLOADVSTORE_FUNC_DECL _16u8 __vload16_c16u8( size_t offset, const unsigned char* ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x8_(__vload16_c16u8));

//16 bit load
extern "C" VLOADVSTORE_FUNC_DECL _2i16p __vload2_c2i16( size_t, const short* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x16_(__vload2_c2i16));

extern "C" VLOADVSTORE_FUNC_DECL _4i16p __vload4_c4i16( size_t, const short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x16_(__vload4_c4i16));

extern "C" VLOADVSTORE_FUNC_DECL _8i16 __vload8_c8i16( size_t, const short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x16_(__vload8_c8i16));

extern "C" VLOADVSTORE_FUNC_DECL _16i16 __vload16_c16i16( size_t, const short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x16_(__vload16_c16i16));

extern "C" VLOADVSTORE_FUNC_DECL _2u16p __vload2_c2u16( size_t, const unsigned short* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x16_(__vload2_c2u16));

extern "C" VLOADVSTORE_FUNC_DECL _4u16p __vload4_c4u16( size_t, const unsigned short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x16_(__vload4_c4u16));

extern "C" VLOADVSTORE_FUNC_DECL _8u16 __vload8_c8u16( size_t, const unsigned short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x16_(__vload8_c8u16));

extern "C" VLOADVSTORE_FUNC_DECL _16u16 __vload16_c16u16( size_t, const unsigned short* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x16_(__vload16_c16u16));

//32 bit load
extern "C" VLOADVSTORE_FUNC_DECL _2i32 __vload2_c2i32( size_t, const int* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload2_c2i32));

extern "C" VLOADVSTORE_FUNC_DECL _4i32 __vload4_c4i32( size_t, const int* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload4_c4i32));

extern "C" VLOADVSTORE_FUNC_DECL _8i32 __vload8_c8i32( size_t, const int* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload8_c8i32));

extern "C" VLOADVSTORE_FUNC_DECL _16i32 __vload16_c16i32( size_t, const int* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload16_c16i32));

extern "C" VLOADVSTORE_FUNC_DECL _2u32 __vload2_c2u32( size_t, const unsigned int* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload2_c2u32));

extern "C" VLOADVSTORE_FUNC_DECL _4u32 __vload4_c4u32( size_t, const unsigned int* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload4_c4u32));

extern "C" VLOADVSTORE_FUNC_DECL _8u32 __vload8_c8u32( size_t, const unsigned int* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload8_c8u32));

extern "C" VLOADVSTORE_FUNC_DECL _16u32 __vload16_c16u32( size_t, const unsigned int* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload16_c16u32));

//64 bit
extern "C" VLOADVSTORE_FUNC_DECL _2i64 __vload2_c2i64( size_t, const __int64* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x64_(__vload2_c2i64));

extern "C" VLOADVSTORE_FUNC_DECL _4i64 __vload4_c4i64( size_t, const __int64* ); 
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x64_(__vload4_c4i64));

extern "C" VLOADVSTORE_FUNC_DECL _8i64 __vload8_c8i64( size_t, const __int64* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x64_(__vload8_c8i64));

extern "C" VLOADVSTORE_FUNC_DECL void __vload16_c16i64(_16i64 *, size_t, const __int64* );

extern "C" VLOADVSTORE_FUNC_DECL _2u64 __vload2_c2u64( size_t, const unsigned __int64* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x64_(__vload2_c2u64));

extern "C" VLOADVSTORE_FUNC_DECL _4u64 __vload4_c4u64( size_t, const unsigned __int64* ); 
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x64_(__vload4_c4u64));

extern "C" VLOADVSTORE_FUNC_DECL _8u64 __vload8_c8u64( size_t, const unsigned __int64* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x64_(__vload8_c8u64));

extern "C" VLOADVSTORE_FUNC_DECL void __vload16_c16u64(_16u64 *, size_t, const unsigned __int64* );


extern "C" VLOADVSTORE_FUNC_DECL float2 __vload2_cf2( size_t, const float* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload2_cf2));

extern "C" VLOADVSTORE_FUNC_DECL float4 __vload4_cf4( size_t, const float* );  
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload4_cf4));

extern "C" VLOADVSTORE_FUNC_DECL float8 __vload8_cf8( size_t, const float* ); 
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload8_cf8));

extern "C" VLOADVSTORE_FUNC_DECL float16 __vload16_cf16( size_t, const float* );
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload16_cf16));


//8 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i8c( _2i8 data, size_t offset, char* ptr );
_//_pragma (use_linkage _lnk_in_2x8_mem_mem_(__vstore2_i8c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i8c( _4i8 data, size_t offset , char* ptr);
_//_pragma (use_linkage _lnk_in_4x8_mem_mem_(__vstore4_i8c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i8c( _8i8 data, size_t offset, char* ptr);
_//_pragma (use_linkage _lnk_in_8x8_mem_mem_(__vstore8_i8c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i8c( _16i8 data, size_t offset, char* ptr);
_//_pragma (use_linkage _lnk_in_16x8_mem_mem_(__vstore16_i8c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u8c( _2u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_2x8_mem_mem_(__vstore2_u8c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u8c( _4u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_4x8_mem_mem_(__vstore4_u8c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u8c( _8u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_8x8_mem_mem_(__vstore8_u8c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u8c( _16u8, size_t, unsigned char* );
_//_pragma (use_linkage _lnk_in_16x8_mem_mem_(__vstore16_u8c));

//16 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i16c( _2i16 data, size_t offset, short* ptr);
_//_pragma (use_linkage _lnk_in_2x16_mem_mem_(__vstore2_i16c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i16c( _4i16 data, size_t offset, short* ptr);  
_//_pragma (use_linkage _lnk_in_4x16_mem_mem_(__vstore4_i16c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i16c( _8i16 data, size_t offset, short* ptr);  
_//_pragma (use_linkage _lnk_in_8x16_mem_mem_(__vstore8_i16c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i16c( _16i16 data, size_t offset, short* ptr);  
_//_pragma (use_linkage _lnk_in_16x16_mem_mem_(__vstore16_i16c));


extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u16c( _2u16 data, size_t offset, unsigned short* ptr);
_//_pragma (use_linkage _lnk_in_2x16_mem_mem_(__vstore2_u16c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u16c( _4u16 data, size_t offset, unsigned short* ptr);  
_//_pragma (use_linkage _lnk_in_4x16_mem_mem_(__vstore4_u16c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u16c( _8u16 data, size_t offset, unsigned short* ptr);  
_//_pragma (use_linkage _lnk_in_8x16_mem_mem_(__vstore8_u16c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u16c( _16u16 data, size_t offset, unsigned short* ptr);  
_//_pragma (use_linkage _lnk_in_16x16_mem_mem_(__vstore16_u16c));

//32 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i32c( _2i32, size_t, int* ); 
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore2_i32c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i32c( _4i32, size_t, int* );  
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore4_i32c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i32c( _8i32, size_t, int* );  
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore8_i32c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i32c( _16i32, size_t, int* );  
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore16_i32c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u32c( _2u32, size_t, unsigned int* ); 
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore2_u32c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u32c( _4u32, size_t, unsigned int* );  
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore4_u32c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u32c( _8u32, size_t, unsigned int* );  
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore8_u32c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u32c( _16u32, size_t, unsigned int* );  
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore16_u32c));

//64 bit store
extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_i64c( _2i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_2x64_mem_mem_(__vstore2_i64c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_i64c( _4i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_4x64_mem_mem_(__vstore4_i64c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_i64c( _8i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_8x64_mem_mem_(__vstore8_i64c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_i64c( _16i64, size_t, __int64* );
_//_pragma (use_linkage _lnk_in_16x64_mem_mem_(__vstore16_i64c));


extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_u64c( _2u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_2x64_mem_mem_(__vstore2_u64c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_u64c( _4u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_4x64_mem_mem_(__vstore4_u64c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_u64c( _8u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_8x64_mem_mem_(__vstore8_u64c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_u64c( _16u64, size_t, unsigned __int64* );
_//_pragma (use_linkage _lnk_in_16x64_mem_mem_(__vstore16_u64c));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore2_fc( float2, size_t, float* );  
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore2_fc));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore4_fc( float4, size_t, float* ); 
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore4_fc));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore8_fc( float8, size_t, float* );  
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore8_fc));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore16_fc( float16, size_t, float* );  
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore16_fc));

extern "C" VLOADVSTORE_FUNC_DECL float __vload_half(size_t, const PHALF_PTR);
_//_pragma (use_linkage _lnk_in_mem_mem_res_f(__vload_half));

extern "C" VLOADVSTORE_FUNC_DECL float __vload_halfg(size_t, const PHALF_PTR);  
_//_pragma (use_linkage _lnk_in_mem_mem_res_f(__vload_halfg));

extern "C" VLOADVSTORE_FUNC_DECL float __vload_halfc(size_t, const PHALF_PTR);  
_//_pragma (use_linkage _lnk_in_mem_mem_res_f(__vload_halfc));

extern "C" VLOADVSTORE_FUNC_DECL float __vload_halfl(size_t, const PHALF_PTR);
_//_pragma (use_linkage _lnk_in_mem_mem_res_f(__vload_halfl));

extern "C" VLOADVSTORE_FUNC_DECL float2 __vload_half2(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload_half2));

extern "C" VLOADVSTORE_FUNC_DECL float2 __vload_half2g(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload_half2g));

extern "C" VLOADVSTORE_FUNC_DECL float2 __vload_half2c(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload_half2c));

extern "C" VLOADVSTORE_FUNC_DECL float2 __vload_half2l(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vload_half2l));

extern "C" VLOADVSTORE_FUNC_DECL float4 __vload_half4(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload_half4));

extern "C" VLOADVSTORE_FUNC_DECL float4 __vload_half4g(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload_half4g));

extern "C" VLOADVSTORE_FUNC_DECL float4 __vload_half4c(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload_half4c));

extern "C" VLOADVSTORE_FUNC_DECL float4 __vload_half4l(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vload_half4l));

extern "C" VLOADVSTORE_FUNC_DECL float8 __vload_half8(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload_half8));

extern "C" VLOADVSTORE_FUNC_DECL float8 __vload_half8g(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload_half8g));

extern "C" VLOADVSTORE_FUNC_DECL float8 __vload_half8c(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload_half8c));

extern "C" VLOADVSTORE_FUNC_DECL float8 __vload_half8l(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vload_half8l));

extern "C" VLOADVSTORE_FUNC_DECL float16 __vload_half16(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload_half16));

extern "C" VLOADVSTORE_FUNC_DECL float16 __vload_half16g(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload_half16g));

extern "C" VLOADVSTORE_FUNC_DECL float16 __vload_half16c(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload_half16c));

extern "C" VLOADVSTORE_FUNC_DECL float16 __vload_half16l(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vload_half16l));


extern "C" VLOADVSTORE_FUNC_DECL float2 __vloada_half2(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vloada_half2));

extern "C" VLOADVSTORE_FUNC_DECL float2 __vloada_half2g(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vloada_half2g));

extern "C" VLOADVSTORE_FUNC_DECL float2 __vloada_half2c(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vloada_half2c));

extern "C" VLOADVSTORE_FUNC_DECL float2 __vloada_half2l(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_2x32_(__vloada_half2l));

extern "C" VLOADVSTORE_FUNC_DECL float4 __vloada_half4(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vloada_half4));

extern "C" VLOADVSTORE_FUNC_DECL float4 __vloada_half4g(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vloada_half4g));

extern "C" VLOADVSTORE_FUNC_DECL float4 __vloada_half4c(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vloada_half4c));

extern "C" VLOADVSTORE_FUNC_DECL float4 __vloada_half4l(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_4x32_(__vloada_half4l));

extern "C" VLOADVSTORE_FUNC_DECL float8 __vloada_half8(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vloada_half8));

extern "C" VLOADVSTORE_FUNC_DECL float8 __vloada_half8g(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vloada_half8g));

extern "C" VLOADVSTORE_FUNC_DECL float8 __vloada_half8c(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vloada_half8c));

extern "C" VLOADVSTORE_FUNC_DECL float8 __vloada_half8l(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_8x32_(__vloada_half8l));

extern "C" VLOADVSTORE_FUNC_DECL float16 __vloada_half16(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vloada_half16));

extern "C" VLOADVSTORE_FUNC_DECL float16 __vloada_half16g(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vloada_half16g));

extern "C" VLOADVSTORE_FUNC_DECL float16 __vloada_half16c(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vloada_half16c));

extern "C" VLOADVSTORE_FUNC_DECL float16 __vloada_half16l(size_t offset, const PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_mem_mem_res_16x32_(__vloada_half16l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half(float data, size_t offset, PHALF_PTR ptr);
extern "C" VLOADVSTORE_FUNC_DECL void __vstore_halfg(float data, size_t offset, PHALF_PTR ptr);
extern "C" VLOADVSTORE_FUNC_DECL void __vstore_halfl(float data, size_t offset, PHALF_PTR ptr);
extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half_rte(float data, size_t offset, PHALF_PTR ptr);
extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half_rteg(float data, size_t offset, PHALF_PTR ptr);
extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half_rtel(float data, size_t offset, PHALF_PTR ptr);
extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half_rtz(float data, size_t offset, PHALF_PTR ptr);
extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half_rtzg(float data, size_t offset, PHALF_PTR ptr);
extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half_rtzl(float data, size_t offset, PHALF_PTR ptr);
extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half_rtp(float data, size_t offset, PHALF_PTR ptr);
extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half_rtpg(float data, size_t offset, PHALF_PTR ptr);
extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half_rtpl(float data, size_t offset, PHALF_PTR ptr);
extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half_rtn(float data, size_t offset, PHALF_PTR ptr);
extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half_rtng(float data, size_t offset, PHALF_PTR ptr);
extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half_rtnl(float data, size_t offset, PHALF_PTR ptr);

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half2(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore_half2));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half2g(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore_half2g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half2l(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore_half2l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half2_rte(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore_half2_rte));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half2_rteg(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore_half2_rteg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half2_rtel(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore_half2_rtel));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half2_rtz(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore_half2_rtz));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half2_rtzg(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore_half2_rtzg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half2_rtzl(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore_half2_rtzl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half2_rtp(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore_half2_rtp));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half2_rtpg(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore_half2_rtpg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half2_rtpl(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore_half2_rtpl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half2_rtn(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore_half2_rtn));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half2_rtng(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore_half2_rtng));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half2_rtnl(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstore_half2_rtnl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half4(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore_half4));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half4g(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore_half4g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half4l(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore_half4l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half4_rte(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore_half4_rte));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half4_rteg(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore_half4_rteg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half4_rtel(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore_half4_rtel));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half4_rtz(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore_half4_rtz));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half4_rtzg(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore_half4_rtzg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half4_rtzl(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore_half4_rtzl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half4_rtp(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore_half4_rtp));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half4_rtpg(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore_half4_rtpg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half4_rtpl(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore_half4_rtpl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half4_rtn(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore_half4_rtn));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half4_rtng(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore_half4_rtng));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half4_rtnl(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstore_half4_rtnl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half8(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore_half8));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half8g(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore_half8g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half8l(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore_half8l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half8_rte(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore_half8_rte));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half8_rteg(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore_half8_rteg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half8_rtel(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore_half8_rtel));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half8_rtz(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore_half8_rtz));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half8_rtzg(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore_half8_rtzg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half8_rtzl(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore_half8_rtzl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half8_rtp(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore_half8_rtp));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half8_rtpg(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore_half8_rtpg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half8_rtpl(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore_half8_rtpl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half8_rtn(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore_half8_rtn));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half8_rtng(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore_half8_rtng));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half8_rtnl(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstore_half8_rtnl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half16(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore_half16));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half16g(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore_half16g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half16l(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore_half16l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half16_rte(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore_half16_rte));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half16_rteg(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore_half16_rteg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half16_rtel(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore_half16_rtel));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half16_rtz(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore_half16_rtz));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half16_rtzg(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore_half16_rtzg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half16_rtzl(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore_half16_rtzl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half16_rtp(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore_half16_rtp));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half16_rtpg(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore_half16_rtpg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half16_rtpl(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore_half16_rtpl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half16_rtn(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore_half16_rtn));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half16_rtng(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore_half16_rtng));

extern "C" VLOADVSTORE_FUNC_DECL void __vstore_half16_rtnl(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstore_half16_rtnl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half2(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstorea_half2));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half2g(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstorea_half2g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half2l(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstorea_half2l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half2_rte(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstorea_half2_rte));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half2_rteg(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstorea_half2_rteg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half2_rtel(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstorea_half2_rtel));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half2_rtz(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstorea_half2_rtz));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half2_rtzg(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstorea_half2_rtzg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half2_rtzl(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstorea_half2_rtzl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half2_rtp(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstorea_half2_rtp));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half2_rtpg(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstorea_half2_rtpg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half2_rtpl(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstorea_half2_rtpl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half2_rtn(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstorea_half2_rtn));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half2_rtng(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstorea_half2_rtng));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half2_rtnl(float2 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_2x32_mem_mem_(__vstorea_half2_rtnl));


extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half4(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstorea_half4));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half4g(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstorea_half4g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half4l(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstorea_half4l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half4_rte(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstorea_half4_rte));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half4_rteg(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstorea_half4_rteg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half4_rtel(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstorea_half4_rtel));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half4_rtz(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstorea_half4_rtz));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half4_rtzg(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstorea_half4_rtzg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half4_rtzl(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstorea_half4_rtzl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half4_rtp(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstorea_half4_rtp));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half4_rtpg(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstorea_half4_rtpg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half4_rtpl(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstorea_half4_rtpl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half4_rtn(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstorea_half4_rtn));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half4_rtng(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstorea_half4_rtng));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half4_rtnl(float4 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_4x32_mem_mem_(__vstorea_half4_rtnl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half8(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstorea_half8));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half8g(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstorea_half8g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half8l(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstorea_half8l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half8_rte(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstorea_half8_rte));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half8_rteg(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstorea_half8_rteg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half8_rtel(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstorea_half8_rtel));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half8_rtz(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstorea_half8_rtz));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half8_rtzg(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstorea_half8_rtzg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half8_rtzl(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstorea_half8_rtzl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half8_rtp(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstorea_half8_rtp));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half8_rtpg(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstorea_half8_rtpg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half8_rtpl(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstorea_half8_rtpl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half8_rtn(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstorea_half8_rtn));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half8_rtng(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstorea_half8_rtng));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half8_rtnl(float8 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_8x32_mem_mem_(__vstorea_half8_rtnl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half16(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstorea_half16));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half16g(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstorea_half16g));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half16l(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstorea_half16l));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half16_rte(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstorea_half16_rte));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half16_rteg(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstorea_half16_rteg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half16_rtel(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstorea_half16_rtel));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half16_rtz(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstorea_half16_rtz));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half16_rtzg(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstorea_half16_rtzg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half16_rtzl(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstorea_half16_rtzl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half16_rtp(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstorea_half16_rtp));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half16_rtpg(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstorea_half16_rtpg));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half16_rtpl(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstorea_half16_rtpl));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half16_rtn(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstorea_half16_rtn));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half16_rtng(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstorea_half16_rtng));

extern "C" VLOADVSTORE_FUNC_DECL void __vstorea_half16_rtnl(float16 data, size_t offset, PHALF_PTR ptr);
_//_pragma (use_linkage _lnk_in_16x32_mem_mem_(__vstorea_half16_rtnl));

#endif



