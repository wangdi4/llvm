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
 * clBuildProgram
 * -------------------
 * (1) get device ids
 * (2) create context
 * (3) create binary
 * (4) create program with binary
 * (5) build program
 ******************************************************************************/

cl_program g_CUDA_clProgram = 0;

char g_CUDA_pwsUserData[256] = "BLA BLA BLA";

bool g_CUDA_bFinished = false;

bool g_CUDA_bResult = false;

static void pfn_notify(cl_program program, void *user_data) {
  g_CUDA_bResult = true;
  char *wstr = (char *)user_data;
  printf("Build Finished !!!\n");
  g_CUDA_bResult &=
      CheckInt("Check program id", (cl_int)g_CUDA_clProgram, (cl_int)program);
  g_CUDA_bResult &= CheckStr("Check user data", (char *)g_CUDA_pwsUserData,
                             (char *)user_data);

  g_CUDA_bFinished = true;
}

bool clBuildCudaProgramTest() {
  bool bResult = true;
  char szCubinName[256];
  strcpy_s(szCubinName, 256, "CUDAtest.cubin");

  printf("clBuildCubinProgramTest\n");
  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  size_t *pBinarySizes;
  void **ppBinaries;
  cl_int *pBinaryStatus;
  cl_context context;

  // get device(s)
  cl_int iRet =
      clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 0, NULL, &uiNumDevices);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return false;
  }

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];
  pBinarySizes = new size_t[uiNumDevices];
  pBinaryStatus = new cl_int[uiNumDevices];

  iRet = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, uiNumDevices, pDevices, NULL);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return false;
  }

  // create context
  context = clCreateContext(0, uiNumDevices, pDevices, NULL, NULL, &iRet);
  if (CL_SUCCESS != iRet) {
    printf("clCreateContext = %s\n", ClErrTxt(iRet));
    return false;
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
  g_CUDA_clProgram = clCreateProgramWithBinary(
      context, uiNumDevices, pDevices, pBinarySizes,
      (const unsigned char **)(&pCont), pBinaryStatus, &iRet);
  bResult &= Check("clCreateProgramWithBinary", CL_SUCCESS, iRet);

  iRet = clBuildProgram(g_CUDA_clProgram, uiNumDevices, pDevices, NULL,
                        pfn_notify, g_CUDA_pwsUserData);
  bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);

  while (!g_CUDA_bFinished) {
    Sleep(1);
  }

  bResult &= g_CUDA_bResult;

  return bResult;
}
