#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include "gtest_wrapper.h"
#include <stdio.h>
#include <string>

extern cl_device_type gDeviceType;

void globalVariableSizeQueryTest() {
  const char *source = "\
    __constant int globalInt = 42;\n\
    __kernel void test_kernel(__global int* p)\n\
    {\n\
        *p = 1;\n\
    }\n\
    ";

  cl_int err = CL_SUCCESS;
  cl_platform_id platform = nullptr;
  cl_device_id device = nullptr;

  // Get platform
  err = clGetPlatformIDs(1, &platform, nullptr);
  ASSERT_EQ(CL_SUCCESS, err) << " clGetPlatformIDs failed.";

  // Get device
  err = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
  ASSERT_EQ(CL_SUCCESS, err) << " clGetDeviceIDs failed on trying to obtain "
                             << gDeviceType << " device type.";

  // Create context
  const cl_context_properties prop[] = {CL_CONTEXT_PLATFORM,
                                        (cl_context_properties)platform, 0};
  cl_context context =
      clCreateContext(prop, 1, &device, nullptr, nullptr, &err);
  ASSERT_EQ(CL_SUCCESS, err) << " clCreateContext failed.";

  // Create program with source
  cl_program program =
      clCreateProgramWithSource(context, 1, (const char **)&source, NULL, &err);
  ASSERT_EQ(CL_SUCCESS, err) << " clCreateProgramWithSource failed.";

  // Build program
  err = clBuildProgram(program, 1, &device, "-cl-opt-disable", NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, err) << "clBuildProgram failed.";

  // Get global variable size
  size_t param_size = 0;
  err = clGetProgramBuildInfo(program, device,
                              CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE,
                              param_size, nullptr, &param_size);
  ASSERT_EQ(CL_SUCCESS, err) << "clGetProgramBuildInfo failed.";

  std::vector<char> param_value(param_size);
  size_t param_size_ret = 0;
  err = clGetProgramBuildInfo(program, device,
                              CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE,
                              param_size, &param_value[0], &param_size_ret);
  ASSERT_EQ(CL_SUCCESS, err) << "clGetProgramBuildInfo failed.";
  ASSERT_EQ(sizeof(int), param_value[0])
      << "Global variable size is not equabl to the expected value";
  ASSERT_EQ(param_size, param_size_ret)
      << "ret value size is different from queried size.";

  clReleaseProgram(program);
  clReleaseContext(context);
}
