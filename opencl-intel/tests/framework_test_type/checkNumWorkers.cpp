#include "CL/cl.h"
#include <stdio.h>
#include "FrameworkTest.h"
#include "common_utils.h"

#include <gtest/gtest.h>

extern cl_device_type gDeviceType;

void checkNumWorkers()
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
    ASSERT_EQ(CL_SUCCESS, iRet) << " clGetDeviceIDs failed";

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
        __global atomic_int count = ATOMIC_VAR_INIT(0);\n\
        __kernel void work() {\n\
            atomic_fetch_add(&count, 1);\n\
            int n;\n\
            do { \n\
                n = atomic_load(&count);\n\
            } while (n != get_global_size(0));\n\
            return;\n\
        }\n\
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

    cl_kernel kern = clCreateKernel(program, "work", &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateKernel failed.";
    cl_command_queue queue;

    queue = clCreateCommandQueueWithProperties(context, device, NULL,
        &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateCommandQueue failed.";

    const size_t work_dim = 1;
    const size_t globalSize[work_dim] = { 4 };
    const size_t localSize[work_dim] = { 1 };
    iRet = clEnqueueNDRangeKernel(queue, kern, work_dim, NULL,
        globalSize, localSize, 0, NULL, NULL);
    printf("NDRange queued \n");
    ASSERT_EQ(CL_SUCCESS, iRet) << " clEnqueueNDRangeKernel failed.";

    clFinish(queue);

    clReleaseKernel(kern);
    clReleaseProgram(program);
    clReleaseContext(context);
    clReleaseDevice(device);

}
