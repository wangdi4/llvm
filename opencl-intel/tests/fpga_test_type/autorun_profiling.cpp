//===--- autorun_profiling.cpp -                                -*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for Autorun profiling API
//
// ===--------------------------------------------------------------------=== //

#include "CL/cl.h"
#include "base_fixture.h"
#include "gtest_wrapper.h"
#include "pretty_printers.h"

#include <algorithm>
#include <cstdlib>

class TestAutorunProfilerAPI : public OCLFPGABaseFixture {};

TEST_F(TestAutorunProfilerAPI, Basic) {
  cl_int (*get_profile_data_device)(cl_device_id, cl_program, cl_bool, cl_bool,
                                    cl_bool, size_t, void *, size_t *,
                                    cl_int *);
  get_profile_data_device =
      (cl_int(*)(cl_device_id, cl_program, cl_bool, cl_bool, cl_bool, size_t,
                 void *, size_t *, cl_int *))
          clGetExtensionFunctionAddress("clGetProfileDataDeviceIntelFPGA");
  ASSERT_NE(nullptr, get_profile_data_device)
      << " clGetExtensionFunctionAddress(\"clGetProfileDataDeviceIntelFPGA\") "
         "failed";

  cl_int error = CL_SUCCESS;
  get_profile_data_device(/*device_id*/ device(),
                          /*program*/ nullptr,
                          /*read_enqueue_kernels*/ false,
                          /*read_auto_kernels*/ true,
                          /*clear_counters_after_readback*/ false,
                          /*param_value_size*/ 0,
                          /*param_value*/ nullptr,
                          /*param_value_size_ret*/ nullptr,
                          /*errcode_ret*/ &error);
  ASSERT_EQ(CL_INVALID_PROGRAM, error)
      << " clGetProfileDataDeviceIntelFPGA returns unexpected result: "
      << ErrToStr(error);

  get_profile_data_device(/*device_id*/ device(),
                          /*program*/ (cl_program)0xdeadbeef,
                          /*read_enqueue_kernels*/ false,
                          /*read_auto_kernels*/ true,
                          /*clear_counters_after_readback*/ false,
                          /*param_value_size*/ 0,
                          /*param_value*/ nullptr,
                          /*param_value_size_ret*/ nullptr,
                          /*errcode_ret*/ &error);
  ASSERT_EQ(CL_INVALID_DEVICE, error)
      << " clGetProfileDataDeviceIntelFPGA returns unexpected result: "
      << ErrToStr(error);
}
