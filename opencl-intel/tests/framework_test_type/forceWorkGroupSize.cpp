// Copyright (c) 2021 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include "TestsHelpClasses.h"
#include "common_utils.h"
#include "test_utils.h"

extern cl_device_type gDeviceType;

class ForceWGSizeTest : public ::testing::TestWithParam<std::vector<int>> {
protected:
  virtual void SetUp() override {
    m_forceWGSize = GetParam();
    unsigned numForceWGSize = m_forceWGSize.size();
    ASSERT_NE(0, numForceWGSize);

    std::string forceWGSizeStr = std::to_string(m_forceWGSize[0]);
    for (unsigned i = 1; i < numForceWGSize; ++i)
      forceWGSizeStr += "," + std::to_string(m_forceWGSize[i]);
    ASSERT_TRUE(
        SETENV("CL_CONFIG_CPU_FORCE_WORK_GROUP_SIZE", forceWGSizeStr.c_str()));

    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    err = clGetDeviceInfo(m_device, CL_DEVICE_MAX_WORK_GROUP_SIZE,
                          sizeof(size_t), &m_maxWorkGroupSize, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo CL_DEVICE_MAX_WORK_GROUP_SIZE");

    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    cl_command_queue_properties properties[] = {CL_QUEUE_PROPERTIES,
                                                CL_QUEUE_PROFILING_ENABLE, 0};
    m_queue = clCreateCommandQueueWithProperties(m_context, m_device,
                                                 properties, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");
  }

  virtual void TearDown() override {
    cl_int err = clReleaseKernel(m_kernel);
    ASSERT_OCL_SUCCESS(err, "clReleaseKernel");

    err = clReleaseProgram(m_program);
    ASSERT_OCL_SUCCESS(err, "clReleaseProgram");

    err = clReleaseCommandQueue(m_queue);
    ASSERT_OCL_SUCCESS(err, "clReleaseCommandQueue");

    err = clReleaseContext(m_context);
    ASSERT_OCL_SUCCESS(err, "clReleaseContext");
  }

  void BuildProgram(unsigned workDim) {
    ASSERT_TRUE(workDim == 1 || workDim == 2 || workDim == 3);
    const char *source;
    if (workDim == 1)
      source = "kernel void test(global int *dst) { \
        if (get_global_id(0) == 0) \
          *dst = (int)get_local_size(0); \
      }";
    else if (workDim == 2)
      source = "kernel void test(global int *dst) { \
        if (get_global_id(0) == 0 && get_global_id(1) == 0) { \
          dst[0] = (int)get_local_size(0); \
          dst[1] = (int)get_local_size(1); \
        } \
      }";
    else
      source = "kernel void test(global int *dst) { \
        if (get_global_id(0) == 0 && get_global_id(1) == 0 && \
            get_global_id(2) == 0) { \
          dst[0] = (int)get_local_size(0); \
          dst[1] = (int)get_local_size(1); \
          dst[2] = (int)get_local_size(2); \
        } \
      }";

    cl_int err;
    m_program = clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");

    err = clBuildProgram(m_program, 1, &m_device, "", nullptr, nullptr);
    if (CL_SUCCESS != err) {
      std::string log;
      ASSERT_NO_FATAL_FAILURE(GetBuildLog(m_device, m_program, log));
      FAIL() << log;
    }

    m_kernel = clCreateKernel(m_program, "test", &err);
    ASSERT_OCL_SUCCESS(err, "clCreateKernel test");
  }

  bool IsNegative(unsigned workDim) {
    unsigned numDim = std::min((unsigned)m_forceWGSize.size(), workDim);
    for (unsigned i = 0; i < numDim; ++i)
      if (m_forceWGSize[i] < 0)
        return true;
    return false;
  }

  bool IsGreater(unsigned workDim, size_t *gdim) {
    unsigned numDim = std::min((unsigned)m_forceWGSize.size(), workDim);
    for (unsigned i = 0; i < numDim; ++i)
      if (m_forceWGSize[i] > m_maxWorkGroupSize || m_forceWGSize[i] > gdim[i])
        return true;
    return false;
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;
  cl_program m_program;
  cl_kernel m_kernel;

  std::vector<int> m_forceWGSize;
  size_t m_maxWorkGroupSize;
};

TEST_P(ForceWGSizeTest, dim1) {
  unsigned workDim = 1;
  ASSERT_NO_FATAL_FAILURE(BuildProgram(workDim));

  cl_int result = 0;
  cl_int err = clSetKernelArgMemPointerINTEL(m_kernel, 0, &result);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  /// Test clMemBlockingFreeINTEL when enqueue failed.
  size_t gdim = 32;
  size_t ldim = 4;
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, workDim, nullptr, &gdim,
                               &ldim, 0, nullptr, nullptr);
  if (IsNegative(workDim) || IsGreater(workDim, &gdim)) {
    ASSERT_EQ(err, CL_INVALID_WORK_GROUP_SIZE);
    return;
  }
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  ASSERT_EQ(m_forceWGSize[0], result);
}

TEST_P(ForceWGSizeTest, dim1Null) {
  unsigned workDim = 1;
  ASSERT_NO_FATAL_FAILURE(BuildProgram(workDim));

  cl_int result = 0;
  cl_int err = clSetKernelArgMemPointerINTEL(m_kernel, 0, &result);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  /// Test clMemBlockingFreeINTEL when enqueue failed.
  size_t gdim = 32;
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, workDim, nullptr, &gdim,
                               nullptr, 0, nullptr, nullptr);
  if (IsNegative(workDim) || IsGreater(workDim, &gdim)) {
    ASSERT_EQ(err, CL_INVALID_WORK_GROUP_SIZE);
    return;
  }
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  ASSERT_EQ(m_forceWGSize[0], result);
}

TEST_P(ForceWGSizeTest, dim2) {
  unsigned workDim = 2;
  ASSERT_NO_FATAL_FAILURE(BuildProgram(workDim));

  cl_int result[2];
  cl_int err = clSetKernelArgMemPointerINTEL(m_kernel, 0, result);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  /// Test clMemBlockingFreeINTEL when enqueue failed.
  size_t gdim[] = {32, 32};
  size_t ldim[] = {4, 4};
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, workDim, nullptr, gdim, ldim,
                               0, nullptr, nullptr);
  if (IsNegative(workDim) || IsGreater(workDim, gdim)) {
    ASSERT_EQ(err, CL_INVALID_WORK_GROUP_SIZE);
    return;
  }
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  ASSERT_EQ(m_forceWGSize[0], result[0]);
  if (m_forceWGSize.size() >= workDim)
    ASSERT_EQ(m_forceWGSize[1], result[1]);
  else
    ASSERT_EQ(1, result[1]);
}

TEST_P(ForceWGSizeTest, dim3) {
  unsigned workDim = 3;
  ASSERT_NO_FATAL_FAILURE(BuildProgram(workDim));

  cl_int result[3];
  cl_int err = clSetKernelArgMemPointerINTEL(m_kernel, 0, result);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  /// Test clMemBlockingFreeINTEL when enqueue failed.
  size_t gdim[] = {32, 32, 32};
  size_t ldim[] = {4, 4, 4};
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, workDim, nullptr, gdim, ldim,
                               0, nullptr, nullptr);
  if (IsNegative(workDim) || IsGreater(workDim, gdim)) {
    ASSERT_EQ(err, CL_INVALID_WORK_GROUP_SIZE);
    return;
  }
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  ASSERT_EQ(m_forceWGSize[0], result[0]);
  unsigned numForceWGSize = m_forceWGSize.size();
  if (numForceWGSize >= workDim) {
    ASSERT_EQ(m_forceWGSize[1], result[1]);
    ASSERT_EQ(m_forceWGSize[2], result[2]);
  } else if (numForceWGSize == 2) {
    ASSERT_EQ(m_forceWGSize[1], result[1]);
    ASSERT_EQ(1, result[2]);
  } else {
    ASSERT_EQ(1, result[1]);
    ASSERT_EQ(1, result[2]);
  }
}

static std::vector<std::vector<int>> sizes = {
    {32, 16, 8, 4}, {32, 16}, {32}, {-1, 1}, {1024}};
INSTANTIATE_TEST_CASE_P(WG, ForceWGSizeTest, ::testing::ValuesIn(sizes), );

class ForceWGSizeSingleTest : public ::testing::Test {
protected:
  virtual void SetUp() override {}

  virtual void TearDown() override {}
};

/// This test covers a check of device kernel in ExecutionModule
/// EnqueueNDRangeKernel when CL_CONFIG_CPU_FORCE_WORK_GROUP_SIZE is set.
TEST_F(ForceWGSizeSingleTest, invalidDeviceProgram) {
  ASSERT_TRUE(SETENV("CL_CONFIG_CPU_FORCE_WORK_GROUP_SIZE", "32"));

  cl_platform_id platform;
  cl_device_id device;

  cl_int err = clGetPlatformIDs(1, &platform, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

  err = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

  cl_uint numComputeUnits;
  err = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint),
                        &numComputeUnits, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo CL_DEVICE_MAX_COMPUTE_UNITS");
  if (numComputeUnits < 2)
    return;

  cl_uint numDevices = 2;
  std::vector<cl_device_id> subDevices(numDevices);
  cl_uint numDevicesRet;
  std::vector<cl_device_partition_property> properties(numDevices + 2);
  properties[0] = CL_DEVICE_PARTITION_BY_COUNTS;
  for (int i = 1; i <= numDevices; ++i)
    properties[i] = 1;
  properties[numDevices + 1] = 0;
  err = clCreateSubDevices(device, &properties[0], numDevices, &subDevices[0],
                           &numDevicesRet);
  ASSERT_OCL_SUCCESS(err, "clCreateSubDevices");
  ASSERT_EQ(numDevices, numDevicesRet);

  cl_context context =
      clCreateContext(nullptr, 2, &subDevices[0], nullptr, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateContext");

  const char *source = "kernel void test() {}";

  cl_program program = clCreateProgramWithSource(
      context, 1, (const char **)&source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");

  // Build program for sub-device 0.
  err = clBuildProgram(program, 1, &subDevices[0], nullptr, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  cl_kernel kernel = clCreateKernel(program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel test");

  // Create command queue for sub-device 1.
  cl_command_queue queue =
      clCreateCommandQueueWithProperties(context, subDevices[1], nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");

  size_t gdim = 32;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &gdim, nullptr, 0,
                               nullptr, nullptr);
  ASSERT_EQ(CL_INVALID_PROGRAM_EXECUTABLE, err);

  err = clReleaseCommandQueue(queue);
  ASSERT_OCL_SUCCESS(err, "clReleaseCommandQueue");

  err = clReleaseKernel(kernel);
  ASSERT_OCL_SUCCESS(err, "clReleaseKernel");

  err = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(err, "clReleaseProgram");

  err = clReleaseContext(context);
  ASSERT_OCL_SUCCESS(err, "clReleaseContext");

  for (auto *d : subDevices) {
    err = clReleaseDevice(d);
    ASSERT_OCL_SUCCESS(err, "clReleaseDevice");
  }
}
