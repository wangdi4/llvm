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

#include <stdlib.h>
#include <string>
#include "debuggingservicetype.h"
#include "CompilationUtils.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"

using Intel::OpenCL::DeviceBackend::CompilationUtils;

namespace intel {

DebuggingServiceType getDebuggingServiceType(bool debuggingEnabled,
                                             llvm::Module *M,
                                             bool useNativeDebugger) {
    if (!debuggingEnabled &&
        !CompilationUtils::getDebugFlagFromMetadata(M)) {
        return None;
    }

    DebuggingServiceType serviceType;
#if _WIN32
    if (useNativeDebugger)
        serviceType = Native;
    else
        serviceType = Simulator;

    // Allow the environment variable to override the flag choice.
    const char *val = getenv("CL_CONFIG_USE_NATIVE_DEBUGGER");
    if (val) {
        if (std::string(val) == "1")
            serviceType = Native;
        else
            serviceType = Simulator;
    }
#else
    // CL_CONFIG_DBG_ENABLE != 1 or unset implies native (GDB) debugging.
    serviceType = Native;

    // CL_CONFIG_DBG_ENABLE == 1 implies Simulator debugging
    const char* val = getenv("CL_CONFIG_DBG_ENABLE");
    if (val && std::string(val) == "1") {
        serviceType = Simulator;
    }
#endif

    return serviceType;
}

}
