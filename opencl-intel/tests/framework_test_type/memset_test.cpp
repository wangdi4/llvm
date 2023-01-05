#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>
#include <time.h>

extern cl_device_type gDeviceType;

const char *kernelSource1 =
    "__kernel void reuse_mem(__global uchar* buf1, __global uchar* buf2) { "
    "buf1[get_global_id(0)] = buf2[get_global_id(0)]; }";
const char *kernelSource2 = "__kernel void memset(__global uchar* buf1, uchar "
                            "val) { buf1[get_global_id(0)] = val; }";
// const char* kernelSource2 = "__kernel void memset(__global uchar* buf1, uchar
// val) { buf1[0] = val;}";

bool reuse_mem_test() {
  cl_device_type deviceType = gDeviceType;
  cl_device_id deviceId;
  cl_device_id *devices = NULL;
  cl_uint numDevices = 0;
  cl_context context;
  cl_command_queue commandQueue;
  cl_mem buf;
  cl_program program;
  cl_kernel kernel;
  cl_int err = CL_SUCCESS;

  /* Get the number of requested devices */
  err |= clGetDeviceIDs(NULL, deviceType, 0, NULL, &numDevices);
  devices = (cl_device_id *)malloc(numDevices * sizeof(cl_device_id));
  err |= clGetDeviceIDs(NULL, deviceType, numDevices, devices, NULL);
  deviceId = devices[0];
  free(devices);
  devices = NULL;

  if (err != CL_SUCCESS) {
    return false;
  }

  context = clCreateContext(NULL, 1, &deviceId, NULL, NULL, &err);
  if (err != CL_SUCCESS) {
    return false;
  }

  commandQueue =
      clCreateCommandQueueWithProperties(context, deviceId, NULL, &err);
  if (err != CL_SUCCESS) {
    return false;
  }

  program = clCreateProgramWithSource(context, 1, &kernelSource1, NULL, &err);
  err |= clBuildProgram(program, 1, &deviceId, NULL, NULL, NULL);
  if (err != CL_SUCCESS) {
    return false;
  }

  const size_t globalSize = 1;
  buf = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR, globalSize, NULL, &err);
  if (err != CL_SUCCESS) {
    return false;
  }

  kernel = clCreateKernel(program, "reuse_mem", &err);
  err |= clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &buf);
  if (err != CL_SUCCESS) {
    return false;
  }

  err |= clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &globalSize,
                                NULL, 0, NULL, NULL);
  err |= clFinish(commandQueue);
  if (err != CL_SUCCESS) {
    return false;
  }

  err |= clReleaseMemObject(buf);
  err |= clReleaseKernel(kernel);
  err |= clReleaseProgram(program);
  err |= clReleaseCommandQueue(commandQueue);
  err |= clReleaseContext(context);
  if (err != CL_SUCCESS) {
    return false;
  }

  return true;
}

bool memset_test() {
#define MEMSET_SIZE 1

  cl_device_type deviceType = gDeviceType;
  cl_device_id deviceId;
  cl_device_id *devices = NULL;
  cl_uint numDevices = 0;
  cl_context context;
  cl_command_queue commandQueue;
  cl_mem buf;
  cl_program program;
  cl_kernel kernel;
  cl_int err = CL_SUCCESS;
  cl_uchar arr[MEMSET_SIZE];
  cl_bool validBuffer = CL_TRUE;

  const cl_uchar success_val = 0xE4;
  const cl_uchar failure_val = 0xC1;

  cl_platform_id platform = 0;
  bool bResult = true;

  err = clGetPlatformIDs(1, &platform, NULL);
  bResult &= Check("clGetPlatformIDs", CL_SUCCESS, err);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  /* Get the number of requested devices */
  err |= clGetDeviceIDs(platform, deviceType, 0, NULL, &numDevices);
  devices = (cl_device_id *)malloc(numDevices * sizeof(cl_device_id));
  err |= clGetDeviceIDs(platform, deviceType, numDevices, devices, NULL);
  deviceId = devices[0];
  free(devices);
  devices = NULL;

  if (err != CL_SUCCESS) {
    return false;
  }

  context = clCreateContext(prop, 1, &deviceId, NULL, NULL, &err);
  if (err != CL_SUCCESS) {
    return false;
  }

  commandQueue =
      clCreateCommandQueueWithProperties(context, deviceId, NULL, &err);
  if (err != CL_SUCCESS) {
    return false;
  }

  program = clCreateProgramWithSource(context, 1, &kernelSource2, NULL, &err);
  err |= clBuildProgram(program, 1, &deviceId, NULL, NULL, NULL);
  if (err != CL_SUCCESS) {
    return false;
  }

  const size_t globalSize = MEMSET_SIZE;

  // smear input with failure val
  memset(arr, failure_val, MEMSET_SIZE);
  buf = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR, globalSize, arr, &err);
  if (err != CL_SUCCESS) {
    return false;
  }

  kernel = clCreateKernel(program, "memset", &err);
  err |= clSetKernelArg(kernel, 0, sizeof(cl_mem), &buf);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_uchar), &success_val);
  if (err != CL_SUCCESS) {
    return false;
  }

  err |= clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &globalSize,
                                NULL, 0, NULL, NULL);
  err |= clFinish(commandQueue);
  err |= clEnqueueReadBuffer(commandQueue, buf, CL_TRUE, 0, MEMSET_SIZE, arr, 0,
                             NULL, NULL);
  if (err != CL_SUCCESS) {
    return false;
  }

  for (int i = 0; i < MEMSET_SIZE; ++i) {
    validBuffer &= (success_val == arr[i]);
  }

  err |= clReleaseMemObject(buf);
  err |= clReleaseKernel(kernel);
  err |= clReleaseProgram(program);
  err |= clReleaseCommandQueue(commandQueue);
  err |= clReleaseContext(context);
  if (err != CL_SUCCESS) {
    return false;
  }

  return validBuffer;
}
