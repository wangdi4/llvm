#include "CL/cl.h"
#include "FrameworkTest.h"

#define NUM_VECS 1000
#define NUM_LOOPS 100000
#define POINTS_INTERVAL (NUM_LOOPS / 100)

extern cl_device_type gDeviceType;

bool MisalignedUseHostPtrTest() {
  printf("MisalignedUseHostPtr Test\n");
  const char *ocl_test_program[] = {"__kernel void k (__global float16 *a)"
                                    "{"
                                    "a[get_global_id(0)] = (float16)(0.0);"
                                    "}"};

  bool bResult = true;

  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  cl_context context;

  cl_platform_id platform = 0;

  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  cl_kernel kernel1;
  cl_float *hostPtr;
  cl_program program;
  cl_command_queue queue1;
  void *ptr = nullptr;
  unsigned int points_interval = 0;

  // get device(s)
  iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
  bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];

  iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, pDevices, NULL);
  bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult)
    goto end;

  // create context
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  bResult &= SilentCheck("clCreateContext", CL_SUCCESS, iRet);
  if (!bResult)
    goto end;

  queue1 = clCreateCommandQueueWithProperties(context, pDevices[0],
                                              NULL /*no properties*/, &iRet);
  bResult &= SilentCheck("clCreateCommandQueueWithProperties - queue1",
                         CL_SUCCESS, iRet);
  if (!bResult)
    goto release_context;

  // create program with source
  program = clCreateProgramWithSource(
      context, 1, (const char **)&ocl_test_program, NULL, &iRet);
  bResult &= SilentCheck("clCreateProgramWithSource", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_queue;

  iRet = clBuildProgram(program, uiNumDevices, pDevices, NULL, NULL, NULL);
  bResult &= SilentCheck("clBuildProgram", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_program;

  // Allocate enough memory
  ptr = malloc(sizeof(cl_float16) * NUM_VECS + 1);
  // arrange for a misaligned pointer
  hostPtr = (cl_float *)((((size_t)ptr) | 1));

  cl_float *mapped_ptr;

  kernel1 = clCreateKernel(program, "k", &iRet);
  bResult &= SilentCheck("clCreateKernel", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_program;

  points_interval = 0;

  cl_mem buf;
  for (size_t i = 0; i < NUM_LOOPS; ++i) {
    buf = clCreateBuffer(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                         sizeof(cl_float16) * NUM_VECS, hostPtr, &iRet);
    bResult &= SilentCheck("clCreateBuffer", CL_SUCCESS, iRet);
    if (!bResult)
      goto release_kernel;

    iRet = clSetKernelArg(kernel1, 0, sizeof(cl_mem), &buf);
    bResult &= SilentCheck("clSetKernelArg", CL_SUCCESS, iRet);
    if (!bResult)
      goto release_buf;

    size_t global_work_size[1] = {NUM_VECS};
    iRet = clEnqueueNDRangeKernel(queue1, kernel1, 1, NULL, global_work_size,
                                  NULL, 0, NULL, NULL);
    bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
    if (!bResult)
      goto release_buf;

    mapped_ptr = (cl_float *)clEnqueueMapBuffer(
        queue1, buf, CL_TRUE, CL_MAP_WRITE, 0, sizeof(cl_float16) * NUM_VECS, 0,
        NULL, NULL, &iRet);
    bResult &= SilentCheck("clEnqueueMapBuffer", CL_SUCCESS, iRet);
    if (!bResult)
      goto release_buf;

    iRet = clEnqueueUnmapMemObject(queue1, buf, mapped_ptr, 0, NULL, NULL);
    bResult &= SilentCheck("clEnqueueUnmapMemObject", CL_SUCCESS, iRet);
    if (!bResult)
      goto release_buf;

    clReleaseMemObject(buf);

    if (0 == points_interval) {
      printf(".");
      fflush(0);
      points_interval = POINTS_INTERVAL;
    };
    --points_interval;
  }
  // Ensure all commands are done executing
  printf("\nWaiting to clFinish....");
  fflush(0);
  iRet = clFinish(queue1);
  printf("\n");
  fflush(0);
  bResult &= SilentCheck("clFinish", CL_SUCCESS, iRet);
  goto release_kernel;

release_buf:
  clReleaseMemObject(buf);
release_kernel:
  clReleaseKernel(kernel1);
release_program:
  clReleaseProgram(program);
release_queue:
  clReleaseCommandQueue(queue1);
release_context:
  clReleaseContext(context);
end:
  delete[] pDevices;
  free(ptr);
  return bResult;
}
