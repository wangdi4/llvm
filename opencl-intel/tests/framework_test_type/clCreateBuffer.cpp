// Copyright (C) 2012-2022 Intel Corporation. All rights reserved.
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

class CreateBufferTest : public ::testing::Test {
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
};

TEST_F(CreateBufferTest, basic) {
  printf("=============================================================\n");
  printf("clCreateBufferTest\n");
  printf("=============================================================\n");
  cl_int iRet = 0;

  printf("m_context = %p\n", (void *)m_context);

  cl_mem buffer1 =
      clCreateBuffer(m_context, CL_MEM_READ_ONLY, 100, NULL, &iRet);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_READ_ONLY) should be OK.";
  printf("buffer1 = %p\n", (void *)buffer1);

  cl_mem buffer2 =
      clCreateBuffer(m_context, CL_MEM_WRITE_ONLY, 100, NULL, &iRet);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_WRITE_ONLY) should be OK.";
  printf("buffer2 = %p\n", (void *)buffer2);

  cl_mem buffer3 = clCreateBuffer(
      m_context, CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY, 100, NULL, &iRet);
  EXPECT_EQ(oclErr(CL_INVALID_VALUE), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY) "
         "should fail.";

  /*
   * Start testing host access permissions.
   */

  // Illegal flag combinations.
  cl_mem bufferForErr;

  bufferForErr =
      clCreateBuffer(m_context, CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY,
                     100, NULL, &iRet);
  EXPECT_EQ(oclErr(CL_INVALID_VALUE), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_NO_ACCESS | "
         "CL_MEM_HOST_READ_ONLY) should fail.";
  EXPECT_EQ(bufferForErr, nullptr);

  bufferForErr =
      clCreateBuffer(m_context, CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_WRITE_ONLY,
                     100, NULL, &iRet);
  EXPECT_EQ(oclErr(CL_INVALID_VALUE), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_NO_ACCESS | "
         "CL_MEM_HOST_WRITE_ONLY) should fail.";
  EXPECT_EQ(bufferForErr, nullptr);

  bufferForErr =
      clCreateBuffer(m_context, CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY,
                     100, NULL, &iRet);
  EXPECT_EQ(oclErr(CL_INVALID_VALUE), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_READ_ONLY | "
         "CL_MEM_HOST_WRITE_ONLY) should fail.";
  EXPECT_EQ(bufferForErr, nullptr);

  cl_mem bufferWithNoHostAccess =
      clCreateBuffer(m_context, CL_MEM_HOST_NO_ACCESS, 100, NULL, &iRet);

  // Release all
  clReleaseMemObject(bufferForErr);
  clReleaseMemObject(bufferWithNoHostAccess);
  clReleaseMemObject(buffer3);
  clReleaseMemObject(buffer2);
  clReleaseMemObject(buffer1);
}

TEST_F(CreateBufferTest, clCreateBufferWithPropertiesINTEL) {
  printf("=============================================================\n");
  printf("clCreateBufferWithPropertiesINTELTest\n");
  printf("=============================================================\n");

  cl_int iRet = 0;

  // valid cl_mem_properties_intel: CL_MEM_CHANNEL_INTEL is the only supported
  // type.
  cl_mem_properties_intel valid_properties[] = {CL_MEM_CHANNEL_INTEL,
                                                0x1234 /*dummy value*/, 0};
  cl_mem buffer1 = clCreateBufferWithPropertiesINTEL(
      m_context, valid_properties, CL_MEM_READ_ONLY, 100, NULL, &iRet);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
      << "clCreateBufferWithPropertiesINTEL with properties"
         "(CL_MEM_CHANNEL_INTEL) should be OK.";

  // invalid cl_mem_properties_intel value:
  cl_mem_properties_intel invalid_properties[] = {CL_MEM_ALLOC_FLAGS_INTEL,
                                                  0x1234 /*dummy value*/, 0};
  cl_mem buffer2 = clCreateBufferWithPropertiesINTEL(
      m_context, invalid_properties, CL_MEM_WRITE_ONLY, 100, NULL, &iRet);
  EXPECT_EQ(oclErr(CL_INVALID_PROPERTY), oclErr(iRet))
      << "clCreateBufferWithPropertiesINTEL with invalid properties"
         "(CL_MEM_ALLOC_FLAGS_INTEL) should fail.";
  printf("buffer2 = %p\n", (void *)buffer2);
  // Release all
  clReleaseMemObject(buffer1);
  clReleaseMemObject(buffer2);
}

TEST_F(CreateBufferTest, clCreateBufferWithProperties) {
  printf("=============================================================\n");
  printf("clCreateBufferWithPropertiesTest\n");
  printf("=============================================================\n");

  cl_int iRet = 0;

  // OpenCL 3.0 does not define any optional properties for buffers.
  cl_mem_properties properties[] = {0};
  cl_mem buffer1 = clCreateBufferWithProperties(
      m_context, properties, CL_MEM_READ_ONLY, 100, NULL, &iRet);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
      << "clCreateBufferWithProperties with properties 0 should be OK.";
  size_t szSize;
  iRet = clGetMemObjectInfo(buffer1, CL_MEM_PROPERTIES, 0, nullptr, &szSize);
  EXPECT_EQ(sizeof(cl_mem_properties), szSize)
      << "clGetMemObjectInfo queried size is unexpected.";

  // Release all
  clReleaseMemObject(buffer1);
}

TEST_F(CreateBufferTest, clCreateSubBuffer) {
  printf("=============================================================\n");
  printf("clCreateSubBufferTest\n");
  printf("=============================================================\n");
  cl_int iRet = 0;

  cl_buffer_region region;
  region.origin = (1 * 128);
  region.size = (2 * 128);

  cl_mem subBufferForTesting;

  /* sub buffers for CL_MEM_HOST_NO_ACCESS */
  cl_mem bufferNoAccess =
      clCreateBuffer(m_context, CL_MEM_HOST_NO_ACCESS, (4 * 128), NULL, &iRet);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_NO_ACCESS) should be OK.";

  subBufferForTesting =
      clCreateSubBuffer(bufferNoAccess, CL_MEM_HOST_READ_ONLY,
                        CL_BUFFER_CREATE_TYPE_REGION, (void *)&region, &iRet);
  EXPECT_EQ(oclErr(CL_INVALID_VALUE), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_READ_ONLY) for parent with "
         "(CL_MEM_HOST_NO_ACCESS) should fail.";

  subBufferForTesting =
      clCreateSubBuffer(bufferNoAccess, CL_MEM_HOST_WRITE_ONLY,
                        CL_BUFFER_CREATE_TYPE_REGION, (void *)&region, &iRet);
  EXPECT_EQ(oclErr(CL_INVALID_VALUE), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_WRITE_ONLY) for parent with "
         "(CL_MEM_HOST_NO_ACCESS) should fail.";

  subBufferForTesting =
      clCreateSubBuffer(bufferNoAccess, CL_MEM_HOST_NO_ACCESS,
                        CL_BUFFER_CREATE_TYPE_REGION, (void *)&region, &iRet);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_NO_ACCESS) for parent with "
         "(CL_MEM_HOST_NO_ACCESS) should be fine.";
  clReleaseMemObject(bufferNoAccess);
  clReleaseMemObject(subBufferForTesting);

  /* sub buffers for CL_MEM_HOST_READ_ONLY */
  cl_mem bufferReadOnlyAccess =
      clCreateBuffer(m_context, CL_MEM_HOST_READ_ONLY, (4 * 128), NULL, &iRet);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_READ_ONLY) should be OK.";

  subBufferForTesting =
      clCreateSubBuffer(bufferReadOnlyAccess, CL_MEM_HOST_WRITE_ONLY,
                        CL_BUFFER_CREATE_TYPE_REGION, (void *)&region, &iRet);
  EXPECT_EQ(oclErr(CL_INVALID_VALUE), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_WRITE_ONLY) for parent with "
         "(CL_MEM_HOST_READ_ONLY) should fail.";

  subBufferForTesting =
      clCreateSubBuffer(bufferReadOnlyAccess, CL_MEM_HOST_READ_ONLY,
                        CL_BUFFER_CREATE_TYPE_REGION, (void *)&region, &iRet);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_READ_ONLY) for parent with "
         "(CL_MEM_HOST_READ_ONLY) should be fine.";
  clReleaseMemObject(subBufferForTesting);

  subBufferForTesting =
      clCreateSubBuffer(bufferReadOnlyAccess, CL_MEM_HOST_NO_ACCESS,
                        CL_BUFFER_CREATE_TYPE_REGION, (void *)&region, &iRet);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_NO_ACCESS) for parent with "
         "(CL_MEM_HOST_READ_ONLY) should be fine.";
  clReleaseMemObject(bufferReadOnlyAccess);
  clReleaseMemObject(subBufferForTesting);

  /* sub buffers for CL_MEM_HOST_WRITE_ONLY */
  cl_mem bufferWriteOnlyAccess =
      clCreateBuffer(m_context, CL_MEM_HOST_WRITE_ONLY, (4 * 128), NULL, &iRet);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_WRITE_ONLY) should be OK.";

  subBufferForTesting =
      clCreateSubBuffer(bufferWriteOnlyAccess, CL_MEM_HOST_READ_ONLY,
                        CL_BUFFER_CREATE_TYPE_REGION, (void *)&region, &iRet);
  EXPECT_EQ(oclErr(CL_INVALID_VALUE), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_READ_ONLY) for parent with "
         "(CL_MEM_HOST_WRITE_ONLY) should fail.";

  subBufferForTesting =
      clCreateSubBuffer(bufferWriteOnlyAccess, CL_MEM_HOST_WRITE_ONLY,
                        CL_BUFFER_CREATE_TYPE_REGION, (void *)&region, &iRet);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_WRITE_ONLY) for parent with "
         "(CL_MEM_HOST_WRITE_ONLY) should be fine.";
  clReleaseMemObject(subBufferForTesting);

  subBufferForTesting =
      clCreateSubBuffer(bufferWriteOnlyAccess, CL_MEM_HOST_NO_ACCESS,
                        CL_BUFFER_CREATE_TYPE_REGION, (void *)&region, &iRet);
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_NO_ACCESS) for parent with "
         "(CL_MEM_HOST_WRITE_ONLY) should be fine.";
  clReleaseMemObject(bufferWriteOnlyAccess);
  clReleaseMemObject(subBufferForTesting);
}

// When CL_MEM_USE_HOST_PTR flag is present, data pointer of a middle-sized
// buffer should be always aligned to DEV_MAXIMUM_ALIGN even if host ptr isn't
// aligned to DEV_MAXIMUM_ALIGN.
TEST_F(CreateBufferTest, useHostPtrNotAligned) {
  size_t num = 1024 * 1024;
  std::vector<int> v(num);
  size_t bufferSize = v.size() * sizeof(v[0]);
  cl_int err;
  cl_mem buffer =
      clCreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                     bufferSize, &v[0], &err);
  void *bufferBaseAddr;
  // clGetMemObjectInfo with CL_MEM_ALLOC_BASE_PTR_INTEL flag is an internal
  // implementation, which is for testing purpose only.
  err = clGetMemObjectInfo(buffer, CL_MEM_ALLOC_BASE_PTR_INTEL,
                           sizeof(bufferBaseAddr), &bufferBaseAddr, nullptr);

  ASSERT_EQ(
      reinterpret_cast<uintptr_t>(bufferBaseAddr) & (DEV_MAXIMUM_ALIGN - 1), 0);

  err = clReleaseMemObject(buffer);
  ASSERT_OCL_SUCCESS(err, "clReleaseBuffer");
}

// Check that content pointed to by host ptr isn't cached if buffer uses host
// ptr but isn't read only. The content should reflect kernel execution results.
TEST_F(CreateBufferTest, useHostPtrNotAlignedWrite) {
  size_t num = 1024 * 1024;
  std::vector<int> v(num, 0);
  size_t bufferSize = v.size() * sizeof(v[0]);
  cl_int err;
  cl_mem buffer =
      clCreateBuffer(m_context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                     bufferSize, &v[0], &err);

  const char *source =
      R"(
      kernel void test(global int *buf) {
        buf[get_global_id(0)] = 1;
      }
  )";
  m_program = clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
  err = clBuildProgram(m_program, 1, &m_device, "-cl-std=CL2.0", nullptr,
                       nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");
  m_kernel = clCreateKernel(m_program, "test", &err);
  ASSERT_OCL_SUCCESS(err, "clCreateKernel");
  err = clSetKernelArg(m_kernel, 0, sizeof(cl_mem), &buffer);
  ASSERT_OCL_SUCCESS(err, "clSetKernelArg");
  err = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, nullptr, &num, nullptr, 0,
                               nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueNDRangeKernel");
  err = clFinish(m_queue);
  ASSERT_OCL_SUCCESS(err, "clFinish");

  ASSERT_EQ(v[0], 1);

  err = clReleaseMemObject(buffer);
  ASSERT_OCL_SUCCESS(err, "clReleaseBuffer");
}
