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

#include <CL/cl.h>

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "common_utils.h"

extern cl_device_type gDeviceType;

class DisableMasterJoinTest : public ::testing::Test {
protected:
  cl_platform_id platform_private;
  cl_device_id device_private;
  cl_context context_private;
  cl_command_queue queue_private;
  cl_kernel kernel_private;
  cl_mem buffer_private;
  cl_program program_private;

  // In test with disabling master join we allocate 1.5 MB for array, set
  // private mem size 2MB via environment CL_CONFIG_CPU_FORCE_PRIVATE_MEM_SIZE.
  // Default stack size is 1MB for windows x86 and x64. And there will be no
  // stack overflow issue since master thread does not participate in kernel
  // computations on O0 mode for Windows.
  const std::string programSources =
      "__kernel void test(__global int* o)\n"
      "{\n"
      "    const int size = (PRI_MEM_SIZE - 1024*512) / "
      "sizeof(int);\n" // PRI_MEM_SIZE MB - 0.5MB of private memory
      "    __private volatile int buf[size];\n"
      "    int gid = get_global_id(0);\n"
      "    for (int i = 0; i < size; ++i)\n"
      "        buf[i] = gid;\n"
      "    o[gid] = buf[gid + 1] + 2;\n"
      "}";

  cl_ulong trySetPrivateMemSize(cl_ulong size, std::string unit = "");
  void clDevicePrivateMemSizeTestBody();
  void cleanupPrivate();
  void setUp();
  void buildFromBinary(const std::vector<unsigned char> &binary,
                       const std::string &options);
};

void DisableMasterJoinTest::setUp() {
  cl_int iRet = clGetPlatformIDs(1, &platform_private, nullptr);
  ASSERT_OCL_SUCCESS(iRet, "clGetPlatformIDs");

  iRet = clGetDeviceIDs(platform_private, gDeviceType, 1, &device_private,
                        nullptr);
  ASSERT_OCL_SUCCESS(iRet, "clGetDeviceIDs");
  // Set auto memory to false
  ASSERT_TRUE(SETENV("CL_CONFIG_AUTO_MEMORY", "false"));

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform_private, 0};
  context_private =
      clCreateContext(prop, 1, &device_private, nullptr, nullptr, &iRet);
  ASSERT_OCL_SUCCESS(iRet, "clCreateContext");

  queue_private = clCreateCommandQueueWithProperties(
      context_private, device_private, nullptr, &iRet);
  ASSERT_OCL_SUCCESS(iRet, "clCreateCommandQueueWithProperties");
}

cl_ulong DisableMasterJoinTest::trySetPrivateMemSize(cl_ulong size,
                                                     std::string unit) {
  std::string str;
  if (unit.empty() || unit == "B")
    str = std::to_string(size) + "B";
  else if (unit == "K" || unit == "KB")
    str = std::to_string(size / 1024) + "K";
  else if (unit == "M" || unit == "MB")
    str = std::to_string(size / 1024 / 1024) + "M";
  else
    return 0;
  // set env variable to change the default value of private mem size
  if (!SETENV("CL_CONFIG_CPU_FORCE_PRIVATE_MEM_SIZE", str.c_str())) {
    return 0;
  }

  return size; // Pass STACK_SIZE to kernel
}

#ifdef _WIN32
TEST_F(DisableMasterJoinTest, testWithSource) {
  printf("testWithSource\n");

  // Set private mem size 2MB
  cl_ulong expectedPrivateMemSize = trySetPrivateMemSize(1024 * 1024 * 2, "M");
  ASSERT_NE(expectedPrivateMemSize, 0) << "trySetPrivateMemSize fail";

  setUp();

  const char *ps = programSources.c_str();
  std::string options =
      "-DPRI_MEM_SIZE=" + std::to_string(expectedPrivateMemSize) +
      " -cl-opt-disable";
  ASSERT_TRUE(BuildProgramSynch(context_private, 1, (const char **)&ps, nullptr,
                                options.c_str(), &program_private));

  clDevicePrivateMemSizeTestBody();
}
#endif

void DisableMasterJoinTest::buildFromBinary(
    const std::vector<unsigned char> &binary, const std::string &options) {
  // Create and build program
  cl_int binaryStatus[1];
  cl_int iRet = CL_SUCCESS;
  const unsigned char *binaries[1] = {&binary[0]};
  size_t binarySize = binary.size();
  program_private =
      clCreateProgramWithBinary(context_private, 1, &device_private,
                                &binarySize, binaries, binaryStatus, &iRet);
  ASSERT_OCL_SUCCESS(iRet, "clCreateProgramWithBinary");
  ASSERT_OCL_SUCCESS(binaryStatus[0], "clCreateProgramWithBinary");

  iRet = clBuildProgram(program_private, 1, &device_private, options.c_str(),
                        nullptr, nullptr);
  ASSERT_OCL_SUCCESS(iRet, "clBuildProgram");
}

#ifdef _WIN32
TEST_F(DisableMasterJoinTest, testWithBinary) {
  printf("testWithBinary\n");

  cl_ulong expectedPrivateMemSize = trySetPrivateMemSize(1024 * 1024 * 2, "M");
  ASSERT_NE(expectedPrivateMemSize, 0) << "trySetPrivateMemSize fail";

  setUp();

  const char *ps = programSources.c_str();
  std::string options =
      "-DPRI_MEM_SIZE=" + std::to_string(expectedPrivateMemSize) +
      " -cl-opt-disable";
  ASSERT_TRUE(BuildProgramSynch(context_private, 1, (const char **)&ps, nullptr,
                                options.c_str(), &program_private));

  size_t binarySize;
  // Get program binary
  cl_int iRet = clGetProgramInfo(program_private, CL_PROGRAM_BINARY_SIZES,
                                 sizeof(binarySize), &binarySize, nullptr);
  ASSERT_OCL_SUCCESS(iRet, "clGetProgramInfo CL_PROGRAM_BINARY_SIZES");

  std::vector<unsigned char> binary(binarySize);
  unsigned char *binaries[1] = {&binary[0]};
  iRet = clGetProgramInfo(program_private, CL_PROGRAM_BINARIES,
                          sizeof(binaries), &binaries, nullptr);
  ASSERT_OCL_SUCCESS(iRet, "clGetProgramInfo CL_PROGRAM_BINARIES");

  // Release program
  iRet = clReleaseProgram(program_private);
  ASSERT_OCL_SUCCESS(iRet, "clReleaseProgram");
  buildFromBinary(binary, options);

  clDevicePrivateMemSizeTestBody();
}
#endif

void DisableMasterJoinTest::cleanupPrivate() {
  if (buffer_private)
    clReleaseMemObject(buffer_private);
  if (kernel_private)
    clReleaseKernel(kernel_private);
  if (queue_private)
    clReleaseCommandQueue(queue_private);
  if (program_private)
    clReleaseProgram(program_private);
  if (context_private)
    clReleaseContext(context_private);
}

void DisableMasterJoinTest::clDevicePrivateMemSizeTestBody() {
  cl_int iRet = CL_SUCCESS;
  const size_t global_work_size = 1;
  buffer_private =
      clCreateBuffer(context_private, CL_MEM_READ_WRITE,
                     global_work_size * sizeof(cl_int), nullptr, &iRet);
  ASSERT_OCL_SUCCESS(iRet, "clCreateBuffer");

  kernel_private = clCreateKernel(program_private, "test", &iRet);
  ASSERT_OCL_SUCCESS(iRet, "clCreateKernel");

  iRet = clSetKernelArg(kernel_private, 0, sizeof(cl_mem), &buffer_private);
  ASSERT_OCL_SUCCESS(iRet, "clSetKernelArg");

  const size_t local_work_size = 1;
  iRet = clEnqueueNDRangeKernel(queue_private, kernel_private, 1, nullptr,
                                &global_work_size, &local_work_size, 0, nullptr,
                                nullptr);
  ASSERT_OCL_SUCCESS(iRet, "clEnqueueNDRangeKernel");

  iRet = clFinish(queue_private);
  ASSERT_OCL_SUCCESS(iRet, "clFinish");

  cl_int data[global_work_size] = {0};

  iRet = clEnqueueReadBuffer(queue_private, buffer_private, CL_TRUE, 0,
                             global_work_size * sizeof(cl_int), data, 0,
                             nullptr, nullptr);
  ASSERT_OCL_SUCCESS(iRet, "clEnqueueReadBuffer");

  for (size_t i = 0; i < global_work_size; ++i) {
    ASSERT_EQ((cl_int)(i + 2), data[i])
        << ("data[" + std::to_string(i) + "] is incorrect");
  }

  cleanupPrivate();
}
