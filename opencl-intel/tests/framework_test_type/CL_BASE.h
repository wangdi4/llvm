// INTEL CONFIDENTIAL
//
// Copyright 2015-2020 Intel Corporation.
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

#ifndef __CL_BASE__
#define __CL_BASE__

#include "CL/cl.h"
#include "cl_config.h"
#include "gtest_wrapper.h"

using Intel::OpenCL::Utils::OPENCL_VERSION;

extern cl_device_type gDeviceType;

class CL_base : public testing::Test {
public:
  CL_base()
      : m_platform(nullptr), m_device(nullptr), m_context(nullptr),
        m_queue(nullptr), m_version(OPENCL_VERSION::OPENCL_VERSION_UNKNOWN) {}

protected:
  virtual void SetUp() override;

  virtual void TearDown() override;

  void GetSimpleSPIRV(std::vector<char> &spirv) const;

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
  cl_context m_context;
  cl_command_queue m_queue;
  OPENCL_VERSION m_version;
};

#endif /*__CL_BASE__*/
