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
  virtual void SetUp() {
    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");
  }

  virtual void TearDown() {
    cl_int err = clReleaseContext(m_context);
    EXPECT_OCL_SUCCESS(err, "clReleaseContext");
  }

  bool fileContains(const std::string &filename, std::string pattern) {
    std::ifstream ifs(filename);
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string buffer = ss.str();
    return buffer.find(pattern) != std::string::npos;
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
};

TEST_F(DumpOptimizedIrAsmTest, dumpOptions) {
  // Create program
  cl_int err;
  const char *source = "kernel void test(global int* dst) {\n"
                       "  dst[get_global_id(0)] = 0;\n"
                       "}";
  cl_program program =
      clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");

  std::string dir = get_exe_dir();
  std::string asmFile = dir + "dump_opt.asm";
  std::string irFile = dir + "dump_opt.ll";

  // Build program
  std::string options =
      "-dump-opt-asm=\"" + asmFile + "\" -dump-opt-llvm=\"" + irFile + "\"";

  err =
      clBuildProgram(program, 1, &m_device, options.c_str(), nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  // Check dumped files contain kernel name
  ASSERT_TRUE(fileContains(asmFile, "test"));
  ASSERT_TRUE(fileContains(irFile, "test"));

  // Delete dumped files
  (void)std::remove(asmFile.c_str());
  (void)std::remove(irFile.c_str());
}
