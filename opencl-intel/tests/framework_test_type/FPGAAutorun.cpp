#include "CL/cl.h"

#include <string>
#include <cstring>
#include "FrameworkTest.h"
#include "gtest/gtest.h"

extern cl_device_type gDeviceType;

void FPGAAutorun()
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
        channel int arr[N + 1];\n\
        channel int in, out;\n\
        \n\
        __attribute__((max_global_work_dim(0)))\n\
        __attribute__((autorun))\n\
        __kernel void single_plus() {\n\
            int a = read_channel_intel(in);\n\
            write_channel_intel(out, a + 1);\n\
        }\n\
        \n\
        __attribute__((max_global_work_dim(0)))\n\
        __attribute__((num_compute_units(N, 1, 1)))\n\
        __attribute__((autorun))\n\
        __kernel void chained_plus() {\n\
            int a = read_channel_intel(arr[get_compute_id(0)]);\n\
            write_channel_intel(arr[get_compute_id(0) + 1], a + 1);\n\
        }\n\
        \n\
        __kernel void reader_for_single_plus(int n, __global int* data) {\n\
            for (int i = 0; i < n; ++i) {\n\
                data[i] = read_channel_intel(out);\n\
            }\n\
        }\n\
        \n\
        __kernel void writer_for_single_plus(int n, __global int* data) {\n\
            for (int i = 0; i < n; ++i) {\n\
                write_channel_intel(in, data[i]);\n\
            }\n\
        }\n\
        \n\
        __kernel void reader_for_chained_plus(int n, __global int* data) {\n\
            for (int i = 0; i < n; ++i) {\n\
                data[i] = read_channel_intel(arr[N]);\n\
            }\n\
        }\n\
        \n\
        __kernel void writer_for_chained_plus(int n, __global int* data) {\n\
            for (int i = 0; i < n; ++i) {\n\
                write_channel_intel(arr[0], data[i]);\n\
            }\n\
        }\n\
    ";

    cl_program program = clCreateProgramWithSource(context, 1, &programSources,
        nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateProgramWithSource failed.";

    const int N = 5;
    std::string buildOptions = "-cl-std=CL2.0 -DN=" + std::to_string(N);
    error = clBuildProgram(program, 1, &device, buildOptions.c_str(), nullptr,
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

    const cl_int numElements = 1000;
    cl_int input_data[numElements] = {0};
    for (cl_int i = 0; i < numElements; ++i)
    {
        input_data[i] = i;
    }
    size_t bufferSize = numElements * sizeof(cl_int);
    cl_mem input_buffer = clCreateBuffer(context,
        CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, bufferSize, input_data, &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateBuffer failed.";

    cl_mem output_buffer = clCreateBuffer(context,
        CL_MEM_WRITE_ONLY, bufferSize, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateBuffer failed.";

    cl_kernel reader_for_single_plus = clCreateKernel(program,
        "reader_for_single_plus", &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateKernel failed.";
    cl_kernel writer_for_single_plus = clCreateKernel(program,
        "writer_for_single_plus", &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateKernel failed.";

    error = clSetKernelArg(reader_for_single_plus, 0, sizeof(cl_int),
        &numElements);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
    error = clSetKernelArg(reader_for_single_plus, 1, sizeof(cl_mem),
        &output_buffer);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

    error = clSetKernelArg(writer_for_single_plus, 0, sizeof(cl_int),
        &numElements);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
    error = clSetKernelArg(writer_for_single_plus, 1, sizeof(cl_mem),
        &input_buffer);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

    cl_command_queue reader_queue = clCreateCommandQueueWithProperties(context,
        device, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error)
        << " clCreateCommandQueueWithProperties failed.";
    cl_command_queue writer_queue = clCreateCommandQueueWithProperties(context,
        device, nullptr, &error);
    ASSERT_EQ(CL_SUCCESS, error)
        << " clCreateCommandQueueWithProperties failed.";

    size_t globalSize[] = {1, 1, 1};
    size_t localSize[] = {1, 1, 1};
    error = clEnqueueNDRangeKernel(writer_queue, writer_for_single_plus, 1,
        nullptr, globalSize, localSize, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";
    error = clEnqueueNDRangeKernel(reader_queue, reader_for_single_plus, 1,
        nullptr, globalSize, localSize, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";

    clFinish(reader_queue);
    cl_int output_data[numElements] = {0};
    error = clEnqueueReadBuffer(reader_queue, output_buffer, CL_TRUE, 0,
        bufferSize, output_data, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueReadBuffer failed.";
    for (cl_int i = 0; i < numElements; ++i)
    {
        ASSERT_EQ(i + 1, output_data[i]) << " invalid value of " << i << "-th"
            << " element of resulting array";
    }

    cl_kernel reader_for_chained_plus = clCreateKernel(program,
        "reader_for_chained_plus", &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateKernel failed.";
    cl_kernel writer_for_chained_plus = clCreateKernel(program,
        "writer_for_chained_plus", &error);
    ASSERT_EQ(CL_SUCCESS, error) << " clCreateKernel failed.";

    const cl_int zero = 0;
    error = clEnqueueFillBuffer(reader_queue, output_buffer, &zero,
        sizeof(cl_int), 0, bufferSize, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueFillBuffer failed.";
    std::memset(output_data, 0, numElements);
    clFinish(reader_queue);

    error = clSetKernelArg(reader_for_chained_plus, 0, sizeof(cl_int),
        &numElements);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
    error = clSetKernelArg(reader_for_chained_plus, 1, sizeof(cl_mem),
        &output_buffer);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

    error = clSetKernelArg(writer_for_chained_plus, 0, sizeof(cl_int),
        &numElements);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";
    error = clSetKernelArg(writer_for_chained_plus, 1, sizeof(cl_mem),
        &input_buffer);
    ASSERT_EQ(CL_SUCCESS, error) << " clSetKernelArg failed.";

    error = clEnqueueNDRangeKernel(writer_queue, writer_for_chained_plus, 1,
        nullptr, globalSize, localSize, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";
    error = clEnqueueNDRangeKernel(reader_queue, reader_for_chained_plus, 1,
        nullptr, globalSize, localSize, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueNDRangeKernel failed.";

    clFinish(reader_queue);
    error = clEnqueueReadBuffer(reader_queue, output_buffer, CL_TRUE, 0,
        bufferSize, output_data, 0, nullptr, nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << " clEnqueueReadBuffer failed.";
    for (cl_int i = 0; i < numElements; ++i)
    {
        ASSERT_EQ(i + N, output_data[i]) << " invalid value of " << i << "-th"
            << " element of resulting array";
    }

    clReleaseCommandQueue(reader_queue);
    clReleaseCommandQueue(writer_queue);
    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer);
    clReleaseKernel(reader_for_single_plus);
    clReleaseKernel(writer_for_single_plus);
    clReleaseKernel(reader_for_chained_plus);
    clReleaseKernel(writer_for_chained_plus);
    clReleaseProgram(program);
    clReleaseContext(context);
}
