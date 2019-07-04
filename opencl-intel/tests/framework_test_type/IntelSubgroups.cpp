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
#include "cl_types.h"
#include "FrameworkTest.h"
#include "cl_device_api.h"
#include "cl_cpu_detect.h"
#include "common_utils.h"

extern cl_device_type gDeviceType;

using namespace Intel::OpenCL::Utils;

cl_platform_id NativeSubgroups::m_platform;
cl_device_id NativeSubgroups::m_device;
cl_context NativeSubgroups::m_context;
cl_command_queue NativeSubgroups::m_queue;

void NativeSubgroups::SetUpTestCase()
{
    if (!SETENV("CL_CONFIG_CPU_ENABLE_NATIVE_SUBGROUPS", "True")) {
        std::cout << "ERROR: Can't set environment variables. Test FAILED";
        FAIL();
        return;
    }

    cl_int iRet = CL_SUCCESS;
    m_platform = 0;
    m_device = NULL;
    m_context = NULL;

    //Get platform.
    iRet = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetPlatformIDs failed.";

    //Get device.
    iRet = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << " clGetDeviceIDs failed on trying to obtain " << gDeviceType << " device type.";

    //Create context.
    const cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)m_platform, 0 };
    m_context = clCreateContext(prop, 1, &m_device, NULL, NULL, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateContext failed. ";

    m_queue = clCreateCommandQueueWithProperties(m_context, m_device, 0, &iRet);
    CheckException("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);
}

void NativeSubgroups::SetUp()
{
    const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
    std::cout << "=============================================================" << std::endl;
    std::cout << test_info->test_case_name() << "." << test_info->name() << std::endl;
    std::cout << "=============================================================" << std::endl;

    Init();
}

void NativeSubgroups::TearDown()
{
    clFinish(m_queue);
}

void NativeSubgroups::TearDownTestCase()
{
    if (!UNSETENV("CL_CONFIG_CPU_ENABLE_NATIVE_SUBGROUPS"))
    {
        std::cout << "ERROR: Can't unset environment variables. Test FAILED";
        FAIL();
        return;
    }
}

void NativeSubgroups::Init()
{
    m_platform = GetPlatform();
    m_device   = GetDeviceID();
    m_context  = GetContext();
    m_queue    = GetQueue();
}

void NativeSubgroups::GetDummySubgroupKernel(cl_kernel& kern) const
{
    const char* kernel = "\
                          __kernel void dummy_kernel(__global uint* ptr)\
                          {\
                                size_t gid = get_global_id(0);\
                                uint max_sg_size = get_max_sub_group_size();\
                                ptr[gid] = max_sg_size;\
                          }\
                          ";
    const size_t kernel_size = strlen(kernel);
    cl_int iRet = CL_SUCCESS;

    cl_program program = clCreateProgramWithSource(m_context, 1, &kernel, &kernel_size, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed. ";

    iRet = clBuildProgram(program, 0, nullptr, "", nullptr, nullptr);
    if( CL_SUCCESS != iRet )
    {
        std::string log("", 1000);
        clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG, log.size(), &log[0], nullptr);
        std::cout << log << std::endl;
    }
    ASSERT_EQ(CL_SUCCESS, iRet) << " clBuildProgram failed. ";

    kern = clCreateKernel(program, "dummy_kernel", &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateKernel failed. ";
}

void NativeSubgroups::NativeSubgroups_MAX_SB_SIZE() const
{
    cl_int iRet = CL_SUCCESS;

    cl_kernel kern = nullptr;
    GetDummySubgroupKernel(kern);

    {// local work size is [32,1,1]
        size_t dummy_ls_vec[] = {32, 1, 1};
        std::vector<size_t> local_work_sizes(dummy_ls_vec, dummy_ls_vec + sizeof(dummy_ls_vec) / sizeof(size_t));
        size_t max_SG_size = 0;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       sizeof(max_SG_size),
                                       &max_SG_size,
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ(sizeof(size_t), returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
        ASSERT_TRUE(((size_t)16 == max_SG_size) ||
                    ((size_t)8  == max_SG_size) ||
                    ((size_t)4  == max_SG_size))
            << " clGetKernelSubGroupInfo failed."
            << " Max subgroup size can't be other than 4, 8, 16. "
            << " Actual value is " << max_SG_size << "."; 
    }

    {// local work sizes is [20,20,20]
        size_t dummy_vec[] = {20, 20, 20};
        std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
        size_t max_SG_size = 0;
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       m_device,
                                       CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       sizeof(max_SG_size),
                                       &max_SG_size,
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ(sizeof(size_t), returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
        ASSERT_TRUE(((size_t)16 == max_SG_size) ||
                    ((size_t)8  == max_SG_size) ||
                    ((size_t)4  == max_SG_size))
            << " clGetKernelSubGroupInfo failed."
            << " Max subgroup size can't be other than 4, 8, 16. "
            << " Actual value is " << max_SG_size << ".";
    }
}

static void CheckSGCount(
    cl_device_id device,
    cl_kernel kern,
    const std::vector<size_t>& local_work_sizes)
{
    cl_int iRet = CL_SUCCESS;

    ASSERT_EQ(3, local_work_sizes.size()) <<
       "Expect local sizes as 3 component vector";

    size_t max_SG_size = 0;
    {
        size_t returned_size = 0;
        iRet = clGetKernelSubGroupInfo(kern,
                                       device,
                                       CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
                                       local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                       &local_work_sizes[0],
                                       sizeof(max_SG_size),
                                       &max_SG_size,
                                       &returned_size);
        ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
        ASSERT_EQ(sizeof(size_t), returned_size)
            << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
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
    iRet = clGetKernelSubGroupInfo(kern,
                                   device,
                                   CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE,
                                   local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                   &local_work_sizes[0],
                                   sizeof(actual_SG_count),
                                   &actual_SG_count,
                                   &returned_size);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
    ASSERT_EQ(sizeof(size_t), returned_size)
        << " clGetKernelSubGroupInfo failed. Expected and returned size differ. ";
    ASSERT_EQ(expected_SG_count, actual_SG_count)
        << " clGetKernelSubGroupInfo failed. SG count is not valid.";
}

void NativeSubgroups::NativeSubgroups_SG_COUNT() const
{
   cl_kernel kern = nullptr;
   GetDummySubgroupKernel(kern);

   {// local work size is [32,1,1]
       size_t dummy_lsz_vec[] = {32, 1, 1};
       std::vector<size_t> local_work_sizes(
           dummy_lsz_vec, dummy_lsz_vec + sizeof(dummy_lsz_vec) / sizeof(size_t));

       CheckSGCount(m_device, kern, local_work_sizes);
   }

   {// local work size is [33,1,1]
       size_t dummy_lsz_vec[] = {33, 1, 1};
       std::vector<size_t> local_work_sizes(
           dummy_lsz_vec, dummy_lsz_vec + sizeof(dummy_lsz_vec) / sizeof(size_t));

       CheckSGCount(m_device, kern, local_work_sizes);
   }

   {// local work size is [33,33,33]
       size_t dummy_lsz_vec[] = {33, 33, 33};
       std::vector<size_t> local_work_sizes(
           dummy_lsz_vec, dummy_lsz_vec + sizeof(dummy_lsz_vec) / sizeof(size_t));

       CheckSGCount(m_device, kern, local_work_sizes);
   }
}

void NativeSubgroups::NativeSubgroups_LOCAL_SIZE_FOR_SG_COUNT() const
{
   cl_int iRet = CL_SUCCESS;

   cl_kernel kern = nullptr;
   GetDummySubgroupKernel(kern);

   // obtain VF for the kernel.
   size_t max_SG_size = 0;
   {
       size_t dummy_vec[] = {1, 1, 1};
       std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
       size_t returned_size = 0;
       iRet = clGetKernelSubGroupInfo(kern,
                                      m_device,
                                      CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE,
                                      local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                      &local_work_sizes[0],
                                      sizeof(max_SG_size),
                                      &max_SG_size,
                                      &returned_size);
       ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
       ASSERT_EQ(sizeof(size_t), returned_size)
           << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
   }

   {// Desired SG count is 10 
       size_t dummy_vec[] = {1, 1, 1};
       std::vector<size_t> local_work_sizes(dummy_vec, dummy_vec + sizeof(dummy_vec) / sizeof(size_t));
       
       size_t desired_SG_count = 10;
       size_t returned_size = 0;
       iRet = clGetKernelSubGroupInfo(kern,
                                      m_device,
                                      CL_KERNEL_LOCAL_SIZE_FOR_SUB_GROUP_COUNT,
                                      sizeof(desired_SG_count),
                                      &desired_SG_count,
                                      local_work_sizes.size() * sizeof(local_work_sizes[0]),
                                      &local_work_sizes[0],
                                      &returned_size);
       ASSERT_EQ(CL_SUCCESS, iRet) << " clGetKernelSubGroupInfo failed. ";
       ASSERT_EQ(sizeof(size_t) * local_work_sizes.size(), returned_size)
           << " clGetKernelSubGroupInfo failed. Expected and returned sizes differ. ";
       size_t expected = max_SG_size * desired_SG_count;
       ASSERT_EQ(expected, local_work_sizes[0])
           << " clGetKernelSubGroupInfo failed. Expected and actual values differ. ";
   }
}
