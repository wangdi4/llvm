//==---------- defines.hpp ----- Preprocessor directives -------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <sycl/detail/defines_elementary.hpp>

#include <climits>

#if __SYCL_ID_QUERIES_FIT_IN_INT__ && __has_builtin(__builtin_assume) &&       \
    !__SYCL_EXPLICIT_SIMD__
#define __SYCL_ASSUME_INT(x) __builtin_assume((x) <= INT_MAX)
#else
#define __SYCL_ASSUME_INT(x)
#if __SYCL_ID_QUERIES_FIT_IN_INT__ && !__has_builtin(__builtin_assume)
#warning "No assumptions will be emitted due to no __builtin_assume available"
#endif
#endif

<<<<<<< HEAD
#if __has_attribute(sycl_special_class) && defined(SYCL_LANGUAGE_VERSION)
=======
#if __has_attribute(sycl_special_class) && (defined __SYCL_DEVICE_ONLY__)
>>>>>>> 236a09d7f6c3b9178c896fb855a3169d9c40b40a
#define __SYCL_SPECIAL_CLASS __attribute__((sycl_special_class))
#else
#define __SYCL_SPECIAL_CLASS
#endif

#if __has_cpp_attribute(__sycl_detail__::sycl_type)
#define __SYCL_TYPE(x) [[__sycl_detail__::sycl_type(x)]]
#else
#define __SYCL_TYPE(x)
#endif
