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

extern cl_device_type gDeviceType;

/*******************************************************************************
 * clCreateQueue
 * -------------------
 * (1) get device ids
 * (2) create context
 * (3) create binary
 * (4) create program with binary
 * (5) build program
 * (6) create kernels
 * (7) create queue
 * (8) Enqueue 3 kernels.
 ******************************************************************************/

#define KERNEL_TEST 0
#define BUFFER_TEST 1

bool clCreateQueueTest() {
  printf("---------------------------------------\n");
  printf("clCreateQueueTest\n");
  printf("---------------------------------------\n");
  bool bResult = true;
  char szDLLName[256];
  strcpy_s(szDLLName, 256, "ocl_program.dll");

  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  size_t *pBinarySizes;
  cl_int *pBinaryStatus;
  cl_context context;

  // get device(s)
  cl_int iRet = clGetDeviceIDs(NULL, gDeviceType, 0, NULL, &uiNumDevices);
  // bResult &= Check("clGetDeviceIDs",CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];
  pBinarySizes = new size_t[uiNumDevices];
  pBinaryStatus = new cl_int[uiNumDevices];

  iRet = clGetDeviceIDs(NULL, gDeviceType, uiNumDevices, pDevices, NULL);
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
  unsigned int uiContSize = sizeof(cl_prog_container) + strlen(szDLLName) + 1;
  // Contruct program comtainer
  cl_prog_container *pCont = (cl_prog_container *)malloc(uiContSize);
  if (NULL == pCont) {
    return false;
  }

  memset(pCont, 0, sizeof(cl_prog_container));

  // Container mask
  memcpy((void *)pCont->mask, _CL_CONTAINER_MASK_, sizeof(pCont->mask));
  pCont->container_type = CL_PROG_CNT_PRIVATE;
  pCont->description.bin_type = CL_PROG_DLL_X86;
  pCont->container_size = strlen(szDLLName) + 1;
  pCont->container = ((char *)pCont) + sizeof(cl_prog_container);
  strncpy_s((char *)pCont->container, strlen(szDLLName) + 1, szDLLName,
            strlen(szDLLName));
  pCont->container = NULL; // Should be NULL for user program

  pBinarySizes[0] = uiContSize;

  // create program with binary
  cl_program program = clCreateProgramWithBinary(
      context, uiNumDevices, pDevices, pBinarySizes,
      (const unsigned char **)(&pCont), pBinaryStatus, &iRet);
  bResult &= Check("clCreateProgramWithBinary", CL_SUCCESS, iRet);

  iRet = clBuildProgram(program, uiNumDevices, pDevices, NULL, NULL, NULL);
  bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);

  //
  // Create queues
  //
  cl_command_queue queue1 =
      clCreateCommandQueue(context, pDevices[0], 0 /*no properties*/, &iRet);
  bResult &= Check("clCreateCommandQueue - queue1", CL_SUCCESS, iRet);

  cl_command_queue queue2;
  cl_command_queue queue3;

  cl_uint src[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                     9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
  cl_uint src2[20] = {20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                      29, 28, 27, 26, 25, 42, 23, 22, 21, 20};
  cl_uint dst[20] = {0};

  if (KERNEL_TEST) {
    queue2 =
        clCreateCommandQueue(context, pDevices[0], 0 /*no properties*/, &iRet);
    bResult &= Check("clCreateCommandQueue - queue2", CL_SUCCESS, iRet);

    queue3 =
        clCreateCommandQueue(context, pDevices[0], 0 /*no properties*/, &iRet);
    bResult &= Check("clCreateCommandQueue - queue3", CL_SUCCESS, iRet);

    cl_kernel kernel1 = clCreateKernel(program, "dot_product", &iRet);
    bResult &= Check("clCreateKernel - dot_product", CL_SUCCESS, iRet);

    cl_kernel kernel2 = clCreateKernel(program, "dot_product_test", &iRet);
    bResult &= Check("clCreateKernel - dot_product_test", CL_SUCCESS, iRet);

    cl_kernel kernel3 = clCreateKernel(program, "foo", &iRet);
    bResult &= Check("clCreateKernel - foo", CL_SUCCESS, iRet);

    // cl_uint uiNumKernels = 0;
    // iRet = clCreateKernelsInProgram(program, 0, NULL, &uiNumKernels);
    // bResult &= Check("clCreateKernelsInProgram - get numbers of kernels",
    // CL_SUCCESS, iRet); bResult &= CheckInt("clCreateKernelsInProgram - check
    // numbers kernels", 14, uiNumKernels);

    // cl_kernel pKernels[14];
    // iRet = clCreateKernelsInProgram(program, uiNumKernels, pKernels, NULL);
    // bResult &= Check("clCreateKernelsInProgram - get kernels", CL_SUCCESS,
    // iRet);

    //
    // Execute commands
    //

    size_t global_work_size[1] = {1};
    size_t local_work_size[1] = {1};
    cl_event waitEvents[5];

    // cmd 0
    clEnqueueNDRangeKernel(queue2, kernel1, 1, NULL, global_work_size,
                           local_work_size, 0, NULL, NULL);
    // cmd 1
    clEnqueueNDRangeKernel(queue2, kernel1, 1, NULL, global_work_size,
                           local_work_size, 0, NULL, NULL);
    // cmd 2
    clEnqueueNDRangeKernel(queue2, kernel1, 1, NULL, global_work_size,
                           local_work_size, 0, NULL, NULL);
    // cmd 3
    clEnqueueNDRangeKernel(queue2, kernel1, 1, NULL, global_work_size,
                           local_work_size, 0, NULL, &waitEvents[0]);
    // cmd 4
    clEnqueueNDRangeKernel(queue2, kernel1, 1, NULL, global_work_size,
                           local_work_size, 0, NULL, &waitEvents[2]);
    // cmd 5
    clEnqueueNDRangeKernel(queue3, kernel1, 1, NULL, global_work_size,
                           local_work_size, /*0, NULL */ 1, waitEvents, NULL);
    // cmd 6
    clEnqueueNDRangeKernel(queue3, kernel1, 1, NULL, global_work_size,
                           local_work_size, 0, NULL, NULL);
    // cmd 7
    clEnqueueNDRangeKernel(queue1, kernel1, 1, NULL, global_work_size,
                           local_work_size, 1, waitEvents, NULL);
    // cmd 8
    clEnqueueNDRangeKernel(queue1, kernel1, 1, NULL, global_work_size,
                           local_work_size, 0, NULL, NULL);
    // cmd 9
    clEnqueueNDRangeKernel(queue1, kernel1, 1, NULL, global_work_size,
                           local_work_size, 0, NULL, &waitEvents[1]);
    // cmd 10
    clEnqueueNDRangeKernel(queue1, kernel1, 1, NULL, global_work_size,
                           local_work_size, 0, NULL, &waitEvents[3]);
    // cmd 11
    clEnqueueNDRangeKernel(queue3, kernel1, 1, NULL, global_work_size,
                           local_work_size, /*0, NULL*/ 1, &waitEvents[3],
                           NULL);
    // cmd 12
    clEnqueueNDRangeKernel(queue3, kernel1, 1, NULL, global_work_size,
                           local_work_size, 0, NULL, &waitEvents[4]);

    // Wait until all queues finish;
    iRet = clWaitForEvents(3, waitEvents + 2);
    bResult &= Check("clWaitForEvents - waitEvents", CL_SUCCESS, iRet);

    //
    // Release objects
    //
    for (int i = 0; i < 5; i++) {
      iRet = clReleaseEvent(waitEvents[i]);
      bResult &= Check("clReleaseEvent - waitEvent", CL_SUCCESS, iRet);
    }
  }

  if (BUFFER_TEST) {

    size_t size = sizeof(cl_uint);
    cl_event waitEvent;

    cl_mem buffer =
        clCreateBuffer(context, CL_MEM_READ_ONLY, size * 20, NULL, &iRet);
    bResult &= Check("clCreateBuffer", CL_SUCCESS, iRet);

    iRet = clEnqueueWriteBuffer(queue1, buffer, false, 0, size * 20, src, 0,
                                NULL, NULL);
    bResult &= Check("clEnqueueWriteBuffer", CL_SUCCESS, iRet);

    iRet = clEnqueueReadBuffer(queue1, buffer, false, size * 10, size * 10, dst,
                               0, NULL, &waitEvent);
    bResult &= Check("clEnqueueReadBuffer", CL_SUCCESS, iRet);

    iRet = clEnqueueWriteBuffer(queue1, buffer, false, 0, size * 20, src2, 0,
                                NULL, NULL);
    bResult &= Check("clEnqueueWriteBuffer", CL_SUCCESS, iRet);

    // Wait until all queues finish;
    iRet = clWaitForEvents(1, &waitEvent);
    bResult &= Check("clWaitForEvents - waitEvents", CL_SUCCESS, iRet);

    iRet = clEnqueueReadBuffer(queue1, buffer, true, 0, size * 10, dst, 0, NULL,
                               NULL);
    bResult &= Check("clEnqueueReadBuffer", CL_SUCCESS, iRet);
  }

  iRet = clReleaseCommandQueue(queue1);
  bResult &= Check("clReleaseCommandQueue - queue1", CL_SUCCESS, iRet);

  if (KERNEL_TEST) {
    iRet = clReleaseCommandQueue(queue2);
    bResult &= Check("clReleaseCommandQueue - queue2", CL_SUCCESS, iRet);

    iRet = clReleaseCommandQueue(queue3);
    bResult &= Check("clReleaseCommandQueue - queue3", CL_SUCCESS, iRet);
  }

  return bResult;
}
