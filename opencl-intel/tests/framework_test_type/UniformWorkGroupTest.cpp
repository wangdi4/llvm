//===-- UniformWorkGroupTest.cpp - Uniform Work Group Test -----*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "CL.h"
#include "TestsHelpClasses.h"
#include "cl_types.h"

void UniformWorkGroupTest() {

  cl_platform_id platform = 0;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  cl_program program;
  cl_kernel kernel;

  const char *compile_flag[] = {"", "-cl-std=CL2.0",
                                "-cl-std=CL2.0 -cl-uniform-work-group-size"};
  cl_int enqueue_result[] = {CL_INVALID_WORK_GROUP_SIZE, CL_SUCCESS,
                             CL_INVALID_WORK_GROUP_SIZE};

  // Get platform
  cl_int iRet = clGetPlatformIDs(1, &platform, nullptr);
  ASSERT_OCL_SUCCESS(iRet, clGetPlatformIDs);

  // Get device
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
  ASSERT_OCL_SUCCESS(iRet, clGetDeviceIDs);

  // Create context
  context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &iRet);
  ASSERT_OCL_SUCCESS(iRet, clCreateContext);

  // Create command queue
  queue = clCreateCommandQueueWithProperties(context, device, nullptr, &iRet);
  ASSERT_OCL_SUCCESS(iRet, clCreateCommandQueueWithProperties);

  const char *dummy_kernel[] = {"__kernel void dummy_kernel() {}"};

  // Create program
  program = clCreateProgramWithSource(context, 1, dummy_kernel, nullptr, &iRet);
  ASSERT_OCL_SUCCESS(iRet, clCreateProgramWithSource);

  size_t local_work_size[] = {4};
  size_t global_work_size[] = {7};

  for (unsigned i = 0; i < 3; ++i) {
    std::string errMsg =
        "Compile flag '" + std::string(compile_flag[i]) + "' failed";

    // Build program
    iRet =
        clBuildProgram(program, 0, nullptr, compile_flag[i], nullptr, nullptr);
    ASSERT_OCL_SUCCESS(iRet, clBuildProgram) << errMsg;

    // Create kernel
    kernel = clCreateKernel(program, "dummy_kernel", &iRet);
    ASSERT_OCL_SUCCESS(iRet, clCreateKernel) << errMsg;

    // Enqueue kernel
    iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size,
                                  local_work_size, 0, NULL, NULL);
    ASSERT_OCL_EQ(enqueue_result[i], iRet, clEnqueueNDRangeKernel) << errMsg;

    iRet = clFinish(queue);
    ASSERT_OCL_SUCCESS(iRet, clFinish) << errMsg;

    iRet = clReleaseKernel(kernel);
    ASSERT_OCL_SUCCESS(iRet, clReleaseKernel) << errMsg;
  }

  iRet = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(iRet, clReleaseProgram);
  iRet = clReleaseContext(context);
  ASSERT_OCL_SUCCESS(iRet, clReleaseContext);
}
