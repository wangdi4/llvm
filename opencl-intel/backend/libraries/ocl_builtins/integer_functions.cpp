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
//  integer_naive_functions.cpp
///////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

// Compiled with Clang as LLVM module
#define ALIGN16 __attribute__((aligned(16)))
#include <intrin.h>

#include "cl_integer_declaration.h"

#define	__max(x,y)	((x)>(y) ? (x) : (y))
#define	__min(x,y)	((x)<(y) ? (x) : (y))
#define __abs(x)	((x)>0 ? (x) : -(x))

#define USHRT_MIN   0
#define UCHAR_MIN   0

ALIGN16 const int   intMinStorage[4]    = {INT_MIN, INT_MIN, INT_MIN, INT_MIN};
ALIGN16 const int   intMaxStorage[4]    = {INT_MAX, INT_MAX, INT_MAX, INT_MAX};
ALIGN16 const int   uintMaxStorage[4]   = {UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX};
	   
ALIGN16 const char  byteLsbMask[16]     = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
ALIGN16 const short wordLsbMask[8]      = {1, 1, 1, 1, 1, 1, 1, 1};
ALIGN16 const int   dwordLsbMask[4]     = {1, 1, 1, 1};
ALIGN16 const int   oneZeroOneMask[4]   = {1, 0, 1, 0};
ALIGN16 const char  byteMsbMask[16]     = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
ALIGN16 const short wordMsbMask[8]      = {0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000};
ALIGN16 const int   dwordMsbMask[4]     = {0x80000000, 0x80000000, 0x80000000, 0x80000000};
ALIGN16 const int   qwordMsbMask[4]     = {0x00000000, 0x80000000, 0x00000000, 0x80000000};
	   
ALIGN16 const char  eightStorage[16]    = {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};
ALIGN16 const short sixteenStorage[8]   = {16, 16, 16, 16, 16, 16, 16, 16};
ALIGN16 const int   thirtyTwoStorage[4] = {32, 32, 32, 32};
ALIGN16 const int   sixtyFourStorage[4] = {64, 0, 64, 0};
	   
ALIGN16 const short wordCharMax[8]      = {CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX};
ALIGN16 const short wordUCharMax[8]     = {UCHAR_MAX, UCHAR_MAX, UCHAR_MAX, UCHAR_MAX, UCHAR_MAX, UCHAR_MAX, UCHAR_MAX, UCHAR_MAX};
ALIGN16 const short wordCharMin[8]      = {CHAR_MIN, CHAR_MIN, CHAR_MIN, CHAR_MIN, CHAR_MIN, CHAR_MIN, CHAR_MIN, CHAR_MIN};
ALIGN16 const int   dwordShortMax[4]    = {SHRT_MAX, SHRT_MAX, SHRT_MAX, SHRT_MAX};
ALIGN16 const int   dwordUShortMax[4]   = {USHRT_MAX, USHRT_MAX, USHRT_MAX, USHRT_MAX};
ALIGN16 const int   dwordShortMin[4]    = {SHRT_MIN, SHRT_MIN, SHRT_MIN, SHRT_MIN};
ALIGN16 const int   qwordIntMax[4]      = {INT_MAX, 0, INT_MAX, 0};
ALIGN16 const int   qwordUIntMax[4]     = {UINT_MAX, 0, UINT_MAX, 0};
ALIGN16 const int   qwordIntMin[4]      = {INT_MIN, 0xFFFFFFFF, INT_MIN, 0xFFFFFFFF};
	   
ALIGN16 const int   dwordEvenMask[4]    = {0xFFFFFFFF, 0, 0xFFFFFFFF, 0};
ALIGN16 const int   dwordOddMask[4]     = {0, 0xFFFFFFFF, 0, 0xFFFFFFFF};

#define FORCEINLINE __attribute__((__always_inline__, __nodebug__))


//Functions copied from the conformance tests
 FORCEINLINE void multiply_unsigned_64_by_64( _1u64 sourceA, _1u64 sourceB, _1u64 *destLow, _1u64 *destHi )
{
	_1u64 lowA, lowB;
	_1u64 highA, highB;
	
	// Split up the values
	lowA = sourceA & 0xffffffff;
	highA = sourceA >> 32;
	lowB = sourceB & 0xffffffff;
	highB = sourceB >> 32;
	
	// Note that, with this split, our multiplication becomes:
	//     ( a * b )
	// = ( ( aHI << 32 + aLO ) * ( bHI << 32 + bLO ) ) >> 64
	// = ( ( aHI << 32 * bHI << 32 ) + ( aHI << 32 * bLO ) + ( aLO * bHI << 32 ) + ( aLO * bLO ) ) >> 64
	// = ( ( aHI * bHI << 64 ) + ( aHI * bLO << 32 ) + ( aLO * bHI << 32 ) + ( aLO * bLO ) ) >> 64
	// = ( aHI * bHI ) + ( aHI * bLO >> 32 ) + ( aLO * bHI >> 32 ) + ( aLO * bLO >> 64 )
	
	// Now, since each value is 32 bits, the max size of any multiplication is:
	// ( 2 ^ 32 - 1 ) * ( 2 ^ 32 - 1 ) = 2^64 - 4^32 + 1 = 2^64 - 2^33 + 1, which fits within 64 bits
	// Which means we can do each component within a 64-bit integer as necessary (each component above marked as AB1 - AB4)
	_1u64 aHibHi = highA * highB;
	_1u64 aHibLo = highA * lowB;
	_1u64 aLobHi = lowA * highB;
	_1u64 aLobLo = lowA * lowB;
	
    // Assemble terms. 
    //  We note that in certain cases, sums of products cannot overflow:
    //
    //      The maximum product of two N-bit unsigned numbers is 
    //  
    //          (2**N-1)^2 = 2**2N - 2**(N+1) + 1
    //
    //      We note that we can add the maximum N-bit number to the 2N-bit product twice without overflow:
    //
    //          (2**N-1)^2 + 2*(2**N-1) = 2**2N - 2**(N+1) + 1 + 2**(N+1) - 2 = 2**2N - 1
    //
    //  If we breakdown the product of two numbers a,b into high and low halves of partial products as follows:
    //
    //                                              a.hi                a.lo
    // x                                            b.hi                b.lo
    //===============================================================================
    //  (b.hi*a.hi).hi      (b.hi*a.hi).lo
    //                      (b.lo*a.hi).hi      (b.lo*a.hi).lo
    //                      (b.hi*a.lo).hi      (b.hi*a.lo).lo
    // +                                        (b.lo*a.lo).hi      (b.lo*a.lo).lo
    //===============================================================================
    //
    // The (b.lo*a.lo).lo term cannot cause a carry, so we can ignore them for now.  We also know from above, that we can add (b.lo*a.lo).hi
    // and (b.hi*a.lo).lo to the 2N bit term [(b.lo*a.hi).hi + (b.lo*a.hi).lo] without overflow.  That takes care of all of the terms
    // on the right half that might carry.  Do that now.
    //
    _1u64 aLobLoHi = aLobLo >> 32;
    _1u64 aLobHiLo = aLobHi & 0xFFFFFFFFUL;
    aHibLo += aLobLoHi + aLobHiLo;

    // That leaves us with these terms:
    //
    //                                              a.hi                a.lo
    // x                                            b.hi                b.lo
    //===============================================================================
    //  (b.hi*a.hi).hi      (b.hi*a.hi).lo
    //                      (b.hi*a.lo).hi      
    //                    [ (b.lo*a.hi).hi + (b.lo*a.hi).lo + other ] 
    // +                                                                (b.lo*a.lo).lo
    //===============================================================================

    // All of the overflow potential from the right half has now been accumulated into the [ (b.lo*a.hi).hi + (b.lo*a.hi).lo ] 2N bit term.
    // We can safely separate into high and low parts. Per our rule above, we know we can accumulate the high part of that and (b.hi*a.lo).hi
    // into the 2N bit term (b.lo*a.hi) without carry.  The low part can be pieced together with (b.lo*a.lo).lo, to give the final low result
    
    (*destHi) = aHibHi + (aHibLo >> 32 ) + (aLobHi >> 32);             // Cant overflow
    (*destLow) = (aHibLo << 32) | ( aLobLo & 0xFFFFFFFFUL );
}

FORCEINLINE void multiply_signed_64_by_64( _1i64 sourceA, _1i64 sourceB, _1u64* destLow, _1i64* destHi )
{
    // Find sign of result
    _1i64 aSign = sourceA >> 63;
    _1i64 bSign = sourceB >> 63;
    _1i64 resultSign = aSign ^ bSign;

    // take absolute values of the argument
    sourceA = (sourceA ^ aSign) - aSign;
    sourceB = (sourceB ^ bSign) - bSign;

    _1u64 hi;
    multiply_unsigned_64_by_64( (_1u64) sourceA, (_1u64) sourceB, destLow, &hi );

    // Fix the sign
    if( resultSign )
    {
        (*destLow) ^= resultSign;
        hi  ^= resultSign;
        (*destLow) -= resultSign;
        
        //carry if necessary
        if( 0 == (*destLow) )
            hi -= resultSign;
    }
    
    (*destHi) = (_1i64) hi;
}

 //////////////////////////////////////////////////////////////////
// abs() implementation - ugentype abs (gentype x) Returns | x | //
///////////////////////////////////////////////////////////////////
_1u8	__attribute__((overloadable)) abs(_1i8 x)
{
	return __abs(x);
}

_1u8	__attribute__((overloadable)) abs(_1u8 x)
{
	return x;
}

_16u8	__attribute__((overloadable)) abs(_16i8 x)
{
#ifdef __SSSE3__
	return (_16u8)_mm_abs_epi8((__m128i)x);
#else
	ALIGN16 _1i8 tempX[16];
	ALIGN16 _1i8 result[16];

	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX, x);
	#pragma ivdep
	for(int i=0; i<16; ++i)
	{
		result[i] = abs(tempX[i]);
	}
	return _mm_load_si128((const __m128i *)result);
#endif
}

_16u8	__attribute__((overloadable)) abs(_16u8 x)
{
	return x;
}

_1u16	__attribute__((overloadable)) abs(_1i16 x)
{
	return __abs(x);
}

_1u16	__attribute__((overloadable)) abs(_1u16 x)
{
	return x;
}

_8u16	__attribute__((overloadable)) abs(_8i16 x)
{
#ifdef __SSSE3__
	return (_8u16)_mm_abs_epi16((__m128i)x);
#else
	ALIGN16 _1i16 tempX[8];
	ALIGN16 _1i16 result[8];

	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX, x);
	#pragma ivdep
	for(int i=0; i<8; ++i)
	{
		result[i] = abs(tempX[i]);
	}
	return _mm_load_si128((const __m128i *)result);
#endif
}

_8u16	__attribute__((overloadable)) abs(_8u16 x)
{
	return x;
}

_1u32	__attribute__((overloadable)) abs(_1i32 x)
{
	return __abs(x);
}

_1u32	__attribute__((overloadable)) abs(_1u32 x)
{
	return x;
}

_4u32	__attribute__((overloadable)) abs(_4i32 x)
{
#ifdef __SSSE3__
	return (_4u32)_mm_abs_epi32((__m128i)x);
#else
	ALIGN16 _1i32 tempX[4];
	ALIGN16 _1i32 result[4];

	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX, x);
	#pragma ivdep
	for(int i=0; i<4; ++i)
	{
		result[i] = abs(tempX[i]);
	}
	return _mm_load_si128((const __m128i *)result);
#endif
}

_4u32	__attribute__((overloadable)) abs(_4u32 x)
{
	return x;
}

_2u64	__attribute__((overloadable)) abs(_2i64 x)
{
	__m128i sign = _mm_srli_epi64((__m128i)x, 63); 
	__m128i mask  = _mm_setzero_si128();
	mask =  _mm_sub_epi64(mask, sign);   
	x = (_2i64)_mm_xor_si128((__m128i)x, mask);
	x =(_2i64)_mm_add_epi64((__m128i)x, sign);
	return (_2u64)x;
}

_2u64	__attribute__((overloadable)) abs(_2u64 x)
{
	return x;
}

_1u64	__attribute__((overloadable)) abs(_1i64 x)
{
	return __abs(x);
}

_1u64	__attribute__((overloadable)) abs(_1u64 x)
{
	return x;
}

/////////////////////////////////////////////////////////////////////////////////////////
//ugentype abs_diff() (gentype x, gentype y) Returns | x – y | without modulo overflow //
/////////////////////////////////////////////////////////////////////////////////////////
_1u8	__attribute__((overloadable)) abs_diff(_1i8 x, _1i8 y)
{
	unsigned char r = x - y;
	if( y > x )
		r = y - x;
	return r;
}

_1u16	__attribute__((overloadable)) abs_diff(_1i16 x, _1i16 y)
{
	unsigned short r = x - y;
	if( y > x )
		r = y - x;
	return r;
}

_1u32	__attribute__((overloadable)) abs_diff(_1i32 x, _1i32 y)
{
	unsigned int r = x - y;
	if( y > x )
		r = y - x;
	return r;
}

_16u8	__attribute__((overloadable)) abs_diff(_16i8 x, _16i8 y)
{
	__m128i tmp1 = _mm_sub_epi8((__m128i)x,(__m128i)y);
	__m128i tmp2 = _mm_sub_epi8((__m128i)y,(__m128i)x);
	__m128i tmpc = _mm_cmpgt_epi8((__m128i)y,(__m128i)x);
	__m128i res = _mm_and_si128(tmpc, tmp2);
	__m128i reb = _mm_andnot_si128(tmpc, tmp1);
	return (_16u8)_mm_or_si128(res, reb);
}

_8u16	__attribute__((overloadable)) abs_diff(_8i16 x, _8i16 y)
{
	__m128i tmp1 = _mm_sub_epi16((__m128i)x,(__m128i)y);
	__m128i tmp2 = _mm_sub_epi16((__m128i)y,(__m128i)x);
	__m128i tmpc = _mm_cmpgt_epi16((__m128i)y,(__m128i)x);
	__m128i res = _mm_and_si128(tmpc, tmp2);
	__m128i reb = _mm_andnot_si128(tmpc, tmp1);
	return (_8u16)_mm_or_si128(res, reb);
}

_4u32	__attribute__((overloadable)) abs_diff(_4i32 x, _4i32 y)
{
	__m128i tmp1 = _mm_sub_epi32((__m128i)x,(__m128i)y);
	__m128i tmp2 = _mm_sub_epi32((__m128i)y,(__m128i)x);
	__m128i tmpc = _mm_cmpgt_epi32((__m128i)y,(__m128i)x);
	__m128i res = _mm_and_si128(tmpc, tmp2);
	__m128i reb = _mm_andnot_si128(tmpc, tmp1);
	return (_4u32)_mm_or_si128(res, reb);
}

_1u64	__attribute__((overloadable)) abs_diff(_1i64 x, _1i64 y)
{
    _1u64 r = x - y;
    if( y > x )
        r = y - x;
    return r;
}

_2u64	__attribute__((overloadable)) abs_diff(_2i64 x, _2i64 y)
{
#ifdef __SSE4_2__
	__m128i tmp1 = _mm_sub_epi64((__m128i)x,(__m128i)y);
	__m128i tmp2 = _mm_sub_epi64((__m128i)y,(__m128i)x);
	__m128i tmpc = _mm_cmpgt_epi64((__m128i)y,(__m128i)x);
	__m128i res = _mm_and_si128(tmpc, tmp2);
	__m128i reb = _mm_andnot_si128(tmpc, tmp1);
	return (_2u64)_mm_or_si128(res, reb);
#else
	ALIGN16 _1i64 tempX[2], tempY[2];
	ALIGN16 _1u64 result[2];

	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX, (__m128i)x);
	_mm_store_si128((__m128i *)tempY, (__m128i)y);
	#pragma ivdep
	for(int i=0; i<2; ++i)
	{
		result[i] = abs_diff(tempX[i] , tempY[i]);
	}
	return (_2u64)_mm_load_si128((const __m128i *)result);
#endif
}

//unsigned
_1u8	__attribute__((overloadable)) abs_diff(_1u8 x, _1u8 y)
{
	unsigned char r = x - y;
	if( y > x )
		r = y - x;
		return r;
}

_1u16	__attribute__((overloadable)) abs_diff(_1u16 x, _1u16 y)
{
	unsigned short r = x - y;
	if( y > x )
		r = y - x;
		return r;
}

_1u32	__attribute__((overloadable)) abs_diff(_1u32 x, _1u32 y)
{
	unsigned int r = x - y;
	if( y > x )
		r = y - x;
	return r;
}

_16u8	__attribute__((overloadable)) abs_diff(_16u8 x, _16u8 y)
{
	__m128i min = _mm_min_epu8((__m128i)x, (__m128i)y);
	x = (_16u8)_mm_max_epu8((__m128i)x, (__m128i)y);
	x = (_16u8)_mm_sub_epi8((__m128i)x, min);
	return x;
}

_8u16	__attribute__((overloadable)) abs_diff(_8u16 x, _8u16 y)
{
#ifdef __SSE4_1__
	__m128i min = _mm_min_epu16((__m128i)x, (__m128i)y);
	x = (_8u16)_mm_max_epu16((__m128i)x, (__m128i)y);
	x = (_8u16)_mm_sub_epi16((__m128i)x, min);
#else
	unsigned short ALIGN16 sX[8], sY[8], sR[8];
	_mm_store_si128((__m128i *)sX,(__m128i) x);
	_mm_store_si128((__m128i *)sY,(__m128i) y);
	for(int i=0; i<8; ++i)
	{
		sR[i] = sX[i] - sY[i];
		if( sY[i] > sX[i] )
			sR[i] = sY[i] - sX[i];
	}
	x = (_8u16)_mm_load_si128((__m128i *)sR);
#endif
	return (_8u16)x;
}

_4u32	__attribute__((overloadable)) abs_diff(_4u32 x, _4u32 y)
{
#ifdef __SSE4_1__
	__m128i min = _mm_min_epu32((__m128i)x, (__m128i)y);
	x = (_4u32)_mm_max_epu32((__m128i)x, (__m128i)y);
	x = (_4u32)_mm_sub_epi32((__m128i)x, min);
#else
	ALIGN16 unsigned int uiX[4], uiY[4], uiR[4];
	_mm_store_si128((__m128i *)uiX,(__m128i) x);
	_mm_store_si128((__m128i *)uiY,(__m128i) y);
	for(int i=0; i<4; ++i)
	{
        uiR[i] = uiX[i] - uiY[i];
        if( uiY[i] > uiX[i] )
			uiR[i] = uiY[i] - uiX[i];
	}
	x = (_4u32)_mm_load_si128((__m128i *)uiR);
#endif
	return x;
}

_2u64	__attribute__((overloadable)) abs_diff(_2u64 x, _2u64 y)
{
	__m128i mask  =	_mm_load_si128((const __m128i*)dwordMsbMask);
	__m128i x_neg =	_mm_xor_si128((__m128i)x, mask);
	__m128i y_neg = _mm_xor_si128((__m128i)y, mask);
	__m128i great = _mm_cmpgt_epi32(y_neg, x_neg);
	__m128i equal = _mm_cmpeq_epi32(y_neg, x_neg);
	mask =			_mm_slli_epi64(great, 32);
	mask =			_mm_and_si128(mask, equal);
	mask =			_mm_or_si128(mask, great);
	mask =			_mm_shuffle_epi32(mask, 0xF5);
	__m128i res =	_mm_sub_epi64((__m128i)x, (__m128i)y);
	res =			_mm_xor_si128(res, mask);
	res =			_mm_sub_epi64(res, mask);
	return (_2u64)res;
}

_1u64	__attribute__((overloadable)) abs_diff(_1u64 x, _1u64 y)
{
	_1u64 r = x - y;
	if( y > x )
		r = y - x;
	return r;
}

///////////////////////////////////////////////////////////////////////////////////
//gentype add_sat (gentype x, gentype y) Returns x + y and saturates the result. //
///////////////////////////////////////////////////////////////////////////////////
_1i8	__attribute__((overloadable)) add_sat(_1i8 x, _1i8 y)
{
	int r = x+y;
	r = __max( r, CHAR_MIN );
	r = __min( r, CHAR_MAX );
	return (char)r;
}
_1u8	__attribute__((overloadable)) add_sat(_1u8 x, _1u8 y)
{
	int r = x+y;
	r = __max( r, UCHAR_MIN );
	r = __min( r, UCHAR_MAX );
	return (unsigned char)r;
}

_1i16	__attribute__((overloadable)) add_sat(_1i16 x, _1i16 y)
{
	int r = x+y;
	r = __max( r, SHRT_MIN );
	r = __min( r, SHRT_MAX );
	return (short)r;
}

_1u16	__attribute__((overloadable)) add_sat(_1u16 x, _1u16 y)
{
	int r = (int) x + (int) y;
	r = __max( r, USHRT_MIN );
	r = __min( r, USHRT_MAX );
	return (unsigned short)r;
}
_1i32	__attribute__((overloadable)) add_sat(_1i32 x, _1i32 y)
{
	int r = (int) x + (int) y;
	if( y > 0 )
	{
		if( r < x )
			r = INT_MAX;
	}
	else
	{
		if( r > x )
			r = INT_MIN;
	}
	return r;
}

_1u32	__attribute__((overloadable)) add_sat(_1u32 x, _1u32 y)
{
	unsigned int r = x+y;
	if( r < x )
		r = UINT_MAX;
	return r;
}

_16i8	__attribute__((overloadable)) add_sat(_16i8 x, _16i8 y)
{
	return (_16i8)_mm_adds_epi8((__m128i)x,(__m128i)y);	
}

_16u8	__attribute__((overloadable)) add_sat(_16u8 x, _16u8 y)
{
	return (_16u8)_mm_adds_epu8((__m128i)x,(__m128i)y);	
}

_8i16	__attribute__((overloadable)) add_sat(_8i16 x, _8i16 y)
{
	return (_8i16)_mm_adds_epi16((__m128i)x,(__m128i)y);	
}

_8u16	__attribute__((overloadable)) add_sat(_8u16 x, _8u16 y)
{
	return (_8u16) _mm_adds_epu16((__m128i)x,(__m128i)y);	
}

_4i32	__attribute__((overloadable)) add_sat(_4i32 x, _4i32 y)
{
	__m128i zero = _mm_setzero_si128();

	__m128i res = _mm_add_epi32((__m128i)x,(__m128i) y);
	__m128i yGTzero = _mm_cmpgt_epi32((__m128i)y,(__m128i) zero);
	__m128i resGTx = _mm_cmpgt_epi32((__m128i)res,(__m128i) x);
	__m128i overflow = _mm_xor_si128(resGTx, yGTzero);
	res = _mm_andnot_si128(overflow, res);
	__m128i max = _mm_and_si128(yGTzero, *((__m128i *)intMaxStorage));
	__m128i min = _mm_andnot_si128(yGTzero, *((__m128i *)intMinStorage));
	__m128i temp = _mm_or_si128(max, min);
	overflow = _mm_and_si128(temp, overflow);
	return (_4i32) _mm_or_si128(res, overflow);

}

_4u32	__attribute__((overloadable)) add_sat(_4u32 x, _4u32 y)
{
	__m128i zero = _mm_setzero_si128();

	__m128i res = _mm_add_epi32((__m128i)x,(__m128i) y);
	__m128i res_neg = _mm_xor_si128(res, *((__m128i *)dwordMsbMask));
	x = (_4u32) _mm_xor_si128((__m128i)x,(__m128i) *((__m128i *)dwordMsbMask));
	__m128i xGTres = _mm_cmpgt_epi32((__m128i)x,(__m128i) res_neg);
	res = _mm_andnot_si128(xGTres, res);
	__m128i overflow = _mm_and_si128(xGTres, *((__m128i *)uintMaxStorage));
	return (_4u32) _mm_or_si128(res, overflow);
}

_2u64	__attribute__((overloadable)) add_sat(_2u64 x, _2u64 y)
{
	ALIGN16 _1u64 tempX[2], tempY[2];
	
	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX,(__m128i) x);
	_mm_store_si128((__m128i *)tempY,(__m128i) y);
	#pragma ivdep
	for(int i=0; i<2; ++i)
	{
		_1i64 r = tempX[i] + tempY[i];
		if( r < tempX[i] )
			r = ULLONG_MAX;
		tempX[i] = r;
	}
	return (_2u64) _mm_load_si128((const __m128i *)tempX);
}

_2i64	__attribute__((overloadable)) add_sat(_2i64 x, _2i64 y)
{
	ALIGN16 _1i64 tempX[2], tempY[2];

	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX,(__m128i) x);
	_mm_store_si128((__m128i *)tempY,(__m128i) y);
	#pragma ivdep
	for(int i=0; i<2; ++i)
	{
		_1i64 r = tempX[i] + tempY[i];
		if( tempY[i] > 0 )
		{
			if( r < tempX[i] )
				r = LLONG_MAX;
		}
		else
		{
			if( r > tempX[i] )
				r = LLONG_MIN;
		}
		tempX[i] = r;
	}
	return (_2i64) _mm_load_si128((const __m128i *)tempX);
}

_1u64	__attribute__((overloadable)) add_sat(_1u64 x, _1u64 y)
{
	// Store to temporary buffer
	_1u64 r = x + y;
	if( r < x )
		r = ULLONG_MAX;
	
	return r;
}

_1i64	__attribute__((overloadable)) add_sat(_1i64 x, _1i64 y)
{
	// Store to temporary buffer
	_1i64 r = x + y;        
	if( y > 0 )
	{
		if( r < x )
		r = LLONG_MAX;
	}
	else
	{
		if( r > x )
		r = LLONG_MIN;
	}
	return r;
}

////////////////////////////////////////////////////////////////////////////////////////
//gentype hadd (gentype x, gentype y) Returns (x + y) >> 1. The intermediate sum does //
//not modulo overflow.                                                                //
////////////////////////////////////////////////////////////////////////////////////////
_1i8	__attribute__((overloadable)) hadd(_1i8 x, _1i8 y)
{
	return ( (short)(x) + (short)(y) ) >> 1 ;
}

_1u8	__attribute__((overloadable)) hadd(_1u8 x, _1u8 y)
{
	return ( (unsigned short)(x) + (unsigned short)(y) ) >> 1 ;
}

_1i16	__attribute__((overloadable)) hadd(_1i16 x, _1i16 y)
{
	return ( (int)(x) + (int)(y) ) >> 1 ;
}

_1u16	__attribute__((overloadable)) hadd(_1u16 x, _1u16 y)
{
	return ( (unsigned int)(x) + (unsigned int)(y) ) >> 1 ;
}

_1i32	__attribute__((overloadable)) hadd(_1i32 x, _1i32 y)
{
	return ( (_1i64)(x) +(_1i64)(y) ) >> 1 ;
}

_1u32	__attribute__((overloadable)) hadd(_1u32 x, _1u32 y)
{
	return ( (_1u64)(x) +(_1u64)(y) ) >> 1 ;
}

_16u8	__attribute__((overloadable)) hadd(_16u8 x, _16u8 y)
{
	__m128i lsb = _mm_and_si128((__m128i) x,(__m128i) *((__m128i *)byteLsbMask) );
	lsb = _mm_and_si128((__m128i) y,(__m128i) lsb );
	x = (_16u8)_mm_andnot_si128(*((__m128i *)byteLsbMask), (__m128i)x);
	x = (_16u8) _mm_srli_epi16((__m128i)x, 1);
	y = (_16u8)_mm_andnot_si128(*((__m128i *)byteLsbMask), (__m128i)y);
	y = (_16u8) _mm_srli_epi16((__m128i)y, 1);
	x = (_16u8) _mm_add_epi8((__m128i)x,(__m128i) y);
	return (_16u8) _mm_add_epi8((__m128i)x,(__m128i) lsb);
}

_16i8	__attribute__((overloadable)) hadd(_16i8 x, _16i8 y)
{
	__m128i lsb = _mm_and_si128((__m128i) x,(__m128i) *((__m128i *)byteLsbMask) );
	lsb = _mm_and_si128((__m128i) y,(__m128i) lsb );
	
	__m128i msb = _mm_and_si128((__m128i)x,(__m128i) *((__m128i *)byteMsbMask));
	x = (_16i8) _mm_andnot_si128(*((__m128i *)byteLsbMask), (__m128i)x);
	x = (_16i8) _mm_srli_epi16((__m128i)x, 1);
	x = (_16i8) _mm_or_si128((__m128i)x,(__m128i) msb);

	msb = _mm_and_si128((__m128i)y,(__m128i) *((__m128i *)byteMsbMask));
	y = (_16i8) _mm_andnot_si128(*((__m128i *)byteLsbMask), (__m128i)y);
	y = (_16i8) _mm_srli_epi16((__m128i)y, 1);
	y = (_16i8) _mm_or_si128((__m128i)y,(__m128i) msb);

	x = (_16i8) _mm_add_epi8((__m128i)x,(__m128i) y);
	return (_16i8) _mm_add_epi8((__m128i)x,(__m128i) lsb);
}

_8u16	__attribute__((overloadable)) hadd(_8u16 x, _8u16 y)
{
	__m128i lsb = _mm_and_si128((__m128i) x,(__m128i) *((__m128i *)wordLsbMask) );
	lsb = _mm_and_si128((__m128i) y,(__m128i) lsb );

	x = (_8u16) _mm_srli_epi16((__m128i)x, 1);
	y = (_8u16) _mm_srli_epi16((__m128i)y, 1);
	x = (_8u16) _mm_add_epi16((__m128i)x,(__m128i) y);
	return (_8u16) _mm_add_epi16((__m128i)x,(__m128i) lsb);
}

_8i16	__attribute__((overloadable)) hadd(_8i16 x, _8i16 y)
{
	__m128i lsb = _mm_and_si128((__m128i) x,(__m128i) *((__m128i *)wordLsbMask) );
	lsb = _mm_and_si128((__m128i) y,(__m128i) lsb );

	x = (_8i16) _mm_srai_epi16((__m128i)x, 1);
	y = (_8i16) _mm_srai_epi16((__m128i)y, 1);
	x = (_8i16) _mm_add_epi16((__m128i)x,(__m128i) y);
	return (_8i16) _mm_add_epi16((__m128i)x,(__m128i) lsb);

}

_4u32	__attribute__((overloadable)) hadd(_4u32 x, _4u32 y)
{
	__m128i lsb = _mm_and_si128((__m128i) x,(__m128i) *((__m128i *)dwordLsbMask) );
	lsb = _mm_and_si128((__m128i) y,(__m128i) lsb );

	x = (_4u32) _mm_srli_epi32((__m128i)x, 1);
	y = (_4u32) _mm_srli_epi32((__m128i)y, 1);
	x = (_4u32) _mm_add_epi32((__m128i)x,(__m128i) y);
	return (_4u32) _mm_add_epi32((__m128i)x,(__m128i) lsb);
}

_4i32	__attribute__((overloadable)) hadd(_4i32 x, _4i32 y)
{
	__m128i lsb = _mm_and_si128((__m128i) x,(__m128i) *((__m128i *)dwordLsbMask) );
	lsb = _mm_and_si128((__m128i) y,(__m128i) lsb );

	x = (_4i32) _mm_srai_epi32((__m128i)x, 1);
	y = (_4i32) _mm_srai_epi32((__m128i)y, 1);
	x = (_4i32) _mm_add_epi32((__m128i)x,(__m128i) y);
	return (_4i32) _mm_add_epi32((__m128i)x,(__m128i) lsb);

}

_2u64	__attribute__((overloadable)) hadd(_2u64 x, _2u64 y)
{
	__m128i lsb = _mm_and_si128((__m128i) x,(__m128i) *((__m128i *)oneZeroOneMask) );
	lsb = _mm_and_si128((__m128i) y,(__m128i) lsb );

	x = (_2u64) _mm_srli_epi64((__m128i)x, 1);
	y = (_2u64) _mm_srli_epi64((__m128i)y, 1);
	x = (_2u64) _mm_add_epi64((__m128i)x,(__m128i) y);
	return (_2u64) _mm_add_epi64((__m128i)x,(__m128i) lsb);
}

_2i64	__attribute__((overloadable)) hadd(_2i64 x, _2i64 y)
{
	 __m128i overflow = _mm_and_si128((__m128i)x,(__m128i) *((__m128i *)oneZeroOneMask));
	 __m128i tmp = _mm_and_si128((__m128i)y,(__m128i) *((__m128i *)oneZeroOneMask));
	 overflow = _mm_add_epi64(overflow, tmp);

	ALIGN16 _1i64 tempX[2], tempY[2], tempZ[2];

	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX,(__m128i) x);
	_mm_store_si128((__m128i *)tempY,(__m128i) y);
	_mm_store_si128((__m128i *)tempZ, overflow);
	#pragma ivdep
	for(int i=0; i<2; ++i)
	{
		tempX[i] = ( (tempX[i]  >> 1 ) + ( tempY[i] >> 1 ) ) + ( tempZ[i] >> 1 );
	}
	return (_2i64) _mm_load_si128((const __m128i *)tempX);

	//__m128i msbMask = _mm_setr_epi32( 0, 0x80, 0, 0x80 );
	//__m128i lsbMask = _mm_setr_epi32( 1, 0, 1, 0 );

	//__m128i lsb = _mm_and_si128( x, lsbMask );
	//lsb = _mm_and_si128( y, lsb );
	//
	//__m128i msb = _mm_and_si128(x, msbMask);
	////x = _mm_andnot_si128(lsbMask, x);
	//x = _mm_srli_epi64(x, 1);
	//x = _mm_or_si128(x, msb);

	//msb = _mm_and_si128(y, msbMask);
	////y = _mm_andnot_si128(lsbMask, y);
	//y = _mm_srli_epi64(y, 1);
	//y = _mm_or_si128(y, msb);

	//x = _mm_add_epi64(x, y);
	//return _mm_add_epi64(x, lsb);
}

_1u64	__attribute__((overloadable)) hadd(_1u64 x, _1u64 y)
{
	_1u64 overflow = ( x & 0x1 ) + ( y & 0x1 );
	return ( ( x >> 1 ) + ( y >> 1 ) ) + ( overflow >> 1 );
}

_1i64	__attribute__((overloadable)) hadd(_1i64 x, _1i64 y)
{
 	_1i64 overflow = ( x & 0x1 ) + ( y & 0x1 );
	return ( ( x >> 1 ) + ( y >> 1 ) ) + ( overflow >> 1 );
}

///////////////////////////////////////////////////////////////////////////////////////
//gentype rhadd (gentype x, gentype y)Returns (x + y + 1) >> 1. The intermediate sum //
//does not modulo overflow.                                                          //
///////////////////////////////////////////////////////////////////////////////////////
_1i8	__attribute__((overloadable)) rhadd(_1i8 x, _1i8 y)
{
	return (short)(x) + (short)(y) + 1 >> 1 ;
}

_1u8	__attribute__((overloadable)) rhadd(_1u8 x, _1u8 y)
{
	return (unsigned short)(x) + (unsigned short)(y) + 1 >> 1 ;
}

_1i16	__attribute__((overloadable)) rhadd(_1i16 x, _1i16 y)
{
	return (int)(x) + (int)(y) + 1 >> 1 ;
}

_1u16	__attribute__((overloadable)) rhadd(_1u16 x, _1u16 y)
{
	return (unsigned int)(x) + (unsigned int)(y) + 1 >> 1 ;
}

_1i32	__attribute__((overloadable)) rhadd(_1i32 x, _1i32 y)
{
	return (_1i64)(x) + (_1i64)(y) + 1 >> 1 ;
}

_1u32	__attribute__((overloadable)) rhadd(_1u32 x, _1u32 y)
{
	return (_1u64)(x) + (_1u64)(y) + 1 >> 1 ;
}


_16u8	__attribute__((overloadable)) rhadd(_16u8 x, _16u8 y)
{
	return (_16u8)_mm_avg_epu8((__m128i)x,(__m128i) y);
}

_16i8	__attribute__((overloadable)) rhadd(_16i8 x, _16i8 y)
{
	__m128i lsbx = _mm_and_si128((__m128i) x,(__m128i) *((__m128i *)byteLsbMask) );
	__m128i lsby = _mm_and_si128((__m128i) y,(__m128i) *((__m128i *)byteLsbMask) );
	__m128i lsb = _mm_or_si128( lsbx, lsby );
	
	__m128i msb = _mm_and_si128((__m128i)x,(__m128i) *((__m128i *)byteMsbMask));
	x = (_16i8)_mm_andnot_si128(*((__m128i *)byteLsbMask), (__m128i)x);
	x = (_16i8) _mm_srli_epi16((__m128i)x, 1);
	x = (_16i8) _mm_or_si128((__m128i)x,(__m128i) msb);

	msb = _mm_and_si128((__m128i)y,(__m128i) *((__m128i *)byteMsbMask));
	y = (_16i8)_mm_andnot_si128(*((__m128i *)byteLsbMask), (__m128i)y);
	y = (_16i8) _mm_srli_epi16((__m128i)y, 1);
	y = (_16i8) _mm_or_si128((__m128i)y,(__m128i) msb);

	x = (_16i8) _mm_add_epi8((__m128i)x,(__m128i) y);
	return (_16i8) _mm_add_epi8((__m128i)x,(__m128i) lsb);
}

_8i16	__attribute__((overloadable)) rhadd(_8i16 x, _8i16 y)
{
	__m128i lsbx = _mm_and_si128((__m128i) x,(__m128i) *((__m128i *)wordLsbMask) );
	__m128i lsby = _mm_and_si128((__m128i) y,(__m128i) *((__m128i *)wordLsbMask) );
	__m128i lsb = _mm_or_si128( lsbx, lsby );

	x = (_8i16) _mm_srai_epi16((__m128i)x, 1);
	y = (_8i16) _mm_srai_epi16((__m128i)y, 1);
	x = (_8i16) _mm_add_epi16((__m128i)x,(__m128i) y);
	return (_8i16) _mm_add_epi16((__m128i)x,(__m128i) lsb);
}

_8u16	__attribute__((overloadable)) rhadd(_8u16 x, _8u16 y)
{
	return (_8u16) _mm_avg_epu16((__m128i)x,(__m128i) y);
}

_4i32	__attribute__((overloadable)) rhadd(_4i32 x, _4i32 y)
{
	__m128i lsbx = _mm_and_si128((__m128i) x,(__m128i) *((__m128i *)dwordLsbMask) );
	__m128i lsby = _mm_and_si128((__m128i) y,(__m128i) *((__m128i *)dwordLsbMask) );
	__m128i lsb = _mm_or_si128( lsbx, lsby );

	x = (_4i32) _mm_srai_epi32((__m128i)x, 1);
	y = (_4i32) _mm_srai_epi32((__m128i)y, 1);
	x = (_4i32) _mm_add_epi32((__m128i)x,(__m128i) y);
	return (_4i32) _mm_add_epi32((__m128i)x,(__m128i) lsb);
}

_4u32	__attribute__((overloadable)) rhadd(_4u32 x, _4u32 y)
{
	__m128i lsbx = _mm_and_si128((__m128i) x,(__m128i) *((__m128i *)dwordLsbMask) );
	__m128i lsby = _mm_and_si128((__m128i) y,(__m128i) *((__m128i *)dwordLsbMask) );
	__m128i lsb = _mm_or_si128( lsbx, lsby );

	x = (_4u32) _mm_srli_epi32((__m128i)x, 1);
	y = (_4u32) _mm_srli_epi32((__m128i)y, 1);
	x = (_4u32) _mm_add_epi32((__m128i)x,(__m128i) y);
	return (_4u32) _mm_add_epi32((__m128i)x,(__m128i) lsb);

}

_2i64	__attribute__((overloadable)) rhadd(_2i64 x, _2i64 y)
{
	ALIGN16  _1i64 tempX[2];
	ALIGN16  _1i64 tempY[2];
	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX,(__m128i) x);
	_mm_store_si128((__m128i *)tempY,(__m128i) y);
	#pragma ivdep
	for(int i=0; i<2; ++i)
	{
		tempX[i] = rhadd(tempX[i], tempY[i]);
	}

	return (_2i64) _mm_load_si128((const __m128i *)tempX);
}

_2u64	__attribute__((overloadable)) rhadd(_2u64 x, _2u64 y)
{
	__m128i lsbx = _mm_and_si128((__m128i) x,(__m128i) *((__m128i *)oneZeroOneMask) );
	__m128i lsby = _mm_and_si128((__m128i) y,(__m128i) *((__m128i *)oneZeroOneMask) );
	__m128i lsb = _mm_or_si128( lsbx, lsby );

	x = (_2u64) _mm_srli_epi64((__m128i)x, 1);
	y = (_2u64) _mm_srli_epi64((__m128i)y, 1);
	x = (_2u64) _mm_add_epi64((__m128i)x,(__m128i) y);
	return (_2u64) _mm_add_epi64((__m128i)x,(__m128i) lsb);
}

_1i64	__attribute__((overloadable)) rhadd(_1i64 x, _1i64 y)
{
	_1i64 overflow = ( x | y ) & 0x1;
	return ( ( x >> 1 ) + ( y >> 1 ) ) + overflow;

}

_1u64	__attribute__((overloadable)) rhadd(_1u64 x, _1u64 y)
{
	_1u64 overflow = ( x | y ) & 0x1;
	return ( ( x >> 1 ) + ( y >> 1 ) ) + overflow;
}

////////////////////////////////////////////////////////////////////////////////
//gentype clz (gentype x) Returns the number of leading 0-bits in x, starting //
//at the most significant bit position.                                       //
////////////////////////////////////////////////////////////////////////////////


_1i8	__attribute__((overloadable)) clz(_1i8 x)
{
	return __builtin_clzc(x);
}

_1u8	__attribute__((overloadable)) clz(_1u8 x)
{
	return __builtin_clzc(x);
}


_1i16	__attribute__((overloadable)) clz(_1i16 x)
{
	return __builtin_clzs(x);
}

_1u16	__attribute__((overloadable)) clz(_1u16 x)
{
	return __builtin_clzs(x);
}

_1i32	__attribute__((overloadable)) clz(_1i32 x)
{
	return __builtin_clz(x);
}

_1u32	__attribute__((overloadable)) clz(_1u32 x)
{
	return __builtin_clz(x);
}

_1i64	__attribute__((overloadable)) clz(_1i64 x)
{
    return __builtin_clzl(x);
}

_1u64	__attribute__((overloadable)) clz( _1u64 x)
{
	return __builtin_clzl(x);
}

////////////////////////////////////////////////////
//gentype mad_hi (gentype a,gentype b, gentype c) //
//Returns mad_hi(a, b) + c.                       //
////////////////////////////////////////////////////
_1i8	__attribute__((overloadable)) mad_hi(_1i8 x, _1i8 y, _1i8 z)
{
	x = mul_hi(x,y);
	return x+z;
}

_1u8	__attribute__((overloadable)) mad_hi(_1u8 x, _1u8 y, _1u8 z)
{
	x = mul_hi(x,y);
	return x+z;
}

_1i16	__attribute__((overloadable)) mad_hi(_1i16 x, _1i16 y, _1i16 z)
{
	x = mul_hi(x,y);
	return x+z;
}

_1u16	__attribute__((overloadable)) mad_hi(_1u16 x, _1u16 y, _1u16 z)
{
	x = mul_hi(x,y);
	return x+z;
}

_1i32	__attribute__((overloadable)) mad_hi(_1i32 x, _1i32 y, _1i32 z)
{
	x = mul_hi(x,y);
	return x+z;

}

_1u32	__attribute__((overloadable)) mad_hi(_1u32 x,_1u32 y, _1u32 z)
{
	x = mul_hi(x,y);
	return x+z;
}


_16i8	__attribute__((overloadable)) mad_hi(_16i8 x, _16i8 y, _16i8 z)
{
	x = mul_hi(x,y);
	return (_16i8) _mm_add_epi8((__m128i)x,(__m128i) z);
}

_16u8	__attribute__((overloadable)) mad_hi(_16u8 x, _16u8 y, _16u8 z)
{
	x = mul_hi(x,y);
	return (_16u8) _mm_add_epi8((__m128i)x,(__m128i) z); 
}

_8i16	__attribute__((overloadable)) mad_hi(_8i16 x, _8i16 y, _8i16 z)
{
	x = mul_hi(x,y);
	return (_8i16) _mm_add_epi16((__m128i)x,(__m128i) z);
}

_8u16	__attribute__((overloadable)) mad_hi(_8u16 x, _8u16 y, _8u16 z)
{
	x = mul_hi(x,y);
	return (_8u16) _mm_add_epi16((__m128i)x,(__m128i) z); 
}

_4i32	__attribute__((overloadable)) mad_hi(_4i32 x, _4i32 y, _4i32 z)
{
	x = mul_hi(x,y);
	return (_4i32) _mm_add_epi32((__m128i)x,(__m128i) z);
}

_4u32	__attribute__((overloadable)) mad_hi(_4u32 x, _4u32 y, _4u32 z)
{
	x = mul_hi(x,y);
	return (_4u32) _mm_add_epi32((__m128i)x,(__m128i) z); 
}

_2i64	__attribute__((overloadable)) mad_hi(_2i64 x, _2i64 y, _2i64 z)
{
	x = mul_hi(x,y);
	return (_2i64) _mm_add_epi64((__m128i)x,(__m128i) z);
}

_2u64	__attribute__((overloadable)) mad_hi(_2u64 x, _2u64 y, _2u64 z)
{
	x = mul_hi(x,y);
	return (_2u64) _mm_add_epi64((__m128i)x,(__m128i) z); 
}

_1i64	__attribute__((overloadable)) mad_hi(_1i64 x, _1i64 y, _1i64 z)
{
	x = mul_hi(x,y);
	return x+z;
}

_1u64	__attribute__((overloadable)) mad_hi(_1u64 x,_1u64 y,_1u64 z)
{
	x = mul_hi(x,y);
	return x+z;
}


///////////////////////////////////////////////////
//gentype clamp (gentype a,gentype b, gentype c) //
//Returns min(max(x, y), z).                     //
///////////////////////////////////////////////////
_1i8	__attribute__((overloadable)) clamp(_1i8 x, _1i8 y, _1i8 z)
{
	return min(max(x, y), z);
}
_1u8	__attribute__((overloadable)) clamp(_1u8 x, _1u8 y, _1u8 z)
{
	return min(max(x, y), z);
}
_1i16	__attribute__((overloadable)) clamp(_1i16 x, _1i16 y, _1i16 z)
{
	return min(max(x, y), z);
}
_1u16	__attribute__((overloadable)) clamp(_1u16 x, _1u16 y, _1u16 z)
{
	return min(max(x, y), z);
}
_1i32	__attribute__((overloadable)) clamp(_1i32 x, _1i32 y, _1i32 z)
{
	return min(max(x, y), z);

}
_1u32	__attribute__((overloadable)) clamp(_1u32 x, _1u32 y, _1u32 z)
{
	return min(max(x, y), z);
}

_16i8	__attribute__((overloadable)) clamp(_16i8 x, _16i8 y, _16i8 z)
{
	return min(max(x, y), z);
}
_16u8	__attribute__((overloadable)) clamp(_16u8 x, _16u8 y, _16u8 z)
{
	return min(max(x, y), z);
}
_8i16	__attribute__((overloadable)) clamp(_8i16 x, _8i16 y, _8i16 z)
{
	return min(max(x, y), z);
}
_8u16	__attribute__((overloadable)) clamp(_8u16 x, _8u16 y, _8u16 z)
{
	return min(max(x, y), z); 
}
_4i32	__attribute__((overloadable)) clamp(_4i32 x, _4i32 y, _4i32 z)
{
	return min(max(x, y), z);
}
_4u32	__attribute__((overloadable)) clamp(_4u32 x, _4u32 y, _4u32 z)
{
	return min(max(x, y), z); 
}
_2i64	__attribute__((overloadable)) clamp(_2i64 x, _2i64 y, _2i64 z)
{
	return min(max(x, y), z);
}
_2u64	__attribute__((overloadable)) clamp(_2u64 x, _2u64 y, _2u64 z)
{
	return min(max(x, y), z);
}

_1i64	__attribute__((overloadable)) clamp(_1i64 x, _1i64 y, _1i64 z)
{
	return min(max(x, y), z);
}
_1u64	__attribute__((overloadable)) clamp(_1u64 x,_1u64 y,_1u64 z)
{
	return min(max(x, y), z);
}


/////////////////////////////////////////////////////
//gentype mad_sat (gentype a,gentype b, gentype c) //
//Returns a * b + c and saturates the result.      //
/////////////////////////////////////////////////////
_1i8	__attribute__((overloadable)) mad_sat(_1i8 x, _1i8 y, _1i8 z)
{
	short res;
	res = ( (short) x * (short) y ) + z;
    res = __max( res, CHAR_MIN );
    res = __min( res, CHAR_MAX );
	return (char)res;
}

_1u8	__attribute__((overloadable)) mad_sat(_1u8 x, _1u8 y, _1u8 z)
{
	unsigned short res;
	res = ( (unsigned short) x * (unsigned short) y ) + z;
    res = __min( res, UCHAR_MAX );
	return (unsigned char)res;
}


_1i16	__attribute__((overloadable)) mad_sat(_1i16 x, _1i16 y, _1i16 z)
{
	int res;
	res = ( (int) x * (int) y ) + z;
    res = __max( res, SHRT_MIN );
    res = __min( res, SHRT_MAX );
	return (short)res;
}


_1u16	__attribute__((overloadable)) mad_sat(_1u16 x, _1u16 y, _1u16 z)
{
	unsigned int res;
	res = ( (unsigned int) x * (unsigned int) y ) + z;
    res = __min( res, USHRT_MAX );
	return (unsigned short)res;
}

_1i32	__attribute__((overloadable)) mad_sat(_1i32 x, _1i32 y, _1i32 z)
{
	_1i64 res;
	res = ( (_1i64) x * (_1i64) y ) + z;
    res = __max( res, INT_MIN );
    res = __min( res, INT_MAX );
	return (int)res;
}

_1u32	__attribute__((overloadable)) mad_sat(_1u32 x, _1u32 y, _1u32 z)
{
	_1u64 res;
	res = ( (_1u64) x * (_1u64) y ) + z;
    res = __min( res, UINT_MAX );
	return (unsigned int)res;
}



_16i8	__attribute__((overloadable)) mad_sat(_16i8 x, _16i8 y, _16i8 z)
{
	__m128i zero = _mm_setzero_si128();

	__m128i lox = _mm_unpacklo_epi8((__m128i)zero,(__m128i) x);
	lox = _mm_srai_epi16(lox, 8);
	__m128i loy = _mm_unpacklo_epi8((__m128i)zero,(__m128i) y);
	loy = _mm_srai_epi16(loy, 8);
	__m128i loz = _mm_unpacklo_epi8((__m128i)zero,(__m128i) z);
	loz = _mm_srai_epi16(loz, 8);

	__m128i reslo = _mm_mullo_epi16(lox, loy);
	reslo = _mm_add_epi16(reslo, loz);
	reslo = _mm_max_epi16(reslo, *((__m128i *)wordCharMin));
	reslo = _mm_min_epi16(reslo, *((__m128i *)wordCharMax));

	__m128i hix = _mm_unpackhi_epi8((__m128i)zero,(__m128i) x);
	hix = _mm_srai_epi16(hix, 8);
	__m128i hiy = _mm_unpackhi_epi8((__m128i)zero,(__m128i) y);
	hiy = _mm_srai_epi16(hiy, 8);
	__m128i hiz = _mm_unpackhi_epi8((__m128i)zero,(__m128i) z);
	hiz = _mm_srai_epi16(hiz, 8);

	__m128i reshi = _mm_mullo_epi16(hix, hiy);
	reshi = _mm_add_epi16(reshi, hiz);
	reshi = _mm_max_epi16(reshi, *((__m128i *)wordCharMin));
	reshi = _mm_min_epi16(reshi, *((__m128i *)wordCharMax));

	__m128i res = _mm_packs_epi16(reslo, reshi);
	return (_16i8) res;
}

_16u8	__attribute__((overloadable)) mad_sat(_16u8 x, _16u8 y, _16u8 z)
{
	__m128i zero = _mm_setzero_si128();

	__m128i lox = _mm_unpacklo_epi8((__m128i)x,(__m128i) zero);
	__m128i loy = _mm_unpacklo_epi8((__m128i)y,(__m128i) zero);
	__m128i loz = _mm_unpacklo_epi8((__m128i)z,(__m128i) zero);

	__m128i reslo = _mm_mullo_epi16(lox, loy);
	reslo = _mm_add_epi16(reslo, loz);

    reslo = (__m128i)min((_8u16)reslo,(_8u16) *((__m128i *)wordUCharMax));

	__m128i hix = _mm_unpackhi_epi8((__m128i)x,(__m128i) zero);
	__m128i hiy = _mm_unpackhi_epi8((__m128i)y,(__m128i) zero);
	__m128i hiz = _mm_unpackhi_epi8((__m128i)z,(__m128i) zero);

	__m128i reshi = _mm_mullo_epi16(hix, hiy);
	reshi = _mm_add_epi16(reshi, hiz);

	reshi = (__m128i)min((_8u16)reshi,(_8u16) *((__m128i *)wordUCharMax));

	__m128i res = _mm_packus_epi16(reslo, reshi);
	return (_16u8) res;
}

_8i16	__attribute__((overloadable)) mad_sat(_8i16 x, _8i16 y, _8i16 z)
{
#ifdef __SSE4_1__
	__m128i zero = _mm_setzero_si128();

	__m128i lox = _mm_unpacklo_epi16((__m128i)zero,(__m128i) x);
	lox = _mm_srai_epi32(lox, 16);
	__m128i loy = _mm_unpacklo_epi16((__m128i)zero,(__m128i) y);
	loy = _mm_srai_epi32(loy, 16);
	__m128i loz = _mm_unpacklo_epi16((__m128i)zero,(__m128i) z);
	loz = _mm_srai_epi32(loz, 16);

	__m128i reslo = _mm_mullo_epi32(lox, loy);
	reslo = _mm_add_epi32(reslo, loz);
	reslo = _mm_max_epi32(reslo, *((__m128i *)dwordShortMin));
	reslo = _mm_min_epi32(reslo, *((__m128i *)dwordShortMax));

	__m128i hix = _mm_unpackhi_epi16((__m128i)zero,(__m128i) x);
	hix = _mm_srai_epi32(hix, 16);
	__m128i hiy = _mm_unpackhi_epi16((__m128i)zero,(__m128i) y);
	hiy = _mm_srai_epi32(hiy, 16);
	__m128i hiz = _mm_unpackhi_epi16((__m128i)zero,(__m128i) z);
	hiz = _mm_srai_epi32(hiz, 16);

	__m128i reshi = _mm_mullo_epi32(hix, hiy);
	reshi = _mm_add_epi32(reshi, hiz);
	reshi = _mm_max_epi32(reshi, *((__m128i *)dwordShortMin));
	reshi = _mm_min_epi32(reshi, *((__m128i *)dwordShortMax));

	__m128i res = _mm_packs_epi32(reslo, reshi);
	return (_8i16) res;
#else
	ALIGN16 short tempX[8], tempY[8], tempZ[8], result[8];

	// Store to temporary buffer
	_mm_store_si128((__m128i*)tempX,(__m128i) x);
	_mm_store_si128((__m128i*)tempY,(__m128i) y);
	_mm_store_si128((__m128i*)tempZ,(__m128i) z);

    _1i64 mx, my, mz, multHi;
	_1u64 multLo, sum;
 	#pragma ivdep
	for(int i=0; i<8; ++i)
	{
       mx = tempX[i];
       my = tempY[i];
	   mz = tempZ[i];

	   multiply_signed_64_by_64( mx, my, &multLo, &multHi );
		
	sum = multLo + mz;
        // carry if overflow
    if( mz >= 0 )
    {
        if( multLo > sum )
        {
            multHi++;
            if( LLONG_MIN == multHi )
            {
                multHi = LLONG_MAX;
                sum = ULLONG_MAX;
            }
        }
    }
    else
    {
        if( multLo < sum )
		{
			multHi--;
			if( LLONG_MAX == multHi )
				{
					multHi = LLONG_MIN;
					sum = 0;
				}
		}
	}

	// saturate
	if( multHi > 0 )
		sum = LLONG_MAX;
	else if( multHi < -1 )
		sum = LLONG_MIN;
	
	 _1i64 res = (_1i64) sum;
	 res = __min( res, (_1i64) SHRT_MAX );
	 res = __max( res, (_1i64) SHRT_MIN );
	 result[i] = (short)res;
	}

	return (_8i16) _mm_load_si128((const __m128i *)result);
#endif
}

_8u16	__attribute__((overloadable)) mad_sat(_8u16 x, _8u16 y, _8u16 z)
{
#ifdef __SSE4_1__
	__m128i zero = _mm_setzero_si128();

	__m128i lox = _mm_unpacklo_epi16((__m128i)x,(__m128i) zero);
	__m128i loy = _mm_unpacklo_epi16((__m128i)y,(__m128i) zero);
	__m128i loz = _mm_unpacklo_epi16((__m128i)z,(__m128i) zero);

	__m128i reslo = _mm_mullo_epi32(lox, loy);
	reslo = _mm_add_epi32(reslo, loz);

	reslo = (__m128i)min((_4u32)reslo,(_4u32) *((__m128i *)dwordUShortMax));

	__m128i hix = _mm_unpackhi_epi16((__m128i)x,(__m128i) zero);
	__m128i hiy = _mm_unpackhi_epi16((__m128i)y,(__m128i) zero);
	__m128i hiz = _mm_unpackhi_epi16((__m128i)z,(__m128i) zero);

	__m128i reshi = _mm_mullo_epi32(hix, hiy);
	reshi = _mm_add_epi32(reshi, hiz);

	reshi = (__m128i)min((_4u32)reshi,(_4u32) *((__m128i *)dwordUShortMax));

	__m128i res = _mm_packus_epi32(reslo, reshi);
	return (_8u16) res;
#else
	ALIGN16 unsigned short tempX[8], tempY[8], tempZ[8], result[8];
	unsigned int size = sizeof(tempX);
	// Store to temporary buffer
	_mm_store_si128((__m128i*)tempX,(__m128i) x);
	_mm_store_si128((__m128i*)tempY,(__m128i) y);
	_mm_store_si128((__m128i*)tempZ,(__m128i) z);

	#pragma ivdep
	for(int i=0; i<8; ++i)
	{
		_1u64 multHi, multLo, mx, my, mz;
		mx = (_1u64)tempX[i];
		my = (_1u64)tempY[i];
		mz = (_1u64)tempZ[i];

		multiply_unsigned_64_by_64( mx, my, &multLo, &multHi );

        multLo += mz;
        multHi += multLo < mz;  // carry if overflow
        if( multHi )
            multLo = 0xFFFFFFFFFFFFFFFFUL;
                
		result[i] = __min( multLo, (_1u64) USHRT_MAX );
	}

	return (_8u16) _mm_load_si128((const __m128i *)result);
#endif
}

_4i32	__attribute__((overloadable)) mad_sat(_4i32 x, _4i32 y, _4i32 z)
{
#ifdef __SSE4_1__
	__m128i zero = _mm_setzero_si128();
	__m128i loz = _mm_and_si128((__m128i)z,(__m128i) *((__m128i *)dwordEvenMask));

	__m128i reslo = _mm_mul_epi32((__m128i)x,(__m128i) y);
	reslo = _mm_add_epi32(reslo, loz);

	reslo = (__m128i)max((_2i64)reslo,(_2i64) *((__m128i *)qwordIntMin));

	reslo = (__m128i)min((_2i64)reslo,(_2i64) *((__m128i *)qwordIntMax));

	reslo = _mm_and_si128(reslo, *((__m128i *)dwordEvenMask));

	x = (_4i32) _mm_srli_epi64((__m128i)x, 32);
	y = (_4i32) _mm_srli_epi64((__m128i)y, 32);
	z = (_4i32) _mm_srli_epi64((__m128i)z, 32);

	__m128i reshi = _mm_mul_epi32((__m128i)x,(__m128i) y);
	reshi = _mm_add_epi32((__m128i)reshi,(__m128i) z);

	reshi = (__m128i)max((_2i64)reshi,(_2i64) *((__m128i *)qwordIntMin));

	reshi = (__m128i)min((_2i64)reshi,(_2i64) *((__m128i *)qwordIntMax));

	reshi = _mm_slli_epi64(reshi, 32);

	__m128i res = _mm_or_si128(reshi, reslo);

	return (_4i32) res;
#else
	ALIGN16 int tempX[4], tempY[4], tempZ[4], result[4];
	unsigned int size = sizeof(tempX);
	// Store to temporary buffer
	_mm_store_si128((__m128i*)tempX,(__m128i) x);
	_mm_store_si128((__m128i*)tempY,(__m128i) y);
	_mm_store_si128((__m128i*)tempZ,(__m128i) z);

	_1i64 mx, my, mz, multHi;
	_1u64 sum, multLo;
	#pragma ivdep
	for(int i=0; i<4; ++i)
	{
       mx = tempX[i];
       my = tempY[i];
	   mz = tempZ[i];

	   multiply_signed_64_by_64( mx, my, &multLo, &multHi );
		
	sum = multLo + mz;
        // carry if overflow
    if( mz >= 0 )
    {
        if( multLo > sum )
        {
            multHi++;
            if( LLONG_MIN == multHi )
            {
                multHi = LLONG_MAX;
                sum = ULLONG_MAX;
            }
        }
    }
    else
    {
        if( multLo < sum )
		{
			multHi--;
			if( LLONG_MAX == multHi )
				{
					multHi = LLONG_MIN;
					sum = 0;
				}
		}
	}

	// saturate
	if( multHi > 0 )
		sum = LLONG_MAX;
	else if( multHi < -1 )
		sum = LLONG_MIN;
	
	 _1i64 res = (_1i64) sum;
	 res = __min( res, (_1i64) INT_MAX );
	 res = __max( res, (_1i64) INT_MIN );
	 result[i] = (int)res;
	}
	return (_4i32) _mm_load_si128((const __m128i *)result);
#endif
}


_4u32	__attribute__((overloadable)) mad_sat(_4u32 x, _4u32 y, _4u32 z)
{
	__m128i zero = _mm_setzero_si128();

	__m128i loz = _mm_and_si128((__m128i)z,(__m128i) *((__m128i *)dwordEvenMask));

	__m128i reslo = _mm_mul_epu32((__m128i)x,(__m128i) y);
	reslo = _mm_add_epi32(reslo, loz);

	reslo = (__m128i)min((_2u64)reslo,(_2u64) *((__m128i *)qwordUIntMax));

	reslo = _mm_and_si128(reslo, *((__m128i *)dwordEvenMask));

	x = (_4u32) _mm_srli_epi64((__m128i)x, 32);
	y = (_4u32) _mm_srli_epi64((__m128i)y, 32);
	z = (_4u32) _mm_srli_epi64((__m128i)z, 32);

	__m128i reshi = _mm_mul_epu32((__m128i)x,(__m128i) y);
	reshi = _mm_add_epi32((__m128i)reshi,(__m128i) z);

	reshi = (__m128i)min((_2u64)reshi,(_2u64) *((__m128i *)qwordUIntMax));

	reshi = _mm_slli_epi64(reshi, 32);

	__m128i res = _mm_or_si128(reshi, reslo);

	return (_4u32) res;
}


_2u64	__attribute__((overloadable)) mad_sat(_2u64 x, _2u64 y, _2u64 z)
{
	ALIGN16 _1u64 tempX[2], tempY[2], tempZ[2], result[2];
	_mm_store_si128((__m128i*)tempX,(__m128i) x);
	_mm_store_si128((__m128i*)tempY,(__m128i) y);
	_mm_store_si128((__m128i*)tempZ,(__m128i) z);

	#pragma ivdep
	for(int i=0; i<2; ++i)
	{
		result[i] = mad_sat(tempX[i], tempY[i], tempZ[i]);
	}

	return (_2u64) _mm_load_si128((const __m128i *)result);
}


_2i64	__attribute__((overloadable)) mad_sat(_2i64 x, _2i64 y, _2i64 z)
{
	ALIGN16 _1i64 tempX[2], tempY[2], tempZ[2], result[2];

	_mm_store_si128((__m128i*)tempX,(__m128i) x);
	_mm_store_si128((__m128i*)tempY,(__m128i) y);
	_mm_store_si128((__m128i*)tempZ,(__m128i) z);

	#pragma ivdep
	for(int i=0; i<2; ++i)
	{
		result[i] = mad_sat(tempX[i], tempY[i], tempZ[i]);
	}

	return (_2i64) _mm_load_si128((const __m128i *)result);
}


_1u64	__attribute__((overloadable)) mad_sat(_1u64 x, _1u64 y, _1u64 z)
{
	_1u64 multHi, multLo;
	multiply_unsigned_64_by_64( x, y, &multLo, &multHi );
	multLo += z;
	multHi += multLo < z;  // carry if overflow
	if( multHi )
		multLo = 0xFFFFFFFFFFFFFFFFUL;

	return multLo;
}

_1i64	__attribute__((overloadable)) mad_sat(_1i64 x, _1i64 y, _1i64 z)
{
	_1i64 multHi;
	_1u64 multLo;
	multiply_signed_64_by_64( x, y, &multLo, &multHi );		
	_1u64 sum = multLo + z;
	// carry if overflow
    if( z >= 0 )
    {
       if( multLo > sum )
        {
            multHi++;
            if( LLONG_MIN == multHi )
           {
                multHi = LLONG_MAX;
                sum = ULLONG_MAX;
            }
        }
    }
    else
    {
        if( multLo < sum )
        {
           multHi--;
            if( LLONG_MAX == multHi )
            {
                multHi = LLONG_MIN;
                sum = 0;
            }
        }
    }
   
    // saturate
    if( multHi > 0 )
        sum = LLONG_MAX;
    else if( multHi < -1 )
        sum = LLONG_MIN;

	return sum;	
}

///////////////////////////////////////
//gentype max (gentype x, gentype y) //
///////////////////////////////////////
_1i8	__attribute__((overloadable)) max(_1i8 x, _1i8 y)
{
	return __max(x,y);
}

_1u8	__attribute__((overloadable)) max(_1u8 x, _1u8 y)
{
	return __max(x,y);
}

_1i16	__attribute__((overloadable)) max(_1i16 x, _1i16 y)
{
	return __max(x,y);
}

_1u16	__attribute__((overloadable)) max(_1u16 x, _1u16 y)
{
	return __max(x,y);
}

_1i32	__attribute__((overloadable)) max(_1i32 x, _1i32 y)
{
	return __max(x,y);
}

_1u32	__attribute__((overloadable)) max(_1u32 x, _1u32 y)
{
	return __max(x,y);
}


_16u8	__attribute__((overloadable)) max(_16u8 x, _16u8 y)
{
	return (_16u8)_mm_max_epu8((__m128i)x ,(__m128i) y);  
}

_16i8	__attribute__((overloadable)) max(_16i8 x, _16i8 y)
{
#ifdef __SSE4_1__
	return (_16i8)_mm_max_epi8((__m128i)x , (__m128i)y);  
#else
	x = (_16i8)_mm_xor_si128( *((__m128i *)byteMsbMask), (__m128i)x );
	y = (_16i8)_mm_xor_si128( *((__m128i *)byteMsbMask), (__m128i)y );
	return (_16i8)_mm_xor_si128( _mm_max_epu8((__m128i)x , (__m128i)y), *((__m128i *)byteMsbMask) ); 
#endif
}

_8u16	__attribute__((overloadable)) max(_8u16 x, _8u16 y)
{
#ifdef __SSE4_1__
	return (_8u16)_mm_max_epu16((__m128i)x , (__m128i)y);  
#else
	x = (_8u16)_mm_xor_si128( *((__m128i *)wordMsbMask), (__m128i)x );
	y = (_8u16)_mm_xor_si128( *((__m128i *)wordMsbMask), (__m128i)y );
	return (_8u16)_mm_xor_si128( _mm_max_epi16((__m128i)x , (__m128i)y), *((__m128i *)wordMsbMask) ); 
#endif
}

_8i16	__attribute__((overloadable)) max(_8i16 x, _8i16 y)
{
	return (_8i16)_mm_max_epi16((__m128i)x , (__m128i)y);  
}

_4u32	__attribute__((overloadable)) max(_4u32 x, _4u32 y)
{
#ifdef __SSE4_1__
	return (_4u32) _mm_max_epu32((__m128i)x ,(__m128i) y);  
#else
	x = (_4u32) _mm_xor_si128( *((__m128i *)dwordMsbMask), (__m128i)x );
	y = (_4u32) _mm_xor_si128( *((__m128i *)dwordMsbMask), (__m128i)y );
	__m128i XgtY = _mm_cmpgt_epi32((__m128i)x,(__m128i) y);
	__m128i res = _mm_and_si128((__m128i)XgtY,(__m128i) x);
	res = _mm_or_si128(_mm_andnot_si128((__m128i)XgtY, (__m128i)y),(__m128i) res);
	return (_4u32) _mm_xor_si128( *((__m128i *)dwordMsbMask), res );
#endif
}

/// !!! This function is copy-pasted to images module.
/// In case of any changes they should also be done in image_callback_functions.cpp
_4i32	__attribute__((overloadable)) max(_4i32 x, _4i32 y)
{
#ifdef __SSE4_1__
	return (_4i32) _mm_max_epi32((__m128i)x ,(__m128i) y);  
#else
 	__m128i mask = _mm_cmpgt_epi32((__m128i)x,(__m128i) y);
	__m128i res = _mm_and_si128((__m128i)mask,(__m128i) x);
	return (_4i32) _mm_or_si128(_mm_andnot_si128((__m128i)mask, (__m128i)y),(__m128i) res);
#endif

}


_2i64	__attribute__((overloadable)) max(_2i64 x, _2i64 y)
{
#ifdef __AVX_1__
	 __m128i tmpX, tmpY, tmp;
	tmp = _mm_cmpgt_epi64(x, y); //tmp = 1 means x > y else tmp = 0
	tmpX = _mm_and_si128(tmp, x); //tmpX = if x>y x else 0
	tmpY = _mm_andnot_si128(tmp, y); //tmpY = if x<y y else 0
	return _mm_add_epi64(tmpX , tmpY);  //return x or y
#else
	_1i64 ALIGN16 tempX[2], tempY[2];

	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX,(__m128i) x);
	_mm_store_si128((__m128i *)tempY,(__m128i) y);
	#pragma ivdep
	for(int i=0; i<2; ++i)
	{
		tempX[i] = __max(tempX[i] , tempY[i]);
	}

	return (_2i64) _mm_load_si128((const __m128i *)tempX);
#endif
}


_2u64	__attribute__((overloadable)) max(_2u64 x, _2u64 y)
{
	_1u64 ALIGN16 tempX[2], tempY[2];

	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX,(__m128i) x);
	_mm_store_si128((__m128i *)tempY,(__m128i) y);
	#pragma ivdep
	for(int i=0; i<2; ++i)
	{
		tempX[i] = __max(tempX[i], tempY[i]);
	}

	return (_2u64) _mm_load_si128((const __m128i *)tempX);

}


_1u64	__attribute__((overloadable)) max(_1u64 x, _1u64 y)
{
	return __max(x, y);
}

_1i64	__attribute__((overloadable)) max(_1i64 x, _1i64 y)
{
		return __max(x, y);
}

///////////////////////////////////////////////////////////////////////////////////
//gentype min (gentype x, gentype y) Returns y if y < x, otherwise it returns x. //
///////////////////////////////////////////////////////////////////////////////////
_1i8	__attribute__((overloadable)) min(_1i8 x, _1i8 y)
{
	return __min(x,y);
}

_1u8	__attribute__((overloadable)) min(_1u8 x, _1u8 y)
{
	return __min(x,y);
}

_1i16   __attribute__((overloadable)) min(_1i16 x, _1i16 y)
{
	return __min(x,y);
}

_1u16   __attribute__((overloadable)) min(_1u16 x, _1u16 y)
{
	return __min(x,y);
}

_1i32   __attribute__((overloadable)) min(_1i32 x, _1i32 y)
{
	return __min(x,y);
}

_1u32	__attribute__((overloadable)) min(_1u32 x, _1u32 y)
{
		return __min(x,y);
}

_16u8	__attribute__((overloadable)) min(_16u8 x, _16u8 y)
{
	return (_16u8) _mm_min_epu8((__m128i)x ,(__m128i) y);  
}

_16i8	__attribute__((overloadable)) min(_16i8 x, _16i8 y)
{
#ifdef __SSE4_1__
	return (_16i8) _mm_min_epi8((__m128i)x ,(__m128i) y);  
#else
	x = (_16i8) _mm_xor_si128( *((__m128i *)byteMsbMask), (__m128i)x );
	y = (_16i8) _mm_xor_si128( *((__m128i *)byteMsbMask), (__m128i)y );
	return (_16i8) _mm_xor_si128( _mm_min_epu8((__m128i)x , (__m128i)y),(__m128i) *((__m128i *)byteMsbMask) ); 
#endif

}

_8u16	__attribute__((overloadable)) min(_8u16 x, _8u16 y)
{
#ifdef __SSE4_1__
	return (_8u16) _mm_min_epu16((__m128i)x ,(__m128i) y);  
#else
	x = (_8u16) _mm_xor_si128( *((__m128i *)wordMsbMask), (__m128i)x );
	y = (_8u16) _mm_xor_si128( *((__m128i *)wordMsbMask), (__m128i)y );
	return (_8u16) _mm_xor_si128( _mm_min_epi16((__m128i)x , (__m128i)y),(__m128i) *((__m128i *)wordMsbMask) ); 
#endif
}


_8i16	__attribute__((overloadable)) min(_8i16 x, _8i16 y)
{
	return (_8i16) _mm_min_epi16((__m128i)x ,(__m128i) y);  
}


_4u32	__attribute__((overloadable)) min(_4u32 x, _4u32 y)
{
#ifdef __SSE4_1__
	return (_4u32) _mm_min_epu32((__m128i)x ,(__m128i) y);  
#else
	x = (_4u32) _mm_xor_si128( *((__m128i *)dwordMsbMask), (__m128i)x );
	y = (_4u32) _mm_xor_si128( *((__m128i *)dwordMsbMask), (__m128i)y );
	__m128i XgtY = _mm_cmpgt_epi32((__m128i)x,(__m128i) y);
	__m128i res = _mm_and_si128((__m128i)XgtY,(__m128i) y);
	res = _mm_or_si128(_mm_andnot_si128((__m128i)XgtY, (__m128i)x),(__m128i) res);
	return (_4u32) _mm_xor_si128( *((__m128i *)dwordMsbMask), res );
#endif
}
/// !!! This function is copy-pasted to images module.
/// In case of any changes they should also be done in image_callback_functions.cpp
_4i32	__attribute__((overloadable)) min(_4i32 x, _4i32 y)
{
#ifdef __SSE4_1__
	return (_4i32) _mm_min_epi32((__m128i)x ,(__m128i) y);  
#else
 	__m128i mask = _mm_cmpgt_epi32((__m128i)x,(__m128i) y);
	__m128i res = _mm_and_si128((__m128i)mask,(__m128i) y);
	return (_4i32) _mm_or_si128(_mm_andnot_si128((__m128i)mask, (__m128i)x),(__m128i) res);
#endif

}

_2i64	__attribute__((overloadable)) min(_2i64 x, _2i64 y)
{
#ifdef __AVX_1__
	 __m128i tmpX, tmpY, tmp;
	tmp = _mm_cmpgt_epi64(y, x); //tmp = 1 means x > y else tmp = 0
	tmpX = _mm_and_si128(tmp, x); //tmpX = if x>y x else 0
	tmpY = _mm_andnot_si128(tmp, y); //tmpY = if x<y y else 0
	return _mm_add_epi64(tmpX , tmpY);  //return x or y
#else
	ALIGN16 _1i64 tempX[2], tempY[2];

	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX,(__m128i) x);
	_mm_store_si128((__m128i *)tempY,(__m128i) y);
	#pragma ivdep
	for(int i=0; i<2; ++i)
	{
		tempX[i] = __min(tempX[i] , tempY[i]);
	}

	return (_2i64) _mm_load_si128((const __m128i *)tempX);
#endif
}


_2u64	__attribute__((overloadable)) min(_2u64 x, _2u64 y)
{
	ALIGN16 _1u64 tempX[2], tempY[2];

	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX,(__m128i) x);
	_mm_store_si128((__m128i *)tempY,(__m128i) y);
	#pragma ivdep
	for(int i=0; i<2; ++i)
	{
		tempX[i] = __min(tempX[i], tempY[i]);
	}

	return (_2u64) _mm_load_si128((const __m128i *)tempX);

}

_1u64	__attribute__((overloadable)) min(_1u64 x, _1u64 y)
{
	return __min(x,y);
}


_1i64	__attribute__((overloadable)) min(_1i64 x, _1i64 y)
{
	return __min(x,y);
}

//////////////////////////////////////////////////////////////////////////////////////////
//gentype mul_hi (gentype x, gentype y) Computes x * y and returns the high half of the //
//product of x and y.                                                                   //
//////////////////////////////////////////////////////////////////////////////////////////
_1i8	__attribute__((overloadable)) mul_hi(_1i8 x, _1i8 y)
{
	return ((short)x*y) >> 8;
}

_1u8	__attribute__((overloadable)) mul_hi(_1u8 x, _1u8 y)
{
	return ((unsigned short)x*y) >> 8;
}

_1i16   __attribute__((overloadable)) mul_hi(_1i16 x, _1i16 y)
{
	return ((int)x*y) >> 16;
}

_1u16   __attribute__((overloadable)) mul_hi(_1u16 x, _1u16 y)
{
	return ((unsigned int)x*y) >> 16;
}

_1i32   __attribute__((overloadable)) mul_hi(_1i32 x, _1i32 y)
{
	return ((_1i64)x*y) >> 32;
}

_1u32	__attribute__((overloadable)) mul_hi(_1u32 x, _1u32 y)
{
	return ((_1u64)x*y) >> 32;
}

_16i8  __attribute__((overloadable)) mul_hi(_16i8 x, _16i8 y)
{
	__m128i zero = _mm_setzero_si128();

	__m128i tempX = _mm_unpacklo_epi8 ((__m128i)zero,(__m128i) x );
	__m128i tempY = _mm_unpacklo_epi8 ((__m128i)zero,(__m128i) y);
	tempX = _mm_srai_epi16(tempX, 8);
	tempY = _mm_srai_epi16(tempY, 8);
	__m128i temp = _mm_mullo_epi16(tempX, tempY);
	__m128i reslo = _mm_srli_epi16(temp, 8);

	tempX = _mm_unpackhi_epi8 ((__m128i)zero,(__m128i) x );
	tempY = _mm_unpackhi_epi8 ((__m128i)zero,(__m128i) y);
	tempX = _mm_srai_epi16(tempX, 8);
	tempY = _mm_srai_epi16(tempY, 8);
	temp = _mm_mullo_epi16(tempX, tempY);
	__m128i reshi = _mm_srli_epi16(temp, 8);

	__m128i res = _mm_packus_epi16 (reslo, reshi);
	return (_16i8) res;
}

_16u8  __attribute__((overloadable)) mul_hi(_16u8 x, _16u8 y)
{
	__m128i zero = _mm_setzero_si128();

	__m128i tempX = _mm_unpacklo_epi8 ((__m128i)x,(__m128i) zero);
	__m128i tempY = _mm_unpacklo_epi8 ((__m128i)y,(__m128i) zero);
	__m128i temp = _mm_mullo_epi16(tempX, tempY);
	__m128i reslo = _mm_srli_epi16(temp, 8);

	tempX = _mm_unpackhi_epi8 ((__m128i)x,(__m128i) zero);
	tempY = _mm_unpackhi_epi8 ((__m128i)y,(__m128i) zero);
	temp = _mm_mullo_epi16(tempX, tempY);
	__m128i reshi = _mm_srli_epi16(temp, 8);

	__m128i res = _mm_packus_epi16 (reslo, reshi);
	return (_16u8) res;
}

_8i16  __attribute__((overloadable)) mul_hi(_8i16 x, _8i16 y)
{
	return (_8i16) _mm_mulhi_epi16((__m128i)x,(__m128i)y);
}

_8u16  __attribute__((overloadable)) mul_hi(_8u16 x, _8u16 y)
{
	return (_8u16) _mm_mulhi_epu16((__m128i)x,(__m128i)y);
}

_4i32  __attribute__((overloadable)) mul_hi(_4i32 x, _4i32 y)
{
#ifdef __SSE4_1__
	__m128i reslo = _mm_mul_epi32((__m128i)x,(__m128i) y);
	reslo = _mm_srli_epi64(reslo, 32);
	x = (_4i32) _mm_srli_epi64((__m128i)x, 32);
	y = (_4i32) _mm_srli_epi64((__m128i)y, 32);
	__m128i reshi = _mm_mul_epi32((__m128i)x,(__m128i) y);
	__m128i res = _mm_blend_epi16 (reslo, reshi, 0b11001100);
	return (_4i32) res;
#else
	ALIGN16 int tempX[4];
	ALIGN16 int tempY[4];
	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX,(__m128i) x);
	_mm_store_si128((__m128i *)tempY,(__m128i) y);
	#pragma ivdep
	for(int i=0; i<4; ++i)
	{
		 tempX[i] = mul_hi(tempX[i], tempY[i]);
	}

	return (_4i32) _mm_load_si128((const __m128i *)tempX);
#endif
}


_4u32  __attribute__((overloadable)) mul_hi(_4u32 x, _4u32 y)
{
	__m128i reslo = _mm_mul_epu32((__m128i)x,(__m128i) y);
	reslo = _mm_srli_epi64(reslo, 32);
	x = (_4u32) _mm_srli_epi64((__m128i)x, 32);
	y = (_4u32) _mm_srli_epi64((__m128i)y, 32);
	__m128i reshi = _mm_mul_epu32((__m128i)x,(__m128i) y);

#ifdef __SSE4_1__
	__m128i res = _mm_blend_epi16 (reslo, reshi, 0b11001100);
#else
	reshi = _mm_and_si128(*((__m128i *)dwordOddMask), reshi); 
	__m128i res = _mm_or_si128(reslo, reshi);
#endif

	return (_4u32) res;
}

_2i64  __attribute__((overloadable)) mul_hi(_2i64 x, _2i64 y)
{
	ALIGN16 _1i64 tempX[2];
	ALIGN16 _1i64 tempY[2];
	_1u64 destLow;
	_1i64 destHi;

	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX,(__m128i) x);
	_mm_store_si128((__m128i *)tempY,(__m128i) y);
	#pragma ivdep
	for(int i=0; i<2; ++i)
	{		
		multiply_signed_64_by_64( tempX[i], tempY[i], &destLow, &destHi );
		tempX[i] =  destHi;
	}

	return (_2i64) _mm_load_si128((const __m128i *)tempX);
}

_2u64  __attribute__((overloadable)) mul_hi(_2u64 x, _2u64 y)
{
	ALIGN16 _1u64 tempX[2];
	ALIGN16 _1u64 tempY[2];
	_1u64 destLow;
	_1u64 destHi;

	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX,(__m128i) x);
	_mm_store_si128((__m128i *)tempY,(__m128i) y);
	#pragma ivdep
	for(int i=0; i<2; ++i)
	{
		multiply_unsigned_64_by_64( tempX[i], tempY[i], &destLow, &destHi );
		tempX[i] =  destHi;
	}

	return (_2u64) _mm_load_si128((const __m128i *)tempX);
}

_1i64  __attribute__((overloadable)) mul_hi(_1i64 x, _1i64 y)
{
	_1u64 destLow;
	_1i64 destHi;
	
	multiply_signed_64_by_64( x, y, &destLow, &destHi );
	return destHi;	
}


_1u64  __attribute__((overloadable)) mul_hi(_1u64 x, _1u64 y)
{
	_1u64 destLow;
	_1u64 destHi;
	
	multiply_unsigned_64_by_64( x, y, &destLow, &destHi );
	return destHi;	
}
//////////////////////////////////////////////////////////////////////////////////////////////
//gentype rotate (gentype v, gentype i) For each element in v, the bits are shifted left by //
//the number of bits given by the corresponding                                             //
//element in i (subject to usual shift modulo rules                                         //
//described in section 6.3). Bits shifted off the left                                      //
//side of the element are shifted back in from the                                          //
//right.                                                                                    //
//////////////////////////////////////////////////////////////////////////////////////////////
_1i8	__attribute__((overloadable)) rotate(_1i8 x, _1i8 count)
{
	return rotate((uchar)x, (uchar)count) ;
}

_1u8	__attribute__((overloadable)) rotate(_1u8 x, _1u8 count)
{
	count &= 7;
	return ( x << count ) | x >> ( 8 - count ) ;
}


_1i16   __attribute__((overloadable)) rotate(_1i16 x, _1i16 count)
{
	return rotate((ushort)x, (ushort)count);
}

_1u16   __attribute__((overloadable)) rotate(_1u16 x, _1u16 count)
{
	count &= 15;
	return ( x << count ) | x >> ( 16 - count ) ;
}

_1i32   __attribute__((overloadable)) rotate(_1i32 x, _1i32 count)
{
	return rotate((uint)x, (uint)count) ;
}

_1u32	__attribute__((overloadable)) rotate(_1u32 x, _1u32 count)
{
	count &= 31;
	return ( x << count ) | x >> ( 32 - count ) ;
}

_16i8	__attribute__((overloadable)) rotate(_16i8 x, _16i8 count)
{
	ALIGN16 char tempX[16];
	ALIGN16 char tempC[16];
	// Store to temporary buffer
	_mm_store_si128((__m128i*)tempX,(__m128i) x);
	_mm_store_si128((__m128i*)tempC,(__m128i) count);
	
#pragma ivdep	
	for(int i=0; i<16; ++i)
	{
		 tempX[i] = rotate(tempX[i], tempC[i]);
	}
	return (_16i8) _mm_load_si128((const __m128i *)tempX);
}

_16u8	__attribute__((overloadable)) rotate(_16u8 x, _16u8 count)
{
	return (_16u8)rotate((_16i8)x, (_16i8)count);
}

_8i16	__attribute__((overloadable)) rotate(_8i16 x, _8i16 count)
{
	ALIGN16 short tempX[8];
	ALIGN16 short tempC[8];
	unsigned int size = sizeof(tempX);

	// Store to temporary buffer
	_mm_store_si128((__m128i*)tempX,(__m128i) x);
	_mm_store_si128((__m128i*)tempC,(__m128i) count);
	
#pragma ivdep	
	for(int i=0; i<8; ++i)
	{
		tempX[i] = rotate(tempX[i], tempC[i]);
	}
	return (_8i16) _mm_load_si128((const __m128i *)tempX);
}

_8u16	__attribute__((overloadable)) rotate(_8u16 x, _8u16 count)
{
	return (_8u16)rotate((_8i16)x, (_8i16)count);
}

_4i32	__attribute__((overloadable)) rotate(_4i32 x, _4i32 count)
{
	//Couldnt use simply sra and srl because they use only 64 low bits in count
	ALIGN16 int tempX[4];
	ALIGN16 int tempC[4];
	unsigned int size = sizeof(tempX);

	// Store to temporary buffer
	_mm_store_si128((__m128i*)tempX,(__m128i) x);
	_mm_store_si128((__m128i*)tempC,(__m128i) count);
	
#pragma ivdep	
	for(int i=0; i<4; ++i)
	{
		tempX[i] = rotate(tempX[i], tempC[i]);
	}
	return (_4i32) _mm_load_si128((const __m128i *)tempX);
}

_4u32	__attribute__((overloadable)) rotate(_4u32 x, _4u32 count)
{
	return (_4u32)rotate((_4i32)x, (_4i32)count);
}

_2i64	__attribute__((overloadable)) rotate(_2i64 x, _2i64 count)
{
	ALIGN16 _1i64 tempX[2];
	ALIGN16 _1i64 tempC[2];
	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX,(__m128i) x);
	_mm_store_si128((__m128i *)tempC,(__m128i) count);
	for(int i=0; i<2; ++i)
	{
		tempX[i] = rotate(tempX[i], tempC[i]);
	}

	return (_2i64) _mm_load_si128((const __m128i *)tempX);
}

_2u64	__attribute__((overloadable)) rotate(_2u64 x, _2u64 count)
{
	return (_2u64)rotate((_2i64)x, (_2i64)count);
}

_1i64	__attribute__((overloadable)) rotate(_1i64 x, _1i64 count)
{
	return (_1i64)rotate((_1u64)x, (_1u64)count) ;
}

_1u64	__attribute__((overloadable)) rotate(_1u64 x, _1u64 count)
{	
	count &= 63;
	if( count == 0 )
	{
		return x;
	}
	return ( x << count ) | x >> ( 64 - count ) ;
}

///////////////////////////////////////////////////////////////////////////////////
//gentype sub_sat (gentype x, gentype y) Returns x - y and saturates the result. //
///////////////////////////////////////////////////////////////////////////////////
_1i8	__attribute__((overloadable)) sub_sat(_1i8 x, _1i8 y)
{
		int r = (int) x - (int) y;
        r = __max( r, CHAR_MIN );
        r = __min( r, CHAR_MAX );
		return r;
}

_1u8	__attribute__((overloadable)) sub_sat(_1u8 x, _1u8 y)
{
		int r = (int) x - (int) y;
        r = __max( r, UCHAR_MIN );
        r = __min( r, UCHAR_MAX );
		return r;
}

_1i16   __attribute__((overloadable)) sub_sat(_1i16 x, _1i16 y)
{
		int r = (int) x - (int) y;
        r = __max( r, SHRT_MIN );
        r = __min( r, SHRT_MAX );
		return r;

}

_1u16   __attribute__((overloadable)) sub_sat(_1u16 x, _1u16 y)
{
		int r = (int) x - (int) y;
        r = __max( r, USHRT_MIN );
        r = __min( r, USHRT_MAX );
		return r;

}

_1i32   __attribute__((overloadable)) sub_sat(_1i32 x, _1i32 y)
{
		int r = x - y;
        if( y < 0 )
        {
            if( r < x )
                r = INT_MAX;
        }
        else
        {
            if( r > x )
                r = INT_MIN;
        }
		return r;

}

_1u32	__attribute__((overloadable)) sub_sat(_1u32 x, _1u32 y)
{
		unsigned int r = x-y;
        if( x < y)
            r = 0;
		return r;

}

_16i8	__attribute__((overloadable)) sub_sat(_16i8 x, _16i8 y)
{
	return (_16i8) _mm_subs_epi8((__m128i)x,(__m128i)y);	
}

_16u8	__attribute__((overloadable)) sub_sat(_16u8 x, _16u8 y)
{
	return (_16u8) _mm_subs_epu8((__m128i)x,(__m128i)y);	
}

_8i16	__attribute__((overloadable)) sub_sat(_8i16 x, _8i16 y)
{
	return (_8i16) _mm_subs_epi16((__m128i)x,(__m128i)y);	
}

_8u16	__attribute__((overloadable)) sub_sat(_8u16 x, _8u16 y)
{
	return (_8u16) _mm_subs_epu16((__m128i)x,(__m128i)y);	
}

_4i32	__attribute__((overloadable)) sub_sat(_4i32 x, _4i32 y)
{
	__m128i zero = _mm_setzero_si128();

	__m128i res = _mm_sub_epi32((__m128i)x,(__m128i) y);	
	__m128i yLTzero = _mm_cmpgt_epi32((__m128i)zero,(__m128i) y);	
	__m128i resGTx = _mm_cmpgt_epi32((__m128i)res,(__m128i) x);	
	__m128i overflow = _mm_xor_si128(resGTx, yLTzero);	
	res = _mm_andnot_si128(overflow, res);	
	__m128i max = _mm_and_si128(yLTzero, *((__m128i *)intMaxStorage));	
	__m128i min = _mm_andnot_si128(yLTzero, *((__m128i *)intMinStorage));	
	__m128i temp = _mm_or_si128(max, min);	
	overflow = _mm_and_si128(temp, overflow);	
	return (_4i32) _mm_or_si128(res, overflow);
}

_4u32	__attribute__((overloadable)) sub_sat(_4u32 x, _4u32 y)
{
	__m128i zero = _mm_setzero_si128();

	__m128i res = _mm_sub_epi32((__m128i)x,(__m128i) y);
	__m128i res_neg = _mm_xor_si128(res, *((__m128i *)dwordMsbMask));
	x = (_4u32) _mm_xor_si128((__m128i)x,(__m128i)  *((__m128i *)dwordMsbMask));
	__m128i xLTres = _mm_cmpgt_epi32((__m128i)res_neg,(__m128i) x);
	return (_4u32) _mm_andnot_si128(xLTres, res);
}


_2u64	__attribute__((overloadable)) sub_sat(_2u64 x, _2u64 y)
{
		ALIGN16 _1u64 tempX[2], tempY[2];

	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX,(__m128i) x);
	_mm_store_si128((__m128i *)tempY,(__m128i) y);
	#pragma ivdep
	for(int i=0; i<2; ++i)
	{
		tempX[i] = sub_sat(tempX[i], tempY[i]);
	}

	return (_2u64) _mm_load_si128((const __m128i *)tempX);
}

_2i64	__attribute__((overloadable)) sub_sat(_2i64 x, _2i64 y)
{
		ALIGN16 _1i64 tempX[4], tempY[4];

	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX,(__m128i) x);
	_mm_store_si128((__m128i *)tempY,(__m128i) y);
	#pragma ivdep
	for(int i=0; i<2; ++i)
	{
		tempX[i] = sub_sat(tempX[i], tempY[i]);
	}

	return (_2i64) _mm_load_si128((const __m128i *)tempX);
}


_1u64	__attribute__((overloadable)) sub_sat(_1u64 x, _1u64 y)
{
	_1u64 r = x - y;
        if( x < y )
            r = 0;

	return r;
}

_1i64	__attribute__((overloadable)) sub_sat(_1i64 x, _1i64 y)
{
		// Store to temporary buffer
	_1i64 r = x - y;        
	if( y < 0 )
        {
            if( r < x )
                r = LLONG_MAX;
        }
     else
        {
            if( r > x )
                r = LLONG_MIN;
        }
	return r;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//gentype mul24 (gentype x, gentype y) Multiply two 24-bit integer values x and y. x and y //
//are 32-bit integers but only the low 24-bits are used                                    //
//to perform the multiplication                                                            //
//the function can take int,int2, int4, int8, int16, uint, uint2, uint4, uint8 or uint16   //
/////////////////////////////////////////////////////////////////////////////////////////////
_1i32	__attribute__((overloadable)) mul24(_1i32 x, _1i32 y)
{
	x = (x << 8 ) >> 8;
    y = (y << 8 ) >> 8;
       return x*y;	
}

_1u32	__attribute__((overloadable)) mul24(_1u32 x, _1u32 y)
{
	return x * y;
}


_4i32	__attribute__((overloadable)) mul24(_4i32 x, _4i32 y)
{
#ifdef __SSE4_1__
	x = (_4i32) _mm_slli_epi32((__m128i)x, 8);
	y = (_4i32) _mm_slli_epi32((__m128i)y, 8);
	x = (_4i32) _mm_srai_epi32((__m128i)x, 8);
	y = (_4i32) _mm_srai_epi32((__m128i)y, 8);
	__m128i reslo = _mm_mul_epi32((__m128i)x,(__m128i) y);
	x = (_4i32) _mm_srli_epi64((__m128i)x, 32);
	y = (_4i32) _mm_srli_epi64((__m128i)y, 32);
	__m128i reshi = _mm_mul_epi32((__m128i)x,(__m128i) y);
	reshi = _mm_slli_epi64(reshi, 32);
	__m128i res = _mm_blend_epi16 (reslo, reshi, 0b11001100);
	return (_4i32) res;
#else

	ALIGN16 int tempX[4], tempY[4];

	// Store to temporary buffer
	_mm_store_si128((__m128i *)tempX,(__m128i) x);
	_mm_store_si128((__m128i *)tempY,(__m128i) y);
	#pragma ivdep
	for(int i=0; i<4; ++i)
	{
		tempX[i] = mul24(tempX[i], tempY[i]);
	}

	return (_4i32) _mm_load_si128((const __m128i *)tempX);
#endif
}

_4u32	__attribute__((overloadable)) mul24(_4u32 x, _4u32 y)
{
	__m128i reslo = _mm_mul_epu32((__m128i)x,(__m128i) y);
	x = (_4u32) _mm_srli_epi64((__m128i)x, 32);
	y = (_4u32) _mm_srli_epi64((__m128i)y, 32);
	__m128i reshi = _mm_mul_epu32((__m128i)x,(__m128i) y);
	reshi = _mm_slli_epi64(reshi, 32);

#ifdef __SSE4_1__
	__m128i res = _mm_blend_epi16 (reslo, reshi, 0b11001100);
#else
	reslo = _mm_and_si128(*((__m128i *)dwordEvenMask), reslo); 
	__m128i res = _mm_or_si128(reslo, reshi);
#endif

	return (_4u32) res;
}

///////////////////////////////////////////////////////
//gentype mad24 (gentype x,gentype y, gentype z)     //
//Multipy two 24-bit integer values x and y and add  //
//the 32-bit integer result to the 32-bit integer z. //
///////////////////////////////////////////////////////
_1i32	__attribute__((overloadable)) mad24(_1i32 x, _1i32 y, _1i32 z)
{
	x = mul24(x,y);
	return x+z;
}

_1u32	__attribute__((overloadable)) mad24(_1u32 x, _1u32 y, _1u32 z)
{
	x = mul24(x,y);
	return x+z;
}

_4i32	__attribute__((overloadable)) mad24(_4i32 x, _4i32 y, _4i32 z)
{
	x = mul24(x,y);
	return (_4i32) _mm_add_epi32((__m128i)x,(__m128i)z);
}

_4u32	__attribute__((overloadable)) mad24(_4u32 x, _4u32 y, _4u32 z)
{
	x = mul24(x,y);
	return (_4u32) _mm_add_epi32((__m128i)x,(__m128i)z);
}


//////////////////////////////////////////////////////////////////////////////////
//shortn upsample (charn hi, ucharn lo) result[i] = ((short)hi[i] << 8) | lo[i] //
//////////////////////////////////////////////////////////////////////////////////
_1i16	__attribute__((overloadable)) upsample(_1i8 x, _1u8 y)
{
	return ((short)x << 8) | y;	
}
_1u16	__attribute__((overloadable)) upsample(_1u8 x, _1u8 y)
{
	return ((unsigned short)x << 8) | y;	
}
_16i16  __attribute__((overloadable)) upsample(_16i8 x, _16u8 y)
{
	_16i16 res;
	__m128i zero = _mm_setzero_si128() ;
	// Upscale to short
	__m128i X = _mm_unpacklo_epi8((__m128i)zero,(__m128i) x);
	__m128i Y = _mm_unpacklo_epi8((__m128i)y,(__m128i) zero);
	res.lo = (_8i16) _mm_or_si128(X, Y);
	X = _mm_unpackhi_epi8((__m128i)zero,(__m128i) x);
	Y = _mm_unpackhi_epi8((__m128i)y,(__m128i) zero);
	res.hi = (_8i16) _mm_or_si128(X, Y);
	return res;
}

_16u16  __attribute__((overloadable)) upsample(_16u8 x, _16u8 y)
{
	_16u16 res;
	__m128i zero = _mm_setzero_si128() ;
	// Upscale to short
	__m128i X = _mm_unpacklo_epi8((__m128i)zero,(__m128i) x);
	__m128i Y = _mm_unpacklo_epi8((__m128i)y,(__m128i) zero);
	res.lo = (_8u16) _mm_or_si128(X, Y);
	X = _mm_unpackhi_epi8((__m128i)zero,(__m128i) x);
	Y = _mm_unpackhi_epi8((__m128i)y,(__m128i) zero);
	res.hi = (_8u16) _mm_or_si128(X, Y);
	return res;
}

////////////////////////////////////////////////////////////////////////////////
//intn upsample (shortn hi, ushortn lo)result[i] = ((int)hi[i] << 16) | lo[i] //
 ///////////////////////////////////////////////////////////////////////////////
_1i32	__attribute__((overloadable)) upsample(_1i16 x, _1u16 y)
{
	return ((int)x << 16) | y;
}

_1u32	__attribute__((overloadable)) upsample(_1u16 x, _1u16 y)
{
	return ((unsigned int)x << 16) | y;
}


_8i32	__attribute__((overloadable)) upsample(_8i16 x, _8u16 y)
{
	_8i32 res;
	__m128i zero = _mm_setzero_si128() ;
	// Upscale to int
	__m128i X = _mm_unpacklo_epi16((__m128i)zero,(__m128i) x);
	__m128i Y = _mm_unpacklo_epi16((__m128i)y,(__m128i) zero);
	res.lo = (_4i32) _mm_or_si128(X, Y);
	X = _mm_unpackhi_epi16((__m128i)zero,(__m128i) x);
	Y = _mm_unpackhi_epi16((__m128i)y,(__m128i) zero);
	res.hi = (_4i32) _mm_or_si128(X, Y);
	return res;
}


_8u32	__attribute__((overloadable)) upsample(_8u16 x, _8u16 y)
{
	_8u32 res;
	__m128i zero = _mm_setzero_si128() ;
	// Upscale to int
	__m128i X = _mm_unpacklo_epi16((__m128i)zero,(__m128i) x);
	__m128i Y = _mm_unpacklo_epi16((__m128i)y,(__m128i) zero);
	res.lo = (_4u32) _mm_or_si128(X, Y);
	X = _mm_unpackhi_epi16((__m128i)zero,(__m128i) x);
	Y = _mm_unpackhi_epi16((__m128i)y,(__m128i) zero);
	res.hi = (_4u32) _mm_or_si128(X, Y);
	return res;
}

///////////////////////////////////////////////////////////////////////////////
//longn upsample (intn hi, uintn lo) result[i] = ((long)hi[i] << 32) | lo[i] //
///////////////////////////////////////////////////////////////////////////////
_1i64  __attribute__((overloadable)) upsample(_1i32 x, _1u32 y)
{
	return ((_1i64)x << 32) | y;
}

_1u64  __attribute__((overloadable)) upsample(_1u32 x, _1u32 y)
{
	return ((_1u64)x << 32) | y;
}


_4i64  __attribute__((overloadable)) upsample(_4i32 x, _4u32 y)
{
	_4i64 res;
	__m128i zero = _mm_setzero_si128() ;
	// Upscale to int
	__m128i X = _mm_unpacklo_epi32((__m128i)zero,(__m128i) x);
	__m128i Y = _mm_unpacklo_epi32((__m128i)y,(__m128i) zero);
	res.lo = (_2i64) _mm_or_si128(X, Y);
	X = _mm_unpackhi_epi32((__m128i)zero,(__m128i) x);
	Y = _mm_unpackhi_epi32((__m128i)y,(__m128i) zero);
	res.hi = (_2i64) _mm_or_si128(X, Y);
	return res;
}


_4u64  __attribute__((overloadable)) upsample(_4u32 x, _4u32 y)
{
	_4u64 res;
	__m128i zero = _mm_setzero_si128() ;
	// Upscale to int
	__m128i X = _mm_unpacklo_epi32((__m128i)zero,(__m128i) x);
	__m128i Y = _mm_unpacklo_epi32((__m128i)y,(__m128i) zero);
	res.lo = (_2u64) _mm_or_si128(X, Y);
	X = _mm_unpackhi_epi32((__m128i)zero,(__m128i) x);
	Y = _mm_unpackhi_epi32((__m128i)y,(__m128i) zero);
	res.hi = (_2u64) _mm_or_si128(X, Y);
	return res;
}

#ifdef __cplusplus
}
#endif