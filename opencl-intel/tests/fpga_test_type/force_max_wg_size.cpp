// Copyright 2023 Intel Corporation.
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
#include "TestsHelpClasses.h"
#include "base_fixture.h"
#include "common_utils.h"

class FPGAForceMaxWGSizeTest : public OCLFPGABaseFixture {
protected:
  void SetUp() override {
    OCLFPGABaseFixture::SetUp();
    ForceMaxWGSize = 1 << 13; // 8192
    std::string S = std::to_string(ForceMaxWGSize);
    ASSERT_TRUE(SETENV(EnvName, S.c_str())) << "Failed to set environment";
  }

  void TearDown() override {
    ASSERT_TRUE(UNSETENV(EnvName)) << "Failed to unset environment";
    OCLFPGABaseFixture::TearDown();
  }

protected:
  size_t ForceMaxWGSize;
  const char *EnvName = "CL_CONFIG_CPU_FORCE_MAX_WORK_GROUP_SIZE";
};

TEST_F(FPGAForceMaxWGSizeTest, DeviceQuery) {
  size_t MaxWGSize = 0;
  cl_int Err = clGetDeviceInfo(device(), CL_DEVICE_MAX_WORK_GROUP_SIZE,
                               sizeof(size_t), &MaxWGSize, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clGetDeviceInfo");
  EXPECT_EQ(MaxWGSize, ForceMaxWGSize);
}

TEST_F(FPGAForceMaxWGSizeTest, Kernel) {
  cl_context Ctx = createContext(device());
  cl_command_queue Q = createCommandQueue(Ctx, device());

  const std::string Source = R"(
    kernel void test(global int *dst) {
      dst[get_global_id(0)] = 1;
    }
  )";

  cl_program Prog = createAndBuildProgram(Ctx, Source);
  cl_kernel Kern = createKernel(Prog, "test");

  // Check maximum work-group size that can be used for the kernel.
  size_t KernelWGSize;
  cl_int Err =
      clGetKernelWorkGroupInfo(Kern, device(), CL_KERNEL_WORK_GROUP_SIZE,
                               sizeof(size_t), &KernelWGSize, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clGetKernelWorkGroupInfo");
  ASSERT_EQ(KernelWGSize, ForceMaxWGSize);

  std::vector<int> Buffer(ForceMaxWGSize);
  Err = clSetKernelArgMemPointerINTEL(Kern, 0, Buffer.data());
  ASSERT_OCL_SUCCESS(Err, "clSetKernelArgMemPointerINTEL");

  Err = clEnqueueNDRangeKernel(Q, Kern, 1, nullptr, &KernelWGSize,
                               &KernelWGSize, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(Err, "clEnqueueNDRangeKernel");

  Err = clFinish(Q);
  ASSERT_OCL_SUCCESS(Err, "clFinish");

  for (size_t I = 0; I < ForceMaxWGSize; ++I)
    ASSERT_EQ(Buffer[I], 1);
}
