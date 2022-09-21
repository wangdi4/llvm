// INTEL CONFIDENTIAL
//
// Copyright 2022 Intel Corporation.
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

#include "CL/cl.h"
#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "common_utils.h"
#include "gtest/gtest.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

extern cl_device_type gDeviceType;

static const char *bcFile = "code_coverage.bc";
static const char *gcdaFile = "code_coverage.gcda";
static const char *gcdaFileAOT = "code_coverage_aot.gcda";
static const char *binaryFile = "code_coverage_linux.bin";

class CodeCoverageTest : public ::testing::Test {
public:
  CodeCoverageTest()
      : m_context(nullptr), m_queue(nullptr), m_program(nullptr),
        m_kernel(nullptr) {}

protected:
  void SetUp() override {
#ifdef _WIN32
    // We skip this test on windows since we don't support clang profiling
    // in LLDJIT.
    GTEST_SKIP();
#endif
    std::string lib;
    // We only run this test if clang profile lib exists.
    if (GetEnv(lib, "CL_CONFIG_FORCE_PROFILE_LIB_PATH") &&
        !llvm::sys::fs::exists(lib)) {
      GTEST_SKIP();
    }

    m_options = "-cl-opt-disable -g";

    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    m_queue =
        clCreateCommandQueueWithProperties(m_context, m_device, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");
  }

  void TearDown() override {
    cl_int err;
    if (m_queue) {
      err = clReleaseCommandQueue(m_queue);
      ASSERT_OCL_SUCCESS(err, "clReleaseCommandQueue");
    }
    if (m_context) {
      err = clReleaseContext(m_context);
      ASSERT_OCL_SUCCESS(err, "clReleaseContext");
    }
  }

protected:
  std::string m_options;
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;
  cl_program m_program;
  cl_kernel m_kernel;
};

TEST_F(CodeCoverageTest, GCOVTestJIT) {
  cl_int err;

  std::vector<unsigned char> binary;
  ASSERT_NO_FATAL_FAILURE(readBinary((get_exe_dir() + bcFile).c_str(), binary));

  size_t lengths = binary.size();
  cl_int binaryStatus;
  const unsigned char *binaryDataRaw =
      reinterpret_cast<const unsigned char *>(binary.data());
  m_program = clCreateProgramWithBinary(m_context, 1, &m_device, &lengths,
                                        &binaryDataRaw, &binaryStatus, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithBinary");

  err = clBuildProgram(m_program, 1, &m_device, m_options.c_str(), nullptr,
                       nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  m_kernel = clCreateKernel(m_program, "test_kernel", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  size_t global_size = 1;
  size_t local_size = 1;
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, NULL, &global_size,
                               &local_size, 0, NULL, NULL);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  clFinish(m_queue);

  clReleaseKernel(m_kernel);
  clReleaseProgram(m_program);

  llvm::SmallString<128> P;
  llvm::sys::fs::current_path(P);
  llvm::sys::path::append(P, gcdaFile);
  ASSERT_TRUE(llvm::sys::fs::exists(P)) << "failed to generate gcov data\n";

  uint64_t fileSize;
  ASSERT_TRUE(!llvm::sys::fs::file_size(P, fileSize) && fileSize != 0)
      << "gcod data should not be empty\n";
}

TEST_F(CodeCoverageTest, GCOVTestAOT) {
  cl_int err;

  std::vector<unsigned char> binary;
  ASSERT_NO_FATAL_FAILURE(
      readBinary((get_exe_dir() + binaryFile).c_str(), binary));

  size_t lengths = binary.size();
  cl_int binaryStatus;
  const unsigned char *binaryDataRaw =
      reinterpret_cast<const unsigned char *>(binary.data());
  m_program = clCreateProgramWithBinary(m_context, 1, &m_device, &lengths,
                                        &binaryDataRaw, &binaryStatus, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithBinary");

  err = clBuildProgram(m_program, 1, &m_device, m_options.c_str(), nullptr,
                       nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  m_kernel = clCreateKernel(m_program, "test_kernel", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  size_t global_size = 1;
  size_t local_size = 1;
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, NULL, &global_size,
                               &local_size, 0, NULL, NULL);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  clFinish(m_queue);

  clReleaseKernel(m_kernel);
  clReleaseProgram(m_program);

  llvm::SmallString<128> P;
  llvm::sys::fs::current_path(P);
  llvm::sys::path::append(P, gcdaFileAOT);
  ASSERT_TRUE(llvm::sys::fs::exists(P)) << "failed to generate gcov data\n";

  uint64_t fileSize;
  ASSERT_TRUE(!llvm::sys::fs::file_size(P, fileSize) && fileSize != 0)
      << "gcod data should not be empty\n";
}
