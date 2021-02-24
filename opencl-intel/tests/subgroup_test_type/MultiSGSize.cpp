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

TEST_P(SGEmulationTest, DISABLED_MultiSGSize) {

  const char *kernel = "__attribute__((noinline))"
                       "int foo(int lid) {"
                       "  return sub_group_scan_inclusive_add(lid);"
                       "}"
                       "__attribute__((intel_reqd_sub_group_size(4)))"
                       "__kernel void basic(__global int* scan) {"
                       "  int lid = get_local_id(0);"
                       "  scan[lid] = foo(lid);"
                       "}"
                       "__attribute__((intel_reqd_sub_group_size(8)))"
                       "__kernel void basic1(__global int* scan) {"
                       "  int lid = get_local_id(0);"
                       "  scan[lid] = foo(lid);"
                       "}";
  const size_t kernel_size = strlen(kernel);
  cl_int iRet = CL_SUCCESS;

  cl_program program =
      clCreateProgramWithSource(m_context, 1, &kernel, &kernel_size, &iRet);
  ASSERT_OCL_SUCCESS(iRet, " clCreateProgramWithSource");

  iRet =
      clBuildProgram(program, 0, nullptr, "", nullptr, nullptr);
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

  cl_int scan[lsize] = {0};
  cl_mem mem_obj_scan = clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR,
                                       sizeof(cl_int) * lsize, scan, &iRet);
  ASSERT_OCL_SUCCESS(iRet, " clCreateBuffer");

  iRet = clSetKernelArg(kern, 0, sizeof(cl_mem), &mem_obj_scan);
  ASSERT_OCL_SUCCESS(iRet, " clSetKernelArg");

  cl_int scan1[lsize] = {0};
  cl_mem mem_obj_scan1 = clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR,
                                        sizeof(cl_int) * lsize, scan1, &iRet);
  ASSERT_OCL_SUCCESS(iRet, " clCreateBuffer");

  iRet = clSetKernelArg(kern1, 0, sizeof(cl_mem), &mem_obj_scan1);
  ASSERT_OCL_SUCCESS(iRet, " clSetKernelArg");

  iRet = clEnqueueNDRangeKernel(m_queue, kern, 1, nullptr, &gsize, &lsize, 0,
                                nullptr, nullptr);
  ASSERT_OCL_SUCCESS(iRet, " clEnqueueNDRangeKernel");
  iRet = clEnqueueNDRangeKernel(m_queue, kern1, 1, nullptr, &gsize, &lsize, 0,
                                nullptr, nullptr);
  ASSERT_OCL_SUCCESS(iRet, " clEnqueueNDRangeKernel");

  iRet = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(iRet, " clFinish");

  cl_int ref = 0;
  for (size_t i = 0; i < lsize; ++i) {
    if (i % 4 == 0)
      ref = 0;
    ref += i;
    ASSERT_EQ(ref, scan[i]) << "Mismatch at work-item: " << i
                            << " Expect: " << ref << " Result: " << scan[i];
  }
  ref = 0;
  for (size_t i = 0; i < lsize; ++i) {
    if (i % 8 == 0)
      ref = 0;
    ref += i;
    ASSERT_EQ(ref, scan1[i]) << "Mismatch at work-item: " << i
                             << " Expect: " << ref << " Result: " << scan1[i];
  }

  iRet = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(iRet, " clReleaseProgram");
  iRet = clReleaseKernel(kern);
  ASSERT_OCL_SUCCESS(iRet, " clReleaseKernel");
  iRet = clReleaseKernel(kern1);
  ASSERT_OCL_SUCCESS(iRet, " clReleaseKernel");
  iRet = clReleaseMemObject(mem_obj_scan);
  ASSERT_OCL_SUCCESS(iRet, " clReleaseMemObject");
  iRet = clReleaseMemObject(mem_obj_scan1);
  ASSERT_OCL_SUCCESS(iRet, " clReleaseMemObject");
}
