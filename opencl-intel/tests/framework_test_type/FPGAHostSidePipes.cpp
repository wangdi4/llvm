//==--- FPGAHostSidePipes.cpp - Tests for host-side pipes.  -*- OpenCL C -*-==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
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
#include <cstdlib>

#include "FrameworkTest.h"

extern cl_device_type gDeviceType;

class HostSidePipesTest : public ::testing::Test {
protected:
    virtual void SetUp();
    virtual void TearDown();

    bool runLoopbackKernel(cl_int numPackets);
    cl_int readPipe(cl_mem pipe, void* mem);
    cl_int writePipe(cl_mem pipe, const void* mem);
    cl_int mapPipe(cl_mem pipe, size_t num_bytes, void** mem);
    cl_int unmapPipe(cl_mem pipe, size_t num_bytes, void* mem);

    cl_platform_id   m_platform;
    cl_device_id     m_device;
    cl_context       m_context;
    cl_command_queue m_queue;

    cl_program m_program;
    cl_kernel  m_loopback;
    cl_mem     m_pipeRead;
    cl_mem     m_pipeWrite;

    static const char* m_program_source;
    const int m_maxBufferSize = 128;
};

const char* HostSidePipesTest::m_program_source = "                                     \n\
#pragma OPENCL EXTENSION cl_intel_fpga_host_pipe : enable                               \n\
__kernel void loopback(read_only pipe int pin __attribute__((intel_host_accessible)),   \n\
                       write_only pipe int pout __attribute__((intel_host_accessible)), \n\
                       int iters) {                                                     \n\
    for (int i = 0; i < iters; ++i) {                                                   \n\
      int val = 0;                                                                      \n\
      while (read_pipe(pin, &val)) {}                                                   \n\
      while (write_pipe(pout, &val)) {}                                                 \n\
    }                                                                                   \n\
  }                                                                                     \n\
";

void HostSidePipesTest::SetUp()
{
    cl_int iRet = CL_SUCCESS;
    iRet = clGetPlatformIDs(/*num_entries=*/1, &m_platform,
                            /*num_platforms=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clGetPlatformIDs failed.";

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

    m_program = clCreateProgramWithSource(m_context, /*count=*/1,
                                          &m_program_source,
                                          /*length=*/nullptr,
                                          &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateProgramWithSource failed.";

    iRet = clBuildProgram(m_program, /*num_devices=*/0, /*device_list=*/nullptr,
                          /*options=*/"-cl-std=CL2.0", /*pfn_notify*/nullptr,
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
               << m_program_source
               << "---------\n"
               << log;
    }

    m_loopback = clCreateKernel(m_program, "loopback", &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateKernel failed.";

    m_pipeRead = clCreatePipe(m_context, CL_MEM_HOST_READ_ONLY,
                              sizeof(cl_int), m_maxBufferSize,
                              nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreatePipe(read) failed.";

    m_pipeWrite = clCreatePipe(m_context, CL_MEM_HOST_WRITE_ONLY,
                               sizeof(cl_int), m_maxBufferSize,
                               nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreatePipe(write) failed.";
}

void HostSidePipesTest::TearDown()
{
    clReleaseMemObject(m_pipeRead);
    clReleaseMemObject(m_pipeWrite);
    clReleaseKernel(m_loopback);
    clReleaseProgram(m_program);
    clReleaseCommandQueue(m_queue);
    clReleaseContext(m_context);
    clReleaseDevice(m_device);
}

cl_int HostSidePipesTest::readPipe(cl_mem pipe, void* mem)
{
    cl_int error = CL_SUCCESS;
    do
    {
        error = clReadPipeIntelFPGA(pipe, mem);
    }
    while (error == CL_OUT_OF_RESOURCES);

    return error;
}

cl_int HostSidePipesTest::writePipe(cl_mem pipe, const void* mem)
{
    cl_int error = CL_SUCCESS;
    do
    {
        error = clWritePipeIntelFPGA(pipe, mem);
    }
    while (error == CL_OUT_OF_RESOURCES);

    return error;
}

cl_int HostSidePipesTest::mapPipe(cl_mem pipe, size_t numBytes, void** mem)
{
    cl_int error = CL_SUCCESS;

    size_t mappedSize = 0;
    do
    {
        *mem = clMapHostPipeIntelFPGA(pipe, 0, numBytes,
                                      &mappedSize, &error);
    }
    while (error == CL_OUT_OF_RESOURCES);

    if (mappedSize != numBytes)
    {
        return CL_OUT_OF_RESOURCES;
    }
    return error;
}

cl_int HostSidePipesTest::unmapPipe(cl_mem pipe, size_t numBytes, void* mem)
{
    cl_int error = CL_SUCCESS;
    size_t unmappedSize = 0;
    do
    {
        error = clUnmapHostPipeIntelFPGA(pipe, mem,
                                         numBytes, &unmappedSize);
    }
    while (error == CL_OUT_OF_RESOURCES);

    if (unmappedSize != numBytes)
    {
        return CL_OUT_OF_RESOURCES;
    }
    return error;
}

bool HostSidePipesTest::runLoopbackKernel(cl_int numPackets)
{
    cl_int error;
    error = clSetKernelArg(m_loopback, 0,
                           sizeof(m_pipeWrite), &m_pipeWrite);
    EXPECT_EQ(error, CL_SUCCESS);
    if (error != CL_SUCCESS)
    {
        return false;
    }

    error = clSetKernelArg(m_loopback, 1,
                           sizeof(m_pipeRead), &m_pipeRead);
    EXPECT_EQ(error, CL_SUCCESS);
    if (error != CL_SUCCESS)
    {
        return false;
    }

    error = clSetKernelArg(m_loopback, 2,
                           sizeof(numPackets), &numPackets);
    EXPECT_EQ(error, CL_SUCCESS);
    if (error != CL_SUCCESS)
    {
        return false;
    }

    error = clEnqueueTask(m_queue, m_loopback, 0, nullptr, nullptr);
    EXPECT_EQ(error, CL_SUCCESS);
    if (error != CL_SUCCESS){
        return false;
    }

    clFlush(m_queue);
    return true;
}


TEST_F(HostSidePipesTest, ReadWrite)
{
    size_t numPackets = m_maxBufferSize;
    ASSERT_TRUE(runLoopbackKernel(numPackets))
        << "Couldn't run loopback kernel";

    for (cl_int i = 0; i < (cl_int)numPackets; ++i)
    {
        ASSERT_EQ(CL_SUCCESS, writePipe(m_pipeWrite, &i));
    }

    for (cl_int i = 0; i < (cl_int)numPackets; ++i)
    {
        cl_int got = -1;
        ASSERT_EQ(CL_SUCCESS, readPipe(m_pipeRead, &got));

        ASSERT_EQ(i, got) << "Verification failed";
    }
}

TEST_F(HostSidePipesTest, MapUnmap)
{
    size_t numPackets = m_maxBufferSize;
    ASSERT_TRUE(runLoopbackKernel(numPackets))
        << "Couldn't run loopback kernel";

    for (cl_int i = 0; i < (cl_int)numPackets; ++i)
    {
        void* mem = nullptr;
        ASSERT_EQ(CL_SUCCESS, mapPipe(m_pipeWrite, sizeof(cl_int), &mem));
        memcpy(mem, &i, sizeof(cl_int));
        ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeWrite, sizeof(cl_int), mem));
    }

    for (cl_int i = 0; i < (cl_int)numPackets; ++i)
    {
        cl_int got = -1;
        void* mem = nullptr;
        ASSERT_EQ(CL_SUCCESS, mapPipe(m_pipeRead, sizeof(cl_int), &mem));
        memcpy(&got, mem, sizeof(cl_int));
        ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeRead, sizeof(cl_int), mem));

        ASSERT_EQ(i, got) << "Verification failed";
    }
}

TEST_F(HostSidePipesTest, Mix)
{
    size_t numPackets = m_maxBufferSize;
    ASSERT_TRUE(runLoopbackKernel(numPackets))
        << "Couldn't run loopback kernel";

    for (cl_int i = 0; i < (cl_int)numPackets; ++i)
    {
        if (i % 2)
        {
            void* mem = nullptr;
            ASSERT_EQ(CL_SUCCESS, mapPipe(m_pipeWrite, sizeof(cl_int), &mem));
            memcpy(mem, &i, sizeof(cl_int));
            ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeWrite, sizeof(cl_int), mem));
        }
        else
        {
            ASSERT_EQ(CL_SUCCESS, writePipe(m_pipeWrite, &i));
        }
    }

    for (cl_int i = 0; i < (cl_int)numPackets; ++i)
    {
        cl_int got = -1;

        if (i % 2)
        {
            void* mem = nullptr;
            ASSERT_EQ(CL_SUCCESS, mapPipe(m_pipeRead, sizeof(cl_int), &mem));
            memcpy(&got, mem, sizeof(cl_int));
            ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeRead, sizeof(cl_int), mem));
        }
        else
        {
            ASSERT_EQ(CL_SUCCESS, readPipe(m_pipeRead, &got));
        }

        ASSERT_EQ(i, got) << "Verification failed";
    }
}

TEST_F(HostSidePipesTest, MapSeq)
{
    size_t numPackets = m_maxBufferSize;
    ASSERT_TRUE(runLoopbackKernel(numPackets))
        << "Couldn't run loopback kernel";

    std::vector<void*> maps;
    std::vector<cl_int> indices;

    for (cl_int i = 0; i < (cl_int)numPackets; ++i)
    {
        void* mem = nullptr;
        ASSERT_EQ(CL_SUCCESS, mapPipe(m_pipeWrite, sizeof(cl_int), &mem));
        maps.push_back(mem);
        indices.push_back(i);
    }

    std::srand(666);
    std::random_shuffle(indices.begin(), indices.end());

    for (cl_int i : indices) {
        void* mem = maps[i];
        memcpy(mem, &i, sizeof(cl_int));
        ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeWrite, sizeof(cl_int), mem));
    }


    for (cl_int i = 0; i < (cl_int)numPackets; ++i) {
        cl_int got = -1;
        void* mem = nullptr;
        ASSERT_EQ(CL_SUCCESS, mapPipe(m_pipeRead, sizeof(cl_int), &mem));
        memcpy(&got, mem, sizeof(cl_int));
        ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeRead, sizeof(cl_int), mem));

        ASSERT_EQ(i, got) << "Verification failed";
    }
}

TEST_F(HostSidePipesTest, MapMulti)
{
    size_t numPackets = m_maxBufferSize;
    ASSERT_TRUE(runLoopbackKernel(numPackets))
        << "Couldn't run loopback kernel";

    void* mem = nullptr;
    ASSERT_EQ(CL_SUCCESS, mapPipe(m_pipeWrite,
                                  sizeof(cl_int) * numPackets,
                                  &mem));

    for (cl_int i = 0; i < (cl_int)numPackets; ++i)
    {
        memcpy((cl_int*)mem + i, &i, sizeof(cl_int));
        ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeWrite, sizeof(cl_int), mem));
    }

    for (cl_int i = 0; i < (cl_int)numPackets; ++i)
    {
        cl_int got = -1;
        void* mem = nullptr;
        ASSERT_EQ(CL_SUCCESS, mapPipe(m_pipeRead, sizeof(cl_int), &mem));
        memcpy(&got, mem, sizeof(cl_int));
        ASSERT_EQ(CL_SUCCESS, unmapPipe(m_pipeRead, sizeof(cl_int), mem));

        ASSERT_EQ(i, got) << "Verification failed";
    }
}

TEST_F(HostSidePipesTest, QueryMaxPipes)
{
    size_t maxReadPipes = 0;
    size_t maxWritePipes = 0;

    size_t valueSizeRet = 0;

    cl_int err = clGetDeviceInfo(m_device, CL_DEVICE_MAX_HOST_READ_PIPES_INTEL,
                                 sizeof(maxReadPipes), &maxReadPipes,
                                 &valueSizeRet);
    ASSERT_EQ(CL_SUCCESS, err) << "clGetDeviceInfo failed";
    ASSERT_EQ(sizeof(maxReadPipes), valueSizeRet);
    ASSERT_GT(maxReadPipes, 0);

    err = clGetDeviceInfo(m_device, CL_DEVICE_MAX_HOST_WRITE_PIPES_INTEL,
                          sizeof(maxWritePipes), &maxWritePipes,
                          &valueSizeRet);
    ASSERT_EQ(CL_SUCCESS, err) << "clGetDeviceInfo failed";
    ASSERT_EQ(sizeof(maxWritePipes), valueSizeRet);
    ASSERT_GT(maxWritePipes, 0);
}

TEST_F(HostSidePipesTest, KernelArgType)
{
    cl_bool expectedAttrs[] = { CL_TRUE, CL_TRUE, CL_FALSE };
    for (size_t i = 0; i < sizeof(expectedAttrs)/sizeof(*expectedAttrs); ++i)
    {
        size_t valueSizeRet = 0;
        cl_bool gotAttr = CL_FALSE;
        cl_int err = clGetKernelArgInfo(m_loopback, i,
                                        CL_KERNEL_ARG_HOST_ACCESSIBLE_PIPE_INTEL,
                                        sizeof(gotAttr), &gotAttr,
                                        &valueSizeRet);

        ASSERT_EQ(CL_SUCCESS, err) << "clGetKernelArgInfo failed";
        ASSERT_EQ(sizeof(gotAttr), valueSizeRet);
        ASSERT_EQ(expectedAttrs[i], gotAttr)
            << "host accessible attribute mismatches";
    }
}

TEST_F(HostSidePipesTest, KernelArgMismatchNegative)
{
    //  The direction indicated in the clCreatePipe flag must be
    //  opposite to the pipe access type specified on the kernel
    //  argument.
    cl_int error;
    error = clSetKernelArg(m_loopback, 0,
                           sizeof(m_pipeRead), &m_pipeRead);
    EXPECT_EQ( CL_INVALID_ARG_VALUE, error);

    error = clSetKernelArg(m_loopback, 1,
                           sizeof(m_pipeWrite), &m_pipeWrite);
    EXPECT_EQ( CL_INVALID_ARG_VALUE, error);
}

TEST_F(HostSidePipesTest, KernelArgNoAttrNegative)
{
    const char* programSrc = "                     \n\
__kernel void no_attr(read_only pipe int pin,      \n\
                      write_only pipe int pout) {} \n\
";

    cl_int error;
    cl_program program = clCreateProgramWithSource(m_context, /*count=*/1,
                                                   &programSrc,
                                                   /*length=*/nullptr,
                                                   &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateProgramWithSource failed.";

    error = clBuildProgram(program, /*num_devices=*/0, /*device_list=*/nullptr,
                           /*options=*/"-cl-std=CL2.0", /*pfn_notify*/nullptr,
                           /*user_data=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clBuildProgram failed.";

    // Host-side pipe can only be bound to a kernel argument with
    // __attribute__((intel_host_accessible))
    cl_kernel noAttrKernel = clCreateKernel(program, "no_attr", &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateKernel failed.";

    error = clSetKernelArg(noAttrKernel, 0,
                           sizeof(m_pipeWrite), &m_pipeWrite);
    EXPECT_EQ( CL_INVALID_ARG_VALUE, error);

    error = clSetKernelArg(noAttrKernel, 1,
                           sizeof(m_pipeRead), &m_pipeRead);
    EXPECT_EQ( CL_INVALID_ARG_VALUE, error);
}
