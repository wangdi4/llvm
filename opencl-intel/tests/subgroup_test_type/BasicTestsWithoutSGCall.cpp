// ===--------------------------------------------------------------------=== //
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "SGEmulationTest.h"

TEST_P(SGEmulationTest, BasicTestsWithoutSGCall) {
  // Kernel doesn't contain any subgroup builtin.
  const char *Kernel = "__kernel void basic() {"
                       "  int lid = get_local_id(0);}";
  const size_t KernelSize = strlen(Kernel);
  cl_int Ret = CL_SUCCESS;

  // Query all supported subgroup sizes.
  size_t RetSize = 0;
  Ret = clGetDeviceInfo(m_device, CL_DEVICE_SUB_GROUP_SIZES_INTEL, 0, nullptr,
                        &RetSize);
  ASSERT_OCL_SUCCESS(Ret, " clGetDeviceInfo");
  std::vector<size_t> SupportedSGSizes(RetSize / sizeof(size_t));
  Ret = clGetDeviceInfo(m_device, CL_DEVICE_SUB_GROUP_SIZES_INTEL, RetSize,
                        SupportedSGSizes.data(), nullptr);
  ASSERT_OCL_SUCCESS(Ret, " clGetDeviceInfo");

  cl_program Program =
      clCreateProgramWithSource(m_context, 1, &Kernel, &KernelSize, &Ret);
  ASSERT_OCL_SUCCESS(Ret, " clCreateProgramWithSource");

  Ret = clBuildProgram(Program, 0, nullptr, "", nullptr, nullptr);
  if (CL_SUCCESS != Ret) {
    std::string Log;
    ASSERT_NO_FATAL_FAILURE(GetBuildLog(m_device, Program, Log));
    FAIL() << Log;
  }

  cl_kernel Kern = clCreateKernel(Program, "basic", &Ret);
  ASSERT_OCL_SUCCESS(Ret, " clCreateKernel");

  constexpr size_t GSize = 16;
  constexpr size_t LSize = 16;

  size_t MaxSGSize = 0;
  Ret = clGetKernelSubGroupInfo(
      Kern, m_device, CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE, sizeof(LSize),
      &LSize, sizeof(MaxSGSize), &MaxSGSize, nullptr);
  ASSERT_OCL_SUCCESS(Ret, " clGetKernelSubGroupInfo");

  ASSERT_NE(
      std::find(SupportedSGSizes.begin(), SupportedSGSizes.end(), MaxSGSize),
      SupportedSGSizes.end())
      << " returned kernel subgroup size must be one of supported subgroup "
         "sizes of the device.";

  Ret = clEnqueueNDRangeKernel(m_queue, Kern, 1, nullptr, &GSize, &LSize, 0,
                               nullptr, nullptr);
  ASSERT_OCL_SUCCESS(Ret, " clEnqueueNDRangeKernel");

  Ret = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(Ret, " clFinish");

  Ret = clReleaseProgram(Program);
  ASSERT_OCL_SUCCESS(Ret, " clReleaseProgram");
  Ret = clReleaseKernel(Kern);
  ASSERT_OCL_SUCCESS(Ret, " clReleaseKernel");
}
