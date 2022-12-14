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
//==------------- math.hpp - Intel specific math API -----------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// The main header of Intel specific math API
//===----------------------------------------------------------------------===//

#pragma once
#include <sycl/ext/intel/math/imf_half_trivial.hpp>
#include <sycl/half_type.hpp>
#include <type_traits>

// _iml_half_internal is internal representation for fp16 type used in intel
// math device library. The definition here should align with definition in
// https://github.com/intel/llvm/blob/sycl/libdevice/imf_half.hpp
#if defined(__SPIR__)
using _iml_half_internal = _Float16;
#else
using _iml_half_internal = uint16_t;
#endif

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
float __imf_normf(int, const float*);
double __imf_norm(int, const double*);
float __imf_rnormf(int, const float*);
double __imf_rnorm(int, const double*);
/* end INTEL_CUSTOMIZATION */
float __imf_ceilf(float);
double __imf_ceil(double);
_iml_half_internal __imf_ceilf16(_iml_half_internal);
float __imf_floorf(float);
double __imf_floor(double);
_iml_half_internal __imf_floorf16(_iml_half_internal);
float __imf_rintf(float);
double __imf_rint(double);
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
__SYCL_INLINE_VER_NAMESPACE(_V1) {
namespace ext {
namespace intel {
namespace math {

#if __cplusplus >= 201703L

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
  return __imf_erfinvf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> erfcinv(Tp x) {
  return __imf_erfinv(x);
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
std::enable_if_t<std::is_same_v<Tp, float>, float> norm(int dim, const Tp* p) {
  return __imf_normf(dim, p);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> norm(int dim, const Tp* p) {
  return __imf_norm(dim, p);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, float>, float> rnorm(int dim, const Tp* p) {
  return __imf_rnormf(dim, p);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> rnorm(int dim, const Tp* p) {
  return __imf_rnorm(dim, p);
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

sycl::half2 ceil(sycl::half2 x) {
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

sycl::half2 floor(sycl::half2 x) {
  return sycl::half2{floor(x.s0()), floor(x.s1())};
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

sycl::half2 rint(sycl::half2 x) {
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

sycl::half2 sqrt(sycl::half2 x) {
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

sycl::half2 rsqrt(sycl::half2 x) {
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

sycl::half2 trunc(sycl::half2 x) {
  return sycl::half2{trunc(x.s0()), trunc(x.s1())};
}

#endif
} // namespace math
} // namespace intel
} // namespace ext
} // __SYCL_INLINE_VER_NAMESPACE(_V1)
} // namespace sycl
