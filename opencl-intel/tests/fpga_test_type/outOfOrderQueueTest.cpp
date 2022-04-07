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
#include "base_fixture.h"
#include "common_utils.h"
#include <CL/cl_ext_intel.h>

class OOOQueueTest : public OCLFPGABaseFixture,
                     public ::testing::WithParamInterface<int> {
protected:
  typedef OCLFPGABaseFixture parent_t;
  void SetUp() override {
    ASSERT_TRUE(SETENV("CL_CONFIG_CPU_EMULATE_DEVICES",
                       std::to_string(GetParam()).c_str()))
        << "failed to set env CL_CONFIG_CPU_EMULATE_DEVICES";
    parent_t::SetUp();

    // Get USM function address
    cl_platform_id platform = parent_t::platform();

    m_clSharedMemAllocINTEL =
        (clSharedMemAllocINTEL_fn)clGetExtensionFunctionAddressForPlatform(
            platform, "clSharedMemAllocINTEL");
    ASSERT_NE(m_clSharedMemAllocINTEL, nullptr)
        << "failed to get address of clSharedMemAllocINTEL";

    m_clMemFreeINTEL =
        (clMemFreeINTEL_fn)clGetExtensionFunctionAddressForPlatform(
            platform, "clMemFreeINTEL");
    ASSERT_NE(m_clMemFreeINTEL, nullptr)
        << "failed to get address of clMemFreeINTEL";

    m_clSetKernelArgMemPointerINTEL = (clSetKernelArgMemPointerINTEL_fn)
        clGetExtensionFunctionAddressForPlatform(
            platform, "clSetKernelArgMemPointerINTEL");
    ASSERT_NE(m_clSetKernelArgMemPointerINTEL, nullptr)
        << "failed to get address of clSetKernelArgMemPointerINTEL";
  }

  void TearDown() override {
    parent_t::TearDown();
    ASSERT_TRUE(UNSETENV("CL_CONFIG_CPU_EMULATE_DEVICES"))
        << "failed to unset env CL_CONFIG_CPU_EMULATE_DEVICES";
  }

protected:
  clSharedMemAllocINTEL_fn m_clSharedMemAllocINTEL;
  clMemFreeINTEL_fn m_clMemFreeINTEL;
  clSetKernelArgMemPointerINTEL_fn m_clSetKernelArgMemPointerINTEL;
};

// Small reproducer for CMPLRLLVM-33706, fpga device only
// Test if there are wrong dependencies for OOO queue. Runtime Command
// NDRangeKernelCommand is enqueued and then barrier command is enqueued
// to wait for runtime command finish. Set kernel execution time as long as
// possible to test if kernel is visiting buffer after buffer is released.
TEST_P(OOOQueueTest, DependencyTest) {
  const char *ocl_test_program[] = {
      "__kernel void blocking_test(__global int *buf)"
      "{"
      "  buf[get_global_id(0)] = 1;"
      "}"};

  for (cl_device_id device : devices()) {
    cl_context context = createContext(device);

    cl_int iRet;
    // Create program with source
    cl_program program = clCreateProgramWithSource(
        context, 1, (const char **)&ocl_test_program, nullptr, &iRet);
    ASSERT_OCL_SUCCESS(iRet, "clCreateProgramWithSource");

    iRet = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    ASSERT_OCL_SUCCESS(iRet, "clBuildProgram");

    // Create Kernel
    cl_kernel kernel = clCreateKernel(program, "blocking_test", &iRet);
    ASSERT_OCL_SUCCESS(iRet, "clCreateKernel");

    // Set buffer size as big as possible
    size_t num = 16384;
    cl_int *bufferA = (cl_int *)m_clSharedMemAllocINTEL(
        context, device, nullptr, num * sizeof(int), 0, &iRet);
    ASSERT_OCL_SUCCESS(iRet, "clSharedMemAllocINTEL");

    iRet = m_clSetKernelArgMemPointerINTEL(kernel, 0, bufferA);
    ASSERT_OCL_SUCCESS(iRet, "clSetKernelArgMemPointerINTEL");

    // Create queue
    const cl_queue_properties props[] = {
        CL_QUEUE_PROPERTIES, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, 0};
    cl_command_queue queue =
        clCreateCommandQueueWithProperties(context, device, props, &iRet);
    ASSERT_OCL_SUCCESS(iRet, "clCreateCommandQueue");

    // Execute kernel
    size_t global_work_size[1] = {num};
    size_t local_work_size[1] = {64};

    iRet = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, global_work_size,
                                  local_work_size, 0, nullptr, nullptr);
    ASSERT_OCL_SUCCESS(iRet, "clEnqueueNDRangeKernel");

    // Add barrier command
    iRet = clEnqueueBarrierWithWaitList(queue, 0, nullptr, nullptr);
    ASSERT_OCL_SUCCESS(iRet, "clEnqueueBarrierWithWaitList");

    iRet = clFinish(queue);
    ASSERT_OCL_SUCCESS(iRet, "clFinish");

    iRet = m_clMemFreeINTEL(context, bufferA);
    ASSERT_OCL_SUCCESS(iRet, "clMemFreeINTEL");

    iRet = clReleaseCommandQueue(queue);
    ASSERT_OCL_SUCCESS(iRet, "clReleaseCommandQueue");

    // Release objects
    iRet = clReleaseKernel(kernel);
    ASSERT_OCL_SUCCESS(iRet, "clReleaseKernel");

    iRet = clReleaseProgram(program);
    ASSERT_OCL_SUCCESS(iRet, "clReleaseProgram");
  }
}

INSTANTIATE_TEST_SUITE_P(OOOQueueTests, OOOQueueTest,
                         ::testing::Values(1, 2, 3, 10));
