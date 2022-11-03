#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif

extern cl_device_type gDeviceType;

/*******************************************************************************
 * clCreateKernel
 * -------------------
 * (1) get device ids
 * (2) create context
 * (3) create binary
 * (4) create program with binary
 * (5) build program
 * (6) create context
 ******************************************************************************/

bool g_bBuildFinished = false;

static void CL_CALLBACK pfn_notify(cl_program program, void *user_data) {
  g_bBuildFinished = true;
}

bool clCreateKernelTest(openBcFunc pFunc) {
  printf("---------------------------------------\n");
  printf("clCreateKernel\n");
  printf("---------------------------------------\n");
  bool bResult = true;

  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  size_t *pBinarySizes;
  cl_int *pBinaryStatus;
  cl_context context;
  unsigned char **ppContainers;

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
  bResult &= Check("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];
  pBinarySizes = new size_t[uiNumDevices];
  pBinaryStatus = new cl_int[uiNumDevices];
  ppContainers = new unsigned char *[uiNumDevices];

  iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, pDevices, NULL);
  bResult &= Check("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    delete[] ppContainers;
    return bResult;
  }

  // create context
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  bResult &= Check("clCreateContext", CL_SUCCESS, iRet);
  if (!bResult) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    delete[] ppContainers;
    return bResult;
  }
  printf("context = %p\n", (void *)context);

  // create binary container
  unsigned int uiContSize = 0;
  FILE *pIRfile = NULL;
  pFunc(pIRfile);
  fpos_t fileSize;
  SET_FPOS_T(fileSize, 0);
  if (NULL == pIRfile) {
    printf("Failed open file.\n");
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    delete[] ppContainers;
    return false;
  }
  fseek(pIRfile, 0, SEEK_END);
  fgetpos(pIRfile, &fileSize);
  uiContSize += (unsigned int)GET_FPOS_T(fileSize);
  fseek(pIRfile, 0, SEEK_SET);

  unsigned char *pCont = (unsigned char *)malloc(uiContSize);
  if (NULL == pCont) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    delete[] ppContainers;
    return false;
  }
  size_t ret = fread(pCont, 1, (size_t)GET_FPOS_T(fileSize), pIRfile);
  if (ret != (size_t)GET_FPOS_T(fileSize)) {
    printf("Failed read file.\n");
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    delete[] ppContainers;
    return false;
  }
  fclose(pIRfile);

  for (unsigned int i = 0; i < uiNumDevices; i++) {
    pBinarySizes[i] = uiContSize;
    ppContainers[i] = pCont;
  }

  // create program with binary
  cl_program program = clCreateProgramWithBinary(
      context, uiNumDevices, pDevices, pBinarySizes,
      const_cast<const unsigned char **>(ppContainers), pBinaryStatus, &iRet);
  bResult &= Check("clCreateProgramWithBinary", CL_SUCCESS, iRet);

  if (!bResult) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    delete[] ppContainers;
    free(pCont);
    return bResult;
  }

  // Try to create kernel before building the program.
  // According to the spec it is invalid: From OpenCL API spec ver 2.0 rev 31:
  // section 5.8.2, clBuildProgram:
  // clBuildProgram must be called for program created using either
  // clCreateProgramWithSource or clCreateProgramWithBinary to build the
  // program executable for one or more devices associated with program.

  cl_kernel invalid_kernel = clCreateKernel(program, "dot_product", &iRet);
  bResult &= Check("clCreateKernel(not-built program) - dot_product",
                   CL_INVALID_PROGRAM_EXECUTABLE, iRet);
  (void)invalid_kernel;

  g_bBuildFinished = false;
  iRet =
      clBuildProgram(program, uiNumDevices, pDevices, NULL, pfn_notify, NULL);
  bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);

  while (!g_bBuildFinished) {
    SLEEP(1);
  }

  cl_kernel kernel1 = clCreateKernel(program, "dot_product", &iRet);
  bResult &= Check("clCreateKernel - dot_product", CL_SUCCESS, iRet);

  // cl_kernel kernel2 = clCreateKernel(program, "dot_product_test", &iRet);
  // bResult &= Check("clCreateKernel - dot_product_test", CL_SUCCESS, iRet);

  // cl_kernel kernel3 = clCreateKernel(program, "foo", &iRet);
  // bResult &= Check("clCreateKernel - foo", CL_SUCCESS, iRet);

  // cl_uint uiNumKernels = 0;
  // iRet = clCreateKernelsInProgram(program, 0, NULL, &uiNumKernels);
  // bResult &= Check("clCreateKernelsInProgram - get numbers of kernels",
  // CL_SUCCESS, iRet); bResult &= CheckInt("clCreateKernelsInProgram - check
  // numbers kernels", 14, uiNumKernels);

  // cl_kernel pKernels[14];
  // iRet = clCreateKernelsInProgram(program, uiNumKernels, pKernels, NULL);
  // bResult &= Check("clCreateKernelsInProgram - get kernels", CL_SUCCESS,
  // iRet); if (bResult)
  //{
  //  size_t szKernelNameLength = 0;
  //  char psKernelName[256] = {0};
  //  printf("Print kernels: ");
  //  for (cl_uint ui=0; ui<uiNumKernels; ++ui)
  //  {
  //    printf("%d: ",pKernels[ui]);
  //    iRet = clGetKernelInfo(pKernels[ui], CL_KERNEL_FUNCTION_NAME,
  // 256, psKernelName, NULL);     bResult &= Check("clGetKernelInfo
  // (function's name)", CL_SUCCESS, iRet);     printf ("%s\n",
  // psKernelName);
  //  }
  //  printf("\n");
  // }

  // Release object
  clReleaseKernel(kernel1);
  clReleaseProgram(program);
  clReleaseContext(context);
  delete[] pDevices;
  delete[] pBinarySizes;
  delete[] pBinaryStatus;
  delete[] ppContainers;
  free(pCont);
  return bResult;
}
