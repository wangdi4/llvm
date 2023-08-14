#include "CL/cl.h"
#include "FrameworkTest.h"
#include <stdio.h>

#define NUM_ITERATIONS 5

extern cl_device_type gDeviceType;

/******************************************************************************
 * Multiple Execution test: test running several ND-Ranges in the same process,
 * with the host thread participating Intended
 *
 * This test will deadlock if for some reason we don't get enough worker threads
 ** Not composable with anything else *
 ******************************************************************************/

bool clMultipleExecutionTest() {
  printf("---------------------------------------\n");
  printf("clMultipleExecutionTest\n");
  printf("---------------------------------------\n");
  const char *ocl_test_program[] = {
      "__kernel void testKernel (__global volatile int *b)"
      "{"
      "int bar = get_global_size(0);"
      "atomic_inc(b);"
      "while (*b < bar) {};"
      "}"};
  bool bResult = true;
  cl_device_id device_id;
  cl_context context;
  cl_uint num_compute_units;

  cl_platform_id platform = 0;

  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  // get device(s)
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device_id, NULL);
  bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  iRet = clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS,
                         sizeof(cl_uint), &num_compute_units, NULL);
  bResult = SilentCheck("clGetDeviceInfo(CL_DEVICE_MAX_COMPUTE_UNITS)",
                        CL_SUCCESS, iRet);
  if (!bResult)
    return bResult;

  size_t globalSize = (size_t)num_compute_units;
  // We must use local size of 1 to ensure the number of work groups equals the
  // number of compute units
  size_t localSize = 1;

  for (unsigned int iteration = 0; iteration < NUM_ITERATIONS; ++iteration) {
    printf("Iteration %u started\n", iteration);

    context = clCreateContext(prop, 1, &device_id, NULL, NULL, &iRet);
    bResult &= SilentCheck("clCreateContext", CL_SUCCESS, iRet);
    if (!bResult)
      return bResult;

    cl_command_queue queue =
        clCreateCommandQueueWithProperties(context, device_id, NULL, &iRet);
    bResult &= SilentCheck("clCreateCommandQueueWithProperties - queue1",
                           CL_SUCCESS, iRet);
    if (!bResult) {
      clReleaseContext(context);
      return bResult;
    }

    // create program with source
    cl_program program = clCreateProgramWithSource(
        context, 1, (const char **)&ocl_test_program, NULL, &iRet);
    bResult &= SilentCheck("clCreateProgramWithSource", CL_SUCCESS, iRet);
    if (!bResult) {
      clReleaseCommandQueue(queue);
      clReleaseContext(context);
      return bResult;
    }

    iRet = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    bResult &= SilentCheck("clBuildProgram", CL_SUCCESS, iRet);
    if (!bResult) {
      clReleaseProgram(program);
      clReleaseCommandQueue(queue);
      clReleaseContext(context);
      return bResult;
    }

    cl_kernel kernel = clCreateKernel(program, "testKernel", &iRet);
    bResult &= SilentCheck("clCreateKernel", CL_SUCCESS, iRet);
    if (!bResult) {
      clReleaseProgram(program);
      clReleaseCommandQueue(queue);
      clReleaseContext(context);
      return bResult;
    }

    int barrier = 0;

    cl_mem buffer =
        clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                       sizeof(int), &barrier, &iRet);
    bResult &= SilentCheck("clCreateBuffer - dst", CL_SUCCESS, iRet);
    if (!bResult) {
      clReleaseKernel(kernel);
      clReleaseProgram(program);
      clReleaseCommandQueue(queue);
      clReleaseContext(context);
      return bResult;
    }

    iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
    bResult &= SilentCheck("clSetKernelArg", CL_SUCCESS, iRet);
    if (!bResult) {
      clReleaseMemObject(buffer);
      clReleaseKernel(kernel);
      clReleaseProgram(program);
      clReleaseCommandQueue(queue);
      clReleaseContext(context);
      return bResult;
    }

    iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize,
                                  &localSize, 0, NULL, NULL);
    bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
    if (!bResult) {
      clReleaseMemObject(buffer);
      clReleaseKernel(kernel);
      clReleaseProgram(program);
      clReleaseCommandQueue(queue);
      clReleaseContext(context);
      return bResult;
    }

    iRet = clFinish(queue);
    bResult &= SilentCheck("clFinish", CL_SUCCESS, iRet);

    clReleaseMemObject(buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    if (!bResult)
      return bResult;

    printf("Iteration %u complete\n", iteration);
  }

  return true;
}
