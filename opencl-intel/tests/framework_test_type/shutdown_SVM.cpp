#include "CL/cl.h"
#include "test_utils.h"
#include <iostream>

extern cl_device_type gDeviceType;

void clShutdownSVMTest() {
  std::cout << "============================================================="
            << std::endl;
  std::cout << "clShutdownSVMTest" << std::endl;
  std::cout << "============================================================="
            << std::endl;

  cl_int iRet = CL_SUCCESS;
  cl_platform_id platform = 0;
  cl_device_id device = NULL;
  cl_context context = NULL;

  iRet = clGetPlatformIDs(1, &platform, NULL);
  CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);

  const cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                         (cl_context_properties)platform, 0};
  context = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
  CheckException("clCreateContextFromType", CL_SUCCESS, iRet);

  clSVMAlloc(context, CL_MEM_READ_WRITE, 2 /*Buffer size*/, 0 /*aligment*/);

  // We don't call cvSVMFree for SVMBuffer
  // and we assume RT release it properly(without hangs on) after our app died.
  clReleaseContext(context);
  clReleaseDevice(device);
}
