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
#include <vector>

extern cl_device_type gDeviceType;

class SetContextDestructorCallbackTest : public ::testing::Test {
protected:
  virtual void SetUp() override {}

  virtual void TearDown() override {}

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
};

struct UserDataType {
  cl_context expectedContext;
  std::vector<size_t> &vectorToModify;
  size_t valueToAdd;
};

static void CL_API_CALL callbackFoo(cl_context, void *) {}
static void CL_API_CALL callbackBar(cl_context context, void *userData) {
  auto pUserData = reinterpret_cast<UserDataType *>(userData);
  ASSERT_EQ(pUserData->expectedContext, context);
  pUserData->vectorToModify.push_back(pUserData->valueToAdd);
}

// cl_int clSetContextDestructorCallback(
//   cl_context context,
//   void (CL_CALLBACK* pfn_notify)(cl_context context, void* user_data),
//   void* user_data);
TEST_F(SetContextDestructorCallbackTest, kernel) {
  cl_int err = CL_SUCCESS;
  err = clGetPlatformIDs(1, &m_platform, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

  err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");
  cl_context context =
      clCreateContext(nullptr, 1, &m_device, nullptr, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateContext");

  // Return CL_INVALID_CONTEXT if context is not a valid context
  err = clSetContextDestructorCallback(nullptr, nullptr, nullptr);
  ASSERT_EQ(CL_INVALID_CONTEXT, err)
      << "clSetContextDestructorCallback with invalid context failed.";

  // Return CL_INVALID_VALUE if pfn_notify is NULL
  err = clSetContextDestructorCallback(context, nullptr, nullptr);
  ASSERT_EQ(CL_INVALID_VALUE, err)
      << "clSetContextDestructorCallback with invalid value failed.";

  // Only user_data is nullptr
  err = clSetContextDestructorCallback(context, callbackFoo, nullptr);
  ASSERT_OCL_SUCCESS(err, "clSetContextDestructorCallback");

  // All parameters are not nullptr
  auto userData = reinterpret_cast<void *>(0x4321);
  err = clSetContextDestructorCallback(context, callbackFoo, userData);
  ASSERT_OCL_SUCCESS(err, "clSetContextDestructorCallback");

  std::vector<size_t> callbacksReturnValues;
  // bind callbacksReturnValues to vectorToModify field of each UserDataType
  // object
  UserDataType userDataArray[] = {{context, callbacksReturnValues, 1},
                                  {context, callbacksReturnValues, 2},
                                  {context, callbacksReturnValues, 3}};

  for (auto &userData : userDataArray) {
    err = clSetContextDestructorCallback(context, callbackBar, &userData);
    ASSERT_OCL_SUCCESS(err, "clSetContextDestructorCallback");
  }

  // Release resources
  err = clReleaseContext(context);
  ASSERT_OCL_SUCCESS(err, "clReleaseContext");

  ASSERT_EQ(3, callbacksReturnValues.size());
  EXPECT_EQ(3, callbacksReturnValues[0]);
  EXPECT_EQ(2, callbacksReturnValues[1]);
  EXPECT_EQ(1, callbacksReturnValues[2]);
}
