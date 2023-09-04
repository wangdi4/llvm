//
// Copyright (C) 2022 Intel Corporation
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

#include "TestsHelpClasses.h"
#include "common_utils.h"
#include "test_utils.h"

extern cl_device_type gDeviceType;

const char *ocl_test_program[] = {"void foo();"
                                  "void bar();"
                                  "__kernel void kernelTest()"
                                  "{"
                                  "  foo();"
                                  "  bar();"
                                  "}"};

class GetProgramBuildInfoTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    // Create program with source
    m_program = clCreateProgramWithSource(
        m_context, 1, (const char **)&ocl_test_program, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
  }

  virtual void TearDown() override {
    cl_int err;
    if (m_kernel) {
      err = clReleaseKernel(m_kernel);
      EXPECT_OCL_SUCCESS(err, "clReleaseKernel");
    }
    if (m_program) {
      err = clReleaseProgram(m_program);
      EXPECT_OCL_SUCCESS(err, "clReleaseProgram");
    }
    if (m_context) {
      err = clReleaseContext(m_context);
      EXPECT_OCL_SUCCESS(err, "clReleaseContext");
    }
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context = nullptr;
  cl_program m_program = nullptr;
  cl_kernel m_kernel = nullptr;
};

/// Check that LLJIT log are saved to program build log
TEST_F(GetProgramBuildInfoTest, LLJITLog) {

  cl_int iRet;

  iRet = clBuildProgram(m_program, 1, &m_device, nullptr, nullptr, nullptr);
  ASSERT_TRUE(iRet != CL_SUCCESS) << "Unimplemented function(s) are used";

  std::string log;
  ASSERT_NO_FATAL_FAILURE(GetBuildLog(m_device, m_program, log));

  // Check LLJIT log is saved to program build log
  ASSERT_TRUE(log.find("JIT session error: Symbols not found: [ bar, foo ]") !=
              std::string::npos)
      << "Can not find LLJIT log";
}

/// Check that LLDJIT log is saved to program build log
#if defined _WIN32
TEST_F(GetProgramBuildInfoTest, LLDJITLog) {
  cl_int iRet;

  iRet = clBuildProgram(m_program, 1, &m_device, "-g", nullptr, nullptr);
  ASSERT_OCL_SUCCESS(iRet, "clBuildProgram");

  // lld::coff::link errors occur when call clCreateKernel
  m_kernel = clCreateKernel(m_program, "kernelTest", &iRet);
  ASSERT_TRUE(iRet != CL_SUCCESS) << "Unimplemented function(s) are used";

  std::string log;
  ASSERT_NO_FATAL_FAILURE(GetBuildLog(m_device, m_program, log));

  // Check LLDJIT log is saved to program build log
#if defined _M_X64 || defined __x86_64__
  ASSERT_TRUE(log.find("error: undefined symbol: foo") != std::string::npos &&
              log.find("error: undefined symbol: bar") != std::string::npos)
      << "Can not find LLDJIT log";
#else
  // windows X86
  ASSERT_TRUE(log.find("error: undefined symbol: _foo") != std::string::npos &&
              log.find("error: undefined symbol: _bar") != std::string::npos)
      << "Can not find LLDJIT log";
#endif
}
#endif
