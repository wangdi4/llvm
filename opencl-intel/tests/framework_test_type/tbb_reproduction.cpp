#include "FrameworkTest.h"

extern cl_device_type gDeviceType;

bool TBBTest() {
  const char *ocl_test_program[] = {
      "__kernel void dot_product (__global const float4 *a, __global const "
      "float4 *b, __global float *c)"
      "{;\n"
      "int tid = get_global_id(0);"
      "for (uint i = 0; i < 100; ++i)"
      "c[tid] += dot(a[tid], b[tid]);"
      "}"};

  bool bResult = true;

  printf("Trying to crash TBB\n");
  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  cl_context context;
  cl_program clProg;

  // Setup Part
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
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return false;
  }

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];

  iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, pDevices, NULL);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    delete[] pDevices;
    return false;
  }

  // create context
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  if (CL_SUCCESS != iRet) {
    printf("clCreateContext = %s\n", ClErrTxt(iRet));
    delete[] pDevices;
    return false;
  }
  printf("context = %p\n", (void *)context);

  printf("Building program %s\n", ocl_test_program[0]);

  clProg = clCreateProgramWithSource(
      context, 1, (const char **)(ocl_test_program), NULL, &iRet);
  bResult &= SilentCheck("clCreateProgramWithSource", CL_SUCCESS, iRet);

  iRet |= clBuildProgram(clProg, 0, NULL, NULL, NULL, NULL);
  bResult &= SilentCheck("clBuildProgram", CL_SUCCESS, iRet);

  cl_kernel dummyKernel = clCreateKernel(clProg, "dot_product", &iRet);
  bResult &= SilentCheck("clCreateKernel", CL_SUCCESS, iRet);

  // Execution Part

  const size_t execution_size = 10000;
  const size_t execution_size2 = 1;

  cl_mem dummyBuffer;
  dummyBuffer =
      clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE,
                     execution_size * sizeof(cl_float4), NULL, &iRet);
  bResult &= SilentCheck("clCreateBuffer", CL_SUCCESS, iRet);

  clSetKernelArg(dummyKernel, 0, sizeof(cl_mem), &dummyBuffer);
  clSetKernelArg(dummyKernel, 1, sizeof(cl_mem), &dummyBuffer);
  clSetKernelArg(dummyKernel, 2, sizeof(cl_mem), &dummyBuffer);

  // Now to the actual test
  const size_t iterations = 100;
  for (size_t i = 0; i < iterations; ++i) {
    cl_command_queue queues[2];

    queues[0] =
        clCreateCommandQueueWithProperties(context, pDevices[0], NULL, &iRet);
    bResult &=
        SilentCheck("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);
    queues[1] =
        clCreateCommandQueueWithProperties(context, pDevices[0], NULL, &iRet);
    bResult &=
        SilentCheck("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);

    cl_event kernelEvent;
    iRet |=
        clEnqueueNDRangeKernel(queues[0], dummyKernel, 1, NULL,
                               &execution_size2, NULL, 0, NULL, &kernelEvent);
    iRet |=
        clEnqueueNDRangeKernel(queues[1], dummyKernel, 1, NULL, &execution_size,
                               NULL, 1, &kernelEvent, NULL);
    bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

    clFlush(queues[0]);
    clFinish(queues[1]);

    clReleaseEvent(kernelEvent);
    clReleaseCommandQueue(queues[1]);
    clReleaseCommandQueue(queues[0]);
  }
  // No crash? test passed
  printf("Test passed!\n");
  delete[] pDevices;

  // Release objects
  clReleaseKernel(dummyKernel);
  clReleaseMemObject(dummyBuffer);
  clReleaseProgram(clProg);
  clReleaseContext(context);
  return bResult;
}
