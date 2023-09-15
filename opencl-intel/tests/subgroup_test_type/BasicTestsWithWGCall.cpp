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

TEST_P(SGEmulationTest, BasicTestsWithWGCall) {

  const char *kernel = "__kernel void basic(__global int* scan_add, __global "
                       "int* wg_reduce_add) {"
                       "  int lid = get_local_id(0);"
                       "  wg_reduce_add[lid] = work_group_reduce_add(lid);"
                       "  scan_add[lid] = sub_group_scan_inclusive_add(lid);"
                       "}";
  const size_t kernel_size = strlen(kernel);
  cl_int iRet = CL_SUCCESS;

  cl_program program =
      clCreateProgramWithSource(m_context, 1, &kernel, &kernel_size, &iRet);
  ASSERT_OCL_SUCCESS(iRet, " clCreateProgramWithSource");

  iRet = clBuildProgram(program, 0, nullptr, "-cl-std=CL2.0", nullptr, nullptr);
  if (CL_SUCCESS != iRet) {
    std::string log;
    ASSERT_NO_FATAL_FAILURE(GetBuildLog(m_device, program, log));
    FAIL() << log;
  }

  cl_kernel kern = clCreateKernel(program, "basic", &iRet);
  ASSERT_OCL_SUCCESS(iRet, " clCreateKernel");

  constexpr size_t gsize = 16;
  constexpr size_t lsize = 16;

  size_t max_sg_size = 0;
  iRet = clGetKernelSubGroupInfo(
      kern, m_device, CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE, sizeof(lsize),
      &lsize, sizeof(max_sg_size), &max_sg_size, nullptr);
  ASSERT_OCL_SUCCESS(iRet, " clGetKernelSubGroupInfo");

  cl_int scan_add[lsize] = {0};
  cl_mem mem_obj_scan = clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR,
                                       sizeof(cl_int) * lsize, scan_add, &iRet);
  ASSERT_OCL_SUCCESS(iRet, " clCreateBuffer");

  cl_int wg_reduce_add[lsize] = {0};
  cl_mem mem_obj_reduce =
      clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, sizeof(cl_int) * lsize,
                     wg_reduce_add, &iRet);
  ASSERT_OCL_SUCCESS(iRet, " clCreateBuffer");

  iRet = clSetKernelArg(kern, 0, sizeof(cl_mem), &mem_obj_scan);
  ASSERT_OCL_SUCCESS(iRet, " clSetKernelArg");

  iRet = clSetKernelArg(kern, 1, sizeof(cl_mem), &mem_obj_reduce);
  ASSERT_OCL_SUCCESS(iRet, " clSetKernelArg");

  iRet = clEnqueueNDRangeKernel(m_queue, kern, 1, nullptr, &gsize, &lsize, 0,
                                nullptr, nullptr);
  ASSERT_OCL_SUCCESS(iRet, " clEnqueueNDRangeKernel");

  iRet = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(iRet, " clFinish");

  int reduce_add_ref = 0;
  for (size_t i = 0; i < lsize; ++i)
    reduce_add_ref += i;

  for (size_t i = 0; i < lsize; ++i) {
    ASSERT_EQ(reduce_add_ref, wg_reduce_add[i])
        << "Mismatch for WG reduce add at work-item: " << i
        << " Expect: " << reduce_add_ref << " Result: " << wg_reduce_add[i];
  }

  int scan_add_ref = 0;
  for (size_t i = 0; i < lsize; ++i) {
    if (i % max_sg_size == 0)
      scan_add_ref = 0;
    scan_add_ref += i;
    ASSERT_EQ(scan_add_ref, scan_add[i])
        << "Mismatch at work-item: " << i << " Expect: " << scan_add_ref
        << " Result: " << scan_add[i];
  }

  iRet = clReleaseKernel(kern);
  ASSERT_OCL_SUCCESS(iRet, " clReleaseKernel");
  iRet = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(iRet, " clReleaseProgram");
  iRet = clReleaseMemObject(mem_obj_scan);
  ASSERT_OCL_SUCCESS(iRet, " clReleaseMemObject");
  iRet = clReleaseMemObject(mem_obj_reduce);
  ASSERT_OCL_SUCCESS(iRet, " clReleaseMemObject");
}

TEST_P(SGEmulationTest, BasicTestsWithWGCallAndSGSize) {
  const char *kernel = "__kernel "
                       "__attribute__((intel_reqd_sub_group_size(SIZE))) void"
                       " basic(__global int* scan_add, __global"
                       " int* wg_reduce_add) {"
                       "  int lid = get_local_id(0);"
                       "  wg_reduce_add[lid] = work_group_reduce_add(lid);"
                       "  scan_add[lid] = sub_group_scan_inclusive_add(lid);"
                       "}";
  const size_t kernel_size = strlen(kernel);

  for (int i = 4; i <= 64; i *= 2) {
    cl_int iRet = CL_SUCCESS;

    cl_program program =
        clCreateProgramWithSource(m_context, 1, &kernel, &kernel_size, &iRet);
    ASSERT_OCL_SUCCESS(iRet, " clCreateProgramWithSource");

    std::string options = "-cl-std=CL2.0 ";
    options += " -DSIZE=" + std::to_string(i);
    iRet =
        clBuildProgram(program, 0, nullptr, options.c_str(), nullptr, nullptr);
    if (CL_SUCCESS != iRet) {
      std::string log;
      ASSERT_NO_FATAL_FAILURE(GetBuildLog(m_device, program, log));
      FAIL() << log;
    }

    cl_kernel kern = clCreateKernel(program, "basic", &iRet);
    ASSERT_OCL_SUCCESS(iRet, " clCreateKernel");

    constexpr size_t gsize = 1024;
    constexpr size_t lsize = 64;

    size_t max_sg_size = 0;
    iRet = clGetKernelSubGroupInfo(
        kern, m_device, CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE, sizeof(lsize),
        &lsize, sizeof(max_sg_size), &max_sg_size, nullptr);
    ASSERT_OCL_SUCCESS(iRet, " clGetKernelSubGroupInfo");

    cl_int scan_add[lsize] = {0};
    cl_mem mem_obj_scan =
        clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, sizeof(cl_int) * lsize,
                       scan_add, &iRet);
    ASSERT_OCL_SUCCESS(iRet, " clCreateBuffer");

    cl_int wg_reduce_add[lsize] = {0};
    cl_mem mem_obj_reduce =
        clCreateBuffer(m_context, CL_MEM_USE_HOST_PTR, sizeof(cl_int) * lsize,
                       wg_reduce_add, &iRet);
    ASSERT_OCL_SUCCESS(iRet, " clCreateBuffer");

    iRet = clSetKernelArg(kern, 0, sizeof(cl_mem), &mem_obj_scan);
    ASSERT_OCL_SUCCESS(iRet, " clSetKernelArg");

    iRet = clSetKernelArg(kern, 1, sizeof(cl_mem), &mem_obj_reduce);
    ASSERT_OCL_SUCCESS(iRet, " clSetKernelArg");

    iRet = clEnqueueNDRangeKernel(m_queue, kern, 1, nullptr, &gsize, &lsize, 0,
                                  nullptr, nullptr);
    ASSERT_OCL_SUCCESS(iRet, " clEnqueueNDRangeKernel");

    iRet = clFinish(m_queue);
    ASSERT_OCL_SUCCESS(iRet, " clFinish");

    int reduce_add_ref = 0;
    for (size_t i = 0; i < lsize; ++i)
      reduce_add_ref += i;

    for (size_t i = 0; i < lsize; ++i) {
      ASSERT_EQ(reduce_add_ref, wg_reduce_add[i])
          << "Mismatch for WG reduce add at work-item: " << i
          << " Expect: " << reduce_add_ref << " Result: " << wg_reduce_add[i];
    }

    int scan_add_ref = 0;
    for (size_t i = 0; i < lsize; ++i) {
      if (i % max_sg_size == 0)
        scan_add_ref = 0;
      scan_add_ref += i;
      ASSERT_EQ(scan_add_ref, scan_add[i])
          << "Mismatch at work-item: " << i << " Expect: " << scan_add_ref
          << " Result: " << scan_add[i];
    }

    iRet = clReleaseKernel(kern);
    ASSERT_OCL_SUCCESS(iRet, " clReleaseKernel");
    iRet = clReleaseProgram(program);
    ASSERT_OCL_SUCCESS(iRet, " clReleaseProgram");
    iRet = clReleaseMemObject(mem_obj_scan);
    ASSERT_OCL_SUCCESS(iRet, " clReleaseMemObject");
    iRet = clReleaseMemObject(mem_obj_reduce);
    ASSERT_OCL_SUCCESS(iRet, " clReleaseMemObject");
  }
}
