#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include "FrameworkTest.h"
#include <gtest/gtest.h>

extern cl_device_type gDeviceType;

/* The test checks that if we pass CL_CONTEXT_FPGA_EMULATOR_INTEL to context
 * properties we are able to compile program with FPGA stuff(channels, attrs),
 * to create buffer with CL_CHANNEL_5_INTELFPGA flag.
*/
void checkFPGAExtension()
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
        #pragma OPENCL EXTENSION cl_altera_channels : enable\n \
        channel float4 rgb __attribute__((depth(43)));\
        __kernel void dummy_kernel()\
        {\
            return;\
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

    cl_mem buffer = clCreateBuffer(context, CL_CHANNEL_5_INTELFPGA,
                                   /*size=*/20,
                                   /*host_ptr=*/nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateBuffer failed.";

    clReleaseMemObject(buffer);
    clReleaseKernel(kern);
    clReleaseProgram(program);
    clReleaseContext(context);
    clReleaseDevice(device);
}
