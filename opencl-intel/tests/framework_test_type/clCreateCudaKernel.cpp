#include "CL/cl.h"
#include "FrameworkTest.h"
#include "Logger.h"
#include "cl_device_api.h"
#include "cl_objects_map.h"
#include "cl_types.h"
#include <stdio.h>
#include <windows.h>

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

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

bool g_bCudaBuildFinished = false;

static void pfn_notify(cl_program program, void *user_data) {
  g_bCudaBuildFinished = true;
}

bool clCreateCudaKernelTest() {
  printf("---------------------------------------\n");
  printf("clCreateKernel\n");
  printf("---------------------------------------\n");
  bool bResult = true;
  char szCubinName[256];
  strcpy_s(szCubinName, 256, "CUDAtest.cubin");

  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  size_t *pBinarySizes;
  void **ppBinaries;
  cl_int *pBinaryStatus;
  cl_context context;

  // get device(s)
  cl_int iRet =
      clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 0, NULL, &uiNumDevices);
  bResult &= Check("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];
  pBinarySizes = new size_t[uiNumDevices];
  pBinaryStatus = new cl_int[uiNumDevices];

  iRet = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, uiNumDevices, pDevices, NULL);
  bResult &= Check("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // create context
  context = clCreateContext(0, uiNumDevices, pDevices, NULL, NULL, &iRet);
  bResult &= Check("clCreateContext", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }
  printf("context = %d\n", context);

  // create binary
  unsigned int tmp_size,
      uiContSize = sizeof(cl_prog_container) + strlen(szCubinName) + 1;
  // Contruct program comtainer
  cl_prog_container *pCont = (cl_prog_container *)malloc(uiContSize);
  if (NULL == pCont) {
    return false;
  }

  memset(pCont, 0, sizeof(cl_prog_container));

  // Container mask
  memcpy((void *)pCont->mask, _CL_CONTAINER_MASK_, sizeof(pCont->mask));
  pCont->container_type = CL_PROG_CNT_PRIVATE;
  pCont->description.bin_type = CL_PROG_BIN_CUBIN;
  pCont->container_size = strlen(szCubinName) + 1;
  pCont->container = ((char *)pCont) + sizeof(cl_prog_container);
  strncpy_s((char *)pCont->container, strlen(szCubinName) + 1, szCubinName,
            strlen(szCubinName));
  cl_dev_program prog;

  pBinarySizes[0] = uiContSize;

  // create program with binary
  cl_program program = clCreateProgramWithBinary(
      context, uiNumDevices, pDevices, pBinarySizes,
      (const unsigned char **)(&pCont), pBinaryStatus, &iRet);
  bResult &= Check("clCreateProgramWithBinary", CL_SUCCESS, iRet);

  iRet =
      clBuildProgram(program, uiNumDevices, pDevices, NULL, pfn_notify, NULL);
  bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);

  while (!g_bCudaBuildFinished) {
    Sleep(1);
  }

  cl_kernel kernel1 = clCreateKernel(program, "aaa", &iRet);
  bResult &= Check("clCreateKernel - aaa", CL_SUCCESS, iRet);

  cl_kernel kernel2 = clCreateKernel(program, "bbb", &iRet);
  bResult &= Check("clCreateKernel - bbb", CL_SUCCESS, iRet);

  cl_kernel kernel3 = clCreateKernel(program, "ccc", &iRet);
  bResult &= Check("clCreateKernel - ccc", CL_SUCCESS, iRet);

  cl_kernel kernel4 = clCreateKernel(program, "dot_product", &iRet);
  bResult &= Check("clCreateKernel - ccc", CL_SUCCESS, iRet);

  cl_uint uiNumKernels = 0;
  iRet = clCreateKernelsInProgram(program, 0, NULL, &uiNumKernels);
  bResult &= Check("clCreateKernelsInProgram - get numbers of kernels",
                   CL_SUCCESS, iRet);
  bResult &= CheckInt("clCreateKernelsInProgram - check numbers kernels", 6,
                      uiNumKernels);

  cl_kernel pKernels[6];
  iRet = clCreateKernelsInProgram(program, uiNumKernels, pKernels, NULL);
  bResult &= Check("clCreateKernelsInProgram - get kernels", CL_SUCCESS, iRet);
  if (bResult) {
    size_t szKernelNameLength = 0;
    char psKernelName[256] = {0};
    printf("Print kernels: ");
    for (cl_uint ui = 0; ui < uiNumKernels; ++ui) {
      printf("%d: ", pKernels[ui]);
      iRet = clGetKernelInfo(pKernels[ui], CL_KERNEL_FUNCTION_NAME, 256,
                             psKernelName, NULL);
      bResult &= Check("clGetKernelInfo (function's name)", CL_SUCCESS, iRet);
      printf("%s\n", psKernelName);
    }
    printf("\n");
  }

  return bResult;
}
