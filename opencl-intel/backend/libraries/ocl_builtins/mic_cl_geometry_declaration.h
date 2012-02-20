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
//  mic_cl_geometry_declaration.h
///////////////////////////////////////////////////////////
#if defined (__MIC__) || defined(__MIC2__)

#ifndef DP_NAN
#define DP_NAN as_double(0xFFF0000000000000L)
#endif

#ifndef SP_NAN
#define SP_NAN as_float(0xFF800000)
#endif

#define RC_RUN_EVEN             0
#define RC_RUN_DOWN             1
#define RC_RUN_UP               2
#define RC_RUN_ZERO             3

// Definition of horizontal (reduce) op
// op: the arithmetic op such as add/sub/max/min
// width: packet element type, such as pi/ps/pd
#define half_quar_reduce( op, data, width) \
  data = _mm512_##op##_##width(data, _mm512_swizzle_##width(data, _MM_SWIZ_REG_CDAB));

#define quar_reduce( op, data, width) \
  data = _mm512_##op##_##width(data, _mm512_swizzle_##width(data, _MM_SWIZ_REG_CDAB)); \
  data = _mm512_##op##_##width(data, _mm512_swizzle_##width(data, _MM_SWIZ_REG_BADC));

// reduce add
#define half_quar_reduce_add_ps(data) \
  half_quar_reduce(add, data, ps)

#define half_quar_reduce_add_pd(data) \
  half_quar_reduce(add, data, pd)

#define quar_reduce_add_ps(data) \
  quar_reduce(add, data, ps)

#define quar_reduce_add_pd(data) \
  quar_reduce(add, data, pd)

// reduce max
#define half_quar_reduce_max_ps(data) \
  half_quar_reduce(max, data, ps)

#define half_quar_reduce_max_pd(data) \
  half_quar_reduce(max, data, pd)

#define quar_reduce_max_ps(data) \
  quar_reduce(max, data, ps)

#define quar_reduce_max_pd(data) \
  quar_reduce(max, data, pd)

// swizzle from CDBA --> (B)BAC
#define s201_s012_conv(data, zeros, k, width)                                                         \
  k = 0xF;                                                                                            \
  data = _mm512_mask_add_##width (data, k, zeros, _mm512_swizzle_##width (data, _MM_SWIZ_REG_BADC));  \
  k = 0x2;                                                                                            \
  /* the s0 has been moved to s2 */                                                                   \
  data = _mm512_mask_add_##width (data, k, zeros, _mm512_swizzle_##width (data, _MM_SWIZ_REG_CCCC));  \
  k = 0x4;                                                                                            \
  /* the s1 has been moved to s3 */                                                                   \
  data = _mm512_mask_add_##width (data, k, zeros, _mm512_swizzle_##width (data, _MM_SWIZ_REG_DDDD))

#define s201_s012_ps( data, zeros, k) \
  s201_s012_conv( data, zeros, k, ps) 

#define s201_s012_pd( data, zeros, k) \
  s201_s012_conv( data, zeros, k, pd) 

// swizzle from CDBA->(D)ACB
#define s120_s012_conv(data, zeros, k, width)   \
  k = 0x7;                                      \
  data = _mm512_mask_add_##width (data, k, zeros, _mm512_swizzle_##width (data, _MM_SWIZ_REG_DACB))

#define s120_s012_ps(data, zeros, k)   \
  s120_s012_conv(data, zeros, k, ps)

#define s120_s012_pd(data, zeros, k)   \
  s120_s012_conv(data, zeros, k, pd)

// macro for normalize
#define e2(data)  data.s01
#define e3(data)  data.s012
#define e4(data)  data.s0123

#define DEF_FP_FUNCTION_A1( FUNC, type, k, mk)          \
type##k __attribute__((overloadable)) FUNC(type##k x)   \
{                                                       \
  type##mk x_reg = (type##mk)0;                         \
  e##k(x_reg) = x;                                      \
  type##mk result = FUNC##_mask(x_reg);                 \
  return e##k(result);                                  \
}


#define DEF_SP( FUNC)                       \
  DEF_FP_FUNCTION_A1( FUNC, float, 2, 16)   \
  DEF_FP_FUNCTION_A1( FUNC, float, 3, 16)   \
  DEF_FP_FUNCTION_A1( FUNC, float, 4, 16)

#define DEF_DP( FUNC)                       \
  DEF_FP_FUNCTION_A1( FUNC, double, 2, 8)   \
  DEF_FP_FUNCTION_A1( FUNC, double, 3, 8)   \
  DEF_FP_FUNCTION_A1( FUNC, double, 4, 8)

#endif // defined (__MIC__) || defined(__MIC2__)
