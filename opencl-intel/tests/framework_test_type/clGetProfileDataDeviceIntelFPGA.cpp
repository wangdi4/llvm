//==--- clGetProfileDataDeviceIntelFPGA.cpp - Test for the function.
//-*- OpenCL C -*-==//
////
//// Copyright (C) 2018 Intel Corporation. All rights reserved.
////
//// The information and source code contained herein is the exclusive property
//// of Intel Corporation and may not be disclosed, examined or reproduced in
//// whole or in part without explicit written authorization from the company.
////
//// ===--------------------------------------------------------------------===
//////

#include <gtest/gtest.h>
#include <CL/cl.h>

#include <algorithm>
#include <cstdlib>

#include "FrameworkTest.h"

extern cl_device_type gDeviceType;

void clGetProfileDataDeviceIntelFPGATest() {
  cl_platform_id platform;
  cl_device_id device;

  cl_int iRet = clGetPlatformIDs(1, &platform, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << " clGetPlatformIDs = " << ClErrTxt(iRet);

  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, nullptr);
  ASSERT_EQ(CL_SUCCESS, iRet) << "clGetDeviceIDs = " << ClErrTxt(iRet);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
  cl_int (*get_profile_data_device) (cl_device_id, cl_program, cl_bool, cl_bool,
                                     cl_bool, size_t, void *, size_t *,
                                     cl_int *);
  get_profile_data_device =
    (cl_int (*) (cl_device_id, cl_program, cl_bool, cl_bool, cl_bool, size_t,
                 void *, size_t *, cl_int *))
        clGetExtensionFunctionAddress("clGetProfileDataDeviceIntelFPGA");
  ASSERT_TRUE(nullptr != get_profile_data_device)
      << " clGetExtensionFunctionAddress(\"clGetProfileDataDeviceIntelFPGA\") "
         "failed";
#pragma GCC diagnosic pop

  get_profile_data_device(/*device_id*/ device,
                          /*program*/ nullptr,
                          /*read_enqueue_kernels*/ false,
                          /*read_auto_kernels*/true,
                          /*clear_counters_after_readback*/ false,
                          /*param_value_size*/ 0,
                          /*param_value*/ nullptr,
                          /*param_value_size_ret*/ nullptr,
                          /*errcode_ret*/ &iRet);
  ASSERT_EQ(CL_INVALID_PROGRAM, iRet) <<
    " clGetProfileDataDeviceIntelFPGA returns unexpected result.";

  get_profile_data_device(/*device_id*/ device,
                          /*program*/ (cl_program)0xdeadbeef,
                          /*read_enqueue_kernels*/ false,
                          /*read_auto_kernels*/true,
                          /*clear_counters_after_readback*/ false,
                          /*param_value_size*/ 0,
                          /*param_value*/ nullptr,
                          /*param_value_size_ret*/ nullptr,
                          /*errcode_ret*/ &iRet);
  ASSERT_EQ(CL_INVALID_DEVICE, iRet) <<
    " clGetProfileDataDeviceIntelFPGA returns unexpected result.";

  clReleaseDevice(device);
}
