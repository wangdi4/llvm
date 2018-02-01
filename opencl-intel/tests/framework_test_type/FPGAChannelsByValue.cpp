#include "CL/cl.h"

#include <string>
#include <cstring>
#include "FrameworkTest.h"
#include "gtest/gtest.h"

extern cl_device_type gDeviceType;

void FPGAChannelsByValue()
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

    const char* programSources = "\
        #pragma OPENCL EXTENSION cl_intel_channels: enable\n\
        channel int a __attribute__((depth(512)));\n\
        channel int b __attribute__((depth(512)));\n\
        channel int c[2] __attribute__((depth(1024)));\n\
        \n\
        __attribute__((noinline))\n\
        void sendOne(channel int ch) {\n\
          write_channel_intel(ch, 1);\n\
        }\n\
        \n\
        __attribute__((noinline))\n\
        void sendTwo(channel int ch) {\n\
          write_channel_intel(ch, 2);\n\
        }\n\
        \n\
        __attribute__((noinline))\n\
        int getFlag(channel int a, channel int b) {\n\
          int i = read_channel_intel(a);\n\
          int j = read_channel_intel(b);\n\
          return i + j;\n\
        }\n\
        \n\
        __attribute__((noinline))\n\
        void send(channel int a, channel int b, int flag) {\n\
          switch (flag) {\n\
            case 0: sendOne(a); break;\n\
            case 1: sendTwo(a); break;\n\
            case 2: sendOne(b); break;\n\
            case 3: sendTwo(b); break;\n\
          }\n\
        }\n\
        \n\
        __kernel void producer(int size) {\n\
          for (int i = 0; i < size; ++i) {\n\
            int flag = getFlag(c[0], c[1]);\n\
            send(a, b, flag);\n\
          }\n\
        }\n\
        \n\
        __kernel void host_reader(__global int2* data, int size) {\n\
          for (int i = 0; i < size; ++i) {\n\
            write_channel_intel(c[0], data[i].x);\n\
            write_channel_intel(c[1], data[i].y);\n\
          }\n\
        }\n\
        \n\
        __kernel void host_writer(__global int* data1, __global int* data2,\n\
            int size) {\n\
          for (int i = 0; i < size; ++i) {\n\
            data1[i] = read_channel_intel(a);\n\
            data2[i] = read_channel_intel(b);\n\
          }\n\
        }\n\
    ";

    cl_program program = clCreateProgramWithSource(context, 1, &programSources,
        nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateProgramWithSource failed.";

    error = clBuildProgram(program, 1, &device, "", nullptr,
        nullptr);
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

    const cl_int numElements = 1024;
    cl_int2 input_data[numElements];
    for (cl_int i = 0, x = 0; i < numElements; ++i) {
        int a = 0, b = 0;
        switch (x) {
            case 0: a = 0; b = 0; break;
            case 1: a = 1; b = 0; break;
            case 2: a = 1; b = 1; break;
            case 3: a = 1; b = 2; break;
        }

        input_data[i].s[0] = a;
        input_data[i].s[1] = b;

        ++x;
        if (x == 4) {
            x = 0;
        }
    }

    size_t bufferSize = numElements * sizeof(cl_int2);
    cl_mem input_buffer = clCreateBuffer(context,
        CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, bufferSize, input_data, &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateBuffer failed.";

    cl_kernel host_reader = clCreateKernel(program, "host_reader", &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateKernel failed.";
    cl_kernel host_writer = clCreateKernel(program, "host_writer", &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateKernel failed.";
    cl_kernel producer = clCreateKernel(program, "producer", &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateKernel failed.";

    error = clSetKernelArg(host_reader, 0, sizeof(cl_mem), &input_buffer);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
    error = clSetKernelArg(host_reader, 1, sizeof(cl_int), &numElements);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

    const cl_int halfNumElements = numElements / 2;
    size_t halfBufferSize = halfNumElements * sizeof(cl_int);
    cl_mem output_buffer1 = clCreateBuffer(context,
        CL_MEM_WRITE_ONLY, halfBufferSize, nullptr, &error);
    cl_mem output_buffer2 = clCreateBuffer(context,
        CL_MEM_WRITE_ONLY, halfBufferSize, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateBuffer failed.";
    error = clSetKernelArg(host_writer, 0, sizeof(cl_mem), &output_buffer1);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
    error = clSetKernelArg(host_writer, 1, sizeof(cl_mem), &output_buffer2);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
    error = clSetKernelArg(host_writer, 2, sizeof(cl_int), &halfNumElements);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

    error = clSetKernelArg(producer, 0, sizeof(cl_int), &numElements);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

    cl_command_queue reader_queue = clCreateCommandQueueWithProperties(context,
        device, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error)
        << " clCreateCommandQueueWithProperties failed.";
    cl_command_queue writer_queue = clCreateCommandQueueWithProperties(context,
        device, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error)
        << " clCreateCommandQueueWithProperties failed.";
    cl_command_queue producer_queue =
        clCreateCommandQueueWithProperties(context, device, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error)
        << " clCreateCommandQueueWithProperties failed.";

    size_t globalSize[] = {1, 1, 1};
    size_t localSize[] = {1, 1, 1};
    error = clEnqueueNDRangeKernel(writer_queue, host_writer, 1,
        nullptr, globalSize, localSize, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";
    error = clEnqueueNDRangeKernel(producer_queue, producer, 1,
        nullptr, globalSize, localSize, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";
    error = clEnqueueNDRangeKernel(reader_queue, host_reader, 1,
        nullptr, globalSize, localSize, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";

    clFinish(writer_queue);
    cl_int output_data1[halfNumElements];
    error = clEnqueueReadBuffer(writer_queue, output_buffer1, CL_TRUE, 0,
        halfBufferSize, output_data1, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueReadBuffer failed.";
    cl_int output_data2[halfNumElements];
    error = clEnqueueReadBuffer(writer_queue, output_buffer2, CL_TRUE, 0,
        halfBufferSize, output_data2, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueReadBuffer failed.";

    for (cl_int i = 0; i < halfNumElements; ++i) {
        cl_int reference = i % 2 == 0 ? 1 : 2;
        ASSERT_EQ(reference, output_data1[i]) << "invalid value of " << i  <<
            "-th element of data1 array";
    }

    for (cl_int i = 0; i < halfNumElements; ++i) {
        cl_int reference = i % 2 == 0 ? 1 : 2;
        ASSERT_EQ(reference, output_data2[i]) << "invalid value of " << i  <<
            "-th element of data2 array";
    }

    clReleaseCommandQueue(reader_queue);
    clReleaseCommandQueue(writer_queue);
    clReleaseCommandQueue(producer_queue);
    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer1);
    clReleaseMemObject(output_buffer2);
    clReleaseKernel(host_reader);
    clReleaseKernel(host_writer);
    clReleaseKernel(producer);
    clReleaseProgram(program);
    clReleaseContext(context);
}

