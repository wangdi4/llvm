//===--- test_pipes.cpp -                                       -*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for atomics on FPGA
//
// ===--------------------------------------------------------------------=== //

#include "CL/cl_ext.h"
#include "common_utils.h"
#include "gtest_wrapper.h"
#include "simple_fixture.h"
#include <fstream>
#include <string>

class TestAtomcis : public OCLFPGASimpleFixture {};

TEST_F(TestAtomcis, No64BitAtomicsSupport) {
  std::string filename = get_exe_dir() + "atomics64.spv";
  std::fstream spirv_file(filename, std::fstream::in | std::fstream::binary |
                                        std::fstream::ate);
  std::vector<char> spirv;
  ASSERT_TRUE(spirv_file) << " Error while opening " << filename << " file. ";

  size_t length = spirv_file.tellg();
  spirv_file.seekg(0, spirv_file.beg);

  spirv.resize(length, 0);
  spirv_file.read(&spirv[0], length);
  ASSERT_TRUE(spirv_file) << "Error in reading " << filename;
  cl_int iRet;
  clCreateProgramWithILKHR_fn CreateProgramWithIL =
      reinterpret_cast<clCreateProgramWithILKHR_fn>(
          clGetExtensionFunctionAddressForPlatform(platform(),
                                                   "clCreateProgramWithILKHR"));
  ASSERT_NE(nullptr, CreateProgramWithIL)
      << "clGetExtensionFunctionAddressForPlatform for "
         "clCreateProgramWithILKHR returned nullptr.";
  cl_device_id deviceId = device();
  cl_program program =
      CreateProgramWithIL(getContext(), spirv.data(), spirv.size(), &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateProgramWithILKHR with SPIRV failed ";

  iRet = clBuildProgram(program, 1, &deviceId, nullptr, nullptr, nullptr);

  ASSERT_EQ(CL_BUILD_PROGRAM_FAILURE, iRet)
      << "clBuildProgram should fail but succeeded. Input contains 64bit "
         "atomics"
         " which are not supported on FPGA emulator.";

  std::string log("", 1000);
  clGetProgramBuildInfo(program, deviceId, CL_PROGRAM_BUILD_LOG, log.size(),
                        &log[0], nullptr);

  ASSERT_TRUE(
      log.find("int 64bit atomics are not supported on FPGA emulator") !=
      std::string::npos);
}
