#include "CL/cl.h"
#include "FrameworkTest.h"

extern cl_device_type gDeviceType;

static const size_t IMMEDIATE_EXECUTION_GLOBAL_SIZE = 4;
static const size_t IMMEDIATE_EXECUTION_LOCAL_SIZE = 4;

bool immediateExecutionTest() {
  printf("---------------------------------------\n");
  printf("immediateExecutionTest\n");
  printf("---------------------------------------\n");
  const char *ocl_test_program[] = {
      "__kernel void copy (__global float* a, __global float* b)"
      "{"
      "int tid = get_global_id(0);"
      "b[tid] = a[tid];"
      "}"};

  bool bResult = true;
  cl_device_id device_id;
  cl_context context;

  cl_platform_id platform = 0;

  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  // get the first CPU device
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device_id, NULL);
  bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Ensure it supports immediate execution
  size_t stExtSize = 0;
  iRet = clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, 0, NULL, &stExtSize);
  bResult &= SilentCheck("clGetDeviceInfo", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Allocate on stack
  char *extensions = (char *)alloca(stExtSize);
  iRet = clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, stExtSize, extensions,
                         NULL);
  bResult &= SilentCheck("clGetDeviceInfo", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  if (!strstr(extensions, "cl_intel_exec_by_local_thread")) {
    printf("Device doesn't report supporting immediate execution, test cannot "
           "continue. Passing vacuously\n");
    return true;
  }

  // create context
  context = clCreateContext(prop, 1, &device_id, NULL, NULL, &iRet);
  bResult &= SilentCheck("clCreateContext", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // create program with source
  cl_program program = clCreateProgramWithSource(
      context, 1, (const char **)&ocl_test_program, NULL, &iRet);
  bResult &= SilentCheck("clCreateProgramWithSource", CL_SUCCESS, iRet);

  iRet = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  bResult &= SilentCheck("clBuildProgram", CL_SUCCESS, iRet);

  //
  // From here down it is the program execution implementation
  //
  cl_float src[IMMEDIATE_EXECUTION_GLOBAL_SIZE];
  cl_float dst[IMMEDIATE_EXECUTION_GLOBAL_SIZE];

  cl_float init = 0.1f;
  for (unsigned int j = 0; j < IMMEDIATE_EXECUTION_GLOBAL_SIZE; j++) {
    src[j] = init;
    dst[j] = 0.0f;
    init += 0.1f;
  }

  //
  // Create an in-order immediate queue
  //
  const cl_queue_properties props[] = {
      CL_QUEUE_PROPERTIES, CL_QUEUE_THREAD_LOCAL_EXEC_ENABLE_INTEL, 0};
  cl_command_queue queue1 =
      clCreateCommandQueueWithProperties(context, device_id, props, &iRet);
  bResult &=
      SilentCheck("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);

  //
  // Create Kernel
  //
  cl_kernel kernel1 = clCreateKernel(program, "copy", &iRet);
  bResult &= SilentCheck("clCreateKernel - copy", CL_SUCCESS, iRet);

  //
  // Create buffers
  //
  size_t size = sizeof(cl_float);

  cl_mem buffer_src =
      clCreateBuffer(context, CL_MEM_READ_ONLY,
                     size * IMMEDIATE_EXECUTION_GLOBAL_SIZE, NULL, &iRet);
  bResult &= SilentCheck("clCreateBuffer - src", CL_SUCCESS, iRet);

  cl_mem buffer_dst =
      clCreateBuffer(context, CL_MEM_READ_WRITE,
                     size * IMMEDIATE_EXECUTION_GLOBAL_SIZE, NULL, &iRet);
  bResult &= SilentCheck("clCreateBuffer - dst", CL_SUCCESS, iRet);

  //
  // Set arguments
  //
  iRet = clSetKernelArg(kernel1, 0, sizeof(cl_mem), &buffer_src);
  bResult &= SilentCheck("clSetKernelArg - buffer_src", CL_SUCCESS, iRet);

  iRet = clSetKernelArg(kernel1, 1, sizeof(cl_mem), &buffer_dst);
  bResult &= SilentCheck("clSetKernelArg - buffer_dst", CL_SUCCESS, iRet);

  //
  // Execute commands - Write buffers
  //
  cl_event writeBufferEvent, markerEvent1, markerEvent2;
  iRet = clEnqueueWriteBuffer(queue1, buffer_src, false, 0,
                              size * IMMEDIATE_EXECUTION_GLOBAL_SIZE, src, 0,
                              NULL, &writeBufferEvent);
  bResult &= SilentCheck("clEnqueueWriteBuffer - src", CL_SUCCESS, iRet);

  iRet =
      clEnqueueMarkerWithWaitList(queue1, 1, &writeBufferEvent, &markerEvent1);
  bResult &= SilentCheck("clEnqueueMarkerWithWaitList - with wait list",
                         CL_SUCCESS, iRet);

  iRet = clEnqueueWriteBuffer(queue1, buffer_dst, false, 0,
                              size * IMMEDIATE_EXECUTION_GLOBAL_SIZE, dst, 1,
                              &markerEvent1, NULL);
  bResult &= SilentCheck("clEnqueueWriteBuffer - dst", CL_SUCCESS, iRet);

  iRet = clEnqueueMarkerWithWaitList(queue1, 0, NULL, &markerEvent2);
  bResult &= SilentCheck("clEnqueueMarkerWithWaitList - wait for all",
                         CL_SUCCESS, iRet);

  clReleaseEvent(writeBufferEvent);
  clReleaseEvent(markerEvent1);
  clReleaseEvent(markerEvent2);

  //
  // Execute kernel
  //
  size_t global_work_size[1] = {IMMEDIATE_EXECUTION_GLOBAL_SIZE};
  size_t local_work_size[1] = {IMMEDIATE_EXECUTION_LOCAL_SIZE};

  cl_event evt;
  iRet = clEnqueueNDRangeKernel(queue1, kernel1, 1, NULL, global_work_size,
                                local_work_size, 0, NULL, &evt);
  bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

  iRet = clEnqueueBarrierWithWaitList(queue1, 1, &evt, NULL);
  bResult &= SilentCheck("clEnqueueBarrierWithWaitList", CL_SUCCESS, iRet);

  cl_uint command_status;
  iRet = clGetEventInfo(evt, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_uint),
                        &command_status, NULL);
  bResult &= SilentCheck("clGetEventInfo", CL_SUCCESS, iRet);
  if (CL_COMPLETE != command_status) {
    if (command_status > 0) {
      printf("Command still in flight, queue not synchronous. Failing test\n");
    } else {
      printf("NDRange failed. Failing test\n");
    }
    iRet = clReleaseEvent(evt);
    iRet = clReleaseMemObject(buffer_dst);
    iRet = clReleaseMemObject(buffer_src);
    iRet = clReleaseKernel(kernel1);
    iRet = clReleaseProgram(program);
    iRet = clReleaseCommandQueue(queue1);
    iRet = clReleaseContext(context);

    return false;
  }

  iRet = clEnqueueReadBuffer(queue1, buffer_dst, CL_TRUE, 0,
                             size * IMMEDIATE_EXECUTION_GLOBAL_SIZE, dst, 0,
                             NULL, NULL);
  bResult &= SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, iRet);

  for (unsigned int i = 0; i < IMMEDIATE_EXECUTION_GLOBAL_SIZE; ++i) {
    if (dst[i] != src[i]) {
      printf("Validation failed for index %u\n", i);
      bResult = false;
      break;
    }
  }
  //
  // Release objects
  //
  iRet = clReleaseEvent(evt);
  bResult &= SilentCheck("clReleaseEvent", CL_SUCCESS, iRet);

  iRet = clReleaseMemObject(buffer_dst);
  bResult &= SilentCheck("clReleaseBuffer - buffer_dst", CL_SUCCESS, iRet);

  iRet = clReleaseMemObject(buffer_src);
  bResult &= SilentCheck("clReleaseBuffer - buffer_src", CL_SUCCESS, iRet);

  iRet = clReleaseKernel(kernel1);
  bResult &= SilentCheck("clReleaseKernel - kernel1", CL_SUCCESS, iRet);

  iRet = clReleaseProgram(program);
  bResult &= SilentCheck("clReleaseProgram - program", CL_SUCCESS, iRet);

  iRet = clReleaseCommandQueue(queue1);
  bResult &= SilentCheck("clReleaseCommandQueue - queue1", CL_SUCCESS, iRet);

  iRet = clReleaseContext(context);
  bResult &= SilentCheck("clReleaseContext - context", CL_SUCCESS, iRet);

  return bResult;
}
