// INTEL_FEATURE_ESIMD_EMBARGO
//==-------- intel_atomic_smoke_bf16.cpp  - DPC++ ESIMD on-device test -----==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// REQUIRES: intel_feature_esimd_embargo

// RUN: %{build} -o %t.out
// RUN: %{run} %t.out

// This test checks LSC atomic operations with bfloat16 type.

#define SKIP_MAIN 1
#include "../../ESIMD/lsc/atomic_smoke.cpp"

template <template <class, int> class Op, bool SkipVecSize1 = false>
bool test_sizes(queue Q, const Config &Cfg) {
  bool Passed = true;
  // TODO: CMP operations for bloat16 are defined wrongly in bfloat16.hpp.
  // See: https://github.com/intel/llvm/issues/11244 for details.
  // Enable when this check when the issue with bfloat16 is fixed.
  if constexpr (!SkipVecSize1) {
    Passed &= test<sycl::ext::oneapi::bfloat16, 1, Op>(Q, Cfg);
  }
  Passed &= test<sycl::ext::oneapi::bfloat16, 2, Op>(Q, Cfg);
  Passed &= test<sycl::ext::oneapi::bfloat16, 4, Op>(Q, Cfg);

  Passed &= test<sycl::ext::oneapi::bfloat16, 8, Op>(Q, Cfg);
  // Passed &= test<sycl::half, 8, Op>(Q, Cfg);
  Passed &= test<sycl::ext::oneapi::bfloat16, 16, Op>(Q, Cfg);
  Passed &= test<sycl::ext::oneapi::bfloat16, 32, Op>(Q, Cfg);
  return Passed;
}

int main() {
  queue Q(esimd_test::ESIMDSelector, esimd_test::createExceptionHandler());

  auto Dev = Q.get_device();
  std::cout << "Running on " << Dev.get_info<info::device::name>() << "\n";

  Config Cfg{
      11,  // int threads_per_group;
      11,  // int n_groups;
      5,   // int start_ind;
      1,   // int masked_lane;
      100, // int repeat;
      111  // int stride;
  };

  bool Passed = true;
  Passed &= test_sizes<ImplFadd>(Q, Cfg);
  Passed &= test_sizes<ImplFsub>(Q, Cfg);

  Passed &= test_sizes<ImplLSCFmax>(Q, Cfg);
  Passed &= test_sizes<ImplLSCFmin>(Q, Cfg);

  // Can't easily reset input to initial state, so just 1 iteration for CAS.
  Cfg.repeat = 1;
  // Decrease number of threads to reduce risk of halting kernel by the driver.
  Cfg.n_groups = 7;
  Cfg.threads_per_group = 3;

  constexpr bool SkipForVecSize1 = true;
  Passed &= test_sizes<ImplFcmpwr, SkipForVecSize1>(Q, Cfg);
  Passed &= test_sizes<ImplLSCFcmpwr, SkipForVecSize1>(Q, Cfg);

  Passed &= test_sizes<ImplLoad>(Q, Cfg);
#ifndef USE_SCALAR_OFFSET
  Passed &= test_sizes<ImplStore>(Q, Cfg);
#endif // USE_SCALAR_OFFSET

  std::cout << (Passed ? "Passed\n" : "FAILED\n");
  return Passed ? 0 : 1;
}
// end INTEL_FEATURE_ESIMD_EMBARGO
