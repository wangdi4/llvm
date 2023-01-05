// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef HOST_PROGRAM_COMMON_H
#define HOST_PROGRAM_COMMON_H

#include "CL/cl.h"

// Use the OpenCL C++ bindings, with exceptions enabled. For MSVC, disable
// warning 4290 (C++ exception specifications ignored) that's emitted from
// CL/cl.hpp
//
#define __CL_ENABLE_EXCEPTIONS
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4290)
#endif // _MSC_VER

// Disable warning 'deprecated-declarations' emitted from CL/cl.hpp
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include "CL/cl.hpp"

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include <iostream>
#include <string>
#include <vector>

#define DTT_LOG_ON

inline void DTT_LOG(const std::string &s) {
#ifdef DTT_LOG_ON
  std::cerr << s << std::endl;
#endif // DTT_LOG_ON
}

// Extra arguments passed to host programs from the command-line.
//
typedef std::vector<std::string> HostProgramExtraArgs;

// Host program functions have this signature
//
typedef void (*HostProgramFunc)(cl::Context context, cl::Device device,
                                cl::Program program,
                                HostProgramExtraArgs extra_args);

#endif // HOST_PROGRAM_COMMON_H
