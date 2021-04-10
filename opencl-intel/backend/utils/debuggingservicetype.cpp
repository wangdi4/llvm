// INTEL CONFIDENTIAL
//
// Copyright 2012-2021 Intel Corporation.
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

#include "debuggingservicetype.h"
#include "CompilationUtils.h"
#include "cl_config.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"

using Intel::OpenCL::DeviceBackend::CompilationUtils;

namespace intel {

DebuggingServiceType getUserDefinedDebuggingServiceType() {
  DebuggingServiceType serviceType = None;
  using Intel::OpenCL::Utils::ConfigFile;
#if _WIN32
  // User can set environment variable to define the service type.
  const char *var = getenv("CL_CONFIG_USE_NATIVE_DEBUGGER");
  if (var)
    serviceType =
        ConfigFile::ConvertStringToType<bool>(var) ? Native : Simulator;
#else
  // CL_CONFIG_DBG_ENABLE == True implies Simulator debugging.
  const char *var = getenv("CL_CONFIG_DBG_ENABLE");
  if (var && ConfigFile::ConvertStringToType<bool>(var))
    serviceType = Simulator;
#endif
  return serviceType;
}

DebuggingServiceType getDebuggingServiceType(bool debuggingEnabled,
                                             llvm::Module *M,
                                             bool useNativeDebugger) {
  if (!debuggingEnabled && !CompilationUtils::getDebugFlagFromMetadata(M)) {
    return None;
  }

  DebuggingServiceType serviceType;
#if _WIN32
    if (useNativeDebugger)
        serviceType = Native;
    else
        serviceType = Simulator;
#else
    // The parameter is used in Windows code
    (void)useNativeDebugger;

    // CL_CONFIG_DBG_ENABLE != 1 or unset implies native (GDB) debugging.
    serviceType = Native;
#endif

    // Allow the environment variable to override the flag choice.
    DebuggingServiceType userOverride = getUserDefinedDebuggingServiceType();
    if (userOverride != None) {
      serviceType = userOverride;
    }


    return serviceType;
}
}
