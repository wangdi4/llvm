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

#include "cl_env.h"
#include "debuggingservicetype.h"

namespace intel {

DebuggingServiceType getDebuggingServiceType(bool debuggingEnabled)
{
    if (!debuggingEnabled) {
        return None;
    }

    // CL_CONFIG_DBG_ENABLE == 1 implies Simulator debugging, no env var
    // implies choice based on OS. Once native debugging is supported on
    // Windows, this environment variable can go away.
#if _WIN32
    return Simulator;
#else

    const char* val = getenv("CL_CONFIG_DBG_ENABLE");

    if (val && string(val) == "1") {
        return Simulator;
    }

    // CL_CONFIG_DBG_ENABLE != 1 or unset implies native (GDB) debugging
    return Native;
#endif
}

}
