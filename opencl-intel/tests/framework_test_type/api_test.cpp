//|
//| TEST: API test
//|
//| Purpose
//| -------
//|
//| Test framework API, according to 1.1 spec.
//|
//| Method
//| ------
//|
//| Pass criteria
//| -------------
//|
//| Return true in case of SUCCESS.

#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>

#define WORK_SIZE 1
#define MAX_SOURCE_SIZE 2048

extern cl_device_type gDeviceType;

bool run_kernel(cl_context &context, cl_device_id &device,
                cl_command_queue &cmd_queue, const char *prog, char *out) {
  cl_int err;
  cl_kernel kernel;
  cl_program program;
  bool res;

  program =
      clCreateProgramWithSource(context, 1, (const char **)&prog, NULL, &err);
  res = SilentCheck("clCreateProgramWithSource", CL_SUCCESS, err);
  if (!res)
    return res;

  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  cl_build_status build_status;
  err |= clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_STATUS,
                               MAX_SOURCE_SIZE, &build_status, NULL);
  if (CL_SUCCESS != err || CL_BUILD_ERROR == build_status) {
    printf("\n build status is: %d \n", build_status);
    char err_str[MAX_SOURCE_SIZE]; // instead of dynamic allocation
    char *err_str_ptr = err_str;
    err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                MAX_SOURCE_SIZE, err_str_ptr, NULL);
    if (err != CL_SUCCESS)
      printf("Build Info error: %d \n", err);
    printf("%s \n", err_str_ptr);
    return res;
  }

  kernel = clCreateKernel(program, "apiTest", &err);
  res = SilentCheck("clCreateKernel", CL_SUCCESS, err);
  if (!res) {
    clReleaseProgram(program);
    return res;
  }

  cl_ulong memSize;
  size_t wgSize[3];
  size_t retSize;
  err = clGetKernelWorkGroupInfo(kernel, NULL, CL_KERNEL_WORK_GROUP_SIZE,
                                 sizeof(size_t), &wgSize[0], &retSize);
  res &= Check("clGetKernelWorkGroupInfo", CL_SUCCESS, err);
  err = clGetKernelWorkGroupInfo(kernel, NULL, CL_KERNEL_LOCAL_MEM_SIZE,
                                 sizeof(cl_ulong), &memSize, &retSize);
  res &= Check("clGetKernelWorkGroupInfo", CL_SUCCESS, err);
  err =
      clGetKernelWorkGroupInfo(kernel, NULL, CL_KERNEL_COMPILE_WORK_GROUP_SIZE,
                               sizeof(size_t[3]), wgSize, &retSize);
  res &= Check("clGetKernelWorkGroupInfo", CL_SUCCESS, err);
  err = clGetKernelWorkGroupInfo(kernel, NULL,
                                 CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                                 sizeof(size_t), &wgSize[0], &retSize);
  res &= Check("clGetKernelWorkGroupInfo", CL_SUCCESS, err);
  err = clGetKernelWorkGroupInfo(kernel, NULL, CL_KERNEL_PRIVATE_MEM_SIZE,
                                 sizeof(cl_ulong), &memSize, &retSize);
  res &= Check("clGetKernelWorkGroupInfo", CL_SUCCESS, err);
  wgSize[0] = -1;
  retSize = 0;
  err =
      clGetKernelWorkGroupInfo(kernel, NULL, CL_KERNEL_COMPILE_WORK_GROUP_SIZE,
                               sizeof(size_t), wgSize, &retSize);
  res &= Check("clGetKernelWorkGroupInfo", CL_INVALID_VALUE, err);
  res &= CheckSize("clGetKernelWorkGroupInfo", -1, wgSize[0]);
  res &= CheckSize("clGetKernelWorkGroupInfo", 0, retSize);
  char numArgs = -1;
  retSize = 0;
  err = clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS, sizeof(char), &numArgs,
                        &retSize);
  res &= Check("clGetKernelInfo", CL_INVALID_VALUE, err);
  res &= CheckCondition("clGetKernelInfo", -1 == numArgs);
  res &= CheckSize("clGetKernelInfo", 0, retSize);

  if (!res) {
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    return res;
  }

  // create buffer - should fail
  cl_mem buff = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_READ_WRITE,
                               sizeof(int), NULL, &err);
  res = SilentCheck("clCreateBuffer", CL_INVALID_VALUE, err);
  if (!res) {
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    return res;
  }
  // create buffer - should succeed
  buff = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(char), NULL, &err);
  res = SilentCheck("clCreateBuffer", CL_SUCCESS, err);
  if (!res) {
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    return res;
  }

  // create sub buffer - should fail
  clCreateSubBuffer(0, CL_MEM_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION, NULL,
                    &err);
  res = SilentCheck("clCreateSubBuffer", CL_INVALID_MEM_OBJECT, err);
  if (!res) {
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    return res;
  }

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&buff);
  res = SilentCheck("clSetKernelArg", CL_SUCCESS, err);
  if (!res) {
    clReleaseMemObject(buff);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    return res;
  }

  size_t global_work_size[1];
  global_work_size[0] = WORK_SIZE;

  cl_event ndEvent;

  err = clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL, global_work_size,
                               NULL, 0, NULL, &ndEvent);
  res = SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, err);
  if (!res) {
    clReleaseMemObject(buff);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    return res;
  }

  cl_event ev;
  // enqueue read buffer - should fail
  err = clEnqueueReadBuffer(cmd_queue, buff, CL_TRUE, 0, sizeof(char), out, 1,
                            NULL, &ev);
  res = SilentCheck("SilentCheck", CL_INVALID_EVENT_WAIT_LIST, err);
  // enqueue read buffer - should fail
  err &= clEnqueueReadBuffer(cmd_queue, buff, CL_TRUE, 0, sizeof(char), out, 1,
                             NULL, NULL);
  res &= SilentCheck("clEnqueueReadBuffer", CL_INVALID_EVENT_WAIT_LIST, err);
  // enqueue read buffer - should succeed
  err = clEnqueueReadBuffer(cmd_queue, buff, CL_TRUE, 0, sizeof(char), out, 0,
                            NULL, NULL);
  res &= SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, err);

  err = clEnqueueCopyBuffer(cmd_queue, buff, buff, 0, 0, sizeof(char), 0, NULL,
                            NULL);
  res &= SilentCheck("clEnqueueCopyBuffer", CL_MEM_COPY_OVERLAP, err);

  clFinish(cmd_queue);
  clReleaseEvent(ndEvent);
  clReleaseMemObject(buff);
  SilentCheck("Second clReleaseMemObject(buff)", CL_INVALID_MEM_OBJECT,
              clReleaseMemObject(buff));
  clReleaseKernel(kernel);
  SilentCheck("Second clReleaseKernel()", CL_INVALID_KERNEL,
              clReleaseKernel(kernel));
  clReleaseProgram(program);
  SilentCheck("Second clReleaseProgram()", CL_INVALID_PROGRAM,
              clReleaseProgram(program));
  return res;
}

bool api_test() {
  printf("---------------------------------------\n");
  printf("API test\n");
  printf("---------------------------------------\n");
  bool bResult = true;
  cl_context context = NULL;
  cl_command_queue cmd_queue = NULL;
  cl_device_id device = NULL;
  cl_int err;
  cl_uint num_devices = 1;
  cl_platform_id platform = NULL;
  cl_char user_data[8];

  // init platform
  err = clGetPlatformIDs(1, &platform, NULL);
  bResult = SilentCheck("clGetPlatformIDs", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // init Devices (only one CPU...) should fail
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, &device, &num_devices);
  bResult = SilentCheck("clGetDeviceIDs:1", CL_INVALID_VALUE, err);
  if (!bResult)
    return bResult;

  // init Devices (only one CPU...) should fail
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, NULL);
  bResult = SilentCheck("clGetDeviceIDs:2", CL_INVALID_VALUE, err);
  if (!bResult)
    return bResult;

  // init Devices (only one CPU...) should succeed
  err = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  bResult = SilentCheck("clGetDeviceIDs", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // init Devices (only one CPU...) should succeed
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, NULL, &num_devices);
  bResult = SilentCheck("clGetDeviceIDs", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  void *invFunc = clGetExtensionFunctionAddress(NULL);
  bResult = CheckSize("clGetExtensionFunctionAddress(NULL)", (size_t)NULL,
                      (size_t)invFunc);
  if (!bResult)
    return bResult;

  // clGetDeviceInfo should fail
  size_t actual_size;
  char dummy[4];
  err = clGetDeviceInfo(device, CL_DEVICE_TYPE, 1, dummy, &actual_size);
  bResult = SilentCheck("clGetDeviceInfo", CL_INVALID_VALUE, err);
  if (!bResult)
    return bResult;

  size_t ulRetValCheck = -1;
  actual_size = 0;
  err = clGetDeviceInfo(device, CL_DEVICE_MAX_PARAMETER_SIZE, 1, &ulRetValCheck,
                        &actual_size);
  bResult = SilentCheck("clGetDeviceInfo", CL_INVALID_VALUE, err);
  bResult &=
      SilentCheckSize("clGetDeviceInfo, ulRetValCheck=-1", ulRetValCheck, -1);
  bResult &= SilentCheckSize("clGetDeviceInfo(CL_DEVICE_MAX_PARAMETER_SIZE), "
                             "actual_size=0",
                             0, actual_size);
  if (!bResult)
    return bResult;

  err = clGetDeviceInfo(device, CL_DEVICE_PLATFORM, 1, &ulRetValCheck,
                        &actual_size);
  bResult = SilentCheck("clGetDeviceInfo", CL_INVALID_VALUE, err);
  bResult &=
      SilentCheckSize("clGetDeviceInfo ulRetValCheck=-1", ulRetValCheck, -1);
  bResult &= SilentCheckSize(
      "clGetDeviceInfo(CL_DEVICE_PLATFORM), actual_size=0", 0, actual_size);
  if (!bResult)
    return bResult;

  err = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 1, &ulRetValCheck,
                          &actual_size);
  bResult = SilentCheck("clGetPlatformInfo", CL_INVALID_VALUE, err);
  bResult &=
      SilentCheckSize("clGetPlatformInfo, ulRetValCheck=-1", ulRetValCheck, -1);
  bResult &=
      SilentCheckSize("clGetPlatformInfo, actual_size=0", 0, actual_size);
  if (!bResult)
    return bResult;

  cl_context_properties inv_prop2[3];
  inv_prop2[0] = -1; // invalid property name
  inv_prop2[1] = (cl_context_properties)platform;
  inv_prop2[2] = 0;
  context = clCreateContextFromType(inv_prop2, gDeviceType, NULL, NULL, &err);
  bResult = SilentCheck("clCreateContextFromType", CL_INVALID_PROPERTY, err);
  if (!bResult)
    return bResult;

  // init context should fail
  context = clCreateContext(NULL, 1, &device, NULL, user_data, &err);
  bResult = SilentCheck("clCreateContext", CL_INVALID_VALUE, err);
  if (!bResult)
    return bResult;

  // init context should fail
  context = clCreateContext(NULL, 1, NULL, NULL, NULL, &err);
  bResult = SilentCheck("clCreateContext", CL_INVALID_VALUE, err);
  if (!bResult)
    return bResult;

  // init context should fail
  context =
      clCreateContextFromType(NULL, CL_DEVICE_TYPE_ALL, NULL, user_data, &err);
  bResult = SilentCheck("clCreateContext", CL_INVALID_VALUE, err);
  if (!bResult)
    return bResult;

  // init context should succeed
  context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  bResult = SilentCheck("clCreateContext", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // Check user event query
  cl_event user_ev;
  user_ev = clCreateUserEvent(context, &err);
  bResult = SilentCheck("clCreateUserEvent", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  cl_context evntCntx;
  size_t envtCntxSize;
  err = clGetEventInfo(user_ev, CL_EVENT_CONTEXT, sizeof(evntCntx), &evntCntx,
                       &envtCntxSize);
  clReleaseEvent(user_ev);

  // Check info query
  bResult = Check("clGetEventInfo", CL_SUCCESS, err);
  if (!bResult)
    return bResult;
  bResult = CheckHandle("clGetEventInfo", context, evntCntx);
  if (!bResult)
    return bResult;
  bResult = CheckSize("clGetEventInfo", sizeof(evntCntx), envtCntxSize);
  if (!bResult)
    return bResult;

  err = clWaitForEvents(1, NULL);
  bResult = Check("clWaitForEvents", CL_INVALID_VALUE, err);
  if (!bResult) {
    return bResult;
  }

  // init Command Queue - should fail
  const cl_queue_properties props[] = {
      CL_QUEUE_PROPERTIES,
      (int)~(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
             CL_QUEUE_PROFILING_ENABLE |
             CL_QUEUE_THREAD_LOCAL_EXEC_ENABLE_INTEL | CL_QUEUE_ON_DEVICE |
             CL_QUEUE_ON_DEVICE_DEFAULT),
      0};
  cmd_queue = clCreateCommandQueueWithProperties(context, device, props, &err);
  bResult =
      SilentCheck("clCreateCommandQueueWithProperties", CL_INVALID_VALUE, err);
  if (!bResult) {
    clReleaseContext(context);
    return bResult;
  }

  // init Command Queue - should succeed
  cmd_queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
  bResult = SilentCheck("clCreateCommandQueueWithProperties", CL_SUCCESS, err);
  if (!bResult) {
    clReleaseContext(context);
    return bResult;
  }

  const char *ocl_test_program = {"__kernel void apiTest(__global char  *fred)\
    {fred[0] = 0;}"};

  char p;

  bResult = run_kernel(context, device, cmd_queue, ocl_test_program, &p);
  printf("\n---------------------------------------\n");
  printf("API test succeeded!\n");
  printf("---------------------------------------\n");

  clFinish(cmd_queue);
  fflush(stdout);
  clReleaseCommandQueue(cmd_queue);
  SilentCheck("Second clReleaseCommandQueue()", CL_INVALID_COMMAND_QUEUE,
              clReleaseCommandQueue(cmd_queue));
  clReleaseContext(context);
  err = clReleaseContext(context);
  bResult = SilentCheck("Second clReleaseContext()", CL_INVALID_CONTEXT, err);

  return bResult;
}
