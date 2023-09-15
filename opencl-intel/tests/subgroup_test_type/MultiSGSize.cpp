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

// This test is check if multiple subgroup emulation size is supported.
TEST_P(SGEmulationTest, MultiSGSize) {

  const char *kernel = "__attribute__((noinline))"
                       "int foo(int lid) {"
                       "  return sub_group_scan_inclusive_add(lid);"
                       "}"
                       "__attribute__((intel_reqd_sub_group_size(4)))"
                       "__kernel void basic4(__global int* scan) {"
                       "  int lid = get_local_id(0);"
                       "  scan[lid] = foo(lid);"
                       "}"
                       "__attribute__((intel_reqd_sub_group_size(8)))"
                       "__kernel void basic8(__global int* scan) {"
                       "  int lid = get_local_id(0);"
                       "  scan[lid] = foo(lid);"
                       "}"
                       "__attribute__((intel_reqd_sub_group_size(16)))"
                       "__kernel void basic16(__global int* scan) {"
                       "  int lid = get_local_id(0);"
                       "  scan[lid] = foo(lid);"
                       "}"
                       "__attribute__((intel_reqd_sub_group_size(32)))"
                       "__kernel void basic32(__global int* scan) {"
                       "  int lid = get_local_id(0);"
                       "  scan[lid] = foo(lid);"
                       "}"
                       "__attribute__((intel_reqd_sub_group_size(64)))"
                       "__kernel void basic64(__global int* scan) {"
                       "  int lid = get_local_id(0);"
                       "  scan[lid] = foo(lid);"
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

  std::vector<std::string> kernel_names = {"basic4", "basic8", "basic16",
                                           "basic32", "basic64"};
  std::vector<int> subgroup_sizes = {4, 8, 16, 32, 64};
  std::vector<cl_kernel> kernels;
  for (size_t i = 0; i < kernel_names.size(); i++) {
    cl_kernel kernel = clCreateKernel(program, kernel_names[i].c_str(), &iRet);
    ASSERT_OCL_SUCCESS(iRet, " clCreateKernel");
    kernels.push_back(kernel);
  }

  constexpr size_t gsize = 128;
  constexpr size_t lsize = 128;
  std::vector<std::array<cl_int, lsize>> scan(kernels.size());
  std::vector<cl_mem> mem_obj_scan(kernels.size());
  for (size_t i = 0; i < kernels.size(); i++) {
    for (size_t j = 0; j < scan[i].size(); j++) {
      scan[i][j] = 0;
    }

    mem_obj_scan[i] =
        clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, sizeof(cl_int) * lsize,
                       scan[i].data(), &iRet);
    ASSERT_OCL_SUCCESS(iRet, " clCreateBuffer");
    iRet = clSetKernelArg(kernels[i], 0, sizeof(cl_mem), &mem_obj_scan[i]);
    ASSERT_OCL_SUCCESS(iRet, " clSetKernelArg");
    iRet = clEnqueueNDRangeKernel(m_queue, kernels[i], 1, nullptr, &gsize,
                                  &lsize, 0, nullptr, nullptr);
    ASSERT_OCL_SUCCESS(iRet, " clEnqueueNDRangeKernel");
  }

  iRet = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(iRet, " clFinish");

  for (size_t i = 0; i < subgroup_sizes.size(); i++) {
    cl_int ref = 0;
    for (size_t j = 0; j < lsize; ++j) {
      if (j % subgroup_sizes[i] == 0)
        ref = 0;
      ref += j;
      ASSERT_EQ(ref, scan[i][j])
          << "Mismatch at work-item: " << j
          << " for subgroup size: " << subgroup_sizes[i]
          << ".\n  Expect: " << ref << " Result: " << scan[i][j];
    }
  }

  iRet = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(iRet, " clReleaseProgram");
  for (size_t i = 0; i < kernels.size(); i++) {
    iRet = clReleaseKernel(kernels[i]);
    ASSERT_OCL_SUCCESS(iRet, " clReleaseKernel");
    iRet = clReleaseMemObject(mem_obj_scan[i]);
    ASSERT_OCL_SUCCESS(iRet, " clReleaseMemObject");
  }
}
