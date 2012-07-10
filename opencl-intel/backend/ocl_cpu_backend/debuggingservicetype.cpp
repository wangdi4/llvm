
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
