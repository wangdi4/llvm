#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_cpu_detect.h"
#include "cl_types.h"
#include "common_utils.h"
#include <fstream>
#include <stdio.h>

using namespace Intel::OpenCL::Utils;

extern cl_device_type gDeviceType;

/*******************************************************************************
  clCheckCPUArchForJIT
  --------------------
  load invalid binaries and try to create program with these binaries
  binary file are saved before for invalid CPU architecture (by
  GenerateBinaryFile() function) Expect fail: clCreateProgramWithBinary
  *****************************************************************************/

bool GenerateBinaryFile() {
  bool bResult = true;

  const char *ocl_test_program[] = {
      "__kernel void test_kernel(__global int* pBuff0, __global int* pBuff1, "
      "__global int* pBuff2)"
      "{"
      "    size_t id = get_global_id(0);"
      "    pBuff0[id] = pBuff1[id] ? 5 : pBuff2[id];"
      "    float cosAngle, sinAngle;"
      "    cosAngle = cos(convert_float(pBuff1[id]));"
      "    sinAngle = sin(convert_float(pBuff2[id]));"
      "}"};

  const char *filename = nullptr;
  printf("GenerateBinaryFile\n");

  if (!CPUDetect::GetInstance()->IsFeatureSupportedOnHost(CFS_AVX10)) {
    if (!SETENV("CL_CONFIG_CPU_TARGET_ARCH", "corei7-avx")) {
      printf("ERROR: GenerateBinaryFile: Can't set environment variable. Test "
             "FAILED\n");
      return false;
    }
    filename = "avx1.bin";
  } else {
    if (!SETENV("CL_CONFIG_CPU_TARGET_ARCH", "corei7")) {
      printf("ERROR: GenerateBinaryFile: Can't set environment variable. Test "
             "FAILED\n");
      return false;
    }
    filename = "sse4.bin";
  }

  cl_platform_id platform = 0;
  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  cl_device_id device;
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return false;
  }

  // create context
  cl_context context = clCreateContext(prop, 1, &device, NULL, NULL, &iRet);
  if (CL_SUCCESS != iRet) {
    printf("clCreateContext = %s\n", ClErrTxt(iRet));
    return false;
  }
  printf("context = %p\n", (void *)context);

  cl_program clProg;
  bResult &= BuildProgramSynch(context, 1, (const char **)&ocl_test_program,
                               NULL, "-cl-denorms-are-zero", &clProg);
  if (!bResult) {
    clReleaseProgram(clProg);
    clReleaseContext(context);
    return bResult;
  }

  size_t binarySize = 0;
  char *pBinaries = NULL;
  if (bResult) {
    // get the binary
    iRet = clGetProgramInfo(clProg, CL_PROGRAM_BINARY_SIZES, sizeof(size_t),
                            &binarySize, NULL);
    bResult &=
        Check("clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)", CL_SUCCESS, iRet);
    if (bResult) {
      pBinaries = new char[binarySize];
      iRet = clGetProgramInfo(clProg, CL_PROGRAM_BINARIES, binarySize,
                              &pBinaries, NULL);
      bResult &=
          Check("clGetProgramInfo(CL_PROGRAM_BINARIES)", CL_SUCCESS, iRet);
      if (bResult) {
        FILE *fout;
        fout = fopen(filename, "wb");
        if (NULL == fout) {
          printf("Failed open file.\n");
          clReleaseProgram(clProg);
          clReleaseContext(context);
          return false;
        }
        fwrite(pBinaries, 1, binarySize, fout);
        fclose(fout);
        printf("Saved successfully!! [size = %zu] \n", binarySize);
      }
    }
  }

  clReleaseProgram(clProg);
  clReleaseContext(context);

  if (!UNSETENV("CL_CONFIG_CPU_TARGET_ARCH")) {
    printf(
        "ERROR GenerateBinaryFile: Can't unset CL_CONFIG_CPU_ARCH environment "
        "variable. Test FAILED\n");
    return false;
  }

  return bResult;
}

bool clCheckCPUArchForJIT() {
  if (gDeviceType != CL_DEVICE_TYPE_CPU &&
      gDeviceType != CL_DEVICE_TYPE_ACCELERATOR /* fpga emulator case */) {
    return true;
  }

  if (!UNSETENV("CL_CONFIG_CPU_TARGET_ARCH")) {
    printf("ERROR clCheckCPUArchForJIT: Can't unset CL_CONFIG_CPU_ARCH "
           "environment variable. Test FAILED\n");
    return false;
  }

  cl_context context;
  cl_device_id device;

  bool bResult = true;

  printf("clCheckCPUArchForJIT\n");

  cl_platform_id platform = 0;

  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  if (CL_SUCCESS != iRet) {
    printf("clGetPlatformIDs = %s\n", ClErrTxt(iRet));
    return false;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};
  // initialize
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return false;
  }

  // create context
  context = clCreateContext(prop, 1, &device, NULL, NULL, &iRet);
  if (CL_SUCCESS != iRet) {
    printf("clCreateContext = %s\n", ClErrTxt(iRet));
    return false;
  }
  printf("Initialized context\n");

  unsigned int uiContSize = 0;

  // open binary file
  FILE *fout = NULL;
  if (!CPUDetect::GetInstance()->IsFeatureSupportedOnHost(CFS_AVX10))
    fout = fopen("avx1.bin", "rb");
  else
    fout = fopen("sse4.bin", "rb");
  fpos_t fileSize;
  SET_FPOS_T(fileSize, 0);
  if (NULL == fout) {
    printf("Failed open file.\n");
    clReleaseContext(context);
    return false;
  }
  fseek(fout, 0, SEEK_END);
  fgetpos(fout, &fileSize);
  uiContSize += (unsigned int)GET_FPOS_T(fileSize);
  fseek(fout, 0, SEEK_SET);

  assert(uiContSize > 0 && "the input file must not be empty");
  unsigned char *pCont = (unsigned char *)malloc(uiContSize);
  if (NULL == pCont) {
    fclose(fout);
    clReleaseContext(context);
    return false;
  }
  // construct program container
  size_t ret =
      fread(((unsigned char *)pCont), 1, (size_t)GET_FPOS_T(fileSize), fout);
  if (ret != (size_t)GET_FPOS_T(fileSize)) {
    printf("Failed read file.\n");
    fclose(fout);
    clReleaseContext(context);
    return false;
  }
  fclose(fout);

  size_t binarySize = uiContSize;

  // create program with binary
  cl_int binaryStatus;
  cl_program program = clCreateProgramWithBinary(
      context, 1, &device, &binarySize,
      const_cast<const unsigned char **>(&pCont), &binaryStatus, &iRet);
  bResult &= Check("clCreateProgramWithBinary", CL_INVALID_BINARY, iRet);
  bResult &= Check("binaryStatus = CL_INVALID_BINARY", CL_INVALID_BINARY,
                   binaryStatus);

  // Release objects
  clReleaseProgram(program);
  clReleaseContext(context);

  return bResult;
}
