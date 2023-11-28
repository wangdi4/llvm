#include "CL/cl.h"
#include "cl_cpu_detect.h"
#include "cl_types.h"
#include "FrameworkTest.h"
#include "gtest_wrapper.h"
#include <stdio.h>

extern cl_device_type gDeviceType;
using namespace Intel::OpenCL::Utils;

void clGetCommandQueueInfo() {
  printf("---------------------------------------\n");
  printf("clGetCommandQueueInfo\n");
  printf("---------------------------------------\n");
  cl_int iRes = CL_SUCCESS;
  cl_device_id device;
  size_t size_ret = 0;

  cl_platform_id platform = 0;

  iRes = clGetPlatformIDs(1, &platform, NULL);
  ASSERT_EQ(iRes, CL_SUCCESS) << "clGetPlatformIDs failed\n";

  ///////////////CPU device///////////////

  iRes = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  ASSERT_EQ(iRes, CL_SUCCESS) << "clGetDeviceIDs failed\n";

  cl_context context =
      clCreateContext(nullptr, 1, &device, nullptr, nullptr, &iRes);
  ASSERT_EQ(iRes, CL_SUCCESS) << "clCreateContext failed\n";

  std::vector<cl_command_queue_properties> queueProps{
      CL_QUEUE_PROPERTIES,
      CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_ON_DEVICE |
          CL_QUEUE_ON_DEVICE_DEFAULT | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
      0};
  cl_command_queue queue = clCreateCommandQueueWithProperties(
      context, device, queueProps.data(), &iRes);
  ASSERT_EQ(iRes, CL_SUCCESS) << "clCreateCommandQueueWithProperties failed\n";

  std::vector<cl_command_queue_properties> queriedProps;
  iRes = clGetCommandQueueInfo(queue, CL_QUEUE_PROPERTIES_ARRAY, 0, nullptr,
                               &size_ret);
  ASSERT_EQ(queueProps.size() * sizeof(cl_command_queue_properties), size_ret)
      << "clGetCommandQueueInfo failed: size is unexpected\n";
  queriedProps.resize(queueProps.size());
  iRes = clGetCommandQueueInfo(queue, CL_QUEUE_PROPERTIES_ARRAY, size_ret,
                               queriedProps.data(), nullptr);
  ASSERT_EQ(queriedProps[0], queueProps[0])
      << "clGetCommandQueueInfo failed: queried property is unexpected\n";

  clReleaseCommandQueue(queue);
  cl_queue_properties props[] = {
      CL_QUEUE_PROPERTIES,
      CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE, 0};
  queue = clCreateCommandQueueWithProperties(context, device, props, &iRes);
  ASSERT_EQ(iRes, CL_SUCCESS)
      << "The 2nd call of clCreateCommandQueueWithProperties failed\n";
  iRes = clGetCommandQueueInfo(queue, CL_QUEUE_PROPERTIES_ARRAY, 0, nullptr,
                               &size_ret);
  ASSERT_EQ(size_ret, 24) << "clGetCommandQueueInfo failed: size is unexpected";

  clReleaseCommandQueue(queue);
  clReleaseContext(context);
}
