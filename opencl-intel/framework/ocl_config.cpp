// INTEL CONFIDENTIAL
//
// Copyright 2008 Intel Corporation.
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

#include "ocl_config.h"
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

// First device lib is a default device lib
#if defined(_WIN32)
#if defined(_M_X64)
static const char *DEFAULT_DEVICES_LIB_LIST = "cpu_device64";
#else  //_M_X64
static const char *DEFAULT_DEVICES_LIB_LIST = "cpu_device32";
#endif //_M_X64
#else  // _WIN32
static const char *DEFAULT_DEVICES_LIB_LIST = "cpu_device";
#endif // _WIN32

OCLConfig::OCLConfig() {
  // BasicCLConfigWrapper
}

OCLConfig::~OCLConfig() {
  // ~BasicCLConfigWrapper
}

string OCLConfig::GetDefaultDevice() const {
  vector<string> vectDevices = GetDevices();
  return (vectDevices.size() > 0) ? vectDevices[0] : "";
}

vector<string> OCLConfig::GetDevices() const {
  vector<string> vectDevices;
  string s = DEFAULT_DEVICES_LIB_LIST;
  if (GetDeviceMode() == FPGA_EMU_DEVICE)
    s += OUTPUT_EMU_SUFF;
  ConfigFile::tokenize(s, vectDevices);
  return vectDevices;
}