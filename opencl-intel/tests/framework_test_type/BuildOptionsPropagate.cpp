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

// This test checks that even when kernel doesn't contain
// opencl.compiler.options metadata "-g" and "-cl-opt-disable" options provided
// to clCompileProgram are propagated to the device compiler.
TEST_F(CL21, BuildOptionsPropagate) {
  cl_int iRet = CL_SUCCESS;
  cl_program program = nullptr;

  std::vector<char> spirv;
  ASSERT_NO_FATAL_FAILURE(GetSimpleSPIRV(spirv));

  program = clCreateProgramWithIL(m_context, spirv.data(), spirv.size(), &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithIL failed. ";

  iRet = clCompileProgram(program, 1, &m_device, "-cl-opt-disable -g", 0,
                          nullptr, nullptr, nullptr, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCompileProgram failed.";

  cl_program programLinked = clLinkProgram(m_context, 1, &m_device, nullptr, 1,
                                           &program, nullptr, nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clLinkProgram failed.";

  size_t LogSize = 0;
  iRet = clGetProgramBuildInfo(programLinked, m_device, CL_PROGRAM_BUILD_LOG, 0,
                               NULL, &LogSize);
  ASSERT_EQ(CL_SUCCESS, iRet) << " Device failed to return log size.";

  char *Log = (char *)malloc(LogSize);
  iRet = clGetProgramBuildInfo(programLinked, m_device, CL_PROGRAM_BUILD_LOG,
                               LogSize, Log, NULL);
  ASSERT_EQ(CL_SUCCESS, iRet)
      << " clGetProgramBuildInfo CL_PROGRAM_BUILD_LOG failed";

  ASSERT_TRUE(std::string(Log).find(
                  "Options used by backend compiler:  -cl-opt-disable -g") !=
              std::string::npos)
      << "Haven't find the message that -g and -cl-opt-disable were passed to "
         "backend";

  free(Log);
  clReleaseProgram(program);
  clReleaseProgram(programLinked);
}
