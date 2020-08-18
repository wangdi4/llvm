//==--- intel-dot-product.cpp - device agnostic implementation of dot-product --==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "device.h"

#ifdef __SYCL_DEVICE_ONLY__

extern "C" {

// This is a place-holder to be replaced by IGC support for
// SPIR-V dot-product extension.

}

#endif // __SYCL_DEVICE_ONLY_
