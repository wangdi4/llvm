//==----------- get_coord_int8_matA.cpp  - DPC++ joint_matrix---------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// REQUIRES: matrix

// INTEL_CUSTOMIZATION
// get_coord() is implemented in xmain OCL CPU
// TODO: enable the upstream test when OCL CPU 2024.0 is uplifted
// RUN: %{build} -o %t.out
// RUN: %{run} %t.out
// end INTEL_CUSTOMIZATION

#include "../common.hpp"
#include <iostream>

using namespace sycl;
using namespace sycl::ext::oneapi::experimental::matrix;

constexpr size_t SG_SZ = 32;
constexpr size_t TN = 16;

#include "../get_coord_int8_matA_impl.hpp"
