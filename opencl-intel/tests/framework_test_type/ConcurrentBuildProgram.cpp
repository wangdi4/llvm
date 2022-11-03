#include "CL/cl.h"
#include "FrameworkTest.h"
#include <stdio.h>

extern cl_device_type gDeviceType;

static volatile bool gBuildDone = false;

static void CL_CALLBACK buildCallback(cl_program prog, void *userData) {
  gBuildDone = true;
}

bool ConcurrentBuildProgramTest() {
  const size_t programSize = 1000;
  const char progPrefix[] = "__kernel void k(__global float* pBuff) {\n";
  const char repeatingLine[] = "pBuff[0] += 1.0f;\n";
  const char progSuffix[] = "}\n";
  const size_t repeatingLineLength = sizeof(repeatingLine) - 1;
  const size_t progPrefixLength = sizeof(progPrefix) - 1;
  const size_t progSuffixLength = sizeof(progSuffix) - 1;
  const size_t progVariableLength =
      programSize - 1 - progPrefixLength - progSuffixLength;

  bool bResult = true;

  char *oclProgram = new char[programSize];
  size_t progChar = 0;
  memcpy(oclProgram + progChar, progPrefix, progPrefixLength);
  progChar += progPrefixLength;
  for (size_t i = 0; i < progVariableLength / repeatingLineLength; ++i) {
    memcpy(oclProgram + progChar, repeatingLine, repeatingLineLength);
    progChar += repeatingLineLength;
  }
  memcpy(oclProgram + progChar, progSuffix, progSuffixLength);
  progChar += progSuffixLength;
  oclProgram[progChar++] = '\0';

  printf("ConcurrentBuildProgramTest\n");
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
    delete[] oclProgram;
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  // get device(s)
  iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
  if (CL_SUCCESS != iRet) {
    delete[] oclProgram;
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
    delete[] oclProgram;
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return false;
  }

  // create context
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  if (CL_SUCCESS != iRet) {
    printf("clCreateContext = %s\n", ClErrTxt(iRet));
    delete[] oclProgram;
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return false;
  }
  printf("context = %p\n", (void *)context);

  printf("Building program %s\n", oclProgram);

  clProg = clCreateProgramWithSource(
      context, 1, const_cast<const char **>(&oclProgram), NULL, &iRet);
  bResult &= Check("clCreateProgramWithSource", CL_SUCCESS, iRet);

  gBuildDone = false;
  iRet = clBuildProgram(clProg, 0, NULL, NULL, buildCallback, NULL);
  bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);

  iRet = clBuildProgram(clProg, 0, NULL, NULL, buildCallback, NULL);
  if (!gBuildDone) {
    bResult &= Check("clBuildProgram", CL_INVALID_OPERATION, iRet);
  }
  while (!gBuildDone)
    ;

  cl_kernel dummy = clCreateKernel(clProg, "k", &iRet);
  bResult &= Check("clCreateKernel", CL_SUCCESS, iRet);

  iRet = clBuildProgram(clProg, 0, NULL, NULL, NULL, NULL);
  bResult &=
      Check("clBuildProgram after kernel creation", CL_INVALID_OPERATION, iRet);

  // Release objects
  delete[] oclProgram;
  delete[] pDevices;
  delete[] pBinarySizes;
  delete[] pBinaryStatus;
  clReleaseKernel(dummy);
  clReleaseProgram(clProg);
  clReleaseContext(context);
  return bResult;
}
