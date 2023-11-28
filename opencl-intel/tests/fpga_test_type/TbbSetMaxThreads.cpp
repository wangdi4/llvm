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
#include "tbb/global_control.h"
#include "test_utils.h"

extern cl_device_type gDeviceType;

class TBBMaxThreadsTest : public ::testing::TestWithParam<size_t> {
protected:
  virtual void SetUp() override {
    m_numThreads = GetParam();
    cl_int err = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");
  }

  virtual void TearDown() override {}

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  size_t m_numThreads;
};

/// Checks that there is no assert fail when host sets max_allowed_parallelism
/// to different values.
TEST_P(TBBMaxThreadsTest, noAssert) {
  auto controller = tbb::global_control{
      tbb::global_control::max_allowed_parallelism, m_numThreads};
  cl_int err;
  cl_context context = clCreateContext(NULL, 1, &m_device, NULL, NULL, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateContext");
  err = clReleaseContext(context);
  ASSERT_OCL_SUCCESS(err, "clReleaseContext");
}

INSTANTIATE_TEST_SUITE_P(TaskExecutor, TBBMaxThreadsTest,
                         ::testing::Values(1, 2, 128));
