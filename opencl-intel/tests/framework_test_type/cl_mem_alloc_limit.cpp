// Copyright (c) 2019 Intel Corporation
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
#include "common_utils.h"
#include "tbb/parallel_for.h"
#include <thread>

extern cl_device_type gDeviceType;

class MemAllocLimitTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
    cl_int err = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    ASSERT_TRUE(SETENV("CL_CONFIG_CPU_FORCE_MAX_MEM_ALLOC_SIZE", "512MB"));
    m_context = clCreateContext(NULL, 1, &m_device, NULL, NULL, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");
  }

  virtual void TearDown() override {
    cl_int err = clReleaseContext(m_context);
    EXPECT_OCL_SUCCESS(err, "clReleaseContext");
    ASSERT_TRUE(UNSETENV("CL_CONFIG_CPU_FORCE_MAX_MEM_ALLOC_SIZE"));
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
};

// Check USM / SVM / Buffer has the same size limitation.
TEST_F(MemAllocLimitTest, memAllocSizeCheck) {
  cl_int err;
  cl_uint alignment = 0;
  cl_ulong deviceAllocSizeLimit = 0;
  err = clGetDeviceInfo(m_device, CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                        sizeof(cl_ulong), &deviceAllocSizeLimit, nullptr);
  ASSERT_OCL_SUCCESS(err,
                     "clGetDeviceInfo queries CL_DEVICE_MAX_MEM_ALLOC_SIZE.");

  size_t size = deviceAllocSizeLimit;
  // The size is configured by CL_CONFIG_CPU_FORCE_MAX_MEM_ALLOC_SIZE env.
  ASSERT_TRUE(512 * 1024 * 1024 == size);
  // Alloc USM
  void *usmbuf = clHostMemAllocINTEL(m_context, NULL, size, alignment, &err);
  ASSERT_OCL_SUCCESS(err, "clHostMemAllocINTEL");
  ASSERT_TRUE(nullptr != usmbuf);
  err = clMemFreeINTEL(m_context, usmbuf);
  ASSERT_OCL_SUCCESS(err, "clMemFreeINTEL");

  usmbuf = clHostMemAllocINTEL(m_context, NULL, size + 1, alignment, &err);
  ASSERT_EQ(CL_INVALID_BUFFER_SIZE, err)
      << "clHostMemAllocINTEL failed with exceeded size.";
  ASSERT_TRUE(nullptr == usmbuf);
  // Alloc SVM
  void *svmbuf = clSVMAlloc(m_context, CL_MEM_READ_WRITE, size, alignment);
  ASSERT_TRUE(nullptr != svmbuf);
  clSVMFree(m_context, svmbuf);

  svmbuf = clSVMAlloc(m_context, CL_MEM_READ_WRITE, size + 1, alignment);
  ASSERT_TRUE(nullptr == svmbuf);

  // Alloc Buffer
  cl_mem oclbuf =
      clCreateBuffer(m_context, CL_MEM_READ_WRITE, size, NULL, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateBuffer");
  err = clReleaseMemObject(oclbuf);
  ASSERT_OCL_SUCCESS(err, "clReleaseMemObject");

  oclbuf = clCreateBuffer(m_context, CL_MEM_READ_WRITE, size + 1, NULL, &err);
  ASSERT_EQ(CL_INVALID_BUFFER_SIZE, err)
      << "clCreatBuffer failed with exceeded size.";
}
