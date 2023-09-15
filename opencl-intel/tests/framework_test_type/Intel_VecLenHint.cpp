//==--- intelVecLenHint.cpp - intel_vec_len_hint attribute test -*- C++
//-*---==//
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
#include "FrameworkTest.h"
#include "cl_cpu_detect.h"
#include "cl_device_api.h"
#include "cl_types.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#define KERNEL(lenght)                                                         \
  vectorizationTo = #lenght;                                                   \
  kernelName = std::string("kernel_" + std::string(#lenght));                  \
  kernelCode =                                                                 \
      "#pragma OPENCL EXTENSION cl_intel_vec_len_hint: enable\n"               \
      "__attribute__((intel_vec_len_hint(" +                                   \
      std::string(#lenght) +                                                   \
      ")))\n"                                                                  \
      "__kernel void kernel_" +                                                \
      std::string(#lenght) +                                                   \
      "(__global int* in,\n"                                                   \
      "                                                  __global int* out)\n" \
      "{\n"                                                                    \
      "    size_t gid = get_global_id(0);\n"                                   \
      "    out[gid] = in[gid];\n"                                              \
      "}\n";

#define KERNEL_TYPE_HINT(lenght, type_hint)                                    \
  vectorizationTo = #lenght;                                                   \
  kernelName = std::string("kernel_" + std::string(#lenght) + "_vec_hint_" +   \
                           std::string(#type_hint));                           \
  kernelCode = "#pragma OPENCL EXTENSION cl_intel_vec_len_hint: enable\n"      \
               "__attribute__((intel_vec_len_hint(" +                          \
               std::string(#lenght) +                                          \
               ")))\n"                                                         \
               "__attribute__((vec_type_hint(" +                               \
               std::string(#type_hint) +                                       \
               ")))\n"                                                         \
               "__kernel void kernel_" +                                       \
               std::string(#lenght) + "_vec_hint_" + std::string(#type_hint) + \
               "(__global int* in, \n"                                         \
               "                                     __global int* out)\n"     \
               "{ \n"                                                          \
               "    size_t gid = get_global_id(0);\n"                          \
               "    out[gid] = in[gid];\n"                                     \
               "} \n";

extern cl_device_type gDeviceType;
using namespace Intel::OpenCL::Utils;

struct SupportForKernelVector {
  std::string kernelCode;
  std::string kernelName;
  std::string vectorizationTo;
  bool expectedRes;

  SupportForKernelVector(std::string Code, std::string Name,
                         std::string vectorization, bool Res)
      : kernelCode(std::move(Code)), kernelName(std::move(Name)),
        vectorizationTo(std::move(vectorization)), expectedRes(Res) {}
};

void intelVecTypeHintTest() {
  std::string vectorizedString = "was successfully vectorized";
  std::string notVectorizedString = "was not vectorized";
  std::string expectedError = "invalid attribute value";
  std::string warningString = "is not supported by the architecture. Fall back "
                              "to autovectorization mode";

  cl_device_id device;
  cl_context context;
  cl_program clProg;
  cl_platform_id platform = 0;

  cl_int iRet = clGetPlatformIDs(1, &platform, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetPlatformIDs = " << ClErrTxt(iRet);

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  // get device(s)
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << "clGetDeviceIDs = " << ClErrTxt(iRet);

  // create context
  context = clCreateContext(prop, 1, &device, nullptr, nullptr, &iRet);
  ASSERT_EQ(CL_SUCCESS, iRet) << "clCreateContext = " << ClErrTxt(iRet);

  // if we can't find a proper CPU - nothing to do here
  const auto CPUID = CPUDetect::GetInstance();
  ASSERT_TRUE(std::find(CPUID->CPUArchStr.begin(), CPUID->CPUArchStr.end(),
                        CPUID->GetCPUArchShortName()) !=
              CPUID->CPUArchStr.end())
      << "Failed to get CPU macro. Test failed.\n";

  std::vector<SupportForKernelVector> kernelVec;
  std::string kernelName = "";
  std::string kernelCode = "";
  std::string vectorizationTo = "";

  // expected to be vectorized
  KERNEL(0);
  kernelVec.emplace_back(kernelCode, kernelName, vectorizationTo, true);
  // expected not to be vectorized
  KERNEL(1);
  kernelVec.emplace_back(kernelCode, kernelName, vectorizationTo, false);
  // expected to fail during a compilation
  KERNEL(7);
  kernelVec.emplace_back(kernelCode, kernelName, vectorizationTo, false);
  // expected to be vectorized
  KERNEL(4);
  kernelVec.emplace_back(kernelCode, kernelName, vectorizationTo, true);
  // expected to be vectorized to 4 ignoring type_hint int
  KERNEL_TYPE_HINT(4, int);
  kernelVec.emplace_back(kernelCode, kernelName, vectorizationTo, true);
  // expected not to be vectorized
  KERNEL_TYPE_HINT(1, float4);
  kernelVec.emplace_back(kernelCode, kernelName, vectorizationTo, false);

  for (size_t i = 0; i != kernelVec.size(); ++i) {
    const char *cKernel = kernelVec[i].kernelCode.c_str();
    clProg = clCreateProgramWithSource(context, 1, &cKernel, nullptr, &iRet);
    EXPECT_EQ(CL_SUCCESS, iRet) << "clCreateProgramWithSource: Failed "
                                << "to create the program. Test failed\n";
    iRet = clBuildProgram(clProg, 0, nullptr, "", nullptr, nullptr);

    // get the log
    size_t szLogSize = 0;
    iRet = clGetProgramBuildInfo(clProg, device, CL_PROGRAM_BUILD_LOG, 0,
                                 nullptr, &szLogSize);
    EXPECT_EQ(CL_SUCCESS, iRet) << "clGetProgramBuildInfo: No build info. "
                                << "Test failed\n";

    std::string strLog("", szLogSize);
    iRet = clGetProgramBuildInfo(clProg, device, CL_PROGRAM_BUILD_LOG,
                                 szLogSize, &strLog[0], &szLogSize);
    EXPECT_EQ(CL_SUCCESS, iRet) << "clGetProgramBuildInfo: No build info. "
                                << "Test failed\n";

    // check the logs
    size_t isLog = strLog.find(kernelVec[i].kernelName, 0);

    // Here we check if there are any of expected strings in the logs
    // and are they corresponding properly to the written test cases.
    // For example we expect to see for vec_len_hint(8) "kernel was
    // successfully vectorized (8)", for vec_len_hint(1) - "kernel
    // wasn't vectorized" and for vec_len_hint(7) we should get
    // a compilation error.
    if (kernelVec[i].expectedRes) {
      EXPECT_NE(isLog, std::string::npos) << "ERROR: Cannot find log about "
                                          << kernelVec[i].kernelName << "!\n";

      if (strLog.find(warningString, 0) != std::string::npos &&
          strLog.find(vectorizedString, 0) == std::string::npos)
        continue;

      std::string tmp = "(" + kernelVec[i].vectorizationTo + ")";
      EXPECT_FALSE((strLog.find(vectorizedString, 0) == std::string::npos ||
                    strLog.find(tmp, 0) == std::string::npos) &&
                   tmp != "(0)")
          << "ERROR: " << kernelVec[i].kernelName
          << " was supposed to be vectorized to " << tmp
          << " - log indicates otherwise!\n"
          << strLog << "\n";
    } else {
      if (strLog.find(expectedError) != std::string::npos)
        continue;

      EXPECT_FALSE(strLog.find(notVectorizedString, 0) == std::string::npos)
          << "ERROR: " << kernelVec[i].kernelName << " was not supposed "
          << "to be vectorized - log indicates otherwise!\n"
          << strLog << "\n";
    }
    clReleaseProgram(clProg);
  }

  // Release objects
  kernelVec.clear();
  clReleaseContext(context);
}
