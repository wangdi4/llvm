// INTEL_FEATURE_ESIMD_EMBARGO
//==----- intel_simd_subscript_operator.cpp  - DPC++ ESIMD on-device test --==//
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

// The test checks that it's possible to write through the simd subscript
// operator. E.g.:
//   simd<int, 4> v = 1;
//   v[1] = 0; // v[1] returns writable simd_view

#define SKIP_MAIN
#include "../../ESIMD/api/simd_subscript_operator.cpp"

using hf8 = sycl::ext::intel::experimental::esimd::hf8;
using bf8 = sycl::ext::intel::experimental::esimd::bf8;
namespace esimd_test {
TID(sycl::ext::intel::experimental::esimd::hf8)
TID(sycl::ext::intel::experimental::esimd::bf8)
} // namespace esimd_test

using namespace sycl;
using namespace sycl::ext::intel::esimd;
using hf8 = sycl::ext::intel::experimental::esimd::hf8;
using bf8 = sycl::ext::intel::experimental::esimd::bf8;

int main(int argc, char **argv) {
  queue q(esimd_test::ESIMDSelector, esimd_test::createExceptionHandler());

  auto dev = q.get_device();
  std::cout << "Running on " << dev.get_info<sycl::info::device::name>()
            << "\n";

  bool passed = true;

  passed &= test<hf8>(q);
  passed &= test<bf8>(q);

  return passed ? 0 : 1;
}
// end INTEL_FEATURE_ESIMD_EMBARGO