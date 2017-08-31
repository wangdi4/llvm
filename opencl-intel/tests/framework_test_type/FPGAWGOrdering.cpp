#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include <vector>
#include "FrameworkTest.h"
#include <gtest/gtest.h>

extern cl_device_type gDeviceType;

void FPGAWGOrdering()
{
    cl_int         iRet     = CL_SUCCESS;
    cl_platform_id platform = nullptr;
    cl_device_id   device   = nullptr;

    //Get platform.
    iRet = clGetPlatformIDs(/*num_entries=*/1, &platform,
                            /*num_platforms=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetPlatformIDs failed.";

    //Get device.
    iRet = clGetDeviceIDs(platform, gDeviceType, /*num_entries=*/1,
                          &device, /*num_devices=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << " clGetDeviceIDs failed on trying to obtain "
        << gDeviceType << " device type.";

    //Create context.
    const cl_context_properties prop[5] = { CL_CONTEXT_PLATFORM,
                                            (cl_context_properties)platform,
                                            CL_CONTEXT_FPGA_EMULATOR_INTEL,
                                            CL_TRUE,
                                            0 };
    cl_context context = clCreateContext(prop, /*num_devices=*/1, &device,
                                         /*pfn_notify=*/nullptr,
                                         /*user_data=*/nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateContext failed.";

    const char* kernel = "\
        __kernel void dummy_kernel(__global int* buf, __global int* success)\
        {\
            size_t gid = get_global_id(0);\
            for(size_t i = 0; i < gid; ++i)\
                if(buf[i] != i)\
                {\
                    printf(\"setting 1\");\
                    *success = 1;\
                }\
            buf[gid] = gid;\
        }\
        ";
    cl_program program = clCreateProgramWithSource(context, /*count=*/1,
                                                   &kernel, /*length=*/nullptr,
                                                   &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed.";

    iRet = clBuildProgram(program, /*num_devices=*/0, /*device_list=*/nullptr,
                          /*options=*/"-cl-std=CL2.0", /*pfn_notify*/nullptr,
                          /*user_data=*/nullptr);
    if(CL_SUCCESS != iRet)
    {
        size_t logSize = 0;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              /*param_value_size=*/0, /*param_value=*/nullptr,
                              &logSize);
        std::string log("", logSize);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              log.size(), &log[0], /*value_size_ret=*/nullptr);
        std::cout << log << std::endl;
    }
    ASSERT_EQ(CL_SUCCESS, iRet) << " clBuildProgram failed.";

    cl_kernel kern = clCreateKernel(program, "dummy_kernel", &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateKernel failed.";

    size_t globalSize[] = {100, 1, 1};
    size_t localSize[]  = {1, 1, 1};
    size_t bufferSize = sizeof(int) * globalSize[0];
    cl_mem buffer = clCreateBuffer(context, CL_MEM_BANK_5_ALTERA,
                                   /*size=*/bufferSize, /*host_ptr=*/nullptr,
                                   &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateBuffer failed.";

    int success = 0;
    cl_mem succBuf = clCreateBuffer(context, CL_MEM_BANK_5_ALTERA,
                                    /*size=*/sizeof(success),
                                    /*host_ptr=*/nullptr,
                                    &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateBuffer failed.";

    cl_command_queue queue =
        clCreateCommandQueueWithProperties(context, device,
                                           /*properties=*/nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateCommandQueueWithProperties failed.";

    int zero = 0;
    iRet = clEnqueueFillBuffer(queue, buffer, &zero,
                               /*pattern_size=*/sizeof(zero),
                               /*offset=*/0, /*size=*/bufferSize,
                               /*num_events_in_wait_list=*/0,
                               /*event_wait_list=*/nullptr, /*event=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clEnqueueFillBuffer failed.";

    iRet = clEnqueueFillBuffer(queue, succBuf, &zero,
                               /*pattern_size=*/sizeof(zero),
                               /*offset=*/0, /*size=*/sizeof(zero),
                               /*num_events_in_wait_list=*/0,
                               /*event_wait_list=*/nullptr, /*event=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clEnqueueFillBuffer failed.";


    iRet = clSetKernelArg(kern, 0, sizeof(buffer), &buffer);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clSetKernelArg 0 failed.";

    iRet = clSetKernelArg(kern, 1, sizeof(succBuf), &succBuf);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clSetKernelArg 1 failed.";

    iRet = clEnqueueNDRangeKernel(queue, kern, /*work_dim=*/1,
                                  /*global_work_offset=*/nullptr,
                                  globalSize, localSize,
                                  /*num_events_in_wait_list=*/0,
                                  /*event_wait_list=*/nullptr,
                                  /*event=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clEnqueueNDRangeKernel failed.";

    iRet = clEnqueueReadBuffer(queue, succBuf, /*blocking_read=*/CL_TRUE,
                               /*offset=*/0, sizeof(success), /*ptr=*/&success,
                               /*num_events_in_wait_list=*/0,
                               /*event_wait_list=*/nullptr,
                               /*event=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clEnqueueReadBuffer failed.";

    ASSERT_EQ(0, success)
        << "Failed. Kernel was not executed in sequential fashion.";

    clReleaseCommandQueue(queue);
    clReleaseMemObject(buffer);
    clReleaseMemObject(succBuf);
    clReleaseKernel(kern);
    clReleaseProgram(program);
    clReleaseContext(context);
    clReleaseDevice(device);
}

void FPGAPipeOrdering()
{
    cl_int         iRet     = CL_SUCCESS;
    cl_platform_id platform = nullptr;
    cl_device_id   device   = nullptr;

    //Get platform.
    iRet = clGetPlatformIDs(/*num_entries=*/1, &platform,
                            /*num_platforms=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetPlatformIDs failed.";

    //Get device.
    iRet = clGetDeviceIDs(platform, gDeviceType, /*num_entries=*/1,
                          &device, /*num_devices=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << " clGetDeviceIDs failed on trying to obtain "
        << gDeviceType << " device type.";

    //Create context.
    const cl_context_properties prop[5] = { CL_CONTEXT_PLATFORM,
                                            (cl_context_properties)platform,
                                            CL_CONTEXT_FPGA_EMULATOR_INTEL,
                                            CL_TRUE,
                                            0 };
    cl_context context = clCreateContext(prop, /*num_devices=*/1, &device,
                                         /*pfn_notify=*/nullptr,
                                         /*user_data=*/nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateContext failed.";

    const char* kernel = "\
        #pragma OPENCL EXTENSION cl_intel_channels : enable \n \
        \n \
        volatile __global int counter = 0; \n \
        channel int ch;\n \
        \n \
        __kernel void channel_writer(__global int *iters) {\n \
            size_t gid = get_global_id(0);\n \
            for (int i = 0; i < *iters; ++i) {\n \
                write_channel_intel(ch, i);\n \
            }\n \
        }\n \
        \n \
        __kernel void channel_reader(__global int *out_buf, __global int *iters) {\n \
            for (int i = 0; i < *iters; ++i) {\n \
                int ind = atomic_inc(&counter); \
                out_buf[ind] = read_channel_intel(ch);\n \
            }\n \
        }\n \
        ";
    cl_program program = clCreateProgramWithSource(context, /*count=*/1,
                                                   &kernel, /*length=*/nullptr,
                                                   &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed.";

    iRet = clBuildProgram(program, /*num_devices=*/0, /*device_list=*/nullptr,
                          /*options=*/"-cl-std=CL2.0", /*pfn_notify*/nullptr,
                          /*user_data=*/nullptr);
    if(CL_SUCCESS != iRet)
    {
        size_t logSize = 0;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              /*param_value_size=*/0, /*param_value=*/nullptr,
                              &logSize);
        std::string log("", logSize);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              log.size(), &log[0], /*value_size_ret=*/nullptr);
        std::cout << log << std::endl;
    }
    ASSERT_EQ(CL_SUCCESS, iRet) << " clBuildProgram failed.";

    cl_kernel kernelWriter = clCreateKernel(program, "channel_writer", &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateKernel failed.";

    cl_kernel kernelReader = clCreateKernel(program, "channel_reader", &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateKernel failed.";

    size_t globalSize = 512;
    size_t localSize  = 128;
    int iterations = 50;
    int numElements = globalSize * iterations;
    size_t outBufferSize = sizeof(int) * numElements;
    std::vector<int> outBufferHost;
    outBufferHost.resize(numElements);
    cl_mem outBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
                                   /*size=*/outBufferSize, /*host_ptr=*/&outBufferHost[0],
                                   &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateBuffer failed.";

    cl_mem iterBuf = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                    /*size=*/sizeof(iterations),
                                    /*host_ptr=*/&iterations,
                                    &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateBuffer failed.";

    cl_command_queue queueWriter =
        clCreateCommandQueueWithProperties(context, device,
                                           /*properties=*/nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateCommandQueueWithProperties failed.";

    cl_command_queue queueReader =
        clCreateCommandQueueWithProperties(context, device,
                                           /*properties=*/nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateCommandQueueWithProperties failed.";

    iRet = clSetKernelArg(kernelWriter, 0, sizeof(iterBuf), &iterBuf);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clSetKernelArg 0 failed.";

    iRet = clSetKernelArg(kernelReader, 0, sizeof(outBuffer), &outBuffer);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clSetKernelArg 0 failed.";

    iRet = clSetKernelArg(kernelReader, 1, sizeof(iterBuf), &iterBuf);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clSetKernelArg 1 failed.";

    iRet = clEnqueueNDRangeKernel(queueWriter, kernelWriter, /*work_dim=*/1,
                                  /*global_work_offset=*/nullptr,
                                  &globalSize, &localSize,
                                  /*num_events_in_wait_list=*/0,
                                  /*event_wait_list=*/nullptr,
                                  /*event=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clEnqueueNDRangeKernel failed.";

    iRet = clEnqueueNDRangeKernel(queueReader, kernelReader, /*work_dim=*/1,
                                  /*global_work_offset=*/nullptr,
                                  &globalSize, &localSize,
                                  /*num_events_in_wait_list=*/0,
                                  /*event_wait_list=*/nullptr,
                                  /*event=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clEnqueueNDRangeKernel failed.";

    clFinish(queueWriter);
    clFinish(queueReader);

    for (int i = 0; i < iterations; ++i)
    {
        for (size_t groupId = 0; groupId < globalSize/localSize; ++groupId)
        {
            for (size_t localId = 1; localId < localSize; ++localId)
            {
                // (shift by iterations) * (shift by wg)
                int expectIndex = i*localSize + groupId*localSize*iterations;
                int curIndex = expectIndex + localId;
                ASSERT_EQ(outBufferHost[expectIndex], outBufferHost[curIndex])
                    << "Incorrect result!";
            }
        }
    }

    clReleaseCommandQueue(queueWriter);
    clReleaseCommandQueue(queueReader);
    clReleaseMemObject(outBuffer);
    clReleaseMemObject(iterBuf);
    clReleaseKernel(kernelWriter);
    clReleaseKernel(kernelWriter);
    clReleaseProgram(program);
    clReleaseContext(context);
    clReleaseDevice(device);
}
