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

#include "SGEmulationTest.h"

TEST_P(SGEmulationTest, MultiKernelTIDResolution) {

  const char *kernel =
      "size_t foo() {"
      "  return get_local_id(0);"
      "}"
      "__kernel void basic(__global unsigned long* local_id) {"
      "  size_t lid = foo();"
      "  local_id[lid] = lid;"
      "}"
      "__kernel void basic1(__global unsigned long* local_id) {"
      "  size_t lid = foo();"
      "  sub_group_barrier(CLK_LOCAL_MEM_FENCE);"
      "  local_id[lid] = lid;"
      "}";
  const size_t kernel_size = strlen(kernel);
  cl_int iRet = CL_SUCCESS;

  cl_program program =
      clCreateProgramWithSource(m_context, 1, &kernel, &kernel_size, &iRet);
  ASSERT_OCL_SUCCESS(iRet, " clCreateProgramWithSource");

  iRet = clBuildProgram(program, 0, nullptr, "", nullptr, nullptr);
  if (CL_SUCCESS != iRet) {
    std::string log;
    ASSERT_NO_FATAL_FAILURE(GetBuildLog(m_device, program, log));
    FAIL() << log;
  }

  cl_kernel kern = clCreateKernel(program, "basic", &iRet);
  ASSERT_OCL_SUCCESS(iRet, " clCreateKernel");
  cl_kernel kern1 = clCreateKernel(program, "basic1", &iRet);
  ASSERT_OCL_SUCCESS(iRet, " clCreateKernel");

  constexpr size_t gsize = 16;
  constexpr size_t lsize = 16;

  size_t max_sg_size = 0;
  iRet = clGetKernelSubGroupInfo(
      kern, m_device, CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE, sizeof(lsize),
      &lsize, sizeof(max_sg_size), &max_sg_size, nullptr);
  ASSERT_OCL_SUCCESS(iRet, " clGetKernelSubGroupInfo");

  cl_ulong lid[lsize] = {0};
  cl_mem mem_obj_lid = clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR,
                                      sizeof(cl_ulong) * lsize, lid, &iRet);
  ASSERT_OCL_SUCCESS(iRet, " clCreateBuffer");

  iRet = clSetKernelArg(kern, 0, sizeof(cl_mem), &mem_obj_lid);
  ASSERT_OCL_SUCCESS(iRet, " clSetKernelArg");

  cl_ulong lid1[lsize] = {0};
  cl_mem mem_obj_lid1 = clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR,
                                       sizeof(cl_ulong) * lsize, lid1, &iRet);
  ASSERT_OCL_SUCCESS(iRet, " clCreateBuffer");

  iRet = clSetKernelArg(kern1, 0, sizeof(cl_mem), &mem_obj_lid1);
  ASSERT_OCL_SUCCESS(iRet, " clSetKernelArg");

  iRet = clEnqueueNDRangeKernel(m_queue, kern, 1, nullptr, &gsize, &lsize, 0,
                                nullptr, nullptr);
  ASSERT_OCL_SUCCESS(iRet, " clEnqueueNDRangeKernel");
  iRet = clEnqueueNDRangeKernel(m_queue, kern1, 1, nullptr, &gsize, &lsize, 0,
                                nullptr, nullptr);
  ASSERT_OCL_SUCCESS(iRet, " clEnqueueNDRangeKernel");

  iRet = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(iRet, " clFinish");

  for (size_t i = 0; i < lsize; ++i) {
    ASSERT_EQ(i, lid[i]) << "Mismatch at work-item: " << i << " Expect: " << i
                         << " Result: " << lid[i];
    ASSERT_EQ(i, lid1[i]) << "Mismatch at work-item: " << i << " Expect: " << i
                          << " Result: " << lid1[i];
  }

  iRet = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(iRet, " clReleaseProgram");
  iRet = clReleaseKernel(kern);
  ASSERT_OCL_SUCCESS(iRet, " clReleaseKernel");
  iRet = clReleaseKernel(kern1);
  ASSERT_OCL_SUCCESS(iRet, " clReleaseKernel");
  iRet = clReleaseMemObject(mem_obj_lid);
  ASSERT_OCL_SUCCESS(iRet, " clReleaseMemObject");
  iRet = clReleaseMemObject(mem_obj_lid1);
  ASSERT_OCL_SUCCESS(iRet, " clReleaseMemObject");
}
