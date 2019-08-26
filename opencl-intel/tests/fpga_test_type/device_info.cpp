//===--- device_info.cpp -                                      -*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //
//
// Internal tests for platform and device information correctness
//
// ===--------------------------------------------------------------------=== //

#include "base_fixture.h"

#include <CL/cl.h>
#include <gtest/gtest.h>

class TestInfo : public OCLFPGABaseFixture {
protected:
  void checkPlatform(cl_platform_info info, const char *ref) {
    char m_str[1024];
    int err =
        clGetPlatformInfo(platform(), info, sizeof(m_str), m_str, nullptr);
    ASSERT_EQ(CL_SUCCESS, err);
    ASSERT_STREQ(ref, m_str);
  }

  void checkDevice(cl_device_id device, cl_device_info info, const char *ref) {
    char m_str[1024];
    int err = clGetDeviceInfo(device, info, sizeof(m_str), m_str, nullptr);
    ASSERT_EQ(CL_SUCCESS, err);
    ASSERT_STREQ(ref, m_str);
  }
};

TEST_F(TestInfo, Platform) {
  checkPlatform(CL_PLATFORM_PROFILE, "EMBEDDED_PROFILE");
  checkPlatform(CL_PLATFORM_VERSION, "OpenCL 1.2 Intel(R) FPGA SDK for OpenCL(TM), Version 19.2");
  checkPlatform(CL_PLATFORM_NAME,    "Intel(R) FPGA Emulation Platform for OpenCL(TM) (preview)");
  checkPlatform(CL_PLATFORM_VENDOR,  "Intel(R) Corporation");
}

TEST_F(TestInfo, Device) {
  for (auto device : devices()) {
    checkDevice(device, CL_DEVICE_PROFILE,          "EMBEDDED_PROFILE");
    checkDevice(device, CL_DEVICE_VERSION,          "OpenCL 1.2 ");
    checkDevice(device, CL_DEVICE_NAME,             "Intel(R) FPGA Emulation Device (preview)");
    checkDevice(device, CL_DEVICE_VENDOR,           "Intel(R) Corporation");
    checkDevice(device, CL_DEVICE_OPENCL_C_VERSION, "OpenCL C 1.2 ");
    checkDevice(device, CL_DEVICE_EXTENSIONS,
        "cl_khr_icd cl_khr_byte_addressable_store cl_intel_fpga_host_pipe cles_khr_int64 cl_khr_il_program cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics ");

    cl_device_type type;
    cl_int err =
        clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(type), &type, nullptr);
    ASSERT_EQ(CL_SUCCESS, err);
    ASSERT_EQ(CL_DEVICE_TYPE_ACCELERATOR, type);

    cl_uint vendorId;
    err = clGetDeviceInfo(device, CL_DEVICE_VENDOR_ID, sizeof(vendorId),
                          &vendorId, nullptr);
    ASSERT_EQ(CL_SUCCESS, err);
    ASSERT_EQ(4466, vendorId);
  }
}

