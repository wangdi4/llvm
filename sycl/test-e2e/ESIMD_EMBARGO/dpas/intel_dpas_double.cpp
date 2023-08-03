// INTEL_FEATURE_ESIMD_EMBARGO
//==---------------- intel_dpas_double.cpp  - DPC++ ESIMD on-device test
//----==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// REQUIRES: intel_feature_esimd_embargo

// See ../intel_README.md for instruction explaining how to use
// Intel Falcon Shores GPU simulator.

// TODO: enable when https://jira.devtools.intel.com/browse/GSD-5670 is fixed.
// UNSUPPORTED: gpu

// RUN: %{build} -fsycl-device-code-split=per_kernel -DESIMD_EMBARGO -o %t.out
// The test requires FCS (Falcon Shores) emulator or FSC GPU!
// RUN: env IGC_VCApiOptions=-ftranslate-legacy-memory-intrinsics %{run} %t.out

// This test verifies DPAS support for double type.

#include "../../ESIMD/dpas/dpas_common.hpp"

int main(int argc, const char *argv[]) {
  queue Q(esimd_test::ESIMDSelector, esimd_test::createExceptionHandler());
  auto Dev = Q.get_device();
  std::cout << "Running on " << Dev.get_info<info::device::name>() << std::endl;

  bool Print = argc > 1 && std::string(argv[1]) == "-debug";
  bool Passed = true;

  constexpr bool LetDeduceArgs = true;
  constexpr int SystolicDepth = 8;
  constexpr int RepeatCount = 4;   // the only avaialble repeat count for double
  constexpr int ExecutionSize = 8; // must be 8 for double type
  constexpr bool UseSrc0 = true;

  constexpr dpas_argument_type df = dpas_argument_type::df;

  Passed &= test<SystolicDepth, RepeatCount, df, df, UseSrc0, ExecutionSize,
                 LetDeduceArgs>(Q, Print);
  Passed &= test<SystolicDepth, RepeatCount, df, df, !UseSrc0, ExecutionSize,
                 LetDeduceArgs>(Q, Print);

  std::cout << (Passed ? "Test Passed\n" : "Test FAILED\n");
  return Passed ? 0 : 1;
}
// end INTEL_FEATURE_ESIMD_EMBARGO
