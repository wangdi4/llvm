// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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
  const char *sExtensionStrings;    // A string for device supported extensions
  bool bImageSupport;               // Does the device support images?
  bool bDoubleSupport;              // Does the device support 64 bit FP?
  bool bEnableSourceLevelProfiling; // Enable source level profiling
  bool bIsFPGAEmu;                  // Is this FPGA emu device?
};

} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel
