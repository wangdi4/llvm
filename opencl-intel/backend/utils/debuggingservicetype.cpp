/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#include <stdlib.h>
#include <string>
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

    if (val && std::string(val) == "1") {
        return Simulator;
    }

    // CL_CONFIG_DBG_ENABLE != 1 or unset implies native (GDB) debugging
    return Native;
#endif
}

}
