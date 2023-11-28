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

#include "TestsHelpClasses.h"
#include "cl_sys_info.h"
#include "common_utils.h"
#include "task_executor.h"
#include "tbb/global_control.h"
#include "tbb/info.h"

extern cl_device_type gDeviceType;

class NumaTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
    ASSERT_TRUE(SETENV("SYCL_CPU_PLACES", "numa_domains"));

    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    std::vector<int> tbbNumaNodes = tbb::info::numa_nodes();
    m_numNumaNodes = (int)tbbNumaNodes.size();

    m_queue = nullptr;
    m_context = nullptr;
  }

  virtual void TearDown() override {
    cl_int err;
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
  cl_platform_id m_platform = nullptr;
  cl_device_id m_device = nullptr;
  cl_context m_context = nullptr;
  cl_command_queue m_queue = nullptr;
  int m_numNumaNodes = 0;
};

/// This test checks that NUMA API is working correctly if SYCL_CPU_NUM_CUS is
/// set to half number of CPUs.
/// Disable the test because it is flaky.
TEST_F(NumaTest, halfCUs) {
  // Skip test if there is only a single NUMA node.
  if (m_numNumaNodes < 2)
    return;

  // Set to use half number of CPUs before clCreateContext.
  std::string halfNumCUs =
      std::to_string(Intel::OpenCL::Utils::GetNumberOfProcessors() / 2);
  ASSERT_TRUE(SETENV("SYCL_CPU_NUM_CUS", halfNumCUs.c_str()));

  cl_int err;
  m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateContext");

  m_queue =
      clCreateCommandQueueWithProperties(m_context, m_device, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");

  size_t num = 1024 * 1024;
  int val = 2;
  std::vector<int> src(num, val);
  std::vector<int> dst(num, 0);

  // Turn off vectorizer for the stream copy kernel.
  ASSERT_TRUE(SETENV("CL_CONFIG_USE_VECTORIZER", "false"));

  // Build program and kernel
  const char *source[] = {
      "__kernel void test(const __global int * restrict src,\n"
      "  __global int * restrict dst) {\n"
      "  size_t gid = get_global_id(0);"
      "  dst[gid] = src[gid];\n"
      "}\n"};
  cl_program program;
  ASSERT_TRUE(
      BuildProgramSynch(m_context, 1, source, nullptr, nullptr, &program));

  cl_kernel kernel;
  kernel = clCreateKernel(program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");

  err = clSetKernelArgMemPointerINTEL(kernel, 0, &src[0]);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  err = clSetKernelArgMemPointerINTEL(kernel, 1, &dst[0]);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArgMemPointerINTEL");

  size_t gdim = num;
  err = clEnqueueNDRangeKernel(m_queue, kernel, 1, nullptr, &gdim, nullptr, 0,
                               nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  ASSERT_TRUE(src == dst) << "Result mismatch";

  err = clReleaseKernel(kernel);
  EXPECT_OCL_SUCCESS(err, "clReleaseKernel");
  err = clReleaseProgram(program);
  EXPECT_OCL_SUCCESS(err, "clReleaseProgram");
}
