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

    // Also test scalar kernel because it has a different logic to compare
    // workgroup size than vectorized kernel.
    if (numForceWGSize == 1)
      ASSERT_TRUE(SETENV("CL_CONFIG_USE_VECTORIZER", "false"));

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

    m_buildOptions = "-cl-std=CL2.0";
    m_program = nullptr;
    m_kernel = nullptr;
  }

  virtual void TearDown() override {
    int err;
    if (m_kernel) {
      err = clReleaseKernel(m_kernel);
      ASSERT_OCL_SUCCESS(err, "clReleaseKernel");
    }

    if (m_program) {
      err = clReleaseProgram(m_program);
      ASSERT_OCL_SUCCESS(err, "clReleaseProgram");
    }

    err = clReleaseCommandQueue(m_queue);
    ASSERT_OCL_SUCCESS(err, "clReleaseCommandQueue");

    err = clReleaseContext(m_context);
    ASSERT_OCL_SUCCESS(err, "clReleaseContext");
  }

  void BuildProgram(unsigned workDim, bool createKernel = true) {
    ASSERT_TRUE(workDim == 1 || workDim == 2 || workDim == 3);
    const char *source;
    if (workDim == 1)
      source = "kernel void test(global int *dst, global int *dummy) { \
        size_t gid = get_global_id(0); \
        if (gid == 0) \
          *dst = (int)get_local_size(0); \
        dummy[gid] = gid; \
      }";
    else if (workDim == 2)
      source = "kernel void test(global int *dst, global int *dummy) { \
        size_t gid0 = get_global_id(0); \
        size_t gid1 = get_global_id(1); \
        if (gid0 == 0 && gid1 == 0) { \
          dst[0] = (int)get_local_size(0); \
          dst[1] = (int)get_local_size(1); \
        } \
        size_t idx = gid1 * get_global_size(0) + gid0; \
        dummy[idx] = idx; \
      }";
    else
      source = "kernel void test(global int *dst, global int *dummy) { \
        size_t gid0 = get_global_id(0); \
        size_t gid1 = get_global_id(1); \
        size_t gid2 = get_global_id(2); \
        if (gid0 == 0 && gid1 == 0 && gid2 == 0) { \
          dst[0] = (int)get_local_size(0); \
          dst[1] = (int)get_local_size(1); \
          dst[2] = (int)get_local_size(2); \
        } \
        size_t idx = gid2 * get_global_size(1) * get_global_size(0) + \
            gid1 * get_global_size(0) + gid0; \
        dummy[idx] = idx; \
      }";

    cl_int err;
    m_program = clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");

    err = clBuildProgram(m_program, 1, &m_device, m_buildOptions.c_str(),
                         nullptr, nullptr);
    if (CL_SUCCESS != err) {
      std::string log;
      ASSERT_NO_FATAL_FAILURE(GetBuildLog(m_device, m_program, log));
      FAIL() << log;
    }

    if (createKernel) {
      m_kernel = clCreateKernel(m_program, "test", &err);
      ASSERT_OCL_SUCCESS(err, "clCreateKernel test");
    }
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
      if ((size_t)m_forceWGSize[i] > m_maxWorkGroupSize ||
          (size_t)m_forceWGSize[i] > gdim[i])
        return true;
    return false;
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;

  std::string m_buildOptions;
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

  size_t gdim = 32;
  size_t ldim = 4;

  std::vector<cl_int> dummy(gdim);
  err = clSetKernelArgMemPointerINTEL(m_kernel, 1, &dummy[0]);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

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

  for (size_t i = 0; i < dummy.size(); ++i)
    ASSERT_EQ(i, dummy[i]);
}

TEST_P(ForceWGSizeTest, dim1Null) {
  unsigned workDim = 1;
  ASSERT_NO_FATAL_FAILURE(BuildProgram(workDim));

  cl_int result = 0;
  cl_int err = clSetKernelArgMemPointerINTEL(m_kernel, 0, &result);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  size_t gdim = 32;

  std::vector<cl_int> dummy(gdim);
  err = clSetKernelArgMemPointerINTEL(m_kernel, 1, &dummy[0]);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

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

  for (size_t i = 0; i < dummy.size(); ++i)
    ASSERT_EQ(i, dummy[i]);
}

TEST_P(ForceWGSizeTest, dim1AOT) {
  unsigned workDim = 1;
  ASSERT_NO_FATAL_FAILURE(BuildProgram(workDim, /*createKernel*/ false));

  cl_program program;
  ASSERT_NO_FATAL_FAILURE(CreateAndBuildProgramFromProgramBinaries(
      m_context, m_device, m_buildOptions, m_program, program));

  cl_int err;
  cl_kernel kernel = clCreateKernel(program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel test");

  cl_int result = 0;
  err = clSetKernelArgMemPointerINTEL(kernel, 0, &result);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  size_t gdim = 32;
  size_t ldim = 4;

  std::vector<cl_int> dummy(gdim);
  err = clSetKernelArgMemPointerINTEL(kernel, 1, &dummy[0]);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  err = clEnqueueNDRangeKernel(m_queue, kernel, workDim, nullptr, &gdim, &ldim,
                               0, nullptr, nullptr);
  if (IsNegative(workDim) || IsGreater(workDim, &gdim)) {
    ASSERT_EQ(err, CL_INVALID_WORK_GROUP_SIZE);
    return;
  }
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  ASSERT_EQ(m_forceWGSize[0], result);

  for (size_t i = 0; i < dummy.size(); ++i)
    ASSERT_EQ(i, dummy[i]);

  err = clReleaseKernel(kernel);
  ASSERT_OCL_SUCCESS(err, "clReleaseKernel");

  err = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(err, "clReleaseProgram");
}

TEST_P(ForceWGSizeTest, dim2) {
  unsigned workDim = 2;
  ASSERT_NO_FATAL_FAILURE(BuildProgram(workDim));

  cl_int result[2];
  cl_int err = clSetKernelArgMemPointerINTEL(m_kernel, 0, result);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  size_t gdim[] = {32, 32};
  size_t ldim[] = {4, 4};

  std::vector<cl_int> dummy(gdim[0] * gdim[1]);
  err = clSetKernelArgMemPointerINTEL(m_kernel, 1, &dummy[0]);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

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

  for (size_t i = 0; i < dummy.size(); ++i)
    ASSERT_EQ(i, dummy[i]);
}

TEST_P(ForceWGSizeTest, dim3) {
  unsigned workDim = 3;
  ASSERT_NO_FATAL_FAILURE(BuildProgram(workDim));

  cl_int result[3];
  cl_int err = clSetKernelArgMemPointerINTEL(m_kernel, 0, result);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  size_t gdim[] = {32, 32, 32};
  size_t ldim[] = {4, 4, 4};

  std::vector<cl_int> dummy(gdim[0] * gdim[1] * gdim[2]);
  err = clSetKernelArgMemPointerINTEL(m_kernel, 1, &dummy[0]);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

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

  for (size_t i = 0; i < dummy.size(); ++i)
    ASSERT_EQ(i, dummy[i]);
}

static std::vector<std::vector<int>> sizes = {
    {32, 16, 8, 4}, {32, 16}, {32}, {5}, {-1, 1}, {1024}};
INSTANTIATE_TEST_SUITE_P(WG, ForceWGSizeTest, ::testing::ValuesIn(sizes));

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
  for (cl_uint i = 1; i <= numDevices; ++i)
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
