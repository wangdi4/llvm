//==------- windows_msvc_math.cpp - DPC++ ESIMD build test -----------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// REQUIRES: windows
<<<<<<< HEAD
// RUN: %clangxx -fsycl -fsyntax-only -Xclang -verify %s -I %sycl_include
=======
// RUN: %clangxx -fsycl -fsyntax-only -Xclang -verify %s
>>>>>>> 3c3926eeb347f74fe99fda3c923611f3f8262318
// expected-no-diagnostics

// The tests validates an ability to build ESIMD code on windows platform.

#include <cmath>
<<<<<<< HEAD
#include <sycl.hpp>
#include <sycl/ext/intel/esimd.hpp>
=======
#include <sycl/ext/intel/esimd.hpp>
#include <sycl/sycl.hpp>
>>>>>>> 3c3926eeb347f74fe99fda3c923611f3f8262318

using namespace sycl;

int main() {
  queue q;

  q.single_task([=]() SYCL_ESIMD_KERNEL {
     float a;
     _FDtest(&a);
   }).wait();

  return 0;
}
