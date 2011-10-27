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
#ifdef __cplusplus
extern "C" {
#endif

#define ALIGN16 __attribute__((aligned(16)))
#include <intrin.h>

#include "cl_types2.h"

void* memcpy(void*, const void*, size_t);

//shuffle masks
ALIGN16 const _1i8 shuffle_char2_mask[] =	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
ALIGN16 const _1i8 shuffle_char4_mask[] =	{3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
ALIGN16 const _1i8 shuffle_char8_mask[] =	{7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
ALIGN16 const _1i8 shuffle_char16_mask[] =	{15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15};

ALIGN16 const _1i16 shuffle_short2_mask[] =	{1, 1, 1, 1, 1, 1, 1, 1};
ALIGN16 const _1i16 shuffle_short4_mask[] =	{3, 3, 3, 3, 3, 3, 3, 3};
ALIGN16 const _1i16 shuffle_short8_mask[] =	{7, 7, 7, 7, 7, 7, 7, 7};
ALIGN16 const _1i16 shuffle_short16_mask[] =	{15, 15, 15, 15, 15, 15, 15, 15};
ALIGN16 const _1i16 shuffle_short32_mask[] =	{31, 31, 31, 31, 31, 31, 31, 31};

ALIGN16 const _1i32 shuffle_int2_mask[] =	{1, 1, 1, 1};
ALIGN16 const _1i32 shuffle_int4_mask[] =	{3, 3, 3, 3};
ALIGN16 const _1i32 shuffle_int8_mask[] =	{7, 7, 7, 7};
ALIGN16 const _1i32 shuffle_int16_mask[] =	{15, 15, 15, 15};
ALIGN16 const _1i32 shuffle_int32_mask[] =	{31, 31, 31, 31};

ALIGN16 const _1i64 shuffle_long2_mask[] =	{1, 1};
ALIGN16 const _1i64 shuffle_long4_mask[] =	{3, 3};
ALIGN16 const _1i64 shuffle_long8_mask[] =	{7, 7};
ALIGN16 const _1i64 shuffle_long16_mask[] =	{15, 15};


//shuffle2 masks
ALIGN16 const _1i8 shuffle2_char2_mask[] =	{0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81};
ALIGN16 const _1i8 shuffle2_char4_mask[] =	{0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83};
ALIGN16 const _1i8 shuffle2_char8_mask[] =	{0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87};
ALIGN16 const _1i8 shuffle2_char16_mask[] =	{0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F, 0x8F};

ALIGN16 const _1i16 shuffle2_short2_mask[] =	{0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41};
ALIGN16 const _1i16 shuffle2_short4_mask[] =	{0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43};
ALIGN16 const _1i16 shuffle2_short8_mask[] =	{0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47};
ALIGN16 const _1i16 shuffle2_short16_mask[] ={0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F};
ALIGN16 const _1i16 shuffle2_short32_mask[] ={0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F, 0x4F};


ALIGN16 const _1i8 shuffle_epi16_smask[] = {0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14};
ALIGN16 const _1i8 shuffle_epi16_amask[] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};

ALIGN16 const _1i8 shuffle_epi32_smask[] = {0, 0, 0, 0, 4, 4, 4, 4, 8, 8, 8, 8, 12, 12, 12, 12};
ALIGN16 const _1i8 shuffle_epi32_amask[] = {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3};

ALIGN16 const _1i8 shuffle_epi64_smask[] = {0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8};
ALIGN16 const _1i8 shuffle_epi64_amask[] = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7};


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
	mask = SHUFFLE_EPI8(mask, *((__m128i *)shuffle_epi16_smask));
	mask = _mm_slli_epi16(mask, 1);
	mask = _mm_adds_epu8(mask, *((__m128i *)shuffle_epi16_amask));
	return SHUFFLE_EPI8(x, mask);		
}

__m128i SHUFFLE_EPI32(__m128i x, __m128i mask)
{
	mask = SHUFFLE_EPI8(mask, *((__m128i *)shuffle_epi32_smask));
	mask = _mm_slli_epi16(mask, 2);
	mask = _mm_adds_epu8(mask, *((__m128i *)shuffle_epi32_amask));
	return SHUFFLE_EPI8(x, mask);		
}

__m128i SHUFFLE_EPI64(__m128i x, __m128i mask)
{
	mask = SHUFFLE_EPI8(mask, *((__m128i *)shuffle_epi64_smask));
	mask = _mm_slli_epi16(mask, 3);
	mask = _mm_adds_epu8(mask, *((__m128i *)shuffle_epi64_amask));
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
	mask = _mm_and_si128(mask, *((__m128*)shuffle_char2_mask));
	tX = SHUFFLE_EPI8(tX, mask);
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
	mask = _mm_and_si128(mask, *((__m128*)shuffle_char4_mask));
	tX = SHUFFLE_EPI8(tX, mask);
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
	mask = _mm_and_si128(mask, *((__m128*)shuffle_char8_mask));
	tX = SHUFFLE_EPI8(tX, mask);
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
	mask = _mm_and_si128(mask, *((__m128*)shuffle_char16_mask));
	x = SHUFFLE_EPI8(x, mask);
	return x;
}

_2u8 __attribute__((overloadable)) shuffle(_2u8 x, _2u8 mask)
{
	return (_2u8)shuffle((_2i8)x, mask);
}

_4u8 __attribute__((overloadable)) shuffle(_2u8 x, _4u8 mask)
{
	return (_4u8)shuffle((_2i8)x, mask);
}

_8u8 __attribute__((overloadable)) shuffle(_2u8 x, _8u8 mask)
{
	return (_8u8)shuffle((_2i8)x, mask);
}

_16u8 __attribute__((overloadable)) shuffle(_2u8 x, _16u8 mask)
{
	return (_16u8)shuffle((_2i8)x, mask);
}

_2u8 __attribute__((overloadable)) shuffle(_4u8 x, _2u8 mask)
{
	return (_2u8)shuffle((_4i8)x, mask);
}

_4u8 __attribute__((overloadable)) shuffle(_4u8 x, _4u8 mask)
{
	return (_4u8)shuffle((_4i8)x, mask);
}

_8u8 __attribute__((overloadable)) shuffle(_4u8 x, _8u8 mask)
{
	return (_8u8)shuffle((_4i8)x, mask);
}

_16u8 __attribute__((overloadable)) shuffle(_4u8 x, _16u8 mask)
{
	return (_16u8)shuffle((_4i8)x, mask);
}

_2u8 __attribute__((overloadable)) shuffle(_8u8 x, _2u8 mask)
{
	return (_2u8)shuffle((_8i8)x, mask);
}

_4u8 __attribute__((overloadable)) shuffle(_8u8 x, _4u8 mask)
{
	return (_4u8)shuffle((_8i8)x, mask);
}

_8u8 __attribute__((overloadable)) shuffle(_8u8 x, _8u8 mask)
{
	return (_8u8)shuffle((_8i8)x, mask);
}

_16u8 __attribute__((overloadable)) shuffle(_8u8 x, _16u8 mask)
{
	return (_16u8)shuffle((_8i8)x, mask);
}

_2u8 __attribute__((overloadable)) shuffle(_16u8 x, _2u8 mask)
{
	return (_2u8)shuffle((_16i8)x, mask);
}

_4u8 __attribute__((overloadable)) shuffle(_16u8 x, _4u8 mask)
{
	return (_4u8)shuffle((_16i8)x, mask);
}

_8u8 __attribute__((overloadable)) shuffle(_16u8 x, _8u8 mask)
{
	return (_8u8)shuffle((_16i8)x, mask);
}

_16u8 __attribute__((overloadable)) shuffle(_16u8 x, _16u8 mask)
{
	return (_16u8)shuffle((_16i8)x, mask);
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
	mask = _mm_and_si128(mask, *((__m128*)shuffle_short2_mask));
	tX = SHUFFLE_EPI16(tX, mask);
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
	mask = _mm_and_si128(mask, *((__m128*)shuffle_short4_mask));
	tX = SHUFFLE_EPI16(tX, mask);
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
	mask = _mm_and_si128(mask, *((__m128*)shuffle_short8_mask));
	x = SHUFFLE_EPI16(x, mask);
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

	mask = _mm_and_si128(mask, *((__m128*)shuffle_short16_mask));

    res1 = SHUFFLE_EPI16(x.lo, mask + (_8u16){56});
    res2 = SHUFFLE_EPI16(x.hi, mask - (_8u16){8});

	return _mm_or_si128(res1, res2);
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
	return (_2u16)shuffle((_2i16)x, mask);
}

_4u16 __attribute__((overloadable)) shuffle(_2u16 x, _4u16 mask)
{
	return (_4u16)shuffle((_2i16)x, mask);
}

_8u16 __attribute__((overloadable)) shuffle(_2u16 x, _8u16 mask)
{
	return (_8u16)shuffle((_2i16)x, mask);
}

_16u16 __attribute__((overloadable)) shuffle(_2u16 x, _16u16 mask)
{
	return (_16u16)shuffle((_2i16)x, mask);
}

_2u16 __attribute__((overloadable)) shuffle(_4u16 x, _2u16 mask)
{
	return (_2u16)shuffle((_4i16)x, mask);
}

_4u16 __attribute__((overloadable)) shuffle(_4u16 x, _4u16 mask)
{
	return (_4u16)shuffle((_4i16)x, mask);
}

_8u16 __attribute__((overloadable)) shuffle(_4u16 x, _8u16 mask)
{
	return (_8u16)shuffle((_4i16)x, mask);
}

_16u16 __attribute__((overloadable)) shuffle(_4u16 x, _16u16 mask)
{
	return (_16u16)shuffle((_4i16)x, mask);
}

_2u16 __attribute__((overloadable)) shuffle(_8u16 x, _2u16 mask)
{
	return (_2u16)shuffle((_8i16)x, mask);
}

_4u16 __attribute__((overloadable)) shuffle(_8u16 x, _4u16 mask)
{
	return (_4u16)shuffle((_8i16)x, mask);
}

_8u16 __attribute__((overloadable)) shuffle(_8u16 x, _8u16 mask)
{
	return (_8u16)shuffle((_8i16)x, mask);
}

_16u16 __attribute__((overloadable)) shuffle(_8u16 x, _16u16 mask)
{
	return (_16u16)shuffle((_8i16)x, mask);
}

_2u16 __attribute__((overloadable)) shuffle(_16u16 x, _2u16 mask)
{
	return (_2u16)shuffle((_16i16)x, mask);
}

_4u16 __attribute__((overloadable)) shuffle(_16u16 x, _4u16 mask)
{
	return (_4u16)shuffle((_16i16)x, mask);
}

_8u16 __attribute__((overloadable)) shuffle(_16u16 x, _8u16 mask)
{
	return (_8u16)shuffle((_16i16)x, mask);
}

_16u16 __attribute__((overloadable)) shuffle(_16u16 x, _16u16 mask)
{
	return (_16u16)shuffle((_16i16)x, mask);
}


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
	mask = _mm_and_si128(mask, *((__m128*)shuffle_int2_mask));
	tX = SHUFFLE_EPI32(tX, mask);
	return tX;
}

_8i32 __attribute__((overloadable)) shuffle(_2i32 x, _8u32 mask)
{
	_8i32 res;

	res.lo = shuffle(x, mask.lo);
	res.hi = shuffle(x, mask.hi);

	return res;
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

_2i32 __attribute__((overloadable)) shuffle(_4i32 x, _2u32 mask)
{
	_4u32 tMask;
	tMask.s01 = mask;
	return shuffle(x, tMask).s01;
}

_4i32 __attribute__((overloadable)) shuffle(_4i32 x, _4u32 mask)
{
	mask = _mm_and_si128(mask, *((__m128*)shuffle_int4_mask));
	x = SHUFFLE_EPI32(x, mask);
	return x;
}

_8i32 __attribute__((overloadable)) shuffle(_4i32 x, _8u32 mask)
{
	_8i32 res;

	res.lo = shuffle(x, mask.lo);
	res.hi = shuffle(x, mask.hi);

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

	mask = _mm_and_si128(mask, *((__m128*)shuffle_int8_mask));
    res1 = SHUFFLE_EPI32(x.lo, mask + (_4u32){28});
    res2 = SHUFFLE_EPI32(x.hi, mask - (_4u32){4});

	return _mm_or_si128(res1, res2);
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

	mask = _mm_and_si128(mask, *((__m128*)shuffle_int16_mask));

    t1 = _mm_adds_epu8(mask, (_4u32){28});
    t2 = _mm_adds_epu8(mask - (_4u32){4}, (_4u32){28});
    t3 = _mm_adds_epu8(mask - (_4u32){8}, (_4u32){28});
    t4 = _mm_adds_epu8(mask - (_4u32){12}, (_4u32){28});
	
	res1 = SHUFFLE_EPI32(x.lo.lo, t1);
	res2 = SHUFFLE_EPI32(x.lo.hi, t2);
	res3 = SHUFFLE_EPI32(x.hi.lo, t3);
	res4 = SHUFFLE_EPI32(x.hi.hi, t4);
	res1 = _mm_or_si128(res1, res2);
	res2 = _mm_or_si128(res3, res4);

	return _mm_or_si128(res1, res2);
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
	return (_2u32)shuffle((_2i32)x, mask);
}

_4u32 __attribute__((overloadable)) shuffle(_2u32 x, _4u32 mask)
{
	return (_4u32)shuffle((_2i32)x, mask);
}

_8u32 __attribute__((overloadable)) shuffle(_2u32 x, _8u32 mask)
{
	return (_8u32)shuffle((_2i32)x, mask);
}

_16u32 __attribute__((overloadable)) shuffle(_2u32 x, _16u32 mask)
{
	return (_16u32)shuffle((_2i32)x, mask);
}

_2u32 __attribute__((overloadable)) shuffle(_4u32 x, _2u32 mask)
{
	return (_2u32)shuffle((_4i32)x, mask);
}

_4u32 __attribute__((overloadable)) shuffle(_4u32 x, _4u32 mask)
{
	return (_4u32)shuffle((_4i32)x, mask);
}

_8u32 __attribute__((overloadable)) shuffle(_4u32 x, _8u32 mask)
{
	return (_8u32)shuffle((_4i32)x, mask);
}

_16u32 __attribute__((overloadable)) shuffle(_4u32 x, _16u32 mask)
{
	return (_16u32)shuffle((_4i32)x, mask);
}

_2u32 __attribute__((overloadable)) shuffle(_8u32 x, _2u32 mask)
{
	return (_2u32)shuffle((_8i32)x, mask);
}

_4u32 __attribute__((overloadable)) shuffle(_8u32 x, _4u32 mask)
{
	return (_4u32)shuffle((_8i32)x, mask);
}

_8u32 __attribute__((overloadable)) shuffle(_8u32 x, _8u32 mask)
{
	return (_8u32)shuffle((_8i32)x, mask);
}

_16u32 __attribute__((overloadable)) shuffle(_8u32 x, _16u32 mask)
{
	return (_16u32)shuffle((_8i32)x, mask);
}

_2u32 __attribute__((overloadable)) shuffle(_16u32 x, _2u32 mask)
{
	return (_2u32)shuffle((_16i32)x, mask);
}

_4u32 __attribute__((overloadable)) shuffle(_16u32 x, _4u32 mask)
{
	return (_4u32)shuffle((_16i32)x, mask);
}

_8u32 __attribute__((overloadable)) shuffle(_16u32 x, _8u32 mask)
{
	return (_8u32)shuffle((_16i32)x, mask);
}

_16u32 __attribute__((overloadable)) shuffle(_16u32 x, _16u32 mask)
{
	return (_16u32)shuffle((_16i32)x, mask);
}


_2i64 __attribute__((overloadable)) shuffle(_2i64 x, _2u64 mask)
{
	mask = _mm_and_si128(mask, *((__m128*)shuffle_long2_mask));
	x = SHUFFLE_EPI64(x, mask);
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

_2i64 __attribute__((overloadable)) shuffle(_4i64 x, _2u64 mask)
{
	_2i64 res;
	_1i64 xVec[4];
	memcpy(xVec, &x, sizeof(_4i64));

	res.s0 = xVec[mask.s0 & 3];
	res.s1 = xVec[mask.s1 & 3];

	return res;
}

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
	return (_2u64)shuffle((_2i64)x, mask);
}

_4u64 __attribute__((overloadable)) shuffle(_2u64 x, _4u64 mask)
{
	return (_4u64)shuffle((_2i64)x, mask);
}

_8u64 __attribute__((overloadable)) shuffle(_2u64 x, _8u64 mask)
{
	return (_8u64)shuffle((_2i64)x, mask);
}

_16u64 __attribute__((overloadable)) shuffle(_2u64 x, _16u64 mask)
{
	return (_16u64)shuffle((_2i64)x, mask);
}

_2u64 __attribute__((overloadable)) shuffle(_4u64 x, _2u64 mask)
{
	return (_2u64)shuffle((_4i64)x, mask);
}

_4u64 __attribute__((overloadable)) shuffle(_4u64 x, _4u64 mask)
{
	return (_4u64)shuffle((_4i64)x, mask);
}

_8u64 __attribute__((overloadable)) shuffle(_4u64 x, _8u64 mask)
{
	return (_8u64)shuffle((_4i64)x, mask);
}

_16u64 __attribute__((overloadable)) shuffle(_4u64 x, _16u64 mask)
{
	return (_16u64)shuffle((_4i64)x, mask);
}

_2u64 __attribute__((overloadable)) shuffle(_8u64 x, _2u64 mask)
{
	return (_2u64)shuffle((_8i64)x, mask);
}

_4u64 __attribute__((overloadable)) shuffle(_8u64 x, _4u64 mask)
{
	return (_4u64)shuffle((_8i64)x, mask);
}

_8u64 __attribute__((overloadable)) shuffle(_8u64 x, _8u64 mask)
{
	return (_8u64)shuffle((_8i64)x, mask);
}

_16u64 __attribute__((overloadable)) shuffle(_8u64 x, _16u64 mask)
{
	return (_16u64)shuffle((_8i64)x, mask);
}

_2u64 __attribute__((overloadable)) shuffle(_16u64 x, _2u64 mask)
{
	return (_2u64)shuffle((_16i64)x, mask);
}

_4u64 __attribute__((overloadable)) shuffle(_16u64 x, _4u64 mask)
{
	return (_4u64)shuffle((_16i64)x, mask);
}

_8u64 __attribute__((overloadable)) shuffle(_16u64 x, _8u64 mask)
{
	return (_8u64)shuffle((_16i64)x, mask);
}

_16u64 __attribute__((overloadable)) shuffle(_16u64 x, _16u64 mask)
{
	return (_16u64)shuffle((_16i64)x, mask);
}

float2 __attribute__((overloadable)) shuffle(float2 x, _2u32 mask)
{
	return (float2)shuffle((_2i32)x, mask);
}

float4 __attribute__((overloadable)) shuffle(float2 x, _4u32 mask)
{
	return (float4)shuffle((_2i32)x, mask);
}

float8 __attribute__((overloadable)) shuffle(float2 x, _8u32 mask)
{
	return (float8)shuffle((_2i32)x, mask);
}

float16 __attribute__((overloadable)) shuffle(float2 x, _16u32 mask)
{
	return (float16)shuffle((_2i32)x, mask);
}

float2 __attribute__((overloadable)) shuffle(float4 x, _2u32 mask)
{
	return (float2)shuffle((_4i32)x, mask);
}

float4 __attribute__((overloadable)) shuffle(float4 x, _4u32 mask)
{
	return (float4)shuffle((_4i32)x, mask);
}

float8 __attribute__((overloadable)) shuffle(float4 x, _8u32 mask)
{
	return (float8)shuffle((_4i32)x, mask);
}

float16 __attribute__((overloadable)) shuffle(float4 x, _16u32 mask)
{
	return (float16)shuffle((_4i32)x, mask);
}

float2 __attribute__((overloadable)) shuffle(float8 x, _2u32 mask)
{
	return (float2)shuffle((_8i32)x, mask);
}

float4 __attribute__((overloadable)) shuffle(float8 x, _4u32 mask)
{
	return (float4)shuffle((_8i32)x, mask);
}

float8 __attribute__((overloadable)) shuffle(float8 x, _8u32 mask)
{
	return (float8)shuffle((_8i32)x, mask);
}

float16 __attribute__((overloadable)) shuffle(float8 x, _16u32 mask)
{
	return (float16)shuffle((_8i32)x, mask);
}

float2 __attribute__((overloadable)) shuffle(float16 x, _2u32 mask)
{
	return (float2)shuffle((_16i32)x, mask);
}

float4 __attribute__((overloadable)) shuffle(float16 x, _4u32 mask)
{
	return (float4)shuffle((_16i32)x, mask);
}

float8 __attribute__((overloadable)) shuffle(float16 x, _8u32 mask)
{
	return (float8)shuffle((_16i32)x, mask);
}

float16 __attribute__((overloadable)) shuffle(float16 x, _16u32 mask)
{
	return (float16)shuffle((_16i32)x, mask);
}

double2 __attribute__((overloadable)) shuffle(double2 x, _2u64 mask)
{
	return (double2)shuffle((_2i64)x, mask);
}

double4 __attribute__((overloadable)) shuffle(double2 x, _4u64 mask)
{
	return (double4)shuffle((_2i64)x, mask);
}

double8 __attribute__((overloadable)) shuffle(double2 x, _8u64 mask)
{
	return (double8)shuffle((_2i64)x, mask);
}

double16 __attribute__((overloadable)) shuffle(double2 x, _16u64 mask)
{
	return (double16)shuffle((_2i64)x, mask);
}

double2 __attribute__((overloadable)) shuffle(double4 x, _2u64 mask)
{
	return (double2)shuffle((_4i64)x, mask);
}

double4 __attribute__((overloadable)) shuffle(double4 x, _4u64 mask)
{
	return (double4)shuffle((_4i64)x, mask);
}

double8 __attribute__((overloadable)) shuffle(double4 x, _8u64 mask)
{
	return (double8)shuffle((_4i64)x, mask);
}

double16 __attribute__((overloadable)) shuffle(double4 x, _16u64 mask)
{
	return (double16)shuffle((_4i64)x, mask);
}

double2 __attribute__((overloadable)) shuffle(double8 x, _2u64 mask)
{
	return (double2)shuffle((_8i64)x, mask);
}

double4 __attribute__((overloadable)) shuffle(double8 x, _4u64 mask)
{
	return (double4)shuffle((_8i64)x, mask);
}

double8 __attribute__((overloadable)) shuffle(double8 x, _8u64 mask)
{
	return (double8)shuffle((_8i64)x, mask);
}

double16 __attribute__((overloadable)) shuffle(double8 x, _16u64 mask)
{
	return (double16)shuffle((_8i64)x, mask);
}

double2 __attribute__((overloadable)) shuffle(double16 x, _2u64 mask)
{
	return (double2)shuffle((_16i64)x, mask);
}

double4 __attribute__((overloadable)) shuffle(double16 x, _4u64 mask)
{
	return (double4)shuffle((_16i64)x, mask);
}

double8 __attribute__((overloadable)) shuffle(double16 x, _8u64 mask)
{
	return (double8)shuffle((_16i64)x, mask);
}

double16 __attribute__((overloadable)) shuffle(double16 x, _16u64 mask)
{
	return (double16)shuffle((_16i64)x, mask);
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

    mask1 = _mm_and_si128(mask + (_16u8){112}, *((__m128*)shuffle2_char16_mask));
	x = SHUFFLE_EPI8(x, mask1);

    mask2 = _mm_and_si128(mask - (_16u8){16}, *((__m128*)shuffle2_char16_mask));
	y = SHUFFLE_EPI8(y, mask2);

	return _mm_or_si128(x, y);
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
	return (_2u8)shuffle2((_2i8)x, (_2i8)y, mask);
}

_4u8 __attribute__((overloadable)) shuffle2(_2u8 x, _2u8 y, _4u8 mask)
{
	return (_4u8)shuffle2((_2i8)x, (_2i8)y, mask);
}

_8u8 __attribute__((overloadable)) shuffle2(_2u8 x, _2u8 y, _8u8 mask)
{
	return (_8u8)shuffle2((_2i8)x, (_2i8)y, mask);
}

_16u8 __attribute__((overloadable)) shuffle2(_2u8 x, _2u8 y, _16u8 mask)
{
	return (_16u8)shuffle2((_2i8)x, (_2i8)y, mask);
}

_2u8 __attribute__((overloadable)) shuffle2(_4u8 x, _4u8 y, _2u8 mask)
{
	return (_2u8)shuffle2((_4i8)x, (_4i8)y, mask);
}

_4u8 __attribute__((overloadable)) shuffle2(_4u8 x, _4u8 y, _4u8 mask)
{
	return (_4u8)shuffle2((_4i8)x, (_4i8)y, mask);
}

_8u8 __attribute__((overloadable)) shuffle2(_4u8 x, _4u8 y, _8u8 mask)
{
	return (_8u8)shuffle2((_4i8)x, (_4i8)y, mask);
}

_16u8 __attribute__((overloadable)) shuffle2(_4u8 x, _4u8 y, _16u8 mask)
{
	return (_16u8)shuffle2((_4i8)x, (_4i8)y, mask);
}

_2u8 __attribute__((overloadable)) shuffle2(_8u8 x, _8u8 y, _2u8 mask)
{
	return (_2u8)shuffle2((_8i8)x, (_8i8)y, mask);
}

_4u8 __attribute__((overloadable)) shuffle2(_8u8 x, _8u8 y, _4u8 mask)
{
	return (_4u8)shuffle2((_8i8)x, (_8i8)y, mask);
}

_8u8 __attribute__((overloadable)) shuffle2(_8u8 x, _8u8 y, _8u8 mask)
{
	return (_8u8)shuffle2((_8i8)x, (_8i8)y, mask);
}

_16u8 __attribute__((overloadable)) shuffle2(_8u8 x, _8u8 y, _16u8 mask)
{
	return (_16u8)shuffle2((_8i8)x, (_8i8)y, mask);
}

_2u8 __attribute__((overloadable)) shuffle2(_16u8 x, _16u8 y, _2u8 mask)
{
	return (_2u8)shuffle2((_16i8)x, (_16i8)y, mask);
}

_4u8 __attribute__((overloadable)) shuffle2(_16u8 x, _16u8 y, _4u8 mask)
{
	return (_4u8)shuffle2((_16i8)x, (_16i8)y, mask);
}

_8u8 __attribute__((overloadable)) shuffle2(_16u8 x, _16u8 y, _8u8 mask)
{
	return (_8u8)shuffle2((_16i8)x, (_16i8)y, mask);
}

_16u8 __attribute__((overloadable)) shuffle2(_16u8 x, _16u8 y, _16u8 mask)
{
	return (_16u8)shuffle2((_16i8)x, (_16i8)y, mask);
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

	mask = _mm_and_si128(mask, *((__m128*)shuffle_short16_mask));

    t1 = _mm_adds_epu8(mask, (_8u16){56});
    t2 = _mm_adds_epu8(mask - (_8u16){8}, (_8u16){56});

	res1 = SHUFFLE_EPI16(x, t1);
	res2 = SHUFFLE_EPI16(y, t2);

	return _mm_or_si128(res1, res2);
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

	mask = _mm_and_si128(mask, *((__m128*)shuffle_short32_mask));

    t1 = _mm_adds_epu8(mask, (_8u16){56});
    t2 = _mm_adds_epu8(mask - (_8u16){8}, (_8u16){56});
    t3 = _mm_adds_epu8(mask - (_8u16){16}, (_8u16){56});
    t4 = _mm_adds_epu8(mask - (_8u16){24}, (_8u16){56});

	res1 = SHUFFLE_EPI16(x.lo, t1);
	res2 = SHUFFLE_EPI16(x.hi, t2);
	res3 = SHUFFLE_EPI16(y.lo, t3);
	res4 = SHUFFLE_EPI16(y.hi, t4);
	res1 = _mm_or_si128(res1, res2);
	res2 = _mm_or_si128(res3, res4);

	return _mm_or_si128(res1, res2);
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
	return (_2u16)shuffle2((_2i16)x, (_2i16)y, mask);
}

_4u16 __attribute__((overloadable)) shuffle2(_2u16 x, _2u16 y, _4u16 mask)
{
	return (_4u16)shuffle2((_2i16)x, (_2i16)y, mask);
}

_8u16 __attribute__((overloadable)) shuffle2(_2u16 x, _2u16 y, _8u16 mask)
{
	return (_8u16)shuffle2((_2i16)x, (_2i16)y, mask);
}

_16u16 __attribute__((overloadable)) shuffle2(_2u16 x, _2u16 y, _16u16 mask)
{
	return (_16u16)shuffle2((_2i16)x, (_2i16)y, mask);
}

_2u16 __attribute__((overloadable)) shuffle2(_4u16 x, _4u16 y, _2u16 mask)
{
	return (_2u16)shuffle2((_4i16)x, (_4i16)y, mask);
}

_4u16 __attribute__((overloadable)) shuffle2(_4u16 x, _4u16 y, _4u16 mask)
{
	return (_4u16)shuffle2((_4i16)x, (_4i16)y, mask);
}

_8u16 __attribute__((overloadable)) shuffle2(_4u16 x, _4u16 y, _8u16 mask)
{
	return (_8u16)shuffle2((_4i16)x, (_4i16)y, mask);
}

_16u16 __attribute__((overloadable)) shuffle2(_4u16 x, _4u16 y, _16u16 mask)
{
	return (_16u16)shuffle2((_4i16)x, (_4i16)y, mask);
}

_2u16 __attribute__((overloadable)) shuffle2(_8u16 x, _8u16 y, _2u16 mask)
{
	return (_2u16)shuffle2((_8i16)x, (_8i16)y, mask);
}

_4u16 __attribute__((overloadable)) shuffle2(_8u16 x, _8u16 y, _4u16 mask)
{
	return (_4u16)shuffle2((_8i16)x, (_8i16)y, mask);
}

_8u16 __attribute__((overloadable)) shuffle2(_8u16 x, _8u16 y, _8u16 mask)
{
	return (_8u16)shuffle2((_8i16)x, (_8i16)y, mask);
}

_16u16 __attribute__((overloadable)) shuffle2(_8u16 x, _8u16 y, _16u16 mask)
{
	return (_16u16)shuffle2((_8i16)x, (_8i16)y, mask);
}

_2u16 __attribute__((overloadable)) shuffle2(_16u16 x, _16u16 y, _2u16 mask)
{
	return (_2u16)shuffle2((_16i16)x, (_16i16)y, mask);
}

_4u16 __attribute__((overloadable)) shuffle2(_16u16 x, _16u16 y, _4u16 mask)
{
	return (_4u16)shuffle2((_16i16)x, (_16i16)y, mask);
}

_8u16 __attribute__((overloadable)) shuffle2(_16u16 x, _16u16 y, _8u16 mask)
{
	return (_8u16)shuffle2((_16i16)x, (_16i16)y, mask);
}

_16u16 __attribute__((overloadable)) shuffle2(_16u16 x, _16u16 y, _16u16 mask)
{
	return (_16u16)shuffle2((_16i16)x, (_16i16)y, mask);
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

_4i32 __attribute__((overloadable)) shuffle2(_4i32 x, _4i32 y, _4u32 mask)
{
	_4i32 res1;
	_4i32 res2;

	mask = _mm_and_si128(mask, *((__m128*)shuffle_int8_mask));
    res1 = SHUFFLE_EPI32(x, mask + (_4u32){28});
    res2 = SHUFFLE_EPI32(y, mask - (_4u32){4});

	return _mm_or_si128(res1, res2);
}

_2i32 __attribute__((overloadable)) shuffle2(_4i32 x, _4i32 y, _2u32 mask)
{
	_4u32 tMask;
	tMask.s01 = mask;
	return shuffle2(x, y, tMask).s01;
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

	mask = _mm_and_si128(mask, *((__m128*)shuffle_int16_mask));

    t1 = _mm_adds_epu8(mask, (_4u32){28});
    t2 = _mm_adds_epu8(mask - (_4u32){4}, (_4u32){28});
    t3 = _mm_adds_epu8(mask - (_4u32){8}, (_4u32){28});
    t4 = _mm_adds_epu8(mask - (_4u32){12}, (_4u32){28});

	res1 = SHUFFLE_EPI32(x.lo, t1);
	res2 = SHUFFLE_EPI32(x.hi, t2);
	res3 = SHUFFLE_EPI32(y.lo, t3);
	res4 = SHUFFLE_EPI32(y.hi, t4);
	res1 = _mm_or_si128(res1, res2);
	res2 = _mm_or_si128(res3, res4);

	return _mm_or_si128(res1, res2);
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

	mask = _mm_and_si128(mask, *((__m128*)shuffle_int32_mask));

    t1 = _mm_adds_epu8(mask, (_4u32){28});
    t2 = _mm_adds_epu8(mask - (_4u32){4}, (_4u32){28});
    t3 = _mm_adds_epu8(mask - (_4u32){8}, (_4u32){28});
    t4 = _mm_adds_epu8(mask - (_4u32){12}, (_4u32){28});
    t5 = _mm_adds_epu8(mask - (_4u32){16}, (_4u32){28});
    t6 = _mm_adds_epu8(mask - (_4u32){20}, (_4u32){28});
    t7 = _mm_adds_epu8(mask - (_4u32){24}, (_4u32){28});
    t8 = _mm_adds_epu8(mask - (_4u32){28}, (_4u32){28});

	res1 = SHUFFLE_EPI32(x.lo.lo, t1);
	res2 = SHUFFLE_EPI32(x.lo.hi, t2);
	res3 = SHUFFLE_EPI32(x.hi.lo, t3);
	res4 = SHUFFLE_EPI32(x.hi.hi, t4);
	res5 = SHUFFLE_EPI32(y.lo.lo, t5);
	res6 = SHUFFLE_EPI32(y.lo.hi, t6);
	res7 = SHUFFLE_EPI32(y.hi.lo, t7);
	res8 = SHUFFLE_EPI32(y.hi.hi, t8);

	res1 = _mm_or_si128(res1, res2);
	res2 = _mm_or_si128(res3, res4);
	res3 = _mm_or_si128(res5, res6);
	res4 = _mm_or_si128(res7, res8);

	res1 = _mm_or_si128(res1, res2);
	res2 = _mm_or_si128(res3, res4);

	return _mm_or_si128(res1, res2);
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
	return (_2u32)shuffle2((_2i32)x, (_2i32)y, mask);
}

_4u32 __attribute__((overloadable)) shuffle2(_2u32 x, _2u32 y, _4u32 mask)
{
	return (_4u32)shuffle2((_2i32)x, (_2i32)y, mask);
}

_8u32 __attribute__((overloadable)) shuffle2(_2u32 x, _2u32 y, _8u32 mask)
{
	return (_8u32)shuffle2((_2i32)x, (_2i32)y, mask);
}

_16u32 __attribute__((overloadable)) shuffle2(_2u32 x, _2u32 y, _16u32 mask)
{
	return (_16u32)shuffle2((_2i32)x, (_2i32)y, mask);
}

_2u32 __attribute__((overloadable)) shuffle2(_4u32 x, _4u32 y, _2u32 mask)
{
	return (_2u32)shuffle2((_4i32)x, (_4i32)y, mask);
}

_4u32 __attribute__((overloadable)) shuffle2(_4u32 x, _4u32 y, _4u32 mask)
{
	return (_4u32)shuffle2((_4i32)x, (_4i32)y, mask);
}

_8u32 __attribute__((overloadable)) shuffle2(_4u32 x, _4u32 y, _8u32 mask)
{
	return (_8u32)shuffle2((_4i32)x, (_4i32)y, mask);
}

_16u32 __attribute__((overloadable)) shuffle2(_4u32 x, _4u32 y, _16u32 mask)
{
	return (_16u32)shuffle2((_4i32)x, (_4i32)y, mask);
}

_2u32 __attribute__((overloadable)) shuffle2(_8u32 x, _8u32 y, _2u32 mask)
{
	return (_2u32)shuffle2((_8i32)x, (_8i32)y, mask);
}

_4u32 __attribute__((overloadable)) shuffle2(_8u32 x, _8u32 y, _4u32 mask)
{
	return (_4u32)shuffle2((_8i32)x, (_8i32)y, mask);
}

_8u32 __attribute__((overloadable)) shuffle2(_8u32 x, _8u32 y, _8u32 mask)
{
	return (_8u32)shuffle2((_8i32)x, (_8i32)y, mask);
}

_16u32 __attribute__((overloadable)) shuffle2(_8u32 x, _8u32 y, _16u32 mask)
{
	return (_16u32)shuffle2((_8i32)x, (_8i32)y, mask);
}

_2u32 __attribute__((overloadable)) shuffle2(_16u32 x, _16u32 y, _2u32 mask)
{
	return (_2u32)shuffle2((_16i32)x, (_16i32)y, mask);
}

_4u32 __attribute__((overloadable)) shuffle2(_16u32 x, _16u32 y, _4u32 mask)
{
	return (_4u32)shuffle2((_16i32)x, (_16i32)y, mask);
}

_8u32 __attribute__((overloadable)) shuffle2(_16u32 x, _16u32 y, _8u32 mask)
{
	return (_8u32)shuffle2((_16i32)x, (_16i32)y, mask);
}

_16u32 __attribute__((overloadable)) shuffle2(_16u32 x, _16u32 y, _16u32 mask)
{
	return (_16u32)shuffle2((_16i32)x, (_16i32)y, mask);
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
	return (_2u64)shuffle2((_2i64)x, (_2i64)y, mask);
}

_4u64 __attribute__((overloadable)) shuffle2(_2u64 x, _2u64 y, _4u64 mask)
{
	return (_4u64)shuffle2((_2i64)x, (_2i64)y, mask);
}

_8u64 __attribute__((overloadable)) shuffle2(_2u64 x, _2u64 y, _8u64 mask)
{
	return (_8u64)shuffle2((_2i64)x, (_2i64)y, mask);
}

_16u64 __attribute__((overloadable)) shuffle2(_2u64 x, _2u64 y, _16u64 mask)
{
	return (_16u64)shuffle2((_2i64)x, (_2i64)y, mask);
}

_2u64 __attribute__((overloadable)) shuffle2(_4u64 x, _4u64 y, _2u64 mask)
{
	return (_2u64)shuffle2((_4i64)x, (_4i64)y, mask);
}

_4u64 __attribute__((overloadable)) shuffle2(_4u64 x, _4u64 y, _4u64 mask)
{
	return (_4u64)shuffle2((_4i64)x, (_4i64)y, mask);
}

_8u64 __attribute__((overloadable)) shuffle2(_4u64 x, _4u64 y, _8u64 mask)
{
	return (_8u64)shuffle2((_4i64)x, (_4i64)y, mask);
}

_16u64 __attribute__((overloadable)) shuffle2(_4u64 x, _4u64 y, _16u64 mask)
{
	return (_16u64)shuffle2((_4i64)x, (_4i64)y, mask);
}

_2u64 __attribute__((overloadable)) shuffle2(_8u64 x, _8u64 y, _2u64 mask)
{
	return (_2u64)shuffle2((_8i64)x, (_8i64)y, mask);
}

_4u64 __attribute__((overloadable)) shuffle2(_8u64 x, _8u64 y, _4u64 mask)
{
	return (_4u64)shuffle2((_8i64)x, (_8i64)y, mask);
}

_8u64 __attribute__((overloadable)) shuffle2(_8u64 x, _8u64 y, _8u64 mask)
{
	return (_8u64)shuffle2((_8i64)x, (_8i64)y, mask);
}

_16u64 __attribute__((overloadable)) shuffle2(_8u64 x, _8u64 y, _16u64 mask)
{
	return (_16u64)shuffle2((_8i64)x, (_8i64)y, mask);
}

_2u64 __attribute__((overloadable)) shuffle2(_16u64 x, _16u64 y, _2u64 mask)
{
	return (_2u64)shuffle2((_16i64)x, (_16i64)y, mask);
}

_4u64 __attribute__((overloadable)) shuffle2(_16u64 x, _16u64 y, _4u64 mask)
{
	return (_4u64)shuffle2((_16i64)x, (_16i64)y, mask);
}

_8u64 __attribute__((overloadable)) shuffle2(_16u64 x, _16u64 y, _8u64 mask)
{
	return (_8u64)shuffle2((_16i64)x, (_16i64)y, mask);
}

_16u64 __attribute__((overloadable)) shuffle2(_16u64 x, _16u64 y, _16u64 mask)
{
	return (_16u64)shuffle2((_16i64)x, (_16i64)y, mask);
}



float2 __attribute__((overloadable)) shuffle2(float2 x, float2 y, _2u32 mask)
{
	return (float2)shuffle2((_2i32)x, (_2i32)y, mask);
}

float4 __attribute__((overloadable)) shuffle2(float2 x, float2 y, _4u32 mask)
{
	return (float4)shuffle2((_2i32)x, (_2i32)y, mask);
}

float8 __attribute__((overloadable)) shuffle2(float2 x, float2 y, _8u32 mask)
{
	return (float8)shuffle2((_2i32)x, (_2i32)y, mask);
}

float16 __attribute__((overloadable)) shuffle2(float2 x, float2 y, _16u32 mask)
{
	return (float16)shuffle2((_2i32)x, (_2i32)y, mask);
}

float2 __attribute__((overloadable)) shuffle2(float4 x, float4 y, _2u32 mask)
{
	return (float2)shuffle2((_4i32)x, (_4i32)y, mask);
}

float4 __attribute__((overloadable)) shuffle2(float4 x, float4 y, _4u32 mask)
{
	return (float4)shuffle2((_4i32)x, (_4i32)y, mask);
}

float8 __attribute__((overloadable)) shuffle2(float4 x, float4 y, _8u32 mask)
{
	return (float8)shuffle2((_4i32)x, (_4i32)y, mask);
}

float16 __attribute__((overloadable)) shuffle2(float4 x, float4 y, _16u32 mask)
{
	return (float16)shuffle2((_4i32)x, (_4i32)y, mask);
}

float2 __attribute__((overloadable)) shuffle2(float8 x, float8 y, _2u32 mask)
{
	return (float2)shuffle2((_8i32)x, (_8i32)y, mask);
}

float4 __attribute__((overloadable)) shuffle2(float8 x, float8 y, _4u32 mask)
{
	return (float4)shuffle2((_8i32)x, (_8i32)y, mask);
}

float8 __attribute__((overloadable)) shuffle2(float8 x, float8 y, _8u32 mask)
{
	return (float8)shuffle2((_8i32)x, (_8i32)y, mask);
}

float16 __attribute__((overloadable)) shuffle2(float8 x, float8 y, _16u32 mask)
{
	return (float16)shuffle2((_8i32)x, (_8i32)y, mask);
}

float2 __attribute__((overloadable)) shuffle2(float16 x, float16 y, _2u32 mask)
{
	return (float2)shuffle2((_16i32)x, (_16i32)y, mask);
}

float4 __attribute__((overloadable)) shuffle2(float16 x, float16 y, _4u32 mask)
{
	return (float4)shuffle2((_16i32)x, (_16i32)y, mask);
}

float8 __attribute__((overloadable)) shuffle2(float16 x, float16 y, _8u32 mask)
{
	return (float8)shuffle2((_16i32)x, (_16i32)y, mask);
}

float16 __attribute__((overloadable)) shuffle2(float16 x, float16 y, _16u32 mask)
{
	return (float16)shuffle2((_16i32)x, (_16i32)y, mask);
}


double2 __attribute__((overloadable)) shuffle2(double2 x, double2 y, _2u64 mask)
{
	return (double2)shuffle2((_2i64)x, (_2i64)y, mask);
}

double4 __attribute__((overloadable)) shuffle2(double2 x, double2 y, _4u64 mask)
{
	return (double4)shuffle2((_2i64)x, (_2i64)y, mask);
}

double8 __attribute__((overloadable)) shuffle2(double2 x, double2 y, _8u64 mask)
{
	return (double8)shuffle2((_2i64)x, (_2i64)y, mask);
}

double16 __attribute__((overloadable)) shuffle2(double2 x, double2 y, _16u64 mask)
{
	return (double16)shuffle2((_2i64)x, (_2i64)y, mask);
}

double2 __attribute__((overloadable)) shuffle2(double4 x, double4 y, _2u64 mask)
{
	return (double2)shuffle2((_4i64)x, (_4i64)y, mask);
}

double4 __attribute__((overloadable)) shuffle2(double4 x, double4 y, _4u64 mask)
{
	return (double4)shuffle2((_4i64)x, (_4i64)y, mask);
}

double8 __attribute__((overloadable)) shuffle2(double4 x, double4 y, _8u64 mask)
{
	return (double8)shuffle2((_4i64)x, (_4i64)y, mask);
}

double16 __attribute__((overloadable)) shuffle2(double4 x, double4 y, _16u64 mask)
{
	return (double16)shuffle2((_4i64)x, (_4i64)y, mask);
}

double2 __attribute__((overloadable)) shuffle2(double8 x, double8 y, _2u64 mask)
{
	return (double2)shuffle2((_8i64)x, (_8i64)y, mask);
}

double4 __attribute__((overloadable)) shuffle2(double8 x, double8 y, _4u64 mask)
{
	return (double4)shuffle2((_8i64)x, (_8i64)y, mask);
}

double8 __attribute__((overloadable)) shuffle2(double8 x, double8 y, _8u64 mask)
{
	return (double8)shuffle2((_8i64)x, (_8i64)y, mask);
}

double16 __attribute__((overloadable)) shuffle2(double8 x, double8 y, _16u64 mask)
{
	return (double16)shuffle2((_8i64)x, (_8i64)y, mask);
}

double2 __attribute__((overloadable)) shuffle2(double16 x, double16 y, _2u64 mask)
{
	return (double2)shuffle2((_16i64)x, (_16i64)y, mask);
}

double4 __attribute__((overloadable)) shuffle2(double16 x, double16 y, _4u64 mask)
{
	return (double4)shuffle2((_16i64)x, (_16i64)y, mask);
}

double8 __attribute__((overloadable)) shuffle2(double16 x, double16 y, _8u64 mask)
{
	return (double8)shuffle2((_16i64)x, (_16i64)y, mask);
}

double16 __attribute__((overloadable)) shuffle2(double16 x, double16 y, _16u64 mask)
{
	return (double16)shuffle2((_16i64)x, (_16i64)y, mask);
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
