//===-------- X86TargetParserTest.cpp - X86 Target Parser tests -------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/TargetParser/X86TargetParser.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringExtras.h"
#include "gtest/gtest.h"

using namespace llvm;

namespace {
#if INTEL_CUSTOMIZATION
struct X86ABITestData {
  const char *TargetCPU;
  const char *ExpectedName;
  char ExpectedIntelISA;
  char ExpectedGnuISA;
  size_t ExpectedIntRegSize;
  size_t ExpectedFPRegSize;
};

bool testVecABIInfo(X86ABITestData &D) {
  auto AI = X86::VectorAbiIsaInfo::getByName(D.TargetCPU);
  bool ret = AI && AI->TargetName.equals(D.ExpectedName) &&
             AI->IntelISA == D.ExpectedIntelISA &&
             AI->GnuISA == D.ExpectedGnuISA &&
             AI->MaxIntVecRegByteSize == D.ExpectedIntRegSize &&
             AI->MaxFPVecRegByteSize == D.ExpectedFPRegSize;
  if (!ret)
    errs() << "Failed for " << D.TargetCPU << "\n";
  return ret;
}

bool testVecABIInfoUndef(const char* TargetCPU) {
  auto AI = X86::VectorAbiIsaInfo::getByName(TargetCPU);
  bool ret = AI == nullptr;
  if (!ret)
    errs() << "Failed negative for " << TargetCPU << "\n";
  return ret;
}
class X86VecABITest : public testing::Test {};

TEST_F(X86VecABITest, testX86VecABIInfo) {
  X86ABITestData Array[] = {
      {"core_i7_sse4_2", "core_i7_sse4_2", 'x', 'b', 16, 16},
      {"corei7", "corei7", 'x', 'b', 16, 16},
      {"ivybridge", "ivybridge", 'y', 'c', 16, 32},
      {"core-avx-i", "core-avx-i", 'y', 'c', 16, 32},
      {"sandybridge", "sandybridge", 'y', 'c', 16, 32},
      {"core_2nd_gen_avx", "core_2nd_gen_avx", 'y', 'c', 16, 32},
      {"haswell", "haswell", 'Y', 'd', 32, 32},
      {"core-avx2", "core-avx2", 'Y', 'd', 32, 32},
      {"core_4th_gen_avx", "core_4th_gen_avx", 'Y', 'd', 32, 32},
      {"core_5th_gen_avx", "core_5th_gen_avx", 'Y', 'd', 32, 32},
      {"common-avx512", "common-avx512", 'Z', 'e', 64, 64},
      {"skylake_avx512", "skylake_avx512", 'Z', 'e', 64, 64},
      {"skylake-avx512", "skylake-avx512", 'Z', 'e', 64, 64},
      {"sapphirerapids", "sapphirerapids", 'Z', 'e', 64, 64},
      {"graniterapids", "graniterapids", 'Z', 'e', 64, 64}};

  for (auto &R : Array) {
    EXPECT_EQ(testVecABIInfo(R), true);
  }
  for (auto S : {"sse42", "core-i7", "avx", "avx2"})
    EXPECT_EQ(testVecABIInfoUndef(S), true);
}
#endif // INTEL_CUSTOMIZATION
} // namespace
