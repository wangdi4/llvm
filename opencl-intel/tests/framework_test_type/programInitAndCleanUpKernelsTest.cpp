// Copyright 2021 Intel Corporation.
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
#include "FrameworkTest.h"
#include "TestsHelpClasses.h"
#include "common_utils.h"

extern cl_device_type gDeviceType;

class ProgramInitAndCleanUpKernelsTest : public ::testing::Test {
protected:
  virtual void SetUp() override {
    cl_int err = CL_SUCCESS;
    err = clGetPlatformIDs(1, &m_platform, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");
    m_context = clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateContext");

    // Create program
    const char *source = "kernel void test(global int* dst) {\n"
                         "  dst[get_global_id(0)] = 1;\n"
                         "}";
    m_program = clCreateProgramWithSource(m_context, 1, &source, nullptr, &err);
    ASSERT_OCL_SUCCESS(err, "clCreateProgramWithSource");
  }

  virtual void TearDown() override {
    // Release resources
    cl_int err = CL_SUCCESS;
    err = clReleaseProgram(m_program);
    ASSERT_OCL_SUCCESS(err, "clReleaseProgram");
    err = clReleaseContext(m_context);
    ASSERT_OCL_SUCCESS(err, "clReleaseContext");
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_program m_program;
};

static void CL_API_CALL callbackFoo(cl_context, void *) {}

// Return CL_INVALID_OPERATION, clean-up kernels is not supported in OpenCL 3.0
TEST_F(ProgramInitAndCleanUpKernelsTest, clSetContextDestructorCallbackTest) {
  cl_int err = CL_SUCCESS;
  err = clSetContextDestructorCallback(nullptr, nullptr, nullptr);
  ASSERT_EQ(CL_INVALID_CONTEXT, err)
      << "clSetContextDestructorCallback with invalid context is failed.";

  err = clSetContextDestructorCallback(m_context, nullptr, nullptr);
  ASSERT_EQ(CL_INVALID_VALUE, err)
      << "clSetContextDestructorCallback with invalid callback is failed.";

  auto userData = reinterpret_cast<void *>(0x4321);
  err = clSetContextDestructorCallback(m_context, callbackFoo, userData);
  ASSERT_EQ(CL_SUCCESS, err) << "clSetContextDestructorCallback is passed.";
}

// Returns CL_FALSE, Program initialization is not supported in OpenCL 3.0
TEST_F(ProgramInitAndCleanUpKernelsTest, cleanUpKernelsTest) {
  cl_int err = CL_SUCCESS;
  cl_bool paramRet = 0;
  cl_bool expectedParam = CL_FALSE;
  size_t paramSizeRet = 0;
  err = clGetProgramInfo(m_program, CL_PROGRAM_SCOPE_GLOBAL_CTORS_PRESENT,
                         sizeof(cl_bool), &paramRet, &paramSizeRet);
  ASSERT_EQ(CL_SUCCESS, err)
      << " clGetProgramInfo with CL_PROGRAM_SCOPE_GLOBAL_CTORS_PRESENT "
         "failed. ";
  ASSERT_EQ(sizeof(cl_bool), paramSizeRet)
      << " clGetProgramInfo with CL_PROGRAM_SCOPE_GLOBAL_CTORS_PRESENT - query "
         "size failed. ";
  ASSERT_EQ(expectedParam, paramRet)
      << " clGetProgramInfo with CL_PROGRAM_SCOPE_GLOBAL_CTORS_PRESENT failed. "
         "param_value changed. ";
  err = clGetProgramInfo(m_program, CL_PROGRAM_SCOPE_GLOBAL_DTORS_PRESENT,
                         sizeof(cl_bool), &paramRet, &paramSizeRet);
  ASSERT_EQ(CL_SUCCESS, err)
      << " clGetProgramInfo with CL_PROGRAM_SCOPE_GLOBAL_DTORS_PRESENT "
         "failed. ";
  ASSERT_EQ(sizeof(cl_bool), paramSizeRet)
      << " clGetProgramInfo with CL_PROGRAM_SCOPE_GLOBAL_DTORS_PRESENT - query "
         "size failed. ";
  ASSERT_EQ(expectedParam, paramRet)
      << " clGetProgramInfo with CL_PROGRAM_SCOPE_GLOBAL_DTORS_PRESENT failed. "
         "param_value changed. ";
}
