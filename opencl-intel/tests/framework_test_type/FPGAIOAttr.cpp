//==--- FPGAIOAttr.cpp - Tests for io pipes.  -*- OpenCL C -*-==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include <gtest/gtest.h>
#include <CL/cl.h>
#include <CL/cl_ext.h>

#include <algorithm>
#include <vector>
#include <cstdlib>

#include "FrameworkTest.h"

#define SIZE 10

extern cl_device_type gDeviceType;

class FPGAIOAttrTest : public ::testing::Test {
protected:
    virtual void SetUp();
    virtual void TearDown();

    std::vector<int> m_inArr;
    cl_platform_id   m_platform;
    cl_device_id     m_device;
    cl_context       m_context;
    cl_command_queue m_queue;

    cl_program m_program;
    cl_kernel  m_kernel;
};

void FPGAIOAttrTest::SetUp()
{
    cl_int iRet = CL_SUCCESS;
    iRet = clGetPlatformIDs(/*num_entries=*/1, &m_platform,
                            /*num_platforms=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clGetPlainArr failed.";

    iRet = clGetDeviceIDs(m_platform, gDeviceType, /*num_entries=*/1,
                          &m_device, /*num_devices=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << "clGetDeviceIDs failed on trying to obtain "
        << gDeviceType << " device type.";

    const cl_context_properties prop[5] = { CL_CONTEXT_PLATFORM,
                                            (cl_context_properties)m_platform,
                                            CL_CONTEXT_FPGA_EMULATOR_INTEL,
                                            CL_TRUE,
                                            0 };
    m_context = clCreateContext(prop,
                                /*num_devices=*/1, &m_device,
                                /*pfn_notify=*/nullptr,
                                /*user_data=*/nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateContext failed.";

    m_queue =
      clCreateCommandQueueWithProperties(m_context, m_device,
                                         /*properties=*/nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateCommandQueueWithProperties failed.";

    m_inArr = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    FILE *test_in = fopen("test_in", "wb");
    EXPECT_TRUE(test_in != nullptr) << "Cannot create input file";
    size_t write = fwrite(&m_inArr[0], sizeof(int), SIZE, test_in);
    ASSERT_EQ(write, SIZE) << "didn't write all elements!";
    fclose(test_in);
}

void FPGAIOAttrTest::TearDown()
{
    clReleaseKernel(m_kernel);
    clReleaseProgram(m_program);
    clReleaseCommandQueue(m_queue);
    clReleaseContext(m_context);
    clReleaseDevice(m_device);
}

#ifdef BUILD_FPGA_EMULATOR
TEST_F(FPGAIOAttrTest, IOChannels)
{
    const char* programSource = "                              \n\
#pragma OPENCL EXTENSION cl_intel_channels : enable            \n\
                                                               \n\
channel int ichIn __attribute__((io(\"test_in\")));            \n\
channel int ichOut __attribute__((io(\"test_out\")));          \n\
                                                               \n\
__kernel void channel_io() {                                   \n\
  for (int i = 0; i < 10; ++i) {                               \n\
    int data = read_channel_intel(ichIn);                      \n\
    write_channel_intel(ichOut, data);                         \n\
  }                                                            \n\
}                                                              \n\
        ";

    cl_int iRet = CL_SUCCESS;
    m_program = clCreateProgramWithSource(m_context, /*count=*/1,
                                          &programSource,
                                          /*length=*/nullptr,
                                          &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateProgramWithSource failed.";

    iRet = clBuildProgram(m_program, /*num_devices=*/0, /*device_list=*/nullptr,
                          /*options=*/"", /*pfn_notify*/nullptr,
                          /*user_data=*/nullptr);
    if(CL_SUCCESS != iRet)
    {
        size_t logSize = 0;
        clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG,
                              /*param_value_size=*/0, /*param_value=*/nullptr,
                              &logSize);
        std::string log("", logSize);
        clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG,
                              log.size(), &log[0], /*value_size_ret=*/nullptr);

        FAIL() << "Build failed\n"
               << "Source:\n"
               << "---------\n"
               << programSource
               << "---------\n"
               << log;
    }

    m_kernel = clCreateKernel(m_program, "channel_io", &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateKernel failed.";

    iRet = clEnqueueTask(m_queue, m_kernel, 0, nullptr, nullptr);
    EXPECT_EQ(iRet, CL_SUCCESS);

    clFinish(m_queue);

    FILE *test_out = fopen("test_out", "rb");
    EXPECT_TRUE(test_out != nullptr) << "Cannot open out file for reading";
    int outArr[SIZE];
    int read = fread(outArr, sizeof(int), SIZE, test_out);
    ASSERT_GT(read, 0) << "Read 0 bytes from file";
    fclose(test_out);
    for (int i = 0; i < SIZE; ++i) {
        ASSERT_EQ(m_inArr[i], outArr[i]);
    }
}

TEST_F(FPGAIOAttrTest, IOPipes)
{
    const char* programSource = "                                               \n\
__kernel void pipe_io(read_only pipe int p1 __attribute__((io(\"test_in\"))),   \n\
                      write_only pipe int p2 __attribute__((io(\"test_out\")))) \n\
{                                                                               \n\
  for (int i = 0; i < 10; ++i) {                                                \n\
    int data;                                                                   \n\
    int ret = read_pipe(p1, &data);                                             \n\
    if (ret > 0)                                                                \n\
      write_pipe(p2, &data);                                                    \n\
  }                                                                             \n\
}                                                                               \n\
        ";

    cl_int iRet = CL_SUCCESS;
    m_program = clCreateProgramWithSource(m_context, /*count=*/1,
                                          &programSource,
                                          /*length=*/nullptr,
                                          &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateProgramWithSource failed.";

    iRet = clBuildProgram(m_program, /*num_devices=*/0, /*device_list=*/nullptr,
                          /*options=*/"", /*pfn_notify*/nullptr,
                          /*user_data=*/nullptr);
    if(CL_SUCCESS != iRet)
    {
        size_t logSize = 0;
        clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG,
                              /*param_value_size=*/0, /*param_value=*/nullptr,
                              &logSize);
        std::string log("", logSize);
        clGetProgramBuildInfo(m_program, m_device, CL_PROGRAM_BUILD_LOG,
                              log.size(), &log[0], /*value_size_ret=*/nullptr);

        FAIL() << "Build failed\n"
               << "Source:\n"
               << "---------\n"
               << programSource
               << "---------\n"
               << log;
    }

    m_kernel = clCreateKernel(m_program, "pipe_io", &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateKernel failed.";

    cl_mem readPipe =
        clCreatePipe(m_context, 0, sizeof(cl_int), SIZE, nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << "clCreatePipe(CL_MEM_HOST_READ_ONLY) failed.";

    cl_mem writePipe =
        clCreatePipe(m_context, 0, sizeof(cl_int), SIZE, nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << "clCreatePipe(CL_MEM_HOST_WRITE_ONLY) failed.";

    iRet = clSetKernelArg(m_kernel, 0, sizeof(readPipe), &readPipe);
    ASSERT_EQ(iRet, CL_SUCCESS);

    iRet = clSetKernelArg(m_kernel, 1, sizeof(writePipe), &writePipe);
    ASSERT_EQ(iRet, CL_SUCCESS);

    iRet = clEnqueueTask(m_queue, m_kernel, 0, nullptr, nullptr);
    ASSERT_EQ(iRet, CL_SUCCESS);

    clFinish(m_queue);

    clReleaseMemObject(writePipe);
    clReleaseMemObject(readPipe);

    FILE *test_out = fopen("test_out", "rb");
    EXPECT_TRUE(test_out != nullptr) << "Cannot open out file for reading";
    int outArr[SIZE];
    int read = fread(outArr, sizeof(int), SIZE, test_out);
    ASSERT_GT(read, 0) << "Read 0 bytes from file";
    fclose(test_out);
    for (int i = 0; i < SIZE; ++i) {
        ASSERT_EQ(m_inArr[i], outArr[i]);
    }
}
#endif // BUILD_FPGA_EMULATOR
