// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
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

#include "SGEmulationTest.h"
#include "common_utils.h"

cl_device_type gDeviceType = CL_DEVICE_TYPE_CPU;

void SGEmulationTest::SetUp() {
  if (!SETENV("CL_CONFIG_CPU_VECTORIZER_TYPE", "vpo"))
    FAIL() << "Failed to set vectorizer type as vpo";
  CL_base::SetUp();
  ASSERT_LE(OPENCL_VERSION::OPENCL_VERSION_2_1, m_version)
      << "Test required OpenCL2.1 version at least";
}

void SGEmulationTest::TearDown() {
  if (!UNSETENV("CL_CONFIG_CPU_VECTORIZER_TYPE"))
    FAIL() << "Failed to unset CL_CONFIG_CPU_VECTORIZER_TYPE";
  CL_base::TearDown();
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
