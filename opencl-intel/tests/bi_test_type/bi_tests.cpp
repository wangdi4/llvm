#include <map>
#include <string>

#include "bi_tests.h"
#include "cl_env.h"
#include "common_utils.h"
#include "options.hpp"

cl_device_type gDeviceType = CL_DEVICE_TYPE_CPU;

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

  device = fetchDevice(platform, gDeviceType);
  ASSERT_TRUE(device != nullptr)
      << "OpenCL device of type" << gDeviceType << " not found";

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

TEST(BuiltinTestType, Test_atomic_min_max_float) {
  EXPECT_TRUE(atomic_min_max_float_test());
}

CommandLineOption<std::string> deviceOption("--device_type");

int main(int argc, char **argv) {
  std::map<std::string, cl_device_type> clDeviceTypeMap;
  clDeviceTypeMap["cpu"] = CL_DEVICE_TYPE_CPU;
  clDeviceTypeMap["fpga_fast_emu"] = CL_DEVICE_TYPE_ACCELERATOR;
  clDeviceTypeMap["default"] = CL_DEVICE_TYPE_DEFAULT;
  clDeviceTypeMap["all"] = CL_DEVICE_TYPE_ALL;
  ::testing::InitGoogleTest(&argc, argv);
  std::string deviceTypeStr;
  if (argc > 1) {
    for (int i = 1; i < argc; i++)
      if (deviceOption.isMatch(argv[i])) {
        deviceTypeStr = deviceOption.getValue(argv[i]);
        auto iter = clDeviceTypeMap.find(deviceTypeStr);
        if (iter == clDeviceTypeMap.end()) {
          printf("error: unkown device option: %s\n", deviceTypeStr.c_str());
          return 1;
        }
        gDeviceType = iter->second;
      }
  }

  if (GetEnv(deviceTypeStr, "CL_DEVICE_TYPE")) {
    std::map<std::string, cl_device_type>::iterator iter =
        clDeviceTypeMap.find(deviceTypeStr);
    if (iter == clDeviceTypeMap.end()) {
      printf("error: unkown value of CL_DEVICE_TYPE env variable: %s\n",
             deviceTypeStr.c_str());
      return 1;
    }
    gDeviceType = iter->second;
  }

  return RUN_ALL_TESTS();
}
