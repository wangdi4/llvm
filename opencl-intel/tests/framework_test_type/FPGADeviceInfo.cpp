//==--- FPGADeviceInfo.cpp - Tests for host-side pipes.  -*- OpenCL C -*-=====//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include <gtest/gtest.h>
#include <CL/cl.h>
#include <CL/cl_ext.h>

#include "FrameworkTest.h"

#include <vector>

extern cl_device_type gDeviceType;

class FPGADeviceInfo : public ::testing::Test {
protected:
    virtual void SetUp();
    virtual void TearDown();

    void checkPlatform(cl_platform_info info, const char* ref);
    void checkDevice(cl_device_id device, cl_device_info info, const char* ref);

    cl_platform_id            m_platform;
    std::vector<cl_device_id> m_devices;
    char                      m_str[1024];
};

void FPGADeviceInfo::SetUp()
{
    cl_int iRet = CL_SUCCESS;
    iRet = clGetPlatformIDs(/*num_entries=*/1, &m_platform,
                            /*num_platforms=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet) << "clGetPlatformIDs failed.";

    cl_uint numDevices = 0;
    iRet = clGetDeviceIDs(m_platform, gDeviceType, 0, nullptr, &numDevices);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << "clGetDeviceIDs failed on trying to obtain number of devices of "
        << gDeviceType << " device type.";
    ASSERT_GE(numDevices, 1)
        << "No devices of " << gDeviceType << " device type found.";

    m_devices.resize(numDevices);
    iRet = clGetDeviceIDs(m_platform, gDeviceType, /*num_entries=*/numDevices,
                          &m_devices.front(), /*num_devices=*/nullptr);
    ASSERT_EQ(CL_SUCCESS, iRet)
        << "clGetDeviceIDs failed on trying to obtain "
        << gDeviceType << " device type.";
}

void FPGADeviceInfo::TearDown()
{
    for (cl_device_id &device : m_devices)
        clReleaseDevice(device);
}

void FPGADeviceInfo::checkPlatform(cl_platform_info info, const char* ref)
{
    int err = clGetPlatformInfo(m_platform, info,
                                sizeof(m_str), m_str, nullptr);
    ASSERT_EQ(CL_SUCCESS, err);
    ASSERT_STREQ(ref, m_str);
}

void FPGADeviceInfo::checkDevice(cl_device_id device, cl_device_info info,
                                 const char *ref)
{
    int err = clGetDeviceInfo(device, info, sizeof(m_str), m_str, nullptr);
    ASSERT_EQ(CL_SUCCESS, err);
    ASSERT_STREQ(ref, m_str);
}

#ifdef BUILD_FPGA_EMULATOR

TEST_F(FPGADeviceInfo, Platform)
{
    checkPlatform(CL_PLATFORM_PROFILE,  "EMBEDDED_PROFILE");
    checkPlatform(CL_PLATFORM_VERSION,  "OpenCL 1.0 Intel(R) FPGA SDK for OpenCL(TM), Version 18.0");
    checkPlatform(CL_PLATFORM_NAME,     "Intel(R) FPGA Emulation Platform for OpenCL(TM) (preview)");
    checkPlatform(CL_PLATFORM_VENDOR,   "Intel(R) Corporation");
}

TEST_F(FPGADeviceInfo, Device)
{
    for (cl_device_id &device : m_devices)
    {
        checkDevice(device, CL_DEVICE_PROFILE,           "EMBEDDED_PROFILE");
        checkDevice(device, CL_DEVICE_VERSION,           "OpenCL 1.0 ");
        checkDevice(device, CL_DEVICE_NAME,              "Intel(R) FPGA Emulation Device (preview)");
        checkDevice(device, CL_DEVICE_VENDOR,            "Intel(R) Corporation");
        checkDevice(device, CL_DEVICE_OPENCL_C_VERSION,  "OpenCL C 1.0 ");
        checkDevice(device, CL_DRIVER_VERSION,           "18.0");

        cl_device_type type;
        cl_int err = clGetDeviceInfo(device, CL_DEVICE_TYPE,
                                     sizeof(type), &type, nullptr);
        ASSERT_EQ(CL_SUCCESS, err);
        ASSERT_EQ(CL_DEVICE_TYPE_ACCELERATOR, type);

        cl_uint vendorId;
        err = clGetDeviceInfo(device, CL_DEVICE_VENDOR_ID,
                              sizeof(vendorId), &vendorId, nullptr);
        ASSERT_EQ(CL_SUCCESS, err);
        ASSERT_EQ(4466, vendorId);
    }
}

#endif // BUILD_FPGA_EMULATOR
