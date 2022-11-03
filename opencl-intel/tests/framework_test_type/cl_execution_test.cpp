#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <math.h>
#include <stdio.h>

#define BUFFERS_LENGTH 20000
// #define TASK_TEST
// #define NATIVE_KERNEL_TEST

extern cl_device_type gDeviceType;

/*******************************************************************************
 * cl_execution_test - implement dot_product example
 * -------------------
 * (1)
 * (2)
 ******************************************************************************/

void CL_CALLBACK _KerenlTestFunc(void *params);

/*******************************************************************************
 * Native Kernel test
 *
 ******************************************************************************/
struct SKernelTestParams {
  size_t szInBuffer1Size;
  cl_uint *pInBuffer1;
  size_t szInBuffer2Size;
  cl_uint *pInBuffer2;
  cl_uint *pOutBuffer;
  size_t szOutBufferSize;
};

void CL_CALLBACK _KerenlTestFunc(void *params) {
  // Set function arguments
  SKernelTestParams *pKernelTestParams = (SKernelTestParams *)params;
  size_t szInBuffer1Size = pKernelTestParams->szInBuffer1Size;
  cl_uint *pInBuffer1 = pKernelTestParams->pInBuffer1;
  size_t szInBuffer2Size = pKernelTestParams->szInBuffer2Size;
  cl_uint *pInBuffer2 = pKernelTestParams->pInBuffer2;
  cl_uint *pOutBuffer = pKernelTestParams->pOutBuffer;
  size_t szOutBufferSize = pKernelTestParams->szOutBufferSize;

  // calculation
  size_t index;
  for (index = 0; index < szOutBufferSize; index++) {
    pOutBuffer[index] = pInBuffer1[index % szInBuffer1Size] *
                        pInBuffer2[index % szInBuffer2Size];
  }
}

bool test_native_kernel(cl_command_queue queue1, cl_mem buffer_srcA,
                        cl_mem buffer_srcB, cl_mem buffer_dst) {
  cl_err_code iRet;
  static const int iDstSize = 40;

  // Create buffers
  cl_uint srcA[] = {1, 2, 3, 4, 5, 6, 7};
  cl_uint srcB[] = {0, 1, 0, 1};
  cl_uint dst[iDstSize] = {0};

  // Write buffers
  iRet = clEnqueueWriteBuffer(queue1, buffer_srcA, CL_FALSE, 0,
                              7 * sizeof(cl_uint), srcA, 0, NULL, NULL);
  if (CL_SUCCESS != iRet) {
    printf("clEnqueueWriteBuffer(1) = %s\n", ClErrTxt(iRet));
    return false;
  }
  iRet = clEnqueueWriteBuffer(queue1, buffer_srcB, CL_FALSE, 0,
                              4 * sizeof(cl_uint), srcB, 0, NULL, NULL);
  if (CL_SUCCESS != iRet) {
    printf("clEnqueueWriteBuffer(1) = %s\n", ClErrTxt(iRet));
    return false;
  }

  // Execute kernel
  // Prepare arguments
  SKernelTestParams params = {7, NULL, 4, NULL, NULL, iDstSize};
  cl_mem memList[] = {buffer_srcA, buffer_srcB, buffer_dst};
  void *ppArgsMemLoc[3];
  ppArgsMemLoc[0] = (void *)(&(params.pInBuffer1));
  ppArgsMemLoc[1] = (void *)(&(params.pInBuffer2));
  ppArgsMemLoc[2] = (void *)(&(params.pOutBuffer));

  iRet = clEnqueueNativeKernel(
      queue1, _KerenlTestFunc, &params, sizeof(SKernelTestParams), 3, memList,
      const_cast<const void **>(ppArgsMemLoc), 0, NULL, NULL);
  if (CL_SUCCESS != iRet) {
    printf("clEnqueueNativeKernel(1) = %s\n", ClErrTxt(iRet));
    return false;
  }

  // Read output and print
  iRet = clEnqueueReadBuffer(queue1, buffer_dst, CL_TRUE, 0,
                             iDstSize * sizeof(cl_uint), dst, 0, NULL, NULL);
  if (CL_SUCCESS != iRet) {
    printf("clEnqueueReadBuffer(1) = %s\n", ClErrTxt(iRet));
    return false;
  }

  //
  // Print kernel output
  //
  printf("\n ==== \n");
  for (int i = 0; i < iDstSize; i++) {
    printf("%d, ", dst[i]);
  }
  printf("\n ==== \n");

  return true;
}

/*******************************************************************************
 *
 *
 ******************************************************************************/
bool clExecutionTest() {
  printf("---------------------------------------\n");
  printf("clExecutionTest\n");
  printf("---------------------------------------\n");
  const char *ocl_test_program[] = {
      "__kernel void dot_product (__global const float4 *a, __global const "
      "float4 *b, __global float4 *c, int numElements)\n"
      "{\n"
      "if ( get_global_size(0) != numElements ) return;\n"
      "int tid = get_global_id(0);\n"
      "c[tid] = fma(a[tid], b[tid], c[tid]);\n"
      "}\n"};

  bool bResult = true;
  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  size_t *pBinarySizes;
  cl_int *pBinaryStatus;
  cl_context context;

  cl_platform_id platform = 0;

  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  // get device(s)
  iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
  bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];
  pBinarySizes = new size_t[uiNumDevices];
  pBinaryStatus = new cl_int[uiNumDevices];

  iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, pDevices, NULL);
  bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return bResult;
  }

  // create context
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  bResult &= SilentCheck("clCreateContext", CL_SUCCESS, iRet);
  if (!bResult) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return bResult;
  }
  printf("context = %p\n", (void *)context);

  //
  // Create queue
  //
  cl_command_queue queue1 = clCreateCommandQueueWithProperties(
      context, pDevices[0], NULL /*no properties*/, &iRet);
  bResult &= SilentCheck("clCreateCommandQueueWithProperties - queue1",
                         CL_SUCCESS, iRet);

  cl_context cntxInfo;
  iRet = clGetCommandQueueInfo(queue1, CL_QUEUE_CONTEXT, sizeof(cl_context),
                               &cntxInfo, NULL);
  bResult &= SilentCheck("clGetCommandQueueInfo", CL_SUCCESS, iRet);
  bResult &= CheckHandle("clGetCommandQueueInfo - context", context, cntxInfo);

  // create program with source
  cl_program program = clCreateProgramWithSource(
      context, 1, (const char **)&ocl_test_program, NULL, &iRet);
  bResult &= SilentCheck("clCreateProgramWithSource", CL_SUCCESS, iRet);

  iRet = clBuildProgram(program, uiNumDevices, pDevices, "-cl-denorms-are-zero",
                        NULL, NULL);
  bResult &= SilentCheck("clBuildProgram", CL_SUCCESS, iRet);

  //
  // Create Kernel
  //
  cl_kernel kernel1 = clCreateKernel(program, "dot_product", &iRet);
  bResult &= SilentCheck("clCreateKernel - dot_product", CL_SUCCESS, iRet);

  //
  // From here down it is the program execution implementation
  //
  cl_float srcA[BUFFERS_LENGTH];
  cl_float srcB[BUFFERS_LENGTH];
  cl_float dst[BUFFERS_LENGTH];

  for (int j = 0; j < BUFFERS_LENGTH; j++) {
    srcA[j] = 0.2f;
    srcB[j] = 0.3f;
    dst[j] = 0.0f;
  }
  double expected_result = 0.06; // dst += A*B

  //
  // Create buffers
  //
  size_t size = sizeof(cl_float);

  cl_mem buffer_srcA = clCreateBuffer(context, CL_MEM_READ_ONLY,
                                      size * BUFFERS_LENGTH, NULL, &iRet);
  bResult &= SilentCheck("clCreateBuffer - srcA", CL_SUCCESS, iRet);

  cl_mem buffer_srcB = clCreateBuffer(context, CL_MEM_READ_ONLY,
                                      size * BUFFERS_LENGTH, NULL, &iRet);
  bResult &= SilentCheck("clCreateBuffer - srcB", CL_SUCCESS, iRet);

  cl_mem buffer_dst = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                     size * BUFFERS_LENGTH, NULL, &iRet);
  bResult &= SilentCheck("clCreateBuffer - dst", CL_SUCCESS, iRet);

#ifdef NATIVE_KERNEL_TEST
  test_native_kernel(queue1, buffer_srcA, buffer_srcB, buffer_dst);
  delete[] pDevices;
  delete[] pBinarySizes;
  delete[] pBinaryStatus;
  return bResult;
#endif

  //
  // Set arguments
  //
  iRet = clSetKernelArg(kernel1, 0, sizeof(cl_mem), &buffer_srcA);
  bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);

  iRet = clSetKernelArg(kernel1, 1, sizeof(cl_mem), &buffer_srcB);
  bResult &= SilentCheck("clSetKernelArg - buffer_srcB", CL_SUCCESS, iRet);

  iRet = clSetKernelArg(kernel1, 2, sizeof(cl_mem), &buffer_dst);
  bResult &= SilentCheck("clSetKernelArg - buffer_dst", CL_SUCCESS, iRet);

  int itemCount = BUFFERS_LENGTH / 4;
  iRet = clSetKernelArg(kernel1, 3, sizeof(int), &itemCount);
  bResult &= SilentCheck("clSetKernelArg - buffer_dst", CL_SUCCESS, iRet);

  //
  // Execute commands - Write buffers
  //
  iRet = clEnqueueWriteBuffer(queue1, buffer_srcA, false, 0,
                              size * BUFFERS_LENGTH, srcA, 0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueWriteBuffer - srcA", CL_SUCCESS, iRet);

  iRet = clEnqueueWriteBuffer(queue1, buffer_srcB, false, 0,
                              size * BUFFERS_LENGTH, srcB, 0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueWriteBuffer - srcB", CL_SUCCESS, iRet);

  iRet = clEnqueueWriteBuffer(queue1, buffer_dst, false, 0,
                              size * BUFFERS_LENGTH, dst, 0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueWriteBuffer - srcB", CL_SUCCESS, iRet);
  //
  // Execute kernel - dot_product
  //

#ifdef TASK_TEST
  for (int index = 0; index < 10; index++) {
    iRet = clEnqueueTask(queue1, kernel1, 0, NULL, NULL);
  }
  iRet = clEnqueueReadBuffer(queue1, buffer_dst, CL_TRUE, 0,
                             size * BUFFERS_LENGTH, dst, 0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, iRet);

  //
  // Print kernel output
  //
  printf("\n ==== \n");
  for (int i = 0; i < 10; i++) {
    printf("%lf, ", dst[i]);
  }
  printf("\n ==== \n");

#else
  size_t global_work_size[1] = {BUFFERS_LENGTH / 4};
  size_t local_work_size[1] = {1};

  for (int index = 0; index < 4; index++) {
    for (int j = 0; j < BUFFERS_LENGTH; j++) {
      dst[j] = 0.0f;
    }
    iRet = clEnqueueWriteBuffer(queue1, buffer_dst, false, 0,
                                size * BUFFERS_LENGTH, dst, 0, NULL, NULL);
    bResult &= SilentCheck("clEnqueueWriteBuffer - dst", CL_SUCCESS, iRet);

    cl_event ndrEvent;
    iRet = clEnqueueNDRangeKernel(queue1, kernel1, 1, NULL, global_work_size,
                                  local_work_size, 0, NULL, &ndrEvent);
    bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

    iRet = clWaitForEvents(1, &ndrEvent);
    bResult &= SilentCheck("clWaitForCompletion", CL_SUCCESS, iRet);
    clReleaseEvent(ndrEvent);

    //
    // Read results. wait for completion - blocking!
    //
    iRet = clEnqueueReadBuffer(queue1, buffer_dst, CL_TRUE, 0,
                               size * BUFFERS_LENGTH, dst, 0, NULL, NULL);
    bResult &= SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, iRet);

    //
    // Print kernel output
    //
    printf("\n ==== Validating Results ==== \n");
    double allowed_epsilon = 0.00001;
    int expected, received;
    memcpy(&expected, &expected_result, sizeof(int));
    for (unsigned int i = 0; i < BUFFERS_LENGTH; i++) {
      double diff = fabs(expected_result - (double)dst[0]);
      if (diff > allowed_epsilon) {
        bResult = false;
        memcpy(&received, &dst[i], sizeof(int));
        printf("\n ERROR: Expected = 0x%x, Received = 0x%x, diff = %e\n",
               expected, received, diff);
        break;
      }
    }
    printf("\n ==== Result: %d \n", bResult);
  }
#endif
  //
  // Release objects
  //
  iRet = clReleaseMemObject(buffer_dst);
  bResult &= SilentCheck("clReleaseBuffer - buffer_dst", CL_SUCCESS, iRet);

  iRet = clReleaseMemObject(buffer_srcA);
  bResult &= SilentCheck("clReleaseBuffer - buffer_srcA", CL_SUCCESS, iRet);

  iRet = clReleaseMemObject(buffer_srcB);
  bResult &= SilentCheck("clReleaseBuffer - buffer_srcB", CL_SUCCESS, iRet);

  iRet = clReleaseKernel(kernel1);
  bResult &= SilentCheck("clReleaseKernel - kernel1", CL_SUCCESS, iRet);

  iRet = clReleaseProgram(program);
  bResult &= SilentCheck("clReleaseProgram - program", CL_SUCCESS, iRet);

  iRet = clFinish(queue1);
  bResult &= SilentCheck("clFinish - queue1", CL_SUCCESS, iRet);

  iRet = clReleaseCommandQueue(queue1);
  bResult &= SilentCheck("clReleaseCommandQueue - queue1", CL_SUCCESS, iRet);

  iRet = clReleaseContext(context);
  bResult &= SilentCheck("clReleaseContext - context", CL_SUCCESS, iRet);

  delete[] pDevices;
  delete[] pBinarySizes;
  delete[] pBinaryStatus;

  return bResult;
}
