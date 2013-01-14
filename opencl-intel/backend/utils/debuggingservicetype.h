/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
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
