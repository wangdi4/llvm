//==--- FPGAPipes.cpp - Tests for host-side pipes.  -*- OpenCL C -*-==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "CL/cl.h"

#include <string>
#include <cstring>
#include "FrameworkTest.h"
#include "gtest/gtest.h"

extern cl_device_type gDeviceType;

void FPGAPipes()
{
    cl_int error = CL_SUCCESS;
    cl_platform_id platform = nullptr;
    cl_device_id device = nullptr;

    error = clGetPlatformIDs(1, &platform, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clGetPlatformIDs failed.";

    error = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clGetDeviceIDs failed to obtain device "
        << "of " << gDeviceType << " type.";

    const cl_context_properties props[5] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)platform,
        CL_CONTEXT_FPGA_EMULATOR_INTEL,
        CL_TRUE,
        0
    };

    cl_context context = clCreateContext(props, 1, &device, nullptr, nullptr,
        &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateContext failed.";

    cl_command_queue queue =
      clCreateCommandQueueWithProperties(context, device,
                                         /*properties=*/nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateCommandQueueWithProperties failed.";

    const char* programSources = "                                      \n\
        __kernel void write(write_only pipe int p, int ptr) {           \n\
            write_pipe(p, &ptr);                                        \n\
        }                                                               \n\
                                                                        \n\
        __kernel void read(read_only pipe int p, global int *ptr) {     \n\
            read_pipe(p, ptr);                                          \n\
        }                                                               \n\
    ";

    cl_program program = clCreateProgramWithSource(context, 1, &programSources,
        nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateProgramWithSource failed.";

    error = clBuildProgram(program, 1, &device, "", nullptr, nullptr);
    EXPECT_EQ(CL_SUCCESS, error) << " clBuildProgram failed.";
    if (CL_SUCCESS != error)
    {
        size_t logSize = 0;
        error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0,
            nullptr, &logSize);
        ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
        std::string log("", logSize);
        error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
            logSize, &log[0], nullptr);
        ASSERT_EQ(CL_SUCCESS, error) << " clGetProgramBuildInfo failed.";
        std::cout << log << std::endl;
        return;
    }

    cl_kernel writeKernel = clCreateKernel(program, "write", &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateKernel failed.";

    cl_kernel readKernel = clCreateKernel(program, "read", &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateKernel failed.";

    cl_mem pipe = clCreatePipe(context, CL_MEM_READ_WRITE,
                               sizeof(cl_int), 1, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreatePipe failed.";

    cl_mem outBuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                   sizeof(cl_int), nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << "clCreateBuffer failed.";

    const int val = 10;
    // Write pipe
    error = clSetKernelArg(writeKernel, 0, sizeof(pipe), &pipe);
    ASSERT_EQ(error, CL_SUCCESS) << "clSetKernelArg0(write) failed";

    error = clSetKernelArg(writeKernel, 1, sizeof(val), &val);
    ASSERT_EQ(error, CL_SUCCESS) << "clSetKernelArg1(write) failed";

    error = clEnqueueTask(queue, writeKernel, 0, nullptr, nullptr);
    ASSERT_EQ(error, CL_SUCCESS) << "clEnqueueTask(write) failed";

    clFinish(queue);

    // Read pipe
    error = clSetKernelArg(readKernel, 0, sizeof(pipe), &pipe);
    ASSERT_EQ(error, CL_SUCCESS) << "clSetKernelArg0(read) failed";

    error = clSetKernelArg(readKernel, 1, sizeof(cl_mem), &outBuf);
    ASSERT_EQ(error, CL_SUCCESS) << "clSetKernelArg1(read) failed";

    error = clEnqueueTask(queue, readKernel, 0, nullptr, nullptr);
    ASSERT_EQ(error, CL_SUCCESS) << "clEnqueueTask(read) failed";

    clFinish(queue);

    int out = 0;
    error = clEnqueueReadBuffer(queue, outBuf, CL_TRUE, 0, sizeof(cl_int),
                                &out, 0, nullptr, nullptr);
    ASSERT_EQ(error, CL_SUCCESS) << "clEnqueueReadBuffer failed";

    ASSERT_EQ(val, out) << "Verification failed";

    clReleaseMemObject(pipe);
    clReleaseMemObject(outBuf);
    clReleaseKernel(readKernel);
    clReleaseKernel(writeKernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    clReleaseDevice(device);
}

