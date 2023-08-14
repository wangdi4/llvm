// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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

#if defined _M_X64 || defined __x86_64__

#include "CL/cl.h"
#include "FrameworkTest.h"
#include "common_utils.h"
#include "gtest_wrapper.h"
#include <fstream>
#include <string>

extern cl_device_type gDeviceType;

TEST(FrameworkTestType, Test_clGetDeviceFunctionPointerINTEL) {
  cl_int error = CL_SUCCESS;

  cl_platform_id platform = nullptr;
  error = clGetPlatformIDs(1, &platform, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << "clGetPlatformIDs failed";

  cl_device_id device = nullptr;
  error = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
  ASSERT_EQ(CL_SUCCESS, error) << "clGetDeviceIDs failed";

  cl_context context =
      clCreateContext(nullptr, 1, &device, nullptr, nullptr, &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateContext failed";

  // This spv file is generated from
  //   sycl/test/function-pointers/fp-as-kernel-arg.cpp
  // using following command:
  //   clang++ -fsycl-device-only -fno-sycl-use-bitcode
  //           -Xclang -fsycl-allow-func-ptr -std=c++14
  //           fp-as-kernel-arg.cpp -o fp-as-kernel-arg.spv
  std::string fileName = get_exe_dir() + "fp-as-kernel-arg.spv";
  const char *funcName = "add";

  std::vector<unsigned char> spvBinary;
  ASSERT_NO_FATAL_FAILURE(readBinary(fileName, spvBinary));

  cl_program program = clCreateProgramWithIL(context, spvBinary.data(),
                                             spvBinary.size(), &error);
  ASSERT_EQ(CL_SUCCESS, error) << "clCreateProgramWithIL failed";

  typedef cl_int(CL_API_CALL * fp_t)(cl_device_id, cl_program, const char *,
                                     cl_ulong *);

  fp_t clGetDeviceFunctionPointerINTELPtr = nullptr;

  clGetDeviceFunctionPointerINTELPtr =
      (fp_t)clGetExtensionFunctionAddress("clGetDeviceFunctionPointerINTEL");

  ASSERT_NE(nullptr, clGetDeviceFunctionPointerINTELPtr)
      << "clGetExtensionFunctionAddress(\"clGetDeviceFunctionPointerINTEL\" "
         "failed. ";

  cl_ulong fp = 0;
  error = clGetDeviceFunctionPointerINTELPtr(device, program, funcName, &fp);
  ASSERT_EQ(CL_INVALID_PROGRAM_EXECUTABLE, error)
      << "clGetDeviceFunctionPointerINTEL must return "
         "CL_INVALID_PROGRAM_EXECUTABLE if program wasn't build";

  error = clGetDeviceFunctionPointerINTELPtr(device, nullptr, funcName, &fp);
  ASSERT_EQ(CL_INVALID_PROGRAM, error)
      << "clGetDeviceFunctionPointerINTEL must return CL_INVALID_PROGRAM if "
         "program is not a valid OpenCL program";

  error = clBuildProgram(program, 0, nullptr, "", nullptr, nullptr);
  EXPECT_EQ(CL_SUCCESS, error) << "clBuildProgram failed";

  if (CL_SUCCESS != error) {
    size_t log_size = 0;
    error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0,
                                  nullptr, &log_size);
    ASSERT_EQ(CL_SUCCESS, error) << "clGetProgramBuildInfo failed";

    std::string log("", log_size);
    error = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                                  log_size, &log[0], nullptr);
    ASSERT_EQ(CL_SUCCESS, error) << "clGetProgramBuildInfo failed";
    FAIL() << log << "\n";
  }

  error =
      clGetDeviceFunctionPointerINTELPtr(device, program, funcName, nullptr);
  ASSERT_EQ(CL_INVALID_VALUE, error)
      << "clGetDeviceFunctionPointerINTEL "
         "must return CL_INVALID_VALUE if function_pointer_ret is nullptr";

  error = clGetDeviceFunctionPointerINTELPtr(device, program, nullptr, &fp);
  ASSERT_EQ(CL_INVALID_VALUE, error)
      << "clGetDeviceFunctionPointerINTEL "
         "must return CL_INVALID_VALUE if function_name is nullptr";

  // TODO: check for CL_INVALID_DEVICE

  error = clGetDeviceFunctionPointerINTELPtr(device, program, funcName, &fp);
  ASSERT_EQ(CL_SUCCESS, error) << "clGetDeviceFunctionPointerINTEL failed";
  ASSERT_NE(0u, fp) << "clGetDeviceFunctionPointerINTEL must return a non-zero "
                       "value via function_pointer_ret on success";
}

#endif
