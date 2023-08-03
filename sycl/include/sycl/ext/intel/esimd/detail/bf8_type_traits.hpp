// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
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
/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ESIMD_EMBARGO */
//==-------------- bf8_type_traits.hpp - DPC++ Explicit SIMD API ---------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Implementation of SIMD element type traits for the bf8 type.
//===----------------------------------------------------------------------===//

#pragma once

#include <sycl/ext/intel/esimd/detail/elem_type_traits.hpp>
#include <sycl/ext/intel/experimental/esimd/bf8.hpp>
#include <sycl/ext/intel/experimental/esimd/detail/math_intrin.hpp>

/// @cond ESIMD_DETAIL

namespace sycl {
inline namespace _V1 {
  namespace ext::intel::esimd::detail {

  // Standalone definitions to use w/o instantiating element_type_traits.
  using bf8 = sycl::ext::intel::experimental::esimd::bf8;

  template <> struct element_type_traits<bf8> {
    using RawT = unsigned char;
    using EnclosingCppT = float;

    static inline constexpr bool use_native_cpp_ops = false;
    static inline constexpr bool is_floating_point = true;
  };

  // ------------------- Type conversion traits

  template <int N> struct vector_conversion_traits<bf8, N> {
    using StdT = __cpp_t<bf8>;
    using RawT = __raw_t<bf8>;

    static ESIMD_INLINE vector_type_t<RawT, N>
    convert_to_raw(vector_type_t<StdT, N> Val) {
#ifdef __SYCL_DEVICE_ONLY__
      vector_type_t<__ESIMD_DNS::__raw_t<half>, N> Input =
          __ESIMD_DNS::convert_vector<__ESIMD_DNS::__raw_t<half>, StdT, N>(Val);
      vector_type_t<RawT, N> Result =
          __esimd_qf_cvt<N, RawT, __ESIMD_DNS::__raw_t<half>>(Input);
      return Result;
#else
      vector_type_t<RawT, N> Output = 0;

      for (int i = 0; i < N; i++) {
        Output[i] = bf8::from_float(Val[i]);
      }
      return Output;
#endif
    }

    static ESIMD_INLINE vector_type_t<StdT, N>
    convert_to_cpp(vector_type_t<RawT, N> Val) {
#ifdef __SYCL_DEVICE_ONLY__
      vector_type_t<__ESIMD_DNS::__raw_t<half>, N> Result =
          __esimd_qf_cvt<N, __ESIMD_DNS::__raw_t<half>, RawT>(Val);
      return __ESIMD_DNS::convert_vector<StdT, __ESIMD_DNS::__raw_t<half>, N>(
          Result);
#else
      vector_type_t<StdT, N> Output = 0;

      for (int i = 0; i < N; i++) {
        Output[i] = bf8::to_float(Val[i]);
      }
      return Output;
#endif
    }
  };

  template <> struct scalar_conversion_traits<bf8> {
    using RawT = __raw_t<bf8>;

    static ESIMD_INLINE RawT bitcast_to_raw(bf8 Val) {
      return sycl::bit_cast<RawT>(Val);
    }

    static ESIMD_INLINE bf8 bitcast_to_wrapper(RawT Val) {
      return sycl::bit_cast<bf8>(Val);
    }
  };

  // Misc
  inline std::ostream &operator<<(std::ostream &O, bf8 const &rhs) {
    O << static_cast<float>(rhs);
    return O;
  }

  template <> struct is_esimd_arithmetic_type<bf8, void> : std::true_type {};

  } // namespace ext::intel::esimd::detail
} // namespace _V1
} // namespace sycl

/// @endcond ESIMD_DETAIL
/* end INTEL_FEATURE_ESIMD_EMBARGO */
/* end INTEL_CUSTOMIZATION */
