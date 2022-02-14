#include "TestsHelpClasses.h"
#include "common_utils.h"
#include "cpu_dev_limits.h"
#include <tbb/global_control.h>

extern cl_device_type gDeviceType;

class PrivateMemSizeTest : public ::testing::Test {
protected:
  void SetUp() override {
    cl_int err = clGetPlatformIDs(1, &platform_private, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatrormIDs");

    err = clGetDeviceIDs(platform_private, gDeviceType, 1, &device_private,
                         nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");
  }

  void TearDown() override {
    if (buffer_private)
      clReleaseMemObject(buffer_private);
    if (kernel_private)
      clReleaseKernel(kernel_private);
    if (queue_private)
      clReleaseCommandQueue(queue_private);
    if (program_private)
      clReleaseProgram(program_private);
    if (context_private)
      clReleaseContext(context_private);
  }

protected:
  void testBody(cl_ulong, const std::string &, bool out_of_recources = false);

  cl_platform_id platform_private = nullptr;
  cl_device_id device_private = nullptr;
  cl_context context_private = nullptr;
  cl_command_queue queue_private = nullptr;
  cl_kernel kernel_private = nullptr;
  cl_mem buffer_private = nullptr;
  cl_program program_private = nullptr;
};

static cl_ulong trySetPrivateMemSize(cl_ulong size, std::string unit = "") {
#ifdef _WIN32
  printf("NOTE:\nDue to some strange behaviour of env variables on Windows\n");
  printf("\tthis test works only if you specify "
         "CL_CONFIG_CPU_FORCE_PRIVATE_MEM_SIZE from shell\n");
  printf("\tIn CI system it is done by .pm runner (framework_test_type.pm)\n");
#endif
  std::string str;
  if (unit.empty() || unit == "B")
    str = std::to_string(size) + "B";
  else if (unit == "K" || unit == "KB")
    str = std::to_string(size / 1024) + "K";
  else if (unit == "M" || unit == "MB")
    str = std::to_string(size / 1024 / 1024) + "M";
  else
    return 0;
  // set env variable to change the default value of private mem size
  if (!SETENV("CL_CONFIG_CPU_FORCE_PRIVATE_MEM_SIZE", str.c_str()))
    return 0;

  return size; // Pass STACK_SIZE to kernel
}

static bool vectorizerMode(bool enabled) {
  std::string mode = enabled ? "True" : "False";
  return SETENV("CL_CONFIG_USE_VECTORIZER", mode.c_str());
}

// Temporarily disable this test with x86 win build, as it's flaky failed.
#if !(defined(_WIN32) && !defined(_WIN64))
TEST_F(PrivateMemSizeTest, Basic) {
  std::string programSources = "__kernel void test(__global int* o)\n"
                               "{\n"
                               "    const int size = (STACK_SIZE/17) / "
                               "sizeof(int);\n" // (STACK_SIZE/(SIMD_WIDTH+1))MB
                                                // of private
                                                // memory
                               "    __private volatile int buf[size];\n"
                               "    int gid = get_global_id(0);\n"
                               "    for (int i = 0; i < size; ++i)\n"
                               "        buf[i] = gid;\n"
                               "    o[gid] = buf[gid + 1] + 2;\n"
                               "}";

  printf("cl_device_private_mem_size_test\n");

  cl_ulong stackSize =
      trySetStackSize(STACK_SIZE); // Extra stack size for scalar kernel
  ASSERT_TRUE(CheckCondition("trySetStackSize", stackSize != 0));

  bool enabledVectorizer = vectorizerMode(true);
  ASSERT_TRUE(CheckCondition("vectorizerMode", enabledVectorizer == true));

  cl_ulong expectedPrivateMemSize = trySetPrivateMemSize(STACK_SIZE, "K");
  ASSERT_TRUE(
      CheckCondition("trySetPrivateMemSize", expectedPrivateMemSize != 0));

  ASSERT_NO_FATAL_FAILURE(testBody(expectedPrivateMemSize, programSources));
}
#endif // #if !(defined(_WIN32) && !defined(_WIN64))

TEST_F(PrivateMemSizeTest, OutOfResources) {
  std::string programSources =
      "__kernel void test(__global int* o)\n"
      "{\n"
      "    const int size = (STACK_SIZE/2) / sizeof(int);\n" // (STACK_SIZE/2)MB
                                                             // of private
                                                             // memory
      "    printf(\"SIZE: %d \\n\", size);\n"
      "    __private volatile int buf[size];\n"
      "    int gid = get_global_id(0);\n"
      "    for (int i = 0; i < size; ++i)\n"
      "        buf[i] = gid;\n"
      "    o[gid] = buf[gid + 1] + 2;\n"
      "}";

  printf("cl_device_private_mem_size_test_out_of_resources\n");

  // set auto memory to false and get out of resource error with huge private
  // memory
  ASSERT_TRUE(SETENV("CL_CONFIG_AUTO_MEMORY", "false"));

  cl_ulong stackSize = trySetStackSize(STACK_SIZE);
  ASSERT_TRUE(CheckCondition("trySetStackSize", stackSize != 0));

  bool enabledVectorizer = vectorizerMode(true);
  ASSERT_TRUE(CheckCondition("vectorizerMode", enabledVectorizer == true));

  cl_ulong expectedPrivateMemSize = trySetPrivateMemSize(STACK_SIZE);
  ASSERT_TRUE(
      CheckCondition("trySetPrivateMemSize", expectedPrivateMemSize != 0));

  ASSERT_NO_FATAL_FAILURE(
      testBody(expectedPrivateMemSize, programSources, true));

  ASSERT_TRUE(UNSETENV("CL_CONFIG_AUTO_MEMORY"));
}

TEST_F(PrivateMemSizeTest, WithoutVectorizer) {
  std::string programSources =
      "__kernel void test(__global int* o)\n"
      "{\n"
      "    const int size = (STACK_SIZE - 1024*1024) / "
      "sizeof(int);\n" // STACK_SIZE MB - 1MB of private memory
      "    printf(\"SIZE: %d \\n\", size);\n"
      "    __private volatile int buf[size];\n"
      "    int gid = get_global_id(0);\n"
      "    for (int i = 0; i < size; ++i)\n"
      "        buf[i] = gid;\n"
      "    o[gid] = buf[gid + 1] + 2;\n"
      "}";

  printf("cl_device_private_mem_size_test_without_vectorizer\n");

  cl_ulong stackSize = trySetStackSize(STACK_SIZE);
  ASSERT_TRUE(CheckCondition("trySetStackSize", stackSize != 0));

  bool disabledVectorizer = vectorizerMode(false);
  ASSERT_TRUE(CheckCondition("vectorizerMode", disabledVectorizer == true));

  cl_ulong expectedPrivateMemSize = trySetPrivateMemSize(STACK_SIZE, "M");
  ASSERT_TRUE(
      CheckCondition("trySetPrivateMemSize", expectedPrivateMemSize != 0));

  ASSERT_NO_FATAL_FAILURE(testBody(expectedPrivateMemSize, programSources));

  bool enableVectorizer = vectorizerMode(true);
  ASSERT_TRUE(CheckCondition("vectorizerMode", enableVectorizer == true));
}

TEST_F(PrivateMemSizeTest, SmallStackSize) {
  ASSERT_TRUE(SETENV("CL_CONFIG_AUTO_MEMORY", "false"));

  cl_ulong stackSize = trySetStackSize(1024 * 1024);
  ASSERT_TRUE(CheckCondition("trySetStackSize", stackSize != 0));

  cl_ulong expectedPrivateMemSize = trySetPrivateMemSize(stackSize);
  ASSERT_TRUE(
      CheckCondition("trySetPrivateMemSize", expectedPrivateMemSize != 0));

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform_private, 0};
  cl_int err;
  context_private =
      clCreateContext(prop, 1, &device_private, nullptr, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateContext");

  size_t activeStackSize =
      tbb::global_control::active_value(tbb::global_control::thread_stack_size);
  ASSERT_EQ(activeStackSize,
            stackSize + CPU_DEV_LCL_MEM_SIZE + CPU_DEV_BASE_STACK_SIZE);

  ASSERT_TRUE(UNSETENV("CL_CONFIG_AUTO_MEMORY"));
}

// Disable this test with x86 win build, as auto memory is disabled by default.
#if !(defined(_WIN32) && !defined(_WIN64))
TEST_F(PrivateMemSizeTest, LargeStackSizeWithAutoMemory) {
  std::string programSources =
      "__kernel void test(__global int* o)\n"
      "{\n"
      "    const int size = (STACK_SIZE/2) / sizeof(int);\n" // (STACK_SIZE/2)MB
                                                             // of private
                                                             // memory
      "    printf(\"SIZE: %d \\n\", size);\n"
      "    __private volatile int buf[size];\n"
      "    int gid = get_global_id(0);\n"
      "    for (int i = 0; i < size; ++i)\n"
      "        buf[i] = gid;\n"
      "    o[gid] = buf[gid + 1] + 2;\n"
      "}";

  printf("cl_device_private_mem_size_test_with_auto_memory\n");

  // set auto memory to true and there is no out of resource error with huge
  // private memory
  ASSERT_TRUE(SETENV("CL_CONFIG_AUTO_MEMORY", "true"));

  cl_ulong stackSize = trySetStackSize(STACK_SIZE);
  ASSERT_TRUE(CheckCondition("trySetStackSize", stackSize != 0));

  bool enabledVectorizer = vectorizerMode(true);
  ASSERT_TRUE(CheckCondition("vectorizerMode", enabledVectorizer == true));

  cl_ulong expectedPrivateMemSize = trySetPrivateMemSize(STACK_SIZE);
  ASSERT_TRUE(
      CheckCondition("trySetPrivateMemSize", expectedPrivateMemSize != 0));

  ASSERT_NO_FATAL_FAILURE(
      testBody(expectedPrivateMemSize, programSources, false));

  ASSERT_TRUE(UNSETENV("CL_CONFIG_AUTO_MEMORY"));
}
#endif // #if !(defined(_WIN32) && !defined(_WIN64))

void PrivateMemSizeTest::testBody(cl_ulong expectedPrivateMemSize,
                                  const std::string &programSources,
                                  bool out_of_recources) {
  cl_int err;

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform_private, 0};
  context_private =
      clCreateContext(prop, 1, &device_private, nullptr, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateContext");

  queue_private = clCreateCommandQueueWithProperties(
      context_private, device_private, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");

  const char *ps = programSources.c_str();
  std::string options =
      "-DSTACK_SIZE=" + std::to_string(expectedPrivateMemSize);
  ASSERT_TRUE(BuildProgramSynch(context_private, 1, (const char **)&ps, nullptr,
                                options.c_str(), &program_private));

  const size_t global_work_size = 1;
  buffer_private =
      clCreateBuffer(context_private, CL_MEM_READ_WRITE,
                     global_work_size * sizeof(cl_int), nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

  kernel_private = clCreateKernel(program_private, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  err = clSetKernelArg(kernel_private, 0, sizeof(cl_mem), &buffer_private);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");

  const size_t local_work_size = 1;
  err = clEnqueueNDRangeKernel(queue_private, kernel_private, 1, nullptr,
                               &global_work_size, &local_work_size, 0, nullptr,
                               nullptr);
  if (out_of_recources) {
    ASSERT_EQ(err, CL_OUT_OF_RESOURCES) << "clEnqueueNDRangeKernel";
    return;
  }
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  err = clFinish(queue_private);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  cl_int data[global_work_size] = {0};

  err = clEnqueueReadBuffer(queue_private, buffer_private, CL_TRUE, 0,
                            global_work_size * sizeof(cl_int), data, 0, nullptr,
                            nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

  for (size_t i = 0; i < global_work_size; ++i) {
    ASSERT_EQ((cl_int)(i + 2), data[i]) << "kernel_private results verify fail";
  }
}
