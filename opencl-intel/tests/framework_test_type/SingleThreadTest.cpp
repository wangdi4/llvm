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

#ifndef _WIN32

#include "CL/cl.h"
#include "TestsHelpClasses.h"
#include "cl_sys_info.h"
#include "common_utils.h"

extern cl_device_type gDeviceType;

/// Each test will run only on a single logical CPU core.
class SingleThreadTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
    setSingleThread();

    cl_int err = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = nullptr;
  }

  virtual void TearDown() override {
    if (m_context) {
      cl_int err = clReleaseContext(m_context);
      EXPECT_OCL_SUCCESS(err, "clReleaseContext");
    }
  }

  /// Use the first CPU that is available to parent thread.
  void setSingleThread() {
    affinityMask_t parentMask;
    threadid_t parentTid = clMyParentThreadId();
    clGetThreadAffinityMask(&parentMask, parentTid);
    int numProcessors = (int)Intel::OpenCL::Utils::GetNumberOfProcessors();
    int cpu = -1;
    for (int i = 0; i < numProcessors; ++i) {
      if (CPU_ISSET(i, &parentMask)) {
        cpu = i;
        break;
      }
    }
    ASSERT_NE(cpu, -1) << "No available CPU is found";
    CPU_ZERO(&parentMask);
    CPU_SET(cpu, &parentMask);
    clSetThreadAffinityMask(&parentMask, parentTid);
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
};

#if 0
/// This test checks that there is no divide by zero in
/// CPUDevice::calculateComputeUnitMap.
TEST_F(SingleThreadTest, AffinityClosePlacesCores) {
  ASSERT_TRUE(SETENV("SYCL_CPU_CU_AFFINITY", "close"));
  ASSERT_TRUE(SETENV("SYCL_CPU_PLACES", "cores"));

  cl_int err;
  m_context = clCreateContext(NULL, 1, &m_device, NULL, NULL, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateContext");
}
#endif
#endif // #ifndef _WIN32
