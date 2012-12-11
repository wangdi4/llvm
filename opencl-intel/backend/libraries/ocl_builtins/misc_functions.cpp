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

///////////////////////////////////////////////////////////
//  relational_functions.cpp
///////////////////////////////////////////////////////////

#if defined (__MIC__) || defined(__MIC2__)
//MIC implementation
#else
#ifdef __cplusplus
extern "C" {
#endif

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#define ALIGN16 __attribute__((aligned(16)))
#include <intrin.h>

#include "cl_types2.h"

void* memcpy(void*, const void*, size_t);

//shuffle masks
constant char16 shuffle_char2_mask =	(char16)(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
constant char16 shuffle_char4_mask =	(char16)(3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3);
constant char16 shuffle_char8_mask =	(char16)(7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7);
constant char16 shuffle_char16_mask =	(char16)(15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15);

constant short8 shuffle_short2_mask =	(short8)(1, 1, 1, 1, 1, 1, 1, 1);
constant short8 shuffle_short4_mask =	(short8)(3, 3, 3, 3, 3, 3, 3, 3);
constant short8 shuffle_short8_mask =	(short8)(7, 7, 7, 7, 7, 7, 7, 7);
constant short8 shuffle_short16_mask =	(short8)(15, 15, 15, 15, 15, 15, 15, 15);
constant short8 shuffle_short32_mask =	(short8)(31, 31, 31, 31, 31, 31, 31, 31);

constant int4 shuffle_int2_mask =	(int4)(1, 1, 1, 1);
constant int4 shuffle_int4_mask =	(int4)(3, 3, 3, 3);
constant int4 shuffle_int8_mask =	(int4)(7, 7, 7, 7);
constant int4 shuffle_int16_mask =	(int4)(15, 15, 15, 15);
constant int4 shuffle_int32_mask =	(int4)(31, 31, 31, 31);

constant long2 shuffle_long2_mask =	(long2)(1, 1);
constant long2 shuffle_long4_mask =	(long2)(3, 3);
constant long2 shuffle_long8_mask =	(long2)(7, 7);
constant long2 shuffle_long16_mask =	(long2)(15, 15);

//shuffle2 masks
constant char16 shuffle2_char2_mask =	(char16)(0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81);
constant char16 shuffle2_char4_mask =	(char16)(0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83);
constant char16 shuffle2_char8_mask =	(char16)(0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87);
constant char16 shuffle2_char16_mask =	(char16)(0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F);

constant short16 shuffle2_short2_mask =	(short16)(0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41);
constant short16 shuffle2_short4_mask =	(short16)(0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43);
constant short16 shuffle2_short8_mask =	(short16)(0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47);
constant short16 shuffle2_short16_mask =(short16)(0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F);
constant short16 shuffle2_short32_mask =(short16)(0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F);


constant char16 shuffle_epi16_smask = (char16)(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
constant char16 shuffle_epi16_amask = (char16)(0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1);

constant char16 shuffle_epi32_smask = (char16)(0, 0, 0, 0, 4, 4, 4, 4, 8, 8, 8, 8, 12, 12, 12, 12);
constant char16 shuffle_epi32_amask = (char16)(0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3);

constant char16 shuffle_epi64_smask = (char16)(0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8);
constant char16 shuffle_epi64_amask = (char16)(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);



#ifdef __SSSE3__
	#define SHUFFLE_EPI8(x, mask)\
		_mm_shuffle_epi8(x, mask)
#else
	__m128i shuffle_epi8(__m128i x, __m128i mask)
	{
		ALIGN16 _1i8 tempX[16];											
		ALIGN16 _1i8 tampMask[16];										
		ALIGN16 _1i8 result[16];										
																		
		_mm_store_si128((__m128i *)tempX, x);							
		_mm_store_si128((__m128i *)tampMask, mask);					
		for(int i=0; i<16; ++i)											
		{																
			_1i8 index = tampMask[i] & 15;								
			result[i] = (tampMask[i] < 0) ? 0 : tempX[index];			
		}																
		return _mm_load_si128((const __m128i *)result);					
	}																	
																		
	#define SHUFFLE_EPI8(x, mask)\
		shuffle_epi8(x, mask)											
#endif

__m128i SHUFFLE_EPI16(__m128i x, __m128i mask)
{
	mask = SHUFFLE_EPI8(mask, __builtin_astype(shuffle_epi16_smask, __m128i));
	mask = _mm_slli_epi16(mask, 1);
	mask = _mm_adds_epu8(mask, __builtin_astype(shuffle_epi16_amask, __m128i));
	return SHUFFLE_EPI8(x, mask);		
}

__m128i SHUFFLE_EPI32(__m128i x, __m128i mask)
{
	mask = SHUFFLE_EPI8(mask, __builtin_astype(shuffle_epi32_smask, __m128i));
	mask = _mm_slli_epi16(mask, 2);
	mask = _mm_adds_epu8(mask, __builtin_astype(shuffle_epi32_amask, __m128i));
	return SHUFFLE_EPI8(x, mask);		
}
__m128i SHUFFLE_EPI64(__m128i x, __m128i mask)
{
	mask = SHUFFLE_EPI8(mask, __builtin_astype(shuffle_epi64_smask, __m128i));
	mask = _mm_slli_epi16(mask, 3);
	mask = _mm_adds_epu8(mask, __builtin_astype(shuffle_epi64_amask, __m128i));
	return SHUFFLE_EPI8(x, mask);		
}

_2i8 __attribute__((overloadable)) shuffle(_2i8 x, _2u8 mask)
{
	_16u8 tMask;
	tMask.s01 = mask;
	return shuffle(x, tMask).s01;
}

_4i8 __attribute__((overloadable)) shuffle(_2i8 x, _4u8 mask)
{
	_16u8 tMask;
	tMask.s0123 = mask;
	return shuffle(x, tMask).s0123;
}

_8i8 __attribute__((overloadable)) shuffle(_2i8 x, _8u8 mask)
{
	_16u8 tMask;
	tMask.s01234567 = mask;
	return shuffle(x, tMask).s01234567;
}

_16i8 __attribute__((overloadable)) shuffle(_2i8 x, _16u8 mask)
{
	_16i8 tX;
	tX.s01 = x;
	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask, __m128i), 
             __builtin_astype(shuffle_char2_mask, __m128i)),
           _16u8);
	tX = __builtin_astype(
         SHUFFLE_EPI8(
           __builtin_astype(tX, __m128i),
           __builtin_astype(mask, __m128i)),
         _16i8);
	return tX;
}

_2i8 __attribute__((overloadable)) shuffle(_4i8 x, _2u8 mask)
{
	_16u8 tMask;
	tMask.s01 = mask;
	return shuffle(x, tMask).s01;
}

_4i8 __attribute__((overloadable)) shuffle(_4i8 x, _4u8 mask)
{
	_16u8 tMask;
	tMask.s0123 = mask;
	return shuffle(x, tMask).s0123;
}

_8i8 __attribute__((overloadable)) shuffle(_4i8 x, _8u8 mask)
{
	_16u8 tMask;
	tMask.s01234567 = mask;
	return shuffle(x, tMask).s01234567;
}

_16i8 __attribute__((overloadable)) shuffle(_4i8 x, _16u8 mask)
{
	_16i8 tX;
	tX.s0123 = x;
	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask, __m128i),
             __builtin_astype(shuffle_char4_mask, __m128i)),
           _16u8);
	tX = __builtin_astype(
         SHUFFLE_EPI8(
           __builtin_astype(tX, __m128i),
           __builtin_astype(mask, __m128i)),
         _16i8);
	return tX;
}

_2i8 __attribute__((overloadable)) shuffle(_8i8 x, _2u8 mask)
{
	_16u8 tMask;
	tMask.s01 = mask;
	return shuffle(x, tMask).s01;
}

_4i8 __attribute__((overloadable)) shuffle(_8i8 x, _4u8 mask)
{
	_16u8 tMask;
	tMask.s0123 = mask;
	return shuffle(x, tMask).s0123;
}

_8i8 __attribute__((overloadable)) shuffle(_8i8 x, _8u8 mask)
{
	_16u8 tMask;
	tMask.s01234567 = mask;
	return shuffle(x, tMask).s01234567;
}

_16i8 __attribute__((overloadable)) shuffle(_8i8 x, _16u8 mask)
{
	_16i8 tX;
	tX.s01234567 = x;
	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask, __m128i),
             __builtin_astype(shuffle_char8_mask, __m128i)),
           _16u8);
	tX = __builtin_astype(
         SHUFFLE_EPI8(__builtin_astype(tX, __m128i),
         __builtin_astype(mask, __m128i)),
       _16i8);
	return tX;
}

_2i8 __attribute__((overloadable)) shuffle(_16i8 x, _2u8 mask)
{
	_16u8 tMask;
	tMask.s01 = mask;
	return shuffle(x, tMask).s01;
}

_4i8 __attribute__((overloadable)) shuffle(_16i8 x, _4u8 mask)
{
	_16u8 tMask;
	tMask.s0123 = mask;
	return shuffle(x, tMask).s0123;
}

_8i8 __attribute__((overloadable)) shuffle(_16i8 x, _8u8 mask)
{
	_16u8 tMask;
	tMask.s01234567 = mask;
	return shuffle(x, tMask).s01234567;
}

_16i8 __attribute__((overloadable)) shuffle(_16i8 x, _16u8 mask)
{
	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask, __m128i),
             __builtin_astype(shuffle_char16_mask, __m128i)),
           _16u8);
	x = __builtin_astype(
        SHUFFLE_EPI8(
          __builtin_astype(x, __m128i),
          __builtin_astype(mask, __m128i)),
        _16i8);
	return x;
}

_2u8 __attribute__((overloadable)) shuffle(_2u8 x, _2u8 mask)
{
	return as_uchar2(shuffle(as_char2(x), mask));
}

_4u8 __attribute__((overloadable)) shuffle(_2u8 x, _4u8 mask)
{
	return as_uchar4(shuffle(as_char2(x), mask));
}

_8u8 __attribute__((overloadable)) shuffle(_2u8 x, _8u8 mask)
{
	return as_uchar8(shuffle(as_char2(x), mask));
}

_16u8 __attribute__((overloadable)) shuffle(_2u8 x, _16u8 mask)
{
	return as_uchar16(shuffle(as_char2(x), mask));
}

_2u8 __attribute__((overloadable)) shuffle(_4u8 x, _2u8 mask)
{
	return as_uchar2(shuffle(as_char4(x), mask));
}

_4u8 __attribute__((overloadable)) shuffle(_4u8 x, _4u8 mask)
{
	return as_uchar4(shuffle(as_char4(x), mask));
}

_8u8 __attribute__((overloadable)) shuffle(_4u8 x, _8u8 mask)
{
	return as_uchar8(shuffle(as_char4(x), mask));
}

_16u8 __attribute__((overloadable)) shuffle(_4u8 x, _16u8 mask)
{
	return as_uchar16(shuffle(as_char4(x), mask));
}

_2u8 __attribute__((overloadable)) shuffle(_8u8 x, _2u8 mask)
{
	return as_uchar2(shuffle(as_char8(x), mask));
}

_4u8 __attribute__((overloadable)) shuffle(_8u8 x, _4u8 mask)
{
	return as_uchar4(shuffle(as_char8(x), mask));
}

_8u8 __attribute__((overloadable)) shuffle(_8u8 x, _8u8 mask)
{
	return as_uchar8(shuffle(as_char8(x), mask));
}

_16u8 __attribute__((overloadable)) shuffle(_8u8 x, _16u8 mask)
{
	return as_uchar16(shuffle(as_char8(x), mask));
}

_2u8 __attribute__((overloadable)) shuffle(_16u8 x, _2u8 mask)
{
	return as_uchar2(shuffle(as_char16(x), mask));
}

_4u8 __attribute__((overloadable)) shuffle(_16u8 x, _4u8 mask)
{
	return as_uchar4(shuffle(as_char16(x), mask));
}

_8u8 __attribute__((overloadable)) shuffle(_16u8 x, _8u8 mask)
{
	return as_uchar8(shuffle(as_char16(x), mask));
}

_16u8 __attribute__((overloadable)) shuffle(_16u8 x, _16u8 mask)
{
	return as_uchar16(shuffle(as_char16(x), mask));
}

_2i16 __attribute__((overloadable)) shuffle(_2i16 x, _2u16 mask)
{
	_8u16 tMask;
	tMask.s01 = mask;
	return shuffle(x, tMask).s01;
}

_4i16 __attribute__((overloadable)) shuffle(_2i16 x, _4u16 mask)
{
	_8u16 tMask;
	tMask.s0123 = mask;
	return shuffle(x, tMask).s0123;
}

_8i16 __attribute__((overloadable)) shuffle(_2i16 x, _8u16 mask)
{
	_8i16 tX;
	tX.s01 = x;
	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask, __m128i),
             __builtin_astype(shuffle_short2_mask, __m128i)),
           _8u16);
	tX = __builtin_astype(
         SHUFFLE_EPI16(
           __builtin_astype(tX, __m128i),
           __builtin_astype(mask, __m128i)),
         _8i16);
	return tX;
}

_16i16 __attribute__((overloadable)) shuffle(_2i16 x, _16u16 mask)
{
	_16i16 res;

	res.lo = shuffle(x, mask.lo);
	res.hi = shuffle(x, mask.hi);

	return res;
}

_2i16 __attribute__((overloadable)) shuffle(_4i16 x, _2u16 mask)
{
	_8u16 tMask;
	tMask.s01 = mask;
	return shuffle(x, tMask).s01;
}

_4i16 __attribute__((overloadable)) shuffle(_4i16 x, _4u16 mask)
{
	_8u16 tMask;
	tMask.s0123 = mask;
	return shuffle(x, tMask).s0123;
}

_8i16 __attribute__((overloadable)) shuffle(_4i16 x, _8u16 mask)
{
	_8i16 tX;
	tX.s0123 = x;
	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask, __m128i),
             __builtin_astype(shuffle_short4_mask, __m128i)),
           _8u16);
	tX = __builtin_astype(
         SHUFFLE_EPI16(
           __builtin_astype(tX, __m128i),
           __builtin_astype(mask, __m128i)),
         _8i16);
	return tX;
}

_16i16 __attribute__((overloadable)) shuffle(_4i16 x, _16u16 mask)
{
	_16i16 res;

	res.lo = shuffle(x, mask.lo);
	res.hi = shuffle(x, mask.hi);

	return res;
}

_2i16 __attribute__((overloadable)) shuffle(_8i16 x, _2u16 mask)
{
	_8u16 tMask;
	tMask.s01 = mask;
	return shuffle(x, tMask).s01;
}

_4i16 __attribute__((overloadable)) shuffle(_8i16 x, _4u16 mask)
{
	_8u16 tMask;
	tMask.s0123 = mask;
	return shuffle(x, tMask).s0123;
}

_8i16 __attribute__((overloadable)) shuffle(_8i16 x, _8u16 mask)
{
	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask, __m128i),
             __builtin_astype(shuffle_short8_mask, __m128i)),
           _8u16);
	x = __builtin_astype(
        SHUFFLE_EPI16(
          __builtin_astype(x, __m128i),
          __builtin_astype(mask, __m128i)),
        _8i16);
	return x;
}

_16i16 __attribute__((overloadable)) shuffle(_8i16 x, _16u16 mask)
{
	_16i16 res;

	res.lo = shuffle(x, mask.lo);
	res.hi = shuffle(x, mask.hi);

	return res;
}

_2i16 __attribute__((overloadable)) shuffle(_16i16 x, _2u16 mask)
{
	_8u16 tMask;
	tMask.s01 = mask;
	return shuffle(x, tMask).s01;
}

_4i16 __attribute__((overloadable)) shuffle(_16i16 x, _4u16 mask)
{
	_8u16 tMask;
	tMask.s0123 = mask;
	return shuffle(x, tMask).s0123;
}

_8i16 __attribute__((overloadable)) shuffle(_16i16 x, _8u16 mask)
{
	_8i16 res1;
	_8i16 res2;

	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask, __m128i),
             __builtin_astype(shuffle_short16_mask, __m128i)),
           _8u16);

    res1 = __builtin_astype(
             SHUFFLE_EPI16(
               __builtin_astype(x.lo, __m128i), 
               (__m128i)(__builtin_astype(mask, ushort8) + (ushort8)(56))),
             _8i16);
    res2 = __builtin_astype(
             SHUFFLE_EPI16(
               __builtin_astype(x.hi, __m128i),
               (__m128i)(__builtin_astype(mask, ushort8) - (ushort8)(8)) ),
             _8i16);

	return __builtin_astype(_mm_or_si128(
           __builtin_astype(res1, __m128i), 
           __builtin_astype(res2, __m128i)),
         _8i16);
}

_16i16 __attribute__((overloadable)) shuffle(_16i16 x, _16u16 mask)
{
	_16i16 res;

	res.lo = shuffle(x, mask.lo);
	res.hi = shuffle(x, mask.hi);

	return res;
}

_2u16 __attribute__((overloadable)) shuffle(_2u16 x, _2u16 mask)
{
	return as_ushort2(shuffle(as_short2(x), mask));
}

_4u16 __attribute__((overloadable)) shuffle(_2u16 x, _4u16 mask)
{
	return as_ushort4(shuffle(as_short2(x), mask));
}

_8u16 __attribute__((overloadable)) shuffle(_2u16 x, _8u16 mask)
{
	return as_ushort8(shuffle(as_short2(x), mask));
}

_16u16 __attribute__((overloadable)) shuffle(_2u16 x, _16u16 mask)
{
	return as_ushort16(shuffle(as_short2(x), mask));
}

_2u16 __attribute__((overloadable)) shuffle(_4u16 x, _2u16 mask)
{
	return as_ushort2(shuffle(as_short4(x), mask));
}

_4u16 __attribute__((overloadable)) shuffle(_4u16 x, _4u16 mask)
{
	return as_ushort4(shuffle(as_short4(x), mask));
}

_8u16 __attribute__((overloadable)) shuffle(_4u16 x, _8u16 mask)
{
	return as_ushort8(shuffle(as_short4(x), mask));
}

_16u16 __attribute__((overloadable)) shuffle(_4u16 x, _16u16 mask)
{
	return as_ushort16(shuffle(as_short4(x), mask));
}

_2u16 __attribute__((overloadable)) shuffle(_8u16 x, _2u16 mask)
{
	return as_ushort2(shuffle(as_short8(x), mask));
}

_4u16 __attribute__((overloadable)) shuffle(_8u16 x, _4u16 mask)
{
	return as_ushort4(shuffle(as_short8(x), mask));
}

_8u16 __attribute__((overloadable)) shuffle(_8u16 x, _8u16 mask)
{
	return as_ushort8(shuffle(as_short8(x), mask));
}

_16u16 __attribute__((overloadable)) shuffle(_8u16 x, _16u16 mask)
{
	return as_ushort16(shuffle(as_short8(x), mask));
}

_2u16 __attribute__((overloadable)) shuffle(_16u16 x, _2u16 mask)
{
	return as_ushort2(shuffle(as_short16(x), mask));
}

_4u16 __attribute__((overloadable)) shuffle(_16u16 x, _4u16 mask)
{
	return as_ushort4(shuffle(as_short16(x), mask));
}

_8u16 __attribute__((overloadable)) shuffle(_16u16 x, _8u16 mask)
{
	return as_ushort8(shuffle(as_short16(x), mask));
}

_16u16 __attribute__((overloadable)) shuffle(_16u16 x, _16u16 mask)
{
	return as_ushort16(shuffle(as_short16(x), mask));
}

#if defined (__AVX2__)
_8i32 __attribute__((overloadable)) shuffle(_2i32 x, _8u32 mask)
{
	_8i32 tx;
	tx.s01 = x;
	return shuffle(tx, mask);
}
_8i32 __attribute__((overloadable)) shuffle(_4i32 x, _8u32 mask)
{
	_8i32 tx;
	tx.lo = x;
	return shuffle(tx, mask);
}
_8i32 __attribute__((overloadable)) shuffle(_8i32 x, _8u32 mask)
{
	return (_8i32) _mm256_permutevar8x32_epi32((__m256i) x, (__m256i) mask);
}

_16i32 __attribute__((overloadable)) shuffle(_8i32 x, _16u32 mask)
{
	_16i32 res;
	res.lo = (_8i32) _mm256_permutevar8x32_epi32((__m256i) x, (__m256i) mask.lo);
	res.hi = (_8i32) _mm256_permutevar8x32_epi32((__m256i) x, (__m256i) mask.hi);
	return res;
}
_16i32 __attribute__((overloadable)) shuffle(_4i32 x, _16u32 mask)
{
	_16i32 res;
	_8i32 tx;
	tx.lo = x;
	res.lo = shuffle(tx, mask.lo);
	res.hi = shuffle(tx, mask.hi);
	return res;
}
_2i32 __attribute__((overloadable)) shuffle(_8i32 x, _2u32 mask)
{
	_8u32 tMask;
	tMask.s01 = mask;
	return shuffle(x, tMask).s01;
}

_4i32 __attribute__((overloadable)) shuffle(_8i32 x, _4u32 mask)
{
	_8u32 tMask;
	tMask.lo = mask;
	return shuffle(x, tMask).lo;
}

#else
_8i32 __attribute__((overloadable)) shuffle(_2i32 x, _8u32 mask)
{
	_8i32 res;
	_4i32 tx;
	tx.lo = x;
	res.lo = shuffle(tx, mask.lo);
	res.hi = shuffle(tx, mask.hi);
	return res;
}

_8i32 __attribute__((overloadable)) shuffle(_4i32 x, _8u32 mask)
{
	_8i32 res;
	res.lo = shuffle(x, mask.lo);
	res.hi = shuffle(x, mask.hi);
	return res;
}

_8i32 __attribute__((overloadable)) shuffle(_8i32 x, _8u32 mask)
{
	_8i32 res;

	res.lo = shuffle(x, mask.lo);
	res.hi = shuffle(x, mask.hi);

	return res;
}

_16i32 __attribute__((overloadable)) shuffle(_8i32 x, _16u32 mask)
{
	_16i32 res;

	res.lo.lo = shuffle(x, mask.lo.lo);
	res.lo.hi = shuffle(x, mask.lo.hi);
	res.hi.lo = shuffle(x, mask.hi.lo);
	res.hi.hi = shuffle(x, mask.hi.hi);

	return res;
}
_16i32 __attribute__((overloadable)) shuffle(_4i32 x, _16u32 mask)
{
	_16i32 res;

	res.lo.lo = shuffle(x, mask.lo.lo);
	res.lo.hi = shuffle(x, mask.lo.hi);
	res.hi.lo = shuffle(x, mask.hi.lo);
	res.hi.hi = shuffle(x, mask.hi.hi);

	return res;
}
_2i32 __attribute__((overloadable)) shuffle(_8i32 x, _2u32 mask)
{
	_4u32 tMask;
	tMask.s01 = mask;
	return shuffle(x, tMask).s01;
}

_4i32 __attribute__((overloadable)) shuffle(_8i32 x, _4u32 mask)
{
	_4i32 res1;
	_4i32 res2;

	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask, __m128i),
             __builtin_astype(shuffle_int8_mask, __m128i)),
           _4u32);
    res1 = __builtin_astype(
             SHUFFLE_EPI32(
               __builtin_astype(x.lo, __m128i), 
               __builtin_astype(mask + (uint4)(28), __m128i)),
             _4i32);
    res2 = __builtin_astype(
             SHUFFLE_EPI32(
               __builtin_astype(x.hi, __m128i), 
               __builtin_astype(mask - (uint4)(4), __m128i)),
             _4i32);

	return __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res1, __m128i),
             __builtin_astype(res2, __m128i)),
           _4i32);
}

#endif

#if defined (__AVX2__) ||  defined(__AVX__) 
_2i32 __attribute__((overloadable)) shuffle(_2i32 x, _2u32 mask)
{
	_4u32 tMask;
	_4i32 tx;
	tMask.s01 = mask;
	tx.s01 = x;
	return shuffle(tx, tMask).s01;
}

_4i32 __attribute__((overloadable)) shuffle(_2i32 x, _4u32 mask)
{
	_4i32 tx;
	tx.s01 = x;
	return shuffle(tx, mask);
}

_4i32 __attribute__((overloadable)) shuffle(_4i32 x, _4u32 mask)
{
	return as_int4(_mm_permutevar_ps((__m128)x,(__m128i)mask));
}

_16i32 __attribute__((overloadable)) shuffle(_2i32 x, _16u32 mask)
{
	_8i32 tx;
	_16i32 res;
	tx.s01 = x;
	res.lo = shuffle(tx, mask.lo);
	res.hi = shuffle(tx, mask.hi);
	return res;
}

#else
_2i32 __attribute__((overloadable)) shuffle(_2i32 x, _2u32 mask)
{
	_4u32 tMask;
	tMask.s01 = mask;
	return shuffle(x, tMask).s01;
}

_4i32 __attribute__((overloadable)) shuffle(_2i32 x, _4u32 mask)
{
	_4i32 tX;
	tX.s01 = x;
	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask, __m128i),
             __builtin_astype(shuffle_int2_mask, __m128i)),
           _4u32);
	tX = __builtin_astype(
         SHUFFLE_EPI32(
           __builtin_astype(tX, __m128i),
           __builtin_astype(mask, __m128i)),
         _4i32);
	return tX;
}

_4i32 __attribute__((overloadable)) shuffle(_4i32 x, _4u32 mask)
{
	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask, __m128i),
             __builtin_astype(shuffle_int4_mask, __m128i)),
           _4u32);
	x = __builtin_astype(
        SHUFFLE_EPI32(
          __builtin_astype(x, __m128i),
          __builtin_astype(mask, __m128i)),
        _4i32);
	return x;
}

_16i32 __attribute__((overloadable)) shuffle(_2i32 x, _16u32 mask)
{
	_16i32 res;

	res.lo.lo = shuffle(x, mask.lo.lo);
	res.lo.hi = shuffle(x, mask.lo.hi);
	res.hi.lo = shuffle(x, mask.hi.lo);
	res.hi.hi = shuffle(x, mask.hi.hi);

	return res;
}

#endif

_2i32 __attribute__((overloadable)) shuffle(_4i32 x, _2u32 mask)
{
	_4u32 tMask;
	tMask.s01 = mask;
	return shuffle(x, tMask).s01;
}

_2i32 __attribute__((overloadable)) shuffle(_16i32 x, _2u32 mask)
{
	_4u32 tMask;
	tMask.s01 = mask;
	return shuffle(x, tMask).s01;
}

_4i32 __attribute__((overloadable)) shuffle(_16i32 x, _4u32 mask)
{
	_4i32 res1;
	_4i32 res2;
	_4i32 res3;
	_4i32 res4;
	_4u32 t1;
	_4u32 t2;
	_4u32 t3;
	_4u32 t4;

	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask,__m128i),
             __builtin_astype(shuffle_int16_mask, __m128i)),
           _4u32);

    t1 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask, __m128i),
             __builtin_astype((uint4)(28), __m128i)),
           _4u32);
    t2 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask - (uint4)(4), __m128i),
             __builtin_astype((uint4)(28), __m128i)),
           _4u32);
    t3 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask - (uint4)(8), __m128i),
             __builtin_astype((uint4)(28), __m128i)),
           _4u32);
    t4 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask - (uint4)(12), __m128i),
             __builtin_astype((uint4)(28), __m128i)),
           _4u32);
	
	res1 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(x.lo.lo,__m128i),
             __builtin_astype(t1,__m128i)),
           _4i32);
	res2 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(x.lo.hi,__m128i),
             __builtin_astype(t2,__m128i)),
           _4i32);
	res3 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(x.hi.lo,__m128i),
             __builtin_astype(t3,__m128i)),
           _4i32);
	res4 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(x.hi.hi,__m128i),
             __builtin_astype(t4,__m128i)),
           _4i32);
	res1 = __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res1,__m128i),
             __builtin_astype(res2,__m128i)),
           _4i32);
	res2 = __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res3,__m128i),
             __builtin_astype(res4,__m128i)),
           _4i32);

	return __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res1,__m128i),
             __builtin_astype(res2,__m128i)),
           _4i32);
}

_8i32 __attribute__((overloadable)) shuffle(_16i32 x, _8u32 mask)
{
	_8i32 res;

	res.lo = shuffle(x, mask.lo);
	res.hi = shuffle(x, mask.hi);

	return res;
}

_16i32 __attribute__((overloadable)) shuffle(_16i32 x, _16u32 mask)
{
	_16i32 res;

	res.lo.lo = shuffle(x, mask.lo.lo);
	res.lo.hi = shuffle(x, mask.lo.hi);
	res.hi.lo = shuffle(x, mask.hi.lo);
	res.hi.hi = shuffle(x, mask.hi.hi);

	return res;
}

_2u32 __attribute__((overloadable)) shuffle(_2u32 x, _2u32 mask)
{
	return as_uint2(shuffle(as_int2(x), mask));
}

_4u32 __attribute__((overloadable)) shuffle(_2u32 x, _4u32 mask)
{
	return as_uint4(shuffle(as_int2(x), mask));
}

_8u32 __attribute__((overloadable)) shuffle(_2u32 x, _8u32 mask)
{
	return as_uint8(shuffle(as_int2(x), mask));
}

_16u32 __attribute__((overloadable)) shuffle(_2u32 x, _16u32 mask)
{
	return as_uint16(shuffle(as_int2(x), mask));
}

_2u32 __attribute__((overloadable)) shuffle(_4u32 x, _2u32 mask)
{
	return as_uint2(shuffle(as_int4(x), mask));
}

_4u32 __attribute__((overloadable)) shuffle(_4u32 x, _4u32 mask)
{
	return as_uint4(shuffle(as_int4(x), mask));
}

_8u32 __attribute__((overloadable)) shuffle(_4u32 x, _8u32 mask)
{
	return as_uint8(shuffle(as_int4(x), mask));
}

_16u32 __attribute__((overloadable)) shuffle(_4u32 x, _16u32 mask)
{
	return as_uint16(shuffle(as_int4(x), mask));
}

_2u32 __attribute__((overloadable)) shuffle(_8u32 x, _2u32 mask)
{
	return as_uint2(shuffle(as_int8(x), mask));
}

_4u32 __attribute__((overloadable)) shuffle(_8u32 x, _4u32 mask)
{
	return as_uint4(shuffle(as_int8(x), mask));
}

_8u32 __attribute__((overloadable)) shuffle(_8u32 x, _8u32 mask)
{
	return as_uint8(shuffle(as_int8(x), mask));
}

_16u32 __attribute__((overloadable)) shuffle(_8u32 x, _16u32 mask)
{
	return as_uint16(shuffle(as_int8(x), mask));
}

_2u32 __attribute__((overloadable)) shuffle(_16u32 x, _2u32 mask)
{
	return as_uint2(shuffle(as_int16(x), mask));
}

_4u32 __attribute__((overloadable)) shuffle(_16u32 x, _4u32 mask)
{
	return as_uint4(shuffle(as_int16(x), mask));
}

_8u32 __attribute__((overloadable)) shuffle(_16u32 x, _8u32 mask)
{
	return as_uint8(shuffle(as_int16(x), mask));
}

_16u32 __attribute__((overloadable)) shuffle(_16u32 x, _16u32 mask)
{
	return as_uint16(shuffle(as_int16(x), mask));
}
_2i64 __attribute__((overloadable)) shuffle(_2i64 x, _2u64 mask)
{
	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask, __m128i),
            __builtin_astype(shuffle_long2_mask, __m128i)),
           _2u64);
	x = __builtin_astype(
        SHUFFLE_EPI64(
          __builtin_astype(x, __m128i),
          __builtin_astype(mask,__m128i)),
        _2i64);
	return x;
}

_4i64 __attribute__((overloadable)) shuffle(_2i64 x, _4u64 mask)
{
	_4i64 res;

	res.lo = shuffle(x, mask.lo);
	res.hi = shuffle(x, mask.hi);

	return res;
}
_8i64 __attribute__((overloadable)) shuffle(_2i64 x, _8u64 mask)
{
	_8i64 res;

	res.lo.lo = shuffle(x, mask.lo.lo);
	res.lo.hi = shuffle(x, mask.lo.hi);
	res.hi.lo = shuffle(x, mask.hi.lo);
	res.hi.hi = shuffle(x, mask.hi.hi);

	return res;
}

_16i64 __attribute__((overloadable)) shuffle(_2i64 x, _16u64 mask)
{
	_16i64 res;

	res.lo.lo.lo = shuffle(x, mask.lo.lo.lo);
	res.lo.lo.hi = shuffle(x, mask.lo.lo.hi);
	res.lo.hi.lo = shuffle(x, mask.lo.hi.lo);
	res.lo.hi.hi = shuffle(x, mask.lo.hi.hi);
	res.hi.lo.lo = shuffle(x, mask.hi.lo.lo);
	res.hi.lo.hi = shuffle(x, mask.hi.lo.hi);
	res.hi.hi.lo = shuffle(x, mask.hi.hi.lo);
	res.hi.hi.hi = shuffle(x, mask.hi.hi.hi);

	return res;
}
#if defined (__AVX2__)
_4i64 __attribute__((overloadable)) shuffle(_4i64 x, _4u64 mask)
{
	__m256i tmplo = _mm256_setzero_si256();
	__m256i tmphi = _mm256_setzero_si256();
	_4i64 res;
	_8u32 tmaskhi;
	_8u32 tmask;
	_8u32 tmasklo = as_uint8(mask);
	tmasklo = tmasklo+tmasklo;
	tmaskhi = tmasklo+1;
	tmaskhi = as_uint8(_mm256_slli_epi64( (__m256i) tmaskhi, 32));
	tmask = as_uint8(_mm256_or_si256( (__m256i) tmasklo, (__m256i) tmaskhi));
	res = as_long4(_mm256_permutevar8x32_epi32( (__m256i) x, (__m256i) tmask));
	return res;
}

_8i64 __attribute__((overloadable)) shuffle(_4i64 x, _8u64 mask)
{
	_8i64 res;
	res.lo = shuffle (x, mask.lo);
	res.hi = shuffle (x, mask.hi);
	return res;
}

_16i64 __attribute__((overloadable)) shuffle(_4i64 x, _16u64 mask)
{
	_16i64 res;
	res.lo = shuffle (x, mask.lo);
	res.hi = shuffle (x, mask.hi);
	return res;
}

#else //__AVX2__
_4i64 __attribute__((overloadable)) shuffle(_4i64 x, _4u64 mask)
{
	_4i64 res;
	_1i64 xVec[4];
	memcpy(xVec, &x, sizeof(_4i64));

	res.s0 = xVec[mask.s0 & 3];
	res.s1 = xVec[mask.s1 & 3];
	res.s2 = xVec[mask.s2 & 3];
	res.s3 = xVec[mask.s3 & 3];

	return res;
}

_8i64 __attribute__((overloadable)) shuffle(_4i64 x, _8u64 mask)
{
	_8i64 res;
	_1i64 xVec[4];
	memcpy(xVec, &x, sizeof(_4i64));

	res.s0 = xVec[mask.s0 & 3];
	res.s1 = xVec[mask.s1 & 3];
	res.s2 = xVec[mask.s2 & 3];
	res.s3 = xVec[mask.s3 & 3];
	res.s4 = xVec[mask.s4 & 3];
	res.s5 = xVec[mask.s5 & 3];
	res.s6 = xVec[mask.s6 & 3];
	res.s7 = xVec[mask.s7 & 3];

	return res;
}

_16i64 __attribute__((overloadable)) shuffle(_4i64 x, _16u64 mask)
{
	_16i64 res;
	_1i64 xVec[4];
	memcpy(xVec, &x, sizeof(_4i64));

	res.s0 = xVec[mask.s0 & 3];
	res.s1 = xVec[mask.s1 & 3];
	res.s2 = xVec[mask.s2 & 3];
	res.s3 = xVec[mask.s3 & 3];
	res.s4 = xVec[mask.s4 & 3];
	res.s5 = xVec[mask.s5 & 3];
	res.s6 = xVec[mask.s6 & 3];
	res.s7 = xVec[mask.s7 & 3];
	res.s8 = xVec[mask.s8 & 3];
	res.s9 = xVec[mask.s9 & 3];
	res.sA = xVec[mask.sA & 3];
	res.sB = xVec[mask.sB & 3];
	res.sC = xVec[mask.sC & 3];
	res.sD = xVec[mask.sD & 3];
	res.sE = xVec[mask.sE & 3];
	res.sF = xVec[mask.sF & 3];

	return res;
}

#endif

_2i64 __attribute__((overloadable)) shuffle(_4i64 x, _2u64 mask)
{
	_2i64 res;
	_1i64 xVec[4];
	memcpy(xVec, &x, sizeof(_4i64));

	res.s0 = xVec[mask.s0 & 3];
	res.s1 = xVec[mask.s1 & 3];

	return res;
}

_2i64 __attribute__((overloadable)) shuffle(_8i64 x, _2u64 mask)
{
	_2i64 res;
	_1i64 xVec[8];
	memcpy(xVec, &x, sizeof(_8i64));

	res.s0 = xVec[mask.s0 & 7];
	res.s1 = xVec[mask.s1 & 7];

	return res;
}

_4i64 __attribute__((overloadable)) shuffle(_8i64 x, _4u64 mask)
{
	_4i64 res;
	_1i64 xVec[8];
	memcpy(xVec, &x, sizeof(_8i64));

	res.s0 = xVec[mask.s0 & 7];
	res.s1 = xVec[mask.s1 & 7];
	res.s2 = xVec[mask.s2 & 7];
	res.s3 = xVec[mask.s3 & 7];

	return res;
}

_8i64 __attribute__((overloadable)) shuffle(_8i64 x, _8u64 mask)
{
	_8i64 res;
	_1i64 xVec[8];
	memcpy(xVec, &x, sizeof(_8i64));

	res.s0 = xVec[mask.s0 & 7];
	res.s1 = xVec[mask.s1 & 7];
	res.s2 = xVec[mask.s2 & 7];
	res.s3 = xVec[mask.s3 & 7];
	res.s4 = xVec[mask.s4 & 7];
	res.s5 = xVec[mask.s5 & 7];
	res.s6 = xVec[mask.s6 & 7];
	res.s7 = xVec[mask.s7 & 7];

	return res;
}

_16i64 __attribute__((overloadable)) shuffle(_8i64 x, _16u64 mask)
{
	_16i64 res;
	_1i64 xVec[8];
	memcpy(xVec, &x, sizeof(_8i64));

	res.s0 = xVec[mask.s0 & 7];
	res.s1 = xVec[mask.s1 & 7];
	res.s2 = xVec[mask.s2 & 7];
	res.s3 = xVec[mask.s3 & 7];
	res.s4 = xVec[mask.s4 & 7];
	res.s5 = xVec[mask.s5 & 7];
	res.s6 = xVec[mask.s6 & 7];
	res.s7 = xVec[mask.s7 & 7];
	res.s8 = xVec[mask.s8 & 7];
	res.s9 = xVec[mask.s9 & 7];
	res.sA = xVec[mask.sA & 7];
	res.sB = xVec[mask.sB & 7];
	res.sC = xVec[mask.sC & 7];
	res.sD = xVec[mask.sD & 7];
	res.sE = xVec[mask.sE & 7];
	res.sF = xVec[mask.sF & 7];

	return res;
}

_2i64 __attribute__((overloadable)) shuffle(_16i64 x, _2u64 mask)
{
	_2i64 res;
	_1i64 xVec[16];
	memcpy(xVec, &x, sizeof(_16i64));

	res.s0 = xVec[mask.s0 & 15];
	res.s1 = xVec[mask.s1 & 15];

	return res;
}

_4i64 __attribute__((overloadable)) shuffle(_16i64 x, _4u64 mask)
{
	_4i64 res;
	_1i64 xVec[16];
	memcpy(xVec, &x, sizeof(_16i64));

	res.s0 = xVec[mask.s0 & 15];
	res.s1 = xVec[mask.s1 & 15];
	res.s2 = xVec[mask.s2 & 15];
	res.s3 = xVec[mask.s3 & 15];

	return res;
}

_8i64 __attribute__((overloadable)) shuffle(_16i64 x, _8u64 mask)
{
	_8i64 res;
	_1i64 xVec[16];
	memcpy(xVec, &x, sizeof(_16i64));

	res.s0 = xVec[mask.s0 & 15];
	res.s1 = xVec[mask.s1 & 15];
	res.s2 = xVec[mask.s2 & 15];
	res.s3 = xVec[mask.s3 & 15];
	res.s4 = xVec[mask.s4 & 15];
	res.s5 = xVec[mask.s5 & 15];
	res.s6 = xVec[mask.s6 & 15];
	res.s7 = xVec[mask.s7 & 15];

	return res;
}

_16i64 __attribute__((overloadable)) shuffle(_16i64 x, _16u64 mask)
{
	_16i64 res;
	_1i64 xVec[16];
	memcpy(xVec, &x, sizeof(_16i64));

	res.s0 = xVec[mask.s0 & 15];
	res.s1 = xVec[mask.s1 & 15];
	res.s2 = xVec[mask.s2 & 15];
	res.s3 = xVec[mask.s3 & 15];
	res.s4 = xVec[mask.s4 & 15];
	res.s5 = xVec[mask.s5 & 15];
	res.s6 = xVec[mask.s6 & 15];
	res.s7 = xVec[mask.s7 & 15];
	res.s8 = xVec[mask.s8 & 15];
	res.s9 = xVec[mask.s9 & 15];
	res.sA = xVec[mask.sA & 15];
	res.sB = xVec[mask.sB & 15];
	res.sC = xVec[mask.sC & 15];
	res.sD = xVec[mask.sD & 15];
	res.sE = xVec[mask.sE & 15];
	res.sF = xVec[mask.sF & 15];

	return res;
}

_2u64 __attribute__((overloadable)) shuffle(_2u64 x, _2u64 mask)
{
	return as_ulong2(shuffle(as_long2(x), mask));
}

_4u64 __attribute__((overloadable)) shuffle(_2u64 x, _4u64 mask)
{
	return as_ulong4(shuffle(as_long2(x), mask));
}

_8u64 __attribute__((overloadable)) shuffle(_2u64 x, _8u64 mask)
{
	return as_ulong8(shuffle(as_long2(x), mask));
}

_16u64 __attribute__((overloadable)) shuffle(_2u64 x, _16u64 mask)
{
	return as_ulong16(shuffle(as_long2(x), mask));
}

_2u64 __attribute__((overloadable)) shuffle(_4u64 x, _2u64 mask)
{
	return as_ulong2(shuffle(as_long4(x), mask));
}

_4u64 __attribute__((overloadable)) shuffle(_4u64 x, _4u64 mask)
{
	return as_ulong4(shuffle(as_long4(x), mask));
}

_8u64 __attribute__((overloadable)) shuffle(_4u64 x, _8u64 mask)
{
	return as_ulong8(shuffle(as_long4(x), mask));
}

_16u64 __attribute__((overloadable)) shuffle(_4u64 x, _16u64 mask)
{
	return as_ulong16(shuffle(as_long4(x), mask));
}

_2u64 __attribute__((overloadable)) shuffle(_8u64 x, _2u64 mask)
{
	return as_ulong2(shuffle(as_long8(x), mask));
}

_4u64 __attribute__((overloadable)) shuffle(_8u64 x, _4u64 mask)
{
	return as_ulong4(shuffle(as_long8(x), mask));
}

_8u64 __attribute__((overloadable)) shuffle(_8u64 x, _8u64 mask)
{
	return as_ulong8(shuffle(as_long8(x), mask));
}

_16u64 __attribute__((overloadable)) shuffle(_8u64 x, _16u64 mask)
{
	return as_ulong16(shuffle(as_long8(x), mask));
}

_2u64 __attribute__((overloadable)) shuffle(_16u64 x, _2u64 mask)
{
	return as_ulong2(shuffle(as_long16(x), mask));
}

_4u64 __attribute__((overloadable)) shuffle(_16u64 x, _4u64 mask)
{
	return as_ulong4(shuffle(as_long16(x), mask));
}

_8u64 __attribute__((overloadable)) shuffle(_16u64 x, _8u64 mask)
{
	return as_ulong8(shuffle(as_long16(x), mask));
}

_16u64 __attribute__((overloadable)) shuffle(_16u64 x, _16u64 mask)
{
	return as_ulong16(shuffle(as_long16(x), mask));
}

float2 __attribute__((overloadable)) shuffle(float2 x, _2u32 mask)
{
	return as_float2(shuffle(as_int2(x), mask));
}

float4 __attribute__((overloadable)) shuffle(float2 x, _4u32 mask)
{
	return as_float4(shuffle(as_int2(x), mask));
}

float8 __attribute__((overloadable)) shuffle(float2 x, _8u32 mask)
{
	return as_float8(shuffle(as_int2(x), mask));
}

float16 __attribute__((overloadable)) shuffle(float2 x, _16u32 mask)
{
	return as_float16(shuffle(as_int2(x), mask));
}

float2 __attribute__((overloadable)) shuffle(float4 x, _2u32 mask)
{
	return as_float2(shuffle(as_int4(x), mask));
}

float4 __attribute__((overloadable)) shuffle(float4 x, _4u32 mask)
{
	return as_float4(shuffle(as_int4(x), mask));
}

float8 __attribute__((overloadable)) shuffle(float4 x, _8u32 mask)
{
	return as_float8(shuffle(as_int4(x), mask));
}

float16 __attribute__((overloadable)) shuffle(float4 x, _16u32 mask)
{
	return as_float16(shuffle(as_int4(x), mask));
}

float2 __attribute__((overloadable)) shuffle(float8 x, _2u32 mask)
{
	return as_float2(shuffle(as_int8(x), mask));
}
#if defined (__AVX2__)

float4 __attribute__((overloadable)) shuffle(float8 x, _4u32 mask)
{
	_8u32 tmask = as_uint8( _mm256_setzero_si256());
	float8 res;
	tmask.lo = mask;
	res = as_float8(_mm256_permutevar8x32_ps ((__m256) x, (__m256)tmask));
	return res.lo;
}
float8 __attribute__((overloadable)) shuffle(float8 x, _8u32 mask)
{
	return as_float8(_mm256_permutevar8x32_ps ((__m256) x, (__m256)mask));
}

float16 __attribute__((overloadable)) shuffle(float8 x, _16u32 mask)
{
	float16 res;
	res.lo = as_float8(shuffle(x, mask.lo));
	res.hi = as_float8(shuffle(x, mask.hi));
	return res; 
}

#else
float4 __attribute__((overloadable)) shuffle(float8 x, _4u32 mask)
{
	return as_float4(shuffle(as_int8(x), mask));
}

float8 __attribute__((overloadable)) shuffle(float8 x, _8u32 mask)
{
	return as_float8(shuffle(as_int8(x), mask));
}

float16 __attribute__((overloadable)) shuffle(float8 x, _16u32 mask)
{
	return as_float16(shuffle(as_int8(x), mask));
}

#endif


float2 __attribute__((overloadable)) shuffle(float16 x, _2u32 mask)
{
	return as_float2(shuffle(as_int16(x), mask));
}

float4 __attribute__((overloadable)) shuffle(float16 x, _4u32 mask)
{
	return as_float4(shuffle(as_int16(x), mask));
}

float8 __attribute__((overloadable)) shuffle(float16 x, _8u32 mask)
{
	return as_float8(shuffle(as_int16(x), mask));
}

float16 __attribute__((overloadable)) shuffle(float16 x, _16u32 mask)
{
	return as_float16(shuffle(as_int16(x), mask));
}

double2 __attribute__((overloadable)) shuffle(double2 x, _2u64 mask)
{
	return as_double2(shuffle(as_long2(x), mask));
}

double4 __attribute__((overloadable)) shuffle(double2 x, _4u64 mask)
{
	return as_double4(shuffle(as_long2(x), mask));
}

double8 __attribute__((overloadable)) shuffle(double2 x, _8u64 mask)
{
	return as_double8(shuffle(as_long2(x), mask));
}

double16 __attribute__((overloadable)) shuffle(double2 x, _16u64 mask)
{
	return as_double16(shuffle(as_long2(x), mask));
}

double2 __attribute__((overloadable)) shuffle(double4 x, _2u64 mask)
{
	return as_double2(shuffle(as_long4(x), mask));
}

double4 __attribute__((overloadable)) shuffle(double4 x, _4u64 mask)
{
	return as_double4(shuffle(as_long4(x), mask));
}

double8 __attribute__((overloadable)) shuffle(double4 x, _8u64 mask)
{
	return as_double8(shuffle(as_long4(x), mask));
}

double16 __attribute__((overloadable)) shuffle(double4 x, _16u64 mask)
{
	return as_double16(shuffle(as_long4(x), mask));
}

double2 __attribute__((overloadable)) shuffle(double8 x, _2u64 mask)
{
	return as_double2(shuffle(as_long8(x), mask));
}

double4 __attribute__((overloadable)) shuffle(double8 x, _4u64 mask)
{
	return as_double4(shuffle(as_long8(x), mask));
}

double8 __attribute__((overloadable)) shuffle(double8 x, _8u64 mask)
{
	return as_double8(shuffle(as_long8(x), mask));
}

double16 __attribute__((overloadable)) shuffle(double8 x, _16u64 mask)
{
	return as_double16(shuffle(as_long8(x), mask));
}

double2 __attribute__((overloadable)) shuffle(double16 x, _2u64 mask)
{
	return as_double2(shuffle(as_long16(x), mask));
}

double4 __attribute__((overloadable)) shuffle(double16 x, _4u64 mask)
{
	return as_double4(shuffle(as_long16(x), mask));
}

double8 __attribute__((overloadable)) shuffle(double16 x, _8u64 mask)
{
	return as_double8(shuffle(as_long16(x), mask));
}

double16 __attribute__((overloadable)) shuffle(double16 x, _16u64 mask)
{
	return as_double16(shuffle(as_long16(x), mask));
}


_16i8 __attribute__((overloadable)) shuffle2(_2i8 x, _2i8 y, _16u8 mask)
{
	_4i8 tX;
	tX.s01 = x;
	tX.s23 = y;

	return shuffle(tX, mask);
}

_2i8 __attribute__((overloadable)) shuffle2(_2i8 x, _2i8 y, _2u8 mask)
{
	_16u8 tMask;
	tMask.s01 = mask;
	return shuffle2(x, y, tMask).s01;
}

_4i8 __attribute__((overloadable)) shuffle2(_2i8 x, _2i8 y, _4u8 mask)
{
	_16u8 tMask;
	tMask.s0123 = mask;
	return shuffle2(x, y, tMask).s0123;
}

_8i8 __attribute__((overloadable)) shuffle2(_2i8 x, _2i8 y, _8u8 mask)
{
	_16u8 tMask;
	tMask.lo = mask;
	return shuffle2(x, y, tMask).lo;
}

_16i8 __attribute__((overloadable)) shuffle2(_4i8 x, _4i8 y, _16u8 mask)
{
	_8i8 tX;
	tX.s0123 = x;
	tX.s4567 = y;

	return shuffle(tX, mask);
}

_2i8 __attribute__((overloadable)) shuffle2(_4i8 x, _4i8 y, _2u8 mask)
{
	_16u8 tMask;
	tMask.s01 = mask;
	return shuffle2(x, y, tMask).s01;
}

_4i8 __attribute__((overloadable)) shuffle2(_4i8 x, _4i8 y, _4u8 mask)
{
	_16u8 tMask;
	tMask.s0123 = mask;
	return shuffle2(x, y, tMask).s0123;
}

_8i8 __attribute__((overloadable)) shuffle2(_4i8 x, _4i8 y, _8u8 mask)
{
	_16u8 tMask;
	tMask.lo = mask;
	return shuffle2(x, y, tMask).lo;
}

_16i8 __attribute__((overloadable)) shuffle2(_8i8 x, _8i8 y, _16u8 mask)
{
	_16i8 tX;
	tX.lo = x;
	tX.hi = y;

	return shuffle(tX, mask);
}

_2i8 __attribute__((overloadable)) shuffle2(_8i8 x, _8i8 y, _2u8 mask)
{
	_16u8 tMask;
	tMask.s01 = mask;
	return shuffle2(x, y, tMask).s01;
}

_4i8 __attribute__((overloadable)) shuffle2(_8i8 x, _8i8 y, _4u8 mask)
{
	_16u8 tMask;
	tMask.s0123 = mask;
	return shuffle2(x, y, tMask).s0123;
}

_8i8 __attribute__((overloadable)) shuffle2(_8i8 x, _8i8 y, _8u8 mask)
{
	_16u8 tMask;
	tMask.lo = mask;
	return shuffle2(x, y, tMask).lo;
}

_16i8 __attribute__((overloadable)) shuffle2(_16i8 x, _16i8 y, _16u8 mask)
{
	_16u8 mask1, mask2;

  mask1 = __builtin_astype(
            _mm_and_si128(
              __builtin_astype(mask + (uchar16)(112),__m128i),
              __builtin_astype(shuffle2_char16_mask, __m128i)),
            _16u8);
	x = __builtin_astype(
        SHUFFLE_EPI8(
          __builtin_astype(x, __m128i),
          __builtin_astype(mask1, __m128i)),
        _16i8);

  mask2 = __builtin_astype(
            _mm_and_si128(
              __builtin_astype(mask - (uchar16)(16),__m128i),
              __builtin_astype(shuffle2_char16_mask, __m128i)),
            _16u8);
	y = __builtin_astype(
        SHUFFLE_EPI8(
          __builtin_astype(y,__m128i),
          __builtin_astype(mask2,__m128i)),
        _16i8);

	return __builtin_astype(
           _mm_or_si128(
             __builtin_astype(x,__m128i),
             __builtin_astype(y,__m128i)),
           _16i8);
}

_2i8 __attribute__((overloadable)) shuffle2(_16i8 x, _16i8 y, _2u8 mask)
{
	_16u8 tMask;
	tMask.s01 = mask;
	return shuffle2(x, y, tMask).s01;
}

_4i8 __attribute__((overloadable)) shuffle2(_16i8 x, _16i8 y, _4u8 mask)
{
	_16u8 tMask;
	tMask.s0123 = mask;
	return shuffle2(x, y, tMask).s0123;
}

_8i8 __attribute__((overloadable)) shuffle2(_16i8 x, _16i8 y, _8u8 mask)
{
	_16u8 tMask;
	tMask.lo = mask;
	return shuffle2(x, y, tMask).lo;
}

_2u8 __attribute__((overloadable)) shuffle2(_2u8 x, _2u8 y, _2u8 mask)
{
	return as_uchar2(shuffle2(as_char2(x), as_char2(y), mask));
}

_4u8 __attribute__((overloadable)) shuffle2(_2u8 x, _2u8 y, _4u8 mask)
{
	return as_uchar4(shuffle2(as_char2(x), as_char2(y), mask));
}

_8u8 __attribute__((overloadable)) shuffle2(_2u8 x, _2u8 y, _8u8 mask)
{
	return as_uchar8(shuffle2(as_char2(x), as_char2(y), mask));
}

_16u8 __attribute__((overloadable)) shuffle2(_2u8 x, _2u8 y, _16u8 mask)
{
	return as_uchar16(shuffle2(as_char2(x), as_char2(y), mask));
}

_2u8 __attribute__((overloadable)) shuffle2(_4u8 x, _4u8 y, _2u8 mask)
{
	return as_uchar2(shuffle2(as_char4(x), as_char4(y), mask));
}

_4u8 __attribute__((overloadable)) shuffle2(_4u8 x, _4u8 y, _4u8 mask)
{
	return as_uchar4(shuffle2(as_char4(x), as_char4(y), mask));
}

_8u8 __attribute__((overloadable)) shuffle2(_4u8 x, _4u8 y, _8u8 mask)
{
	return as_uchar8(shuffle2(as_char4(x), as_char4(y), mask));
}

_16u8 __attribute__((overloadable)) shuffle2(_4u8 x, _4u8 y, _16u8 mask)
{
	return as_uchar16(shuffle2(as_char4(x), as_char4(y), mask));
}

_2u8 __attribute__((overloadable)) shuffle2(_8u8 x, _8u8 y, _2u8 mask)
{
	return as_uchar2(shuffle2(as_char8(x), as_char8(y), mask));
}

_4u8 __attribute__((overloadable)) shuffle2(_8u8 x, _8u8 y, _4u8 mask)
{
	return as_uchar4(shuffle2(as_char8(x), as_char8(y), mask));
}

_8u8 __attribute__((overloadable)) shuffle2(_8u8 x, _8u8 y, _8u8 mask)
{
	return as_uchar8(shuffle2(as_char8(x), as_char8(y), mask));
}

_16u8 __attribute__((overloadable)) shuffle2(_8u8 x, _8u8 y, _16u8 mask)
{
	return as_uchar16(shuffle2(as_char8(x), as_char8(y), mask));
}

_2u8 __attribute__((overloadable)) shuffle2(_16u8 x, _16u8 y, _2u8 mask)
{
	return as_uchar2(shuffle2(as_char16(x), as_char16(y), mask));
}

_4u8 __attribute__((overloadable)) shuffle2(_16u8 x, _16u8 y, _4u8 mask)
{
	return as_uchar4(shuffle2(as_char16(x), as_char16(y), mask));
}

_8u8 __attribute__((overloadable)) shuffle2(_16u8 x, _16u8 y, _8u8 mask)
{
	return as_uchar8(shuffle2(as_char16(x), as_char16(y), mask));
}

_16u8 __attribute__((overloadable)) shuffle2(_16u8 x, _16u8 y, _16u8 mask)
{
	return as_uchar16(shuffle2(as_char16(x), as_char16(y), mask));
}



_8i16 __attribute__((overloadable)) shuffle2(_2i16 x, _2i16 y, _8u16 mask)
{
	_4i16 tX;
	tX.s01 = x;
	tX.s23 = y;

	return shuffle(tX, mask);
}

_2i16 __attribute__((overloadable)) shuffle2(_2i16 x, _2i16 y, _2u16 mask)
{
	_8u16 tMask;
	tMask.s01 = mask;
	return shuffle2(x, y, tMask).s01;
}

_4i16 __attribute__((overloadable)) shuffle2(_2i16 x, _2i16 y, _4u16 mask)
{
	_8u16 tMask;
	tMask.s0123 = mask;
	return shuffle2(x, y, tMask).s0123;
}

_16i16 __attribute__((overloadable)) shuffle2(_2i16 x, _2i16 y, _16u16 mask)
{
	_16i16 res;

	res.lo = shuffle2(x, y, mask.lo);
	res.hi = shuffle2(x, y, mask.hi);
	
	return res;
}

_8i16 __attribute__((overloadable)) shuffle2(_4i16 x, _4i16 y, _8u16 mask)
{
	_8i16 tX;
	tX.s0123 = x;
	tX.s4567 = y;

	return shuffle(tX, mask);
}

_2i16 __attribute__((overloadable)) shuffle2(_4i16 x, _4i16 y, _2u16 mask)
{
	_8u16 tMask;
	tMask.s01 = mask;
	return shuffle2(x, y, tMask).s01;
}

_4i16 __attribute__((overloadable)) shuffle2(_4i16 x, _4i16 y, _4u16 mask)
{
	_8u16 tMask;
	tMask.s0123 = mask;
	return shuffle2(x, y, tMask).s0123;
}

_16i16 __attribute__((overloadable)) shuffle2(_4i16 x, _4i16 y, _16u16 mask)
{
	_16i16 res;

	res.lo = shuffle2(x, y, mask.lo);
	res.hi = shuffle2(x, y, mask.hi);
	
	return res;
}

_8i16 __attribute__((overloadable)) shuffle2(_8i16 x, _8i16 y, _8u16 mask)
{
	_8i16 res1;
	_8i16 res2;
	_8u16 t1;
	_8u16 t2;

	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask,__m128i),
             __builtin_astype(shuffle_short16_mask, __m128i)),
           _8u16);

  t1 = __builtin_astype(
         _mm_adds_epu8(
           __builtin_astype(mask,__m128i),
           __builtin_astype((ushort8)(56),__m128i)),
         _8u16);
  t2 = __builtin_astype(
         _mm_adds_epu8(
           __builtin_astype(mask - (ushort8)(8),__m128i),
           __builtin_astype((ushort8)(56),__m128i)),
         _8u16);

	res1 = __builtin_astype(
           SHUFFLE_EPI16(
             __builtin_astype(x,__m128i),
             __builtin_astype(t1,__m128i)),
           _8i16);
	res2 = __builtin_astype(
           SHUFFLE_EPI16(
             __builtin_astype(y,__m128i),
             __builtin_astype(t2,__m128i)),
           _8i16);

	return __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res1,__m128i),
             __builtin_astype(res2,__m128i)),
           _8i16);
}

_2i16 __attribute__((overloadable)) shuffle2(_8i16 x, _8i16 y, _2u16 mask)
{
	_8u16 tMask;
	tMask.s01 = mask;
	return shuffle2(x, y, tMask).s01;
}

_4i16 __attribute__((overloadable)) shuffle2(_8i16 x, _8i16 y, _4u16 mask)
{
	_8u16 tMask;
	tMask.s0123 = mask;
	return shuffle2(x, y, tMask).s0123;
}

_16i16 __attribute__((overloadable)) shuffle2(_8i16 x, _8i16 y, _16u16 mask)
{	
	_16i16 res;

	res.lo = shuffle2(x, y, mask.lo);
	res.hi = shuffle2(x, y, mask.hi);

	return res;
}

_8i16 __attribute__((overloadable)) shuffle2(_16i16 x, _16i16 y, _8u16 mask)
{
	_8i16 res1;
	_8i16 res2;
	_8i16 res3;
	_8i16 res4;
	_8u16 t1;
	_8u16 t2;
	_8u16 t3;
	_8u16 t4;

	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask,__m128i),
             __builtin_astype(shuffle_short32_mask, __m128i)),
           _8u16);

    t1 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask,__m128i),
             __builtin_astype((ushort8)(56),__m128i)),
           _8u16);
    t2 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask - (ushort8)(8),__m128i),
             __builtin_astype((ushort8)(56),__m128i)),
           _8u16);
    t3 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask - (ushort8)(16),__m128i),
             __builtin_astype((ushort8)(56),__m128i)),
           _8u16);
    t4 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask - (ushort8)(24),__m128i),
             __builtin_astype((ushort8)(56),__m128i)),
           _8u16);

	res1 = __builtin_astype(
           SHUFFLE_EPI16(
             __builtin_astype(x.lo,__m128i),
             __builtin_astype(t1,__m128i)),
           _8i16);
	res2 = __builtin_astype(
           SHUFFLE_EPI16(
             __builtin_astype(x.hi,__m128i),
             __builtin_astype(t2,__m128i)),
           _8i16);
	res3 = __builtin_astype(
           SHUFFLE_EPI16(
             __builtin_astype(y.lo,__m128i),
             __builtin_astype(t3,__m128i)),
           _8i16);
	res4 = __builtin_astype(
           SHUFFLE_EPI16(
             __builtin_astype(y.hi,__m128i),
             __builtin_astype(t4,__m128i)),
           _8i16);
	res1 = __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res1,__m128i),
             __builtin_astype(res2,__m128i)),
           _8i16);
	res2 = __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res3,__m128i),
             __builtin_astype(res4,__m128i)),
           _8i16);

	return __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res1,__m128i),
             __builtin_astype(res2,__m128i)),
           _8i16);
}

_2i16 __attribute__((overloadable)) shuffle2(_16i16 x, _16i16 y, _2u16 mask)
{
	_8u16 tMask;
	tMask.s01 = mask;
	return shuffle2(x, y, tMask).s01;
}

_4i16 __attribute__((overloadable)) shuffle2(_16i16 x, _16i16 y, _4u16 mask)
{
	_8u16 tMask;
	tMask.s0123 = mask;
	return shuffle2(x, y, tMask).s0123;
}

_16i16 __attribute__((overloadable)) shuffle2(_16i16 x, _16i16 y, _16u16 mask)
{
	_16i16 res;

	res.lo = shuffle2(x, y, mask.lo);
	res.hi = shuffle2(x, y, mask.hi);

	return res;
}

_2u16 __attribute__((overloadable)) shuffle2(_2u16 x, _2u16 y, _2u16 mask)
{
	return as_ushort2(shuffle2(as_short2(x), as_short2(y), mask));
}

_4u16 __attribute__((overloadable)) shuffle2(_2u16 x, _2u16 y, _4u16 mask)
{
	return as_ushort4(shuffle2(as_short2(x), as_short2(y), mask));
}

_8u16 __attribute__((overloadable)) shuffle2(_2u16 x, _2u16 y, _8u16 mask)
{
	return as_ushort8(shuffle2(as_short2(x), as_short2(y), mask));
}

_16u16 __attribute__((overloadable)) shuffle2(_2u16 x, _2u16 y, _16u16 mask)
{
	return as_ushort16(shuffle2(as_short2(x), as_short2(y), mask));
}

_2u16 __attribute__((overloadable)) shuffle2(_4u16 x, _4u16 y, _2u16 mask)
{
	return as_ushort2(shuffle2(as_short4(x), as_short4(y), mask));
}

_4u16 __attribute__((overloadable)) shuffle2(_4u16 x, _4u16 y, _4u16 mask)
{
	return as_ushort4(shuffle2(as_short4(x), as_short4(y), mask));
}

_8u16 __attribute__((overloadable)) shuffle2(_4u16 x, _4u16 y, _8u16 mask)
{
	return as_ushort8(shuffle2(as_short4(x), as_short4(y), mask));
}

_16u16 __attribute__((overloadable)) shuffle2(_4u16 x, _4u16 y, _16u16 mask)
{
	return as_ushort16(shuffle2(as_short4(x), as_short4(y), mask));
}

_2u16 __attribute__((overloadable)) shuffle2(_8u16 x, _8u16 y, _2u16 mask)
{
	return as_ushort2(shuffle2(as_short8(x), as_short8(y), mask));
}

_4u16 __attribute__((overloadable)) shuffle2(_8u16 x, _8u16 y, _4u16 mask)
{
	return as_ushort4(shuffle2(as_short8(x), as_short8(y), mask));
}

_8u16 __attribute__((overloadable)) shuffle2(_8u16 x, _8u16 y, _8u16 mask)
{
	return as_ushort8(shuffle2(as_short8(x), as_short8(y), mask));
}

_16u16 __attribute__((overloadable)) shuffle2(_8u16 x, _8u16 y, _16u16 mask)
{
	return as_ushort16(shuffle2(as_short8(x), as_short8(y), mask));
}

_2u16 __attribute__((overloadable)) shuffle2(_16u16 x, _16u16 y, _2u16 mask)
{
	return as_ushort2(shuffle2(as_short16(x), as_short16(y), mask));
}

_4u16 __attribute__((overloadable)) shuffle2(_16u16 x, _16u16 y, _4u16 mask)
{
	return as_ushort4(shuffle2(as_short16(x), as_short16(y), mask));
}

_8u16 __attribute__((overloadable)) shuffle2(_16u16 x, _16u16 y, _8u16 mask)
{
	return as_ushort8(shuffle2(as_short16(x), as_short16(y), mask));
}

_16u16 __attribute__((overloadable)) shuffle2(_16u16 x, _16u16 y, _16u16 mask)
{
	return as_ushort16(shuffle2(as_short16(x), as_short16(y), mask));
}


_4i32 __attribute__((overloadable)) shuffle2(_2i32 x, _2i32 y, _4u32 mask)
{
	_4i32 tX;
	tX.s01 = x;
	tX.s23 = y;

	return shuffle(tX, mask);
}

_2i32 __attribute__((overloadable)) shuffle2(_2i32 x, _2i32 y, _2u32 mask)
{
	_4u32 tMask;
	tMask.s01 = mask;
	return shuffle2(x, y, tMask).s01;
}
#if defined (__AVX2__) || defined (__AVX__)
_8i32 __attribute__((overloadable)) shuffle2(_2i32 x, _2i32 y, _8u32 mask)
{
	return shuffle((_4i32)(x,y), mask);
}

_16i32 __attribute__((overloadable)) shuffle2(_2i32 x, _2i32 y, _16u32 mask)
{
	_16i32 res;

	res.lo = shuffle((_4i32)(x, y), mask.lo);
	res.hi = shuffle((_4i32)(x, y), mask.hi);
	
	return res;
}

_8i32 __attribute__((overloadable)) shuffle2(_4i32 x, _4i32 y, _8u32 mask)
{

	return shuffle((_8i32)(x, y), mask);
}

_16i32 __attribute__((overloadable)) shuffle2(_4i32 x, _4i32 y, _16u32 mask)
{
	_16i32 res;

	res.lo = shuffle((_8i32)(x, y), mask.lo);
	res.hi = shuffle((_8i32)(x, y), mask.hi);
	
	return res;
}

_4i64 __attribute__((overloadable)) shuffle2(_2i64 x, _2i64 y, _4u64 mask)
{

	return shuffle((_4i64)(x,y), mask);
}

_8i64 __attribute__((overloadable)) shuffle2(_2i64 x, _2i64 y, _8u64 mask)
{
	_8i64 res;
	
	res.lo = shuffle((_4i64)(x, y), mask.lo);
	res.hi = shuffle((_4i64)(x, y), mask.hi);
	
	return res;
}

_16i64 __attribute__((overloadable)) shuffle2(_2i64 x, _2i64 y, _16u64 mask)
{
	_16i64 res;
	res.lo.lo = shuffle((_4i64)(x, y), mask.lo.lo);
	res.lo.hi = shuffle((_4i64)(x, y), mask.lo.hi);
	res.hi.lo = shuffle((_4i64)(x, y), mask.hi.lo);
	res.hi.hi = shuffle((_4i64)(x, y), mask.hi.hi);
	
	return res;
}

#else
_8i32 __attribute__((overloadable)) shuffle2(_2i32 x, _2i32 y, _8u32 mask)
{
	_8i32 res;

	res.lo = shuffle2(x, y, mask.lo);
	res.hi = shuffle2(x, y, mask.hi);
	
	return res;
}

_16i32 __attribute__((overloadable)) shuffle2(_2i32 x, _2i32 y, _16u32 mask)
{
	_16i32 res;

	res.lo.lo = shuffle2(x, y, mask.lo.lo);
	res.lo.hi = shuffle2(x, y, mask.lo.hi);
	res.hi.lo = shuffle2(x, y, mask.hi.lo);
	res.hi.hi = shuffle2(x, y, mask.hi.hi);
	
	return res;
}

_8i32 __attribute__((overloadable)) shuffle2(_4i32 x, _4i32 y, _8u32 mask)
{
	_8i32 res;

	res.lo = shuffle2(x, y, mask.lo);
	res.hi = shuffle2(x, y, mask.hi);
	
	return res;
}

_16i32 __attribute__((overloadable)) shuffle2(_4i32 x, _4i32 y, _16u32 mask)
{
	_16i32 res;

	res.lo.lo = shuffle2(x, y, mask.lo.lo);
	res.lo.hi = shuffle2(x, y, mask.lo.hi);
	res.hi.lo = shuffle2(x, y, mask.hi.lo);
	res.hi.hi = shuffle2(x, y, mask.hi.hi);
	
	return res;
}

_4i64 __attribute__((overloadable)) shuffle2(_2i64 x, _2i64 y, _4u64 mask)
{
	_4i64 res;
	_1i64 xVec[4];
	memcpy(&xVec[0], &x, sizeof(_2i64));
	memcpy(&xVec[2], &y, sizeof(_2i64));

	res.s0 = xVec[mask.s0 & 3];
	res.s1 = xVec[mask.s1 & 3];
	res.s2 = xVec[mask.s2 & 3];
	res.s3 = xVec[mask.s3 & 3];

	return res;
}

_8i64 __attribute__((overloadable)) shuffle2(_2i64 x, _2i64 y, _8u64 mask)
{
	_8i64 res;
	_1i64 xVec[4];
	memcpy(&xVec[0], &x, sizeof(_2i64));
	memcpy(&xVec[2], &y, sizeof(_2i64));

	res.s0 = xVec[mask.s0 & 3];
	res.s1 = xVec[mask.s1 & 3];
	res.s2 = xVec[mask.s2 & 3];
	res.s3 = xVec[mask.s3 & 3];
	res.s4 = xVec[mask.s4 & 3];
	res.s5 = xVec[mask.s5 & 3];
	res.s6 = xVec[mask.s6 & 3];
	res.s7 = xVec[mask.s7 & 3];

	return res;
}

_16i64 __attribute__((overloadable)) shuffle2(_2i64 x, _2i64 y, _16u64 mask)
{
	_16i64 res;
	_1i64 xVec[4];
	memcpy(&xVec[0], &x, sizeof(_2i64));
	memcpy(&xVec[2], &y, sizeof(_2i64));

	res.s0 = xVec[mask.s0 & 3];
	res.s1 = xVec[mask.s1 & 3];
	res.s2 = xVec[mask.s2 & 3];
	res.s3 = xVec[mask.s3 & 3];
	res.s4 = xVec[mask.s4 & 3];
	res.s5 = xVec[mask.s5 & 3];
	res.s6 = xVec[mask.s6 & 3];
	res.s7 = xVec[mask.s7 & 3];
	res.s8 = xVec[mask.s8 & 3];
	res.s9 = xVec[mask.s9 & 3];
	res.sA = xVec[mask.sA & 3];
	res.sB = xVec[mask.sB & 3];
	res.sC = xVec[mask.sC & 3];
	res.sD = xVec[mask.sD & 3];
	res.sE = xVec[mask.sE & 3];
	res.sF = xVec[mask.sF & 3];

	return res;
}

#endif

_4i32 __attribute__((overloadable)) shuffle2(_4i32 x, _4i32 y, _4u32 mask)
{
	_4i32 res1;
	_4i32 res2;

	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask,__m128i),
             __builtin_astype(shuffle_int8_mask, __m128i)),
           _4u32);
  res1 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(x,__m128i),
             __builtin_astype(mask + (uint4)(28),__m128i)),
           _4i32);
  res2 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(y,__m128i),
             __builtin_astype(mask - (uint4)(4),__m128i)),
           _4i32);

	return __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res1,__m128i),
             __builtin_astype(res2,__m128i)),
           _4i32);
}

_2i32 __attribute__((overloadable)) shuffle2(_4i32 x, _4i32 y, _2u32 mask)
{
	_4u32 tMask;
	tMask.s01 = mask;
	return shuffle2(x, y, tMask).s01;
}

_4i32 __attribute__((overloadable)) shuffle2(_8i32 x, _8i32 y, _4u32 mask)
{
	_4i32 res1;
	_4i32 res2;
	_4i32 res3;
	_4i32 res4;
	_4u32 t1;
	_4u32 t2;
	_4u32 t3;
	_4u32 t4;

	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask,__m128i),
             __builtin_astype(shuffle_int16_mask, __m128i)),
           _4u32);

  t1 = __builtin_astype(
         _mm_adds_epu8(
           __builtin_astype(mask,__m128i),
           __builtin_astype((uint4)(28),__m128i)),
         _4u32);
  t2 = __builtin_astype(
         _mm_adds_epu8(
           __builtin_astype(mask - (uint4)(4),__m128i),
           __builtin_astype((uint4)(28),__m128i)),
         _4u32);
  t3 = __builtin_astype(
         _mm_adds_epu8(
           __builtin_astype(mask - (uint4)(8),__m128i),
           __builtin_astype((uint4)(28),__m128i)),
         _4u32);
  t4 = __builtin_astype(
         _mm_adds_epu8(
           __builtin_astype(mask - (uint4)(12),__m128i),
           __builtin_astype((uint4)(28),__m128i)),
         _4u32);

	res1 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(x.lo,__m128i),
             __builtin_astype(t1,__m128i)),
           _4i32);
	res2 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(x.hi,__m128i),
             __builtin_astype(t2,__m128i)),
           _4i32);
	res3 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(y.lo,__m128i),
             __builtin_astype(t3,__m128i)),
           _4i32);
	res4 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(y.hi,__m128i),
             __builtin_astype(t4,__m128i)),
           _4i32);
	res1 = __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res1,__m128i),
             __builtin_astype(res2,__m128i)),
           _4i32);
	res2 = __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res3,__m128i),
             __builtin_astype(res4,__m128i)),
           _4i32);

	return __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res1,__m128i),
             __builtin_astype(res2,__m128i)),
           _4i32);
}

_2i32 __attribute__((overloadable)) shuffle2(_8i32 x, _8i32 y, _2u32 mask)
{
	_4u32 tMask;
	tMask.s01 = mask;
	return shuffle2(x, y, tMask).s01;
}

_8i32 __attribute__((overloadable)) shuffle2(_8i32 x, _8i32 y, _8u32 mask)
{
	_8i32 res;

	res.lo = shuffle2(x, y, mask.lo);
	res.hi = shuffle2(x, y, mask.hi);
	
	return res;
}

_16i32 __attribute__((overloadable)) shuffle2(_8i32 x, _8i32 y, _16u32 mask)
{
	_16i32 res;

	res.lo.lo = shuffle2(x, y, mask.lo.lo);
	res.lo.hi = shuffle2(x, y, mask.lo.hi);
	res.hi.lo = shuffle2(x, y, mask.hi.lo);
	res.hi.hi = shuffle2(x, y, mask.hi.hi);
	
	return res;
}


_4i32 __attribute__((overloadable)) shuffle2(_16i32 x, _16i32 y, _4u32 mask)
{
	_4i32 res1;
	_4i32 res2;
	_4i32 res3;
	_4i32 res4;
	_4i32 res5;
	_4i32 res6;
	_4i32 res7;
	_4i32 res8;
	_4u32 t1;
	_4u32 t2;
	_4u32 t3;
	_4u32 t4;
	_4u32 t5;
	_4u32 t6;
	_4u32 t7;
	_4u32 t8;

	mask = __builtin_astype(
           _mm_and_si128(
             __builtin_astype(mask,__m128i),
             __builtin_astype(shuffle_int32_mask, __m128i)),
           _4u32);

    t1 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask,__m128i),
             __builtin_astype((uint4)(28),__m128i)),
           _4u32);
    t2 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask - (uint4)(4),__m128i),
             __builtin_astype((uint4)(28),__m128i)),
           _4u32);
    t3 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask - (uint4)(8),__m128i),
             __builtin_astype((uint4)(28),__m128i)),
           _4u32);
    t4 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask - (uint4)(12),__m128i),
             __builtin_astype((uint4)(28),__m128i)),
           _4u32);
    t5 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask - (uint4)(16),__m128i),
             __builtin_astype((uint4)(28),__m128i)),
           _4u32);
    t6 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask - (uint4)(20),__m128i),
             __builtin_astype((uint4)(28),__m128i)),
           _4u32);
    t7 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask - (uint4)(24),__m128i),
             __builtin_astype((uint4)(28),__m128i)),
           _4u32);
    t8 = __builtin_astype(
           _mm_adds_epu8(
             __builtin_astype(mask - (uint4)(28),__m128i),
             __builtin_astype((uint4)(28),__m128i)),
           _4u32);

	res1 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(x.lo.lo,__m128i),
             __builtin_astype(t1,__m128i)),
           _4i32);
	res2 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(x.lo.hi,__m128i),
             __builtin_astype(t2,__m128i)),
           _4i32);
	res3 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(x.hi.lo,__m128i),
             __builtin_astype(t3,__m128i)),
           _4i32);
	res4 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(x.hi.hi,__m128i),
             __builtin_astype(t4,__m128i)),
           _4i32);
	res5 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(y.lo.lo,__m128i),
             __builtin_astype(t5,__m128i)),
           _4i32);
	res6 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(y.lo.hi,__m128i),
             __builtin_astype(t6,__m128i)),
           _4i32);
	res7 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(y.hi.lo,__m128i),
             __builtin_astype(t7,__m128i)),
           _4i32);
	res8 = __builtin_astype(
           SHUFFLE_EPI32(
             __builtin_astype(y.hi.hi,__m128i),
             __builtin_astype(t8,__m128i)),
           _4i32);

	res1 = __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res1,__m128i),
             __builtin_astype(res2,__m128i)),
           _4i32);
	res2 = __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res3,__m128i),
             __builtin_astype(res4,__m128i)),
           _4i32);
	res3 = __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res5,__m128i),
             __builtin_astype(res6,__m128i)),
           _4i32);
	res4 = __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res7,__m128i),
             __builtin_astype(res8,__m128i)),
           _4i32);

	res1 = __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res1,__m128i),
             __builtin_astype(res2,__m128i)),
           _4i32);
	res2 = __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res3,__m128i),
             __builtin_astype(res4,__m128i)),
           _4i32);

	return __builtin_astype(
           _mm_or_si128(
             __builtin_astype(res1,__m128i),
             __builtin_astype(res2,__m128i)),
           _4i32);
}

_2i32 __attribute__((overloadable)) shuffle2(_16i32 x, _16i32 y, _2u32 mask)
{
	_4u32 tMask;
	tMask.s01 = mask;
	return shuffle2(x, y, tMask).s01;
}

_8i32 __attribute__((overloadable)) shuffle2(_16i32 x, _16i32 y, _8u32 mask)
{
	_8i32 res;

	res.lo = shuffle2(x, y, mask.lo);
	res.hi = shuffle2(x, y, mask.hi);
	
	return res;
}

_16i32 __attribute__((overloadable)) shuffle2(_16i32 x, _16i32 y, _16u32 mask)
{
	_16i32 res;

	res.lo.lo = shuffle2(x, y, mask.lo.lo);
	res.lo.hi = shuffle2(x, y, mask.lo.hi);
	res.hi.lo = shuffle2(x, y, mask.hi.lo);
	res.hi.hi = shuffle2(x, y, mask.hi.hi);
	
	return res;
}


_2u32 __attribute__((overloadable)) shuffle2(_2u32 x, _2u32 y, _2u32 mask)
{
	return as_uint2(shuffle2(as_int2(x), as_int2(y), mask));
}

_4u32 __attribute__((overloadable)) shuffle2(_2u32 x, _2u32 y, _4u32 mask)
{
	return as_uint4(shuffle2(as_int2(x), as_int2(y), mask));
}

_8u32 __attribute__((overloadable)) shuffle2(_2u32 x, _2u32 y, _8u32 mask)
{
	return as_uint8(shuffle2(as_int2(x), as_int2(y), mask));
}

_16u32 __attribute__((overloadable)) shuffle2(_2u32 x, _2u32 y, _16u32 mask)
{
	return as_uint16(shuffle2(as_int2(x), as_int2(y), mask));
}

_2u32 __attribute__((overloadable)) shuffle2(_4u32 x, _4u32 y, _2u32 mask)
{
	return as_uint2(shuffle2(as_int4(x), as_int4(y), mask));
}

_4u32 __attribute__((overloadable)) shuffle2(_4u32 x, _4u32 y, _4u32 mask)
{
	return as_uint4(shuffle2(as_int4(x), as_int4(y), mask));
}

_8u32 __attribute__((overloadable)) shuffle2(_4u32 x, _4u32 y, _8u32 mask)
{
	return as_uint8(shuffle2(as_int4(x), as_int4(y), mask));
}

_16u32 __attribute__((overloadable)) shuffle2(_4u32 x, _4u32 y, _16u32 mask)
{
	return as_uint16(shuffle2(as_int4(x), as_int4(y), mask));
}

_2u32 __attribute__((overloadable)) shuffle2(_8u32 x, _8u32 y, _2u32 mask)
{
	return as_uint2(shuffle2(as_int8(x), as_int8(y), mask));
}

_4u32 __attribute__((overloadable)) shuffle2(_8u32 x, _8u32 y, _4u32 mask)
{
	return as_uint4(shuffle2(as_int8(x), as_int8(y), mask));
}

_8u32 __attribute__((overloadable)) shuffle2(_8u32 x, _8u32 y, _8u32 mask)
{
	return as_uint8(shuffle2(as_int8(x), as_int8(y), mask));
}

_16u32 __attribute__((overloadable)) shuffle2(_8u32 x, _8u32 y, _16u32 mask)
{
	return as_uint16(shuffle2(as_int8(x), as_int8(y), mask));
}

_2u32 __attribute__((overloadable)) shuffle2(_16u32 x, _16u32 y, _2u32 mask)
{
	return as_uint2(shuffle2(as_int16(x), as_int16(y), mask));
}

_4u32 __attribute__((overloadable)) shuffle2(_16u32 x, _16u32 y, _4u32 mask)
{
	return as_uint4(shuffle2(as_int16(x), as_int16(y), mask));
}

_8u32 __attribute__((overloadable)) shuffle2(_16u32 x, _16u32 y, _8u32 mask)
{
	return as_uint8(shuffle2(as_int16(x), as_int16(y), mask));
}

_16u32 __attribute__((overloadable)) shuffle2(_16u32 x, _16u32 y, _16u32 mask)
{
	return as_uint16(shuffle2(as_int16(x), as_int16(y), mask));
}


_2i64 __attribute__((overloadable)) shuffle2(_2i64 x, _2i64 y, _2u64 mask)
{
	_2i64 res;
	_1i64 xVec[4];
	memcpy(&xVec[0], &x, sizeof(_2i64));
	memcpy(&xVec[2], &y, sizeof(_2i64));

	res.s0 = xVec[mask.s0 & 3];
	res.s1 = xVec[mask.s1 & 3];

	return res;
}



_2i64 __attribute__((overloadable)) shuffle2(_4i64 x, _4i64 y, _2u64 mask)
{
	_2i64 res;
	_1i64 xVec[8];
	memcpy(&xVec[0], &x, sizeof(_4i64));
	memcpy(&xVec[4], &y, sizeof(_4i64));

	res.s0 = xVec[mask.s0 & 7];
	res.s1 = xVec[mask.s1 & 7];

	return res;
}

_4i64 __attribute__((overloadable)) shuffle2(_4i64 x, _4i64 y, _4u64 mask)
{
	_4i64 res;
	_1i64 xVec[8];
	memcpy(&xVec[0], &x, sizeof(_4i64));
	memcpy(&xVec[4], &y, sizeof(_4i64));

	res.s0 = xVec[mask.s0 & 7];
	res.s1 = xVec[mask.s1 & 7];
	res.s2 = xVec[mask.s2 & 7];
	res.s3 = xVec[mask.s3 & 7];

	return res;
}


_8i64 __attribute__((overloadable)) shuffle2(_4i64 x, _4i64 y, _8u64 mask)
{
	_8i64 res;
	_1i64 xVec[8];
	memcpy(&xVec[0], &x, sizeof(_4i64));
	memcpy(&xVec[4], &y, sizeof(_4i64));

	res.s0 = xVec[mask.s0 & 7];
	res.s1 = xVec[mask.s1 & 7];
	res.s2 = xVec[mask.s2 & 7];
	res.s3 = xVec[mask.s3 & 7];
	res.s4 = xVec[mask.s4 & 7];
	res.s5 = xVec[mask.s5 & 7];
	res.s6 = xVec[mask.s6 & 7];
	res.s7 = xVec[mask.s7 & 7];

	return res;
}

_16i64 __attribute__((overloadable)) shuffle2(_4i64 x, _4i64 y, _16u64 mask)
{
	_16i64 res;
	_1i64 xVec[8];
	memcpy(&xVec[0], &x, sizeof(_4i64));
	memcpy(&xVec[4], &y, sizeof(_4i64));

	res.s0 = xVec[mask.s0 & 7];
	res.s1 = xVec[mask.s1 & 7];
	res.s2 = xVec[mask.s2 & 7];
	res.s3 = xVec[mask.s3 & 7];
	res.s4 = xVec[mask.s4 & 7];
	res.s5 = xVec[mask.s5 & 7];
	res.s6 = xVec[mask.s6 & 7];
	res.s7 = xVec[mask.s7 & 7];
	res.s8 = xVec[mask.s8 & 7];
	res.s9 = xVec[mask.s9 & 7];
	res.sA = xVec[mask.sA & 7];
	res.sB = xVec[mask.sB & 7];
	res.sC = xVec[mask.sC & 7];
	res.sD = xVec[mask.sD & 7];
	res.sE = xVec[mask.sE & 7];
	res.sF = xVec[mask.sF & 7];

	return res;
}


_2i64 __attribute__((overloadable)) shuffle2(_8i64 x, _8i64 y, _2u64 mask)
{
	_2i64 res;
	_1i64 xVec[16];
	memcpy(&xVec[0], &x, sizeof(_8i64));
	memcpy(&xVec[8], &y, sizeof(_8i64));

	res.s0 = xVec[mask.s0 & 15];
	res.s1 = xVec[mask.s1 & 15];

	return res;
}

_4i64 __attribute__((overloadable)) shuffle2(_8i64 x, _8i64 y, _4u64 mask)
{
	_4i64 res;
	_1i64 xVec[16];
	memcpy(&xVec[0], &x, sizeof(_8i64));
	memcpy(&xVec[8], &y, sizeof(_8i64));

	res.s0 = xVec[mask.s0 & 15];
	res.s1 = xVec[mask.s1 & 15];
	res.s2 = xVec[mask.s2 & 15];
	res.s3 = xVec[mask.s3 & 15];

	return res;
}

_8i64 __attribute__((overloadable)) shuffle2(_8i64 x, _8i64 y, _8u64 mask)
{
	_8i64 res;
	_1i64 xVec[16];
	memcpy(&xVec[0], &x, sizeof(_8i64));
	memcpy(&xVec[8], &y, sizeof(_8i64));

	res.s0 = xVec[mask.s0 & 15];
	res.s1 = xVec[mask.s1 & 15];
	res.s2 = xVec[mask.s2 & 15];
	res.s3 = xVec[mask.s3 & 15];
	res.s4 = xVec[mask.s4 & 15];
	res.s5 = xVec[mask.s5 & 15];
	res.s6 = xVec[mask.s6 & 15];
	res.s7 = xVec[mask.s7 & 15];

	return res;
}

_16i64 __attribute__((overloadable)) shuffle2(_8i64 x, _8i64 y, _16u64 mask)
{
	_16i64 res;
	_1i64 xVec[16];
	memcpy(&xVec[0], &x, sizeof(_8i64));
	memcpy(&xVec[8], &y, sizeof(_8i64));

	res.s0 = xVec[mask.s0 & 15];
	res.s1 = xVec[mask.s1 & 15];
	res.s2 = xVec[mask.s2 & 15];
	res.s3 = xVec[mask.s3 & 15];
	res.s4 = xVec[mask.s4 & 15];
	res.s5 = xVec[mask.s5 & 15];
	res.s6 = xVec[mask.s6 & 15];
	res.s7 = xVec[mask.s7 & 15];
	res.s8 = xVec[mask.s8 & 15];
	res.s9 = xVec[mask.s9 & 15];
	res.sA = xVec[mask.sA & 15];
	res.sB = xVec[mask.sB & 15];
	res.sC = xVec[mask.sC & 15];
	res.sD = xVec[mask.sD & 15];
	res.sE = xVec[mask.sE & 15];
	res.sF = xVec[mask.sF & 15];

	return res;
}


_2i64 __attribute__((overloadable)) shuffle2(_16i64 x, _16i64 y, _2u64 mask)
{
	_2i64 res;
	_1i64 xVec[32];
	memcpy(&xVec[0], &x, sizeof(_16i64));
	memcpy(&xVec[16], &y, sizeof(_16i64));

	res.s0 = xVec[mask.s0 & 31];
	res.s1 = xVec[mask.s1 & 31];

	return res;
}

_4i64 __attribute__((overloadable)) shuffle2(_16i64 x, _16i64 y, _4u64 mask)
{
	_4i64 res;
	_1i64 xVec[32];
	memcpy(&xVec[0], &x, sizeof(_16i64));
	memcpy(&xVec[16], &y, sizeof(_16i64));

	res.s0 = xVec[mask.s0 & 31];
	res.s1 = xVec[mask.s1 & 31];
	res.s2 = xVec[mask.s2 & 31];
	res.s3 = xVec[mask.s3 & 31];

	return res;
}

_8i64 __attribute__((overloadable)) shuffle2(_16i64 x, _16i64 y, _8u64 mask)
{
	_8i64 res;
	_1i64 xVec[32];
	memcpy(&xVec[0], &x, sizeof(_16i64));
	memcpy(&xVec[16], &y, sizeof(_16i64));

	res.s0 = xVec[mask.s0 & 31];
	res.s1 = xVec[mask.s1 & 31];
	res.s2 = xVec[mask.s2 & 31];
	res.s3 = xVec[mask.s3 & 31];
	res.s4 = xVec[mask.s4 & 31];
	res.s5 = xVec[mask.s5 & 31];
	res.s6 = xVec[mask.s6 & 31];
	res.s7 = xVec[mask.s7 & 31];

	return res;
}

_16i64 __attribute__((overloadable)) shuffle2(_16i64 x, _16i64 y, _16u64 mask)
{
	_16i64 res;
	_1i64 xVec[32];
	memcpy(&xVec[0], &x, sizeof(_16i64));
	memcpy(&xVec[16], &y, sizeof(_16i64));

	res.s0 = xVec[mask.s0 & 31];
	res.s1 = xVec[mask.s1 & 31];
	res.s2 = xVec[mask.s2 & 31];
	res.s3 = xVec[mask.s3 & 31];
	res.s4 = xVec[mask.s4 & 31];
	res.s5 = xVec[mask.s5 & 31];
	res.s6 = xVec[mask.s6 & 31];
	res.s7 = xVec[mask.s7 & 31];
	res.s8 = xVec[mask.s8 & 31];
	res.s9 = xVec[mask.s9 & 31];
	res.sA = xVec[mask.sA & 31];
	res.sB = xVec[mask.sB & 31];
	res.sC = xVec[mask.sC & 31];
	res.sD = xVec[mask.sD & 31];
	res.sE = xVec[mask.sE & 31];
	res.sF = xVec[mask.sF & 31];

	return res;
}


_2u64 __attribute__((overloadable)) shuffle2(_2u64 x, _2u64 y, _2u64 mask)
{
	return as_ulong2(shuffle2(as_long2(x), as_long2(y), mask));
}

_4u64 __attribute__((overloadable)) shuffle2(_2u64 x, _2u64 y, _4u64 mask)
{
	return as_ulong4(shuffle2(as_long2(x), as_long2(y), mask));
}

_8u64 __attribute__((overloadable)) shuffle2(_2u64 x, _2u64 y, _8u64 mask)
{
	return as_ulong8(shuffle2(as_long2(x), as_long2(y), mask));
}

_16u64 __attribute__((overloadable)) shuffle2(_2u64 x, _2u64 y, _16u64 mask)
{
	return as_ulong16(shuffle2(as_long2(x), as_long2(y), mask));
}

_2u64 __attribute__((overloadable)) shuffle2(_4u64 x, _4u64 y, _2u64 mask)
{
	return as_ulong2(shuffle2(as_long4(x), as_long4(y), mask));
}

_4u64 __attribute__((overloadable)) shuffle2(_4u64 x, _4u64 y, _4u64 mask)
{
	return as_ulong4(shuffle2(as_long4(x), as_long4(y), mask));
}

_8u64 __attribute__((overloadable)) shuffle2(_4u64 x, _4u64 y, _8u64 mask)
{
	return as_ulong8(shuffle2(as_long4(x), as_long4(y), mask));
}

_16u64 __attribute__((overloadable)) shuffle2(_4u64 x, _4u64 y, _16u64 mask)
{
	return as_ulong16(shuffle2(as_long4(x), as_long4(y), mask));
}

_2u64 __attribute__((overloadable)) shuffle2(_8u64 x, _8u64 y, _2u64 mask)
{
	return as_ulong2(shuffle2(as_long8(x), as_long8(y), mask));
}

_4u64 __attribute__((overloadable)) shuffle2(_8u64 x, _8u64 y, _4u64 mask)
{
	return as_ulong4(shuffle2(as_long8(x), as_long8(y), mask));
}

_8u64 __attribute__((overloadable)) shuffle2(_8u64 x, _8u64 y, _8u64 mask)
{
	return as_ulong8(shuffle2(as_long8(x), as_long8(y), mask));
}

_16u64 __attribute__((overloadable)) shuffle2(_8u64 x, _8u64 y, _16u64 mask)
{
	return as_ulong16(shuffle2(as_long8(x), as_long8(y), mask));
}

_2u64 __attribute__((overloadable)) shuffle2(_16u64 x, _16u64 y, _2u64 mask)
{
	return as_ulong2(shuffle2(as_long16(x), as_long16(y), mask));
}

_4u64 __attribute__((overloadable)) shuffle2(_16u64 x, _16u64 y, _4u64 mask)
{
	return as_ulong4(shuffle2(as_long16(x), as_long16(y), mask));
}

_8u64 __attribute__((overloadable)) shuffle2(_16u64 x, _16u64 y, _8u64 mask)
{
	return as_ulong8(shuffle2(as_long16(x), as_long16(y), mask));
}

_16u64 __attribute__((overloadable)) shuffle2(_16u64 x, _16u64 y, _16u64 mask)
{
	return as_ulong16(shuffle2(as_long16(x), as_long16(y), mask));
}



float2 __attribute__((overloadable)) shuffle2(float2 x, float2 y, _2u32 mask)
{
	return as_float2(shuffle2(as_int2(x), as_int2(y), mask));
}

float4 __attribute__((overloadable)) shuffle2(float2 x, float2 y, _4u32 mask)
{
	return as_float4(shuffle2(as_int2(x), as_int2(y), mask));
}

float8 __attribute__((overloadable)) shuffle2(float2 x, float2 y, _8u32 mask)
{
	return as_float8(shuffle2(as_int2(x), as_int2(y), mask));
}

float16 __attribute__((overloadable)) shuffle2(float2 x, float2 y, _16u32 mask)
{
	return as_float16(shuffle2(as_int2(x), as_int2(y), mask));
}

float2 __attribute__((overloadable)) shuffle2(float4 x, float4 y, _2u32 mask)
{
	return as_float2(shuffle2(as_int4(x), as_int4(y), mask));
}

float4 __attribute__((overloadable)) shuffle2(float4 x, float4 y, _4u32 mask)
{
	return as_float4(shuffle2(as_int4(x), as_int4(y), mask));
}

float8 __attribute__((overloadable)) shuffle2(float4 x, float4 y, _8u32 mask)
{
	return as_float8(shuffle2(as_int4(x), as_int4(y), mask));
}

float16 __attribute__((overloadable)) shuffle2(float4 x, float4 y, _16u32 mask)
{
	return as_float16(shuffle2(as_int4(x), as_int4(y), mask));
}

float2 __attribute__((overloadable)) shuffle2(float8 x, float8 y, _2u32 mask)
{
	return as_float2(shuffle2(as_int8(x), as_int8(y), mask));
}

float4 __attribute__((overloadable)) shuffle2(float8 x, float8 y, _4u32 mask)
{
	return as_float4(shuffle2(as_int8(x), as_int8(y), mask));
}

float8 __attribute__((overloadable)) shuffle2(float8 x, float8 y, _8u32 mask)
{
	return as_float8(shuffle2(as_int8(x), as_int8(y), mask));
}

float16 __attribute__((overloadable)) shuffle2(float8 x, float8 y, _16u32 mask)
{
	return as_float16(shuffle2(as_int8(x), as_int8(y), mask));
}

float2 __attribute__((overloadable)) shuffle2(float16 x, float16 y, _2u32 mask)
{
	return as_float2(shuffle2(as_int16(x), as_int16(y), mask));
}

float4 __attribute__((overloadable)) shuffle2(float16 x, float16 y, _4u32 mask)
{
	return as_float4(shuffle2(as_int16(x), as_int16(y), mask));
}

float8 __attribute__((overloadable)) shuffle2(float16 x, float16 y, _8u32 mask)
{
	return as_float8(shuffle2(as_int16(x), as_int16(y), mask));
}

float16 __attribute__((overloadable)) shuffle2(float16 x, float16 y, _16u32 mask)
{
	return as_float16(shuffle2(as_int16(x), as_int16(y), mask));
}


double2 __attribute__((overloadable)) shuffle2(double2 x, double2 y, _2u64 mask)
{
	return as_double2(shuffle2(as_long2(x), as_long2(y), mask));
}

double4 __attribute__((overloadable)) shuffle2(double2 x, double2 y, _4u64 mask)
{
	return as_double4(shuffle2(as_long2(x), as_long2(y), mask));
}

double8 __attribute__((overloadable)) shuffle2(double2 x, double2 y, _8u64 mask)
{
	return as_double8(shuffle2(as_long2(x), as_long2(y), mask));
}

double16 __attribute__((overloadable)) shuffle2(double2 x, double2 y, _16u64 mask)
{
	return as_double16(shuffle2(as_long2(x), as_long2(y), mask));
}

double2 __attribute__((overloadable)) shuffle2(double4 x, double4 y, _2u64 mask)
{
	return as_double2(shuffle2(as_long4(x), as_long4(y), mask));
}

double4 __attribute__((overloadable)) shuffle2(double4 x, double4 y, _4u64 mask)
{
	return as_double4(shuffle2(as_long4(x), as_long4(y), mask));
}

double8 __attribute__((overloadable)) shuffle2(double4 x, double4 y, _8u64 mask)
{
	return as_double8(shuffle2(as_long4(x), as_long4(y), mask));
}

double16 __attribute__((overloadable)) shuffle2(double4 x, double4 y, _16u64 mask)
{
	return as_double16(shuffle2(as_long4(x), as_long4(y), mask));
}

double2 __attribute__((overloadable)) shuffle2(double8 x, double8 y, _2u64 mask)
{
	return as_double2(shuffle2(as_long8(x), as_long8(y), mask));
}

double4 __attribute__((overloadable)) shuffle2(double8 x, double8 y, _4u64 mask)
{
	return as_double4(shuffle2(as_long8(x), as_long8(y), mask));
}

double8 __attribute__((overloadable)) shuffle2(double8 x, double8 y, _8u64 mask)
{
	return as_double8(shuffle2(as_long8(x), as_long8(y), mask));
}

double16 __attribute__((overloadable)) shuffle2(double8 x, double8 y, _16u64 mask)
{
	return as_double16(shuffle2(as_long8(x), as_long8(y), mask));
}

double2 __attribute__((overloadable)) shuffle2(double16 x, double16 y, _2u64 mask)
{
	return as_double2(shuffle2(as_long16(x), as_long16(y), mask));
}

double4 __attribute__((overloadable)) shuffle2(double16 x, double16 y, _4u64 mask)
{
	return as_double4(shuffle2(as_long16(x), as_long16(y), mask));
}

double8 __attribute__((overloadable)) shuffle2(double16 x, double16 y, _8u64 mask)
{
	return as_double8(shuffle2(as_long16(x), as_long16(y), mask));
}

double16 __attribute__((overloadable)) shuffle2(double16 x, double16 y, _16u64 mask)
{
	return as_double16(shuffle2(as_long16(x), as_long16(y), mask));
}


void mem_fence(cl_mem_fence_flags flags)
{
	_mm_mfence();
}

void read_mem_fence(cl_mem_fence_flags flags)
{
	_mm_lfence();
}

void write_mem_fence(cl_mem_fence_flags flags)
{
	_mm_sfence();
}

#ifdef __cplusplus
}
#endif

#endif // defined (__MIC__) || defined(__MIC2__)
