#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
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
