#include "CL/cl.h"
#include "test_utils.h"
#include <iostream>
#include <string>
#include <vector>

extern cl_device_type gDeviceType;

const char *dummy_kernel[] = {"__kernel void dummy_kernel() {}"};

bool UnloadPlatformCompiler() {
  std::cout << "============================================================="
            << std::endl;
  std::cout << "UnloadPlatformCompiler" << std::endl;
  std::cout << "============================================================="
            << std::endl;

  cl_int iRet = CL_SUCCESS;

  // Get platform.
  cl_platform_id platform = 0;
  iRet = clGetPlatformIDs(1, &platform, NULL);
  CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);

  // Get device.
  cl_device_id device = NULL;
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

  // Create context.
  cl_context context = NULL;
  {
    const cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                           (cl_context_properties)platform, 0};
    context = clCreateContext(prop, 1, &device, NULL, NULL, &iRet);
    CheckException("clCreateContext(", CL_SUCCESS, iRet);
  }

  // call with invalid value
  iRet = clUnloadPlatformCompiler((cl_platform_id)(-1));
  CheckException("clUnloadPlatformCompiler with invalid platform",
                 CL_INVALID_PLATFORM, iRet);

  // valid call
  iRet = clUnloadPlatformCompiler(platform);
  CheckException("clUnloadPlatformCompiler", CL_SUCCESS, iRet);

  // Create and build program to check that RT will "reload" compiler.
  cl_program program = 0;
  {
    size_t dummy_kernel_lengths[] = {strlen(dummy_kernel[0])};
    program = clCreateProgramWithSource(context, 1, dummy_kernel,
                                        dummy_kernel_lengths, &iRet);
    CheckException("clCreateProgramWithSource", CL_SUCCESS, iRet);

    iRet = clBuildProgram(program, 1, &device, "", NULL, NULL);
    if (CL_BUILD_PROGRAM_FAILURE == iRet) {
      std::string log(1000, ' ');
      iRet = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                   log.size(), &log[0], NULL);
      std::cout << log << std::endl;
    }
    CheckException("clBuildProgram", CL_SUCCESS, iRet);
  }

  return true;
}
