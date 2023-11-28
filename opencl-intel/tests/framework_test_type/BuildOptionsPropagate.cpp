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

#include "CL21.h"
#include "TestsHelpClasses.h"

// This test checks that only option of the last clLinkProgram is returned.
TEST_F(CL21, BuildOptionsPropagate) {
  cl_int err = CL_SUCCESS;
  cl_program program = nullptr;

  std::vector<char> spirv;
  ASSERT_NO_FATAL_FAILURE(GetSimpleSPIRV(spirv));

  program = clCreateProgramWithIL(m_context, spirv.data(), spirv.size(), &err);
  ASSERT_OCL_SUCCESS(err, "clCreateProgramWithIL");

  err = clCompileProgram(program, 1, &m_device, "-cl-opt-disable -g", 0,
                         nullptr, nullptr, nullptr, nullptr);
  ASSERT_OCL_SUCCESS(err, "clCompileProgram");

  cl_program programLinked = clLinkProgram(m_context, 1, &m_device, nullptr, 1,
                                           &program, nullptr, nullptr, &err);
  ASSERT_OCL_SUCCESS(err, "clLinkProgram");

  size_t buildOptionSize = 0;
  err = clGetProgramBuildInfo(programLinked, m_device, CL_PROGRAM_BUILD_OPTIONS,
                              0, NULL, &buildOptionSize);
  ASSERT_OCL_SUCCESS(err, "clGetProgramBuildInfo CL_PROGRAM_BUILD_OPTIONS");

  ASSERT_EQ(buildOptionSize, 1u);

  err = clReleaseProgram(program);
  ASSERT_OCL_SUCCESS(err, "clReleaseProgram program");
  err = clReleaseProgram(programLinked);
  ASSERT_OCL_SUCCESS(err, "clReleaseProgram programLinked");
}
