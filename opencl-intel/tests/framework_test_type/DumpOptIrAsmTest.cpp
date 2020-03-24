// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "common_utils.h"
#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <gtest/gtest.h>

extern cl_device_type gDeviceType;

class DumpOptimizedIrAsmTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    // Create program
    m_kernelName = "test";
    std::string source = "kernel void " + m_kernelName +
                         "(global int* dst) {\n"
                         "  dst[get_global_id(0)] = 0;\n"
                         "}";
    const char *csource = source.c_str();
    m_program =
        clCreateProgramWithSource(m_context, 1, &csource, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
  }

  virtual void TearDown() override {
    cl_int err = clReleaseProgram(m_program);
    EXPECT_OCL_SUCCESS(err, "clReleaseProgram");
    err = clReleaseContext(m_context);
    EXPECT_OCL_SUCCESS(err, "clReleaseContext");
  }

  bool fileContains(const std::string &filename,
                    const std::vector<std::string> &patterns) {
    std::ifstream ifs(filename);
    if (!ifs)
      return false;
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string buffer = ss.str();
    for (auto &pattern : patterns)
      if (buffer.find(pattern) == std::string::npos)
        return false;
    return true;
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_program m_program;
  std::string m_kernelName;
};

TEST_F(DumpOptimizedIrAsmTest, buildOptions) {
  std::string dir = get_exe_dir();
  std::string asmFile = dir + "dump_opt.asm";
  std::string irFile = dir + "dump_opt.ll";

  // Build program
  std::string options =
      "-dump-opt-asm=\"" + asmFile + "\" -dump-opt-llvm=\"" + irFile + "\"";

  cl_int err = clBuildProgram(m_program, 1, &m_device, options.c_str(), nullptr,
                              nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  // Check dumped files contain kernel name and section
  std::vector<std::string> patterns = {m_kernelName};
  ASSERT_TRUE(fileContains(irFile, patterns));
  patterns.push_back("Disassembly of section");
  ASSERT_TRUE(fileContains(asmFile, patterns));

  // Delete dumped files
  (void)std::remove(asmFile.c_str());
  (void)std::remove(irFile.c_str());
}

TEST_F(DumpOptimizedIrAsmTest, buildOptionsDebug) {
  std::string dir = get_exe_dir();
  std::string asmFile = dir + "dump_opt_debug.asm";
  std::string irFile = dir + "dump_opt_debug.ll";

  // Build program
  std::string options =
      "-dump-opt-asm=\"" + asmFile + "\" -dump-opt-llvm=\"" + irFile + "\" -g";

  cl_int err = clBuildProgram(m_program, 1, &m_device, options.c_str(), nullptr,
                              nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  // Check dumped files contain kernel name and section
  std::vector<std::string> patterns = {m_kernelName};
  ASSERT_TRUE(fileContains(irFile, patterns));
  patterns.push_back("Disassembly of section .debug");
  ASSERT_TRUE(fileContains(asmFile, patterns));

  // Delete dumped files
  (void)std::remove(asmFile.c_str());
  (void)std::remove(irFile.c_str());
}

TEST_F(DumpOptimizedIrAsmTest, env) {
  // Set env
  std::string envName = "CL_CONFIG_DUMP_ASM";
  ASSERT_TRUE(SETENV(envName.c_str(), "True"))
      << ("Failed to set env " + envName);

  // Build program
  cl_int err =
      clBuildProgram(m_program, 1, &m_device, nullptr, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  // Unset env
  ASSERT_TRUE(UNSETENV(envName.c_str()))
      << ("Failed to unset env " + envName);

  // Check dumped file contains kernel name and section
  std::string asmFile = "framework_test_type1.asm";
  std::vector<std::string> patterns = {m_kernelName, "Disassembly of section"};
  ASSERT_TRUE(fileContains(asmFile, patterns));

  // Delete dumped file
  (void)std::remove(asmFile.c_str());
}
