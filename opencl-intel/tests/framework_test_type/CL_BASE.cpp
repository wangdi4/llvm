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

#include "CL_BASE.h"
#include "TestsHelpClasses.h"
#include "common_utils.h"

void CL_base::SetUp() {
  // Get platform.
  cl_int err = clGetPlatformIDs(1, &m_platform, NULL);
  ASSERT_EQ(CL_SUCCESS, err) << " clGetPlatformIDs failed.";

  // Check OpenCL 2.1 availability.
  size_t platVerSize = 0;
  err =
      clGetPlatformInfo(m_platform, CL_PLATFORM_VERSION, 0, NULL, &platVerSize);
  ASSERT_EQ(CL_SUCCESS, err) << " clGetPlatformInfo failed on trying to obtain "
                                "CL_PLATFORM_VERSION string's size.";
  ASSERT_NE((size_t)0, platVerSize)
      << " CL_PLATFORM_VERSION string's size is 0 ";

  std::string platVer("", platVerSize);
  err = clGetPlatformInfo(m_platform, CL_PLATFORM_VERSION, platVer.size(),
                          &platVer[0], NULL);
  ASSERT_EQ(CL_SUCCESS, err) << " clGetPlatformInfo failed on trying to obtain "
                                "CL_PLATFORM_VERSION string.";
  if (!platVer.compare(0, 10, "OpenCL 3.0"))
    m_version = OPENCL_VERSION::OPENCL_VERSION_3_0;
  else if (!platVer.compare(0, 10, "OpenCL 2.1"))
    m_version = OPENCL_VERSION::OPENCL_VERSION_2_1;
  else if (!platVer.compare(0, 10, "OpenCL 2.0"))
    m_version = OPENCL_VERSION::OPENCL_VERSION_2_0;
  else if (!platVer.compare(0, 10, "OpenCL 1.2"))
    m_version = OPENCL_VERSION::OPENCL_VERSION_1_2;
  else if (!platVer.compare(0, 10, "OpenCL 1.0"))
    m_version = OPENCL_VERSION::OPENCL_VERSION_1_0;
  else
    FAIL() << "Unexpected OpenCL platform version " << platVer;

  // Get device.
  err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL);
  ASSERT_EQ(CL_SUCCESS, err) << " clGetDeviceIDs failed on trying to obtain "
                             << gDeviceType << " device type.";

  // Create context.
  const cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                         (cl_context_properties)m_platform, 0};
  m_context = clCreateContext(prop, 1, &m_device, NULL, NULL, &err);
  ASSERT_OCL_SUCCESS(err, " clCreateContext");

  cl_command_queue_properties properties[] = {CL_QUEUE_PROPERTIES,
                                              CL_QUEUE_PROFILING_ENABLE, 0};
  m_queue =
      clCreateCommandQueueWithProperties(m_context, m_device, properties, &err);
  ASSERT_OCL_SUCCESS(err, "clCreateCommandQueueWithProperties");
}

void CL_base::TearDown() {
  if (m_queue)
    ASSERT_OCL_SUCCESS(clReleaseCommandQueue(m_queue), "clReleaseCommandQueue");
  if (m_context)
    ASSERT_OCL_SUCCESS(clReleaseContext(m_context), "clReleaseContext");
}

void CL_base::GetSimpleSPIRV(std::vector<char> &spirv) const {
  std::string filename = get_exe_dir() + "test.spv";
  std::fstream spirv_file(filename, std::fstream::in | std::fstream::binary |
                                        std::fstream::ate);
  ASSERT_TRUE(spirv_file) << " Error while opening " << filename << " file. ";

  size_t length = spirv_file.tellg();
  spirv_file.seekg(0, spirv_file.beg);

  spirv.resize(length, 0);
  spirv_file.read(&spirv[0], length);
  ASSERT_TRUE(spirv_file) << "Error in reading " << filename;
}
