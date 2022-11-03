// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
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

#include "cl_sys_info.h"

#include <driverversion.h>

// OCL product version string will change with each commit, it will thwart
// build-same check. So we put it in a specific section and then ignore this
// section when doing build-same check.
#ifdef _WIN32
#pragma section(".oclver", read)
__declspec(allocate(".oclver"))
#else
__attribute__((section(".oclver")))
#endif
    const char OCL_VER_EXT[] = VERSIONSTRING_WITH_EXT;

////////////////////////////////////////////////////////////////////////////////
// return the product version:
// Arguments - year, LLVM version, month, digit (0) - output version numbers
///////////////////////////////////////////////////////////////////////////////
const char *Intel::OpenCL::Utils::GetModuleProductVersion() {
  return OCL_VER_EXT;
}
