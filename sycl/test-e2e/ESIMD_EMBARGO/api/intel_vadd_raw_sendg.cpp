// INTEL_FEATURE_ESIMD_EMBARGO
//==--------- intel_vadd_raw_sendg.cpp  - DPC++ ESIMD on-device test-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------===//
// REQUIRES: intel_feature_esimd_embargo

// See ../intel_README.md for instruction explaining how to use
// Intel Falcon Shores GPU simulator.

// RUN: %{build} -fsycl-esimd-force-stateless-mem -o %t.out
// The test requires FCS (Falcon Shores) emulator or FSC GPU!
// RUN: env IGC_VCApiOptions=-ftranslate-legacy-memory-intrinsics %{run} %t.out

// This test checks the functionality of the raw_sendg function.

#include "../../ESIMD/esimd_test_utils.hpp"

#include <iostream>
#include <sycl/ext/intel/esimd.hpp>
#include <sycl/sycl.hpp>

using namespace sycl;

using namespace sycl::ext::intel;
using namespace sycl::ext::intel::esimd;

template <typename T> int test(queue q) {
  constexpr unsigned Size = 1024 * 128;
  constexpr unsigned VL = sizeof(T) == 4 ? 16 : 32;
  T *A = new T[Size];
  T *B = new T[Size];
  T *C = new T[Size];

  for (unsigned i = 0; i < Size; ++i) {
    A[i] = B[i] = i;
    C[i] = 0;
  }

  try {
    buffer<T, 1> bufa(A, range<1>(Size));
    buffer<T, 1> bufb(B, range<1>(Size));
    buffer<T, 1> bufc(C, range<1>(Size));

    // We need that many workgroups
    range<1> GlobalRange{Size / VL};

    // We need that many threads in each group
    range<1> LocalRange{1};

    auto e = q.submit([&](handler &cgh) {
      auto PA = bufa.template get_access<access::mode::read>(cgh);
      auto PB = bufb.template get_access<access::mode::read>(cgh);
      auto PC = bufc.template get_access<access::mode::write>(cgh);
      cgh.parallel_for(
          GlobalRange * LocalRange, [=](id<1> i) SYCL_ESIMD_KERNEL {
            unsigned int offset = i * VL * sizeof(T);

            simd<T, VL> va;
            va.copy_from(PA, offset);

            simd<T, VL> vb;
            vb.copy_from(PB, offset);

            simd<T, VL> vc =
                experimental::esimd::raw_sendg<T, VL, VL, 2, 0x19409180>(va, vb,
                                                                         0, 0);
            vc.copy_to(PC, offset);
          });
    });
    e.wait();
  } catch (sycl::exception const &e) {
    std::cout << "SYCL exception caught: " << e.what() << '\n';

    delete[] A;
    delete[] B;
    delete[] C;
    return 1;
  }

  int err_cnt = 0;

  for (unsigned i = 0; i < Size; ++i) {
    if (A[i] + B[i] != C[i]) {
      if (++err_cnt < 10) {
        std::cout << "failed at index " << i << ", " << C[i] << " != " << A[i]
                  << " + " << B[i] << "\n";
      }
    }
  }

  delete[] A;
  delete[] B;
  delete[] C;

  std::cout << (err_cnt > 0 ? "FAILED\n" : "Passed\n");
  return err_cnt;
}

int main(void) {

  queue q(esimd_test::ESIMDSelector, esimd_test::createExceptionHandler());

  auto dev = q.get_device();
  std::cout << "Running on " << dev.get_info<sycl::info::device::name>()
            << "\n";
  int err_cnt = 0;
  err_cnt += test<int>(q);
  err_cnt += test<float>(q);
  if (dev.has(sycl::aspect::fp16)) {
    err_cnt += test<sycl::half>(q);
  }
  return err_cnt > 0 ? 1 : 0;
}
// end INTEL_FEATURE_ESIMD_EMBARGO
