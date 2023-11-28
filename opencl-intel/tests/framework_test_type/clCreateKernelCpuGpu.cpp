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

bool g_bCpuGpuBuildFinished = false;

static void pfn_notify(cl_program program, void *user_data) {
  g_bCpuGpuBuildFinished = true;
}

bool clCreateKernelCpuGpuTest() {
  printf("---------------------------------------\n");
  printf("clCreateKernel\n");
  printf("---------------------------------------\n");
  bool bResult = true;
  char szDLLName[256];
  strcpy_s(szDLLName, 256, "ocl_program.dll");
  char szCubinName[256];
  strcpy_s(szCubinName, 256, "CUDAtest.cubin");

  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  size_t *pBinarySizes;
  const void **ppBinaries;
  cl_int *pBinaryStatus;
  cl_context context;

  // get device(s)
  cl_int iRet = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU, 0,
                               NULL, &uiNumDevices);
  bResult &= Check("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];
  pBinarySizes = new size_t[uiNumDevices];
  ppBinaries = (const void **)new int[uiNumDevices];
  pBinaryStatus = new cl_int[uiNumDevices];

  iRet = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU,
                        uiNumDevices, pDevices, NULL);
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

  // create binaries

  cl_prog_container *pCont;
  unsigned int uiContSize;

  uiContSize = sizeof(cl_prog_container) + strlen(szDLLName) + 1;
  pCont = (cl_prog_container *)malloc(uiContSize);
  if (NULL == pCont) {
    return false;
  }
  memset(pCont, 0, sizeof(cl_prog_container));
  memcpy((void *)pCont->mask, _CL_CONTAINER_MASK_, sizeof(pCont->mask));
  pCont->container_type = CL_PROG_CNT_PRIVATE;
  pCont->description.bin_type = CL_PROG_DLL_X86;
  pCont->container_size = strlen(szDLLName) + 1;
  pCont->container = ((char *)pCont) + sizeof(cl_prog_container);
  strncpy_s((char *)pCont->container, strlen(szDLLName) + 1, szDLLName,
            strlen(szDLLName));

  pBinarySizes[0] = uiContSize;
  ppBinaries[0] = pCont;

  uiContSize = sizeof(cl_prog_container) + strlen(szCubinName) + 1;
  pCont = (cl_prog_container *)malloc(uiContSize);
  if (NULL == pCont) {
    return false;
  }
  memset(pCont, 0, sizeof(cl_prog_container));
  memcpy((void *)pCont->mask, _CL_CONTAINER_MASK_, sizeof(pCont->mask));
  pCont->container_type = CL_PROG_CNT_PRIVATE;
  pCont->description.bin_type = CL_PROG_BIN_CUBIN;
  pCont->container_size = strlen(szCubinName) + 1;
  pCont->container = ((char *)pCont) + sizeof(cl_prog_container);
  strncpy_s((char *)pCont->container, strlen(szCubinName) + 1, szCubinName,
            strlen(szCubinName));

  pBinarySizes[1] = uiContSize;
  ppBinaries[1] = pCont;

  // create program with binary
  cl_program program = clCreateProgramWithBinary(
      context, uiNumDevices, pDevices, pBinarySizes,
      (const unsigned char **)ppBinaries, pBinaryStatus, &iRet);
  bResult &= Check("clCreateProgramWithBinary", CL_SUCCESS, iRet);

  iRet =
      clBuildProgram(program, uiNumDevices, pDevices, NULL, pfn_notify, NULL);
  bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);

  while (!g_bCpuGpuBuildFinished) {
    Sleep(1);
  }

  cl_kernel kernel1 = clCreateKernel(program, "fooi", &iRet);
  bResult &= Check("clCreateKernel - fooi", CL_SUCCESS, iRet);

  cl_kernel kernel2 = clCreateKernel(program, "foof", &iRet);
  // this function should fail since foof argument list is different in each
  // device;
  bResult &= !(Check("clCreateKernel - foof", CL_SUCCESS, iRet));

  return bResult;
}
