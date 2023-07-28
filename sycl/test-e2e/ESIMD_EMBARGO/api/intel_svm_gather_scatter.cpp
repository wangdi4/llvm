// INTEL_FEATURE_ESIMD_EMBARGO
//==--- intel_svm_gather_scatter.cpp  - DPC++ ESIMD svm gather/scatter test ==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// REQUIRES: intel_feature_esimd_embargo

// TODO: Enable when CMPLRLLVM-49839 is fixed.
// UNSUPPORTED: gpu

// See ../intel_README.md for instruction explaining how to use
// Intel Falcon Shores GPU simulator.

// RUN: %{build} -fsycl-device-code-split=per_kernel -DESIMD_EMBARGO -o %t.out
// The test requires FCS (Falcon Shores) emulator or FSC GPU!
// RUN: env IGC_VCApiOptions=-ftranslate-legacy-memory-intrinsics %{run} %t.out

// Regression test for SVM gather/scatter API.

#define SKIP_MAIN
#include "../../ESIMD/api/svm_gather_scatter.cpp"

using hf8 = sycl::ext::intel::experimental::esimd::hf8;
using bf8 = sycl::ext::intel::experimental::esimd::bf8;
namespace esimd_test {
TID(sycl::ext::intel::experimental::esimd::hf8)
TID(sycl::ext::intel::experimental::esimd::bf8)
} // namespace esimd_test

int main(void) {
  queue Q(esimd_test::ESIMDSelector, esimd_test::createExceptionHandler());
  auto Dev = Q.get_device();
  std::cout << "Running on " << Dev.get_info<sycl::info::device::name>()
            << "\n";

  bool Pass = true;

  Pass &= test<hf8, 1>(Q);
  Pass &= test<bf8, 1>(Q);
  Pass &= test<hf8, 2>(Q);
  Pass &= test<hf8, 4>(Q);
  Pass &= test<hf8, 8>(Q);
  Pass &= test<hf8, 16>(Q);
  Pass &= test<hf8, 32>(Q);
  Pass &= test<bf8, 2>(Q);
  Pass &= test<bf8, 4>(Q);
  Pass &= test<bf8, 8>(Q);
  Pass &= test<bf8, 16>(Q);
  Pass &= test<bf8, 32>(Q);

  std::cout << (Pass ? "Test Passed\n" : "Test FAILED\n");
  return Pass ? 0 : 1;
}
// end INTEL_FEATURE_ESIMD_EMBARGO