// Copyright (c) 2020 Intel Corporation
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

#include "CL/cl.h"
#include "FrameworkTestThreads.h"
#include "TestsHelpClasses.h"
#include <numeric>

extern cl_device_type gDeviceType;

class WorkDim1VecDimUniteWGTest : public ::testing::TestWithParam<bool> {
protected:
  virtual void SetUp() override {
    if (GetParam())
      HasNonUniformWG = true;

    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    cl_command_queue_properties properties[] = {CL_QUEUE_PROPERTIES,
                                                CL_QUEUE_PROFILING_ENABLE, 0};
    m_queue = clCreateCommandQueueWithProperties(m_context, m_device,
                                                 properties, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");
  }

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
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context = nullptr;
  cl_command_queue m_queue = nullptr;
  cl_program m_program = nullptr;
  cl_kernel m_kernel = nullptr;
  bool HasNonUniformWG = false;
};

static void BuildProgram(cl_context context, cl_device_id device,
                         const char *source[], int count, cl_program &program) {
  cl_int err;
  program = clCreateProgramWithSource(context, count, source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
  err = clBuildProgram(program, 1, &device, "-cl-std=CL2.0", nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");
}

TEST_P(WorkDim1VecDimUniteWGTest, VecDim0) {
  cl_int err = CL_SUCCESS;

  static const char *source = R"(
__kernel void test(__global int *dst, ulong size)
{
  if (get_global_id(0) < size)
    dst[get_global_id(0)] = 1;
  else
    dst[get_global_id(0)] = 64;
}
)";

  ASSERT_NO_FATAL_FAILURE(
      BuildProgram(m_context, m_device, (const char **)&source, 1, m_program));

  m_kernel = clCreateKernel(m_program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  cl_uint max_compute_units = 0;
  cl_ulong max_mem_alloc_size = 0;

  err = clGetDeviceInfo(m_device, CL_DEVICE_MAX_COMPUTE_UNITS,
                        sizeof(max_compute_units), &max_compute_units, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");

  err =
      clGetDeviceInfo(m_device, CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                      sizeof(max_mem_alloc_size), &max_mem_alloc_size, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");

  size_t local_work_size = 5;
  size_t VF = 16;
  size_t global_work_size = local_work_size * max_compute_units * VF;

  if (HasNonUniformWG) {
    global_work_size += 1;
  }

  cl_uint buffer_size = (global_work_size / local_work_size + HasNonUniformWG) *
                        local_work_size * sizeof(int);
  if (buffer_size > max_mem_alloc_size) {
    buffer_size = max_mem_alloc_size;
    global_work_size = max_mem_alloc_size / sizeof(int);
  }

  // Create buffer.
  cl_mem buffer;
  buffer = clCreateBuffer(m_context, (cl_mem_flags)(CL_MEM_READ_WRITE),
                          buffer_size, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");

  // Fill the buffer.
  const int fill_pattern = 0x0;
  err = clEnqueueFillBuffer(m_queue, buffer, (void *)&fill_pattern,
                            sizeof(fill_pattern), 0, buffer_size, 0, nullptr,
                            nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueFillBuffer");

  // Set m_kernel arguments.
  err = clSetKernelArg(m_kernel, 0, sizeof(buffer), &buffer);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");
  cl_ulong gw_size_ulong = global_work_size;
  err = clSetKernelArg(m_kernel, 1, sizeof(gw_size_ulong), &gw_size_ulong);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");

  // Enqueue m_kernel.
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, nullptr, &global_work_size,
                               &local_work_size, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");

  // Compare result.
  void *mapped = clEnqueueMapBuffer(m_queue, buffer, CL_TRUE, CL_MAP_READ, 0,
                                    buffer_size, 0, nullptr, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clEnqueueMapBuffer");

  size_t errors = 0;
  cl_uint *data = (cl_uint *)mapped;
  for (cl_uint i = 0; i < buffer_size / sizeof(int); ++i) {
    if ((i < global_work_size && data[i] != 1) ||
        (i >= global_work_size && data[i] != 0))
      errors++;
  }

  ASSERT_EQ(errors, 0) << " number of errors: " << errors;

  err = clEnqueueUnmapMemObject(m_queue, buffer, mapped, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueUnmapMemObject");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  err = clReleaseMemObject(buffer);
  ASSERT_OCL_SUCCESS(err, "clReleaseMemObject");
}

TEST_P(WorkDim1VecDimUniteWGTest, VecDim1) {
  const char *source[] = {R"(kernel void test(global size_t *dst) {
    dst[get_global_id(0) + get_global_size(0)] = get_global_id(dst[get_global_id(0)]);
  })"};

  ASSERT_NO_FATAL_FAILURE(
      BuildProgram(m_context, m_device, source, 1, m_program));

  cl_int err;
  m_kernel = clCreateKernel(m_program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  size_t globalSize = 2;
  constexpr size_t localSize = 2;
  if (HasNonUniformWG)
    globalSize += 1;
  std::vector<size_t> dst(globalSize * 2);
  std::iota(dst.begin(), dst.end(), 0);

  err = clSetKernelArgMemPointerINTEL(m_kernel, 0, &dst[0]);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, nullptr, &globalSize,
                               &localSize, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  for (size_t i = globalSize; i < dst.size(); ++i)
    ASSERT_EQ(dst[i], 0) << "wrong output at index " << i;
}

INSTANTIATE_TEST_SUITE_P(VecDim, WorkDim1VecDimUniteWGTest, ::testing::Bool());
