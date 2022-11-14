#include "TestsHelpClasses.h"
#include "common_utils.h"
#include "cpu_dev_limits.h"

#ifndef _WIN32
#include <sys/resource.h>
#endif

extern cl_device_type gDeviceType;

class LocalMemSizeTest : public ::testing::Test {
protected:
  virtual void TearDown() override {
    cl_int err;
    if (m_kernel) {
      err = clReleaseKernel(m_kernel);
      EXPECT_OCL_SUCCESS(err, "clReleaseKernel");
    }
    if (m_program) {
      err = clReleaseProgram(m_program);
      EXPECT_OCL_SUCCESS(err, "clReleaseProgram");
    }
    if (m_queue) {
      err = clReleaseCommandQueue(m_queue);
      EXPECT_OCL_SUCCESS(err, "clReleaseCommandQueue");
    }
    if (m_context) {
      err = clReleaseContext(m_context);
      EXPECT_OCL_SUCCESS(err, "clReleaseContext");
    }
  }

protected:
  void init() {
    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    m_queue =
        clCreateCommandQueueWithProperties(m_context, m_device, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");
  }

  void testBody(cl_ulong expectedLocalMemSize, cl_ulong stackSize,
                const char *programSource[]);

  cl_platform_id m_platform = nullptr;
  cl_device_id m_device = nullptr;
  cl_context m_context = nullptr;
  cl_command_queue m_queue = nullptr;
  cl_program m_program = nullptr;
  cl_kernel m_kernel = nullptr;
};

static cl_ulong trySetLocalMemSize(cl_ulong size) {
  std::string str = std::to_string(size) + "B";
  // set env variable to change the default value of local mem size
  if (!SETENV("CL_CONFIG_CPU_FORCE_LOCAL_MEM_SIZE", str.c_str()))
    return 0;

  return size;
}

// Test env CL_CONFIG_CPU_FORCE_LOCAL_MEM_SIZE to force a large local memory
// size.
TEST_F(LocalMemSizeTest, forceLargeSize) {
  // Allocate local size with 1MB, which should be larger than
  // CPU_DEV_LCL_MEM_SIZE. Ideally we shall query CL_DEVICE_LOCAL_MEM_SIZE,
  // but device isn't initialized yet.
  cl_ulong forceLocalSize = 1024 * 1024;
  ASSERT_LT(CPU_DEV_LCL_MEM_SIZE, forceLocalSize)
      << "need to increase forceLocalSize";

  cl_ulong expectedLocalMemSize = trySetLocalMemSize(forceLocalSize);
  ASSERT_NE(expectedLocalMemSize, 0) << "trySetLocalMemSize failed";

  ASSERT_NO_FATAL_FAILURE(init());

  const char *programSource = R"(
      __kernel void test(__global int* o) {
        const int size = LOCAL_SIZE / sizeof(int);
        __local int buf[size];
        int pwi = size / get_local_size(0);
        int lid = get_local_id(0);
        int gid = get_global_id(0);
        for (int i = lid * pwi; i < lid * pwi + pwi; ++i)
          buf[i] = gid;
        o[gid] = buf[pwi * lid + 1] + 2;
      })";

  ASSERT_NO_FATAL_FAILURE(testBody(expectedLocalMemSize, 0, &programSource));
}

// Test both large local memory size and larger stack size.
TEST_F(LocalMemSizeTest, forceLargeLocalSizeAndStackSize) {
  // Disable auto memory since test of stack size always succeeds in auto memory
  // mode.
  ASSERT_TRUE(SETENV("CL_CONFIG_AUTO_MEMORY", "false"));

  // Allocate local size with 1MB, which should be larger than
  // CPU_DEV_LCL_MEM_SIZE. Ideally we shall query CL_DEVICE_LOCAL_MEM_SIZE,
  // but device isn't initialized yet.
  cl_ulong forceLocalSize = 1024 * 1024;
  ASSERT_LT(CPU_DEV_LCL_MEM_SIZE, forceLocalSize)
      << "need to increase forceLocalSize";

  // Add with CPU_DEV_TBB_STACK_SIZE because TBB thread stack size is
  // approximately the sum of forceLocalSize and CPU_DEV_TBB_STACK_SIZE when
  // auto memory is disabled. Therefore, TBB thread stack size is increased
  // after env CL_CONFIG_CPU_FORCE_LOCAL_MEM_SIZE is set, which is necessary
  // since sum of local and private memory in the kernel exceeds
  // CPU_DEV_TBB_STACK_SIZE.
  cl_ulong stackSize = trySetStackSize(forceLocalSize + CPU_DEV_TBB_STACK_SIZE);
  ASSERT_NE(stackSize, 0) << "trySetStackSize failed";

  cl_ulong expectedLocalMemSize = trySetLocalMemSize(forceLocalSize);
  ASSERT_NE(expectedLocalMemSize, 0) << "trySetLocalMemSize failed";

  ASSERT_NO_FATAL_FAILURE(init());

  const char *programSource = R"(
      __kernel void test(__global int* o) {
        const int size = LOCAL_SIZE / sizeof(int);
        __local int buf[size];
        // Substract 1MB, which is CPU_DEV_STACK_EXTRA_SIZE.
        // Program is build in O0 mode so that unused 'a' isn't optimized out.
        char a[STACK_SIZE - LOCAL_SIZE - 1024 * 1024];
        int pwi = size / get_local_size(0);
        int lid = get_local_id(0);
        int gid = get_global_id(0);
        for (int i = lid * pwi; i < lid * pwi + pwi; ++i)
          buf[i] = gid;
        o[gid] = buf[pwi * lid + 1] + 2;
      })";

  ASSERT_NO_FATAL_FAILURE(
      testBody(expectedLocalMemSize, stackSize, &programSource));
}

#ifndef _WIN32
TEST_F(LocalMemSizeTest, unlimitedStackSize) {
  const char *programSource = R"(
      __kernel void test(__global int* o) {
        const int size = 7 * 1024 / sizeof(int); // 7 KB of local memory
        __local int buf[size];
        int pwi = size / get_local_size(0);
        int lid = get_local_id(0);
        int gid = get_global_id(0);
        for (int i = lid * pwi; i < lid * pwi + pwi; ++i)
          buf[i] = gid;
        o[gid] = buf[pwi * lid + 1] + 2;
      })";

  cl_ulong stackSize = trySetStackSize(RLIM_INFINITY);
  ASSERT_NE(stackSize, 0) << "trySetStackSize failed";

  ASSERT_NO_FATAL_FAILURE(init());

  if (gDeviceType == CL_DEVICE_TYPE_ACCELERATOR) {
    // TODO move to fpga_test_type
    ASSERT_NO_FATAL_FAILURE(testBody(256 * 1024, (cl_ulong)0, &programSource));
  }

  ASSERT_NO_FATAL_FAILURE(testBody(32 * 1024, (cl_ulong)0, &programSource));
}
#endif

void LocalMemSizeTest::testBody(cl_ulong expectedLocalMemSize,
                                cl_ulong stackSize,
                                const char *programSource[]) {
  cl_ulong localMemSize = 0;
  cl_int err = clGetDeviceInfo(m_device, CL_DEVICE_LOCAL_MEM_SIZE,
                               sizeof(cl_ulong), &localMemSize, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");
  ASSERT_EQ(expectedLocalMemSize, localMemSize)
      << "CL_DEVICE_LOCAL_MEM_SIZE mismatch";

  std::string options =
      "-cl-opt-disable -DSTACK_SIZE=" + std::to_string(stackSize) +
      " -DLOCAL_SIZE=" + std::to_string(expectedLocalMemSize);
  ASSERT_TRUE(BuildProgramSynch(m_context, 1, programSource, nullptr,
                                options.c_str(), &m_program));

  const size_t global_work_size = 10000;
  cl_mem buffer =
      clCreateBuffer(m_context, CL_MEM_READ_WRITE,
                     global_work_size * sizeof(cl_int), nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

  m_kernel = clCreateKernel(m_program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  err = clSetKernelArg(m_kernel, 0, sizeof(cl_mem), &buffer);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");

  const size_t local_work_size = 100;
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, nullptr, &global_work_size,
                               &local_work_size, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  cl_int data[global_work_size] = {0};

  err = clEnqueueReadBuffer(m_queue, buffer, CL_TRUE, 0,
                            global_work_size * sizeof(cl_int), data, 0, nullptr,
                            nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadBuffer");

  for (size_t i = 0; i < global_work_size; ++i)
    ASSERT_EQ((cl_int)(i + 2), data[i]) << "data[" << i << "] mismatch";
}
