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
//  mic_cl_relational_declaration.h
///////////////////////////////////////////////////////////
#if defined (__MIC__) || defined(__MIC2__)

#include <intrin.h>
#include "mic_defines.h"
#include "mic_intrinsic_common_vars.h"

#define s_eq(x,y) x==y?-1:0
#define v_eq(x,y) _mm512_cmpeq_ps(x, y)

#define s_neq(x,y) x!=y?-1:0
#define v_neq(x,y) _mm512_cmpneq_ps(x, y)

#define s_nle(x,y) x>y?-1:0
#define v_nle(x,y) _mm512_cmpnle_ps(x, y)

#define s_nlt(x,y) x>=y?-1:0
#define v_nlt(x,y) _mm512_cmpnlt_ps(x, y)

#define s_lt(x,y) x<y?-1:0
#define v_lt(x,y) _mm512_cmplt_ps(x, y)

#define s_le(x,y) x<=y?-1:0
#define v_le(x,y) _mm512_cmple_ps(x, y)

// cast data for intX
#define vcast_int2(data) ((int16)data).s01
#define vcast_int3(data) ((int16)data).s012
#define vcast_int4(data) ((int16)data).s0123
#define vcast_int8(data) ((int16)data).lo
#define vcast_int16(data) ((int16)data)

#define cmp_scalar(x, y, op, fun_name) \
int  __attribute__((overloadable)) fun_name(float x, float y) \
{ \
  return s_##op (x,y); \
}

#define cmp_kernel2(x, y, op) \
  float16 x_reg, y_reg; \
  x_reg.lo.lo.lo = x; \
  y_reg.lo.lo.lo = y; \
  __mmask16 equal = v_##op (cast_reg(x_reg), cast_reg(y_reg));

#define cmp_kernel3(x, y, op) \
  float16 x_reg, y_reg; \
  x_reg.s012 = x; \
  y_reg.s012 = y; \
  __mmask16 equal = v_##op (cast_reg(x_reg), cast_reg(y_reg));

#define cmp_kernel4(x, y, op) \
  float16 x_reg, y_reg; \
  x_reg.lo.lo = x; \
  y_reg.lo.lo = y; \
  __mmask16 equal = v_##op (cast_reg(x_reg), cast_reg(y_reg));

#define cmp_kernel8(x, y, op) \
  float16 x_reg, y_reg; \
  x_reg.lo = x; \
  x_reg.lo = y; \
  __mmask16 equal = v_##op (cast_reg(x_reg), cast_reg(y_reg));

#define cmp_kernel16(x, y, op) \
  __mmask16 equal = v_##op (cast_reg(x), cast_reg(y));

#define cmp_vector(num, x, y, op, fun_name) \
int##num __attribute__((overloadable)) fun_name(float##num x, float##num y) \
{ \
  __m512 zeros = _mm512_setzero(); \
  __m512 ones =  _mm512_loadd(void_const_cast(const_vector_ones[0]), _MM_FULLUPC_NONE, _MM_BROADCAST32_NONE, _MM_HINT_NONE); \
  \
  cmp_kernel##num (x, y, op) \
  \
  __m512i tempi = cast_ireg(zeros); \
  tempi = _mm512_mask_or_pi(tempi, equal, cast_ireg(ones), cast_ireg(ones)); \
  return vcast_##int##num(zeros); \
}

#define gen_isequal \
cmp_scalar(x, y, eq, isequal) \
cmp_vector(2, x, y, eq, isequal) \
cmp_vector(3, x, y, eq, isequal) \
cmp_vector(4, x, y, eq, isequal) \
cmp_vector(8, x, y, eq, isequal) \
cmp_vector(16, x, y, eq, isequal)

#define gen_isnotequal \
cmp_scalar(x, y, eq, isnotequal) \
cmp_vector(2, x, y, neq, isnotequal) \
cmp_vector(3, x, y, neq, isnotequal) \
cmp_vector(4, x, y, neq, isnotequal) \
cmp_vector(8, x, y, neq, isnotequal) \
cmp_vector(16, x, y, neq, isnotequal)

#define gen_isgreater \
cmp_scalar(x, y, nle, isgreater) \
cmp_vector(2, x, y, nle, isgreater) \
cmp_vector(3, x, y, nle, isgreater) \
cmp_vector(4, x, y, nle, isgreater) \
cmp_vector(8, x, y, nle, isgreater) \
cmp_vector(16, x, y, nle, isgreater)

#define gen_isgreaterequal \
cmp_scalar(x, y, nlt, isgreaterequal) \
cmp_vector(2, x, y, nlt, isgreaterequal) \
cmp_vector(3, x, y, nlt, isgreaterequal) \
cmp_vector(4, x, y, nlt, isgreaterequal) \
cmp_vector(8, x, y, nlt, isgreaterequal) \
cmp_vector(16, x, y, nlt, isgreaterequal)

#define gen_isless \
cmp_scalar(x, y, lt, isless) \
cmp_vector(2, x, y, lt, isless) \
cmp_vector(3, x, y, lt, isless) \
cmp_vector(4, x, y, lt, isless) \
cmp_vector(8, x, y, lt, isless) \
cmp_vector(16, x, y, lt, isless)

#define gen_islessequal \
cmp_scalar(x, y, le, islessequal) \
cmp_vector(2, x, y, le, islessequal) \
cmp_vector(3, x, y, le, islessequal) \
cmp_vector(4, x, y, le, islessequal) \
cmp_vector(8, x, y, le, islessequal) \
cmp_vector(16, x, y, le, islessequal)

// islessgreater

// any/all scalar
#define any_all(func, type, mask) \
int __attribute__((overloadable)) func(type x) \
{ \
  return (x&mask)?1:0; \
}

#define gen_any_all_scalar \
any_all(any, char, 0x80) \
any_all(all, char, 0x80) \
any_all(any, short, 0x8000) \
any_all(all, short, 0x8000) \
any_all(any, int, 0x80000000) \
any_all(all, int, 0x80000000) \
any_all(any, long, const_long_msb) \
any_all(all, long, const_long_msb)

#define DEF_FLOAT_RELATIONAL_vx( FUNC ) \
int2 __attribute__((overloadable)) FUNC(float2 x) \
{ \
  float16 x_reg; \
  x_reg.s01 = x; \
 \
  int16 result = FUNC(x_reg); \
 \
  return result.s01; \
} \
 \
int3 __attribute__((overloadable)) FUNC(float3 x) \
{ \
  float16 x_reg; \
  x_reg.s012 = x; \
 \
  int16 result = FUNC(x_reg); \
 \
  return result.s012; \
} \
 \
int4 __attribute__((overloadable)) FUNC(float4 x) \
{ \
  float16 x_reg; \
  x_reg.s0123 = x; \
  \
  int16 result = FUNC(x_reg); \
  \
  return result.s0123; \
} \
  \
int8 __attribute__((overloadable)) FUNC(float8 x) \
{ \
  float16 x_reg; \
  x_reg.lo = x; \
  \
  int16 result = FUNC(x_reg); \
  \
  return result.lo; \
} 

#define DEF_FLOAT_RELATIONAL_vx_vx( FUNC ) \
int2 __attribute__((overloadable)) FUNC(float2 x, float2 y) \
{ \
  float16 x_reg, y_reg; \
  x_reg.s01 = x; \
  y_reg.s01 = y; \
 \
  int16 result = FUNC(x_reg, y_reg); \
  \
  return result.s01; \
} \
\
int3 __attribute__((overloadable)) FUNC(float3 x, float3 y) \
{ \
  float16 x_reg, y_reg; \
  x_reg.s012 = x; \
  y_reg.s012 = y; \
  \
  int16 result = FUNC(x_reg, y_reg); \
  \
  return result.s012; \
} \
  \
int4 __attribute__((overloadable)) FUNC(float4 x, float4 y) \
{ \
  float16 x_reg, y_reg; \
  x_reg.s0123 = x; \
  y_reg.s0123 = y; \
  \
  int16 result = FUNC(x_reg, y_reg); \
  \
  return result.s0123; \
} \
 \
int8 __attribute__((overloadable)) FUNC(float8 x, float8 y) \
{ \
  float16 x_reg, y_reg; \
  x_reg.lo = x; \
  y_reg.lo = y; \
  \
  int16 result = FUNC(x_reg, y_reg); \
  \
  return result.lo; \
} 

#define e2(x) x.s01
#define e3(x) x.s012
#define e4(x) x.lo.lo
#define e8(x) x.lo

#define DEF_FUNCTION_TYPE_vx_vx_x( FUNC, data_type, select_type, k) \
data_type##k __attribute__((overloadable)) FUNC( data_type##k a, data_type##k b, select_type##k c) \
{ \
  data_type##16 result; \
  data_type##16 a_reg, b_reg; \
  select_type##16 c_reg; \
  \
  e##k(a_reg) = a; \
  e##k(b_reg) = b; \
  e##k(c_reg) = c; \
  result = FUNC(a_reg, b_reg, c_reg); \
  return e##k(result); \
}

#define DEF_FUNCTION_TYPE_vx_vx_x_ALL(FUNC, data_type, control_type) \
  DEF_FUNCTION_TYPE_vx_vx_x( FUNC, data_type, control_type, 2) \
  DEF_FUNCTION_TYPE_vx_vx_x( FUNC, data_type, control_type, 3) \
  DEF_FUNCTION_TYPE_vx_vx_x( FUNC, data_type, control_type, 4) \
  DEF_FUNCTION_TYPE_vx_vx_x( FUNC, data_type, control_type, 8)

#define DEF_FUNCION_ANY_ALL_ONE(any_func, all_func, in_type, mask, k) \
int __attribute__((overloadable)) any(in_type##k x) \
{ \
  __mmask16 k1 = mask; \
  in_type##16 vector_reg; \
  e##k(vector_reg) = x; \
  \
  return any_func(vector_reg, k1); \
} \
int __attribute__((overloadable)) all(in_type##k x) \
{ \
  __mmask16 k1 = mask; \
  in_type##16 vector_reg; \
  e##k(vector_reg) = x; \
  \
  return all_func(vector_reg, k1); \
}

#define DEF_FUNCION_ANY_ALL(in_type, any_func, all_func) \
  DEF_FUNCION_ANY_ALL_ONE(any_func, all_func, in_type, 0xFFFC, 2) \
  DEF_FUNCION_ANY_ALL_ONE(any_func, all_func, in_type, 0xFFF8, 3) \
  DEF_FUNCION_ANY_ALL_ONE(any_func, all_func, in_type, 0xFFF0, 4) \
  DEF_FUNCION_ANY_ALL_ONE(any_func, all_func, in_type, 0xFF00, 8)

#define DEF_FUNCTION_OVERLOAD_vx( FUNC, output_type, ref_type, new_type) \
output_type __attribute__((overloadable)) FUNC(new_type x) \
{ \
  return FUNC((ref_type)x); \
}

#define DEF_FUNCTION_OVERLOAD_vx_ALL( FUNC, output_type, ref_type, new_type) \
  DEF_FUNCTION_OVERLOAD_vx(FUNC, output_type, ref_type##2, new_type##2) \
  DEF_FUNCTION_OVERLOAD_vx(FUNC, output_type, ref_type##3, new_type##3) \
  DEF_FUNCTION_OVERLOAD_vx(FUNC, output_type, ref_type##4, new_type##4) \
  DEF_FUNCTION_OVERLOAD_vx(FUNC, output_type, ref_type##8, new_type##8) \
  DEF_FUNCTION_OVERLOAD_vx(FUNC, output_type, ref_type##16, new_type##16)

#define DEF_ANY_FUNCTION_OVERLOAD(ref_type, new_type) DEF_FUNCTION_OVERLOAD_vx_ALL( any, int, ref_type, new_type)
#define DEF_ALL_FUNCTION_OVERLOAD(ref_type, new_type) DEF_FUNCTION_OVERLOAD_vx_ALL( all, int, ref_type, new_type)

#define DEF_FUNCTION_OVERLOAD_vx_vx_vx( FUNC, ref_type, new_type) \
new_type __attribute__((overloadable)) FUNC(new_type a, new_type b, new_type c) \
{ \
  ref_type result = FUNC((ref_type)a, (ref_type)b, (ref_type) c); \
  return (new_type) result; \
}

#define DEF_FUNCTION_OVERLOAD_vx_vx_x( FUNC, ref_type, new_type, type) \
new_type __attribute__((overloadable)) FUNC(new_type a, new_type b, type c) \
{ \
  ref_type result = FUNC((ref_type)a, (ref_type)b, c); \
  return (new_type) result; \
}

#define DEF_FUNCTION_OVERLOAD_vx_vx_vx_ALL( FUNC, ref_type, new_type) \
  DEF_FUNCTION_OVERLOAD_vx_vx_vx(FUNC, ref_type, new_type) \
  DEF_FUNCTION_OVERLOAD_vx_vx_vx(FUNC, ref_type##2, new_type##2) \
  DEF_FUNCTION_OVERLOAD_vx_vx_vx(FUNC, ref_type##3, new_type##3) \
  DEF_FUNCTION_OVERLOAD_vx_vx_vx(FUNC, ref_type##4, new_type##4) \
  DEF_FUNCTION_OVERLOAD_vx_vx_vx(FUNC, ref_type##8, new_type##8) \
  DEF_FUNCTION_OVERLOAD_vx_vx_vx(FUNC, ref_type##16, new_type##16)

#define DEF_FUNCTION_OVERLOAD_vx_vx_x_ALL( FUNC, ref_type, new_type, type) \
  DEF_FUNCTION_OVERLOAD_vx_vx_x(FUNC, ref_type, new_type, type) \
  DEF_FUNCTION_OVERLOAD_vx_vx_x(FUNC, ref_type##2, new_type##2, type##2) \
  DEF_FUNCTION_OVERLOAD_vx_vx_x(FUNC, ref_type##3, new_type##3, type##3) \
  DEF_FUNCTION_OVERLOAD_vx_vx_x(FUNC, ref_type##4, new_type##4, type##4) \
  DEF_FUNCTION_OVERLOAD_vx_vx_x(FUNC, ref_type##8, new_type##8, type##8) \
  DEF_FUNCTION_OVERLOAD_vx_vx_x(FUNC, ref_type##16, new_type##16, type##16)

#define DEF_SEL_SIGNED_OVERLOAD( ref_type, new_type ) \
  DEF_FUNCTION_OVERLOAD_vx_vx_x_ALL( select, ref_type, new_type, ref_type)

#define DEF_SEL_UNSIGNED_OVERLOAD( ref_type, new_type) \
  DEF_FUNCTION_OVERLOAD_vx_vx_x_ALL( select, ref_type, new_type, new_type)

#define DEF_BITSELECT_OVERLOAD( ref_type, new_type) \
  DEF_FUNCTION_OVERLOAD_vx_vx_vx_ALL( bitselect, ref_type, new_type)

#define DEF_SEL_FP_OVERLOAD( ref_type, new_type, type) \
  DEF_FUNCTION_OVERLOAD_vx_vx_x_ALL( select, ref_type, new_type, type)

#endif // RELATIONAL_FUNCTIONS_MIC__HPP
