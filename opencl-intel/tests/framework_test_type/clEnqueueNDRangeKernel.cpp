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

class EnqueueNDRangeKernelTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");
  }

  virtual void TearDown() override {}

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
};

/// Checks that clEnqueueNDRangeKernel returns CL_INVALID_PROGRAM_EXECUTABLE if
/// kernel is built on a device but queue is created from another device.
TEST_F(EnqueueNDRangeKernelTest, invalidDeviceProgram) {
  cl_uint numComputeUnits;
  cl_int err = clGetDeviceInfo(m_device, CL_DEVICE_MAX_COMPUTE_UNITS,
                               sizeof(cl_uint), &numComputeUnits, nullptr);
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
  err = clCreateSubDevices(m_device, &properties[0], numDevices, &subDevices[0],
                           &numDevicesRet);
  ASSERT_OCL_SUCCESS(err, "clCreateSubDevices");
  ASSERT_EQ(numDevices, numDevicesRet);

  cl_context context = clCreateContext(nullptr, numDevices, &subDevices[0],
                                       nullptr, nullptr, &err);
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
