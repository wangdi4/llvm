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
//  mic_geometry_functions.cpp
///////////////////////////////////////////////////////////
#if defined (__MIC__) || defined(__MIC2__)

#ifdef __cplusplus
extern "C" {
#endif

#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#include <intrin.h>


#include "mic_defines.h"
#include "mic_cl_geometry_declaration.h"

__attribute__((overloadable)) float sqrt(float);
__attribute__((overloadable)) double sqrt(double);
__attribute__((overloadable)) float half_rsqrt(float);

// dot
float __attribute__((overloadable)) dot(float x, float y)
{
  return (x*y);
}

float __attribute__((overloadable)) dot(float2 x, float2 y)
{
  float16 x_reg = (float16)0.0;
  float16 y_reg = (float16)0.0;
  x_reg.s01 = x;
  y_reg.s01 = y;
  __m512 result = (__m512)x_reg;
  result = _mm512_mul_ps(result, cast_reg(y_reg));
  half_quar_reduce_add_ps(result);

  return ((float16)result).s0;
}

float __attribute__((overloadable)) dot(float3 x, float3 y)
{
  float16 x_reg = (float16)0.0;
  float16 y_reg = (float16)0.0;
  x_reg.s012 = x;
  y_reg.s012 = y;
  __m512 result = (__m512)x_reg;
  result = _mm512_mul_ps(result, cast_reg(y_reg));
  quar_reduce_add_ps(result);

  return ((float16)result).s0;
}

float __attribute__((overloadable)) dot(float4 x, float4 y)
{
  float16 x_reg = (float16)0;
  float16 y_reg = (float16)0;
  x_reg.s0123 = x;
  y_reg.s0123 = y;
  __m512 temp = (__m512)x_reg;
  __m512 result = (__m512)x_reg;
  result = _mm512_mul_ps(result, cast_reg(y_reg));
  quar_reduce_add_ps(result);

  return ((float16)result).s0;
}

// dot double
double __attribute__((overloadable)) dot(double x, double y)
{
  return (x*y);
}

double __attribute__((overloadable)) dot(double2 x, double2 y)
{
  double8 x_reg = (double)0;
  double8 y_reg = (double)0;
  x_reg.s01 = x;
  y_reg.s01 = y;
  __m512d data = cast_dreg(x_reg*y_reg);
  half_quar_reduce_add_pd(data);

  return ((double8)data).s0;
}

double __attribute__((overloadable)) dot(double3 x, double3 y)
{
  double8 x_reg = (double)0;
  double8 y_reg = (double)0;
  x_reg.s012 = x;
  y_reg.s012 = y;
  __m512d data = cast_dreg(x_reg*y_reg);
  quar_reduce_add_pd(data);

  return ((double8)data).s0;
}

double __attribute__((overloadable)) dot(double4 x, double4 y)
{
  double8 x_reg = (double)0;
  double8 y_reg = (double)0;
  x_reg.s0123 = x;
  y_reg.s0123 = y;
  __m512d data = cast_dreg(x_reg*y_reg);
  quar_reduce_add_pd(data);

  return ((double8)data).s0;
}

// length
float  __attribute__((overloadable)) length(float x)
{
  return fabs(x);
}

float  __attribute__((overloadable)) length(float2 x)
{
  return hypot(x.s0, x.s1);
}

float __attribute__((overloadable)) length(float16 x)
{
  __m512d data = _mm512_cvtl_ps2pd(cast_reg(x));
  data = _mm512_mul_pd(data, data);
  quar_reduce_add_pd(data);
  __m512 res = _mm512_undefined_ps();
  res = _mm512_cvtl_pd2ps(res, data, RC_RUN_DOWN);
  
  float sum = ((float16)res).s0;

  return sqrt(sum);
}

float __attribute__((overloadable)) length(float3 x)
{
  float16 x_reg = (float16)0.0;
  x_reg.s012 = x;
  return length(x_reg);
}

float __attribute__((overloadable)) length(float4 x)
{
  float16 x_reg = (float16)0.0;
  x_reg.s0123 = x;
  return length(x_reg);
}

double __attribute__((overloadable)) length(double2 x)
{
  return hypot(x.s0, x.s1);
}

double __attribute__((overloadable)) length(double8 data)
{
  long8 sign_mask   = (long8)(0x7fffffffffffffffL);
  long8 exp_upper   = (long8)(0x5ff0000000000000L); // the upper limit of exponent that might trigger overflow
  long8 exp_zero    = (long8)(0x4000000000000000L); // zero exponent
  long8 exp_lower   = (long8)(0x2010000000000000L); // the lower limit of exponent that might trigger underflow

  double8 norm    = (double8)(1.0);
  double8 denorm  = (double8)(1.0);
  // Get the norm and denorm
  __m512i abs_data = _mm512_and_pq(cast_ireg(data), cast_ireg(sign_mask));
  // The "any" function is implemented through _mm512_kortestz, so there should result no (or almost no)
  // branch prediction penality
  __mmask16 match     = _mm512_cmplt_pi(abs_data, cast_ireg(exp_lower));
  __mmask16 zero_exp  = _mm512_cmple_pi(abs_data, cast_ireg(exp_zero));
  // Any of the data is smaller than the underflow threshold, pluse none of the data will result overflow after multiplied
  // with norm; !_mm512_kortestz is a "any" operation and _mm512_kortestc is a "all" operation
  if (!_mm512_kortestz(match, match) && _mm512_kortestc(zero_exp, zero_exp)){
    norm    = (double8)exp_upper;
    denorm  = (double8)exp_lower;
  }

  // According to OpenCL manual, overflow needs to be addressed first, then underflow
  match = _mm512_cmpnle_pi(abs_data, cast_ireg(exp_upper));
  if (!_mm512_kortestz(match, match)){
    norm    = (double8)exp_lower;
    denorm  = (double8)exp_upper;
  }

  data = _mm512_mul_pd(data, cast_dreg(norm));
  data = _mm512_mul_pd(data, data);

  quar_reduce_add_pd(data);

  return sqrt(data.s0)*denorm.s0;
}

double __attribute__((overloadable)) length(double3 x)
{
  double8 data = (double8)0;
  data.s012 = x;
  return length(data);
}

double __attribute__((overloadable)) length(double4 x)
{
  double8 data = (double8)0;
  data.s0123 = x;
  return length(data);
}

// normalize
float __attribute__((overloadable)) normalize(float x)
{
  int ifnan = isnan(x);
  int nan = 0xFF800000;
  int result = (as_int(x) & 0x80000000) | 0x3f800000;
  result = (ifnan & nan) | (result & ~nan);
  return (x==0)?0.0:as_float(result);
}

float16  __attribute__((overloadable)) normalize_mask(float16 x)
{

  int16 const_vector_msb = (int16)0x80000000;
  float16 one = (float16)1.0f;
  int16 ifnan = isnan(x);
  int16 ifinf = isinf(x);
  float16 all_nan = (float16)SP_NAN; // SP NAN

  // one_vector=( x >0)?1.0:-1.0
  __m512i sign_mask = _mm512_and_pi((__m512i)x, cast_ireg(const_vector_msb));
  __m512i one_vector = _mm512_or_pi(cast_ireg(one), sign_mask); // 1 *sign_bit(x)

  // data = isinf(x)?one:data
  __m512i data = cast_ireg(x);
  data = _mm512_andn_pi(data, cast_ireg(ifinf));
  one_vector = _mm512_and_pi(one_vector, cast_ireg(ifinf));
  data = _mm512_or_pi(data, one_vector);

  // returun 16NAN if any of the element is NAN
  if (any(ifnan))
    return all_nan;

  __mmask16 equal = _mm512_cmpeq_ps(x, cast_reg((float16)0.0));
  int flag;

  // <Caveat> Use a more efficient 4-element length since normalize only support up to 4 elements
  float norm = length(x.lo.lo);

  // norm = all_zero?1.0:norm;
  flag = _mm512_kortestc(equal, equal);
  norm = flag?1.0:norm;

  float16 result = x / norm;

  return result;
}
DEF_SP(normalize)

double __attribute__((overloadable)) normalize(double x)
{
  long ifnan = isnan(x);
  long nan = 0xFFF0000000000000L;
  long result = (as_long(x) & 0x8000000000000000L) | 0x3FF0000000000000L;
  result = (ifnan & nan) | (result & ~nan);
  return (x==0)?0.0:as_double(result);
}

double8  __attribute__((overloadable)) normalize_mask(double8 x)
{
  long8 const_vector_long_msb= (long8)0x8000000000000000L;
  double8 one = (double8)1.0;
  long8 ifnan = isnan(x);
  long8 ifinf = isinf(x);
  double8 all_nan = (double8)DP_NAN; // DP NAN

  // one_vector=( x >0)?1.0:-1.0
  __m512i sign_mask = _mm512_and_pq((__m512i)x, cast_ireg(const_vector_long_msb));
  __m512i one_vector = _mm512_or_pq(cast_ireg(one), sign_mask); // 1 *sign_bit(x)

  // data = isinf(x)?one:data
  __m512i data = cast_ireg(x);
  data = _mm512_andn_pq(data, cast_ireg(ifinf));
  one_vector = _mm512_and_pq(one_vector, cast_ireg(ifinf));
  data = _mm512_or_pq(data, one_vector);

  // returun 16NAN if any of the element is NAN
  if (any(ifnan))
    return all_nan;

  __mmask16 equal = _mm512_cmpeq_pd(x, cast_dreg((double8)0.0));
  int flag;

  // <Caveat> Use a more efficient 4-element length since normalize only support up to 4 elements
  double norm = length(x.lo.lo);

  // norm = all_zero?1.0:norm;
  flag = _mm512_kortestc(equal, equal);
  norm = flag?1.0:norm;

  double8 result = x / norm;

  return result;
}
DEF_DP(normalize)

  // cross
float4 __attribute__((overloadable)) cross(float4 x, float4 y)
{
  float3 temp = cross (x.s012, y.s012);
  float4 result;
  result.s012 = temp;
  result.s3 =0;

  return result;
}

//
// = (x2.y3-x3.y2)i + (x3.y1 -x1.y3)j + (x1.y2 - x2.y1)k
// temp_x = (x2, x3, x1)
// temp_y = (y3, y1, y2)
// temp1= temp_x * temp_y
// temp_x = (x3, x1, x2)
// temp_y = (y2, y3, y1)
// temp2 = temp_x * temp_y
// result = temp1 - temp2
//
float3 __attribute__((overloadable)) cross(float3 x, float3 y)
{
  __m512 zeros;
  __m512 result;
  float16 f_result = (float16)0.0f;
  __m512 x_reg, y_reg;
  __mmask16 k;

  zeros = cast_reg((float16)0.0f);
  // Below is an implementaiton of "temp_x.s012 = x.s120"
  f_result.s012 = x;
  x_reg = cast_reg(f_result);
  s120_s012_ps(x_reg, zeros, k);

  // Below is an implementation of temp_y.s012 = y.s201"
  f_result.s012 = y;
  y_reg = cast_reg(f_result);
  s201_s012_ps(y_reg, zeros, k);

  x_reg = _mm512_mul_ps(x_reg, y_reg);
  result = x_reg;

  f_result.s012 = x;
  x_reg = cast_reg(f_result);
  s201_s012_ps(x_reg, zeros, k);

  f_result.s012 = y;
  y_reg = cast_reg(f_result);
  s120_s012_ps(y_reg, zeros, k); // imply k <--0x7

  y_reg = _mm512_mul_ps(y_reg, x_reg);
  result = _mm512_sub_ps(result, y_reg);

  return ((float16)result).s012;
}

double4 __attribute__((overloadable)) cross(double4 x, double4 y)
{
  double3 temp = cross (x.s012, y.s012);
  double4 result;
  result.s012 = temp;
  result.s3 =0;

  return result;
}

//
// = (x2.y3-x3.y2)i + (x3.y1 -x1.y3)j + (x1.y2 - x2.y1)k
// temp_x = (x2, x3, x1)
// temp_y = (y3, y1, y2)
// temp1= temp_x * temp_y
// temp_x = (x3, x1, x2)
// temp_y = (y2, y3, y1)
// temp2 = temp_x * temp_x
// result = temp1 - temp2
//
double3 __attribute__((overloadable)) cross(double3 x, double3 y)
{
  __m512d zeros;
  __m512d result;
  double8 f_result = (double8)0.0;
  __m512d x_reg, y_reg;
  __mmask16 k;

  zeros = cast_dreg((double8)0);

  f_result.s012 = x;
  x_reg = cast_dreg(f_result);
  s120_s012_pd(x_reg, zeros, k);

  f_result.s012 = y;
  y_reg = cast_dreg(f_result);
  s201_s012_pd(y_reg, zeros, k);

  x_reg = _mm512_mul_pd(x_reg, y_reg);
  result = x_reg;

  f_result.s012 = x;
  x_reg = cast_dreg(f_result);
  s201_s012_pd(x_reg, zeros, k);

  f_result.s012 = y;
  y_reg = cast_dreg(f_result);
  s120_s012_pd(y_reg, zeros, k); // imply k <--0x7

  y_reg = _mm512_mul_pd(y_reg, x_reg);
  result = _mm512_sub_pd(result, y_reg);

  return ((double8)result).s012;
}

// distance
float __attribute__((overloadable)) distance(float x, float y)
{
  return fabs(x-y);
}

float __attribute__((overloadable)) distance(float2 x, float2 y)
{
  return  length(x-y);
}

float __attribute__((overloadable)) distance(float3 x, float3 y)
{
  return  length(x-y);
}

float __attribute__((overloadable)) distance(float4 x, float4 y)
{
  return  length(x-y);
}

double __attribute__((overloadable)) distance(double x, double y)
{
  return length(x-y);
}

double __attribute__((overloadable)) distance(double2 x, double2 y)
{
  return  length(x-y);
}

double __attribute__((overloadable)) distance(double3 x, double3 y)
{
  return  length(x-y);
}

double __attribute__((overloadable)) distance(double4 x, double4 y)
{
  return  length(x-y);
}

// fast_length
float __attribute__((overloadable)) fast_length(float x)
{
  return fabs(x);
}

float __attribute__((overloadable)) fast_length(float2 x)
{
  return half_sqrt(dot(x,x));
}

float __attribute__((overloadable)) fast_length(float3 x)
{
  return half_sqrt(dot(x,x));
}

float __attribute__((overloadable)) fast_length(float4 x)
{
  return half_sqrt(dot(x,x));
}

// fast_distance
float __attribute__((overloadable)) fast_distance(float x, float y)
{
  return fast_length(x-y);
}

float __attribute__((overloadable)) fast_distance(float2 x, float2 y)
{
  return fast_length(x-y);
}

float __attribute__((overloadable)) fast_distance(float3 x, float3 y)
{
  return fast_length(x-y);
}

float __attribute__((overloadable)) fast_distance(float4 x, float4 y)
{
  return fast_length(x-y);
}

// fast_normalize
float __attribute__((overloadable)) fast_normalize(float x)
{
  return (x < sqrt(FLT_MIN))?x:(x/fabs(x));
}

float16  __attribute__((overloadable)) fast_normalize_mask(float16 x)
{
  // Below instruction is added to force FTZ happens when program runs
  // on FTZ mode; no mode checking is necessary
  float16 ref = sqrt(x);

  __m512 temp;
  float sum;
  float16 zeros = (float16)0.0f;
  __mmask16 small_val = _mm512_cmpeq_ps(ref, cast_reg(zeros));
  temp = (__m512)x;
  temp = (__m512)_mm512_mask_and_pi(cast_ireg(temp), small_val, cast_ireg(zeros), cast_ireg(zeros));
  temp = _mm512_mul_ps(temp, temp);
  quar_reduce_add_ps(temp);
  sum = ((float16)temp).s0;

  // Below include checking if all(source == 0) and sum-of-squae is < FLT_MIN
  sum = (sum < FLT_MIN)?1.0:sum;

  sum = half_rsqrt(sum);
  temp = (__m512)x;
  temp = _mm512_mul_ps(temp, (__m512)((float16)sum));
  
  return (float16)temp;
}

DEF_SP(fast_normalize)

#ifdef __cplusplus
}
#endif

#endif // defined (__MIC__) || defined(__MIC2__)

