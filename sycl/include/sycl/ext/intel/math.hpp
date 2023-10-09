// INTEL_CUSTOMIZATION
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//==-------------- math.hpp - Intel specific math API ----------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// The main header of Intel specific math API
//===----------------------------------------------------------------------===//

#pragma once

// _iml_half_internal is internal representation for fp16 type used in intel
// math device library. The definition here should align with definition in
// https://github.com/intel/llvm/blob/sycl/libdevice/imf_half.hpp
#if defined(__SPIR__)
using _iml_half_internal = _Float16;
#else
#include <cstdint> // for uint16_t
using _iml_half_internal = uint16_t;
#endif

#include <sycl/builtins.hpp>
#include <sycl/ext/intel/math/imf_fp_conversions.hpp>
#include <sycl/ext/intel/math/imf_half_trivial.hpp>
#include <sycl/ext/intel/math/imf_rounding_math.hpp>
#include <sycl/ext/intel/math/imf_simd.hpp>
#include <sycl/ext/oneapi/bfloat16.hpp>
#include <sycl/half_type.hpp>
#include <type_traits>

extern "C" {
float __imf_saturatef(float);
float __imf_copysignf(float, float);
double __imf_copysign(double, double);
_iml_half_internal __imf_copysignf16(_iml_half_internal, _iml_half_internal);
/* INTEL_CUSTOMIZATION */
float __imf_erfinvf(float);
double __imf_erfinv(double);
float __imf_erfcinvf(float);
double __imf_erfcinv(double);
float __imf_cdfnormf(float);
double __imf_cdfnorm(double);
float __imf_cdfnorminvf(float);
double __imf_cdfnorminv(double);
float __imf_normf(int, const float *);
double __imf_norm(int, const double *);
float __imf_rnormf(int, const float *);
double __imf_rnorm(int, const double *);
float __imf_cosf(float);
double __imf_cos(double);
_iml_half_internal __imf_cosf16(_iml_half_internal);
float __imf_exp10f(float);
double __imf_exp10(double);
_iml_half_internal __imf_exp10f16(_iml_half_internal);
float __imf_exp2f(float);
double __imf_exp2(double);
_iml_half_internal __imf_exp2f16(_iml_half_internal);
float __imf_expf(float);
double __imf_exp(double);
_iml_half_internal __imf_expf16(_iml_half_internal);
float __imf_log10f(float);
double __imf_log10(double);
_iml_half_internal __imf_log10f16(_iml_half_internal);
float __imf_log2f(float);
double __imf_log2(double);
_iml_half_internal __imf_log2f16(_iml_half_internal);
float __imf_logf(float);
double __imf_log(double);
_iml_half_internal __imf_logf16(_iml_half_internal);
float __imf_sinf(float);
double __imf_sin(double);
_iml_half_internal __imf_sinf16(_iml_half_internal);
float __imf_pownf(float, int);
double __imf_pown(double, int);
float __imf_i0f(float);
double __imf_i0(double);
float __imf_i1f(float);
double __imf_i1(double);
float __imf_j0f(float);
double __imf_j0(double);
float __imf_j1f(float);
double __imf_j1(double);
float __imf_jnf(int, float);
float __imf_y0f(float);
double __imf_y0(double);
float __imf_y1f(float);
double __imf_y1(double);
float __imf_ynf(int, float);
/* end INTEL_CUSTOMIZATION */
float __imf_ceilf(float);
double __imf_ceil(double);
_iml_half_internal __imf_ceilf16(_iml_half_internal);
float __imf_floorf(float);
double __imf_floor(double);
_iml_half_internal __imf_floorf16(_iml_half_internal);
float __imf_rintf(float);
double __imf_rint(double);
_iml_half_internal __imf_invf16(_iml_half_internal);
float __imf_invf(float);
double __imf_inv(double);
_iml_half_internal __imf_rintf16(_iml_half_internal);
float __imf_sqrtf(float);
double __imf_sqrt(double);
_iml_half_internal __imf_sqrtf16(_iml_half_internal);
float __imf_rsqrtf(float);
double __imf_rsqrt(double);
_iml_half_internal __imf_rsqrtf16(_iml_half_internal);
float __imf_truncf(float);
double __imf_trunc(double);
_iml_half_internal __imf_truncf16(_iml_half_internal);
};

namespace sycl {
inline namespace _V1 {
namespace ext::intel::math {

static_assert(sizeof(sycl::half) == sizeof(_iml_half_internal),
              "sycl::half is not compatible with _iml_half_internal.");

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> saturate(Tp x) {
  return __imf_saturatef(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> copysign(Tp x, Tp y) {
  return __imf_copysignf(x, y);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> copysign(Tp x, Tp y) {
  return __imf_copysign(x, y);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> copysign(Tp x,
                                                                      Tp y) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  _iml_half_internal yi = __builtin_bit_cast(_iml_half_internal, y);
  return __builtin_bit_cast(sycl::half, __imf_copysignf16(xi, yi));
}

/* INTEL_CUSTOMIZATION */
template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> erfinv(Tp x) {
  return __imf_erfinvf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> erfinv(Tp x) {
  return __imf_erfinv(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> erfcinv(Tp x) {
  return __imf_erfcinvf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> erfcinv(Tp x) {
  return __imf_erfcinv(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> cdfnorm(Tp x) {
  return __imf_cdfnormf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> cdfnorm(Tp x) {
  return __imf_cdfnorm(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> cdfnorminv(Tp x) {
  return __imf_cdfnorminvf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> cdfnorminv(Tp x) {
  return __imf_cdfnorminv(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> norm(int dim, const Tp *p) {
  return __imf_normf(dim, p);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> norm(int dim,
                                                          const Tp *p) {
  return __imf_norm(dim, p);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> rnorm(int dim, const Tp *p) {
  return __imf_rnormf(dim, p);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> rnorm(int dim,
                                                           const Tp *p) {
  return __imf_rnorm(dim, p);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> cos(Tp x) {
  return __imf_cosf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> cos(Tp x) {
  return __imf_cos(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> cos(Tp x) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  return __builtin_bit_cast(sycl::half, __imf_cosf16(xi));
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half2>, sycl::half2> cos(Tp x) {
  return sycl::half2{cos(x.s0()), cos(x.s1())};
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> exp10(Tp x) {
  return __imf_exp10f(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> exp10(Tp x) {
  return __imf_exp10(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> exp10(Tp x) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  return __builtin_bit_cast(sycl::half, __imf_exp10f16(xi));
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half2>, sycl::half2> exp10(Tp x) {
  return sycl::half2{exp10(x.s0()), exp10(x.s1())};
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> exp2(Tp x) {
  return __imf_exp2f(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> exp2(Tp x) {
  return __imf_exp2(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> exp2(Tp x) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  return __builtin_bit_cast(sycl::half, __imf_exp2f16(xi));
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half2>, sycl::half2> exp2(Tp x) {
  return sycl::half2{exp2(x.s0()), exp2(x.s1())};
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> exp(Tp x) {
  return __imf_expf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> exp(Tp x) {
  return __imf_exp(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> exp(Tp x) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  return __builtin_bit_cast(sycl::half, __imf_expf16(xi));
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half2>, sycl::half2> exp(Tp x) {
  return sycl::half2{exp(x.s0()), exp(x.s1())};
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> log10(Tp x) {
  return __imf_log10f(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> log10(Tp x) {
  return __imf_log10(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> log10(Tp x) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  return __builtin_bit_cast(sycl::half, __imf_log10f16(xi));
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half2>, sycl::half2> log10(Tp x) {
  return sycl::half2{log10(x.s0()), log10(x.s1())};
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> log2(Tp x) {
  return __imf_log2f(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> log2(Tp x) {
  return __imf_log2(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> log2(Tp x) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  return __builtin_bit_cast(sycl::half, __imf_log2f16(xi));
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half2>, sycl::half2> log2(Tp x) {
  return sycl::half2{log2(x.s0()), log2(x.s1())};
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> log(Tp x) {
  return __imf_logf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> log(Tp x) {
  return __imf_log(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> log(Tp x) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  return __builtin_bit_cast(sycl::half, __imf_logf16(xi));
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half2>, sycl::half2> log(Tp x) {
  return sycl::half2{log(x.s0()), log(x.s1())};
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> sin(Tp x) {
  return __imf_sinf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> sin(Tp x) {
  return __imf_sin(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> sin(Tp x) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  return __builtin_bit_cast(sycl::half, __imf_sinf16(xi));
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half2>, sycl::half2> sin(Tp x) {
  return sycl::half2{sin(x.s0()), sin(x.s1())};
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> powi(Tp x, int y) {
  return __imf_pownf(x, y);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> powi(Tp x, int y) {
  return __imf_pown(x, y);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> cyl_bessel_i0(Tp x) {
  return __imf_i0f(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> cyl_bessel_i0(Tp x) {
  return __imf_i0(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> cyl_bessel_i1(Tp x) {
  return __imf_i1f(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> cyl_bessel_i1(Tp x) {
  return __imf_i1(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> j0(Tp x) {
  return __imf_j0f(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> j0(Tp x) {
  return __imf_j0(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> j1(Tp x) {
  return __imf_j1f(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> j1(Tp x) {
  return __imf_j1(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> jn(int n, Tp x) {
  return __imf_jnf(n, x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> y0(Tp x) {
  return __imf_y0f(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> y0(Tp x) {
  return __imf_y0(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> y1(Tp x) {
  return __imf_y1f(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> y1(Tp x) {
  return __imf_y1(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> yn(int n, Tp x) {
  return __imf_ynf(n, x);
}

/* end INTEL_CUSTOMIZATION */
template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> ceil(Tp x) {
  return __imf_ceilf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> ceil(Tp x) {
  return __imf_ceil(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> ceil(Tp x) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  return __builtin_bit_cast(sycl::half, __imf_ceilf16(xi));
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half2>, sycl::half2> ceil(Tp x) {
  return sycl::half2{ceil(x.s0()), ceil(x.s1())};
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> floor(Tp x) {
  return __imf_floorf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> floor(Tp x) {
  return __imf_floor(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> floor(Tp x) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  return __builtin_bit_cast(sycl::half, __imf_floorf16(xi));
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half2>, sycl::half2> floor(Tp x) {
  return sycl::half2{floor(x.s0()), floor(x.s1())};
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> inv(Tp x) {
  return __imf_invf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> inv(Tp x) {
  return __imf_inv(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> inv(Tp x) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  return __builtin_bit_cast(sycl::half, __imf_invf16(xi));
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half2>, sycl::half2> inv(Tp x) {
  return sycl::half2{inv(x.s0()), inv(x.s1())};
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> rint(Tp x) {
  return __imf_rintf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> rint(Tp x) {
  return __imf_rint(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> rint(Tp x) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  return __builtin_bit_cast(sycl::half, __imf_rintf16(xi));
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half2>, sycl::half2> rint(Tp x) {
  return sycl::half2{rint(x.s0()), rint(x.s1())};
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> sqrt(Tp x) {
  return __imf_sqrtf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> sqrt(Tp x) {
  return __imf_sqrt(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> sqrt(Tp x) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  return __builtin_bit_cast(sycl::half, __imf_sqrtf16(xi));
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half2>, sycl::half2> sqrt(Tp x) {
  return sycl::half2{sqrt(x.s0()), sqrt(x.s1())};
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> rsqrt(Tp x) {
  return __imf_rsqrtf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> rsqrt(Tp x) {
  return __imf_rsqrt(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> rsqrt(Tp x) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  return __builtin_bit_cast(sycl::half, __imf_rsqrtf16(xi));
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half2>, sycl::half2> rsqrt(Tp x) {
  return sycl::half2{rsqrt(x.s0()), rsqrt(x.s1())};
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> trunc(Tp x) {
  return __imf_truncf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> trunc(Tp x) {
  return __imf_trunc(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half>, sycl::half> trunc(Tp x) {
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  return __builtin_bit_cast(sycl::half, __imf_truncf16(xi));
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, sycl::half2>, sycl::half2> trunc(Tp x) {
  return sycl::half2{trunc(x.s0()), trunc(x.s1())};
}

} // namespace ext::intel::math
} // namespace _V1
} // namespace sycl
