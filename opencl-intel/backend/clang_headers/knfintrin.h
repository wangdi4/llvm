/*===---- knfintrin.h - KNF intrinsics ------------------------------------===
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *===-----------------------------------------------------------------------===
 */
#ifndef __KNF_INTRIN__
#define __KNF_INTRIN__

#ifndef __KNF__
#error "KNF istruction set is disabled"
#endif

#define OUT

#define CONST_VECTOR16(TYPE, VAL)\
	(_16##TYPE##32)(VAL, VAL, VAL, VAL,\
						VAL, VAL, VAL, VAL,\
						VAL, VAL, VAL, VAL,\
						VAL, VAL, VAL, VAL)

#define CONST_VECTOR8(TYPE, VAL)\
	(_8##TYPE##64)(VAL, VAL, VAL, VAL,\
				   VAL, VAL, VAL, VAL)

typedef enum __mm_swizzle {
	__mm_swizzleDCBA = 0,
	__mm_swizzleCDAB = 1,
	__mm_swizzleBADC = 2,
	__mm_swizzleAAAA = 3,
	__mm_swizzleBBBB = 4,
	__mm_swizzleCCCC = 5,
	__mm_swizzleDDDD = 6,
	__mm_swizzleDACB = 7
} swizzle;

typedef enum __mm_swizzle_up_convert16 {
	__mm_16to16 = 0,	//f(x) = x
	__mm_1to16  = 1,	//f(x1) = 16 X x1
	__mm_4to16  = 2,	//f(x1,...,x4) = 4 X x1,...,x4
	__mm_16u8to116u32 = 4,
	__mm_16i8to116i32 = 5,
	__mm_16u16to16u32 = 6,
	__mm_16i16to16i32 = 7,
} swizle_up_convert16;

typedef enum __mm_swizzle_up_convert8 {
	__mm_8to8 = 0,	//f(x1,...,x8) = x1,...,x8
	__mm_1to8  = 1,	//f(x1) = 8 X x1
	__mm_4to8 = 2,	//f(x1,...,x4) = 2 X x1,...,x4
} swizle_up_convert8;

//const char  MAX_INT8  = (char)0x7f;
//const short MAX_INT16 = (short)0x7fff;
//const int   MAX_INT32 = 0x7fffffff;
//const long  MAX_INT64 = 0x7fffffffffffffffL;

typedef double __m512pd  __attribute__ ((__vector_size__(64)));
typedef float  __m512ps  __attribute__ ((__vector_size__(64)));
typedef int    __m512i   __attribute__ ((__vector_size__(64)));
typedef long   __m512l   __attribute__ ((__vector_size__(64)));
typedef unsigned long __m512ul __attribute__ ((__vector_size__(64)));
typedef unsigned int  __m512ui __attribute__ ((__vector_size__(64)));
typedef unsigned short   __mmask; 
typedef unsigned char    __cmask;

__inline__ __m512ps __attribute__((__always_inline__, __nodebug__))
_float4_to_m512( float4* f ) {
	return *(__m512ps*)f;	
}

__inline__ __m512pd __attribute__((__always_inline__, __nodebug__))
_double2_to_m512d( double2* d ) {
	return *(__m512pd*)d;
}

__inline__ long2 __attribute__((__always_inline__, __nodebug__))
_mmask_to_long2( __mmask m ) {
	return (long2)m; //TODO: validate that... bitcast won't do...
}

//
// comparision methods
//
__inline__ __mmask __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cmplt_ps( __mmask k1, __m512ps v1, __m512ps v2 ) {
	return __builtin_ia32_mm512_mask_cmplts_ps(k1, v1, v2);
}

__inline__ __mmask __attribute__((__always_inline__, __nodebug__))
_mm512_cmplts_ps(__m512ps x, __m512ps y) {
	return __builtin_ia32_mm512_cmplts_ps(x, y);
}

__inline__ __mmask __attribute__((__always_inline__, __nodebug__))
_mm512_cmplt_ps(__m512ps x, __m512ps y) {
	return __builtin_ia32_mm512_cmplts_ps(x, y);
}

__inline__ __mmask __attribute__((__always_inline__, __nodebug__))
_mm512_cmpgt_ps(__m512ps x, __m512ps y) {
	return _mm512_cmplt_ps(y, x);
}

__inline__ __mmask __attribute__((__always_inline__, __nodebug__))
_mm512_cmplt_pi(__m512i x, __m512i y) {
	return __builtin_ia32_mm512_cmplts_pi(x, y);
}

__inline__ __cmask __attribute__((__always_inline__, __nodebug__))
_mm512_cmplt_pl(__m512l x, __m512l y) {
	return __builtin_ia32_mm512_cmplts_pl(x, y);
}

__inline__ __cmask __attribute__((__always_inline__, __nodebug__))
_mm512_cmplt_pd ( __m512pd v1, __m512pd v2 ) {
//	return __builtin_ia32_mm512_cmplts_pd(v1, v2);
	return (__cmask)0;
}

//
//memory operations
//

//stores v into buffer under write mask
__inline__ void __attribute__((__always_inline__, __nodebug__))
_mm512_masked_stored ( OUT void* buffer, __mmask m, __m512ps v ) {
	__builtin_ia32_mm512_masked_stored( buffer, m, v );
}

__inline__ __m512ps __attribute__((__always_inline__, __nodebug__))
 _mm512_loadd(const void *mt, swizzle conversion ) {
	return __builtin_ia32_mm512_loadd( mt, conversion );
}

__inline__ __m512ps __attribute__((__always_inline__, __nodebug__))
_mm512_broadcast16(const void *mt, swizle_up_convert16 bcast ) {
	return __builtin_ia32_mm512_broadcast_dd( mt, bcast );
}

__inline__ __m512pd __attribute__((__always_inline__, __nodebug__))
_mm512_broadcast8(const void *mt, swizle_up_convert8 bcast ) {
	return __builtin_ia32_mm512_broadcast_dq( mt, bcast );
}
//
//mask operations
//

// bitwise not on the given mask
__inline__ __mmask __attribute__((__always_inline__, __nodebug__))
_mm512_knot( __mmask m ) {
	return __builtin_ia32_mm512_knot(m);
}

//
// integer function
//
__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_min_pi ( __m512i v1, __m512i v2 ) {
	return __builtin_ia32_mm512_min_pi(v1, v2);
}

__inline__ __m512l __attribute__((__always_inline__, __nodebug__))
_mm512_min_pl ( __m512l v1, __m512l v2 ) {
	return __builtin_ia32_mm512_min_pl(v1, v2);
}

__inline__ __m512ui __attribute__((__always_inline__, __nodebug__))
_mm512_min_pu ( __m512ui v1, __m512ui v2 ) {
	return __builtin_ia32_mm512_min_pu(v1, v2);
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_max_pi ( __m512i v1, __m512i v2 ) {
	return __builtin_ia32_mm512_max_pi(v1, v2);
}

__inline__ __m512l __attribute__((__always_inline__, __nodebug__))
_mm512_max_pl ( __m512l v1, __m512l v2 ) {
	return __builtin_ia32_mm512_max_pl(v1, v2);
}

__inline__ __m512ui __attribute__((__always_inline__, __nodebug__))
_mm512_max_pu ( __m512ui v1, __m512ui v2 ) {
	return __builtin_ia32_mm512_max_pu(v1, v2);
}

//
//bitwise operations
//
__inline__ __mmask __attribute__((__always_inline__, __nodebug__))
_mm512_kor ( __mmask k1, __mmask k2 ){
	return __builtin_ia32_mm512_kor( k1, k2 );	
}

__inline__ __mmask __attribute__((__always_inline__, __nodebug__))
_mm512_ckor ( __cmask k1, __cmask k2 ){
	return __builtin_ia32_mm512_ckor( k1, k2 );	
}

__inline__ __m512i  __attribute__((__always_inline__, __nodebug__))
_mm512_or_pi ( __m512i  v1, __m512i v2 ){
	return __builtin_ia32_mm512_or_pi( v1, v2 );	
}

//bitwise or
__inline__ __m512l  __attribute__((__always_inline__, __nodebug__))
_mm512_or_pl ( __m512l  v1, __m512l v2 ){
	return __builtin_ia32_mm512_or_pl( v1, v2 );	
}

//bitwise and
__inline__ __m512i  __attribute__((__always_inline__, __nodebug__))
_mm512_and_pi(__m512i v1, __m512i v2 ){
	return __builtin_ia32_mm512_and_pi( v1, v2 );
}

//shift right arithmetic 32
__inline__ __m512i  __attribute__((__always_inline__, __nodebug__))
_mm512_sra_pi(__m512i v1, __m512i v2 ){
	return __builtin_ia32_mm512_sra_pi(v1, v2);
}

//shift right arithmetic 64 
__inline__ __m512l  __attribute__((__always_inline__, __nodebug__))
_mm512_sra_pl(__m512l v1, __m512l v2 ){
	return __builtin_ia32_mm512_sra_pl(v1, v2);
}

//bitwise xor 32
__inline__ __m512i  __attribute__((__always_inline__, __nodebug__))
_mm512_xor_pi(__m512i v1, __m512i v2 ){
	return __builtin_ia32_mm512_xor_pi(v1, v2);
}

//bitwise xor 64
__inline__ __m512l  __attribute__((__always_inline__, __nodebug__))
_mm512_xor_pl(__m512l x, __m512l y ){
	return __builtin_ia32_mm512_xor_pl(x, y);
}

//shift right logical 32
__inline__ __m512i  __attribute__((__always_inline__, __nodebug__))
_mm512_srl_pi(__m512i v1, __m512i v2 ){
	return __builtin_ia32_mm512_srl_pi(v1, v2);
}

//shift right logical 64
__inline__ __m512l  __attribute__((__always_inline__, __nodebug__))
_mm512_srl_pl(__m512l v1, __m512l v2 ){
	return __builtin_ia32_mm512_srl_pl(v1, v2);
}

//shift left logical 32
__inline__ __m512i  __attribute__((__always_inline__, __nodebug__))
_mm512_sll_pi (__m512i v1, __m512i v2 ) {
	return __builtin_ia32_mm512_sll_pi(v1, v2);
}

//
// Vector conversions
//

// converts an 8 vector of float-64 into the lower part of an 16 vector of
//  float 32.
__inline__ __m512ps  __attribute__((__always_inline__, __nodebug__))
_mm512_cvtl_pd2ps( __m512pd v ) {
	return __builtin_ia32_mm512_cvtls_pd2ps( v );
}

__inline__ __m512ps  __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvtl_pd2ps( __m512pd v, __mmask m ) {
	return __builtin_ia32_mm512_mask_cvtls_pd2ps( v, m );
}

__inline__ __m512ps  __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvth_pd2ps( __m512pd v, __mmask m ) {
	return __builtin_ia32_mm512_mask_cvths_pd2ps( v, m );
}

__inline__ __m512i __attribute__((__always_inline__, __nodebug__))
_mm512_cvt_k2pi( __mmask m ) {
	return __builtin_ia32_mm512_cvt_k2pi( m );
}

__inline__ __m512l __attribute__((__always_inline__, __nodebug__))
_mm512_cvt_c2pl( __cmask m ) {
	return __builtin_ia32_mm512_cvt_c2pl( m );
}

// converts an 8 vector of float-64 into the higher part of an 16 vector of
//  float 32.
__inline__ __m512ps  __attribute__((__always_inline__, __nodebug__))
_mm512_cvth_ps2pd( __m512pd v ) {
	return __builtin_ia32_mm512_cvths_ps2pd( v );
}

__inline__ __m512pd  __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvth_ps2pd( __m512ps v, __mmask m ) {
	return __builtin_ia32_mm512_mask_cvths_ps2pd( v, m );
}

// converts the lower part of a 16 vector of floats-32 into an 8 vector of 
// float 64.
__inline__ __m512pd  __attribute__((__always_inline__, __nodebug__))
_mm512_cvtl_ps2pd( __m512ps v ) {
	return __builtin_ia32_mm512_cvtls_ps2pd( v );
}

__inline__ __m512pd  __attribute__((__always_inline__, __nodebug__))
_mm512_mask_cvtl_ps2pd( __m512ps v, __mmask m ) {
	return __builtin_ia32_mm512_mask_cvtls_ps2pd( v, m );
}

//
// Arithmetic
//
__inline__ __m512i  __attribute__((__always_inline__, __nodebug__))
_mm512_mull_pi ( __m512i v1, __m512i v2 ) {
	return __builtin_ia32_mm512_mull_pi(v1, v2);
}

__inline__ __m512i  __attribute__((__always_inline__, __nodebug__))
_mm512_mull_pu ( __m512i v1, __m512i v2 ) {
	return __builtin_ia32_mm512_mull_pu(v1, v2);
}

//multiplies two 64 bit integers, returning the hi portion of the result
__inline__ long  __attribute__((__always_inline__, __nodebug__))
_mm512_mulh_sl ( long v1, long v2 ) {
	return __builtin_ia32_mm512_mulh_sl( v1, v2 );
}

//multiplies two 64 unsigned bit integers, returning the hi portion of the result
__inline__ unsigned long  __attribute__((__always_inline__, __nodebug__))
_mm512_mulh_slu ( unsigned long v1, unsigned long v2 ) {
	return __builtin_ia32_mm512_mulh_slu( v1, v2 );
}

//multiplies two 16X32 bit integer vectors, returning the hi portion of the result
__inline__ __m512i  __attribute__((__always_inline__, __nodebug__))
_mm512_mulh_pi ( __m512i v1, __m512i v2 ) {
	return __builtin_ia32_mm512_mulh_pi( v1, v2 );
}

//multiplies two 8X64 bit integer vectors, returning the hi portion of the result
__inline__ __m512l  __attribute__((__always_inline__, __nodebug__))
_mm512_mulh_pl( __m512l v1, __m512l v2 ) {
	return __builtin_ia32_mm512_mulh_pl(v1, v2);
}

//multiplies two 16X32 bit unsigned integer vectors, returning the hi portion of the result
__inline__ __m512i  __attribute__((__always_inline__, __nodebug__))
_mm512_mulh_piu ( __m512i v1, __m512i v2 ) {
	return __builtin_ia32_mm512_mulh_piu(v1, v2);
}

//multiplies two 8X64 bit unsigned integer vectors, returning the hi portion of the result
__inline__ __m512l  __attribute__((__always_inline__, __nodebug__))
_mm512_mulh_plu ( __m512l v1, __m512l v2 ) {
	return __builtin_ia32_mm512_mulh_plu(v1, v2);
}

//
//sub
//
__inline__ __m512i  __attribute__((__always_inline__, __nodebug__))
_mm512_sub_pi ( __m512i x, __m512i y) {
	return __builtin_ia32_mm512_sub_pi( x, y);
}

__inline__ __m512i  __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sub_pi ( __m512i x, __m512i y, __mmask m ) {
	return __builtin_ia32_mm512_mask_sub_pi( x, y, m );
}

__inline__ __m512l  __attribute__((__always_inline__, __nodebug__))
_mm512_sub_pl ( __m512l x, __m512l y) {
	return __builtin_ia32_mm512_sub_pl( x, y);
}

__inline__ __m512l  __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sub_pl ( __m512l x, __m512l y, __mmask m ) {
	return __builtin_ia32_mm512_mask_sub_pl( x, y, m );
}

__inline__ __m512ps  __attribute__((__always_inline__, __nodebug__))
_mm512_sub_ps ( __m512ps x, __m512ps y ) {
	return __builtin_ia32_mm512_sub_ps( x, y );
}

__inline__ __m512ps __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sub_ps ( __m512ps x, __m512ps y, __mmask m ) {
	return __builtin_ia32_mm512_mask_sub_ps( x, y, m );
}

__inline__ __m512pd  __attribute__((__always_inline__, __nodebug__))
_mm512_sub_pd ( __m512pd x, __m512pd y ) {
	return __builtin_ia32_mm512_sub_pd( x, y );
}

__inline__ __m512pd  __attribute__((__always_inline__, __nodebug__))
_mm512_mask_sub_pd ( __m512pd x, __m512pd y, __cmask m ) {
	return __builtin_ia32_mm512_mask_sub_pd( x, y, m );
}

//
//add
//
__inline__ __m512i  __attribute__((__always_inline__, __nodebug__))
_mm512_add_pi ( __m512i v1, __m512i v2 ) {
	return __builtin_ia32_mm512_add_pi(v1, v2);
}

//
// adds element by element of v1 to v2 under writeMask.
// The carry of each element is written bask to carryMask.
//
__inline__ __m512i  __attribute__((__always_inline__, __nodebug__))
_mm512_mask_addsetc_pi( __m512i v1, __mmask writeMask, __m512i v2
	, OUT __mmask* carryRes ) {
	return __builtin_ia32_mm512_mask_addsetc_pi( v1, writeMask, v2,	carryRes );
}

__inline__ __m512i  __attribute__((__always_inline__, __nodebug__))
_mm512_mask_adc_pi( __m512i v1, __mmask writeMask, __mmask carry, __m512i v2
	, OUT __mmask* carryRes ) {
	return __builtin_ia32_mm512_mask_adc_pi( v1, writeMask, carry, v2, carryRes);
}

__inline__ __m512l  __attribute__((__always_inline__, __nodebug__))
_mm512_add_pl ( __m512l v1, __m512l v2 ) {
	__mmask m = 0x5555, carry;
	v1 = _mm512_mask_addsetc_pi( v1, m, v2, &carry);
	m <<= 1;
	carry <<= 1;
	v1 = _mm512_mask_adc_pi( v1, m, carry, v2, &m);
	return v1;
}

// 1/x
__inline__ __m512ps  __attribute__((__always_inline__, __nodebug__))
_mm512_rcprefine_ps( __m512ps v ) {
	return __builtin_ia32_mm512_rcprefines_ps( v );
}

__inline__ __m512ps  __attribute__((__always_inline__, __nodebug__))
_mm512_mask_rcprefine_ps( __m512ps v, __mmask m ) {
	return __builtin_ia32_mm512_mask_rcprefines_ps( v, m );
}

// 'approximated'  1 / sqrt
__inline__ __m512ps  __attribute__((__always_inline__, __nodebug__))
_mm512_rsqrtlut_ps( __m512ps v ) {
	return __builtin_ia32_mm512_rsqrtlut_ps( v );
}

__inline__ __m512ps  __attribute__((__always_inline__, __nodebug__))
_mm512_mask_rsqrtlut_ps( __m512ps v , __mmask m ) {
	return __builtin_ia32_mm512_mask_rsqrtlut_ps( v, m );
}

__inline__ float16   __attribute__((__always_inline__, __nodebug__))
__attribute__((overloadable)) masked_sqrt( float16 v, __mmask m ) {
	v = (float16)_mm512_mask_rsqrtlut_ps( (__m512ps)v, m );
	return (float16)_mm512_mask_rcprefine_ps(v, m );
}

__inline__ double8 __attribute__((__always_inline__, __nodebug__))
__attribute__((overloadable)) masked_sqrt( double8 v, __mmask m ) {
	float16 floats = _mm512_mask_cvtl_pd2ps( (__m512pd)v, m );
	floats = masked_sqrt ( floats, m );
	return (double8)_mm512_mask_cvtl_ps2pd( (__m512ps)floats, m );
}

#endif //__KNF_INTRIN__
