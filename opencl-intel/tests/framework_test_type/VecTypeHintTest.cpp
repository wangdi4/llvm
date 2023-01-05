#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_device_api.h"
#include "cl_types.h"
#include <stdio.h>
#include <string>

extern cl_device_type gDeviceType;

bool VecTypeHintTest() {
  bool bResult = true;
  const char *ocl_test_program[] = {
      "kernel void kernel1(global int *a) { a[get_global_id(0)] = 0; }\n"
      "kernel __attribute__((vec_type_hint(int))) void kernel2(global int *a) "
      "{ a[get_global_id(0)] = 0; }\n"
      "kernel __attribute__((vec_type_hint(float))) void kernel3(global int "
      "*a) { a[get_global_id(0)] = 0; }\n"
      "kernel __attribute__((vec_type_hint(short))) void kernel4(global int "
      "*a) { a[get_global_id(0)] = 0; }\n"
      "kernel __attribute__((vec_type_hint(int4))) void kernel5(global int *a) "
      "{ a[get_global_id(0)] = 0; }\n"
      "kernel __attribute__((vec_type_hint(float4))) void kernel6(global int "
      "*a) { a[get_global_id(0)] = 0; }\n"
      "kernel __attribute__((vec_type_hint(short16))) void kernel7(global int "
      "*a) { a[get_global_id(0)] = 0; }\n"};
  const int num_kernels = 7;
  const bool expectedRes[] = {true, true, true, true, false, false, false};
  const char trueString[] = " successfully vectorized";
  const char falseString[] = " not vectorized";

  printf("VecTypeHintTest\n");
  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  size_t *pBinarySizes;

  cl_int *pBinaryStatus;
  cl_context context;
  cl_program clProg;

  cl_platform_id platform = 0;

  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  // get device(s)
  iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return false;
  }

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];
  pBinarySizes = new size_t[uiNumDevices];
  pBinaryStatus = new cl_int[uiNumDevices];

  iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, pDevices, NULL);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return false;
  }

  // create context
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  if (CL_SUCCESS != iRet) {
    printf("clCreateContext = %s\n", ClErrTxt(iRet));
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return false;
  }
  printf("context = %p\n", (void *)context);

  bResult &= BuildProgramSynch(context, 1, (const char **)&ocl_test_program,
                               NULL, NULL, &clProg);
  if (!bResult) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return bResult;
  }

  size_t szLogSize = 0;
  // get the binary
  iRet = clGetProgramBuildInfo(clProg, pDevices[0], CL_PROGRAM_BUILD_LOG, 0,
                               NULL, &szLogSize);
  bResult &=
      Check("clGetProgramBuildInfo(CL_PROGRAM_BUILD_LOG)", CL_SUCCESS, iRet);
  if (bResult) {
    char *pLog = new char[szLogSize];
    iRet = clGetProgramBuildInfo(clProg, pDevices[0], CL_PROGRAM_BUILD_LOG,
                                 szLogSize, pLog, &szLogSize);
    bResult &=
        Check("clGetProgramBuildInfo(CL_PROGRAM_BUILD_LOG)", CL_SUCCESS, iRet);

    // check the logs
    std::string strLog(pLog);

    size_t place = 0;
    for (int i = 0; i < num_kernels; i++) {
      std::ostringstream patternStrStream;
      patternStrStream << "Kernel \"kernel" << i + 1 << "\" was";
      std::string patternStr = patternStrStream.str();

      size_t new_place = strLog.find(patternStr, place) + patternStr.size();
      if (new_place == string::npos) {
        printf("ERROR: Cannot find log about kernel%d!\n", i + 1);
        bResult = false;
      }
      place = new_place;

      if (expectedRes[i]) {
        std::string str = strLog.substr(place, strlen(trueString));
        if (str != trueString) {
          printf("ERROR: kernel%d was supposed to be vectorized - log "
                 "indicates otherwise!\n",
                 i + 1);
          bResult = false;
        }
      } else {
        std::string str = strLog.substr(place, strlen(falseString));
        if (str != falseString) {
          printf("ERROR: kernel%d was not supposed to be vectorized - log "
                 "indicates otherwise!\n",
                 i + 1);
          bResult = false;
        }
      }
    }
    if (!bResult) {
      puts("-------- Build log --------");
      puts(pLog);
      puts("--------- Log End ---------");
    }
    delete[] pLog;
  }

  // Release objects
  delete[] pDevices;
  delete[] pBinarySizes;
  delete[] pBinaryStatus;
  clReleaseProgram(clProg);
  clReleaseContext(context);
  return bResult;
}
