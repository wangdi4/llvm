#include "bi_tests.h"

cl_platform_id fetchPlatform() {

  cl_uint numPlatforms;
  cl_int error = clGetPlatformIDs(0, NULL, &numPlatforms);
  if (error != CL_SUCCESS || numPlatforms == 0) {
    return nullptr;
  }

  std::vector<cl_platform_id> platforms(numPlatforms);
  error = clGetPlatformIDs(numPlatforms, &platforms[0], NULL);
  if (error != CL_SUCCESS) {
    return nullptr;
  }

  return platforms[0];
}

cl_device_id fetchDevice(cl_platform_id platform, cl_device_type type) {
  cl_uint numDevices;
  cl_int error = clGetDeviceIDs(platform, type, 0, NULL, &numDevices);
  if (error != CL_SUCCESS || numDevices == 0) {
    return nullptr;
  }

  std::vector<cl_device_id> devices(numDevices);
  error = clGetDeviceIDs(platform, type, numDevices, &devices[0], NULL);
  if (error != CL_SUCCESS) {
    return nullptr;
  }

  return devices[0];
}


void BITest::SetUp() {
  platform = fetchPlatform();
  ASSERT_TRUE(platform != nullptr) << "No OpenCL platforms available";

  device = fetchDevice(platform, CL_DEVICE_TYPE_CPU);
  ASSERT_TRUE(device != nullptr) << "OpenCL CPU device not found";

  cl_int error;
  context = clCreateContext(NULL, 1, &device, NULL, NULL, &error);
  ASSERT_EQ(error, CL_SUCCESS) << "clCreateContext failed";
}

void BITest::TearDown() {
  for (auto b : buffers) {
    clReleaseMemObject(b);
  }
  for (auto q : queues) {
    clReleaseCommandQueue(q);
  }
  clReleaseContext(context);
  clReleaseDevice(device);
}

cl_command_queue BITest::createCommandQueue() {
  cl_int error;
  cl_command_queue q =
    clCreateCommandQueueWithProperties(context, device, NULL, &error);

  if (error != CL_SUCCESS) {
    return nullptr;
  }
  queues.push_back(q);
  return q;
}

cl_mem BITest::createBuffer(size_t size, cl_mem_flags flags, void *host_ptr) {
  cl_int error;
  cl_mem buf = clCreateBuffer(context, flags, size, host_ptr, &error);

  if (error != CL_SUCCESS) {
    return nullptr;
  }

  buffers.push_back(buf);
  return buf;
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
