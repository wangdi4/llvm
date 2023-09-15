// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

///////////////////////////////////////////////////////////
//  clang_device_info.h
///////////////////////////////////////////////////////////

#pragma once

namespace Intel {
namespace OpenCL {
namespace ClangFE {

struct CLANG_DEV_INFO {
  // A string for device supported extensions
  const char *sExtensionStrings = nullptr;
  // A string for OpenCL 3.0 feature macros
  const char *sOpenCLCFeatureStrings = nullptr;
  // Does the device support images?
  bool bImageSupport = false;
  // Does the device support 16 bit FP?
  bool bHalfSupport = false;
  // Does the device support 64 bit FP?
  bool bDoubleSupport = false;
  // Enable source level profiling
  bool bEnableSourceLevelProfiling = false;
  // Is this FPGA emu device?
  bool bIsFPGAEmu = false;
};

} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel
