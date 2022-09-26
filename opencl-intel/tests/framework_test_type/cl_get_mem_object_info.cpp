// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "CL/cl.h"
#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "cl_types.h"
#include <stdio.h>

extern cl_device_type gDeviceType;

class GetMemObjectInfoTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
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

  virtual void TearDown() override {
    cl_int err = clReleaseCommandQueue(m_queue);
    EXPECT_OCL_SUCCESS(err, "clReleaseCommandQueue");

    err = clReleaseContext(m_context);
    EXPECT_OCL_SUCCESS(err, "clReleaseContext");
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;
};

TEST_F(GetMemObjectInfoTest, CL_MEM_HOST_PTR) {
  size_t num = 1024 * 1024;
  std::vector<int> v(num);
  size_t bufferSize = v.size() * sizeof(v[0]);

  // Test CL_MEM_USE_HOST_PTR not present.
  cl_int err;
  cl_mem buffer =
      clCreateBuffer(m_context, CL_MEM_READ_WRITE, bufferSize, nullptr, &err);
  void *queriedHostPtr = (void *)&v[0];
  err = clGetMemObjectInfo(buffer, CL_MEM_HOST_PTR, sizeof(queriedHostPtr),
                           &queriedHostPtr, nullptr);
  ASSERT_EQ(queriedHostPtr, nullptr);

  // Test CL_MEM_USE_HOST_PTR.
  queriedHostPtr = nullptr;
  cl_mem bufferUseHostPtr =
      clCreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                     bufferSize, (void *)&v[0], &err);
  err = clGetMemObjectInfo(bufferUseHostPtr, CL_MEM_HOST_PTR,
                           sizeof(queriedHostPtr), &queriedHostPtr, nullptr);
  ASSERT_EQ(queriedHostPtr, (void *)&v[0]);

  // Test both CL_MEM_USE_HOST_PTR and CL_MEM_COPY_HOST_PTR.
  queriedHostPtr = nullptr;
  cl_mem bufferUseHostPtr2 = clCreateBuffer(
      m_context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR,
      bufferSize, (void *)&v[0], &err);
  err = clGetMemObjectInfo(bufferUseHostPtr2, CL_MEM_HOST_PTR,
                           sizeof(queriedHostPtr), &queriedHostPtr, nullptr);
  ASSERT_EQ(queriedHostPtr, (void *)&v[0]);

  err = clReleaseMemObject(buffer);
  ASSERT_OCL_SUCCESS(err, "clReleaseBuffer");
  err = clReleaseMemObject(bufferUseHostPtr);
  ASSERT_OCL_SUCCESS(err, "clReleaseBuffer");
  err = clReleaseMemObject(bufferUseHostPtr2);
  ASSERT_OCL_SUCCESS(err, "clReleaseBuffer");
}
