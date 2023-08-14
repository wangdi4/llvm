#include "CL/cl.h"
#include "FrameworkTest.h"
#include <memory>

extern cl_device_type gDeviceType;

// Test what clk_event_t instance is correctly passed to device side kernel
// through its arguments

bool ClkEventAsKernelArg() {
  bool bResult = true;
  cl_context context;
  cl_command_queue cmd_queue;
  cl_device_id device;
  cl_platform_id platform;
  cl_int err;

  // init platform
  err = clGetPlatformIDs(1, &platform, NULL);
  bResult = SilentCheck("clGetPlatformIDs", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // init device
  err = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  bResult = SilentCheck("clGetDeviceIDs", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  bResult = SilentCheck("clCreateContext", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // Create host command queue
  cmd_queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);
  bResult = SilentCheck("clCreateCommandQueueWithProperties on host",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;
  // Create device command queue
  cl_queue_properties deviceQueueProps[] = {
      CL_QUEUE_PROPERTIES,
      CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_ON_DEVICE |
          CL_QUEUE_ON_DEVICE_DEFAULT,
      0};
  clCreateCommandQueueWithProperties(context, device, deviceQueueProps, &err);
  bResult = SilentCheck("clCreateCommandQueueWithProperties on device",
                        CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // host_kernel enqueues device_kernel and passes to it a user event
  // device kernel stores result of is_valid_event to output buffer
  const char *ocl_test_program =
      "__kernel void device_kernel(__global int * inout, clk_event_t event) {"
      "  *inout = is_valid_event(event);"
      "  release_event(event);"
      "}"
      ""
      "__kernel void host_kernel(__global int * inout) {"
      "  clk_event_t event = create_user_event();"
      "  enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_NO_WAIT,"
      "                 ndrange_1D(1),"
      "                 ^{ device_kernel(inout, event); });"
      "}";

  // Create a buffer
  cl_mem inout =
      clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_int), NULL, &err);
  bResult = SilentCheck("clCreateBuffer", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // Build the kernels
  cl_program program;
  program =
      clCreateProgramWithSource(context, 1, &ocl_test_program, NULL, &err);
  bResult = SilentCheck("clCreateProgramWithSource", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  err = clBuildProgram(program, 1, &device, "-cl-std=CL2.0", NULL, NULL);
  bResult = SilentCheck("clBuildProgram", CL_SUCCESS, err);

  if (!bResult) {
    size_t log_buffer_len;
    err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL,
                                &log_buffer_len);
    int bScopedRes = SilentCheck("clGetProgramBuildInfo", CL_SUCCESS, err);
    if (!bScopedRes)
      return bScopedRes;
    std::unique_ptr<char> log_buffer(new char[log_buffer_len]);
    err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                log_buffer_len, log_buffer.get(), NULL);
    bScopedRes = SilentCheck("clGetProgramBuildInfo", CL_SUCCESS, err);
    if (!bScopedRes)
      return bScopedRes;
    std::cout << "* Build failure. Log:         *\n" << std::endl;
    std::cout << log_buffer.get() << std::endl;
  }

  if (!bResult)
    return bResult;

  cl_kernel kernel;
  kernel = clCreateKernel(program, "host_kernel", &err);
  bResult = SilentCheck("clCreateKernel: kernel", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  err = clSetKernelArg(kernel, 0, sizeof(inout), &inout);
  bResult = SilentCheck("clSetKernelArg: kernel", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // Set 0 as input value to host kernel
  cl_int value = 0;
  err = clEnqueueWriteBuffer(cmd_queue, inout, CL_FALSE, 0, sizeof(cl_int),
                             &value, 0, NULL, NULL);
  bResult = SilentCheck("clEnqueueWriteBuffer", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // Enqueue host kernel
  size_t global_size = 1;
  err = clEnqueueNDRangeKernel(cmd_queue, kernel, 1, NULL, &global_size, NULL,
                               0, NULL, NULL);
  clFinish(cmd_queue);

  // Read result of is_valid_event called by device_kernel
  err = clEnqueueReadBuffer(cmd_queue, inout, CL_TRUE, 0, sizeof(cl_int),
                            &value, 0, NULL, NULL);
  bResult = SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, err);
  if (!bResult)
    return bResult;

  // Result must be set to true
  if (!value) {
    std::cout << "===============================\n"
              << "* Incorrect device side event *\n"
              << "===============================" << std::endl;
    bResult = false;
  }

  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseMemObject(inout);
  clReleaseCommandQueue(cmd_queue);
  clReleaseContext(context);
  return bResult;
}
