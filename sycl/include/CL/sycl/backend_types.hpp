//==-------------- backend_types.hpp - SYCL backend types ------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#pragma once

#include <CL/sycl/detail/defines.hpp>

__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {

<<<<<<< HEAD
enum class backend { host, opencl, cuda, level0 }; // INTEL
=======
enum class backend : char { host, opencl, level0, cuda };
>>>>>>> d32da99fed6595c11c4be40f8db2312e11b07cdf

template <backend name, typename SYCLObjectT> struct interop;

} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)
