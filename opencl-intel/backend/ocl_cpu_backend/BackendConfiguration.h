// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

#pragma once
#include "CompilerConfig.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class ICLDevBackendOptions;

//*****************************************************************************
// Represents the global backend configuration.
// It is a singletinon that must be initialized explicitly.
class BackendConfiguration
{
public:
    /**
     * Status initialization. Must be called once, in single threaded
     * environment.
     */
    static void Init();
    /**
     * Termination. Must be called once, in single threaded environment
     */
    static void Terminate();
    /**
     * Singleton instance getter.
     */
    static const BackendConfiguration& GetInstance();
    /**
     * Returns the global compiler configuration.
     */
    GlobalCompilerConfig GetGlobalCompilerConfig( const ICLDevBackendOptions* pBackendOptions ) const;
    /**
     * Returns the CPU compiler instance configuration.
     */
    CompilerConfig GetCPUCompilerConfig(const ICLDevBackendOptions* pBackendOptions) const;

private:
    BackendConfiguration(){}
    ~BackendConfiguration(){}

    static BackendConfiguration* s_pInstance;
};

}}}
