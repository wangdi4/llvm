#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>

/*******************************************************************************
 * clBuildEmptyProgram
 * -------------------
 * (1) get device ids
 * (2) create context
 * (3) create binary
 * (4) create program with empty source
 * (5) build program
 * (6) try to create kernel in program
 * (7) check that create kernel failed
 ******************************************************************************/

bool clBuildEmptyProgramTest() {
  bool bResult = true;
  const char *ocl_test_program[] = {""};

  printf("clBuildEmptyProgramTest\n");
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
  iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &uiNumDevices);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return false;
  }

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];
  pBinarySizes = new size_t[uiNumDevices];
  pBinaryStatus = new cl_int[uiNumDevices];

  iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, uiNumDevices, pDevices,
                        NULL);
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
    clReleaseContext(context);
    return bResult;
  }

  if (bResult) {
    // get the binary
    iRet = clGetProgramInfo(clProg, CL_PROGRAM_BINARY_SIZES,
                            sizeof(size_t) * uiNumDevices, pBinarySizes, NULL);
    bResult &=
        Check("clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)", CL_SUCCESS, iRet);
    if (!bResult) {
      delete[] pDevices;
      delete[] pBinarySizes;
      delete[] pBinaryStatus;
      clReleaseProgram(clProg);
      clReleaseContext(context);
      return bResult;
    }

    size_t sumBinariesSize = 0;
    char **pBinaries = new char *[uiNumDevices];
    for (unsigned int i = 0; i < uiNumDevices; i++) {
      pBinaries[i] = new char[pBinarySizes[i]];
      sumBinariesSize += pBinarySizes[i];
    }
    iRet = clGetProgramInfo(clProg, CL_PROGRAM_BINARIES, sumBinariesSize,
                            pBinaries, NULL);
    bResult &= Check("clGetProgramInfo(CL_PROGRAM_BINARIES)", CL_SUCCESS, iRet);

    for (unsigned int i = 0; i < uiNumDevices; i++) {
      delete[] (pBinaries[i]);
    }
    delete[] pBinaries;

    clCreateKernel(clProg, "test_kernel", &iRet);
    bResult = SilentCheck("clCreateKernel", CL_INVALID_KERNEL_NAME, iRet);
    if (!bResult) {
      delete[] pDevices;
      delete[] pBinarySizes;
      delete[] pBinaryStatus;
      clReleaseProgram(clProg);
      clReleaseContext(context);
      return bResult;
    }

    cl_kernel kernels[5] = {0};
    cl_uint num_kernels;
    iRet = clCreateKernelsInProgram(clProg, 5, kernels, &num_kernels);
    bResult = SilentCheck("clCreateKernelsInProgram", CL_SUCCESS, iRet);
    if (!bResult) {
      delete[] pDevices;
      delete[] pBinarySizes;
      delete[] pBinaryStatus;
      clReleaseProgram(clProg);
      clReleaseContext(context);
      return bResult;
    }

    bResult = SilentCheck("Check num kernels", 0, num_kernels);
    if (!bResult) {
      delete[] pDevices;
      delete[] pBinarySizes;
      delete[] pBinaryStatus;
      clReleaseProgram(clProg);
      clReleaseContext(context);
      return bResult;
    }
  }

  // Release objects
  delete[] pDevices;
  delete[] pBinarySizes;
  delete[] pBinaryStatus;
  clReleaseProgram(clProg);
  clReleaseContext(context);
  return bResult;
}
