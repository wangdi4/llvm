//==--- CPUSignaturesDef.cpp - CPU signature defines test   -*- C++ -*---==//
////
//// Copyright (C) 2015 Intel Corporation. All rights reserved.
////
//// The information and source code contained herein is the exclusive property
//// of Intel Corporation and may not be disclosed, examined or reproduced in
//// whole or in part without explicit written authorization from the company.
////
//// ===--------------------------------------------------------------------===
/////
#include "CL.h"
#include "cl_cpu_detect.h"
#include <iostream>
#include <string>

using namespace Intel::OpenCL::Utils;

TEST_F(CL, CPUSignatureDefine) {
  cl_int iRet = CL_SUCCESS;

  const auto CPUID = CPUDetect::GetInstance();

  ASSERT_STRNE("UNKNOWN", CPUID->GetCPUArchShortName().c_str())
      << "CPU is not detected.";

  std::string kernel("");
  for (const auto &arch : CPUID->CPUArchStr) {
    const std::string defName = "__INTEL_OPENCL_CPU_" + arch + "__";
    if (arch == CPUID->GetCPUArchShortName())
      kernel += "#ifndef " + defName + "\n" + "    #error " + defName +
                " is expected to be defined!\n" + "#endif\n";
    else
      kernel += "#ifdef " + defName + "\n" + "    #error " + defName +
                " is not expected to be defined!\n" + "#endif\n";
  }

  const char *cKernel = kernel.c_str();

  cl_program program = clCreateProgramWithSource(
      m_context, 1, &cKernel, /*kernel_length=*/nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clCreateProgramWithSource failed. ";

  iRet = clBuildProgram(program, /*num_devices=*/0, /*device_list=*/nullptr,
                        /*options=*/"", /*pfn_notify=*/nullptr,
                        /*user_data=*/nullptr);
  if (CL_SUCCESS != iRet) {
    std::string log("\0", 1000);
    clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG, log.size(),
                          &log[0], nullptr);
    std::cout << log << std::endl;
  }
  ASSERT_EQ(CL_SUCCESS, iRet) << " clBuildProgram failed. ";
}
