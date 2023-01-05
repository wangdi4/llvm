// INTEL CONFIDENTIAL
//
// Copyright 2022 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "CL/cl.h"
#include "CL/cl_gvp_ext.h"
#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "gtest/gtest.h"

extern cl_device_type gDeviceType;

class GVAccessExtentionTest : public ::testing::Test {
public:
  GVAccessExtentionTest() : m_context(nullptr), m_queue(nullptr) {}

protected:
  void SetUp() override {
    m_options = "-cl-std=CL2.0";

    cl_int err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");

    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    m_queue =
        clCreateCommandQueueWithProperties(m_context, m_device, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");

    m_clEnqueueReadGlobalVariableINTEL = (clEnqueueReadGlobalVariableINTEL_fn)
        clGetExtensionFunctionAddressForPlatform(
            m_platform, "clEnqueueReadGlobalVariableINTEL");
    ASSERT_NE(nullptr, m_clEnqueueReadGlobalVariableINTEL)
        << "clGetExtensionFunctionAddressForPlatform("
           "\"clEnqueueReadGlobalVariableINTEL\") failed.";

    m_clEnqueueWriteGlobalVariableINTEL = (clEnqueueWriteGlobalVariableINTEL_fn)
        clGetExtensionFunctionAddressForPlatform(
            m_platform, "clEnqueueWriteGlobalVariableINTEL");
    ASSERT_NE(nullptr, m_clEnqueueWriteGlobalVariableINTEL)
        << "clGetExtensionFunctionAddressForPlatform("
           "\"clEnqueueWriteGlobalVariableINTEL\") failed.";
  }

  void TearDown() override {
    cl_int err;
    if (m_queue) {
      err = clReleaseCommandQueue(m_queue);
      ASSERT_OCL_SUCCESS(err, "clReleaseCommandQueue");
    }
    if (m_context) {
      err = clReleaseContext(m_context);
      ASSERT_OCL_SUCCESS(err, "clReleaseContext");
    }
  }

protected:
  std::string m_options;
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;
  clEnqueueReadGlobalVariableINTEL_fn m_clEnqueueReadGlobalVariableINTEL;
  clEnqueueWriteGlobalVariableINTEL_fn m_clEnqueueWriteGlobalVariableINTEL;
};

TEST_F(GVAccessExtentionTest, readGlobalVariable) {
  const char *source = R"(
    global int gv = 123;
    kernel void test(global int* p) {
      *p = gv;
    }
  )";

  cl_int err;
  cl_program program =
      clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");

  err = clBuildProgram(program, 1, &m_device, m_options.c_str(), nullptr,
                       nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  // Read global variable "gv"
  int gv;
  err = m_clEnqueueReadGlobalVariableINTEL(
      m_queue, program, "gv", false, sizeof(int), 0, &gv, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadGlobalVariableINTEL");
  ASSERT_EQ(123, gv);
}

TEST_F(GVAccessExtentionTest, readAfterWriteGlobalVariable) {
  const char *source = R"(
    global int gv;
    kernel void test(global int* p) {
      *p = gv;
    }
  )";

  cl_int err;
  cl_program program =
      clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");

  err = clBuildProgram(program, 1, &m_device, m_options.c_str(), nullptr,
                       nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  // Write a new value to global variable "gv" and test if we write successfully
  const int gvToWrite = 234;
  err = m_clEnqueueWriteGlobalVariableINTEL(m_queue, program, "gv", false,
                                            sizeof(int), 0, &gvToWrite, 0,
                                            nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueWriteGlobalVariableINTEL");

  int gv;
  err = m_clEnqueueReadGlobalVariableINTEL(
      m_queue, program, "gv", false, sizeof(int), 0, &gv, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadGlobalVariableINTEL");
  ASSERT_EQ(234, gv);
}

TEST_F(GVAccessExtentionTest, readGlobalVariableArrary) {
  const char *source = R"(
    global int gvArr[4] = {1, 2, 3, 4};
    kernel void test(global int* p) {
      int gid = get_global_id(0);
      if (gid < 4)
        p[gid] = gvArr[gid];
    }
  )";

  cl_int err;
  cl_program program =
      clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");

  err = clBuildProgram(program, 1, &m_device, m_options.c_str(), nullptr,
                       nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  // Read global variable "gvArr"
  int gvArr[4];
  int gvArrRef[4] = {1, 2, 3, 4};
  err = m_clEnqueueReadGlobalVariableINTEL(m_queue, program, "gvArr", false,
                                           sizeof(gvArr), 0, gvArr, 0, nullptr,
                                           nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadGlobalVariableINTEL");
  ASSERT_EQ(0, memcmp(gvArr, gvArrRef, sizeof(gvArr)));

  // Read global variable "gvArr" with offset
  int gvArrRef2[4] = {3, 4, 0, 0};
  memset(gvArr, 0, sizeof(gvArr));
  err = m_clEnqueueReadGlobalVariableINTEL(m_queue, program, "gvArr", false,
                                           sizeof(gvArr) / 2, sizeof(gvArr) / 2,
                                           gvArr, 0, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadGlobalVariableINTEL");
  ASSERT_EQ(0, memcmp(gvArr, gvArrRef2, sizeof(gvArr)));

  // The region being read specified by (offset, size) should be
  // fully contained by the size of the global variable
  err = m_clEnqueueReadGlobalVariableINTEL(
      m_queue, program, "gvArr", false, sizeof(gvArr) / 2 + 1,
      sizeof(gvArr) / 2, gvArr, 0, nullptr, nullptr);
  ASSERT_EQ(CL_INVALID_VALUE, err);
}

TEST_F(GVAccessExtentionTest, readAfterWriteGlobalVariableArrary) {
  const char *source = R"(
    global int gvArr[4];
    kernel void test(global int* p) {
      int gid = get_global_id(0);
      if (gid < 4)
        p[gid] = gvArr[gid];
    }
  )";

  cl_int err;
  cl_program program =
      clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");

  err = clBuildProgram(program, 1, &m_device, m_options.c_str(), nullptr,
                       nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  // Write a new value to global variable "gvArr" and test if we write
  // successfully
  int gvArr[4];
  const int gvArrToWrite[4] = {6, 7, 8, 9};
  err = m_clEnqueueWriteGlobalVariableINTEL(m_queue, program, "gvArr", false,
                                            sizeof(gvArr), 0, gvArrToWrite, 0,
                                            nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueWriteGlobalVariableINTEL");

  err = m_clEnqueueReadGlobalVariableINTEL(m_queue, program, "gvArr", false,
                                           sizeof(gvArr), 0, gvArr, 0, nullptr,
                                           nullptr);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadGlobalVariableINTEL");
  ASSERT_EQ(0, memcmp(gvArr, gvArrToWrite, sizeof(gvArr)));

  // The region being written specified by (offset, size) should be
  // fully contained by the size of the global variable
  err = m_clEnqueueWriteGlobalVariableINTEL(
      m_queue, program, "gvArr", false, sizeof(gvArr) / 2 + 1,
      sizeof(gvArr) / 2, gvArrToWrite, 0, nullptr, nullptr);
  ASSERT_EQ(CL_INVALID_VALUE, err);
}

TEST_F(GVAccessExtentionTest, readAndWriteGlobalVariableCommandType) {
  const char *source = R"(
    global int gv = 123;
    kernel void test(global int* p) {
      *p = gv;
    }
  )";

  cl_int err;
  cl_program program =
      clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");

  err = clBuildProgram(program, 1, &m_device, m_options.c_str(), nullptr,
                       nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  // Read global variable "gv"
  int gv;
  cl_event event;
  cl_command_type cmdType;
  err = m_clEnqueueReadGlobalVariableINTEL(
      m_queue, program, "gv", false, sizeof(int), 0, &gv, 0, nullptr, &event);
  ASSERT_OCL_SUCCESS(err, "clEnqueueReadGlobalVariableINTEL");
  // Query event command type
  err = clGetEventInfo(event, CL_EVENT_COMMAND_TYPE, sizeof(cl_command_type),
                       &cmdType, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetEventInfo");
  ASSERT_EQ(CL_COMMAND_READ_GLOBAL_VARIABLE_INTEL, cmdType);

  // Write a new value to global variable "gv" and test if we write successfully
  const int gvToWrite = 234;
  err = m_clEnqueueWriteGlobalVariableINTEL(m_queue, program, "gv", false,
                                            sizeof(int), 0, &gvToWrite, 0,
                                            nullptr, &event);
  ASSERT_OCL_SUCCESS(err, "clEnqueueWriteGlobalVariableINTEL");
  // Query event command type
  err = clGetEventInfo(event, CL_EVENT_COMMAND_TYPE, sizeof(cl_command_type),
                       &cmdType, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetEventInfo");
  ASSERT_EQ(CL_COMMAND_WRITE_GLOBAL_VARIABLE_INTEL, cmdType);
}

TEST_F(GVAccessExtentionTest, readAfterWriteGlobalVariableWaitForEvents) {
  const char *source = R"(
    global int gv;
    kernel void test(global int* p) {
      *p = gv;
    }
  )";

  cl_int err;
  cl_program program =
      clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");

  err = clBuildProgram(program, 1, &m_device, m_options.c_str(), nullptr,
                       nullptr);
  ASSERT_OCL_SUCCESS(err, "clBuildProgram");

  for (int i = 0; i < 10; i++) {
    cl_event ew;
    cl_event er;
    // Write global variable "gv"
    err = m_clEnqueueWriteGlobalVariableINTEL(
        m_queue, program, "gv", false, sizeof(int), 0, &i, 0, nullptr, &ew);
    ASSERT_OCL_SUCCESS(err, "clEnqueueWriteGlobalVariableINTEL");

    int gv;
    // Read global variable "gv"
    err = m_clEnqueueReadGlobalVariableINTEL(m_queue, program, "gv", false,
                                             sizeof(int), 0, &gv, 1, &ew, &er);
    ASSERT_OCL_SUCCESS(err, "clEnqueueReadGlobalVariableINTEL");
    err = clWaitForEvents(1, &er);
    ASSERT_OCL_SUCCESS(err, "clWaitForEvents");
    ASSERT_EQ(i, gv);
    err = clWaitForEvents(1, &ew);
    ASSERT_OCL_SUCCESS(err, "clWaitForEvents");
    err = clWaitForEvents(1, &er);
    ASSERT_OCL_SUCCESS(err, "clWaitForEvents");
  }
}
