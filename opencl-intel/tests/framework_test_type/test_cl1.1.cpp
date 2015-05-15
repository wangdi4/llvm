#include "CL/cl.h"
#include <stdio.h>
#include "FrameworkTest.h"

extern cl_device_type gDeviceType;

static const char* simple_kernel[] = {"\
    __kernel void test( __global int* buffer)\
    {}\
"};


bool clBuildWithCL11option()
{
  bool bResult = true;

  cl_platform_id platform = 0;
  cl_device_id device;
  cl_context context;

  // get platform
  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= SilentCheck(L"clGetPlatformIDs", CL_SUCCESS, iRet);
  if (!bResult)
  {
    return bResult;
  }

  // get device
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  bResult &= SilentCheck(L"clGetDeviceIDs",CL_SUCCESS, iRet);
  if (!bResult)
  {
    return bResult;
  }

  // create context
  context = clCreateContext(NULL, 1, &device, NULL, NULL, &iRet);
  bResult &= SilentCheck(L"clCreateContext",CL_SUCCESS, iRet);
  if (!bResult)
  {
    return bResult;
  }

  // create program with source
  cl_program program = clCreateProgramWithSource(context, 1, (const char**)&simple_kernel, NULL, &iRet);
  bResult &= SilentCheck(L"clCreateProgramWithSource", CL_SUCCESS, iRet);
  if (!bResult)
  {
    return bResult;
  }

  // build program
  iRet = clBuildProgram(program, 1, &device, "-cl-std=CL1.1", NULL, NULL);
  bResult &= SilentCheck(L"clBuildProgram", CL_SUCCESS, iRet);
  if (!bResult)
  {
    return bResult;
  }

  return bResult;
}
