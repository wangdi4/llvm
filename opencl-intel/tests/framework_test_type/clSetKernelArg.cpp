#include "CL/cl.h"

#include <string>
#include <cstring>
#include "FrameworkTest.h"
#include "gtest/gtest.h"

extern cl_device_type gDeviceType;

void clSetKernelArgTest()
{
    const char* programSource = "                                          \n\
__kernel void k1(__local  __attribute__((local_mem_size(1042))) int *a)    \n\
{ }                                                                        \n\
        ";

    cl_int iRet = CL_SUCCESS;
    cl_platform_id   platform;
    cl_device_id     device;
    cl_context       context;
    cl_program       program;
    cl_kernel        kernel;

    iRet = clGetPlatformIDs(/*num_entries=*/1, &platform,
                            /*num_platforms=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clGetPlatformIDs failed.";

    iRet = clGetDeviceIDs(platform, gDeviceType, /*num_entries=*/1,
                          &device, /*num_devices=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << "clGetDeviceIDs failed on trying to obtain "
        << gDeviceType << " device type.";

    const cl_context_properties prop[5] = { CL_CONTEXT_PLATFORM,
                                            (cl_context_properties)platform,
                                            CL_CONTEXT_FPGA_EMULATOR_INTEL,
                                            CL_TRUE,
                                            0 };
    context = clCreateContext(prop,
                              /*num_devices=*/1, &device,
                              /*pfn_notify=*/nullptr,
                              /*user_data=*/nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateContext failed.";

    program = clCreateProgramWithSource(context, /*count=*/1,
                                          &programSource,
                                          /*length=*/nullptr,
                                          &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateProgramWithSource failed.";

    iRet = clBuildProgram(program, /*num_devices=*/0, /*device_list=*/nullptr,
                          /*options=*/"", /*pfn_notify*/nullptr,
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

        FAIL() << "Build failed\n"
               << "Source:\n"
               << "---------\n"
               << programSource
               << "---------\n"
               << log;
    }

    kernel = clCreateKernel(program, "k1", &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateKernel failed.";

    iRet = clSetKernelArg(kernel, 0, sizeof(int) * 256, NULL);
    ASSERT_EQ(iRet, CL_SUCCESS);

    iRet = clSetKernelArg(kernel, 0, 1042, NULL);
    ASSERT_EQ(iRet, CL_SUCCESS);

    iRet = clSetKernelArg(kernel, 0, sizeof(int) * 1042, NULL);
    ASSERT_EQ(iRet, CL_INVALID_ARG_SIZE);

    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
    clReleaseDevice(device);
}

void clSetKernelArgLocalArgTest()
{
    const char* programSource = "\n\
__kernel void k1(__local int *a) \n\
{ }                              \n\
        ";

    cl_int iRet = CL_SUCCESS;
    cl_platform_id   platform;
    cl_device_id     device;
    cl_context       context;
    cl_program       program;
    cl_kernel        kernel;

    iRet = clGetPlatformIDs(/*num_entries=*/1, &platform,
                            /*num_platforms=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clGetPlatformIDs failed.";

    iRet = clGetDeviceIDs(platform, gDeviceType, /*num_entries=*/1,
                          &device, /*num_devices=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << "clGetDeviceIDs failed on trying to obtain "
        << gDeviceType << " device type.";

    const cl_context_properties prop[5] = { CL_CONTEXT_PLATFORM,
                                            (cl_context_properties)platform,
                                            CL_CONTEXT_FPGA_EMULATOR_INTEL,
                                            CL_TRUE,
                                            0 };
    context = clCreateContext(prop,
                              /*num_devices=*/1, &device,
                              /*pfn_notify=*/nullptr,
                              /*user_data=*/nullptr, &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateContext failed.";

    program = clCreateProgramWithSource(context, /*count=*/1,
                                          &programSource,
                                          /*length=*/nullptr,
                                          &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateProgramWithSource failed.";

    iRet = clBuildProgram(program, /*num_devices=*/0, /*device_list=*/nullptr,
                          /*options=*/"", /*pfn_notify*/nullptr,
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

        FAIL() << "Build failed\n"
               << "Source:\n"
               << "---------\n"
               << programSource
               << "---------\n"
               << log;
    }

    kernel = clCreateKernel(program, "k1", &iRet);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateKernel failed.";

    iRet = clSetKernelArg(kernel, 0, sizeof(int) * 256, NULL);
    ASSERT_EQ(iRet, CL_SUCCESS);

    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
    clReleaseDevice(device);
}
