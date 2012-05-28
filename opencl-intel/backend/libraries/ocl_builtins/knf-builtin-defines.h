// Copyright (c) 2006-2012 Intel Corporation
//


#pragma once

#ifdef __cplusplus
extern "C" {
#endif

const int const_char_msb = 0x80;
const int const_short_msb = 0x8000;
const int const_mask_0xaaaa = 0xaaaa;
const int const_fp_mantissa  = 0x7fffff;
const int const_fp_exp = 0x7f800000;
const int const_fp_no_sign = 0x7fffffff;
const int const_msb = 0x80000000;
const long const_dp_exp = 0x7ff0000000000000L;
const long const_dp_no_sign = 0x7fffffffffffffffL;
const long const_long_mantissa = 0x000fffffffffffffL;
const long const_dp_msb = 0x8000000000000000L;
const int const_dp_exp_hi = 0x7ff00000;

const float const_inv_pi_180f = 57.295779513082320876798154814105f;
const double const_inv_pi_180 = 57.295779513082320876798154814105;
const float const_pi_180f = 0.017453292519943295769236907684883f;
const double const_pi_180 = 0.017453292519943295769236907684883;


#define new_cast(new_type, var) *(new_type *)(&var)

// Apply add reduce between element of i and i+1 (stride-1)
#define add_reduce1_ps(data) \
  data = _mm512_add_ps(data, _mm512_swizzle_ps(data, _MM_SWIZ_REG_CDAB))

#define add_reduce1_pd(data) \
  data = _mm512_add_pd(data, _mm512_swizzle_pd(data, _MM_SWIZ_REG_CDAB))

// Apply add reduce between element of (i, i+1) and (i+2, i+3) (stride-2)
#define add_reduce2_ps(data) \
  data = _mm512_add_ps(data, _mm512_swizzle_ps(data, _MM_SWIZ_REG_BADC))

#define add_reduce2_pd(data) \
  data = _mm512_add_pd(data, _mm512_swizzle_pd(data, _MM_SWIZ_REG_BADC))

// Data swizzling for a 3-element vector; swizzle from CDBA->(D)ACB
#define s120_s012_ps(data, zeros, k) \
  k = 0x7; \
  data = _mm512_mask_add_ps (data, k, zeros, _mm512_swizzle_ps (data, _MM_SWIZ_REG_DACB))

#define s120_s012_pd(data, zeros, k) \
  k = 0x7; \
  data = _mm512_mask_add_pd (data, k, zeros, _mm512_swizzle_pd (data, _MM_SWIZ_REG_DACB))


// Convert a 16-bit bit-mask into a 16 x 32-bit mask
int16 __attribute__((overloadable)) mask2pi(__mmask16 flags){
  __m512i true_cond = (__m512i)((int16)(0xFFFFFFFF));
  __m512i ret_cond = (__m512i)_mm512_setzero();
  ret_cond = _mm512_mask_or_epi32(ret_cond, flags, true_cond, true_cond);
  return as_int16(ret_cond);
}
// Convert an 8-bit bit-mask into an 8 x 64-bit mask
long8 __attribute__((overloadable)) mask2pq(__mmask16 flags){
  __m512i true_cond = (__m512i)((int16)(0xFFFFFFFF));
  __m512i ret_cond = (__m512i)_mm512_setzero();
  ret_cond = _mm512_mask_or_epi64(ret_cond, flags, true_cond, true_cond);
  return as_long8(ret_cond);
}

// Expand an 8x32-bit mask to an 8 x 64-bit mask
long8 __attribute__((overloadable)) mask_expand_lo(int16 source){
  __m512i true_cond = (__m512i)((int16)(0xFFFFFFFF));
  __m512i ret_cond = (__m512i)_mm512_setzero();
  __mmask16 mask = _mm512_cmpeq_epi32((__m512i)source, true_cond);
  ret_cond = _mm512_mask_or_epi64(ret_cond, mask, true_cond, true_cond);
  return new_cast(long8, ret_cond);
}

// Replace the lower 32-bit of an 8x64-bit vector with the upper 32-bit;
// FEDCBA9876543210 --> FFDDBB9977553311
int16 __attribute__((overloadable)) unpack_hi(long8 src){
  __m512i dest = (__m512i)(src);
  __mmask16 mask = 0x5555;
  dest = _mm512_mask_or_epi32(dest, mask, (__m512i)((int16)0), _mm512_swizzle_epi32(dest, _MM_SWIZ_REG_CDAB));
  return new_cast(int16, dest);
}

uint16 __attribute__((overloadable)) unpack_hi(ulong8 src){
  __m512i dest = (__m512i)(src);
  __mmask16 mask = 0x5555;
  dest = _mm512_mask_or_epi32(dest, mask, (__m512i)((int16)0), _mm512_swizzle_epi32(dest, _MM_SWIZ_REG_CDAB));
  return new_cast(uint16, dest);
}

float16 __attribute__((overloadable)) unpack_hi(double8 src){
  __m512i dest = (__m512i)(src);
  __mmask16 mask = 0x5555;
  dest = _mm512_mask_or_epi32(dest, mask, (__m512i)((int16)0), _mm512_swizzle_epi32(dest, _MM_SWIZ_REG_CDAB));
  return new_cast(float16, dest);
}

// Replace the upper 32-bit of an 8x64-bit vector with the lower 32-bit;
// FEDCBA9876543210 --> EECCAA8866442200
int16 __attribute__((overloadable)) unpack_lo(int16 src){
  __m512i dest = (__m512i)(src);
  __mmask16 mask = 0xaaaa;
  dest = _mm512_mask_or_epi32(dest, mask, (__m512i)((int16)0), _mm512_swizzle_epi32(dest, _MM_SWIZ_REG_CDAB));
  return new_cast(int16, dest);
}
long8 __attribute__((overloadable)) unpack_lo(long8 src){
  __m512i dest = (__m512i)(src);
  __mmask16 mask = 0xaaaa;
  dest = _mm512_mask_or_epi32(dest, mask, (__m512i)((int16)0), _mm512_swizzle_epi32(dest, _MM_SWIZ_REG_CDAB));
  return new_cast(long8, dest);
}
ulong8 __attribute__((overloadable)) unpack_lo(ulong8 src){
  __m512i dest = (__m512i)(src);
  __mmask16 mask = 0xaaaa;
  dest = _mm512_mask_or_epi32(dest, mask, (__m512i)((int16)0), _mm512_swizzle_epi32(dest, _MM_SWIZ_REG_CDAB));
  return new_cast(ulong8, dest);
}
double8 __attribute__((overloadable)) unpack_lo(double8 src){
  __m512i dest = (__m512i)(src);
  __mmask16 mask = 0xaaaa;
  dest = _mm512_mask_or_epi32(dest, mask, (__m512i)((int16)0), _mm512_swizzle_epi32(dest, _MM_SWIZ_REG_CDAB));
  return new_cast(double8, dest);
}
// MIC vector truncation
char16 __attribute__((overloadable)) trunc_to_char16(int16 source){
  return convert_char16(source);
}
uchar16 __attribute__((overloadable)) trunc_to_uchar16(uint16 source){
  return convert_uchar16(source);
}
uchar16 __attribute__((overloadable)) trunc_to_uchar16(int16 source){
  return convert_uchar16(source);
}
short16 __attribute__((overloadable)) trunc_to_short16(int16 source){
  return convert_short16(source);
}
ushort16 __attribute__((overloadable)) trunc_to_ushort16(uint16 source){
  return convert_ushort16(source);
}
ushort16 __attribute__((overloadable)) trunc_to_ushort16(int16 source){
  return convert_ushort16(source);
}
int16 __attribute__((overloadable)) trunc_to_int16(int16 source){
  return source;
}
uint16 __attribute__((overloadable)) trunc_to_uint16(int16 source){
  return (uint16)source;
}
uint16 __attribute__((overloadable)) trunc_to_uint16(uint16 source){
  return source;
}
float16 __attribute__((overloadable)) trunc_to_float16(int16 source){
  return as_float16(source);
}
float16 __attribute__((overloadable)) trunc_to_float16(float16 source){
  return source;
}
long8 __attribute__((overloadable)) trunc_to_long8(int16 source){
  return as_long8(source);
}
long8 __attribute__((overloadable)) trunc_to_long8(long8 source){
  return source;
}
ulong8 __attribute__((overloadable)) trunc_to_ulong8(int16 source){
  return as_ulong8(source);
}
ulong8 __attribute__((overloadable)) trunc_to_ulong8(ulong8 source){
  return source;
}
double8 __attribute__((overloadable)) trunc_to_double8(int16 source){
  return as_double8(source);
}
double8 __attribute__((overloadable)) trunc_to_double8(double8 source){
  return source;
}

// MIC vector expansion
int16 __attribute__((overloadable)) ext_from(char16 source){
  return convert_int16(source);
}
uint16 __attribute__((overloadable)) ext_from(uchar16 source){
  return convert_uint16(source);
}
int16 __attribute__((overloadable)) ext_from(short16 source){
  return convert_int16(source);
}
uint16 __attribute__((overloadable)) ext_from(ushort16 source){
  return convert_uint16(source);
}
uint16 __attribute__((overloadable)) ext_from(uint16 source){
  return source;
}
int16 __attribute__((overloadable)) ext_from(int16 source){
  return source;
}
float16 __attribute__((overloadable)) ext_from(float16 source){
  return source;
}
ulong8 __attribute__((overloadable)) ext_from(ulong8 source){
  return source;
}
long8 __attribute__((overloadable)) ext_from(long8 source){
  return source;
}
double8 __attribute__((overloadable)) ext_from(double8 source){
  return source;
}
// MIC vector expansion (force sign extension)
int16 __attribute__((overloadable)) sign_ext_from(char16 source){
  __m512i data = _mm512_extload_epi32((void const *)(&source), _MM_UPCONV_EPI32_SINT8, _MM_BROADCAST32_NONE, _MM_HINT_NONE);
  return as_int16(data);
}
int16 __attribute__((overloadable)) sign_ext_from(uchar16 source){
  __m512i data = _mm512_extload_epi32((void const *)(&source), _MM_UPCONV_EPI32_SINT8, _MM_BROADCAST32_NONE, _MM_HINT_NONE);
  return as_int16(data);
}
int16 __attribute__((overloadable)) sign_ext_from(short16 source){
  __m512i data = _mm512_extload_epi32((void const *)(&source), _MM_UPCONV_EPI32_SINT16, _MM_BROADCAST32_NONE, _MM_HINT_NONE);
  return as_int16(data);
}
int16 __attribute__((overloadable)) sign_ext_from(ushort16 source){
  __m512i data = _mm512_extload_epi32((void const *)(&source), _MM_UPCONV_EPI32_SINT16, _MM_BROADCAST32_NONE, _MM_HINT_NONE);
  return as_int16(data);
}
int16 __attribute__((overloadable)) sign_ext_from(int16 source){
  return source;
}
int16 __attribute__((overloadable)) sign_ext_from(uint16 source){
  return as_int16(source);
}
int16 __attribute__((overloadable)) sign_ext_from(float16 source){
  return as_int16(source);
}
int16 __attribute__((overloadable)) sign_ext_from(long8 source){
  return new_cast(int16, source);
}
int16 __attribute__((overloadable)) sign_ext_from(ulong8 source){
  return new_cast(int16, source);
}
int16 __attribute__((overloadable)) sign_ext_from(double8 source){
  return new_cast(int16, source);
}

// Data swizzling for a 3-element vector; swizzle from CDBA --> (B)BAC
float16 __attribute__((overloadable)) s201_s012_ps(float16 in_data, __mmask16 k) {
  __m512 data = (__m512)in_data;
  __m512 zeros = _mm512_setzero_ps();
  k = 0xF;
  data = _mm512_mask_add_ps (data, k, zeros, _mm512_swizzle_ps (data, _MM_SWIZ_REG_BADC));
  k = 0x2;
  /* the s0 has been moved to s2 */
  data = _mm512_mask_add_ps (data, k, zeros, _mm512_swizzle_ps (data, _MM_SWIZ_REG_CCCC));
  k = 0x4;
  /* the s1 has been moved to s3 */
  data = _mm512_mask_add_ps (data, k, zeros, _mm512_swizzle_ps (data, _MM_SWIZ_REG_DDDD));
  return as_float16(data);
}
double8 __attribute__((overloadable)) s201_s012_pd(double8 in_data, __mmask16 k) {
  __m512d data = (__m512d)in_data;
  __m512d zeros = _mm512_setzero_pd();
  k = 0xF;
  data = _mm512_mask_add_pd (data, k, zeros, _mm512_swizzle_pd (data, _MM_SWIZ_REG_BADC));
  k = 0x2;
  /* the s0 has been moved to s2 */
  data = _mm512_mask_add_pd (data, k, zeros, _mm512_swizzle_pd (data, _MM_SWIZ_REG_CCCC));
  k = 0x4;
  /* the s1 has been moved to s3 */
  data = _mm512_mask_add_pd (data, k, zeros, _mm512_swizzle_pd (data, _MM_SWIZ_REG_DDDD));
  return as_double8(data);
}

// gather
int16 __attribute__((overloadable)) gather(int16 index, char16 x_reg){
  __m512i data = _mm512_i32extgather_epi32((__m512i)index, (void *)(&x_reg), _MM_UPCONV_EPI32_SINT8, _MM_SCALE_1, _MM_HINT_NONE);
  return as_int16(data);
}
uint16 __attribute__((overloadable)) gather(int16 index, uchar16 x_reg){
  __m512i data = _mm512_i32extgather_epi32((__m512i)index, (void *)(&x_reg), _MM_UPCONV_EPI32_UINT8, _MM_SCALE_1, _MM_HINT_NONE);
  return as_uint16(data);
}
int16 __attribute__((overloadable)) gather(int16 index, short16 x_reg){
  __m512i data = _mm512_i32extgather_epi32((__m512i)index, (void *)(&x_reg), _MM_UPCONV_EPI32_SINT16, _MM_SCALE_2, _MM_HINT_NONE);
  return as_int16(data);
}
uint16 __attribute__((overloadable)) gather(int16 index, ushort16 x_reg){
  __m512i data = _mm512_i32extgather_epi32((__m512i)index, (void *)(&x_reg), _MM_UPCONV_EPI32_UINT16, _MM_SCALE_2, _MM_HINT_NONE);
  return as_uint16(data);
}
int16 __attribute__((overloadable)) gather(int16 index, int16 x_reg){
  __m512i data = _mm512_i32extgather_epi32((__m512i)index, (void *)(&x_reg), _MM_UPCONV_EPI32_NONE, _MM_SCALE_4, _MM_HINT_NONE);
  return as_int16(data);
}
uint16 __attribute__((overloadable)) gather(int16 index, uint16 x_reg){
  __m512i data = _mm512_i32extgather_epi32((__m512i)index, (void *)(&x_reg), _MM_UPCONV_EPI32_NONE, _MM_SCALE_4, _MM_HINT_NONE);
  return as_uint16(data);
}
float16 __attribute__((overloadable)) gather(int16 index, float16 x_reg){
  __m512 data = _mm512_i32extgather_ps((__m512i)index, (void *)(&x_reg), _MM_UPCONV_PS_NONE, _MM_SCALE_4, _MM_HINT_NONE);
  return as_float16(data);
}
long8 __attribute__((overloadable)) gather(int16 index, long8 x_reg){
  __m512i data = _mm512_i32loextgather_epi64((__m512i)index, (void *)(&x_reg), _MM_UPCONV_EPI64_NONE, _MM_SCALE_8, _MM_HINT_NONE);
  return as_long8(data);
}
ulong8 __attribute__((overloadable)) gather(int16 index, ulong8 x_reg){
  __m512i data = _mm512_i32loextgather_epi64((__m512i)index, (void *)(&x_reg), _MM_UPCONV_EPI64_NONE, _MM_SCALE_8, _MM_HINT_NONE);
  return as_ulong8(data);
}
double8 __attribute__((overloadable)) gather(int16 index, double8 x_reg){
  __m512d data = _mm512_i32loextgather_pd((__m512i)index, (void *)(&x_reg), _MM_UPCONV_PD_NONE, _MM_SCALE_8, _MM_HINT_NONE);
  return as_double8(data);
}

// shuffle: source vector is 8x long
long8 __attribute__((overloadable)) shuffle_long_x8(long8 source, ulong8 input_index, ulong8 index_mask){
    __m512i index = _mm512_and_epi32((__m512i)input_index, (__m512i)index_mask);
    int16 const_vector_unpack = (int16)(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
    __m512i const_vector_0_1_x8 = (__m512i)((int16)(0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1));
    // Gather mask (mask) and transform it so each index i is converted to "2i" and "2i+1"
    // Gather data from source (xx) with new index
    int16 unpack_index = gather(const_vector_unpack, (int16)index);
    index = (__m512i)unpack_index;
    // index i -> 2i, 2i+1
    index = _mm512_sll_epi32(index, (__m512i)((int16)1));
    index = _mm512_add_epi32(index, const_vector_0_1_x8);
    long8 result = (long8)gather((int16)index, (int16)source);
    return result;
}
ulong8 __attribute__((overloadable)) shuffle_long_x8(ulong8 source, ulong8 input_index, ulong8 index_mask){
    __m512i index = _mm512_and_epi32((__m512i)input_index, (__m512i)index_mask);
    int16 const_vector_unpack = (int16)(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
    __m512i const_vector_0_1_x8 = (__m512i)((int16)(0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1));
    // Gather mask (mask) and transform it so each index i is converted to "2i" and "2i+1"
    // Gather data from source (xx) with new index
    int16 unpack_index = gather(const_vector_unpack, (int16)index);
    index = (__m512i)unpack_index;
    // index i -> 2i, 2i+1
    index = _mm512_sll_epi32(index, (__m512i)((int16)1));
    index = _mm512_add_epi32(index, const_vector_0_1_x8);
    ulong8 result = (ulong8)gather((int16)index, (uint16)source);
    return result;
}
double8 __attribute__((overloadable)) shuffle_long_x8(double8 source, ulong8 input_index, ulong8 index_mask){
    __m512i index = _mm512_and_epi32((__m512i)input_index, (__m512i)index_mask);
    int16 const_vector_unpack = (int16)(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
    __m512i const_vector_0_1_x8 = (__m512i)((int16)(0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1));
    // Gather mask (mask) and transform it so each index i is converted to "2i" and "2i+1"
    // Gather data from source (xx) with new index
    int16 unpack_index = gather(const_vector_unpack, (int16)index);
    index = (__m512i)unpack_index;
    // index i -> 2i, 2i+1
    index = _mm512_sll_epi32(index, (__m512i)((int16)1));
    index = _mm512_add_epi32(index, const_vector_0_1_x8);
    double8 result = (double8)gather((int16)index, (int16)source);
    return result;
}

// shuffle: source is 16x long
long8 __attribute__((overloadable)) shuffle_long_x16(long16 source, ulong8 input_index, ulong8 index_mask){
    __m512i index = _mm512_and_epi32((__m512i)input_index, (__m512i)index_mask);
    int16 const_vector_unpack = (int16)(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
    __m512i const_vector_0_1_x8 = (__m512i)((int16)(0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1));
    __m512i half_index_threshold = (__m512i)((int16)8);
    // Gather mask (mask) and transform it so each index i is converted to "2i" and "2i+1"
    // Gather data from source (xx) with new index
    int16 unpack_index = gather(const_vector_unpack, (int16)index);
    index = (__m512i)(unpack_index);
    __mmask16 upper_mask = _mm512_cmpnlt_epi32(index, half_index_threshold);
    index = _mm512_mask_sub_epi32(index, upper_mask, index, half_index_threshold);
    //index = _mm512_and_epi32(index, (__m512i)index_mask);
    index = _mm512_sll_epi32(index, (__m512i)((int16)1));
    index = _mm512_add_epi32(index, const_vector_0_1_x8);

    long8 upper_data = source.hi;
    long8 lower_data = source.lo;
    upper_data = (long8)gather((int16)index, (int16)upper_data);
    lower_data = (long8)gather((int16)index, (int16)lower_data);
    __m512i result = (__m512i)(lower_data);
    result = _mm512_mask_or_epi32(result, upper_mask, (__m512i)upper_data, (__m512i)upper_data);
    return (long8)result;
}
ulong8 __attribute__((overloadable)) shuffle_long_x16(ulong16 source, ulong8 input_index, ulong8 index_mask){
    __m512i index = _mm512_and_epi32((__m512i)input_index, (__m512i)index_mask);
    int16 const_vector_unpack = (int16)(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
    __m512i const_vector_0_1_x8 = (__m512i)((int16)(0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1));
    __m512i half_index_threshold = (__m512i)((int16)8);
    // Gather mask (mask) and transform it so each index i is converted to "2i" and "2i+1"
    // Gather data from source (xx) with new index
    int16 unpack_index = gather(const_vector_unpack, (int16)index);
    index = (__m512i)(unpack_index);
    __mmask16 upper_mask = _mm512_cmpnlt_epi32(index, half_index_threshold);
    index = _mm512_mask_sub_epi32(index, upper_mask, index, half_index_threshold);
    //index = _mm512_and_epi32(index, (__m512i)index_mask);
    index = _mm512_sll_epi32(index, (__m512i)((int16)1));
    index = _mm512_add_epi32(index, const_vector_0_1_x8);

    ulong8 upper_data = source.hi;
    ulong8 lower_data = source.lo;
    upper_data = (ulong8)gather((int16)index, (uint16)upper_data);
    lower_data = (ulong8)gather((int16)index, (uint16)lower_data);
    __m512i result = (__m512i)(lower_data);
    result = _mm512_mask_or_epi32(result, upper_mask, (__m512i)upper_data, (__m512i)upper_data);
    return (ulong8)result;
}
double8 __attribute__((overloadable)) shuffle_long_x16(double16 source, ulong8 input_index, ulong8 index_mask){
    __m512i index = _mm512_and_epi32((__m512i)input_index, (__m512i)index_mask);
    int16 const_vector_unpack = (int16)(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
    __m512i const_vector_0_1_x8 = (__m512i)((int16)(0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1));
    __m512i half_index_threshold = (__m512i)((int16)8);
    // Gather mask (mask) and transform it so each index i is converted to "2i" and "2i+1"
    // Gather data from source (xx) with new index
    int16 unpack_index = gather(const_vector_unpack, (int16)index);
    index = (__m512i)(unpack_index);
    __mmask16 upper_mask = _mm512_cmpnlt_epi32(index, half_index_threshold);
    index = _mm512_mask_sub_epi32(index, upper_mask, index, half_index_threshold);
    //index = _mm512_and_epi32(index, (__m512i)index_mask);
    index = _mm512_sll_epi32(index, (__m512i)((int16)1));
    index = _mm512_add_epi32(index, const_vector_0_1_x8);

    double8 upper_data = source.hi;
    double8 lower_data = source.lo;
    upper_data = (double8)gather((int16)index, (int16)upper_data);
    lower_data = (double8)gather((int16)index, (int16)lower_data);
    __m512i result = (__m512i)(lower_data);
    result = _mm512_mask_or_epi32(result, upper_mask, (__m512i)upper_data, (__m512i)upper_data);
    return (double8)result;
}
// shuffle2 (char/uchar/short/ushort/int/uint/float
char16 __attribute__((overloadable)) shuffle2_x16(char16 x, char16 y, uchar16 input_index, int vec_size){
    uint16 ext_index = ext_from(input_index);
    int16 index_mask = (int16)(2*vec_size - 1);
    __m512i index = _mm512_and_epi32((__m512i)index_mask, (__m512i)ext_index);
    __m512i index_threshold = (__m512i)((int16)vec_size);

    __mmask16 upper = _mm512_cmpnlt_epi32(index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper, index, index_threshold);
    int16 data1 = gather((int16)index, x);
    int16 data2 = gather((int16)index, y);
    __m512i data = (__m512i)data1;
    data = _mm512_mask_or_epi32(data, upper, (__m512i)data2, (__m512i)data2);
    char16 result = trunc_to_char16((int16)data);
    return result;
}
uchar16 __attribute__((overloadable)) shuffle2_x16(uchar16 x, uchar16 y, uchar16 input_index, int vec_size){
    uint16 ext_index = ext_from(input_index);
    int16 index_mask = (int16)(2*vec_size - 1);
    __m512i index = _mm512_and_epi32((__m512i)index_mask, (__m512i)ext_index);
    __m512i index_threshold = (__m512i)((int16)vec_size);

    __mmask16 upper = _mm512_cmpnlt_epi32(index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper, index, index_threshold);
    uint16 data1 = gather((int16)index, x);
    uint16 data2 = gather((int16)index, y);
    __m512i data = (__m512i)data1;
    data = _mm512_mask_or_epi32(data, upper, (__m512i)data2, (__m512i)data2);
    uchar16 result = trunc_to_uchar16((uint16)data);
    return result;
}
short16 __attribute__((overloadable)) shuffle2_x16(short16 x, short16 y, ushort16 input_index, int vec_size){
    uint16 ext_index = ext_from(input_index);
    int16 index_mask = (int16)(2*vec_size - 1);
    __m512i index = _mm512_and_epi32((__m512i)index_mask, (__m512i)ext_index);
    __m512i index_threshold = (__m512i)((int16)vec_size);

    __mmask16 upper = _mm512_cmpnlt_epi32(index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper, index, index_threshold);
    int16 data1 = gather((int16)index, x);
    int16 data2 = gather((int16)index, y);
    __m512i data = (__m512i)data1;
    data = _mm512_mask_or_epi32(data, upper, (__m512i)data2, (__m512i)data2);
    short16 result = trunc_to_short16((int16)data);
    return result;
}
ushort16 __attribute__((overloadable)) shuffle2_x16(ushort16 x, ushort16 y, ushort16 input_index, int vec_size){
    uint16 ext_index = ext_from(input_index);
    int16 index_mask = (int16)(2*vec_size - 1);
    __m512i index = _mm512_and_epi32((__m512i)index_mask, (__m512i)ext_index);
    __m512i index_threshold = (__m512i)((int16)vec_size);

    __mmask16 upper = _mm512_cmpnlt_epi32(index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper, index, index_threshold);
    uint16 data1 = gather((int16)index, x);
    uint16 data2 = gather((int16)index, y);
    __m512i data = (__m512i)data1;
    data = _mm512_mask_or_epi32(data, upper, (__m512i)data2, (__m512i)data2);
    ushort16 result = trunc_to_ushort16((uint16)data);
    return result;
}
int16 __attribute__((overloadable)) shuffle2_x16(int16 x, int16 y, uint16 input_index, int vec_size){
    uint16 ext_index = ext_from(input_index);
    int16 index_mask = (int16)(2*vec_size - 1);
    __m512i index = _mm512_and_epi32((__m512i)index_mask, (__m512i)ext_index);
    __m512i index_threshold = (__m512i)((int16)vec_size);

    __mmask16 upper = _mm512_cmpnlt_epi32(index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper, index, index_threshold);
    int16 data1 = gather((int16)index, x);
    int16 data2 = gather((int16)index, y);
    __m512i result = (__m512i)data1;
    result = _mm512_mask_or_epi32(result, upper, (__m512i)data2, (__m512i)data2);
    return (int16)result;
}
uint16 __attribute__((overloadable)) shuffle2_x16(uint16 x, uint16 y, uint16 input_index, int vec_size){
    uint16 ext_index = ext_from(input_index);
    int16 index_mask = (int16)(2*vec_size - 1);
    __m512i index = _mm512_and_epi32((__m512i)index_mask, (__m512i)ext_index);
    __m512i index_threshold = (__m512i)((int16)vec_size);

    __mmask16 upper = _mm512_cmpnlt_epi32(index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper, index, index_threshold);
    uint16 data1 = gather((int16)index, x);
    uint16 data2 = gather((int16)index, y);
    __m512i result = (__m512i)data1;
    result = _mm512_mask_or_epi32(result, upper, (__m512i)data2, (__m512i)data2);
    return (uint16)result;
}
float16 __attribute__((overloadable)) shuffle2_x16(float16 x, float16 y, uint16 input_index, int vec_size){
    uint16 ext_index = ext_from(input_index);
    int16 index_mask = (int16)(2*vec_size - 1);
    __m512i index = _mm512_and_epi32((__m512i)index_mask, (__m512i)ext_index);
    __m512i index_threshold = (__m512i)((int16)vec_size);

    __mmask16 upper = _mm512_cmpnlt_epi32(index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper, index, index_threshold);
    float16 data1 = gather((int16)index, x);
    float16 data2 = gather((int16)index, y);
    __m512i result = (__m512i)data1;
    result = _mm512_mask_or_epi32(result, upper, (__m512i)data2, (__m512i)data2);
    return (float16)result;
}
// shuffle2: source vector is 8x long
long8 __attribute__((overloadable)) shuffle2_long_x8(long8 x, long8 y, ulong8 input_index, int vec_size){
    ulong8 index_mask = (ulong8)(2*vec_size - 1);
    __m512i index = _mm512_and_epi32((__m512i)index_mask, (__m512i)input_index);

    int16 const_vector_unpack = (int16)(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
    __m512i const_vector_0_1_x8 = (__m512i)((int16)( 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1));
    // Gather mask (mask) and transform it so each index i is converted to "2i" and "2i+1"
    // Gather data from source (x and y) with new index
    int16 ext_index = gather(const_vector_unpack, (int16)index);
    index = (__m512i)ext_index;
    __m512i index_threshold = (__m512i)((int16)vec_size);
    __mmask16 upper = _mm512_cmpnlt_epi32(index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper, index, index_threshold);

    index = _mm512_sll_epi32(index, (__m512i)((int16)1));
    index = _mm512_add_epi32(index, const_vector_0_1_x8);

    long8 data1 = (long8)gather((int16)index, (int16)x);
    long8 data2 = (long8)gather((int16)index, (int16)y);
    __m512i result = (__m512i)data1;
    result = _mm512_mask_or_epi32(result, upper, (__m512i)data2, (__m512i)data2);
    return (long8)result;
}
ulong8 __attribute__((overloadable)) shuffle2_long_x8(ulong8 x, ulong8 y, ulong8 input_index, int vec_size){
    ulong8 index_mask = (ulong8)(2*vec_size - 1);
    __m512i index = _mm512_and_epi32((__m512i)index_mask, (__m512i)input_index);
    int16 const_vector_unpack = (int16)(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
    __m512i const_vector_0_1_x8 = (__m512i)((int16)( 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1));
    // Gather mask (mask) and transform it so each index i is converted to "2i" and "2i+1"
    // Gather data from source (x and y) with new index
    int16 ext_index = gather(const_vector_unpack, (int16)index);
    index = (__m512i)ext_index;
    __m512i index_threshold = (__m512i)((int16)vec_size);
    __mmask16 upper = _mm512_cmpnlt_epi32(index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper, index, index_threshold);

    index = _mm512_sll_epi32(index, (__m512i)((int16)1));
    index = _mm512_add_epi32(index, const_vector_0_1_x8);

    ulong8 data1 = (ulong8)gather((int16)index, (uint16)x);
    ulong8 data2 = (ulong8)gather((int16)index, (uint16)y);
    __m512i result = (__m512i)data1;
    result = _mm512_mask_or_epi32(result, upper, (__m512i)data2, (__m512i)data2);
    return (ulong8)result;
}
double8 __attribute__((overloadable)) shuffle2_long_x8(double8 x, double8 y, ulong8 input_index, int vec_size){
    ulong8 index_mask = (ulong8)(2*vec_size - 1);
    __m512i index = _mm512_and_epi32((__m512i)index_mask, (__m512i)input_index);
    int16 const_vector_unpack = (int16)(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
    __m512i const_vector_0_1_x8 = (__m512i)((int16)( 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1));
    // Gather mask (mask) and transform it so each index i is converted to "2i" and "2i+1"
    // Gather data from source (x and y) with new index
    int16 ext_index = gather(const_vector_unpack, (int16)index);
    index = (__m512i)ext_index;
    __m512i index_threshold = (__m512i)((int16)vec_size);
    __mmask16 upper = _mm512_cmpnlt_epi32(index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper, index, index_threshold);

    index = _mm512_sll_epi32(index, (__m512i)((int16)1));
    index = _mm512_add_epi32(index, const_vector_0_1_x8);

    double8 data1 = (double8)gather((int16)index, (int16)x);
    double8 data2 = (double8)gather((int16)index, (int16)y);
    __m512i result = (__m512i)data1;
    result = _mm512_mask_or_epi32(result, upper, (__m512i)data2, (__m512i)data2);
    return (double8)result;
}

// shuffle2: source vector is 16x long
long8 __attribute__((overloadable)) shuffle2_long_x16(long16 x, long16 y, ulong8 input_index, ulong8 index_mask){
    __m512i index = _mm512_and_epi32((__m512i)input_index, (__m512i)index_mask);
    __m512i index_threshold = (__m512i)((int16)16);
    int16 const_vector_unpack = (int16)(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
    __m512i const_vector_0_1_x8 = (__m512i)((int16)( 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1));
    // Gather mask (mask) and transform it so each index i is converted to "2i" and "2i+1"
    // Gather data from source (xx) with new index
    int16 ext_index = gather(const_vector_unpack, (int16)index);
    index = (__m512i)ext_index;
    index = _mm512_sll_epi32(index, (__m512i)((int16)1));
    index = _mm512_add_epi32(index, const_vector_0_1_x8);
    __mmask16 upper16 = _mm512_cmpnlt_epi32(index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper16, index, index_threshold);
    __mmask16 upper32 = _mm512_mask_cmpnlt_epi32(upper16, index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper32, index, index_threshold);
    __mmask16 upper48 = _mm512_mask_cmpnlt_epi32(upper32, index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper48, index, index_threshold);
    long8 data1 = (long8)gather((int16)index, (int16)x.lo);
    long8 data2 = (long8)gather((int16)index, (int16)x.hi);
    long8 data3 = (long8)gather((int16)index, (int16)y.lo);
    long8 data4 = (long8)gather((int16)index, (int16)y.hi);
    __m512i result = (__m512i)data1;
    result = _mm512_mask_or_epi32(result, upper16, (__m512i)data2, (__m512i)data2);
    result = _mm512_mask_or_epi32(result, upper32, (__m512i)data3, (__m512i)data3);
    result = _mm512_mask_or_epi32(result, upper48, (__m512i)data4, (__m512i)data4);
    return (long8)result;
}
ulong8 __attribute__((overloadable)) shuffle2_long_x16(ulong16 x, ulong16 y, ulong8 input_index, ulong8 index_mask){
    __m512i index = _mm512_and_epi32((__m512i)input_index, (__m512i)index_mask);
    __m512i index_threshold = (__m512i)((int16)16);
    int16 const_vector_unpack = (int16)(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
    __m512i const_vector_0_1_x8 = (__m512i)((int16)( 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1));
    // Gather mask (mask) and transform it so each index i is converted to "2i" and "2i+1"
    // Gather data from source (xx) with new index
    int16 ext_index = gather(const_vector_unpack, (int16)index);
    index = (__m512i)ext_index;
    index = _mm512_sll_epi32(index, (__m512i)((int16)1));
    index = _mm512_add_epi32(index, const_vector_0_1_x8);
    __mmask16 upper16 = _mm512_cmpnlt_epi32(index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper16, index, index_threshold);
    __mmask16 upper32 = _mm512_mask_cmpnlt_epi32(upper16, index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper32, index, index_threshold);
    __mmask16 upper48 = _mm512_mask_cmpnlt_epi32(upper32, index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper48, index, index_threshold);
    ulong8 data1 = (ulong8)gather((int16)index, (uint16)x.lo);
    ulong8 data2 = (ulong8)gather((int16)index, (uint16)x.hi);
    ulong8 data3 = (ulong8)gather((int16)index, (uint16)y.lo);
    ulong8 data4 = (ulong8)gather((int16)index, (uint16)y.hi);
    __m512i result = (__m512i)data1;
    result = _mm512_mask_or_epi32(result, upper16, (__m512i)data2, (__m512i)data2);
    result = _mm512_mask_or_epi32(result, upper32, (__m512i)data3, (__m512i)data3);
    result = _mm512_mask_or_epi32(result, upper48, (__m512i)data4, (__m512i)data4);
    return (ulong8)result;
}
double8 __attribute__((overloadable)) shuffle2_long_x16(double16 x, double16 y, ulong8 input_index, ulong8 index_mask){
    __m512i index = _mm512_and_epi32((__m512i)input_index, (__m512i)index_mask);
    __m512i index_threshold = (__m512i)((int16)16);
    int16 const_vector_unpack = (int16)(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
    __m512i const_vector_0_1_x8 = (__m512i)((int16)( 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1));
    // Gather mask (mask) and transform it so each index i is converted to "2i" and "2i+1"
    // Gather data from source (xx) with new index
    int16 ext_index = gather(const_vector_unpack, (int16)index);
    index = (__m512i)ext_index;
    index = _mm512_sll_epi32(index, (__m512i)((int16)1));
    index = _mm512_add_epi32(index, const_vector_0_1_x8);
    __mmask16 upper16 = _mm512_cmpnlt_epi32(index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper16, index, index_threshold);
    __mmask16 upper32 = _mm512_mask_cmpnlt_epi32(upper16, index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper32, index, index_threshold);
    __mmask16 upper48 = _mm512_mask_cmpnlt_epi32(upper32, index, index_threshold);
    index = _mm512_mask_sub_epi32(index, upper48, index, index_threshold);
    double8 data1 = (double8)gather((int16)index, (int16)x.lo);
    double8 data2 = (double8)gather((int16)index, (int16)x.hi);
    double8 data3 = (double8)gather((int16)index, (int16)y.lo);
    double8 data4 = (double8)gather((int16)index, (int16)y.hi);
    __m512i result = (__m512i)data1;
    result = _mm512_mask_or_epi32(result, upper16, (__m512i)data2, (__m512i)data2);
    result = _mm512_mask_or_epi32(result, upper32, (__m512i)data3, (__m512i)data3);
    result = _mm512_mask_or_epi32(result, upper48, (__m512i)data4, (__m512i)data4);
    return (double8)result;
}

// 0a0b0c0d0e0f0g0h -> 00000000abcdefgh or
// a0b0c0d0e0f0g0h0 -> 00000000abcdefgh
//
uchar __attribute__((__always_inline__, overloadable))
_mask16z8(ushort m16)
{
    uint r = _mm_bitinterleave11_32(m16, m16); // 0a0b ->  00aa00bb or a0b0 -> aa00bb00
    uint hi = _mm_quadmask16_32(r >> 16);
    uint lo = _mm_quadmask16_32(r);
    return (hi << 4) | lo;
}

// 00000000abcdefgh -> 0a0b0c0d0e0f0g0h
ushort __attribute__((__always_inline__, overloadable))
_mask8z16e(uchar m8) // even
{
    return _mm_bitinterleave11_16(0, m8);
}

// 00000000abcdefgh -> a0b0c0d0e0f0g0h0
ushort __attribute__((__always_inline__, overloadable))
_mask8z16o(uchar m8) // odd
{
    return _mm_bitinterleave11_16(m8, 0);
}

// x < 0
uchar __attribute__((__always_inline__, overloadable))
_cmplt_zero(long8 x)
{
    ushort m16 = _mm512_mask_cmplt_epi32(0x55, _mm512_swizzle_epi32(x, _MM_SWIZ_REG_CDAB), (__m512i)(int16)0);
    return _mask16z8(m16);
}

// x > 0
uchar __attribute__((__always_inline__, overloadable))
_cmpgt_zero(long8 x)
{
    ushort m16 = _mm512_mask_cmplt_epi32(0x55, (__m512i)(int16)0, _mm512_swizzle_epi32(x, _MM_SWIZ_REG_CDAB));
    return _mask16z8(m16);
}

// addsetc
long8 __attribute__((__always_inline__, overloadable))
_addsetc(long8 x, long8 y, uchar *cout)
{
    ushort c16 = 0;
    __m512i r = _mm512_mask_addsetc_epi32((__m512i)x, 0x5555, c16, y, &c16);
    r = _mm512_mask_adc_epi32(r, 0xAAAA, c16 << 1, y, &c16);
    *cout = _mask16z8(c16);
    return as_long8(r);
}
ulong8 __attribute__((__always_inline__, overloadable))
_addsetc(ulong8 x, ulong8 y, uchar *cout)
{
    return as_ulong8(_addsetc(as_long8(x), as_long8(y), cout));
}

// adc
long8 __attribute__((__always_inline__, overloadable))
_adc(long8 x, uchar cin, long8 y, uchar *cout)
{
    ushort c16 = _mask8z16e(cin);
    __m512i r = _mm512_mask_adc_epi32((__m512i)x, 0x5555, c16, (__m512i)y, &c16);
    r = _mm512_mask_adc_epi32(r, 0xAAAA, c16 << 1, (__m512i)y, &c16);
    *cout = _mask16z8(c16);
    return as_long8(r);
}
ulong8 __attribute__((__always_inline__, overloadable))
_adc(ulong8 x, uchar cin, ulong8 y, uchar *cout)
{
    return as_ulong8(_adc(as_long8(x), cin, as_long8(y), cout));
}

// soa length help utilities
// length2
float8  __attribute__((__always_inline__, overloadable))
length2_up_convert(float8 x, float8 y)
{
  float16 x_reg;
  float16 y_reg;
  x_reg.lo = x;
  y_reg.lo = y;

  double8 x_dp = as_double8(_mm512_cvtpslo_pd( x_reg));
  double8 y_dp = as_double8(_mm512_cvtpslo_pd( y_reg));
  double8 sum = x_dp * x_dp + y_dp *y_dp;
  sum = sqrt(sum);
  __m512 res = _mm512_cvt_roundpd_pslo(sum, _MM_FROUND_CUR_DIRECTION);
  return as_float16(res).lo;
}

float16  __attribute__((__always_inline__, overloadable))
length2_up_convert(float16 x, float16 y)
{
  float16 res;
  res.lo = length2_up_convert(x.lo, y.lo);
  res.hi = length2_up_convert(x.hi, y.hi);
  return res;
}

float4  __attribute__((__always_inline__, overloadable))
length2_up_convert(float4 x, float4 y)
{
  float8 x_reg; x_reg.lo = x;
  float8 y_reg; y_reg.lo = y;
  float8 res = length2_up_convert(x_reg, y_reg);
  return res.lo;
}

// length3
float8  __attribute__((__always_inline__, overloadable))
length3_up_convert(float8 x, float8 y, float8 z)
{
  float16 x_reg;
  float16 y_reg;
  float16 z_reg;
  x_reg.lo = x;
  y_reg.lo = y;
  z_reg.lo = z;

  double8 x_dp = as_double8(_mm512_cvtpslo_pd( x_reg));
  double8 y_dp = as_double8(_mm512_cvtpslo_pd( y_reg));
  double8 z_dp = as_double8(_mm512_cvtpslo_pd( z_reg));
  double8 sum = x_dp * x_dp + y_dp *y_dp + z_dp *z_dp;
  sum = sqrt(sum);
  __m512 res = _mm512_cvt_roundpd_pslo(sum, _MM_FROUND_CUR_DIRECTION);
  return as_float16(res).lo;
}

float4  __attribute__((__always_inline__, overloadable))
length3_up_convert(float4 x, float4 y, float4 z)
{
  float8 x_reg; x_reg.lo = x;
  float8 y_reg; y_reg.lo = y;
  float8 z_reg; z_reg.lo = z;
  float8 res = length3_up_convert(x_reg, y_reg, z_reg);
  return res.lo;
}

float16  __attribute__((__always_inline__, overloadable))
length3_up_convert(float16 x, float16 y, float16 z)
{
  float16 res;
  res.lo = length3_up_convert(x.lo, y.lo, z.lo);
  res.hi = length3_up_convert(x.hi, y.hi, z.hi);
  return res;
}

// length4 conversion
float8  __attribute__((__always_inline__, overloadable))
length4_up_convert(float8 x, float8 y, float8 z, float8 w)
{
  float16 x_reg;
  float16 y_reg;
  float16 z_reg;
  float16 w_reg;

  x_reg.lo = x;
  y_reg.lo = y;
  z_reg.lo = z;
  w_reg.lo = w;

  double8 x_dp = as_double8(_mm512_cvtpslo_pd( x_reg));
  double8 y_dp = as_double8(_mm512_cvtpslo_pd( y_reg));
  double8 z_dp = as_double8(_mm512_cvtpslo_pd( z_reg));
  double8 w_dp = as_double8(_mm512_cvtpslo_pd( w_reg));
  double8 sum = x_dp * x_dp + y_dp *y_dp + z_dp * z_dp + w_dp *w_dp;
  sum = sqrt(sum);
  __m512 res = _mm512_cvt_roundpd_pslo(sum, _MM_FROUND_CUR_DIRECTION);
  return as_float16(res).lo;
}

float16  __attribute__((__always_inline__, overloadable))
length4_up_convert(float16 x, float16 y, float16 z, float16 w)
{
  float16 res;
  res.lo = length4_up_convert(x.lo, y.lo, z.lo, w.lo);
  res.hi = length4_up_convert(x.hi, y.hi, z.hi, w.hi);
  return res;
}

float4  __attribute__((__always_inline__, overloadable))
length4_up_convert(float4 x, float4 y, float4 z, float4 w)
{
  float8 x_reg; x_reg.lo = x;
  float8 y_reg; y_reg.lo = y;
  float8 z_reg; z_reg.lo = z;
  float8 w_reg; w_reg.lo = w;
  float8 res = length4_up_convert(x_reg, y_reg, z_reg, w_reg);
  return res.lo;
}

// Below atomic functions are replicated from CPU implementation
// as defined in atom_function.cpp. The only difference is that MIC
// implementation removes _mm_mfence() since there is equivalent native
// support due to its in-order execution pipe. Any bug found here shall also trace
// back its origin in CPU implementation for certanity.
//#define VATOMICS_FUNC_DECL

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

typedef int intrin_type;

/**
 * Implementation NOTICE:
 *
 * This implementation assumes that local pool (workgroup threads) are executed by
 * only ONE phyiscal thread, this means that the operations in local memory will be
 * invoked in a serial way, thus there's no need to synchronize the access to the
 * local memory;
 *
 * In the following implementation there's a helper functions there are two versions
 * of this:
 *  one for global memory (with _global postfix)
 *  one for local  memory (with _local  postfix)
 */

// global calls implementations
int __inline__ __attribute__((always_inline)) atomicInc_global(volatile int* p)
{
  return __sync_fetch_and_add(p, 1);
}

int __inline__ __attribute__((always_inline)) atomicDec_global(volatile int* p)
{
  return __sync_fetch_and_sub(p, 1);
}

int __inline__ __attribute__((always_inline)) atomicAdd_global(volatile int* p, int val)
{
  return __sync_fetch_and_add(p, val);
}

int __inline__ __attribute__((always_inline)) atomicSub_global(volatile int* p, int val)
{
  return __sync_fetch_and_sub(p, val);
}

__inline__ __attribute__((always_inline)) int _InterlockedCompareExchange_global(volatile int * const Destination, const int Comperand, const int Exchange)
{
  return __sync_val_compare_and_swap(Destination, Comperand, Exchange);
}

__inline__ __attribute__((always_inline)) int InterlockedExchange_global(volatile int * const Target, const int Value)
{
  //_mm_mfence();
  return __sync_lock_test_and_set(Target, Value);
}

__inline__ __attribute__((always_inline)) int _InterlockedAnd_global(volatile int * const value, const int mask)
{
  return __sync_fetch_and_and(value, mask);
}

__inline__ __attribute__((always_inline)) int _InterlockedOr_global(volatile int * const value, const int mask)
{
  return __sync_fetch_and_or(value, mask);
}

__inline__ __attribute__((always_inline)) int _InterlockedXor_global(volatile int * const value, const int mask)
{
  return __sync_fetch_and_xor(value, mask);
}

// local calls implementations
int __inline__ __attribute__((always_inline)) atomicInc_local(volatile int* p)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    int oldValue = *p;
    *p = oldValue + 1;
    return oldValue;
}

int __inline__ __attribute__((always_inline)) atomicDec_local(volatile int* p)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    int oldValue = *p;
    *p = oldValue - 1;
    return oldValue;
}

int __inline__ __attribute__((always_inline)) atomicAdd_local(volatile int* p, int val)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    int oldValue = *p;
    *p = oldValue + val;
    return oldValue;
}

int __inline__ __attribute__((always_inline)) atomicSub_local(volatile int* p, int val)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    int oldValue = *p;
    *p = oldValue - val;
    return oldValue;
}

__inline__ __attribute__((always_inline)) int _InterlockedCompareExchange_local(volatile int * const Destination, const int Comperand, const int Exchange)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    int oldValue = *Destination;
    if (oldValue == Comperand)
    {
        *Destination = Exchange;
    }
    return oldValue;
}

__inline__ __attribute__((always_inline)) int InterlockedExchange_local(volatile int * const Target, const int Value)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    int oldValue = *Target;
    *Target = Value;
    return oldValue;
}

__inline__ __attribute__((always_inline)) int _InterlockedAnd_local(volatile int * const value, const int mask)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    int oldValue = *value;
    *value = oldValue & mask;
    return oldValue;
}

__inline__ __attribute__((always_inline)) int _InterlockedOr_local(volatile int * const value, const int mask)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    int oldValue = *value;
    *value = oldValue | mask;
    return oldValue;
}

__inline__ __attribute__((always_inline)) int _InterlockedXor_local(volatile int * const value, const int mask)
{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    int oldValue = *value;
    *value = oldValue ^ mask;
    return oldValue;
}

// helper min, max implementations (not syncronized)
int __inline__ __attribute__((always_inline)) unsigned_min(unsigned int p0, unsigned int p1)
{
    return (p0 < p1) ? p0 : p1;
}

int __inline__ __attribute__((always_inline)) unsigned_max(unsigned int p0, unsigned int p1)
{
    return (p0 > p1) ? p0 : p1;
}

int __inline__ __attribute__((always_inline)) signed_min(int p0, int p1)
{
    return (p0 < p1) ? p0 : p1;
}

int __inline__ __attribute__((always_inline)) signed_max(int p0, int p1)
{
    return (p0 > p1) ? p0 : p1;
}

#ifdef __cplusplus
}
#endif
