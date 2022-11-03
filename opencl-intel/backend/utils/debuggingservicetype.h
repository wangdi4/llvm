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

#ifndef DEBUGGINGSERVICETYPE_H
#define DEBUGGINGSERVICETYPE_H

namespace llvm {
class Module;
}

namespace intel {

enum DebuggingServiceType { None, Native, Simulator };

/// Returns the debugging service defined by the user via an environment
/// variable which toggles the Simulator/Native debugging type:
///   Windows:      CL_CONFIG_USE_NATIVE_DEBUGGER
///   Linux/other:  CL_CONFIG_DBG_ENABLE
DebuggingServiceType getUserDefinedDebuggingServiceType();

/// Returns the debugging service to use based on the enabled flag
/// (corresponding to the "-g" build option users can enable and the environment
/// variable for toggling the debugging type)
DebuggingServiceType getDebuggingServiceType(bool debuggingEnabled,
                                             llvm::Module *M,
                                             bool useNativeDebugger);

} // namespace intel

#endif // DEBUGGINGSERVICETYPE_H
