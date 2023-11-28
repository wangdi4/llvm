#include "CL/cl_fpga_ext.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include "gtest_wrapper.h"
#include <stdio.h>

extern cl_device_type gDeviceType;

/* The test checks that we are not able to compile program with FPGA
 * stuff(channels, attrs), to create buffer with CL_CHANNEL_5_INTELFPGA flag.
 */
void checkFPGAExtensionNEGATIVE() {
  cl_int iRet = CL_SUCCESS;
  cl_platform_id platform = nullptr;
  cl_device_id device = nullptr;

  // Get platform.
  iRet = clGetPlatformIDs(/*num_entries=*/1, &platform,
                          /*num_platforms=*/nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetPlatformIDs failed.";

  // Get device.
  iRet = clGetDeviceIDs(platform, gDeviceType, /*num_entries=*/1, &device,
                        /*num_devices=*/nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetDeviceIDs failed on trying to obtain "
                              << gDeviceType << " device type.";
  if (gDeviceType == CL_DEVICE_TYPE_ACCELERATOR) {
    return;
  }

  // Create context.
  const cl_context_properties prop[5] = {CL_CONTEXT_PLATFORM,
                                         (cl_context_properties)platform, 0};
  cl_context context = clCreateContext(prop, /*num_devices=*/1, &device,
                                       /*pfn_notify=*/nullptr,
                                       /*user_data=*/nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateContext failed.";

  const char *kernel = "\
        #pragma OPENCL EXTENSION cl_intel_channels : enable\n \
        channel float4 rgb __attribute__((depth(43)));\
        __kernel void dummy_kernel()\
        {\
            return;\
        }\
        ";
  cl_program program = clCreateProgramWithSource(context, /*count=*/1, &kernel,
                                                 /*length=*/nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed.";

  iRet = clBuildProgram(program, /*num_devices=*/0, /*device_list=*/nullptr,
                        /*options=*/"-cl-std=CL2.0", /*pfn_notify*/ nullptr,
                        /*user_data=*/nullptr);
  ASSERT_EQ(CL_BUILD_PROGRAM_FAILURE, iRet)
      << " clBuildProgram is not supposed to succefully build program which"
      << " contains fpga specific code";

  clReleaseProgram(program);

  const char *kernelValid = "\
        __kernel void dummy_kernel()\
        {\
            int channel;\
            return;\
        }\
        ";
  program = clCreateProgramWithSource(context, /*count=*/1, &kernelValid,
                                      /*length=*/nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed.";

  iRet = clBuildProgram(program, /*num_devices=*/0, /*device_list=*/nullptr,
                        /*options=*/"-cl-std=CL2.0", /*pfn_notify*/ nullptr,
                        /*user_data=*/nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " Kernel with 'channel' identifier must be successfully compiled if"
      << " extension is not supported.";

  clCreateBuffer(context, CL_CHANNEL_5_INTELFPGA, /*size=*/20,
                 /*host_ptr=*/nullptr, &iRet);
  ASSERT_EQ(CL_INVALID_VALUE, iRet)
      << " clCreateBuffer is not supposed to accept CL_CHANNEL_5_INTELFPGA";

  clReleaseContext(context);
  clReleaseDevice(device);
}

TEST(FrameworkTestType, Test_FPGAExtensionsNegative) {
  ASSERT_NO_FATAL_FAILURE(checkFPGAExtensionNEGATIVE());
}
