//==--- ZeroLocalSize.cpp - Test on choosing optimal local size-*- C++ -*---==//
////
//// Copyright (C) 2015 Intel Corporation. All rights reserved.
////
//// The information and source code contained herein is the exclusive property
//// of Intel Corporation and may not be disclosed, examined or reproduced in
//// whole or in part without explicit written authorization from the company.
////
//// ===--------------------------------------------------------------------= //
#include "CL.h"
#include "CL/cl_ext.h"

TEST_F(CL, DISABLED_ZeroLocalSize) {
  cl_int iRet = CL_SUCCESS;

  const char *src = "__kernel void memcpy(__global uint* localSize) \
        { \
            for(int i = 0; i < 3; ++i) \
                localSize[i] = (uint)get_local_size(i); \
        }";

  // Create queue that will execute all the job using master thread only.
  cl_queue_properties props[] = {CL_QUEUE_PROPERTIES,
                                 CL_QUEUE_THREAD_LOCAL_EXEC_ENABLE_INTEL, 0};
  cl_command_queue immediateQueue =
      clCreateCommandQueueWithProperties(m_context, m_device, props, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateCommandQueueWithProperties failed. ";

  cl_program program =
      clCreateProgramWithSource(m_context, 1, &src, nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed. ";

  iRet = clBuildProgram(program, /*num_devices=*/0,
                        /*device_list=*/nullptr, /*options=*/"",
                        /*pfn_notify=*/nullptr, /*user_data=*/nullptr);
  if (CL_SUCCESS != iRet) {
    std::string log("\0", 1000);
    clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG, log.size(),
                          &log[0], nullptr);
    std::cout << log << std::endl;
  }
  ASSERT_EQ(CL_SUCCESS, iRet) << " clBuildProgram failed. ";

  cl_kernel kern = clCreateKernel(program, "memcpy", &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateKernel failed. ";

  cl_mem memLocalSize = clCreateBuffer(m_context, CL_MEM_WRITE_ONLY,
                                       /*size=*/sizeof(cl_uint) * 3,
                                       /*host_ptr=*/nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateBuffer failed. ";

  iRet = clSetKernelArg(kern, /*arg_index=*/0, sizeof(memLocalSize),
                        &memLocalSize);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clSetKernelArg failed. ";

  const std::vector<size_t> globalSize = {1000, 1000, 1000};
  const cl_uint workDim = 3;
  iRet = clEnqueueNDRangeKernel(immediateQueue, kern, workDim,
                                /*global_work_offset=*/nullptr, &globalSize[0],
                                /*local_work_size=*/nullptr,
                                /*num_events_in_wait_list=*/0,
                                /*event_wait_list=*/nullptr,
                                /*event=*/nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueNDRangeKernel failed. ";

  cl_uint *localSize = (cl_uint *)clEnqueueMapBuffer(
      immediateQueue, memLocalSize,
      /*blocking_map=*/CL_TRUE, CL_MAP_READ, /*offset=*/0,
      /*size=*/sizeof(cl_uint) * 3,
      /*num_events_in_wait_list=*/0,
      /*event_wait_list=*/nullptr,
      /*event=*/nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueMapBuffer failed. ";

  for (cl_uint i = 0; i < workDim; ++i)
    ASSERT_EQ(globalSize[i], localSize[i])
        << "localSize[" << i << "] is not extended!";
}
