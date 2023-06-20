// Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "BackendWrapper.h"
#include "CPUCompiler.h"
#include "CompilerConfig.h"
#include "common_utils.h"
#include "gtest_wrapper.h"
#include "llvm/ADT/StringRef.h"

class CPUDetectTest : public ::testing::Test {};

void TestArch(const std::string &CPUArch) {
  ASSERT_TRUE(SETENV("CL_CONFIG_CPU_TARGET_ARCH", CPUArch.c_str()));
  CompilerConfig Config;
  // Use CL_CONFIG_CPU_TARGET_ARCH to set m_cpuArch
  Config.LoadConfig();
  Config.SkipBuiltins();
  CPUCompiler Compiler(Config);
  auto *CPUId = Compiler.GetCpuId();
  ASSERT_TRUE(STRING_EQ(CPUArch, CPUId->GetCPUName()))
      << "Can not get expected CPU name";
  bool HasFeatures = llvm::StringSwitch<bool>(CPUArch)
                         .Case("corei7", CPUId->HasSSE41() && CPUId->HasSSE42())
                         .Case("corei7-avx", CPUId->HasAVX1())
                         .Case("core-avx2", CPUId->HasAVX2())
                         .Case("skx", CPUId->HasAVX512SKX())
                         .Case("cascadelake", CPUId->HasAVX512CLX())
                         .Case("icelake-client", CPUId->HasAVX512ICL())
                         .Case("icelake-server", CPUId->HasAVX512ICX())
                         .Case("sapphirerapids", CPUId->HasSPR())
                         .Case("graniterapids", CPUId->HasGNR())
                         .Default(false);
  ASSERT_TRUE(HasFeatures) << "Can not get expected CPU features";
}

TEST_F(CPUDetectTest, ResetCPUTargetARCH) {
  TestArch("corei7");
  TestArch("corei7-avx");
  TestArch("core-avx2");
  TestArch("skx");
  TestArch("cascadelake");
  TestArch("icelake-client");
  TestArch("icelake-server");
  TestArch("sapphirerapids");
  TestArch("graniterapids");
}
