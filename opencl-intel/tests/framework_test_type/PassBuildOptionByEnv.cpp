#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include "common_utils.h"
#include "gtest_wrapper.h"
#include <stdio.h>
#include <string>

extern cl_device_type gDeviceType;

static const char *source = R"(
  __kernel void test_kernel(__global int* p)
  {
      *p = 1;
  }
  )";

void passBuildOptionByEnvTest() {

  cl_int err = CL_SUCCESS;
  cl_platform_id platform = nullptr;
  cl_device_id device = nullptr;

  err = clGetPlatformIDs(1, &platform, nullptr);
  ASSERT_EQ(err, CL_SUCCESS) << " clGetPlatformIDs failed.";

  err = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
  ASSERT_EQ(err, CL_SUCCESS) << " clGetDeviceIDs failed on trying to obtain "
                             << gDeviceType << " device type.";

  const cl_context_properties prop[] = {CL_CONTEXT_PLATFORM,
                                        (cl_context_properties)platform, 0};
  cl_context context =
      clCreateContext(prop, 1, &device, nullptr, nullptr, &err);
  ASSERT_EQ(err, CL_SUCCESS) << " clCreateContext failed.";

  cl_program program = clCreateProgramWithSource(
      context, 1, (const char **)&source, nullptr, &err);
  ASSERT_EQ(err, CL_SUCCESS) << " clCreateProgramWithSource failed.";

  err = SETENV("OPENCL_PROGRAM_COMPILE_OPTIONS", "-g -cl-opt-disable");
  ASSERT_EQ(err, 1) << " setenv failed.";

  err = clCompileProgram(program, 1, &device, "", 0, nullptr, nullptr, nullptr,
                         nullptr);
  ASSERT_EQ(err, CL_SUCCESS) << " clCompileProgram failed.";

  char param_value[128];
  err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_OPTIONS,
                              sizeof(param_value), param_value, nullptr);
  ASSERT_EQ(err, CL_SUCCESS) << "clGetProgramBuildInfo failed.";
  ASSERT_NE(strstr(param_value, "-g -cl-opt-disable"), nullptr)
      << " failed to pass build options through env.";

  err = clBuildProgram(program, 1, &device, "", nullptr, nullptr);
  ASSERT_EQ(err, CL_SUCCESS) << " clBuildProgram failed.";

  err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_OPTIONS,
                              sizeof(param_value), param_value, nullptr);
  ASSERT_EQ(err, CL_SUCCESS) << "clGetProgramBuildInfo failed.";
  ASSERT_NE(strstr(param_value, "-g -cl-opt-disable"), nullptr)
      << " failed to pass build options through env.";

  err = SETENV("OPENCL_PROGRAM_COMPILE_OPTIONS", "-non-used-option");
  ASSERT_EQ(err, 1) << " setenv failed.";

  err = clBuildProgram(program, 1, &device, "", nullptr, nullptr);
  ASSERT_EQ(err, CL_INVALID_BUILD_OPTIONS)
      << " pass invalid build option to ocl backend.";

  clReleaseProgram(program);
  clReleaseContext(context);
}
