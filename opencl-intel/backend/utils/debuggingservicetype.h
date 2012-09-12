#ifndef DEBUGGINGSERVICETYPE_H
#define DEBUGGINGSERVICETYPE_H

namespace intel {

enum DebuggingServiceType { None, Native, Simulator };

/// Returns the debugging service to use based on the enabled flag (corresponding
/// to the "-g" build option users can enable and the environment variable
/// CL_CONFIG_DBG_ENABLE (which toggles the Simulator debugging type)
DebuggingServiceType getDebuggingServiceType(bool debuggingEnabled);

}

#endif // DEBUGGINGSERVICETYPE_H
