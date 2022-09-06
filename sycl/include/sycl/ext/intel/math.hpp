<<<<<<< HEAD
=======
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
>>>>>>> 4e11128887d9fe065d744687589c42b382ab4795
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
<<<<<<< HEAD
=======
/* INTEL_CUSTOMIZATION */
float __imf_erfinvf(float);
double __imf_erfinv(double);
float __imf_cdfnormf(float);
double __imf_cdfnorm(double);
/* end INTEL_CUSTOMIZATION */
>>>>>>> 4e11128887d9fe065d744687589c42b382ab4795
};

namespace sycl {
__SYCL_INLINE_VER_NAMESPACE(_V1) {
namespace ext {
namespace intel {
namespace math {

#if __cplusplus >= 201703L
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
  static_assert(sizeof(sycl::half) == sizeof(_iml_half_internal),
                "sycl::half is not compatible with _iml_half_internal.");
  _iml_half_internal xi = __builtin_bit_cast(_iml_half_internal, x);
  _iml_half_internal yi = __builtin_bit_cast(_iml_half_internal, y);
  return __builtin_bit_cast(sycl::half, __imf_copysignf16(xi, yi));
}

<<<<<<< HEAD
=======
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
std::enable_if_t<std::is_same_v<Tp, float>, float> cdfnorm(Tp x) {
  return __imf_cdfnormf(x);
}

template <typename Tp>
std::enable_if_t<std::is_same_v<Tp, double>, double> cdfnorm(Tp x) {
  return __imf_cdfnorm(x);
}
/* end INTEL_CUSTOMIZATION */
>>>>>>> 4e11128887d9fe065d744687589c42b382ab4795
#endif
} // namespace math
} // namespace intel
} // namespace ext
} // __SYCL_INLINE_VER_NAMESPACE(_V1)
} // namespace sycl
