// INTEL_FEATURE_ESIMD_EMBARGO
//==- intel_512_grf.cpp  - DPC++ ESIMD on-device test -==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===-------------------------------------------------===//
//
// This tests support for 512 GRF size.
//
// REQUIRES: intel_feature_esimd_embargo
// UNSUPPORTED: esimd_emulator
// RUN: %{build} -o %t.out
// RUN: not env SYCL_PI_TRACE=-1 %{run} %t.out 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-NO-VAR
// RUN: not env SYCL_PROGRAM_COMPILE_OPTIONS="-g" SYCL_PI_TRACE=-1 %{run} %t.out 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-WITH-VAR
#include "../../ESIMD/esimd_test_utils.hpp"
#include <iostream>
#include <sycl/ext/intel/esimd.hpp>
#include <sycl/ext/intel/experimental/grf_size_properties.hpp>
#include <sycl/sycl.hpp>

using namespace sycl;
using namespace sycl::detail;
using namespace sycl::ext::intel::esimd;
using namespace sycl::ext::intel::experimental;
using namespace sycl::ext::intel::experimental::esimd;

bool checkResult(const std::vector<float> &A, int Inc) {
  int err_cnt = 0;
  unsigned Size = A.size();

  for (unsigned i = 0; i < Size; ++i) {
    if (A[i] != i + Inc)
      if (++err_cnt < 10)
        std::cerr << "failed at A[" << i << "]: " << A[i] << " != " << i + Inc
                  << "\n";
  }

  if (err_cnt > 0) {
    std::cout << "  pass rate: "
              << ((float)(Size - err_cnt) / (float)Size) * 100.0f << "% ("
              << (Size - err_cnt) << "/" << Size << ")\n";
    return false;
  }
  return true;
}

int main(void) {
  constexpr unsigned Size = 32;
  constexpr unsigned VL = 16;

  std::vector<float> A(Size);

  for (unsigned i = 0; i < Size; ++i) {
    A[i] = i;
  }
  try {
    buffer<float, 1> bufa(A.data(), range<1>(Size));
    queue q(esimd_test::ESIMDSelector, esimd_test::createExceptionHandler());
    sycl::ext::oneapi::experimental::properties prop{grf_size<512>};
    auto dev = q.get_device();
    std::cout << "Running on " << dev.get_info<info::device::name>() << "\n";

    auto e = q.submit([&](handler &cgh) {
      auto PA = bufa.get_access<access::mode::read_write>(cgh);
      cgh.parallel_for<class EsimdKernel512GRF>(
          Size, prop, [=](id<1> i) SYCL_ESIMD_KERNEL {
            unsigned int offset = i * VL * sizeof(float);
            simd<float, VL> va;
            va.copy_from(PA, offset);
            simd<float, VL> vc = va + 1;
            vc.copy_to(PA, offset);
          });
    });
    e.wait();
  } catch (sycl::exception const &e) {
    std::cout << "SYCL exception caught: " << e.what() << '\n';
    return 2;
  }

  if (checkResult(A, 3)) {
    std::cout << "ESIMD 512 grf kernel passed\n";
  } else {
    std::cout << "ESIMD 512 grf kernel failed\n";
    return 1;
  }

  return 0;
}

// CHECK-LABEL: ---> piProgramBuild(
// CHECK-NO-VAR: -vc-codegen -disable-finalizer-msg -ze-exp-register-file-size=512
// CHECK-WITH-VAR: -g -vc-codegen -disable-finalizer-msg -ze-exp-register-file-size=512
// TODO: Remove error check once flag is implemented in GPU driver
// CHECK: invalid api option: -ze-exp-register-file-size=512
// end INTEL_FEATURE_ESIMD_EMBARGO
