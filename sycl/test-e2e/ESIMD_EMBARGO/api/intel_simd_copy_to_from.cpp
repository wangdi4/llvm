// INTEL_FEATURE_ESIMD_EMBARGO
//==--- intel_simd_copy_to_from.cpp  - DPC++ ESIMD simd::copy_to/from test ==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// REQUIRES: intel_feature_esimd_embargo

// RUN: %{build} -fsycl-device-code-split=per_kernel -DESIMD_EMBARGO -o %t.out
// The test verifies the work with FCS (Falcon Shores) specific data types,
// but it only copies data. Thus, it does not require FCS GPU or simulator.
// RUN: env IGC_VCApiOptions=-ftranslate-legacy-memory-intrinsics %{run} %t.out

// This test checks simd::copy_from/to methods with alignment flags.

#define SKIP_MAIN
#include "../../ESIMD/api/simd_copy_to_from.cpp"

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

  Pass &= testUSM<hf8>(Q);
  Pass &= testAcc<hf8>(Q);
  Pass &= testUSM<bf8>(Q);
  Pass &= testAcc<bf8>(Q);

  std::cout << (Pass ? "Test Passed\n" : "Test FAILED\n");
  return Pass ? 0 : 1;
}
// end INTEL_FEATURE_ESIMD_EMBARGO