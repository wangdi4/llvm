// INTEL_CUSTOMIZATION
//
// Modifications, Copyright (C) 2021 Intel Corporation
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
//==----------------- util.hpp - DPC++ Explicit SIMD API  ------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Utility functions used for implementing experimental Explicit SIMD APIs.
//===----------------------------------------------------------------------===//

#pragma once

/// @cond ESIMD_DETAIL

#include <sycl/ext/intel/esimd/detail/util.hpp>

__SYCL_INLINE_NAMESPACE(cl) {
<<<<<<< HEAD
namespace sycl {
namespace ext {
namespace intel {
namespace experimental {
namespace esimd {
namespace detail {

/// ESIMD intrinsic operand size in bytes.
struct OperandSize {
  enum { BYTE = 1, WORD = 2, DWORD = 4, QWORD = 8, OWORD = 16, GRF = 32 };
};

/// Compute next power of 2 of a constexpr with guaranteed compile-time
/// evaluation.
template <unsigned int N, unsigned int K, bool K_gt_eq_N> struct NextPowerOf2;

template <unsigned int N, unsigned int K> struct NextPowerOf2<N, K, true> {
  static constexpr unsigned int get() { return K; }
};

template <unsigned int N, unsigned int K> struct NextPowerOf2<N, K, false> {
  static constexpr unsigned int get() {
    return NextPowerOf2<N, K * 2, K * 2 >= N>::get();
  }
};

template <unsigned int N> constexpr unsigned int getNextPowerOf2() {
  return NextPowerOf2<N, 1, (1 >= N)>::get();
}

template <> constexpr unsigned int getNextPowerOf2<0>() { return 0; }

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ESIMD_EMBARGO */
constexpr unsigned int roundUpNextMultiple(unsigned int N, unsigned int M) {
  return ((N + M - 1) / M) * M;
}
/* end INTEL_FEATURE_ESIMD_EMBARGO */
/* end INTEL_CUSTOMIZATION */

/// Compute binary logarithm of a constexpr with guaranteed compile-time
/// evaluation.
template <unsigned int N, bool N_gt_1> struct Log2;

template <unsigned int N> struct Log2<N, false> {
  static constexpr unsigned int get() { return 0; }
};

template <unsigned int N> struct Log2<N, true> {
  static constexpr unsigned int get() {
    return 1 + Log2<(N >> 1), ((N >> 1) > 1)>::get();
  }
};

template <unsigned int N> constexpr unsigned int log2() {
  return Log2<N, (N > 1)>::get();
}

/// Check if a given 32 bit positive integer is a power of 2 at compile time.
static ESIMD_INLINE constexpr bool isPowerOf2(unsigned int n) {
  return (n & (n - 1)) == 0;
}

static ESIMD_INLINE constexpr bool isPowerOf2(unsigned int n,
                                              unsigned int limit) {
  return (n & (n - 1)) == 0 && n <= limit;
}
=======
namespace __ESIMD_EDNS {
>>>>>>> c557d7884625226508591abb062cde4edfff8e24

template <unsigned int N, unsigned int M>
constexpr unsigned int roundUpNextMultiple() {
  return ((N + M - 1) / M) * M;
}

<<<<<<< HEAD
/// type traits
template <typename T> struct is_esimd_vector : public std::false_type {};

template <typename T, int N>
struct is_esimd_vector<simd<T, N>> : public std::true_type {};

template <typename T, int N>
using is_hw_int_type =
    typename std::bool_constant<std::is_integral_v<T> && (sizeof(T) == N)>;

template <typename T> using is_qword_type = is_hw_int_type<T, 8>;
template <typename T> using is_dword_type = is_hw_int_type<T, 4>;
template <typename T> using is_word_type = is_hw_int_type<T, 2>;
template <typename T> using is_byte_type = is_hw_int_type<T, 1>;

template <typename T, int N>
using is_hw_fp_type = typename std::bool_constant<std::is_floating_point_v<T> &&
                                                  (sizeof(T) == N)>;

template <typename T> using is_fp_type = is_hw_fp_type<T, 4>;
template <typename T> using is_df_type = is_hw_fp_type<T, 8>;

template <typename T>
using is_fp_or_dword_type =
    typename std::bool_constant<is_fp_type<T>::value ||
                                is_dword_type<T>::value>;

/* INTEL_CUSTOMIZATION */
/* INTEL_FEATURE_ESIMD_EMBARGO */

template <typename T>
struct is_hf_type
    : std::integral_constant<
          bool,
          std::is_same<detail::half,
                       typename sycl::detail::remove_const_t<T>>::value> {};

template <typename T, int N> struct is_hf_type<raw_vector_type<T, N>> {
  static const bool value = is_hf_type<T>::value;
};

template <typename T, int N> struct is_hf_type<simd<T, N>> {
  static const bool value = is_hf_type<T>::value;
};

=======
>>>>>>> c557d7884625226508591abb062cde4edfff8e24
/// Compile-time checks if first template parameter is equal for any other
template <typename...> struct is_one_of {
  static constexpr bool value = false;
};

template <typename Checked, typename First, typename... Other>
struct is_one_of<Checked, First, Other...> {
  static constexpr bool value =
      std::is_same<typename std::remove_const<Checked>::type,
                   typename std::remove_const<First>::type>::value ||
      is_one_of<Checked, Other...>::value;
};
template <typename Checked, typename... T>
inline constexpr bool is_one_of_v = is_one_of<Checked, T...>::value;

/// Compile-time checks if compile-time known  element of enum class is equal
/// for any other compile-time known elements of enum
template <typename enumClass, enumClass... E> struct is_one_of_enum {
  static constexpr bool value = false;
};

template <typename enumClass, enumClass Checked, enumClass First,
          enumClass... Else>
struct is_one_of_enum<enumClass, Checked, First, Else...> {
  static constexpr bool value =
      (Checked == First) || is_one_of_enum<enumClass, Checked, Else...>::value;
};
template <typename enumClass, enumClass... T>
inline constexpr bool is_one_of_enum_v = is_one_of_enum<enumClass, T...>::value;

<<<<<<< HEAD
/* end INTEL_FEATURE_ESIMD_EMBARGO */
/* end INTEL_CUSTOMIZATION */

/// Convert types into vector types
template <typename T> struct simd_type { using type = simd<T, 1>; };
template <typename T, int N> struct simd_type<raw_vector_type<T, N>> {
  using type = simd<T, N>;
};

template <typename T> struct simd_type<T &> {
  using type = typename simd_type<T>::type;
};
template <typename T> struct simd_type<T &&> {
  using type = typename simd_type<T>::type;
};
template <typename T> struct simd_type<const T> {
  using type = typename simd_type<T>::type;
};

template <typename T> struct dword_type { using type = T; };
template <> struct dword_type<char> { using type = int; };
template <> struct dword_type<short> { using type = int; };
template <> struct dword_type<uchar> { using type = uint; };
template <> struct dword_type<ushort> { using type = uint; };

template <typename T> struct byte_type { using type = T; };
template <> struct byte_type<short> { using type = char; };
template <> struct byte_type<int> { using type = char; };
template <> struct byte_type<ushort> { using type = uchar; };
template <> struct byte_type<uint> { using type = uchar; };

template <typename T> struct word_type { using type = T; };
template <> struct word_type<char> { using type = short; };
template <> struct word_type<int> { using type = short; };
template <> struct word_type<uchar> { using type = ushort; };
template <> struct word_type<uint> { using type = ushort; };

// Utility for compile time loop unrolling.
template <unsigned N> class ForHelper {
  template <unsigned I, typename Action> static inline void repeat(Action A) {
    if constexpr (I < N)
      A(I);
    if constexpr (I + 1 < N)
      repeat<I + 1, Action>(A);
  }

public:
  template <typename Action> static inline void unroll(Action A) {
    ForHelper::template repeat<0, Action>(A);
  }
};

} // namespace detail

} // namespace esimd
} // namespace experimental
} // namespace intel
} // namespace ext
} // namespace sycl
=======
} // namespace __ESIMD_EDNS
>>>>>>> c557d7884625226508591abb062cde4edfff8e24
} // __SYCL_INLINE_NAMESPACE(cl)

/// @endcond ESIMD_DETAIL
