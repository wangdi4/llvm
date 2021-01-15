#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <gtest/gtest.h>
#include <stdio.h>

#include <string>

extern cl_device_type gDeviceType;

const char *source = "\
  __global ulong value[20] = {0};\n\
  void block_fn(size_t tid, __global int* res)\n\
  {\n\
      res[tid] = -2;\n\
  }\n\
  void check_res(size_t tid, const clk_event_t evt, __global int* res)\n\
  {\n\
      capture_event_profiling_info (evt, CLK_PROFILING_COMMAND_EXEC_TIME, &value[tid*2]);\n\
      if (value[tid*2] > 0 && value[tid*2+1] > 0) res[tid] =  0;\n\
      else                                        res[tid] = -4;\n\
      release_event(evt);\n\
  }\n\
  kernel void enqueue_block_profiling(__global int* res)\n\
  {\n\
      size_t tid = get_global_id(0);\n\
      res[tid] = -1;\n\
      queue_t def_q = get_default_queue();\n\
      ndrange_t ndrange = ndrange_1D(1);\n\
      clk_event_t block_evt1;\n\
      void (^kernelBlock)(void)  = ^{ block_fn (tid, res); };\n\
      int enq_res = enqueue_kernel(def_q, CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange, 0, NULL, &block_evt1, kernelBlock);\n\
      if(enq_res != CLK_SUCCESS) { res[tid] = -1; return; }\n\
      void (^checkBlock) (void)  = ^{ check_res(tid, block_evt1, res); };\n\
      enq_res = enqueue_kernel(def_q, CLK_ENQUEUE_FLAGS_NO_WAIT, ndrange, 1, &block_evt1, NULL, checkBlock);\n\
      if(enq_res != CLK_SUCCESS) { res[tid] = -3; return; }\n\
  }\n\
  ";

void enqueueBlockProfilingTest() {
  cl_int err = CL_SUCCESS;
  cl_int res[10] = {0};
  cl_uint maxQueueSize = 0;
  cl_platform_id platform = nullptr;
  cl_device_id device = nullptr;
  cl_command_queue queue;
  size_t global = 10;
  size_t local = 1;
  cl_event event;

  err = clGetPlatformIDs(1, &platform, nullptr);
  ASSERT_EQ(CL_SUCCESS, err) << " clGetPlatformIDs failed.";

  err = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
  ASSERT_EQ(CL_SUCCESS, err) << " clGetDeviceIDs failed on trying to obtain "
                             << gDeviceType << " device type.";

  const cl_context_properties prop[] = {CL_CONTEXT_PLATFORM,
                                        (cl_context_properties)platform, 0};
  cl_context context =
      clCreateContext(prop, 1, &device, nullptr, nullptr, &err);
  ASSERT_EQ(CL_SUCCESS, err) << " clCreateContext failed.";

  cl_command_queue_properties queueCreateProps[] = {0, 0, 0};

  queue = clCreateCommandQueueWithProperties(context, device,
                                             &queueCreateProps[0], &err);

  err = clGetDeviceInfo(device, CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE,
                        sizeof(maxQueueSize), &maxQueueSize, 0);
  ASSERT_EQ(CL_SUCCESS, err) << " failed to get maxQueueSize.";

  cl_command_queue_properties queue_prop_def[] = {
      CL_QUEUE_PROPERTIES,
      CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_ON_DEVICE |
          CL_QUEUE_ON_DEVICE_DEFAULT | CL_QUEUE_PROFILING_ENABLE,
      CL_QUEUE_SIZE, maxQueueSize, 0};

  clCreateCommandQueueWithProperties(context, device, queue_prop_def, &err);
  ASSERT_EQ(CL_SUCCESS, err) << " clCreateCommandQueueWithProperties failed.";

  cl_program program =
      clCreateProgramWithSource(context, 1, (const char **)&source, NULL, &err);
  ASSERT_EQ(CL_SUCCESS, err) << " clCreateProgramWithSource failed.";

  err = clBuildProgram(program, 1, &device, "-g -cl-opt-disable -cl-std=CL2.0",
                       NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, err) << "clBuildProgram failed.";

  cl_kernel kernel = clCreateKernel(program, "enqueue_block_profiling", &err);
  ASSERT_EQ(CL_SUCCESS, err) << "clCreateKernel failed.";

  cl_mem mem = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                              sizeof(res), res, &err);
  ASSERT_EQ(CL_SUCCESS, err) << "clCreateBuffer failed.";

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem);
  ASSERT_EQ(CL_SUCCESS, err) << "clSetKernelArg failed.";

  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, &local, 0, NULL,
                               &event);
  ASSERT_EQ(CL_SUCCESS, err) << "clEnqueueNDRangeKernel failed.";

  err = clEnqueueReadBuffer(queue, mem, CL_TRUE, 0, sizeof(res), res, 0, NULL,
                            NULL);
  ASSERT_EQ(CL_SUCCESS, err) << "clEnqueueReadBuffer failed.";

  for (int i = 0; i < (sizeof(res) / sizeof(cl_int)); i++) {
    ASSERT_EQ(res[i], 0) << "kernel results validation failed.";
  }

  clReleaseProgram(program);
  clReleaseContext(context);
}
