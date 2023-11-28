// INTEL CONFIDENTIAL
//
// Copyright 2022 Intel Corporation.
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

// The test is mostly written as it was provided by user
// with modifications (a bit) to integrate it to our test system.

// Currently, it's not implemented for Windows platform due to
// linker's problem.

#ifndef _WIN32

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200
#define TBB_PREVIEW_GLOBAL_CONTROL 1

#include "CL/opencl.hpp"
#include "common_utils.h"
#include "tbb/global_control.h"
#include <cassert>
#include <iostream>
#include <string>

namespace {
// Find a device where either the device or platform name contains the filter
// string
cl::Device find_matching(const std::string &filter) {
  std::vector<cl::Platform> platforms;
  cl::Platform::get(&platforms);

  for (auto &platform : platforms) {
    const std::string pname{platform.getInfo<CL_PLATFORM_NAME>()};
    const bool pmatch = (pname.find(filter) < std::string::npos);

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    for (auto &device : devices) {
      const std::string dname{device.getInfo<CL_DEVICE_NAME>()};

      if (pmatch || (dname.find(filter) < std::string::npos)) {
        std::cout << "Using " << pname << ":" << dname << "\n";
        return device;
      }
    }
  }

  // No recovery
  assert(false);
  return cl::Device();
}
} // namespace

bool TbbSetMaxThreads(int NumThreads) {
  // Usage:
  // Run with no args -> success
  // Run with num threads set to device maximum -> success
  // Run with any other number of threads -> context constructor hangs

  const std::string filter{"Intel"};
  const cl::Device device{find_matching(filter)};

  size_t nthreads = NumThreads;

  // W/O for FPGA: Set OCL_TBB_NUM_WORKERS to default test's number of threads
  const std::string dname{device.getInfo<CL_DEVICE_NAME>()};
  if (dname.find("FPGA") != std::string::npos)
    SETENV("OCL_TBB_NUM_WORKERS", std::to_string(nthreads).c_str());
  std::cout << "Setting max threads to " << nthreads << "\n";
  auto controller = tbb::global_control{
      tbb::global_control::max_allowed_parallelism, nthreads};

  std::cout << "About to construct context\n";
  cl::Context context{device};
  std::cout << "Done\n";

  return true;
}

#endif
