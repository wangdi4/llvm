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

#include "IntelSubgroups.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include "common_utils.h"

using namespace Intel::OpenCL::Utils;

void SubgroupsTest::GetDummySubgroupKernel(cl_kernel &kern) const {
  const char *kernel = "__kernel void dummy_kernel(__global uint* ptr)\
                          {\
                                size_t gid = get_global_id(0);\
                                uint max_sg_size = get_max_sub_group_size();\
                                ptr[gid] = max_sg_size;\
                          }\
                          ";
  const size_t kernel_size = strlen(kernel);
  cl_int iRet = CL_SUCCESS;

  cl_program program =
      clCreateProgramWithSource(m_context, 1, &kernel, &kernel_size, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed. ";

  iRet = clBuildProgram(program, 0, nullptr, "", nullptr, nullptr);
  if (CL_SUCCESS != iRet) {
    std::string log;
    ASSERT_NO_FATAL_FAILURE(GetBuildLog(m_device, program, log));
    FAIL() << log;
  }

  kern = clCreateKernel(program, "dummy_kernel", &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateKernel failed. ";
}

void SubgroupsTest::CheckSGCount(cl_device_id device, cl_kernel kern,
                                 const std::vector<size_t> &local_work_sizes) {
  cl_int iRet = CL_SUCCESS;

  ASSERT_EQ(3, local_work_sizes.size())
      << "Expect local sizes as 3 component vector";

  size_t max_SG_size = 0;
  {
    size_t returned_size = 0;
    iRet = clGetKernelSubGroupInfo(
        kern, device, CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
        local_work_sizes.size() * sizeof(local_work_sizes[0]),
        &local_work_sizes[0], sizeof(max_SG_size), &max_SG_size,
        &returned_size);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
    ASSERT_EQ(sizeof(size_t), returned_size)
        << " clGetKernelSubGroupInfo failed. Expected and returned sizes "
           "differ. ";
  }

  size_t actual_SG_count = 0; // initialize with zero.
  size_t returned_size = 0;
  // test kernel is one-dimensional, so assume that it will
  // be always vectorized along 0-dimension.
  // The following calculation routine will allow for extra
  // subgroup when WGSize % VF != 0.
  size_t expected_SG_count = ((local_work_sizes[0] - 1) / max_SG_size) + 1;
  expected_SG_count *= local_work_sizes[1];
  expected_SG_count *= local_work_sizes[2];
  iRet = clGetKernelSubGroupInfo(
      kern, device, CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE,
      local_work_sizes.size() * sizeof(local_work_sizes[0]),
      &local_work_sizes[0], sizeof(actual_SG_count), &actual_SG_count,
      &returned_size);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
  ASSERT_EQ(sizeof(size_t), returned_size)
      << " clGetKernelSubGroupInfo failed. Expected and returned size differ. ";
  ASSERT_EQ(expected_SG_count, actual_SG_count)
      << " clGetKernelSubGroupInfo failed. SG count is not valid.";
}

TEST_F(SubgroupsTest, Subgroups_MAX_SB_SIZE) {
  cl_int iRet = CL_SUCCESS;

  cl_kernel kern = nullptr;
  GetDummySubgroupKernel(kern);

  { // local work size is [32,1,1]
    size_t dummy_ls_vec[] = {32, 1, 1};
    std::vector<size_t> local_work_sizes(
        dummy_ls_vec, dummy_ls_vec + sizeof(dummy_ls_vec) / sizeof(size_t));
    size_t max_SG_size = 0;
    size_t returned_size = 0;
    iRet = clGetKernelSubGroupInfo(
        kern, m_device, CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
        local_work_sizes.size() * sizeof(local_work_sizes[0]),
        &local_work_sizes[0], sizeof(max_SG_size), &max_SG_size,
        &returned_size);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
    ASSERT_EQ(sizeof(size_t), returned_size)
        << " clGetKernelSubGroupInfo failed. Expected and returned sizes "
           "differ. ";
    ASSERT_TRUE(((size_t)16 == max_SG_size) || ((size_t)8 == max_SG_size) ||
                ((size_t)4 == max_SG_size))
        << " clGetKernelSubGroupInfo failed."
        << " Max subgroup size can't be other than 4, 8, 16. "
        << " Actual value is " << max_SG_size << ".";
  }

  { // local work sizes is [20,20,20]
    size_t dummy_vec[] = {20, 20, 20};
    std::vector<size_t> local_work_sizes(
        dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
    size_t max_SG_size = 0;
    size_t returned_size = 0;
    iRet = clGetKernelSubGroupInfo(
        kern, m_device, CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
        local_work_sizes.size() * sizeof(local_work_sizes[0]),
        &local_work_sizes[0], sizeof(max_SG_size), &max_SG_size,
        &returned_size);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
    ASSERT_EQ(sizeof(size_t), returned_size)
        << " clGetKernelSubGroupInfo failed. Expected and returned sizes "
           "differ. ";
    ASSERT_TRUE(((size_t)16 == max_SG_size) || ((size_t)8 == max_SG_size) ||
                ((size_t)4 == max_SG_size))
        << " clGetKernelSubGroupInfo failed."
        << " Max subgroup size can't be other than 4, 8, 16. "
        << " Actual value is " << max_SG_size << ".";
  }
}

TEST_F(SubgroupsTest, Subgroups_SG_COUNT) {
  cl_kernel kern = nullptr;
  GetDummySubgroupKernel(kern);

  { // local work size is [32,1,1]
    size_t dummy_lsz_vec[] = {32, 1, 1};
    std::vector<size_t> local_work_sizes(
        dummy_lsz_vec, dummy_lsz_vec + sizeof(dummy_lsz_vec) / sizeof(size_t));

    CheckSGCount(m_device, kern, local_work_sizes);
  }

  { // local work size is [33,1,1]
    size_t dummy_lsz_vec[] = {33, 1, 1};
    std::vector<size_t> local_work_sizes(
        dummy_lsz_vec, dummy_lsz_vec + sizeof(dummy_lsz_vec) / sizeof(size_t));

    CheckSGCount(m_device, kern, local_work_sizes);
  }

  { // local work size is [33,33,33]
    size_t dummy_lsz_vec[] = {33, 33, 33};
    std::vector<size_t> local_work_sizes(
        dummy_lsz_vec, dummy_lsz_vec + sizeof(dummy_lsz_vec) / sizeof(size_t));

    CheckSGCount(m_device, kern, local_work_sizes);
  }
}

TEST_F(SubgroupsTest, Subgroups_LOCAL_SIZE_FOR_SG_COUNT) {
  cl_int iRet = CL_SUCCESS;

  cl_kernel kern = nullptr;
  GetDummySubgroupKernel(kern);

  // obtain VF for the kernel.
  size_t max_SG_size = 0;
  {
    size_t dummy_vec[] = {1, 1, 1};
    std::vector<size_t> local_work_sizes(
        dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
    size_t returned_size = 0;
    iRet = clGetKernelSubGroupInfo(
        kern, m_device, CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
        local_work_sizes.size() * sizeof(local_work_sizes[0]),
        &local_work_sizes[0], sizeof(max_SG_size), &max_SG_size,
        &returned_size);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
    ASSERT_EQ(sizeof(size_t), returned_size)
        << " clGetKernelSubGroupInfo failed. Expected and returned sizes "
           "differ. ";
  }

  { // Desired SG count is 10
    size_t dummy_vec[] = {1, 1, 1};
    std::vector<size_t> local_work_sizes(
        dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));

    size_t desired_SG_count = 10;
    size_t returned_size = 0;
    iRet = clGetKernelSubGroupInfo(
        kern, m_device, CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
        sizeof(desired_SG_count), &desired_SG_count,
        local_work_sizes.size() * sizeof(local_work_sizes[0]),
        &local_work_sizes[0], &returned_size);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
    ASSERT_EQ(sizeof(size_t) * local_work_sizes.size(), returned_size)
        << " clGetKernelSubGroupInfo failed. Expected and returned sizes "
           "differ. ";
    size_t expected = max_SG_size * desired_SG_count;
    ASSERT_EQ(expected, local_work_sizes[0])
        << " clGetKernelSubGroupInfo failed. Expected and actual values "
           "differ. ";
  }
}
