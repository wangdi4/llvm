//==----- imf_wrapper.cpp - wrappers for intel math library functions ------==//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "device_imf.hpp"

#ifdef __LIBDEVICE_IMF_ENABLED__

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_saturatef(float);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_half2float(_iml_half_internal);

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_float2int_rd(float);

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_float2int_rn(float);

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_float2int_ru(float);

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_float2int_rz(float);

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_float2uint_rd(float);

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_float2uint_rn(float);

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_float2uint_ru(float);

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_float2uint_rz(float);

DEVICE_EXTERN_C_INLINE
long long int __devicelib_imf_float2ll_rd(float);

DEVICE_EXTERN_C_INLINE
long long int __devicelib_imf_float2ll_rn(float);

DEVICE_EXTERN_C_INLINE
long long int __devicelib_imf_float2ll_ru(float);

DEVICE_EXTERN_C_INLINE
long long int __devicelib_imf_float2ll_rz(float);

DEVICE_EXTERN_C_INLINE
unsigned long long int __devicelib_imf_float2ull_rd(float);

DEVICE_EXTERN_C_INLINE
unsigned long long int __devicelib_imf_float2ull_rn(float);

DEVICE_EXTERN_C_INLINE
unsigned long long int __devicelib_imf_float2ull_ru(float);

DEVICE_EXTERN_C_INLINE
unsigned long long int __devicelib_imf_float2ull_rz(float);

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_float_as_int(float);

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_float_as_uint(float);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_int2float_rd(int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_int2float_rn(int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_int2float_ru(int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_int2float_rz(int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_int_as_float(int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_ll2float_rd(long long int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_ll2float_rn(long long int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_ll2float_ru(long long int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_ll2float_rz(long long int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_uint2float_rd(unsigned int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_uint2float_rn(unsigned int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_uint2float_ru(unsigned int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_uint2float_rz(unsigned int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_uint_as_float(unsigned int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_ull2float_rd(unsigned long long int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_ull2float_rn(unsigned long long int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_ull2float_ru(unsigned long long int);

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_ull2float_rz(unsigned long long int);

DEVICE_EXTERN_C_INLINE
float __imf_saturatef(float x) { return __devicelib_imf_saturatef(x); }

DEVICE_EXTERN_C_INLINE
int __imf_float2int_rd(float x) { return __devicelib_imf_float2int_rd(x); }

DEVICE_EXTERN_C_INLINE
int __imf_float2int_rn(float x) { return __devicelib_imf_float2int_rn(x); }

DEVICE_EXTERN_C_INLINE
int __imf_float2int_ru(float x) { return __devicelib_imf_float2int_ru(x); }

DEVICE_EXTERN_C_INLINE
int __imf_float2int_rz(float x) { return __devicelib_imf_float2int_rz(x); }

DEVICE_EXTERN_C_INLINE
unsigned int __imf_float2uint_rd(float x) {
  return __devicelib_imf_float2uint_rd(x);
}

DEVICE_EXTERN_C_INLINE
unsigned int __imf_float2uint_rn(float x) {
  return __devicelib_imf_float2uint_rn(x);
}

DEVICE_EXTERN_C_INLINE
unsigned int __imf_float2uint_ru(float x) {
  return __devicelib_imf_float2uint_ru(x);
}

DEVICE_EXTERN_C_INLINE
unsigned int __imf_float2uint_rz(float x) {
  return __devicelib_imf_float2uint_rz(x);
}

DEVICE_EXTERN_C_INLINE
long long int __imf_float2ll_rd(float x) {
  return __devicelib_imf_float2ll_rd(x);
}

DEVICE_EXTERN_C_INLINE
long long int __imf_float2ll_rn(float x) {
  return __devicelib_imf_float2ll_rn(x);
}

DEVICE_EXTERN_C_INLINE
long long int __imf_float2ll_ru(float x) {
  return __devicelib_imf_float2ll_ru(x);
}

DEVICE_EXTERN_C_INLINE
long long int __imf_float2ll_rz(float x) {
  return __devicelib_imf_float2ll_rz(x);
}

DEVICE_EXTERN_C_INLINE
unsigned long long int __imf_float2ull_rd(float x) {
  return __devicelib_imf_float2ull_rd(x);
}

DEVICE_EXTERN_C_INLINE
unsigned long long int __imf_float2ull_rn(float x) {
  return __devicelib_imf_float2ull_rn(x);
}

DEVICE_EXTERN_C_INLINE
unsigned long long int __imf_float2ull_ru(float x) {
  return __devicelib_imf_float2ull_ru(x);
}

DEVICE_EXTERN_C_INLINE
unsigned long long int __imf_float2ull_rz(float x) {
  return __devicelib_imf_float2ull_rz(x);
}

DEVICE_EXTERN_C_INLINE
int __imf_float_as_int(float x) { return __devicelib_imf_float_as_int(x); }

DEVICE_EXTERN_C_INLINE
unsigned int __imf_float_as_uint(float x) {
  return __devicelib_imf_float_as_uint(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_int2float_rd(int x) { return __devicelib_imf_int2float_rd(x); }

DEVICE_EXTERN_C_INLINE
float __imf_int2float_rn(int x) { return __devicelib_imf_int2float_rn(x); }

DEVICE_EXTERN_C_INLINE
float __imf_int2float_ru(int x) { return __devicelib_imf_int2float_ru(x); }

DEVICE_EXTERN_C_INLINE
float __imf_int2float_rz(int x) { return __devicelib_imf_int2float_rz(x); }

DEVICE_EXTERN_C_INLINE
float __imf_int_as_float(int x) { return __devicelib_imf_int_as_float(x); }

DEVICE_EXTERN_C_INLINE
float __imf_ll2float_rd(long long int x) {
  return __devicelib_imf_ll2float_rd(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_ll2float_rn(long long int x) {
  return __devicelib_imf_ll2float_rn(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_ll2float_ru(long long int x) {
  return __devicelib_imf_ll2float_ru(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_ll2float_rz(long long int x) {
  return __devicelib_imf_ll2float_rz(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_uint2float_rd(unsigned int x) {
  return __devicelib_imf_uint2float_rd(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_uint2float_rn(unsigned int x) {
  return __devicelib_imf_uint2float_rn(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_uint2float_ru(unsigned int x) {
  return __devicelib_imf_uint2float_ru(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_uint2float_rz(unsigned int x) {
  return __devicelib_imf_uint2float_rz(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_uint_as_float(unsigned int x) {
  return __devicelib_imf_uint_as_float(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_ull2float_rd(unsigned long long int x) {
  return __devicelib_imf_ull2float_rd(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_ull2float_rn(unsigned long long int x) {
  return __devicelib_imf_ull2float_rn(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_ull2float_ru(unsigned long long int x) {
  return __devicelib_imf_ull2float_ru(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_ull2float_rz(unsigned long long int x) {
  return __devicelib_imf_ull2float_rz(x);
}

DEVICE_EXTERN_C_INLINE
float __imf_half2float(_iml_half_internal x) {
  return __devicelib_imf_half2float(x);
}

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_brev(unsigned int);

DEVICE_EXTERN_C_INLINE
unsigned long long int __devicelib_imf_brevll(unsigned long long int);

DEVICE_EXTERN_C_INLINE
unsigned int __imf_brev(unsigned int x) { return __devicelib_imf_brev(x); }

DEVICE_EXTERN_C_INLINE
unsigned long long int __imf_brevll(unsigned long long int x) {
  return __devicelib_imf_brevll(x);
}

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_clz(int);

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_clzll(long long int);

DEVICE_EXTERN_C_INLINE
int __imf_clz(int x) { return __devicelib_imf_clz(x); }

DEVICE_EXTERN_C_INLINE
int __imf_clzll(long long int x) { return __devicelib_imf_clzll(x); }

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_popc(unsigned int);

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_popcll(unsigned long long int);

DEVICE_EXTERN_C_INLINE
int __imf_popc(unsigned int x) { return __devicelib_imf_popc(x); }

DEVICE_EXTERN_C_INLINE
int __imf_popcll(unsigned long long int x) { return __devicelib_imf_popcll(x); }

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_sad(int, int, unsigned int);

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_usad(unsigned int, unsigned int, unsigned int);

DEVICE_EXTERN_C_INLINE
unsigned int __imf_sad(int x, int y, unsigned int z) {
  return __devicelib_imf_sad(x, y, z);
}

DEVICE_EXTERN_C_INLINE
unsigned int __imf_usad(unsigned int x, unsigned int y, unsigned int z) {
  return __devicelib_imf_usad(x, y, z);
}

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_byte_perm(unsigned int, unsigned int,
                                       unsigned int);

DEVICE_EXTERN_C_INLINE
unsigned int __imf_byte_perm(unsigned int x, unsigned int y, unsigned int s) {
  return __devicelib_imf_byte_perm(x, y, s);
}

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_ffs(int);

DEVICE_EXTERN_C_INLINE
int __imf_ffs(int x) { return __devicelib_imf_ffs(x); }

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_ffsll(long long int);

DEVICE_EXTERN_C_INLINE
int __imf_ffsll(long long int x) { return __devicelib_imf_ffsll(x); }

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_rhadd(int, int);

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_uhadd(int, int);

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_urhadd(unsigned int, unsigned int);

DEVICE_EXTERN_C_INLINE
int __imf_rhadd(int x, int y) { return __devicelib_imf_rhadd(x, y); }

DEVICE_EXTERN_C_INLINE
unsigned int __imf_uhadd(unsigned int x, unsigned int y) {
  return __devicelib_imf_uhadd(x, y);
}

DEVICE_EXTERN_C_INLINE
unsigned int __imf_urhadd(unsigned int x, unsigned int y) {
  return __devicelib_imf_urhadd(x, y);
}

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_mul24(int, int);

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_umul24(unsigned int, unsigned int);

DEVICE_EXTERN_C_INLINE
int __imf_mul24(int x, int y) { return __devicelib_imf_mul24(x, y); }

DEVICE_EXTERN_C_INLINE
unsigned int __imf_umul24(unsigned int x, unsigned int y) {
  return __devicelib_imf_umul24(x, y);
}

DEVICE_EXTERN_C_INLINE
int __devicelib_imf_mulhi(int, int);

DEVICE_EXTERN_C_INLINE
unsigned int __devicelib_imf_umulhi(unsigned int, unsigned int);

DEVICE_EXTERN_C_INLINE
long long int __devicelib_imf_mul64hi(long long int, long long int);

DEVICE_EXTERN_C_INLINE
unsigned long long int __devicelib_imf_umul64hi(unsigned long long int,
                                                unsigned long long int);

DEVICE_EXTERN_C_INLINE
long long int __imf_mul64hi(long long int x, long long int y) {
  return __devicelib_imf_mul64hi(x, y);
}

DEVICE_EXTERN_C_INLINE
unsigned long long int __imf_umul64hi(unsigned long long int x,
                                      unsigned long long int y) {
  return __devicelib_imf_umul64hi(x, y);
}

DEVICE_EXTERN_C_INLINE
int __imf_mulhi(int x, int y) { return __devicelib_imf_mulhi(x, y); }

DEVICE_EXTERN_C_INLINE
unsigned int __imf_umulhi(unsigned int x, unsigned int y) {
  return __devicelib_imf_umulhi(x, y);
}

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_fmaf(float, float, float);

DEVICE_EXTERN_C_INLINE
float __imf_fmaf(float x, float y, float z) {
  return __devicelib_imf_fmaf(x, y, z);
}

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_floorf(float);

DEVICE_EXTERN_C_INLINE
float __imf_floorf(float x) { return __devicelib_imf_floorf(x); }

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_ceilf(float);

DEVICE_EXTERN_C_INLINE
float __imf_ceilf(float x) { return __devicelib_imf_ceilf(x); }

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_truncf(float);

DEVICE_EXTERN_C_INLINE
float __imf_truncf(float x) { return __devicelib_imf_truncf(x); }

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_rintf(float);

DEVICE_EXTERN_C_INLINE
float __imf_rintf(float x) { return __devicelib_imf_rintf(x); }

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_nearbyintf(float);

DEVICE_EXTERN_C_INLINE
float __imf_nearbyintf(float x) { return __devicelib_imf_nearbyintf(x); }

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_sqrtf(float);

DEVICE_EXTERN_C_INLINE
float __imf_sqrtf(float x) { return __devicelib_imf_sqrtf(x); }

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_rsqrtf(float);

DEVICE_EXTERN_C_INLINE
float __imf_rsqrtf(float x) { return __devicelib_imf_rsqrtf(x); }

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_invf(float);

DEVICE_EXTERN_C_INLINE
float __imf_invf(float x) { return __devicelib_imf_invf(x); }

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_fabsf(float);

DEVICE_EXTERN_C_INLINE
float __imf_fabsf(float x) { return __devicelib_imf_fabsf(x); }

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_fmaxf(float, float);

DEVICE_EXTERN_C_INLINE
float __imf_fmaxf(float x, float y) { return __devicelib_imf_fmaxf(x, y); }

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_fminf(float, float);

DEVICE_EXTERN_C_INLINE
float __imf_fminf(float x, float y) { return __devicelib_imf_fminf(x, y); }

DEVICE_EXTERN_C_INLINE
float __devicelib_imf_copysignf(float, float);

DEVICE_EXTERN_C_INLINE
float __imf_copysignf(float x, float y) {
  return __devicelib_imf_copysignf(x, y);
}

DEVICE_EXTERN_C_INLINE
_iml_half_internal __devicelib_imf_fmaf16(_iml_half_internal,
                                          _iml_half_internal,
                                          _iml_half_internal);

DEVICE_EXTERN_C_INLINE
_iml_half_internal __imf_fmaf16(_iml_half_internal x, _iml_half_internal y,
                                _iml_half_internal z) {
  return __devicelib_imf_fmaf16(x, y, z);
}

DEVICE_EXTERN_C_INLINE
_iml_half_internal __devicelib_imf_floorf16(_iml_half_internal);

DEVICE_EXTERN_C_INLINE
_iml_half_internal __imf_floorf16(_iml_half_internal x) {
  return __devicelib_imf_floorf16(x);
}

DEVICE_EXTERN_C_INLINE
_iml_half_internal __devicelib_imf_ceilf16(_iml_half_internal);

DEVICE_EXTERN_C_INLINE
_iml_half_internal __imf_ceilf16(_iml_half_internal x) {
  return __devicelib_imf_ceilf16(x);
}

DEVICE_EXTERN_C_INLINE
_iml_half_internal __devicelib_imf_truncf16(_iml_half_internal);

DEVICE_EXTERN_C_INLINE
_iml_half_internal __imf_truncf16(_iml_half_internal x) {
  return __devicelib_imf_truncf16(x);
}

DEVICE_EXTERN_C_INLINE
_iml_half_internal __devicelib_imf_rintf16(_iml_half_internal);

DEVICE_EXTERN_C_INLINE
_iml_half_internal __imf_rintf16(_iml_half_internal x) {
  return __devicelib_imf_rintf16(x);
}

DEVICE_EXTERN_C_INLINE
_iml_half_internal __devicelib_imf_nearbyintf16(_iml_half_internal);

DEVICE_EXTERN_C_INLINE
_iml_half_internal __imf_nearbyintf16(_iml_half_internal x) {
  return __devicelib_imf_nearbyintf16(x);
}

DEVICE_EXTERN_C_INLINE
_iml_half_internal __devicelib_imf_sqrtf16(_iml_half_internal);

DEVICE_EXTERN_C_INLINE
_iml_half_internal __imf_sqrtf16(_iml_half_internal x) {
  return __devicelib_imf_sqrtf16(x);
}

DEVICE_EXTERN_C_INLINE
_iml_half_internal __devicelib_imf_rsqrtf16(_iml_half_internal);

DEVICE_EXTERN_C_INLINE
_iml_half_internal __imf_rsqrtf16(_iml_half_internal x) {
  return __devicelib_imf_rsqrtf16(x);
}

DEVICE_EXTERN_C_INLINE
_iml_half_internal __devicelib_imf_invf16(_iml_half_internal);

DEVICE_EXTERN_C_INLINE
_iml_half_internal __imf_invf16(_iml_half_internal x) {
  return __devicelib_imf_invf16(x);
}

DEVICE_EXTERN_C_INLINE
_iml_half_internal __devicelib_imf_fabsf16(_iml_half_internal);

DEVICE_EXTERN_C_INLINE
_iml_half_internal __imf_fabsf16(_iml_half_internal x) {
  return __devicelib_imf_fabsf16(x);
}

DEVICE_EXTERN_C_INLINE
_iml_half_internal __devicelib_imf_fmaxf16(_iml_half_internal,
                                           _iml_half_internal);

DEVICE_EXTERN_C_INLINE
_iml_half_internal __imf_fmaxf16(_iml_half_internal x, _iml_half_internal y) {
  return __devicelib_imf_fmaxf16(x, y);
}

DEVICE_EXTERN_C_INLINE
_iml_half_internal __devicelib_imf_fminf16(_iml_half_internal,
                                           _iml_half_internal);

DEVICE_EXTERN_C_INLINE
_iml_half_internal __imf_fminf16(_iml_half_internal x, _iml_half_internal y) {
  return __devicelib_imf_fminf16(x, y);
}

DEVICE_EXTERN_C_INLINE
_iml_half_internal __devicelib_imf_copysignf16(_iml_half_internal,
                                               _iml_half_internal);

DEVICE_EXTERN_C_INLINE
_iml_half_internal __imf_copysignf16(_iml_half_internal x,
                                     _iml_half_internal y) {
  return __devicelib_imf_copysignf16(x, y);
}

#ifdef INTEL_CUSTOMIZATION
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_acoshf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_acosf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_asinhf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_asinf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_atan2f (float x, float y);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_atanhf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_atanf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_cbrtf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_coshf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_cospif (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_cosf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_erfcinvf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_erfcf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_erfinvf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_erff (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_exp10f (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_exp2f (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_expm1f (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_expf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_fdimf (float x, float y);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_fmodf (float x, float y);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_frexpf (float x, int32_t* z);
DEVICE_EXTERN_C_INLINE
int32_t __devicelib_imf_ilogbf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_rcbrtf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_ldexpf (float x, int32_t y);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_lgammaf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_logf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_log10f (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_log1pf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_log2f (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_logbf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_modff (float x, float* z);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_nextafterf (float x, float y);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_norm3df (float x, float y, float z);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_norm4df (float x, float y, float z, float t);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_powf (float x, float y);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_remainderf (float x, float y);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_remquof (float x, float y, int32_t* z);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_rhypotf (float x, float y);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_rnorm3df (float x, float y, float z);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_rnorm4df (float x, float y, float z, float t);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_roundf (float x);
DEVICE_EXTERN_C_INLINE
void __devicelib_imf_sincosf (float x, float* y, float* z);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_sinhf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_sinpif (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_sinf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_tanhf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_tanf (float x);
DEVICE_EXTERN_C_INLINE
float __devicelib_imf_tgammaf (float x);

DEVICE_EXTERN_C_INLINE
float __imf_acoshf (float x) { return __devicelib_imf_acoshf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_acosf (float x) { return __devicelib_imf_acosf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_asinhf (float x) { return __devicelib_imf_asinhf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_asinf (float x) { return __devicelib_imf_asinf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_atan2f (float x, float y) { return __devicelib_imf_atan2f (x, y); }
DEVICE_EXTERN_C_INLINE
float __imf_atanhf (float x) { return __devicelib_imf_atanhf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_atanf (float x) { return __devicelib_imf_atanf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_cbrtf (float x) { return __devicelib_imf_cbrtf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_coshf (float x) { return __devicelib_imf_coshf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_cospif (float x) { return __devicelib_imf_cospif (x); }
DEVICE_EXTERN_C_INLINE
float __imf_cosf (float x) { return __devicelib_imf_cosf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_erfcinvf (float x) { return __devicelib_imf_erfcinvf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_erfcf (float x) { return __devicelib_imf_erfcf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_erfinvf (float x) { return __devicelib_imf_erfinvf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_erff (float x) { return __devicelib_imf_erff (x); }
DEVICE_EXTERN_C_INLINE
float __imf_exp10f (float x) { return __devicelib_imf_exp10f (x); }
DEVICE_EXTERN_C_INLINE
float __imf_exp2f (float x) { return __devicelib_imf_exp2f (x); }
DEVICE_EXTERN_C_INLINE
float __imf_expm1f (float x) { return __devicelib_imf_expm1f (x); }
DEVICE_EXTERN_C_INLINE
float __imf_expf (float x) { return __devicelib_imf_expf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_fdimf (float x, float y) { return __devicelib_imf_fdimf (x, y); }
DEVICE_EXTERN_C_INLINE
float __imf_fmodf (float x, float y) { return __devicelib_imf_fmodf (x, y); }
DEVICE_EXTERN_C_INLINE
float __imf_frexpf (float x, int32_t* z) { return __devicelib_imf_frexpf (x, z); }
DEVICE_EXTERN_C_INLINE
int32_t __imf_ilogbf (float x) { return __devicelib_imf_ilogbf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_rcbrtf (float x) { return __devicelib_imf_rcbrtf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_ldexpf (float x, int32_t y) { return __devicelib_imf_ldexpf (x, y); }
DEVICE_EXTERN_C_INLINE
float __imf_lgammaf (float x) { return __devicelib_imf_lgammaf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_logf (float x) { return __devicelib_imf_logf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_log10f (float x) { return __devicelib_imf_log10f (x); }
DEVICE_EXTERN_C_INLINE
float __imf_log1pf (float x) { return __devicelib_imf_log1pf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_log2f (float x) { return __devicelib_imf_log2f (x); }
DEVICE_EXTERN_C_INLINE
float __imf_logbf (float x) { return __devicelib_imf_logbf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_modff (float x, float* z) { return __devicelib_imf_modff (x, z); }
DEVICE_EXTERN_C_INLINE
float __imf_nextafterf (float x, float y) { return __devicelib_imf_nextafterf (x, y); }
DEVICE_EXTERN_C_INLINE
float __imf_norm3df (float x, float y, float z) { return __devicelib_imf_norm3df (x, y, z); }
DEVICE_EXTERN_C_INLINE
float __imf_norm4df (float x, float y, float z, float t) { return __devicelib_imf_norm4df (x, y, z, t); }
DEVICE_EXTERN_C_INLINE
float __imf_powf (float x, float y) { return __devicelib_imf_powf (x, y); }
DEVICE_EXTERN_C_INLINE
float __imf_remainderf (float x, float y) { return __devicelib_imf_remainderf (x, y); }
DEVICE_EXTERN_C_INLINE
float __imf_remquof (float x, float y, int32_t* z) { return __devicelib_imf_remquof (x, y, z); }
DEVICE_EXTERN_C_INLINE
float __imf_rhypotf (float x, float y) { return __devicelib_imf_rhypotf (x, y); }
DEVICE_EXTERN_C_INLINE
float __imf_rnorm3df (float x, float y, float z) { return __devicelib_imf_rnorm3df (x, y, z); }
DEVICE_EXTERN_C_INLINE
float __imf_rnorm4df (float x, float y, float z, float t) { return __devicelib_imf_rnorm4df (x, y, z, t); }
DEVICE_EXTERN_C_INLINE
float __imf_roundf (float x) { return __devicelib_imf_roundf (x); }
DEVICE_EXTERN_C_INLINE
void __imf_sincosf (float x, float* y, float* z) { __devicelib_imf_sincosf (x, y, z); return; }
DEVICE_EXTERN_C_INLINE
float __imf_sinhf (float x) { return __devicelib_imf_sinhf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_sinpif (float x) { return __devicelib_imf_sinpif (x); }
DEVICE_EXTERN_C_INLINE
float __imf_sinf (float x) { return __devicelib_imf_sinf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_tanhf (float x) { return __devicelib_imf_tanhf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_tanf (float x) { return __devicelib_imf_tanf (x); }
DEVICE_EXTERN_C_INLINE
float __imf_tgammaf (float x) { return __devicelib_imf_tgammaf (x); }

#endif
#endif // __LIBDEVICE_IMF_ENABLED__
