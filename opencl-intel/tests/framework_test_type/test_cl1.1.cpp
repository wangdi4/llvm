#include "CL/cl.h"
#include "FrameworkTest.h"
#include <stdio.h>

extern cl_device_type gDeviceType;

static const char *simple_kernel[] = {"\
    __kernel void test( __global int* buffer)\
    {}\
"};

bool clBuildWithCL11option() {
  bool bResult = true;

  cl_platform_id platform = 0;
  cl_device_id device;
  cl_context context;

  // get platform
  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // get device
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // create context
  context = clCreateContext(NULL, 1, &device, NULL, NULL, &iRet);
  bResult &= SilentCheck("clCreateContext", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // create program with source
  cl_program program = clCreateProgramWithSource(
      context, 1, (const char **)&simple_kernel, NULL, &iRet);
  bResult &= SilentCheck("clCreateProgramWithSource", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // build program
  iRet = clBuildProgram(program, 1, &device, "-cl-std=CL1.1", NULL, NULL);
  bResult &= SilentCheck("clBuildProgram", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  return bResult;
}
