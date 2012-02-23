// Copyright (c) 2006-2011 Intel Corporation
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
//  mic_relational_functions.cpp
///////////////////////////////////////////////////////////
#if defined (__MIC__) || defined(__MIC2__)

#ifdef __cplusplus
extern "C" {
#endif

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#include <intrin.h>
#include "mic_cl_relational_declaration.h"

// Generating scalar/vector functions for generic comparision operators
gen_isequal
gen_isnotequal
gen_isgreater
gen_isgreaterequal
gen_isless
gen_islessequal

// islessgreater
int __attribute__((overloadable)) islessgreater (float x, float y)
{
  return ((x < y) || (x > y))?1:0;
}

int16 __attribute__((overloadable)) islessgreater (float16 x, float16 y)
{
  __m512i zeros = (__m512i)_mm512_setzero();
  __m512i ones = cast_ireg((int16)(0xFFFFFFFF));

  __mmask16 isless = _mm512_cmplt_ps(x, y);
  __mmask16 isgreater = _mm512_cmpnle_ps(x, y);
  isless = _mm512_kor(isless, isgreater);

  zeros = _mm512_mask_or_pi(zeros, isless, ones, ones);

  return (int16)zeros;
}

// Functions for special FP number detection
int __attribute__((overloadable)) __attribute__((overloadable)) isfinite (float x)
{
  return ((fp2int(x) & const_fp_exp) < const_fp_exp)?1:0;
}

int __attribute__((overloadable)) __attribute__((overloadable)) isinf (float x)
{
  return ((fp2int(x) & const_no_sign) == const_fp_exp)?1:0;
}

int __attribute__((overloadable)) __attribute__((overloadable)) isnan (float x)
{
  return ((fp2int(x) & const_no_sign) > const_fp_exp)?1:0;
}

int __attribute__((overloadable)) isnormal (float x)
{
  return ( ((fp2int(x) & const_fp_exp)!=0) && ( (fp2int(x) & const_fp_exp) != const_fp_exp) )?1:0;
}

int  __attribute__((overloadable)) isordered (float x, float y)
{
  return ((x==x) && (y==y))?1:0;
}

int __attribute__((overloadable)) isunordered (float x, float y)
{
  return isnan (x) | isnan (y);
}

int __attribute__((overloadable)) signbit (float x)
{
  return (fp2int(x) & const_msb)?1:0;
}

// isfinite
int16 __attribute__((overloadable)) isfinite (float16 x)
{
  __m512i zeros = (__m512i)_mm512_setzero();
  __m512i ones = cast_ireg((int16)(0xFFFFFFFF));

  __m512i no_sign = _mm512_and_pi(cast_ireg(x), pcast_ireg(const_vector_exp));
  __mmask16 equal = _mm512_cmplt_pi(no_sign, pcast_ireg(const_vector_exp));

  zeros = _mm512_mask_or_pi(zeros, equal, ones, ones);

  return (int16)zeros;
}

// isinf
int16 __attribute__((overloadable)) isinf (float16 x)
{
  __m512i zeros = (__m512i)_mm512_setzero();
  __m512i ones = cast_ireg((int16)(0xFFFFFFFF));

  __m512i no_sign = _mm512_and_pi(cast_ireg(x), pcast_ireg(const_vector_nosign));
  __mmask16 equal = _mm512_cmpeq_pi(no_sign, pcast_ireg(const_vector_exp));

  zeros = _mm512_mask_or_pi(zeros, equal, ones, ones);
  
  return (int16)zeros;
}

// isnan
int16 __attribute__((overloadable)) isnan (float16 x)
{
  __m512i zeros = (__m512i)_mm512_setzero();
  __m512i ones = cast_ireg((int16)(0xFFFFFFFF));

  __mmask16 equal = _mm512_cmpunord_ps(cast_reg(x), cast_reg(x));

  zeros = _mm512_mask_or_pi(zeros, equal, ones, ones);

  return (int16)zeros;
}

// isnormal
int16  __attribute__((overloadable)) isnormal (float16 x)
{
  __m512i zeros = (__m512i)_mm512_setzero();
  __m512i ones = cast_ireg((int16)(0xFFFFFFFF));

  __m512i no_sign = _mm512_and_pi(cast_ireg(x), pcast_ireg(const_vector_exp));
  __mmask16 equal = _mm512_cmplt_pi(no_sign, pcast_ireg(const_vector_exp));
  equal = _mm512_mask_cmpnle_pi(equal, no_sign, cast_ireg(zeros));

  zeros = _mm512_mask_or_pi(zeros, equal, ones, ones);

  return (int16)zeros;
}

// isordered
int16  __attribute__((overloadable)) isordered (float16 x, float16 y)
{
  __m512i zeros = (__m512i)_mm512_setzero();
  __m512i ones = cast_ireg((int16)(0xFFFFFFFF));

  __mmask16 equal = _mm512_cmpord_ps(cast_reg(x), cast_reg(x));

  zeros = _mm512_mask_or_pi(zeros, equal, ones, ones);
  return (int16)zeros;
}

// isunordered
int16 __attribute__((overloadable)) isunordered (float16 x, float16 y)
{
  __m512i zeros = (__m512i)_mm512_setzero();
  __m512i ones = cast_ireg((int16)(0xFFFFFFFF));

  __mmask16 equal = _mm512_cmpunord_ps(cast_reg(x), cast_reg(y));

  zeros = _mm512_mask_or_pi(zeros, equal, ones, ones);
  return (int16)zeros;
}

// signbit
int16 __attribute__((overloadable)) signbit (float16 x)
{
  __m512i zeros = (__m512i)_mm512_setzero();
  __m512i ones = cast_ireg((int16)(0xFFFFFFFF));

  __m512i no_sign = _mm512_and_pi(cast_ireg(x), pcast_ireg(const_vector_msb));
  __mmask16 equal = _mm512_cmpneq_pi(no_sign, zeros);

  zeros = _mm512_mask_or_pi(zeros, equal, ones, ones);
  return (int16)zeros;
}

DEF_FLOAT_RELATIONAL_vx_vx( islessgreater)
DEF_FLOAT_RELATIONAL_vx( isfinite )
DEF_FLOAT_RELATIONAL_vx( isinf )
DEF_FLOAT_RELATIONAL_vx( isnan )
DEF_FLOAT_RELATIONAL_vx( isnormal )
DEF_FLOAT_RELATIONAL_vx_vx( isordered )
DEF_FLOAT_RELATIONAL_vx_vx( isunordered )
DEF_FLOAT_RELATIONAL_vx( signbit )

// Generating scalar functions for any/all operators
gen_any_all_scalar
DEF_FUNCTION_TYPE_vx_vx_x_ALL( select, char, char)
DEF_FUNCTION_TYPE_vx_vx_x_ALL( bitselect, char, char)
DEF_FUNCTION_TYPE_vx_vx_x_ALL( select, char, uchar)

DEF_FUNCTION_TYPE_vx_vx_x_ALL( select, short, short)
DEF_FUNCTION_TYPE_vx_vx_x_ALL( bitselect, short, short)
DEF_FUNCTION_TYPE_vx_vx_x_ALL( select, short, ushort)

DEF_FUNCTION_TYPE_vx_vx_x_ALL( select, int, int)
DEF_FUNCTION_TYPE_vx_vx_x_ALL( bitselect, int, int)
DEF_FUNCTION_TYPE_vx_vx_x_ALL( select, int, uint)

DEF_FUNCTION_TYPE_vx_vx_x_ALL( select, long, long)
DEF_FUNCTION_TYPE_vx_vx_x_ALL( bitselect, long, long)
DEF_FUNCTION_TYPE_vx_vx_x_ALL( select, long, ulong)


// any/all for char
int any_char(char16 vector_reg, __mmask16 k1)
{
  __m512i zeros = (__m512i)_mm512_setzero();
  __m512 x_reg = ext_from_s8(vector_reg);

  x_reg = cast_reg(_mm512_mask_and_pi(cast_ireg(x_reg), k1, zeros, zeros));
  __m512i no_sign = _mm512_and_pi(cast_ireg(x_reg), pcast_ireg(const_vector_msb));
  __mmask16 equal = _mm512_cmpeq_pi(no_sign, zeros);
  int all_zero = _mm512_kortestz(equal, equal);

  return all_zero?0:1;
}

int all_char(char16 vector_reg, __mmask16 k1)
{
  __m512 x_reg = ext_from_s8(vector_reg);
  x_reg = _mm512_mask_loadd(x_reg, k1, (void const *)const_vector_msb, _MM_FULLUPC_NONE, _MM_BROADCAST32_NONE, _MM_HINT_NONE);

  __m512i no_sign = _mm512_and_pi(cast_ireg(x_reg), pcast_ireg(const_vector_msb));
  __mmask16 equal = _mm512_cmpeq_pi(no_sign, pcast_ireg(const_vector_msb));
  int all_one = _mm512_kortestc(equal, equal);

  return all_one?1:0;
}

int __attribute__((overloadable)) any(char16 x)
{
  __m512i zeros = (__m512i)_mm512_setzero();
  __m512 x_reg = ext_from_s8(x);

  __m512i no_sign = _mm512_and_pi(cast_ireg(x_reg), pcast_ireg(const_vector_msb));
  __mmask16 equal = _mm512_cmpeq_pi(no_sign, zeros);
  int all_zero = _mm512_kortestz(equal, equal);

  return all_zero?0:1;
}

int __attribute__((overloadable)) all(char16 x)
{
  __m512 x_reg = ext_from_s8(x);

  __m512i no_sign = _mm512_and_pi(cast_ireg(x_reg), pcast_ireg(const_vector_msb));
  __mmask16 equal = _mm512_cmpeq_pi(no_sign, pcast_ireg(const_vector_msb));
  int all_one = _mm512_kortestc(equal, equal);

  return all_one?1:0;
}
DEF_FUNCION_ANY_ALL(char, any_char, all_char)

// any/all for uchar
DEF_ANY_FUNCTION_OVERLOAD(char, uchar)
DEF_ALL_FUNCTION_OVERLOAD(char, uchar)

int any_short(short16 vector_reg, __mmask16 k1)
{
  __m512i zeros = (__m512i)_mm512_setzero();
  __m512 x_reg = ext_from_s16(vector_reg);

  x_reg = cast_reg(_mm512_mask_and_pi(cast_ireg(x_reg), k1, zeros, zeros));
  __m512i no_sign = _mm512_and_pi(cast_ireg(x_reg), pcast_ireg(const_vector_msb));
  __mmask16 equal = _mm512_cmpeq_pi(no_sign, zeros);
  int all_zero = _mm512_kortestz(equal, equal);

  return all_zero?0:1;
}

int all_short(short16 vector_reg, __mmask16 k1)
{
  __m512 x_reg = ext_from_s16(vector_reg);
  x_reg = _mm512_mask_loadd(x_reg, k1, (void const *)const_vector_msb, _MM_FULLUPC_NONE, _MM_BROADCAST32_NONE, _MM_HINT_NONE);

  __m512i no_sign = _mm512_and_pi(cast_ireg(x_reg), pcast_ireg(const_vector_msb));
  __mmask16 equal = _mm512_cmpeq_pi(no_sign, pcast_ireg(const_vector_msb));
  int all_one = _mm512_kortestc(equal, equal);

  return all_one?1:0;
}

int __attribute__((overloadable)) any(short16 x)
{
  __m512i zeros = (__m512i)_mm512_setzero();
  __m512 x_reg = ext_from_s16(x);

  __m512i no_sign = _mm512_and_pi(cast_ireg(x_reg), pcast_ireg(const_vector_msb));
  __mmask16 equal = _mm512_cmpeq_pi(no_sign, zeros);
  int all_zero = _mm512_kortestz(equal, equal);

  return all_zero?0:1;
}

int __attribute__((overloadable)) all(short16 x)
{
  __m512 x_reg = ext_from_s16(x);

  __m512i no_sign = _mm512_and_pi(cast_ireg(x_reg), pcast_ireg(const_vector_msb));
  __mmask16 equal = _mm512_cmpeq_pi(no_sign, pcast_ireg(const_vector_msb));
  int all_one = _mm512_kortestc(equal, equal);

  return all_one?1:0;
}
DEF_FUNCION_ANY_ALL(short, any_short, all_short)

// any/all for ushort
DEF_ANY_FUNCTION_OVERLOAD( short, ushort)
DEF_ALL_FUNCTION_OVERLOAD( short, ushort)

// any/all for int
int any_int(int16 vector_reg, __mmask16 k1)
{
  __m512i zeros = (__m512i)_mm512_setzero();
  __m512 x_reg = cast_reg(vector_reg);

  x_reg = cast_reg(_mm512_mask_and_pi(cast_ireg(x_reg), k1, zeros, zeros));
  __m512i no_sign = _mm512_and_pi(cast_ireg(x_reg), pcast_ireg(const_vector_msb));
  __mmask16 equal = _mm512_cmpeq_pi(no_sign, zeros);
  int all_zero = _mm512_kortestz(equal, equal);

  return all_zero?0:1;
}

int all_int(int16 vector_reg, __mmask16 k1)
{
  __m512 x_reg = cast_reg(vector_reg);
  x_reg = _mm512_mask_loadd(x_reg, k1, (void const *)const_vector_zeros, _MM_FULLUPC_NONE, _MM_BROADCAST32_NONE, _MM_HINT_NONE);

  __m512i no_sign = _mm512_and_pi(cast_ireg(x_reg), pcast_ireg(const_vector_msb));
  __mmask16 equal = _mm512_cmpeq_pi(no_sign, pcast_ireg(const_vector_msb));
  int all_one = _mm512_kortestc(equal, equal);

  return all_one?1:0;
}

int __attribute__((overloadable)) any(int16 x)
{
  __m512i no_sign = _mm512_and_pi(cast_ireg(x), pcast_ireg(const_vector_msb));
  __mmask16 equal = _mm512_cmpeq_pi(no_sign, pcast_ireg(const_vector_zeros));
  int all_zero = _mm512_kortestz(equal, equal);

  return all_zero?0:1;
}

int __attribute__((overloadable)) all(int16 x)
{
  __m512i no_sign = _mm512_and_pi(cast_ireg(x), pcast_ireg(const_vector_msb));
  __mmask16 equal = _mm512_cmpeq_pi(no_sign, pcast_ireg(const_vector_msb));
  int all_one = _mm512_kortestc(equal, equal);

  return all_one?1:0;
}
DEF_FUNCION_ANY_ALL(int, any_int, all_int)

// any/all for uint
DEF_ANY_FUNCTION_OVERLOAD( int, uint)
DEF_ALL_FUNCTION_OVERLOAD( int, uint)

// any/all for long
int __attribute__((overloadable)) any(long2 x)
{
  return ( ( (x.lo & const_long_msb) !=0 ) ||
       ( ( (x.hi & const_long_msb) !=0 ) ) )?1:0;
}

int __attribute__((overloadable)) all(long2 x)
{
  return ( ( ( x.lo & const_long_msb) == const_long_msb) &&
       ( ( x.hi & const_long_msb) == const_long_msb) )?1:0;
}

// Using two 32-bit element for comparison
int __attribute__((overloadable)) any(long3 x)
{
  __mmask16 k1 = 0xFFC0;
  long8 vector_reg;
  vector_reg.s012 = x;

  return any_int((int16)vector_reg, k1);
}

int __attribute__((overloadable)) all(long3 x)
{
  __mmask16 k1 = 0xFFC0;
  long8 vector_reg;
  vector_reg.s012 = x;

  return all_int((int16)vector_reg, k1);
}

int __attribute__((overloadable)) any(long4 x)
{
  __mmask16 k1 = 0xFF00;
  long8 vector_reg;
  vector_reg.lo = x;

  return any_int((int16)vector_reg, k1);
}

int __attribute__((overloadable)) all(long4 x)
{
  __mmask16 k1 = 0xFF00;
  long8 vector_reg;
  vector_reg.lo = x;

  return all_int((int16)vector_reg, k1);
}

int __attribute__((overloadable)) any(long8 x)
{
  return any((int16)x);
}

int __attribute__((overloadable)) all(long8 x)
{
  return all((int16)x);
}

int __attribute__((overloadable)) any(long16 x)
{
  int any_zero = any((int16)x.lo);
  any_zero = any_zero || any((int16)x.hi);

  return any_zero;
}

int __attribute__((overloadable)) all(long16 x)
{
  int all_zero = all((int16)x.lo);
  all_zero = all_zero && all((int16)x.hi);

  return all_zero;
}

// any/all for ulong
DEF_ANY_FUNCTION_OVERLOAD( long, ulong)
DEF_ALL_FUNCTION_OVERLOAD( long, ulong)

// sel_signed/bitselect/sel_unsigned
#define sel_kernel(a, b, c, d) ((c & d)?b:a)

#define bitsel_kernel(a, b, c) ((b&c) | (a & ~c))

#define fp_bitsetl_kernel(a, b, c) (fp2int(a) & fp2int(c) | (fp2int(b) & ~fp2int(c) ))


char __attribute__((overloadable)) select(char a, char b, char c)
{
  return sel_kernel(a, b, c, const_char_msb);
}

char __attribute__((overloadable)) select(char a, char b, uchar c)
{
  return sel_kernel(a, b, c, const_char_msb);
}

char __attribute__((overloadable)) bitselect(char a, char b, char c)
{
  return (b & c) | (a & ~c);
}

char16 __attribute__((overloadable)) select(char16 a, char16 b, char16 c)
{
  char16 result;
  __m512 c_reg = ext_from_s8(c);
  __m512 a_reg = ext_from_s8(a);
  __m512 b_reg = ext_from_s8(b);

  __m512i no_sign = _mm512_and_pi(cast_ireg(c_reg), pcast_ireg(const_vector_msb));
  __mmask16 sel = _mm512_cmpeq_pi(no_sign, pcast_ireg(const_vector_msb));
  __m512i tempi = cast_ireg(b_reg);

  tempi = _mm512_mask_or_pi(tempi, sel, cast_ireg(a_reg), cast_ireg(a_reg));
  trunc_to_s8(tempi, result);

  return result;
}

char16 __attribute__((overloadable)) bitselect(char16 a, char16 b, char16 c)
{
  char16 result;
  __m512 c_reg = ext_from_s8(c);
  __m512 a_reg = ext_from_s8(a);
  __m512 b_reg = ext_from_s8(b);

  __m512i result_true = _mm512_and_pi(cast_ireg(b_reg), cast_ireg(c_reg));
  __m512i result_false = _mm512_andn_pi(cast_ireg(a_reg), cast_ireg(c_reg));
  result_true = _mm512_or_pi(result_true, result_false);
  trunc_to_s8(result_true, result);

  return result;
}

char16 __attribute__((overloadable)) select(char16 a, char16 b, uchar16 c)
{
  return select(a, b, (char16)c);
}

// sel/bitselect uchar
DEF_SEL_SIGNED_OVERLOAD( char, uchar)
DEF_SEL_UNSIGNED_OVERLOAD(char, uchar)
DEF_BITSELECT_OVERLOAD(char, uchar)

// sel/bitselect short
short __attribute__((overloadable)) select(short a, short b, short c)
{
  return (c&0x8000)?b:a;
}

short __attribute__((overloadable)) select(short a, short b, ushort c)
{
  return (c&0x8000)?b:a;
}

short __attribute__((overloadable)) bitselect(short a, short b, short c)
{
  return (b & c) | (a & ~c);
}

short16 __attribute__((overloadable)) select(short16 a, short16 b, short16 c)
{
  short16 result;
  __m512 c_reg = ext_from_s16(c);
  __m512 a_reg = ext_from_s16(a);
  __m512 b_reg = ext_from_s16(b);

  __m512i no_sign = _mm512_and_pi(cast_ireg(c_reg), pcast_ireg(const_vector_msb));
  __mmask16 sel = _mm512_cmpeq_pi(no_sign, pcast_ireg(const_vector_msb));
  __m512i tempi = cast_ireg(b_reg);

  tempi = _mm512_mask_or_pi(tempi, sel, cast_ireg(a_reg), cast_ireg(a_reg));
  trunc_to_s16(tempi, result);

  return result;
}

short16 __attribute__((overloadable)) bitselect(short16 a, short16 b, short16 c)
{
  short16 result;
  __m512 c_reg = ext_from_s16(c);
  __m512 a_reg = ext_from_s16(a);
  __m512 b_reg = ext_from_s16(b);

  __m512i result_true = _mm512_and_pi(cast_ireg(b_reg), cast_ireg(c_reg));
  __m512i result_false = _mm512_andn_pi(cast_ireg(a_reg), cast_ireg(c_reg));
  result_true = _mm512_or_pi(result_true, result_false);
  trunc_to_s16(result_true, result);

  return result;
}

short16 __attribute__((overloadable)) select(short16 a, short16 b, ushort16 c)
{
  return select(a, b, (short16)c);
}

// sel/bitselect ushort
DEF_SEL_SIGNED_OVERLOAD(short, ushort)
DEF_SEL_UNSIGNED_OVERLOAD(short, ushort)
DEF_BITSELECT_OVERLOAD(short, ushort)

// sel/bitselect int
int __attribute__((overloadable)) select(int a, int b, int c)
{
  return (c&const_msb)?b:a;
}

int __attribute__((overloadable)) select(int a, int b, uint c)
{
  return (c&const_msb)?b:a;
}

int __attribute__((overloadable)) bitselect(int a, int b, int c)
{
  return (b & c) | (a & ~c);
}

int16 __attribute__((overloadable)) select(int16 a, int16 b, int16 c)
{
  __m512i no_sign = _mm512_and_pi(cast_ireg(c), pcast_ireg(const_vector_msb));
  __mmask16 sel = _mm512_cmpeq_pi(no_sign, pcast_ireg(const_vector_msb));
  __m512i tempi = cast_ireg(b);

  tempi = _mm512_mask_or_pi(tempi, sel, cast_ireg(a), cast_ireg(a));

  return (int16)tempi;
}

int16 __attribute__((overloadable)) bitselect(int16 a, int16 b, int16 c)
{
  __m512i result_true = _mm512_and_pi(cast_ireg(b), cast_ireg(c));
  __m512i result_false = _mm512_andn_pi(cast_ireg(a), cast_ireg(c));
  result_true = _mm512_or_pi(result_true, result_false);

  return (int16)result_true;
}

int16 __attribute__((overloadable)) select(int16 a, int16 b, uint16 c)
{
  return select(a, b, (int16)c);
}

// sel/bitselect uint
DEF_SEL_SIGNED_OVERLOAD( int, uint)
DEF_SEL_UNSIGNED_OVERLOAD(int, uint)
DEF_BITSELECT_OVERLOAD(int, uint)

// sel/bitselect float
DEF_SEL_FP_OVERLOAD( int, float, int)
DEF_SEL_FP_OVERLOAD(int, float, uint)
DEF_BITSELECT_OVERLOAD(int, float)

// sel/bitselect long
// mask = const_long_msb
long __attribute__((overloadable)) select(long a, long b, long c)
{
  return (c&const_long_msb)?b:a;
}

long __attribute__((overloadable)) select(long a, long b, ulong c)
{
  return (c&const_long_msb)?b:a;
}

long __attribute__((overloadable)) bitselect(long a, long b, long c)
{
  return (b & c) | (a & ~c);
}

long16 __attribute__((overloadable)) select(long16 a, long16 b, long16 c)
{
  long16 result;
  __m512i no_sign = _mm512_and_pi(cast_ireg(c.lo), pcast_ireg(const_vector_long_msb));
  __mmask16 sel = _mm512_cmpeq_pi(no_sign, pcast_ireg(const_vector_long_msb));
  result.lo = (long8)_mm512_mask_or_pi(cast_ireg(b.lo), sel, cast_ireg(a.lo), cast_ireg(a.lo));

  no_sign = _mm512_and_pi(cast_ireg(c.hi), pcast_ireg(const_vector_long_msb));
  sel = _mm512_cmpeq_pi(no_sign, pcast_ireg(const_vector_long_msb));
  result.hi = (long8)_mm512_mask_or_pi(cast_ireg(b.hi), sel, cast_ireg(a.hi), cast_ireg(a.hi));

  return result;
}

long16 __attribute__((overloadable)) bitselect(long16 a, long16 b, long16 c)
{
  long16 result;
  __m512i result_true = _mm512_and_pi(cast_ireg(b.lo), cast_ireg(c.lo));
  __m512i result_false = _mm512_andn_pi(cast_ireg(a.lo), cast_ireg(c.lo));
  result.lo = (long8)_mm512_or_pi(result_true, result_false);

  result_true = _mm512_and_pi(cast_ireg(b.hi), cast_ireg(c.hi));
  result_false = _mm512_andn_pi(cast_ireg(a.hi), cast_ireg(c.hi));
  result.hi = (long8)_mm512_or_pi(result_true, result_false);

  return result;
}

long16 __attribute__((overloadable)) select(long16 a, long16 b, ulong16 c)
{
  return select(a, b, (long16)c);
}

// sel/bitselect ulong
DEF_SEL_SIGNED_OVERLOAD( long, ulong)
DEF_SEL_UNSIGNED_OVERLOAD(long, ulong)
DEF_BITSELECT_OVERLOAD(long, ulong)

#ifdef __cplusplus
  }
#endif

#endif // defined (__MIC__) || defined(__MIC2__)
