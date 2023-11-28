// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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

#include "CL/cl_ext.h"
#include "gtest_wrapper.h"
#include <iostream>

extern cl_device_type gDeviceType;

void clGetKernelSubGroupInfo() {
  std::cout << "============================================================="
            << std::endl;
  std::cout << "GetKernelSubGroupInfo" << std::endl;
  std::cout << "============================================================="
            << std::endl;

  cl_int iRet = CL_SUCCESS;

  // Get platform.
  cl_platform_id platform = 0;
  iRet = clGetPlatformIDs(1, &platform, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet); // << "clGetPlatformIDs failed.";

  // Get device.
  cl_device_id device = NULL;
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet) << "clGetDeviceIDs failed. ";

  // Create context.
  cl_context context = NULL;
  {
    const cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                           (cl_context_properties)platform, 0};
    context = clCreateContext(prop, 1, &device, NULL, NULL, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateContext failed. ";
  }

  // Create and build program.
  const char *kernel_source = "\
                              __kernel void dummy_kernel()\
                              {\
                                return;\
                              }";

  const size_t kernel_size = strlen(kernel_source);

  cl_program program = clCreateProgramWithSource(context, 1, &kernel_source,
                                                 &kernel_size, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed. ";

  iRet = clBuildProgram(program, 0, nullptr, "", nullptr, nullptr);

  if (CL_SUCCESS != iRet) {
    std::string log("", 1000);
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log.size(),
                          &log[0], nullptr);
    std::cout << log << std::endl;
  }
  ASSERT_EQ(CL_SUCCESS, iRet) << " clBuildProgram failed. ";

  cl_kernel kernel = clCreateKernel(program, "dummy_kernel", &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateKernel failed. ";

  // Test only one case as GetKernelSubGroupInfo is proxied to
  // GetKernelSubGroupInfo.
  { // local work sizes are [20,20,20]
    size_t dummy_vec[] = {20, 20, 20};
    std::vector<size_t> local_work_sizes(
        dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
    size_t max_SG_size = 0;
    size_t returned_size = 0;
    iRet = clGetKernelSubGroupInfo(
        kernel, device, CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
        local_work_sizes.size() * sizeof(local_work_sizes[0]),
        &local_work_sizes[0], sizeof(max_SG_size), &max_SG_size,
        &returned_size);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
    ASSERT_EQ(sizeof(size_t), returned_size)
        << " clGetKernelSubGroupInfo failed. Expected and returned sizes "
           "differ. ";
    ASSERT_LT((size_t)0, max_SG_size) << " clGetKernelSubGroupInfo failed. Max "
                                         "subgroup size can't be less than 1. ";
  }

  // Tests accessibility via clGetExtensionFunctionAddress
  {
    // local work sizes are [20,20,20]
    size_t dummy_vec[] = {20, 20, 20};
    std::vector<size_t> local_work_sizes(
        dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
    size_t number_of_SG = 0;
    size_t returned_size = 0;
    iRet = clGetKernelSubGroupInfo(
        kernel, device, CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE,
        local_work_sizes.size() * sizeof(local_work_sizes[0]),
        &local_work_sizes[0], sizeof(number_of_SG), &number_of_SG,
        &returned_size);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
    ASSERT_EQ(sizeof(size_t), returned_size)
        << " clGetKernelSubGroupInfo failed. Expected and returned size "
           "differ. ";
    ASSERT_LT((size_t)0, number_of_SG)
        << " clGetKernelSubGroupInfo failed. Max subgroup size can't be less "
           "than 1. ";
  }

  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseContext(context);
  clReleaseDevice(device);
}
